/*
 * lin.c
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
#include "config.h"
#include "radio.h"
#include "uart.h"
#include "ulin.h"

#define NAD_BROADCAST   127

#define ULIN_BREAK      0x00
#define ULIN_SYNC       0x55

enum {
  ULIN_IDLE,
  ULIN_RECEIVING,
  ULIN_RECEIVED,
  ULIN_SENDING
};

unsigned char ulin_txbuf[ONE_WIRE_BUFFER_SIZE];
uint8_t identifier = 0;

static uint16_t ulin_err_count = 0;
static uint8_t ulin_state = ULIN_IDLE;

void ulin_init(void)
{
#if (UART_ONE_WIRE_PORT == 0)
  uart0_init();
#endif
#if (UART_ONE_WIRE_PORT == 1)
  uart1_init();
#endif
}

uint8_t ulin_synced(uint8_t * ulin_frame)
{
  if (ulin_frame[0] == ULIN_BREAK)
    if (ulin_frame[1] == ULIN_BREAK)
      if (ulin_frame[2] == ULIN_SYNC)
        return 1;

  ulin_err_count++;

  return 0;
}

void ulin_handler(uint8_t * ulin_frame)
{
  if (ulin_frame[3] == NAD_BROADCAST) {
    if (rf_get_channel() != ulin_frame[4])
      rf_set_channel(ulin_frame[4]);
  }

  /* either the frame being handled or not, always flush it before leave */
  ulin_flush_rxbuf();
}

void ulin_input_handler(void)
{
  /* LIN buffer is full and need to be processed */
  if (ulin_state == ULIN_RECEIVED) {
    return;
  }

  /* the 1st BREAK detection */
  if ((ulin_state == ULIN_IDLE) && (ulin_get_data() == ULIN_BREAK)) {
    ulin_flush_rxbuf();
  }

  /* if no sync detected after three data being received, flush the buffer */
  if (ulin_rxpos == 3 && !ulin_synced(ulin_rxbuf))
      ulin_flush_rxbuf();

  ulin_rxbuf[ulin_rxpos] = ulin_get_data();
  ulin_rxpos++;
  ulin_rxlen++;

  if (ulin_rxpos >= 13)
    ulin_state = ULIN_RECEIVED;
  else
    ulin_state = ULIN_RECEIVING;
}

void ulin_service(void)
{
  if( ulin_state == ULIN_RECEIVED ) {
    if (ulin_synced(ulin_rxbuf))
      ulin_handler(ulin_rxbuf);

    ulin_state = ULIN_IDLE;
  }

#if CONFIG_MODULE_MASTER
      ulin_tx_mode();
      ulin_state = ULIN_SENDING;

      ulin_txbuf[0] = 0x00;
      ulin_txbuf[1] = 0x00;
      ulin_txbuf[2] = 0x55;
      ulin_txbuf[3] = NAD_BROADCAST;
      ulin_txbuf[4] = rf_get_channel();
      ulin_txbuf[5] = 0x00;
      ulin_txbuf[6] = 0x00;
      ulin_txbuf[7] = 0x00;
      ulin_txbuf[8] = 0x00;
      ulin_txbuf[9] = 0x00;
      ulin_txbuf[10] = 0x00;
      ulin_txbuf[11] = 0x00;
      ulin_txbuf[12] = 0xAA;

      ulin_send(ulin_txbuf, 13);

      ulin_rx_mode();
      ulin_state = ULIN_IDLE;
#endif
}
