/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/***************************************************************************
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * file : Parser.h
 *
 ***************************************************************************/

#ifndef _TRACELOG_ANALYSER_H_
#define _TRACELOG_ANALYSER_H_

#define TRACELOG_128b_TYPEIO_SHIFT         11
#define TRACELOG_128b_TYPEIO_MASK          0x00000800

#define TRACELOG_128b_TYPEUBUS_SHIFT       12
#define TRACELOG_128b_TYPEUBUS_MASK        0x00001000

#define TRACELOG_128b_TYPEWR_SHIFT         9
#define TRACELOG_128b_TYPEWR_MASK          0x00000200

#define TRACELOG_128b_CLIENTID_SHIFT       0
#define TRACELOG_128b_CLIENTID_MASK        0x000000ff

#define TRACELOG_128b_TYPEAK_SHIFT         10
#define TRACELOG_128b_TYPEAK_MASK          0x00000400

#define TRACELOG_128b_LOSTEVENTS_SHIFT     13
#define TRACELOG_128b_LOSTEVENTS_MASK      0x00002000

#define TRACELOG_128b_POSTTRIGGER_SHIFT    14
#define TRACELOG_128b_POSTTRIGGER_MASK     0x00004000

#define TRACELOG_128b_VALID_SHIFT          15
#define TRACELOG_128b_VALID_MASK           0x00008000

#define TRACELOG_128b_TS_UPPER_SHIFT       16
#define TRACELOG_128b_TS_UPPER_MASK        0xffff0000

#define TRACELOG_128b_TS_LOWER_SHIFT       0
#define TRACELOG_128b_TS_LOWER_MASK        0xffffffff

#define TRACELOG_128b_ADDR_SHIFT           0
#define TRACELOG_128b_ADDR_MASK            0xffffffff

#define TRACELOG_128b_DATAWORD_SHIFT       0
#define TRACELOG_128b_DATAWORD_MASK        0xffffffff

#define TRACELOG_128b_ADDR_LOWER_SHIFT     11
#define TRACELOG_128b_ADDR_LOWER_MASK      0xff000000

#define TRACELOG_128b_ADDR_UPPER_SHIFT     24
#define TRACELOG_128b_ADDR_UPPER_MASK      0x00000800

#define TRACELOG_128b_BURST_SIZE_SHIFT     8
#define TRACELOG_128b_BURST_SIZE_MASK      0x0000ff00

#define TRACELOG_128b_ACC_ID_SHIFT         16
#define TRACELOG_128b_ACC_ID_MASK          0x00ff0000

#define TRACELOG_128b_COMMAND_SHIFT        0
#define TRACELOG_128b_COMMAND_MASK         0x000000ff

#define EXTRACT_BITS(WORD, MASK, SHIFT) ((WORD & MASK) >> SHIFT)

enum BusType {
    eGISB = 1,
    eSCB,
    eUBUS,
};

struct Raw128b {
    unsigned int clientId            :8;
    unsigned int reserved            :1;
    unsigned int typeWrite           :1;
    unsigned int typeAck             :1;
    unsigned int typeIO              :1;
    unsigned int typeUBUS            :1;
    unsigned int lostEvents          :1;
    unsigned int postTrigger         :1;
    unsigned int valid               :1;
    unsigned int timeStampUpper      :16;
    unsigned int timeStampLower      :32;
    unsigned int addressLower        :32;
    unsigned int dataWord            :32;
    unsigned int command             :8;
    unsigned int burstSize           :8;
    unsigned int accessId            :8;
    unsigned int addressUpper        :8;
};

struct GISBBus {
    unsigned int clientId            :8;
    unsigned int typeWrite           :1;
    unsigned int typeAck             :1;
    unsigned int typeIO              :1;
    unsigned int typeUBUS            :1;
    unsigned int lostEvents          :1;
    unsigned int postTrigger         :1;
    unsigned int valid               :1;
    unsigned int timeStampUpper      :16;
    unsigned int timeStampLower      :32;
    unsigned int dataWord            :32;
    unsigned int addressLower        :32;
    unsigned int addressUpper        :8;
};

struct SCBBus {
    unsigned int clientId            :8;
    unsigned int reserved            :1;
    unsigned int typeWrite           :1;
    unsigned int typeAck             :1;
    unsigned int typeIO              :1;
    unsigned int typeUBUS            :1;
    unsigned int lostEvents          :1;
    unsigned int postTrigger         :1;
    unsigned int valid               :1;
    unsigned int timeStampUpper      :16;
    unsigned int timeStampLower      :32;
    unsigned int command             :8;
    unsigned int burstSize           :8;
    unsigned int accessId            :8;
    unsigned int addressLower        :32;
    unsigned int addressUpper        :8;
};
#endif /* _TRACELOG_ANALYSER_H_ */
