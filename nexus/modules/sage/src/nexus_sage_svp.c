/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_security_client.h"
#include "nexus_security.h"
#endif
#include "priv/nexus_security_regver_priv.h"

#include "bsagelib_boot.h"
#include "bkni.h"

#include "secure_video_module_ids.h"
#include "secure_video_command_ids.h"

#include "nexus_sage_image.h"


BDBG_MODULE(nexus_sage_svp);

#define SAGERESPONSE_TIMEOUT 5000 /* in ms */
#define SAGERESPONSE_TOGGLE_TIMEOUT 20000 /* in ms */
#define SAGE_SVP_MAX_SHARED_BLOCK_MEM 2 /* At most 2 blocks passed to SAGE */

#define SAGE_SVP_INDICATION_QUEUE_SIZE 5

union sageSvpSharedMem
{
    BAVC_CoreList coreList;
    uint64_t heapInfo[BCHP_P_MEMC_COUNT+1]; /* URR(s) + XRR */
    BDTU_RemapSettings remapSettings;
};

struct sageSvpInfo {
    BSAGElib_ClientHandle sagelibClientHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
    BSAGElib_InOutContainer *sageContainer;
    uint32_t uiLastAsyncId;
    BKNI_EventHandle response;
    BKNI_EventHandle indication;
    BKNI_EventHandle initEvent;
    bool init; /* SVP delayed init complete */
    uint8_t *pSharedMem[SAGE_SVP_MAX_SHARED_BLOCK_MEM];
    struct
    {
        BSAGElib_RpcRemoteHandle sageRpcHandle;
        uint32_t indication_id;
        uint32_t value;
    } indicationData[SAGE_SVP_INDICATION_QUEUE_SIZE];
    uint8_t secureGfxHeapIndex;
    uint8_t v3dRefCount;
    struct {
        unsigned refcnt, toggles;
    } aeCoreStat[BAVC_CoreId_eMax];
    uint32_t apiVer;
    unsigned indicationReadPtr;
    unsigned indicationWritePtr;
    NEXUS_EventCallbackHandle eventIndicationRecvCallback;
    NEXUS_SageModuleInternalSettings *internalSettings;
};

static const struct {
    uint32_t region_id;
    uint8_t mem_cnt;
} g_BSAGELIB_RegionIdMap[] = {
    {BSAGElib_RegionId_Urr0, 0},
    {BSAGElib_RegionId_Urr1, 1},
    {BSAGElib_RegionId_Urr2, 2},
};

static struct sageSvpInfo *lHandle;

NEXUS_SageMemoryBlock svp_ta;         /* raw ta binary in memory */

static NEXUS_Error NEXUS_Sage_P_EnableHvd(bool enable);

/* Does some sanity checks. To be used before trying to do any communication
* with the SAGE side SVP code */
static NEXUS_Error NEXUS_Sage_SVP_isReady(void)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

    if(!lHandle)
    {
        rc=BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    if(!lHandle->init)
    {
        rc = BKNI_WaitForEvent(lHandle->initEvent, SAGERESPONSE_TIMEOUT);
        if (rc == BERR_TIMEOUT)
        {
            BDBG_ERR(("%s: Timeout (%dms) waiting for SVP Init",
                BSTD_FUNCTION, SAGERESPONSE_TIMEOUT));
            rc=BERR_TRACE(NEXUS_NOT_INITIALIZED);
            goto EXIT;
        }
        else if (rc)
        {
            rc=BERR_TRACE(NEXUS_NOT_INITIALIZED);
            goto EXIT;
        }
    }

    if(!lHandle->hSagelibRpcModuleHandle)
    {
        rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto EXIT;
    }

EXIT:
    return rc;
}

static void NEXUS_Sage_SVP_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_SVP_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
)
{
    BSTD_UNUSED(async_argument);

    /* Save information for later use */
    lHandle->indicationData[lHandle->indicationWritePtr].sageRpcHandle = sageRpcHandle;
    lHandle->indicationData[lHandle->indicationWritePtr].indication_id = indication_id;
    lHandle->indicationData[lHandle->indicationWritePtr].value = value;

    if (++lHandle->indicationWritePtr >= SAGE_SVP_INDICATION_QUEUE_SIZE)
    {
        lHandle->indicationWritePtr = 0;
    }

    if (lHandle->indicationWritePtr == lHandle->indicationReadPtr)
    {
        BDBG_ERR(("[%s] Indication queue overflow - increase queue size", BSTD_FUNCTION));
        /* Drop the oldest to prevent any wrap type condition */
        if (++lHandle->indicationReadPtr >= SAGE_SVP_INDICATION_QUEUE_SIZE)
        {
            lHandle->indicationReadPtr = 0;
        }
    }

    BKNI_SetEvent_isr(lHandle->indication);

    return;
}

