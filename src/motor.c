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
/*
#define MOTOR1_PIN1   P0_0
#define MOTOR1_PIN2   P0_4
#define MOTOR2_PIN1   P0_1
#define MOTOR2_PIN2   P0_5

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x10
#define MOTOR2_PIN1_MASK  0x02
#define MOTOR2_PIN2_MASK  0x20
*/
#define LED1_PIN   P1_0
#define LED1_MASK  0x01

#define LED1 0x01
#define LED_ALL LED1

#define MOTOR1_PIN1   P1_4
#define MOTOR1_PIN2   P0_4
#define MOTOR2_PIN1   P1_1
#define MOTOR2_PIN2   P0_5

#define MOTOR1_PIN1_MASK  0x10
#define MOTOR1_PIN2_MASK  0x10
#define MOTOR2_PIN1_MASK  0x02
#define MOTOR2_PIN2_MASK  0x20

static char motors_status = 0;
static char leds_status = 0;

/* motor_speedN should be set to an integer from 0 - 100 */
char motor1_speed = 75;
char motor2_speed = 75;
char motor_speed_changed = 0;

void motor_init(void)
{
#if CONFIG_MOTOR_PWM_ENABLE
  T3CTL &= ~0x10; // stop timer 3
  T3CTL |= 0x04;  // clear timer 3
  T3CTL &= ~0x08; // disable timer 3 overflow interrupts

  T4CTL &= ~0x10; // stop timer 4
  T4CTL |= 0x04;  // clear timer 4
  T4CTL &= ~0x08; // disable timer 4 overflow interrupts

  //T1CTL = ???
  PERCFG &= ~PERCFG_T1CFG; /* timer 1 alternative 0 location */

  T3CTL = (0xA0 | 0x02);
  PERCFG &= ~PERCFG_T3CFG; /* timer 3 alternative 0 location */

  T4CTL = (0xA0 | 0x02);
  PERCFG &= ~PERCFG_T4CFG; /* timer 4 alternative 0 location */

  P2DIR |= 0xC0; /* give priority to timer 1 channels 2-3 */

  /* Timer 1 configuration */
  T1CC0L = 100;      /* period */
  T1CC0H = 0;

  T1CCTL2 |= 0x24;   /* channel 2 output compare; clear on compare, set on 0 */
  T1CCTL3 |= 0x24;   /* channel 3 output compare; clear on compare, set on 0 */

  T1CC2L = motor1_speed;
  T1CC2H = 0;
  T1CC3L = motor2_speed;
  T1CC3H = 0;

  /* Timer 3 configuration */
  T3CC0 = 100;     /* period */
  T3CCTL1 |= 0x24; /* channel 1 output compare; clear on compare, set on 0 */
  T3CC1 = motor1_speed;
  T3CTL |= 0x10; // start timer 3

  /* Timer 4 configuration */
  T4CC0 = 100;     /* period */
  T4CCTL1 |= 0x24; /* channel 1 output compare; clear on compare, set on 0 */
  T4CC1 = motor2_speed;
  T4CTL |= 0x10; // start timer 4
#endif

  //P0SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK); /* general-purpose IO */
  //P0DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK); /* for output*/

  P0SEL &= ~(MOTOR1_PIN2_MASK | MOTOR2_PIN2_MASK); /* general-purpose IO */
  P1SEL &= ~(MOTOR1_PIN1_MASK | MOTOR2_PIN1_MASK);

  P0DIR |=  (MOTOR1_PIN2_MASK | MOTOR2_PIN2_MASK); /* for output*/
  P1DIR |=  (MOTOR1_PIN1_MASK | MOTOR2_PIN1_MASK);

  MOTOR1_PIN1 = 0;
  MOTOR1_PIN2 = 0;
  MOTOR2_PIN1 = 0;
  MOTOR2_PIN2 = 0;

  motors_status = 0;

  P1SEL &= ~LED1_MASK;
  P1DIR |= LED1_MASK;
  LED1_PIN = 0;

  leds_status = 0;
}

void motor_set_speed(char motor_number, char speed)
{

  if (motor_number == 1) {
    if ((motors_status & MOTOR1_MASK) == MOTOR1_CW) {
      T3CC1 = speed;
    } else if ((motors_status & MOTOR1_MASK) == MOTOR1_CCW) {
      T1CC2L = speed;
      T1CC2H = 0;
    }
  } else if (motor_number == 2) {
    if ((motors_status & MOTOR2_MASK) == MOTOR2_CW) {
      T4CC1 = speed;
    } else if ((motors_status & MOTOR2_MASK) == MOTOR2_CCW) {
      T1CC3L = speed;
      T1CC3H = 0;
    }
  }
}

