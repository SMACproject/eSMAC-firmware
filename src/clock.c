/*
 * clock.c
 *
 * Created on: May 31, 2014
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

#include "cc253x.h"
#include "sfr-bits.h"
#include "clock.h"

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 128

/* Sleep timer runs on the 32k RC osc. */
/* One clock tick is 7.8 ms */
#define TICK_VAL (32768/128)  /* 256 */
/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/
/* Do NOT remove the absolute address and do NOT remove the initialiser here */
#if defined __IAR_SYSTEMS_ICC__
/* TODO How to declare a variable at a particular location in IAR? */
static volatile unsigned long timer_value = 0;
#else
__xdata __at(0x0000) static unsigned long timer_value = 0;
#endif

static volatile clock_time_t count = 0; /* Uptime in ticks */
static volatile clock_time_t seconds = 0; /* Uptime in secs */
/*---------------------------------------------------------------------------*/
void clock_init(void)
{
  /* Make sure we know where we stand */
  CLKCONCMD = CLKCONCMD_OSC32K | CLKCONCMD_OSC;
  while(!(CLKCONSTA & CLKCONCMD_OSC));

  /* Stay with 32 KHz RC OSC, change system clock to 32 MHz */
  CLKCONCMD &= ~CLKCONCMD_OSC;
  while(CLKCONSTA & CLKCONCMD_OSC);
  
  /* Tickspeed 500 kHz for timers[1-4] */
  CLKCONCMD |= CLKCONCMD_TICKSPD2 | CLKCONCMD_TICKSPD1;
  while(CLKCONSTA != CLKCONCMD);

  /* Initialize tick value */
  timer_value = ST0;
  timer_value += ((unsigned long int)ST1) << 8;
  timer_value += ((unsigned long int)ST2) << 16;
  timer_value += TICK_VAL;
  ST2 = (unsigned char)(timer_value >> 16);
  ST1 = (unsigned char)(timer_value >> 8);
  ST0 = (unsigned char)timer_value;

  //STIE = 1; /* IEN0.STIE interrupt enable */
}
/*---------------------------------------------------------------------------*/
clock_time_t clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
unsigned long clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
void clock_delay_usec(uint16_t len)
{
  while(len--) {
    __asm_begin
      ASM(nop)
    __asm_end;
  }
}
/*---------------------------------------------------------------------------*/
#if defined __IAR_SYSTEMS_ICC__
#pragma vector=ST_VECTOR
__interrupt void clock_isr(void)
#else
void clock_isr (void) __interrupt (ST_VECTOR)
#endif
{
  STIE = 0;

  /*
   * Read value of the ST0:ST1:ST2, add TICK_VAL and write it back.
   * Next interrupt occurs after the current time + TICK_VAL
   */
  timer_value = ST0;
  timer_value += ((unsigned long int)ST1) << 8;
  timer_value += ((unsigned long int)ST2) << 16;
  timer_value += TICK_VAL;
  ST2 = (unsigned char)(timer_value >> 16);
  ST1 = (unsigned char)(timer_value >> 8);
  ST0 = (unsigned char)timer_value;

  ++count;

  /* Make sure the CLOCK_CONF_SECOND is a power of two, to ensure
     that the modulo operation below becomes a logical and and not
     an expensive divide. Algorithm from Wikipedia:
     http://en.wikipedia.org/wiki/Power_of_two */
#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change CLOCK_CONF_SECOND in contiki-conf.h.
#endif
  if(count % CLOCK_CONF_SECOND == 0) {
    ++seconds;
  }

  STIF = 0; /* IRCON.STIF */
  STIE = 1;
}
