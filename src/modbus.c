/*
 * modbubs.c
 *
 * Created on: March 25, 2015
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
#include "modbus.h"

enum {
  MODBUS_INIT,
  MODBUS_IDLE,
  MODBUS_EMISSION,
  MODBUS_RECEPTION,
  MODBUS_CONTROL_WAIT
};

static volatile uint8_t modbus_250us_count;
static volatile uint8_t modbus_state;
static volatile uint8_t modbus_timing;

void modbus_init(void)
{
  modbus_state = MODBUS_INIT;
}

void modbus_service (void)
{
    switch (modbus_state) {
    case MODBUS_INIT:
      /* get the time now */
      modbus_timing = modbus_250us_count;
      break;
    case MODBUS_IDLE:
      break;
    default:
      /* should not fall within this unless error is encountered */
      break;
  }
}

int modbus_send(uint8_t addr, uint8_t function, uint8_t * payload, uint8_t len)
{
  return 0;
}

int modbus_receive(uint8_t * addr, uint8_t * function, uint8_t * payload)
{
  uint8_t len = 0;
  
  return len;
}

void modbus_t35_expired (void)
{
  
}

void modbus_t15_expired (void)
{
  
}

void modbus_250us_cb(void)
{
  modbus_250us_count++;
}

