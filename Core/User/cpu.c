/*
 * cpu.c
 *
 *  Created on: Dec 22, 2023
 *      Author: agapitov
 */
#include "cpu.h"
#ifdef USE_INTERFACE_USB
#include "drivers/transport/usb_cdc_transport.h"
#endif

#ifdef USE_INTERFACE_USB
#define BOOTLOADER_INTERFACE_NAME "USB CDC"
#else
#define BOOTLOADER_INTERFACE_NAME "UART"
#endif

static void hal_deinit_all(void)
{
  HAL_UART_DeInit(&huart1);

#ifdef USE_INTERFACE_USB
  usb_cdc_kill();
#endif

#ifdef USE_INTERFACE_USB
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
#endif
  __HAL_RCC_GPIOA_CLK_DISABLE();

  HAL_RCC_DeInit();
  HAL_DeInit();

  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
}

static void jump_to_app(void)
{
  typedef void (*pFunction)(void);
  uint32_t jump_address = *(uint32_t*)(APPLICATION_ADDRESS + sizeof(uint32_t));
  pFunction jump_to_application = (pFunction)jump_address;

  __disable_irq();

  hal_deinit_all();

  SCB->VTOR = APPLICATION_ADDRESS;
  __set_MSP(*(uint32_t*)APPLICATION_ADDRESS);
  __enable_irq();

  jump_to_application();
}

void cpu(dev_ctx_t *ctx)
{
  uint8_t cmd = 0;

  const uint32_t timeout_ms = 2000;
#ifdef USE_INTERFACE_USB
  while(hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED) {};
#endif

  // Show Program Information
  ctx->printf("\r\n\r\n");
  ctx->printf("=========================\r\n");
  ctx->printf("=       BOOTLOADER      =\r\n");
  ctx->printf("=     VERSION: 1.2.1    =\r\n");
  ctx->printf("=========================\r\n");
  ctx->printf("\r\n\r\n");

  volatile uint32_t key = *(volatile uint32_t*)APPLICATION_ADDRESS;
  uint8_t byte = 0;
  if(key == 0xFFFFFFFF)
  {
    ctx->printf("Download image via %s is NOT available\r\n", BOOTLOADER_INTERFACE_NAME);
    ctx->printf("Install factory firmware...\r\n");
    ctx->data_get(ctx->handle, &byte, 1, timeout_ms);
    if(byte != '*') while(1) {};
  }
  else
  {
    ctx->printf("Download image via %s is available\r\n", BOOTLOADER_INTERFACE_NAME);
  }

  while(1)
  {
    // Show Main Menu
    ctx->printf("Press '1' to download image to the Internal Flash...\r\n");
    ctx->data_get(ctx->handle, &cmd, 1, timeout_ms);
    if (cmd == '1')
    {
      download_to_flash(ctx);
      ctx->printf("Jump to main program after downloading...\r\n\r\n");
      jump_to_app();
    }
    if (cmd == '#')
    {
      ctx->printf("Start flash erasing, please wait...\r\n");
      if(flash_erase_application(USER_FLASH_SIZE) == HAL_OK) ctx->printf("Flash erased successfully\r\n");
      else ctx->printf("Flash erase failed\r\n");
      NVIC_SystemReset();
    }
    else
    {
      ctx->printf("Jump to main program by timeout...\r\n\r\n");
      jump_to_app();
    }
  }
}
