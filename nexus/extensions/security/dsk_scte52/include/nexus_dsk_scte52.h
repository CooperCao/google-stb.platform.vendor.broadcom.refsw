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
#ifndef NEXUS_DSK_SCTE52_H__
#define NEXUS_DSK_SCTE52_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Summary: Configuration parmaters for NEXUS_DskScte52_LoadLicenseConfig
 */
typedef struct NEXUS_DskScte52LicenseConfig {
    NEXUS_SecurityKeyladderType     keyladderType;      /* default NEXUS_SecurityKeyladderType_eAes128 */
    NEXUS_SecurityOperation         operation;          /* default NEXUS_SecurityOperation_eDecrypt */
    NEXUS_SecurityGlobalKeyOwnerID  globalKeyOwnerId;   /* default NEXUS_SecurityGlobalKeyOwnerID_eMSP1 */
    unsigned                        askmGlobalKeyIndex; /* default 0 */
    unsigned                        caVendorId;         /* default 0xAB1F */
    NEXUS_SecurityOtpId             otpId;              /* STB owner ID, default NEXUS_SecurityOtpId_eOneVal */

} NEXUS_DskScte52LicenseConfig;

/*
 * Summary: returns the default setting for NEXUS_DskScte52LicenseConfig
 */
void NEXUS_DskScte52_GetDefaultLicenseConfig(
    NEXUS_DskScte52LicenseConfig  *pConfig       /* [out] */
    );

/*
 * Summary:
 *  Sets the contorl word protection key License and IV for DSK Scte52
 */
NEXUS_Error NEXUS_DskScte52_LoadLicenseConfig(
    const NEXUS_DskScte52LicenseConfig  *pConfig
    );

#ifdef __cplusplus
}
#endif

#endif
