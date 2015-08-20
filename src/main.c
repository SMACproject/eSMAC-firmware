/*
 * main.c
 *
 * Created on: Jul 7, 2015
 *     Author: Ekawahyu Susilo
 *
 * Copyright (c) 2015, Chongqing Aisenke Electronic Technology Co., Ltd.
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "modules.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "clock.h"
#include "rtimer.h"
#include "uart.h"
#include "jsmn.h"
#include "json.h"
#include "spi.h"
#include "flash.h"
#include "radio.h"

enum {
  UART0_IDLE,
  UART0_RECEIVING,
  UART0_RECEIVED,
  UART0_SENDING,
  UART0_SENT
};

static uint8_t uart0_state;

void main(void)
{
  EA = 0;
  
  clock_init();
  clock_delay_usec(60000); // workaround to wait for LSM9DS0 ready

  uart_init();
  rtimer_init();
  rf_init();
  module_init();

  // comment this line out to prevent bluetooth board from crashing
  printf("\nSMAC2.0 - [%x:%x]\n", SHORT_ADDR1, SHORT_ADDR0);
  
  EA = 1;
  MEMCTR = 7;
  
  while(1)
  {
    if( uart0_state == UART0_RECEIVED )
    {
      uart0_state = UART0_SENDING;
      rf_send(serial_rxbuf , serial_rxlen);
      uart0_flush_rxbuf();
      uart0_state = UART0_IDLE;
    }
  }
}

void uart0_receiving_timeout (void)
{
  if (uart0_state == UART0_RECEIVING) {
    // print it to console
    uart0_sendbuf( serial_rxbuf , serial_rxlen);
    uart0_state = UART0_RECEIVED;
  }
  //led_set(0);
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX0_VECTOR
__interrupt void uart0_isr(void)
#else
void uart0_isr (void) __interrupt (URX0_VECTOR)
#endif
{
  URX0IF = 0;

  if (serial_rxpos >= 128 || uart0_state == UART0_SENDING) return;

  serial_rxbuf[serial_rxpos] = U0DBUF;
  serial_rxpos++;
  serial_rxlen++;

  uart0_state = UART0_RECEIVING;
  rtimer_schedule(100, uart0_receiving_timeout);
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX1_VECTOR
__interrupt void uart1_isr(void)
#else
void uart1_isr (void) __interrupt (URX1_VECTOR)
#endif
{
  URX1IF = 0;
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=RF_VECTOR
__interrupt void rf_isr(void)
#else
void rf_isr (void) __interrupt (RF_VECTOR)
#endif
{
  EA = 0;

  if (RFIRQF0 & ( 1<<6 ))
  {
    rf_receive_isr();

    S1CON = 0;
    RFIRQF0 &= ~(1<<6);
  }
  EA = 1;
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=T1_VECTOR
__interrupt void rtimer_isr(void)
#else
void rtimer_isr (void) __interrupt (T1_VECTOR)
#endif
{
  T1IE = 0; /* Ignore Timer 1 Interrupts */

  if (T1STAT & T1STAT_CH0IF) {
    /* No more interrupts from Channel 1 till next rtimer_arch_schedule() call */
    T1STAT &= ~T1STAT_CH0IF;
    T1CCTL0 &= ~T1CCTL_IM;

    /* Callback function call */
    if (rtimer_callback) rtimer_callback();

    /* Release rtimer lock and callback */
    rtimer_busy = 0;
    rtimer_callback = NULL;

    //rtimer_schedule(27, toggle_led);
  }

  T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}
