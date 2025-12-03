/*
 * uart_transport.c
 *
 *  Created on: Dec 3, 2025
 *      Author: agapitov
 */

#include "uart_transport.h"

int32_t uart_write(void *handle, uint8_t *data, uint16_t len, uint32_t timeout)
{
  HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_SET);   // RS485 TX
  HAL_StatusTypeDef st = HAL_UART_Transmit((UART_HandleTypeDef*)handle, data, len, timeout);
  HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_RESET); // RS485 RX
  return (st == HAL_OK) ? 0 : -1;
}

int32_t uart_read(void *handle, uint8_t *data, uint16_t len, uint32_t timeout)
{
  HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_RESET); // RS485 RX
  HAL_StatusTypeDef st = HAL_UART_Receive((UART_HandleTypeDef*)handle, data, len, timeout);
  return (st == HAL_OK) ? 0 : -1;
}

void uart_printf(const char *format, ...)
{
  char temp[128];
  va_list args;

  va_start(args, format);
  int len = vsnprintf(temp, sizeof(temp), format, args);
  va_end(args);
  if(len < 0) return;

  uart_write(&huart1, (uint8_t*)temp, len, 100);
}

dev_ctx_t uart_ctx =
{
  .data_send = uart_write,
  .data_get  = uart_read,
  .mdelay    = HAL_Delay,
  .printf    = uart_printf,
  .handle    = &huart1,
  .priv_data = NULL
};

