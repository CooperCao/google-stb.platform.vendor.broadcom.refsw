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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkSecurity.h $
*
* DESCRIPTION:
*   Network security definitions and declaration.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
*****************************************************************************************/


#ifndef _BB_ZB_PRO_NWK_SECURITY_H
#define _BB_ZB_PRO_NWK_SECURITY_H

/************************* INCLUDES *****************************************************/
#include "private/bbZbProNwkCommonPrivate.h"
#include "private/bbZbProNwkBufferManager.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Possible types of the Network key. See ZigBee Spec r20, Table 4.2.
 */
#define ZBPRO_NWK_STANDARD_SECURITY_KEY     0x01
#define ZBPRO_NWK_HIGH_SECURITY_KEY         0x05

/**//**
 * \brief Type of the Network Security Material descriptor. See ZigBee Spec r20, Table 4.2.
 */
typedef struct _ZbProNwkSecurityMaterialDescr_t
{
    ZbProSspKey_t               key;
    ZbProSspFrameCnt_t          incomingFrameCounterSet[ZBPRO_NWK_NEIGHBOR_TABLE_SIZE];
    ZbProSspFrameCnt_t          outgoingFrameCounter;
    ZbProSspNwkKeySeqNum_t      keySeqNumber;
    union
    {
        uint8_t     plain;
        struct {
            uint8_t  keyType  : 3;
            uint8_t  busy     : 1;
            uint8_t  reserved : 4;
        };
    };
} ZbProNwkSecurityMaterialDescr_t;

/**//**
 * \brief Network Security Information Base.
 */
typedef struct _ZbProNwkSecurityMaterialStorage_t
{
    ZbProNwkSecurityMaterialDescr_t storage[ZBPRO_NWK_SECURITY_KEYS_AMOUNT];
} ZbProNwkSecurityMaterialStorage_t;

/*********************** FUNCTION PROTOTYPES ******************************************/
/*************************************************************************************//**
  \brief Resets NWK-NIB(SecurityMaterialStorage) attributes to their default values.
*****************************************************************************************/
void zbProNwkNibApiSecurityMaterialStorageReset(void);

/*************************************************************************************//**
  \brief Resets counters from NWK-NIB(SecurityMaterialStorage) attributes to zero values.
  \note This function needs when self-leaving procedure is processing.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkNibApiSecurityMaterialStorageResetCounters(void);

/************************************************************************************//**
    \brief Remove the specified Incoming frame counter.
    \param[in] extSrcAddr - extended address of device of interest.
****************************************************************************************/
void zbProNwkSecurityRemoveRxFrameCounter(const uint8_t neighborIndex);

/***********************************************************************************//**
    \brief Performs an encryption of the payload.
    \param[in] buf - pointer to the output buffer structure.
    \param[in] callback - callback function which is called when encryption is finished.
 **************************************************************************************/
NWK_PRIVATE void zbProNwkEncryptOutputPacket(ZbProNwkOutputBuffer_t *const buf, ZbProSspEncryptReqCb_t *callback);

/***********************************************************************************//**
    \brief Performs a decryption of the payload.
    \param[in] buf - pointer to the output buffer structure.
    \param[in] callback - callback function which is called when decryption is finished.
 **************************************************************************************/
NWK_PRIVATE void zbProNwkDecryptOutputPacket(ZbProNwkOutputBuffer_t *const buf, ZbProSspDecryptReqCb_t *callback);

/***********************************************************************************//**
    \brief Performs a decryption of the payload.
    \param[in] buf - pointer to the input buffer structure.
    \param[in] callback - callback function which is called when decryption is finished.
 **************************************************************************************/
NWK_PRIVATE void zbProNwkDecryptInputPacket(ZbProNwkInputBuffer_t *const buf, ZbProSspDecryptReqCb_t *callback);

#endif /* _BB_ZB_PRO_NWK_SECURITY_H */