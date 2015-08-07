/*
 * sensor.c
 *
 * Created on: Aug 6, 2015
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

#include <stdint.h>
#include <string.h>
#include "config.h"
#include "cc253x.h"
#include "sfr-bits.h"

uint16_t sensor_temperature(void)
{
  uint16_t reading;
  /*
   * For single-shot AD conversions, we may only write to ADCCON3[3:0] once
   * (This write triggers the conversion). We thus use the variable 'command'
   * to store intermediate steps (reference, decimation rate, input channel)
   */
  uint8_t command;

  /* 1.25V ref, max decimation rate */
  command = ADCCON3_EDIV1 | ADCCON3_EDIV0;

  /* TR0 or TESTREG0 value is not retained during LPM2, that is why we reconfigure
   * it again in here
   */
#if defined __IAR_SYSTEMS_ICC__
  TR0 = 1;
#else
  TESTREG0 = 1;
#endif

  /* Clear the Interrupt Flag */
  ADCIF = 0;

  /* Depending on the desired reading, append the input bits to 'command' and
   * enable the corresponding input channel in ADCCFG if necessary */
  command |= ADCCON3_ECH3 | ADCCON3_ECH2 | ADCCON3_ECH1;

  /* Writing in bits 3:0 of ADCCON3 will trigger a single conversion */
  ADCCON3 = command;

  /*
   * When the conversion is complete, the ADC interrupt flag is set. We don't
   * use an ISR here, we just wait on the flag and clear it afterwards.
   */
  while(!ADCIF);

  /* Clear the Interrupt Flag */
  ADCIF = 0;

  reading = ADCL;
  reading |= (((uint8_t) ADCH) << 8);
  /* 12-bit decimation rate: 4 LS bits are noise */
  reading >>= 4;

  return reading;
}

uint16_t sensor_battery (void)
{
  uint16_t reading;
  /*
   * For single-shot AD conversions, we may only write to ADCCON3[3:0] once
   * (This write triggers the conversion). We thus use the variable 'command'
   * to store intermediate steps (reference, decimation rate, input channel)
   */
  uint8_t command;

  /* 1.25V ref, max decimation rate */
  command = ADCCON3_EDIV1 | ADCCON3_EDIV0;

  /* TR0 or TESTREG0 value is not retained during LPM2, that is why we reconfigure
   * it again in here
   */
#if defined __IAR_SYSTEMS_ICC__
  TR0 = 1;
#else
  TESTREG0 = 1;
#endif

  /* Clear the Interrupt Flag */
  ADCIF = 0;

  /* Depending on the desired reading, append the input bits to 'command' and
   * enable the corresponding input channel in ADCCFG if necessary */
  command |= ADCCON3_ECH3 | ADCCON3_ECH2 | ADCCON3_ECH1 | ADCCON3_ECH0;

  /* Writing in bits 3:0 of ADCCON3 will trigger a single conversion */
  ADCCON3 = command;

  /*
   * When the conversion is complete, the ADC interrupt flag is set. We don't
   * use an ISR here, we just wait on the flag and clear it afterwards.
   */
  while(!ADCIF);

  /* Clear the Interrupt Flag */
  ADCIF = 0;

  reading = ADCL;
  reading |= (((uint8_t) ADCH) << 8);
  /* 12-bit decimation rate: 4 LS bits are noise */
  reading >>= 4;

  return reading;
}

