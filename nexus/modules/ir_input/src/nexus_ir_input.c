/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_ir_input_module.h"
#include "bkir.h"
#include "priv/nexus_core.h"
#include "priv/nexus_ir_input_standby_priv.h"

BDBG_MODULE(nexus_ir_input);

NEXUS_ModuleHandle g_NEXUS_irInputModule;

#define BKIR_KirInterruptDevice_eMax (BKIR_KirInterruptDevice_eCir+1)

struct NEXUS_IrInput {
    NEXUS_OBJECT(NEXUS_IrInput);
    unsigned index;
    NEXUS_IrInputSettings settings;
    NEXUS_IsrCallbackHandle dataReady;
    uint32_t lastcode;
    NEXUS_Time lasttime;
    bool lasttime_set;
    /* data queued at isr time */
    NEXUS_IrInputEvent *queue;
    unsigned rptr, wptr;
    bool overflow;

    struct NEXUS_IrChannel *irChannel;
    BKIR_KirInterruptDevice interruptDevice; /* irChannel->irInput[interruptDevice] == this */
};

BDBG_OBJECT_ID(NEXUS_IrChannel);

struct NEXUS_IrChannel {
    BDBG_OBJECT(NEXUS_IrChannel)
    BKIR_ChannelHandle kirChannel;
    unsigned refcnt;
    NEXUS_IrInputHandle irInput[BKIR_KirInterruptDevice_eMax];
    NEXUS_IrInputDataFilter dataFilter;
    bool wakePatternEnabled;
};

static struct IrInputModule {
    NEXUS_IrInputModuleSettings settings;
    BKIR_Handle kir;
    struct NEXUS_IrChannel channel[BKIR_N_CHANNELS];
} g_NEXUS_irInput;

static BERR_Code NEXUS_IrInput_P_DataReady_isr(BKIR_ChannelHandle hChn, void *context);

/****************************************
* Module functions
***************/

