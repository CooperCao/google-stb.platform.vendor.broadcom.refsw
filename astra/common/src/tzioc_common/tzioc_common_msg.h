/***************************************************************************
 * Copyright (c)2016 Broadcom. All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2 (GPLv2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 ***************************************************************************/

#ifndef TZIOC_COMMON_MSG_H
#define TZIOC_COMMON_MSG_H
#include "tzioc_common_ring.h"

#ifndef TZIOC_MSG_DEFINES
#define TZIOC_MSG_DEFINES

typedef struct tzioc_msg_hdr {
    uint8_t  ucType;                    /* msg type */
    uint8_t  ucOrig;                    /* originating client */
    uint8_t  ucDest;                    /* destination client */
    uint8_t  ucSeq;                     /* msg sequence number */
    uint32_t ulLen;                     /* byte length of msg payload */
    uint8_t  aucPayload[];              /* start of payload */
} tzioc_msg_hdr;

#define TZIOC_MSG_PAYLOAD(pHdr)         ((uint8_t *)pHdr + sizeof(tzioc_msg_hdr))

#define TZIOC_MSG_SIZE_MAX              1024
#define TZIOC_MSG_PAYLOAD_MAX           (TZIOC_MSG_SIZE_MAX - \
                                         sizeof(struct tzioc_msg_hdr))
#endif /* TZIOC_MSG_DEFINES */

#define TZIOC_MSG_START_CODE            0x47534D49 /* "IMSG" in ASCII */

/* msg control block */
struct tzioc_msg_cb {
    struct tzioc_ring_buf *pSndRing;    /* send msg ring */
    struct tzioc_ring_buf *pRcvRing;    /* receive msg ring */
};

#ifdef __cplusplus
extern "C" {
#endif

int __tzioc_msg_send(
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload);

int __tzioc_msg_receive(
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload,
    uint32_t ulSize);

#ifdef __cplusplus
}
#endif

/* To be initialized by the specific OS */
extern struct tzioc_msg_cb *pTziocMsgCB;

#endif /* TZIOC_COMMON_MSG_H */
