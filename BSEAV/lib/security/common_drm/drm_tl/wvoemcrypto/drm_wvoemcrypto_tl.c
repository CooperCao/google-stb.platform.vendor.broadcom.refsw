/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include <string.h>
#include <errno.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_platform_client.h"
#include "nexus_random_number.h"
#include "nexus_otpmsp.h"

#include "drm_common_tl.h"
#include "drm_wvoemcrypto_tl.h"
#include "drm_common.h"
#include "drm_common_swcrypto.h"
#include "drm_data.h"
#include "drm_common_command_ids.h"

#include "bsagelib_types.h"
#include "sage_srai.h"

#define MAX_USAGE_TABLE_SIZE         (6544)
#define OVERWRITE_USAGE_TABLE_ON_ROOTFS  (1)

#define INVALID_KEYSLOT_ID   (-1)

static uint32_t gNumSessions;
static Drm_WVOemCryptoHostSessionCtx_t *gHostSessionCtx;
static Drm_WVoemCryptoKeySlot_t gKeySlotCache[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
static uint32_t gKeySlotCacheAllocated = 0;
static uint32_t gKeySlotCacheMode = DRM_WVOEMCRYPTO_SESSION_KEY_CACHE;

/* #define DEBUG 1 */
void dump(const unsigned char* data, unsigned length, const char* prompt)
{
#ifndef DEBUG
    BSTD_UNUSED(data);
    BSTD_UNUSED(length);
    BSTD_UNUSED(prompt);
#endif
#ifdef DEBUG
    unsigned int i;

    if (prompt)
    {
        fprintf(stderr, "%s:", prompt);
    }
    for (i = 0; i < length; ++i)
    {
        if (i % 16 == 0)
        {
            fprintf(stderr, "\n\t");
        }
        fprintf(stderr, "%02x ", data[i]);
    }
    fprintf(stderr, "\n");
#endif
}

#define DRM_WVOEMCRYPTO_SHA256_DIGEST_LENGTH    32

/* SRAI module handle */
static SRAI_ModuleHandle gmoduleHandle = NULL;

Drm_WVOemCryptoParamSettings_t gWvOemCryptoParamSettings;

/* 16 byte padding countre mode with non-zero byteOffset */
static uint8_t *gPadding = NULL;
#define PADDING_SZ_16_BYTE  16

/* API to read usage table */
#ifdef ANDROID
#define USAGE_TABLE_FILE_PATH        "/data/mediadrm/UsageTable.dat"
#define USAGE_TABLE_BACKUP_FILE_PATH "/data/mediadrm/UsageTable.bkp"
#else
#define USAGE_TABLE_FILE_PATH        "UsageTable.dat"
#define USAGE_TABLE_BACKUP_FILE_PATH "UsageTable.bkp"
#endif

/* BTP buffer size for external iv support */
#define BTP_SIZE 188

/* Scatter/gather definitions */
static DmaBlockInfo_t **gWvEncDmaBlockInfoList;
static NEXUS_DmaJobBlockSettings *gWvClrDmaJobBlockSettingsList;

static uint32_t gWvClrNumDmaBlocks;
static BKNI_EventHandle gWVClrEvent;
static NEXUS_DmaHandle gWVClrDma;
static NEXUS_DmaJobHandle gWVClrJob;
static BKNI_MutexHandle gWVClrMutex;
static bool gWVClrQueued;

static uint8_t *gWVUsageTable;
static uint8_t gWVUsageTableDigest[SHA256_DIGEST_SIZE] = {0x00};


#define MAX_SG_DMA_BLOCKS DRM_COMMON_TL_MAX_DMA_BLOCKS
#define WV_OEMCRYPTO_FIRST_SUBSAMPLE 1
#define WV_OEMCRYPTO_LAST_SUBSAMPLE 2

static DrmRC DRM_WvOemCrypto_P_ReadUsageTable(uint8_t *pUsageTableSharedMemory,
                                              uint32_t *pUsageTableSharedMemorySize);
static DrmRC DRM_WvOemCrypto_P_OverwriteUsageTable(uint8_t *pEncryptedUsageTable,
                                                   uint32_t encryptedUsageTableLength);


BDBG_MODULE(drm_wvoemcrypto_tl);

/******************************************************************************
** FUNCTION:
**  DRM_WVOemCrypto_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pOemCryptoParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_WVOEMCrypto_GetDefaultParamSettings(Drm_WVOemCryptoParamSettings_t *pOemCryptoParamSettings)
{
    BDBG_ENTER(DRM_WVOEMCrypto_GetDefaultParamSettings);
    ChipType_e chip_type;

    BKNI_Memset((uint8_t*)pOemCryptoParamSettings, 0x00, sizeof(Drm_WVOemCryptoParamSettings_t));
#ifdef USE_UNIFIED_COMMON_DRM
    pOemCryptoParamSettings->drmCommonInit.drmType = 0;
#else
    pOemCryptoParamSettings->drmCommonInit.drmType = BSAGElib_BinFileDrmType_eWidevine;
#endif
    DRM_Common_GetDefaultStructSettings(&pOemCryptoParamSettings->drmCommonOpStruct);
    pOemCryptoParamSettings->drmCommonInit.drmCommonInit.heap = NULL;

    pOemCryptoParamSettings->drm_bin_file_path = bdrm_get_drm_bin_file_path();
    chip_type = DRM_Common_GetChipType();
#if USE_UNIFIED_COMMON_DRM
    if(chip_type == ChipType_eZS)
    {
        pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_dev_bin_file_path();
    }
    else
    {
        pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_bin_file_path();
    }
#else
    if(chip_type == ChipType_eZS)
    {
        pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_wv_dev_bin_file_path();
    }
    else
    {
        pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_wv_bin_file_path();
    }
#endif

    BDBG_MSG(("%s DRM %s TA %s ",__FUNCTION__,pOemCryptoParamSettings->drm_bin_file_path, pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path));

    BDBG_LEAVE(DRM_WVOEMCrypto_GetDefaultParamSettings);
    return;
}


/******************************************************************************
** FUNCTION:
** DRM_WVOEMCrypto_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** wvOemcryptoParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_WVOemCrypto_SetParamSettings(Drm_WVOemCryptoParamSettings_t *pWvOemCryptoParamSettings)
{
    BDBG_ENTER(DRM_WVOemCrypto_SetParamSettings);

    gWvOemCryptoParamSettings.drmCommonInit.drmType = pWvOemCryptoParamSettings->drmCommonInit.drmType;
    gWvOemCryptoParamSettings.drmCommonInit.drmCommonInit.heap = pWvOemCryptoParamSettings->drmCommonInit.drmCommonInit.heap;
    gWvOemCryptoParamSettings.drmCommonInit.ta_bin_file_path= pWvOemCryptoParamSettings->drmCommonInit.ta_bin_file_path;
    gWvOemCryptoParamSettings.drm_bin_file_path = pWvOemCryptoParamSettings->drm_bin_file_path;
    BKNI_Memcpy(&gWvOemCryptoParamSettings.drmCommonOpStruct,&pWvOemCryptoParamSettings->drmCommonOpStruct,sizeof(DrmCommonOperationStruct_t));

    BDBG_LEAVE(DRM_WVOemCrypto_SetParamSettings);
    return;
}

/**************************************************************************************************

Desc
    Closes the crypto operation and releases all related resources.
Parameters
    None
Returns
    OEMCrypto_SUCCESS success
    OEMCrypto_ERROR_TERMINATE_FAILED failed to deinitialize crypto hardware
Threading
No other OEMCrypto calls are made while this function is running. After this function is called,
no other OEMCrypto calls will be made until another call to OEMCrypto_Initialize() is made.
****************************************************************************************************/
DrmRC DRM_WVOemCrypto_UnInit(int *wvRc)
{
    DrmRC rc = Drm_Success;
    *wvRc = SAGE_OEMCrypto_SUCCESS;
    unsigned int i;

    BDBG_ENTER(DRM_WVOemCrypto_UnInit);

    if (gPadding != NULL)
    {
        NEXUS_Memory_Free(gPadding);
        gPadding = NULL;
    }

    /* This will invoke drm_wvoemcrypto_finalize on sage side*/
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleFinalize(gmoduleHandle);
#else
    rc = DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Widevine, gmoduleHandle);
#endif
    if(rc == Drm_Success)
    {
        *wvRc = SAGE_OEMCrypto_SUCCESS;
    }
    else
    {
        *wvRc = SAGE_OEMCrypto_ERROR_TERMINATE_FAILED;
        rc = Drm_Err;
    }
#ifdef USE_UNIFIED_COMMON_DRM
    DRM_Common_TL_Finalize();
#else
    DRM_Common_TL_Finalize_TA(Common_Platform_Widevine);
#endif

    if(gHostSessionCtx != NULL)
    {
        BKNI_Free(gHostSessionCtx);
        gHostSessionCtx = NULL;
    }

    if(gWvEncDmaBlockInfoList != NULL)
    {
        BKNI_Free(gWvEncDmaBlockInfoList);
        gWvEncDmaBlockInfoList = NULL;
    }

    if(gWVClrDma != NULL)
    {
        if(gWVClrJob != NULL)
        {
            NEXUS_DmaJob_Destroy(gWVClrJob);
            gWVClrJob = NULL;
        }
        NEXUS_Dma_Close(gWVClrDma);
        gWVClrDma = NULL;
    }

    if (gWvClrDmaJobBlockSettingsList != NULL)
    {
        BKNI_Free(gWvClrDmaJobBlockSettingsList);
        gWvClrDmaJobBlockSettingsList = NULL;
    }

    if (gWVClrMutex != NULL)
    {
        BKNI_DestroyMutex(gWVClrMutex);
        gWVClrMutex = NULL;
    }

    if(gWVUsageTable != NULL)
    {
        SRAI_Memory_Free(gWVUsageTable);
        gWVUsageTable = NULL;
    }

    /*free the keyslots*/
    for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
    {
        if(gKeySlotCache[i].hSwKeySlot != NULL)
        {
            BDBG_MSG(("%s:Freeing Keyslot...",__FUNCTION__));
            NEXUS_Security_FreeKeySlot(gKeySlotCache[i].hSwKeySlot);
            gKeySlotCache[i].hSwKeySlot = NULL;
        }
    }
    gKeySlotCacheAllocated = 0;

    BDBG_LEAVE(DRM_WVOemCrypto_UnInit);
    return Drm_Success;
}

