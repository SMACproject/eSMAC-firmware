/*
 * radio.c
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
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "cc253x.h"
#include "uart.h"
#include "radio.h"
#include "jsmn.h"
#include "json.h"

#if defined __IAR_SYSTEMS_ICC__
  volatile unsigned char *macp = &X_IEEE_ADDR;
#else
  __xdata unsigned char *macp = &X_IEEE_ADDR;
#endif

char rf_rx_buf[128];
static uint8_t rf_channel;
uint8_t destination0;
uint8_t destination1;

void rf_init(uint8_t channel)
{
  uint8_t ext_addr[8];
  int8_t i;
  
  /*
   * Read IEEE address from flash, store in ext_addr.
   * Invert endianness (from little to big endian)
   */
  for(i = 7; i >= 0; --i) {
    ext_addr[i] = *macp;
    macp++;
  }
  
  RXCTRL = 0x3F;
  FSCTRL = 0x55;

  TXPOWER   = 0xD5;
  CCACTRL0  = 0xF8;

  //FRMFILT0  = 0x00; // uncomment to disable frame filtering (for sniffer)

  TXFILTCFG = 0x09;
  AGCCTRL1 =  0x15;
  FSCAL1 =    0x00;

  /* Auto ACKs and CRC calculation, default RX and TX modes with FIFOs */
  FRMCTRL0 = 0x40;
  FRMCTRL0 |= 0x20;

  /* Disable source address matching and autopend */
  //SRCMATCH = 0;
  
  PAN_ID0 = 0x49; // TI PANID
  PAN_ID1 = 0x54;
  SHORT_ADDR0 = ext_addr[7]; // short address
  SHORT_ADDR1 = ext_addr[6];
  destination0 = 255;
  destination1 = 255;

  rf_set_channel(channel);

  RFIRQM0 |= (1<<6);
  IEN2 |= (1<<0);

  RFST = 0xED; // ISFLUSHRX
  RFST = 0xE3; // ISRXON
}

uint8_t rf_set_channel(uint8_t channel)
{
  RFST = 0xEF; // ISRFOFF
  RFST = 0xED; // ISFLUSHRX

  if (channel >= 11 && channel <= 26) {
    FREQCTRL = 11 + ((channel-11) * 5);
    rf_channel = channel;
  }

  RFST = 0xED; // ISFLUSHRX
  RFST = 0xE3; // ISRXON

  if (channel >= 11 && channel <= 26)
    return channel;
  else
    return 0;
}

uint8_t rf_get_channel(void)
{
  return rf_channel;
}

uint8_t rf_get_short_addr1(void)
{
  return SHORT_ADDR1;
}

uint8_t rf_get_short_addr0(void)
{
  return SHORT_ADDR0;
}

void rf_receive_isr(void)
{
  int rf_rx_len = 0;
  int rssi = 0;
  int i;
  char crc_ok = 0;

  /* a workaround to prevent new radio message to override RX buffer
   * while jsmn is parsing it
   */
  if (json_parser_is_busy()) return;

  rf_rx_len = RFD - 13;
  rf_rx_len &= 0x7F;
  for (i = 0; i < 11; i++)
  {
    rf_rx_buf[i] = RFD;
  }
  for (i = 0; i < rf_rx_len; i++)
  {
    rf_rx_buf[i] = RFD;
  }

  rssi = RFD - 73; // RSSI
  crc_ok = RFD; // Check for CRC bit 7

  RFST = 0xED;
  if( crc_ok & 0x80 )
  {
    uart0_send( rf_rx_buf, rf_rx_len); /* print it to console */
    
#if CONFIG_MODULE_MASTER
#else
    /* parse with jsmn and execute */
    json_string_received(rf_rx_buf, sizeof(rf_rx_buf)/sizeof(rf_rx_buf[0]));
#endif

  }
  else
  {
    printf("\nCRC Error\n");
  }
}

void rf_send( char *pbuf , int len)
{
  int i;

  RFST = 0xE3; // ISRXON
  while( FSMSTAT1 & (( 1<<1 ) | ( 1<<5 )));

  RFIRQM0 &= ~(1<<6);
  IEN2 &= ~(1<<0);

  RFST = 0xEE; // ISFLUSHTX
  RFIRQF1 = ~(1<<1);

  // 802.15.4 data frame format (11 bytes header + 2 FCS)
  RFD = len + 11 + 2;
  RFD = 0x01; //FT_DATA
  RFD = 0x88; // DEST_ADDR_16 | SRC_ADDR_16
  RFD = 100; // FSN any number for now
  // destination PANID and ADDR
  RFD = PAN_ID0;
  RFD = PAN_ID1;
  RFD = destination0;
  RFD = destination1;
  // source PANID and ADDR
  RFD = PAN_ID0;
  RFD = PAN_ID1;
  RFD = SHORT_ADDR0;
  RFD = SHORT_ADDR1;

  for (i = 0; i < len; i++)
  {
    RFD = *pbuf++;
  }

  RFST = 0xE9; // ISTXON
  while (!(RFIRQF1 & (1<<1)));
  RFIRQF1 = ~(1<<1);

  RFIRQM0 |= (1<<6);
  IEN2 |= (1<<0);
}

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
  RFIRQF0 = 0; //???????
  EA = 1;
}