void NEXUS_IrInputModule_GetDefaultSettings(NEXUS_IrInputModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

NEXUS_ModuleHandle NEXUS_IrInputModule_Init(const NEXUS_IrInputModuleSettings *pSettings)
{
    BKIR_Settings kirSettings;
    BERR_Code rc;
    unsigned ch;
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_IrInputModuleSettings defaultSettings;

    BDBG_ASSERT(!g_NEXUS_irInputModule);

    if (!pSettings) {
        NEXUS_IrInputModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    g_NEXUS_irInputModule = NEXUS_Module_Create("ir_input", &moduleSettings);
    if (!g_NEXUS_irInputModule) {
        return NULL;
    }

    NEXUS_LockModule();
    BKNI_Memset(&g_NEXUS_irInput, 0, sizeof(g_NEXUS_irInput));
    g_NEXUS_irInput.settings = *pSettings;

    BKIR_GetDefaultSettings(&kirSettings, g_pCoreHandles->chp);
    rc = BKIR_Open(&g_NEXUS_irInput.kir, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &kirSettings);
    if (rc) {rc=BERR_TRACE(rc);goto error;}

    BDBG_MSG(("creating %d KIR channels", BKIR_N_CHANNELS));
    for (ch=0;ch<BKIR_N_CHANNELS;ch++) {
        BKIR_ChannelSettings channelSettings;
        struct NEXUS_IrChannel *irChannel = &g_NEXUS_irInput.channel[ch];
        BDBG_OBJECT_SET(irChannel, NEXUS_IrChannel);

        BKIR_GetChannelDefaultSettings(g_NEXUS_irInput.kir, ch, &channelSettings);
        channelSettings.intMode = true; /* we don't really care about the event from KIR because there's no ISR-time buffering.
            We must still register with INT ourselves.  But, if you don't set this flag, interrupts are not enabled properly. */
        rc = BKIR_OpenChannel(g_NEXUS_irInput.kir, &irChannel->kirChannel, ch, &channelSettings);
        if (rc) {rc=BERR_TRACE(rc); goto error;}

        BDBG_MSG(("%p KIR Channel %u, IrChannel %p", (void *)irChannel->kirChannel, ch, (void *)irChannel));

        NEXUS_IrInput_GetDefaultDataFilter(&irChannel->dataFilter);
        irChannel->wakePatternEnabled = false;

        /*Disable Sejin device as it causes false interrupts */
        BKIR_DisableIrDevice(irChannel->kirChannel, (BKIR_KirDevice)NEXUS_IrInputMode_eSejin38KhzKbd);
    }

    NEXUS_UnlockModule();
    return g_NEXUS_irInputModule;

error:
    NEXUS_UnlockModule();
    NEXUS_IrInputModule_Uninit();
    return NULL;
}

void NEXUS_IrInputModule_Uninit(void)
{
    unsigned ch;
    NEXUS_LockModule();
    for (ch=0;ch<BKIR_N_CHANNELS;ch++) {
        struct NEXUS_IrChannel *irChannel = &g_NEXUS_irInput.channel[ch];
        unsigned i;

        BDBG_OBJECT_ASSERT(irChannel, NEXUS_IrChannel);
        for (i=0;i<BKIR_KirInterruptDevice_eMax;i++) {
            if (irChannel->irInput[i]) {
                NEXUS_IrInput_Close(irChannel->irInput[i]);
            }
        }
        BDBG_ASSERT(irChannel->refcnt == 0);
        if (irChannel->kirChannel) {
            BKIR_CloseChannel(irChannel->kirChannel);
        }
        BDBG_OBJECT_DESTROY(irChannel, NEXUS_IrChannel);
    }

    if (g_NEXUS_irInput.kir) {
        BKIR_Close(g_NEXUS_irInput.kir);
    }
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_irInputModule);
    g_NEXUS_irInputModule = NULL;
}

/****************************************
* API functions
***************/

/* get KIR isr from INT and dispatch data to appropriate NEXUS_IrInputHandle */
static BERR_Code NEXUS_IrInput_P_DataReady_isr(BKIR_ChannelHandle hChn, void *context)
{
    BERR_Code rc;
    NEXUS_IrInputEvent event;
    struct NEXUS_IrChannel *irChannel = context;
    BKIR_KirInterruptDevice interruptDevice;
    unsigned char kirCode[8];
    NEXUS_Time time;

    BDBG_OBJECT_ASSERT(irChannel, NEXUS_IrChannel);
    BDBG_ASSERT(irChannel->kirChannel == hChn);

    BKNI_Memset(kirCode, 0, sizeof(kirCode));
    rc = BKIR_Read_isr(hChn, &interruptDevice, kirCode);
    if (rc) {rc=BERR_TRACE(rc); return 0;}

    BKNI_Memset(&event, 0, sizeof(event));

    rc = BKIR_IsPreambleA_isrsafe(hChn, &event.preamble.preambleA);
    if (rc) {rc=BERR_TRACE(rc); return 0;}
    rc = BKIR_IsPreambleB_isrsafe(hChn, &event.preamble.preambleB);
    if (rc) {rc=BERR_TRACE(rc); return 0;}

    event.code = (uint32_t)kirCode[0] |
        ((uint32_t)kirCode[1] << 8) |
        ((uint32_t)kirCode[2] << 16) |
        ((uint32_t)kirCode[3] << 24);
    event.codeHigh = (uint32_t)kirCode[4] |
        ((uint32_t)kirCode[5] << 8);

    BDBG_MSG(("NEXUS_IrInput_P_DataReady_isr dev %d, code %#x %#x (%s%s)",
	interruptDevice, event.code, event.codeHigh, event.preamble.preambleA?"A":"", event.preamble.preambleB?"B":""));

    if (interruptDevice < BKIR_KirInterruptDevice_eMax && irChannel->irInput[interruptDevice]) {
        NEXUS_IrInputHandle irInput = irChannel->irInput[interruptDevice];
        unsigned time_diff;

        BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);

        NEXUS_Time_Get_isrsafe(&time);
        if (irInput->lasttime_set) {
            time_diff = NEXUS_Time_Diff_isrsafe(&time, &irInput->lasttime);
        }
        else {
            time_diff = 0;
        }
        event.interval = time_diff;

        switch (irInput->settings.mode) {
        default:
            /* If the repeatFilter is NOT set, then use the hardware repeat */
            if (irInput->settings.repeatFilterTime == 0) {
                /* TODO: should be BKIR_IsRepeated_isr. */
                rc = BKIR_IsRepeated_isrsafe(hChn, &event.repeat);
                if (rc) {rc=BERR_TRACE(rc);}
                break;
            }
            /* if repeatFilterTime is set, drop through and handle in software */
        case NEXUS_IrInputMode_eRemoteB:
        case NEXUS_IrInputMode_eTwirpKbd: /* we know these devices do not support hardware repeats */
        case NEXUS_IrInputMode_eSejin38KhzKbd:
        case NEXUS_IrInputMode_eSejin56KhzKbd:
        case NEXUS_IrInputMode_eCirXmp:
        case NEXUS_IrInputMode_eCirRcmmRcu:
            if (irInput->settings.repeatFilterTime == 0)  /* maintain compatibility with older application code */
                irInput->settings.repeatFilterTime = 150; /* that did not over-ride the default timeout */
            /* no hardware to detect this, so measure time */
            event.repeat = (irInput->lastcode == event.code) && time_diff < irInput->settings.repeatFilterTime;
            break;
        }
        irInput->lasttime = time;
        irInput->lasttime_set = true;
        irInput->lastcode = event.code;

        /* add to queue and fire event */
        irInput->queue[irInput->wptr++] = event;
        if (irInput->wptr == irInput->settings.eventQueueSize)
            irInput->wptr = 0;
        BDBG_MSG(("add rptr=%d, wptr=%d, code=%x repeat=%c, interval=%d [queue size %d]", irInput->rptr, irInput->wptr,
            event.code, event.repeat?'y':'n', event.interval, irInput->settings.eventQueueSize));
        if (irInput->wptr == irInput->rptr)
        {
            BDBG_WRN(("Overflow"));
            irInput->overflow = true;
        }
        NEXUS_IsrCallback_Fire_isr(irInput->dataReady);
    }

    return 0;
}

