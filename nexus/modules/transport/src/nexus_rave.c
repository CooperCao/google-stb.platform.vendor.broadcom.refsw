/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
#include "nexus_transport_module.h"
#include "biobits.h"
#if NEXUS_SW_RAVE_PEEK_EXTENSION
/* The NEXUS_SW_RAVE_PEEK_EXTENSION forces SW RAVE in ePointersOnly mode.
This is only supported for codecs that don't require SW RAVE for ITB manipulation (e.g. MPEG, AVC).
By not copying ITB, it requires minimal memory.
It allows the extension to peek at the ITB before it goes to the decoder. */
#include "nexus_sw_rave_extension.h"
#endif

BDBG_MODULE(nexus_rave);

#define NEXUS_RAVE_CONTEXT_MAP(RAVE) (((RAVE)->swRave.raveHandle && (RAVE)->swRave.enabled) ? &(RAVE)->swRave.xptContextMap : &(RAVE)->xptContextMap)

/* for SW rave, we need 100 msec. for numBytesOutput, 250 msec is enough (a 20Mbps stream and 1.5MB CDB will wrap every 800 msec) */
#define NEXUS_RAVE_TIMER_PERIOD(RAVE) ((RAVE)->swRave.enabled?100:250)

static int nexus_rave_add_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannel *pidChannel);
static void nexus_rave_remove_pid(NEXUS_RaveHandle rave);

void NEXUS_Rave_GetDefaultOpenSettings_priv(NEXUS_RaveOpenSettings *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

static void NEXUS_RaveErrorCounter_Reenable_priv(void *arg)
{
    struct NEXUS_Rave_P_ErrorCounter *r = arg;
    r->rave_overflow_int_timer = NULL;
    (void)BINT_EnableCallback(r->cdbOverflow_int);
    (void)BINT_EnableCallback(r->itbOverflow_int);
}

static void
NEXUS_Rave_P_ErrorCounterInt_isr(void *context, int param)
{
    struct NEXUS_Rave_P_ErrorCounter *r = context;
    BXPT_RaveIntName intName = param;
    NEXUS_Error rc;
    bool disabled = false;

    switch (intName) {
        case BXPT_RaveIntName_eEmuErr:
            r->emuErr++;
            break;
        case BXPT_RaveIntName_ePusiErr:
            r->pusiErr++;
            break;
        case BXPT_RaveIntName_eTeiErr:
            r->teiErr++;
            break;
        case BXPT_RaveIntName_eCcErr:
            r->ccErr++;
            break;
        /* overflow errors are generated one per packet. disable on first interrupt, then re-enable later */
        case BXPT_RaveIntName_eCdbOverflow:
            r->cdbOverflow++;
            rc = BINT_DisableCallback_isr(r->cdbOverflow_int);
            if (rc) { rc=BERR_TRACE(rc); }
            disabled = true;
            break;
        case BXPT_RaveIntName_eItbOverflow:
            r->itbOverflow++;
            rc = BINT_DisableCallback_isr(r->itbOverflow_int);
            if (rc) { rc=BERR_TRACE(rc); }
            disabled = true;
            break;
        default:
            break;
    }

    if (disabled) {
        BKNI_SetEvent(r->isrEvent);
     }
}

static void NEXUS_Rave_P_ErrorCounter_IsrEvent(void *arg)
{
    struct NEXUS_Rave_P_ErrorCounter *r = arg;
    if (!r->rave_overflow_int_timer) {
        r->rave_overflow_int_timer = NEXUS_ScheduleTimer(2000, NEXUS_RaveErrorCounter_Reenable_priv, arg);
    }
}

NEXUS_Error NEXUS_RaveErrorCounter_Init_priv(struct NEXUS_Rave_P_ErrorCounter *r, BXPT_RaveCx_Handle rave)
{
    NEXUS_Error rc;
    BINT_Id int_id;

    r->rave = rave;

    rc = BKNI_CreateEvent(&r->isrEvent);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    r->isrEventCallback = NEXUS_RegisterEvent(r->isrEvent, NEXUS_Rave_P_ErrorCounter_IsrEvent, r);
    if (!r->isrEventCallback) { rc=BERR_TRACE(rc); goto error; }

    NEXUS_RaveErrorCounter_Reset_priv(r);

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_eEmuErr, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->emuErr_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_eEmuErr);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->emuErr_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_ePusiErr, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->pusiErr_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_ePusiErr);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->pusiErr_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_eTeiErr, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->teiErr_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_eTeiErr);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->teiErr_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_eCcErr, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->ccErr_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_eCcErr);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->ccErr_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_eCdbOverflow, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->cdbOverflow_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_eCdbOverflow);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->cdbOverflow_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BXPT_Rave_GetIntId(r->rave, BXPT_RaveIntName_eItbOverflow, &int_id);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_CreateCallback(&r->itbOverflow_int, g_pCoreHandles->bint, int_id, NEXUS_Rave_P_ErrorCounterInt_isr, r, BXPT_RaveIntName_eItbOverflow);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(r->itbOverflow_int);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    return NEXUS_SUCCESS;

error:
    NEXUS_RaveErrorCounter_Uninit_priv(r);
    return rc;
}

void NEXUS_RaveErrorCounter_Reset_priv(struct NEXUS_Rave_P_ErrorCounter *r)
{
    r->emuErr = 0;
    r->pusiErr = 0;
    r->teiErr = 0;
    r->ccErr = 0;
    r->cdbOverflow = 0;
    r->itbOverflow = 0;
}

void NEXUS_RaveErrorCounter_Uninit_priv(struct NEXUS_Rave_P_ErrorCounter *r)
{
    if (r->rave_overflow_int_timer) {
        NEXUS_CancelTimer(r->rave_overflow_int_timer);
        r->rave_overflow_int_timer = NULL;
    }
    if (r->emuErr_int) {
        BINT_DisableCallback(r->emuErr_int);
        BINT_DestroyCallback(r->emuErr_int);
        r->emuErr_int = NULL;
    }
    if (r->pusiErr_int) {
        BINT_DisableCallback(r->pusiErr_int);
        BINT_DestroyCallback(r->pusiErr_int);
        r->pusiErr_int = NULL;
    }
    if (r->teiErr_int) {
        BINT_DisableCallback(r->teiErr_int);
        BINT_DestroyCallback(r->teiErr_int);
        r->teiErr_int = NULL;
    }
    if (r->ccErr_int) {
        BINT_DisableCallback(r->ccErr_int);
        BINT_DestroyCallback(r->ccErr_int);
        r->ccErr_int = NULL;
    }
    if (r->cdbOverflow_int) {
        BINT_DisableCallback(r->cdbOverflow_int);
        BINT_DestroyCallback(r->cdbOverflow_int);
        r->cdbOverflow_int = NULL;
    }
    if (r->itbOverflow_int) {
        BINT_DisableCallback(r->itbOverflow_int);
        BINT_DestroyCallback(r->itbOverflow_int);
        r->itbOverflow_int = NULL;
    }
    if (r->isrEventCallback) {
        NEXUS_UnregisterEvent(r->isrEventCallback);
        r->isrEventCallback = NULL;
    }
    if (r->isrEvent) {
        BKNI_DestroyEvent(r->isrEvent);
        r->isrEvent = NULL;
    }
}

