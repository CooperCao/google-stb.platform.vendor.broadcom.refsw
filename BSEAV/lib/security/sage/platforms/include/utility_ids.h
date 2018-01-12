/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef UTILITY_H__
#define UTILITY_H__

/* utility module IDs  */
typedef enum Utility_ModuleId_e{
    Utility_ModuleId_eHeartbeat = 0x01,
    Utility_ModuleId_eRsa       = 0x02,
    Utility_ModuleId_eSecureRsa = 0x03,
    Utility_ModuleId_eSsce      = 0x04
}Utility_ModuleId_e;

enum {
    /* Heartbeat command IDs */
    Heartbeat_CommandId_eTakePulse = 0x001,
    Heartbeat_CommandId_eMax,

    /* Rsa command IDs */
    Rsa_CommandId_eGetPublicKey   = 0x100,
    Rsa_CommandId_eVerify         = 0x101,
    Rsa_CommandId_eSign           = 0x102,
    Rsa_CommandId_ePublicEncrypt  = 0x103,
    Rsa_CommandId_ePublicDecrypt  = 0x104,
    Rsa_CommandId_ePrivateEncrypt = 0x105,
    Rsa_CommandId_ePrivateDecrypt = 0x106,
    Rsa_CommandId_eMax,

    /* Secure Rsa command IDs */
    SecureRsa_CommandId_eLoadRsaPackage    = 0x200,
    SecureRsa_CommandId_eGetStatus         = 0x201,
    SecureRsa_CommandId_eRemoveKey         = 0x202,
    SecureRsa_CommandId_eRsaSign           = 0x210,
    SecureRsa_CommandId_eRsaVerify         = 0x211,
    SecureRsa_CommandId_eRsaHostUsage      = 0x212,
    SecureRsa_CommandId_eRsaDecryptKey3    = 0x213,
    SecureRsa_CommandId_eRsaDecryptKpk     = 0x214,
    SecureRsa_CommandId_eRsaLoadPublicKey  = 0x215,
    SecureRsa_CommandId_eKey3Import        = 0x220,
    SecureRsa_CommandId_eKey3Export        = 0x221,
    SecureRsa_CommandId_eKey3Route         = 0x222,
    SecureRsa_CommandId_eKey3Unroute       = 0x223,
    SecureRsa_CommandId_eKey3CalculateHmac = 0x224,
    SecureRsa_CommandId_eKey3AppendSha     = 0x225,
    SecureRsa_CommandId_eKey3LoadClearIkr  = 0x226,
    SecureRsa_CommandId_eKey3IkrDecryptIkr = 0x227,
    SecureRsa_CommandId_eKpkDecryptRsa     = 0x230,
    SecureRsa_CommandId_eKpkDecryptIkr     = 0x231,
    SecureRsa_CommandId_eMax,

    /* SSCE command IDs */
    Ssce_CommandId_eCreateKey         = 0x300,
    Ssce_CommandId_eLoadKey           = 0x301,
    Ssce_CommandId_eUpdateCertificate = 0x302,
    Ssce_CommandId_eRetrieve          = 0x303,
    Ssce_CommandId_eSign              = 0x304,
    Ssce_CommandId_eMax
};


#endif /* #ifndef UTILITY_H__ */