static void NEXUS_Sage_SVP_P_SageIndicationEventHandler(void *pContext)
{
    uint32_t id, value;

    BSTD_UNUSED(pContext);

    while (1)
    {
        BKNI_EnterCriticalSection();
        if(lHandle->indicationReadPtr == lHandle->indicationWritePtr)
        {
            /* All done */
            BKNI_LeaveCriticalSection();
            break;
        }
        id = lHandle->indicationData[lHandle->indicationReadPtr].indication_id;
        value = lHandle->indicationData[lHandle->indicationReadPtr].value;
        if (++lHandle->indicationReadPtr >= SAGE_SVP_INDICATION_QUEUE_SIZE)
        {
            lHandle->indicationReadPtr = 0;
        }
        BKNI_LeaveCriticalSection();

        switch(id)
        {
            case bvn_monitor_IndicationType_eLockdown:
                BDBG_ERR(("============== SVP BVN LOCKDOWN STATUS: %s %s %s %s %s %s ==============",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eNone)) ? "CLEAR" : "LOCKDOWN",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eGeneral)) ? "GENERAL" : "",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eBvn)) ? "BVN" : "",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eHdmiRx)) ? "HDMI-RX" : "",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eHDR)) ? "HDR" : "",
                           (value & (1 << bvn_monitor_Indication_Lockdown_eMacrovision)) ? "Macrovision" : ""));
                break;
            default:
                BDBG_ERR(("SAGE SVP received an unknown indication (0x%x)", id));
                break;
        }
    }
}

static BERR_Code NEXUS_Sage_SVP_P_WaitForSage(int timeoutMsec)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_ResponseData data;

    if (lHandle->uiLastAsyncId == 0)
    {
        rc = BERR_SUCCESS;
        goto done;
    }

    /* Wait for response from sage  */
    rc = BKNI_WaitForEvent(lHandle->response, timeoutMsec);
    if (rc == BERR_TIMEOUT)
    {
        BDBG_ERR(("%s: Timeout (%dms) waiting for sage response from previous request",
            BSTD_FUNCTION, timeoutMsec));
        rc = BERR_TRACE(rc);
        goto done;
    }
    else if (rc)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    /* Get Sage response */
    rc = BSAGElib_Client_GetResponse(lHandle->sagelibClientHandle, &data);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    if(data.rc != BERR_SUCCESS)
    {
        rc = data.rc;
        if(data.rc == BSAGE_ERR_PLATFORM_ID)
        {
            BDBG_WRN(("SAGE WARNING: Unknown Platform"));
        }
        else
        {
            BDBG_ERR(("SAGE ERROR: 0x%x (%s)", rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        }
    }
    else
    {
        if (lHandle->sageContainer->basicOut[SECURE_VIDEO_OUT_RETCODE] != BERR_SUCCESS)
        {
            rc = BERR_TRACE(lHandle->sageContainer->basicOut[SECURE_VIDEO_OUT_RETCODE]);
        }
    }

done:

    return rc;
}

/* Note... this WILL result in nexus/sage getting out of sync... this should be
fine as long as any nexus app is following the rules... but you could shoot
yourself in the foot if you try to do dynamic urr */
/* The long and short... any attempts to toggle either gfx or urr, will result in
* ALL gfx/urr getting toggled. This should be fine if the app intends to toggle
* everything (i.e. pre-dynamic-urr) and correctly walks through all nexus heaps */
static NEXUS_Error NEXUS_Sage_SVP_P_UpdateHeaps_2_5(bool disable)
{
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_KeySlotHandle scrubbingKeyHandle = 0;
    NEXUS_SecurityKeySlotInfo  keyslotInfo;
    uint8_t *pDmaMemoryPool = NULL;
    unsigned size = 4*1024;
    /* Actually missed deleting this enum... not needed w/ new code */
    secureVideo_Toggle_e xrr=bvn_monitor_Command_eIgnore, urr=bvn_monitor_Command_eIgnore;
    unsigned heapIndex;
    BERR_Code rc;

    if(disable)
    {
        /* Disable everything */
        xrr=bvn_monitor_Command_eDisable;
        urr=bvn_monitor_Command_eDisable;
    }
    else
    {
        urr=bvn_monitor_Command_eEnable;

        for (heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
            NEXUS_MemoryStatus status;

            NEXUS_HeapHandle heap = g_pCoreHandles->heap[heapIndex].nexus;
            if (!heap)
            {
                continue;
            }

            rc = NEXUS_Heap_GetStatus(heap, &status);
            if (rc!=NEXUS_SUCCESS)
            {
                continue;
            }

            if (!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE))
            {
                continue;
            }

            if((status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) ||
                (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT) ||
                (status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS))
            {
                if(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF)
                {
                    /* Anything marked as unsecure means we toggle everything off */
                    urr=bvn_monitor_Command_eDisable;
                }
            }

        }
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[1]=(int32_t)urr;
    lHandle->sageContainer->basicIn[2]=(int32_t)xrr;

    if((urr==bvn_monitor_Command_eDisable) || (xrr==bvn_monitor_Command_eDisable))
    {
        /* Only need to allocate resources on disable */
        NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
        keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
        keySlotSettings.client = NEXUS_SecurityClientType_eSage;
        scrubbingKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
        if(scrubbingKeyHandle == NULL)
         {
            rc = NEXUS_UNKNOWN;
            BDBG_ERR(("NEXUS_Security_AllocateKeySlot() failure"));
            goto EXIT;
         }

        NEXUS_Security_GetKeySlotInfo(scrubbingKeyHandle, &keyslotInfo);
        lHandle->sageContainer->basicIn[3] = (int32_t)keyslotInfo.keySlotNumber;
        BDBG_MSG(("%s - keyslotInfo.keySlotNumber %d", BSTD_FUNCTION, keyslotInfo.keySlotNumber));

        /* Allocate enough memory for DMA descriptors */
        pDmaMemoryPool=BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle, size, BSAGElib_MemoryType_Restricted);
        if(pDmaMemoryPool == NULL)
        {
            rc = NEXUS_UNKNOWN;
            BDBG_ERR(("%s - Error calling SRAI_Memory_Allocate()", BSTD_FUNCTION));
            goto EXIT;
        }

        lHandle->sageContainer->blocks[0].data.ptr  = pDmaMemoryPool;
        lHandle->sageContainer->blocks[0].len = size;
    }

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eToggle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eToggle, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
    }

