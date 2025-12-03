/*
 * cpu.c
 *
 *  Created on: Dec 22, 2023
 *      Author: agapitov
 */
#include "cpu.h"


static void jump_to_app(void)
{
	typedef  void (*pFunction)(void);
	pFunction jump_to_application;
	uint32_t  jump_address;

	__disable_irq();

	jump_address = *(uint32_t*)(APPLICATION_ADDRESS + sizeof(uint32_t));
	jump_to_application = (pFunction) jump_address;

	SCB->VTOR = APPLICATION_ADDRESS;
	__enable_irq();
	__set_MSP(*(uint32_t*)APPLICATION_ADDRESS);
	jump_to_application();
}

void cpu(dev_ctx_t *ctx)
{
	uint8_t cmd = 0;

	const uint32_t timeout_ms = 2000;

	//Show Program Information
	ctx->printf("\r\n\r\n");
	ctx->printf("=========================\r\n");
	ctx->printf("=       BOOTLOADER      =\r\n");
	ctx->printf("=     VERSION: 1.1.0    =\r\n");
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
