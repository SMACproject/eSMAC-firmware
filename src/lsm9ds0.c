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
#include <stdint.h>
#include <string.h>
#include "spi.h"
#include "lsm9ds0.h"
#include "lsm330dlc.h"

int lsm9ds0_gyro_init(void)
{
      /* initialize gyro */
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
