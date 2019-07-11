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

#ifndef KEYMASTER_H__
#define KEYMASTER_H__

/* utility module IDs  */
typedef enum Keymaster_ModuleId_e{
    Keymaster_ModuleId_eKeymaster = 0x01,
    Keymaster_ModuleId_eGatekeeper = 0x02
}Keymaster_ModuleId_e;

enum {
    /* Keymaster command IDs */
    KM_CommandId_eConfigure                = 0x0001,
    KM_CommandId_eAddRngEntropy            = 0x0002,
    KM_CommandId_eGenerateKeyStart         = 0x0003,
    KM_CommandId_eGenerateKeyComplete      = 0x0004,
    KM_CommandId_eGetKeyCharacteristics    = 0x0005,
    KM_CommandId_eImportKeyStart           = 0x0006,
    KM_CommandId_eImportKeyComplete        = 0x0007,
    KM_CommandId_eExportKeyStart           = 0x0008,
    KM_CommandId_eExportKeyComplete        = 0x0009,
    KM_CommandId_eAttestKeyStart           = 0x000A,
    KM_CommandId_eAttestKeyComplete        = 0x000B,
    KM_CommandId_eUpgradeKeyStart          = 0x000C,
    KM_CommandId_eUpgradeKeyComplete       = 0x000D,
    KM_CommandId_eDeleteKey                = 0x000E,
    KM_CommandId_eDeleteAllKeys            = 0x000F,
    KM_CommandId_eCryptoBegin              = 0x0010,
    KM_CommandId_eCryptoUpdate             = 0x0011,
    KM_CommandId_eCryptoFinish             = 0x0012,
    KM_CommandId_eCryptoGetDataStart       = 0x0013,
    KM_CommandId_eCryptoGetDataComplete    = 0x0014,
    KM_CommandId_eCryptoAbort              = 0x0015,
    KM_CommandId_eGetConfiguration         = 0x0016,
    KM_CommandId_eShutdown                 = 0x0017,
    KM_CommandId_eCacheKey                 = 0x0018,
    KM_CommandId_eGetHmacSharingParams     = 0x0019,
    KM_CommandId_eComputeSharedHmac        = 0x001A,
    KM_CommandId_eVerifyAuthorization      = 0x001B,
    KM_CommandId_eImportWrappedKey         = 0x001C,
    KM_CommandId_eImportKey                = 0x001D,
    KM_CommandId_eDestroyAttestationIds    = 0x001E,
    KM_CommandId_eMax
};

enum {
    /* Gatekeeper command IDs */
    KM_CommandId_eGatekeeperEnroll         = 0x0001,
    KM_CommandId_eGatekeeperVerify         = 0x0002,
    KM_CommandId_eGatekeeperMax
};


#endif /* #ifndef KEYMASTER_H__ */
