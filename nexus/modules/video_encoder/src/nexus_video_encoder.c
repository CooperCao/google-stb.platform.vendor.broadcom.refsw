/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
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
#include "nexus_video_encoder_module.h"
#include "nexus_power_management.h"

#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_display_priv.h"
#include "priv/nexus_video_window_priv.h"
#include "priv/nexus_core_img_id.h"
#include "priv/nexus_core.h"
#include "bxudlib.h"
#include "b_objdb.h"
#include "bvce_debug.h"
#include "nexus_video_encoder_security.h"

BDBG_MODULE(nexus_video_encoder);
BDBG_FILE_MODULE(nexus_video_encoder_status);
BDBG_FILE_MODULE(vce_proc);

static NEXUS_Error NEXUS_VideoEncoderModule_P_PostInit(void);

NEXUS_VideoEncoder_P_State g_NEXUS_VideoEncoder_P_State;

void
NEXUS_VideoEncoderModule_GetDefaultSettings( NEXUS_VideoEncoderModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* we can't set meaningless default heap assignment since it's chip/platform speciific */

    return;
}

static NEXUS_VideoEncoderModuleSettings g_moduleSettings;

static void NEXUS_VideoEncoderModule_P_Print_Vce(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;

    BDBG_MODULE_LOG(vce_proc, ("VideoEncoderModule:"));
    for (i=0; i<NEXUS_NUM_VCE_DEVICES; i++) {
        unsigned j;
        if (!g_NEXUS_VideoEncoder_P_State.vceDevices[i].vce) continue;
        BDBG_MODULE_LOG(vce_proc, ("VCE%u[dev%u/ch%u]: (%p) general:%uMB secure:%uMB picture:%uMB", i,
            g_moduleSettings.vceMapping[i].device, g_moduleSettings.vceMapping[i].channel, (void *)g_NEXUS_VideoEncoder_P_State.vceDevices[i].vce,
            g_moduleSettings.heapSize[i].general/1024/1024,
            g_moduleSettings.heapSize[i].secure/1024/1024,
            g_moduleSettings.heapSize[i].picture/1024/1024));
        for (j=0; j < NEXUS_NUM_VCE_CHANNELS; j++) {
            NEXUS_VideoEncoderHandle videoEncoder = &g_NEXUS_VideoEncoder_P_State.vceDevices[i].channels[j];
            if (videoEncoder->enc) {
                NEXUS_VideoEncoderStatus status;
                if(NEXUS_VideoEncoder_GetStatus(videoEncoder, &status)) continue;

                BDBG_MODULE_LOG(vce_proc, (" device%u channel%d: vceChannel=%p, %s", i, j, (void *)videoEncoder->enc, videoEncoder->startSettings.nonRealTime ? "NRT" : "RT"));
                BDBG_MODULE_LOG(vce_proc, ("  started=%c, codec=%d, profile=%#x, level=%p, stcCh=%p", videoEncoder->started?'y':'n',
                    videoEncoder->started?videoEncoder->startSettings.codec:0,
                    videoEncoder->started?videoEncoder->startSettings.profile:0, (void *)videoEncoder->startSettings.level, (void *)videoEncoder->startSettings.stcChannel));
                BDBG_MODULE_LOG(vce_proc, ("  CDB: %d/%d (%d%%), ITB: %d/%d (%d%%)",
                    status.data.fifoDepth, status.data.fifoSize, status.data.fifoSize?status.data.fifoDepth*100/status.data.fifoSize:0,
                    status.index.fifoDepth, status.index.fifoSize, status.index.fifoSize?status.index.fifoDepth*100/status.index.fifoSize:0));
                BDBG_MODULE_LOG(vce_proc, ("  Encode: encoded=%d lastPicID=%d picPerSeconds=%d", status.picturesEncoded, status.pictureIdLastEncoded,
                    status.picturesPerSecond));
                BDBG_MODULE_LOG(vce_proc, ("  Drops: received=%d dropsFRC=%d dropsHRD=%d dropsErr=%d", status.picturesReceived, status.picturesDroppedFRC,
                    status.picturesDroppedHRD, status.picturesDroppedErrors));
                BDBG_MODULE_LOG(vce_proc, ("  Flags: errorCount=%d errorFlags=%u eventFlags=%u",
                    status.errorCount, status.errorFlags, status.eventFlags));
            }
        }
    }
#endif
}

NEXUS_ModuleHandle
NEXUS_VideoEncoderModule_Init(const NEXUS_VideoEncoderModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BERR_Code rc;

    BDBG_ASSERT(pSettings);

    BDBG_CASSERT(NEXUS_NUM_VCE_DEVICES <= NEXUS_MAX_VCE_DEVICES);
    BDBG_ASSERT(g_NEXUS_VideoEncoder_P_State.module==NULL);
    BDBG_ASSERT(pSettings->display);
    BDBG_ASSERT(pSettings->transport);
    BKNI_Memset(&g_NEXUS_VideoEncoder_P_State, 0, sizeof(g_NEXUS_VideoEncoder_P_State));
    g_NEXUS_VideoEncoder_P_State.config = *pSettings;
    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.dbgPrint = NEXUS_VideoEncoderModule_P_Print_Vce;
    moduleSettings.dbgModules = "nexus_video_encoder";
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* encoder interface is slow */
    g_NEXUS_VideoEncoder_P_State.module = NEXUS_Module_Create("video_encoder", &moduleSettings);
    if(g_NEXUS_VideoEncoder_P_State.module == NULL) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }

    g_moduleSettings = *pSettings;

    if (!pSettings->deferInit) {
        NEXUS_LockModule();
        rc = NEXUS_VideoEncoderModule_P_PostInit();
        NEXUS_UnlockModule();
        if (rc) {
            NEXUS_Module_Destroy(g_NEXUS_VideoEncoder_P_State.module);
            g_NEXUS_VideoEncoder_P_State.module = NULL;
        }
    }

    return g_NEXUS_VideoEncoder_P_State.module;
}

static void NEXUS_VideoEncoder_P_WatchdogHandler_isr(void *context, int32_t unused1, const BVCE_WatchdogCallbackInfo *unused2)
{
    NEXUS_VideoEncoder_P_Device *device = context;
    BSTD_UNUSED(unused1);
    BSTD_UNUSED(unused2);
    BKNI_SetEvent_isr(device->watchdogEvent);
    return;
}

static void NEXUS_VideoEncoder_P_WatchdogHandler(void *context)
{
    BERR_Code rc;
    NEXUS_VideoEncoder_P_Device *device = context;
    unsigned i;
    BDBG_WRN(("Video Encoder Watchdog"));
    rc = BVCE_ProcessWatchdog(device->vce);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);/* keep going */ }
    for(i=0;i<NEXUS_NUM_VCE_CHANNELS;i++ ) {
        if(device->channels[i].enc && device->channels[i].startSettings.input) {
            NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.display);
            NEXUS_DisplayModule_ClearDisplay_priv(device->channels[i].startSettings.input);
            NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.display);
        }
    }
    for(i=0;i<NEXUS_NUM_VCE_CHANNELS;i++ ) {
        if(device->channels[i].enc && device->channels[i].startSettings.input) {
            NEXUS_TaskCallback_Fire(device->channels[i].watchdogCallbackHandler);
        }
    }

    return;
}

static void NEXUS_VideoEncoder_P_DebugLog_Shutdown(NEXUS_VideoEncoder_P_Device *device)
{
    if(device->debugLog.timer) {
        NEXUS_CancelTimer(device->debugLog.timer);
        device->debugLog.timer = NULL;
    }
    if(device->debugLog.reader) {
        BDBG_FifoReader_Destroy(device->debugLog.reader);
        device->debugLog.reader = NULL;
    }
    if(device->debugLog.entry) {
        BKNI_Free(device->debugLog.entry);
        device->debugLog.entry = NULL;
    }
    if(device->debugLog.fifo) {
        BMMA_Unlock(device->debugLog.fifoInfo.hBlock, (uint8_t *)device->debugLog.fifo - device->debugLog.fifoInfo.uiOffset);
        device->debugLog.fifo = NULL;
    }
    return;
}

