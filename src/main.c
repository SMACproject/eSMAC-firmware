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

  /* comment this line out to prevent bluetooth board from crashing */
  printf("\nSMAC2.0 - [%x:%x]\n", rf_get_short_addr1(), rf_get_short_addr0());
  
  enable_interrupt();

  /* module specific initialization - modules.h */
  module_init();

  /* flash bank used as storage */
  flash_bank_select(FLASH_BANK_7);

  /* looping services */
  while(1)
  {
    json_service();
    serial_service();
    ulin_service();
  }
}