EXIT:
    if(pDmaMemoryPool)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, pDmaMemoryPool);
    }
    if(scrubbingKeyHandle)
    {
        NEXUS_Security_FreeKeySlot(scrubbingKeyHandle);
    }

    return rc;
#else
    BSTD_UNUSED( disable );
    BDBG_ERR(("%s: TBD not supported", BSTD_FUNCTION));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

/** Update heap info
* Called "on the fly" to handle enable/disable/shrink/grow of
* URR/XRR. On the "disable" case (only for shutdown/s3), also make
* sure any additional heaps (FWRR) are also cleaned up
*/
static NEXUS_Error NEXUS_Sage_SVP_P_UpdateHeaps(bool disable)
{
    unsigned heapIndex;
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint64_t *start=(uint64_t *)lHandle->pSharedMem[0];
    uint64_t *size=(uint64_t *)lHandle->pSharedMem[1];

#ifdef NEXUS_SAGE_SVP_TEST
    /* TODO */
#endif

    rc=NEXUS_Sage_SVP_isReady();
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    switch(lHandle->apiVer)
    {
        case 0x00020005:
            return NEXUS_Sage_SVP_P_UpdateHeaps_2_5(disable);
        default:
            break;
    }

    BKNI_Memset(lHandle->pSharedMem[0], 0, sizeof(union sageSvpSharedMem));
    BKNI_Memset(lHandle->pSharedMem[1], 0, sizeof(union sageSvpSharedMem));

    for (heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
        NEXUS_MemoryStatus status;

        NEXUS_HeapHandle heap = g_pCoreHandles->heap[heapIndex].nexus;
        if (!heap)
        {
            continue;
        }

        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc!=NEXUS_SUCCESS)
        {
            continue;
        }

        if (!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE))
        {
            continue;
        }

        if(status.heapType & NEXUS_HEAP_TYPE_EXPORT_REGION)
        {
            start[BCHP_P_MEMC_COUNT]=status.offset;
            if(!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF))
            {
                size[BCHP_P_MEMC_COUNT]=status.size;
            }
            continue;
        }

        if((status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) ||
            (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT) ||
            (status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS))
        {
            if((start[status.memcIndex]==0) ||
                ((size[status.memcIndex]==0) && !(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF)))
            {
                start[status.memcIndex]=(uint32_t)status.offset;
            }

            if(!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF))
            {
                size[status.memcIndex]+=status.size;
            }
        }
    }

    if(disable)
    {
        BKNI_Memset(size, 0, sizeof(uint64_t)*(BCHP_P_MEMC_COUNT+1));
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_UPDATEHEAPS_IN_COUNT]=BCHP_P_MEMC_COUNT+1;
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_START].len=sizeof(uint64_t)*(BCHP_P_MEMC_COUNT+1);
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_START].data.ptr=(uint8_t *)start;
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_SIZE].len=sizeof(uint64_t)*(BCHP_P_MEMC_COUNT+1);
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_SIZE].data.ptr=(uint8_t *)size;

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eUpdateHeaps, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eUpdateHeaps, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
    }

    /* Special case for disable.. i.e. it's a cleanup for shutdown or S3...
    * also clean up FWRR */
    if(disable && g_pCoreHandles->heap[NEXUS_FIRMWARE_HEAP].nexus &&
       (lHandle->apiVer>=0x00020009))
    {
        rc = NEXUS_Sage_P_EnableHvd(false);
        if (rc != BERR_SUCCESS)
        {
            rc = BERR_TRACE(rc);
        }
    }

EXIT:
    return rc;
}

void NEXUS_Sage_P_SvpUninit(void)
{
    BDBG_ASSERT(lHandle);

    /* Destroy event(s) */
    if (lHandle->response)
    {
        BKNI_DestroyEvent(lHandle->response);
        lHandle->response = NULL;
    }

    if (lHandle->eventIndicationRecvCallback) {
        NEXUS_UnregisterEvent(lHandle->eventIndicationRecvCallback);
        lHandle->eventIndicationRecvCallback = NULL;
    }

    if (lHandle->indication)
    {
        BKNI_DestroyEvent(lHandle->indication);
        lHandle->indication = NULL;
    }

    if (lHandle->initEvent)
    {
        BKNI_DestroyEvent(lHandle->initEvent);
        lHandle->initEvent = NULL;
    }

    /* Free local info */
    BKNI_Free(lHandle);
    lHandle=NULL;
}