static void
NEXUS_VideoEncoder_P_DebugLogTimer(void *context)
{
    NEXUS_VideoEncoder_P_Device *device = context;
    for(;;) {
        BERR_Code rc = BDBG_FifoReader_Read(device->debugLog.reader, device->debugLog.entry, device->debugLog.fifoInfo.uiElementSize);
        if(rc==BERR_SUCCESS) {
            BVCE_Debug_PrintLogMessageEntry(device->debugLog.entry);
        } else if(rc==BERR_FIFO_OVERFLOW) {
            BDBG_LOG(("DebugLog FIFO OVERFLOW"));
            BDBG_FifoReader_Resync(device->debugLog.reader);
        } else if(rc==BERR_FIFO_NO_DATA) {
            break;
        } else {
            break;
        }
    }
    device->debugLog.timer = NEXUS_ScheduleTimer(20, NEXUS_VideoEncoder_P_DebugLogTimer, device);
    return;
}

static NEXUS_Error NEXUS_VideoEncoderModule_P_PostInit(void)
{
    unsigned i, j;
    NEXUS_Error rc;
    const NEXUS_VideoEncoderModuleSettings *pSettings = &g_moduleSettings;

    /* Open ViCE devices */
    for(i=0;i<NEXUS_NUM_VCE_DEVICES;i++ ) {
        BVCE_PlatformSettings platformSettings;
        BVCE_OpenSettings  vceSettings;
        BVCE_CallbackSettings callbackSettings;
        NEXUS_VideoEncoder_P_Device *device = g_NEXUS_VideoEncoder_P_State.vceDevices+i;
        g_NEXUS_VideoEncoder_P_State.vceDevices[i].vce = NULL;

        BKNI_Memset(device->channels, 0, sizeof(device->channels));

        for(j=0; j<NEXUS_NUM_VIDEO_ENCODERS; j++) {
            if ((int)i==pSettings->vceMapping[j].device) break; /* mapped / used */
        }
        /* if device i is not mapped or used by any encoder channels, skip */
        if(j == NEXUS_NUM_VIDEO_ENCODERS) continue;

        device->mma = g_pCoreHandles->heap[pSettings->heapIndex[i].system].mma;
        BVCE_GetDefaultPlatformSettings( &platformSettings );
        platformSettings.hBox = g_pCoreHandles->box;
        platformSettings.uiInstance = i;
        BVCE_GetDefaultOpenSettings(&platformSettings, &vceSettings);
        if(pSettings->heapSize[i].picture) {
            vceSettings.stMemoryConfig.uiPictureMemSize = pSettings->heapSize[i].picture;
        }
        if(pSettings->heapSize[i].secure) {
            vceSettings.stMemoryConfig.uiSecureMemSize = pSettings->heapSize[i].secure;
        }

        vceSettings.hFirmwareMem[0] = g_pCoreHandles->heap[pSettings->heapIndex[i].firmware[0]].mma;
        vceSettings.hFirmwareMem[1] = g_pCoreHandles->heap[pSettings->heapIndex[i].firmware[1]].mma;
        vceSettings.hSecureMem = g_pCoreHandles->heap[pSettings->heapIndex[i].secure].mma;
        vceSettings.hPictureMem = g_pCoreHandles->heap[pSettings->heapIndex[i].picture].mma;
        if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_VCE,&device->img_context,&device->img_interface )== NEXUS_SUCCESS)
        {
            BDBG_WRN(("FW download used"));
            vceSettings.pImgInterface = &device->img_interface;
            vceSettings.pImgContext   = device->img_context;
        }
        vceSettings.bVerificationMode = NEXUS_GetEnv("video_encoder_verification")!=NULL;
        vceSettings.uiInstance = i;
        BDBG_MSG(("VCE[%u] fw heap[%u] secure heap[%u] pic heap[%u]", vceSettings.uiInstance
                                                                    , pSettings->heapIndex[i].firmware[0]
                                                                    , pSettings->heapIndex[i].secure
                                                                    , pSettings->heapIndex[i].picture ));
        NEXUS_VideoEncoder_P_GetSecurityCallbacks(&vceSettings, i /*deviceId*/);

        rc = BVCE_Open( &device->vce, g_pCoreHandles->chp, g_pCoreHandles->reg, device->mma, g_pCoreHandles->bint, &vceSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        {
            const char *bvce_debug = NEXUS_GetEnv("BVCE_Debug");
            if(bvce_debug) {
                BVCE_ArcInstance arcInstance = (BVCE_ArcInstance) (bvce_debug[0] - '0');
                char bvce_debug_line[64];
                if(arcInstance < BVCE_ArcInstance_eMax) {
                    unsigned i;
                    bvce_debug++;
                    /* convert from "ItbStcOn_DbgXXX" to "ItbStcOn\nDbgXXX\n\n" */
                    for(i=0;i<sizeof(bvce_debug_line)/sizeof(*bvce_debug_line)-4;i++) {
                        char ch=bvce_debug[i];
                        if(ch=='\0') {
                            break;
                        }
                        bvce_debug_line[i] = (ch=='_') ? '\n' : ch;
                    }
                    bvce_debug_line[i] = '\n';
                    bvce_debug_line[i+1] = '\n';
                    bvce_debug_line[i+2] = '\0';
                    bvce_debug_line[i+3] = '\0'; /* double null terminated command string */
                    rc = BVCE_Debug_SendCommand(device->vce, arcInstance, bvce_debug_line);
                    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
                } else {
                    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
            }
        }
        if(NEXUS_GetEnv("bvce_log")) {
            rc = BVCE_Debug_GetLogMessageFifo(device->vce, &device->debugLog.fifoInfo);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

            device->debugLog.entry = BKNI_Malloc(device->debugLog.fifoInfo.uiElementSize);
            if(device->debugLog.entry==NULL) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

            device->debugLog.fifo = BMMA_Lock(device->debugLog.fifoInfo.hBlock);
            if(device->debugLog.fifo == NULL) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}

            device->debugLog.fifo = (uint8_t *)device->debugLog.fifo + device->debugLog.fifoInfo.uiOffset;
            rc = BDBG_FifoReader_Create(&device->debugLog.reader, device->debugLog.fifo);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

            device->debugLog.timer = NEXUS_ScheduleTimer(20, NEXUS_VideoEncoder_P_DebugLogTimer, device);
            if(!device->debugLog.timer) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
        }
        rc = BKNI_CreateEvent(&device->watchdogEvent);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}
        device->watchdogEventHandler = NEXUS_RegisterEvent(device->watchdogEvent, NEXUS_VideoEncoder_P_WatchdogHandler, device);
        if (!device->watchdogEventHandler) {rc=BERR_TRACE(NEXUS_UNKNOWN); goto error;}
        BVCE_GetDefaultCallbackSettings(&callbackSettings);
        callbackSettings.stWatchdog.bEnable = true;
        callbackSettings.stWatchdog.fCallback = NEXUS_VideoEncoder_P_WatchdogHandler_isr;
        callbackSettings.stWatchdog.pPrivateContext = device;
        rc = BVCE_SetCallbackSettings(device->vce, &callbackSettings);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}
    }
    return NEXUS_SUCCESS;

error:
    for(i=0;i<NEXUS_NUM_VCE_DEVICES;i++ ) {
        NEXUS_VideoEncoder_P_Device *device = g_NEXUS_VideoEncoder_P_State.vceDevices+i;
        NEXUS_VideoEncoder_P_DebugLog_Shutdown(device);
        if (device->watchdogEventHandler) {
            NEXUS_UnregisterEvent(device->watchdogEventHandler);
        }
        if (device->watchdogEvent) {
            BKNI_DestroyEvent(device->watchdogEvent);
        }
        if (device->vce) {
            BVCE_Close(device->vce);
        }
        if (device->img_context) {
            Nexus_Core_P_Img_Destroy(device->img_context);
        }
    }
    return rc;
}