/***********************************************************************************************
Desc: Initializes the crypto hardware.
Parameters:None
Returns:OEMCrypto_SUCCESS success
         OEMCrypto_ERROR_INIT_FAILED failed to initialize crypto hardware
************************************************************************************************/
DrmRC DRM_WVOemCrypto_Initialize(Drm_WVOemCryptoParamSettings_t *pWvOemCryptoParamSettings,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_Initialize);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(pWvOemCryptoParamSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /*this will inturn call drm_wvoemcrypto_init on sage side*/
    rc = DRM_Common_TL_Initialize(&pWvOemCryptoParamSettings->drmCommonInit);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module, Error :%d", __FUNCTION__,rc));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error in allocating container memory", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /*
     * For the Usage Table, either we're going to read it from the rootfs
     * OR expect it to be created on the SAGE side and returned.
     * Therefore either way, allocate the max size
     * */
    /* shared block [0] is for the DRM bin file so use [1] */

    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[1].data.ptr = gWVUsageTable;
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (on return)", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[1].len = MAX_USAGE_TABLE_SIZE;
    BDBG_MSG(("%s -  %p -> length = '%u'", __FUNCTION__, container->blocks[1].data.ptr, container->blocks[1].len));

    /* read contents of UsageTable.dat and place in shared pointer 'container->blocks[1].data.ptr' */
    rc = DRM_WvOemCrypto_P_ReadUsageTable(container->blocks[1].data.ptr, &container->blocks[1].len);
    if(rc == Drm_Success)
    {
        BDBG_MSG(("%s - Usage Table detected on rootfs and read into shared memory", __FUNCTION__));
    }
    else if(rc == Drm_FileErr)
    {
        BDBG_WRN(("%s - Usage Table not detected on rootfs, assuming initial creation...", __FUNCTION__));
        container->basicIn[2] = OVERWRITE_USAGE_TABLE_ON_ROOTFS;
        BKNI_Memset(container->blocks[1].data.ptr, 0x00, container->blocks[1].len);
    }
    else
    {
        BDBG_ERR(("%s - Usage Table detected on rootfs but error occurred reading it.", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    current_time = time(NULL);
    container->basicIn[0] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", __FUNCTION__, current_time));

    /* Signal a central key cache used to allow a larger session count. */
    container->basicIn[1] = DRM_WVOEMCRYPTO_CENTRAL_KEY_CACHE;

#if DEBUG
    DRM_MSG_PRINT_BUF("Host side UT header (after reading or creating)", container->blocks[1].data.ptr, 144);
#endif

    /* Initialize SAGE widevine module */
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eWVOemcrypto, pWvOemCryptoParamSettings->drm_bin_file_path, container, &gmoduleHandle);
#else
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_Widevine, Widevine_ModuleId_eDRM, pWvOemCryptoParamSettings->drm_bin_file_path, container, &gmoduleHandle);
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* Obtain supported key slot cache mode */
    gKeySlotCacheMode = container->basicOut[1];

    if (gPadding == NULL)
    {
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_MemoryAllocationSettings memSettings;
        NEXUS_Platform_GetClientConfiguration(&clientConfig); /* nexus_platform_client.h */
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);

        NEXUS_Memory_Allocate(PADDING_SZ_16_BYTE, &memSettings, (void **)&gPadding);
        if (gPadding == NULL)
        {
            BDBG_ERR(("%s - couldn't allocate memory for padding", __FUNCTION__));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gPadding, 0x0,PADDING_SZ_16_BYTE);
    }

    BDBG_MSG(("%s - Did an encrypted Usage Table return to host? (container->basicOut[2] = '%u')", __FUNCTION__, container->basicOut[2]));
    if(container->basicOut[2] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        /* Should only come in here when Usage Table is created for first time */
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[1].data.ptr,
                                                   container->blocks[1].len);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error creating Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit;
        }
    }

    if (gWvClrDmaJobBlockSettingsList == NULL)
    {
        gWvClrDmaJobBlockSettingsList = BKNI_Malloc(MAX_SG_DMA_BLOCKS * sizeof(NEXUS_DmaJobBlockSettings));
        if(gWvClrDmaJobBlockSettingsList == NULL)
        {
            BDBG_ERR(("%s - Error allocationg memory for DMA Job block settings", __FUNCTION__ ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gWvClrDmaJobBlockSettingsList, 0x0, MAX_SG_DMA_BLOCKS * sizeof(NEXUS_DmaJobBlockSettings));
    }

    if(gWVClrMutex == NULL)
    {
        if(BKNI_CreateMutex(&gWVClrMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling creating mutex", __FUNCTION__));
            rc = Drm_Err;
            goto ErrorExit;
        }
    }

    /* Initialize the key slot cache */
    BKNI_Memset(gKeySlotCache, 0, sizeof(gKeySlotCache));

ErrorExit:

    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if(*wvRc!= SAGE_OEMCrypto_SUCCESS)
    {
        rc =  Drm_Err;
    }

    if(rc != Drm_Success)
    {
        /* Cleanup if we are in an error condition */
        if(gWVClrMutex != NULL)
        {
            BKNI_DestroyMutex(gWVClrMutex);
            gWVClrMutex = NULL;
        }

        if (gWvClrDmaJobBlockSettingsList != NULL)
        {
            BKNI_Free(gWvClrDmaJobBlockSettingsList);
            gWvClrDmaJobBlockSettingsList = NULL;
        }

        if (gPadding != NULL)
        {
            NEXUS_Memory_Free(gPadding);
            gPadding = NULL;
        }

        if(gWVUsageTable != NULL)
        {
            SRAI_Memory_Free(gWVUsageTable);
            gWVUsageTable = NULL;
        }
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Initialize);
    return rc;
}


/*********************************************************************************************
Description:
Open a new crypto security engine context. The security engine hardware and firmware shall
acquire resources that are needed to support the session, and return a session handle that
identifies that session in future calls.

Parameters:
[out] session: an opaque handle that the crypto firmware uses to identify the session.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_TOO_MANY_SESSIONS failed because too many sessions are open
OEMCrypto_ERROR_OPEN_SESSION_FAILED there is a resource issue or the security
engine is not properly initialized.

Threading:
No other Open/Close session calls will be made while this function is running. Functions on
existing sessions may be called while this function is active.
**********************************************************************************************/

DrmRC DRM_WVOemCrypto_OpenSession(uint32_t* session,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_OpenSession);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eOpenSession, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Operation failed. Setting session context to NULL, wvRC = %d",
                                                                __FUNCTION__, *wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }
    if(*wvRc != 0)
    {
        BDBG_ERR(("%s:Sage openssession command failed with error %d",__FUNCTION__,*wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }
    /* if success, extract index from container */
    (*session) = container->basicOut[1];

    BDBG_MSG(("%s: opened session with id = %d",__FUNCTION__,container->basicOut[1] ));

    if(gNumSessions == 0)
    {
        rc = DRM_WVOemCrypto_GetMaxNumberOfSessions(&gNumSessions, wvRc);
        if(rc != Drm_Success || gNumSessions == 0)
        {
            BDBG_ERR(("%s - Error obtaining maximum number of sessions (%d)", __FUNCTION__, gNumSessions));
            goto ErrorExit;
        }
    }

    if(gHostSessionCtx == NULL)
    {
        gHostSessionCtx = BKNI_Malloc(gNumSessions * sizeof(Drm_WVOemCryptoHostSessionCtx_t));
        if(gHostSessionCtx == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for session context", __FUNCTION__ ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gHostSessionCtx, 0x0, gNumSessions * sizeof(Drm_WVOemCryptoHostSessionCtx_t));
    }

    if(gWvEncDmaBlockInfoList == NULL)
    {
        gWvEncDmaBlockInfoList = BKNI_Malloc(gNumSessions * sizeof(DmaBlockInfo_t *));
        if(gWvEncDmaBlockInfoList == NULL)
        {
            BDBG_ERR(("%s - Error allocationg memory for DMA block list", __FUNCTION__ ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gWvEncDmaBlockInfoList, 0x0, gNumSessions * sizeof(DmaBlockInfo_t *));
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_OpenSession);

    return rc;
}


/**********************************************************************************************************
Description:
Closes the crypto security engine session and frees any associated resources.

Parameters:
[in] session: handle for the session to be closed.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_INVALID_SESSION no open session with that id.
OEMCrypto_ERROR_CLOSE_SESSION_FAILED illegal/unrecognized handle or the security
engine is not properly initialized.

Threading:
No other Open/Close session calls will be made while this function is running. Functions on
existing sessions may be called while this function is active.
**********************************************************************************************************/
DrmRC drm_WVOemCrypto_CloseSession(uint32_t session,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    *wvRc=SAGE_OEMCrypto_SUCCESS;
    unsigned int i;

    BDBG_ENTER(DRM_WVOemCrypto_CloseSession);
    BDBG_MSG(("%s closesession with id=%d",__FUNCTION__, session));
    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_CLOSE_SESSION_FAILED ;
        goto ErrorExit;
    }
    container->basicIn[0]=session;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eCloseSession, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_CLOSE_SESSION_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    *wvRc= container->basicOut[2];

    if(container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Operation failed. Setting session context to NULL sage rc = (0x%08x), wvRc = %d",
                                          __FUNCTION__, container->basicOut[0], container->basicOut[2]));
    }

    if (gWvEncDmaBlockInfoList[session] != NULL) {
        SRAI_Memory_Free((uint8_t*)gWvEncDmaBlockInfoList[session]);
        gWvEncDmaBlockInfoList[session] = NULL;
    }

    if (gHostSessionCtx[session].btp_info.btp_sage_buffer)
    {
        BDBG_MSG(("%s  Freeing btp buffer 0x%08x", __FUNCTION__, gHostSessionCtx[session].btp_info.btp_sage_buffer ));
        SRAI_Memory_Free( gHostSessionCtx[session].btp_info.btp_sage_buffer );
        gHostSessionCtx[session].btp_info.btp_sage_buffer = NULL;
    }

    gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;

    if(gKeySlotCacheMode == DRM_WVOEMCRYPTO_SESSION_KEY_CACHE)
    {
        /*free the session keyslot(s)*/
        for(i = 0; i < gHostSessionCtx[session].num_key_slots; i++)
        {
            if(gHostSessionCtx[session].key_slot_ptr[i] &&
                gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot != NULL)
            {
                BDBG_MSG(("%s:Freeing Keyslot...",__FUNCTION__));
                NEXUS_Security_FreeKeySlot(gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot);
                gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot = NULL;
                gKeySlotCacheAllocated--;
            }
            gHostSessionCtx[session].key_slot_ptr[i] = NULL;
        }
        gHostSessionCtx[session].num_key_slots = 0;
    }


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    if (*wvRc!= SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_CloseSession);

    return rc;

}

/***************************************************************************************************
Description:
Generates a 32bit nonce to detect possible replay attack on the key control block. The nonce is
stored in secure memory and will be used for the next call to LoadKeys.

Parameters:
[in] session: handle for the session to be used.

Results:
nonce: the nonce is also stored in secure memory. At least 4 nonces should be stored for each
session.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
****************************************************************************************************/
DrmRC drm_WVOemCrypto_GenerateNonce(uint32_t session,
                                    uint32_t* nonce,
                                    int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_GenerateNonce);

    *wvRc=SAGE_OEMCrypto_SUCCESS;

    BDBG_MSG(("%s:Session ID=%d",__FUNCTION__, session));

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateNonce, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {

        BDBG_ERR(("%s - Operation failed. Setting session context to NULL with wvRC = %d ",
                                             __FUNCTION__, *wvRc));
         rc =  Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*nonce) = container->basicOut[1];
    BDBG_MSG(("%s:nonce=0x%x",__FUNCTION__,*nonce));

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    if (*wvRc!= SAGE_OEMCrypto_SUCCESS){
        rc =  Drm_Err;
    }
    BDBG_LEAVE(DRM_WVOemCrypto_GenerateNonce);

    return rc;
}

DrmRC drm_WVOemCrypto_GenerateDerivedKeys(uint32_t session,
                                          const uint8_t* mac_key_context,
                                          uint32_t mac_key_context_length,
                                          const uint8_t* enc_key_context,
                                          uint32_t enc_key_context_length,
                                          int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_GenerateDerivedKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(mac_key_context == NULL)
    {
        BDBG_ERR(("%s - mac_key_context buffer is NULL", __FUNCTION__));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(enc_key_context == NULL)
    {
        BDBG_ERR(("%s - enc_key_context buffer is NULL", __FUNCTION__));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if((enc_key_context_length==0)||(mac_key_context_length==0))
    {
        BDBG_ERR(("%s - encrypted key length  (%u) or mac key context length (%u) is invalid", __FUNCTION__, enc_key_context_length ,mac_key_context_length));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(mac_key_context_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(mac_key_context_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error in allocating memory for MAC key context (%u bytes)", __FUNCTION__, mac_key_context_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
            goto ErrorExit;
        }
        container->blocks[0].len = mac_key_context_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, mac_key_context, mac_key_context_length);
    }

    if(enc_key_context_length != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(enc_key_context_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error in allocating memory for encrypted key context (%u bytes)", __FUNCTION__, enc_key_context_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,enc_key_context, enc_key_context_length);
        container->blocks[1].len = enc_key_context_length ;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateDerivedKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
     *wvRc = container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command to GenerateDerivedKeys failed (0x%08x/ wcRc = %d)",
                                    __FUNCTION__, sage_rc, container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];
ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    if (*wvRc!= SAGE_OEMCrypto_SUCCESS){
        rc =  Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemMCrypto_GenerateDerivedKeys);

    return rc;
}


/*************************************************************************************************************
Description:
Generates a HMACSHA256 signature using the mac_key[client] for license request signing
under the license server protocol for AES CTR/CBC mode.

Parameters:
[in] session: crypto session identifier.
[in] message: pointer to memory containing message to be signed.
[in] message_length: length of the message, in bytes.
[out] signature: pointer to memory to received the computed signature. May be null on the first
call in order to find required buffer size.
[in/out] signature_length: (in) length of the signature buffer, in bytes.
(out) actual length of the signature, in bytes.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_SHORT_BUFFER if signature buffer is not large enough to hold buffer.
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
***************************************************************************************************************/
DrmRC drm_WVOemCrypto_GenerateSignature(uint32_t session,
                                        const uint8_t* message,
                                        size_t message_length,
                                        uint8_t* signature,
                                        size_t* signature_length,
                                        int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_GenerateSignature);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL)
    {
        BDBG_ERR(("%s - message buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if (*signature_length < DRM_WVOEMCRYPTO_SHA256_DIGEST_LENGTH)
    {
        *signature_length = SHA256_DIGEST_LENGTH;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(message_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", __FUNCTION__, message_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
            goto ErrorExit;
        }
        container->blocks[0].len = message_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);
    }

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(*signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature (%u bytes)", __FUNCTION__, (*signature_length)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memcpy(container->blocks[1].data.ptr, signature, (*signature_length));
    container->blocks[1].len = *signature_length ;


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateSignature, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending SAGE command", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE ;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];

    *wvRc = container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to GenerateSignature but actual operation ailed (0x%08x), wvRC = %d ",
                                                   __FUNCTION__, sage_rc, *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *signature_length = container->basicOut[1];

    /*the signature is updated in the data->ptr of block 1.copy back the generated signature*/
    BKNI_Memcpy(signature, container->blocks[1].data.ptr, *signature_length);

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
    }

    if (*wvRc != SAGE_OEMCrypto_SUCCESS){
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_GenerateSignature);

    return rc;
}

uint32_t getSizeOfKeyObjectArray(const Drm_WVOemCryptoKeyObject* key_array,uint32_t num_of_keys)
{
    uint32_t sz = 0;
    uint32_t i = 0;

    for(i = 0; i < num_of_keys; i++)
    {
        sz+=key_array[i].key_id_length;
        sz+= WVCDM_KEY_IV_SIZE;
        sz+=key_array[i].key_data_length;
        sz+=WVCDM_KEY_IV_SIZE; /*control iv size*/
        sz+=WVCDM_KEY_CONTROL_SIZE; /*key control  size*/
    }
    return sz;
}

uint32_t getSizeOfKeyObjectArray_V10(const Drm_WVOemCryptoKeyObject_V10* key_array,uint32_t num_of_keys)
{
    uint32_t sz = 0;
    uint32_t i = 0;

    for(i = 0; i < num_of_keys; i++)
    {
        sz+=key_array[i].key_id_length;
        sz+= WVCDM_KEY_IV_SIZE;
        sz+=key_array[i].key_data_length;
        sz+=WVCDM_KEY_IV_SIZE; /*control iv size*/
        sz+=WVCDM_KEY_CONTROL_SIZE; /*key control  size*/
    }
    return sz;
}

/******************************************************************************************************************
Description:
Installs a set of keys for performing decryption in the current session.

Parameters:
[in] session: crypto session identifier.
[in] message: pointer to memory containing message to be verified.
[in] message_length: length of the message, in bytes.
[in] signature: pointer to memory containing the signature.
[in] signature_length: length of the signature, in bytes.
[in] enc_mac_key_iv: IV for decrypting new mac_key. Size is 128 bits.
[in] enc_mac_keys: encrypted mac_keys for generating new mac_keys. Size is 512 bits.
[in] num_keys: number of keys present.
[in] key_array: set of keys to be installed.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_UNKNOWN_FAILURE
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_SIGNATURE_FAILURE
OEMCrypto_ERROR_INVALID_NONCE
OEMCrypto_ERROR_TOO_MANY_KEYS

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.

*****************************************************************************************************************/
DrmRC drm_WVOemCrypto_LoadKeys(uint32_t session,
                               const uint8_t* message,
                               uint32_t       message_length,
                               const uint8_t* signature,
                               uint32_t       signature_length,
                               const uint8_t* enc_mac_key_iv,
                               const uint8_t* enc_mac_keys,
                               uint32_t       num_keys,
                               void*          key_array,
                               const uint8_t* pst,
                               uint32_t       pst_length,
                               int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    uint32_t key_object_shared_block_length = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoKeyObject* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject* keyArray = (Drm_WVOemCryptoKeyObject*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", __FUNCTION__, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", __FUNCTION__, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", __FUNCTION__, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memcpy(container->blocks[1].data.ptr,signature, signature_length);
    container->blocks[1].len = signature_length ;


    if(enc_mac_key_iv != NULL)
    {
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", __FUNCTION__));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        container->blocks[2].len = WVCDM_KEY_IV_SIZE;
        BKNI_Memcpy(container->blocks[2].data.ptr, enc_mac_key_iv, WVCDM_KEY_IV_SIZE);
    }


    if(enc_mac_keys!=NULL)
    {
        container->blocks[3].data.ptr = SRAI_Memory_Allocate(2*WVCDM_MAC_KEY_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[3].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", __FUNCTION__));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[3].data.ptr, enc_mac_keys, 2*WVCDM_MAC_KEY_SIZE);
        container->blocks[3].len = 2*WVCDM_MAC_KEY_SIZE ;
    }

    /* allocate for sending key object data*/
    key_array_sz = getSizeOfKeyObjectArray(keyArray, num_keys);
    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", __FUNCTION__, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoKeyObject))*num_keys), SRAI_MemoryType_Shared);
    if(container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", __FUNCTION__, ((sizeof(Drm_WVOemCryptoKeyObject))*num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoKeyObject))*num_keys);
    for(i = 0, key_object_shared_block_length = 0; i < num_keys; i++)
    {
        key_object_shared_block_length = i * sizeof(Drm_WVOemCryptoKeyObject);
        BKNI_Memcpy(container->blocks[5].data.ptr + key_object_shared_block_length,
            keyArray + i, sizeof(Drm_WVOemCryptoKeyObject));
    }

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", __FUNCTION__, pst_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[6].data.ptr, pst, pst_length);
        container->blocks[6].len = pst_length ;
    }

    /*
     * Fill in Key Array
     * */
    for(i=0,key_object_shared_block_length=0; i < num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",__FUNCTION__, i,key_object_shared_block_length,keyArray[i].key_id_length));
        /* copy key_id  */
        BKNI_Memcpy(container->blocks[4].data.ptr+key_object_shared_block_length,
                    keyArray[i].key_id,
                    keyArray[i].key_id_length);

        key_object_shared_block_length += keyArray[i].key_id_length;
        BDBG_MSG(("copy the key data iv at offset %d",key_object_shared_block_length));

        /* copy key_data_iv */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray[i].key_data_iv,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;

        BDBG_MSG(("copy the key data  at offset %d",key_object_shared_block_length));

        /* copy key_data */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray[i].key_data,
                    (keyArray[i].key_data_length));

        key_object_shared_block_length += (keyArray[i].key_data_length);

        BDBG_MSG(("copy the key control iv   at offset %d",key_object_shared_block_length));

        /* copy key_control_iv */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray[i].key_control_iv,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;

        BDBG_MSG(("copy the key control   at offset %d",key_object_shared_block_length));

        /* copy key_control */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray[i].key_control,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;
    }

    /* update shared block length */
    if(key_array_sz != key_object_shared_block_length)
    {
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", __FUNCTION__, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;



    pKeyObj = (Drm_WVOemCryptoKeyObject*)container->blocks[5].data.ptr;

    for(i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id =&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray[i].key_id_length;

        pKeyObj[i].key_data_iv =&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray[i].key_id_length];

        pKeyObj[i].key_data = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray[i].key_data_length;

        pKeyObj[i].key_control_iv = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray[i].key_data_length];

        pKeyObj[i].key_control = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray[i].key_data_length];
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", __FUNCTION__, DrmWVOEMCrypto_CommandId_eLoadKeys));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =    container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS){
        BDBG_ERR(("%s - widevine return code (0x%08x)", __FUNCTION__, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", __FUNCTION__, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        if(container->blocks[3].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[3].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }

        if(container->blocks[4].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[4].data.ptr);
            container->blocks[4].data.ptr = NULL;
        }

        if(container->blocks[5].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[5].data.ptr);
            container->blocks[5].data.ptr = NULL;
        }

        if(container->blocks[6].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[6].data.ptr);
            container->blocks[6].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_LoadKeys);
    return rc;
}

DrmRC drm_WVOemCrypto_LoadKeys_V9_or_V10(uint32_t session,
                               const uint8_t* message,
                               uint32_t       message_length,
                               const uint8_t* signature,
                               uint32_t       signature_length,
                               const uint8_t* enc_mac_key_iv,
                               const uint8_t* enc_mac_keys,
                               uint32_t       num_keys,
                               void*          key_array,
                               const uint8_t* pst,
                               uint32_t       pst_length,
                               int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    uint32_t key_object_shared_block_length = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoKeyObject_V10* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject_V10* keyArray_V10 = (Drm_WVOemCryptoKeyObject_V10*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys_V9_or_V10);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", __FUNCTION__, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", __FUNCTION__, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", __FUNCTION__, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memcpy(container->blocks[1].data.ptr,signature, signature_length);
    container->blocks[1].len = signature_length ;


    if(enc_mac_key_iv != NULL)
    {
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", __FUNCTION__));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        container->blocks[2].len = WVCDM_KEY_IV_SIZE;
        BKNI_Memcpy(container->blocks[2].data.ptr, enc_mac_key_iv, WVCDM_KEY_IV_SIZE);
    }


    if(enc_mac_keys!=NULL)
    {
        container->blocks[3].data.ptr = SRAI_Memory_Allocate(2*WVCDM_MAC_KEY_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[3].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", __FUNCTION__));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[3].data.ptr, enc_mac_keys, 2*WVCDM_MAC_KEY_SIZE);
        container->blocks[3].len = 2*WVCDM_MAC_KEY_SIZE ;
    }

    /* allocate for sending key object data*/
    key_array_sz = getSizeOfKeyObjectArray_V10(keyArray_V10, num_keys);
    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", __FUNCTION__, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoKeyObject_V10))*num_keys), SRAI_MemoryType_Shared);
    if(container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", __FUNCTION__, ((sizeof(Drm_WVOemCryptoKeyObject_V10))*num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoKeyObject_V10))*num_keys);
    for(i = 0, key_object_shared_block_length = 0; i < num_keys; i++)
    {
        key_object_shared_block_length = i * sizeof(Drm_WVOemCryptoKeyObject_V10);
        BKNI_Memcpy(container->blocks[5].data.ptr + key_object_shared_block_length,
            keyArray_V10 + i, sizeof(Drm_WVOemCryptoKeyObject_V10));
    }

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", __FUNCTION__, pst_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[6].data.ptr, pst, pst_length);
        container->blocks[6].len = pst_length ;
    }

    /*
     * Fill in Key Array
     * */
    for(i=0,key_object_shared_block_length=0; i < num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",__FUNCTION__, i,key_object_shared_block_length,keyArray_V10[i].key_id_length));
        /* copy key_id  */
        BKNI_Memcpy(container->blocks[4].data.ptr+key_object_shared_block_length,
                    keyArray_V10[i].key_id,
                    keyArray_V10[i].key_id_length);

        key_object_shared_block_length += keyArray_V10[i].key_id_length;
        BDBG_MSG(("copy the key data iv at offset %d",key_object_shared_block_length));

        /* copy key_data_iv */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray_V10[i].key_data_iv,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;

        BDBG_MSG(("copy the key data  at offset %d",key_object_shared_block_length));

        /* copy key_data */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray_V10[i].key_data,
                    (keyArray_V10[i].key_data_length));

        key_object_shared_block_length += (keyArray_V10[i].key_data_length);

        BDBG_MSG(("copy the key control iv   at offset %d",key_object_shared_block_length));

        /* copy key_control_iv */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray_V10[i].key_control_iv,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;

        BDBG_MSG(("copy the key control   at offset %d",key_object_shared_block_length));

        /* copy key_control */
        BKNI_Memcpy(&container->blocks[4].data.ptr[key_object_shared_block_length],
                    keyArray_V10[i].key_control,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;
    }

    /* update shared block length */
    if(key_array_sz != key_object_shared_block_length)
    {
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", __FUNCTION__, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;



    pKeyObj = (Drm_WVOemCryptoKeyObject_V10*)container->blocks[5].data.ptr;

    for(i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id =&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray_V10[i].key_id_length;

        pKeyObj[i].key_data_iv =&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray_V10[i].key_id_length];

        pKeyObj[i].key_data = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray_V10[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray_V10[i].key_data_length;

        pKeyObj[i].key_control_iv = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray_V10[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray_V10[i].key_data_length];

        pKeyObj[i].key_control = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray_V10[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray_V10[i].key_data_length];
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys_V9_or_V10, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", __FUNCTION__, DrmWVOEMCrypto_CommandId_eLoadKeys));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =    container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS){
        BDBG_ERR(("%s - widevine return code (0x%08x)", __FUNCTION__, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", __FUNCTION__, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        if(container->blocks[3].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[3].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }

        if(container->blocks[4].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[4].data.ptr);
            container->blocks[4].data.ptr = NULL;
        }

        if(container->blocks[5].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[5].data.ptr);
            container->blocks[5].data.ptr = NULL;
        }

        if(container->blocks[6].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[6].data.ptr);
            container->blocks[6].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_LoadKeys_V9_or_V10);
    return rc;
}

uint32_t getSizeOfKeyRefreshObjectArray(const Drm_WVOemCryptoKeyRefreshObject* key_array, uint32_t num_of_keys)
{
    uint32_t sz = 0;
    uint32_t i = 0;

    for(i = 0; i < num_of_keys; i++)
    {
        sz += key_array[i].key_id_length; /* size of keyid */
        sz += WVCDM_KEY_IV_SIZE;
        sz += WVCDM_KEY_CONTROL_SIZE;
    }
    return sz;
}

/****************************************************************************************************
Description:
Updates an existing set of keys for continuing decryption in the current session.

Parameters:
[in] session: handle for the session to be used.
[in] message: pointer to memory containing message to be verified.
[in] message_length: length of the message, in bytes.
[in] signature: pointer to memory containing the signature.
[in] signature_length: length of the signature, in bytes.
[in] num_keys: number of keys present.
[in] key_array: set of key updates.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_SIGNATURE_FAILURE
OEMCrypto_ERROR_INVALID_NONCE
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.

******************************************************************************************************/
DrmRC drm_WVOemCrypto_RefreshKeys(uint32_t session,
                                  const uint8_t* message,
                                      uint32_t message_length,
                                      const uint8_t* signature,
                                      uint32_t signature_length,
                                      uint32_t num_keys,
                                      void* key_array,
                                      int *wvRc)
{

    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    int j = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoKeyRefreshObject* pKeyObj = NULL;
    BSAGElib_InOutContainer *container = NULL;
    Drm_WVOemCryptoKeyRefreshObject* keyArray = (Drm_WVOemCryptoKeyRefreshObject*)key_array;

    BDBG_ENTER(drm_WVOemCrypto_RefreshKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL)
    {
        BDBG_ERR(("%s - message buffer is NULL", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(message_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", __FUNCTION__, message_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = message_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);
    }

    if(signature_length != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Signature (%u bytes)", __FUNCTION__, signature_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, signature, signature_length);
        container->blocks[1].len = signature_length ;
    }

    /* allocate for sending key object*/
    key_array_sz = getSizeOfKeyRefreshObjectArray(key_array,num_keys);
    BDBG_MSG(("%s:Key array sz is %d",__FUNCTION__,key_array_sz));

    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key array (%u bytes)", __FUNCTION__, key_array_sz));
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memset(container->blocks[4].data.ptr, 0xff,key_array_sz);

    for( i=0,j=0; i<num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,j=%d, key id length is %d",__FUNCTION__, i, j, keyArray[i].key_id_length));

        if((keyArray[i].key_control_iv != NULL))
        {
            BDBG_MSG(("copy the keyid at offset %d",j));

            BKNI_Memcpy(container->blocks[4].data.ptr+j, keyArray[i].key_id, keyArray[i].key_id_length);

            j += keyArray[i].key_id_length;

            BDBG_MSG(("copy the key control iv   at offset %d",j));

            BKNI_Memcpy(&container->blocks[4].data.ptr[j], keyArray[i].key_control_iv, WVCDM_KEY_IV_SIZE);
            j += WVCDM_KEY_IV_SIZE;

            BDBG_MSG(("copy the key control   at offset %d",j));
            BKNI_Memcpy(&container->blocks[4].data.ptr[j], keyArray[i].key_control, WVCDM_KEY_IV_SIZE);
            j += WVCDM_KEY_IV_SIZE;
        }
        else /*keycontrol blk is in clear */
        {
            BDBG_MSG(("%s:keycontrol blk is in clear j=%d",__FUNCTION__, j));

            if(keyArray[i].key_id_length != 0)
            {
                BKNI_Memcpy(container->blocks[4].data.ptr+j, keyArray[i].key_id, keyArray[i].key_id_length);
                j += keyArray[i].key_id_length;
            }
            BKNI_Memcpy(&container->blocks[4].data.ptr[j], keyArray[i].key_control, WVCDM_KEY_IV_SIZE);
            j += WVCDM_KEY_IV_SIZE;
        }
    }
    container->blocks[4].len = j;
    BDBG_MSG(("%s: container->blocks[4].data.ptr is %p",__FUNCTION__, (void *)container->blocks[4].data.ptr));


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoKeyRefreshObject))*num_keys), SRAI_MemoryType_Shared);
    if(container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", __FUNCTION__, ((sizeof(Drm_WVOemCryptoKeyRefreshObject))*num_keys)));
        rc = Drm_Err;
        goto ErrorExit;
    }

    pKeyObj = (Drm_WVOemCryptoKeyRefreshObject*)container->blocks[5].data.ptr;
    for(i=0; i<num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        if(keyArray[i].key_control_iv != NULL)
        {
            BDBG_MSG(("%s:Keycontrol block isencrypted",__FUNCTION__));

            pKeyObj[i].key_id = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];
            dump(pKeyObj[i].key_id,keyArray[i].key_id_length,"KeyID in sage mem:");

            BDBG_MSG(("key_id ptr = %p,offset=%d",(void *)pKeyObj[i].key_id,(i*container->blocks[4].len)/num_keys));
            pKeyObj[i].key_id_length = keyArray[i].key_id_length;

            BDBG_MSG(("key_id len =0x%x",pKeyObj[i].key_id_length));
            pKeyObj[i].key_control_iv = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length];

            BDBG_MSG(("key_control_iv ptr =%p, offset=%d",(void *)pKeyObj[i].key_control_iv, (i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length));
            pKeyObj[i].key_control = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys +keyArray[i].key_id_length+WVCDM_KEY_IV_SIZE];

            BDBG_MSG(("key_control ptr =%p",(void *)pKeyObj[i].key_control));
        }
        else
        {
            BDBG_MSG(("%s:Keycontrol block is clear",__FUNCTION__));

            pKeyObj[i].key_id_length = keyArray[i].key_id_length;
            if(pKeyObj[i].key_id_length == 0){
                BDBG_MSG(("%s:one KCB for all keys has to be used",__FUNCTION__));
                pKeyObj[i].key_id = NULL;
            }
            else
            {
                pKeyObj[i].key_id = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];
                dump(pKeyObj[i].key_id, keyArray[i].key_id_length, "KeyID in sage mem:");
            }
            pKeyObj[i].key_control_iv = NULL;
            pKeyObj[i].key_control = &container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length];
            dump(container->blocks[4].data.ptr,WVCDM_KEY_CONTROL_SIZE,"clear key ctrl blk:");

        }
    }

    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoKeyRefreshObject))*num_keys);

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eRefreshKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc   = container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to Refreshkeys but actual operation failed (0x%08x), wvRc = %d ",
                         __FUNCTION__, sage_rc, *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[4].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[4].data.ptr);
            container->blocks[4].data.ptr = NULL;
        }

        if(container->blocks[5].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[5].data.ptr);
            container->blocks[5].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    if (*wvRc!= SAGE_OEMCrypto_SUCCESS)
    {
        rc =  Drm_Err;
    }
    BDBG_LEAVE(drm_WVOemCrypto_RefreshKeys);
    return rc;
}