static bool NEXUS_Rave_P_RequiresSwRave(NEXUS_TransportType transportType, NEXUS_VideoCodec codec)
{
    switch(transportType) {
    case NEXUS_TransportType_eUnknown:
    case NEXUS_TransportType_eAvi: /* DivX */
    case NEXUS_TransportType_eAsf:
    case NEXUS_TransportType_eMkv:
        switch (codec) {
        case NEXUS_VideoCodec_eMpeg4Part2:
        case NEXUS_VideoCodec_eDivx311:
        case NEXUS_VideoCodec_eVc1SimpleMain:
            return true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

NEXUS_Error NEXUS_Rave_P_UseSecureHeap(NEXUS_HeapHandle heap, bool useSecureHeap, bool *pUseSecureHeap)
{
    /* validate use of CRR (secure heap) */
    if (heap && heap == pTransport->settings.secureHeap) {
        /* the primary method is the user selecting the CRR explicitly */
        *pUseSecureHeap = true;
    }
    else if (useSecureHeap && pTransport->settings.secureHeap) {
        /* for backward compat, we allow a boolean as long as CRR is enabled and there's no mismatch */
        if (heap && heap != pTransport->settings.secureHeap) {
            BDBG_ERR(("secure heap misconfiguration: %p %p", (void *)pTransport->settings.secureHeap, (void *)heap));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        *pUseSecureHeap = true;
    }
    else {
        *pUseSecureHeap = false;
    }
    return 0;
}

NEXUS_RaveHandle NEXUS_Rave_Open_priv(const NEXUS_RaveOpenSettings *pSettings)
{
    NEXUS_RaveHandle rave = NULL;
    BERR_Code rc;
    unsigned i;
    bool allocSoftRave = false;
    bool useSecureHeap;
    BXPT_Rave_AllocCxSettings allocSettings;

    NEXUS_ASSERT_MODULE();

    rc = NEXUS_Rave_P_UseSecureHeap(pSettings->heap, false, &useSecureHeap);
    if (rc) return NULL;

    for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
        /* rave[0] means we are hardcoded for only one RAVE "channel", but many "contexts" */
        if (!pTransport->rave[0].context[i]) {
            break;
        }
    }
    if (i == BXPT_NUM_RAVE_CONTEXTS) {
        BDBG_ERR(("No more RAVE contexts"));
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    rave = BKNI_Malloc(sizeof(*rave));
    if (!rave) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(rave, 0, sizeof(*rave));
    BDBG_OBJECT_SET(rave, NEXUS_Rave);
    pTransport->rave[0].context[i] = rave;
    rave->array_index = i;
    rave->pidChannel = NULL;
    rave->numOutputBytes = 0;
    rave->useSecureHeap = useSecureHeap;

    rave->itb.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;
    rave->swRave.heap = rave->itb.heap;
    if (pSettings->heap) {
        rave->cdb.heap = NEXUS_Heap_GetMmaHandle(pSettings->heap);
    }
    else if (useSecureHeap) {
        rave->cdb.heap = pTransport->settings.secureHeap ? NEXUS_Heap_GetMmaHandle(pTransport->settings.secureHeap) : NULL;
    }
    else {
        rave->cdb.heap = rave->itb.heap;
    }
    if (!rave->cdb.heap || !rave->itb.heap) {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    BXPT_Rave_GetDefaultAllocCxSettings(&allocSettings);
    allocSettings.RequestedType = pSettings->record?BXPT_RaveCx_eRecord:(useSecureHeap?BXPT_RaveCx_eAvR:BXPT_RaveCx_eAv);
    allocSettings.BufferCfg = pSettings->config;
    allocSettings.BufferCfg.Cdb.Alignment = 8; /* non-configurable 256 byte alignment */
    allocSettings.BufferCfg.Itb.Alignment = 7; /* non-configurable 128 byte alignment */
    if (pSettings->config.Itb.Length % 128) {
        BDBG_ERR(("Itb.Length %#x is not a multiple of 128 bytes", pSettings->config.Itb.Length));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    rave->cdb.block = allocSettings.CdbBlock = BMMA_Alloc(rave->cdb.heap, pSettings->config.Cdb.Length, 1 << allocSettings.BufferCfg.Cdb.Alignment, NULL);
    if (!rave->cdb.block) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
    rave->cdb.ptr = BMMA_Lock(rave->cdb.block);
    rave->itb.block = allocSettings.ItbBlock = BMMA_Alloc(rave->itb.heap, pSettings->config.Itb.Length, 1 << allocSettings.BufferCfg.Itb.Alignment, NULL);
    if (!rave->itb.block) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
    rave->itb.ptr = BMMA_Lock(rave->itb.block);
#if BXPT_P_HAS_AVS_PLUS_WORKAROUND
    if (pSettings->supportedCodecs[NEXUS_VideoCodec_eAvs]) {
        BXPT_Rave_AllocCxSettings avsAllocSettings = allocSettings;
        rave->avsReference.cdbBlock = avsAllocSettings.CdbBlock = BMMA_Alloc(rave->cdb.heap, pSettings->config.Cdb.Length, 1 << allocSettings.BufferCfg.Cdb.Alignment, NULL);
        if (!rave->avsReference.cdbBlock) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
        rave->avsReference.cdb_ptr = BMMA_Lock(rave->avsReference.cdbBlock);
        rave->avsReference.itbBlock = avsAllocSettings.ItbBlock = BMMA_Alloc(rave->itb.heap, pSettings->config.Itb.Length, 1 << allocSettings.BufferCfg.Itb.Alignment, NULL);
        if (!rave->avsReference.itbBlock) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
        rave->avsReference.itb_ptr = BMMA_Lock(rave->avsReference.itbBlock);
        rc = BXPT_Rave_AllocAvsCxPair(pTransport->rave[0].channel, &allocSettings, &avsAllocSettings, &rave->raveHandle, &rave->avsReference.raveHandle);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    else
#endif
    {
        rc = BXPT_Rave_AllocCx(pTransport->rave[0].channel, &allocSettings, &rave->raveHandle);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    rc = BXPT_Rave_GetContextRegisters(rave->raveHandle, &rave->xptContextMap);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    /* because of XPT sw rave, we can't assume the HW index number. so, we derive it from the registers. */
    rave->index = NEXUS_RAVE_INDEX(rave->xptContextMap.CDB_Read);
    rave->swRave.index = -1;

#if NEXUS_SW_RAVE_SUPPORT
#if NEXUS_SW_RAVE_PEEK_EXTENSION
    BSTD_UNUSED(i);
    allocSoftRave = true;
#else
    for (i=0;i<NEXUS_VideoCodec_eMax;i++) {
        if (pSettings->supportedCodecs[i] && NEXUS_Rave_P_RequiresSwRave(NEXUS_TransportType_eUnknown, i)) {
            allocSoftRave = true;
            break;
        }
    }
#endif
    if (allocSoftRave) {
        BXPT_RaveSoftMode mode = BXPT_RaveSoftMode_eCopyItb;

#if NEXUS_SW_RAVE_PEEK_EXTENSION
        mode = BXPT_RaveSoftMode_ePointersOnly;
#endif
        BXPT_Rave_GetDefaultAllocCxSettings(&allocSettings);
        if (mode != BXPT_RaveSoftMode_ePointersOnly) {
            rave->swRave.block = allocSettings.ItbBlock = BMMA_Alloc(rave->swRave.heap, pSettings->config.Itb.Length, 1 << 8, NULL);
            if (!rave->swRave.block) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
        }
        allocSettings.SoftRaveSrcCx = rave->raveHandle;
        allocSettings.SoftRaveMode = mode;

        rc = BXPT_Rave_AllocCx(pTransport->rave[0].channel, &allocSettings, &rave->swRave.raveHandle);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        rc = BXPT_Rave_GetContextRegisters(rave->swRave.raveHandle, &rave->swRave.xptContextMap);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        rave->swRave.index = NEXUS_RAVE_INDEX(rave->swRave.xptContextMap.CDB_Read);

#if NEXUS_SW_RAVE_PEEK_EXTENSION
        rc = NEXUS_P_SwRaveExtension_Init(rave);
        if (rc) { BERR_TRACE(rc); goto error; }
#endif
    }
#endif

    rc = NEXUS_RaveErrorCounter_Init_priv(&rave->raveErrors, rave->raveHandle);
    if (rc) {
        goto error;
    }

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    {
        NEXUS_RaveCaptureCreateSettings rccSettings;
        NEXUS_RaveCapture_GetDefaultCreateSettings(&rccSettings);
        rccSettings.rave = rave;
        rave->cap = NEXUS_RaveCapture_Create(&rccSettings);
        if (rave->cap)
        {
            NEXUS_RaveCapture_Open(rave->cap);
        }
    }
#endif

    rave->openSettings = *pSettings;
    return rave;

error:
    if (rave) {
        NEXUS_Rave_Close_priv(rave);
    }
    return NULL;
}

void NEXUS_Rave_Close_priv(NEXUS_RaveHandle rave)
{
    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    if (rave->raveHandle) {
        NEXUS_Rave_Disable_priv(rave);
        NEXUS_Rave_RemovePidChannel_priv(rave);

#if NEXUS_SW_RAVE_SUPPORT
        if (rave->swRave.raveHandle) {
#if NEXUS_SW_RAVE_PEEK_EXTENSION
            NEXUS_P_SwRaveExtension_Uninit(rave);
#endif
            BXPT_Rave_FreeContext(rave->swRave.raveHandle);
            rave->swRave.raveHandle = NULL;

            if (rave->swRave.block) {
                BMMA_Free(rave->swRave.block);
                rave->swRave.block = NULL;
            }
        }
#endif
        BXPT_Rave_FreeContext(rave->raveHandle);
        rave->raveHandle = NULL;
    }
    if (rave->cdb.block) {
        BMMA_Unlock(rave->cdb.block, rave->cdb.ptr);
        BMMA_Free(rave->cdb.block);
        rave->cdb.block = NULL;
        rave->cdb.ptr = NULL;
    }
    if (rave->itb.block) {
        BMMA_Unlock(rave->itb.block, rave->itb.ptr);
        BMMA_Free(rave->itb.block);
        rave->itb.block = NULL;
        rave->itb.ptr = NULL;
    }
#if BXPT_P_HAS_AVS_PLUS_WORKAROUND
    if (rave->avsReference.raveHandle) {
        BXPT_Rave_FreeContext(rave->avsReference.raveHandle);
        rave->avsReference.raveHandle = NULL;
    }
    if (rave->avsReference.cdbBlock) {
        BMMA_Unlock(rave->avsReference.cdbBlock, rave->avsReference.cdb_ptr);
        BMMA_Free(rave->avsReference.cdbBlock);
        rave->avsReference.cdbBlock = NULL;
        rave->avsReference.cdb_ptr = NULL;
    }
    if (rave->avsReference.itbBlock) {
        BMMA_Unlock(rave->avsReference.itbBlock, rave->avsReference.itb_ptr);
        BMMA_Free(rave->avsReference.itbBlock);
        rave->avsReference.itbBlock = NULL;
        rave->avsReference.itb_ptr = NULL;
    }
#endif

    NEXUS_RaveErrorCounter_Uninit_priv(&rave->raveErrors);

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    if (rave->cap)
    {
        NEXUS_RaveCapture_Close(rave->cap);
        NEXUS_RaveCapture_Destroy(rave->cap);
        rave->cap = NULL;
    }
#endif

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    if (rave->xcCap)
    {
        NEXUS_TransportClientCapture_Close(rave->xcCap);
        NEXUS_TransportClientCapture_Destroy(rave->xcCap);
        rave->xcCap = NULL;
    }
#endif

    pTransport->rave[0].context[rave->array_index] = NULL;
    BDBG_OBJECT_DESTROY(rave, NEXUS_Rave);
    BKNI_Free(rave);
}

void NEXUS_Rave_GetDefaultSettings_priv(NEXUS_RaveSettings *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static void nexus_rave_check_wrap(NEXUS_RaveHandle rave)
{
    if (rave->settings.numOutputBytesEnabled) {
        BAVC_XptContextMap *pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
        uint32_t valid = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Valid);
        if (valid < rave->lastValid && rave->lastValid) {
            /* every time it wraps, we read WRAP-BASE. this is the total number of bytes in the CDB for the past wrap.
            see GetStatus where we add VALID-BASE to improve the result */
            unsigned n = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Wrap) - BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Base);
            rave->numOutputBytes += n;
            BDBG_MSG(("rave read %u", (unsigned)rave->numOutputBytes));
        }
        rave->lastValid = valid;
    }
}

static void NEXUS_Rave_P_Timer(void *context)
{
    NEXUS_RaveHandle rave = context;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    rave->timer = NULL;

    if (rave->swRave.raveHandle && rave->swRave.enabled) {
#if NEXUS_SW_RAVE_PEEK_EXTENSION
        /* this extension allows for a custom SW RAVE algorithm */
        NEXUS_P_SwRaveExtension_Advance(rave);
#else
        BXPT_Rave_AdvanceSoftContext(rave->swRave.raveHandle);
#endif
    }
    nexus_rave_check_wrap(rave);
    rave->timer = NEXUS_ScheduleTimer(NEXUS_RAVE_TIMER_PERIOD(rave), NEXUS_Rave_P_Timer, rave);
    BDBG_ASSERT(rave->timer);
}

static void NEXUS_Rave_P_ConvertForES(NEXUS_TransportType originalTransportType, BAVC_StreamType streamType, BAVC_StreamType *raveStreamType )
{

    /* SW7405-5140: if audio is ES format. Nexus needs to tell XPT PI to
    * disable DISABLE_BEFORE_PES filter.  If DISABLE_BEFORE_PES filter is enabled,
    * the RAVE discard all ES data as RAVE looks for a valid PES startcode. */
    *raveStreamType = streamType;
    if ( originalTransportType == NEXUS_TransportType_eEs )
    {
        /* limit change to ES input only */
        *raveStreamType = BAVC_StreamType_eEs;
    }
}

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
static void NEXUS_Rave_P_CreateTransportClientCapture(NEXUS_RaveHandle rave, const NEXUS_RaveSettings * pSettings)
{
    NEXUS_TransportClientCaptureCreateSettings xcccSettings;

    NEXUS_TransportClientCapture_GetDefaultCreateSettings(&xcccSettings);
    xcccSettings.pidChannel = pSettings->pidChannel;
    xcccSettings.clientType = NEXUS_TransportClientType_eRave;
    rave->xcCap = NEXUS_TransportClientCapture_Create(&xcccSettings);
    if (rave->xcCap)
    {
        NEXUS_TransportClientCapture_Open(rave->xcCap);
    }
}
#endif

NEXUS_Error NEXUS_Rave_ConfigureVideo_priv(NEXUS_RaveHandle rave,
    NEXUS_VideoCodec codec, const NEXUS_RaveSettings *pSettings)
{
    BERR_Code rc;
    BXPT_Rave_AvSettings AvCtxCfg;
    BAVC_StreamType streamType;
#if NEXUS_SW_RAVE_SUPPORT
    bool swRave;
    BXPT_RaveSoftMode sw_rave_mode = BXPT_RaveSoftMode_eCopyItb;
#endif
    NEXUS_P_HwPidChannel *hwPidChannel = pSettings->hwPidChannel;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    NEXUS_Rave_P_CreateTransportClientCapture(rave, pSettings);
#endif
    rave->numOutputBytes = 0;

    if(hwPidChannel==NULL) {
        if(pSettings->pidChannel==NULL) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        hwPidChannel = pSettings->pidChannel->hwPidChannel;
    }

    rc = NEXUS_P_TransportType_ToMagnum_isrsafe(hwPidChannel->status.transportType, &streamType);
    if (rc) return BERR_TRACE(rc);

    /* get default state into structure */
    BXPT_Rave_GetAvConfig(rave->raveHandle, &AvCtxCfg);
    NEXUS_Rave_P_ConvertForES( hwPidChannel->status.originalTransportType, streamType, &AvCtxCfg.InputFormat );
    AvCtxCfg.OutputFormat = BAVC_StreamType_eEs;

    /* default all EsRange off, then allow codecs to turn on what's needed */
    BKNI_Memset(AvCtxCfg.EsRanges, 0, sizeof(AvCtxCfg.EsRanges)); /* all disabled */

#if NEXUS_SW_RAVE_SUPPORT
    swRave = false;
    if (NEXUS_GetEnv("force_sw_rave")) {
        swRave = true;
    }
    else if (NEXUS_Rave_P_RequiresSwRave(hwPidChannel->status.originalTransportType, codec)) {
        swRave = true;
    }
#if NEXUS_SW_RAVE_PEEK_EXTENSION
    if (swRave) {
        /* if swRave is required by the codec, then NEXUS_SW_RAVE_PEEK_EXTENSION does not work. */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    /* force SW RAVE on for peeking */
    swRave = true;
    sw_rave_mode = BXPT_RaveSoftMode_ePointersOnly;
#endif
#endif

    switch (codec) {
    case NEXUS_VideoCodec_eH263:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eH263;
        AvCtxCfg.EsRanges[0].RangeHi = 0xFE; /* everything */
        AvCtxCfg.EsRanges[0].RangeLo = 0x00;
        AvCtxCfg.EsRanges[0].RangeIsASlice = false;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xBD;
        break;
    case NEXUS_VideoCodec_eH264_Svc:
    case NEXUS_VideoCodec_eH264_Mvc:
    case NEXUS_VideoCodec_eH264:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAvcVideo;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xBD;
        AvCtxCfg.EsRanges[0].RangeHi = 0xFE; /* everything */
        AvCtxCfg.EsRanges[0].RangeLo = 0x00;
        AvCtxCfg.EsRanges[0].RangeIsASlice = false;
        AvCtxCfg.EsRanges[0].Enable = true;
        break;
    case NEXUS_VideoCodec_eVc1:
    case NEXUS_VideoCodec_eVc1SimpleMain:
      AvCtxCfg.StreamIdHi = 0xFD;
      AvCtxCfg.StreamIdLo = 0xBD;
      if(codec == NEXUS_VideoCodec_eVc1SimpleMain && swRave) {
            /* this allows PES entries through, nothing else. */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eVC1SimpleMain;
#if NEXUS_SW_RAVE_SUPPORT
            sw_rave_mode = BXPT_RaveSoftMode_eVc1SimpleMain;
#endif
        } else {
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eVc1Video;
            AvCtxCfg.EsRanges[0].RangeHi = 0xFE; /* everything */
            AvCtxCfg.EsRanges[0].RangeLo = 0x00;
            AvCtxCfg.EsRanges[0].RangeIsASlice = false;
            AvCtxCfg.EsRanges[0].Enable = true;
        }
        break;
    case NEXUS_VideoCodec_eMpeg1:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpeg1Video;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.EsRanges[0].RangeHi = 0xAF; /* MPEG slices */
        AvCtxCfg.EsRanges[0].RangeLo = 0x01;
        AvCtxCfg.EsRanges[0].RangeIsASlice = true;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.EsRanges[1].RangeHi = 0xBF; /* start of frame */
        AvCtxCfg.EsRanges[1].RangeLo = 0xB0;
        AvCtxCfg.EsRanges[1].RangeIsASlice = false;
        AvCtxCfg.EsRanges[1].Enable = true;
        AvCtxCfg.EsRanges[2].RangeHi = 0x0; /* start of picture */
        AvCtxCfg.EsRanges[2].RangeLo = 0x0;
        AvCtxCfg.EsRanges[2].RangeIsASlice = false;
        AvCtxCfg.EsRanges[2].Enable = true;
        break;
    case NEXUS_VideoCodec_eMpeg2:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpeg2Video;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.EsRanges[0].RangeHi = 0xAF; /* MPEG slices */
        AvCtxCfg.EsRanges[0].RangeLo = 0x01;
        AvCtxCfg.EsRanges[0].RangeIsASlice = true;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.EsRanges[1].RangeHi = 0xBF; /* start of frame */
        AvCtxCfg.EsRanges[1].RangeLo = 0xB0;
        AvCtxCfg.EsRanges[1].RangeIsASlice = false;
        AvCtxCfg.EsRanges[1].Enable = true;
        AvCtxCfg.EsRanges[2].RangeHi = 0x0; /* start of picture */
        AvCtxCfg.EsRanges[2].RangeLo = 0x0;
        AvCtxCfg.EsRanges[2].RangeIsASlice = false;
        AvCtxCfg.EsRanges[2].Enable = true;
        break;
    case NEXUS_VideoCodec_eDivx311:
#if NEXUS_SW_RAVE_SUPPORT
        if (swRave) {
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpeg4Part2;
            AvCtxCfg.StreamIdHi = 0xEF;
            AvCtxCfg.StreamIdLo = 0xE0;
            sw_rave_mode = BXPT_RaveSoftMode_eDivX_311;
            break;
        }
#endif
        /* if not SW rave, then fall through as Mpeg4Part2 */

    case NEXUS_VideoCodec_eMpeg4Part2:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpeg4Part2;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.EsRanges[0].RangeHi = 0x2F;
        AvCtxCfg.EsRanges[0].RangeLo = 0x00;
        AvCtxCfg.EsRanges[0].RangeIsASlice = true;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.EsRanges[1].RangeHi = 0xB8;
        AvCtxCfg.EsRanges[1].RangeLo = 0xB0;
        AvCtxCfg.EsRanges[1].RangeIsASlice = false;
        AvCtxCfg.EsRanges[1].Enable = true;
#if NEXUS_SW_RAVE_SUPPORT
        sw_rave_mode = BXPT_RaveSoftMode_eDivX;
        if (pSettings->disableReordering) {
            sw_rave_mode = BXPT_RaveSoftMode_eDivX_noReorder;
        }
#endif
        break;
    case NEXUS_VideoCodec_eAvs:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAvsVideo;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.EsRanges[0].RangeHi = 0xAF; /* AVS slices */
        AvCtxCfg.EsRanges[0].RangeLo = 0x00;
        AvCtxCfg.EsRanges[0].RangeIsASlice = true;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.EsRanges[1].RangeHi = 0xBF; /* start of frame and pictures */
        AvCtxCfg.EsRanges[1].RangeLo = 0xB0;
        AvCtxCfg.EsRanges[1].RangeIsASlice = false;
        AvCtxCfg.EsRanges[1].Enable = true;
        break;
    case NEXUS_VideoCodec_eVp6:
#if NEXUS_BDSP_VP6_SUPPORT /* If not, AVD can decode VP6 */
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eVp6Video;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.EsRanges[0].RangeHi = 0xFE; /* everything */
        AvCtxCfg.EsRanges[0].RangeLo = 0x00;
        AvCtxCfg.EsRanges[0].RangeIsASlice = false;
        AvCtxCfg.EsRanges[0].Enable = true;
        AvCtxCfg.EsRanges[1] = AvCtxCfg.EsRanges[0];
        AvCtxCfg.EsRanges[2] = AvCtxCfg.EsRanges[0];
        AvCtxCfg.EsRanges[3] = AvCtxCfg.EsRanges[0];
        break;
#endif
    case NEXUS_VideoCodec_eRv40:
    case NEXUS_VideoCodec_eVp8:
    case NEXUS_VideoCodec_eVp9:
    case NEXUS_VideoCodec_eMotionJpeg:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eVC1SimpleMain;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        break;
    case NEXUS_VideoCodec_eSpark:
        if (hwPidChannel->status.originalTransportType==NEXUS_TransportType_eEs) {
            /* for spark ES, we must do H263-like ES indexing */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eH263;
            AvCtxCfg.EsRanges[0].RangeHi = 0xFE; /* everything */
            AvCtxCfg.EsRanges[0].RangeLo = 0x00;
            AvCtxCfg.EsRanges[0].RangeIsASlice = false;
            AvCtxCfg.EsRanges[0].Enable = true;
            AvCtxCfg.StreamIdHi = 0xEF;
            AvCtxCfg.StreamIdLo = 0xBD;
        }
        else {
            /* otherwise, we send transport a PES with BCMV headers and no ES indexing is needed */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eVC1SimpleMain;
            AvCtxCfg.StreamIdHi = 0xEF;
            AvCtxCfg.StreamIdLo = 0xE0;
        }
        break;
    case NEXUS_VideoCodec_eH265:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAvcVideo;
        AvCtxCfg.OutputFormat = BAVC_StreamType_eEs;
        AvCtxCfg.EsRanges[ 0 ].RangeHi = 0x7F;
        AvCtxCfg.EsRanges[ 0 ].RangeLo = 0;
        AvCtxCfg.EsRanges[ 0 ].Enable = true;
        AvCtxCfg.EsRanges[ 1 ].Enable = false;
        AvCtxCfg.EsRanges[ 2 ].Enable = false;
        AvCtxCfg.EsRanges[ 3 ].Enable = false;
        AvCtxCfg.StreamIdHi = 0xEF;
        AvCtxCfg.StreamIdLo = 0xE0;
        AvCtxCfg.BandHoldEn = true;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* override pes filtering */
    if (hwPidChannel->settings.pesFiltering.low || hwPidChannel->settings.pesFiltering.high) {
        AvCtxCfg.StreamIdHi = hwPidChannel->settings.pesFiltering.high;
        AvCtxCfg.StreamIdLo = hwPidChannel->settings.pesFiltering.low;
    }

    BDBG_CASSERT(BXPT_HAS_PID_CHANNEL_PES_FILTERING);

#if NEXUS_SW_RAVE_SUPPORT
    /* coverity[dead_error_begin] */
    if (swRave) {
        if (!rave->swRave.raveHandle) {
            BDBG_ERR(("supportedCodecs[] mismatch between open and start time. No SW RAVE context available."));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        rc = BXPT_Rave_ResetSoftContext(rave->swRave.raveHandle, sw_rave_mode);
        if (rc) return BERR_TRACE(rc);
        rave->swRave.enabled = true;
    }
    else {
        rave->swRave.enabled = false;
    }
#endif

    AvCtxCfg.BandHoldEn = pSettings->bandHold;
    /* unconditionally disable CC check. we already have CC check in parserband and pidchannel. if someone must have CC check in RAVE, we
    should expose decoder API to make it conditional, like NEXUS_ParserBandSettings.continuityCountEnabled */
    AvCtxCfg.DisableContinuityCheck = true;

#if BXPT_HAS_BPP_SEARCH
    /* only enable BPP search for non-ts (TS uses BTP) */
    if (pSettings->pidChannel->hwPidChannel->status.originalTransportType != NEXUS_TransportType_eTs)
    {
        BDBG_MSG(("Enabling BPP Search"));
        AvCtxCfg.EnableBPPSearch = true; /* allows LAST BPP command to flow to ITB. if feature works, no reason to expose knob to turn off. */
    }
    else
    {
        BDBG_MSG(("Disabling BPP Search"));
        AvCtxCfg.EnableBPPSearch = false;
    }
#endif

    BDBG_MSG(("RAVE_CX%d configured for video codec %d, pid channel %d", rave->index, codec, hwPidChannel->status.pidChannelIndex));

    rc = nexus_rave_add_pid(rave, hwPidChannel);
    if (rc) return BERR_TRACE(rc);

    if(pSettings->otfPvr) {
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eOTFVideo;
    }

    rc = BXPT_Rave_SetAvConfig(rave->raveHandle, &AvCtxCfg);
    if (rc) {return BERR_TRACE(rc);}

    /* NOTE: do not flush rave here. if playback has already started, then we could lose a small
    amount of data. rave should already be in a flushed state, either from initial conditions
    or from a flush after stopping decode. */
    rave->settings = *pSettings;
    rave->lastValid = 0;

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Rave_ConfigureAudio_priv(
    NEXUS_RaveHandle rave,
    NEXUS_AudioCodec codec,
    const NEXUS_RaveSettings *pSettings
    )
{
    BERR_Code rc;
    BXPT_Rave_AvSettings AvCtxCfg;
    BAVC_StreamType streamType;
    NEXUS_PidChannelStatus pidChannelStatus;
    NEXUS_P_HwPidChannel *hwPidChannel = pSettings->hwPidChannel;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    NEXUS_Rave_P_CreateTransportClientCapture(rave, pSettings);
#endif
    rave->numOutputBytes = 0;

    if(hwPidChannel==NULL) {
        if(pSettings->pidChannel==NULL) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        hwPidChannel = pSettings->pidChannel->hwPidChannel;
    }

    rc = NEXUS_P_TransportType_ToMagnum_isrsafe(hwPidChannel->status.transportType, &streamType);
    if (rc) return BERR_TRACE(rc);

    /* There are 4 types of DSS A/V streams:
    DSS SD video - this is DSS ES
    DSS HD video - this is DSS PES
    DSS MPEG audio - this actually uses MPEG1 system headers, but it's very similar to PES, therefore DSS PES
    Therefore we convert DSS ES to DSS PES here.
    DSS AC3 audio - uses MPEG2 System PES, therefore DSS PES
    */
    if (streamType == BAVC_StreamType_eDssEs)
    {
        streamType = BAVC_StreamType_eDssPes;
    }

    BXPT_Rave_GetAvConfig(rave->raveHandle, &AvCtxCfg);
    NEXUS_Rave_P_ConvertForES( hwPidChannel->status.originalTransportType, streamType, &AvCtxCfg.InputFormat );
    AvCtxCfg.OutputFormat = BAVC_StreamType_eEs;

#if BCHP_CHIP == 7400
    /* Old FW arch only supports AD on MPEG */
    if ( pSettings->audioDescriptor && (codec != NEXUS_AudioCodec_eMpeg && codec != NEXUS_AudioCodec_eMp3) )
    {
        BDBG_ERR(("Audio descriptors are only supported with MPEG audio currently"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif

    /* TODO: bsettop_p_rap_set_rave_thresholds(rap, mpeg->audio[program].format, cfg->playback, rave->raveHandle, &AvCtxCfg); */
    BKNI_Memset(AvCtxCfg.EsRanges, 0, sizeof(AvCtxCfg.EsRanges)); /* all disabled */

    switch (codec) {
    case NEXUS_AudioCodec_eMpeg:
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpegAudioWithDescriptor;
        }
        else
        {
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpegAudio;
        }
        break;
    case NEXUS_AudioCodec_eMp3:
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpegAudioWithDescriptor;
        }
        else
        {
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMpegAudioLayer3;
        }
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAls:
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAudioDescriptor;
        }
        else
        {
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAacAudio;
        }
        break;
    case NEXUS_AudioCodec_eAacPlusLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
        /* baudio_format_aac_plus_loas is also handled here as it has the same value as baudio_format_aac_plus */
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAudioDescriptor;
        }
        else
        {
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAacHe;
        }
        break;
    case NEXUS_AudioCodec_eDra:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eDra;
        break;
    case NEXUS_AudioCodec_ePcm: /* PCM audio in PES uses the same BCMA header/encapsulation */
    case NEXUS_AudioCodec_ePcmWav:
    case NEXUS_AudioCodec_eWmaStd:
    case NEXUS_AudioCodec_eWmaStdTs:
    case NEXUS_AudioCodec_eWmaPro:
    case NEXUS_AudioCodec_eAmrNb:
    case NEXUS_AudioCodec_eAmrWb:
    case NEXUS_AudioCodec_eAdpcm:
    case NEXUS_AudioCodec_eVorbis:
    case NEXUS_AudioCodec_eG711:
    case NEXUS_AudioCodec_eG726:
    case NEXUS_AudioCodec_eG729:
    case NEXUS_AudioCodec_eG723_1:
    case NEXUS_AudioCodec_eFlac:
    case NEXUS_AudioCodec_eApe:
    case NEXUS_AudioCodec_eOpus:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eWma;
        break;
    case NEXUS_AudioCodec_eIlbc:
    case NEXUS_AudioCodec_eIsac:
    case NEXUS_AudioCodec_eCook:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAmr;
        break;
    case NEXUS_AudioCodec_eAc3:
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAudioDescriptor;
        }
        else
        {
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAc3Plus; /* Use AC3+ ITB type for AC3 */
        }
        break;
    case NEXUS_AudioCodec_eAc3Plus:
        if ( pSettings->audioDescriptor )
        {
            /* Currently there is no other way to specify this in RAVE */
            AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAudioDescriptor;
        }
        else
        {
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAc3Plus;
        }
        break;
    case NEXUS_AudioCodec_eLpcmDvd :
    case NEXUS_AudioCodec_eLpcm1394:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eLpcmAudio;
        break;
    case NEXUS_AudioCodec_eLpcmBluRay:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eLpcmAudio;
        break;
    case NEXUS_AudioCodec_eDts:
    case NEXUS_AudioCodec_eDtsHd:
    case NEXUS_AudioCodec_eDtsLegacy:
    case NEXUS_AudioCodec_eDtsExpress:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eDtsAudio;
        break;
    case NEXUS_AudioCodec_eMlp:
        AvCtxCfg.ItbFormat = BAVC_ItbEsType_eMlpAudio;
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* this is intended to cover 0xC0-0xDF(mpeg audio) and 0xBD, 0xFD (non-mpeg audio) */
    AvCtxCfg.StreamIdHi = 0xFD;
    AvCtxCfg.StreamIdLo = 0xBD;

    /* override pes filtering */
    if (hwPidChannel->settings.pesFiltering.low || hwPidChannel->settings.pesFiltering.high) {
        AvCtxCfg.StreamIdHi = hwPidChannel->settings.pesFiltering.high;
        AvCtxCfg.StreamIdLo = hwPidChannel->settings.pesFiltering.low;
    }

    rc = NEXUS_P_HwPidChannel_GetStatus(hwPidChannel, &pidChannelStatus);
    if (rc) {return BERR_TRACE(rc);}

    {
        BXPT_Rave_ContextSettings CtxConfig;
        BXPT_Rave_GetContextConfig(rave->raveHandle, &CtxConfig);

        CtxConfig.PesSidExtMode = BXPT_Rave_PesSidExtNormal;

        /* this is the default */
        CtxConfig.EnablePrivateHdrItbEntry = false;
        CtxConfig.AudFrameInfo = 0;

        if ( pidChannelStatus.originalTransportType == NEXUS_TransportType_eVob ||
            (pidChannelStatus.originalTransportType == NEXUS_TransportType_eMpeg2Pes && pidChannelStatus.remappedPid >= 0x100))
        {
            switch ( AvCtxCfg.ItbFormat )
            {
            case BAVC_ItbEsType_eLpcmAudio:
                CtxConfig.EnablePrivateHdrItbEntry = true;
                CtxConfig.AudFrameInfo = 7;
                break;
            case BAVC_ItbEsType_eAc3gAudio:
            case BAVC_ItbEsType_eAc3Plus:
            case BAVC_ItbEsType_eDtsAudio:
                CtxConfig.EnablePrivateHdrItbEntry = false;
                CtxConfig.AudFrameInfo = 4;
                break;
            default:
                break;
            }
        }
        else if ( pidChannelStatus.originalTransportType == NEXUS_TransportType_eTs )
        {
            switch ( codec )
            {
            case NEXUS_AudioCodec_eLpcmBluRay:
            case NEXUS_AudioCodec_eLpcm1394:
                /* 1394 LPCM and BD-LPCM are very similar to DVD-LPCM over MPEG2-TS.  They have a 4-byte header instead of 7 however. */
                CtxConfig.EnablePrivateHdrItbEntry = true;
                CtxConfig.AudFrameInfo = 4;
                break;
            default:
                break;
            }
            if(hwPidChannel->combinedPid&0xFF0000) {
                CtxConfig.PesSidExtMode = BXPT_Rave_PesSidExtIndependent;
                CtxConfig.PesExtSearchMode = BXPT_Rave_PesExtSearchSpecial;
                CtxConfig.SidExtIndependent = hwPidChannel->combinedPid>>16;

                if(codec == NEXUS_AudioCodec_eAc3Plus) {
                    /* AC3 plus needs both base and extansion data */
                    CtxConfig.PesSidExtMode = BXPT_Rave_PesSidExtKeepAll;
                    CtxConfig.SidExtDependent = CtxConfig.SidExtIndependent;
                    CtxConfig.SidExtIndependent --;
                }
            }
        }
        rc = BXPT_Rave_SetContextConfig(rave->raveHandle, &CtxConfig);
        if (rc) {return BERR_TRACE(rc);}
    }

    AvCtxCfg.BandHoldEn = pSettings->bandHold;
    /* unconditionally disable CC check. we already have CC check in parserband and pidchannel. if someone must have CC check in RAVE, we
    should expose decoder API to make it conditional, like NEXUS_ParserBandSettings.continuityCountEnabled */
    AvCtxCfg.DisableContinuityCheck = true;

    BDBG_MSG(("RAVE_CX%d configured for audio codec %d, pid channel %d", rave->index, codec, hwPidChannel->status.pidChannelIndex));

    rc = nexus_rave_add_pid(rave, hwPidChannel);
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_Rave_SetAvConfig(rave->raveHandle, &AvCtxCfg);
    if (rc) {return BERR_TRACE(rc);}

    /* NOTE: do not flush rave here. if playback has already started, then we could lose a small
    amount of data. rave should already be in a flushed state, either from initial conditions
    or from a flush after stopping decode. */
    rave->settings = *pSettings;
    rave->lastValid = 0;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Rave_ConfigureAll_priv(NEXUS_RaveHandle rave, const NEXUS_RaveSettings *pSettings)
{
    BERR_Code rc;
    BXPT_Rave_AvSettings AvCtxCfg;
    BAVC_StreamType streamType;
    NEXUS_P_HwPidChannel *hwPidChannel = pSettings->hwPidChannel;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    NEXUS_Rave_P_CreateTransportClientCapture(rave, pSettings);
#endif
    rave->numOutputBytes = 0;

    if(hwPidChannel==NULL) {
        if(pSettings->pidChannel==NULL) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        hwPidChannel = pSettings->pidChannel->hwPidChannel;
    }
    rc = NEXUS_P_TransportType_ToMagnum_isrsafe(hwPidChannel->status.transportType, &streamType);
    if (rc) return BERR_TRACE(rc);

    /* get default state into structure */
    BXPT_Rave_GetAvConfig(rave->raveHandle, &AvCtxCfg);
    NEXUS_Rave_P_ConvertForES( hwPidChannel->status.originalTransportType, streamType, &AvCtxCfg.InputFormat );
    AvCtxCfg.OutputFormat = BAVC_StreamType_eEs;

    /* default all EsRange off, then allow codecs to turn on what's needed */
    AvCtxCfg.EsRanges[0].Enable = false;
    AvCtxCfg.EsRanges[1].Enable = false;
    AvCtxCfg.EsRanges[2].Enable = false;
    AvCtxCfg.EsRanges[3].Enable = false;
    AvCtxCfg.StreamIdHi = 0xFF;
    AvCtxCfg.StreamIdLo = 0x00;
    AvCtxCfg.BandHoldEn = pSettings->bandHold;
    /* unconditionally disable CC check. we already have CC check in parserband and pidchannel. if someone must have CC check in RAVE, we
    should expose decoder API to make it conditional, like NEXUS_ParserBandSettings.continuityCountEnabled */
    AvCtxCfg.DisableContinuityCheck = true;

    /* override pes filtering */
    if (hwPidChannel->settings.pesFiltering.low || hwPidChannel->settings.pesFiltering.high) {
        AvCtxCfg.StreamIdHi = hwPidChannel->settings.pesFiltering.high;
        AvCtxCfg.StreamIdLo = hwPidChannel->settings.pesFiltering.low;
    }

    BDBG_MSG(("RAVE_CX%d configured for data, pid channel %d", rave->index, hwPidChannel->status.pidChannelIndex));

    rc = nexus_rave_add_pid(rave, hwPidChannel);
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_Rave_SetAvConfig(rave->raveHandle, &AvCtxCfg);
    if (rc) {return BERR_TRACE(rc);}
    rave->settings = *pSettings;
    rave->lastValid = 0;

    return BERR_SUCCESS;
}

void NEXUS_Rave_RemovePidChannel_priv(NEXUS_RaveHandle rave)
{
    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();
    nexus_rave_remove_pid(rave);
    return;
}

void NEXUS_Rave_Enable_priv(NEXUS_RaveHandle rave)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    if (rave->cap)
    {
        NEXUS_RaveCapture_Start(rave->cap);
    }
#endif

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    if (rave->xcCap)
    {
        NEXUS_TransportClientCapture_Start(rave->xcCap);
    }
#endif

    if (rave->swRave.enabled || rave->settings.numOutputBytesEnabled) {
        if (!rave->timer) {
            rave->timer = NEXUS_ScheduleTimer(NEXUS_RAVE_TIMER_PERIOD(rave), NEXUS_Rave_P_Timer, rave);
            BDBG_ASSERT(rave->timer);
        }
    }

    NEXUS_RaveErrorCounter_Reset_priv(&rave->raveErrors);
    rc = BXPT_Rave_EnableContext(rave->raveHandle);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}

    rave->enabled = true;
}

void NEXUS_Rave_Disable_priv(NEXUS_RaveHandle rave)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    if (rave->xcCap)
    {
        NEXUS_TransportClientCapture_Stop(rave->xcCap);
    }
#endif

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    if (rave->cap)
    {
        NEXUS_RaveCapture_Stop(rave->cap);
    }
#endif

    if (rave->timer) {
        NEXUS_CancelTimer(rave->timer);
        rave->timer = NULL;
    }

    rc=BXPT_Rave_DisableContext(rave->raveHandle);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    rave->enabled = false;
    return;
}

void NEXUS_Rave_Flush_priv(NEXUS_RaveHandle rave)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED
    if (rave->cap)
    {
        NEXUS_RaveCapture_Flush(rave->cap);
    }
#endif

#if NEXUS_RAVE_INPUT_CAPTURE_ENABLED
    if (rave->xcCap)
    {
        NEXUS_TransportClientCapture_Flush(rave->xcCap);
    }
#endif

    if (rave->settings.numOutputBytesEnabled) {
        BAVC_XptContextMap *pXptContextMap;
        pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
        rave->numOutputBytes += BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Valid) - BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Base);
    }
    rave->lastValid = 0;
    rc = BXPT_Rave_FlushContext(rave->raveHandle);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}

#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.raveHandle) {
        rc = BXPT_Rave_FlushContext(rave->swRave.raveHandle);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    }
#endif
}

NEXUS_Error NEXUS_Rave_GetStatus_priv(NEXUS_RaveHandle rave, NEXUS_RaveStatus *pStatus)
{
    BAVC_XptContextMap *pXptContextMap;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->index = rave->index;
    pStatus->swRaveIndex = rave->swRave.index;
#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.raveHandle && rave->swRave.enabled) {
        pStatus->xptContextMap = rave->swRave.xptContextMap;
    }
    else
#endif
    {
        pStatus->xptContextMap = rave->xptContextMap;
    }
    pStatus->itbBlock = rave->itb.block;
    pStatus->cdbBlock = rave->cdb.block;

    nexus_rave_check_wrap(rave); /* force an update on the "ballpark" figure, or we risk reporting a smaller-than-previous value */
    pStatus->numOutputBytes = rave->numOutputBytes;
    /* augment with VALID-BASE. this is useful for low bitrate streams that are just starting decode. */
    pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
    pStatus->numOutputBytes += BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Valid) - BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Base);
    pStatus->enabled = rave->enabled;
