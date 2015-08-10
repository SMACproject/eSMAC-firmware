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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "led.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "jsmn.h"
#include "json.h"

/* TODO workaround */
#include "radio.h"
#include "sensor.h"

#define LED1_PIN   P1_0
#define LED2_PIN   P1_1
#define LED3_PIN   P1_4
#define LED4_PIN   P2_0

#define LED1_MASK  0x01
#define LED2_MASK  0x02
#define LED3_MASK  0x10
#define LED4_MASK  0x01

static char leds_status = 0;

uint16_t sensor_reading;
float sane = 0;
uint8_t dec;
float frac;

char reply_buf[128];
int  reply_len = 0;

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

/* TODO this is not supposed to be here anymore */
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
          } else if (jsoneq(json_string, &t[i], "led2") == 0) {
#ifdef CONFIG_DONT_HAVE_SPRINTF_WITH_STRING_LENGTH_FORMATTING
            strncpy(buffer, json_string + t[i+1].start, t[i+1].end-t[i+1].start);
            res = t[i+1].end-t[i+1].start;
#else
            res = sprintf(buffer, "%.*s", t[i+1].end-t[i+1].start,
                          json_string + t[i+1].start);
#endif
            if(res) {
#ifdef CONFIG_DONT_HAVE_STRTOL
              if (atol(buffer) == 0) {leds_status &= ~LED2; rf_send("2of", 3);}
              if (atol(buffer) == 1) {leds_status |= LED2; rf_send("2on", 3);}
#else
              if (strtol(buffer, &pEnd, 10) == 0) {leds_status &= ~LED2; rf_send("2of", 3);}
              if (strtol(buffer, &pEnd, 10) == 1) {leds_status |= LED2; rf_send("2on", 3);}
#endif
            }
            i++;
          } else if (jsoneq(json_string, &t[i], "led3") == 0) {
#ifdef CONFIG_DONT_HAVE_SPRINTF_WITH_STRING_LENGTH_FORMATTING
            strncpy(buffer, json_string + t[i+1].start, t[i+1].end-t[i+1].start);
            res = t[i+1].end-t[i+1].start;
#else
            res = sprintf(buffer, "%.*s", t[i+1].end-t[i+1].start,
                          json_string + t[i+1].start);
#endif
            if(res) {
#ifdef CONFIG_DONT_HAVE_STRTOL
              if (atol(buffer) == 0) {leds_status &= ~LED3; rf_send("3of", 3);}
              if (atol(buffer) == 1) {leds_status |= LED3; rf_send("3on", 3);}
#else
              if (strtol(buffer, &pEnd, 10) == 0) {leds_status &= ~LED3; rf_send("3of", 3);}
              if (strtol(buffer, &pEnd, 10) == 1) {leds_status |= LED3; rf_send("3on", 3);}
#endif
            }
            i++;
          } else if (jsoneq(json_string, &t[i], "led4") == 0) {
#ifdef CONFIG_DONT_HAVE_SPRINTF_WITH_STRING_LENGTH_FORMATTING
            strncpy(buffer, json_string + t[i+1].start, t[i+1].end-t[i+1].start);
            res = t[i+1].end-t[i+1].start;
#else
            res = sprintf(buffer, "%.*s", t[i+1].end-t[i+1].start,
                          json_string + t[i+1].start);
#endif
            if(res) {
#ifdef CONFIG_DONT_HAVE_STRTOL
              if (atol(buffer) == 0) {leds_status &= ~LED4; rf_send("4of", 3);}
              if (atol(buffer) == 1) {leds_status |= LED4; rf_send("4on", 3);}
#else
              if (strtol(buffer, &pEnd, 10) == 0) {leds_status &= ~LED4; rf_send("4of", 3);}
              if (strtol(buffer, &pEnd, 10) == 1) {leds_status |= LED4; rf_send("4on", 3);}
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

  /*
   * TODO temporary temperature and battery sensor JSON string handler
   */
  for (i = 1; i < r; i++) {
    if (jsoneq(json_string, &t[i], "sensor") == 0) {
                  if (jsoneq(json_string, &t[i+1], "temperature") == 0) {
                    /*
                     * Temperature:
                     * Using 1.25V ref. voltage (1250mV).
                     * Typical AD Output at 25 degC: 1480
                     * Typical Co-efficient     : 4.5 mV/degC
                     *
                     * Thus, at 12bit decimation (and ignoring the VDD co-efficient as well
                     * as offsets due to lack of calibration):
                     *
                     *          AD - 1480
                     * T = 25 + ---------
                     *              4.5
                     */
                    sensor_reading = sensor_temperature();
                    sane = 25 + ((sensor_reading - 1480) / 4.5);
                    dec = sane;
                    frac = sane - dec;
                    memset(reply_buf, 0, sizeof(reply_buf));
                    sprintf(reply_buf,"Temp=%d.%02u C (%d)\n", dec, (unsigned int)(frac*100), sensor_reading);
                    rf_send(reply_buf, 25);
                  }
                  if (jsoneq(json_string, &t[i+1], "battery") == 0) {
                    /*
                     * Power Supply Voltage.
                     * Using 1.25V ref. voltage.
                     * AD Conversion on VDD/3
                     *
                     * Thus, at 12bit resolution:
                     *
                     *          ADC x 1.15 x 3
                     * Supply = -------------- V
                     *               2047
                     */
                    sensor_reading = sensor_battery();
                    sane = sensor_reading * 1.15 * 3 / 2047;
                    dec = sane;
                    frac = sane - dec;
                    memset(reply_buf, 0, sizeof(reply_buf));
                    sprintf(reply_buf,"Supply=%d.%02u V (%d)\n", dec, (unsigned int)(frac*100), sensor_reading);
                    rf_send(reply_buf, 25);
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

  /*
   * end of temporary temperature and battery sensor JSON string handler
   */

  return 0;
}