void NEXUS_IrInput_GetDefaultSettings(NEXUS_IrInputSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->repeatFilterTime = 0; /* assume they want to use hardware repeat mode */
    pSettings->eventQueueSize = 20;
    pSettings->mode = NEXUS_IrInputMode_eRemoteA;
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);
}

/*
 * pSettings->channel_number corresponds to the KIR Channel to be Opened
*/
NEXUS_IrInputHandle NEXUS_IrInput_Open(unsigned index, const NEXUS_IrInputSettings *pSettings)
{
    NEXUS_IrInputHandle irInput = NULL;
    BERR_Code rc;
    NEXUS_IrInputSettings defaultSettings;

    if (!pSettings) {
        NEXUS_IrInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    irInput = BKNI_Malloc(sizeof(*irInput));
    if (!irInput) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_IrInput, irInput);
    irInput->index = index;

    /* allow dataReady callback to be invoke from dataready isr */
    irInput->dataReady = NEXUS_IsrCallback_Create(irInput, NULL);
    if (!irInput->dataReady) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}

    rc = NEXUS_IrInput_SetSettings(irInput, pSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }

    return irInput;

error:
    if (irInput) {
        NEXUS_IrInput_Close(irInput);
    }
    return NULL;
}

/* NEXUS_IrInput_P_Disconnect can be called from SetSettings or Close */
static void NEXUS_IrInput_P_Disconnect(NEXUS_IrInputHandle irInput)
{
    irInput->irChannel->irInput[irInput->interruptDevice] = NULL;
    irInput->interruptDevice = BKIR_KirInterruptDevice_eNone;
    BKIR_DisableIrDevice(irInput->irChannel->kirChannel, irInput->settings.mode);

    BDBG_ASSERT(irInput->irChannel->refcnt);
    if (--irInput->irChannel->refcnt == 0) {
        BKIR_UnregisterCallback(irInput->irChannel->kirChannel);
    }
    irInput->irChannel = NULL;
}