/** Handle different sage svp ta api versions
*
* This is the SVP TA "api" version, and is not directly tied to any higher "svp version". Supporting
* older handled by disabling features and/or adjusting api usage. Supporting newer svp ta may be possible
* based on info provided by svp module init.
*/
static void NEXUS_Sage_P_SvpHandleApiVer(uint32_t sageApiVersion, uint32_t minCompat)
{
    if(sageApiVersion==lHandle->apiVer)
    {
        /* Everything is in sync */
        return;
    }

    BDBG_MSG(("HOST (0x%08x) vs. SAGE (0x%08x)", lHandle->apiVer, sageApiVersion));

    switch(sageApiVersion)
    {
        case 0: /* Intentional fall-through.. */
                /* Pre 0x00020006, SAGE did not return a version (i.e. 0) */
                /* No Pre 0x00020006 release made with sage 3.x other than 0x00020005 */
        case 0x00020005: /* Can handle this, no hdmi/dtu/fwrr. Use "legacy" update heaps code */
            BDBG_WRN(("OLDER SAGE SVP API VERSION SET DETECTED! Secure HDMI Rx/Secure DTU remap/FWRR NOT supported"));
            lHandle->apiVer=0x00020005;
            break;
        case 0x00020006: /* Can handle this, no hdmi/dtu/fwrr */
            BDBG_WRN(("OLDER SAGE SVP API VERSION SET DETECTED! Secure HDMI Rx/Secure DTU remap/FWRR NOT supported"));
            lHandle->apiVer=0x00020006;
            break;
        case 0x00020007: /* Can handle this, no dtu/fwrr */
            BDBG_WRN(("OLDER SAGE SVP API VERSION SET DETECTED! Secure DTU remap/FWRR NOT supported"));
            lHandle->apiVer=0x00020007;
            break;
        case 0x00020008: /* Can handle this, no fwrr */
            BDBG_WRN(("OLDER SAGE SVP API VERSION SET DETECTED! FWRR NOT supported"));
            lHandle->apiVer=0x00020008;
            break;
        default:
            if(sageApiVersion>=0x00020009)
            {
                /* minCompat is valid. Forward compatibility MAY be allowed/supported */
                /* This can happen when no existing api has been removed or changed */
                if(SECURE_VIDEO_VER_ID>=minCompat)
                {
                    BDBG_WRN(("NEWER SAGE SVP API VERSION SET DETECTED! But it is compatible"));
                    break;
                }
            }
            BDBG_ERR(("INCOMPATIBLE SAGE SVP API VERSION SET. SVP WILL NOT BE FUNCTIONAL"));
            return; /* Do not update version reported to SAGE. SAGE will reject all commands */
    }

    /* Update version reported to SAGE */
    lHandle->apiVer=sageApiVersion;
}

/* Some of the init needs to be delayed until SAGE is running */
/* TODO: Move some of this (platform open/init, module open/init, into
* more generic functions that can be used across nexus */
void NEXUS_Sage_P_SvpInitDelayed(void *pParam)
{
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code rc;
    int i;
    NEXUS_SageImageHolder svpTAImg =
        {"SVP TA", SAGE_IMAGE_FirmwareID_eSage_TA_SVP, NULL};

    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;

    BSTD_UNUSED(pParam);

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot Create IMG", BSTD_FUNCTION));
    }

    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        svpTAImg.id = SAGE_IMAGE_FirmwareID_eSage_TA_SVP_Development;
    }

    svp_ta.buf = NULL;
    svp_ta.len = 0;
    svpTAImg.raw = &svp_ta;

    /* Load SVP TA into memory */
    rc = NEXUS_SageModule_P_Load(&svpTAImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, svpTAImg.name));
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_SVP_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_SVP_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    if(svp_ta.buf != NULL)
    {
        /* Install SVP TA bin */
        rc = BSAGElib_Rai_Platform_Install(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_VIDEO,
                            svp_ta.buf, svp_ta.len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Could not install SVP TA binary, assuming it is pre-loaded - buff[0x%p], size[%lu]",
                                        svp_ta.buf, (unsigned long)svp_ta.len));

            /* fall through, assuming TA was already loaded*/
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);
    }

    /* Allocate a single container and reuse */
    lHandle->sageContainer = BSAGElib_Rai_Container_Allocate(lHandle->sagelibClientHandle);
    if (lHandle->sageContainer == NULL)
    {
        BDBG_ERR(("Error allocating BSAGElib_InOutContainer"));
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto EXIT;
    }

    /* Allocate some shared memory and re-use */
    for(i = 0;i < SAGE_SVP_MAX_SHARED_BLOCK_MEM;i++)
    {
        lHandle->pSharedMem[i] = BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle, sizeof(union sageSvpSharedMem), BSAGElib_MemoryType_Global);
        if(!lHandle->pSharedMem[i])
        {
            BDBG_ERR(("Failed to allocate shared memory (%d)", i));
            rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            goto EXIT;
        }
    }

    /* Open SVP platform */
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_VIDEO,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE SVP Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers and no XRR/eXport */
            BDBG_WRN(("SVP will not be possible"));
        }
        else
        {
            rc = BERR_TRACE(rc);
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
        * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened SVP SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SVP platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_SVP_INVALID_URR)
        {
            /* Note error, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no access to CRR */
            BDBG_ERR(("INVALID URR(s) specified! Current SAGE binary requires secure picture buffers!"));
            BDBG_ERR(("Please correctly configure heaps to allow for SVP."));
            BDBG_ERR(("ALL ACCESS to NEXUS_VIDEO_SECURE_HEAP blocked. Video playback NOT possible!"));
        }
        else
        {
            rc = BERR_TRACE(rc);
        }
        goto EXIT;
    }
    BDBG_MSG(("Initialized SVP SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* Init SVP Monitor Module */
    /* Doesn't seem to be a way to query of a module has been initialized or not.... */
    /* TODO */
    rc = BSAGElib_Rai_Module_Init(lHandle->hSagelibRpcPlatformHandle, secure_video_ModuleId_ebvn_monitor,
        lHandle->sageContainer, &lHandle->hSagelibRpcModuleHandle,  /*out */
        (void *) lHandle, &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SVP Monitor module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        rc = BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Initialized SAGE SVP Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));

    NEXUS_Sage_P_SvpHandleApiVer(lHandle->sageContainer->basicOut[SECURE_VIDEO_OUT_VER],
                                 lHandle->sageContainer->basicOut[SECURE_VIDEO_OUT_VER_COMPAT_MIN]);

EXIT:
    /* Init complete */
    lHandle->init = true;
    BKNI_SetEvent(lHandle->initEvent);

    /* This has to be after event */
    if(lHandle->hSagelibRpcModuleHandle)
    {
        NEXUS_Sage_SVP_P_UpdateHeaps(false);
    }

    BDBG_MSG(("SAGE SVP init complete (0x%x)", rc));
#if 0
    if (img_context) {
        NEXUS_SVP_P_Img_Destroy(img_context);
    }
#else
if (img_context) {
    Nexus_SageModule_P_Img_Destroy(img_context);
}
#endif
    return;
}

