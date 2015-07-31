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

#include "cc253x.h"

/*
#define MOTOR1_PIN1   P1_0
#define MOTOR1_PIN2   P1_1
#define MOTOR2_PIN1   P1_4
#define MOTOR2_PIN2   P2_0

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x02
#define MOTOR2_PIN1_MASK  0x10
#define MOTOR2_PIN2_MASK  0x01
*/
#define MOTOR1_PIN1   P0_0
#define MOTOR1_PIN2   P0_4
#define MOTOR2_PIN1   P0_1
#define MOTOR2_PIN2   P0_5

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x10
#define MOTOR2_PIN1_MASK  0x02
#define MOTOR2_PIN2_MASK  0x20

static char motors_status = 0;

void motor_init(void)
{
/*
  P1SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK);
  P1DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK);
  P2SEL &= ~MOTOR2_PIN2_MASK;
  P2DIR |=  MOTOR2_PIN2_MASK;
*/
  P0SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);
  P0DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);
  MOTOR1_PIN1 = 0;
  MOTOR1_PIN2 = 0;
  MOTOR2_PIN1 = 0;
  MOTOR2_PIN2 = 0;
  motors_status = 0;
}

void motor_set(char motors)
{
  MOTOR1_PIN1 = motors & 0x01;
  MOTOR1_PIN2 = (motors & 0x02) >> 1;
  MOTOR2_PIN1 = (motors & 0x04) >> 2;
  MOTOR2_PIN2 = (motors & 0x08) >> 3;
  motors_status = motors;
}

char motor_get(void)
{
  return motors_status;
}
