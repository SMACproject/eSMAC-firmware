/*
 * lin.h
 *
 * Created on: Sep 6, 2015
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


#ifndef SRC_ULIN_H_
#define SRC_ULIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ONE_WIRE_TX   0

#if (UART_ONE_WIRE_PORT == 0)
#define ulin_get_data       uart0_get_data
#define ulin_rxbuf          uart0_rxbuf
#define ulin_rxpos          uart0_rxpos
#define ulin_rxlen          uart0_rxlen
#define ulin_send           uart0_send
#define ulin_flush_rxbuf    uart0_flush_rxbuf
#define ulin_rx_mode        uart0_rx_mode
#define ulin_tx_mode        uart0_tx_mode
#else
#define ulin_get_data       uart1_get_data
#define ulin_rxbuf          uart1_rxbuf
#define ulin_rxpos          uart1_rxpos
#define ulin_rxlen          uart1_rxlen
#define ulin_send           uart1_send
#define ulin_flush_rxbuf    uart1_flush_rxbuf
#define ulin_rx_mode        uart1_rx_mode
#define ulin_tx_mode        uart1_tx_mode
#endif

void ulin_init(void);
uint8_t ulin_synced (uint8_t * ulin_frame);
void ulin_handler(uint8_t * ulin_frame);
void ulin_service(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_ULIN_H_ */
