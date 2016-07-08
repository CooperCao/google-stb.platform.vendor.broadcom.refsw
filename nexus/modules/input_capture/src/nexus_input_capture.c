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
#include "nexus_input_capture_module.h"
#include "priv/nexus_core.h"
#include "bicp.h"
#if BBCP_SUPPORT
#include "bbcp.h"
#endif

BDBG_MODULE(nexus_input_capture);

struct NEXUS_InputCapture {
    NEXUS_OBJECT(NEXUS_InputCapture);
    BICP_ChannelHandle icpHandle;
#if BBCP_SUPPORT
    BBCP_ChannelHandle bcpHandle;
#endif
    NEXUS_InputCaptureType type;
    unsigned channelNumber;
    unsigned edgeCount;

    NEXUS_InputCaptureSettings settings;
    NEXUS_EventCallbackHandle eventHandler;
    NEXUS_TaskCallbackHandle triggerCallback;
};

struct NEXUS_InputCaptureTimeout {
    NEXUS_OBJECT(NEXUS_InputCaptureTimeout);
    NEXUS_InputCaptureTimeoutSettings settings;
    unsigned timeoutIndex;
};

#define NUM_BICAP_CHANNELS 2
#define NUM_FIFO_ELEMENTS 128

#ifndef NEXUS_INPUT_CAPTURE_DO_NOT_ALLOW_OVERFLOW
#define NEXUS_INPUT_CAPTURE_DO_NOT_ALLOW_OVERFLOW 1
#endif

typedef struct NEXUS_BicapSwFifo {
    uint32_t wptr;
    uint32_t rptr;
    uint32_t data[NUM_FIFO_ELEMENTS];
} NEXUS_BicapSwFifo;
static NEXUS_BicapSwFifo bicapSwFifo[NUM_BICAP_CHANNELS];

NEXUS_ModuleHandle g_NEXUS_inputCaptureModule;
struct {
    NEXUS_InputCaptureModuleSettings settings;
    BICP_Handle icp;
#if BBCP_SUPPORT
    BBCP_Handle bcp;
#endif
    NEXUS_InputCaptureHandle lastOpened; /* see TODO later */
    NEXUS_InputCaptureHandle opened[NUM_BICAP_CHANNELS];
    NEXUS_EventCallbackHandle eventHandler;
} g_NEXUS_inputCapture;

typedef enum NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect
{
    NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect_eBicapClkForInput0,
    NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect_eBicapClkForInput1,
    NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect_eMax
} NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect;

typedef enum NEXUS_InputCaptureFifoInactivityTimeoutEventSelect
{
    NEXUS_InputCaptureFifoInactivityTimeoutEventSelect_eInOrOut,
    NEXUS_InputCaptureFifoInactivityTimeoutEventSelect_eIn,
    NEXUS_InputCaptureFifoInactivityTimeoutEventSelect_eOut,
    NEXUS_InputCaptureFifoInactivityTimeoutEventSelect_eMax
} NEXUS_InputCaptureFifoInactivityTimeoutEventSelect;

typedef enum NEXUS_InputCaptureFifoInactivityTimeoutClockSelect
{
    NEXUS_InputCaptureFifoInactivityTimeoutClockSelect_eBicapClk,
    NEXUS_InputCaptureFifoInactivityTimeoutClockSelect_eToutClk,
    NEXUS_InputCaptureFifoInactivityTimeoutClockSelect_eMax
} NEXUS_InputCaptureFifoInactivityTimeoutClockSelect;

typedef struct NEXUS_InputCaptureFifoInactivityTimeoutSettings
{
    NEXUS_InputCaptureFifoInactivityTimeoutBicapClockSelect bicapClockSelect;
    NEXUS_InputCaptureFifoInactivityTimeoutEventSelect      eventSelect;
    NEXUS_InputCaptureFifoInactivityTimeoutClockSelect      clockSelect;
    unsigned                                                timeout;
} NEXUS_InputCaptureFifoInactivityTimeoutSettings;