/*****************************************************************************************************
Desription:
Select a content key and install it in the hardware key ladder for subsequent decryption
operations (OEMCrypto_DecryptCENC()) for this session. The specified key must have been
previously "installed" via OEMCrypto_LoadKeys() or OEMCrypto_RefreshKeys().

Parameters:
[in] session: crypto session identifier.
[in] key_id: pointer to the Key ID.
[in] key_id_length: length of the Key ID, in bytes.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_INVALID_SESSION crypto session ID invalid or not open
OEMCrypto_ERROR_NO_DEVICE_KEY failed to decrypt device key
OEMCrypto_ERROR_NO_CONTENT_KEY failed to decrypt content key
OEMCrypto_ERROR_CONTROL_INVALID invalid or unsupported control input
OEMCrypto_ERROR_KEYBOX_INVALID cannot decrypt and read from Keybox
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
www.widevine.com Confidential Page 36 of 60
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
*****************************************************************************************************/

DrmRC drm_WVOemCrypto_SelectKey(const uint32_t session,
                                    const uint8_t* key_id,
                                    uint32_t key_id_length,
                                    int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t keySlotSelected;
    uint32_t keySlotID[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
    bool allocate_slot = false;
    unsigned int i;

    BDBG_ENTER(drm_WVOemCrypto_SelectKey);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(key_id == NULL)
    {
        BDBG_ERR(("%s - key_id buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:session =%d, key_id_len=%d",__FUNCTION__,session,key_id_length));

    gHostSessionCtx[session].session_id=session;

    BKNI_Memcpy(gHostSessionCtx[session].key_id, key_id, key_id_length);
    gHostSessionCtx[session].key_id_length =key_id_length;

    /* A central key cache should utilize the maximum amount of slots */
    if(gKeySlotCacheMode == DRM_WVOEMCRYPTO_CENTRAL_KEY_CACHE && gKeySlotCacheAllocated < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT)
    {
        allocate_slot = true;
    }

    /* Session based key cache needs only a one time allocation per session */
    if(gKeySlotCacheMode == DRM_WVOEMCRYPTO_SESSION_KEY_CACHE)
    {
        if(gHostSessionCtx[session].num_key_slots < DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
	{
            allocate_slot = true;
	}
    }

    if(allocate_slot)
    {
        for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
        {
            if(gKeySlotCache[i].hSwKeySlot == NULL)
            {
                BDBG_MSG(("%s:Allocate keyslot for index %d",__FUNCTION__, i));
                NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
                keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
                keyslotSettings.client = NEXUS_SecurityClientType_eSage;
                gKeySlotCache[i].hSwKeySlot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
                if(gKeySlotCache[i].hSwKeySlot == NULL)
                {
                    BDBG_ERR(("%s - Error allocating keyslot at index %d", __FUNCTION__, i));
                    if(gHostSessionCtx[session].num_key_slots > 0)
                    {
                        /* Continue onwards if we allocated at least one key slot */
                        break;
                    }
                    else
                    {
                        rc = BERR_INVALID_PARAMETER;
                        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                        goto ErrorExit;
                    }
                }

                BDBG_MSG(("%s: ======Allocated nexus key slot for index %d, keyslot handle=%p =======",
                    __FUNCTION__, i, (void *)gKeySlotCache[i].hSwKeySlot));

                NEXUS_Security_GetKeySlotInfo(gKeySlotCache[i].hSwKeySlot, &keyslotInfo);

                BDBG_MSG(("%s - keyslotID[%d] Keyslot number = '%u'", __FUNCTION__, i, keyslotInfo.keySlotNumber));

                gKeySlotCache[i].keySlotID = keyslotInfo.keySlotNumber;

                gHostSessionCtx[session].key_slot_ptr[gHostSessionCtx[session].num_key_slots] = &gKeySlotCache[i];
		gHostSessionCtx[session].num_key_slots++;

                gKeySlotCacheAllocated++;

                if(gKeySlotCacheMode == DRM_WVOEMCRYPTO_SESSION_KEY_CACHE &&
                    gHostSessionCtx[session].num_key_slots >= DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
                {
                    /* Cap the amount of slots to allocate in session key cache mode */
                    break;
                }
            }
        }
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* Setup key slot ID array */
    for(i = 0; i < gHostSessionCtx[session].num_key_slots; i++)
    {
       if(gHostSessionCtx[session].key_slot_ptr[i] != NULL)
       {
           keySlotID[i] = gHostSessionCtx[session].key_slot_ptr[i]->keySlotID;
       }
       else
       {
           BDBG_ERR(("%s - Error accessing key slot", __FUNCTION__));
           rc = BERR_INVALID_PARAMETER;
           *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
           goto ErrorExit;
       }
    }

    /* allocate buffers accessible by Sage*/
    if(key_id_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(key_id_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)", __FUNCTION__, key_id_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        container->blocks[0].len = key_id_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, key_id, key_id_length);
        dump(container->blocks[0].data.ptr,key_id_length,"key id in sage mem:");
    }

    /* Provide allocated key slots */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(sizeof(keySlotID[i]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)",
            __FUNCTION__, sizeof(keySlotID[i]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[1].len = sizeof(keySlotID[i]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT;

    BKNI_Memcpy(container->blocks[1].data.ptr, keySlotID,
        sizeof(keySlotID[i]) * gHostSessionCtx[session].num_key_slots);
    dump(container->blocks[1].data.ptr, sizeof(keySlotID[i]) * gHostSessionCtx[session].num_key_slots,"key slot ID in sage mem:");

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = key_id_length;
    container->basicIn[2] = gHostSessionCtx[session].num_key_slots;

    /* Set to an invalid value to see if key slot was selected */
    container->basicOut[3] = INVALID_KEYSLOT_ID;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eSelectKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:Sage process command is sucesss, next extract the status of the command",__FUNCTION__));

    *wvRc = container->basicOut[2];
    BDBG_MSG(("%s:* WVRC=%d",__FUNCTION__,*wvRc));

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to loadkey but actual operation failed (0x%08x, 0x%08x)", __FUNCTION__, container->basicOut[0],
            container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if (container->basicOut[3] == INVALID_KEYSLOT_ID)
    {
        /* If key slot was not selected but a successful operation, an AES decryption key was not selected.
         * Key slot is expected to not be selected in this scenario.
         */
        goto IgnoreKeySlot;
    }

    keySlotSelected = container->basicOut[3];

    for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
    {
        if(gKeySlotCache[i].hSwKeySlot != NULL && keySlotSelected == gKeySlotCache[i].keySlotID)
        {
           gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = gKeySlotCache[i].hSwKeySlot;
           break;
        }
    }
    if(i == DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT)
    {
        BDBG_ERR(("%s - Error unknown keyslot %d selected", __FUNCTION__, keySlotSelected));
        rc = Drm_Err;
        goto ErrorExit;
    }
    BDBG_MSG(("%s - Selected by Sage: keyslotID[%d] Keyslot number = '%u'", __FUNCTION__, i, keyslotInfo.keySlotNumber));

    /* Set the cipher mode */
    gHostSessionCtx[session].cipher_mode = container->basicOut[1];

IgnoreKeySlot:
ErrorExit:

    BDBG_MSG(("%s: free container block 0:",__FUNCTION__));
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_MSG(("%s:set the return value based on wvRc",__FUNCTION__));
    if (*wvRc!= 0)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_SelectKey);

   return rc;
}



void DRM_WVOemcrypto_EndianSwap(unsigned int *x)
{
    *x = (*x>>24) |
        ((*x<<8) & 0x00FF0000) |
        ((*x>>8) & 0x0000FF00) |
        (*x<<24);
}

static void drm_WVOemCrypto_P_Complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static DrmRC drm_WVOemCrypto_P_Wait_Clear_Transfer(void)
{
    NEXUS_DmaJobStatus status;
    NEXUS_Error nexus_rc;
    DrmRC drm_rc = Drm_Success;

    if (gWVClrEvent == NULL || gWVClrDma == NULL || gWVClrJob == NULL)
    {
        /* Simply warn no job pending to wait upon */
        BDBG_WRN(("%s: No DMA job pending", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_WaitForEvent(gWVClrEvent, BKNI_INFINITE);

    nexus_rc = NEXUS_DmaJob_GetStatus(gWVClrJob, &status);
    if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: DmaJob_GetStatus err=%d", __FUNCTION__, nexus_rc));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    if (status.currentState != NEXUS_DmaJobState_eComplete ) {
        BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed", __FUNCTION__ ));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

ErrorExit:

    gWVClrQueued = false;

    if(gWVClrDma != NULL) {
        if(gWVClrJob != NULL) {
            NEXUS_DmaJob_Destroy(gWVClrJob);
            gWVClrJob = NULL;
        }
        NEXUS_Dma_Close(gWVClrDma);
        gWVClrDma = NULL;
    }

    return drm_rc;
}

static DrmRC drm_WVOemCrypto_P_Transfer_Clear_Buffer(void)
{
    NEXUS_DmaJobSettings dmaJobSettings;
    NEXUS_DmaJobBlockSettings *blockSettings;
    NEXUS_Error nexus_rc;
    DrmRC drm_rc = Drm_Success;
    unsigned int i;

    if (gWvClrNumDmaBlocks == 0)
    {
        /* Return with warning if there is nothing to transfer */
        BDBG_WRN(("%s: No valid data to transfer", __FUNCTION__));
        goto ErrorExit;
    }

    if (gWVClrQueued || gWVClrDma != NULL || gWVClrJob != NULL)
    {
        BDBG_WRN(("%s: DMA did not finish, forcing wait", __FUNCTION__));
        drm_rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
        if (drm_rc != Drm_Success)
        {
            BDBG_ERR(("%s: Failed to verify DMA transfer complete", __FUNCTION__));
            goto ErrorExit;
        }
    }

    if (gWVClrEvent == NULL)
    {
        BKNI_CreateEvent(&gWVClrEvent);
        if (gWVClrEvent == NULL)
        {
            BDBG_ERR(("%s: Failed to create event", __FUNCTION__));
            drm_rc = Drm_Err;
            goto ErrorExit;
        }
    }

    gWVClrDma = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    if (gWVClrDma == NULL)
    {
        BDBG_ERR(("%s: Failed to NEXUS_Dma_Open !!!", __FUNCTION__));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    NEXUS_DmaJob_GetDefaultSettings(&dmaJobSettings);
    dmaJobSettings.completionCallback.callback = drm_WVOemCrypto_P_Complete;
    dmaJobSettings.completionCallback.context = gWVClrEvent;
    dmaJobSettings.bypassKeySlot = NEXUS_BypassKeySlot_eGR2R;
    dmaJobSettings.numBlocks = gWvClrNumDmaBlocks;
    gWVClrJob = NEXUS_DmaJob_Create(gWVClrDma, &dmaJobSettings);

    if (gWVClrJob == NULL)
    {
        BDBG_ERR(("%s: Failed to NEXUS_DmaJob_Create !!!", __FUNCTION__));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    blockSettings = &gWvClrDmaJobBlockSettingsList[gWvClrNumDmaBlocks - 1];

    /* start DMA transfer when last subsample or beyond MAX */
    BDBG_MSG(("%s  nb_blks=%d",  __FUNCTION__, gWvClrNumDmaBlocks));

    blockSettings->scatterGatherCryptoEnd = true;

    /* cache flush on source address if needed */
    for (i = 0; i < gWvClrNumDmaBlocks; i++)
    {
        blockSettings = &gWvClrDmaJobBlockSettingsList[i];
        if (!blockSettings->cached)
        {
            NEXUS_FlushCache(blockSettings->pSrcAddr, blockSettings->blockSize);
        }
    }

    nexus_rc = NEXUS_DmaJob_ProcessBlocks(gWVClrJob, gWvClrDmaJobBlockSettingsList, gWvClrNumDmaBlocks);
    if (nexus_rc == NEXUS_DMA_QUEUED)
    {
        /* Exit here as DMA has not finished yet.
         * Expect a call to drm_WVOemCrypto_P_Wait_Clear_Transfer()
         */
        gWVClrQueued = true;
        gWvClrNumDmaBlocks= 0;
        return drm_rc;

    } else if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: error in dma transfer, err:%d", __FUNCTION__, nexus_rc));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    /* reset the number of blocks */
    gWvClrNumDmaBlocks= 0;

ErrorExit:
    if(gWVClrDma != NULL)
    {
        if(gWVClrJob != NULL)
        {
            NEXUS_DmaJob_Destroy(gWVClrJob);
            gWVClrJob = NULL;
        }
        NEXUS_Dma_Close(gWVClrDma);
        gWVClrDma = NULL;
    }

    return drm_rc;
}

static DrmRC drm_WVOemCrypto_P_CopyBuffer_Secure_SG(
    uint8_t *destination,
    const uint8_t *data_addr,
    uint32_t data_length,
    uint32_t subsample_flags,
    bool block_wait)
{
    bool isLastSubsample = false;
    NEXUS_DmaJobSettings dmaJobSettings;
    NEXUS_DmaJobBlockSettings *blockSettings;
    DrmRC rc = Drm_Success;
    unsigned int i;

    isLastSubsample = subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE;

    BKNI_AcquireMutex(gWVClrMutex);

    /* Skip adding additional block if pointer or length invalid
     * but check for last subsample flag as it may just be a signal
     * to execute the DMA transfer
     */
    if (data_addr != NULL && destination != NULL && data_length > 0)
    {
        blockSettings = &gWvClrDmaJobBlockSettingsList[gWvClrNumDmaBlocks];
        BDBG_MSG(("%s: entry#=%d, data_addr=0x%08x, len=%d, isLastSubSample=%d",
            __FUNCTION__, gWvClrNumDmaBlocks, data_addr, data_length, isLastSubsample));

        NEXUS_DmaJob_GetDefaultBlockSettings(blockSettings);
        blockSettings->pDestAddr = destination;
        blockSettings->pSrcAddr = data_addr;
        blockSettings->blockSize = data_length;
        blockSettings->scatterGatherCryptoStart = (gWvClrNumDmaBlocks == 0);
        blockSettings->scatterGatherCryptoEnd = false;
        blockSettings->cached = false;

        gWvClrNumDmaBlocks++;
    }

    if (!isLastSubsample && gWvClrNumDmaBlocks < MAX_SG_DMA_BLOCKS)
    {
        /* Exit allowing dma blocks to queue up */
        BKNI_ReleaseMutex(gWVClrMutex);
        return Drm_Success;
    }
    else
    {
        if (gWvClrNumDmaBlocks == 0)
        {
            /* 0 bytes have been transferred successfully so return success. */
            goto ErrorExit;
        }

        if (gWvClrNumDmaBlocks >= MAX_SG_DMA_BLOCKS)
        {
            /* Force a wait for transfer */
            block_wait = true;
        }

        rc = drm_WVOemCrypto_P_Transfer_Clear_Buffer();
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s: Unable to initiate DMA transfer",__FUNCTION__));
            goto ErrorExit;
        }

        if (block_wait && gWVClrQueued)
        {
            rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
            if (rc != Drm_Success)
            {
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", __FUNCTION__));
                goto ErrorExit;
            }
        }
    }

ErrorExit:
    BKNI_ReleaseMutex(gWVClrMutex);
    return rc;
}

static DrmRC drm_WVOemCrypto_P_Decrypt_Transfer(uint32_t session)
{
    bool isLastSubsample = false;
    DmaBlockInfo_t *blockInfo;
    uint32_t dmaBlockIdx;
    DrmRC rc;

    dmaBlockIdx = gHostSessionCtx[session].drmCommonOpStruct.num_dma_block;
    if (dmaBlockIdx == 0)
    {
        /* 0 bytes have been transferred successfully so return success. */
        return Drm_Success;
    }

    if (gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock == NULL)
    {
        BDBG_ERR(("%s: error, DMA block info pointer is NULL", __FUNCTION__));
        return Drm_Err;
    }

    blockInfo = gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock;

    /* Mark the end of the scatter/gather list */
    blockInfo[dmaBlockIdx-1].sg_end = true;

    BDBG_MSG(("%s: session[%d], nb_blks=%d", __FUNCTION__, session, dmaBlockIdx));

    /*start M2M transfer*/
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_M2mOperation(&gHostSessionCtx[session].drmCommonOpStruct, true, true);
#else
    rc = DRM_Common_TL_M2mOperation_TA(Common_Platform_Widevine, &gHostSessionCtx[session].drmCommonOpStruct, true, true);
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Call to 'DRM_Common_TL_M2mOperation' failed 0x%x", __FUNCTION__, rc));
        return rc;
    }

    BDBG_MSG(("%s - Decrypted %d dma blocks", __FUNCTION__, dmaBlockIdx));
    gHostSessionCtx[session].drmCommonOpStruct.num_dma_block = 0;

    return rc;
}


static DrmRC drm_WVOemCrypto_P_DecryptDMA_Secure_SG(uint8_t *data_addr,
                                                     uint8_t *out_addr,
                                                     uint32_t data_length,
                                                     uint32_t enc_subsample_flags,
                                                     uint32_t *block_offset,
                                                     uint32_t session)
{
    bool isFirstEncSubsample = false;
    bool isLastEncSubsample = false;
    DmaBlockInfo_t *blockInfo;
    uint32_t dmaBlockIdx;
    DrmRC rc = Drm_Success;

    isFirstEncSubsample = enc_subsample_flags & WV_OEMCRYPTO_FIRST_SUBSAMPLE;
    isLastEncSubsample = enc_subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE;

    /* Allocate the dma block memory if it doesn't exist yet */
    blockInfo = gWvEncDmaBlockInfoList[session];
    if (blockInfo == NULL) {
        blockInfo = (DmaBlockInfo_t *)SRAI_Memory_Allocate(sizeof(DmaBlockInfo_t) * MAX_SG_DMA_BLOCKS, SRAI_MemoryType_Shared);
        if (blockInfo == NULL)
        {
            BDBG_ERR(("%s: error allocating dma blockinfo memory", __FUNCTION__));
            rc = Drm_Err;
            goto ErrorExit;
        }
        gWvEncDmaBlockInfoList[session] = blockInfo;
    }

    dmaBlockIdx = gHostSessionCtx[session].drmCommonOpStruct.num_dma_block;
    BDBG_MSG(("%s  session=%d, data_addr=0x%08x, len=%d, isFirstEncSubsample=%d, isLastEncSubsample=%d",
        __FUNCTION__, session, data_addr,data_length, isFirstEncSubsample, isLastEncSubsample ));

    if (data_addr != NULL && out_addr != NULL && data_length > 0)
    {
        if (gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0) {
            dmaBlockIdx = 0;

            if (gHostSessionCtx[session].btp_info.btp_sage_buffer != NULL)
            {
                /* BTP block */
                blockInfo[dmaBlockIdx].pDstData = gHostSessionCtx[session].btp_info.btp_sage_buffer;
                blockInfo[dmaBlockIdx].pSrcData = gHostSessionCtx[session].btp_info.btp_sage_buffer;
                blockInfo[dmaBlockIdx].uiDataSize = gHostSessionCtx[session].btp_info.btp_sage_size;
                blockInfo[dmaBlockIdx].sg_start = true;
                blockInfo[dmaBlockIdx].sg_end = true;
                dmaBlockIdx++;
            }

            if (*block_offset != 0)
            {
                /* Add padding for correct IV counter */
                BDBG_ASSERT(dmaBlockIdx < (MAX_SG_DMA_BLOCKS - 1));
                blockInfo[dmaBlockIdx].pDstData = gPadding;
                blockInfo[dmaBlockIdx].pSrcData = gPadding;
                blockInfo[dmaBlockIdx].uiDataSize = *block_offset;
                blockInfo[dmaBlockIdx].sg_start = true;
                blockInfo[dmaBlockIdx].sg_end = false;
                dmaBlockIdx++;
            }
        }

        blockInfo[dmaBlockIdx].pDstData = out_addr;
        blockInfo[dmaBlockIdx].pSrcData = data_addr;
        blockInfo[dmaBlockIdx].uiDataSize = data_length;
        blockInfo[dmaBlockIdx].sg_start = (isFirstEncSubsample && *block_offset == 0);
        blockInfo[dmaBlockIdx].sg_end = false; /* End marker set on transfer execution */
        dmaBlockIdx++;
        gHostSessionCtx[session].drmCommonOpStruct.num_dma_block = dmaBlockIdx;
    }

    if (!isLastEncSubsample && dmaBlockIdx < MAX_SG_DMA_BLOCKS)
    {
        /* only have the dma blocks built */
        return Drm_Success;
    }
    else
    {
        if (dmaBlockIdx > 0)
        {
            /*start M2M transfer only when it is the last subsample or blockInfo is full*/
            gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock = gWvEncDmaBlockInfoList[session];
            rc = drm_WVOemCrypto_P_Decrypt_Transfer(session);
        }
    }
ErrorExit:
    return rc;
}

static DrmRC drm_WVOemCrypto_P_DecryptDMA_NonSecure(const uint8_t* cipher_data,
                                 void* clear_data,
                                 uint32_t cipher_data_length,
                                 uint32_t* out_sz,
                                 uint32_t *block_offset,
                                 uint32_t session)
{

    uint32_t cipher_data_len_aligned = 0;
    DrmRC  rc = Drm_Success;
    DmaBlockInfo_t dmaBlock[2];
    uint8_t *pDecryptBuf = NULL;
    uint32_t dmaBlockIdx = 0;

    BDBG_MSG(("%s - Entered", __FUNCTION__));


    BDBG_MSG((" data length is %d, aligned data len is %d, block_offset =%d",cipher_data_length,cipher_data_len_aligned,*block_offset));


    if(((cipher_data_length+*block_offset )% 16)!=0)
    {
        cipher_data_len_aligned = cipher_data_length+*block_offset +(16- ((cipher_data_length+*block_offset ) % 16)) ;
    }
    else
    {
        cipher_data_len_aligned = cipher_data_length+*block_offset ;
    }

    BDBG_MSG((" data length is %d, aligned data len is %d, block_offset =%d",cipher_data_length,cipher_data_len_aligned,*block_offset));


    /* Allocate buffer memory */
    pDecryptBuf = SRAI_Memory_Allocate(cipher_data_len_aligned, SRAI_MemoryType_Shared);
    if(pDecryptBuf == NULL)
    {
        BDBG_ERR(("%s: %d Out of memory\n", __FUNCTION__, __LINE__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:successfully allocated memory to input",__FUNCTION__));

    BKNI_Memcpy(pDecryptBuf+(*block_offset),cipher_data,cipher_data_length);

    if(cipher_data_len_aligned > cipher_data_length)
    {
        BKNI_Memset((uint8_t *)pDecryptBuf+(*block_offset)+cipher_data_length, 0x00,cipher_data_len_aligned - cipher_data_length);
    }

    /* set DMA parameters */

    BDBG_MSG(("%s: blk0 btp buff 0x%08x, size=%d",__FUNCTION__,gHostSessionCtx[session].btp_info.btp_sage_buffer,
        gHostSessionCtx[session].btp_info.btp_sage_size));

    if (dmaBlockIdx == 0 && gHostSessionCtx[session].btp_info.btp_sage_buffer != NULL)
    {
        dmaBlock[dmaBlockIdx].pDstData = gHostSessionCtx[session].btp_info.btp_sage_buffer;
        dmaBlock[dmaBlockIdx].pSrcData = gHostSessionCtx[session].btp_info.btp_sage_buffer;
        dmaBlock[dmaBlockIdx].uiDataSize = gHostSessionCtx[session].btp_info.btp_sage_size;
        dmaBlock[dmaBlockIdx].sg_start = true;
        dmaBlock[dmaBlockIdx].sg_end = true;
        ++dmaBlockIdx;
    }

    dmaBlock[dmaBlockIdx].pDstData = pDecryptBuf;
    dmaBlock[dmaBlockIdx].pSrcData = pDecryptBuf;
    dmaBlock[dmaBlockIdx].uiDataSize = cipher_data_len_aligned;
    dmaBlock[dmaBlockIdx].sg_start = true;
    dmaBlock[dmaBlockIdx].sg_end = true;
    ++dmaBlockIdx;

    gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock = dmaBlock;
    gHostSessionCtx[session].drmCommonOpStruct.num_dma_block = dmaBlockIdx;

    rc = drm_WVOemCrypto_P_Decrypt_Transfer(session);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Call to 'drm_WVOemCrypto_P_Decrypt_Transfer' failed", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memcpy(clear_data, pDecryptBuf + *block_offset, cipher_data_length);

    *out_sz = cipher_data_length;

ErrorExit:
    if(pDecryptBuf)
    {
        SRAI_Memory_Free(pDecryptBuf);
    }

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    return rc;
}

static DrmRC drm_WVOemCrypto_P_DecryptPatternBlock(uint32_t session,
                                     const uint8_t* data_addr,
                                     uint32_t data_length,
                                     bool is_encrypted,
                                     Drm_WVOemCryptoBufferType_t buffer_type,
                                     const uint8_t* iv,
                                     uint32_t block_offset,
                                     uint8_t* out_buffer,
                                     uint32_t *out_sz,
                                     uint8_t subsample_flags,
                                     int *wvRc)
{
    BSAGElib_InOutContainer *container = NULL;

    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    *wvRc = SAGE_OEMCrypto_SUCCESS;
    bool isSecureDecrypt = (buffer_type == Drm_WVOEMCrypto_BufferType_Secure);

    BDBG_ENTER(drm_WVOemCrypto_P_DecryptPatternBlock);
    BDBG_MSG(("%s: Input data len=%d,is_encrypted=%d, sf:%d, secure:%d",
                __FUNCTION__, data_length,is_encrypted,subsample_flags, isSecureDecrypt));

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - data buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    dump(data_addr,data_length,"encrypted data:");

    if(iv == NULL)
    {
        BDBG_ERR(("%s - iv buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if (!is_encrypted)
    {
        if (isSecureDecrypt)
        {
            if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(out_buffer, data_addr, data_length, subsample_flags, false) != Drm_Success)
            {
                BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }

            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                 /* Last subsample is clear. Transfer the encrypted block list */
                 if(drm_WVOemCrypto_P_DecryptDMA_Secure_SG(NULL, NULL, 0,
                     subsample_flags, &block_offset, session) != Drm_Success)
                 {
                     BDBG_ERR(("%s: decryption failed for DMA SG Secure Buffer Type",__FUNCTION__));
                     rc = Drm_Err;
                     *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                     goto ErrorExit;
                 }
            }
        }
        else
        {
            BKNI_Memcpy(out_buffer,data_addr, data_length);
            return Drm_Success;
        }
    }
    else
    {
        if(gHostSessionCtx[session].cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
        {
             /* Verify block offset is 0 when cipher mode is CBC */
             if(block_offset != 0)
             {
                 BDBG_ERR(("%s: Invalid block offset provided",__FUNCTION__));
                 rc = Drm_Err;
                 *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                 goto ErrorExit;
             }
        }

        /* Mark if this is the first encrypted subsample */
        if (gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            subsample_flags |= WV_OEMCRYPTO_FIRST_SUBSAMPLE;
        }

        /* Load the IV on the first block of a DMA */
        if(gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            container = SRAI_Container_Allocate();
            if(container == NULL)
            {
                BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            /* Allocate buffers accessible by Sage*/
            if(iv != NULL)
            {
                container->blocks[0].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
                if(container->blocks[0].data.ptr == NULL)
                {
                    BDBG_ERR(("%s: Out of memory for IV (%u bytes)", __FUNCTION__, WVCDM_KEY_IV_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                container->blocks[0].len = WVCDM_KEY_IV_SIZE;
                BKNI_Memcpy(container->blocks[0].data.ptr, iv, WVCDM_KEY_IV_SIZE);
            }

            /* Allocate BTP buffer if needed */
            if (gHostSessionCtx[session].btp_info.btp_sage_buffer == NULL)
            {
                gHostSessionCtx[session].btp_info.btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_SagePrivate);
                if(gHostSessionCtx[session].btp_info.btp_sage_buffer == NULL)
                {
                    BDBG_ERR(("%s: Out of memory for BTP (%u bytes)", __FUNCTION__, BTP_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                gHostSessionCtx[session].btp_info.btp_sage_size = BTP_SIZE;
            }

            /* Map to parameters into srai_inout_container */
            container->basicIn[0] = session;
            container->basicIn[1] = WVCDM_KEY_IV_SIZE;
            container->basicIn[2] = buffer_type;

            container->basicIn[3]= BTP_SIZE;
            container->blocks[1].data.ptr = gHostSessionCtx[session].btp_info.btp_sage_buffer;
            container->blocks[1].len = BTP_SIZE;

            /* This command loads the IV and validates key control */
            sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDecryptCENC, container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error in DrmWVOEMCrypto_CommandId_eDecryptCENC", __FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            *wvRc = container->basicOut[2];
            if (container->basicOut[0] != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Command was sent successfully to load IV in decryptCENC but actual operation failed (0x%08x)",
                    __FUNCTION__, container->basicOut[0]));
                rc = Drm_Err;
                goto ErrorExit;
            }

            BDBG_MSG(("%s:extract wv error code",__FUNCTION__));
            if(*wvRc != SAGE_OEMCrypto_SUCCESS)
            {
                BDBG_ERR(("%s:WVRC error",__FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }

        /* Now kickoff dma */
        if(isSecureDecrypt)
        {
            if(drm_WVOemCrypto_P_DecryptDMA_Secure_SG((uint8_t *)data_addr, out_buffer,
                data_length, subsample_flags, &block_offset, session) != Drm_Success)
            {
                BDBG_ERR(("%s: decryption failed for SG Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }

            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(NULL, NULL, 0, subsample_flags, false) != Drm_Success)
                {
                    BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",__FUNCTION__));
                    rc = Drm_Err;
                    *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                    goto ErrorExit;
                }
            }
        }
        else
        {
            if (drm_WVOemCrypto_P_DecryptDMA_NonSecure(data_addr, out_buffer, data_length,
                out_sz, &block_offset, session) != Drm_Success)
            {
                BDBG_ERR(("%s: decryption failed for non-Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }
        }
    }

    if(isSecureDecrypt)
    {
        /* If the clear buffer DMA job is queued, wait for completion here */
        BKNI_AcquireMutex(gWVClrMutex);
        if (gWVClrQueued)
        {
            rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
            if (rc != Drm_Success)
            {
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", __FUNCTION__));
            }
        }
        BKNI_ReleaseMutex(gWVClrMutex);
    }

ErrorExit:
    BDBG_MSG(("%s: Free the container",__FUNCTION__));

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    if(*wvRc !=SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_P_DecryptPatternBlock);

    return rc;
}

void ctr128_inc64(uint32_t increase_by, uint8_t* counter)
{
    uint32_t n, i;

    for (i = 0; i < increase_by; i++)
    {
        n = 16;

        do {
            if (++counter[--n] != 0)
                break;
        } while (n > 8);
    }
}

/****************************************************************************************************
Description:
Decrypts (AES128CTR) or copies the payload in the buffer referenced by the *data_addr
parameter into the buffer referenced by the out_buffer parameter, using the session context
indicated by the session parameter.

Parameters:
[in] session: crypto session identifier.
[in] data_addr: An unaligned pointer to this segment of the stream.
[in] data_length: The length of this segment of the stream, in bytes.
[in] is_encrypted: True if the buffer described by data_addr, data_length is encrypted. If
is_encrypted is false, only the data_addr and data_length parameters are used. The iv and offset
arguments are ignored.
[in] iv: The initial value block to be used for content decryption.
[in] block_offset: If nonzero, the decryption block boundary is different from the start of the data.
[in] out_buffer: A callerowned descriptor that specifies the handling of the decrypted byte
stream. See OEMCrypto_DestbufferDesc for details.
[in] subsample_flags: bitwise flags indicating if this is the first, middle, or last subsample in a
chunk of data. 1 = first subsample, 2 = last subsample, 3 = both first and last subsample, 0 =
neither first nor last subsample.

Returns:
OEMCrypto_SUCCESS
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_DECRYPT_FAILED
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
******************************************************************************************************/

DrmRC drm_WVOemCrypto_DecryptCTR_V10(uint32_t session,
                                     const uint8_t* data_addr,
                                     uint32_t data_length,
                                     bool is_encrypted,
                                     Drm_WVOemCryptoBufferType_t buffer_type,
                                     const uint8_t* iv,
                                     uint32_t block_offset,
                                     void* out_buffer,
                                     uint32_t *out_sz,
                                     uint8_t subsample_flags,
                                     int *wvRc)
{
    BSAGElib_InOutContainer *container = NULL;

    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    *wvRc = SAGE_OEMCrypto_SUCCESS;
    bool isSecureDecrypt = (buffer_type == Drm_WVOEMCrypto_BufferType_Secure);

    BDBG_ENTER(drm_WVOemCrypto_DecryptCTR_V10);
    BDBG_MSG(("%s: Input data len=%d,is_encrypted=%d, sf:%d, secure:%d",
                __FUNCTION__, data_length,is_encrypted,subsample_flags, isSecureDecrypt));

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - data buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    dump(data_addr,data_length,"encrypted data:");

    if(iv == NULL)
    {
        BDBG_ERR(("%s - iv buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if (!is_encrypted)
    {
        if (isSecureDecrypt)
        {
            if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(out_buffer, data_addr, data_length, subsample_flags, false) != Drm_Success)
            {
                BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }

            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                 /* Last subsample is clear. Transfer the encrypted block list */
                 if(drm_WVOemCrypto_P_DecryptDMA_Secure_SG(NULL, NULL, 0,
                     subsample_flags, &block_offset, session) != Drm_Success)
                 {
                     BDBG_ERR(("%s: decryption failed for DMA SG Secure Buffer Type",__FUNCTION__));
                     rc = Drm_Err;
                     *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                     goto ErrorExit;
                 }
            }
        }
        else
        {
            BKNI_Memcpy(out_buffer,data_addr, data_length);
            return Drm_Success;
        }
    }
    else
    {
        /* Mark if this is the first encrypted subsample */
        if (gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            subsample_flags |= WV_OEMCRYPTO_FIRST_SUBSAMPLE;
        }

        /* Load the IV on the first block of a DMA */
        if(gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            container = SRAI_Container_Allocate();
            if(container == NULL)
            {
                BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            /* Allocate buffers accessible by Sage*/
            if(iv != NULL)
            {
                container->blocks[0].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
                if(container->blocks[0].data.ptr == NULL)
                {
                    BDBG_ERR(("%s: Out of memory for IV (%u bytes)", __FUNCTION__, WVCDM_KEY_IV_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                container->blocks[0].len = WVCDM_KEY_IV_SIZE;
                BKNI_Memcpy(container->blocks[0].data.ptr, iv, WVCDM_KEY_IV_SIZE);
            }

            /* Allocate BTP buffer if needed */
            if (gHostSessionCtx[session].btp_info.btp_sage_buffer == NULL)
            {
                gHostSessionCtx[session].btp_info.btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_SagePrivate);
                if(gHostSessionCtx[session].btp_info.btp_sage_buffer == NULL)
                {
                    BDBG_ERR(("%s: Out of memory for BTP (%u bytes)", __FUNCTION__, BTP_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                gHostSessionCtx[session].btp_info.btp_sage_size = BTP_SIZE;
            }

            /* Map to parameters into srai_inout_container */
            container->basicIn[0] = session;
            container->basicIn[1] = WVCDM_KEY_IV_SIZE;
            container->basicIn[2] = buffer_type;

            container->basicIn[3]= BTP_SIZE;
            container->blocks[1].data.ptr = gHostSessionCtx[session].btp_info.btp_sage_buffer;
            container->blocks[1].len = BTP_SIZE;

            /* This command loads the IV and validates key control */
            sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDecryptCTR_V10, container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error in DrmWVOEMCrypto_CommandId_eDecryptCENC", __FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            *wvRc = container->basicOut[2];
            if (container->basicOut[0] != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Command was sent successfully to load IV in decryptCENC but actual operation failed (0x%08x)",
                    __FUNCTION__, container->basicOut[0]));
                rc = Drm_Err;
                goto ErrorExit;
            }

            BDBG_MSG(("%s:extract wv error code",__FUNCTION__));
            if(*wvRc != SAGE_OEMCrypto_SUCCESS)
            {
                BDBG_ERR(("%s:WVRC error",__FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }

        /* Now kickoff dma */
        if(isSecureDecrypt)
        {
            if(drm_WVOemCrypto_P_DecryptDMA_Secure_SG((uint8_t *)data_addr, out_buffer,
                data_length, subsample_flags, &block_offset, session) != Drm_Success)
            {
                BDBG_ERR(("%s: decryption failed for SG Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }

            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(NULL, NULL, 0, subsample_flags, false) != Drm_Success)
                {
                    BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",__FUNCTION__));
                    rc = Drm_Err;
                    *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                    goto ErrorExit;
                }
            }
        }
        else
        {
            if (drm_WVOemCrypto_P_DecryptDMA_NonSecure(data_addr, out_buffer, data_length,
                out_sz, &block_offset, session) != Drm_Success)
            {
                BDBG_ERR(("%s: decryption failed for non-Secure Buffer Type",__FUNCTION__));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }
        }
    }

    if(isSecureDecrypt)
    {
        /* If the clear buffer DMA job is queued, wait for completion here */
        BKNI_AcquireMutex(gWVClrMutex);
        if (gWVClrQueued)
        {
            rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
            if (rc != Drm_Success)
            {
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", __FUNCTION__));
            }
        }
        BKNI_ReleaseMutex(gWVClrMutex);
    }

ErrorExit:
    BDBG_MSG(("%s: Free the container",__FUNCTION__));

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    if(*wvRc !=SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_DecryptCTR_V10);

    return rc;
}

/****************************************************************************************************
Description:
Decrypts (AES128CTR/CBC) or copies the payload in the buffer referenced by the *data_addr
parameter into the buffer referenced by the out_buffer parameter, using the session context
indicated by the session parameter.

Parameters:
[in] session: crypto session identifier.
[in] data_addr: An unaligned pointer to this segment of the stream.
[in] data_length: The length of this segment of the stream, in bytes.
[in] is_encrypted: True if the buffer described by data_addr, data_length is encrypted. If
is_encrypted is false, only the data_addr and data_length parameters are used. The iv and offset
arguments are ignored.
[in] iv: The initial value block to be used for content decryption.
[in] block_offset: If nonzero, the decryption block boundary is different from the start of the data.
[in] out_buffer: A callerowned descriptor that specifies the handling of the decrypted byte
stream. See OEMCrypto_DestbufferDesc for details.
[in] pattern: struct to describe the pattern of encrypt/clear blocks in 16 byte lengths.
[in] subsample_flags: bitwise flags indicating if this is the first, middle, or last subsample in a
chunk of data. 1 = first subsample, 2 = last subsample, 3 = both first and last subsample, 0 =
neither first nor last subsample.

Returns:
OEMCrypto_SUCCESS
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_DECRYPT_FAILED
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
******************************************************************************************************/

DrmRC drm_WVOemCrypto_DecryptCENC(uint32_t session,
                                  const uint8_t* data_addr,
                                  uint32_t data_length,
                                  bool is_encrypted,
                                  Drm_WVOemCryptoBufferType_t buffer_type,
                                  const uint8_t* iv,
                                  uint32_t block_offset,
                                  void* out_buffer,
                                  uint32_t *out_sz,
                                  void *pattern,
                                  uint8_t subsample_flags,
                                  int *wvRc)
{
    DrmRC rc = Drm_Success;
    Drm_WVOemCryptoEncryptPattern_t *pPattern = (Drm_WVOemCryptoEncryptPattern_t *)pattern;
    uint32_t data_processed = 0; /* Amount of data processed */
    uint32_t data_remain = data_length; /* Amount of data remaining to process */
    uint32_t blockDataLength; /* Length of given sub pattern block to process */
    bool block_encrypted;
    uint8_t block_subsample_flags = 0;
    uint32_t block_out_sz = 0;
    bool process_skip_flag = false; /* Toggle between encrypt/clear blocks */
    Drm_WVOemCryptoCipherMode cipher_mode = gHostSessionCtx[session].cipher_mode;
    uint8_t *dst_ptr = (uint8_t *)out_buffer;
    uint8_t *src_ptr = (uint8_t *)data_addr;
    uint8_t block_iv[WVCDM_KEY_IV_SIZE];

    BDBG_ENTER(drm_WVOemCrypto_DecryptCENC);

    *out_sz = 0;
    BKNI_Memcpy(block_iv, &iv[0], WVCDM_KEY_IV_SIZE);

    while (data_processed < data_length)
    {
        data_remain = data_length - data_processed;
        if (is_encrypted)
        {
            block_encrypted = true;

            /* Check for partial block in CBC cipher mode */
            if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC && data_remain < AES_BLOCK_SIZE)
            {
                /* Partial block is in the clear */
                blockDataLength = data_remain;
                block_encrypted = false;
            }
            else
            {
                /* Evaluate pattern */
                if (pPattern->skip == 0)
                {
                    if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
                    {
                         /* Assign aligned buffer length to decrypt in CBC cipher mode */
                         blockDataLength = data_remain - (data_remain % AES_BLOCK_SIZE);
                    }
                    else
                    {
                         /* Assign entire buffer length to decrypt in CTR cipher mode */
                         blockDataLength = data_remain;
                    }
                }
                else
                {
                    if (!process_skip_flag)
                    {
                        /* Handle encrypt pattern block */
                        blockDataLength = pPattern->encrypt * AES_BLOCK_SIZE;
                        if (blockDataLength > data_remain)
                        {
                            if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
                            {
                                /* CBC must consider possible partial block */
                                blockDataLength = data_remain - (data_remain % AES_BLOCK_SIZE);
                            }
                            else
                            {
                                /* CTR ignores partial block */
                                blockDataLength = data_remain;
                            }
                        }
                        process_skip_flag = true; /* Signal to process skip portion on next iteration */
                    }
                    else
                    {
                        /* Handle skip/clear pattern block */
                        blockDataLength = pPattern->skip * AES_BLOCK_SIZE;
                        if (blockDataLength > data_remain)
                        {
                            /* Ignore partial block check as we are handling clear data */
                            blockDataLength = data_remain;
                        }
                        block_encrypted = false;
                        process_skip_flag = false; /* Signal to process encrypt portion on next iteration */
                    }
                }
            }
        }
        else
        {
            block_encrypted = false;
            blockDataLength = data_remain;
        }

        block_out_sz = 0;
        block_subsample_flags = 0;

        /* Set sample flags if applicable to first pattern block and last pattern block */
        if (subsample_flags & WV_OEMCRYPTO_FIRST_SUBSAMPLE)
        {
            if (data_processed == 0)
            {
                block_subsample_flags |= WV_OEMCRYPTO_FIRST_SUBSAMPLE;
            }
        }

        if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
        {
            if (data_processed + blockDataLength >= data_length)
            {
                block_subsample_flags |= WV_OEMCRYPTO_LAST_SUBSAMPLE;
            }
        }

        rc = drm_WVOemCrypto_P_DecryptPatternBlock(session, src_ptr, blockDataLength,
            block_encrypted, buffer_type, block_iv, block_offset, dst_ptr, &block_out_sz,
            block_subsample_flags, wvRc);

        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s: Failed to decrypt subsample subpattern block", __FUNCTION__));
            goto ErrorExit;
        }

        *out_sz += block_out_sz;

        /* Handle IV update for mid subsample transfers */
        if (block_encrypted && pPattern->skip > 0)
        {
            if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
            {
                BKNI_Memcpy(block_iv, src_ptr + blockDataLength - WVCDM_KEY_IV_SIZE, WVCDM_KEY_IV_SIZE);
            }
            else
            {
                ctr128_inc64(pPattern->encrypt, block_iv);
            }
        }

        /* Update pattern trackers */
        data_processed += blockDataLength;
        src_ptr += blockDataLength;
        dst_ptr += blockDataLength;
    }

ErrorExit:
    BDBG_LEAVE(drm_WVOemCrypto_DecryptCENC);

    return rc;
}

/********************************************************************************************************
Description:
Decrypts a wrapped keybox and installs it in the security processor

Parameters:
[in] keybox pointer
to encrypted Keybox data as input
[in] keyboxLength length
of the keybox data in bytes

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_BAD_MAGIC
OEMCrypto_ERROR_BAD_CRC
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES

Threading
This function is not called simultaneously with any other functions

*********************************************************************************************************/
DrmRC drm_WVOemCrypto_InstallKeybox(const uint8_t* keybox,
                                        uint32_t keyBoxLength)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_InstallKeybox);

    if(keybox == NULL)
    {
        BDBG_ERR(("%s - keybox buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(keyBoxLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(keyBoxLength, SRAI_MemoryType_Shared);
        container->blocks[0].len = keyBoxLength;
        BKNI_Memcpy(container->blocks[0].data.ptr, keybox, keyBoxLength);
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = keyBoxLength;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eInstallKeybox, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Installing key box", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to install keybox but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
        rc = Drm_Err;
        goto ErrorExit;
    }

  ErrorExit:

     if(container != NULL)
     {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
     }


    BDBG_LEAVE(drm_WVOemCrypto_InstallKeybox);

   return rc;
}

/********************************************************************************************************
Description:
Validates the Widevine Keybox loaded into the security processor device

Parameters:
none

Returns:
OEMCrypto_SUCCESS
OEMCrypto_ERROR_BAD_MAGIC
OEMCrypto_ERROR_BAD_CRC

Threading:
This function may be called simultaneously with any session functions.
***********************************************************************************************************/
DrmRC drm_WVOemCrypto_IsKeyboxValid(int *wvRc)
{

    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_IsKeyboxValid);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eIsKeyboxValid, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error validating key box in sage. sage command failed", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s:Command was sent successfully to validate keybox but actual operation failed (0x%08x), wvRc = %d",
                            __FUNCTION__, container->basicOut[0], *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }


ErrorExit:
    if(container != NULL){
        SRAI_Container_Free(container);
        container = NULL;
    }

    if (*wvRc!= SAGE_OEMCrypto_SUCCESS){
        rc =  Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_IsKeyboxValid);

    return rc;
}


/*********************************************************************************************************************
Description:
Retrieve DeviceID from the Keybox.

Parameters:
[out] deviceId pointer
to the buffer that receives the Device ID
[in/out] idLength – on input, size of the caller’s device ID buffer. On output, the number of bytes
written into the buffer.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_SHORT_BUFFER if the buffer is too small to return device ID
OEMCrypto_ERROR_NO_DEVICEID failed to return Device Id

Threading:
This function may be called simultaneously with any session functions.

**********************************************************************************************************************/

DrmRC drm_WVOemCrypto_GetDeviceID(uint8_t* deviceID,
                                  size_t* idLength,
                                  int* wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_GetDeviceID);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(deviceID == NULL)
    {
        BDBG_MSG(("%s - deviceID buffer is NULL", __FUNCTION__));
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(deviceID == NULL)
    {
        container->blocks[0].data.ptr =NULL;
        container->blocks[0].len = 0;
    }
    else
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(SAGE_WVKBOX_DEVID_LEN, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for device ID (%u bytes)", __FUNCTION__, SAGE_WVKBOX_DEVID_LEN));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = *idLength;

    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGetDeviceID, container);
    *wvRc=container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Getting DeviceID with wvRC=%d", __FUNCTION__,*wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }


    *idLength = container->basicOut[1];
    BDBG_MSG(("%s: idlength=%d",__FUNCTION__,*idLength));


    if (*wvRc!= SAGE_OEMCrypto_SUCCESS)
    {
        rc =  Drm_Err;
        goto ErrorExit;
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to GetDeviceID but actual operation failed (0x%08x)", __FUNCTION__, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    else
    {
        if(deviceID != NULL)
        {
            BKNI_Memcpy( deviceID, container->blocks[0].data.ptr, *idLength);
            BDBG_MSG(("%s: Device is %s",__FUNCTION__,deviceID ));
        }
        else
        {
            BDBG_WRN(("%s:deviceID buffer is NULL!!!",__FUNCTION__));
        }
    }

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_MSG(("%s: Exiting funciton",__FUNCTION__));
    BDBG_LEAVE(drm_WVOemCrypto_GetDeviceID);

    return rc;
}



/********************************************************************************************************************
Description:
Decrypt and return the Key Data field from the Keybox.

Parameters:
[out] keyData pointer
to the buffer to hold the Key Data field from the Keybox
[in/out] keyDataLength – on input, the allocated buffer size. On output, the number of bytes in
Key Data

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_SHORT_BUFFER if the buffer is too small to return KeyData
OEMCrypto_ERROR_NO_KEYDATA

Threading:
This function may be called simultaneously with any session functions.
*********************************************************************************************************************/
DrmRC drm_WVOemCrypto_GetKeyData(uint8_t* keyData,
                                 size_t* keyDataLength,
                                 int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_GetKeyData);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(keyData == NULL)
    {
        BDBG_ERR(("%s - keydata  is NULL ", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(keyDataLength == NULL)
    {
        BDBG_ERR(("%s - keyDataLength is NULL ", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container ", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(*keyDataLength != 0)
    {
        BDBG_MSG(("----------------%s: keyDataLength is %d",__FUNCTION__,*keyDataLength));
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(*keyDataLength, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data (%u bytes)", __FUNCTION__, *keyDataLength));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = *keyDataLength;
        BKNI_Memcpy(container->blocks[0].data.ptr, keyData, *keyDataLength);
    }
    else
    {
        BDBG_MSG(("%s: ------------Input keydata len is zero. lets get the needed size and set short buffer error",__FUNCTION__));
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(SAGE_WVKBOX_KEYDATA_LEN, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data buffer (%u bytes)", __FUNCTION__, SAGE_WVKBOX_KEYDATA_LEN));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = SAGE_WVKBOX_KEYDATA_LEN;
    }



    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGetKeyData, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Getting key data", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:SRAI_Module_ProcessCommand completed ",__FUNCTION__));

    /* if success, extract status from container */


    *wvRc = container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to GetKeyData but actual operation failed (0x%08x), wvRc=%d", __FUNCTION__, container->basicOut[0],*wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*keyDataLength < (uint32_t)container->basicOut[1])
    {
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }
    else
    {
        *keyDataLength = container->basicOut[1];
        BDBG_MSG(("%s:copying %d bytes of keydata to host memory",__FUNCTION__,*keyDataLength));
        BKNI_Memcpy(keyData,container->blocks[0].data.ptr,*keyDataLength);
    }

ErrorExit:

     if(container != NULL)
     {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }


    BDBG_LEAVE(drm_WVOemCrypto_GetDeviceID);

   return rc;
}


/**********************************************************************************************************************
Description:
Returns a buffer filled with hardware generated random bytes.

Parameters:
[out] randomData pointer
to the buffer that receives random data
[in] dataLength length
of the random data buffer in bytes

Returns :
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_RNG_FAILED failed to generate random number
OEMCrypto_ERROR_RNG_NOT_SUPPORTED function not supported

Threading:
This function may be called simultaneously with any session functions.
***********************************************************************************************************************/
DrmRC drm_WVOemCrypto_GetRandom(uint8_t* randomData, uint32_t dataLength)
{
    DrmRC rc = Drm_Success;
    uint32_t tempSz=0;
    uint32_t ii = 0;
    BDBG_ENTER(drm_WVOemCrypto_GetRandom);

    if(dataLength >NEXUS_MAX_RANDOM_NUMBER_LENGTH)
    {
        BDBG_MSG(("%s - size greater than the HW limit, calling multiple times", __FUNCTION__));
        tempSz = NEXUS_MAX_RANDOM_NUMBER_LENGTH;
    }
    else
    {
        tempSz=dataLength;
    }

    for(ii=0; ii < dataLength; ii += tempSz)
    {
        if((dataLength-ii) < NEXUS_MAX_RANDOM_NUMBER_LENGTH)
        {
            tempSz=dataLength-ii;
            /*BDBG_MSG(("%s - tempSz is %d", __FUNCTION__, tempSz));*/
        }

        /*BDBG_MSG(("%s - sz is %d,random buffer is at 0x%x,  i:%d", __FUNCTION__, tempSz, randomData+ii,ii));*/
        if(tempSz % 4 !=0)
        {
            rc = DRM_Common_GenerateRandomNumber(tempSz+(4 -(tempSz%4)), randomData + ii);
        }
        else
        {
            rc = DRM_Common_GenerateRandomNumber(tempSz,randomData + ii);
        }

        if(rc!=Drm_Success){
            goto ErrorExit;
        }
    }


ErrorExit:

    return rc;

}


/************************************************************************************************************************
Description:
OEMCrypto_WrapKeybox() is used to generate an OEMencrypted keybox that may be passed to OEMCrypto_InstallKeybox() for
provisioning

Parameters:
[in] keybox pointer
to Keybox data to encrypt. May be NULL on the first call to test size of wrapped keybox. The keybox may either be clear or previously encrypted.
[in] keyboxLength length
the keybox data in bytes
[out] wrappedKeybox – Pointer to wrapped keybox
[out] wrappedKeyboxLength – Pointer to the length of the wrapped keybox in bytes
[in] transportKey – Optional. AES transport key. If provided, the keybox parameter was
previously encrypted with this key. The keybox will be decrypted with the transport key using
AESCBC and a null IV.
[in] transportKeyLength – Optional. Number of bytes in the transportKey, if used.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_WRITE_KEYBOX failed to encrypt the keybox
OEMCrypto_ERROR_SHORT_BUFFER if keybox is provided as NULL, to determine the size of
the wrapped keybox
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_NOT_IMPLEMENTED

Threading:
This function is not called simultaneously with any other functions
*************************************************************************************************************************/
DrmRC drm_WVOemCrypto_WrapKeybox(const uint8_t* keybox,
                                 uint32_t keyBoxLength,
                                 uint8_t* wrappedKeybox,
                                 uint32_t* wrappedKeyBoxLength,
                                 const uint8_t* transportKey,
                                 uint32_t transportKeyLength)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_WrapKeybox);

    if(keybox == NULL)
    {
        BDBG_ERR(("%s - keybox buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(wrappedKeybox!=NULL)
    {
        BDBG_ERR(("%s - wrappedKeybox buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(transportKey == NULL)
    {
        BDBG_ERR(("%s - transportKey is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(keyBoxLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(keyBoxLength, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data buffer (%u bytes)", __FUNCTION__, SAGE_WVKBOX_KEYDATA_LEN));
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = keyBoxLength;
        BKNI_Memcpy(container->blocks[0].data.ptr, keybox, keyBoxLength);
    }

    if(*wrappedKeyBoxLength !=0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate( *wrappedKeyBoxLength, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for wrapped keybox (%u bytes)", __FUNCTION__, *wrappedKeyBoxLength));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,wrappedKeybox, *wrappedKeyBoxLength);
        container->blocks[1].len =  *wrappedKeyBoxLength ;
    }

    if(transportKeyLength!=0)
    {
        /* allocate for output buffer*/

        container->blocks[2].data.ptr = SRAI_Memory_Allocate(transportKeyLength, SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for transport key buffer (%u bytes)", __FUNCTION__, transportKeyLength));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[2].data.ptr,transportKey, transportKeyLength);
        container->blocks[2].len = transportKeyLength;
    }



    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eWrapKeybox, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling wrap key box", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to WrapKeybox but actual operation failed (0x%08x)", __FUNCTION__, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /*copy the wrapped keybox back to the app buffer*/
    BKNI_Memcpy(wrappedKeybox, container->blocks[2].data.ptr, *wrappedKeyBoxLength);

ErrorExit:

  if(container != NULL)
  {
      if(container->blocks[0].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[0].data.ptr);
          container->blocks[0].data.ptr = NULL;
      }

      if(container->blocks[1].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[1].data.ptr);
          container->blocks[1].data.ptr = NULL;
      }

      if(container->blocks[2].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[2].data.ptr);
          container->blocks[2].data.ptr = NULL;
      }

      SRAI_Container_Free(container);
      container = NULL;
  }

    BDBG_LEAVE(drm_WVOemCrypto_WrapKeybox);

   return rc;
}

/**************************************************************************************************************
Description:
Reencrypt the device RSA key with an internal key (such as the OEM key or Widevine
Keybox key) and the generated IV using AES128CBC with PKCS#5 padding.
Copy the rewrapped key to the buffer specified by wrapped_rsa_key and the size of the
wrapped key to wrapped_rsa_key_length.

Parameters:
[in] session: crypto session identifier.
[in] message: pointer to memory containing message to be verified.
[in] message_length: length of the message, in bytes.
[in] signature: pointer to memory containing the HMACSHA256
signature for message, received
from the provisioning server.
[in] signature_length: length of the signature, in bytes.
[in] nonce: A pointer to the nonce provided in the provisioning response.
[in] enc_rsa_key: Encrypted device private RSA key received from the provisioning server.
Format is PKCS#8, binary DER encoded, and encrypted with the derived encryption key, using
AES128CBC
with PKCS#5 padding.
[in] enc_rsa_key_length: length of the encrypted RSA key, in bytes.
[in] enc_rsa_key_iv: IV for decrypting RSA key. Size is 128 bits.
[out] wrapped_rsa_key: pointer to buffer in which encrypted RSA key should be stored. May be
null on the first call in order to find required buffer size.
[in/out] wrapped_rsa_key_length: (in) length of the encrypted RSA key, in bytes.
(out) actual length of the encrypted RSA key

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_RSA_KEY
OEMCrypto_ERROR_SIGNATURE_FAILURE
OEMCrypto_ERROR_INVALID_NONCE
OEMCrypto_ERROR_BUFFER_TO_SMALL
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
*******************************************************************************************************************************/
DrmRC drm_WVOemCrypto_RewrapDeviceRSAKey(uint32_t session,
                                             const uint8_t* message,
                                             uint32_t message_length,
                                             const uint8_t* signature,
                                             uint32_t signature_length,
                                             const uint32_t* nonce,
                                             const uint8_t* enc_rsa_key,
                                             uint32_t enc_rsa_key_length,
                                             const uint8_t* enc_rsa_key_iv,
                                             uint8_t* wrapped_rsa_key,
                                             size_t*  wrapped_rsa_key_length,
                                             int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_RewrapDeviceRSAKey);

    BDBG_MSG(("%s:Entered : wrapped_rsa_key_length=%d, enc_rsa_key_length=%d, message_length=%d, sig_len=%d",
            __FUNCTION__,*wrapped_rsa_key_length,enc_rsa_key_length,message_length,signature_length ));

    if(message == NULL)
    {
        BDBG_ERR(("%s - message buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature ==NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(nonce == NULL)
    {
        BDBG_ERR(("%s - nonce is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(enc_rsa_key == NULL)
    {
        BDBG_ERR(("%s - enc_rsa_key buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(enc_rsa_key_iv == NULL)
    {
        BDBG_ERR(("%s - enc_rsa_key_iv buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container memory", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(message_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for message buffer (%u bytes)", __FUNCTION__, message_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len =message_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);
    }

    if(signature_length !=0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate( signature_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for signature buffer (%u bytes)", __FUNCTION__, signature_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, signature, signature_length);
        container->blocks[1].len = signature_length ;
    }

    if(enc_rsa_key_length!=0)
    {
        /* allocate for output buffer*/

        container->blocks[2].data.ptr = SRAI_Memory_Allocate(enc_rsa_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", __FUNCTION__, enc_rsa_key_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[2].data.ptr,enc_rsa_key, enc_rsa_key_length);
        container->blocks[2].len = enc_rsa_key_length;
    }


    /*pass iv*/
    container->blocks[3].data.ptr = SRAI_Memory_Allocate(16, SRAI_MemoryType_Shared);
    if(container->blocks[3].data.ptr == NULL)
    {
        BDBG_ERR(("%s: Out of memory for rsa iv buffer (16 bytes)", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[3].data.ptr, enc_rsa_key_iv, 16);
    container->blocks[3].len = 16;

    if(wrapped_rsa_key == NULL)
    {
        BDBG_WRN(("%s - wrapped_rsa_key is null (SHORT_BUFFER)", __FUNCTION__));
        container->blocks[4].data.ptr =NULL;
        container->blocks[4].len = 0;
    }
    else
    {
        BDBG_MSG(("%s - wrapped_rsa_key is not null, wrapped_rsa_key_length is %d ", __FUNCTION__, *wrapped_rsa_key_length));
        container->blocks[4].data.ptr = SRAI_Memory_Allocate(*wrapped_rsa_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[4].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", __FUNCTION__, *wrapped_rsa_key_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[4].len = *wrapped_rsa_key_length;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = *nonce;
    BDBG_MSG(("%s: nonce=%d",__FUNCTION__,*nonce));


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eRewrapDeviceRSAKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling RewrapDeviceRSAKey", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =container->basicOut[2];
    *wrapped_rsa_key_length = container->basicOut[1];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        if(*wvRc == SAGE_OEMCrypto_ERROR_SHORT_BUFFER)
            BDBG_WRN(("%s - Command was sent succuessfully, and SHORT_BUFFER was returned", __FUNCTION__));
        else
            BDBG_ERR(("%s - Command was sent succuessfully to RewrapDeviceRSAKey but actual operation failed (0x%08x), wvRC=%d", __FUNCTION__, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

     BDBG_MSG(("%s: sage returned wrapped key length as %d",__FUNCTION__,container->basicOut[1]));

    /*copy the wrapped keybox back to the app buffer*/
     if(wrapped_rsa_key!=NULL){
        BKNI_Memcpy(wrapped_rsa_key, container->blocks[4].data.ptr, *wrapped_rsa_key_length);
     }

     if(*wvRc!=0)
     {
        BDBG_ERR(("%s:rewrap Device RSA key sage cmnd returned error rc =%d",__FUNCTION__,*wvRc));
         rc = Drm_Err;
     }

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[0].data.ptr);
          container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[1].data.ptr);
          container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[2].data.ptr);
          container->blocks[2].data.ptr = NULL;
        }

        if(container->blocks[3].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[3].data.ptr);
          container->blocks[3].data.ptr = NULL;
        }

        if(container->blocks[4].data.ptr != NULL){
          SRAI_Memory_Free(container->blocks[4].data.ptr);
          container->blocks[4].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }


    BDBG_LEAVE(drm_WVOemCrypto_RewrapDeviceRSAKey);

    return rc;
}



/**************************************************************************************************************************************
Description:
Loads a wrapped RSA private key to secure memory for use by this session in future calls to
OEMCrypto_GenerateRSASignature

Parameters:
[in] session: crypto session identifier.
[in] wrapped_rsa_key: wrapped device RSA key stored on the device. Format is PKCS#8, binary
DER encoded, and encrypted with a key internal to the OEMCrypto instance, using
AES128CBC
with PKCS#5 padding. This is the wrapped key generated by
OEMCrypto_RewrapDeviceRSAKey.
[in] wrapped_rsa_key_length: length of the wrapped key buffer, in bytes.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_RSA_KEY
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.

***************************************************************************************************************************************/
DrmRC drm_WVOemCrypto_LoadDeviceRSAKey(uint32_t session,
                                           const uint8_t* wrapped_rsa_key,
                                           uint32_t wrapped_rsa_key_length,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_LoadDeviceRSAKey);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(wrapped_rsa_key_length!=0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(wrapped_rsa_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", __FUNCTION__, wrapped_rsa_key_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr,wrapped_rsa_key, wrapped_rsa_key_length);
        container->blocks[0].len = wrapped_rsa_key_length;
    }

    container->basicIn[0] = session;
    container->basicIn[1] = wrapped_rsa_key_length;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadDeviceRSAKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    *wvRc =  container->basicOut[2];

    if(container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command sent to SAGE but actual operation failed (0x%08x), wvRc = %d", __FUNCTION__, container->basicOut[0], container->basicOut[2]));
    }

ErrorExit:
    if(*wvRc !=0)
    {
        rc = Drm_Err;
    }

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
          SRAI_Memory_Free(container->blocks[0].data.ptr);
          container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_LoadDeviceRSAKey);

    return rc;
}


/**********************************************************************************************************************
Description:
sign messages using the device private RSA key

Parameters:
[in] session: crypto session identifier.
[in] message: pointer to memory containing message to be signed.
[in] message_length: length of the message, in bytes.
[out] signature: buffer to hold the message signature. On return, it will contain the message
signature generated with the device private RSA key using RSASSAPSS.
Will be null on the
first call in order to find required buffer size.
[in/out] signature_length: (in) length of the signature buffer, in bytes.
(out) actual length of the signature

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_BUFFER_TOO_SMALL if the signature buffer is too small.
OEMCrypto_ERROR_INVALID_RSA_KEY
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
***********************************************************************************************************************/
DrmRC drm_WVOemCrypto_GenerateRSASignature(uint32_t session,
                                               const uint8_t* message,
                                               uint32_t message_length,
                                               uint8_t* signature,
                                               size_t* signature_length,
                                               WvOemCryptoRSA_Padding_Scheme padding_scheme,
                                               int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_GenerateSignature);

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer is NULL or message length is invalid (%u)", __FUNCTION__, message_length));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate shared memory buffers */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s: Out of memory for message buffer (%u bytes)", __FUNCTION__, message_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    if(signature == NULL)
    {
        container->blocks[1].data.ptr = NULL;
        container->blocks[1].len = 0 ;
    }
    else
    {
        if(*signature_length != 0)
        {
            container->blocks[1].data.ptr = SRAI_Memory_Allocate(*signature_length, SRAI_MemoryType_Shared);
            if(container->blocks[1].data.ptr == NULL)
            {
                BDBG_ERR(("%s: Out of memory for signature buffer (%u bytes)", __FUNCTION__, *signature_length));
                *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                rc = Drm_Err;
                goto ErrorExit;
            }
            container->blocks[1].len = *signature_length ;
        }
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = padding_scheme;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateRSASignature, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling GenerateRSASignature", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *signature_length = container->basicOut[1] ;
    *wvRc =  container->basicOut[2];

    BDBG_MSG(("%s:return signature size is '%d' (wvRc = 0x%08x) ",__FUNCTION__,*signature_length, (*wvRc)));

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        if (*wvRc == SAGE_OEMCrypto_ERROR_SHORT_BUFFER)
        {
            BDBG_WRN(("%s - Command was sent succuessfully, and SHORT_BUFFER was returned", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s - Command was sent succuessfully to generate rsa signature but actual operation failed (0x%08x), wvRc = %d", __FUNCTION__, container->basicOut[0], container->basicOut[2]));
        }
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc == 0)
    {
        if(signature != NULL)
        {
           /*the signature is updated in the data->ptr of block 1.copy back the generated signature*/
            BKNI_Memcpy(signature,container->blocks[1].data.ptr, *signature_length);
            dump( signature,*signature_length,"RSA signature");
        }
    }
    else
    {
        rc =Drm_Err;
    }

ErrorExit:
    if(container !=NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_GenerateSignature);

    return rc;
}

/******************************************************************************************************************************************
Description:
Generates three secondary keys, mac_key[server], mac_key[client] and encrypt_key

Parameters:
[in] session: handle for the session to be used.
[in] enc_session_key: session key, encrypted with the device RSA key (from the device
certifcate) using RSAOAEP.
n_key_l[in] enc_sessioength: length of session_key, in bytes.
[in] mac_key_context: pointer to memory containing context data for computing the HMAC
generation key.
[in] mac_key_context_length: length of the HMAC key context data, in bytes.
[in] enc_key_context: pointer to memory containing context data for computing the encryption
key.
[in] enc_key_context_length: length of the encryption key context data, in bytes.
Results
mac_key[server]: the 256 bit mac key is generated and stored in secure memory.
mac_key[client]: the 256 bit mac key is generated and stored in secure memory.
enc_key: the 128 bit encryption key is generated and stored in secure memory.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_DEVICE_NOT_RSA_PROVISIONED
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
www.
*****************************************************************************************************************************************/

DrmRC drm_WVOemCrypto_DeriveKeysFromSessionKey(
                                    uint32_t session,
                                    const uint8_t* enc_session_key,
                                    uint32_t enc_session_key_length,
                                    const uint8_t* mac_key_context,
                                    uint32_t mac_key_context_length,
                                    const uint8_t* enc_key_context,
                                    uint32_t enc_key_context_length,
                                    int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_DeriveKeysFromSessionKey);

    if(enc_session_key == NULL)
    {
        BDBG_ERR(("%s - enc_session_key buffer is NULL", __FUNCTION__));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(mac_key_context == NULL)
    {
        BDBG_ERR(("%s - mac_key_context buffer is NULL", __FUNCTION__));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if( enc_key_context == NULL)
    {
        BDBG_ERR(("%s -  enc_key_context is NULL", __FUNCTION__));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(enc_session_key_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(enc_session_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted session key", __FUNCTION__));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len =enc_session_key_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, enc_session_key, enc_session_key_length);
    }

    if(mac_key_context_length != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(mac_key_context_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for mac key context", __FUNCTION__));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,mac_key_context, mac_key_context_length);
        container->blocks[1].len = mac_key_context_length ;
    }

    if(enc_key_context_length != 0)
    {
        container->blocks[2].data.ptr = SRAI_Memory_Allocate( enc_key_context_length, SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for encrypted mac key context (%u bytes)", __FUNCTION__, enc_key_context_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[2].len = enc_key_context_length;
        BKNI_Memcpy(container->blocks[2].data.ptr,enc_key_context,  enc_key_context_length);
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = enc_session_key_length;
    container->basicIn[2] = mac_key_context_length;
    container->basicIn[3] = enc_key_context_length;


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDeriveKeysFromSessionKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling Derive keys from Session", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc =  container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x),wvRC = %d", __FUNCTION__, container->basicOut[0], *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_DeriveKeysFromSessionKey);

    return rc;
}

DrmRC drm_WVOemCrypto_GetHDCPCapability(uint32_t *current, uint32_t *maximum, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    if((current == NULL) || (maximum == NULL) || (wvRc == NULL))
    {
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container.", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGetHDCPCapability, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error processing SAGE command.", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error in SAGE command. (0x%08x), wvRC = %d", __FUNCTION__, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(*wvRc != 0)
    {
        BDBG_ERR(("%s - WV Error in SAGE command (0x%08x).", __FUNCTION__, *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *current = (uint32_t)container->basicOut[1];
    *maximum = (uint32_t)container->basicOut[3];

    BDBG_MSG(("%s - current: %d, max: %d.", __FUNCTION__, *current, *maximum));

ErrorExit:
    if(container!=NULL)
    {
        SRAI_Container_Free(container);
    }
    return rc;
}


/*********************************************************************************************************************************
decription:
This function encrypts a generic buffer of data using the current key.

Parameters:
[in] session: crypto session identifier.
[in] in_buffer: pointer to memory containing data to be encrypted.
[in] buffer_length: length of the buffer, in bytes. The algorithm may restrict buffer_length to be a
multiple of block size.
[in] iv: IV for encrypting data. Size is 128 bits.
[in] algorithm: Specifies which encryption algorithm to use.
[out] out_buffer: pointer to buffer in which encrypted data should be stored.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.

************************************************************************************************************************************/
DrmRC drm_WVOemCrypto_Generic_Encrypt(uint32_t session,
                                          const uint8_t* in_buffer,
                                          uint32_t buffer_length,
                                          const uint8_t* iv,
                                          Drm_WVOemCryptoAlgorithm algorithm,
                                          uint8_t* out_buffer,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container =NULL;

    BDBG_ENTER(drm_WVOemCrypto_Generic_Encrypt);

    if(in_buffer == NULL)
    {
        BDBG_ERR(("%s - in_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if( iv == NULL)
    {
        BDBG_ERR(("%s -  iv is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if( buffer_length % AES_BLOCK_SIZE != 0 )
    {
        BDBG_MSG(("%s: buffers size bad.",__FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(buffer_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for buffer (%u bytes)", __FUNCTION__, buffer_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = buffer_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, in_buffer, buffer_length);
    }

    /*iv */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate( WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key/iv (%u bytes)", __FUNCTION__, WVCDM_KEY_IV_SIZE));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[1].data.ptr,iv,  WVCDM_KEY_IV_SIZE);
    container->blocks[1].len =  WVCDM_KEY_IV_SIZE ;

    /*out buffer*/
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for large buffer (%u bytes)", __FUNCTION__, buffer_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[2].len = buffer_length;
    BKNI_Memcpy(container->blocks[2].data.ptr, out_buffer,  buffer_length);

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] =  0;

    switch(algorithm)
    {
        case 0: /*aes cbc no padding OEMCrypto_AES_CBC_128_NO_PADDING*/
            container->basicIn[2] = Drm_WVOemCryptoAlgorithm_AES_CBC_128_NO_PADDING;
            break;
        case 1: /*OEMCrypto_HMAC_SHA256: HMAC SHA is not valid for encrypt operation hence send err*/
            default:
            BDBG_MSG(("%s: algo not supported",__FUNCTION__));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Encrypt, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling Generic_Encrypt", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =  container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to Generic_Encrypt but actual operation failed (0x%08x), wvRC = %d", __FUNCTION__, container->basicOut[0],container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc!=0)
    {
        rc =Drm_Err;
        goto ErrorExit;
    }
    /*extract the outBuffer*/
    BKNI_Memcpy(out_buffer, container->blocks[2].data.ptr, buffer_length);

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_Generic_Encrypt);

   return rc;

}


/******************************************************************************************************************
Description:
This function decrypts a generic buffer of data using the current key.

Parameters:
[in] session: crypto session identifier.
[in] in_buffer: pointer to memory containing data to be encrypted.
[in] buffer_length: length of the buffer, in bytes. The algorithm may restrict buffer_length to be a
multiple of block size.
[in] iv: IV for encrypting data. Size is 128 bits.
[in] algorithm: Specifies which encryption algorithm to use.
[out] out_buffer: pointer to buffer in which decrypted data should be stored.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_DECRYPT_FAILED
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.

********************************************************************************************************************/

DrmRC drm_WVOemCrypto_Generic_Decrypt(uint32_t session,
                                          const uint8_t* in_buffer,
                                          uint32_t buffer_length,
                                          const uint8_t* iv,
                                          Drm_WVOemCryptoAlgorithm algorithm,
                                          uint8_t* out_buffer,
                                          int* wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    int num_blocks = 3;
    BSAGElib_InOutContainer *container = NULL;
    int j = 0;

    BDBG_ENTER(drm_WVOemCrypto_Generic_Decrypt);

    if(in_buffer == NULL)
    {
        BDBG_ERR(("%s - in_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if( iv == NULL)
    {
        BDBG_ERR(("%s -  iv is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if( buffer_length % AES_BLOCK_SIZE != 0 )
    {
        BDBG_MSG(("%s: buffers size bad.",__FUNCTION__));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        BDBG_MSG(("%s: buffers size bad.",__FUNCTION__));
        goto ErrorExit1;
    }

    switch(algorithm)
    {

        case 0: /*aes cbc no padding OEMCrypto_AES_CBC_128_NO_PADDING*/
            break;
        case 1: /*OEMCrypto_HMAC_SHA256: HMAC SHA is not valid for encrypt operation hence send err*/
        default:
            BDBG_MSG(("%s: algo not supported",__FUNCTION__));
            rc = Drm_Err;
            *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit1;
    }



    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(buffer_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
        container->blocks[0].len = buffer_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, in_buffer, buffer_length);
    }

    /*iv */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate( WVCDM_KEY_IV_SIZE, SRAI_MemoryType_Shared);
    BKNI_Memcpy(container->blocks[1].data.ptr,iv,  WVCDM_KEY_IV_SIZE);
    container->blocks[1].len =  WVCDM_KEY_IV_SIZE ;

    /*out buffer*/

    container->blocks[2].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
    container->blocks[2].len = buffer_length;
    BKNI_Memcpy(container->blocks[2].data.ptr,out_buffer, buffer_length);

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] =  1; /*1 for decrypt */
    container->basicIn[2] = Drm_WVOemCryptoAlgorithm_AES_CBC_128_NO_PADDING;


    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Decrypt, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error processing Generic_decrypt command", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:extract status from container",__FUNCTION__));
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    BDBG_MSG(("%s:extract WVRC status from container",__FUNCTION__));
    *wvRc =  container->basicOut[2];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x), wvRC = %d", __FUNCTION__, sage_rc,container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(*wvRc!=0)
    {
        rc =Drm_Err;
        goto ErrorExit;
    }
    BDBG_MSG(("%s:extract outbuffer from container",__FUNCTION__));
    /*extract the outBuffer*/
    BKNI_Memcpy(out_buffer, container->blocks[2].data.ptr, buffer_length);

ErrorExit:
    BDBG_MSG(("%s:free container blks",__FUNCTION__));
    if(container != NULL)
    {
        for(j=0;j<num_blocks;j++)
        {
            if(container->blocks[j].data.ptr != NULL)
            {
                SRAI_Memory_Free(container->blocks[j].data.ptr);
                container->blocks[j].data.ptr = NULL;
            }
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_MSG(("%s:free container ",__FUNCTION__));

ErrorExit1:
    BDBG_MSG(("%s:at ErrorExit1 ...",__FUNCTION__));
    BDBG_LEAVE(drm_WVOemCrypto_Generic_Decrypt);

    return rc;
}



/***********************************************************************************************************************************
Description:
This function signs a generic buffer of data using the current key.

Parameters:
[in] session: crypto session identifier.
[in] in_buffer: pointer to memory containing data to be encrypted.
[in] buffer_length: length of the buffer, in bytes.
[in] algorithm: Specifies which algorithm to use.
[out] signature: pointer to buffer in which signature should be stored. May be null on the first call
in order to find required buffer size.
[in/out] signature_length: (in) length of the signature buffer, in bytes.
(out) actual length of the signature

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_SHORT_BUFFER if signature buffer is not large enough to hold the output
signature.
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
************************************************************************************************************************************/\
DrmRC drm_WVOemCrypto_Generic_Sign(uint32_t session,
                                       const uint8_t* in_buffer,
                                       uint32_t buffer_length,
                                       int algorithm,
                                       uint8_t* signature,
                                       size_t* signature_length,
                                       int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    int num_blocks = 2;
    int j=0;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_Generic_Sign);

    if(in_buffer == NULL)
    {
        BDBG_ERR(("%s - in_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    switch(algorithm)
    {
    case 1:
        break;
    case 0:
    default:
        BDBG_MSG(("%s:Bad algo!!!!!!!!!!!!!!!",__FUNCTION__));
        rc =Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit1;

    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }


    /* allocate buffers accessible by Sage*/
    if(buffer_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
        container->blocks[0].len = buffer_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, in_buffer, buffer_length);
    }

   if(signature!=NULL)
   {
       BDBG_MSG(("%s:allocating buffer for signature",__FUNCTION__));
        container->blocks[1].data.ptr = SRAI_Memory_Allocate( 32, SRAI_MemoryType_Shared);
        container->blocks[1].len = 32;
   }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = 2; /*for SAGE_Crypto_ShaVariant_eSha256*/

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Sign, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    BDBG_MSG(("%s:extract the wvrc",__FUNCTION__));
    *wvRc =  container->basicOut[2];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully generic sign but actual operation failed (0x%08x), wvRC = %d", __FUNCTION__, sage_rc, container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    else
    {
        BDBG_MSG(("%s: sage command sucessfully returned",__FUNCTION__));
    }

    BDBG_MSG(("%s:extract the signature length",__FUNCTION__));
    *signature_length = container->basicOut[1] ;

    BDBG_MSG(("%s:print the returned signature length",__FUNCTION__));
    BDBG_MSG(("%s:sage returned signature length =%d",__FUNCTION__,*signature_length));

    BDBG_MSG(("%scheck wvRc",__FUNCTION__));

    if(*wvRc!=0)
    {
        rc =Drm_Err;
        goto ErrorExit;
    }

    if(signature!=NULL)
    {
        BDBG_MSG(("%s:copy signature",__FUNCTION__));
        /*extract the outBuffer*/
        BKNI_Memcpy(signature, container->blocks[1].data.ptr,  32);
    }
ErrorExit:

    if(container!=NULL)
    {
        BDBG_MSG(("%s:free containser ",__FUNCTION__));
        for(j=0;j<num_blocks;j++)
        {
            if(container->blocks[j].data.ptr != NULL){
                SRAI_Memory_Free(container->blocks[j].data.ptr);
                container->blocks[j].data.ptr = NULL;
            }
        }

        SRAI_Container_Free(container);
        container = NULL;
    }
ErrorExit1:
    BDBG_LEAVE(drm_WVOemCrypto_Generic_Sign);

   return rc;
}

/***********************************************************************************************************************************
Description:
This function verfies the signature of a generic buffer of data using the current key.

Parameters:
[in] session: crypto session identifier.
[in] in_buffer: pointer to memory containing data to be encrypted.
[in] buffer_length: length of the buffer, in bytes.
[in] algorithm: Specifies which algorithm to use.
[in] signature: pointer to buffer in which signature resides.
[in] signature_length: length of the signature buffer, in bytes.

Returns:
OEMCrypto_SUCCESS success
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_SIGNATURE_FAILURE
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
www.widevine.

************************************************************************************************************************************/
DrmRC drm_WVOemCrypto_Generic_Verify(uint32_t session,
                                         const uint8_t* in_buffer,
                                         uint32_t buffer_length,
                                         int algorithm,
                                         const uint8_t* signature,
                                         uint32_t signature_length,
                                         int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    int num_blocks=2;
    int j=0;
    BSAGElib_InOutContainer *container = NULL;

     BDBG_ENTER(drm_WVOemCrypto_Generic_Verify);

    if(in_buffer == NULL)
    {
        BDBG_ERR(("%s - in_buffer buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(buffer_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
        container->blocks[0].len = buffer_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, in_buffer, buffer_length);
    }

   if(signature_length != 0)
   {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate( signature_length, SRAI_MemoryType_Shared);
        BKNI_Memcpy(container->blocks[1].data.ptr,signature,  signature_length);
        container->blocks[1].len = signature_length;
   }

    switch(algorithm)
    {
    case 1:
        break;
    case 0:
    default:
        BDBG_MSG(("%s:Bad algo!!!!!!!!!!!!!!!",__FUNCTION__));
        rc =Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = 2; /*algorithm;*/

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Verify, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x), wvRC = %d ", __FUNCTION__, container->basicOut[0],container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc != 0)
    {
        rc = Drm_Err;
    }

ErrorExit:
     if(container != NULL)
     {
        for(j=0;j<num_blocks;j++)
        {
            if(container->blocks[j].data.ptr != NULL)
            {
                SRAI_Memory_Free(container->blocks[j].data.ptr);
                container->blocks[j].data.ptr = NULL;
            }
        }
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(drm_WVOemCrypto_Generic_Verify);

    return rc;

}

/**********************************************************************************************
 * DRM_OEMCrypto_SupportsUsageTable
 *   This is used to determine if the device can support a usage table.
 *
 * PARAMETERS:
 *     none
 *
 * RETURNS:
 *     SAGE_OEMCrypto_SUCCESS supported
 *     SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 *
 * This method changed in API version 9.
 * ***********************************************************************/
DrmRC DRM_WVOemCrypto_SupportsUsageTable(int *wvRc)
{
    BDBG_ENTER(DRM_WVOemCrypto_SupportsUsageTable);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    BDBG_LEAVE(DRM_WVOemCrypto_SupportsUsageTable);

    return Drm_Success;
}

/**********************************************************************************************
 * DRM_OEMCrypto_UpdateUsageTable
 * Propagate values from all open sessions to the Session Usage Table. If
 * any values have changed, increment the generation number, sign, and save the table. During
 * playback, this function will be called approximately once per minute.
 * Devices that do not implement a Session Usage Table may return
 * OEMCrypto_ERROR_NOT_IMPLEMENTED.
 *
 * This function will not be called simultaneously with any session functions.
 *
 * PARAMETERS:
 *     none
 *
 * RETURNS:
 *     SAGE_OEMCrypto_SUCCESS success
 *     SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 *     SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 * This method changed in API version 9.
 * ***********************************************************************/
DrmRC DRM_WVOemCrypto_UpdateUsageTable(int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_UpdateUsageTable);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[0].data.ptr = gWVUsageTable;
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", __FUNCTION__, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[0].len = MAX_USAGE_TABLE_SIZE;
    BDBG_MSG(("%s -  %p -> length = '%u'", __FUNCTION__, container->blocks[0].data.ptr, container->blocks[0].len));

    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current Epoch time = %u <<<<<<", __FUNCTION__, (unsigned)current_epoch_time));

    /* send command to SAGE */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eUpdateUsageTable, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eUpdateUsageTable));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x)", __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Possible encrypted Usage Table returned from SAGE */
    if(container->basicOut[2] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[0].data.ptr, container->blocks[0].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
            goto ErrorExit;
        }
    }

    *wvRc = SAGE_OEMCrypto_SUCCESS;

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_UpdateUsageTable);

    return Drm_Success;
}


/***************************************************************************************
 * DRM_WVOemCrypto_DeactivateUsageEntry
 *
 * Find the entry in the Usage Table with a matching PST. Mark the status of that entry as
 * "inactive". If it corresponds to an open session, the status of that session will also be marked
 * as "inactive".
 * Increment Usage Table's generation number, sign, encrypt, and save the Usage Table.
 *
 * PARAMETERS:
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT (no entry has matching PST.)
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ***************************************************************************************************/
DrmRC DRM_WVOemCrypto_DeactivateUsageEntry(uint8_t *pst,
                                           uint32_t pst_length,
                                           int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_DeactivateUsageEntry);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", __FUNCTION__, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Allocate memory for PST */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for PST (%u bytes)", __FUNCTION__, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = pst_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);

    /*
     * Assign memory for a read Usage Table
     * */
    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[1].data.ptr = gWVUsageTable;
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", __FUNCTION__, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[1].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", __FUNCTION__, current_epoch_time));

    /* send command to SAGE */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if successful look at basicOut[x]?? */
    *wvRc = container->basicOut[2];

    /* Only if entry found and deleted do we overwrite the Usage Table on Rootfs */
    if(container->basicOut[1] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[1].data.ptr, container->blocks[1].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwriting Usage Table in rootfs (%s), wvRC = %d", __FUNCTION__, strerror(errno),container->basicOut[2]));
            goto ErrorExit;
        }
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x)",
                __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }


ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }


    BDBG_LEAVE(DRM_WVOemCrypto_DeactivateUsageEntry);

    return rc;
}



/*********************************************************************************
 * DRM_WVOemCrypto_ReportUsage
 *
 * Increment Usage Table’s generation number, sign, encrypt, and save the Usage Table.
 *
 * PARAMETERS:
 * [in] sessionContext: handle for the session to be used.
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 * [out] buffer: pointer to buffer in which usage report should be stored.
 *               May be null on the first call in order to find required buffer size.
 * [in/out] buffer_length: (in) length of the report buffer, in bytes.
 * (out) actual length of the report
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_SHORT_BUFFER (buffer not large enough to hold output signature)
 * SAGE_OEMCrypto_ERROR_INVALID_SESSION (no open session with specified id)
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT (no entry has matching PST)
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ********************************************************************/
DrmRC DRM_WVOemCrypto_ReportUsage(uint32_t sessionContext,
                                  const uint8_t *pst,
                                  uint32_t pst_length,
                                  WvOEMCryptoPstReport *buffer,
                                  size_t *buffer_length,
                                  int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_ReportUsage);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", __FUNCTION__, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /* verify 'sessionContext' is valid or on SAGE side? or both? */

    /* if requesting size only then we don't need to send request to SAGE? */
    if(buffer == NULL || (*buffer_length) < sizeof(WvOEMCryptoPstReport))
    {
        BDBG_MSG(("%s - buffer is NULL or invalid input size (%u), therefore this is a size request (%u + %u)",
                __FUNCTION__, (*buffer_length), sizeof(WvOEMCryptoPstReport), pst_length));
        (*buffer_length) = sizeof(WvOEMCryptoPstReport) + pst_length;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container->basicIn[0] = sessionContext;

    /* allocate memory for PST */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for PST (%u bytes)", __FUNCTION__, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = pst_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);

    /*
     * send buffer
     * */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate((*buffer_length), SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for buffer (%u bytes)", __FUNCTION__, (*buffer_length)));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[1].len = (*buffer_length);
    BKNI_Memset(container->blocks[1].data.ptr, 0x00, (*buffer_length));

    /*
     * assign memory to return UsageTable
     * */
    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[2].data.ptr = gWVUsageTable;
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for returned Usage Table", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[2].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    container->basicIn[1] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", __FUNCTION__, current_epoch_time));

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eReportUsage, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eReportUsage));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];

    BDBG_MSG(("%s - Copying (*buffer_length) = %u   *** container->blocks[1].len (%u bytes) wvRC =  %d", __FUNCTION__, (*buffer_length), container->blocks[1].len, container->basicOut[2]));
    BKNI_Memcpy(buffer, container->blocks[1].data.ptr, container->blocks[1].len);

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x)", __FUNCTION__, DrmWVOEMCrypto_CommandId_eReportUsage, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* At this point the session should already be associated with the given entry */

    /* ALWAYS overwrite Usage Table, regardless if table changed */
    rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[2].data.ptr, container->blocks[2].len);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
        goto ErrorExit;
    }

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_ReportUsage);

    return Drm_Success;
}



/***********************************************************************************************
 * DRM_WVOemCrypto_DeleteUsageEntry
 *
 * Verifies the signature of the given message using the session’s mac_key[server]
 * and the algorithm HMACSHA256, then deletes an entry from the session table. The
 * session should already be associated with the given entry, from a previous call to
 * DRM_WVOemCrypto_ReportUsage.
 * After performing all verifications, and deleting the entry from the Usage Table,
 * the Usage Table generation number is incrememented, then signed, encrypted, and saved.
 *
 * PARAMETERS:
 * [in] sessionContext: handle for the session to be used.
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 * [in] message: pointer to memory containing message to be verified.
 * [in] message_length: length of the message, in bytes.
 * [in] signature: pointer to memory containing the signature.
 * [in] signature_length: length of the signature, in bytes.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_INVALID_SESSION no open session with that id.
 * SAGE_OEMCrypto_ERROR_SIGNATURE_FAILURE
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ******************************************************************************************/
DrmRC DRM_WVOemCrypto_DeleteUsageEntry(uint32_t sessionContext,
                                        const uint8_t* pst,
                                        uint32_t pst_length,
                                        const uint8_t *message,
                                        uint32_t message_length,
                                        const uint8_t *signature,
                                        uint32_t signature_length,
                                        int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_DeleteUsageEntry);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", __FUNCTION__, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - Message buffer is NULL (%p) or length is 0 (%u)", __FUNCTION__, message, message_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length != HMAC_SHA256_SIGNATURE_SIZE)
    {
        BDBG_ERR(("%s - Signature buffer is NULL (%p) or length is not 32 bytes (%u)", __FUNCTION__, signature, signature_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }


    /* verify 'sessionContext' is valid or on SAGE side? or both? */

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container->basicIn[0] = sessionContext;

    /*
     * allocate memory for pst and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for digest of PST (%u bytes)", __FUNCTION__, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = pst_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);

    /*
     * allocate memory for message and copy to container
     * */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for Message", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[1].len = message_length;
    BKNI_Memcpy(container->blocks[1].data.ptr, message, message_length);

    /*
     * allocate memory for signature and copy to container
     * */
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for Signature", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[2].len = signature_length;
    BKNI_Memcpy(container->blocks[2].data.ptr, signature, signature_length);

    /*
     * assign memory to return UsageTable
     * */
    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[3].data.ptr = gWVUsageTable;
    if(container->blocks[3].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for Signature", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[3].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    BDBG_MSG(("%s - current Epoch time = '%u' <<<<<<", __FUNCTION__, (unsigned)current_epoch_time));
    container->basicIn[1] = current_epoch_time;

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDeleteUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeleteUsageEntry));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x), wvRc = %d ",
                    __FUNCTION__, DrmWVOEMCrypto_CommandId_eReportUsage, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* not clear in the spec but only overwrite if entry found? or always (assuming always) */
    rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[3].data.ptr, container->blocks[3].len);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
        goto ErrorExit;
    }


ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_DeleteUsageEntry);

    return Drm_Success;
}



/*****************************************************************************************
 * DRM_WVOemCrypto_DeleteUsageTable
 *
 * This is called when the CDM system believes there are major problems or resource issues.
 * The entire table should be cleaned and a new table should be created.
 *
 * PARAMETERS:
 * none
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC DRM_WVOemCrypto_DeleteUsageTable(int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    time_t current_epoch_time = 0;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_DeleteUsageTable);


    /* remove files from rootfs */
    BDBG_MSG(("%s - Deleting tables from rootfs", __FUNCTION__));
    if(remove(USAGE_TABLE_FILE_PATH) != 0){
        BDBG_ERR(("%s - error deleting '%s'from rootfs (%s)", __FUNCTION__, USAGE_TABLE_FILE_PATH, strerror(errno)));
    }

    if(remove(USAGE_TABLE_BACKUP_FILE_PATH) != 0){
        BDBG_ERR(("%s - error deleting '%s'from rootfs (%s)", __FUNCTION__, USAGE_TABLE_BACKUP_FILE_PATH, strerror(errno)));
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /*
     * Assign memory for NEW usage table
     * */
    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[0].data.ptr = gWVUsageTable;
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", __FUNCTION__, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[0].len = MAX_USAGE_TABLE_SIZE;

    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", __FUNCTION__, current_epoch_time));

    /*
     * send command to SAGE to wipe Usage Table memory
     * */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eDeleteUsageTable, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeleteUsageTable));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x), wvRC = %d ", __FUNCTION__, DrmWVOEMCrypto_CommandId_eDeleteUsageTable, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    /* overwrite before or after analyzing basicOut[0] */
    rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[0].data.ptr, container->blocks[0].len);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error overwriting Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
        goto ErrorExit;
    }

ErrorExit:

    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_DeleteUsageTable);

    return Drm_Success;
}

static DrmRC
DRM_WvOemCrypto_P_ReadUsageTable(uint8_t *pUsageTableSharedMemory, uint32_t *pUsageTableSharedMemorySize)
{
    DrmRC rc = Drm_Success;
    FILE * fptr = NULL;
    uint32_t filesize = 0;
    uint32_t read_size = 0;
    uint8_t *usage_table_buff = NULL;
    bool bUsageTableBackupExists = false;
    char *pActiveUsageTableFilePath = NULL;
    uint8_t digest[32] = {0x00};
    bool integrity_valid = false;

    BDBG_ENTER(DRM_WvOemCrypto_P_ReadUsageTable);

    /* if module uses a bin file - read it and add it to the container */
    BDBG_MSG(("%s - Attempting to read Usage Table at filepath '%s'", __FUNCTION__, USAGE_TABLE_FILE_PATH));

    /* Verify backup file accessible */
    if(access(USAGE_TABLE_BACKUP_FILE_PATH, R_OK|W_OK) != 0)
    {
        BDBG_WRN(("%s - '%s' not detected or file is not read/writeable (errno = %s)", __FUNCTION__, USAGE_TABLE_BACKUP_FILE_PATH, strerror(errno)));
        bUsageTableBackupExists = false;
        /* Continue onwards as main usage table may still be accessible */
    }
    else
    {
        bUsageTableBackupExists = true;
    }

    if(access(USAGE_TABLE_FILE_PATH, R_OK|W_OK) != 0)
    {
        BDBG_WRN(("%s - '%s' not detected or file is not read/writeable (errno = %s)", __FUNCTION__, USAGE_TABLE_FILE_PATH, strerror(errno)));
        if(!bUsageTableBackupExists)
        {
            BDBG_WRN(("%s - Main and backup usage tables are not read/writeable (errno = %s)", __FUNCTION__, strerror(errno)));
            rc = Drm_FileErr;
            goto ErrorExit;
        }
        else
        {
            pActiveUsageTableFilePath = USAGE_TABLE_BACKUP_FILE_PATH;
        }
    }
    else
    {
        pActiveUsageTableFilePath = USAGE_TABLE_FILE_PATH;
    }

    while(!integrity_valid)
    {
        /*
         * determine file size and read
         * */
        rc = DRM_Common_P_GetFileSize(pActiveUsageTableFilePath, &filesize);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - '%s' Error determine file size of bin file", __FUNCTION__, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        if(filesize <= SHA256_DIGEST_SIZE || filesize > (MAX_USAGE_TABLE_SIZE + SHA256_DIGEST_SIZE))
        {
            BDBG_ERR(("%s - '%s' Invalid file size %u bytes", __FUNCTION__, pActiveUsageTableFilePath, filesize));
            goto UseBackup;
        }

        DRM_Common_MemoryAllocate(&usage_table_buff, filesize);
        if(usage_table_buff == NULL)
        {
            BDBG_ERR(("%s - Error allocating '%u' bytes", __FUNCTION__, filesize));
            rc = Drm_MemErr;
            goto ErrorExit;
        }

        fptr = fopen(pActiveUsageTableFilePath, "rb");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - %s Error opening drm bin file (%s)", __FUNCTION__, pActiveUsageTableFilePath, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        read_size = fread(usage_table_buff, 1, filesize, fptr);
        if(read_size != filesize)
        {
            BDBG_ERR(("%s - %s Error reading Usage Table file size (%u != %u)", __FUNCTION__, pActiveUsageTableFilePath, read_size, filesize));
            goto UseBackup;
        }

        /*
         * Verify integrity before sending to SAGE
         * */
        rc = DRM_Common_SwSha256(usage_table_buff, digest, filesize-SHA256_DIGEST_SIZE);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error calculating SHA of '%s'", __FUNCTION__, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        if(BKNI_Memcmp(digest, &usage_table_buff[filesize-SHA256_DIGEST_SIZE], SHA256_DIGEST_SIZE) != 0)
        {
            BDBG_ERR(("%s - Error comparing SHA of '%s'", __FUNCTION__, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        /* Passed all checks. Valid usage table available */
        integrity_valid = true;
        break;

UseBackup:
        if(bUsageTableBackupExists &&
            strncmp(pActiveUsageTableFilePath, USAGE_TABLE_BACKUP_FILE_PATH, strlen(USAGE_TABLE_BACKUP_FILE_PATH)) != 0)
        {
            /* Time to call for backup */
            pActiveUsageTableFilePath = USAGE_TABLE_BACKUP_FILE_PATH;

            /* reset initial conditions */
            if(usage_table_buff != NULL)
            {
                DRM_Common_MemoryFree(usage_table_buff);
                usage_table_buff = NULL;
            }

            if(fptr != NULL)
            {
                fclose(fptr);
                fptr = NULL;
            }

            filesize = 0;
            BKNI_Memset(digest, 0x00, sizeof(digest));
        }
        else
        {
            /* Could not find a valid usage table */
            BDBG_ERR(("%s - Error, unable to read a valid usage table.", __FUNCTION__));
            rc = Drm_FileErr;
            goto ErrorExit;
        }
    }

    /*
     * Valid usage table found, verify if MAX_USAGE_TABLE_SIZE will fit in detected file
     * */
    if(pUsageTableSharedMemory == NULL || (*pUsageTableSharedMemorySize) < filesize-SHA256_DIGEST_SIZE)
    {
        BDBG_ERR(("%s - Error, shared pointer memory is NULL or allocated size (%u) is not enough. File size detected is '%u' bytes",
                             __FUNCTION__, (*pUsageTableSharedMemorySize), filesize-SHA256_DIGEST_SIZE));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BKNI_Memcpy(pUsageTableSharedMemory, usage_table_buff, filesize-SHA256_DIGEST_SIZE);
        (*pUsageTableSharedMemorySize) = filesize-SHA256_DIGEST_SIZE;

ErrorExit:

    if(usage_table_buff != NULL)
    {
        DRM_Common_MemoryFree(usage_table_buff);
        usage_table_buff = NULL;
    }

    if(fptr != NULL)
    {
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(DRM_WvOemCrypto_P_ReadUsageTable);

    return rc;
}


static DrmRC DRM_WvOemCrypto_P_OverwriteUsageTable(uint8_t *pEncryptedUsageTable,
                                                       uint32_t encryptedUsageTableLength)
{
    DrmRC rc = Drm_Success;
    uint32_t write_size = 0;
    FILE * fptr = NULL;
    uint8_t digest[SHA256_DIGEST_SIZE] = {0x00};
    char *pActiveUsageTableFilePath = NULL;
    uint32_t ii =0;
    int fd;

    /* DRM_MSG_PRINT_BUF("pEncryptedUsageTable (before writing)", pEncryptedUsageTable, 144); */

    if(pEncryptedUsageTable == NULL || encryptedUsageTableLength != MAX_USAGE_TABLE_SIZE)
    {
        BDBG_ERR(("%s - pointer to encrypted Usage Table buffer is NULL or length (%u) is invalid", __FUNCTION__, encryptedUsageTableLength));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /*
     * Calculate SHA 256 diget
     * */
    rc = DRM_Common_SwSha256(pEncryptedUsageTable, digest, encryptedUsageTableLength);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calculating SHA", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* If table has not changed, do not force a sync*/
    if(BKNI_Memcmp(gWVUsageTableDigest, digest, SHA256_DIGEST_SIZE) == 0)
    {
        goto ErrorExit;
    }
    else
    {
        /* Store digest for latter comparison */
        BKNI_Memcpy(gWVUsageTableDigest, digest, SHA256_DIGEST_SIZE);
    }

    for(ii = 0; ii < 2; ii++)
    {
        if(ii == 0)
        {
            pActiveUsageTableFilePath = USAGE_TABLE_FILE_PATH;
        }
        else
        {
            pActiveUsageTableFilePath = USAGE_TABLE_BACKUP_FILE_PATH;
        }

        BDBG_MSG(("%s - Overwriting file '%s'", __FUNCTION__, pActiveUsageTableFilePath));

        fptr = fopen(pActiveUsageTableFilePath, "w+b");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening Usage Table file (%s) in 'w+b' mode.  '%s'", __FUNCTION__, pActiveUsageTableFilePath, strerror(errno)));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        /*
         * Write actual Usage Table data
         * */
        write_size = fwrite(pEncryptedUsageTable, 1, encryptedUsageTableLength, fptr);
        if(write_size != encryptedUsageTableLength)
        {
            BDBG_ERR(("%s - Error writing drm bin file size to rootfs (%u != %u)", __FUNCTION__, write_size, encryptedUsageTableLength));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        write_size = fwrite(digest, 1, SHA256_DIGEST_SIZE, fptr);
        if(write_size != SHA256_DIGEST_SIZE)
        {
            BDBG_ERR(("%s - Error appending digest to Usage Table file (%ss) (%u != %u)", __FUNCTION__, pActiveUsageTableFilePath, write_size, SHA256_DIGEST_SIZE));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        fd = fileno(fptr);
#ifdef ANDROID
        /* File system performance must be sufficient enough to sync at this point */
        fsync(fd);
#endif
        /* close file descriptor */
        if(fptr != NULL)
        {
            fclose(fptr);
            fptr = NULL;
        }
    }/* end of for */


ErrorExit:
    if(fptr != NULL)
    {
        fclose(fptr);
        fptr = NULL;
    }
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}

/*WV v10 APIs*/


DrmRC DRM_WVOemCrypto_GetNumberOfOpenSessions(uint32_t* noOfOpenSessions,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_GetNumberOfOpenSessions);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGetNumberOfOpenSessions, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent sucessfully but failed to GetNumberOfOpenSessions with err wvRC = %d", __FUNCTION__,*wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*noOfOpenSessions) = container->basicOut[1];

    BDBG_MSG(("%s: Number current open sessions = %d",__FUNCTION__,container->basicOut[1] ));


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_GetNumberOfOpenSessions);

    return rc;
}



DrmRC DRM_WVOemCrypto_GetMaxNumberOfSessions(uint32_t* noOfMaxSessions,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_GetMaxNumberOfSessions);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container in GetMaxNumberOfSessions", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eGetMaxNumberOfSessions, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - command sent successfully, but actual operation failed with err -(wvRC = %d)", __FUNCTION__,*wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }



    /* if success, extract index from container */
    (*noOfMaxSessions) = container->basicOut[1];

    BDBG_MSG(("%s: Max no. of sessions supported = %d",__FUNCTION__,container->basicOut[1] ));


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_GetMaxNumberOfSessions);

   return rc;
}

/********************************************************************************************************
Description:
copies a buffer from source to destination without encrypt/decrypt

Parameters:
[in] destination
addr to copy the buffer to
[in] data_addr length
addr to copy the buffer from

Returns:
OEMCrypto_SUCCESS success

Threading
This function is not called simultaneously with any other functions

*********************************************************************************************************/
DrmRC Drm_WVOemCrypto_CopyBuffer(uint8_t* destination,
                           const uint8_t* data_addr,
                           uint32_t data_length,
                           Drm_WVOemCryptoBufferType_t buffer_type,
                           uint32_t subsample_flags
                           )
{
    bool isSecureDecrypt = (buffer_type == Drm_WVOEMCrypto_BufferType_Secure);
    DrmRC rc = Drm_Success;

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - source buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if( data_length == 0)
    {
        BDBG_ERR(("%s copy buffer size is zero !!!!!!",__FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    dump(data_addr,data_length,"srcdata:");

    if(destination == NULL)
    {
        BDBG_ERR(("%s - destination buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }
    BDBG_MSG(("%s: Copying the buffer isSecureDecrypt %d....",__FUNCTION__, isSecureDecrypt));
    if (!isSecureDecrypt)
    {
       BKNI_Memcpy(destination,data_addr, data_length);
    }
    else
    {
        rc = drm_WVOemCrypto_P_CopyBuffer_Secure_SG(destination, data_addr,
            data_length, subsample_flags, true);
    }

ErrorExit:
    return rc;
}


DrmRC Drm_WVOemCrypto_QueryKeyControl(uint32_t session,
                                      const uint8_t* key_id,
                                      uint32_t key_id_length,
                                      uint8_t* key_control_block,
                                      uint32_t *key_control_block_length,
                                      int*wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    if(key_id == NULL)
    {
        BDBG_ERR(("%s - key_id buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if ((key_control_block_length == NULL)
          || (*key_control_block_length < WVCDM_KEY_CONTROL_SIZE))
    {
        BDBG_ERR(("%s : OEMCrypto_ERROR_SHORT_BUFFER error", __FUNCTION__));
        *key_control_block_length = WVCDM_KEY_CONTROL_SIZE;
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER ;
        goto ErrorExit;
    }

    *key_control_block_length = WVCDM_KEY_CONTROL_SIZE;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container in QuerykeyControl", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(key_id_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(key_id_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", __FUNCTION__, key_id_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = key_id_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, key_id, key_id_length);
    }

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(*key_control_block_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", __FUNCTION__, *key_control_block_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
    /* allocate memory for returning the keycontrol block*/
    container->blocks[1].len = WVCDM_KEY_CONTROL_SIZE;
    BKNI_Memset(container->blocks[1].data.ptr, 0x0, *key_control_block_length);

    container->basicIn[0] = session;
    container->basicIn[1] = *key_control_block_length;

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eQueryKeyControl, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
     *wvRc =    container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to QueryControlBlock but actual operation failed (0x%08x), wvRC = %d ", __FUNCTION__, sage_rc, container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    if(*wvRc != 0)
    {
        BDBG_ERR(("%s - failed with WvRc= (0x%08x)", __FUNCTION__, *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* copy the sage returned key control back to application memory */
    BKNI_Memcpy(key_control_block,container->blocks[1].data.ptr,*key_control_block_length);
    dump(key_control_block,*key_control_block_length,"returned key control box:");

ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(drm_WVOemCrypto_QueryKeyControl);
    return rc;


}

DrmRC DRM_WVOemCrypto_ForceDeleteUsageEntry( const uint8_t* pst,
                                        uint32_t pst_length,int*wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_ForceDeleteUsageEntry);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", __FUNCTION__, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /*
     * allocate memory for pst and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for digest of PST (%u bytes)", __FUNCTION__, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = pst_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);

    /*
     * assign memory to return UsageTable
     * */
    if(gWVUsageTable == NULL)
    {
        gWVUsageTable = SRAI_Memory_Allocate(MAX_USAGE_TABLE_SIZE, SRAI_MemoryType_Shared);
    }

    container->blocks[1].data.ptr = gWVUsageTable;
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for Signature", __FUNCTION__));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[1].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    BDBG_MSG(("%s - current Epoch time = '%lu' <<<<<<", __FUNCTION__, current_epoch_time));
    container->basicIn[0] = current_epoch_time;

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", __FUNCTION__, DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    __FUNCTION__, DrmWVOEMCrypto_CommandId_eReportUsage, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc != 0)
    {
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* not clear in the spec but only overwrite if entry found? or always (assuming always) */
    rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[1].data.ptr, container->blocks[1].len);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", __FUNCTION__, strerror(errno)));
        goto ErrorExit;
    }


ErrorExit:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
       SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_ForceDeleteUsageEntry);

    return Drm_Success;
}

DrmRC DRM_WVOemCrypto_Security_Patch_Level(uint8_t* security_patch_level,int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Security_Patch_Level);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gmoduleHandle, DrmWVOEMCrypto_CommandId_eSecurityPatchLevel, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent succuessfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    __FUNCTION__, DrmWVOEMCrypto_CommandId_eSecurityPatchLevel, container->basicOut[0], container->basicOut[2]));
        rc =Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*security_patch_level) = container->basicOut[1];

    BDBG_MSG(("%s: Security patch level = %d",__FUNCTION__,container->basicOut[1] ));


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Security_Patch_Level);

   return rc;
}
