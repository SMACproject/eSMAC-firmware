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

#define SERIAL_TIMEOUT_COUNT 50 /* FIXME not deterministic, need accurate timing */

enum {
  SERIAL_IDLE,
  SERIAL_RECEIVING,
  SERIAL_RECEIVING_IDLE,
  SERIAL_TIMEOUT,
  SERIAL_SENDING
};

static uint8_t serial_state = SERIAL_IDLE;
static uint8_t idle_counter = 0;
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

void serial_input_handler(void)
{
  if (serial_rxpos >= 128 || serial_state == SERIAL_SENDING) return;
  serial_rxbuf[serial_rxpos] = serial_get_data();
  serial_rxpos++;
  serial_rxlen++;

  serial_state = SERIAL_RECEIVING;
}

void serial_service(void)
{
  switch (serial_state) {
    case SERIAL_TIMEOUT:
      serial_state = SERIAL_SENDING;
      serial_send(serial_rxbuf, serial_rxlen);
      rf_send(serial_rxbuf, serial_rxlen);
      serial_flush_rxbuf();
      serial_state = SERIAL_IDLE;
      break;
    case SERIAL_RECEIVING:
      idle_counter = 0;
      serial_state = SERIAL_RECEIVING_IDLE;
      break;
    case SERIAL_RECEIVING_IDLE:
      idle_counter++;
      if (idle_counter >= SERIAL_TIMEOUT_COUNT) {
        idle_counter = 0;
        serial_state = SERIAL_TIMEOUT;
      }
      break;
    default:
      /* do something here */
      break;
  }
}