static void NEXUS_IrInput_P_Finalizer(NEXUS_IrInputHandle irInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_IrInput, irInput);
    if (irInput->irChannel) {
        NEXUS_IrInput_P_Disconnect(irInput);
    }
    if (irInput->queue) {
        BKNI_Free(irInput->queue);
    }
    if (irInput->dataReady) {
        NEXUS_IsrCallback_Destroy(irInput->dataReady);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_IrInput, irInput);
    BKNI_Free(irInput);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_IrInput, NEXUS_IrInput_Close);

void NEXUS_IrInput_GetSettings( NEXUS_IrInputHandle irInput, NEXUS_IrInputSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);
    *pSettings = irInput->settings;
}

NEXUS_Error NEXUS_IrInput_SetSettings( NEXUS_IrInputHandle irInput, const NEXUS_IrInputSettings *pSettings )
{
    struct NEXUS_IrChannel *new_channel;
    BERR_Code rc;
    BKIR_KirInterruptDevice interruptDevice;

    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);

    /* param validation */
    if (pSettings->channel_number >= BKIR_N_CHANNELS) {
        BDBG_ERR(("Invalid channel_number %d, num_channels=%d", pSettings->channel_number, BKIR_N_CHANNELS));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if ( !pSettings->eventQueueSize ) {
        BDBG_ERR(("Invalid queue size %d", pSettings->eventQueueSize));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (pSettings->eventQueueSize != irInput->settings.eventQueueSize) {
        if (irInput->queue) {
            BKNI_Free(irInput->queue);
        }
        irInput->queue = BKNI_Malloc(pSettings->eventQueueSize*sizeof(NEXUS_IrInputEvent));
        if (!irInput->queue) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
    }
    BDBG_ASSERT(irInput->queue);

    switch (pSettings->mode) {
    case NEXUS_IrInputMode_eTwirpKbd:
        interruptDevice = BKIR_KirInterruptDevice_eTwirpKbd; break;
    case NEXUS_IrInputMode_eSejin38KhzKbd:
    case NEXUS_IrInputMode_eSejin56KhzKbd:
        interruptDevice = BKIR_KirInterruptDevice_eSejinKbd; break;
    case NEXUS_IrInputMode_eRemoteA:
        interruptDevice = BKIR_KirInterruptDevice_eRemoteA; break;
    case NEXUS_IrInputMode_eRemoteB:
        interruptDevice = BKIR_KirInterruptDevice_eRemoteB; break;
    default:
        interruptDevice = BKIR_KirInterruptDevice_eCir; break;
    }

    /* only one IrInput can use an interrupt device on the same channel. verify this before disconnecting. */
    new_channel = &g_NEXUS_irInput.channel[pSettings->channel_number];
    BDBG_OBJECT_ASSERT(new_channel, NEXUS_IrChannel);
    if (new_channel->irInput[interruptDevice] && new_channel->irInput[interruptDevice] != irInput) {
        BDBG_ERR(("cannot use mode %d on channel_number %d because of interrupt device %d conflict",
            pSettings->mode, pSettings->channel_number, interruptDevice));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* now that we've validated the new state, we can proceed with the disconnect/connect without risk of failure */

    /* disconnect from channel if changing channel_number or interruptDevice */
    if (irInput->irChannel && (pSettings->channel_number != irInput->settings.channel_number || interruptDevice != irInput->interruptDevice)) {
        NEXUS_IrInput_P_Disconnect(irInput);
    }

    /* connect to a channel */
    if (!irInput->irChannel) {
        irInput->irChannel = new_channel;
        irInput->interruptDevice = interruptDevice;
        irInput->irChannel->irInput[irInput->interruptDevice] = irInput;
        if (irInput->irChannel->refcnt++ == 0) {
            BKIR_RegisterCallback(irInput->irChannel->kirChannel, NEXUS_IrInput_P_DataReady_isr, irInput->irChannel);
        }
        BDBG_MSG(("%p: kirChannel=%p irChannel=%p", (void *)irInput, (void *)irInput->irChannel->kirChannel, (void *)irInput->irChannel));
    }
    BDBG_OBJECT_ASSERT(irInput->irChannel, NEXUS_IrChannel);

    /* compile-time assert that the Nexus and Magnum enums are in sync. If they are not, please sync them up or create a conversion function. */
    BDBG_CASSERT(BKIR_KirDevice_eNumKirDevice == (BKIR_KirDevice)NEXUS_IrInputMode_eMax);

    if (pSettings->useCustomSettings) {
        /* User is only allowed to use 1 Custom CIR on a platform, no matter how many HW IR Receivers there are */
        CIR_Param cirParam;

        /* compile-time assert that the Nexus and Magnum structs are at least equal in size */
        BDBG_CASSERT(sizeof(pSettings->customSettings) == sizeof(cirParam));

        /* set the custom device type that the custom settings will apply to */
        BKIR_SetCustomDeviceType(irInput->irChannel->kirChannel, (BKIR_KirDevice)pSettings->mode);

        BKNI_Memcpy(&cirParam, &pSettings->customSettings, sizeof(NEXUS_IrInputCustomSettings));
        BKIR_SetCustomCir(irInput->irChannel->kirChannel, &cirParam);

        /* enable device with custom settings */
        rc = BKIR_EnableIrDevice(irInput->irChannel->kirChannel, BKIR_KirDevice_eCirCustom);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        rc = BKIR_EnableIrDevice(irInput->irChannel->kirChannel, (BKIR_KirDevice)pSettings->mode);
        if (rc) return BERR_TRACE(rc);
    }

    if (pSettings->pulseWidthFilter != irInput->settings.pulseWidthFilter) {
        if (pSettings->pulseWidthFilter) {
            /* converse of (128*filter_width+2)/27) */
            unsigned filter_width = (pSettings->pulseWidthFilter*27-2)/128;
            /* print programmed value to reduce confusion between nexus units and magnum/HW units */
            BDBG_LOG(("%p: pulseWidthFilter of %d microseconds => filter_width %d", (void*)irInput, pSettings->pulseWidthFilter, filter_width));
            rc = BKIR_EnableFilter1(irInput->irChannel->kirChannel, filter_width);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            (void)BKIR_DisableFilter1(irInput->irChannel->kirChannel);
        }
    }

    NEXUS_IsrCallback_Set(irInput->dataReady, &pSettings->dataReady);

    irInput->settings = *pSettings;

    return 0;
}

NEXUS_Error NEXUS_IrInput_GetEvents(NEXUS_IrInputHandle irInput, NEXUS_IrInputEvent *pEvents, size_t numEvents, size_t *pNumEventsRead, bool *pOverflow)
{
    size_t numEventsRead=0;

    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);
    if (numEvents && !pEvents) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    *pOverflow = irInput->overflow;
    irInput->overflow = false;
    /* no critical section needed for simple producer/consumer algo */
    while (numEvents && irInput->rptr != irInput->wptr) {
        pEvents[numEventsRead] = irInput->queue[irInput->rptr];
        if (++irInput->rptr == irInput->settings.eventQueueSize) {
            irInput->rptr = 0;
        }
        numEventsRead++;
        numEvents--;
    }
    *pNumEventsRead = numEventsRead;
    BDBG_MSG(("GetEvents returning %u events", (unsigned)numEventsRead));
    return 0;
}

void NEXUS_IrInput_FlushEvents(NEXUS_IrInputHandle irInput)
{
    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);
    irInput->rptr = irInput->wptr;
}

