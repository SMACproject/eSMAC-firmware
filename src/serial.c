/*
 * serial.c
 *
 * Created on: Sep 6, 2015
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

#include <stdint.h>
#include "rtimer.h"
#include "uart.h"
#include "serial.h"
#include "radio.h"

enum {
  SERIAL_IDLE,
  SERIAL_RECEIVING,
  SERIAL_RECEIVED,
  SERIAL_SENDING
};

static uint8_t serial_state = SERIAL_IDLE;
struct rtimer serial_rtimer;

void serial_init(void)
{
#if (UART_STDOUT_PORT == 0)
  uart0_init();
#endif
#if (UART_STDOUT_PORT == 1)
  uart1_init();
#endif
}

void serial_receiving_timeout (void)
{
  if (serial_state == SERIAL_RECEIVING) {
    serial_send(serial_rxbuf , serial_rxlen);
    serial_state = SERIAL_RECEIVED;
  }
}

void serial_input_handler(void)
{
  if (serial_rxpos >= 128 || serial_state == SERIAL_SENDING) return;

  serial_rxbuf[serial_rxpos] = serial_get_data();
  serial_rxpos++;
  serial_rxlen++;

  serial_state = SERIAL_RECEIVING;
  //rtimer_schedule(100, serial_receiving_timeout);
  //rtimer_set(&serial_rtimer, rtimer_now()+100, serial_receiving_timeout);
}

void serial_service(void)
{
  if( serial_state == SERIAL_RECEIVED ) {
    serial_state = SERIAL_SENDING;
    rf_send(serial_rxbuf , serial_rxlen);
    serial_flush_rxbuf();

    serial_state = SERIAL_IDLE;
  }
}

