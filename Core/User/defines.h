/*
 * defines.h
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */

#ifndef USER_DEFINES_H_
#define USER_DEFINES_H_

#define APPLICATION_ADDRESS 0x08010000UL
//End of the Flash address
#define USER_FLASH_END_ADDRESS 0x08040000UL
//Define the user application size
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - APPLICATION_ADDRESS + 1)

#endif /* USER_DEFINES_H_ */
