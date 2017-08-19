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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the RF4CE Network Layer component utility functions.
 *
*******************************************************************************/

#ifndef BBRF4CENWKUTILITIES_H
#define BBRF4CENWKUTILITIES_H

#include "bbRF4CEConfig.h"
#include "bbSysTypes.h"

#include "bbRF4CENWKStaticData.h"

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Checks queues and relaunches requests.
 ****************************************************************************************/
void RF4CE_NWK_CheckQueues(void);

/************************************************************************************//**
 \brief Generates unique msdu handler.
 ****************************************************************************************/
inline uint8_t rf4ceNwkGetUniqueMsduHandler(void)
{
    return GET_RF4CE_NWK_STATIC_DATA_FIELD()->dataHandle++;
};

/************************************************************************************//**
 \brief Calculates a logical channel next to active one.
 ****************************************************************************************/
inline uint8_t rf4ceNwkGetNextLogicalChannel(void)
{
    uint8_t activeChannel = GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.autoNIB.nwkBaseChannel;
    uint8_t nextChannel = 0;

    switch (activeChannel)
    {
        case RF4CE_NWK_LOGICALCHANNEL_15:
            nextChannel = RF4CE_NWK_LOGICALCHANNEL_20;
            break;
        case RF4CE_NWK_LOGICALCHANNEL_20:
            nextChannel = RF4CE_NWK_LOGICALCHANNEL_25;
            break;
        case RF4CE_NWK_LOGICALCHANNEL_25:
            nextChannel = RF4CE_NWK_LOGICALCHANNEL_15;
            break;
        default:
            SYS_DbgHalt(RF4CE_NWK_INVALID_ACTIVE_CHANNEL);
    }

    return nextChannel;
}



inline uint8_t rf4ceNwkFrameChannelFromBaseChannel(uint8_t baseChannel)
{
    uint8_t result = 0;

    switch(baseChannel)
    {
        case RF4CE_NWK_LOGICALCHANNEL_15:
            result = RF4CE_NWK_FRAME_CHANNEL_15;
            break;
        case RF4CE_NWK_LOGICALCHANNEL_20:
            result = RF4CE_NWK_FRAME_CHANNEL_20;
            break;
        case RF4CE_NWK_LOGICALCHANNEL_25:
            result = RF4CE_NWK_FRAME_CHANNEL_25;
            break;
        default:
            SYS_DbgHalt(RF4CE_NWK_INVALID_BASE_CHANNEL);
            break;
    }

    return result;
}

/************************************************************************************//**
 \brief Returns pointer to the first empty pairing table entry if it exists and
    NULL otherwise.
 ****************************************************************************************/
inline RF4CE_PairingTableEntry_t *rf4ceNwkPairingTableFindEmpty(void)
{
    RF4CE_PairingTableEntry_t *const nwkPairingTable =
        GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable.nwkPairingTable;

    for (uint8_t ix = 0; ix < RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES; ++ix)
        if (!IS_RF4CE_PAIRING_ENTRY_USED(&nwkPairingTable[ix]))
        {
            return &nwkPairingTable[ix];
        }
    return NULL;
}

inline RF4CE_PairingTableEntry_t *rf4ceNwkPairingTableFindByShort(uint16_t panId, uint16_t shortAddr)
{
    RF4CE_PairingTableEntry_t *const nwkPairingTable =
        GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable.nwkPairingTable;

    for (uint8_t ix = 0; ix < RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES; ++ix)
    {
        if (IS_RF4CE_PAIRING_ENTRY_USED(&nwkPairingTable[ix]) &&
            panId == nwkPairingTable[ix].dstPanId &&
            shortAddr == nwkPairingTable[ix].dstNetAddr)
        {
            return &nwkPairingTable[ix];
        }
    }

    return NULL;
}

inline RF4CE_PairingTableEntry_t *rf4ceNwkPairingTableFindByIeee(const uint64_t *const ieeeAddr)
{
    RF4CE_PairingTableEntry_t *const nwkPairingTable =
        GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable.nwkPairingTable;

    for (uint8_t ix = 0; ix < RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES; ++ix)
    {
        if (IS_RF4CE_PAIRING_ENTRY_USED(&nwkPairingTable[ix]) &&
            *ieeeAddr == nwkPairingTable[ix].dstIeeeAddr)
        {
            return &nwkPairingTable[ix];
        }
    }

    return NULL;
}

inline uint8_t rf4ceNwkPairingTableGetIndexByPointer(const RF4CE_PairingTableEntry_t *const entry)
{
    RF4CE_PairingTableEntry_t *const nwkPairingTable =
        GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable.nwkPairingTable;

    const size_t ix = entry - nwkPairingTable;
    return (ix < RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES) ?
        (uint8_t)ix : RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES;
}

#endif // BBRF4CENWKUTILITIES_H

/* eof bbRF4CENWKUtilities.h */