#if NEXUS_HAS_SAGE
    /* we know if CRR is actually secure if SAGE is enabled and we're using the secure heap. For now, just test if is compiled in. */
    pStatus->crrEnabled = rave->useSecureHeap;
#endif

    /* to match the allocation logic */
    if(rave->useSecureHeap) {
        pStatus->heap = pTransport->settings.secureHeap;
    } else {
        pStatus->heap = rave->openSettings.heap ? rave->openSettings.heap : g_pCoreHandles->heap[pTransport->settings.mainHeapIndex].nexus;
    }
    return NEXUS_SUCCESS;
}

void NEXUS_Rave_GetCdbBufferInfo_isr(NEXUS_RaveHandle rave, unsigned *depth, unsigned *size)
{
    BERR_Code rc;
    BXPT_Rave_BufferInfo buffer_info;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    BKNI_ASSERT_ISR_CONTEXT();

    rc = BXPT_Rave_GetBufferInfo_isr(rave->raveHandle, &buffer_info);
    if(rc==BERR_SUCCESS) {
        *depth = buffer_info.CdbDepth;
        *size = buffer_info.CdbSize;
    } else {
        rc = BERR_TRACE(rc);
        *depth=0;
        *size=0;
    }
    return;
}

void NEXUS_Rave_GetItbBufferInfo(NEXUS_RaveHandle rave, unsigned *depth, unsigned *size)
{
    BERR_Code rc;
    uint32_t base;
    uint32_t end;
    BXPT_Rave_ContextPtrs CtxPtrs;
    BAVC_XptContextMap * pXptContextMap;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);

    rc = BXPT_Rave_CheckBuffer(rave->raveHandle, &CtxPtrs );
    if(rc==BERR_SUCCESS) {
        *depth = CtxPtrs.Itb.ByteCount + CtxPtrs.Itb.WrapByteCount;
    } else {
        rc = BERR_TRACE(rc);
        *depth=0;
    }

    pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
    base = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_Base);
    end = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_End);
    *size = end - base + 1;

    return;
}

