/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_core_module.h"
#include "priv/nexus_implicit_objects.h"
#if NEXUS_POWER_MANAGEMENT
#include "bchp_pwr.h"
#endif
#include "bint_stats.h"
#include "nexus_base_statistics.h"
#include "bkni_metrics.h"
#include "priv/nexus_core_preinit.h"
#include "bchp_memc_gen_0.h"

BDBG_MODULE(nexus_avs);

static NEXUS_AvsSettings g_avsSettings;

void NEXUS_GetAvsSettings( NEXUS_AvsSettings *pSettings )
{
    *pSettings = g_avsSettings;
}

NEXUS_Error NEXUS_SetAvsSettings( const NEXUS_AvsSettings *pSettings )
{
    BDBG_ASSERT(pSettings);

    if (pSettings->hardStopOffset > 15)
        return NEXUS_INVALID_PARAMETER;
    if (pSettings->maxVoltageStopOffset > 15)
        return NEXUS_INVALID_PARAMETER;

    /* Note: if this hardware does not support these settings they will not be used */
    g_avsSettings = *pSettings;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_GetAvsDomainStatus(
    NEXUS_AvsDomain domain,  /* [in] index of domain to fetch status */
    NEXUS_AvsStatus *pStatus /* [out] the current domain-specific status */
)
{
    BCHP_AvsData data;
    BERR_Code rc;

    /* Note: if the AVS hardware is not supported this call will return an error */
    rc = BCHP_GetAvsData_isrsafe(g_NexusCore.publicHandles.chp, &data);
    if(rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    pStatus->enabled     = data.enabled;
    pStatus->tracking    = data.tracking;
    pStatus->heartbeat   = data.heartbeat;
    switch (domain) {
    case NEXUS_AvsDomain_eMain :
        pStatus->voltage      = data.voltage;
        pStatus->temperature  = data.temperature;
        break;

    case NEXUS_AvsDomain_eCpu :
        if (data.voltage1 == 0xffffffff)
            return NEXUS_INVALID_PARAMETER;
        pStatus->voltage      = data.voltage1;
        pStatus->temperature  = data.temperature1;
        break;

    default :
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return rc;
}

#if NEXUS_AVS_MONITOR
void
NEXUS_Core_P_MonitorPvt(void *context)
{
    BCHP_AvsSettings avsSettings;

    /* Note: if this hardware does not support these settings they will not be used */
    avsSettings.hardStopOffset = g_avsSettings.hardStopOffset;
    avsSettings.maxVoltageStopOffset = g_avsSettings.maxVoltageStopOffset;

    BSTD_UNUSED(context);

    BCHP_MonitorPvt(g_NexusCore.publicHandles.chp, &avsSettings);

    g_NexusCore.pvtTimer = NEXUS_ScheduleTimer(1000, NEXUS_Core_P_MonitorPvt, NULL);

    if(!g_NexusCore.pvtTimer) {
        BDBG_WRN(("NEXUS_Core_P_Timer: can't schedule PVT timer"));
    }
    return;
}
#endif /*NEXUS_AVS_MONITOR*/
