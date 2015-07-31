/*
 * Copyright (c) 2004  Brian S. Dean <bsd@bdmicro.com>
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY BRIAN S. DEAN ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BRIAN S. DEAN BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 */

/*
 * $Id: pkt.c,v 1.7 2007/08/02 01:15:10 bsd Exp $
 *
 */

/*
 * This module implements the ROBIN network protocol.
 */

//#include <avr/io.h>
//#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cc253x.h"
#include "sfr-bits.h"
#include "clock.h"
#include "led.h"
#include "robin.h"

static uint8_t pkt_buf[PKT_MAX_PLEN];
static uint8_t pkt_ibuf[PKT_MAX_PLEN];
static uint8_t pkt_send_buf[PKT_MAX_PLEN + PKT_N_LEADIN];

//extern volatile uint8_t uart1_rx;
extern volatile uint16_t ms_count;

RSTATE * i1r = NULL; /* ROBIN state for first ROBIN stack */

void pkt_init(RSTATE * r)
{
  r->ridx        = 0;
  r->sidx        = 0;
  r->slen        = 0;
  r->myid        = 0;
  r->tx_complete = 0;
  r->rx          = 0;
  r->err         = 0;
  r->promiscuous = 0;
  r->ibuf        = pkt_ibuf;
  r->rbuf        = pkt_buf;
  r->sbuf        = pkt_send_buf;
  r->state       = PKT_STATE_START;
  return;
}

#if !PKT_TRACE
uint32_t new_baud = 0;
void pkt_recvd(RSTATE * r)
{
  memcpy(r->rbuf, r->ibuf, PKT_LEN(r->ibuf));
  r->rx = 1;
  r->state = PKT_STATE_START;
}

void pkt_abort(RSTATE * r)
{
  r->state = PKT_STATE_START;
}
#endif

uint8_t pkt_compute_cksum(uint8_t * p)
{
  uint8_t ck, plen, i;

  plen = PKT_LEN(p) - 1;
  ck = p[0];
  for (i=1; i<plen; i++)
    ck += p[i];

  return ck;
}


/*
 * transmit complete interrupt handler - load the next byte into the
 * send buffer, or flag the end of transmission.  Explicitly disable
 * the TXC interrupt flag bit.  If we are called as an interrupt
 * handler, this is not necessary, but sometimes we are called by the
 * SIG_UART1_RECV handler - it is higher priority and gets called
 * before this one, and it seems that the TXC bit is somehow getting
 * cleared in this process, inhibiting this handler, and thus calls us
 * directly when necessary.
 */
/*SIGNAL(SIG_UART_TRANS)
{
  if (i1r) {
    if (i1r->sidx == i1r->slen) {
      i1r->tx_complete = 1;
      return;
    }

    UDR = i1r->sbuf[i1r->sidx++];
  }
  UCSRA |= _BV(TXC);
}*/

