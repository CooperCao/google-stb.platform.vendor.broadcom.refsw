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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_uhf_input_module.h"
#include "buhf.h"
#include "priv/nexus_core.h"
/*#include "blst_list.h"*/

#if (BCHP_CHIP == 7425) || (BCHP_CHIP == 7422) || (BCHP_CHIP==7344) || (BCHP_CHIP==7346) || (BCHP_CHIP == 7435) || (BCHP_CHIP==73465)
#include "bchp_int_id_uhfr_intr2.h"
#include "bchp_uhfr_intr2.h"
#else
#include "bchp_int_id_uhfr_1.h"
#endif
#if (BCHP_CHIP == 7400)
#include "bchp_uhfr_2.h"
#include "bchp_int_id_uhfr_2.h"
#endif
#if (BCHP_CHIP == 7403)
#include "bchp_clk.h"
#endif

BDBG_MODULE(nexus_uhf_input);

NEXUS_ModuleHandle g_NEXUS_uhfInputModule;

#if NEXUS_NUM_UHF_INPUTS

/* This is the handle being passed back to the callers */
struct NEXUS_UhfInput {
    NEXUS_OBJECT(NEXUS_UhfInput);
    BUHF_Handle uhf;
    NEXUS_UhfInputSettings settings;
    NEXUS_IsrCallbackHandle dataReady;
    NEXUS_Time lasttime;
    uint32_t   lastcode;

    /* data queued at isr time */
    NEXUS_UhfInputEvent *queue;
    unsigned rptr, wptr;
    bool overflow;
};

struct UhfInputModule {
    NEXUS_UhfInputModuleSettings mod_settings;
    BLST_D_HEAD(channellist, NEXUS_UhfInput) list;

    /* These are used for S3 standby/resume operations */
    NEXUS_UhfInputHandle uhfInput;
    BUHF_Settings uhfSettings;
    bool s3standby;
} g_NEXUS_uhfInput;

static void NEXUS_UhfInput_P_DataReady_isr(BUHF_Handle uhf, void *context);

/****************************************
* Module functions
***************/

