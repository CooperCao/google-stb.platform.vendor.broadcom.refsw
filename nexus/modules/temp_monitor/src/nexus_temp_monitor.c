/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * This module interfaces with tmn PI module and the application.
 * It provides to the application an average temperature at a programmed
 * interval and kind of fan operation that need to be done through PWM
 * nexus module.Based on the Fan Operation returned, the application can call
 * Nexus PWM to adjust the speed the of the fan to control the temperature
 * of an System On Chip.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "nexus_temp_monitor_module.h"
#include "priv/nexus_core.h"
#include "bint.h"
#include "btmn.h"
#include "bchp_tmon.h"
#if BCHP_CHIP==7420
#include "bchp_upg1_intr2.h"
#else
/* 40nm silicon */
#include "bchp_upg_aux_intr2.h"
#endif


#ifndef NEXUS_NUM_TEMP_MONITORS
#define NEXUS_NUM_TEMP_MONITORS 2
#endif

BDBG_MODULE(nexus_temp_monitor);

NEXUS_ModuleHandle g_NEXUS_tempMonitorModule;

struct {
    NEXUS_TempMonitorModuleSettings settings;
    NEXUS_TempMonitorHandle tempMonitor[NEXUS_NUM_TEMP_MONITORS];
    BTMN_Handle hTmn;
} g_NEXUS_tempMonitor;

struct NEXUS_TempMonitor {
    NEXUS_OBJECT(NEXUS_TempMonitor);
    unsigned index;
    NEXUS_TempMonitorSettings settings;
    NEXUS_TempMonitorStatus status;
    NEXUS_IsrCallbackHandle dataReady;
};

/*************************
* Private API functions
**************************/

static void NEXUS_TempMonitor_P_DataReady_isr(void *context, int param)
{
    NEXUS_TempMonitorHandle tempMonitor;
    uint32_t tmon_top, tmon_bottom, tmon_mid1, tmon_mid0;
    int i;
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    BTMN_GetVoltageValues_isr(g_NEXUS_tempMonitor.hTmn, &tmon_top, &tmon_bottom, &tmon_mid1, &tmon_mid0);
    BDBG_MSG(("NEXUS_TempMonitor_P_DataReady %d %d %d %d", tmon_top, tmon_bottom, tmon_mid1, tmon_mid0));
    for (i = 0 ; i < NEXUS_NUM_TEMP_MONITORS; i++) {
        tempMonitor = g_NEXUS_tempMonitor.tempMonitor[i];
        if (tempMonitor) {
            tempMonitor->status.valid = true;
            tempMonitor->status.top = tmon_top;
            tempMonitor->status.bottom = tmon_bottom;
            tempMonitor->status.middle = (i == 0) ? tmon_mid0 : tmon_mid1;
            NEXUS_IsrCallback_Fire_isr(tempMonitor->dataReady);
        }
    }
    return;
}

static void NEXUS_TempMonitor_P_OverTemp_isr(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    /* TODO */
    return;
}