static BERR_Code NEXUS_Rave_P_ConvertPointer(uint8_t **ptr)
{
    /* do nothing, ptr is already cached */
    BSTD_UNUSED(ptr);
    return NEXUS_SUCCESS;
}

/* get BXPT_Rave_ContextPtrs and convert Itb.DataPtr to cached. caller must flush. */
static BERR_Code get_itb_context(BXPT_RaveCx_Handle rave, BXPT_Rave_ContextPtrs *pCtxPtrs)
{
    BERR_Code rc;

    rc = BXPT_Rave_CheckBuffer(rave, pCtxPtrs);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Rave_P_ConvertPointer(&pCtxPtrs->Itb.DataPtr);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

#if NEXUS_NUM_SOFT_VIDEO_DECODERS
NEXUS_Error NEXUS_Rave_CheckBuffer_priv(NEXUS_RaveHandle rave, struct BXPT_Rave_ContextPtrs *pCtxPtrs)
{
    BXPT_RaveCx_Handle raveHandle;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    raveHandle = rave->raveHandle;
#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.enabled) {
        raveHandle = rave->swRave.raveHandle;
    }
#endif
    if(raveHandle==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    rc = get_itb_context(rave->raveHandle, pCtxPtrs );
    if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc); }

    if ( !pCtxPtrs->Itb.DataPtr || pCtxPtrs->Itb.ByteCount < NEXUS_RAVE_P_ITB_SIZE || pCtxPtrs->Itb.ByteCount > 0x1000000) {
        return NEXUS_NOT_AVAILABLE;
    }
    if ( !pCtxPtrs->Cdb.DataPtr || pCtxPtrs->Cdb.ByteCount > 0x1000000) {
        return NEXUS_NOT_AVAILABLE;
    }
    rc = NEXUS_Rave_P_ConvertPointer(&pCtxPtrs->Itb.WrapDataPtr);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Rave_P_ConvertPointer(&pCtxPtrs->Cdb.DataPtr);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Rave_P_ConvertPointer(&pCtxPtrs->Cdb.WrapDataPtr);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Rave_UpdateReadOffset_priv(NEXUS_RaveHandle rave, size_t CdbByteCount, size_t ItbByteCount)
{
    BXPT_RaveCx_Handle raveHandle;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    raveHandle = rave->raveHandle;
#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.enabled) {
        raveHandle = rave->swRave.raveHandle;
    }
