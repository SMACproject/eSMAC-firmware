/*
 * led.c
 *
 * Created on: Jun 23, 2015
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

#include "cc253x.h"
#include "led.h"

#define LED1_PIN   P1_0
#define LED2_PIN   P1_1
#define LED3_PIN   P1_4
#define LED4_PIN   P2_0

#define LED1_MASK  0x01
#define LED2_MASK  0x02
#define LED3_MASK  0x10
#define LED4_MASK  0x01

static char leds_status = 0;

void led_init(void)
{
  P1SEL &= ~(LED1_MASK | LED2_MASK | LED3_MASK);
  P1DIR |= (LED1_MASK | LED2_MASK | LED3_MASK);
  P2SEL &= ~LED4_MASK;
  P2DIR |= LED4_MASK;
  LED1_PIN = 0;
  LED2_PIN = 0;
  LED3_PIN = 0;
  LED4_PIN = 0;
  leds_status = 0;
}

void led1_init(void)
{
  P1SEL &= ~LED1_MASK;
  P1DIR |= LED1_MASK;
  LED1_PIN = 0;
  leds_status = 0;
}

void led_set(char leds)
{
  LED1_PIN = leds & 0x01;
  LED2_PIN = (leds & 0x02) >> 1;
  LED3_PIN = (leds & 0x04) >> 2;
  LED4_PIN = (leds & 0x08) >> 3;
  leds_status = leds;
}

char led_get(void)
{
  return leds_status;
}