void
NEXUS_VideoEncoderModule_Uninit(void)
{
    unsigned i,j;
    if(g_NEXUS_VideoEncoder_P_State.module==NULL) {return;}

    NEXUS_LockModule();

    for(i=0;i<NEXUS_NUM_VCE_DEVICES;i++ ) {
        NEXUS_VideoEncoder_P_Device *device = g_NEXUS_VideoEncoder_P_State.vceDevices+i;
        BVCE_CallbackSettings callbackSettings;

        if (!device->vce) {
            continue;
        }
        NEXUS_VideoEncoder_P_DebugLog_Shutdown(device);
        BVCE_GetCallbackSettings(device->vce, &callbackSettings);
        callbackSettings.stWatchdog.bEnable = false;
        BVCE_SetCallbackSettings(device->vce, &callbackSettings);

        if(device->watchdogEventHandler) {
            NEXUS_UnregisterEvent(device->watchdogEventHandler);
            device->watchdogEventHandler = NULL;
        }
        if(device->watchdogEvent) {
            BKNI_DestroyEvent(device->watchdogEvent);
            device->watchdogEvent = NULL;
        }
        for(j=0;j<NEXUS_NUM_VCE_CHANNELS;j++ ) {
            if(device->channels[j].enc) {
                NEXUS_VideoEncoder_Close(&device->channels[j]);
            }
        }
        BVCE_Close(device->vce);
        if (device->img_context) {
            Nexus_Core_P_Img_Destroy(device->img_context);
        }

        NEXUS_VideoEncoder_P_DisableFwVerification(i);
    }

    NEXUS_UnlockModule();

    NEXUS_Module_Destroy(g_NEXUS_VideoEncoder_P_State.module);
    g_NEXUS_VideoEncoder_P_State.module = NULL;
    return;
}

NEXUS_Error NEXUS_VideoEncoderModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

#if NEXUS_POWER_MANAGEMENT
    unsigned i;

    BSTD_UNUSED(pSettings);

    for(i=0; i<NEXUS_NUM_VCE_DEVICES; i++) {
    NEXUS_VideoEncoder_P_Device *device = g_NEXUS_VideoEncoder_P_State.vceDevices+i;
    if(device->vce) {
        if(enabled) {
            rc = BVCE_Power_Standby(device->vce);
            NEXUS_VideoEncoder_P_DisableFwVerification( i );
        } else {
            rc = BVCE_Power_Resume(device->vce);
        }
    }
    }
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif

    return rc;
}


void NEXUS_VideoEncoder_GetDefaultOpenSettings(NEXUS_VideoEncoderOpenSettings *pSettings)
{
    BVCE_Channel_OpenSettings encSettings;
    BVCE_Channel_MemoryBoundsSettings boundSettings;

    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BVCE_Channel_GetDefaultOpenSettings(g_pCoreHandles->box, &encSettings);
    pSettings->data.fifoSize = encSettings.stMemoryConfig.uiDataMemSize;
    pSettings->index.fifoSize = encSettings.stMemoryConfig.uiIndexMemSize;
    pSettings->maxChannelCount = encSettings.uiMaxNumChannels;
    pSettings->enableDataUnitDetecton = encSettings.stOutput.bEnableDataUnitDetection;
    pSettings->type = encSettings.eMultiChannelMode;
    BVCE_Channel_GetDefaultMemoryBoundsSettings(g_pCoreHandles->box, &boundSettings);
    pSettings->memoryConfig.interlaced = (boundSettings.eInputType == BAVC_ScanType_eInterlaced);
    pSettings->memoryConfig.maxWidth   = boundSettings.stDimensions.stMax.uiWidth;
    pSettings->memoryConfig.maxHeight   = boundSettings.stDimensions.stMax.uiHeight;
    BDBG_CASSERT((int)BVCE_MultiChannelMode_eMulti == (int)NEXUS_VideoEncoderType_eMulti);
    BDBG_CASSERT((int)BVCE_MultiChannelMode_eSingle == (int)NEXUS_VideoEncoderType_eSingle);
    BDBG_CASSERT((int)BVCE_MultiChannelMode_eMultiNRTOnly == (int)NEXUS_VideoEncoderType_eMultiNonRealTime);
    BDBG_CASSERT((int)BVCE_MultiChannelMode_eCustom == (int)NEXUS_VideoEncoderType_eCustom);
    BDBG_CWARNING((int)BVCE_MultiChannelMode_eMax == (int)NEXUS_VideoEncoderType_eMax);
    return ;
}

#if BVCE_P_DUMP_ARC_DEBUG_LOG
static void
NEXUS_VideoEncoder_P_LogTimer(void *context)
{
    unsigned i, j;
    unsigned logged;

    BSTD_UNUSED(context);
    for(logged=0,i=0;i<NEXUS_NUM_VCE_DEVICES;i++) {
        NEXUS_VideoEncoder_P_Device *device = &g_NEXUS_VideoEncoder_P_State.vceDevices[i];
        uint32_t bytesRead = 0;
        char log[128];

        for(j=0; j<BVCE_ArcInstance_eMax; j++) {
            BVCE_Debug_ReadBuffer( device->vce, j,log,sizeof(log)-1,&bytesRead);
            if(bytesRead>0) {
                BDBG_ASSERT(bytesRead<sizeof(log));
                logged += bytesRead;
                log[bytesRead]='\0';
                BDBG_MSG(("ARC%d:%s", j, log));
            }
        }
    }
    g_NEXUS_VideoEncoder_P_State.logTimer = NEXUS_ScheduleTimer(logged>0?10:100, NEXUS_VideoEncoder_P_LogTimer, NULL);
    return;
}
#endif

static NEXUS_HeapHandle get_heap(NEXUS_HeapHandle userHeap, unsigned platformIndex)
{
    if (userHeap) {
        /* always prefer what the user specifies */
        return userHeap;
    }
    else {
        /* otherwise, return specified heap, or first heap with eFull mapping */
        return NEXUS_P_DefaultHeap(g_pCoreHandles->heap[platformIndex].nexus, NEXUS_DefaultHeapType_eFull);
    }
}

static void NEXUS_P_VideoEncoder_DataReady_isr(void *context, int32_t unused_1, const BVCE_Channel_EventCallbackInfo *unused_2)
{
    NEXUS_VideoEncoderHandle encoder = context;

    BSTD_UNUSED(unused_1);
    BSTD_UNUSED(unused_2);

    NEXUS_OBJECT_ASSERT(NEXUS_VideoEncoder, encoder);
    NEXUS_IsrCallback_Fire_isr(encoder->dataReadyCallback);
    return;
}