#if BBCP_SUPPORT
static uint32_t NEXUS_InputCapture_P_AddToSwFifo(uint32_t channel, uint32_t data);
static void NEXUS_InputCapture_P_Fifo_Event(void *context);
#endif
static void NEXUS_InputCapture_P_CreateSwFifo(void );
static void NEXUS_InputCapture_P_DestroySwFifo(void );

/****************************************
* Module functions
***************/

void NEXUS_InputCaptureModule_GetDefaultSettings(NEXUS_InputCaptureModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

NEXUS_ModuleHandle NEXUS_InputCaptureModule_Init(const NEXUS_InputCaptureModuleSettings *pSettings)
{
    BERR_Code rc;
#if BBCP_SUPPORT
    BKNI_EventHandle event;
    BBCP_Settings defSettings;
#endif
    NEXUS_ModuleSettings moduleSettings;
    int i;

    BDBG_ASSERT(!g_NEXUS_inputCaptureModule);

    if (pSettings) {
        g_NEXUS_inputCapture.settings = *pSettings;
    }
    else {
        NEXUS_InputCaptureModule_GetDefaultSettings(&g_NEXUS_inputCapture.settings);
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &g_NEXUS_inputCapture.settings.common);
    g_NEXUS_inputCaptureModule = NEXUS_Module_Create("input_capture", &moduleSettings);
    if (!g_NEXUS_inputCaptureModule) {
        return NULL;
    }

	NEXUS_LockModule();
    rc = BICP_Open(&g_NEXUS_inputCapture.icp, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, NULL);
    if (rc) {
        NEXUS_InputCaptureModule_Uninit();
        rc=BERR_TRACE(NEXUS_UNKNOWN);
		NEXUS_UnlockModule();
        return NULL;
    }

    for (i=0; i<NUM_BICAP_CHANNELS; i++) {
        g_NEXUS_inputCapture.opened[i]=0;
    }

#if BBCP_SUPPORT
    rc = BBCP_GetDefaultSettings( &defSettings );
    rc = BBCP_Open(&g_NEXUS_inputCapture.bcp, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &defSettings);
    if (rc) {
        BICP_Close(g_NEXUS_inputCapture.icp);
        NEXUS_InputCaptureModule_Uninit();
        rc=BERR_TRACE(NEXUS_UNKNOWN);
		NEXUS_UnlockModule();
        return NULL;
    }
#endif

	NEXUS_InputCapture_P_CreateSwFifo();

#if BBCP_SUPPORT
    /* TODO: the ICP/BCP event requires two task switches to get the callback to the app. if this is a problem,
    get an isr callback added to ICP/BCP, or tap into INT basemodule. */
    rc = BBCP_GetEventHandle(g_NEXUS_inputCapture.bcp, &event);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    g_NEXUS_inputCapture.eventHandler = NEXUS_RegisterEvent(event, NEXUS_InputCapture_P_Fifo_Event, &g_NEXUS_inputCapture/*NULL*/ /*inputCapture*/);
#endif

	NEXUS_UnlockModule();
    return g_NEXUS_inputCaptureModule;
#if BBCP_SUPPORT
error:
    BBCP_Close(g_NEXUS_inputCapture.bcp);
    BICP_Close(g_NEXUS_inputCapture.icp);
	NEXUS_UnlockModule();
    return NULL;
#endif
}

void NEXUS_InputCaptureModule_Uninit()
{
	NEXUS_LockModule();
    NEXUS_InputCapture_P_DestroySwFifo();

#if BBCP_SUPPORT
    if (g_NEXUS_inputCapture.eventHandler) {
        NEXUS_UnregisterEvent(g_NEXUS_inputCapture.eventHandler);
    }
#endif

    if (g_NEXUS_inputCapture.icp) {
        BICP_Close(g_NEXUS_inputCapture.icp);
    }

#if BBCP_SUPPORT
    if (g_NEXUS_inputCapture.bcp) {
        BBCP_Close(g_NEXUS_inputCapture.bcp);
    }
#endif
	NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_inputCaptureModule);
    g_NEXUS_inputCaptureModule = NULL;
}

