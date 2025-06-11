/*
 * flash.c
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */
#include "includes.h"

//Global Variables
uint8_t file_name[FILE_NAME_LENGTH]; //array to store filename of download *.bin
uint8_t buf_1k[1024] ={0};

//Convert an interger to a string
//str: converted string
//intnum: integer waiting for converting
static void int_to_str(uint8_t* str, int32_t intnum)
{
  uint32_t i, Div = 1000000000, j = 0, Status = 0;

  for (i = 0; i < 10; i++)
  {
    str[j++] = (intnum / Div) + 48;

    intnum = intnum % Div;
    Div /= 10;
    if ((str[j-1] == '0') & (Status == 0))
    {
      j = 0;
    }
    else
    {
      Status++;
    }
  }
}

void download_to_flash(void)
{
  uint8_t Number[10] = "          ";
  int32_t Size = 0;

  user_printf("\n\r Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  Size = Ymodem_receive(&buf_1k[0],APPLICATION_ADDRESS);
  if (Size > 0)
  {
    user_printf("-------------------\n");
    user_printf("\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
    user_printf((char*)file_name);
    int_to_str(Number, Size);
    user_printf("\n\r Size: ");
    user_printf((char*)Number);
    user_printf(" Bytes\r\n");
  }
  else if (Size == -1)
  {
    user_printf("\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (Size == -2)
  {
    user_printf("\n\n\rVerification failed!\n\r");
  }
  else if (Size == -3)
  {
    user_printf("\r\n\nAborted by user.\n\r");
  }
  else
  {
    user_printf("\n\rFailed to receive the file!\n\r");
  }
}

HAL_StatusTypeDef flash_erase_application()
{
  __disable_irq();
  HAL_FLASH_Unlock();

  HAL_StatusTypeDef status = HAL_ERROR;

  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t pageError;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Banks = FLASH_BANK_1;
  EraseInitStruct.NbSectors = 2;
  EraseInitStruct.Sector = 4;

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
uint32_t flash_write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t flash_addr = *FlashAddress;

  __disable_irq();

  HAL_FLASH_Unlock();

  for (uint32_t i = 0; i < DataLength; i++)
  {
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_addr,  *(Data+i)) != HAL_OK)
    {
      /* Error occurred while writing data in Flash memory */
      HAL_FLASH_Lock();
      __enable_irq();
      return (1);
    }

    /* Check the written value */
    volatile uint32_t data_wr = *(Data+i);
    volatile uint32_t data_rd = *((uint32_t*)flash_addr);

    if (data_wr != data_rd)
    {
      /* Flash content doesn't match SRAM content */
      HAL_FLASH_Lock();
      __enable_irq();
      return(2);
    }
    /* Increment FLASH destination address */
    flash_addr += sizeof(uint32_t);
  }

  HAL_FLASH_Lock();
  __enable_irq();
  return (0);
}




