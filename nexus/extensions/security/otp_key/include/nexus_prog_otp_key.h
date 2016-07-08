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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef NEXUS_PROG_OTP_KEY_H__
#define NEXUS_PROG_OTP_KEY_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_otpmsp.h"


#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_OTP_KEY_LENGTH    (36) /* The same as BHSM_OTP_ENC_KEY_DATA_LEN */

typedef struct NEXUS_ProgramOtpKey {

   /* In: keyDest */
   NEXUS_OtpKeyType                        keyType;

   /* In: VKLId */
   NEXUS_SecurityVirtualKeyladderID        virtualKeyLadderID;

   /* In: KeyLayer*/
   NEXUS_SecurityKeySource                 keyLayer;

   /* In: OTPKeyData */
   uint8_t                                 keyData[NEXUS_MAX_OTP_KEY_LENGTH];

   /* In: OTP Key data size */
   uint32_t                                keyDataSize;

}   NEXUS_ProgramOtpKey;

/**
Summary:
Retrieve the default otp key settings.
**/

NEXUS_Error NEXUS_Security_GetDefaultProgramOtpKey(
   NEXUS_ProgramOtpKey *pOtpKey
   );

/**
Summary:
Configure the OTP key programming values.
**/

NEXUS_Error NEXUS_Security_ProgramOtpKey(
   const NEXUS_ProgramOtpKey *pOtpKey
   );

#ifdef __cplusplus
}
#endif

#endif