static NEXUS_Error NEXUS_TempMonitor_P_SetInterval(void)
{
    int i;
    bool enabled = false;
    unsigned min_interval = 0x7f;
    NEXUS_Error rc;

    for (i = 0; i < NEXUS_NUM_TEMP_MONITORS; i++) {
        NEXUS_TempMonitorHandle tempMonitor = g_NEXUS_tempMonitor.tempMonitor[i];
        if (tempMonitor && tempMonitor->settings.enabled) {
            unsigned interval;
            enabled = true;
            interval = tempMonitor->settings.measureInterval;
            
            /* bound it */
            if (interval < 1000) {
                interval = 1000; /* Minimum 1 second polling value */
            } 
            else if (interval > 10000) {
                interval = 10000; /* Maximum 10 second polling value */
            } 
            
            interval /= 100; /* BTMN value is in steps of 100 msec. */
            if (interval < min_interval) {
                /* Program BTMN module with smallest interval setting */
                min_interval = interval;
            }
        }
    }

    if (enabled) {
        uint8_t interval;
        if (BTMN_GetMeasurementInterval(g_NEXUS_tempMonitor.hTmn, &interval) == BERR_SUCCESS) {
            if (interval != min_interval) {
                rc = BTMN_SetMeasurementInterval (g_NEXUS_tempMonitor.hTmn, min_interval);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }

    BDBG_MSG(("tmn enabled=%d, interval=%d", enabled, min_interval));
    rc = BTMN_Enable(g_NEXUS_tempMonitor.hTmn, enabled);
    if (rc) return BERR_TRACE(rc);
    
    /* TODO: when to invalidate status? */
    
    return 0;
}


/******************
* Module functions
*******************/

void NEXUS_TempMonitorModule_GetDefaultSettings(NEXUS_TempMonitorModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    return;
 }

static void NEXUS_TempMonitor_P_Close(void);

static NEXUS_Error NEXUS_TempMonitor_P_Open(void)
{
    BTMN_Settings tmnSettings;
    NEXUS_Error rc;
    
    if (g_NEXUS_tempMonitor.hTmn) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    BTMN_GetDefaultSettings(&tmnSettings, g_pCoreHandles->chp);
    rc = BTMN_Open(&g_NEXUS_tempMonitor.hTmn, g_pCoreHandles->chp,g_pCoreHandles->reg,g_pCoreHandles->bint, &tmnSettings);
    if (rc) return BERR_TRACE(rc);
    
    BTMN_InstallDataReadyInterruptCallback(g_NEXUS_tempMonitor.hTmn,(BTMN_CallbackFunc) NEXUS_TempMonitor_P_DataReady_isr, (void *) NULL, 0);
    BTMN_InstallOverTempInterruptCallback(g_NEXUS_tempMonitor.hTmn,(BTMN_CallbackFunc) NEXUS_TempMonitor_P_OverTemp_isr, (void *) NULL, 0);
    
    rc = NEXUS_TempMonitor_P_SetInterval();
    if (rc) {
        NEXUS_TempMonitor_P_Close();
        return BERR_TRACE(rc);
    }

    return 0;
}

static void NEXUS_TempMonitor_P_Close(void)
{
    if (g_NEXUS_tempMonitor.hTmn) {
        BTMN_UnInstallDataReadyInterruptCallback(g_NEXUS_tempMonitor.hTmn);
        BTMN_UnInstallOverTempInterruptCallback(g_NEXUS_tempMonitor.hTmn);
        BTMN_Close(g_NEXUS_tempMonitor.hTmn);
        g_NEXUS_tempMonitor.hTmn = NULL;
    }
}

NEXUS_ModuleHandle NEXUS_TempMonitorModule_Init(const NEXUS_TempMonitorModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_Error rc;
    BDBG_ASSERT(!g_NEXUS_tempMonitorModule);

    BKNI_Memset(&g_NEXUS_tempMonitor, 0, sizeof(g_NEXUS_tempMonitor));
    if (pSettings){
        g_NEXUS_tempMonitor.settings = *pSettings;
    }
    else {
        NEXUS_TempMonitorModule_GetDefaultSettings(&g_NEXUS_tempMonitor.settings);
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(moduleSettings.priority, &g_NEXUS_tempMonitor.settings.common);
    g_NEXUS_tempMonitorModule = NEXUS_Module_Create("tempmonitor", &moduleSettings);
    if (g_NEXUS_tempMonitorModule == NULL) {
        goto error;
    }

    rc = NEXUS_TempMonitor_P_Open();
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return g_NEXUS_tempMonitorModule;

error:
    if (g_NEXUS_tempMonitorModule) {
        NEXUS_Module_Destroy(g_NEXUS_tempMonitorModule);
        g_NEXUS_tempMonitorModule = NULL;
    }
    return NULL;
}

void NEXUS_TempMonitorModule_Uninit(void)
{
    NEXUS_TempMonitor_P_Close();
    NEXUS_Module_Destroy(g_NEXUS_tempMonitorModule);
    g_NEXUS_tempMonitorModule = NULL;
    return;
}


/**************
* API functions
***************/

void NEXUS_TempMonitor_GetDefaultSettings(NEXUS_TempMonitorSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->enabled = true;
    pSettings->measureInterval = 1000; /* 1 second */
}

NEXUS_TempMonitorHandle NEXUS_TempMonitor_Open(unsigned index, const NEXUS_TempMonitorSettings *pSettings)
{
    NEXUS_TempMonitorHandle tempMonitor;
    BERR_Code rc;
    NEXUS_TempMonitorSettings defaultSettings;

    BDBG_MSG((">NEXUS_TempMonitor_Open"));

    if (index >= NEXUS_NUM_TEMP_MONITORS) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("temp_monitor[%d] not available", index));
        return NULL;
    }

    if (g_NEXUS_tempMonitor.tempMonitor[index]) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("temp_monitor[%d] already open", index));
        return NULL;
    }

    if (!pSettings) {
        NEXUS_TempMonitor_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    tempMonitor = BKNI_Malloc(sizeof(*tempMonitor));
    if (!tempMonitor) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_TempMonitor, tempMonitor);
    tempMonitor->index = index;

    /* allow dataReady callback to be invoke from dataready isr */
    tempMonitor->dataReady = NEXUS_IsrCallback_Create(tempMonitor, NULL);
    if (!tempMonitor->dataReady) {goto error;}

    /* store settings and handle before SetSettings(), to allow P_SetInterval() to go through */
    tempMonitor->settings = *pSettings;
    BKNI_EnterCriticalSection();
    g_NEXUS_tempMonitor.tempMonitor[index] = tempMonitor;
    BKNI_LeaveCriticalSection();

    rc = NEXUS_TempMonitor_SetSettings(tempMonitor, pSettings);
    if (rc) {rc = BERR_TRACE(rc);goto error;}

    BDBG_MSG(("<NEXUS_TempMonitor_Open"));
    return tempMonitor;