/****************************************
* API functions
***************/

void NEXUS_InputCapture_GetDefaultSettings(NEXUS_InputCaptureSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bicap.systemClockDivisor = 0xf;
    pSettings->bicap.timeoutClockDivisor = 0;
    pSettings->bicap.countMode = NEXUS_InputCaptureCountMode_eSaturate;
    pSettings->bicap.invertSignalInput = false;
    pSettings->edgeSelect = NEXUS_InputCaptureEdgeSelect_eBoth;
    pSettings->filter.enable = false;
    pSettings->filter.clockSelect = NEXUS_InputCaptureFilterClockSelect_eBicapClock;
    pSettings->filter.value = 0;
}

static void NEXUS_InputCapture_P_Event(void *context)
{
    NEXUS_InputCaptureHandle inputCapture = context;
    NEXUS_TaskCallback_Fire(inputCapture->triggerCallback);
}

#if BBCP_SUPPORT
static void NEXUS_InputCapture_P_Fifo_Event(void *context)
{
    uint32_t count, dummy, data, data_for_ch[NUM_BICAP_CHANNELS], ch=0;
    BSTD_UNUSED(context);

    for (ch=0; ch<NUM_BICAP_CHANNELS; ch++) {
        data_for_ch[ch]=0;
    }

    BBCP_GetOneFifoData (g_NEXUS_inputCapture.bcp, &count, &data);
    if (count)
    {
        while (count)
        {
/*            BDBG_MSG(("NEXUS_InputCapture_P_Fifo_Event: count=%d, data=%04xh", count, data)); */

            /* determine which channel */
            ch = ((1<<13) & data) ? 1 : 0;

            if (g_NEXUS_inputCapture.opened[ch])
            {
                if ((g_NEXUS_inputCapture.opened[ch]->settings.bicap.countMode==NEXUS_InputCaptureCountMode_eNoMaxCount) ||
                   ((data & 0xfff) <= 0xff8))
                {
                    g_NEXUS_inputCapture.opened[ch]->edgeCount++;
                    NEXUS_InputCapture_P_AddToSwFifo(ch, data);
                    data_for_ch[ch] = 1;
                }
                else
                    BDBG_WRN(("one of the timeout codes ...0x%x", data));
            }
            else
                BDBG_ERR(("NEXUS_InputCaptureHandle null for ch %d", ch));

            BBCP_GetOneFifoData (g_NEXUS_inputCapture.bcp, &dummy, &data);
            count--;
        }
        for (ch=0; ch<NUM_BICAP_CHANNELS; ch++) {
            if (data_for_ch[ch])
                NEXUS_TaskCallback_Fire(g_NEXUS_inputCapture.opened[ch]->triggerCallback);
        }
    }
    else
    {
        BDBG_WRN(("interrupt received, but count=0"));
    }
}
#endif

