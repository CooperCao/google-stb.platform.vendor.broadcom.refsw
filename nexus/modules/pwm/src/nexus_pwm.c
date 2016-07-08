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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bpwm.h"
#include "bkni_multi.h"

#include "nexus_pwm_module.h"
#include "priv/nexus_pwm_standby_priv.h"
#include "nexus_platform_features.h"

#include "priv/nexus_core.h"

BDBG_MODULE(nexus_pwm);

struct NEXUS_PwmChannel
{
    NEXUS_OBJECT(NEXUS_PwmChannel);
    BPWM_ChannelHandle             hPwmChn;
    NEXUS_PwmChannelSettings    stSettings;
    struct {
        uint16_t controlWord;
        NEXUS_PwmFreqModeType frequencyMode;
        uint16_t onInterval;
        uint16_t periodInterval;
    } standby;
};

struct NEXUS_Pwm_P_Context
{
    BPWM_Handle         hPwm;
    struct NEXUS_PwmChannel hChannel[MAX_PWM_CHANNELS];
    bool s3standby[MAX_PWM_CHANNELS];
};

static struct NEXUS_Pwm_P_Context g_PwmData;
NEXUS_ModuleHandle g_NEXUS_pwmModule;

/***************************************************************************

Pwm Module functions

***************************************************************************/
void NEXUS_PwmModule_GetDefaultSettings(NEXUS_PwmModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
}

NEXUS_ModuleHandle NEXUS_PwmModule_Init(const NEXUS_PwmModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BERR_Code rc;
    BPWM_Settings pwmSettings;

    BDBG_ASSERT(!g_NEXUS_pwmModule);
    BDBG_ASSERT(pSettings);


    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(moduleSettings.priority, &pSettings->common);
    g_NEXUS_pwmModule = NEXUS_Module_Create("pwm", &moduleSettings);
    if (!g_NEXUS_pwmModule) {
        return NULL;
    }

    /* bring up PWM PI */
    BKNI_Memset(&g_PwmData,0,sizeof(g_PwmData));

    BPWM_GetDefaultSettings(&pwmSettings, g_pCoreHandles->chp);
    pwmSettings.hInterrupt = g_pCoreHandles->bint;
    rc=BPWM_Open(&g_PwmData.hPwm, (BCHP_Handle)g_pCoreHandles->chp,g_pCoreHandles->reg, &pwmSettings);
    if (rc) { BERR_TRACE(NEXUS_UNKNOWN); return NULL;}

    return g_NEXUS_pwmModule;
}

void NEXUS_PwmModule_Uninit(void)
{
    if (g_PwmData.hPwm != NULL)
    {
        BERR_Code rc;
        rc=BPWM_Close(g_PwmData.hPwm);
        if (rc) { BERR_TRACE(NEXUS_UNKNOWN);}
        g_PwmData.hPwm = NULL;
    }
    NEXUS_Module_Destroy(g_NEXUS_pwmModule);
    g_NEXUS_pwmModule = NULL;
}


