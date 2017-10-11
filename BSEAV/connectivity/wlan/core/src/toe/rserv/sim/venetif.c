/*
 * FILE-CSTYLED
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Driver for Virtual Ethernet.
 * Some of this code was derived from lwip's ethernetif.c.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>

#include "netif/etharp.h"

#include "venet.h"
#include "venetc.h"
#include "venetif.h"

#define DPRINTF			if (0) printf

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#define OUTPUT_QSIZE		128

struct venetif {
  venetc_t *vt;

  sys_thread_t i_thread;
  sys_thread_t o_thread;

  /* Structure for buffering output data */
  struct output_state {
    sys_sem_t ready;
    struct {
      unsigned char *data;
      int len;
    } q[OUTPUT_QSIZE];
    volatile int in_ptr;
    volatile int out_ptr;
  } output;

  struct venetif_stats stats;
};

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/*
 * Output data is bufferd up and written by a separate thread in order
 * to prevent the possibility of deadlock.
 */

static void
output_thread(void *arg)
{
  struct netif *netif = arg;
  struct venetif *venetif = netif->state;
  struct output_state *o = &venetif->output;

  for (;;) {
    DPRINTF("output_thread: wait for output\n");

    sys_sem_wait(o->ready);

    /* Send all the packets that are there (more may be added while we're sending) */

    while (o->out_ptr != o->in_ptr) {
      DPRINTF("output_thread: writing %d bytes (in_ptr=%d out_ptr=%d)\n",
              o->q[o->out_ptr].len, o->in_ptr, o->out_ptr);

      venetc_send(venetif->vt, o->q[o->out_ptr].data, o->q[o->out_ptr].len);

      free(o->q[o->out_ptr].data);

      o->q[o->out_ptr].data = NULL;
      o->q[o->out_ptr].len = 0;

      o->out_ptr = (o->out_ptr + 1) % OUTPUT_QSIZE;
    }
  }
}

/* If the whole packet won't fit, doesn't anything and returns -1 */

static int
output_enqueue(struct netif *netif, unsigned char *data, int len)
{
  struct venetif *venetif = netif->state;
  struct output_state *o = &venetif->output;

  DPRINTF("output_enqueue: enqueueing %d bytes (in_ptr=%d out_ptr=%d)\n",
          len, o->in_ptr, o->out_ptr);

  venetif->stats.txframe++;
  venetif->stats.txbyte += len;

  if ((o->in_ptr + 1) % OUTPUT_QSIZE == o->out_ptr) {
    printf("output_enqueue: queue full; dropped packet\n");
    venetif->stats.txdrop++;
    return -1;
  }

  if ((o->q[o->in_ptr].data = malloc(len)) == NULL) {
    printf("output_enqueue: out of memory, len=%d; dropped packet\n", len);
    venetif->stats.txerror++;
    return -1;
  }

  memcpy(o->q[o->in_ptr].data, data, len);

  o->q[o->in_ptr].len = len;

  o->in_ptr = (o->in_ptr + 1) % OUTPUT_QSIZE;

  sys_sem_signal(o->ready);

  return 0;
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  unsigned char *msg;
  int msglen;
  int i, j;

  DPRINTF("venetif: low_level_output p=%p\n", (void *)p);

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

  if (p->tot_len == 0 || (msg = malloc(p->tot_len)) == NULL) {
    fprintf(stderr, "venetif: could not allocate %d bytes\n", p->tot_len);
    exit(1);
  }

  /* Flatten packet */

  msglen = p->tot_len;

  i = 0;

  for (q = p; q != NULL; q = q->next) {
    for (j = 0; j < q->len; j++) {
      msg[i++] = ((unsigned char *)q->payload)[j];
    }
  }

  if (i != msglen)
    abort();

  if (output_enqueue(netif, msg, msglen) < 0) {
    LINK_STATS_INC(link.drop);
    DPRINTF("venetif: dropped packet (%d bytes)\n", msglen);
  } else
    DPRINTF("venetif: sent packet (%d bytes)\n", msglen);

  free(msg);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif
  
  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */

static struct pbuf *
low_level_input(struct netif *netif)
{
  struct venetif *venetif = netif->state;
  struct pbuf *p, *q;
  unsigned char *msg;
  int msglen;
  int i, j;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  if (venetc_recv(venetif->vt, &msg, &msglen) <= 0) {
    fprintf(stderr, "venetif: read error\n");
    exit(1);
  }
  venetif->stats.rxframe++;
  venetif->stats.rxbyte += msglen;

#if ETH_PAD_SIZE
  msglen += ETH_PAD_SIZE;				/* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, (u16_t)msglen, PBUF_POOL);
  
  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */

    i = 0;

    for (q = p; q != NULL; q = q->next) {
      for (j = 0; j < q->len; j++) {
	((unsigned char *)q->payload)[j] = msg[i++];
      }
    }

    LWIP_ASSERT("i == msglen", i == msglen);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    /* Drop packet */
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    venetif->stats.rxerror++;
  }

  venetc_free(venetif->vt, msg);

  return p;  
}