/*NEXUS_Sage_P_GetBSAGElib_RegionId is the fix for Coverity Issue: SWSTB-2217*/
static uint32_t NEXUS_Sage_P_GetBSAGElib_RegionId(uint8_t bchp_mem_count)
{
    unsigned i;
    for (i = 0; i < sizeof(g_BSAGELIB_RegionIdMap) / sizeof(g_BSAGELIB_RegionIdMap[0]); i++)
    {
        if (g_BSAGELIB_RegionIdMap[i].mem_cnt == bchp_mem_count)
        {
            return g_BSAGELIB_RegionIdMap[i].region_id;
        }
    }
    return BSAGElib_RegionId_eInvalid;
}

NEXUS_Error NEXUS_Sage_P_SvpInit(NEXUS_SageModuleInternalSettings *internalSettings)
{
    BERR_Code rc;

    if(!internalSettings)
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if(lHandle)
    {
        /* Shouldn't get here */
        NEXUS_Sage_P_SvpStop(true);
        NEXUS_Sage_P_SvpUninit();
    }

    lHandle=BKNI_Malloc(sizeof(*lHandle));
    if(!lHandle)
    {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(lHandle, 0, sizeof(*lHandle));
    lHandle->secureGfxHeapIndex=NEXUS_MAX_HEAPS;
    lHandle->internalSettings=internalSettings;

    rc = BKNI_CreateEvent(&lHandle->response);
    rc |= BKNI_CreateEvent(&lHandle->indication);
    rc |= BKNI_CreateEvent(&lHandle->initEvent);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(rc);
        goto ERROR_EXIT;
    }

    /* register event */
    lHandle->eventIndicationRecvCallback = NEXUS_RegisterEvent(lHandle->indication, NEXUS_Sage_SVP_P_SageIndicationEventHandler, (void *)lHandle);
    if (lHandle->eventIndicationRecvCallback == NULL)
    {
        rc = BERR_TRACE(rc);
        goto ERROR_EXIT;
    }

    lHandle->apiVer=SECURE_VIDEO_VER_ID;

    return NEXUS_SUCCESS;

ERROR_EXIT:
    NEXUS_Sage_P_SvpUninit();

    return rc;
}

