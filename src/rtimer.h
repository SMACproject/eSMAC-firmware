/*
 * rtimer.h
 *
 * Created on: March 27, 2015
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

#ifndef RTIMER_H_
#define RTIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RTIMER_SECOND (15625U)

//typedef uint16_t timer_t;
typedef uint16_t rtimer_clock_t;
typedef void (* rtimer_callback_t)(void);

extern uint8_t rtimer_busy;


struct rtimer {
  rtimer_clock_t time;
  rtimer_callback_t func;
  struct rtimer *next;
};

void rtimer_init(void);
rtimer_clock_t rtimer_now(void);
uint8_t rtimer_set(struct rtimer *rtimer, rtimer_clock_t time, rtimer_callback_t func);
//uint8_t rtimer_schedule(rtimer_clock_t t);
//void rtimer_run_next(void);
uint8_t rtimer_is_scheduled(void);

#ifdef __cplusplus
}
#endif

#endif /* RTIMER_H_ */
