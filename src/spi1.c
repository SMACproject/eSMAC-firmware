/*
 * spi1.c
 *
 * Created on: Mar 10, 2014
 *     Author: Ekawahyu Susilo
 *
 * Copyright (c) 2014, Chongqing Aisenke Electronic Technology Co., Ltd.
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

#include "spi.h"
#include "spi1.h"
#include "spi-arch.h"
#include "cc253x.h"
#include "sfr-bits.h"

#define SPI1_MISO_PIN   P1_7
#define SPI1_MOSI_PIN   P1_6
#define SPI1_CLK_PIN    P1_5

#define SPI1_MISO_MASK  0x80
#define SPI1_MOSI_MASK  0x40
#define SPI1_CLK_MASK   0x20

#if SPI1_ENABLE
static char spi1_locked = 0;

void
spi1_init(unsigned char	mode,
    unsigned char	freq,
    unsigned char	endianess)
{
  /* USART1 is configured as SPI master mode */
  U1CSR &= ~0xA0;

  /* USART1 is set to use alternative 2 location */
  PERCFG |= PERCFG_U1CFG;
  P1SEL |= SPI1_MISO_MASK | SPI1_MOSI_MASK | SPI1_CLK_MASK;
  P1DIR |= SPI1_MOSI_MASK | SPI1_CLK_MASK;
  P1DIR &= ~SPI1_MISO_MASK;

  /* Set SPI clock speed assuming 32MHz crystal is installed */
  switch (freq) {
  case SPI_ONE_HUNDRED_HZ: /* 0.85% error */
    U1BAUD = 167;
    U1GCR = 1;
    break;
  case SPI_FOUR_HUNDRED_HZ: /* 0.1% error */
    U1BAUD = 163;
    U1GCR = 3;
    break;
  case SPI_ONE_HUNDRED_KHZ: /* 0.1% error */
    U1BAUD = 154;
    U1GCR = 11;
    break;
  case SPI_FOUR_HUNDRED_KHZ: /* 0.1% error */
    U1BAUD = 154;
    U1GCR = 13;
    break;
  case SPI_ONE_MHZ: /* 0% error */
    U1BAUD = 0;
    U1GCR = 15;
    break;
  case SPI_TWO_MHZ:
    U1BAUD = 0;
    U1GCR = 16; /* 0% error */
    break;
  default:
    /* TODO: not supported */
    break;
  }

  /* configure SPI clock phase and polarity */
  switch (mode) {
  case SPI_MODE0:
    U1GCR &= ~0xC0; /* CPOL = 0, CPHA = 0 */
    break;
  case SPI_MODE1:
    U1GCR &= ~0xC0; /* CPOL = 0, CPHA = 1 */
    U1GCR |= 0x40;
    break;
  case SPI_MODE2:
    U1GCR &= ~0xC0; /* CPOL = 1, CPHA = 0 */
    U1GCR |= 0x80;
    break;
  case SPI_MODE3:
    U1GCR |= 0xC0; /* CPOL = 1, CPHA = 1 */
    break;
  default:
    /* TODO: invalid mode */
    break;
  }

  /* configure endianess */
  if (endianess == SPI_MSB_FIRST)
    U1GCR |= 0x20;
  else
    U1GCR &= ~0x20;

  /* configure architecture chip selects */
  spi_arch_deselect_all();
}

void
spi1_write(unsigned char data)
{
  U1CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
  U1DBUF = data;
  while(!(U1CSR & UCSR_TX_BYTE)); /* Wait until byte has been transmitted. */
}

unsigned char
spi1_read(void)
{
  unsigned char data_read;

  U1CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
  U1DBUF = 0;
  while(!(U1CSR & UCSR_TX_BYTE)); /* Wait until byte has been transmitted. */
  data_read = U1DBUF;

  return data_read;
}

unsigned char
spi1_read_write(unsigned char data)
{
  unsigned char data_read;

  U1CSR &= ~UCSR_TX_BYTE; /* Clear TX_BYTE status */
  U1DBUF = data;
  while(!(U1CSR & UCSR_TX_BYTE)); /* Wait until byte has been transmitted. */
  data_read = U1DBUF;

  return data_read;
}
#endif
