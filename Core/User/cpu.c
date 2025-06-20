/*
 * cpu.c
 *
 *  Created on: Dec 22, 2023
 *      Author: agapitov
 */
#include "includes.h"

extern UART_HandleTypeDef huart1;

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

void cpu()
{
	uint8_t cmd = 0;

	const uint32_t timeout_us = 2000000;

	user_printf_init(0x00, user_putc);
	//Show Program Information
	user_printf("\r\n\r\n");
	user_printf("\r\n=========================");
	user_printf("\r\n=     UART BOOTLOADER   =");
	user_printf("\r\n=     VERSION: 1.0.1    =");
	user_printf("\r\n=========================");
	user_printf("\r\n\r\n");

	volatile uint32_t key = *(volatile uint32_t*)APPLICATION_ADDRESS;
	if(key == 0xFFFFFFFF)
	{
		user_printf("\r\nDownload image via UART is NOT available");
		user_printf("\r\nInstall factory firmware...\r\n");
		if(Usart1_GetByte(timeout_us) != '*') while(1) {};
	}
	else
	{
		user_printf("\r\nDownload image via UART is available");
	}

	while(1)
	{
		//Show Main Menu
		user_printf("\r\nPress '1' to download image to the Internal Flash...");
		//Receive a byte from usart1
		cmd = Usart1_GetByte(timeout_us);
		if (cmd == '1')
		{
			download_to_flash();
			user_printf("\r\nJump to main program after downloading...");
			jump_to_app();
		}
		if (cmd == '#')
		{
			user_printf("\r\nStart flash erasing, please wait...");
			if(flash_erase_application() == HAL_OK) user_printf("\r\nFlash erased successfully");
			else user_printf("\r\nFlash erase failed");
			NVIC_SystemReset();
		}
    else
    {
      user_printf("\r\nJump to main program by timeout...\r\n");
      jump_to_app();
    }
	}
}
