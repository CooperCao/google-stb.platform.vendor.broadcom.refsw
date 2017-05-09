/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_SECURITY_INIT_H__
#define NEXUS_SECURITY_INIT_H__

#include "nexus_types.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings used to configure the Security module.

Description:

See Also:
NEXUS_SecurityModule_GetDefaultInternalSettings
NEXUS_SecurityModule_Init
**/
typedef struct NEXUS_SecurityModuleInternalSettings
{
    NEXUS_ModuleHandle transport;
    bool callTransportPostInit;
} NEXUS_SecurityModuleInternalSettings;

/**
Summary:
Get default settings for the Security module initialization.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.

See Also:
NEXUS_SecurityModule_Init
**/
void NEXUS_SecurityModule_GetDefaultInternalSettings(
    NEXUS_SecurityModuleInternalSettings *pSettings /* [out] */
    );

/**
Summary:
Get default settings for the Security module initialization.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.

See Also:
NEXUS_SecurityModule_Init
**/
void NEXUS_SecurityModule_GetDefaultSettings(
    NEXUS_SecurityModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the Security module.

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this 
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.securitySettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

The keyslot table can only be initialized once per power cycle of the chip, and on some chips the values of a previous initialization can be read back.
NEXUS_SecurityModule_GetCurrentSettings can read that information.
**/    
NEXUS_ModuleHandle NEXUS_SecurityModule_Init(
    const NEXUS_SecurityModuleInternalSettings *pModuleSettings,
    const NEXUS_SecurityModuleSettings *pSettings
    );

/**
Summary:
Retrieve the current Security module settings.

Description:
This function retrieves the current Security module settings.

If the application has modified pSettings->numKeySlotsForType, or wishes to read the number of keyslots allocated per type, this returns the information.

See Also:
NEXUS_SecurityModule_Init
**/     
void NEXUS_SecurityModule_GetCurrentSettings(
    NEXUS_ModuleHandle module,
    NEXUS_SecurityModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the Security module.

Description:
Called by NEXUS_Platform_Uninit
**/     
void NEXUS_SecurityModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