void NEXUS_IrInput_GetDefaultCustomSettings(NEXUS_IrInputCustomSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_IrInput_GetCustomSettingsForMode(NEXUS_IrInputMode mode, NEXUS_IrInputCustomSettings *pSettings)
{
    BERR_Code rc;
    CIR_Param cirParam;

    rc = BKIR_GetDefaultCirParam((BKIR_KirDevice)mode, &cirParam);
    if (rc==BERR_SUCCESS) {
        BKNI_Memcpy(pSettings, &cirParam, sizeof(NEXUS_IrInputCustomSettings));
        return NEXUS_SUCCESS;
    }

    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

NEXUS_Error NEXUS_IrInput_GetPreambleStatus(NEXUS_IrInputHandle irInput, NEXUS_IrInputPreambleStatus *pStatus)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);

    rc = BKIR_IsPreambleA_isrsafe(irInput->irChannel->kirChannel, &pStatus->preambleA);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BKIR_IsPreambleB_isrsafe(irInput->irChannel->kirChannel, &pStatus->preambleB);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;

error:
    return NEXUS_UNKNOWN;
}

NEXUS_Error NEXUS_IrInput_ReadEvent(NEXUS_IrInputHandle irInput, NEXUS_IrInputEvent *pEvent)
{
    uint32_t code, codeHigh;
    bool preambleA, preambleB;
    BDBG_OBJECT_ASSERT(irInput, NEXUS_IrInput);
    BKIR_GetLastKey(irInput->irChannel->kirChannel, &code, &codeHigh, &preambleA, &preambleB);
    BKNI_Memset(pEvent, 0, sizeof(*pEvent));
    pEvent->code = code;
    pEvent->codeHigh = codeHigh;
    pEvent->preamble.preambleA = preambleA;
    pEvent->preamble.preambleB = preambleB;
    return NEXUS_SUCCESS;
}

