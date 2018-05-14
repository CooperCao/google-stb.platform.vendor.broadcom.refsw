/***************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef NEXUS_SAGE_INIT_H__
#define NEXUS_SAGE_INIT_H__

#include "nexus_types.h"
#include "nexus_sage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Settings used to configure the Sage module.

Description:

See Also:
NEXUS_SageModule_GetDefaultSettings
NEXUS_SageModule_Init
***************************************************************************/
typedef struct NEXUS_SageModuleInternalSettings
{
    NEXUS_ModuleHandle security; /* Used for security. */
    struct {
        NEXUS_MemoryBlockHandle block;
        unsigned size;
    } videoDecoderFirmware;
    bool lazyUnmap;
} NEXUS_SageModuleInternalSettings;


/***************************************************************************
Summary:
Get default settings for the Security module initialization.

Description:
This is required in order to make application code resilient to the addition
of new structure members in the future.

See Also:
NEXUS_SageModule_Init
***************************************************************************/
void NEXUS_SageModule_GetDefaultSettings(
    NEXUS_SageModuleSettings *pSettings
    );

/***************************************************************************
Summary:
Get default settings for the Security module initialization (Internal).

Description:
This is required in order to make application code resilient to the addition
of new structure members in the future.

See Also:
NEXUS_SageModule_Init
***************************************************************************/
void NEXUS_SageModule_GetDefaultInternalSettings
(
 NEXUS_SageModuleInternalSettings *pInternalSettings
 );

/***************************************************************************
Summary:
Initialize the Sage module.

Description:
This function is called by NEXUS_Platform_Init, not by applications.
***************************************************************************/
NEXUS_ModuleHandle NEXUS_SageModule_Init(
    const NEXUS_SageModuleSettings *pSettings,
    const NEXUS_SageModuleInternalSettings *pInternalSettings
    );

/***************************************************************************
Summary:
Un-Init the Sage Module
***************************************************************************/
void NEXUS_SageModule_Uninit(
    void
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_INIT_H__ */
