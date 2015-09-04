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

#define ONE_WIRE_TX   0

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

#if (UART_STDOUT_PORT == 0)
#define serial_rxbuf        uart0_rxbuf
#define serial_rxpos        uart0_rxpos
#define serial_rxlen        uart0_rxlen
#define serial_send         uart0_send
#define serial_flush_rxbuf  uart0_flush_rxbuf
#else
#define serial_rxbuf        uart1_rxbuf
#define serial_rxpos        uart1_rxpos
#define serial_rxlen        uart1_rxlen
#define serial_send         uart1_send
#define serial_flush_rxbuf  uart1_flush_rxbuf
#endif

#if (UART_ONE_WIRE_PORT == 0)
#define lin_rxbuf         uart0_rxbuf
#define lin_rxpos         uart0_rxpos
#define lin_rxlen         uart0_rxlen
#define lin_send          uart0_send
#define lin_flush_rxbuf   uart0_flush_rxbuf
#define lin_rx_mode       uart0_rx_mode
#define lin_tx_mode       uart0_tx_mode
#else
#define lin_rxbuf         uart1_rxbuf
#define lin_rxpos         uart1_rxpos
#define lin_rxlen         uart1_rxlen
#define lin_send          uart1_send
#define lin_flush_rxbuf   uart1_flush_rxbuf
#define lin_rx_mode       uart1_rx_mode
#define lin_tx_mode       uart1_tx_mode
#endif

#define UART_SET_SPEED(N, M, E) do{ U##N##BAUD = M; U##N##GCR = E; } while(0)
#define UART_115_M   216
#define UART_115_E    11

#define UART_LOCK   0x80

extern char uart0_rxbuf[128];
extern int  uart0_rxpos;
extern int  uart0_rxlen;

extern char uart1_rxbuf[128];
extern int  uart1_rxpos;
extern int  uart1_rxlen;

void uart_init (void);
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
