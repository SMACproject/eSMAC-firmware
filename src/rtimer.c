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

#include <stdlib.h>
#include <stdint.h>

#include "cc253x.h"
#include "sfr-bits.h"
#include "rtimer.h"
//#include "led.h"

#define RT_MODE_COMPARE() do { T1CCTL0 |= T1CCTL_MODE; } while(0)
#define RT_MODE_CAPTURE() do { T1CCTL0 &= ~T1CCTL_MODE; } while(0)

uint8_t rtimer_busy;
void (* rtimer_callback)(void) = NULL;

void rtimer_init(void)
{
  /*
   * - Modulo mode, counting to T1CC0H:T1CC0L
   * - Prescale by 32:
   *   Tick Speed has been prescaled to 500 kHz already in clock_init()
   *   We further prescale by 32 resulting in 15625 Hz for this timer.
   */
  T1CTL = (T1CTL_DIV1 | T1CTL_MODE1);
  
  /* Set Timer 1 Interrupt Priority to highest one */
  IP1 = 0x02;
  IP0 = 0x02;

  T1STAT = 0;

  /* Timer 1, Channel 0. Compare Mode (0x04), Interrupt mask on (0x40) */
  T1CCTL0 = T1CCTL_MODE | T1CCTL_IM;
  
  /* Interrupt Mask Flags: No interrupt on overflow */
  OVFIM = 0;
  
  /* Acknowledge Timer 1 Interrupts */
  T1IE = 1;
  
  /* Release rtimer lock and assign callback to NULL */
  rtimer_busy = 0;
  rtimer_callback = NULL;
}

uint8_t rtimer_schedule(timer_t t, void(*callback)(void))
{
  if (1/*!rtimer_busy*/) {
    /* Switch to capture mode before writing T1CC0x and
     * set the compare mode values so we can get an interrupt after t */
    RT_MODE_CAPTURE();
    T1CNTL = 0;
    T1CNTH = 0;
    T1CC0L = (unsigned char)t;
    T1CC0H = (unsigned char)(t >> 8);
    RT_MODE_COMPARE();
  
    /* Turn on compare mode interrupt */
    T1STAT = 0;
    T1CCTL0 |= T1CCTL_IM;
    
    /* Lock rtimer and assigned a callback */
    rtimer_busy = 1;
    if (callback) rtimer_callback = callback;
  }
  
  return rtimer_busy;
}

/*void toggle_led()
{
  if (led_get()) led_set(0); else led_set(LED1);
}*/
