/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/******************************************************************************
*
* DESCRIPTION:
*       Contains definitions for interface for ZigBee PRO NWK Broadcast Table.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_BROADCAST_TABLE_H
#define _ZBPRO_NWK_BROADCAST_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkConfig.h"

#include "private/bbZbProNwkCommonPrivate.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Broadcast transaction record (BTR) Status Values.
 */
typedef enum _ZbProNwkBtrStatus_t
{
    ZBPRO_NWK_BTR_NEW,      /* New entry has successfully added to table. */
    ZBPRO_NWK_BTR_UPDATED,  /* The received packet is not been in BTR and it is added to the table. */
    ZBPRO_NWK_BTR_DUPLICATED,    /* A packet with given sequence number has already been received. */
    ZBPRO_NWK_BTR_ABSENT,   /* Table have no space to add new entry. */
} ZbProNwkBtrStatus_t;

/**//**
 * \brief BTT time-to-leave type.
 */
typedef uint16_t ZbProNwkBttTtl_t;

/**//**
 * \brief Type of mask to track duplicates.
 */
typedef uint32_t ZbProNwkDuplicateMask_t;

/**//**
 * \brief Broadcast transaction record (BTR).
 */
typedef struct _ZbProNwkBtr_t
{
    /* The bit mask indicates received packets from particular node.
     * Most significant bit of this mask is responsible for packed with sequence number: seqNumber.
     * So it is possible to store information about last 32 sequence numbers,
     * when uint32_t type is used for ZbProNwkDuplicateMask_t. */
    ZbProNwkDuplicateMask_t mask;
    /* Short address of node from which duplicates are tracked. */
    ZBPRO_NWK_NwkAddr_t address;
    /* Current value of time-to-leave. */
    ZbProNwkBttTtl_t ttl;
    /* Most recent sequence number which is received from the node. */
    uint8_t seqNumber;
} ZbProNwkBtr_t;

/**//**
 * \brief Broadcast transaction table (BTT).
 */
typedef struct _ZbProNwkBTT_t
{
    /* A pointer to the table. */
    ZbProNwkBtr_t     table[ZBPRO_NWK_BTT_SIZE];
    /* Last stamp of System Time. */
    uint32_t          lastStamp;
    /* Variable to track timeout and to age out an entry. */
    uint16_t          agingPeriod;
    /* Variable to track timeout and to age out an entry. */
    ZbProNwkBttTtl_t  maxTTL;
} ZbProNwkBTT_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Prepare Broadcast Transaction Table (BTT) to real work.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBttReset(void);

/************************************************************************************//**
  \brief Search for BTR record in BTT and add if it is not found.

  \param[in] address, seqNumber - record to search for or to add if not found.
  \return ZBPRO_NWK_BTR_DUPLICATED - when duplicate packet is received.
          ZBPRO_NWK_BTR_UPDATED - new packet is received and information about the packet is added to the table.
****************************************************************************************/
NWK_PRIVATE ZbProNwkBtrStatus_t zbProNwkBttCheckAndAdd(const ZBPRO_NWK_NwkAddr_t address, const uint8_t seqNumber);

/************************************************************************************//**
  \brief Clear info about given transaction from the rejection table.

  \param[in] address, seqNumber - BTR record to clear.
  \return None.
****************************************************************************************/
NWK_PRIVATE void zbProNwkBttClear(const ZBPRO_NWK_NwkAddr_t address, const uint8_t seqNumber);

#endif /* _ZBPRO_NWK_BROADCAST_TABLE_H */

/* eof bbZbProNwkBroadcastTable.h */
