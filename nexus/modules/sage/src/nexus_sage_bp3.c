/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "priv/bsagelib_shared_globalsram.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#include "nexus_security_client.h"
#include "bsagelib_boot.h"
#include "bkni.h"
#include "strings.h"

#include "bp3_module_ids.h"
#include "bp3_platform_ids.h"
#include "nexus_sage_image.h"
#include "nexus_sage_types.h"
#include "bchp_jtag_otp.h"
#include "bchp_sun_top_ctrl.h"


BDBG_MODULE(nexus_sage_bp3);

#define NUM_BP3_FEATURE_LIST 32
/* NA - used to be a feature on an older chip */
/* NP - features not provisionable (enabled by default) */
const char *bp3FeatureListVideo0[NUM_BP3_FEATURE_LIST] = {
    "NP", /* "Broadcom H264/AVC", */
    "NP", /* "Broadcom MPEG-2", */
    "NA",
    "NP", /* "Broadcom H263", */
    "NP", /* "Broadcom VC1", */
    "NP", /* "Broadcom MPEG1", */
    "NP", /* "Broadcom MPEG2DTV", */
    "NA",
    "NP", /* "Broadcom MPEG-4 Part2/Divx", */
    "NP", /* "Broadcom AVS", */
    "NP", /* "Broadcom MPEG2_DSS_PES", */
    "NP", /* "Broadcom H264/SVC", */
    "NA",
    "NP", /* "Broadcom H264/MVC", */
    "NP", /* "Broadcom VP6", */
    "NA",
    "NP", /* "Broadcom WebM/VP8", */
    "NP", /* "Broadcom RV9", */
    "NP", /* "Broadcom SPARK", */
    "Broadcom H265(HEVC)",
    "NP", /* "Broadcom VP9", */
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "NP", /* "Broadcom HD Decode" */
};
const char *bp3FeatureListVideo1[NUM_BP3_FEATURE_LIST] = {
    "NP", /* "Broadcom 10 bit", */
    "NP", /* "Broadcom 4Kp30", */
    "NP", /* "Broadcom 4Kp60", */
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
const char *bp3FeatureListHost[NUM_BP3_FEATURE_LIST] = {
    "Rovi Macrovision",
    "Dolby Vision HDR",
    "Technicolor HDR",
    "Technicolor ITM",
    "Broadcom QAM",
    "Broadcom EchoStar-FE DiSeqC Turbo code",
    "Broadcom DIRECTV-FE FTM",
    "Broadcom S2X",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
const char *bp3FeatureListSage[NUM_BP3_FEATURE_LIST] = {
    "NP", /* "Broadcom Adv. Countermeasure", */
    "Broadcom CA Multi2",
    "Broadcom CA DVB-CSA3",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
const char *bp3FeatureListAudio[NUM_BP3_FEATURE_LIST] = {
    "Dolby Post Proc: DAP",
    "Dolby Decode Digital",
    "Dolby Decode Digital Plus",
    "Dolby Decode AC4",
    "Dolby Decode TrueHD",
    "Dolby MS10/11",
    "Dolby MS12V1",
    "Dolby MS12V2",
    "DTS TruVolume",
    "DTS Digital Surround",
    "DTS HD (M6)",
    "DTS HDMA (M8)",
    "DTS Headphone:X",
    "DTS Virtual:X",
    "DTS X",
    "Reserved",
    "Dolby MS12 v1.3 Profile C",
    "Dolby MS12 v1.3 Profile D",
    "Dolby MS12 v1.3 Profile B",
    "Dolby MS12 v1.3 Profile A",
    "Dolby DAP Content Processing",
    "Dolby DAP Virtualizer",
    "Dolby DAP Device Processing",
    "Dolby MS12 v2.3 Adv Output Channels",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

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
#define NEXUS_BP3_BOND_OPTION_BP3_PRODUCTION_PART 0

typedef struct NEXUS_Sage_BP3_Feature_List{
    uint32_t video0;
    uint32_t audio0;
    uint32_t host;
    uint32_t sage;
    uint32_t reserved;
    uint32_t video1;
}NEXUS_Sage_BP3_Feature_List;

/* print one enabled feature list on a BP3 part */
static BERR_Code NEXUS_Sage_Get_BP3_Feature_List(
    NEXUS_Sage_BP3_Feature_List *pBp3FeatureList,
    bool printBp3Features
    )
{
    uint32_t    regAddr = 0;
    uint32_t    regValue = 0xFFFFFFFF;
    BREG_Handle hReg = NULL;
    uint32_t    bondOption = 0xFFFFFFFF;
    uint32_t    index = 0;
    uint32_t    productID = 0;

    if (pBp3FeatureList == NULL)
    {
        BDBG_WRN(("Invalid pointer to bp3 feature list"));
        return NEXUS_INVALID_PARAMETER;
    }
    hReg = g_pCoreHandles->reg;
    bondOption = (BREG_Read32(hReg, BCHP_JTAG_OTP_GENERAL_STATUS_8)) & 0x000000FF;
    productID = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID)) & 0xFFFF0000;

    if ((bondOption != NEXUS_BP3_BOND_OPTION_BP3_PRODUCTION_PART) &&
        (productID != 0x72680000) &&
        (productID != 0x72710000))
    {
        BDBG_WRN(("BP3 feature list is not supported on non-BP3 parts %d", bondOption));
        return NEXUS_NOT_SUPPORTED;
    }
    else
    {
        BKNI_Memset(pBp3FeatureList, 0, sizeof(NEXUS_Sage_BP3_Feature_List));
        /* Print Feature List for a BP3 enabled part */
        BDBG_MSG(("1 enabled Feature Bits:"));
        BDBG_LOG(("Names of Features that are BP3 provisioned:"));
        /* Read feature list from Global SRAM */
        regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3AudioFeatureList0);
        regValue = BREG_Read32(hReg, regAddr);

        if (bondOption == NEXUS_BP3_BOND_OPTION_BP3_PRODUCTION_PART)
        {
            BDBG_MSG(("Audio   Feature List 0x%08X",~regValue));
            for (index=0; index < NUM_BP3_FEATURE_LIST; index++)
            {
                if (~regValue & 1)
                {
                    if (NEXUS_StrCmp(bp3FeatureListAudio[index],"Reserved") == 0)
                    {
                        BDBG_MSG(("check bp3 audio features list %d",index));
                    }
                    else if (NEXUS_StrCmp(bp3FeatureListAudio[index],"NP") != 0)
                    {
                        if (printBp3Features)
                        {
                            BDBG_LOG(("%s",bp3FeatureListAudio[index]));
                        }
                    }
                }
                regValue = regValue >> 1;
            }
            regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3VideoFeatureList0);
            regValue = BREG_Read32(hReg, regAddr);
            BDBG_MSG(("Video 0 Feature List 0x%08X",~regValue));

            for (index=0; index < NUM_BP3_FEATURE_LIST; index++)
            {
                if (~regValue & 1)
                {
                    if ((NEXUS_StrCmp(bp3FeatureListVideo0[index],"Reserved") == 0) ||
                        (NEXUS_StrCmp(bp3FeatureListVideo0[index],"NA") == 0))
                    {
                        BDBG_MSG(("check bp3 video0 features list %d",index));
                    }
                    else if (NEXUS_StrCmp(bp3FeatureListVideo0[index],"NP") != 0)
                    {
                        if (printBp3Features)
                        {
                            BDBG_LOG(("%s",bp3FeatureListVideo0[index]));
                        }
                    }
                }
                regValue = regValue >> 1;
            }
            regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3VideoFeatureList1);
            regValue = BREG_Read32(hReg, regAddr);
            BDBG_MSG(("Video 1 Feature List 0x%08X",~regValue));
            for (index=0; index < NUM_BP3_FEATURE_LIST; index++)
            {
                if (~regValue & 1)
                {
                    if (NEXUS_StrCmp(bp3FeatureListVideo1[index],"Reserved") == 0)
                    {
                        BDBG_MSG(("check bp3 video1 features list %d",index));
                    }
                    else if (NEXUS_StrCmp(bp3FeatureListVideo1[index],"NP") != 0)
                    {
                        if (printBp3Features)
                        {
                            BDBG_LOG(("%s",bp3FeatureListVideo1[index]));
                        }
                    }
                }
                regValue = regValue >> 1;
            }
            regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3HostFeatureList);
            regValue = BREG_Read32(hReg, regAddr);
            BDBG_MSG(("Host    Feature List 0x%08X",~regValue));
            for (index=0; index < NUM_BP3_FEATURE_LIST; index++)
            {
                if (~regValue & 1)
                {
                    if (NEXUS_StrCmp(bp3FeatureListHost[index],"Reserved") == 0)
                    {
                        BDBG_MSG(("check bp3 host features list %d",index));
                    }
                    else if (NEXUS_StrCmp(bp3FeatureListHost[index],"NP") != 0)
                    {
                        if (printBp3Features)
                        {
                            BDBG_LOG(("%s",bp3FeatureListHost[index]));
                        }
                    }
                }
                regValue = regValue >> 1;
            }
            regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3SAGEFeatureList);
            regValue = BREG_Read32(hReg, regAddr);
            BDBG_MSG(("Sage    Feature List 0x%08X",~regValue));
            for (index=0; index < NUM_BP3_FEATURE_LIST; index++)
            {
                if (~regValue & 1)
                {
                    if (NEXUS_StrCmp(bp3FeatureListSage[index],"Reserved") == 0)
                    {
                        BDBG_MSG(("check bp3 sage features list %d",index));
                    }
                    else if (NEXUS_StrCmp(bp3FeatureListSage[index],"NP") != 0)
                    {
                        if (printBp3Features)
                        {
                            BDBG_LOG(("%s",bp3FeatureListSage[index]));
                        }
                    }
                }
                regValue = regValue >> 1;
            }
        }
        else if ((productID == 0x72680000) ||(productID != 0x72710000))
        {
            /* legacy 7268b0 and 7271b0 parts with bond option such as 03 are allowed to provision DV HDR, TCH HDR, and TCH ITM */
            /* Other features are not managed by BP3/SAGE. */
            regAddr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3HostFeatureList);
            regValue = BREG_Read32(hReg, regAddr);
            regValue = regValue >> 1;
            for (index=1; index < 4; index++)
            {
                if (~regValue & 1)
                {
                    if (printBp3Features)
                    {
                        BDBG_LOG(("%s",bp3FeatureListHost[index]));
                    }
                }
                regValue = regValue >> 1;
            }
        }

    }
    return NEXUS_SUCCESS;
}


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
    uint32_t                *pFeatureList = NULL; /* Video0, video1, audio, host */
    uint32_t                featureListSize = NEXUS_BP3_NUM_FEATURE_BLOCKS*NEXUS_BP3_FEATURE_BLOCK_BYTE_SIZE;
    uint32_t                index = 0;
    BREG_Handle             hReg = NULL;
    uint32_t                bondOption = 0xFFFFFFFF;
    uint32_t                productID = 0;
    NEXUS_Sage_BP3_Feature_List bp3FeatureList; /* all BP3 global SRAM registers including sage and reserved */

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
        BDBG_LOG(("Sage BP3 TA running"));
    }
    BDBG_MSG(("Initialized BP3 SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    hReg = g_pCoreHandles->reg;
    bondOption = (BREG_Read32(hReg, BCHP_JTAG_OTP_GENERAL_STATUS_8)) & 0x000000FF;
    BDBG_LOG(("bond option %d",bondOption));

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

    /* Read the chip's Product_ID */
    productID = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID)) & 0xFFFF0000;

    /* Read bp3.bin for BP3 part or 7268/7271 with 01 or 03 bond option if the file exists */
    if ((pSettings->imageExists[bp3BinImg.id]) &&
        ((bondOption == NEXUS_BP3_BOND_OPTION_BP3_PRODUCTION_PART) ||
         (((productID == 0x72680000) || (productID == 0x72710000)) &&
          ((bondOption == 0x01) || (bondOption == 0x03)))
        ))
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
                /*
                            BDBG_LOG(("Audio   Feature List 0x%08X", pFeatureList[0]));
                            BDBG_LOG(("Video 0 Feature List 0x%08X", pFeatureList[1]));
                            BDBG_LOG(("Video 1 Feature List 0x%08X", pFeatureList[2]));
                            BDBG_LOG(("Host    Feature List 0x%08X", pFeatureList[3]));
                            */
                BKNI_Memset(&bp3FeatureList, 0, sizeof(NEXUS_Sage_BP3_Feature_List));
                rc = NEXUS_Sage_Get_BP3_Feature_List(&bp3FeatureList, true);
                if (rc != BERR_SUCCESS)
                {
                    /* continue even if failed to read the status */
                    rc = BERR_SUCCESS;
                }
            }
        }
    }
    else
    {
#ifdef BP3_PROVISIONING
        /* bp3.bin file may not exist if a BP3 part has not been provisioned yet. */
        BDBG_LOG(("%s - BP3 Provisioning compile flag enabled.",BSTD_FUNCTION));
#else
        if (NEXUS_GetEnv("BP3_PROVISIONING"))
        {
            /* bp3.bin file may not exist if a BP3 part has not been provisioned yet. */
            BDBG_LOG(("%s - BP3 Provisioning run-time flag enabled.",BSTD_FUNCTION));
        }
        else
        {
            if (bondOption == NEXUS_BP3_BOND_OPTION_BP3_PRODUCTION_PART)
            {
                /* BP3 production part */
                BDBG_ERR(("#####################################################################"));
                BDBG_ERR(("%s - bp3.bin doesn't exist. ",BSTD_FUNCTION));
                BDBG_ERR(("%s - provisioned features are disabled",BSTD_FUNCTION));
                BDBG_ERR(("#####################################################################"));
            }
            else
            {
                BDBG_LOG(("non-bp3 part doesn't require bp3.bin for non-bp3 features"));
            }
        }
#endif
    }
    rc = BERR_SUCCESS;

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
