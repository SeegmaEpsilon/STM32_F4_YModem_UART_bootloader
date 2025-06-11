/*
 * flash.h
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */

#ifndef LIB_FLASH_H_
#define LIB_FLASH_H_

void download_to_flash(void);
HAL_StatusTypeDef flash_erase_application();
uint32_t flash_write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength);

#endif /* LIB_FLASH_H_ */