NEXUS_InputCaptureHandle NEXUS_InputCapture_Open(unsigned index, const NEXUS_InputCaptureSettings *pSettings)
{
    NEXUS_InputCaptureHandle inputCapture;
    BICP_ChannelSettings icpSettings;
    BERR_Code rc;
    NEXUS_InputCaptureSettings defaultSettings;
    BKNI_EventHandle event;

    /* Let the PI check index */
    if (!pSettings) {
        NEXUS_InputCapture_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    inputCapture = BKNI_Malloc(sizeof(*inputCapture));
    if (!inputCapture) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_InputCapture, inputCapture);
    inputCapture->settings = *pSettings;
    inputCapture->edgeCount = 0;

    if (inputCapture->settings.type == NEXUS_InputCaptureType_eIcap)
    {
        /* TODO: the ICP event requires two task switches to get the callback to the app. if this is a problem,
        get an isr callback added to ICP, or tap into INT basemodule. */
        inputCapture->triggerCallback = NEXUS_TaskCallback_Create(inputCapture, NULL);
        NEXUS_TaskCallback_Set(inputCapture->triggerCallback, &pSettings->dataReady);

        BICP_GetChannelDefaultSettings(g_NEXUS_inputCapture.icp, index, &icpSettings);
        icpSettings.rejectCnt = pSettings->filter.value;

        rc = BICP_OpenChannel(g_NEXUS_inputCapture.icp, &inputCapture->icpHandle, index, &icpSettings);
        if (rc) {rc=BERR_TRACE(rc); goto error;}

        rc = BICP_GetEventHandle(inputCapture->icpHandle, &event);
        if (rc) {rc=BERR_TRACE(rc); goto error;}
        inputCapture->eventHandler = NEXUS_RegisterEvent(event, NEXUS_InputCapture_P_Event, inputCapture);

        switch (pSettings->edgeSelect) {
            case NEXUS_InputCaptureEdgeSelect_eNegative:
                rc = BICP_EnableEdge(inputCapture->icpHandle, BICP_EdgeConfig_eNegative);
                if (rc) {rc = BERR_TRACE(rc); goto error;}
                break;
            case NEXUS_InputCaptureEdgeSelect_ePositive:
                rc = BICP_EnableEdge(inputCapture->icpHandle, BICP_EdgeConfig_ePositive);
                if (rc) {rc = BERR_TRACE(rc); goto error;}
                break;
            case NEXUS_InputCaptureEdgeSelect_eBoth:
                rc = BICP_EnableEdge(inputCapture->icpHandle, BICP_EdgeConfig_eBoth);
                if (rc) {rc = BERR_TRACE(rc); goto error;}
                break;
            default:
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto error;
        }

        g_NEXUS_inputCapture.lastOpened = inputCapture;
    }
    else
    {
#if BBCP_SUPPORT
        BBCP_ChannelSettings bcpSettings;
        inputCapture->channelNumber = index;

        inputCapture->triggerCallback = NEXUS_TaskCallback_Create(inputCapture, NULL);
        NEXUS_TaskCallback_Set(inputCapture->triggerCallback, &pSettings->dataReady);

        BBCP_GetChannelDefaultSettings(g_NEXUS_inputCapture.bcp, index, &bcpSettings);
        bcpSettings.edge.tout_clk_div = pSettings->bicap.timeoutClockDivisor;
        bcpSettings.edge.sys_clk_div = pSettings->bicap.systemClockDivisor;
        bcpSettings.edge.cnt_mode = pSettings->bicap.countMode;
        switch (pSettings->edgeSelect)
        {
            case NEXUS_InputCaptureEdgeSelect_eNegative:
                bcpSettings.edge.pedgedet_en = 0;
                bcpSettings.edge.nedgedet_en = 1;
                break;
            case NEXUS_InputCaptureEdgeSelect_ePositive:
                bcpSettings.edge.pedgedet_en = 1;
                bcpSettings.edge.nedgedet_en = 0;
                break;
            case NEXUS_InputCaptureEdgeSelect_eBoth:
                bcpSettings.edge.pedgedet_en = 1;
                bcpSettings.edge.nedgedet_en = 1;
                break;
            default:
                break;
        }
        bcpSettings.edge.insig_inv = pSettings->bicap.invertSignalInput;
        bcpSettings.edge.bicap_en = 0; /* Disable by default */
        bcpSettings.filter.bypass = !pSettings->filter.enable;
        switch (pSettings->filter.clockSelect)
        {
            case NEXUS_InputCaptureFilterClockSelect_eBicapClock:
                bcpSettings.filter.clk_sel = 0;
                break;

            case NEXUS_InputCaptureFilterClockSelect_eSystemClock:
                bcpSettings.filter.clk_sel = 1;
                break;

            default:
                break;
        }
        bcpSettings.filter.val = pSettings->filter.value;

        rc = BBCP_OpenChannel(g_NEXUS_inputCapture.bcp, &inputCapture->bcpHandle, index, &bcpSettings);
        if (rc) {rc=BERR_TRACE(rc); goto error;}

        g_NEXUS_inputCapture.opened[index] = inputCapture;
        g_NEXUS_inputCapture.lastOpened = inputCapture;
#else
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        BKNI_Free(inputCapture);
        inputCapture = NULL;
#endif
    }
    return inputCapture;
error:
    NEXUS_InputCapture_Close(inputCapture);
    return NULL;
}