#endif
    if(raveHandle==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    rc = BXPT_Rave_UpdateReadOffset(raveHandle, CdbByteCount, ItbByteCount);
    if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}
#endif

NEXUS_Error NEXUS_Rave_ScanItb_priv(NEXUS_RaveHandle rave, bool (*one_itb)(void *, const NEXUS_Rave_P_ItbEntry *), void *context)
{
    BXPT_Rave_ContextPtrs CtxPtrs;
    BXPT_RaveCx_Handle raveHandle;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    raveHandle = rave->raveHandle;
#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.enabled) {
        raveHandle = rave->swRave.raveHandle;
    }
#endif
    if(raveHandle==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    rc = get_itb_context(rave->raveHandle, &CtxPtrs );
    if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc); }
    /* check for an empty or reset buffer. super large ByteCount can happen because of reset */
    if ( !CtxPtrs.Itb.DataPtr || CtxPtrs.Itb.ByteCount < NEXUS_RAVE_P_ITB_SIZE || CtxPtrs.Itb.ByteCount > 0x1000000) {
        return NEXUS_NOT_AVAILABLE;
    }
    {
        const NEXUS_Rave_P_ItbEntry *entry;
        unsigned itbCount;

        entry = (void *)CtxPtrs.Itb.DataPtr;
        for(itbCount = CtxPtrs.Itb.ByteCount/NEXUS_RAVE_P_ITB_SIZE;itbCount;itbCount--,entry++) {
            NEXUS_FlushCache(entry, sizeof(*entry));
            if(one_itb(context, entry)) {
                return NEXUS_SUCCESS;
            }
        }
        rc = NEXUS_Rave_P_ConvertPointer(&CtxPtrs.Itb.WrapDataPtr);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        entry = (void *)CtxPtrs.Itb.WrapDataPtr;
        for(itbCount = CtxPtrs.Itb.WrapByteCount/NEXUS_RAVE_P_ITB_SIZE;itbCount;itbCount--,entry++) {
            NEXUS_FlushCache(entry, sizeof(*entry));
            if(one_itb(context, entry)) {
                return NEXUS_SUCCESS;
            }
        }
    }
    return NEXUS_NOT_AVAILABLE; /* no BERR_TRACE */
}