void NEXUS_IrInput_GetDefaultDataFilter( NEXUS_IrInputDataFilter *pPattern )
{
    BKNI_Memset(pPattern, 0, sizeof(*pPattern));
    pPattern->patternWord0 = ~0;
    pPattern->patternWord1 = ~0;
    pPattern->mask0 = ~0;
    pPattern->mask1 = ~0;
}

NEXUS_Error NEXUS_IrInput_EnableDataFilter(NEXUS_IrInputHandle handle, const NEXUS_IrInputDataFilter *pPattern)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_IrInput);

    rc = BKIR_EnableDataFilter(handle->irChannel->kirChannel,
            pPattern->filterWord[0].enabled ? pPattern->filterWord[0].patternWord : ((uint64_t)pPattern->patternWord1 << 32) | pPattern->patternWord0,
            pPattern->filterWord[1].enabled ? pPattern->filterWord[1].patternWord : NEXUS_IR_INPUT_FILTER_DISABLED,
            pPattern->filterWord[0].enabled ? pPattern->filterWord[0].mask : ((uint64_t)pPattern->mask1 << 32) | pPattern->mask0,
            pPattern->filterWord[1].enabled ? pPattern->filterWord[1].mask : NEXUS_IR_INPUT_FILTER_DISABLED);

    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

void NEXUS_IrInput_DisableDataFilter(NEXUS_IrInputHandle handle)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_IrInput);

    rc = BKIR_DisableDataFilter(handle->irChannel->kirChannel);
    if (rc) BERR_TRACE(rc);
}

NEXUS_Error NEXUS_IrInput_SetDataFilter(NEXUS_IrInputHandle handle, const NEXUS_IrInputDataFilter *pPattern)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_IrInput);

    if(!pPattern)
        NEXUS_IrInput_GetDefaultDataFilter(&handle->irChannel->dataFilter);
    else
        BKNI_Memcpy(&handle->irChannel->dataFilter, pPattern, sizeof(*pPattern));

    return NEXUS_SUCCESS;
}

