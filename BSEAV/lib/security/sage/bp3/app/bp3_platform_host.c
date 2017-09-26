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

#include <stdio.h> /* printf */

#include "bsagelib_types.h"
#include "sage_srai.h"

#include "bp3_platform_host.h"
#include "bp3_module_host.h"
#include "bp3_platform_ids.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#include "bsagelib_types.h"
#include "sage_srai.h"
#include "bdbg.h"
#endif

BDBG_MODULE(bp3_platform_host);

static SRAI_PlatformHandle hBP3Platform = NULL;
void SAGE_BP3Platform_UnInstall()
{

    SRAI_Platform_UnInstall(SAGE_PLATFORM_ID_BP3);
}

int SAGE_BP3Platform_Install(uint8_t* taBinBuff, uint32_t taBinSize)
{
    BERR_Code sage_rc;
    int rc = 0;

    sage_rc = SRAI_Platform_Install(SAGE_PLATFORM_ID_BP3, taBinBuff, taBinSize);
    if (sage_rc != BERR_SUCCESS) {
        rc = 1;
        printf("%s: Cannot Install Platform\n",__FUNCTION__);
    }
    return rc;
}

void SAGE_BP3Platform_Uninit(void)
{
    if (hBP3Platform) {
        SAGE_BP3Module_Uninit();
        SRAI_Platform_Close(hBP3Platform);
        hBP3Platform = NULL;
    }
}


int SAGE_BP3Platform_Init(void)
{
    BERR_Code sage_rc;
    BSAGElib_State state;
    int rc = 0;

#ifdef NXCLIENT_SUPPORT
    SRAI_Settings appSettings;

    /* Get Current Settings */
    SRAI_GetSettings(&appSettings);

    /* Customize appSettings, for example if designed to use NxClient API */
    appSettings.generalHeapIndex     = NXCLIENT_FULL_HEAP;
    appSettings.videoSecureHeapIndex = NXCLIENT_VIDEO_SECURE_HEAP;

    /* Save/Apply new settings */
    SRAI_SetSettings(&appSettings);
#endif

    /* Open the platform first */
    sage_rc = SRAI_Platform_Open(SAGE_PLATFORM_ID_BP3,
                                 &state,
                                 &hBP3Platform);
    if (sage_rc != BERR_SUCCESS) {
        rc = 1;
        printf("%s: Cannot Open Platform\n",__FUNCTION__);
        goto end;
    }

    /* Check init state */
    if (state != BSAGElib_State_eInit)
    {
        BDBG_MSG(("Initializing BP3 platform"));
        /* Not yet initialized: send init command*/
        sage_rc = SRAI_Platform_Init(hBP3Platform, NULL);
        if (sage_rc != BERR_SUCCESS)
        {
            rc = 2;
            printf("%s: Failed to initialize BP3 platform.\n",__FUNCTION__);
            goto end;
        }
    }

    /* Initialize module */
    BDBG_MSG(("Initializing BP3 Module"));
    if (SAGE_BP3Module_Init(hBP3Platform)) {
        rc = 3;
        goto end;
    }

end:
    if (rc) {
        /* error: cleanup platform */
        SAGE_BP3Platform_Uninit();
    }
    return rc;
}
