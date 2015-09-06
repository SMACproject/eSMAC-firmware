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
  SERIAL_IDLE,
  SERIAL_RECEIVING,
  SERIAL_RECEIVED,
  SERIAL_SENDING
};

enum {
  LIN_IDLE,
  LIN_RECEIVING,
  LIN_RECEIVED,
  LIN_SENDING
};

#define SPEED 1

unsigned char mydata1, mydata2;
unsigned char loopback1, loopback2;

unsigned char lin_txbuf[20];
static uint16_t err_count = 0;

static volatile uint8_t serial_state = SERIAL_IDLE;
static volatile uint8_t lin_state = LIN_IDLE;

void main(void)
{
#if ONE_WIRE_TX
  uint8_t loop;
  uint8_t identifier = 1;
#endif
  EA = 0;
  
  clock_init();
  clock_delay_usec(60000); // workaround to wait for LSM9DS0 ready

  uart_init();
  rtimer_init();
  rf_init(RADIO_CHANNEL);
  module_init();

  // comment this line out to prevent bluetooth board from crashing
  printf("\nSMAC2.0 - [%x:%x]\n", SHORT_ADDR1, SHORT_ADDR0);
  
  EA = 1;
  MEMCTR = 7;
  
  while(1)
  {
    if( serial_state == SERIAL_RECEIVED )
    {
      serial_state = SERIAL_SENDING;
      rf_send(serial_rxbuf , serial_rxlen);
      serial_flush_rxbuf();
      serial_state = SERIAL_IDLE;
    }
#if ONE_WIRE_TX
      lin_tx_mode();
      lin_state = LIN_SENDING;
      //led_set(LED3);

      lin_txbuf[0] = 0x00;
      lin_txbuf[1] = 0x00;
      lin_txbuf[2] = 0x55;
      lin_txbuf[3] = identifier++;
      if (identifier == 64) identifier = 0;
      lin_txbuf[4] = 0x12;
      lin_txbuf[5] = 0x34;
      lin_txbuf[6] = 0x56;
      lin_txbuf[7] = 0x78;
      lin_txbuf[8] = 0x90;
      lin_txbuf[9] = 0xAB;
      lin_txbuf[10] = 0xCD;
      lin_txbuf[11] = 0xEF;
      lin_txbuf[12] = 0xAA;

      lin_send(lin_txbuf, 13);

      //led_set(0);
      lin_rx_mode();
      lin_state = LIN_IDLE;
      for (loop=0; loop < SPEED; loop++) clock_delay_usec(1000);
#else // ONE_WIRE_TX //
    if( lin_state == LIN_RECEIVED )
    {
      if (lin_rxbuf[0] == 0 && lin_rxbuf[1] == 0 && lin_rxbuf[2] == 0x55) {
        if (lin_rxbuf[3] == 1)
          led_set(LED3);
        else
          led_set(0);
      }
      lin_flush_rxbuf();
      lin_state = LIN_IDLE;
    }
#endif
  }
}

void serial_receiving_timeout (void)
{
  if (serial_state == SERIAL_RECEIVING) {
    serial_send(serial_rxbuf , serial_rxlen);
    printf("%i-%i-%i-%i\n", lin_state, lin_rxpos, lin_rxlen, err_count);
    serial_state = SERIAL_RECEIVED;
  }
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX0_VECTOR
__interrupt void uart0_isr(void)
#else
void uart0_isr (void) __interrupt (URX0_VECTOR)
#endif
{
  URX0IF = 0;

#if (UART_STDOUT_PORT == 0)
  if (serial_rxpos >= 128 || serial_state == SERIAL_SENDING) return;

  serial_rxbuf[serial_rxpos] = U0DBUF;
  serial_rxpos++;
  serial_rxlen++;

  serial_state = SERIAL_RECEIVING;
  rtimer_schedule(100, serial_receiving_timeout);
}
#endif

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX1_VECTOR
__interrupt void uart1_isr(void)
#else
void uart1_isr (void) __interrupt (URX1_VECTOR)
#endif
{
  URX1IF = 0;

#if (UART_ONE_WIRE_PORT == 1)
  /* LIN buffer is full and need to be processed */
  if (lin_state == LIN_RECEIVED) {
    return;
  }

  /* The 1st BREAK detection */
  if ((lin_state == LIN_IDLE) && (U1DBUF == 0)) {
    lin_flush_rxbuf();
  }

  /* No sync detected */
  if (lin_rxpos == 3) {
    if (lin_rxbuf[0] != 0 || lin_rxbuf[1] != 0 || lin_rxbuf[2] != 0x55) {
      lin_flush_rxbuf();
      err_count++;
    }
  }

  lin_rxbuf[lin_rxpos] = U1DBUF;
  lin_rxpos++;
  lin_rxlen++;

  if (lin_rxpos >= 13)
    lin_state = LIN_RECEIVED;
  else
    lin_state = LIN_RECEIVING;
#endif
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
  }

  T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}