#if NEXUS_INSERT_IR_INPUT
NEXUS_Error NEXUS_IrInput_P_InsertEvents( unsigned index, const NEXUS_IrInputEvent *pEvents, size_t numEvents )
{
    NEXUS_IrInputHandle irInput;
    unsigned i,ch;
    bool foundIrHandle=false;
    size_t numEventsQd=0;

    if ( !g_NEXUS_irInputModule ) {
         BDBG_ERR(("g_NEXUS_irInputModule is NULL!"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (numEvents && !pEvents) {
         BDBG_ERR(("pEvents is NULL!"));
         return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* find IrInput Handle corresponding to Index passed in */
    for (ch=0;ch<BKIR_N_CHANNELS;ch++) {
         struct NEXUS_IrChannel *irChannel = &g_NEXUS_irInput.channel[ch];
         BDBG_OBJECT_ASSERT(irChannel, NEXUS_IrChannel);

         for (i=0;i<BKIR_KirInterruptDevice_eMax;i++) {
               if ( irChannel->irInput[i] ) {
                     /* if NEXUS_ANY_ID, we get the first valid IR input handle */
                     if ( index == NEXUS_ANY_ID || irChannel->irInput[i]->index == index) {
                          irInput = irChannel->irInput[i];
                          foundIrHandle = true;
                          break;
                     }
               }
         }

        if ( foundIrHandle ) break;
    }

    if ( !foundIrHandle )
    {
        BDBG_ERR(("Could not find IrInput Handle corresponding to index %d!", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("NEXUS_IrInput_InsertEvents: found IrHandle 0x%08lx at channel %d and interrupt device %d",
        irInput, ch, irInput->interruptDevice ));

    /* place events on queue */
    BKNI_EnterCriticalSection();
    while (numEvents) {
        irInput->queue[irInput->wptr++] = pEvents[numEventsQd];
        if (irInput->wptr == irInput->settings.eventQueueSize)
             irInput->wptr = 0;

        BDBG_MSG(("NEXUS_IrInput_InsertEvents: add rptr=%d, wptr=%d, code=%x, [queue size %d]", irInput->rptr, irInput->wptr,
            pEvents[numEventsQd].code, irInput->settings.eventQueueSize));

        if (irInput->wptr == irInput->rptr) {
             BDBG_WRN(("Overflow"));
             irInput->overflow = true;
        }

        numEventsQd++;
        numEvents--;
    }

    /* fire isr callback */
    NEXUS_IsrCallback_Fire_isr(irInput->dataReady);
    BKNI_LeaveCriticalSection();

    return NEXUS_SUCCESS;
}
#endif

NEXUS_Error NEXUS_IrInputModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned ch;
    BERR_Code rc;

    for (ch=0;ch<BKIR_N_CHANNELS;ch++) {
        if(!g_NEXUS_irInput.channel[ch].kirChannel)
            continue;

        if(enabled) {
            if(pSettings->wakeupSettings.ir) {
                NEXUS_IrInputDataFilter *pPattern = &g_NEXUS_irInput.channel[ch].dataFilter;
                if(pPattern->filterWord[0].enabled || pPattern->filterWord[1].enabled) {
                    rc = BKIR_EnableDataFilter(g_NEXUS_irInput.channel[ch].kirChannel,
                            pPattern->filterWord[0].enabled ? pPattern->filterWord[0].patternWord : ((uint64_t)pPattern->patternWord1 << 32) | pPattern->patternWord0,
                            pPattern->filterWord[1].enabled ? pPattern->filterWord[1].patternWord : NEXUS_IR_INPUT_FILTER_DISABLED,
                            pPattern->filterWord[0].enabled ? pPattern->filterWord[0].mask : ((uint64_t)pPattern->mask1 << 32) | pPattern->mask0,
                            pPattern->filterWord[1].enabled ? pPattern->filterWord[1].mask : NEXUS_IR_INPUT_FILTER_DISABLED);
                    g_NEXUS_irInput.channel[ch].wakePatternEnabled = true;
                }
            }
        } else {
            if(g_NEXUS_irInput.channel[ch].wakePatternEnabled) {
                rc = BKIR_DisableDataFilter(g_NEXUS_irInput.channel[ch].kirChannel);
                g_NEXUS_irInput.channel[ch].wakePatternEnabled = false;
            }
        }
    }
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return NEXUS_SUCCESS;
}
