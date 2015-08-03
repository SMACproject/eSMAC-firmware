/*
 * motor.h
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

#ifndef MOTOR_H_
#define MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR1_1 0x01
#define MOTOR1_2 0x02
#define MOTOR2_1 0x04
#define MOTOR2_2 0x08

#define MOTOR1_MASK (MOTOR1_1 | MOTOR1_2)
#define MOTOR2_MASK (MOTOR2_1 | MOTOR2_2)

#define MOTOR1_OFF    0x00
#define MOTOR1_CW     MOTOR1_1
#define MOTOR1_CCW    MOTOR1_2
#define MOTOR1_BRAKE  (MOTOR1_1 | MOTOR1_2)

#define MOTOR2_OFF    0x00
#define MOTOR2_CW     MOTOR2_1
#define MOTOR2_CCW    MOTOR2_2
#define MOTOR2_BRAKE  (MOTOR2_1 | MOTOR2_2)

extern char motor1_speed;
extern char motor2_speed;

void motor_init(void);
void motor_set(char motors);
void motor_set_speed(char motor_number, char speed);
char motor_get(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_H_ */
