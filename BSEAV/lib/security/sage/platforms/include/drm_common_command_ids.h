/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef __DRM_COMMON_COMMAND_IDS_H__
#define __DRM_COMMON_COMMAND_IDS_H__


#ifdef __cplusplus
extern "C" {
#endif

/*
 * List of supported SAGE commands
 * */

typedef enum DrmWVOemCrypto_CommandId_e
{
    DrmWVOEMCrypto_CommandId_eInstallKeybox            = 0,
    DrmWVOEMCrypto_CommandId_eGetKeyData               = 1,
    DrmWVOEMCrypto_CommandId_eIsKeyboxValid            = 2,
    DrmWVOEMCrypto_CommandId_eGetDeviceID              = 4,
    DrmWVOEMCrypto_CommandId_eWrapKeybox               = 5,
    DrmWVOEMCrypto_CommandId_eOpenSession              = 6,
    DrmWVOEMCrypto_CommandId_eCloseSession             = 7,
    DrmWVOEMCrypto_CommandId_eDecryptCTR_V10           = 8,
    DrmWVOEMCrypto_CommandId_eGenerateDerivedKeys      = 9,
    DrmWVOEMCrypto_CommandId_eGenerateSignature        = 10,
    DrmWVOEMCrypto_CommandId_eGenerateNonce            = 11,
    DrmWVOEMCrypto_CommandId_eLoadKeys_V9_or_V10       = 12,
    DrmWVOEMCrypto_CommandId_eRefreshKeys              = 13,
    DrmWVOEMCrypto_CommandId_eSelectKey_V13            = 14,
    DrmWVOEMCrypto_CommandId_eRewrapDeviceRSAKey       = 15,
    DrmWVOEMCrypto_CommandId_eLoadDeviceRSAKey         = 16,
    DrmWVOEMCrypto_CommandId_eGenerateRSASignature     = 17,
    DrmWVOEMCrypto_CommandId_eDeriveKeysFromSessionKey = 18,
    DrmWVOEMCrypto_CommandId_eGeneric_Encrypt          = 19,
    DrmWVOEMCrypto_CommandId_eGeneric_Decrypt          = 20,
    DrmWVOEMCrypto_CommandId_eGeneric_Sign             = 21,
    DrmWVOEMCrypto_CommandId_eGeneric_Verify           = 22,

    /* APIs for v9 support */
    DrmWVOEMCrypto_CommandId_eUpdateUsageTable         = 23,
    DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry_V12 = 24,
    DrmWVOEMCrypto_CommandId_eReportUsage              = 25,
    DrmWVOEMCrypto_CommandId_eDeleteUsageEntry         = 26,
    DrmWVOEMCrypto_CommandId_eDeleteUsageTable         = 27,

    DrmWVOEMCrypto_CommandId_eGetHDCPCapability        = 28,

    /*APIs for v10 Support*/
    DrmWVOEMCrypto_CommandId_eGetNumberOfOpenSessions  = 29,
    DrmWVOEMCrypto_CommandId_eGetMaxNumberOfSessions   = 30,
    DrmWVOEMCrypto_CommandId_eQueryKeyControl          = 31,
    DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry    = 32,

    /* APIs for v11 Support */
    DrmWVOEMCrypto_CommandId_eDecryptCENC              = 33,
    DrmWVOEMCrypto_CommandId_eSecurityPatchLevel       = 34,
    DrmWVOEMCrypto_CommandId_eLoadKeys_V11_or_V12      = 35,

    /* APIs for v12 Support */
    DrmWVOEMCrypto_CommandId_eGetProvisioningMethod    = 36,
    DrmWVOEMCrypto_CommandId_eGetOEMPublicCertificate  = 37,
    DrmWVOEMCrypto_CommandId_eRewrapDeviceRSAKey30     = 38,

    /* APIs for v13 Support */
    DrmWVOEMCrypto_CommandId_eSupportedCertificates    = 39,
    DrmWVOEMCrypto_CommandId_eIsSRMUpdateSupported     = 40,
    DrmWVOEMCrypto_CommandId_eGetCurrentSRMVersion     = 41,
    DrmWVOEMCrypto_CommandId_eLoadSRM                  = 42,
    DrmWVOEMCrypto_CommandId_eLoadKeys_V13             = 43,
    DrmWVOEMCrypto_CommandId_eRemoveSRM                = 44,
    DrmWVOEMCrypto_CommandId_eCreateUsageTableHeader   = 45,
    DrmWVOEMCrypto_CommandId_eLoadUsageTableHeader     = 46,
    DrmWVOEMCrypto_CommandId_eCreateNewUsageEntry      = 47,
    DrmWVOEMCrypto_CommandId_eLoadUsageEntry           = 48,
    DrmWVOEMCrypto_CommandId_eUpdateUsageEntry         = 49,
    DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry     = 50,
    DrmWVOEMCrypto_CommandId_eShrinkUsageTableHeader   = 51,
    DrmWVOEMCrypto_CommandId_eMoveEntry                = 52,
    DrmWVOEMCrypto_CommandId_eCopyOldUsageEntry        = 53,
    DrmWVOEMCrypto_CommandId_eCreateOldUsageEntry      = 54,
    DrmWVOEMCrypto_CommandId_eIsMgnInitialized         = 55,
    DrmWVOEMCrypto_CommandId_eNoWritePending           = 56,
    DrmWVOEMCrypto_CommandId_eIsSsdReady               = 57,
    DrmWVOEMCrypto_CommandId_eGetAnalogOutputFlags     = 58,
    DrmWVOEMCrypto_CommandId_eLoadEntitledContentKeys  = 59,
    DrmWVOEMCrypto_CommandId_eSelectKey                = 60,
    DrmWVOEMCrypto_CommandId_eLoadKeys                 = 61,
    DrmWVOEMCrypto_CommandId_eLoadTestKeybox           = 62,

    DrmWVOEMCrypto_CommandId_eMax
}DrmWVOemCrypto_CommandId_e;

typedef enum DrmWVOemCrypto_NotificationId_e
{
    DrmWVOEMCrypto_NotificationId_eKeySlotInvalidated = 0,
    DrmWVOEMCrypto_NotificationId_eKeySlotsFlushed    = 1,

    DrmWVOEMCrypto_NotificationId_eMax
}DrmWVOemCrypto_NotificationId_e;

/*
 * List of supported SAGE commands for Dtcp-IP module
 * */
typedef enum DrmDtcpIpTl_CommandId_e
{
    DrmDtcpIpTl_CommandId_GetRNG = 0,
    DrmDtcpIpTl_CommandId_GetRNGMax,
    DrmDtcpIpTl_CommandId_GetDeviceCertificate,
    DrmDtcpIpTl_CommandId_ModAdd,
    DrmDtcpIpTl_CommandId_ComputeRttMac,
    DrmDtcpIpTl_CommandId_ComputeRttMac_2,
    DrmDtcpIpTl_CommandId_CheckOverFlow,
    DrmDtcpIpTl_CommandId_GetFirstPhaseValue,
    DrmDtcpIpTl_CommandId_GetSharedSecret,
    DrmDtcpIpTl_CommandId_SignData_BinKey,
    DrmDtcpIpTl_CommandId_VerifyData_BinKey,
    DrmDtcpIpTl_CommandId_CreateContentKey,
    DrmDtcpIpTl_CommandId_UpdateKeyIv,
    DrmDtcpIpTl_CommandId_FreeKeySlot,
    DrmDtcpIpTl_CommandId_eMax
}DrmDtcpIpTl_CommandId_e;

/*
 * List of supported SAGE commands for Edrm module
 * */
typedef enum DrmEdrmTl_CommandId_e
{
    DrmEdrmTl_CommandId_GetDeviceCertificate = 0,
    DrmEdrmTl_CommandId_GetDevicePublicKey,
}DrmEdrmTl_CommandId_e;


#ifdef __cplusplus
}
#endif

#endif /*__DRM_COMMON_COMMAND_IDS_H__*/
