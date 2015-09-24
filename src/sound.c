/*
 * sound.c
 *
 * Created on: Sep 16, 2015
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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "clock.h"
#include "rtimer.h"
#include "sound.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "jsmn.h"
#include "json.h"

#define LED1_PIN   P1_0
#define LED1_MASK  0x01

#define LED1 0x01
#define LED_ALL LED1

#define BUZZER1_PIN1   P1_4
#define BUZZER1_PIN2   P0_4
#define BUZZER2_PIN1   P1_1
#define BUZZER2_PIN2   P0_5

#define BUZZER1_PIN1_MASK  0x10
#define BUZZER1_PIN2_MASK  0x10
#define BUZZER2_PIN1_MASK  0x02
#define BUZZER2_PIN2_MASK  0x20

struct rtimer sound_tick;
static volatile uint8_t sound_flag = 0;
static char leds_status = 0;

void led_set(char leds)
{
  LED1_PIN = leds & 0x01;
  leds_status = leds;
}

char led_get(void)
{
  return leds_status;
}

void sound_callback(void)
{
  sound_flag = 0;
}

void sound_play(uint16_t note, uint16_t duration)
{
  rtimer_set(&sound_tick, rtimer_now() + duration, sound_callback);
  sound_flag = 1;

  while(sound_flag) {
    if (note) {
      BUZZER1_PIN1 = 0;
      BUZZER1_PIN2 = 0;
      BUZZER2_PIN1 = 0;
      BUZZER2_PIN2 = 1;
      clock_delay_usec(note);
      BUZZER1_PIN1 = 0;
      BUZZER1_PIN2 = 0;
      BUZZER2_PIN1 = 0;
      BUZZER2_PIN2 = 0;
      clock_delay_usec(note);
    }
  }

  clock_delay_usec(10000);
}

void sound_init(void)
{
  P0SEL &= ~(BUZZER1_PIN2_MASK | BUZZER2_PIN2_MASK); /* general-purpose IO */
  P1SEL &= ~(BUZZER1_PIN1_MASK | BUZZER2_PIN1_MASK);

  P0DIR |=  (BUZZER1_PIN2_MASK | BUZZER2_PIN2_MASK); /* for output*/
  P1DIR |=  (BUZZER1_PIN1_MASK | BUZZER2_PIN1_MASK);

  BUZZER1_PIN1 = 0;
  BUZZER1_PIN2 = 0;
  BUZZER2_PIN1 = 0;
  BUZZER2_PIN2 = 0;

  P1SEL &= ~LED1_MASK;
  P1DIR |= LED1_MASK;
  LED1_PIN = 0;

  leds_status = 0;

  /*
  1 - E E E C E G G
  2 - C G E A B A# A G
  3 - E G A F G E C D B
  2 - C G E A B A# A G
  3 - E G A F G E C D B
  4 - G F# F D# E
  5 - A A C A C D
  4 - G F# F D# E + C C C
  4 - G F# F D# E
  5 - A A C A C D
  4 - G F# F D# E
  5 - A A C A C D + C# D C

  6 - C C C C D E C A G
  C C C D E
  6 - C C C C D E C A G
  1 - E E E C E G...G!
  */

  sound_play(E5, SOUND_SECOND >> 4);
  sound_play(0 , SOUND_SECOND >> 4);
  sound_play(E5, SOUND_SECOND >> 4);
  sound_play(E5, SOUND_SECOND >> 4);
  sound_play(0 , SOUND_SECOND >> 4);
  sound_play(C5, SOUND_SECOND >> 4);
  sound_play(E5, SOUND_SECOND >> 4);
  sound_play(0 , SOUND_SECOND >> 4);
  sound_play(G5, SOUND_SECOND >> 4);
  sound_play(0 , SOUND_SECOND);
}

int json_parser(char *json_string, uint8_t len)
{
  int i, r, res;
  jsmn_parser p;
  jsmntok_t t[JSON_MAX_TOKEN];
  char buffer[JSON_STRING_BUFFER_SIZE];
  char * pEnd = NULL;

  memset(buffer, 0, sizeof(buffer));
  jsmn_init(&p);

  r = jsmn_parse(&p, json_string, len, t, sizeof(t)/sizeof(t[0]));
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
                  led_set(LED1);
                  sound_play(G5, SOUND_SECOND >> 4);
                  sound_play(C6, SOUND_SECOND >> 5);
                  sound_play(G6, SOUND_SECOND >> 4);
                  led_set(0);
                  return 0;
                }
                if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                  led_set(LED1);
                  sound_play(G6, SOUND_SECOND >> 4);
                  sound_play(C6, SOUND_SECOND >> 5);
                  sound_play(G5, SOUND_SECOND >> 4);
                  led_set(0);
                  return 0;
                }
                i++;
    } else if (jsoneq(json_string, &t[i], "motor2") == 0) {
                if (jsoneq(json_string, &t[i+1], "cw") == 0) {
                  led_set(LED1);
                  sound_play(G5, SOUND_SECOND >> 4);
                  sound_play(C6, SOUND_SECOND >> 5);
                  sound_play(G6, SOUND_SECOND >> 4);
                  led_set(0);
                  return 0;
                }
                if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                  led_set(LED1);
                  sound_play(G6, SOUND_SECOND >> 4);
                  sound_play(C6, SOUND_SECOND >> 5);
                  sound_play(G5, SOUND_SECOND >> 4);
                  led_set(0);
                  return 0;
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

  return 0;
}