NEXUS_Error NEXUS_Sage_P_SvpStart(void)
{
    if(lHandle->init)
    {
        /* Seems like SVP was already configured, must be a reset or S3 */
        NEXUS_Sage_P_SvpStop(true);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Sage_P_SvpStop(bool reset)
{
    NEXUS_Error rc;
    uint8_t i;

    BDBG_ASSERT(lHandle);

    /* Close SVP:Monitor module */
    if (lHandle->hSagelibRpcModuleHandle != NULL)
    {
        if(!reset) /* On a SAGE reset, don't try to communicate */
        {
            /* Scrub first, failure to do so may result in not being able to restart nexus */
            NEXUS_Sage_SVP_P_UpdateHeaps(true);

            BSAGElib_Rai_Module_Uninit(lHandle->hSagelibRpcModuleHandle, &lHandle->uiLastAsyncId);
            rc=NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
            BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
            BDBG_MSG(("Uninit & remove SVP SAGE Module: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));
        }
        lHandle->hSagelibRpcModuleHandle = NULL;
    }

    /* Close SVP Platform */
    if(lHandle->hSagelibRpcPlatformHandle)
    {
        if(!reset) /* On a SAGE reset, don't try to communicate */
        {
            BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
            rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
            if (rc != BERR_SUCCESS){
                rc = BERR_TRACE(rc);
            }
            BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        }
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    if(svp_ta.buf != NULL)
    {
        /* UnInstall SVP TA bin */
        rc = BSAGElib_Rai_Platform_UnInstall(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_VIDEO);
        if (rc != BERR_SUCCESS){
            BDBG_WRN(("Could not UnInstall SVP TA "));
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);

        NEXUS_Memory_Free(svp_ta.buf);
        svp_ta.buf = NULL;
        svp_ta.len = 0;
    }

    /* Free shared memory */
    for(i = 0;i<SAGE_SVP_MAX_SHARED_BLOCK_MEM;i++)
    {
        if(lHandle->pSharedMem[i])
        {
            BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, lHandle->pSharedMem[i]);
            lHandle->pSharedMem[i] = NULL;
        }
    }

    /* Free container */
    if(lHandle->sageContainer)
    {
        BSAGElib_Rai_Container_Free(lHandle->sagelibClientHandle, lHandle->sageContainer);
        lHandle->sageContainer=NULL;
    }

    /* Close BSage client */
    if(lHandle->sagelibClientHandle)
    {
        BSAGElib_CloseClient(lHandle->sagelibClientHandle);
        lHandle->sagelibClientHandle=NULL;
    }
}

NEXUS_Error NEXUS_Sage_P_SvpEnterS3(void)
{
    /* Scrub */
    return NEXUS_Sage_SVP_P_UpdateHeaps(true);
}

NEXUS_Error NEXUS_Sage_P_SvpSetRegions(void)
{
    NEXUS_Addr start[BCHP_P_MEMC_COUNT]={0};
    uint32_t size[BCHP_P_MEMC_COUNT]={0};
    NEXUS_Addr sec_gfx_start=0;
    uint32_t sec_gfx_size=0;
    uint8_t i;
    NEXUS_MemoryStatus status;
    NEXUS_Error rc=NEXUS_SUCCESS;
    uint32_t regionId;

    for (i = 0; i < NEXUS_MAX_HEAPS; i++) {
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[i].nexus;

        if (!heap) continue;
        NEXUS_Heap_GetStatus(heap, &status);

        if(!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE) ||
            !(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS))
            continue;

        if(size[status.memcIndex])
        {
            /* Can't have more than one secure picture buffer per memc */
            rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto EXIT;
        }

        start[status.memcIndex]=status.offset;
        size[status.memcIndex]=status.size;
    }

    for (i = 0; i < NEXUS_MAX_HEAPS; i++) {
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[i].nexus;

        if (!heap) continue;
        NEXUS_Heap_GetStatus(heap, &status);

        if(status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS)
        {
            /* Secure graphics is "above" URR */
            if(!size[status.memcIndex] ||
                ((start[status.memcIndex] + size[status.memcIndex]) != status.offset))
            {
                rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto EXIT;
            }

            /* Only pass 1st secure gfx heap as a separate heap */
            /* Otherwise "add" it to the associated picture buffer heap */
            if(!sec_gfx_start)
            {
                sec_gfx_start=status.offset;
                sec_gfx_size=status.size;
                lHandle->secureGfxHeapIndex=i;
            }
            else
            {
                size[status.memcIndex]+=status.size;
            }
        }

        if(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)
        {
            /* picture buffer extension is "below" URR */
            if(!size[status.memcIndex] ||
                ((status.offset + status.size) != start[status.memcIndex]))
            {
                rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto EXIT;
            }

            start[status.memcIndex]=status.offset;
            size[status.memcIndex]=status.size;
        }
    }

    for (i = 0; i < BCHP_P_MEMC_COUNT; i++)
    {
        regionId = NEXUS_Sage_P_GetBSAGElib_RegionId(i);
        if(BSAGElib_RegionId_eInvalid == regionId)
        {
            continue;
        }
        rc = NEXUS_SageModule_P_AddRegion(regionId, start[i], size[i]);
        if (rc != NEXUS_SUCCESS)
        {
            rc=BERR_TRACE(rc);
            goto EXIT;
        }
    }

    /* Add 1st secure gfx heap if it exists */
    if(sec_gfx_size)
    {
        rc = NEXUS_SageModule_P_AddRegion(BSAGElib_RegionId_SecGfx, sec_gfx_start, sec_gfx_size);
        if (rc != NEXUS_SUCCESS)
        {
            rc=BERR_TRACE(rc);
            goto EXIT;
        }
    }

EXIT:
    return rc;
}

/* Not to be called from within SAGE module itself */
NEXUS_Error NEXUS_Sage_UpdateHeaps(void)
{
    NEXUS_Error rc;

    NEXUS_LockModule();
    rc=NEXUS_Sage_SVP_P_UpdateHeaps(false);
    NEXUS_UnlockModule();

    return rc;
}

/* Not to be called from within SAGE module itself */
NEXUS_Error NEXUS_Sage_AddSecureCores(const BAVC_CoreList *pCoreList)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    uint32_t coreListSize=sizeof(*pCoreList);

    NEXUS_LockModule();

    rc=NEXUS_Sage_SVP_isReady();
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    if(lHandle->apiVer<=0x00020006)
    {
        /* SAGE will be expecting a smaller list size */
        coreListSize-=(2*sizeof(__typeof__(pCoreList->aeCores[0])));
    }

    for (i=0;i<BAVC_CoreId_eMax;i++) {
        if (pCoreList->aeCores[i]) {
            if (lHandle->aeCoreStat[i].refcnt++ == 0) {
                lHandle->aeCoreStat[i].toggles++;
            }
        }
    }

    /* Some check on 3D.. must have a secure gfx heap to allow this */
    if(pCoreList->aeCores[BAVC_CoreId_eV3D_0] || pCoreList->aeCores[BAVC_CoreId_eV3D_1])
    {
        if(lHandle->secureGfxHeapIndex==NEXUS_MAX_HEAPS)
        {
            rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto EXIT;
        }
        lHandle->v3dRefCount+=pCoreList->aeCores[BAVC_CoreId_eV3D_0] ? 1 : 0;
        lHandle->v3dRefCount+=pCoreList->aeCores[BAVC_CoreId_eV3D_1] ? 1 : 0;
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    BKNI_Memset(lHandle->pSharedMem[0], 0, sizeof(union sageSvpSharedMem));
    BKNI_Memcpy(lHandle->pSharedMem[0], pCoreList, sizeof(*pCoreList));
    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_SETCORES_IN_ADD]=true;
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].len = coreListSize;
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].data.ptr = lHandle->pSharedMem[0];

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eSetCores, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eSetCores, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

#ifdef NEXUS_SAGE_SVP_TEST
    NEXUS_Sage_P_SecureCores_test(pCoreList, true);
#endif

EXIT:
    NEXUS_UnlockModule();

    return rc;
}