void NEXUS_UhfInputModule_GetDefaultSettings(NEXUS_UhfInputModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

NEXUS_ModuleHandle NEXUS_UhfInputModule_Init(const NEXUS_UhfInputModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_UhfInputModuleSettings defaultSettings;

    BDBG_ASSERT(!g_NEXUS_uhfInputModule);
    
    if (!pSettings) {
        NEXUS_UhfInputModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    g_NEXUS_uhfInputModule = NEXUS_Module_Create("uhf_input", &moduleSettings);

    BKNI_Memset(&g_NEXUS_uhfInput, 0, sizeof(g_NEXUS_uhfInput));
    g_NEXUS_uhfInput.mod_settings = *pSettings;

    BLST_D_INIT(&g_NEXUS_uhfInput.list);

    return g_NEXUS_uhfInputModule;
}

void NEXUS_UhfInputModule_Uninit()
{
    NEXUS_Module_Destroy(g_NEXUS_uhfInputModule);
    g_NEXUS_uhfInputModule = NULL;
}

NEXUS_Error NEXUS_UhfInputModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BUHF_StandbySettings standbySettings;
    bool wakeup = pSettings->wakeupSettings.uhf;
    BERR_Code rc;        
    
    /* If module or device isn't open then nothing to do here. */
    if (!g_NEXUS_uhfInput.uhfInput || !g_NEXUS_uhfInput.uhfInput->uhf)
        return 0;
    
    /* In S3 the UHF module loses power and all of its registers settings.  This is not preserved
    ** by the module.  So for S3 modes we need to close and re-open the device (using the original
    ** settings) instead of just using standby and resume interfaces.
    */
    if (enabled) {
        if (pSettings->mode!=NEXUS_StandbyMode_eDeepSleep) { /* not S3 */
            BUHF_GetDefaultStandbySettings(g_NEXUS_uhfInput.uhfInput->uhf, &standbySettings);
            standbySettings.bEnableWakeup = wakeup;
            rc = BUHF_Standby(g_NEXUS_uhfInput.uhfInput->uhf, &standbySettings);
            if (rc) { return BERR_TRACE(rc); }
        } 
        else {
            BUHF_UnregisterCallback(g_NEXUS_uhfInput.uhfInput->uhf);
            BUHF_Close(g_NEXUS_uhfInput.uhfInput->uhf);
            /* note: don't destroy the handle here or we won't be able to re-enter on resume */
            g_NEXUS_uhfInput.s3standby = true;
        }
    }
    else {
        if (!g_NEXUS_uhfInput.s3standby) { /* not S3 */
            rc = BUHF_Resume(g_NEXUS_uhfInput.uhfInput->uhf);
            if (rc) { return BERR_TRACE(rc); }
        }
        else {
            g_NEXUS_uhfInput.uhfInput->uhf = NULL; /* zero this in case this doesn't happen if open fails */
            rc = BUHF_Open(&g_NEXUS_uhfInput.uhfInput->uhf, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, 0, &g_NEXUS_uhfInput.uhfSettings);
            if (rc) { rc=BERR_TRACE(rc); return rc; }
            BUHF_RegisterCallback(g_NEXUS_uhfInput.uhfInput->uhf, (BUHF_Callback)NEXUS_UhfInput_P_DataReady_isr, g_NEXUS_uhfInput.uhfInput);
            g_NEXUS_uhfInput.s3standby = false;
        }
    }

#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
  
  return NEXUS_SUCCESS;
}

/****************************************
* API functions
***************/

/* get KIR isr from INT and dispatch data to appropriate NEXUS_IrInputHandle */
static void NEXUS_UhfInput_P_DataReady_isr(BUHF_Handle uhf, void *context)
{
    /*NEXUS_UhfInputHandle uhfInput = g_NEXUS_uhfInput.uhfInput; */
    BERR_Code rc;
    NEXUS_UhfInputEvent event;
    NEXUS_UhfInputHandle uhfInput = (NEXUS_UhfInputHandle)context;
    BUHF_Data data;
    NEXUS_Time time;
    unsigned time_diff;

    BDBG_MSG(("Data received at UHFR: %s (context=%x)", __FUNCTION__, (unsigned)context));

    BKNI_Memset(&event, 0, sizeof(event));

    rc = BUHF_Read(uhf, &data);
    if (rc != BERR_SUCCESS) return;

    if (uhfInput->settings.channel == NEXUS_UhfInputMode_eChannel9) {
        event.code = data.value;
        event.header = 0x10;
        BDBG_MSG(("Data received at UHFR: 0x10%08x Cust2 type Packet", data.value));
    } else {
        if (data.prType == BUHF_PrType_e1)
            event.code = data.value >> 6;
        else if (data.prType == BUHF_PrType_e2)
            event.code = data.value >> 27;
        BDBG_MSG(("Data received at UHFR: 0x%08x, Preamble type %d (orig data=0x%08x)", event.code, data.prType, data.value));
    }

    NEXUS_Time_Get(&time);
    time_diff = NEXUS_Time_Diff_isrsafe(&time, &uhfInput->lasttime);
    event.repeat = (uhfInput->lastcode == event.code) && time_diff < uhfInput->settings.repeatFilterTime;

    uhfInput->lasttime = time;
    uhfInput->lastcode = event.code;

    if (event.repeat) event.code |= 0x80000000;

    /* add to queue and fire event */
    uhfInput->queue[uhfInput->wptr++] = event;
    if (uhfInput->wptr == uhfInput->settings.eventQueueSize)
        uhfInput->wptr = 0;
    BDBG_MSG(("add rptr=%d, wptr=%d, code=%x repeat=%c, interval=%d [queue size %d]",
        uhfInput->rptr, uhfInput->wptr, event.code, event.repeat?'y':'n', time_diff, uhfInput->settings.eventQueueSize));
    if (uhfInput->wptr == uhfInput->rptr) {
        BDBG_WRN(("Overflow"));
        uhfInput->overflow = true;
    }
    NEXUS_IsrCallback_Fire_isr(uhfInput->dataReady);
}

void NEXUS_UhfInput_GetDefaultSettings(NEXUS_UhfInputSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->repeatFilterTime = 150;
    pSettings->eventQueueSize = 10;
    pSettings->channel = NEXUS_UhfInputMode_eChannel1;
}

NEXUS_UhfInputHandle NEXUS_UhfInput_Open(unsigned index, const NEXUS_UhfInputSettings *pSettings)
{
    NEXUS_UhfInputHandle uhfInput;
    BUHF_Settings uhfSettings;
    BERR_Code rc = BERR_SUCCESS;

    NEXUS_UhfInputSettings defaultSettings;

    if (!pSettings) {
        NEXUS_UhfInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* TODO: Add support for second receiver. add it in standby as well */
    if (index > 0) {
        BDBG_WRN(("Only the first receiver is currently supported"));
        return NULL;
    }

    uhfInput = BKNI_Malloc(sizeof(*uhfInput));
    if (!uhfInput) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_UhfInput, uhfInput);
    uhfInput->queue = BKNI_Malloc(pSettings->eventQueueSize*sizeof(NEXUS_UhfInputEvent));
    if (!uhfInput->queue) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    uhfInput->settings = *pSettings;

     /* You Must Open UHF Remote PI because Settings cannot be changed once its open */
    BUHF_GetDefaultSettings(&uhfSettings);
    uhfSettings.channel = pSettings->channel;
    uhfSettings.mode = BUHF_Mode_eAdvanced; /* Set in UHF Advanced mode. TODO: IR Mode */
    rc = BUHF_Open(&uhfInput->uhf, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, 0, &uhfSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    /* We need these for stand-by/resume so we can close and reopen device */
    g_NEXUS_uhfInput.uhfInput = uhfInput;
    g_NEXUS_uhfInput.uhfSettings = uhfSettings;

    /* allow dataReady callback to be invoke from dataready isr */
    uhfInput->dataReady = NEXUS_IsrCallback_Create(uhfInput, NULL);
    if(!uhfInput->dataReady) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    NEXUS_IsrCallback_Set(uhfInput->dataReady, &pSettings->dataReady);

    BUHF_RegisterCallback(uhfInput->uhf, (BUHF_Callback)NEXUS_UhfInput_P_DataReady_isr, uhfInput);

    return uhfInput;
error:
    NEXUS_UhfInput_Close(uhfInput);
    return NULL;
}

static void NEXUS_UhfInput_P_Finalizer(NEXUS_UhfInputHandle uhfInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_UhfInput, uhfInput);

    BUHF_UnregisterCallback(uhfInput->uhf);

    if (uhfInput->queue) {
        BKNI_Free(uhfInput->queue);
        uhfInput->queue = NULL;
    }
    if (uhfInput->dataReady) {
        NEXUS_IsrCallback_Destroy(uhfInput->dataReady);
        uhfInput->dataReady = NULL;
    }

    /* Close UHF Handle */
    if (uhfInput->uhf) {
        BUHF_Close(uhfInput->uhf);
        uhfInput->uhf = NULL;
    }

    NEXUS_OBJECT_DESTROY(NEXUS_UhfInput, uhfInput);
    BKNI_Free(uhfInput);
    g_NEXUS_uhfInput.uhfInput = NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_UhfInput, NEXUS_UhfInput_Close);

NEXUS_Error NEXUS_UhfInput_GetEvents(NEXUS_UhfInputHandle uhfInput, NEXUS_UhfInputEvent *pEvents, size_t numEvents, size_t *pNumEventsRead, bool *pOverflow)
{
    size_t numEventsRead=0;

    if (!pEvents || !pNumEventsRead || !pOverflow) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    *pOverflow = uhfInput->overflow;
    uhfInput->overflow = false;
    /* no critical section needed for simple producer/consumer algo */
    while (numEvents && uhfInput->rptr != uhfInput->wptr) {
        pEvents[numEventsRead] = uhfInput->queue[uhfInput->rptr];
        if (++uhfInput->rptr == uhfInput->settings.eventQueueSize) {
            uhfInput->rptr = 0;
        }
        numEventsRead++;
        numEvents--;
    }
    *pNumEventsRead = numEventsRead;
    BDBG_MSG(("GetEvents returning %d events", numEventsRead));
    return 0;
}

void NEXUS_UhfInput_FlushEvents(NEXUS_UhfInputHandle uhfInput)
{
    BDBG_OBJECT_ASSERT(uhfInput, NEXUS_UhfInput);
    uhfInput->rptr = uhfInput->wptr;
}

#else

void NEXUS_UhfInput_GetDefaultSettings(
    NEXUS_UhfInputSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Uhf Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;

}

NEXUS_UhfInputHandle NEXUS_UhfInput_Open(  /* attr{destructor=NEXUS_UhfInput_Close}  */
    unsigned index,
    const NEXUS_UhfInputSettings *pSettings  /* May be passed as NULL for defaults */
    )
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(index);
    BDBG_WRN(("Uhf Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_UhfInput_Close(
    NEXUS_UhfInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("Uhf Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}


NEXUS_Error NEXUS_UhfInput_GetEvents(
    NEXUS_UhfInputHandle handle,
    NEXUS_UhfInputEvent *pEvents,   /* Pointer to an array of events */
    size_t numEvents,               /* Size of the event array */
    size_t *pNumEventsRead,         /* [out] Number of events actually read */
    bool *pOverflow                 /* [out] Has an overflow occurred? */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pEvents);
    BSTD_UNUSED(numEvents);
    BSTD_UNUSED(pNumEventsRead);
    BSTD_UNUSED(pOverflow);
    BDBG_WRN(("Uhf Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}


void NEXUS_UhfInput_FlushEvents(
    NEXUS_UhfInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("Uhf Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

#endif /* NEXUS_NUM_UHF_INPUTS */
