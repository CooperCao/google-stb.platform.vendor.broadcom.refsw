/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 *
 **************************************************************************/
#ifndef NEXUS_SMARTCARD_INIT_H__
#define NEXUS_SMARTCARD_INIT_H__

#include "nexus_smartcard.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings used to configure the Smartcard module.

Description:

See Also:
NEXUS_SmartcardModule_GetDefaultInternalSettings
NEXUS_SmartcardModule_Init
**/
typedef struct NEXUS_SmartcardModuleInternalSettings
{
    NEXUS_ModuleHandle audio; /* needed for smartcard implementations that use an audio PLL */
} NEXUS_SmartcardModuleInternalSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_SmartcardModule_Init
**/
void NEXUS_SmartcardModule_GetDefaultInternalSettings(
    NEXUS_SmartcardModuleInternalSettings *pSettings /* [out] */
    );

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_SmartcardModule_Init
**/
void NEXUS_SmartcardModule_GetDefaultSettings(
    NEXUS_SmartcardModuleSettings *pSettings /* [out] */
    );


/**
Summary:
Initialize the Smartcard module.

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.smartCardSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_SmartcardModule_Init(
    const NEXUS_SmartcardModuleInternalSettings *pModuleSettings,
    const NEXUS_SmartcardModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the Smartcard module.

Description:
Called by NEXUS_Platform_Uninit
**/
void NEXUS_SmartcardModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