void pkt_byte_rx(RSTATE * r, uint8_t c)
{
  static uint8_t to;
  static uint8_t dlen;
  
  switch (r->state) {
    case PKT_STATE_START:  /* start state - 1 or more start bytes */
      if (c == PKT_START_BYTE)
        r->state = PKT_STATE_LEADIN;
      break;
    case PKT_STATE_LEADIN:  /* start state - 1 leadin byte */
      if (c == PKT_LEADIN_BYTE)
        r->state = PKT_STATE_ADDRESS; /* next byte is the address */
      else if (c != PKT_START_BYTE)
        r->state = PKT_STATE_START;
      break;
    case PKT_STATE_ADDRESS:  /* expecting an address packet */
      led_set(LED1);
      to = c;
      if ((to == r->myid) || r->promiscuous) {
        r->ridx = 0;
        r->err = 0;
        r->rx  = 0;
        r->ibuf[r->ridx++] = c; /* destination address */
        r->cksum = c;
      }
      r->state = PKT_STATE_SRC;
      break;
    case PKT_STATE_SRC: /* expecting sender address */
      if ((to == r->myid) || r->promiscuous) {
        r->ibuf[r->ridx++] = c; /* src address */
        r->cksum += c;
      }
      r->state = PKT_STATE_FLAGS;
      break;
    case PKT_STATE_FLAGS: /* expecting packet flags */
      if ((to == r->myid) || r->promiscuous) {
        r->ibuf[r->ridx++] = c; /* flags */
        r->cksum += c;
      }
      r->state = PKT_STATE_LEN;
      break;
    case PKT_STATE_LEN: /* expecting data length */
      r->ibuf[r->ridx++] = c; /* data length */
      r->cksum += c;
      dlen = c;
      if (dlen == 0) { /* zero length packet, skip data state */
        r->state = PKT_STATE_CKSUM;
      }
      else { /* packet has 'dlen' bytes of data to follow + a cksum */
        if (dlen + 4 > PKT_MAX_PLEN) {
          pkt_abort(r); /* packet too long for use */
          break;
        }
        r->state = PKT_STATE_DATA;
      }
      break;
    case PKT_STATE_DATA: /* expecting 'dlen' data bytes */
      r->ibuf[r->ridx++] = c; /* data byte */
      r->cksum += c;
      if (--dlen == 0) { /* that was the last of the data */
        r->state = PKT_STATE_CKSUM;
      }
      break;
    case PKT_STATE_CKSUM: /* expecting a checksum */
      r->ibuf[r->ridx] = c; /* checksum byte */
      if ((to == r->myid) || r->promiscuous) {
        if (r->cksum != r->ibuf[r->ridx]) /* compare checksums */
          r->err = 1;
        pkt_recvd(r);
      }
      led_set(0);
      r->state = PKT_STATE_START;  /* start over waiting for a new packet */
      break;
    default: /* unknown state - state machine error */
      led_set(0);
      pkt_abort(r);
      break;
  }
}


/*
 * UART1 byte received interrupt handler - fetch the character, check
 * for framing error, figure out what to do with the data byte
 * depending on the current state of packet reception.  If we get a
 * complete packet, wake up the waiting thread.
 */
/*SIGNAL(SIG_UART_RECV)
{
  uint8_t c, statusa;

  statusa = UCSRA;
  c  = UDR;

#if PKT_TRACE
  ringbuf8_put(&uart1buf, c);
  uart1_rx = 1;
#else
  if (i1r) {
    pkt_byte_rx(i1r, c);
  }
#endif  

  if (statusa & _BV(TXC))
    SIG_UART_TRANS();
}*/


/*
 * pkt_tx() - transmit the packet contained in the 'sbuf' field of
 * 'r'.  Don't return until the whole packet has been shifted out the
 * UART.
 */
void pkt_tx(RSTATE * r, uint8_t monitor)
{
  /* enable transmitter and possibly the receiver */
  r->tx_ena();
  /* allow a moment for the transmitter to enable */
  __asm_begin
    ASM(nop)
  __asm_end;

  if (monitor) {
    r->promiscuous = 1;
  }
  else {
    r->promiscuous = 0;
    r->rx_dis();
  }

  r->tx_complete  = 0;
  r->sidx = 1;

  led_set(LED1);
  /* start transmission by sending the first byte */
  r->tx_start(r->sbuf[0]);

  /* the rest of the transmission is interrupt driven */
  while (!r->tx_complete)
    ;

  led_set(0);
  
  /* disable transmitter, enable receiver */
  r->tx_dis();
  r->rx_ena();
}


/*
 * randno - return a random number between 'lo' and 'hi'-1
 */
uint16_t randno(uint16_t lo, uint16_t hi)
{
  return lo + rand() % (hi - lo);
}


/*
 * pkt_send() - send a packet to the specified recipient using the
 * specified from address and flags byte.  'buf' points to the packet
 * data payload and 'len' contains the data length.
 */