#if BDBG_DEBUG_BUILD
/* Core Id Table Information */
const struct NEXUS_SageSvpHwBlock g_NEXUS_SvpHwBlockTbl[] = {
#define BCHP_P_MEMC_DEFINE_SVP_HWBLOCK(svp_block, access) { #svp_block },
#include "memc/bchp_memc_svp_hwblock.h"
    {"  -  "},
#undef BCHP_P_MEMC_DEFINE_SVP_HWBLOCK
};

BDBG_FILE_MODULE(nexus_sage_module);
void NEXUS_Sage_P_PrintSvp(void)
{
    unsigned i;
    for (i=0;i<BAVC_CoreId_eMax;i++) {
        /* if it's never toggled, don't bother printing */
        if (lHandle && lHandle->aeCoreStat[i].toggles) {
            BDBG_MODULE_LOG(nexus_sage_module, ("core[%s][%u] refcnt %u, toggles %u", g_NEXUS_SvpHwBlockTbl[i].achName, i, lHandle->aeCoreStat[i].refcnt, lHandle->aeCoreStat[i].toggles));
        }
    }
}
#endif

/* Not to be called from within SAGE module itself */
void NEXUS_Sage_RemoveSecureCores(const BAVC_CoreList *pCoreList)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned ref;
    NEXUS_Addr offset= 0;
    NEXUS_MemoryBlockHandle hMemBlock=NULL;
    unsigned i;
    uint32_t coreListSize=sizeof(*pCoreList);

    NEXUS_LockModule();

    rc=NEXUS_Sage_SVP_isReady();
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    if(lHandle->apiVer<=0x00020006)
    {
        /* SAGE will be expecting a smaller list size */
        coreListSize-=(2*sizeof(__typeof__(pCoreList->aeCores[0])));
    }

    for (i=0;i<BAVC_CoreId_eMax;i++) {
        if (pCoreList->aeCores[i]) {
            if (lHandle->aeCoreStat[i].refcnt == 0) {
                BDBG_ERR(("NEXUS_Sage_RemoveSecureCores: unbalanced refcnt aeCores[%u] == 0", i));
            }
            else if (--lHandle->aeCoreStat[i].refcnt == 0) {
                lHandle->aeCoreStat[i].toggles++;
            }
        }
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    BKNI_Memset(lHandle->pSharedMem[0], 0, sizeof(union sageSvpSharedMem));
    BKNI_Memcpy(lHandle->pSharedMem[0], pCoreList, sizeof(*pCoreList));
    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_SETCORES_IN_ADD]=false;
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].len = coreListSize;
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].data.ptr = lHandle->pSharedMem[0];

    /* Some check on 3D.. must have a secure gfx heap to allow this */
    if(pCoreList->aeCores[BAVC_CoreId_eV3D_0] || pCoreList->aeCores[BAVC_CoreId_eV3D_1])
    {
        if(lHandle->secureGfxHeapIndex==NEXUS_MAX_HEAPS)
        {
            rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto EXIT;
        }

        ref=pCoreList->aeCores[BAVC_CoreId_eV3D_0] ? 1: 0;
        ref+=pCoreList->aeCores[BAVC_CoreId_eV3D_1] ? 1: 0;
        BDBG_ASSERT(lHandle->v3dRefCount>=ref);
        lHandle->v3dRefCount-=ref;

        if(lHandle->v3dRefCount==0)
        {
            /* Fully removing v3d.... need to lock some memory */
            hMemBlock = NEXUS_MemoryBlock_Allocate(g_pCoreHandles->heap[lHandle->secureGfxHeapIndex].nexus,
                        SECURE_VIDEO_V3D_SIZE, SECURE_VIDEO_V3D_ALIGNMENT, NULL);
            if(hMemBlock==NULL)
            {
                rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
                goto EXIT;
            }
            rc = NEXUS_MemoryBlock_LockOffset(hMemBlock, &offset);
            if(rc!=NEXUS_SUCCESS)
            {
                rc = BERR_TRACE(rc);
                goto EXIT;
            }

            /* TODO.. verify casting? */
            lHandle->sageContainer->basicIn[SECURE_VIDEO_SETCORES_IN_V3D_OFFSET]=(uint32_t)offset;
            lHandle->sageContainer->basicIn[SECURE_VIDEO_SETCORES_IN_V3D_SIZE]=SECURE_VIDEO_V3D_SIZE;
        }
    }

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eSetCores, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eSetCores, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

