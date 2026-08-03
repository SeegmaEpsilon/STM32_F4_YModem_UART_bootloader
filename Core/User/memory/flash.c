/*
 * flash.c
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */
#include "flash.h"

//Global Variables
uint8_t file_name[FILE_NAME_LENGTH]; //array to store filename of download *.bin
uint8_t buf_1k[1024] = { 0 };

static uint32_t flash_get_sector(uint32_t address)
{
  if(address < 0x08004000UL) return FLASH_SECTOR_0;
  if(address < 0x08008000UL) return FLASH_SECTOR_1;
  if(address < 0x0800C000UL) return FLASH_SECTOR_2;
  if(address < 0x08010000UL) return FLASH_SECTOR_3;
  if(address < 0x08020000UL) return FLASH_SECTOR_4;
  if(address < 0x08040000UL) return FLASH_SECTOR_5;
  if(address < 0x08060000UL) return FLASH_SECTOR_6;
  if(address < 0x08080000UL) return FLASH_SECTOR_7;
  if(address < 0x080A0000UL) return FLASH_SECTOR_8;
  if(address < 0x080C0000UL) return FLASH_SECTOR_9;
  if(address < 0x080E0000UL) return FLASH_SECTOR_10;
  return FLASH_SECTOR_11;
}

int32_t download_to_flash(dev_ctx_t *ctx)
{
  int32_t Size = 0;

  ctx->printf("  Waiting for the file to be sent ... (press 'a' to abort)\r\n");
  Size = Ymodem_receive(ctx, &buf_1k[0], APPLICATION_ADDRESS);
  if(Size > 0)
  {
    ctx->printf("--------------------------------\r\n");
    ctx->printf("Programming Completed Successfully!\r\n");
    ctx->printf("--------------------------------\r\n");
    ctx->printf("Name: %s\r\n", file_name);
    ctx->printf("Size: %d Bytes\r\n", Size);
  }
  else if(Size == -1)
  {
    ctx->printf("The image size is higher than the allowed space memory!\r\n");
  }
  else if(Size == -2)
  {
    ctx->printf("Flash erase or write failed!\r\n");
  }
  else if(Size == -3)
  {
    ctx->printf("Aborted by user.\r\n");
  }
  else
  {
    ctx->printf("Failed to receive the file!\r\n");
  }

  return Size;
}

HAL_StatusTypeDef flash_erase_area(uint32_t address, uint32_t size)
{
  uint32_t endAddress = address + size;
  if(size == 0 || endAddress <= address || address < APPLICATION_ADDRESS || endAddress > DEVICE_CONFIG_ADDRESS + DEVICE_CONFIG_SIZE)
  {
    return HAL_ERROR;
  }

  uint32_t startSector = flash_get_sector(address);
  uint32_t endSector = flash_get_sector(endAddress - 1);

  __disable_irq();
  HAL_FLASH_Unlock();

  HAL_StatusTypeDef status = HAL_ERROR;

  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t pageError;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Banks = FLASH_BANK_1;
  EraseInitStruct.NbSectors = endSector - startSector + 1;
  EraseInitStruct.Sector = startSector;

  status = HAL_FLASHEx_Erase(&EraseInitStruct, &pageError);

  HAL_FLASH_Lock();
  __enable_irq();

  return status;
}

//This function writes a data buffer in flash (data are 32-bit aligned).
//After writing data buffer, the flash content is checked.
//FlashAddress: start address for writing data buffer
//Data: pointer on data buffer
//DataLength: length of data buffer (unit is 32-bit word)
//return:
//      0=Data successfully written to Flash memory
//      1=Error occurred while writing data in Flash memory
//      2=Written Data in flash memory is different from expected one
uint32_t flash_write(__IO uint32_t *FlashAddress, uint32_t *Data, uint32_t DataLength)
{
  uint32_t flash_addr = *FlashAddress;

  __disable_irq();

  HAL_FLASH_Unlock();

  for(uint32_t i = 0; i < DataLength; i++)
  {
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_addr, *(Data + i)) != HAL_OK)
    {
      /* Error occurred while writing data in Flash memory */
      HAL_FLASH_Lock();
      __enable_irq();
      return (1);
    }

    /* Check the written value */
    volatile uint32_t data_wr = *(Data + i);
    volatile uint32_t data_rd = *((uint32_t*)flash_addr);

    if(data_wr != data_rd)
    {
      /* Flash content doesn't match SRAM content */
      HAL_FLASH_Lock();
      __enable_irq();
      return (2);
    }
    /* Increment FLASH destination address */
    flash_addr += sizeof(uint32_t);
  }

  HAL_FLASH_Lock();
  __enable_irq();
  return (0);
}
