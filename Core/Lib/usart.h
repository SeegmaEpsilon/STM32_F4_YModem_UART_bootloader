/*
 * usart.h
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Cawa
 */

#ifndef LIB_USART_H_
#define LIB_USART_H_

#include <stm32f4xx.h>

#define PRINT_PORT USART1

FlagStatus USART_GetFlagStatus(USART_TypeDef* USARTx, uint16_t USART_FLAG);
uint8_t Usart1_GetByte(uint32_t timeout);
uint32_t Usart1_RxByte(uint8_t *key);

uint32_t Send_Byte (uint8_t c);
uint32_t SerialKeyPressed(uint8_t *key);
int32_t Receive_Byte (uint8_t *c, uint32_t timeout);

#endif /* LIB_USART_H_ */
