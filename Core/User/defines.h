/*
 * defines.h
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */

#ifndef USER_DEFINES_H_
#define USER_DEFINES_H_

/* Define USE_INTERFACE_USB in compiler settings to use USB CDC instead of UART/RS485. */

#define APPLICATION_ADDRESS   0x08010000UL
#define DEVICE_CONFIG_ADDRESS 0x08020000UL

//End of the Flash address (exclusive)
#define USER_FLASH_END_ADDRESS 0x08020000UL

#define DEVICE_CONFIG_SIZE   0x20000UL
//Define the user application size
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - APPLICATION_ADDRESS)

#endif /* USER_DEFINES_H_ */