int8_t pkt_send(RSTATE * r, uint8_t collision_detect, uint8_t to,
                uint8_t from, uint8_t flags, uint8_t * buf, uint8_t len)
{
  uint8_t ck, i, leadin, plen;
  uint8_t resend, tries=5, resends=0;
  uint16_t doze;


  if (len + 5 > PKT_MAX_PLEN)
    return -1; /* error: data is too long for a packet */

  i = 0;
  r->sbuf[i++] = PKT_START_BYTE;
  r->sbuf[i++] = PKT_START_BYTE;
  r->sbuf[i++] = PKT_START_BYTE;
  r->sbuf[i++] = PKT_LEADIN_BYTE;
  leadin = i;
  r->sbuf[i++] = to;
  ck = to;  /* begin checksum calculation */

  r->sbuf[i++] = from;  /* sender address */
  ck += from;
    
  r->sbuf[i++] = flags;  /* packet control flags */
  ck += flags;

  r->sbuf[i++] = len;  /* data len */
  ck += len;

  if (len) {
    /* send data */
    while (len) {
      r->sbuf[i++] = *buf;
      ck += *buf++;
      len--;
    }
  }

  r->sbuf[i++] = ck; /* send checksum */
  
  plen = i;  /* length of the whole packet */

  r->slen = plen;

retry:
  if (collision_detect)
    r->rx = 0;
  
  pkt_tx(r, collision_detect);

  if (collision_detect) {
    r->promiscuous = 0;

    ms_count = 0;
    while (r->rx != 1 && ms_count < 5)
      ;

    resend = 0;
    if (r->rx == 0) {
      /* no packet recv'd within 5 ms, possible collision */
      resend = 1;
    }
    else {
      /* packet recv'd, but check to make sure it matches what we tx'd */
      if (memcmp(r->rbuf, &r->sbuf[leadin], PKT_LEN(&r->sbuf[leadin])) != 0) {
        /* pkt received, but it doesn't match - toss it and try again */
        //printf("no match\n");
        //hexdump_buf(0, pkt_buf, PKT_LEN(pkt_buf));
        //printf("does not match:\n");
        //hexdump_buf(0, &pkt_send_buf[leadin], PKT_LEN(&pkt_send_buf[leadin]));
        resend = 2;
      }
    }
    
    if (resend) {
      /* sleep a random interval and resend it */
      doze = randno(10, 40);
      //printf("collision, resend=%d, resend in %u\n", resend, doze);
      printf("*");
      //ms_sleep(doze);
      clock_delay_usec(doze*1000);
      resends++;
      if (tries--)
        goto retry;
      return -resend;
    }

    /* packet matched, toss our copy of it */
    r->rx = 0;
  }

  return resends;
}


/*
 * pkt_wait() - wait for a packet to arrive, or timeout after
 * 'timeout' milliseconds.  If a packet arrives, return 0.  If we
 * timeout instead, return -1.
 */
int8_t pkt_wait(RSTATE * r, const uint16_t timeout)
{
  ms_count = 0;
  while (r->rx == 0)
    if (ms_count >= timeout+1)
      break;
  if (r->rx == 0)
    return -1;

  return 0;
}


/*
 * pkt_wack() - wait for an ACK packet to arrive or timeout after
 * 'timeout' ms.  If we receive an ACK packet, return 0.  If we
 * timeout, return -1.  If we receive a packet, but the ACK or ACK bit
 * was not set, return -2.
 */
int8_t pkt_wait_ack(RSTATE * r, const uint16_t timeout)
{
  int8_t rc;

  rc = pkt_wait(r, timeout);
  if (rc)
    return -1;

  if (PKT_ACK(r->rbuf))
    return 0;

  return -2;
}


/*
 * pkt_send_wack() - send a packet as with 'pkt_send()', and wait for
 * an ACK response, as with 'pkt_wack()'.  If the 'clear' flag is
 * non-zero, and it looks like we received a good ACK packet in
 * response, then reset the 'pkt_rx' flag to indicate that no more
 * packets are waiting, effectively throwing the ACK packet away since
 * it has served its purpose by this point.
 */
int8_t pkt_send_wait_ack(RSTATE * r, uint8_t collision_detect, uint8_t to,
                         uint8_t from, uint8_t flags, uint8_t * buf,
                         uint8_t len, uint8_t tries, uint16_t timeout,
                         uint8_t clear)
{
  int8_t rc=0;
  uint8_t i;

  for (i=0; i<tries; i++) {
    r->rx = 0;
    rc = pkt_send(r, collision_detect, to, from, flags, buf, len);
    if (rc < 0)
      return -1;

    rc = pkt_wait_ack(r, timeout);
    if (rc == 0) {
      if (clear)
        r->rx = 0;
      return 0;
    }
  }

  return -1;
}

/*
 * pkt_ack() - inline function to send an empty ACK response to the
 * sender of the packet pointed to by 'p'.
 */
int8_t pkt_ack(RSTATE * r, uint8_t * p)
{
  return pkt_send(r, 0, PKT_SRC(p), r->myid, PKTF_ACK, NULL, 0);
}