NEXUS_VideoEncoderHandle
NEXUS_VideoEncoder_Open(unsigned index, const NEXUS_VideoEncoderOpenSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_VideoEncoderOpenSettings settings;
    BVCE_Channel_OpenSettings encSettings;
    NEXUS_VideoEncoder_P_Device *device = &g_NEXUS_VideoEncoder_P_State.vceDevices[0];
    NEXUS_VideoEncoderHandle encoder=NULL;
    NEXUS_HeapHandle heap;
    unsigned deviceId, chanId;
    BVCE_Channel_CallbackSettings callbackSettings;

    if(index>=NEXUS_NUM_VCE_CHANNELS*NEXUS_NUM_VCE_DEVICES) {rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto error;}

    if(g_moduleSettings.vceMapping[index].device == -1) {
        BDBG_ERR(("Video encoder[%u] is not supported by this platform!", index));
        rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto error;
    }
    deviceId = g_moduleSettings.vceMapping[index].device;
    chanId   = g_moduleSettings.vceMapping[index].channel;
    BDBG_MSG(("Open video encoder %u: VCE[%u] channel[%u]", index, deviceId, chanId));

    /* deferred init */
    if (!g_NEXUS_VideoEncoder_P_State.vceDevices[deviceId].vce) {
        rc = NEXUS_VideoEncoderModule_P_PostInit();
        if (rc) {
            BERR_TRACE(rc);
            return NULL;
        }
    }

    /* video encoder index/device/channel mapping */
    device = &g_NEXUS_VideoEncoder_P_State.vceDevices[deviceId];
    encoder = &device->channels[chanId];
    if (!device->vce) goto error; /* Encoder may be unavailable due to RTS use case setting */
    if(encoder->enc!=NULL) {rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);encoder=NULL;goto error; }

    if(pSettings==NULL) {
        NEXUS_VideoEncoder_GetDefaultOpenSettings(&settings);
        pSettings=&settings;
    }
    NEXUS_OBJECT_INIT(NEXUS_VideoEncoder, encoder);
    encoder->watchdogCallbackHandler = NEXUS_TaskCallback_Create(encoder, NULL);
    if(!encoder->watchdogCallbackHandler) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
    encoder->dataReadyCallback = NEXUS_IsrCallback_Create(encoder, NULL);
    if(!encoder->dataReadyCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}

    encoder->index = index;
    encoder->openSettings = *pSettings;
    encoder->device = device;
    encoder->frame.mma = NULL;
    encoder->frame.nexus = NULL;
    encoder->meta.mma = NULL;
    encoder->meta.nexus = NULL;
    encoder->debugLog.mma = NULL;
    encoder->debugLog.nexus = NULL;

    BVCE_Channel_GetDefaultOpenSettings(g_pCoreHandles->box, &encSettings);

    /* the channel instance ID is the context ID in a core or device */
    encSettings.uiInstance = chanId;
    encSettings.eMultiChannelMode = pSettings->type;
    encSettings.uiMaxNumChannels = pSettings->maxChannelCount;
    encSettings.stOutput.bEnableDataUnitDetection = pSettings->enableDataUnitDetecton;

    /* if user sets memory config */
    if(pSettings->memoryConfig.maxWidth && pSettings->memoryConfig.maxHeight) {
        BVCE_Channel_MemoryBoundsSettings channelSettings;
        BVCE_Channel_MemorySettings memSettings;
        BVCE_MemoryConfig memConfig;
        BCHP_MemoryInfo memoryInfo;

        BVCE_Channel_GetDefaultMemoryBoundsSettings(g_pCoreHandles->box, &channelSettings);
        channelSettings.stDimensions.stMax.uiWidth = pSettings->memoryConfig.maxWidth;
        channelSettings.stDimensions.stMax.uiHeight = pSettings->memoryConfig.maxHeight;
        channelSettings.eInputType = pSettings->memoryConfig.interlaced?BAVC_ScanType_eInterlaced:BAVC_ScanType_eProgressive;

        rc = BCHP_GetMemoryInfo(g_pCoreHandles->reg, &memoryInfo);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        BKNI_Memset(&memSettings,0,sizeof(memSettings));
        memSettings.pstMemoryInfo = &memoryInfo;
        memSettings.uiInstance = chanId;
        /* uiPicture, uiSecure aren't populated, since it isn't used */


        BVCE_Channel_GetMemoryConfig(g_pCoreHandles->box, &memSettings, &channelSettings, &memConfig);

        encSettings.stMemoryConfig.uiPictureMemSize = memConfig.uiPictureMemSize;
        encSettings.stMemoryConfig.uiSecureMemSize  = memConfig.uiSecureMemSize;
        BDBG_MSG(("Memory Config(%ux%u%c) heap size: picture %d, secure %d",
            pSettings->memoryConfig.maxWidth, pSettings->memoryConfig.maxHeight,
            pSettings->memoryConfig.interlaced?'i' : 'p',
            memConfig.uiPictureMemSize, memConfig.uiSecureMemSize));
    }

    encSettings.stOutput.bAllocateOutput = true;
    encSettings.stMemoryConfig.uiDataMemSize = pSettings->data.fifoSize;
    encSettings.stMemoryConfig.uiIndexMemSize = pSettings->index.fifoSize;
    BDBG_MSG(("CDB size = 0x%x, ITB size = 0x%x", encSettings.stMemoryConfig.uiDataMemSize, encSettings.stMemoryConfig.uiIndexMemSize));
    /* ITB buffer must be non-secure to be parsed by host mux manager; and
     * ITB buffer must be on MEMC0 since CABAC hw generates the ITB and CABAC is tied to MEMC0 for security reason; */
    encSettings.stOutput.hIndexMem = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;
    /* apply application preferences and defaults */
    heap = get_heap(pSettings->index.heap, g_pCoreHandles->defaultHeapIndex);
    if (heap) {
       encSettings.stOutput.hIndexMem = NEXUS_Heap_GetMmaHandle(heap);
    }
    heap = get_heap(pSettings->data.heap, g_moduleSettings.heapIndex[deviceId].output);
    if (heap) {
       encSettings.stOutput.hDataMem = NEXUS_Heap_GetMmaHandle(heap);
    }

    rc = BVCE_Channel_Open(device->vce, &encoder->enc, &encSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    BVCE_Channel_GetDefaultCallbackSettings(&callbackSettings);
    callbackSettings.stDataReady.bEnable = true;
    callbackSettings.stDataReady.fCallback = NEXUS_P_VideoEncoder_DataReady_isr;
    callbackSettings.stDataReady.pPrivateContext = encoder;
    rc = BVCE_Channel_SetCallbackSettings(encoder->enc, &callbackSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error_set;}

    NEXUS_TaskCallback_Set(encoder->watchdogCallbackHandler, &pSettings->watchdogCallback);

    NEXUS_VideoEncoder_GetDefaultSettings(&encoder->settings);

#if BVCE_P_DUMP_ARC_DEBUG_LOG
    g_NEXUS_VideoEncoder_P_State.logTimer = NEXUS_ScheduleTimer(100, NEXUS_VideoEncoder_P_LogTimer, NULL);
#endif
    return encoder;
error_set:
    BVCE_Channel_Close(encoder->enc);
error:
    if(encoder) {
        encoder->enc = NULL; /* clear 'enc' field, it's used to mark 'opened' encoders */
    }
    return NULL;
}

void NEXUS_VideoEncoder_GetDefaultSettings( NEXUS_VideoEncoderSettings *pSettings )
{
    BVCE_Channel_EncodeSettings encodeSettings;
    BVCE_Channel_GetDefaultEncodeSettings(g_pCoreHandles->box, &encodeSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bitrateMax = encodeSettings.stBitRate.uiMax;
    pSettings->bitrateTarget = encodeSettings.stBitRate.uiTarget;
    pSettings->encoderDelay = encodeSettings.uiA2PDelay;
    pSettings->frameRate = NEXUS_P_FrameRate_FromMagnum_isrsafe(encodeSettings.stFrameRate.eFrameRate);
    pSettings->variableFrameRate = encodeSettings.stFrameRate.bVariableFrameRateMode;
    pSettings->enableFieldPairing = encodeSettings.bITFPEnable;
    pSettings->streamStructure.newGopOnSceneChange = encodeSettings.stGOPStructure.bAllowNewGOPOnSceneChange;
    pSettings->streamStructure.openGop = encodeSettings.stGOPStructure.bAllowOpenGOP;
    pSettings->streamStructure.duration = encodeSettings.stGOPStructure.uiDuration;
    pSettings->streamStructure.framesP = encodeSettings.stGOPStructure.uiNumberOfPFrames;
    pSettings->streamStructure.framesB = encodeSettings.stGOPStructure.uiNumberOfBFrames;
}

static void
NEXUS_VideoEncoder_P_Finalizer(NEXUS_VideoEncoderHandle encoder)
{
    BVCE_Channel_CallbackSettings callbackSettings;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoEncoder, encoder);

    if (encoder->started) {
        NEXUS_VideoEncoder_Stop(encoder, NULL);
    }

    /* cancel timer here to be symmetric with open */
#if BVCE_P_DUMP_ARC_DEBUG_LOG
    if(g_NEXUS_VideoEncoder_P_State.logTimer) {
        NEXUS_CancelTimer(g_NEXUS_VideoEncoder_P_State.logTimer);
        g_NEXUS_VideoEncoder_P_State.logTimer = NULL;
    }
#endif

    /* disable channel callbacks before closing to avoid staled callback crash after close */
    BVCE_Channel_GetCallbackSettings(encoder->enc, &callbackSettings);
    callbackSettings.stDataReady.bEnable = false;
    callbackSettings.stEvent.bEnable     = false;
    BVCE_Channel_SetCallbackSettings(encoder->enc, &callbackSettings);

    BVCE_Channel_Close(encoder->enc);
    NEXUS_IsrCallback_Destroy(encoder->dataReadyCallback);
    NEXUS_TaskCallback_Destroy(encoder->watchdogCallbackHandler);
    NEXUS_OBJECT_DESTROY(NEXUS_VideoEncoder, encoder);
    encoder->enc = NULL;
    return;
}