#include "bxpt_priv.h"


static bool NEXUS_Rave_P_ScanItb_Pts(void *context, const NEXUS_Rave_P_ItbEntry *itb)
{
    const uint32_t *pts = context;
    unsigned type = B_GET_BITS(itb->word[0], 31, 24);

    BDBG_CASSERT(brave_itb_video_start_code ==     (unsigned)NEXUS_Rave_P_ItbType_eExtractedData);
    BDBG_CASSERT(brave_itb_base_address ==         (unsigned)NEXUS_Rave_P_ItbType_eBaseAddress);
    BDBG_CASSERT(brave_itb_pts_dts ==              (unsigned)NEXUS_Rave_P_ItbType_ePtsDts);
    BDBG_CASSERT(brave_itb_pcr_offset ==           (unsigned)NEXUS_Rave_P_ItbType_ePcrOffset);
    BDBG_CASSERT(brave_itb_btp ==                  (unsigned)NEXUS_Rave_P_ItbType_eBtp);
    BDBG_CASSERT(brave_itb_private_hdr ==          (unsigned)NEXUS_Rave_P_ItbType_ePrivateHdr);
    BDBG_CASSERT(brave_itb_rts ==                  (unsigned)NEXUS_Rave_P_ItbType_eRts);
    BDBG_CASSERT(brave_itb_pcr ==                  (unsigned)NEXUS_Rave_P_ItbType_ePcr);
    BDBG_CASSERT(brave_itb_ip_stream_out ==        (unsigned)NEXUS_Rave_P_ItbType_eIpStreamOut);
    BDBG_CASSERT(brave_itb_termination ==          (unsigned)NEXUS_Rave_P_ItbType_eTermination);

    if(type==NEXUS_Rave_P_ItbType_ePtsDts) {
        if( *pts == itb->word[1]) {
            return true;
        }
    }
    return false;
}

