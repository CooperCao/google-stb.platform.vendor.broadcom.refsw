/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "nexus_base.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_priv.h"
#include "bchp_common.h"
#include "breg_mem.h"

#if NEXUS_POWER_MANAGEMENT && !NEXUS_CPU_ARM
#if defined (BCHP_AON_PM_L2_REG_START)
#include "bchp_aon_pm_l2.h"
#elif defined (BCHP_PM_L2_REG_START)
#include "bchp_pm_l2.h"
#endif
#if defined (BCHP_WKTMR_REG_START)
#include "bchp_wktmr.h"
#endif
#endif

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend_init.h"
#include "priv/nexus_frontend_standby_priv.h"
#endif


BDBG_MODULE(nexus_platform_standby);


#if NEXUS_POWER_MANAGEMENT
NEXUS_PlatformStandbyState g_standbyState = {{NEXUS_PlatformStandbyMode_eOn,
                          {false,false,false,false,false,false,false,0}, false},
                         false};
#endif

void NEXUS_Platform_GetStandbySettings( NEXUS_PlatformStandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT
   *pSettings = g_standbyState.settings;
#else
   BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

#if NEXUS_POWER_MANAGEMENT
#if !NEXUS_CPU_ARM

#if defined (BCHP_AON_PM_L2_REG_START)
#define BCHP(val)   BCHP_AON_PM_L2_CPU_##val
#elif defined (BCHP_PM_L2_REG_START)
#define BCHP(val)   BCHP_PM_L2_CPU_##val
#endif

static void NEXUS_Platform_P_ResetWakeupDevices(const NEXUS_PlatformStandbySettings *pSettings)
{
    BSTD_UNUSED(pSettings);

    BREG_Write32(g_pCoreHandles->reg, BCHP(MASK_SET), 0xFFFFFFFF);
    BREG_Write32(g_pCoreHandles->reg, BCHP(CLEAR), 0xFFFFFFFF);
}

static void NEXUS_Platform_P_SetWakeupDevices(const NEXUS_PlatformStandbySettings *pSettings)
{
    unsigned val;

    val = BREG_Read32(g_pCoreHandles->reg, BCHP(MASK_CLEAR));

    /* Set IR for wakeup */
#if BCHP(MASK_CLEAR_IRR_INTR_MASK)
    if(pSettings->wakeupSettings.ir) {
        val |= BCHP(MASK_CLEAR_IRR_INTR_MASK);
    }
#endif

    /* Set keypad for wakeup */
#if BCHP(MASK_CLEAR_KPD_INTR_MASK)
    if(pSettings->wakeupSettings.keypad) {
        val |= BCHP(MASK_CLEAR_KPD_INTR_MASK);
    }
#endif

    /* Set gpio for wakeup */
#if BCHP(MASK_CLEAR_GPIO_MASK)
    if(pSettings->wakeupSettings.gpio) {
        val |= BCHP(MASK_CLEAR_GPIO_MASK);
    }
#endif

    /* Set cec for wakeup */
#if BCHP(MASK_CLEAR_CEC_INTR_MASK)
    if(pSettings->wakeupSettings.cec) {
        val |= BCHP(MASK_CLEAR_CEC_INTR_MASK);
    }
#endif

    /* Set xpt for wakeup */
#if BCHP(MASK_CLEAR_XPT_PMU_MASK)
    if(pSettings->wakeupSettings.transport) {
        val |= BCHP(MASK_CLEAR_XPT_PMU_MASK);
    }
#endif

    /* Set UHF for wakeup */
#if BCHP(MASK_CLEAR_UHFR_MASK)
    if(pSettings->wakeupSettings.uhf) {
        val |= BCHP(MASK_CLEAR_UHFR_MASK);
    }
#endif

    /* Set timeout for wakeup */
#if BCHP(MASK_CLEAR_TIMER_INTR_MASK)
    if(pSettings->wakeupSettings.timeout) {
        val |= BCHP(MASK_CLEAR_TIMER_INTR_MASK);
    }
#endif

    BREG_Write32(g_pCoreHandles->reg, BCHP(MASK_CLEAR), val);

    if(pSettings->wakeupSettings.timeout) {
        unsigned counter = BREG_Read32(g_pCoreHandles->reg, BCHP_WKTMR_COUNTER);
        BREG_Write32(g_pCoreHandles->reg, BCHP_WKTMR_EVENT, 1);
        BREG_Write32(g_pCoreHandles->reg, BCHP_WKTMR_ALARM, (counter + pSettings->wakeupSettings.timeout));
    }
}

static void NEXUS_Platform_P_GetStandbyStatus(NEXUS_PlatformStandbyStatus *pStatus)
{
    unsigned l2CpuIntStatus;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    l2CpuIntStatus = BREG_Read32(g_pCoreHandles->reg, BCHP(STATUS));

    BDBG_MSG(("l2CpuIntStatus %08x", l2CpuIntStatus));

#if BCHP(STATUS_TIMER_INTR_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_TIMER_INTR_MASK)) {
        pStatus->wakeupStatus.timeout = true;
    }
#endif

    l2CpuIntStatus &= ~(BREG_Read32(g_pCoreHandles->reg, BCHP(MASK_STATUS)));

#if BCHP(STATUS_IRR_INTR_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_IRR_INTR_MASK)) {
        pStatus->wakeupStatus.ir = true;
    }
