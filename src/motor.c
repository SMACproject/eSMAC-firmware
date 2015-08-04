/*
 * motor.c
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

#include "config.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "motor.h"

#define MOTOR1_PIN1   P0_0
#define MOTOR1_PIN2   P0_4
#define MOTOR2_PIN1   P0_1
#define MOTOR2_PIN2   P0_5

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x10
#define MOTOR2_PIN1_MASK  0x02
#define MOTOR2_PIN2_MASK  0x20

static char motors_status = 0;

/* motor_speedN should be set to an integer from 0 - 100 */
char motor1_speed = 50;
char motor2_speed = 50;

void motor_init(void)
{
#if CONFIG_MOTOR_PWM_ENABLE
  PERCFG &= ~PERCFG_T1CFG;           /* timer 1 alternative 0 location */
  P2DIR = (P2DIR & ~0xC0) | 0xC0;    /* give priority to timer 1 chanels 2-3 */
#endif
  P0SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);    /* general-purpose IO */
  P0DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);    /* for output*/

  MOTOR1_PIN1 = 0;
  MOTOR1_PIN2 = 0;
  MOTOR2_PIN1 = 0;
  MOTOR2_PIN2 = 0;
  motors_status = 0;

  T1CC0L = 100;      /* duty cycle*/
  T1CC0H = 0;

  T1CCTL2 |= 0x24;   /* channel 2 output compare; clear on compare, set on 0*/
  T1CCTL3 |= 0x24;   /* channel 3 output compare; clear on compare, set on 0*/

  T1CC2L = 100 - motor1_speed;
  T1CC2H = 0;
  T1CC3L = motor2_speed;
  T1CC3H = 0;
}

void motor_set_speed(char motor_number, char speed)
{

  if (motor_number == 1) {
    T1CC2L = speed;
    T1CC2H = 0;
  } else if (motor_number == 2) {
    T1CC3L = speed;
    T1CC3H = 0;
  }
}

void motor_set(char motors)
{
#if CONFIG_MOTOR_PWM_ENABLE
  if ((motors & MOTOR1_MASK) == MOTOR1_OFF) {
    P0SEL &= ~MOTOR1_PIN2_MASK; // general-purpose IO
    MOTOR1_PIN1 = 0;
    MOTOR1_PIN2 = 0;

  } else {
    P0SEL |=  MOTOR1_PIN2_MASK; // peripheral IO

    if ((motors & MOTOR1_MASK) == MOTOR1_CW) {
      motor_set_speed(1, 100-motor1_speed);
      MOTOR1_PIN1 = 1;
    } else {
      motor_set_speed(1, motor1_speed);
      MOTOR1_PIN1 = 0;
    }
  }

  if ((motors & MOTOR2_MASK) == MOTOR2_OFF) {
    P0SEL &= ~MOTOR2_PIN2_MASK; // general-purpose IO
    MOTOR2_PIN1 = 0;
    MOTOR2_PIN2 = 0;

  } else {
    P0SEL |= MOTOR2_PIN2_MASK; // peripheral IO

    if ((motors & MOTOR2_MASK) == MOTOR2_CW) {
      motor_set_speed(2, 100-motor2_speed);
      MOTOR2_PIN1 = 1;
    } else {
      motor_set_speed(2, motor2_speed);
      MOTOR2_PIN1 = 0;
    }
  }
#else
  MOTOR1_PIN1 = motors & 0x01;
  MOTOR1_PIN2 = (motors & 0x02) >> 1;
  MOTOR2_PIN1 = (motors & 0x04) >> 2;
  MOTOR2_PIN2 = (motors & 0x08) >> 3;
#endif

  motors_status = motors;
}

char motor_get(void)
{
  return motors_status;
}