bool NEXUS_Rave_FindPts_priv(NEXUS_RaveHandle rave, uint32_t pts)
{
    NEXUS_Error rc;

    rc = NEXUS_Rave_ScanItb_priv(rave, NEXUS_Rave_P_ScanItb_Pts, &pts);
    return rc==NEXUS_SUCCESS;
}

#define ITB_START_CODE 0x00

bool NEXUS_Rave_FindVideoStartCode_priv(NEXUS_RaveHandle rave, uint8_t startCode)
{
    uint8_t *ItbByte,i;
    BXPT_Rave_ContextPtrs CtxPtrs;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

#if NEXUS_SW_RAVE_SUPPORT
    if (rave->swRave.raveHandle && rave->swRave.enabled) {
        rc = get_itb_context(rave->swRave.raveHandle, &CtxPtrs );
        if (rc) {rc = BERR_TRACE(rc); return false;}
    }
    else
#endif
    {
        rc = get_itb_context(rave->raveHandle, &CtxPtrs );
        if (rc) {rc = BERR_TRACE(rc); return false;}
    }

    /* check for an empty or reset buffer. super large ByteCount can happen because of reset */
    if ( !CtxPtrs.Itb.DataPtr || CtxPtrs.Itb.ByteCount < NEXUS_RAVE_P_ITB_SIZE || CtxPtrs.Itb.ByteCount > 0x1000000) {
        return false;
    }

    /*
    ** If the entire pic is in the CDB, there will be a startcode at the end of the ITB.
    ** Each ITB entry is NEXUS_RAVE_P_ITB_SIZE bytes, so back up to the start of the last entry.
    */

    ItbByte = CtxPtrs.Itb.DataPtr + ( CtxPtrs.Itb.ByteCount - NEXUS_RAVE_P_ITB_SIZE );
    NEXUS_FlushCache(ItbByte, NEXUS_RAVE_P_ITB_SIZE);

#if BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE
    /* detect valid video start code entry */
    if(ItbByte[3] != ITB_START_CODE) {
        return false;
    }
    for(i=0;i<NEXUS_RAVE_P_ITB_SIZE;i++)
    {
        /* ignore offset byte */
        if(i%2) {
            if(ItbByte[i]==startCode)
                return true;
        }
    }
#else
    if(ItbByte[0] != ITB_START_CODE)
        return false;
    for(i=0;i<NEXUS_RAVE_P_ITB_SIZE;i++)
    {
        /* ignore offset byte */
        if(i%2)
            continue;
        if(ItbByte[i]==startCode)
            return true;
    }
#endif
    return false;
}

struct NEXUS_Rave_P_VideoStartCodeCountThresholdDetector
{
    unsigned count;
    unsigned threshold;
};

#if NEXUS_VIDEO_DECODER_GARBAGE_FIFO_WATCHDOG_SUPPORT
static bool NEXUS_Rave_P_CompareVideoStartCodeCount(void * ctx, const NEXUS_Rave_P_ItbEntry * entry)
{
    struct NEXUS_Rave_P_VideoStartCodeCountThresholdDetector * pDetector = ctx;
    unsigned type = B_GET_BITS(entry->word[0], 31, 24);

    BDBG_CASSERT(brave_itb_video_start_code ==     (unsigned)NEXUS_Rave_P_ItbType_eExtractedData);
    BDBG_CASSERT(brave_itb_base_address ==         (unsigned)NEXUS_Rave_P_ItbType_eBaseAddress);
    BDBG_CASSERT(brave_itb_pts_dts ==              (unsigned)NEXUS_Rave_P_ItbType_ePtsDts);
    BDBG_CASSERT(brave_itb_pcr_offset ==           (unsigned)NEXUS_Rave_P_ItbType_ePcrOffset);
    BDBG_CASSERT(brave_itb_btp ==                  (unsigned)NEXUS_Rave_P_ItbType_eBtp);
    BDBG_CASSERT(brave_itb_private_hdr ==          (unsigned)NEXUS_Rave_P_ItbType_ePrivateHdr);
    BDBG_CASSERT(brave_itb_rts ==                  (unsigned)NEXUS_Rave_P_ItbType_eRts);
    BDBG_CASSERT(brave_itb_pcr ==                  (unsigned)NEXUS_Rave_P_ItbType_ePcr);
    BDBG_CASSERT(brave_itb_ip_stream_out ==        (unsigned)NEXUS_Rave_P_ItbType_eIpStreamOut);
    BDBG_CASSERT(brave_itb_termination ==          (unsigned)NEXUS_Rave_P_ItbType_eTermination);

    if (type == NEXUS_Rave_P_ItbType_eExtractedData)
    {
        pDetector->count++;
        if (pDetector->count >= pDetector->threshold) return true;
    }

    return false;
}

