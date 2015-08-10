/*
 * main.c
 *
 * Created on: Jul 7, 2015
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
#include "config.h"
#include "modules.h"
#include "cc253x.h"
#include "sfr-bits.h"
#include "clock.h"
#include "rtimer.h"
#include "uart.h"
#include "jsmn.h"
#include "json.h"
#include "spi.h"
#include "flash.h"
#include "radio.h"

/*#define FORWARD 0x06
#define BACKWARD 0x09
#define TURN_LEFT 0x04
#define TURN_RIGHT 0x02
#define ROTATE_LEFT 0x05
#define ROTATE_RIGHT 0x0A

#define STEP1 0x0A
#define STEP2 0x02
#define STEP3 0x06
#define STEP4 0x04
#define STEP5 0x05
#define STEP6 0x01
#define STEP7 0x09
#define STEP8 0x08

#define SPEED 3
#define SPEED_STEP 20*/

enum {
  UART0_IDLE,
  UART0_RECEIVING,
  UART0_RECEIVED,
  UART0_SENDING,
  UART0_SENT
};

/*void delay_step(void)
{
  volatile uint16_t loop;
  for (loop=0; loop < SPEED_STEP; loop++) clock_delay_usec(100);
}*/

/*char is_auto_run = 0;
char is_streaming_imu = 0;*/

static uint8_t uart0_state;

/*void timer1_init();
void timer1_disbale();
void timer1_enable();*/

void main(void)
{
  /*volatile uint16_t loop;
  volatile uint8_t move1, move2;
  volatile uint8_t mydata1 = 0;
  volatile uint8_t mydata2 = 0;
  volatile uint8_t loopback1, loopback2;*/
  
  EA = 0;
  
  clock_init();
/*#if OBJECT_LED
  led_init();
#endif
#if OBJECT_LED1
  led1_init();
#endif*/

  clock_delay_usec(60000); // workaround to wait for LSM9DS0 ready

#if SPI1_ENABLE
  spi_init();
  lsm9ds0_gyro_init();
  lsm9ds0_accelerometer_init();
#endif
  uart_init();
  rtimer_init();
  rf_init();

  module_init();

  // comment this line out to prevent bluetooth board from crashing
  printf("\nSMAC2.0 - [%x:%x]\n", SHORT_ADDR1, SHORT_ADDR0);
  
  EA = 1;
  MEMCTR = 7;
  
  /* connect temperature sensor to the SoC */
  ATEST = 1;
#if defined __IAR_SYSTEMS_ICC__
  TR0 = 1;
#else
  TESTREG0 = 1;
#endif
  APCFG = 0; /* disables input channels */
  
  while(1)
  {
    if( uart0_state == UART0_RECEIVED )
    {
      uart0_state = UART0_SENDING;
      rf_send(serial_rxbuf , serial_rxlen);
      uart0_flush_rxbuf();
      uart0_state = UART0_IDLE;
    }
#if OBJECT_MOTOR
    if ( is_auto_run )
    {
      for (move1=0; move1<8; move1++){
        led_set(LED1);
        motor_set(FORWARD);
        clock_delay_usec(60000);
        motor_set(0);
        for (loop=0; loop < SPEED; loop++) clock_delay_usec(60000);
        led_set(0);
        
        if (!is_auto_run) break;
        
        motor_set(TURN_LEFT);
        clock_delay_usec(60000);
        motor_set(0);
        for (loop=0; loop < SPEED; loop++) clock_delay_usec(60000);
      }
      
      for (move2=0; move2<8; move2++){
        led_set(LED1);
        motor_set(FORWARD);
        clock_delay_usec(60000);
        motor_set(0);
        for (loop=0; loop < SPEED; loop++) clock_delay_usec(60000);
        led_set(0);
        
        if (!is_auto_run) break;
        
        motor_set(TURN_RIGHT);
        clock_delay_usec(60000);
        motor_set(0);
        for (loop=0; loop < SPEED; loop++) clock_delay_usec(60000);
      }
    }
    
    /*
    motor_set(STEP1);
    delay_step();
    motor_set(STEP2);
    delay_step();
    motor_set(STEP3);
    delay_step();
    motor_set(STEP4);
    delay_step();
    motor_set(STEP5);
    delay_step();
    motor_set(STEP6);
    delay_step();
    motor_set(STEP7);
    delay_step();
    motor_set(STEP8);
    delay_step();
    */
#endif
  }
}

void uart0_receiving_timeout (void)
{
  if (uart0_state == UART0_RECEIVING) {
    // print it to console
    uart0_sendbuf( serial_rxbuf , serial_rxlen);
    uart0_state = UART0_RECEIVED;
  }
  //led_set(0);
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX0_VECTOR
__interrupt void uart0_isr(void)
#else
void uart0_isr (void) __interrupt (URX0_VECTOR)
#endif
{
  URX0IF = 0;

  if (serial_rxpos >= 128 || uart0_state == UART0_SENDING) return;

  serial_rxbuf[serial_rxpos] = U0DBUF;
  serial_rxpos++;
  serial_rxlen++;

  uart0_state = UART0_RECEIVING;
  rtimer_schedule(100, uart0_receiving_timeout);
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=URX1_VECTOR
__interrupt void uart1_isr(void)
#else
void uart1_isr (void) __interrupt (URX1_VECTOR)
#endif
{
  URX1IF = 0;
}

/*
#pragma vector=T1_VECTOR
__interrupt void Timer1_ISR(void)
{
  T1STAT &= ~( 1<< 0);

  is_serial_receive = 1;
  timer1_disable();
}
*/

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=RF_VECTOR
__interrupt void rf_isr(void)
#else
void rf_isr (void) __interrupt (RF_VECTOR)
#endif
{
  EA = 0;

  if (RFIRQF0 & ( 1<<6 ))
  {
    rf_receive_isr();

    S1CON = 0;
    RFIRQF0 &= ~(1<<6);
  }
  EA = 1;
}

#if defined __IAR_SYSTEMS_ICC__
#pragma vector=T1_VECTOR
__interrupt void rtimer_isr(void)
#else
void rtimer_isr (void) __interrupt (T1_VECTOR)
#endif
{
  T1IE = 0; /* Ignore Timer 1 Interrupts */

  if (T1STAT & T1STAT_CH0IF) {
    /* No more interrupts from Channel 1 till next rtimer_arch_schedule() call */
    T1STAT &= ~T1STAT_CH0IF;
    T1CCTL0 &= ~T1CCTL_IM;

    /* Callback function call */
    if (rtimer_callback) rtimer_callback();

    /* Release rtimer lock and callback */
    rtimer_busy = 0;
    rtimer_callback = NULL;

    //rtimer_schedule(27, toggle_led);
  }

  T1IE = 1; /* Acknowledge Timer 1 Interrupts */
}
