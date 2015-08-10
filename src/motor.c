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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "motor.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "jsmn.h"
#include "json.h"

/* TODO workaround */
#include "radio.h"
char is_auto_run = 0;

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
char motor_speed_changed = 0;

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

int json_parser(char *json_string)
{
  int i, r;
  jsmn_parser p;
  jsmntok_t t[JSON_MAX_TOKEN];
  char buffer[JSON_STRING_BUFFER_SIZE];

  memset(buffer, 0, sizeof(buffer));
  jsmn_init(&p);

  r = jsmn_parse(&p, json_string, strlen(json_string), t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
    printf("Failed to parse JSON: %d\n", r);
    return 1;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object expected\n");
    return 1;
  }

  for (i = 1; i < r; i++) {

    if (jsoneq(json_string, &t[i], "motor1") == 0) {
                if (jsoneq(json_string, &t[i+1], "cw") == 0) {
                  motors_status |= MOTOR1_1;
                  motors_status &= ~MOTOR1_2;
                  rf_send("m1:1",4);
                }
                if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                  motors_status &= ~MOTOR1_1;
                  motors_status |= MOTOR1_2;
                  rf_send("m1:1",4);
                }
                if (jsoneq(json_string, &t[i+1], "off") == 0) {
                  motors_status &= ~MOTOR1_1;
                  motors_status &= ~MOTOR1_2;
                  rf_send("m1:0",4);
                }
                if (jsoneq(json_string, &t[i+1], "brake") == 0) {
                  motors_status |= MOTOR1_1;
                  motors_status |= MOTOR1_2;
                }
                i++;
    } else if (jsoneq(json_string, &t[i], "motor2") == 0) {
                if (jsoneq(json_string, &t[i+1], "cw") == 0) {
                  motors_status |= MOTOR2_1;
                  motors_status &= ~MOTOR2_2;
                  rf_send("m2:1",4);
                }
                if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                  motors_status &= ~MOTOR2_1;
                  motors_status |= MOTOR2_2;
                  rf_send("m2:1",4);
                }
                if (jsoneq(json_string, &t[i+1], "off") == 0) {
                  motors_status &= ~MOTOR2_1;
                  motors_status &= ~MOTOR2_2;
                  rf_send("m2:0",4);
                }
                if (jsoneq(json_string, &t[i+1], "brake") == 0) {
                  motors_status |= MOTOR2_1;
                  motors_status |= MOTOR2_2;
                }
                i++;
    } else if (jsoneq(json_string, &t[i], "mode") == 0) {
                if (jsoneq(json_string, &t[i+1], "manual") == 0) {
                  is_auto_run = 0;
                }
                if (jsoneq(json_string, &t[i+1], "auto") == 0) {
                  is_auto_run = 1;
                }
              } else if (jsoneq(json_string, &t[i], "speed1") == 0) {
                motor_speed_changed = 1;
                if (jsoneq(json_string, &t[i+1], "1") == 0) {
                  motor1_speed = 10;
                } else if (jsoneq(json_string, &t[i+1], "2") == 0) {
                  motor1_speed = 20;
                } else if (jsoneq(json_string, &t[i+1], "3") == 0) {
                  motor1_speed = 30;
                } else if (jsoneq(json_string, &t[i+1], "4") == 0) {
                  motor1_speed = 40;
                } else if (jsoneq(json_string, &t[i+1], "5") == 0) {
                  motor1_speed = 50;
                } else if (jsoneq(json_string, &t[i+1], "6") == 0) {
                  motor1_speed = 60;
                } else if (jsoneq(json_string, &t[i+1], "7") == 0) {
                  motor1_speed = 70;
                } else if (jsoneq(json_string, &t[i+1], "8") == 0) {
                  motor1_speed = 80;
                } else if (jsoneq(json_string, &t[i+1], "9") == 0) {
                  motor1_speed = 90;
                } else if (jsoneq(json_string, &t[i+1], "10") == 0) {
                  motor1_speed = 99;
                }
              } else if (jsoneq(json_string, &t[i], "speed2") == 0) {
                motor_speed_changed = 1;
                if (jsoneq(json_string, &t[i+1], "1") == 0) {
                  motor2_speed = 10;
                } else if (jsoneq(json_string, &t[i+1], "2") == 0) {
                  motor2_speed = 20;
                } else if (jsoneq(json_string, &t[i+1], "3") == 0) {
                  motor2_speed = 30;
                } else if (jsoneq(json_string, &t[i+1], "4") == 0) {
                  motor2_speed = 40;
                } else if (jsoneq(json_string, &t[i+1], "5") == 0) {
                  motor2_speed = 50;
                } else if (jsoneq(json_string, &t[i+1], "6") == 0) {
                  motor2_speed = 60;
                } else if (jsoneq(json_string, &t[i+1], "7") == 0) {
                  motor2_speed = 70;
                } else if (jsoneq(json_string, &t[i+1], "8") == 0) {
                  motor2_speed = 80;
                } else if (jsoneq(json_string, &t[i+1], "9") == 0) {
                  motor2_speed = 90;
                } else if (jsoneq(json_string, &t[i+1], "10") == 0) {
                  motor2_speed = 99;
                }
              } else {
/*#ifdef CONFIG_DONT_HAVE_SPRINTF_WITH_STRING_LENGTH_FORMATTING
                strncpy(buffer, json_string + t[i].start, t[i].end-t[i].start);
                printf("Unexpected key: %s (%i:%i)\n", buffer, t[i].type, t[i].size);
#else
                sprintf(buffer, "Unexpected key: %.*s (%i:%i)\n", t[i].end-t[i].start,
                    json_string + t[i].start, t[i].type, t[i].size);
                printf("%s", buffer);
#endif*/
              }

  }

  motor_set(motors_status);
  //clock_delay_usec(60000);
  //motor_set(0);

  return 0;
}
