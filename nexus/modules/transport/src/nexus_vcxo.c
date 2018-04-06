/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_transport_module.h"

BDBG_MODULE(nexus_vcxo);

#ifndef NEXUS_NUM_VCXOS
#define NEXUS_NUM_VCXOS 0
#endif

#if NEXUS_NUM_VCXOS > 0
static NEXUS_VcxoSettings g_vcxoSettings[NEXUS_NUM_VCXOS];

#if BCHP_CHIP == 7211
#include "nexus_vcxo_impl_ott.c"
#elif BCHP_CHIP == 7019 || BCHP_CHIP == 7125 || \
    BCHP_CHIP == 7205 || \
    BCHP_CHIP == 7325 || BCHP_CHIP == 7335 || BCHP_CHIP == 7340 || BCHP_CHIP == 7342 || \
    BCHP_CHIP == 7400 || BCHP_CHIP == 7405 || BCHP_CHIP == 7408 || BCHP_CHIP == 7409 || BCHP_CHIP == 7410 || BCHP_CHIP == 7420 || BCHP_CHIP == 7466 || BCHP_CHIP == 7468 || \
    BCHP_CHIP == 3380
#include "nexus_vcxo_impl_65nm.c"
#else
#include "nexus_vcxo_impl_40nm.c"
#endif
#endif /* NEXUS_NUM_VCXOS > 0 */

void NEXUS_Vcxo_GetSettings(
    NEXUS_Vcxo vcxo,
    NEXUS_VcxoSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);

    if ( vcxo >= NEXUS_NUM_VCXOS || vcxo < NEXUS_Vcxo_e0 )
    {
        BDBG_ERR(("VCXO %u not supported on this chip/platform", vcxo));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

#if NEXUS_NUM_VCXOS > 0
    *pSettings = g_vcxoSettings[vcxo];
#endif
}

NEXUS_Error NEXUS_Vcxo_SetSettings(
    NEXUS_Vcxo vcxo,
    const NEXUS_VcxoSettings *pSettings
    )
{
    unsigned timebaseIndex;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pSettings);

    if ( vcxo >= NEXUS_NUM_VCXOS || vcxo < NEXUS_Vcxo_e0 )
    {
        BDBG_ERR(("VCXO %u not supported on this chip/platform", vcxo));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    errCode = NEXUS_Timebase_GetIndex(pSettings->timebase, &timebaseIndex);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

#if NEXUS_NUM_VCXOS > 0
    NEXUS_Vcxo_P_SetTimebase(vcxo, timebaseIndex);
    BDBG_MSG(("Setting VCXO %u to use timebase %u", (unsigned)vcxo, timebaseIndex));
    g_vcxoSettings[vcxo] = *pSettings;
#endif

    return BERR_SUCCESS;
}

void NEXUS_Vcxo_Init(void)
{
    if ( g_NEXUS_Transport_P_State.settings.initVcxos )
    {
        #if NEXUS_NUM_VCXOS > 0
        unsigned i;
        NEXUS_Vcxo_P_Init();
        for ( i = 0; i < NEXUS_NUM_VCXOS; i++ )
        {
            NEXUS_Error rc;
            NEXUS_VcxoSettings vcxoSettings;
            NEXUS_Vcxo_GetSettings(i, &vcxoSettings);
            rc = NEXUS_Vcxo_SetSettings(i, &vcxoSettings);
            if (rc) BERR_TRACE(rc); /* keep going */
        }
        #endif
    }
}

void NEXUS_Vcxo_Uninit(void)
{
    return;
}
