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
 *      RF4CE NWK Discovery Pair private functions.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_DISCOVERY_PAIR_H
#define _RF4CE_NWK_DISCOVERY_PAIR_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysBasics.h"
#include "bbSysPayload.h"

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Calculates the amount of bytes necessary for a variable message part.

 \param[in] appCaps - Application Capabilities.
 \return The calculated size.
 ****************************************************************************************/
uint32_t RF4CE_NWK_GetVarPartSize(uint8_t appCaps);

/************************************************************************************//**
 \brief Copys Variable Massage Part from source payload to destination payload.

 \param[in] appCaps - Application Capabilities.
 \param[in] srcOffset - Source Offset.
 \param[in] src - pointer to the Source payload.
 \param[in] dst - pointer to the Destination payload.
 \param[in] dstOffset - pointer to Destination Offset.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_CopyVarDataFromBlockIdToBlockId(uint8_t appCaps, uint32_t srcOffset, SYS_DataPointer_t *src, SYS_DataPointer_t *dst, uint32_t *dstOffset);

/************************************************************************************//**
 \brief Copys Variable Massage Part from source Block ID to destination conventional memory.

 \param[in] appCaps - Application Capabilities.
 \param[in] dst - Destination Buffer.
 \param[in] src - pointer to the Source payload.
 \param[in] dstOffset - pointer to Destination Offset.
 \return The offset of copied profile list.
 ****************************************************************************************/
uint32_t RF4CE_NWK_CopyVarDataFromBlockIdToMemory(uint8_t appCaps, void *dst, SYS_DataPointer_t *src, uint32_t *dstOffset);

/************************************************************************************//**
 \brief Checks if at least one entry from List A is equal to an entry of List B.

 \param[in] listA - pointer to the List A.
 \param[in] nListA - number of elements in the List A.
 \param[in] listB - pointer to the List B.
 \param[in] nListB - number of elements in the List B.
 \return true if at least one entry from List A is equal to an entry of List B.
 ****************************************************************************************/
bool RF4CE_NWK_CompareLists(uint8_t *listA, uint32_t nListA, uint8_t *listB, uint32_t nListB);

/************************************************************************************//**
 \brief Cancels current Auto-Discovery request.
 ****************************************************************************************/
void rf4cenwkCancelAutoDiscovery(void);

#endif /* _RF4CE_NWK_DISCOVERY_PAIR_H */

/* eof bbRF4CENWKDiscoveryPair.h */