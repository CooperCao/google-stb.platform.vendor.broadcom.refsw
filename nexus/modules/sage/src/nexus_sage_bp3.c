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
 *
 ******************************************************************************/

#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#include "nexus_security_client.h"
#include "bsagelib_boot.h"
#include "bkni.h"

#include "bp3_module_ids.h"
#include "bp3_platform_ids.h"
#include "nexus_sage_image.h"
#include "nexus_sage_types.h"


BDBG_MODULE(nexus_sage_bp3);

struct sageBP3Info {
    BSAGElib_ClientHandle sagelibClientHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
    BSAGElib_InOutContainer *sageContainer;
    uint32_t uiLastAsyncId;
    BKNI_EventHandle response;
    BKNI_EventHandle indication;
    struct
    {
        BSAGElib_RpcRemoteHandle sageRpcHandle;
        uint32_t indication_id;
        uint32_t value;
    } indicationData;
};

static struct sageBP3Info *lHandle;

#define SAGERESPONSE_TIMEOUT 10000 /* in ms */

NEXUS_SageMemoryBlock bp3_ta;       /* raw ta binary in memory */
NEXUS_SageMemoryBlock bp3_bin;      /* bp3.bin in memory */

static void NEXUS_Sage_BP3_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_BP3_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
)
{
    BSTD_UNUSED(async_argument);
    /* Save information for later use */
    lHandle->indicationData.sageRpcHandle = sageRpcHandle;
    lHandle->indicationData.indication_id = indication_id;
    lHandle->indicationData.value = value;

    BKNI_SetEvent_isr(lHandle->indication);

    return;
}

static BERR_Code NEXUS_Sage_BP3_P_WaitForSage(int timeoutMsec)
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
            BSTD_FUNCTION, SAGERESPONSE_TIMEOUT));
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

done:

    return rc;
}