#endif

#if BCHP(STATUS_KPD_INTR_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_KPD_INTR_MASK)) {
        pStatus->wakeupStatus.keypad = true;
    }
#endif

#if BCHP(STATUS_GPIO_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_GPIO_MASK)) {
        pStatus->wakeupStatus.gpio = true;
    }
#endif

#if BCHP(STATUS_CEC_INTR_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_CEC_INTR_MASK)) {
        pStatus->wakeupStatus.cec = true;
    }
#endif

#if BCHP(STATUS_XPT_PMU_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_XPT_PMU_MASK)) {
        pStatus->wakeupStatus.transport = true;
    }
#endif

#if BCHP(STATUS_UHFR_PMU_MASK)
    if (l2CpuIntStatus & BCHP(STATUS_UHFR_PMU_MASK)) {
        pStatus->wakeupStatus.uhf = true;
    }
#endif


}
#endif /* NEXUS_CPU_ARM */

static NEXUS_Error NEXUS_Platform_P_Standby(const NEXUS_PlatformStandbySettings *pSettings)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_Platform_P_ModuleInfo *module_info;
    NEXUS_StandbySettings standby_settings;

    BDBG_CASSERT(sizeof(standby_settings) == sizeof(*pSettings));
    BKNI_Memcpy(&standby_settings, pSettings, sizeof(*pSettings));

    for (module_info = BLST_Q_FIRST(&g_NEXUS_platformHandles.handles); module_info; module_info = BLST_Q_NEXT(module_info, link)) {
        bool power_down=true, lock=true;

        if (pSettings->mode == NEXUS_PlatformStandbyMode_eActive && module_info->lock_mode == NEXUS_PlatformStandbyLockMode_ePassiveOnly) {
            power_down = lock = false;
        }
        if (module_info->lock_mode == NEXUS_PlatformStandbyLockMode_eNone) {
            lock = false;
        }

        /* We could transition from active to passive or vice versa */
        /* So modules could transition from standby -> resume and locked -> unlocked and vice versa */
        if (lock && !module_info->locked) {
            BDBG_MSG(("Disable module %s", NEXUS_Module_GetName(module_info->module)));
            NEXUS_Module_Disable(module_info->module);
            module_info->locked = true;
        }

        if (module_info->standby) {
            if (power_down && !module_info->powerdown) {
                BDBG_MSG(("Standby module %s", NEXUS_Module_GetName(module_info->module)));
                errCode = module_info->standby(power_down, &standby_settings);
                if (errCode) { errCode = BERR_TRACE(errCode);goto err; }
                    module_info->powerdown = true;
	    }
	    else if (!power_down && module_info->powerdown) {
                    BDBG_MSG(("Resume module %s", NEXUS_Module_GetName(module_info->module)));
                    errCode = module_info->standby(power_down, &standby_settings);
                    if (errCode) { errCode = BERR_TRACE(errCode);goto err; }
                    module_info->powerdown = false;
	    }
	}
        if (!lock && module_info->locked) {
            BDBG_MSG(("Enable module %s", NEXUS_Module_GetName(module_info->module)));
            NEXUS_Module_Enable(module_info->module);
            module_info->locked = false;
        }
    }

