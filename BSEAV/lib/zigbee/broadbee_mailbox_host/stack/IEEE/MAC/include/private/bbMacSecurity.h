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
 *      MAC frame security procedures interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_SECURITY_H
#define _BB_MAC_SECURITY_H

/************************* INCLUDES ***********************************************************************************/
#include "private/bbMacSecurityDefs.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief Enumerator for the status for the security procedures.
 */
typedef enum _MAC_SecurityStatus_t
{
    MAC_SECURITY_PASSED = 0,                 /*!< Check or retrieval procedure successfully finished. */
    MAC_SECURITY_FAILED,                     /*!< Check or retrieval procedure failed. */
    MAC_SECURITY_CONDITIONALLY_PASSED        /*!< Need to be checked additionally. */
} MAC_SecurityStatus_t;

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Outgoing frame security procedure.
 * \param[in]   pMpduSurr       Pointer to the secured frame descriptor. It holds SecurityLevel, KeyIdMode, KeySource,
 *  KeyIndex and all necessary parameters of the outgoing frame.
 * \param[in]   secMpduLen      Secured frame MPDU length, in bytes, calculated by the caller.
 * \param[out]  pKey            Pointer to the buffer allocated by the caller for the retrieved Key.
 * \return  Status of operation, either SUCCESS if procedure finished successfully, or one of failure statuses
 *  otherwise: UNSUPPORTED_SECURITY, FRAME_TOO_LONG, COUNTER_ERROR, UNAVAILABLE_KEY.
 * \note    This function does not start the forward CCM* transformation. It must be done by the caller. Hence, this
 *  function updates the local macFrameCounter because forward CCM* transformation may not return failure.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.1.
 */
MAC_Status_t  MAC_SecurityOutgoing(
        const MacMpduSurr_t *const pMpduSurr,
        const PHY_FrameLen_t secMpduLen,
        MAC_SecurityKey_t pKey);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Incoming frame security procedure.
 * \param[in]   pMpduSurr               Pointer to the unsecured frame descriptor. It holds SecurityLevel, KeyIdMode,
 *  KeySource, KeyIndex and all necessary parameters of the incoming frame.
 * \param[out]  pKeyDescriptorsIdSet    Pointer to the buffer allocated by the caller for the set of indices of
 *  KeyDescriptor, DeviceDescriptor, KeyDeviceDescriptor retrieved for postponed processing.
 * \param[out]  pKey                    Pointer to the buffer allocated by the caller for the retrieved Key.
 * \param[out]  pSourceAddress          Pointer to the buffer allocated by the caller for the retrieved Extended Address
 *  of the remote device originated unsecured frame.
 * \return  Status of operation, either SUCCESS if procedure finished successfully, or one of failure statuses
 *  otherwise: UNSUPPORTED_LEGACY, UNSUPPORTED_SECURITY, IMPROPER_SECURITY_LEVEL, UNAVAILABLE_KEY, IMPROPER_KEY_TYPE,
 *  COUNTER_ERROR.
 * \note    This function does not start the inverse CCM* transformation, and does not update the remote FrameCounter
 *  stored in the DeviceDescriptor, as well as the Blacklisted flag of the stored KeyDeviceDescriptor. These must be
 *  done by the caller on completion of the inverse CCM* transformation.
 * \note    This function may not return the SECURITY_ERROR failure status because it may be returned only after CCM*
 *  operation which is not started by this function.
 * \details Indices of KeyDescriptor, DeviceDescriptor, and KeyDeviceDescriptor will be used by the caller after
 *  completion of the inverse CCM* transformation for updating values of the DeviceDescriptor field FrameCounter and
 *  KeyDeviceDescriptor field Blacklisted.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.8.2.3.
 */
MAC_Status_t  MAC_SecurityIncoming(
        const MacMpduSurr_t *const pMpduSurr,
        MAC_KeyDescriptorsIdSet_t *const pKeyDescriptorsIdSet,
        MAC_SecurityKey_t pKey,
        MAC_ExtendedAddress_t *const pSourceAddress);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Validation function for the security parameters of the
 *          any MAC primitives, which are supported security.
 * \param[in]   securityLevel      Security Level.
 * \param[in]   pSecurityParams    pointer to the Security Parameters structure.
 * \return
 *      TRUE if all parameters are valid.
 */
bool MAC_SecurityValidateSecurityParams(
                                  MAC_SecurityLevel_t const          securityLevel,
                                  MAC_SecurityParams_t const * const pSecurityParams);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Nonce composer function.
 * \param[in]   pMpduSurr       pointer to the secured frame descriptor. It holds SecurityLevel and FrameCounter.
 * \param[in]   sourceAddress   Extended Address of the originating device
 * \param[out]  pNonce          pointer to the buffer for CCM* Nonce.
 * \return
 *      nothing.
 *  See IEEE Std 802.15.4-2006, subclause 7.6.3.2.
 */
void MAC_SecurityNonceComposer(const MacMpduSurr_t *const pMpduSurr,
                               const MAC_ExtendedAddress_t sourceAddress,
                               MAC_SecurityNonce_t pNonce);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Evaluates MIC size, in bytes, after given Security Level.
 * \param[in]   securityLevel       Security Level code, from 0x0 to 0x7.
 * \return  Size of MIC field, in bytes, from the set 0, 4, 8, 16.
 */
SYS_DataLength_t MAC_SecurityGetMICSize(const MAC_SecurityLevel_t securityLevel);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Converter to the bid-endian representation.
 * \param[in]   src     pointer to value, which need to be converted (source).
 * \param[out]  dst     pointer to result value (destination).
 * \param[in]   len     length in bytes of the from value and result value.
 * \return
 *      nothing.
 * \note   Arrays pointed with \p from and \p to must not intersect.
 */
void MAC_SecurityConvertToBigEndian(void *const dst, const void *const src, const size_t len);

#endif /* _BB_MAC_SECURITY_H */

/* eof bbMacSecurity.h */