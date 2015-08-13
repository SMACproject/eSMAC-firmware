/*
 * lsm9ds0.c
 *
 * Created on : Sept 24, 2014
 *     Author : Ekawahyu Susilo
 *
 * Copyright (c) 2014, STORM Lab Vanderbilt University.
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
#include "clock.h"
#include "spi.h"
#include "lsm9ds0.h"
#include "lsm330dlc.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "jsmn.h"
#include "json.h"

/* TODO workaround */
#include "radio.h"
#include "flash.h"

#define LED1_PIN   P1_0
#define LED1_MASK  0x01

#define LED1 0x01
#define LED_ALL LED1

static char leds_status = 0;

char is_streaming_imu = 0;

uint8_t flash_buffer[4];
uint8_t read_buffer[4];
uint16_t flash_count = 0;
uint16_t read_count = 0;

struct gyro_context mygyro;
struct accelerometer_context myaccel;

char reply_buf[128];
int  reply_len = 0;

void imu_init(void)
{
  spi_init();
  lsm9ds0_gyro_init();
  lsm9ds0_accelerometer_init();

  P1SEL &= ~LED1_MASK;
  P1DIR |= LED1_MASK;
  LED1_PIN = 0;

  leds_status = 0;
}

int lsm9ds0_gyro_init(void)
{
      /* initialize gyroscope */
      spi_select(LSM9DS0_GYRO_CS);
      spi_write(CTRL_REG1_G);
      spi_write((DRBW_1000 | LPen_G | xyz_en_G));
      spi_deselect(LSM9DS0_GYRO_CS);

      spi_select(LSM9DS0_GYRO_CS);
      spi_write(CTRL_REG4_G);
      spi_write(GYR_2000_DPS);
      spi_deselect(LSM9DS0_GYRO_CS);

      /*
      spi_select(LSM9DS0_GYRO_CS);
      spi_write(WHO_AM_I_G);
      return_spi = spi_read();
      spi_deselect(LSM9DS0_GYRO_CS);
      */
      
      return 0;
}

void lsm9ds0_gyro_acquire(int16_t * x, int16_t * y, int16_t * z)
{
    unsigned char read_byte_high;
    unsigned char read_byte_low;
    
    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_X_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);

    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_X_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);

    *x = read_byte_high;
    *x = ((*x << 8) & 0xFF00) | read_byte_low;
    
    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_Y_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);

    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_Y_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);

    *y = read_byte_high;
    *y = ((*y << 8) & 0xFF00) | read_byte_low;

    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_Z_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);

    spi_select(LSM9DS0_GYRO_CS);
    spi_write(GYR_REG_OUT_Z_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_GYRO_CS);
    
    *z = read_byte_high;
    *z = ((*z << 8) & 0xFF00) | read_byte_low;
}

int lsm9ds0_accelerometer_init(void)
{
      /* initialize accelerometer */
      spi_select(LSM9DS0_ACCELEROMETER_CS);
      spi_write(CTRL_REG1_A);
      spi_write(ACC_400_Hz_A | xyz_en_A);
      spi_deselect(LSM9DS0_ACCELEROMETER_CS);

      spi_select(LSM9DS0_ACCELEROMETER_CS);
      spi_write(CTRL_REG4_G);
      spi_write(ACC_2G_A | HR_A);
      spi_deselect(LSM9DS0_ACCELEROMETER_CS);
      
      return 0;
}

void lsm9ds0_accelerometer_acquire(int16_t * x, int16_t * y, int16_t * z)
{
    unsigned char read_byte_high;
    unsigned char read_byte_low;
        
    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_X_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_X_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    *x = read_byte_high;
    *x = ((*x << 8) & 0xFF00) | read_byte_low;
    
    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_Y_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_Y_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    *y = read_byte_high;
    *y = ((*y << 8) & 0xFF00) | read_byte_low;
    
    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_Z_L);
    read_byte_low = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    spi_select(LSM9DS0_ACCELEROMETER_CS);
    spi_write(ACC_REG_OUT_Z_H);
    read_byte_high = spi_read();
    spi_deselect(LSM9DS0_ACCELEROMETER_CS);

    *z = read_byte_high;
    *z = ((*z << 8) & 0xFF00) | read_byte_low;
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

    if (jsoneq(json_string, &t[i], "imu") == 0) {
                if (jsoneq(json_string, &t[i+1], "record") == 0) {
                  flash_count = 0;
                  flash_erase_page(112);
                  flash_erase_page(113);
                  flash_erase_page(114);
                  led_set(0);
                  led_set(LED1);
                  while (flash_count < 6000) {
                    lsm9ds0_gyro_acquire(&mygyro.x, &mygyro.y, &mygyro.z);
                    lsm9ds0_accelerometer_acquire(&myaccel.x, &myaccel.y, &myaccel.z);
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
                  led_set(0);
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
                    memset(reply_buf, 0, sizeof(reply_buf));
                    sprintf(reply_buf, "\n%5.0i\t%5.0i\t%5.0i\t%5.0i\t%5.0i\t%5.0i", myaccel.x, myaccel.y, myaccel.z, mygyro.x, mygyro.y, mygyro.z);
                    rf_send(reply_buf, 60);
                    clock_delay_usec(10000);
                  }
                  led_set(0);
                }
                if (jsoneq(json_string, &t[i+1], "stream") == 0) {
                  led_set(0);
                  led_set(LED1);
                  read_count = 0;

                  /* do something here */

                  led_set(0);
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
