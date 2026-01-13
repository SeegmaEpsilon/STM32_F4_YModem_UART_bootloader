/*
 //@brief :Data Rx and flash via Ymodem protocol
 //        (*)STM32F40x_41x IAP flash is excuted in Ymodem_Receive() function
 //        (*)Modified from STM AN395 example
 //@Author:RdMaxes
 //@Data  :2014/07/15
 //@Usage :
 //        (*)IAP_Port is defined in ymodem.h
 //        (*)This source file does not include IAP_Port initialize part
 //        (*)IAP_Port setting: USART2, 230400bps, 8, n, 1
 //@Note  :
 */

//Includes 
#include "ymodem.h"
#include "string.h"
#include "stdlib.h"
#include "crypto/crypto.h"

//extern variables
extern uint8_t file_name[FILE_NAME_LENGTH];

static int32_t Receive_Byte(dev_ctx_t *ctx, uint8_t *b, uint32_t to)
{
  return ctx->data_get(ctx->handle, b, 1, to);
}

static void Send_Byte(dev_ctx_t *ctx, uint8_t b)
{
  ctx->data_send(ctx->handle, &b, 1, 1000);
}

//Rx a packet from sender
//data: pointer to store rx data
//length: packet length
//timeout: rx time out
//return:0=normally return
//      -1=timeout or packet error
//       1=abort by user 
static int32_t Receive_Packet(dev_ctx_t *ctx, uint8_t *data, int32_t *length, uint32_t timeout)
{
  uint16_t i, packet_size;
  uint8_t c;

  *length = 0;
  if(Receive_Byte(ctx, &c, timeout) != 0)
  {
    return -1;
  }
  switch(c)
  {
    case SOH:
      packet_size = PACKET_SIZE;
      break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
      return 0;
    case CA:
      if((Receive_Byte(ctx, &c, timeout) == 0) && (c == CA))
      {
        *length = -1;
        return 0;
      }
      else
      {
        return -1;
      }
    case ABORT1:
    case ABORT2:
      return 1;
    default:
      return -1;
  }
  *data = c;
  for(i = 1; i < (packet_size + PACKET_OVERHEAD); i++)
  {
    if(Receive_Byte(ctx, data + i, timeout) != 0)
    {
      return -1;
    }
  }
  *length = packet_size;
  return 0;
}

//Receive a file using the ymodem protocol
//buf: pointer for data storage
//appaddr: User Application address
//return: size of IAP file
int32_t Ymodem_receive(dev_ctx_t *ctx, uint8_t *buf, uint32_t appaddr)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], *file_ptr, *buf_ptr, flag_EOT;
  int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
  char file_size[FILE_SIZE_LENGTH];
  uint32_t flashdestination, ramsource;

  uint8_t encrypted_mode = 0;
  uint32_t bytes_left = 0;

  //Initialize flashdestination variable
  flashdestination = appaddr;
  flag_EOT = 0;
  for(session_done = 0, errors = 0, session_begin = 0;;)
  {
    for(packets_received = 0, file_done = 0, buf_ptr = buf;;)
    {
      switch(Receive_Packet(ctx, packet_data, &packet_length, NAK_TIMEOUT))
      {
        case 0:
          errors = 0;
          switch(packet_length)
          {
            /* Abort by sender */
            case -1:
              Send_Byte(ctx, ACK);
              return 0;
              /* End of transmission */
            case 0:
              if(flag_EOT == 0) //first EOT
              {
                Send_Byte(ctx, NACK);
                flag_EOT = 1;
              }
              else if(flag_EOT == 1) //second EOT
              {
                Send_Byte(ctx, ACK);
                Send_Byte(ctx, 'C');
                file_done = 1;
              }
              break;
              /* Normal packet */
            default:
              if((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))
              {
                Send_Byte(ctx, NACK); //local data sequence number is different to rx data packet.
              }
              else
              {
                if(packets_received == 0)
                {
                  /* Filename packet */
                  if(packet_data[PACKET_HEADER] != 0)
                  {
                    /* Filename packet has valid data */
                    for(i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
                    {
                      file_name[i++] = *file_ptr++;
                    }
                    file_name[i++] = '\0';
                    for(i = 0, file_ptr++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);)
                    {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';

                    size = atoi(file_size);

                    bytes_left = (uint32_t)size;
                    encrypted_mode = 0;

                    /* Test the size of the image to be sent */
                    /* Image size is greater than Flash size */
                    if(size > (int32_t)(USER_FLASH_SIZE + 1))
                    {
                      /* End session */
                      Send_Byte(ctx, CA);
                      Send_Byte(ctx, CA);
                      return -1;
                    }
                    /* erase user application area */
                    if(flash_erase_application() != HAL_OK)
                    {
                      ctx->printf("Error flash erasing, check core's power supply\r\n");
                    }
                    Send_Byte(ctx, ACK);
                    Send_Byte(ctx, CRC16);
                  }
                  /* Filename packet is empty, end session */
                  else
                  {
                    Send_Byte(ctx, ACK);
                    file_done = 1;
                    session_done = 1;
                    break;
                  }
                }
                /* Data packet */
                else
                {
                  const char *KEY = "SWAG";
                  const uint8_t KEY_SIZE = 4;
                  const uint8_t IV_SIZE = 16;
                  const uint8_t CRYPTO_HEADER_SIZE = KEY_SIZE + IV_SIZE;

                  uint8_t *payload = packet_data + PACKET_HEADER;

                  uint32_t n = (bytes_left < (uint32_t)packet_length) ? bytes_left : (uint32_t)packet_length;
                  if (n == 0) { Send_Byte(ctx, ACK); continue; }
                  bytes_left -= n;

                  if(packets_received == 1) // first real data packet
                  {
                    if(n >= CRYPTO_HEADER_SIZE && memcmp(payload, KEY, KEY_SIZE) == 0)
                    {
                      encrypted_mode = 1;

                      secure_init(payload + KEY_SIZE);        // IV[16] - IV_SIZE

                      payload += CRYPTO_HEADER_SIZE;              // пропускаем FWEN+IV
                      n -= CRYPTO_HEADER_SIZE;
                    }
                    else
                    {
                      encrypted_mode = 0;
                    }
                  }

                  memcpy(buf_ptr, payload, n);

                  if(encrypted_mode)
                  {
                    secure_decrypt_buffer(buf_ptr, n);
                  }

                  ramsource = (uint32_t)buf_ptr;

                  /* Write received data in Flash */
                  if(flash_write(&flashdestination, (uint32_t*)ramsource, (uint16_t)n / 4) == 0)
                  {
                    flashdestination += n;
                    Send_Byte(ctx, ACK);
                  }
                  else /* An error occurred while writing to Flash memory */
                  {
                    /* End session */
                    Send_Byte(ctx, CA);
                    Send_Byte(ctx, CA);
                    return -2;
                  }
                }
                packets_received++;
                session_begin = 1;
              }
          }
          break;
        case 1:
          Send_Byte(ctx, CA);
          Send_Byte(ctx, CA);
          return -3;
        default:
          if(session_begin > 0)
          {
            errors++;
          }
          if(errors > MAX_ERRORS)
          {
            Send_Byte(ctx, CA);
            Send_Byte(ctx, CA);
            return 0;
          }
          Send_Byte(ctx, CRC16);
          break;
      }
      if(file_done != 0)
      {
        break;
      }
    }
    if(session_done != 0)
    {
      break;
    }
  }
  return (int32_t)size;
}
