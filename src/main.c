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
#include "intr.h"
#include "clock.h"
#include "rtimer.h"
#include "radio.h"
#include "flash.h"
#include "serial.h"
#include "ulin.h"
#include "modules.h"
#include "jsmn.h"
#include "json.h"

#include "led.h"

struct rtimer blink1, blink2, blink3, blink4;
uint8_t blink1_status = 0, blink2_status = 0, blink3_status = 0, blink4_status = 0;

void blink1_handler(void)
{
  if (blink1_status) {
    blink1_status = 0;
    led_set(led_get() & ~LED1);
  }
  else {
    blink1_status = 1;
    led_set(led_get() | LED1);
  }
  rtimer_set(&blink1, rtimer_now()+1875, blink1_handler);
}

void blink2_handler(void)
{
  if (blink2_status) {
    blink2_status = 0;
    led_set(led_get() & ~LED2);
  }
  else {
    blink2_status = 1;
    led_set(led_get() | LED2);
  }
  rtimer_set(&blink2, rtimer_now()+10, blink2_handler);
}

void blink3_handler(void)
{
  if (blink3_status) {
    blink3_status = 0;
    led_set(led_get() & ~LED3);
  }
  else {
    blink3_status = 1;
    led_set(led_get() | LED3);
  }
  rtimer_set(&blink3, rtimer_now()+23, blink3_handler);
}

void blink4_handler(void)
{
  if (blink4_status) {
    blink4_status = 0;
    led_set(led_get() & ~LED4);
  }
  else {
    blink4_status = 1;
    led_set(led_get() | LED4);
  }
  rtimer_set(&blink4, rtimer_now()+15000, blink4_handler);
}

void main(void)
{
  disable_interrupt();
  
  clock_init();

  /* workaround to wait for LSM9DS0 ready */
  clock_delay_usec(60000);

  /* serial port */
  serial_init();

  /* one wire UART based LIN (ULIN) */
  ulin_init();

  /* real time timer */
  rtimer_init();

  /* radio configuration */
  rf_init(RADIO_CHANNEL);

  /* module specific initialization - modules.h */
  module_init();

  /* comment this line out to prevent bluetooth board from crashing */
  printf("\nSMAC2.0 - [%x:%x]\n", rf_get_short_addr1(), rf_get_short_addr0());
  
  enable_interrupt();

  /* flash bank used as storage */
  flash_bank_select(FLASH_BANK_7);
  
  rtimer_set(&blink1, rtimer_now()+1000, blink1_handler);
  rtimer_set(&blink2, rtimer_now()+10, blink2_handler);
  rtimer_set(&blink3, rtimer_now()+23, blink3_handler);
  rtimer_set(&blink4, rtimer_now()+1000, blink4_handler);

  /* looping services */
  while(1)
  {
    serial_service();

    ulin_service();
  }
}
