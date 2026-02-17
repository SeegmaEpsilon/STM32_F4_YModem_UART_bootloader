/*
 * usb_cdc_transport.c
 *
 *  Created on: Feb 17, 2026
 *      Author: Cawa
 */

#include "usb_cdc_transport.h"
#include "usbd_cdc.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef CDC_RX_RING_SIZE
#define CDC_RX_RING_SIZE 4096u
#endif

static uint8_t  s_rx_ring[CDC_RX_RING_SIZE];
static volatile uint32_t s_head = 0;
static volatile uint32_t s_tail = 0;

static inline uint32_t ring_next(uint32_t v) { return (v + 1u) % CDC_RX_RING_SIZE; }

static inline uint32_t ring_available(void)
{
  uint32_t h = s_head, t = s_tail;
  return (h >= t) ? (h - t) : (CDC_RX_RING_SIZE - (t - h));
}

static inline void crit_enter(uint32_t *primask)
{
  *primask = __get_PRIMASK();
  __disable_irq();
}

static inline void crit_exit(uint32_t primask)
{
  if (primask == 0u) __enable_irq();
}

void usb_cdc_rx_push(const uint8_t *buf, uint32_t len)
{
  for (uint32_t i = 0; i < len; ++i)
  {
    uint32_t next = ring_next(s_head);
    if (next == s_tail)
    {
      s_tail = ring_next(s_tail);
    }
    s_rx_ring[s_head] = buf[i];
    s_head = next;
  }
}

void usb_cdc_kill()
{
  USBD_Stop(&hUsbDeviceHS);
  USBD_DeInit(&hUsbDeviceHS);

  __HAL_RCC_USB_OTG_HS_FORCE_RESET();
  __HAL_RCC_USB_OTG_HS_RELEASE_RESET();
}

static USBD_CDC_HandleTypeDef* cdc_handle(USBD_HandleTypeDef *pdev)
{
  return (USBD_CDC_HandleTypeDef*)pdev->pClassData;
}

static int usb_ready(USBD_HandleTypeDef *pdev)
{
  return (pdev && pdev->dev_state == USBD_STATE_CONFIGURED && pdev->pClassData != NULL);
}

static uint32_t time_left(uint32_t t0, uint32_t timeout_ms)
{
  uint32_t now = HAL_GetTick();
  uint32_t dt  = now - t0;
  return (dt >= timeout_ms) ? 0u : (timeout_ms - dt);
}

int32_t usb_cdc_read(void *handle, uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  if (!data || len == 0) return 0;

  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef*)handle;
  uint32_t t0 = HAL_GetTick();

  uint16_t got = 0;
  while (got < len)
  {
    if (timeout_ms && time_left(t0, timeout_ms) == 0u) return -1;

    if (!usb_ready(pdev))
    {
      HAL_Delay(1);
      continue;
    }

    if (ring_available() == 0u)
    {
      HAL_Delay(1);
      continue;
    }

    uint32_t pm;
    crit_enter(&pm);
    while (got < len && s_tail != s_head)
    {
      data[got++] = s_rx_ring[s_tail];
      s_tail = ring_next(s_tail);
    }
    crit_exit(pm);
  }

  return 0;
}

int32_t usb_cdc_write(void *handle, uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  if (!data || len == 0) return 0;

  USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef*)handle;
  uint32_t t0 = HAL_GetTick();

  while (!usb_ready(pdev))
  {
    if (timeout_ms && time_left(t0, timeout_ms) == 0u) return -1;
    HAL_Delay(1);
  }

  USBD_CDC_HandleTypeDef *hcdc = cdc_handle(pdev);
  if (!hcdc) return -1;

  const uint16_t CHUNK = 256;
  uint16_t off = 0;

  while (off < len)
  {
    uint16_t n = (uint16_t)((len - off) > CHUNK ? CHUNK : (len - off));

    while (hcdc->TxState != 0u)
    {
      if (timeout_ms && time_left(t0, timeout_ms) == 0u) return -1;
      HAL_Delay(1);
    }

    USBD_CDC_SetTxBuffer(pdev, data + off, n);
    if (USBD_CDC_TransmitPacket(pdev) != USBD_OK)
    {
      HAL_Delay(1);
      continue;
    }

    off += n;
  }

  return 0;
}

static void usb_cdc_printf(const char *fmt, ...)
{
  char tmp[128];
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  if (n <= 0) return;

  (void)usb_cdc_write(&hUsbDeviceHS, (uint8_t*)tmp, (uint16_t)n, 200);
}

dev_ctx_t usb_cdc_ctx =
{
  .data_send = usb_cdc_write,
  .data_get  = usb_cdc_read,
  .mdelay    = HAL_Delay,
  .printf    = usb_cdc_printf,
  .handle    = &hUsbDeviceHS,
  .priv_data = NULL
};