static void
NEXUS_VideoEncoder_P_Release(NEXUS_VideoEncoderHandle encoder)
{
    /* when application calls _Close (or exits) , remove existing NEXUS_MemoryBlock <-> BMMA_Block mapping */
    if(encoder->frame.nexus) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, encoder->frame.nexus, Release);
        NEXUS_MemoryBlock_Free(encoder->frame.nexus);
        encoder->frame.nexus=NULL;
    }
    if(encoder->meta.nexus) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, encoder->meta.nexus, Release);
        NEXUS_MemoryBlock_Free(encoder->meta.nexus);
        encoder->meta.nexus=NULL;
    }
    if(encoder->debugLog.nexus) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, encoder->debugLog.nexus, Release);
        NEXUS_MemoryBlock_Free(encoder->debugLog.nexus);
        encoder->debugLog.nexus=NULL;
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoEncoder, NEXUS_VideoEncoder_Close);

void
NEXUS_VideoEncoder_GetDefaultStartSettings(NEXUS_VideoEncoderStartSettings *pSettings )
{
    BVCE_Channel_StartEncodeSettings encodeSettings;

    BVCE_Channel_GetDefaultStartEncodeSettings(g_pCoreHandles->box, &encodeSettings);

    BDBG_CASSERT(NEXUS_VideoCodecLevel_eUnknown ==  (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eUnknown);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e00 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e00);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e10 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e10);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e1B ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e1B);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e11 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e11);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e12 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e12);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e13 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e13);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e20 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e20);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e21 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e21);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e22 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e22);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e30 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e30);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e31 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e31);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e32 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e32);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e40 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e40);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e41 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e41);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e42 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e42);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e50 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e50);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e51 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e51);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e60 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e60);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_e62 ==       (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_e62);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_eLow ==      (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eLow);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_eMain ==     (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eMain);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_eHigh ==     (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eHigh);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_eHigh1440 == (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eHigh1440);
    BDBG_CASSERT(NEXUS_VideoCodecLevel_eMax ==      (NEXUS_VideoCodecLevel)BAVC_VideoCompressionLevel_eMax);

    BDBG_CASSERT(NEXUS_VideoCodecProfile_eUnknown ==           (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eUnknown);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eSimple ==            (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eSimple);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eMain ==              (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eMain);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eHigh ==              (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eHigh);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eAdvanced ==          (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eAdvanced);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eJizhun ==            (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eJizhun);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eSnrScalable ==       (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eSnrScalable);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eSpatiallyScalable == (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eSpatiallyScalable);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eAdvancedSimple ==    (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eAdvancedSimple);
    BDBG_CASSERT(NEXUS_VideoCodecProfile_eBaseline ==          (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eBaseline);
    BDBG_CWARNING(NEXUS_VideoCodecProfile_eMax ==              (NEXUS_VideoCodecProfile)BAVC_VideoCompressionProfile_eMax);

    BDBG_CASSERT(NEXUS_EntropyCoding_eAuto ==    (NEXUS_EntropyCoding)BVCE_EntropyCoding_eDefault);
    BDBG_CASSERT(NEXUS_EntropyCoding_eCavlc ==      (NEXUS_EntropyCoding)BVCE_EntropyCoding_eCAVLC);
    BDBG_CASSERT(NEXUS_EntropyCoding_eCabac ==      (NEXUS_EntropyCoding)BVCE_EntropyCoding_eCABAC);
    BDBG_CWARNING(NEXUS_EntropyCoding_eMax  ==      (NEXUS_EntropyCoding)BVCE_EntropyCoding_eMax);


    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->nonRealTime = encodeSettings.bNonRealTimeEncodeMode;
    pSettings->lowDelayPipeline = encodeSettings.bPipelineLowDelayMode;
    pSettings->rateBufferDelay = encodeSettings.uiRateBufferDelay;
    pSettings->adaptiveLowDelayMode = encodeSettings.bAdaptiveLowDelayMode;
    pSettings->encodeUserData = false;
    pSettings->bounds.inputDimension.max.width = encodeSettings.stBounds.stDimensions.stMax.uiWidth;
    pSettings->bounds.inputDimension.max.height = encodeSettings.stBounds.stDimensions.stMax.uiHeight;
    pSettings->bounds.inputDimension.maxInterlaced.width = encodeSettings.stBounds.stDimensions.stMaxInterlaced.uiWidth;
    pSettings->bounds.inputDimension.maxInterlaced.height = encodeSettings.stBounds.stDimensions.stMaxInterlaced.uiHeight;
    pSettings->bounds.bitrate.upper.bitrateMax = encodeSettings.stBounds.stBitRate.stLargest.uiMax;
    pSettings->bounds.bitrate.upper.bitrateTarget = encodeSettings.stBounds.stBitRate.stLargest.uiTarget;
    pSettings->bounds.streamStructure.max.framesB = encodeSettings.stBounds.stGOPStructure.uiNumberOfBFrames;
    pSettings->bounds.outputFrameRate.min = NEXUS_P_FrameRate_FromMagnum_isrsafe(encodeSettings.stBounds.stFrameRate.eMin);
    pSettings->bounds.outputFrameRate.max = NEXUS_P_FrameRate_FromMagnum_isrsafe(encodeSettings.stBounds.stFrameRate.eMax);
    pSettings->bounds.inputFrameRate.min = NEXUS_P_FrameRate_FromMagnum_isrsafe(encodeSettings.stBounds.stInputFrameRate.eMin);

    pSettings->interlaced = encodeSettings.eInputType == BAVC_ScanType_eInterlaced;
    pSettings->codec = NEXUS_P_VideoCodec_FromMagnum(encodeSettings.stProtocolInfo.eProtocol);
    pSettings->profile = encodeSettings.stProtocolInfo.eProfile;
    pSettings->level = encodeSettings.stProtocolInfo.eLevel;
    pSettings->stcChannel = NULL;
    pSettings->numParallelEncodes = encodeSettings.uiNumParallelNRTEncodes;
    pSettings->entropyCoding = encodeSettings.eForceEntropyCoding;
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);

    pSettings->segmentModeRateControl.enable = encodeSettings.stRateControl.stSegmentMode.bEnable;
    pSettings->segmentModeRateControl.duration = encodeSettings.stRateControl.stSegmentMode.uiDuration;
    pSettings->segmentModeRateControl.upperTolerance = encodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance;
    pSettings->segmentModeRateControl.lowerTolerance = encodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance;
    pSettings->memoryBandwidthSaving.singleRefP = encodeSettings.stMemoryBandwidthSaving.bSingleRefP;
    pSettings->memoryBandwidthSaving.requiredPatchesOnly = encodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly;

    return;
}

static NEXUS_Error
NEXUS_VideoEncoder_P_ConvertSettings(const NEXUS_VideoEncoderSettings *pSettings, BVCE_Channel_EncodeSettings *encodeSettings)
{
    BERR_Code rc;

    BVCE_Channel_GetDefaultEncodeSettings(g_pCoreHandles->box, encodeSettings);
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(pSettings->frameRate, &encodeSettings->stFrameRate.eFrameRate);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    encodeSettings->stFrameRate.bVariableFrameRateMode = pSettings->variableFrameRate;
    encodeSettings->bITFPEnable = pSettings->enableFieldPairing;
    encodeSettings->stBitRate.uiMax = pSettings->bitrateMax;
    encodeSettings->stBitRate.uiTarget = pSettings->bitrateTarget;
    encodeSettings->stGOPStructure.bAllowNewGOPOnSceneChange = pSettings->streamStructure.newGopOnSceneChange;
    encodeSettings->stGOPStructure.bAllowOpenGOP = pSettings->streamStructure.openGop;
    encodeSettings->stGOPStructure.uiDuration = pSettings->streamStructure.duration;
    if(pSettings->streamStructure.adaptiveDuration) {
        const char *gopRampFactor = NEXUS_GetEnv("BVCE_GopRampFactor");
        if(gopRampFactor) {
            encodeSettings->stGOPStructure.uiDurationRampUpFactor = NEXUS_atoi(gopRampFactor);
        } else {
            encodeSettings->stGOPStructure.uiDurationRampUpFactor = 4;
        }
    }
    encodeSettings->stGOPStructure.uiNumberOfPFrames = pSettings->streamStructure.framesP;
    encodeSettings->stGOPStructure.uiNumberOfBFrames = pSettings->streamStructure.framesB;
    encodeSettings->uiA2PDelay = pSettings->encoderDelay;
    return BERR_SUCCESS;
}