static void NEXUS_InputCapture_P_Finalizer(NEXUS_InputCaptureHandle inputCapture)
{
    NEXUS_OBJECT_ASSERT(NEXUS_InputCapture, inputCapture);
#if BBCP_SUPPORT
    if (inputCapture->settings.type == NEXUS_InputCaptureType_eBicap)
    {
        if (inputCapture == g_NEXUS_inputCapture.lastOpened) {
            g_NEXUS_inputCapture.lastOpened = NULL;
        }

        g_NEXUS_inputCapture.opened[inputCapture->channelNumber] = 0;

        if (inputCapture->bcpHandle) {
            BBCP_CloseChannel(inputCapture->bcpHandle);
        }

        NEXUS_TaskCallback_Destroy(inputCapture->triggerCallback);
    }
    else
#endif
    {
        if (inputCapture->eventHandler) {
            NEXUS_UnregisterEvent(inputCapture->eventHandler);
        }
        if (inputCapture->icpHandle) {
            BICP_CloseChannel(inputCapture->icpHandle);
        }

        NEXUS_TaskCallback_Destroy(inputCapture->triggerCallback);

        if (inputCapture == g_NEXUS_inputCapture.lastOpened) {
            g_NEXUS_inputCapture.lastOpened = NULL;
        }
    }

    NEXUS_OBJECT_DESTROY(NEXUS_InputCapture, inputCapture);
    BKNI_Free(inputCapture);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_InputCapture, NEXUS_InputCapture_Close);

/****************************************
* Module functions
***************/

static void NEXUS_InputCapture_P_CreateSwFifo(void )
{
    int i;
    for (i=0; i<NUM_BICAP_CHANNELS; i++)
    {
        BKNI_Memset(bicapSwFifo[i].data, 0, sizeof(uint32_t)*NUM_FIFO_ELEMENTS);
        bicapSwFifo[i].wptr = 0;
        bicapSwFifo[i].rptr = 0;
    }
}

static void NEXUS_InputCapture_P_DestroySwFifo(void )
{
}

#if BBCP_SUPPORT
static uint32_t NEXUS_InputCapture_P_AddToSwFifo(uint32_t channel, uint32_t data)
{
    uint32_t status = 0;

    BDBG_MSG(("enter NEXUS_InputCapture_P_AddToSwFifo, channel=%d, fifo_wptr=%d, fifo_rptr=%d", channel, bicapSwFifo[channel].wptr, bicapSwFifo[channel].rptr));

    #if NEXUS_INPUT_CAPTURE_DO_NOT_ALLOW_OVERFLOW==1
        if ((bicapSwFifo[channel].wptr == NUM_FIFO_ELEMENTS-1) && (bicapSwFifo[channel].rptr==0))
        {
            BDBG_WRN(("buffer overflow"));
            status = 1;
            goto done;
        }

        if ((bicapSwFifo[channel].wptr+1) == bicapSwFifo[channel].rptr)
        {
            BDBG_WRN(("buffer overflow"));
            status = 2;
            goto done;
        }
    #endif

    bicapSwFifo[channel].data[bicapSwFifo[channel].wptr] = data;
    bicapSwFifo[channel].wptr++;
    if (bicapSwFifo[channel].wptr == NUM_FIFO_ELEMENTS) bicapSwFifo[channel].wptr=0;

    BDBG_MSG(("exit NEXUS_InputCapture_P_AddToSwFifo, channel=%d, fifo_wptr=%d, fifo_rptr=%d", channel, bicapSwFifo[channel].wptr, bicapSwFifo[channel].rptr));

#if NEXUS_INPUT_CAPTURE_DO_NOT_ALLOW_OVERFLOW==1
done:
#endif

    return status;
}
#endif

