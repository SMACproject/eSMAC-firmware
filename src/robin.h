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
 * $Id: pkt.h,v 1.5 2004/09/25 06:05:30 bsd Exp $
 */

#ifndef __pkt_h__
#define __pkt_h__

#include <stdint.h>

/* states for the packet reception state machine */
enum {
  PKT_STATE_START,
  PKT_STATE_LEADIN,
  PKT_STATE_ADDRESS,
  PKT_STATE_SRC,
  PKT_STATE_FLAGS,
  PKT_STATE_LEN,
  PKT_STATE_DATA,
  PKT_STATE_CKSUM
};

typedef struct rstate {
  uint8_t * ibuf;  /* pointer to active receive buffer */
  uint8_t * sbuf;  /* pointer to transmit buffer */
  uint8_t * rbuf;  /* pointer to the buffer holding the last rx'd packet */
  volatile uint8_t ridx;    /* index into rx buf */
  volatile uint8_t sidx;    /* index into tx buf */
  volatile uint8_t slen;    /* length ot the packet being transmitted */
  volatile uint8_t state;   /* packet rx state */
  volatile uint8_t cksum;   /* packet checksum */
  uint8_t myid;             /* our network address */
  volatile uint8_t tx_complete ; /* transmit complete flag */
  volatile uint8_t rx          ; /* packet received flag */
  volatile uint8_t err         ; /* packet error flag */
  volatile uint8_t promiscuous ; /* promiscuous mode */
  void (*tx_ena)(void);
  void (*tx_dis)(void);
  void (*rx_ena)(void);
  void (*rx_dis)(void);
  void (*tx_start)(uint8_t c);
} RSTATE;


#define PKT_OVERHEAD  5 /* 5 bytes of overhead for each packet */
#define PKT_MAX_PLEN 64 /* max packet length, give a data length of 59 */
#define PKT_MAX_DLEN (PKT_MAX_PLEN - PKT_OVERHEAD) /* max packet data length */

#define PKT_N_LEADIN    4     /* no. bytes of leadin */
#define PKT_LEADIN_BYTE 0x99
#define PKT_START_BYTE  0xaa



extern RSTATE * i1r; /* interface 1 ROBIN state */



/* packet flags */
#define PKTF_ACK        0x01
#define PKTF_NACK       0x02
#define PKTF_REQACK     0x04
#define PKTF_REQID      0x08
#define PKTF_COMMAND    0x10
#define PKTF_RESERVED   0x20
#define PKTF_USER1      0x40
#define PKTF_USER2      0x80

/* accessor macros to get at packet fields */
#define PKT_DST(x)   ((x)[0])
#define PKT_SRC(x)   ((x)[1])
#define PKT_FLAGS(x) ((x)[2])
#define PKT_ACK(x)   (PKT_FLAGS(x) & PKTF_ACK)
#define PKT_DLEN(x)  ((x)[3])
#define PKT_DATA(x)  (&((x)[4]))
#define PKT_LEN(x)   (PKT_DLEN(x) + 5)
#define PKT_CKSUM(x) ((x)[PKT_LEN(x)-1])
#define PKT_COMMAND(x) (PKT_FLAGS(x) & PKTF_COMMAND)

void pkt_init(RSTATE * r);

int8_t pkt_send(RSTATE * r, uint8_t collision_detect, uint8_t to, uint8_t from,
                uint8_t flags, uint8_t * buf, uint8_t len);

uint8_t pkt_compute_cksum(uint8_t * p);

int8_t pkt_wait(RSTATE * r, const uint16_t timeout);

int8_t pkt_wait_ack(RSTATE * r, uint16_t timeout);

int8_t pkt_send_wait_ack(RSTATE * r, uint8_t collision_detect, uint8_t to, uint8_t from,
                         uint8_t flags, uint8_t * buf, uint8_t len, 
                         uint8_t tries, uint16_t timeout, uint8_t clear);

int8_t pkt_ack(RSTATE * r, uint8_t * p);

#endif