bool NEXUS_Rave_CompareVideoStartCodeCount_priv(
    NEXUS_RaveHandle rave,
    unsigned threshold
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct NEXUS_Rave_P_VideoStartCodeCountThresholdDetector detector;
    bool thresholdMet = false;

    detector.count = 0;
    detector.threshold = threshold;
    rc = NEXUS_Rave_ScanItb_priv(rave, NEXUS_Rave_P_CompareVideoStartCodeCount, (void *)&detector);
    if (rc == NEXUS_SUCCESS)
    {
        thresholdMet = true;
    }

    return thresholdMet;
}
#endif

void NEXUS_Rave_GetAudioFrameCount_priv(
    NEXUS_RaveHandle rave,
    unsigned *pFrameCount
    )
{
    BXPT_Rave_ContextPtrs ptrs;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(NULL != pFrameCount);

    *pFrameCount = 0;
    rc = BXPT_Rave_CheckBuffer(rave->raveHandle, &ptrs);
    if (rc == BERR_SUCCESS) {
        *pFrameCount = (ptrs.Itb.ByteCount+ptrs.Itb.WrapByteCount) / NEXUS_RAVE_P_ITB_SIZE;
    }
}

NEXUS_Error NEXUS_Rave_SetCdbThreshold_priv(
    NEXUS_RaveHandle rave,
    unsigned cdbDepth       /* CDB threshold in bytes (0 is default) */
    )
{
    BERR_Code errCode;
    BXPT_Rave_ContextThresholds thresholds;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    errCode = BXPT_Rave_GetDefaultThresholds(rave->raveHandle, &thresholds);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( cdbDepth > 0 )
    {
        size_t cdbUpper = (cdbDepth + 255)/256; /* Specified in units of 256 bytes */
        /* Only permit values less than default */
        if ( cdbUpper < thresholds.CdbUpper )
        {
            thresholds.CdbUpper = cdbUpper;
        }
    }

    errCode = BXPT_Rave_SetThresholds(rave->raveHandle, &thresholds);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_Rave_GetCdbPointers_isr( NEXUS_RaveHandle rave, uint32_t *validPointer, uint32_t *readPointer)
{
    BAVC_XptContextMap *pXptContextMap;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    BKNI_ASSERT_ISR_CONTEXT();

    pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
    *validPointer = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Valid);
    *readPointer = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->CDB_Read);
}

NEXUS_Error NEXUS_Rave_GetPtsRange_priv( NEXUS_RaveHandle rave, uint32_t *pMostRecentPts, uint32_t *pLeastRecentPts )
{
    BAVC_XptContextMap *pXptContextMap;
    uint32_t valid_offset, read_offset, base_offset, end_offset;
    uint32_t *valid, *read, *base, *end, *itb_mem;
    bool foundPts=false;
    unsigned itr = 0; /* debug stats */
    const NEXUS_P_HwPidChannel *hwPidChannel;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_ASSERT_MODULE();

    hwPidChannel = rave->pidChannel;
    if(hwPidChannel && hwPidChannel->status.originalTransportType == NEXUS_TransportType_eEs) { /* don't try to look for PTS'es in ES streams */
        return NEXUS_NOT_SUPPORTED;
    }

    pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(rave);
    valid_offset = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_Valid);
    valid_offset -= valid_offset % NEXUS_RAVE_P_ITB_SIZE; /* VALID points to last byte in ITB entry, move to first byte for easy algo */
    read_offset = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_Read);
    read_offset -= read_offset % NEXUS_RAVE_P_ITB_SIZE; /* READ may not be updated in ITB units */
    base_offset = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_Base);
    end_offset = BREG_Read32(g_pCoreHandles->reg, pXptContextMap->ITB_End);
    end_offset -= end_offset % NEXUS_RAVE_P_ITB_SIZE; /* END points to last byte in ITB entry, move to first byte for easy algo */

    /* validate that we're in range */
    if (!base_offset || !end_offset || valid_offset < base_offset || read_offset < base_offset) {
        return NEXUS_UNKNOWN;
    }

    valid = NEXUS_OffsetToCachedAddr(valid_offset);
    if(valid==NULL) {return BERR_TRACE(NEXUS_UNKNOWN);}

    read = NEXUS_OffsetToCachedAddr(read_offset);
    if(read==NULL) {return BERR_TRACE(NEXUS_UNKNOWN);}

    base = NEXUS_OffsetToCachedAddr(base_offset);
    if(base==NULL) {return BERR_TRACE(NEXUS_UNKNOWN);}

    end = NEXUS_OffsetToCachedAddr(end_offset);
    if(end==NULL) {return BERR_TRACE(NEXUS_UNKNOWN);}

    itb_mem = valid;
    do {
        int itb_type;
        NEXUS_FlushCache(itb_mem,NEXUS_RAVE_P_ITB_SIZE);

        itb_type = (itb_mem[0]>>24) & 0xFF;
        /* 0x21 for unified ITB, 0x2 for legacy ITB */
        if (itb_type == 2 || itb_type == 0x21) {
            uint32_t pts = itb_mem[1];
            BDBG_MSG(("Found Pts pts %#x, %d itrs", pts, itr));
            /* PTS found */
            if ( !foundPts )
            {
                if ( pMostRecentPts )
                {
                    *pMostRecentPts = pts;
                }
                foundPts = true;
            }
            if ( pLeastRecentPts )
            {
                /* Keep searching for older PTS */
                *pLeastRecentPts = pts;
            }
            else
            {
                /* No need to keep searching */
                break;
            }
        }
        if (itb_mem == read) { /* if valid == read, we have just processed the last element. */
            break;
        }

        itb_mem -= NEXUS_RAVE_P_ITB_SIZE/sizeof(itb_mem[0]); /* walk backwards */
        itr++;

        if (itb_mem < base) {
            itb_mem = end; /* end points to the last valid */
        }
    } while (itb_mem != valid); /* this should not occur if read pointer is in range */

    if ( foundPts )
    {
        return NEXUS_SUCCESS;
    }
    else
    {
        BDBG_MSG(("No Pts found, %d itrs", itr));
        return NEXUS_UNKNOWN;
    }
}

int nexus_rave_add_one_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannelHandle pidChannel)
{
    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    return BXPT_Rave_AddPidChannel(rave->raveHandle, pidChannel->status.pidChannelIndex, false);
}

void nexus_rave_remove_one_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannelHandle pidChannel)
{
    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    (void)BXPT_Rave_RemovePidChannel(rave->raveHandle, pidChannel->status.pidChannelIndex);
}

static int nexus_rave_add_pid(NEXUS_RaveHandle rave, NEXUS_P_HwPidChannel *pidChannel)
{
    int rc;
    NEXUS_P_HwPidChannel *slave;
    struct NEXUS_Rave_P_ErrorCounter_Link* raveLink;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    if (pidChannel->master) {
        /* can't add slave here */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    rc = nexus_rave_add_one_pid(rave, pidChannel);
    if (rc) return BERR_TRACE(rc);

    rave->pidChannel = pidChannel;
    BLST_S_INSERT_HEAD(&pidChannel->raves, rave, pidchannel_link);

    for (slave = BLST_S_FIRST(&pidChannel->slaves);slave;slave = BLST_S_NEXT(slave,slave_link)) {
        rc = nexus_rave_add_one_pid(rave, slave);
        if (rc) {rc = BERR_TRACE(rc);goto error;}
    }
    pidChannel->destinations |= NEXUS_PIDCHANNEL_P_DESTINATION_RAVE;

    /* alloc a rave error counter container and add to this pid channel */
    raveLink = BKNI_Malloc(sizeof(*raveLink));
    if (raveLink==NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); /* keep going */
    }
    else {
        raveLink->counter = &rave->raveErrors;
        BLST_S_DICT_ADD(&pidChannel->raveCounters, raveLink, NEXUS_Rave_P_ErrorCounter_Link, counter, pidchannel_link, err_duplicate);
    }

    return 0;

error:
    nexus_rave_remove_pid(rave);
    return rc;
err_duplicate:
    BKNI_Free(raveLink);
    return 0;
}

static void nexus_rave_remove_pid(NEXUS_RaveHandle rave)
{
    NEXUS_P_HwPidChannel *slave;
    struct NEXUS_Rave_P_ErrorCounter_Link* raveLink;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);
    if (!rave->pidChannel) {
        return;
    }

    rave->pidChannel->destinations &= ~(NEXUS_PIDCHANNEL_P_DESTINATION_RAVE);
    BLST_S_DICT_REMOVE(&rave->pidChannel->raveCounters, raveLink, &rave->raveErrors, NEXUS_Rave_P_ErrorCounter_Link, counter, pidchannel_link);
    if (raveLink) {
        BKNI_Free(raveLink);
    }
    else {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); /* keep going */
    }

    nexus_rave_remove_one_pid(rave, rave->pidChannel);
    BLST_S_REMOVE(&rave->pidChannel->raves, rave, NEXUS_Rave, pidchannel_link);

    for (slave = BLST_S_FIRST(&rave->pidChannel->slaves);slave;slave = BLST_S_NEXT(slave,slave_link)) {
        nexus_rave_remove_one_pid(rave, slave);
    }
    rave->pidChannel = NULL;
}
