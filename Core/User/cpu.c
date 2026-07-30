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

#define APP_RAM_START      0x20000000UL
#define APP_RAM_END        0x20020000UL

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

static uint8_t app_is_valid(void)
{
  uint32_t stack = *(uint32_t*)APPLICATION_ADDRESS;
  uint32_t reset = *(uint32_t*)(APPLICATION_ADDRESS + sizeof(uint32_t));
  uint32_t reset_addr = reset & ~1UL;

  if(stack < APP_RAM_START || stack > APP_RAM_END) return 0;
  if((reset & 1UL) == 0) return 0;
  if(reset_addr < APPLICATION_ADDRESS || reset_addr >= USER_FLASH_END_ADDRESS) return 0;

  return 1;
}

static void print_app_status(dev_ctx_t *ctx)
{
  if(app_is_valid()) ctx->printf("Application is valid\r\n");
  else ctx->printf("Application is invalid\r\n");
}

static void print_menu(dev_ctx_t *ctx)
{
  ctx->printf("Press '1' to download image to the Internal Flash...\r\n");
  ctx->printf("Commands: v-verify, g-go, e-erase app, r-reset\r\n");
}

static void drop_rx_line(dev_ctx_t *ctx)
{
  uint8_t b = 0;

  while(ctx->data_get(ctx->handle, &b, 1, 5) == 0)
  {
    if(b == '\r' || b == '\n') break;
  }
}

static void jump_to_app(void)
{
  typedef void (*pFunction)(void);
  uint32_t jump_address = *(uint32_t*)(APPLICATION_ADDRESS + sizeof(uint32_t));
  pFunction jump_to_application = (pFunction)jump_address;

  if(!app_is_valid()) return;

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
  uint32_t start = HAL_GetTick();
  while(hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED && HAL_GetTick() - start < timeout_ms) {};
#endif

  // Show Program Information
  ctx->printf("\r\n\r\n");
  ctx->printf("=========================\r\n");
  ctx->printf("=     F4  BOOTLOADER    =\r\n");
  ctx->printf("=     VERSION: 1.3.0    =\r\n");
  ctx->printf("=========================\r\n");
  ctx->printf("\r\n\r\n");

  if(!app_is_valid())
  {
    ctx->printf("Application is invalid\r\n");
    ctx->printf("Install firmware via %s\r\n", BOOTLOADER_INTERFACE_NAME);
  }
  else
  {
    ctx->printf("Application is valid\r\n");
  }

  print_menu(ctx);

  while(1)
  {
    cmd = 0;

    ctx->printf(".");

    if(ctx->data_get(ctx->handle, &cmd, 1, timeout_ms) != 0)
    {
      if(app_is_valid())
      {
        ctx->printf("Jump to main program by timeout...\r\n\r\n");
        jump_to_app();
      }
      continue;
    }

    if(cmd == '\r' || cmd == '\n' || cmd == ' ') continue;
    if(cmd == ':')
    {
      drop_rx_line(ctx);
      print_menu(ctx);
      continue;
    }

    switch(cmd)
    {
      case '1':
        if(download_to_flash(ctx) > 0 && app_is_valid())
        {
          ctx->printf("Jump to main program after downloading...\r\n\r\n");
          jump_to_app();
        }
        else
        {
          ctx->printf("Application is invalid\r\n");
        }
        break;

      case 'v':
        print_app_status(ctx);
        break;

      case 'g':
        if(app_is_valid())
        {
          ctx->printf("Jump to main program...\r\n\r\n");
          jump_to_app();
        }
        else
        {
          ctx->printf("Application is invalid\r\n");
        }
        break;

      case 'e':
        ctx->printf("Erase application? y/n\r\n");
        cmd = 0;
        if(ctx->data_get(ctx->handle, &cmd, 1, timeout_ms) == 0 && cmd == 'y')
        {
          ctx->printf("Start flash erasing, please wait...\r\n");
          if(flash_erase_application(USER_FLASH_SIZE) == HAL_OK) ctx->printf("Flash erased successfully\r\n");
          else ctx->printf("Flash erase failed\r\n");
          NVIC_SystemReset();
        }
        else
        {
          ctx->printf("Erase canceled\r\n");
        }
        break;

      case 'r':
        ctx->printf("System reset...\r\n");
        NVIC_SystemReset();
        break;

      default:
        continue;
    }

    print_menu(ctx);
  }
}