NEXUS_PwmChannelHandle NEXUS_Pwm_OpenChannel(unsigned index, const NEXUS_PwmChannelSettings *pSettings)
{
     BERR_Code rc;
     NEXUS_PwmChannelSettings defaultSettings;
     BPWM_ChannelSettings pwmChannelSettings;
     NEXUS_PwmChannelHandle pwm;

     if (!g_PwmData.hPwm) {
         BDBG_ERR(("PWM module not initialized"));
         return NULL;
    }
    if (index >= MAX_PWM_CHANNELS) {
         BERR_TRACE(NEXUS_INVALID_PARAMETER);
         return NULL;
    }

    /* Init PWM_Channel setting */
    if (!pSettings) {
        NEXUS_Pwm_GetDefaultChannelSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    pwm = &g_PwmData.hChannel[index];
    if (pwm->hPwmChn) {
         BDBG_ERR(("PWM channel %d already opened", index));
         return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_PwmChannel,pwm);
    pwm->stSettings = *pSettings;

    /* Open PWM_Channel context */
    rc = BPWM_GetChannelDefaultSettings(g_PwmData.hPwm, index, &pwmChannelSettings);
    if (rc) {BERR_TRACE(NEXUS_UNKNOWN); return NULL;}

    pwmChannelSettings.FreqMode = pSettings->eFreqMode;
    pwmChannelSettings.openDrainb = pSettings->openDrain;

    rc = BPWM_OpenChannel(g_PwmData.hPwm, &pwm->hPwmChn, index, &pwmChannelSettings);
    if (rc) {BERR_TRACE(NEXUS_UNKNOWN); return NULL;}

    return pwm;
}

static void NEXUS_PwmChannel_P_Finalizer(NEXUS_PwmChannelHandle hChn)
{
    BERR_Code rc;

    NEXUS_OBJECT_ASSERT(NEXUS_PwmChannel, hChn);

    rc = BPWM_CloseChannel(hChn->hPwmChn);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    NEXUS_OBJECT_DESTROY(NEXUS_PwmChannel,hChn);
    hChn->hPwmChn = NULL; /* must NULL out this for re-open test */
    return;
}


NEXUS_OBJECT_CLASS_MAKE(NEXUS_PwmChannel, NEXUS_Pwm_CloseChannel);

void NEXUS_Pwm_GetDefaultChannelSettings(NEXUS_PwmChannelSettings *pSettings)
{
    BPWM_ChannelSettings ChannelSettings;

    (void)BPWM_GetChannelDefaultSettings(g_PwmData.hPwm, 0, &ChannelSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->openDrain=ChannelSettings.openDrainb;
    pSettings->eFreqMode=ChannelSettings.FreqMode;
}

NEXUS_Error NEXUS_Pwm_SetControlWord(NEXUS_PwmChannelHandle hChn,
    uint16_t cWord)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_SetControlWord(hChn->hPwmChn,cWord);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_GetControlWord(NEXUS_PwmChannelHandle hChn,
    uint16_t *cWord)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_GetControlWord(hChn->hPwmChn,cWord);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_SetFreqMode(NEXUS_PwmChannelHandle hChn,
    NEXUS_PwmFreqModeType frequencyMode)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_SetFreqMode(hChn->hPwmChn,frequencyMode);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_GetFreqMode(NEXUS_PwmChannelHandle hChn,
    NEXUS_PwmFreqModeType *pFrequencyMode)
{
    BERR_Code rc = BERR_SUCCESS;
    BPWM_FreqModeType cfg_frequencyMode;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_GetFreqMode(hChn->hPwmChn,&cfg_frequencyMode);
    if (rc) return BERR_TRACE(rc);

    *pFrequencyMode=cfg_frequencyMode;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_SetOnInterval(NEXUS_PwmChannelHandle hChn, uint16_t OnInterval)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc = BPWM_SetOnInterval(hChn->hPwmChn, OnInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_RampOnInterval( NEXUS_PwmChannelHandle hChn, uint16_t onInterval )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc = BPWM_RampOnInterval(hChn->hPwmChn, onInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_GetOnInterval(NEXUS_PwmChannelHandle hChn,
    uint16_t *OnInterval)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_GetOnInterval(hChn->hPwmChn,OnInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_SetPeriodInterval(NEXUS_PwmChannelHandle hChn,
    uint16_t PeriodInterval)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_SetPeriodInterval(hChn->hPwmChn,PeriodInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_GetPeriodInterval(NEXUS_PwmChannelHandle hChn,
    uint16_t *pPeriodInterval)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_GetPeriodInterval(hChn->hPwmChn,pPeriodInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_SetOnAndPeriodInterval(NEXUS_PwmChannelHandle hChn,
    uint16_t OnInterval,
    uint16_t PeriodInterval)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_SetOnAndPeriodInterval(hChn->hPwmChn,OnInterval,PeriodInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_GetOnAndPeriodInterval(NEXUS_PwmChannelHandle hChn,
    uint16_t *OnInterval,
    uint16_t *PeriodInterval)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_GetOnAndPeriodInterval(hChn->hPwmChn,OnInterval,PeriodInterval);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_Start(NEXUS_PwmChannelHandle hChn)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_Start(hChn->hPwmChn);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Pwm_Stop(NEXUS_PwmChannelHandle hChn)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, NEXUS_PwmChannel);

    rc=BPWM_Stop(hChn->hPwmChn);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PwmModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned i;
    NEXUS_Error rc;

    if (!g_PwmData.hPwm) { /* module not initialized */
        return NEXUS_SUCCESS;
    }

    /* pwm has no (known) clocks to shutdown. we only need to save and restore state for S3 */
    for (i=0; i<MAX_PWM_CHANNELS; i++) {
        NEXUS_PwmChannelHandle p = &g_PwmData.hChannel[i];
        if (!p->hPwmChn) { continue; } /* channel not opened by app */

        if (enabled) {
            BPWM_ChannelHandle pwm = p->hPwmChn;
            BPWM_Stop(pwm); /* always force a stop. app must re-start, if needed */

            if (pSettings->mode==NEXUS_StandbyMode_eDeepSleep) { /* S3 */
                /* save channel settings before closing */
                BPWM_GetControlWord(pwm, &p->standby.controlWord);
                BPWM_GetFreqMode(pwm, (BPWM_FreqModeType*)&p->standby.frequencyMode);
                BPWM_GetOnInterval(pwm, &p->standby.onInterval);
                BPWM_GetPeriodInterval(pwm, &p->standby.periodInterval);
                BPWM_CloseChannel(pwm);
                g_PwmData.s3standby[i] = true;
            }
        }
        else {
            if (g_PwmData.s3standby[i]) { /* S3 */
                BPWM_ChannelHandle pwm = NULL;
                BPWM_ChannelSettings pwmChannelSettings;
                rc = BPWM_GetChannelDefaultSettings(g_PwmData.hPwm, i, &pwmChannelSettings);
                if (rc) {return BERR_TRACE(NEXUS_UNKNOWN);}
                pwmChannelSettings.FreqMode = p->stSettings.eFreqMode;
                pwmChannelSettings.openDrainb = p->stSettings.openDrain;

                rc = BPWM_OpenChannel(g_PwmData.hPwm, &p->hPwmChn, i, &pwmChannelSettings);
                if (rc) {return BERR_TRACE(NEXUS_UNKNOWN);}

                /* restore channel settings after opening */
                pwm = p->hPwmChn;
                rc = BPWM_SetControlWord(pwm, p->standby.controlWord);
                if (rc) {rc = BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
                rc = BPWM_SetFreqMode(pwm, p->standby.frequencyMode);
                if (rc) {rc = BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
                rc = BPWM_SetOnInterval(pwm, p->standby.onInterval);
                if (rc) {rc = BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
                rc = BPWM_SetPeriodInterval(pwm, p->standby.periodInterval);
                if (rc) {rc = BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
            }
            g_PwmData.s3standby[i] = false;
        }
    }
    
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(enabled);
    return NEXUS_SUCCESS;
#endif

}