void motor_set(char motors)
{
  motors_status = motors;

#if CONFIG_MOTOR_PWM_ENABLE
  if ((motors & MOTOR1_MASK) == MOTOR1_OFF) {
    P1SEL &= ~MOTOR1_PIN1_MASK; // general-purpose IO
    P0SEL &= ~MOTOR1_PIN2_MASK; // general-purpose IO
    MOTOR1_PIN1 = 0;
    MOTOR1_PIN2 = 0;

  } else {
    if ((motors & MOTOR1_MASK) == MOTOR1_CW) {
      P1SEL |=  MOTOR1_PIN1_MASK; // peripheral IO
      P0SEL &= ~MOTOR1_PIN2_MASK; // general-purpose IO
      MOTOR1_PIN2 = 0;
      motor_set_speed(1, motor1_speed);
    } else if ((motors & MOTOR1_MASK) == MOTOR1_CCW) {
      P1SEL &= ~MOTOR1_PIN1_MASK; // general-purpose IO
      P0SEL |=  MOTOR1_PIN2_MASK; // peripheral IO
      MOTOR1_PIN1 = 0;
      motor_set_speed(1, motor1_speed);
    }
  }

  if ((motors & MOTOR2_MASK) == MOTOR2_OFF) {
    P1SEL &= ~MOTOR2_PIN1_MASK; // general-purpose IO
    P0SEL &= ~MOTOR2_PIN2_MASK; // general-purpose IO
    MOTOR2_PIN1 = 0;
    MOTOR2_PIN2 = 0;

  } else {
    if ((motors & MOTOR2_MASK) == MOTOR2_CW) {
      P1SEL |=  MOTOR2_PIN1_MASK; // peripheral IO
      P0SEL &= ~MOTOR2_PIN2_MASK; // general-purpose IO
      MOTOR2_PIN2 = 0;
      motor_set_speed(2, motor2_speed);
    } else if ((motors & MOTOR2_MASK) == MOTOR2_CCW) {
      P1SEL &= ~MOTOR2_PIN1_MASK; // general-purpose IO
      P0SEL |= MOTOR2_PIN2_MASK; // peripheral IO
      MOTOR2_PIN1 = 0;
      motor_set_speed(2, motor2_speed);
    }
  }
#else
  MOTOR1_PIN1 = motors & 0x01;
  MOTOR1_PIN2 = (motors & 0x02) >> 1;
  MOTOR2_PIN1 = (motors & 0x04) >> 2;
  MOTOR2_PIN2 = (motors & 0x08) >> 3;
#endif
}

char motor_get(void)
{
  return motors_status;
}

void led_set(char leds)
{
  LED1_PIN = leds & 0x01;
  leds_status = leds;
}

char led_get(void)
{
  return leds_status;
}

int json_parser(char *json_string)
{
  int i, r, res;
  jsmn_parser p;
  jsmntok_t t[JSON_MAX_TOKEN];
  char buffer[JSON_STRING_BUFFER_SIZE];
  char * pEnd = NULL;

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
                i++;
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
                i++;
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
                i++;
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

  for (i = 1; i < r; i++) {

          if (jsoneq(json_string, &t[i], "led1") == 0) {
#ifdef CONFIG_DONT_HAVE_SPRINTF_WITH_STRING_LENGTH_FORMATTING
                        strncpy(buffer, json_string + t[i+1].start, t[i+1].end-t[i+1].start);
                        res = t[i+1].end-t[i+1].start;
#else
                        res = sprintf(buffer, "%.*s", t[i+1].end-t[i+1].start,
                                      json_string + t[i+1].start);
#endif
                        if(res) {
#ifdef CONFIG_DONT_HAVE_STRTOL
                          if (atol(buffer) == 0) {leds_status &= ~LED1; rf_send("1of", 3);}
                          if (atol(buffer) == 1) {leds_status |= LED1; rf_send("1on", 3);}
#else
                          if (strtol(buffer, &pEnd, 10) == 0) {leds_status &= ~LED1; rf_send("1of", 3);}
                          if (strtol(buffer, &pEnd, 10) == 1) {leds_status |= LED1; rf_send("1on", 3);}
#endif
                        }
                        i++;
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

  led_set(leds_status);

  return 0;
}