error:
    if (tempMonitor){
       NEXUS_TempMonitor_Close(tempMonitor);
    }

    return NULL;
}

static void NEXUS_TempMonitor_P_Finalizer(NEXUS_TempMonitorHandle tempMonitor)
{
    NEXUS_OBJECT_ASSERT(NEXUS_TempMonitor, tempMonitor);

    BKNI_EnterCriticalSection();
    g_NEXUS_tempMonitor.tempMonitor[tempMonitor->index] = NULL;
    BKNI_LeaveCriticalSection();
    
    if (tempMonitor->dataReady) {
        NEXUS_IsrCallback_Destroy(tempMonitor->dataReady);
    }

    tempMonitor->settings.enabled = false;
    /* this will disable if it's the last one */
    (void)NEXUS_TempMonitor_P_SetInterval();

    NEXUS_OBJECT_DESTROY(NEXUS_TempMonitor, tempMonitor);
    BKNI_Free(tempMonitor);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_TempMonitor,NEXUS_TempMonitor_Close);

NEXUS_Error NEXUS_TempMonitor_GetStatus(NEXUS_TempMonitorHandle tempMonitor, NEXUS_TempMonitorStatus *pStatus)
{
    int onChipTemperature;
    NEXUS_Error rc;
    NEXUS_OBJECT_ASSERT(NEXUS_TempMonitor, tempMonitor);
    
    BKNI_EnterCriticalSection();
    *pStatus = tempMonitor->status;
    BKNI_LeaveCriticalSection();
    
#if BTMN_SUPPORTS_ONCHIPTEMP
/* see btmn.h for BCHP_CHIP for which chips support this feature */
    rc = BTMN_GetOnChipTemp(g_NEXUS_tempMonitor.hTmn, &onChipTemperature);
    if (rc) return BERR_TRACE(rc);
    pStatus->onChipTemperature = onChipTemperature;
#else
    BSTD_UNUSED(onChipTemperature);
    BSTD_UNUSED(rc);
#endif
    
    return 0;
}

void NEXUS_TempMonitor_GetSettings(NEXUS_TempMonitorHandle tempMonitor, NEXUS_TempMonitorSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_TempMonitor, tempMonitor);
    *pSettings = tempMonitor->settings;
}

NEXUS_Error NEXUS_TempMonitor_SetSettings(NEXUS_TempMonitorHandle tempMonitor,const NEXUS_TempMonitorSettings *pSettings)
{
    NEXUS_Error rc;
    
    NEXUS_OBJECT_ASSERT(NEXUS_TempMonitor, tempMonitor);
    BDBG_MSG((">NEXUS_TempMonitor_SetSettings"));
    
    rc = NEXUS_TempMonitor_P_SetInterval();
    if (rc) return BERR_TRACE(rc);

    tempMonitor->settings = *pSettings;

    NEXUS_IsrCallback_Set(tempMonitor->dataReady, &pSettings->dataReady);

    BDBG_MSG(("<NEXUS_TempMonitor_SetSettings"));
    return rc;
}

NEXUS_Error NEXUS_TempMonitorModule_Standby(bool enabled)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Error rc;
    if (enabled) {
        NEXUS_TempMonitor_P_Close();
    } else {
        rc = NEXUS_TempMonitor_P_Open();
        if (rc) return BERR_TRACE(rc);
    }

    return 0;
#else
    BSTD_UNUSED(enabled);
    return NEXUS_SUCCESS;
#endif
}