static void
input_thread(void *arg)
{
  struct netif *netif = arg;
  struct venetif *venetif = netif->state;
  struct eth_hdr *ethhdr;

  DPRINTF("input_thread: running\n");

  for (;;) {
    struct pbuf *p;

    DPRINTF("input_thread: waiting for packet\n");

    p = low_level_input(netif);
    if (p == NULL) {
      fprintf(stderr, "venetif: input overrun\n");
      continue;
    }

    /* point to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;
    switch (htons(ethhdr->type)) {
      /* IP packet? */
    case ETHTYPE_IP:
      /* update ARP table */
      etharp_ip_input(netif, p);
      /* skip Ethernet header */
      pbuf_header(p, -(s16_t)sizeof(struct eth_hdr));
      /* pass to network layer */
      netif->input(p, netif);
      break;
      
    case ETHTYPE_ARP:
      /* pass p to ARP module  */
      etharp_arp_input(netif, (struct eth_addr *)venetif->vt->mac, p);
      break;
    default:
      fprintf(stderr, "input_thread: dropping packet of unknown type\n");
      venetif->stats.rxdrop++;
      pbuf_free(p);
      break;
    }
  }
}

static void
low_level_init(struct netif *netif)
{
  struct venetif *venetif = netif->state;
  char *hostname, *s;
  int port;

  if ((hostname = getenv("VENET_HOST")) == NULL) {
    fprintf(stderr, "venetif: venet server hostname not set in VENET_HOST variable\n");
    exit(1);
  }

  if ((s = getenv("VENET_PORT")) == NULL) {
    fprintf(stderr, "venetif: venet server port not set in VENET_PORT variable\n");
    exit(1);
  }

  port = (int)strtol(s, 0, 0);

  if ((venetif->vt = venetc_open(hostname, port)) == NULL) {
    fprintf(stderr, "venetif: error connecting to venet server\n");
    exit(1);
  }

  printf("Connected to switch (local MAC %x:%x:%x:%x:%x:%x)\n",
         venetif->vt->mac[0], venetif->vt->mac[1], venetif->vt->mac[2],
         venetif->vt->mac[3], venetif->vt->mac[4], venetif->vt->mac[5]);

  venetif->i_thread = sys_thread_new(input_thread, netif, DEFAULT_THREAD_PRIO);

  venetif->output.ready = sys_sem_new(0);
  memset(&venetif->output.q, 0, sizeof(venetif->output.q));
  venetif->output.in_ptr = 0;
  venetif->output.out_ptr = 0;

  venetif->o_thread = sys_thread_new(output_thread, netif, DEFAULT_THREAD_PRIO);

  /* set MAC hardware address */
  netif->hwaddr_len = 6;
  memcpy(netif->hwaddr, venetif->vt->mac, 6);

  /* set maximum transfer unit */
  netif->mtu = VENET_MTU;
  
  /* broadcast capability */
  netif->flags = NETIF_FLAG_BROADCAST;
}

static void
low_level_stop(struct netif *netif)
{
  struct venetif *venetif = netif->state;

  venetc_close(venetif->vt);
}

/*
 * venetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t
venetif_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
 /* resolve hardware address, then send (or queue) packet */
  DPRINTF("venetif_output: send to 0x%x\n", (unsigned int)ipaddr->addr);
  return etharp_output(netif, ipaddr, p);
}

static void
arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/*
 * venetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t
venetif_init(struct netif *netif)
{
  struct venetif *venetif;

  venetif = malloc(sizeof(struct venetif));
  
  if (venetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("venetif_init: out of memory\n"));
    return ERR_MEM;
  }

  memset(venetif, 0, sizeof(*venetif));

  netif->state = venetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = venetif_output;
  netif->linkoutput = low_level_output;
  
  low_level_init(netif);

  etharp_init();

  netif_set_up(netif);

  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);

  return ERR_OK;
}

/*
 * venetif_stop()
 */

err_t
venetif_stop(struct netif *netif)
{
  struct venetif *venetif = netif->state;

  sys_thread_kill(venetif->i_thread);
  sys_thread_kill(venetif->o_thread);

  low_level_stop(netif);

  free(venetif);

  netif->state = NULL;

  return ERR_OK;
}

/*
 * venetif_info()
 */

err_t
venetif_info(struct netif *netif, int *mtu, unsigned char mac[6])
{
  struct venetif *venetif = netif->state;

  *mtu = VENET_MTU;

  if (venetif)
    memcpy(mac, venetif->vt->mac, 6);
  else
    memset(mac, 0, 6);

  return ERR_OK;
}

/*
 * venetif_stats_get()
 */

err_t
venetif_stats_get(struct netif *netif, struct venetif_stats *vfs)
{
  if (netif != NULL && netif->state != NULL) {
    struct venetif *venetif = netif->state;
    memcpy(vfs, &venetif->stats, sizeof(*vfs));
  } else
    memset(vfs, 0, sizeof(*vfs));

  return ERR_OK;
}