NEXUS_Error NEXUS_InputCapture_Enable(NEXUS_InputCaptureHandle inputCapture)
{
    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
#if BBCP_SUPPORT
    BBCP_SetBicapEnable(inputCapture->bcpHandle, true);
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_InputCapture_Disable(NEXUS_InputCaptureHandle inputCapture)
{
    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
#if BBCP_SUPPORT
    BBCP_SetBicapEnable(inputCapture->bcpHandle, false);
#endif
}

void NEXUS_InputCaptureTimeout_GetDefaultSettings(
    NEXUS_InputCaptureTimeoutSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->inputCapture = 0; /* the input capture selected. this only applies to BICAP. */
    pSettings->edgeSelect = NEXUS_InputCaptureEdgeSelect_eBoth;
    pSettings->clockSelect = NEXUS_InputCaptureFilterClockSelect_eBicapClock;
    pSettings->clockCycles = 0;
}

NEXUS_InputCaptureTimeoutHandle NEXUS_InputCaptureTimeout_Open(
    unsigned index, /* supports NEXUS_ANY_ID */
    const NEXUS_InputCaptureTimeoutSettings *pSettings /* attr{null_allowed=y} */
    )
{
    BERR_Code rc;
    NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout;
    NEXUS_InputCaptureTimeoutSettings defaultSettings;

    /* Let the PI check index */
    if (!pSettings) {
        NEXUS_InputCaptureTimeout_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    inputCaptureTimeout = BKNI_Malloc(sizeof(*inputCaptureTimeout));
    if (!inputCaptureTimeout) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_InputCaptureTimeout, inputCaptureTimeout);
    inputCaptureTimeout->settings = *pSettings;
    inputCaptureTimeout->timeoutIndex = index;
    NEXUS_InputCaptureTimeout_SetSettings(inputCaptureTimeout, pSettings);

    return inputCaptureTimeout;
}

static void NEXUS_InputCaptureTimeout_P_Finalizer(
    NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout
    )
{
    NEXUS_OBJECT_DESTROY(NEXUS_InputCaptureTimeout, inputCaptureTimeout);
    BKNI_Free(inputCaptureTimeout);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_InputCaptureTimeout, NEXUS_InputCaptureTimeout_Close);

NEXUS_Error NEXUS_InputCapture_ReadData(
    NEXUS_InputCaptureHandle inputCapture,
    NEXUS_InputCaptureData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumRead} */
    unsigned numEntries, /* max number that can be read */
    unsigned *pNumRead /* [out] actual number that were read */
    )
{
    uint32_t status=0;
    uint32_t data;
    uint32_t channel;
    *pNumRead=0;

    if (!numEntries || !pData) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    channel = inputCapture->channelNumber;

    BDBG_MSG(("enter NEXUS_InputCapture_Get_From_Sw_Fifo:  fifo_rptr=%d, fifo_wptr=%d", bicapSwFifo[channel].rptr, bicapSwFifo[channel].wptr));

    if (bicapSwFifo[channel].wptr == bicapSwFifo[channel].rptr)
    {
        goto done;
    }

    data = bicapSwFifo[channel].data[bicapSwFifo[channel].rptr];
    *pNumRead=1;

    pData->polarity = (data & (1 << 12)) ? true : false;
    pData->clockCycles = data & 0xfff;
    bicapSwFifo[channel].rptr++;
    if (bicapSwFifo[channel].rptr == NUM_FIFO_ELEMENTS) bicapSwFifo[channel].rptr=0;

    BDBG_MSG(("exit NEXUS_InputCapture_Get_From_Sw_Fifo:  fifo_rptr=%d, fifo_wptr=%d", bicapSwFifo[channel].rptr, bicapSwFifo[channel].wptr));

done:
    return status;
}

NEXUS_Error NEXUS_InputCapture_GetEdgeCount(
    NEXUS_InputCaptureHandle inputCapture,
    unsigned *pNumEdges /* [out] number of edges */
    )
{
    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    *pNumEdges = inputCapture->edgeCount;

    BDBG_MSG(("enter NEXUS_InputCapture_GetEdgeCount:  numEdges=%d", *pNumEdges));
    return NEXUS_SUCCESS;
}

