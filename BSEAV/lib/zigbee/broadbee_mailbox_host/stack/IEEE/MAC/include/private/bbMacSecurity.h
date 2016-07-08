/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSecurity.h $
*
* DESCRIPTION:
*   MAC frame security procedures interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SECURITY_H
#define _BB_MAC_SECURITY_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacSecurityDefs.h"


/************************* DEFINITIONS **************************************************/
/*************************************************************************************//**
 * \brief Enumerator for the status for the security procedures.
*****************************************************************************************/
typedef enum _MAC_SecurityStatus_t
{
    MAC_SECURITY_PASSED = 0,                 /*!< Check or retrieval procedure successfully
                                                  finished. */
    MAC_SECURITY_FAILED,                     /*!< Check or retrieval procedure failed. */
    MAC_SECURITY_CONDITIONALLY_PASSED        /*!< Need to be checked additionally. */
} MAC_SecurityStatus_t;

/*************************************************************************************//**
 * \brief   Outgoing frame security procedure.
 * \param[in]   pMpduSurr       pointer to the secured frame descriptor. It holds SecurityLevel, KeyIdMode, KeySource,
 *  KeyIndex.
 * \param[in]   secMpduLen      secured frame MPDU length, in bytes, calculated by the caller.
 * \param[out]  pKey            pointer to the buffer for the retrieved Key.
 * \return
 *      status
 * \details Possible values for the \p status parameter are the following:
 *  - UNSUPPORTED_SECURITY      The Security Enabled subfield of the Frame Control field
 *                              of the frame to be secured is inconsistent with the
 *                              \p secLevel parameter OR
 *                              macSecurityEnabled attribute is set to FALSE and the
 *                              \p secLevel is not equal to zero.
 *
 *  - FRAME_TOO_LONG            A frame resulting from processing has a length that is
 *                              greater than aMaxPHYPacketSize.
 *
 *  - COUNTER_ERROR             macFrameCounter attribute has the value 0xffffffff.
 *
 *  - UNAVAILABLE_KEY           the outgoing frame key retrieval procedure failed.
 *
 *  - SUCCESS                   the procedure finished successfully.
 *
 * \note   If the status is SUCCESS the result secured frame will be returned
 *         through in/out parameter \p pMpduSurr
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.5.8.2.1.
*****************************************************************************************/
MAC_Status_t MAC_SecurityOutgoing(const MacMpduSurr_t *const pMpduSurr,
                                  const PHY_FrameLen_t secMpduLen,
                                  MAC_SecurityKey_t pKey);


/*************************************************************************************//**
 * \brief   Incoming frame security procedure.
 * \param[in/out]    pMpduSurr         pointer to the frame to be unsecured.
 * \param[out]       pSecLevel         security level.
 * \param[out]       pKeyIdMode        key identifier mode.
 * \param[out]       pKeySource        key source.
 * \param[out]       pKeyIndex         key index.
 * \return
 *      usecured frame (through the \p pMpduSurr)
 *      security level (through the \p pSecLevel)
 *      key identifier mode (through the \p pKeyIdMode)
 *      key source (through the \p pKeySource)
 *      key index (through the \p pKeyIndex)
 *      status
 * \note   If the status is SUCCESS the result secured frame will be returned
 *         through in/out parameter \p pMpduSurr and parameters \p pSecLevel,
 *         \p pKeyIdMode, \p pKeySource and \p pKeyIndex will be valid.
 * \details Possible values for the \p status parameter are the following:
 *  - UNSUPPORTED_LEGACY           Security Enabled subfield of the Frame Control field
 *                                 of the frame to be secured is set to one, but
 *                                 the Frame Version number of the frame control field
 *                                 is set to zero.
 *
 *  - UNSUPPORTED_SECURITY         The Security Enabled subfield of the Frame Control field
 *                                 of the frame to be secured is inconsistent with the
 *                                 \p pSecLevel parameter OR
 *                                 macSecurityEnabled attribute is set to FALSE and the
 *                                 \p pSecLevel is not equal to zero.
 *
 *  - IMPROPER_SECURITY_LEVEL      The incomming security level checking procedure
 *                                 failed. Or it returned "conditionally passed" and
 *                                 Exempt element of the DeviceDescriptor is set to FALSE.
 *
 *  - UNAVAILABLE_KEY              The incoming frame security material retrieval procedure
 *                                 failed.
 *
 *  - IMPROPER_KEY_TYPE            The incoming key usage policy checking procedure failed.
 *
 *  - COUNTER_ERROR                macFrameCounter attribute has the value 0xffffffff OR
 *                                 frame counter is not greater than or equal to the
 *                                 FrameCounter element of the DeviceDescriptor.
 *
 *  - MAC_SECURITY_ERROR           The CCM* inverse transformation process failed.
 *
 *  - SUCCESS                      The procedure finished successfully.
 *
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.5.8.2.3.
*****************************************************************************************/
MAC_Status_t MAC_SecurityIncoming(MacMpduSurr_t                * const pMpduSurr,
                                  MAC_KeyDescriptorId_t        * const pKeyDescrId,
                                  MAC_DeviceDescriptorId_t     * const pDeviceDescrId,
                                  MAC_KeyDeviceDescriptorId_t  * const pKeyDeviceDescrId);


/*************************************************************************************//**
 * \brief   Validation function for the security parameters of the
 *          any MAC primitives, which are supported security.
 * \param[in]   securityLevel      Security Level.
 * \param[in]   pSecurityParams    pointer to the Security Parameters structure.
 * \return
 *      TRUE if all parameters are valid.
*****************************************************************************************/
bool MAC_SecurityValidateSecurityParams(
                                  MAC_SecurityLevel_t const          securityLevel,
                                  MAC_SecurityParams_t const * const pSecurityParams);


/*************************************************************************************//**
 * \brief   Nonce composer function.
 * \param[in]   pMpduSurr       pointer to the secured frame descriptor. It holds SecurityLevel and FrameCounter.
 * \param[in]   sourceAddress   Extended Address of the originating device
 * \param[out]  pNonce          pointer to the buffer for CCM* Nonce.
 * \return
 *      nothing.
 *  See IEEE Std 802.15.4-2006, subclaus 7.6.3.2.
*****************************************************************************************/
void MAC_SecurityNonceComposer(const MacMpduSurr_t *const pMpduSurr,
                               const MAC_ExtendedAddress_t sourceAddress,
                               MAC_SecurityNonce_t pNonce);


/*************************************************************************************//**
 * \brief   Evaluates MIC size, in bytes, after given Security Level.
 * \param[in]   securityLevel       Security Level code, from 0x0 to 0x7.
 * \return  Size of MIC field, in bytes, from the set 0, 4, 8, 16.
*****************************************************************************************/
SYS_DataLength_t MAC_SecurityGetMICSize(const MAC_SecurityLevel_t securityLevel);


/**//**
 * \brief   Converter to the bid-endian representation.
 * \param[in]    from     pointer to value, which need to be converted.
 * \param[out]   to       pointer to result value.
 * \param[in]    len      length in bytes of the from value and result value.
 * \return
 *      nothing.
 * \note   Arrays pointed with \p from and \p to must not intersect.
*/
void macSecurityConvertToBigEndian(void const * const from, void * const to, uint8_t len);


#endif /* _BB_MAC_SECURITY_H */
