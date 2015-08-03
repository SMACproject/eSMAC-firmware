/*
 * uart.c
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

#include "config.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "uart.h"
#include "errnum.h"

char serial_rxbuf[128];
int  serial_rxpos = 0;
int  serial_rxlen = 0;
//char is_serial_receive = 0;

void uart_init (void)
{
#if (UART_STDOUT_PORT == 0)
    UART_SET_SPEED(0, UART_115_M, UART_115_E);
    PERCFG &= ~PERCFG_U0CFG;  /* alternative 1 = P0.2-5 */
    P0SEL |= 0x0C;            /* peripheral select for TX and RX */
    P0DIR |= 0x08;            /* TX out */
    P0DIR &= ~0x04;           /* RX in */
    U0UCR = 0x02;             /* defaults: 8N1, no flow, high stop bit */
    U0CSR = UCSR_MODE | UCSR_RE;  /* UART mode, RX enabled */
    U0UCR |= 0x80;            /* flush it */
    URX0IE = 1;               /* enable RX interrupt */
    //UTX0IF = 1;
#elif (UART_STDOUT_PORT == 1)
    UART_SET_SPEED(1, UART_115_M, UART_115_E);
    PERCFG |= PERCFG_U1CFG;  /* alternative 2 = P1.7-4 */
#if ONE_WIRE_TX
    P1SEL |= 0xC0;            /* peripheral select for TX and RX */
    P1DIR |= 0x40;            /* TX out */
    P1DIR &= ~0x80;           /* RX in */
#else
    P1SEL |= 0x00;            /* peripheral select for TX and RX */
    P1DIR &= ~0x40;           /* TX in */
    P1DIR &= ~0x80;           /* RX in */ 
#endif
    U1UCR = 0x02;             /* defaults: 8N1, no flow, high stop bit */
    U1CSR = UCSR_MODE | UCSR_RE;  /* UART mode, RX enabled */
    U1UCR |= 0x80;            /* flush it */
    //URX1IE = 1;               /* enable RX interrupt */
    //UTX1IF = 1;
#endif
}

void uart1_tx_mode (void)
{
    //UART_SET_SPEED(1, UART_115_M, UART_115_E);
    //PERCFG |= PERCFG_U1CFG;  /* alternative 2 = P1.7-4 */

    P1SEL |= 0xC0;            /* peripheral select for TX and RX */
    P1DIR |= 0x40;            /* TX out */
    P1DIR &= ~0x80;           /* RX in */

    //U1UCR = 0x02;             /* defaults: 8N1, no flow, high stop bit */
    //U1CSR = UCSR_MODE | UCSR_RE;  /* UART mode, RX enabled */
    U1UCR |= 0x80;            /* flush it */
    //URX1IE = 1;               /* enable RX interrupt */
}

void uart1_rx_mode (void)
{
    //UART_SET_SPEED(1, UART_115_M, UART_115_E);
    //PERCFG |= PERCFG_U1CFG;  /* alternative 2 = P1.7-4 */

    P1SEL |= 0x00;            /* peripheral select for TX and RX */
    P1DIR &= ~0x40;           /* TX in */
    P1DIR &= ~0x80;           /* RX in */ 

    //U1UCR = 0x02;             /* defaults: 8N1, no flow, high stop bit */
    //U1CSR = UCSR_MODE | UCSR_RE;  /* UART mode, RX enabled */
    U1UCR |= 0x80;            /* flush it */
    //URX1IE = 1;               /* enable RX interrupt */
}

void uart0_flush_rxbuf()
{
  serial_rxpos = 0;
  serial_rxlen = 0;
}

void uart0_sendbuf(char *pbuf , int len)
{
  int i;

  for(i = 0 ; i < len ; i++)
  {
    U0CSR &= ~UCSR_TX_BYTE; // Clear TX_BYTE status
    U0DBUF = *pbuf;
    pbuf++;
    while(!(U0CSR & UCSR_TX_BYTE)); // Wait until byte has been transmitted.
    U0CSR &= ~UCSR_TX_BYTE; // Clear TX_BYTE status
  }
}

#if defined __IAR_SYSTEMS_ICC__
int
putchar(int c)
#else
void
putchar(char c)
#endif
{
#if (UART_STDOUT_PORT == 0)
  U0CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
  U0DBUF = c;
  while(!(U0CSR & UCSR_TX_BYTE)); /* Wait until byte has been transmitted. */
  U0CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
#elif (UART_STDOUT_PORT == 1)
  U1CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
  U1DBUF = c;
  while(!(U1CSR & UCSR_TX_BYTE)); /* Wait until byte has been transmitted. */
  U1CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
#endif
#if defined __IAR_SYSTEMS_ICC__
  return c;
#endif
}