void NEXUS_InputCaptureTimeout_GetSettings(NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout, NEXUS_InputCaptureTimeoutSettings *timeout)
{
#if BBCP_SUPPORT
    BBCP_Timeout_Settings bbcp_timeout;

    BDBG_OBJECT_ASSERT(inputCaptureTimeout, NEXUS_InputCaptureTimeout);
    if ( inputCaptureTimeout->timeoutIndex > 3 ) return;

    BBCP_GetTimeout(g_NEXUS_inputCapture.bcp, inputCaptureTimeout->timeoutIndex, &bbcp_timeout);
    timeout->inputCapture = g_NEXUS_inputCapture.opened[bbcp_timeout.input_sel];
    switch (bbcp_timeout.edge_sel)
    {
        case 1:
            timeout->edgeSelect = NEXUS_InputCaptureEdgeSelect_ePositive;
            break;
        case 2:
            timeout->edgeSelect = NEXUS_InputCaptureEdgeSelect_eNegative;
            break;
        case 3:
            timeout->edgeSelect = NEXUS_InputCaptureEdgeSelect_eBoth;
            break;
        default:
            break;
    }
    timeout->clockSelect = bbcp_timeout.clk_sel;
    timeout->clockCycles = bbcp_timeout.tout;
#else
    BDBG_OBJECT_ASSERT(inputCaptureTimeout, NEXUS_InputCaptureTimeout);
    BSTD_UNUSED(timeout);
#endif
    return;
}

