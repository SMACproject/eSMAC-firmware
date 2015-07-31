/*
 * json.c
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
#include "jsmn.h"
#include "clock.h"
#include "led.h"
#include "motor.h"
#include "flash.h"
#include "radio.h"
#include "lsm330dlc.h"
#include "lsm9ds0.h"


/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

extern char is_auto_run;
extern char is_streaming_imu;

struct gyro_context mygyro;
struct accelerometer_context myaccel;

int counter = 0;
#if (OBJECT_LED | OBJECT_LED1)
char led = 0;
#endif
#if OBJECT_MOTOR
char motor = 0;
#endif

char imu_buf[128];
int  imu_len = 0;
  
uint8_t flash_buffer[4];
uint8_t read_buffer[4];
uint16_t flash_count = 0;
uint16_t read_count = 0;

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int json_parser(char *json_string) {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[20]; /* We expect no more than 20 tokens */
        
        char buffer[50];
        int res;
        char * pEnd;

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

#if OBJECT_LED
        led = led_get();
        
	/* Loop over all keys of the root object */
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
              if (atol(buffer) == 0) led &= ~LED1;
              if (atol(buffer) == 1) led |= LED1;
#else
              if (strtol(buffer, &pEnd, 10) == 0) led &= ~LED1;
              if (strtol(buffer, &pEnd, 10) == 1) led |= LED1;
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
              if (atol(buffer) == 0) led &= ~LED2;
              if (atol(buffer) == 1) led |= LED2;
#else
              if (strtol(buffer, &pEnd, 10) == 0) led &= ~LED2;
              if (strtol(buffer, &pEnd, 10) == 1) led |= LED2;
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
              if (atol(buffer) == 0) led &= ~LED3;
              if (atol(buffer) == 1) led |= LED3;
#else
              if (strtol(buffer, &pEnd, 10) == 0) led &= ~LED3;
              if (strtol(buffer, &pEnd, 10) == 1) led |= LED3;
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
              if (atol(buffer) == 0) led &= ~LED4;
              if (atol(buffer) == 1) led |= LED4;
#else
              if (strtol(buffer, &pEnd, 10) == 0) led &= ~LED4;
              if (strtol(buffer, &pEnd, 10) == 1) led |= LED4;
#endif
            }
            i++;
          } else {
            printf("Unexpected key: %.*s (%i:%i)\n", t[i].end-t[i].start,
                   json_string + t[i].start, t[i].type, t[i].size);
          }
	}
        
        led_set(led);
#endif
#if OBJECT_LED1
        led = led_get();
        
	/* Loop over all keys of the root object */
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
              if (atol(buffer) == 0) led &= ~LED1;
              if (atol(buffer) == 1) led |= LED1;
#else
              if (strtol(buffer, &pEnd, 10) == 0) led &= ~LED1;
              if (strtol(buffer, &pEnd, 10) == 1) led |= LED1;
#endif
            }
            i++;
          } else {
            /*printf("Unexpected key: %.*s (%i:%i)\n", t[i].end-t[i].start,
                   json_string + t[i].start, t[i].type, t[i].size);*/
          }
	}
        
        led_set(led);
#endif
#if OBJECT_MOTOR
        motor = motor_get();
        
	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(json_string, &t[i], "motor1") == 0) {
                  if (jsoneq(json_string, &t[i+1], "cw") == 0) {
                    motor |= MOTOR1_PIN1;
                    motor &= ~MOTOR1_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                    motor &= ~MOTOR1_PIN1;
                    motor |= MOTOR1_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "off") == 0) {
                    motor &= ~MOTOR1_PIN1;
                    motor &= ~MOTOR1_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "brake") == 0) {
                    motor |= MOTOR1_PIN1;
                    motor |= MOTOR1_PIN2;
                  }
                  i++;
		} else if (jsoneq(json_string, &t[i], "motor2") == 0) {
                  if (jsoneq(json_string, &t[i+1], "cw") == 0) {
                    motor |= MOTOR2_PIN1;
                    motor &= ~MOTOR2_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "ccw") == 0) {
                    motor &= ~MOTOR2_PIN1;
                    motor |= MOTOR2_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "off") == 0) {
                    motor &= ~MOTOR2_PIN1;
                    motor &= ~MOTOR2_PIN2;
                  }
                  if (jsoneq(json_string, &t[i+1], "brake") == 0) {
                    motor |= MOTOR2_PIN1;
                    motor |= MOTOR2_PIN2;
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
                } else {
                  /*printf("Unexpected key: %.*s (%i)\n", t[i].end-t[i].start,
                         json_string + t[i].start, t[i].size);*/
		}
	}
        
        motor_set(motor);
        clock_delay_usec(60000);
        motor_set(0);
