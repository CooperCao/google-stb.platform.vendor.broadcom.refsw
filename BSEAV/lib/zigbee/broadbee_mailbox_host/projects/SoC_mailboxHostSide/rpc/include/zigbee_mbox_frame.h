/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef _ZIGBEE_MBOX_FRAME_H_
#define _ZIGBEE_MBOX_FRAME_H_

#define MBOX_FRAME_SIZE_IN_WORDS 32

#define MBOX_FRAME_HEADER_SIZE_IN_WORDS 1
#define MBOX_FRAME_HEADER_OFFSET 0
#define MBOX_FRAME_PAYLOAD_OFFSET MBOX_FRAME_HEADER_SIZE_IN_WORDS
#define MBOX_FRAME_PAYLOAD_SIZE_IN_WORDS MBOX_FRAME_SIZE_IN_WORDS-MBOX_FRAME_HEADER_SIZE_IN_WORDS

#define MBOX_FRAME_MSG_SUBSYSID_SHIFT 29
#define MBOX_FRAME_MSG_SUBSYSID_MASK 0x7

#define MBOX_FRAME_MSG_FRAGMENT_SHIFT 28
#define MBOX_FRAME_MSG_FRAGMENT_MASK 0x1
#define MBOX_FRAME_GET_MSG_FRAGMENT(p) ((p >> MBOX_FRAME_MSG_FRAGMENT_SHIFT) & MBOX_FRAME_MSG_FRAGMENT_MASK)
#define MBOX_FRAME_SET_MSG_FRAGMENT(p, fragment) \
    { \
        *p &= ~(MBOX_FRAME_MSG_FRAGMENT_MASK << MBOX_FRAME_MSG_FRAGMENT_SHIFT); \
        *p |= ((fragment & MBOX_FRAME_MSG_FRAGMENT_MASK) << MBOX_FRAME_MSG_FRAGMENT_SHIFT); \
    }

#define MBOX_FRAME_MSG_ID_SHIFT 18
#define MBOX_FRAME_MSG_ID_MASK 0x3ff
#define MBOX_FRAME_GET_MSG_ID(p) ((p >> MBOX_FRAME_MSG_ID_SHIFT) & MBOX_FRAME_MSG_ID_MASK)
#define MBOX_FRAME_SET_MSG_ID(p, msg_id) \
    { \
        *p &= ~(MBOX_FRAME_MSG_ID_MASK << MBOX_FRAME_MSG_ID_SHIFT); \
        *p |= ((msg_id & MBOX_FRAME_MSG_ID_MASK) << MBOX_FRAME_MSG_ID_SHIFT); \
    }

#define MBOX_FRAME_MSG_TYPE_SHIFT 16
#define MBOX_FRAME_MSG_TYPE_MASK 0x3

#define MBOX_FRAME_MSG_SEQ_NUM_SHIFT 8
#define MBOX_FRAME_MSG_SEQ_NUM_MASK 0xff
#define MBOX_FRAME_GET_MSG_SEQ_NUM(p) ((p >> MBOX_FRAME_MSG_SEQ_NUM_SHIFT) & MBOX_FRAME_MSG_SEQ_NUM_MASK)
#define MBOX_FRAME_SET_MSG_SEQ_NUM(p, msg_seq_num) \
    { \
        *p &= ~(MBOX_FRAME_MSG_SEQ_NUM_MASK << MBOX_FRAME_MSG_SEQ_NUM_SHIFT); \
        *p |= ((msg_seq_num & MBOX_FRAME_MSG_SEQ_NUM_MASK) << MBOX_FRAME_MSG_SEQ_NUM_SHIFT); \
    }

#define MBOX_FRAME_MSG_PROTOCOL_VERSION_SHIFT 5
#define MBOX_FRAME_MSG_PROTOCOL_VERSION_MASK 0x7

#define MBOX_FRAME_LENGTH_SHIFT 0
#define MBOX_FRAME_LENGTH_MASK 0x1f
#define MBOX_FRAME_GET_LENGTH(p) ((p >> MBOX_FRAME_LENGTH_SHIFT) & MBOX_FRAME_LENGTH_MASK)
#define MBOX_FRAME_SET_LENGTH(p, length) \
    { \
        *p &= ~(MBOX_FRAME_LENGTH_MASK << MBOX_FRAME_LENGTH_SHIFT); \
        *p |= (((length) & MBOX_FRAME_LENGTH_MASK) << MBOX_FRAME_LENGTH_SHIFT); \
    }

/* 10 bit code, used in message header */
typedef enum zigbeeMboxMsgId {
    MBOX_Dummy = 0,
#ifdef TEST
    MBOX_ClientCoreLoopback
#endif
} zigbeeMboxMsgId;

#define MBOX_RESPONSE 0x200

#endif /*_ZIGBEE_MBOX_FRAME_H_*/