NEXUS_Error
NEXUS_VideoEncoder_P_ConvertStartSettings(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderStartSettings *pSettings, BVCE_Channel_StartEncodeSettings *startEncodeSettings)
{
    BERR_Code rc;
    unsigned stcChannelIndex;

    if(pSettings->stcChannel == NULL) {rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto error;}

    BVCE_Channel_GetDefaultStartEncodeSettings(g_pCoreHandles->box, startEncodeSettings);
    startEncodeSettings->stProtocolInfo.eProtocol = NEXUS_P_VideoCodec_ToMagnum(pSettings->codec, NEXUS_TransportType_eEs);
    startEncodeSettings->stProtocolInfo.eLevel    = pSettings->level;
    startEncodeSettings->stProtocolInfo.eProfile  = pSettings->profile;
    startEncodeSettings->bNonRealTimeEncodeMode   = pSettings->nonRealTime;
    startEncodeSettings->bPipelineLowDelayMode    = pSettings->lowDelayPipeline;
    startEncodeSettings->bAdaptiveLowDelayMode    = pSettings->adaptiveLowDelayMode;
    startEncodeSettings->uiRateBufferDelay        = pSettings->rateBufferDelay;
    startEncodeSettings->eForceEntropyCoding      = pSettings->entropyCoding;
    if((pSettings->bounds.inputDimension.max.width > encoder->openSettings.memoryConfig.maxWidth) ||
       (pSettings->bounds.inputDimension.max.height > encoder->openSettings.memoryConfig.maxHeight)) {
        BDBG_ERR(("bound size: %ux%u > open settings memory alloc size: %ux%u",
            pSettings->bounds.inputDimension.max.width, pSettings->bounds.inputDimension.max.height,
            encoder->openSettings.memoryConfig.maxWidth, encoder->openSettings.memoryConfig.maxHeight));
        rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto error;
    }
    startEncodeSettings->stBounds.stDimensions.stMax.uiWidth = pSettings->bounds.inputDimension.max.width;
    startEncodeSettings->stBounds.stDimensions.stMax.uiHeight = pSettings->bounds.inputDimension.max.height;
    startEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiWidth = pSettings->bounds.inputDimension.maxInterlaced.width;
    startEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiHeight = pSettings->bounds.inputDimension.maxInterlaced.height;
    startEncodeSettings->stBounds.stBitRate.stLargest.uiMax = pSettings->bounds.bitrate.upper.bitrateMax;
    startEncodeSettings->stBounds.stBitRate.stLargest.uiTarget = pSettings->bounds.bitrate.upper.bitrateTarget;
    startEncodeSettings->stBounds.stGOPStructure.uiNumberOfBFrames = pSettings->bounds.streamStructure.max.framesB;
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(pSettings->bounds.outputFrameRate.min, &startEncodeSettings->stBounds.stFrameRate.eMin);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(pSettings->bounds.outputFrameRate.max, &startEncodeSettings->stBounds.stFrameRate.eMax);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(pSettings->bounds.inputFrameRate.min, &startEncodeSettings->stBounds.stInputFrameRate.eMin);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    startEncodeSettings->eInputType = pSettings->interlaced ? BAVC_ScanType_eInterlaced : BAVC_ScanType_eProgressive;
    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.transport);
    NEXUS_StcChannel_GetIndex_priv(pSettings->stcChannel, &stcChannelIndex);
    NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.transport);
    startEncodeSettings->uiStcIndex = stcChannelIndex;
    startEncodeSettings->uiNumParallelNRTEncodes = pSettings->numParallelEncodes;

    return BERR_SUCCESS;

error:
    return rc;
}


