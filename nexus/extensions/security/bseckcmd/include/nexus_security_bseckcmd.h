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

#ifndef NEXUS_SECURITY_BSECKCMD_H__
#define NEXUS_SECURITY_BSECKCMD_H__

/*=***********************************
*************************************/
#include "nexus_types.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum NEXUS_SecurityFirstTierKeyID
{
    NEXUS_SecurityFirstTierKeyID_eKey0Prime,
    NEXUS_SecurityFirstTierKeyID_eKey0,

    NEXUS_SecurityFirstTierKeyID_eMax
} NEXUS_SecurityFirstTierKeyID;

typedef enum NEXUS_SecuritySecondTierKeyID
{
    NEXUS_SecuritySecondTierKeyID_eNone, /* not multi-tier key */
    NEXUS_SecuritySecondTierKeyID_eKey1,
    NEXUS_SecuritySecondTierKeyID_eKey2,
    NEXUS_SecuritySecondTierKeyID_eKey3,
    NEXUS_SecuritySecondTierKeyID_eKey4,

    NEXUS_SecuritySecondTierKeyID_eMax
} NEXUS_SecuritySecondTierKeyID;


typedef struct NEXUS_SecurityVerifySecondTierKeySettings
{
    NEXUS_SecuritySecondTierKeyID     keyDestination;          /* where verified key is to be installed */
    NEXUS_SecurityFirstTierKeyID      firstTierRootKeySource;  /* key used for verification - first tier  */
    NEXUS_SecuritySecondTierKeyID     secondTierRootKeySource; /* key used for verification - multi-tier key */
    uint64_t keyAddress;              /* address in flash for key+signature */ 
} NEXUS_SecurityVerifySecondTierKeySettings;



/**
Summary:
Initialize second tier key verification settings
**/
void NEXUS_Security_GetDefaultVerifySecondTierKeySettings(
    NEXUS_SecurityVerifySecondTierKeySettings  *pSettings      /* [out] */
    );


/**
Summary:
Send BSECK command to verify a key to be installed as second tier key
**/

NEXUS_Error NEXUS_Security_VerifySecondTierKey(
    const NEXUS_SecurityVerifySecondTierKeySettings  *pSecondTierKeySettings
    );



typedef enum NEXUS_SecurityCodeVerifierMode
{
    NEXUS_SecurityCodeVerifierMode_eRsaSignature,
    NEXUS_SecurityCodeVerifierMode_eOtpHash2,

    NEXUS_SecurityCodeVerifierMode_eMax
} NEXUS_SecurityCodeVerifier;


typedef enum NEXUS_SecurityCodeLocation
{
    NEXUS_SecurityCodeLocation_eFlash,
    NEXUS_SecurityCodeLocation_eDram, 

    NEXUS_SecurityCodeLocation_eMax
} NEXUS_SecurityCodeLocation;

typedef struct NEXUS_SecurityVerifySecondStageCodeSettings
{
    NEXUS_SecurityCodeLocation codeLocation;   /* Location of second stage code   */
    uint64_t codeAddress;       /* address of second stage code, relative to codeLocation */
    uint64_t signatureAddress;  /* address of signature of second stage code, relative to codeLocation */
    
    NEXUS_SecurityCodeVerifier  verifierMode;    /* How second stage code is to be verified     */
    NEXUS_SecuritySecondTierKeyID  verifierKey;    /* Second tier key to be used for verification */

} NEXUS_SecurityVerifySecondStageCodeSettings;

/**
Summary:
Initialize second stage code verification settings
**/
void NEXUS_Security_GetDefaultVerifySecondStageCodeSettings(
    NEXUS_SecurityVerifySecondStageCodeSettings  *pSettings      /* [out] */
    );


/**
Summary:
Send BSECK command to verify a key to be installed as second tier key

If verification fails, ...
**/
NEXUS_Error NEXUS_Security_VerifySecondStageCode(
    const NEXUS_SecurityVerifySecondStageCodeSettings  *pCodeVerifySettings
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