err:
    return errCode;
}

static NEXUS_Error NEXUS_Platform_P_Resume(const NEXUS_PlatformStandbySettings *pSettings)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_Platform_P_ModuleInfo *module_info;
    NEXUS_StandbySettings standby_settings;

    BDBG_CASSERT(sizeof(standby_settings) == sizeof(*pSettings));
    BKNI_Memcpy(&standby_settings, pSettings, sizeof(*pSettings));

    for (module_info = BLST_Q_LAST(&g_NEXUS_platformHandles.handles); module_info; module_info = BLST_Q_PREV(module_info, link)) {
        if (module_info->standby && module_info->powerdown) {
            BDBG_MSG(("Resume module %s", NEXUS_Module_GetName(module_info->module)));
            errCode = module_info->standby(false, &standby_settings);
            if (errCode) { errCode=BERR_TRACE(errCode);goto err; }
            module_info->powerdown = false;
        }
        if (module_info->locked) {
            BDBG_MSG(("Enable module %s", NEXUS_Module_GetName(module_info->module)));
            NEXUS_Module_Enable(module_info->module);
            module_info->locked = false;
        }
    }

err:
    return errCode;
}
#endif /* NEXUS_POWER_MANAGEMENT */

NEXUS_Error NEXUS_Platform_P_SetStandbySettings( const NEXUS_PlatformStandbySettings *pSettings, bool resetWakeupStatus )
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BDBG_ASSERT(pSettings);

    if (g_NEXUS_platformModule == NULL ) {
        return NEXUS_NOT_INITIALIZED;
    }

    BSTD_UNUSED(resetWakeupStatus);

    /* If state is unchanged; let app reconfigure wakeup up devices and exit */
    /* If Sage is defined, then we need to wakeup and go to sleep again. This allows
     * for Sage BSP handshake to complete.
     * TODO : Only resume Sage and put it back to sleep instead of waking up all modules */
#ifndef NEXUS_HAS_SAGE
    if (pSettings->mode == g_standbyState.settings.mode) {
        goto set_wakeup;
    }
#endif

    BDBG_MSG(("Entering Standby mode %d", pSettings->mode));

    BDBG_CASSERT(NEXUS_PlatformStandbyMode_eOn == (NEXUS_PlatformStandbyMode)NEXUS_StandbyMode_eOn);
    BDBG_CASSERT(NEXUS_PlatformStandbyMode_eActive == (NEXUS_PlatformStandbyMode)NEXUS_StandbyMode_eActive);
    BDBG_CASSERT(NEXUS_PlatformStandbyMode_ePassive == (NEXUS_PlatformStandbyMode)NEXUS_StandbyMode_ePassive);
    BDBG_CASSERT(NEXUS_PlatformStandbyMode_eDeepSleep == (NEXUS_PlatformStandbyMode)NEXUS_StandbyMode_eDeepSleep);

    if (g_standbyState.settings.mode == NEXUS_PlatformStandbyMode_eDeepSleep && pSettings->mode!= NEXUS_PlatformStandbyMode_eDeepSleep) {
        NEXUS_Platform_P_InitBoard();
        NEXUS_Platform_P_InitPinmux();
    }

    /* First resume/unlock all modules so that S3 specific standby settings can be applied and frontend can be uninit */
    errCode = NEXUS_Platform_P_Resume(pSettings);
    if (errCode) { errCode = BERR_TRACE(errCode);goto err; }

    switch(pSettings->mode) {
    case NEXUS_PlatformStandbyMode_eDeepSleep:
#if NEXUS_VIDEO_SECURE_HEAP
        NEXUS_Platform_P_SetStandbyExclusionRegion(NEXUS_VIDEO_SECURE_HEAP);
#endif
#if NEXUS_SAGE_SECURE_HEAP
        NEXUS_Platform_P_SetStandbyExclusionRegion(NEXUS_SAGE_SECURE_HEAP);
#endif
        /* fall through */

    case NEXUS_PlatformStandbyMode_eActive:
    case NEXUS_PlatformStandbyMode_ePassive:
#if !NEXUS_CPU_ARM
        NEXUS_Platform_P_ResetWakeupDevices(pSettings);
#endif
        errCode = NEXUS_Platform_P_Standby(pSettings);
        if (errCode) { errCode = BERR_TRACE(errCode);goto err; }
	break;

    case NEXUS_PlatformStandbyMode_eOn:
        break;

    case NEXUS_PlatformStandbyMode_eMax:
        BDBG_WRN(("Unknown Power State"));
        errCode = NEXUS_UNKNOWN ;
        break;
    }