static NEXUS_Error
NEXUS_VideoEncoder_P_ApplySettings(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings)
{
    BVCE_Channel_EncodeSettings encodeSettings;
    BERR_Code rc;

    rc = NEXUS_VideoEncoder_P_ConvertSettings(pSettings, &encodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    rc = BVCE_Channel_SetEncodeSettings(encoder->enc, &encodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_Start(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderStartSettings *pSettings)
{
    BERR_Code rc;
    BVCE_Channel_StartEncodeSettings startEncodeSettings;
    BXUDlib_Settings xudSettings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);

    if (encoder->started) {
        BDBG_ERR(("VideoEncoder already started"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    xudSettings.sinkInterface.pPrivateSinkContext = (void *)encoder->enc;
    xudSettings.sinkInterface.userDataAdd_isr = (BXUDlib_UserDataSink_Add)BVCE_Channel_UserData_AddBuffers_isr;
    xudSettings.sinkInterface.userDataStatus_isr = (BXUDlib_UserDataSink_Status)BVCE_Channel_UserData_GetStatus_isr;

    rc = NEXUS_VideoEncoder_P_ConvertStartSettings(encoder, pSettings, &startEncodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    rc = NEXUS_VideoEncoder_P_ApplySettings(encoder, &encoder->settings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    NEXUS_IsrCallback_Set(encoder->dataReadyCallback, &pSettings->dataReady);

    startEncodeSettings.stRateControl.stSegmentMode.bEnable = pSettings->segmentModeRateControl.enable;
    startEncodeSettings.stRateControl.stSegmentMode.uiDuration = pSettings->segmentModeRateControl.duration;
    startEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance = pSettings->segmentModeRateControl.upperTolerance;
    startEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance = pSettings->segmentModeRateControl.lowerTolerance;
    startEncodeSettings.stMemoryBandwidthSaving.bSingleRefP = pSettings->memoryBandwidthSaving.singleRefP;
    startEncodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly = pSettings->memoryBandwidthSaving.requiredPatchesOnly;

    rc = BVCE_Channel_StartEncode(encoder->enc, &startEncodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    encoder->startSettings = *pSettings;

    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.display);
    if(encoder->settings.streamStructure.adaptiveDuration && encoder->settings.streamStructure.duration) {
        const char *resolRamp = NEXUS_GetEnv("STG_ResolutionRampCount");
        if(resolRamp) {
            unsigned rampCount = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(encoder->settings.frameRate)/1000 *
                encoder->settings.streamStructure.duration/1000;
            BDBG_MSG(("video encoder %u started with video rampup count = %u", encoder->index, rampCount));
            rc = NEXUS_DisplayModule_SetStgResolutionRamp_priv(pSettings->input, rampCount);
            if (rc) { BERR_TRACE(rc); goto unlock; }
        }
    }
    rc = NEXUS_DisplayModule_SetUserDataEncodeMode_priv(pSettings->input, pSettings->encodeUserData, &xudSettings, pSettings->window);
    if (rc) {
        BERR_TRACE(rc);
        /* fall through unlock */
    }
    else {
        rc = NEXUS_VideoWindow_BypassVideoProcessing_priv(pSettings->input, pSettings->window, pSettings->bypassVideoProcessing);
        if (rc) {
            BERR_TRACE(rc);
            /* fall through unlock */
        }
    }
unlock:
    NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.display);
    if(rc!=BERR_SUCCESS) goto error;

    encoder->started = true;
    return NEXUS_SUCCESS;
error:
    return rc;
}

void
NEXUS_VideoEncoder_GetDefaultStopSettings(NEXUS_VideoEncoderStopSettings *pSettings)
{
    BVCE_Channel_StopEncodeSettings stopEncodeSettings;

    BDBG_ASSERT(pSettings);
    BDBG_CASSERT((unsigned)NEXUS_VideoEncoderStopMode_eNormal==(unsigned)BVCE_Channel_StopMode_eNormal);
    BDBG_CASSERT((unsigned)NEXUS_VideoEncoderStopMode_eImmediate==(unsigned)BVCE_Channel_StopMode_eImmediate);
    BDBG_CASSERT((unsigned)NEXUS_VideoEncoderStopMode_eAbort==(unsigned)BVCE_Channel_StopMode_eAbort);
    BDBG_CWARNING((unsigned)NEXUS_VideoEncoderStopMode_eMax==(unsigned)BVCE_Channel_StopMode_eMax);
    BVCE_Channel_GetDefaultStopEncodeSettings(&stopEncodeSettings);
    pSettings->mode = stopEncodeSettings.eStopMode;
    return;
}

void
NEXUS_VideoEncoder_Stop( NEXUS_VideoEncoderHandle encoder,const NEXUS_VideoEncoderStopSettings *pSettings)
{
    BERR_Code rc;
    BVCE_Channel_StopEncodeSettings stopSettings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);

    if (!encoder->started) {
        BDBG_ERR(("VideoEncoder not started yet!"));
        return;
    }
    NEXUS_IsrCallback_Set(encoder->dataReadyCallback, NULL);
    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.display);
    rc = NEXUS_DisplayModule_SetUserDataEncodeMode_priv(encoder->startSettings.input, false, NULL, NULL);
    NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.display);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    BVCE_Channel_GetDefaultStopEncodeSettings(&stopSettings);
    if(pSettings) {
        stopSettings.eStopMode = pSettings->mode;
    }
    rc = BVCE_Channel_StopEncode(encoder->enc, &stopSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    encoder->started = false;
    return;
}

void
NEXUS_VideoEncoder_GetSettings(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);
    *pSettings = encoder->settings;
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_SetSettings( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);

    rc = NEXUS_VideoEncoder_P_ApplySettings(encoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    encoder->settings = *pSettings;

    return NEXUS_SUCCESS;
error:
    return rc;
}



NEXUS_Error
NEXUS_VideoEncoder_GetBuffer( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderDescriptor **pBuffer, size_t *pSize, const NEXUS_VideoEncoderDescriptor **pBuffer2, size_t *pSize2)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pSize);
    BDBG_ASSERT(pBuffer2);
    BDBG_ASSERT(pSize2);

    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, flags, BAVC_VideoBufferDescriptor, stCommon.uiFlags);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, originalPts, BAVC_VideoBufferDescriptor, stCommon.uiOriginalPTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, pts, BAVC_VideoBufferDescriptor, stCommon.uiPTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, stcSnapshot, BAVC_VideoBufferDescriptor, stCommon.uiSTCSnapshot);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, escr, BAVC_VideoBufferDescriptor, stCommon.uiESCR);

    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, ticksPerBit, BAVC_VideoBufferDescriptor, stCommon.uiTicksPerBit);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, shr, BAVC_VideoBufferDescriptor, stCommon.iSHR);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, offset, BAVC_VideoBufferDescriptor, stCommon.uiOffset);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, length, BAVC_VideoBufferDescriptor, stCommon.uiLength);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, videoFlags, BAVC_VideoBufferDescriptor, uiVideoFlags);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, dts, BAVC_VideoBufferDescriptor, uiDTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, dataUnitType, BAVC_VideoBufferDescriptor, uiDataUnitType);
    NEXUS_ASSERT_STRUCTURE(NEXUS_VideoEncoderDescriptor, BAVC_VideoBufferDescriptor);

    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS );
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA );
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_RAP == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START);


    rc = BVCE_Channel_Output_GetBufferDescriptors(encoder->enc, (void *)pBuffer, pSize, (void *)pBuffer2, pSize2);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_ReadComplete(NEXUS_VideoEncoderHandle encoder, unsigned descriptorsCompleted)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);

    rc = BVCE_Channel_Output_ConsumeBufferDescriptors(encoder->enc , descriptorsCompleted);

    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_GetStatus(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderStatus *pStatus)
{
    NEXUS_Error rc;
    BVCE_Channel_Status status;
    BVCE_VersionInfo versionInfo;
    BVCE_Debug_FifoInfo fifoInfo;

    BAVC_VideoBufferStatus bufferStatus;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);

    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_INVALID_INPUT_DIMENSION        == NEXUS_VIDEOENCODER_ERROR_INVALID_INPUT_DIMENSION);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_LATE                 == NEXUS_VIDEOENCODER_ERROR_USER_DATA_LATE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_DUPLICATE            == NEXUS_VIDEOENCODER_ERROR_USER_DATA_DUPLICATE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_ADJUSTS_WRONG_FRAME_RATE       == NEXUS_VIDEOENCODER_ERROR_ADJUSTS_WRONG_FRAME_RATE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_BVN_FRAME_RATE     == NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_BVN_FRAME_RATE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_RESOLUTION         == NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_RESOLUTION);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_MIN_FRAME_RATE    == NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_MIN_FRAME_RATE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_PIC_RESOLUTION    == NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_PIC_RESOLUTION);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_BITRATE_EXCEEDED           == NEXUS_VIDEOENCODER_ERROR_MAX_BITRATE_EXCEEDED);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_BIN_BUFFER_FULL                == NEXUS_VIDEOENCODER_ERROR_BIN_BUFFER_FULL);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL                       == NEXUS_VIDEOENCODER_ERROR_CDB_FULL);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL     == NEXUS_VIDEOENCODER_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_EBM_FULL                             == NEXUS_VIDEOENCODER_ERROR_EBM_FULL);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_SLICES_EXCEEDED              == NEXUS_VIDEOENCODER_ERROR_MAX_NUM_SLICES_EXCEEDED);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED  == NEXUS_VIDEOENCODER_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION    == NEXUS_VIDEOENCODER_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MBARC_BOOT_FAILURE                   == NEXUS_VIDEOENCODER_ERROR_MBARC_BOOT_FAILURE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG      == NEXUS_VIDEOENCODER_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_CRITICAL                             == NEXUS_VIDEOENCODER_ERROR_CRITICAL);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE == NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE == NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED    == NEXUS_VIDEOENCODER_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED);

    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_EVENT_INPUT_CHANGE                   == NEXUS_VIDEOENCODER_EVENT_INPUT_CHANGE);
    BDBG_CASSERT(BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS                            == NEXUS_VIDEOENCODER_EVENT_EOS);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&bufferStatus, 0, sizeof(bufferStatus));
    rc = BVCE_Channel_Output_GetBufferStatus(encoder->enc, &bufferStatus);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    BDBG_MODULE_MSG(nexus_video_encoder_status, ("base addr@%p : metadata base addr@%p", bufferStatus.stCommon.pFrameBufferBaseAddress, bufferStatus.stCommon.pMetadataBufferBaseAddress));
    /* create persistent NEXUS_MemoryBlock <-> BMMA_Block mapping */
    if ( NULL != bufferStatus.stCommon.hFrameBufferBlock ) {
        if(encoder->frame.nexus==NULL) {
            NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.core);
            encoder->frame.nexus = NEXUS_MemoryBlock_FromMma_priv( bufferStatus.stCommon.hFrameBufferBlock );
            NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.core);
            if(encoder->frame.nexus==NULL) {
                rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;
            }
            encoder->frame.mma = bufferStatus.stCommon.hFrameBufferBlock;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, encoder->frame.nexus, Acquire);
        }
        if(encoder->frame.mma != bufferStatus.stCommon.hFrameBufferBlock) {
            rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        pStatus->bufferBlock = encoder->frame.nexus;
    }
    if ( NULL != bufferStatus.stCommon.hMetadataBufferBlock ) {
        if(encoder->meta.nexus==NULL) {
            NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.core);
            encoder->meta.nexus = NEXUS_MemoryBlock_FromMma_priv( bufferStatus.stCommon.hMetadataBufferBlock);
            NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.core);
            if(encoder->meta.nexus==NULL) {
                rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;
            }
            encoder->meta.mma = bufferStatus.stCommon.hMetadataBufferBlock;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, encoder->meta.nexus, Acquire);
        }
        if(encoder->meta.mma != bufferStatus.stCommon.hMetadataBufferBlock) {
            rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        pStatus->metadataBufferBlock = encoder->meta.nexus;
    }
    pStatus->data.fifoDepth  = bufferStatus.stCommon.stCDB.uiDepth;
    pStatus->data.fifoSize   = bufferStatus.stCommon.stCDB.uiSize;
    pStatus->index.fifoDepth = bufferStatus.stCommon.stITB.uiDepth;
    pStatus->index.fifoSize  = bufferStatus.stCommon.stITB.uiSize;
    rc = BVCE_Channel_GetStatus(encoder->enc, &status);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    pStatus->errorFlags = status.uiErrorFlags;
    pStatus->eventFlags = status.uiEventFlags;
    pStatus->errorCount = status.uiTotalErrors;
    pStatus->picturesReceived = status.uiTotalPicturesReceived;
    pStatus->picturesDroppedFRC = status.uiTotalPicturesDroppedDueToFrameRateConversion;
    pStatus->picturesDroppedHRD = status.uiTotalPicturesDroppedDueToHRDUnderflow;
    pStatus->picturesDroppedErrors = status.uiTotalPicturesDroppedDueToErrors;
    pStatus->picturesEncoded = status.uiTotalPicturesEncoded;
    pStatus->pictureIdLastEncoded = status.uiLastPictureIdEncoded;
    pStatus->picturesPerSecond = status.uiAverageFramesPerSecond;
    rc = BVCE_GetVersionInfo(encoder->device->vce, &versionInfo);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    pStatus->version.firmware = versionInfo.uiFirmwareVersion;

    rc = BVCE_Debug_GetLogMessageFifo(encoder->device->vce, &fifoInfo);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    if ( NULL != fifoInfo.hBlock) {
        if(encoder->debugLog.nexus==NULL) {
            NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.core);
            encoder->debugLog.nexus = NEXUS_MemoryBlock_FromMma_priv(fifoInfo.hBlock);
            NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.core);
            if(encoder->debugLog.nexus==NULL) {
                rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;
            }
            encoder->debugLog.mma = fifoInfo.hBlock;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, encoder->debugLog.nexus, Acquire);
        }
        if(encoder->debugLog.mma != fifoInfo.hBlock) {
            rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        pStatus->debugLog.buffer = encoder->debugLog.nexus;
    }
    pStatus->debugLog.elementSize = fifoInfo.uiElementSize;
    pStatus->debugLog.offset = fifoInfo.uiOffset;

    return NEXUS_SUCCESS;
