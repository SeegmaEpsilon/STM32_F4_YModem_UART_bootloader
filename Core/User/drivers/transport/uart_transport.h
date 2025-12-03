/*
 * uart_transport.h
 *
 *  Created on: Dec 3, 2025
 *      Author: agapitov
 */

#ifndef USER_DRIVERS_TRANSPORT_UART_TRANSPORT_H_
#define USER_DRIVERS_TRANSPORT_UART_TRANSPORT_H_

#include "interface/interface.h"
#include "stdarg.h"
#include "stdio.h"
#include "main.h" // где huart1

int32_t uart_write(void *handle, uint8_t *data, uint16_t len, uint32_t timeout);
int32_t uart_read (void *handle, uint8_t *data, uint16_t len, uint32_t timeout);

extern dev_ctx_t uart_ctx;

#endif /* USER_DRIVERS_TRANSPORT_UART_TRANSPORT_H_ */
