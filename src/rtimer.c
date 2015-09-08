/*
 * rtimer.c
 *
 * Created on: March 26, 2015
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
#include <stdlib.h>
#include "config.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "rtimer.h"

#include "uart.h"
#include "serial.h"

#define RT_MODE_COMPARE() do { T1CCTL1 |= T1CCTL_MODE; } while(0)
#define RT_MODE_CAPTURE() do { T1CCTL1 &= ~T1CCTL_MODE; } while(0)

static struct rtimer *next_rtimer = NULL;

//uint8_t rtimer_busy;
//void (* rtimer_callback)(void) = NULL;

void rtimer_init(void)
{
  /*
   * - Free running mode
   * - Prescale by 32:
   *   Tick Speed has been prescaled to 500 kHz already in clock_init()
   *   We further prescale by 32 resulting in 15625 Hz for this timer.
   */
  T1CTL = (T1CTL_DIV1 | T1CTL_MODE0);
  
  /* Set Timer 1 Interrupt Priority to highest one */
  IP1 = 0x02;
  IP0 = 0x02;

  T1STAT = 0;

  /* Timer 1, Channel 1. Compare Mode (0x04), Interrupt mask on (0x40) */
  T1CCTL1 = T1CCTL_MODE | T1CCTL_IM;
  
  /* Interrupt Mask Flags: No interrupt on overflow */
#if defined __IAR_SYSTEMS_ICC__
  T1OVFIM = 0;
#else
  OVFIM = 0;
#endif
  
  /* Acknowledge Timer 1 Interrupts */
  T1IE = 1;
}

rtimer_clock_t rtimer_now(void)
{
  return (T1CNTL + (T1CNTH << 8));
}

void rtimer_schedule(rtimer_clock_t t)
{
  /* Switch to capture mode before writing T1CC1x and
   * set the compare mode values so we can get an interrupt after t */
  RT_MODE_CAPTURE();
  T1CC1L = (unsigned char)t;
  T1CC1H = (unsigned char)(t >> 8);
  RT_MODE_COMPARE();

  /* Turn on compare mode interrupt */
  T1STAT = 0;
  T1CCTL1 |= T1CCTL_IM;
}

void rtimer_set(struct rtimer *rtimer,
    rtimer_clock_t time,
    rtimer_callback_t func)
{
  struct rtimer *t;
  rtimer_clock_t rt_now, rt1, rt2;
  uint8_t first = 0;

  /* halt T1 counter */
  T1CTL &= ~(T1CTL_MODE1 | T1CTL_MODE0);

  rt_now = rtimer_now();

  rtimer->func = func;
  rtimer->time = time;
  rtimer->next = NULL;

  /* FIXME there can be only one rtimer value associated to one interrupt event.
   * increment it by one for additional 64us to the assigned value.
   */
  t = next_rtimer;
  while (t) {
    if (t->time == rtimer->time) rtimer->time++;
    t = t->next;
  }

  if(next_rtimer == NULL) {
    first = 1;
    next_rtimer = rtimer;
  }
  else {
    t = next_rtimer;

    /* if the new rtimer will occur earlier than the one currently running, schedule it */
    if (rt_now > rtimer->time)
      rt1 = rtimer->time + ~rt_now + 1;
    else
      rt1 = rtimer->time - rt_now;

    if (rt_now > t->time)
      rt2 = t->time + ~rt_now + 1;
    else
      rt2 = t->time - rt_now;

    if (rt1 < rt2) {
      first = 1;
      rtimer->next = t;
      next_rtimer = rtimer;
    }

    /* otherwise try to put it in order within the list */
    else {
      do {
        rtimer->next = t->next;

        if (rt_now > rtimer->time)
          rt1 = rtimer->time + ~rt_now + 1;
        else
          rt1 = rtimer->time - rt_now;

        if (rt_now > t->next->time)
          rt2 = t->next->time + ~rt_now + 1;
        else
          rt2 = t->next->time - rt_now;

        if ((rt1 < rt2) || (rtimer->next == NULL)) {
          t->next = rtimer;
          break;
        }
        else {
          t = rtimer->next;
        }
      } while(rtimer->next);
    }

  }

  if(first == 1) {
    rtimer_schedule(next_rtimer->time);
  }

  /* continue T1 counter */
  T1CTL |= T1CTL_MODE0;
}

void rtimer_run_next(void)
{
  struct rtimer *t;
  if(next_rtimer == NULL) {
    return;
  }
  t = next_rtimer;
  next_rtimer = t->next;
  t->func();
  if(next_rtimer != NULL) {
    rtimer_schedule(next_rtimer->time);
  }
  return;
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=T1_VECTOR
__interrupt void rtimer_isr(void)
#else
void rtimer_isr (void) __interrupt (T1_VECTOR)
#endif
{
  T1IE = 0; /* Ignore Timer 1 Interrupts */

  if (T1STAT & T1STAT_CH1IF) {
    /* No more interrupts from Channel 1 till next rtimer_arch_schedule() call */
    T1STAT &= ~T1STAT_CH1IF;
    T1CCTL1 &= ~T1CCTL_IM;

    rtimer_run_next();
  }

  T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}

uint8_t rtimer_is_scheduled(void)
{
  if (next_rtimer == NULL) return 0;
  else return 1;
}
