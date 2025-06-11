/*
 * usart.c
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */
#include "includes.h"

void USART_SendData(USART_TypeDef* USARTx, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));
  assert_param(IS_USART_DATA(Data));

  /* Transmit Data */
  USARTx->DR = (Data & (uint16_t)0x01FF);
}

FlagStatus USART_GetFlagStatus(USART_TypeDef* USARTx, uint16_t USART_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));
  assert_param(IS_USART_FLAG(USART_FLAG));

  if ((USARTx->SR & USART_FLAG) != (uint16_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

uint32_t Usart1_RxByte(uint8_t *key)
{
  if ( USART_GetFlagStatus(PRINT_PORT, UART_FLAG_RXNE) != RESET)
  {
    *key = (uint8_t)PRINT_PORT->DR;
    return 1;
  }
  else
  {
    return 0;
  }
}

uint8_t Usart1_GetByte(uint32_t timeout)
{
  uint8_t key = 0;

  /* Waiting for user input */
  while (timeout--)
  {
    if (Usart1_RxByte((uint8_t*)&key)) break;
  }
  return key;

}

//send a byte via IAP_Port
//c: byte to send
//return: 0
uint32_t Send_Byte (uint8_t c)
{
  while (USART_GetFlagStatus(IAP_Port, UART_FLAG_TXE) == RESET);
  USART_SendData(IAP_Port, c);
  while (USART_GetFlagStatus(IAP_Port, UART_FLAG_TC) == RESET);

  return 0;
}

//check if one byte rx successfully
//key: variabl to store rx data
//return: 1=success, 0=fail
uint32_t SerialKeyPressed(uint8_t *key)
{
  if ( USART_GetFlagStatus(IAP_Port, UART_FLAG_RXNE) != RESET)
  {
    *key = (uint8_t)IAP_Port->DR;
    return 1;
  }
  else
  {
    return 0;
  }
}


//Rx a byte from sender
//c: variable to store data
//timeout: read time out
//return: 0=success, -1=fail
int32_t Receive_Byte (uint8_t *c, uint32_t timeout)
{
  while (timeout-- > 0)
  {
    if (SerialKeyPressed(c) == 1)
    {
      return 0;
    }
  }
  return -1;
}
