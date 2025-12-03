/*
 * interface.h
 *
 *  Created on: Dec 3, 2025
 *      Author: agapitov
 */

#ifndef LIB_INTERFACE_H_
#define LIB_INTERFACE_H_

#include "stdint.h"

typedef int32_t (*dev_write_ptr)(void *handle, uint8_t *data, uint16_t len, uint32_t timeout);
typedef int32_t (*dev_read_ptr)(void *handle, uint8_t *data, uint16_t len, uint32_t timeout);
typedef void (*dev_printf_ptr)(const char *fmt, ...);

typedef void (*dev_mdelay_ptr)(uint32_t ms);

typedef struct
{
  /** Component mandatory fields **/
  dev_write_ptr data_send;
  dev_read_ptr data_get;
  /** Component optional fields **/
  dev_mdelay_ptr mdelay;
  dev_printf_ptr printf;
  /** Customizable optional pointer **/
  void *handle;

  /** private data **/
  void *priv_data;
} dev_ctx_t;

#endif /* LIB_INTERFACE_H_ */