void NEXUS_Sage_P_BP3Uninit(void)
{
    NEXUS_Error rc;

    if(!lHandle)
    {
        /* Nothing to do */
        return;
    }

    /* Close BP3 Platform */
    if(lHandle->hSagelibRpcModuleHandle)
    {
        BSAGElib_Rai_Module_Uninit(lHandle->hSagelibRpcModuleHandle, &lHandle->uiLastAsyncId);
        rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        if(rc!=BERR_SUCCESS)
        {
            BDBG_ERR(("timed out waiting for sage %s",BSTD_FUNCTION));
        }
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle = NULL;
    }

    if(lHandle->hSagelibRpcPlatformHandle)
    {
        BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
        rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        if(rc!=BERR_SUCCESS)
        {
            BDBG_ERR(("timed out waiting for sage %s",BSTD_FUNCTION));
        }
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    if(bp3_ta.buf != NULL){
        /* UnInstall BP3 TA bin */
        rc = BSAGElib_Rai_Platform_UnInstall(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_BP3);
        if (rc != BERR_SUCCESS){
            BDBG_WRN(("Could not UnInstall BP3 TA "));
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);

        NEXUS_Memory_Free(bp3_ta.buf);
        bp3_ta.buf = NULL;
        bp3_ta.len = 0;
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

    /* Destroy event(s) */
    if (lHandle->response)
    {
        BKNI_DestroyEvent(lHandle->response);
        lHandle->response = NULL;
    }

    if (lHandle->indication)
    {
        BKNI_DestroyEvent(lHandle->indication);
        lHandle->indication = NULL;
    }

    /* Free local info */
    BKNI_Free(lHandle);
    lHandle=NULL;
}


#define NEXUS_BP3_CCF_STATUS_BYTE_SIZE 4
#define NEXUS_BP3_CCF_MAX_NUM_BLOCKS 5
#define NEXUS_BP3_FEATURE_BLOCK_BYTE_SIZE 4
#define NEXUS_BP3_NUM_FEATURE_BLOCKS 4
#define NEXUS_BP3_BOND_OPTION_RELEASE 0

/* Some of the init needs to be delayed until SAGE is running */
NEXUS_Error NEXUS_Sage_P_BP3Init(NEXUS_SageModuleSettings *pSettings)
{
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code               rc;
    void                   *img_context = NULL;
    BIMG_Interface          img_interface;
    NEXUS_SageImageHolder   bp3TAImg  = {"BP3 TA",  SAGE_IMAGE_FirmwareID_eSage_TA_BP3,  NULL};
    NEXUS_SageImageHolder   bp3BinImg = {"BP3 BIN", SAGE_IMAGE_FirmwareID_eSage_BP3_BIN, NULL};
    uint32_t                *pCcfStatus = NULL;
    uint32_t                ccfStatusSize = NEXUS_BP3_CCF_MAX_NUM_BLOCKS*NEXUS_BP3_CCF_STATUS_BYTE_SIZE;
    uint32_t                *pFeatureList = NULL;
    uint32_t                featureListSize = NEXUS_BP3_NUM_FEATURE_BLOCKS*NEXUS_BP3_FEATURE_BLOCK_BYTE_SIZE;
    uint32_t                index = 0;
    uint32_t                prodOtpData = 0xFFFFFFFF;

    if(lHandle)
    {
        NEXUS_Sage_P_BP3Uninit();
    }

    lHandle=BKNI_Malloc(sizeof(*lHandle));
    if(!lHandle)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NEXUS_NOT_INITIALIZED;
    }

    BKNI_Memset(lHandle, 0, sizeof(*lHandle));

    rc = BKNI_CreateEvent(&lHandle->response);
    rc |= BKNI_CreateEvent(&lHandle->indication);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot Create IMG", BSTD_FUNCTION));
    }
    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        bp3TAImg.id = SAGE_IMAGE_FirmwareID_eSage_TA_BP3_Development;
    }

    bp3_ta.buf = NULL;
    bp3_ta.len = 0;
    bp3TAImg.raw = &bp3_ta;

    /* Load BP3 TA into memory */
    rc = NEXUS_SageModule_P_Load(&bp3TAImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, bp3TAImg.name));
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_BP3_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_BP3_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    if(bp3_ta.buf != NULL)
    {
        /* Install BP3 TA bin */
        rc = BSAGElib_Rai_Platform_Install(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_BP3,
                            bp3_ta.buf, bp3_ta.len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Could not install BP3 TA binary, assuming it is pre-loaded - buff[0x%p], size[%lu]",
                                        bp3_ta.buf, (unsigned long)bp3_ta.len));

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
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto EXIT;
    }

    /* Open BP3 platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_BP3,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE BP3 Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers */
            BDBG_WRN(("BP3 not running"));
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
                * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened BP3 SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE BP3 platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }
    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        goto EXIT;
    }
    else
    {
        prodOtpData = lHandle->sageContainer->basicOut[0];
        BDBG_LOG(("bond option %d",prodOtpData));
        BDBG_LOG(("Sage BP3 TA running"));
    }
    BDBG_MSG(("Initialized BP3 SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* Initialize BP3 Module */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Module_Init (
            lHandle->hSagelibRpcPlatformHandle,
            BP3_ModuleId_eBP3,
            lHandle->sageContainer,
            &lHandle->hSagelibRpcModuleHandle,  /*out */
            (void *) lHandle,
            &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE BP3 module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }
    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }
    BDBG_MSG(("Initialized SAGE BP3 Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));
    /* Read bp3.bin */
    if (pSettings->imageExists[bp3BinImg.id])
    {
        bp3_bin.buf = NULL;
        bp3_bin.len = 0;
        bp3BinImg.raw = &bp3_bin;

        /* allocate memory for status */
        pCcfStatus = (uint32_t *)BSAGElib_Rai_Memory_Allocate(
            lHandle->sagelibClientHandle,
            ccfStatusSize,
            BSAGElib_MemoryType_Global);
        if (pCcfStatus == NULL)
        {
            BDBG_ERR(("failed to allocate mem for BP3 CCF status"));
            rc = NEXUS_NOT_AVAILABLE;
            goto EXIT;
        }
        lHandle->sageContainer->blocks[1].data.ptr = (uint8_t *)pCcfStatus;
        lHandle->sageContainer->blocks[1].len      = ccfStatusSize;

        /* allocate memory for feature list */
        pFeatureList = (uint32_t *) BSAGElib_Rai_Memory_Allocate(
            lHandle->sagelibClientHandle,
            featureListSize,
            BSAGElib_MemoryType_Global);
        if (pFeatureList == NULL)
        {
            BDBG_ERR(("failed to allocate mem for BP3 Feature List"));
            rc = NEXUS_NOT_AVAILABLE;
            goto EXIT;
        }
        lHandle->sageContainer->blocks[2].data.ptr = (uint8_t *)pFeatureList;
        lHandle->sageContainer->blocks[2].len      = featureListSize;

        /* Load BP Bin file into memory */
        rc = NEXUS_SageModule_P_Load (&bp3BinImg, &img_interface, img_context);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_LOG((" %s - Cannot load bp3.bin file.",BSTD_FUNCTION));
        }
        else
        {
            /* bp3.bin file exists.  Pass the info to BP3 TA. */
            lHandle->sageContainer->blocks[0].len      = bp3BinImg.raw->len;
            lHandle->sageContainer->blocks[0].data.ptr = bp3BinImg.raw->buf;
            rc = BSAGElib_Rai_Module_ProcessCommand(
                    lHandle->hSagelibRpcModuleHandle,
                    BP3_CommandId_eProcessBP3BinFile,
                    lHandle->sageContainer,
                    &lHandle->uiLastAsyncId);
            BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
                      (void*)lHandle->hSagelibRpcModuleHandle, BP3_CommandId_eProcessBP3BinFile, lHandle->uiLastAsyncId));
            rc = NEXUS_Sage_BP3_P_WaitForSage(SAGERESPONSE_TIMEOUT);
            if (rc != BERR_SUCCESS)
            {
                rc=BERR_TRACE(rc);
            }
            BDBG_LOG(("Processed bp3.bin rc=%d", lHandle->sageContainer->basicOut[0]));

            /* Print CCF block status for number of CCF blocks in bp3.bin */
            for (index=0; index < NEXUS_BP3_CCF_MAX_NUM_BLOCKS; index++)
            {
                if (index == 0)
                {
                    BDBG_MSG(("CCF block %d or header status %d",index,pCcfStatus[index]));
                }
                else
                {
                    BDBG_MSG(("CCF block %d status %d",index,pCcfStatus[index]));
                }
            }
            if (pCcfStatus[0] == BP3_Error_eInternalDevBondOption)
            {
                BDBG_LOG(("BP3 Internal Development part.  Feature list is not applicable"));
            }
            else
            {
                /* Print Feature List for a BP3 enabled part */
                BDBG_LOG(("1 enabled Feature List:"));
                BDBG_LOG(("Audio   Feature List 0x%08X", pFeatureList[0]));
                BDBG_LOG(("Video 0 Feature List 0x%08X", pFeatureList[1]));
                BDBG_LOG(("Video 1 Feature List 0x%08X", pFeatureList[2]));
                BDBG_LOG(("Host    Feature List 0x%08X", pFeatureList[3]));
            }
        }
    }
    else
    {
#ifdef BP3_PROVISIONING
        /* bp3.bin file may not exist if a BP3 part has not been provisioned yet. */
        BDBG_LOG(("%s - Provisioning enabled.",BSTD_FUNCTION));
#else
        if ((prodOtpData & 0xFF) == 0)
        {
            /* BP3 production part */
            BDBG_ERR(("#####################################################################"));
            BDBG_ERR(("%s - bp3.bin doesn't exist and provisioning is disabled",BSTD_FUNCTION));
            BDBG_ERR(("#####################################################################"));
            BDBG_ASSERT(0);
        }
        else
        {
            BDBG_LOG(("non-bp3 part doesn't require bp3.bin for non-bp3 features"));
        }
#endif
    }


EXIT:
    BDBG_MSG(("SAGE BP3 init complete (0x%x)", rc));
    if (pCcfStatus)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, (uint8_t *)pCcfStatus);
        pCcfStatus = NULL;
    }
    if (pFeatureList)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, (uint8_t *)pFeatureList);
        pFeatureList = NULL;
    }
    if (img_context)
    {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }
    return rc;
}
