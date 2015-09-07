/*
 * uart.h
 *
 * Created on: May 29, 2014
 *     Author: Ekawahyu Susilo
 *
 * Copyright (c) 2014, Chongqing Aisenke Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holder.
 *
 */

#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UART_CONF_STDOUT_PORT
#define UART_STDOUT_PORT UART_CONF_STDOUT_PORT
#else
#define UART_STDOUT_PORT 0
#endif

#ifdef UART_CONF_ONE_WIRE_PORT
#define UART_ONE_WIRE_PORT UART_CONF_ONE_WIRE_PORT
#else
#define UART_ONE_WIRE_PORT 1
#endif

#if (UART_STDOUT_PORT == UART_ONE_WIRE_PORT)
#error "uart.h: cannot set STDOUT and ONE_WIRE with the same port number!"
#endif

#define UART_BUFFER_SIZE        128
#define ONE_WIRE_BUFFER_SIZE    13

#if (UART_STDOUT_PORT == 0)
#define UART0_BUFFER_SIZE   UART_BUFFER_SIZE
#endif
#if (UART_STDOUT_PORT == 1)
#define UART1_BUFFER_SIZE   UART_BUFFER_SIZE
#endif
#if (UART_ONE_WIRE_PORT == 0)
#define UART0_BUFFER_SIZE   ONE_WIRE_BUFFER_SIZE
#endif
#if (UART_ONE_WIRE_PORT == 1)
#define UART1_BUFFER_SIZE   ONE_WIRE_BUFFER_SIZE
#endif

#define UART_SET_SPEED(N, M, E) do{ U##N##BAUD = M; U##N##GCR = E; } while(0)
#define UART_115200_M   216
#define UART_115200_E    11
#define UART_19200_M     59
#define UART_19200_E      9

extern unsigned char uart0_rxbuf[UART0_BUFFER_SIZE];
extern uint8_t  uart0_rxpos;
extern uint8_t  uart0_rxlen;

extern unsigned char uart1_rxbuf[UART1_BUFFER_SIZE];
extern uint8_t  uart1_rxpos;
extern uint8_t  uart1_rxlen;

extern void serial_input_handler(void);
extern void ulin_input_handler(void);

void uart0_init (void);
void uart1_init (void);
uint8_t uart0_get_data(void);
uint8_t uart1_get_data(void);
void uart0_tx_mode (void);
void uart0_rx_mode (void);
void uart1_tx_mode (void);
void uart1_rx_mode (void);
void uart0_send(char *pbuf , int len);
void uart0_flush_rxbuf(void);
void uart1_send(char *pbuf , int len);
void uart1_flush_rxbuf(void);

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