#ifdef NEXUS_SAGE_SVP_TEST
    NEXUS_Sage_P_SecureCores_test(pCoreList, false);
#endif

EXIT:
    if(hMemBlock)
    {
        NEXUS_MemoryBlock_UnlockOffset(hMemBlock);
        NEXUS_MemoryBlock_Free(hMemBlock);
    }

    NEXUS_UnlockModule();
}

/* Not to be called from within SAGE module itself */
NEXUS_Error NEXUS_Sage_SecureRemap(unsigned memcIndex, const BDTU_RemapSettings *pSettings)
{
    NEXUS_Error rc;

    NEXUS_LockModule();

    if(!pSettings)
    {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto EXIT;
    }

    rc = NEXUS_Sage_SVP_isReady();
    if(rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    if(lHandle->apiVer<0x00020008)
    {
        BDBG_ERR(("SAGE/SVP Binaries must be updated"));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto EXIT;
    }

    BKNI_Memset(lHandle->pSharedMem[0], 0, sizeof(union sageSvpSharedMem));
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    BKNI_Memcpy(lHandle->pSharedMem[0], pSettings, sizeof(*pSettings));
    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_SETCORES_IN_ADD] = false;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_REMAP_IN_MEMC] = memcIndex;
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].len = sizeof(*pSettings);
    lHandle->sageContainer->blocks[SECURE_VIDEO_SETCORES_BLOCK_CORELIST].data.ptr = lHandle->pSharedMem[0];

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eSecureRemap, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eUpdateHeaps, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }
    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
    }

EXIT:
    NEXUS_UnlockModule();

    return rc;
}

static NEXUS_Error NEXUS_Sage_P_EnableHvd(bool enable)
{
    NEXUS_Error rc;
    uint64_t *start=(uint64_t *)lHandle->pSharedMem[0];
    uint64_t *size=(uint64_t *)lHandle->pSharedMem[1];
    NEXUS_Addr offset;
    uint32_t size2;
    bool required;

    NEXUS_Module_Lock(lHandle->internalSettings->security);
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    required=NEXUS_Security_RegionVerification_IsRequired_priv(NEXUS_SecurityRegverRegionID_eVDEC0_OLA);
#else
    required=NEXUS_Security_RegionVerification_IsRequired_priv(NEXUS_SecurityRegverRegionID_eAvd0Inner);
#endif
    NEXUS_Module_Unlock(lHandle->internalSettings->security);

    if(lHandle->apiVer<0x00020009)
    {
        if(required)
        {
            BDBG_ERR(("SAGE/SVP Binaries must be updated"));
            /* rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); */
            rc=NEXUS_SUCCESS;
        }
        else
        {
            BDBG_WRN(("SAGE/SVP binaries should be updated. HVD FW auth will not be possible"));
            rc=NEXUS_SUCCESS;
        }
        goto EXIT;
    }
    else {
        if(required && NEXUS_P_Core_SecureArchIssue_isrsafe() && lHandle->internalSettings->lazyUnmap) {
            /* see NEXUS_Platform_P_LazyUnmap() logic for required versions */
            BDBG_ERR(("Linux upgrade required for region verification with HVD FW on this silicon"));
            BKNI_Fail();
        }
    }

    rc = NEXUS_Sage_SVP_isReady();
    if(rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    BKNI_Memset(lHandle->pSharedMem[0], 0, sizeof(union sageSvpSharedMem));
    BKNI_Memset(lHandle->pSharedMem[1], 0, sizeof(union sageSvpSharedMem));

    rc = NEXUS_SageModule_P_GetHeapBoundaries(NEXUS_FIRMWARE_HEAP, &offset, &size2, NULL);
    if(rc!=NEXUS_SUCCESS)
    {
        return BERR_TRACE(rc);
    }
    if(enable) {
        size[0]=size2;
    }
    start[0]=offset;

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->basicIn[SECURE_VIDEO_IN_VER]=lHandle->apiVer;
    lHandle->sageContainer->basicIn[SECURE_VIDEO_UPDATEHEAPS_IN_COUNT]=1;
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_START].len=sizeof(uint64_t)*(1);
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_START].data.ptr=(uint8_t *)start;
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_SIZE].len=sizeof(uint64_t)*(1);
    lHandle->sageContainer->blocks[SECURE_VIDEO_UPDATEHEAPS_BLOCK_SIZE].data.ptr=(uint8_t *)size;

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eUpdateHeaps, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
    }
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eUpdateHeaps, lHandle->uiLastAsyncId));
    rc = NEXUS_Sage_SVP_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
    }

EXIT:
    return rc;
}


NEXUS_Error NEXUS_Sage_EnableHvd(void)
{
    return NEXUS_Sage_P_EnableHvd(true);
}

void NEXUS_Sage_DisableHvd(void)
{
    NEXUS_Error rc;

    rc=NEXUS_Sage_P_EnableHvd(false);
    if(rc!=NEXUS_SUCCESS)
    {
        BERR_TRACE(rc);
    }

    return;
}
