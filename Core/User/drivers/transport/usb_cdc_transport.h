/*
 * usb_cdc_transport.h
 *
 *  Created on: Feb 17, 2026
 *      Author: Cawa
 */

#ifndef USER_DRIVERS_TRANSPORT_USB_CDC_TRANSPORT_H_
#define USER_DRIVERS_TRANSPORT_USB_CDC_TRANSPORT_H_

#include "interface/interface.h"
#include "main.h"
#include "usbd_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void usb_cdc_kill(void);

// Вызывается из CDC_Receive_FS/HS() (usbd_cdc_if.c)
void usb_cdc_rx_push(const uint8_t *buf, uint32_t len);

int32_t usb_cdc_write(void *handle, uint8_t *data, uint16_t len, uint32_t timeout_ms);
int32_t usb_cdc_read (void *handle, uint8_t *data, uint16_t len, uint32_t timeout_ms);

extern dev_ctx_t usb_cdc_ctx;

extern USBD_HandleTypeDef hUsbDeviceHS;

#ifdef __cplusplus
}
#endif

#endif /* USER_DRIVERS_TRANSPORT_USB_CDC_TRANSPORT_H_ */