error:
    return BERR_TRACE(rc);
}

void NEXUS_VideoEncoder_GetDefaultClearStatus(NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BDBG_ASSERT(pClearStatus);
    pClearStatus->errorFlags =
        NEXUS_VIDEOENCODER_ERROR_INVALID_INPUT_DIMENSION |
        NEXUS_VIDEOENCODER_ERROR_USER_DATA_LATE |
        NEXUS_VIDEOENCODER_ERROR_USER_DATA_DUPLICATE |
        NEXUS_VIDEOENCODER_ERROR_ADJUSTS_WRONG_FRAME_RATE |
        NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_BVN_FRAME_RATE |
        NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_RESOLUTION |
        NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_MIN_FRAME_RATE |
        NEXUS_VIDEOENCODER_ERROR_MISMATCH_BVN_PIC_RESOLUTION |
        NEXUS_VIDEOENCODER_ERROR_MAX_BITRATE_EXCEEDED |
        NEXUS_VIDEOENCODER_ERROR_BIN_BUFFER_FULL |
        NEXUS_VIDEOENCODER_ERROR_CDB_FULL |
        NEXUS_VIDEOENCODER_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL |
        NEXUS_VIDEOENCODER_ERROR_EBM_FULL |
        NEXUS_VIDEOENCODER_ERROR_MAX_NUM_SLICES_EXCEEDED |
        NEXUS_VIDEOENCODER_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED |
        NEXUS_VIDEOENCODER_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION |
        NEXUS_VIDEOENCODER_ERROR_MBARC_BOOT_FAILURE |
        NEXUS_VIDEOENCODER_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG |
        NEXUS_VIDEOENCODER_ERROR_CRITICAL |
        NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE |
        NEXUS_VIDEOENCODER_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE |
        NEXUS_VIDEOENCODER_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED;

    pClearStatus->eventFlags = NEXUS_VIDEOENCODER_EVENT_INPUT_CHANGE | NEXUS_VIDEOENCODER_EVENT_EOS;

    return;
}

void NEXUS_VideoEncoder_ClearStatus(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pClearStatus);
    if(pClearStatus==NULL) {
        rc = BVCE_Channel_ClearStatus(encoder->enc, NULL);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    } else {
        BVCE_Channel_Status status;
        BKNI_Memset(&status, 0, sizeof(status));
        status.uiErrorFlags = pClearStatus->errorFlags;
        status.uiEventFlags = pClearStatus->eventFlags;
        rc = BVCE_Channel_ClearStatus(encoder->enc, &status);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    }
error:
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_GetDelayRange(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings, const NEXUS_VideoEncoderStartSettings *pStartSettings, NEXUS_VideoEncoderDelayRange *pDelayRange)
{
    BERR_Code rc;
    BVCE_Channel_StartEncodeSettings startEncodeSettings;
    BVCE_Channel_EncodeSettings encodeSettings;
    BVCE_A2PDelay A2PDelay;


    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pStartSettings);
    BDBG_ASSERT(pDelayRange);


    rc = NEXUS_VideoEncoder_P_ConvertStartSettings(encoder, pStartSettings, &startEncodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    rc = NEXUS_VideoEncoder_P_ConvertSettings(pSettings, &encodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    rc = BVCE_GetA2PDelayInfo(&encodeSettings, &startEncodeSettings, &A2PDelay);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    pDelayRange->min = A2PDelay.uiMin;
    pDelayRange->max = A2PDelay.uiMax;

    return NEXUS_SUCCESS;

error:
    return rc;
}

void
NEXUS_VideoEncoder_GetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BVCE_Channel_EncodeSettings_OnInputChange settings;
    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);

    BKNI_Memset(&settings, 0, sizeof(settings));
    BVCE_Channel_GetEncodeSettings_OnInputChange(encoder->enc, &settings);
    pSettings->bitrateMax = settings.stBitRate.uiMax;
    pSettings->bitrateTarget = settings.stBitRate.uiTarget;

    return;
}

NEXUS_Error NEXUS_VideoEncoder_SetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BVCE_Channel_EncodeSettings_OnInputChange settings;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);
    BVCE_Channel_GetEncodeSettings_OnInputChange(encoder->enc, &settings);
    settings.stBitRate.uiMax = pSettings->bitrateMax ;
    settings.stBitRate.uiTarget  = pSettings->bitrateTarget ;
    rc = BVCE_Channel_SetEncodeSettings_OnInputChange(encoder->enc, &settings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_VideoEncoder_InsertRandomAccessPoint(NEXUS_VideoEncoderHandle encoder)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    rc = BVCE_Channel_BeginNewRAP(encoder->enc);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

void NEXUS_GetVideoEncoderCapabilities( NEXUS_VideoEncoderCapabilities *pCapabilities )
{
#if BCHP_CHIP == 7425
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));
    pCapabilities->videoEncoder[0].supported = true;
    pCapabilities->videoEncoder[0].displayIndex = 3;
    pCapabilities->videoEncoder[0].memory = g_moduleSettings.videoEncoder[0].memory;
    pCapabilities->videoEncoder[1].supported = true;
    pCapabilities->videoEncoder[1].displayIndex = 2;
    pCapabilities->videoEncoder[1].memory = g_moduleSettings.videoEncoder[1].memory;
#else
    unsigned i, j;
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));
    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        unsigned device = g_moduleSettings.vceMapping[i].device;
        unsigned channel = g_moduleSettings.vceMapping[i].channel;
        pCapabilities->videoEncoder[i].supported = g_pCoreHandles->boxConfig->stVce.stInstance[device].uiChannels & (1 << channel);
        if (!pCapabilities->videoEncoder[i].supported) continue;
        for (j=0;j<BBOX_VDC_DISPLAY_COUNT;j++) {
            if (g_pCoreHandles->boxConfig->stVdc.astDisplay[j].stStgEnc.ulEncoderCoreId == device &&
                g_pCoreHandles->boxConfig->stVdc.astDisplay[j].stStgEnc.ulEncoderChannel == channel)
            {
                pCapabilities->videoEncoder[i].displayIndex = j;
                break;
            }
        }
        pCapabilities->videoEncoder[i].memory = g_moduleSettings.videoEncoder[i].memory;
        pCapabilities->videoEncoder[i].deviceIndex = device;
        pCapabilities->videoEncoder[i].channelIndex = channel;
        BDBG_MSG(("videoEncoder[%u] display%u %ux%u%c", i, j,
            pCapabilities->videoEncoder[i].memory.maxWidth, pCapabilities->videoEncoder[i].memory.maxHeight, pCapabilities->videoEncoder[i].memory.interlaced?'i':'p'));
    }
#endif
}

unsigned NEXUS_VideoEncoder_GetIndex_isrsafe(NEXUS_VideoEncoderHandle encoder)
{
    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    return encoder->index;
}