#endif
#if OBJECT_IMU
	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(json_string, &t[i], "imu") == 0) {
                  if (jsoneq(json_string, &t[i+1], "record") == 0) {
                    flash_count = 0;
                    flash_erase_page(112);
                    flash_erase_page(113);
                    flash_erase_page(114);
                    led_set(LED1);
                    while (flash_count < 6000) {
                      mygyro = lsm9ds0_gyro_acquire();
                      myaccel = lsm9ds0_accelerometer_acquire();
                      flash_buffer[0] = (myaccel.x >> 8) & 0x00FF;
                      flash_buffer[1] = myaccel.x & 0x00FF;
                          flash_buffer[2] = (myaccel.y >> 8) & 0x00FF;
                          flash_buffer[3] = myaccel.y & 0x00FF;
                          flash_dma_write(flash_buffer, 4, ((uint32_t)112 << 11) + flash_count);
                          flash_count += 4;
                          
                          flash_buffer[0] = (myaccel.z >> 8) & 0x00FF;
                          flash_buffer[1] = myaccel.z & 0x00FF;
                          flash_buffer[2] = (mygyro.x >> 8) & 0x00FF;
                          flash_buffer[3] = mygyro.x & 0x00FF;
                          flash_dma_write(flash_buffer, 4, ((uint32_t)112 << 11) + flash_count);
                          flash_count += 4;
                          
                          flash_buffer[0] = (mygyro.y >> 8) & 0x00FF;
                          flash_buffer[1] = mygyro.y & 0x00FF;
                          flash_buffer[2] = (mygyro.z >> 8) & 0x00FF;
                          flash_buffer[3] = mygyro.z & 0x00FF;
                          flash_dma_write(flash_buffer, 4, ((uint32_t)112 << 11) + flash_count);
                          flash_count += 4;
                          clock_delay_usec(10000);
                     }
                    led_set(0);
                  }
                  if (jsoneq(json_string, &t[i+1], "download") == 0) {
                    led_set(LED1);
                    read_count = 0;
                    while(read_count < 6000){
                      flash_dma_read(read_buffer, 4, ((uint32_t)112 << 11) + read_count);
                      myaccel.x = (uint16_t)read_buffer[0] << 8 | read_buffer[1];
                      myaccel.y = (uint16_t)read_buffer[2] << 8 | read_buffer[3];
                      read_count += 4;
                      flash_dma_read(read_buffer, 4, ((uint32_t)112 << 11) + read_count);
                      myaccel.z = (uint16_t)read_buffer[0] << 8 | read_buffer[1];
                      mygyro.x = (uint16_t)read_buffer[2] << 8 | read_buffer[3];
                      read_count += 4;
                      flash_dma_read(read_buffer, 4, ((uint32_t)112 << 11) + read_count);
                      mygyro.y = (uint16_t)read_buffer[0] << 8 | read_buffer[1];
                      mygyro.z = (uint16_t)read_buffer[2] << 8 | read_buffer[3];
                      read_count += 4;
                      sprintf(imu_buf, "\n%5.0i\t%5.0i\t%5.0i\t%5.0i\t%5.0i\t%5.0i", myaccel.x, myaccel.y, myaccel.z, mygyro.x, mygyro.y, mygyro.z);
                      rf_send(imu_buf, 60);
                      clock_delay_usec(10000);
                    }
                    led_set(0);
                  }
                  if (jsoneq(json_string, &t[i+1], "stream") == 0) {
                    led_set(LED1);
                    read_count = 0;
                    
                    /* do something here */
                    
                    led_set(0);
                  }
                  i++;
		} else {
                  printf("Unexpected key: %.*s (%i)\n", t[i].end-t[i].start,
                         json_string + t[i].start, t[i].size);
		}
	}
#endif
        return 0;
}