#ifndef NEXUS_HAS_SAGE
set_wakeup:
#endif
#if !NEXUS_CPU_ARM
    if (pSettings->mode == NEXUS_PlatformStandbyMode_ePassive || pSettings->mode == NEXUS_PlatformStandbyMode_eDeepSleep) {
        NEXUS_Platform_P_SetWakeupDevices(pSettings);
    }
#endif
    g_standbyState.settings = *pSettings;

err:
    /* If we fail while transtioning from On -> Standby; try to recover back to On mode
       Standby -> On mode failure is fatal and we cannot recover at this point. */
    if (errCode != NEXUS_SUCCESS) {
        if (g_standbyState.settings.mode == NEXUS_PlatformStandbyMode_eOn) {
            NEXUS_Platform_P_Resume(&g_standbyState.settings);
        } else {
            BDBG_ERR(("*******************************************************************"));
            BDBG_ERR(("FATAL ERROR WHILE RESUMING! THE SYSTEM IS IN AN UNRECOVERABLE STATE"));
            BDBG_ERR(("*******************************************************************"));
        }
    }

    return errCode;

#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(resetWakeupStatus);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_Platform_SetStandbySettings_driver( const NEXUS_PlatformStandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT
    return NEXUS_Platform_P_SetStandbySettings(pSettings, true);
#else
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_Platform_GetStandbyStatus_driver(NEXUS_PlatformStandbyStatus *pStatus)
{
#if NEXUS_POWER_MANAGEMENT && !NEXUS_CPU_ARM
    NEXUS_Platform_P_GetStandbyStatus(pStatus);
#else
    BSTD_UNUSED(pStatus);
#endif

    return NEXUS_SUCCESS;
}

/* These apis have been deprecated but are maintained only for backward compatibility */

void NEXUS_Platform_GetDefaultStandbySettings( NEXUS_PlatformStandbySettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Platform_InitStandby( const NEXUS_PlatformStandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_PlatformSettings *pPlatformSettings;
    NEXUS_Error rc;

    /* Coverity: 35269 */
    pPlatformSettings = BKNI_Malloc(sizeof(NEXUS_PlatformSettings));
    if (!pPlatformSettings) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    NEXUS_Platform_GetDefaultSettings(pPlatformSettings);
    pPlatformSettings->standbySettings = *pSettings;
    rc = NEXUS_Platform_Init(pPlatformSettings);
    BKNI_Free(pPlatformSettings);
    if (rc) return BERR_TRACE(rc);
#else
    BSTD_UNUSED(pSettings);
#endif
    return 0;
}

void NEXUS_Platform_UninitStandby(void)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Platform_Uninit();
#endif
}

NEXUS_Error NEXUS_Platform_PreStandby(void)
{
    return 0;
}

NEXUS_Error NEXUS_Platform_PostStandby(void)
{
    return 0;
}