NEXUS_Error NEXUS_InputCaptureTimeout_SetSettings(NEXUS_InputCaptureTimeoutHandle inputCaptureTimeout, const NEXUS_InputCaptureTimeoutSettings *timeout)
{
#if BBCP_SUPPORT
    BBCP_Timeout_Settings bbcp_timeout;

    BDBG_OBJECT_ASSERT(inputCaptureTimeout, NEXUS_InputCaptureTimeout);
    BDBG_OBJECT_ASSERT(timeout->inputCapture, NEXUS_InputCapture);
    bbcp_timeout.input_sel = timeout->inputCapture->channelNumber;
    switch (timeout->edgeSelect)
    {
        case NEXUS_InputCaptureEdgeSelect_ePositive:
            bbcp_timeout.edge_sel = 1;
            break;
        case NEXUS_InputCaptureEdgeSelect_eNegative:
            bbcp_timeout.edge_sel = 2;
            break;
        case NEXUS_InputCaptureEdgeSelect_eBoth:
        default:
            bbcp_timeout.edge_sel = 3;
            break;
    }
    bbcp_timeout.clk_sel = timeout->clockSelect;
    bbcp_timeout.tout = timeout->clockCycles;
    return( BBCP_SetTimeout(g_NEXUS_inputCapture.bcp, inputCaptureTimeout->timeoutIndex, &bbcp_timeout) );
#else
    BSTD_UNUSED(inputCaptureTimeout);
    BSTD_UNUSED(timeout);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

#if 0
static BERR_Code NEXUS_InputCapture_GetFifoInactivityTimeout(NEXUS_InputCaptureFifoInactivityTimeoutSettings *inact)
{
    BERR_Code rc;
    BBCP_FifoInactTimeout_Settings bbcp_inact;

    rc = BBCP_GetFifoInactTimeout(g_NEXUS_inputCapture.bcp, &bbcp_inact);

    inact->bicapClockSelect = bbcp_inact.bicap_clk_sel;
    inact->eventSelect = bbcp_inact.event_sel;
    inact->clockSelect = bbcp_inact.clk_sel;
    inact->timeout = bbcp_inact.tout;

    return rc;
}

static BERR_Code NEXUS_InputCapture_SetFifoInactivityTimeout(NEXUS_InputCaptureFifoInactivityTimeoutSettings *inact)
{
    BERR_Code rc;
    BBCP_FifoInactTimeout_Settings bbcp_inact;

    bbcp_inact.bicap_clk_sel = inact->bicapClockSelect;
    bbcp_inact.event_sel = inact->eventSelect;
    bbcp_inact.clk_sel = inact->clockSelect;
    bbcp_inact.tout = inact->timeout;

    rc = BBCP_SetFifoInactTimeout(g_NEXUS_inputCapture.bcp, &bbcp_inact);

    return rc;
}
#endif

void NEXUS_InputCapture_GetSettings(
    NEXUS_InputCaptureHandle inputCapture,
    NEXUS_InputCaptureSettings *settings
    )
{
#if BBCP_SUPPORT
    BBCP_Edge_Settings edge;
    BBCP_Filter_Settings filter;

    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    BBCP_GetEdge(inputCapture->bcpHandle, &edge);
    settings->bicap.timeoutClockDivisor = edge.tout_clk_div;
    settings->bicap.systemClockDivisor = edge.sys_clk_div;
    settings->bicap.countMode = edge.cnt_mode;
    if (edge.pedgedet_en && edge.nedgedet_en)
        settings->edgeSelect = NEXUS_InputCaptureEdgeSelect_eBoth;
    else if (edge.pedgedet_en)
        settings->edgeSelect = NEXUS_InputCaptureEdgeSelect_ePositive;
    else if (edge.nedgedet_en)
        settings->edgeSelect = NEXUS_InputCaptureEdgeSelect_eNegative;
    settings->bicap.invertSignalInput = edge.insig_inv;

    BBCP_GetFilter(inputCapture->bcpHandle, &filter);
    settings->filter.enable = !filter.bypass;
    switch (filter.clk_sel)
    {
        case 0:
            settings->filter.clockSelect = NEXUS_InputCaptureFilterClockSelect_eBicapClock;
            break;

        case 1:
            settings->filter.clockSelect = NEXUS_InputCaptureFilterClockSelect_eSystemClock;
            break;

        default:
            break;
    }
    settings->filter.value = filter.val;
#else
    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    BSTD_UNUSED(settings);
#endif

    return;
}

NEXUS_Error NEXUS_InputCapture_SetSettings(
    NEXUS_InputCaptureHandle inputCapture,
    const NEXUS_InputCaptureSettings *settings
    )
{
#if BBCP_SUPPORT
    BERR_Code rc;
    BBCP_Edge_Settings edge;
    BBCP_Filter_Settings filter;

    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    edge.tout_clk_div = settings->bicap.timeoutClockDivisor;
    edge.sys_clk_div = settings->bicap.systemClockDivisor;
    edge.cnt_mode = settings->bicap.countMode;
    switch (settings->edgeSelect)
    {
        case NEXUS_InputCaptureEdgeSelect_eBoth:
            edge.pedgedet_en = 1;
            edge.nedgedet_en = 1;
            break;

        case NEXUS_InputCaptureEdgeSelect_ePositive:
            edge.pedgedet_en = 1;
            edge.nedgedet_en = 0;
            break;

        case NEXUS_InputCaptureEdgeSelect_eNegative:
            edge.pedgedet_en = 0;
            edge.nedgedet_en = 1;
            break;

        default:
            edge.pedgedet_en = 0;
            edge.nedgedet_en = 0;
            break;
    }
    edge.insig_inv = settings->bicap.invertSignalInput;
    edge.bicap_en = 0;
    rc = BBCP_SetEdge(inputCapture->bcpHandle, &edge);

    filter.bypass = !settings->filter.enable;
    switch (settings->filter.clockSelect)
    {
        case NEXUS_InputCaptureFilterClockSelect_eBicapClock:
        default:
            filter.clk_sel = 0;
            break;

        case NEXUS_InputCaptureFilterClockSelect_eSystemClock:
            filter.clk_sel = 1;
            break;
    }
    filter.val = settings->filter.value;
    rc = BBCP_SetFilter(inputCapture->bcpHandle, &filter);

    return rc;
#else
    BDBG_OBJECT_ASSERT(inputCapture, NEXUS_InputCapture);
    BSTD_UNUSED(settings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

