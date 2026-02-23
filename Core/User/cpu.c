/*
 * cpu.c
 *
 *  Created on: Dec 22, 2023
 *      Author: agapitov
 */
#include "cpu.h"
#include "drivers/transport/usb_cdc_transport.h"
#include "usbd_core.h"

void HAL_DeInit_All()
{
  // Отключить используемые периферийные устройства
  HAL_UART_MspDeInit(&huart1);
  USBD_DeInit(&hUsbDeviceHS);

  // Отключить тактирование GPIO
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOA_CLK_DISABLE();

  // Отключить все тактовые генераторы периферийных устройств
  HAL_RCC_DeInit();
  // Деинитизация HAL
  HAL_DeInit();

  // Отключить SysTick
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
}

static void jump_to_app(void)
{
  HAL_DeInit_All();
  // Function pointer to the application's reset handler
  void (*app_reset_handler)(void);

  // Retrieve the stack pointer and reset handler from application's vector table
  uint32_t app_stack_pointer = *((volatile uint32_t *)APPLICATION_ADDRESS);
  uint32_t app_reset_handler_address = *((volatile uint32_t *)(APPLICATION_ADDRESS + 4U));

  // Set the MSP (Main Stack Pointer) to the application's stack pointer
  __set_MSP(app_stack_pointer);

  // Assign the reset handler address to the function pointer
  app_reset_handler = (void (*)(void))app_reset_handler_address;

  // Call the application's reset handler
  app_reset_handler();
}

void cpu(dev_ctx_t *ctx)
{
	uint8_t cmd = 0;

	const uint32_t timeout_ms = 2000;
	while(hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED);

	//Show Program Information
	ctx->printf("\r\n\r\n");
	ctx->printf("=========================\r\n");
	ctx->printf("=       BOOTLOADER      =\r\n");
	ctx->printf("=     VERSION: 1.2.0    =\r\n");
	ctx->printf("=========================\r\n");
	ctx->printf("\r\n\r\n");

	volatile uint32_t key = *(volatile uint32_t*)APPLICATION_ADDRESS;
	uint8_t byte = 0;
	if(key == 0xFFFFFFFF)
	{
		ctx->printf("Download image via UART is NOT available\r\n");
		ctx->printf("Install factory firmware...\r\n");
		ctx->data_get(ctx->handle, &byte, 1, timeout_ms);
		if(byte != '*') while(1) {};
	}
	else
	{
		ctx->printf("Download image via UART is available\r\n");
	}

	while(1)
	{
		//Show Main Menu
		ctx->printf("Press '1' to download image to the Internal Flash...\r\n");
		//Receive a byte from usart1
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
			if(flash_erase_application() == HAL_OK) ctx->printf("Flash erased successfully\r\n");
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
