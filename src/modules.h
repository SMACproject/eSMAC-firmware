/*
 * modules.h
 *
 * Created on: Aug 10, 2015
 *     Author: nodino
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

#ifndef SRC_MODULES_H_
#define SRC_MODULES_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined MODULE_BLUETOOTH
#include "bluetooth.h"
#define module_init(x) bluetooth_init(x)
#elif defined MODULE_LED
#include "led.h"
#define module_init(x) led_init(x)
#elif defined MODULE_MOTOR
#include "motor.h"
#define module_init(x) motor_init(x)
#elif defined MODULE_IMU
#include "lsm330dlc.h"
#include "lsm9ds0.h"
#define module_init(x) imu_init(x)
#elif defined MODULE_BATTERY_WITH_SENSOR
#define module_init(x) battery_sensor_init(x)
#else
#define module_init(x)
#error modules.h: module_init(x) is not defined, building a module requires initialization!
#endif

#ifdef __cplusplus
}
#endif

#endif /* SRC_MODULES_H_ */
