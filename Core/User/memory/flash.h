/*
 * flash.h
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */

#ifndef LIB_FLASH_H_
#define LIB_FLASH_H_

#include "stdint.h"
#include "main.h"
#include "protocol/ymodem/ymodem.h"
#include "defines.h"

int32_t download_to_flash(dev_ctx_t *ctx);
HAL_StatusTypeDef flash_erase_area(uint32_t address, uint32_t size);
uint32_t flash_write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength);

#endif /* LIB_FLASH_H_ */
