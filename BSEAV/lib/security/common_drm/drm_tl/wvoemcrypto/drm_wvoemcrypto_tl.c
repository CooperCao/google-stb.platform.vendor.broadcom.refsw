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

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_platform_client.h"
#include "nexus_random_number.h"
#include "nexus_security.h"
#include "nexus_base_mmap.h"

#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_otpmsp.h"
#else
#include "nexus_otp_msp.h"
#include "nexus_otp_key.h"
#include "nexus_otp_msp_indexes.h"
#endif

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

#define SSD_WAIT_TIMEOUT (-1)

static uint32_t gNumSessions = 0;
static Drm_WVOemCryptoHostSessionCtx_t *gHostSessionCtx;
static Drm_WVoemCryptoKeySlot_t gKeySlotCache[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
static uint32_t gKeySlotCacheAllocated = 0;
static bool gCentralKeySlotCacheMode = false;
static bool gBigUsageTableFormat = false;
static bool gEventDrivenVerify = false;
static bool gSsdSupported = false;
static bool gAntiRollbackHw = false;

static bool gWVCasMode = false;

static uint32_t gKeySlotMaxAvail = 0;

static uint32_t gLastHeaderLen = 0;
static uint32_t gLastEntryLen = 0;

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
static SRAI_ModuleHandle gWVmoduleHandle = NULL;
static SRAI_ModuleHandle gSSDmoduleHandle = NULL;

Drm_WVOemCryptoParamSettings_t gWvOemCryptoParamSettings;

/* 16 byte padding countre mode with non-zero byteOffset */
static uint8_t *gPadding = NULL;
#define PADDING_SZ_16_BYTE  16

/* API to read usage table */
#ifdef ANDROID
#if defined(ANDROID_DATA_VENDOR_PLATFORM_SPLIT)
#define USAGE_TABLE_FILE_PATH        "/data/vendor/mediadrm/UsageTable.dat"
#define USAGE_TABLE_BACKUP_FILE_PATH "/data/vendor/mediadrm/UsageTable.bkp"
#else
#define USAGE_TABLE_FILE_PATH        "/data/mediadrm/UsageTable.dat"
#define USAGE_TABLE_BACKUP_FILE_PATH "/data/mediadrm/UsageTable.bkp"
#endif
#else
#define USAGE_TABLE_FILE_PATH        "UsageTable.dat"
#define USAGE_TABLE_BACKUP_FILE_PATH "UsageTable.bkp"
#endif

/* BTP buffer size for external iv support */
#define BTP_SIZE 188

#define MAX_SG_DMA_BLOCKS DRM_COMMON_TL_MAX_DMA_BLOCKS
#define WV_OEMCRYPTO_FIRST_SUBSAMPLE 1
#define WV_OEMCRYPTO_LAST_SUBSAMPLE 2

#define MAX_INIT_QUERY_RETRIES 20

#define WV_DECRYPT_VERIFY_INTERVAL 2
#define WV_KEY_SELECT_COUNT_MAX 58

#define WV_CENTRAL_MAX_NUM_KEY_SLOT 8

typedef struct Drm_WVOemCryptoEncDmaList
{
    DmaBlockInfo_t *dma_block_info;
    uint8_t **non_sec_src;
    uint8_t **non_sec_dst;
    uint32_t *non_sec_len;
}Drm_WVOemCryptoEncDmaList;

/* Scatter/gather definitions */
static Drm_WVOemCryptoEncDmaList *gWvEncDmaBlockInfoList;
static NEXUS_DmaJobBlockSettings *gWvClrDmaJobBlockSettingsList;

static uint32_t gWvClrNumDmaBlocks;
static BKNI_EventHandle gWVClrEvent;
static NEXUS_DmaHandle gWVClrDma;
static NEXUS_DmaJobHandle gWVClrJob;
static BKNI_MutexHandle gWVClrMutex = NULL;
static bool gWVClrQueued = false;
static BKNI_MutexHandle gWVKeySlotMutex = NULL;
static BKNI_MutexHandle gWVSelectKeyMutex = NULL;

static uint8_t *gWVUsageTable;
static uint8_t gWVUsageTableDigest[SHA256_DIGEST_SIZE] = {0x00};

static DrmRC DRM_WvOemCrypto_P_ReadUsageTable(uint8_t *pUsageTableSharedMemory,
                                              uint32_t *pUsageTableSharedMemorySize);
static DrmRC DRM_WvOemCrypto_P_OverwriteUsageTable(uint8_t *pEncryptedUsageTable,
                                                   uint32_t encryptedUsageTableLength);

static void DRM_WVOemcrypto_P_Indication_CB(SRAI_ModuleHandle module,
                                            uint32_t arg, uint32_t id, uint32_t value);

static DrmRC DRM_WVOemCrypto_P_Query_Mgn_Initialized(void);
static bool DRM_WVOemCrypto_P_Query_No_Write_Pending(void);

static DrmRC drm_WVOemCrypto_P_Do_SelectKey_V13(const uint32_t session,
                                                const uint8_t* key_id,
                                                uint32_t key_id_length,
                                                int *wvRc);

DrmRC drm_WVOemCrypto_P_Do_SelectKey(const uint32_t session,
                                     const uint8_t* key_id,
                                     uint32_t key_id_length,
                                     uint32_t cipher_mode,
                                     int *wvRc);


/*  nexus bounce buffer.
 *
 *  copy source data into a nexus bounce buffer if the source address
 *  is allocated in linux memory.
 *  this is a feature|limitation of the xpt|dma firmware which requires
 *  nexus buffers to operate.
 */
typedef struct Drm_WVOemCryptoBounceBuffer
{
   uint8_t *buffer;
   size_t  size;

} Drm_WVOemCryptoBounceBuffer;

static Drm_WVOemCryptoBounceBuffer gWVDecryptBounceBuffer[MAX_SG_DMA_BLOCKS];
static int gWVDecryptBounceBufferIndex = -1;

static Drm_WVOemCryptoBounceBuffer gWVCopyBounceBuffer[MAX_SG_DMA_BLOCKS];
static int gWVCopyBounceBufferIndex = -1;

/*  android native handle.
 *
 *  destination secure buffers are wrapped into an android native handle.
 *  rather than pulling the android include here, we just add the minimal
 *  needed to decipher the content.
 *
 *  Drm_WVOemCryptoNativeHandle: copy of android's native_handle_t
 *  Drm_WVOemCryptoOpaqueHandle: encapsulation of the native handle content.
 */
typedef struct Drm_WVOemCryptoNativeHandle
{
    int version;
    int numFds;
    int numInts;
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#endif
    int data[0];        /* numFds + numInts ints */
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
} Drm_WVOemCryptoNativeHandle;

typedef struct Drm_WVOemCryptoOpaqueHandle
{
   uint8_t *pBuffer;
   NEXUS_MemoryBlockHandle hBuffer;
   size_t clearBuffSize;
   size_t clearBuffOffset;
} Drm_WVOemCryptoOpaqueHandle;

static uint8_t *gWVDecryptBufferNativeHandle = NULL;
static uint8_t *gWVDecryptBufferSecure = NULL;

static uint8_t *gWVCopyBufferNativeHandle = NULL;
static uint8_t *gWVCopyBufferSecure = NULL;


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

    BDBG_MSG(("%s DRM %s TA %s ",BSTD_FUNCTION,pOemCryptoParamSettings->drm_bin_file_path, pOemCryptoParamSettings->drmCommonInit.ta_bin_file_path));

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
    gWvOemCryptoParamSettings.api_version = pWvOemCryptoParamSettings->api_version;
    gWvOemCryptoParamSettings.config_flags = pWvOemCryptoParamSettings->config_flags;

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
#ifdef USE_UNIFIED_COMMON_DRM
    DrmRC rc = Drm_Success;
#else
    DrmRC rc = Drm_Success;
    DrmRC rc2 = Drm_Success;
#endif
    *wvRc = SAGE_OEMCrypto_SUCCESS;
    unsigned int i;

    BDBG_ENTER(DRM_WVOemCrypto_UnInit);

    BDBG_WRN(("%s - Uninitializing", BSTD_FUNCTION));

    if(gBigUsageTableFormat && gSsdSupported)
    {
        DRM_WVOemCrypto_P_Query_No_Write_Pending();
    }

    if (gPadding != NULL)
    {
        NEXUS_Memory_Free(gPadding);
        gPadding = NULL;
    }

    /* This will invoke drm_wvoemcrypto_finalize on sage side*/
#ifdef USE_UNIFIED_COMMON_DRM
    if (gWVmoduleHandle != NULL)
    {
        rc = DRM_Common_TL_ModuleFinalize(gWVmoduleHandle);
    }
    if(rc == Drm_Success)
    {
        *wvRc = SAGE_OEMCrypto_SUCCESS;
    }
    else
    {
        *wvRc = SAGE_OEMCrypto_ERROR_TERMINATE_FAILED;
        rc = Drm_Err;
    }
#else
    if (gWVmoduleHandle != NULL)
    {
        rc = DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Widevine, gWVmoduleHandle);
    }
    if (gSSDmoduleHandle != NULL)
    {
        rc2 = DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Widevine, gSSDmoduleHandle);
    }
    if((rc == Drm_Success) && (rc2 == Drm_Success))
    {
        *wvRc = SAGE_OEMCrypto_SUCCESS;
    }
    else
    {
        *wvRc = SAGE_OEMCrypto_ERROR_TERMINATE_FAILED;
        rc = Drm_Err;
    }
#endif

#ifdef USE_UNIFIED_COMMON_DRM
    DRM_Common_TL_Finalize();
#else
    DRM_Common_TL_Finalize_TA(Common_Platform_Widevine);
#endif

    /*free the keyslots*/
    for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
    {
        if(gKeySlotCache[i].hSwKeySlot != NULL)
        {
            BDBG_MSG(("%s:Freeing Keyslot at index %d...",BSTD_FUNCTION, i));
#if (NEXUS_SECURITY_API_VERSION==1)
            NEXUS_Security_FreeKeySlot(gKeySlotCache[i].hSwKeySlot);
#else
            NEXUS_KeySlot_Free(gKeySlotCache[i].hSwKeySlot);
#endif
            gKeySlotCache[i].hSwKeySlot = NULL;
            if(gKeySlotCache[i].btp_sage_buffer != NULL)
            {
                SRAI_Memory_Free(gKeySlotCache[i].btp_sage_buffer);
                gKeySlotCache[i].btp_sage_buffer = NULL;
            }
        }
    }
    gKeySlotCacheAllocated = 0;

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

    if (gWVKeySlotMutex != NULL)
    {
        BKNI_DestroyMutex(gWVKeySlotMutex);
        gWVKeySlotMutex = NULL;
    }

    if (gWVSelectKeyMutex != NULL)
    {
        BKNI_DestroyMutex(gWVSelectKeyMutex);
        gWVSelectKeyMutex = NULL;
    }

    if(gWVUsageTable != NULL)
    {
        SRAI_Memory_Free(gWVUsageTable);
        gWVUsageTable = NULL;
    }

    gAntiRollbackHw = false;
    gSsdSupported = false;
    gWVCasMode = false;

    if (gWVDecryptBounceBufferIndex >= 0)
    {
       for (int i = 0; (i <= gWVDecryptBounceBufferIndex) && (i < MAX_SG_DMA_BLOCKS) ; i++)
       {
          if (gWVDecryptBounceBuffer[i].buffer)
          {
             NEXUS_Memory_Free(gWVDecryptBounceBuffer[i].buffer);
             gWVDecryptBounceBuffer[i].buffer = NULL;
             gWVDecryptBounceBuffer[i].size = 0;
          }
       }
       gWVDecryptBounceBufferIndex = -1;
    }

    if (gWVCopyBounceBufferIndex >= 0)
    {
       for (int i = 0; (i <= gWVCopyBounceBufferIndex) && (i < MAX_SG_DMA_BLOCKS) ; i++)
       {
          if (gWVCopyBounceBuffer[i].buffer)
          {
             NEXUS_Memory_Free(gWVCopyBounceBuffer[i].buffer);
             gWVCopyBounceBuffer[i].buffer = NULL;
             gWVCopyBounceBuffer[i].size = 0;
          }
       }
       gWVCopyBounceBufferIndex = -1;
    }

    BDBG_WRN(("%s - Uninitializing complete", BSTD_FUNCTION));

    BDBG_LEAVE(DRM_WVOemCrypto_UnInit);
    return rc;
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
    BSAGElib_InOutContainer *wv_container = NULL;
    BSAGElib_InOutContainer *ssd_container = NULL;
    time_t current_time = 0;
    bool drm_common_tl_initialized = false;
    bool drm_common_tl_module_initialized= false;
    NEXUS_SecurityCapabilities securityCapabilities;
    uint32_t keyslots_avail = 0;
    SRAI_Module_InitSettings wv_init_settings;
    SRAI_Module_InitSettings ssd_init_settings;

    BDBG_ENTER(DRM_WVOemCrypto_Initialize);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(pWvOemCryptoParamSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(pWvOemCryptoParamSettings->config_flags & DRM_WVOEMCRYPTO_INIT_OPTION_CAS)
    {
        gWVCasMode = true;
    }

    BDBG_WRN(("%s - Initializing api version %d", BSTD_FUNCTION, pWvOemCryptoParamSettings->api_version));

    /*this will inturn call drm_wvoemcrypto_init on sage side*/
    rc = DRM_Common_TL_Initialize(&pWvOemCryptoParamSettings->drmCommonInit);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module, Error :%d", BSTD_FUNCTION,rc));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    else
    {
        drm_common_tl_initialized = true;
    }

    wv_container = SRAI_Container_Allocate();
    if(wv_container == NULL)
    {
        BDBG_ERR(("%s - Error in allocating container memory", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    ssd_container = SRAI_Container_Allocate();
    if(ssd_container == NULL)
    {
        BDBG_ERR(("%s - Error in allocating container memory", BSTD_FUNCTION));
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

    wv_container->blocks[1].data.ptr = gWVUsageTable;
    if(wv_container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (on return)", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    wv_container->blocks[1].len = MAX_USAGE_TABLE_SIZE;
    BDBG_MSG(("%s -  %p -> length = '%u'", BSTD_FUNCTION, wv_container->blocks[1].data.ptr, wv_container->blocks[1].len));

    /* read contents of UsageTable.dat and place in shared pointer 'wv_container->blocks[1].data.ptr' */
    rc = DRM_WvOemCrypto_P_ReadUsageTable(wv_container->blocks[1].data.ptr, &wv_container->blocks[1].len);
    if(rc == Drm_Success)
    {
        BDBG_MSG(("%s - Usage Table detected on rootfs and read into shared memory", BSTD_FUNCTION));
    }
    else if(rc == Drm_FileErr)
    {
        BDBG_MSG(("%s - Usage Table not detected on rootfs, assuming initial creation...", BSTD_FUNCTION));
        wv_container->basicIn[2] = OVERWRITE_USAGE_TABLE_ON_ROOTFS;
        BKNI_Memset(wv_container->blocks[1].data.ptr, 0x00, wv_container->blocks[1].len);
    }
    else
    {
        BDBG_ERR(("%s - Usage Table detected on rootfs but error occurred reading it.", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    if(gWVClrMutex == NULL)
    {
        if(BKNI_CreateMutex(&gWVClrMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling creating clear buffer mutex", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }
    }

    if(gWVKeySlotMutex == NULL)
    {
        if(BKNI_CreateMutex(&gWVKeySlotMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling creating key slot mutex", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }
    }

    if(gWVSelectKeyMutex == NULL)
    {
        if(BKNI_CreateMutex(&gWVSelectKeyMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling creating select key mutex", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }
    }

    current_time = time(NULL);
    wv_container->basicIn[0] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));

    wv_container->basicIn[1] = 0;
    wv_container->basicIn[1] |= DRM_WVOEMCRYPTO_CENTRAL_KEY_CACHE;

    /* Only support big usage table format if api version is set to v13 or greater */
    if(pWvOemCryptoParamSettings->api_version >= 13)
    {
        wv_container->basicIn[1] |= DRM_WVOEMCRYPTO_BIG_USAGE_TABLE_FORMAT;
    }

    wv_container->basicIn[1] |= DRM_WVOEMCRYPTO_EVENT_DRIVEN_VERIFY;

#if DEBUG
    DRM_MSG_PRINT_BUF("Host side UT header (after reading or creating)", wv_container->blocks[1].data.ptr, 144);
#endif

    ssd_init_settings.indicationCallback = NULL;
    ssd_init_settings.indicationCallbackArg = 0;

    /* Initialize SAGE widevine module */
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eWVOemcrypto, pWvOemCryptoParamSettings->drm_bin_file_path, wv_container, &gWVmoduleHandle);
#else

    /* Do not initialize SSD for WV CAS or for older API versions */
    if(!gWVCasMode && pWvOemCryptoParamSettings->api_version >= 13)
    {
        rc = DRM_Common_TL_ModuleInitialize_TA_Ext(Common_Platform_Widevine, SSD_ModuleId_eClient, NULL, ssd_container, &gSSDmoduleHandle, &ssd_init_settings);
        if(rc != Drm_Success)
        {
            /* SSD is not supported */
            BDBG_WRN(("%s - Error initializing SSD module (0x%08x)", BSTD_FUNCTION, ssd_container->basicOut[0]));
        }
        else
        {
            gSsdSupported = true;
        }
    }

    wv_init_settings.indicationCallback = DRM_WVOemcrypto_P_Indication_CB;
    wv_init_settings.indicationCallbackArg = 0;

    rc = DRM_Common_TL_ModuleInitialize_TA_Ext(Common_Platform_Widevine, Widevine_ModuleId_eDRM, pWvOemCryptoParamSettings->drm_bin_file_path, wv_container, &gWVmoduleHandle, &wv_init_settings);
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing WV module (0x%08x)", BSTD_FUNCTION, wv_container->basicOut[0]));
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    else
    {
        drm_common_tl_module_initialized = true;
    }

    /* Obtain supported key slot cache mode */
    gCentralKeySlotCacheMode = (wv_container->basicOut[1] & DRM_WVOEMCRYPTO_CENTRAL_KEY_CACHE);
    gKeySlotCacheAllocated = 0;

    /* Obtain supported usage table format */
    gBigUsageTableFormat = (wv_container->basicOut[1] & DRM_WVOEMCRYPTO_BIG_USAGE_TABLE_FORMAT);

    /* Obtain support for event driven decrypt verification */
    gEventDrivenVerify = (wv_container->basicOut[1] & DRM_WVOEMCRYPTO_EVENT_DRIVEN_VERIFY);

    if (gPadding == NULL)
    {
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_MemoryAllocationSettings memSettings;
        NEXUS_Platform_GetClientConfiguration(&clientConfig); /* nexus_platform_client.h */
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);

        NEXUS_Memory_Allocate(PADDING_SZ_16_BYTE, &memSettings, (void **)&gPadding);
        if (gPadding == NULL)
        {
            BDBG_ERR(("%s - couldn't allocate memory for padding", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gPadding, 0x0,PADDING_SZ_16_BYTE);
    }

    BDBG_MSG(("%s - Did an encrypted Usage Table return to host? (wv_container->basicOut[2] = '%u')", BSTD_FUNCTION, wv_container->basicOut[2]));
    if(!gBigUsageTableFormat && wv_container->basicOut[2] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        /* Should only come in here when Usage Table is created for first time */
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(wv_container->blocks[1].data.ptr,
                                                   wv_container->blocks[1].len);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error creating Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit;
        }
    }

    if (gWvClrDmaJobBlockSettingsList == NULL)
    {
        gWvClrDmaJobBlockSettingsList = BKNI_Malloc(MAX_SG_DMA_BLOCKS * sizeof(NEXUS_DmaJobBlockSettings));
        if(gWvClrDmaJobBlockSettingsList == NULL)
        {
            BDBG_ERR(("%s - Error allocationg memory for DMA Job block settings", BSTD_FUNCTION ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gWvClrDmaJobBlockSettingsList, 0x0, MAX_SG_DMA_BLOCKS * sizeof(NEXUS_DmaJobBlockSettings));
    }

    gWvClrNumDmaBlocks = 0;
    gWVClrQueued = false;

    /* Initialize the key slot cache */
    BKNI_Memset(gKeySlotCache, 0, sizeof(gKeySlotCache));

    NEXUS_GetSecurityCapabilities(&securityCapabilities);
#if (NEXUS_SECURITY_API_VERSION==1)
    /* No proper define for type 3 keyslot so request numerically */
    keyslots_avail = securityCapabilities.keySlotTableSettings.numKeySlotsForType[3];
#else
    keyslots_avail = securityCapabilities.numKeySlotsForType[NEXUS_KeySlotType_eIvPerBlock];
#endif

    if(keyslots_avail < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT)
    {
        gKeySlotMaxAvail = keyslots_avail;
    }
    else
    {
        gKeySlotMaxAvail = DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT;
    }

    if(gCentralKeySlotCacheMode && gKeySlotMaxAvail > WV_CENTRAL_MAX_NUM_KEY_SLOT)
    {
        gKeySlotMaxAvail = WV_CENTRAL_MAX_NUM_KEY_SLOT;
    }

ErrorExit:

    if(wv_container != NULL)
    {
        SRAI_Container_Free(wv_container);
        wv_container = NULL;
    }

    if(ssd_container != NULL)
    {
        SRAI_Container_Free(ssd_container);
        ssd_container = NULL;
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

        if(gWVKeySlotMutex != NULL)
        {
            BKNI_DestroyMutex(gWVKeySlotMutex);
            gWVKeySlotMutex = NULL;
        }

        if(gWVSelectKeyMutex != NULL)
        {
            BKNI_DestroyMutex(gWVSelectKeyMutex);
            gWVSelectKeyMutex = NULL;
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

        if (drm_common_tl_module_initialized)
        {
#ifdef USE_UNIFIED_COMMON_DRM
           if (gWVmoduleHandle != NULL)
           {
               (void)DRM_Common_TL_ModuleFinalize(gWVmoduleHandle);
               gWVmoduleHandle = NULL;
           }
#else
           if (gWVmoduleHandle != NULL)
           {
               (void)DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Widevine, gWVmoduleHandle);
               gWVmoduleHandle = NULL;
           }
           if (gSSDmoduleHandle != NULL)
           {
              (void)DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Widevine, gSSDmoduleHandle);
              gSSDmoduleHandle = NULL;
           }
#endif
        }

        if (drm_common_tl_initialized)
        {
#ifdef USE_UNIFIED_COMMON_DRM
           DRM_Common_TL_Finalize();
#else
           DRM_Common_TL_Finalize_TA(Common_Platform_Widevine);
#endif
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

    if(gNumSessions == 0)
    {
        rc = DRM_WVOemCrypto_GetMaxNumberOfSessions(&gNumSessions, wvRc);
        if(rc != Drm_Success || gNumSessions == 0)
        {
            BDBG_ERR(("%s - Error obtaining maximum number of sessions (%d), rc:%d", BSTD_FUNCTION, gNumSessions, rc));
            if (rc == Drm_Success)
                rc = Drm_Err;
            goto ErrorExit;
        }
    }

    if(gHostSessionCtx == NULL)
    {
        gHostSessionCtx = BKNI_Malloc(gNumSessions * sizeof(Drm_WVOemCryptoHostSessionCtx_t));
        if(gHostSessionCtx == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for session context", BSTD_FUNCTION ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gHostSessionCtx, 0x0, gNumSessions * sizeof(Drm_WVOemCryptoHostSessionCtx_t));
    }

    if(gWvEncDmaBlockInfoList == NULL)
    {
        gWvEncDmaBlockInfoList = BKNI_Malloc(gNumSessions * sizeof(Drm_WVOemCryptoEncDmaList));
        if(gWvEncDmaBlockInfoList == NULL)
        {
            BDBG_ERR(("%s - Error allocationg memory for DMA block list", BSTD_FUNCTION ));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memset(gWvEncDmaBlockInfoList, 0x0, gNumSessions * sizeof(Drm_WVOemCryptoEncDmaList));
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eOpenSession, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
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
                                                                BSTD_FUNCTION, *wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }
    if(*wvRc != 0)
    {
        BDBG_ERR(("%s:Sage openssession command failed with error %d",BSTD_FUNCTION,*wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }
    /* if success, extract index from container */
    (*session) = container->basicOut[1];

    BDBG_MSG(("%s: opened session with id = %d",BSTD_FUNCTION,container->basicOut[1] ));

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
    uint32_t i;

    BDBG_ENTER(DRM_WVOemCrypto_CloseSession);
    BDBG_MSG(("%s closesession with id=%d",BSTD_FUNCTION, session));
    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_CLOSE_SESSION_FAILED ;
        goto ErrorExit;
    }
    container->basicIn[0]=session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eCloseSession, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_CLOSE_SESSION_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    *wvRc= container->basicOut[2];

    if(container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Operation failed. Setting session context to NULL sage rc = (0x%08x), wvRc = %d",
                                          BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
    }

ErrorExit:

    if (gWvEncDmaBlockInfoList)
    {
        if (gWvEncDmaBlockInfoList[session].dma_block_info != NULL)
        {
            SRAI_Memory_Free((uint8_t*)(gWvEncDmaBlockInfoList[session].dma_block_info));
            gWvEncDmaBlockInfoList[session].dma_block_info = NULL;
        }

        if (gWvEncDmaBlockInfoList[session].non_sec_src != NULL)
        {
            for(i = 0; i < MAX_SG_DMA_BLOCKS; i++)
            {
                if(gWvEncDmaBlockInfoList[session].non_sec_src[i] != NULL)
                {
                    SRAI_Memory_Free(gWvEncDmaBlockInfoList[session].non_sec_src[i]);
                    gWvEncDmaBlockInfoList[session].non_sec_src[i] = NULL;
                }
            }

            BKNI_Free((uint8_t*)(gWvEncDmaBlockInfoList[session].non_sec_src));
            gWvEncDmaBlockInfoList[session].non_sec_src = NULL;
        }

        if (gWvEncDmaBlockInfoList[session].non_sec_dst != NULL)
        {
            BKNI_Free((uint8_t*)(gWvEncDmaBlockInfoList[session].non_sec_dst));
            gWvEncDmaBlockInfoList[session].non_sec_dst = NULL;
        }

        if (gWvEncDmaBlockInfoList[session].non_sec_len != NULL)
        {
            BKNI_Free((uint8_t*)(gWvEncDmaBlockInfoList[session].non_sec_len));
            gWvEncDmaBlockInfoList[session].non_sec_len = NULL;
        }
    }

    if (gHostSessionCtx)
    {
        gHostSessionCtx[session].btp_sage_buffer_ptr = NULL;
        gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
        gHostSessionCtx[session].drmCommonOpStruct.num_dma_block = 0;
    }

    if(!gCentralKeySlotCacheMode && gHostSessionCtx)
    {
        /*free the session keyslot(s)*/
        for(i = 0; i < gHostSessionCtx[session].num_key_slots; i++)
        {
            if(gHostSessionCtx[session].key_slot_ptr[i] &&
                gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot != NULL)
            {
                BDBG_MSG(("%s:Freeing Keyslot...",BSTD_FUNCTION));
#if (NEXUS_SECURITY_API_VERSION==1)
                NEXUS_Security_FreeKeySlot(gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot);
#else
                NEXUS_KeySlot_Free(gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot);
#endif
                gHostSessionCtx[session].key_slot_ptr[i]->hSwKeySlot = NULL;
                if(gKeySlotCache[i].btp_sage_buffer != NULL)
                {
                    SRAI_Memory_Free(gKeySlotCache[i].btp_sage_buffer);
                    gKeySlotCache[i].btp_sage_buffer = NULL;
                }
                gKeySlotCacheAllocated--;
            }
            gHostSessionCtx[session].key_slot_ptr[i] = NULL;
        }
        gHostSessionCtx[session].num_key_slots = 0;
    }

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

    BDBG_MSG(("%s:Session ID=%d",BSTD_FUNCTION, session));

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc= SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateNonce, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
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
                                             BSTD_FUNCTION, *wvRc));
         rc =  Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*nonce) = container->basicOut[1];
    BDBG_MSG(("%s:nonce=0x%x",BSTD_FUNCTION,*nonce));

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
        BDBG_ERR(("%s - mac_key_context buffer is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(enc_key_context == NULL)
    {
        BDBG_ERR(("%s - enc_key_context buffer is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if((enc_key_context_length==0)||(mac_key_context_length==0))
    {
        BDBG_ERR(("%s - encrypted key length  (%u) or mac key context length (%u) is invalid", BSTD_FUNCTION, enc_key_context_length ,mac_key_context_length));
        rc = Drm_InvalidParameter;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error in allocating memory for MAC key context (%u bytes)", BSTD_FUNCTION, mac_key_context_length));
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
            BDBG_ERR(("%s - Error in allocating memory for encrypted key context (%u bytes)", BSTD_FUNCTION, enc_key_context_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,enc_key_context, enc_key_context_length);
        container->blocks[1].len = enc_key_context_length ;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateDerivedKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
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
                                    BSTD_FUNCTION, sage_rc, container->basicOut[2]));
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
        BDBG_ERR(("%s - message buffer is NULL", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", BSTD_FUNCTION, message_length));
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
        BDBG_ERR(("%s - Error allocating memory for signature (%u bytes)", BSTD_FUNCTION, (*signature_length)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memcpy(container->blocks[1].data.ptr, signature, (*signature_length));
    container->blocks[1].len = *signature_length ;


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateSignature, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending SAGE command", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE ;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];

    *wvRc = container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to GenerateSignature but actual operation ailed (0x%08x), wvRC = %d ",
                                                   BSTD_FUNCTION, sage_rc, *wvRc));
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

uint32_t getSizeOfKeyObjectArray_V11_to_V13(const Drm_WVOemCryptoKeyObject_V11_to_V13* key_array,uint32_t num_of_keys)
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

uint32_t getSizeOfEntitledContentKeyObjectArray(const Drm_WVOemCryptoEntitledContentKeyObject* key_array,uint32_t num_of_keys)
{
    uint32_t sz = 0;
    uint32_t i = 0;

    for(i = 0; i < num_of_keys; i++)
    {
        sz+=key_array[i].entitlement_key_id_length;
        sz+=key_array[i].content_key_id_length;
        sz+= WVCDM_KEY_IV_SIZE;
        sz+=key_array[i].content_key_data_length;
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
[in] srm_requirement:

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
DrmRC drm_WVOemCrypto_LoadKeys_V13(uint32_t session,
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
                                   const uint8_t* srm_requirement,
                                   int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    uint32_t key_object_shared_block_length = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoSageKeyObject_V11_to_V13* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject_V11_to_V13* keyArray = (Drm_WVOemCryptoKeyObject_V11_to_V13*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys_V13);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", BSTD_FUNCTION, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", BSTD_FUNCTION, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", BSTD_FUNCTION, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[3].data.ptr, enc_mac_keys, 2*WVCDM_MAC_KEY_SIZE);
        container->blocks[3].len = 2*WVCDM_MAC_KEY_SIZE ;
    }

    /* allocate for sending key object data*/
    key_array_sz = getSizeOfKeyObjectArray_V11_to_V13(keyArray, num_keys);
    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys);

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", BSTD_FUNCTION, pst_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[6].data.ptr, pst, pst_length);
        container->blocks[6].len = pst_length ;
    }

    if(srm_requirement)
    {
        container->blocks[7].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_CONTROL_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for SRM requirement data (%u bytes)", BSTD_FUNCTION, WVCDM_KEY_CONTROL_SIZE));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[7].data.ptr, srm_requirement, WVCDM_KEY_CONTROL_SIZE);
        container->blocks[7].len = WVCDM_KEY_CONTROL_SIZE;
    }

    /*
     * Fill in Key Array
     * */
    for(i=0,key_object_shared_block_length=0; i < num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",BSTD_FUNCTION, i,key_object_shared_block_length,keyArray[i].key_id_length));
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
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", BSTD_FUNCTION, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;



    pKeyObj = (Drm_WVOemCryptoSageKeyObject_V11_to_V13*)container->blocks[5].data.ptr;

    for (i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray[i].key_id_length;

        pKeyObj[i].key_data_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray[i].key_id_length];

        pKeyObj[i].key_data = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray[i].key_data_length;

        pKeyObj[i].key_control_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray[i].key_data_length];

        pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray[i].key_data_length];

        pKeyObj[i].cipher_mode = keyArray[i].cipher_mode;
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys_V13, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadKeys_V13));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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

    BDBG_LEAVE(drm_WVOemCrypto_LoadKeys_V13);
    return rc;
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
DrmRC drm_WVOemCrypto_LoadKeys_V11_or_V12(uint32_t session,
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
    Drm_WVOemCryptoSageKeyObject_V11_to_V13* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject_V11_to_V13* keyArray = (Drm_WVOemCryptoKeyObject_V11_to_V13*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys_V11_or_V12);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", BSTD_FUNCTION, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", BSTD_FUNCTION, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", BSTD_FUNCTION, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[3].data.ptr, enc_mac_keys, 2*WVCDM_MAC_KEY_SIZE);
        container->blocks[3].len = 2*WVCDM_MAC_KEY_SIZE ;
    }

    /* allocate for sending key object data*/
    key_array_sz = getSizeOfKeyObjectArray_V11_to_V13(keyArray, num_keys);
    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoSageKeyObject_V11_to_V13)) * num_keys);

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",BSTD_FUNCTION, i,key_object_shared_block_length,keyArray[i].key_id_length));
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
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", BSTD_FUNCTION, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;



    pKeyObj = (Drm_WVOemCryptoSageKeyObject_V11_to_V13*)container->blocks[5].data.ptr;

    for (i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray[i].key_id_length;

        pKeyObj[i].key_data_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray[i].key_id_length];

        pKeyObj[i].key_data = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray[i].key_data_length;

        pKeyObj[i].key_control_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray[i].key_data_length];

        pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray[i].key_data_length];

        pKeyObj[i].cipher_mode = keyArray[i].cipher_mode;
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys_V11_or_V12, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadKeys_V11_or_V12));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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

    BDBG_LEAVE(drm_WVOemCrypto_LoadKeys_V11_or_V12);
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
    Drm_WVOemCryptoSageKeyObject_V10* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject_V10* keyArray_V10 = (Drm_WVOemCryptoKeyObject_V10*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys_V9_or_V10);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", BSTD_FUNCTION, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", BSTD_FUNCTION, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", BSTD_FUNCTION, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageKeyObject_V10)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageKeyObject_V10)) * num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoSageKeyObject_V10)) * num_keys);

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",BSTD_FUNCTION, i,key_object_shared_block_length,keyArray_V10[i].key_id_length));
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
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", BSTD_FUNCTION, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;

    pKeyObj = (Drm_WVOemCryptoSageKeyObject_V10*)container->blocks[5].data.ptr;

    for(i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray_V10[i].key_id_length;

        pKeyObj[i].key_data_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray_V10[i].key_id_length];

        pKeyObj[i].key_data = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray_V10[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray_V10[i].key_data_length;

        pKeyObj[i].key_control_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray_V10[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray_V10[i].key_data_length];

        pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray_V10[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray_V10[i].key_data_length];
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys_V9_or_V10, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadKeys_V9_or_V10));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =    container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS){
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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
    Drm_WVOemCryptoSageKeyRefreshObject* pKeyObj = NULL;
    BSAGElib_InOutContainer *container = NULL;
    Drm_WVOemCryptoKeyRefreshObject* keyArray = (Drm_WVOemCryptoKeyRefreshObject*)key_array;

    BDBG_ENTER(drm_WVOemCrypto_RefreshKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL)
    {
        BDBG_ERR(("%s - message buffer is NULL", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", BSTD_FUNCTION, message_length));
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
            BDBG_ERR(("%s - Error allocating memory for Signature (%u bytes)", BSTD_FUNCTION, signature_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, signature, signature_length);
        container->blocks[1].len = signature_length ;
    }

    /* allocate for sending key object*/
    key_array_sz = getSizeOfKeyRefreshObjectArray(key_array,num_keys);
    BDBG_MSG(("%s:Key array sz is %d",BSTD_FUNCTION,key_array_sz));

    container->blocks[4].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[4].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key array (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memset(container->blocks[4].data.ptr, 0xff,key_array_sz);

    for( i=0,j=0; i<num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,j=%d, key id length is %d",BSTD_FUNCTION, i, j, keyArray[i].key_id_length));

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
            BDBG_MSG(("%s:keycontrol blk is in clear j=%d",BSTD_FUNCTION, j));

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
    BDBG_MSG(("%s: container->blocks[4].data.ptr is %p",BSTD_FUNCTION, (void *)container->blocks[4].data.ptr));


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageKeyRefreshObject)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageKeyRefreshObject)) * num_keys)));
        rc = Drm_Err;
        goto ErrorExit;
    }

    pKeyObj = (Drm_WVOemCryptoSageKeyRefreshObject*)container->blocks[5].data.ptr;
    for(i=0; i<num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        if(keyArray[i].key_control_iv != NULL)
        {
            BDBG_MSG(("%s:Keycontrol block isencrypted",BSTD_FUNCTION));

            pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];
            dump((const unsigned char*)pKeyObj[i].key_id,keyArray[i].key_id_length,"KeyID in sage mem:");

            BDBG_MSG(("key_id ptr = %p,offset=%d",(void *)pKeyObj[i].key_id,(i*container->blocks[4].len)/num_keys));
            pKeyObj[i].key_id_length = keyArray[i].key_id_length;

            BDBG_MSG(("key_id len =0x%x",pKeyObj[i].key_id_length));
            pKeyObj[i].key_control_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length];

            BDBG_MSG(("key_control_iv ptr =%p, offset=%d",(void *)pKeyObj[i].key_control_iv, (i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length));
            pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys +keyArray[i].key_id_length+WVCDM_KEY_IV_SIZE];

            BDBG_MSG(("key_control ptr =%p",(void *)pKeyObj[i].key_control));
        }
        else
        {
            BDBG_MSG(("%s:Keycontrol block is clear",BSTD_FUNCTION));

            pKeyObj[i].key_id_length = keyArray[i].key_id_length;
            if(pKeyObj[i].key_id_length == 0){
                BDBG_MSG(("%s:one KCB for all keys has to be used",BSTD_FUNCTION));
                pKeyObj[i].key_id = 0;
            }
            else
            {
                pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];
                dump((const unsigned char*)pKeyObj[i].key_id, keyArray[i].key_id_length, "KeyID in sage mem:");
            }
            pKeyObj[i].key_control_iv = 0;
            pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys + keyArray[i].key_id_length];
            dump((const unsigned char*)container->blocks[4].data.ptr,WVCDM_KEY_CONTROL_SIZE,"clear key ctrl blk:");

        }
    }

    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoSageKeyRefreshObject)) * num_keys);

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eRefreshKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc   = container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to Refreshkeys but actual operation failed (0x%08x), wvRc = %d ",
                         BSTD_FUNCTION, sage_rc, *wvRc));
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
DrmRC drm_WVOemCrypto_SelectKey_V13(const uint32_t session,
                                    const uint8_t* key_id,
                                    uint32_t key_id_length,
                                    int *wvRc)
{
    DrmRC rc = Drm_Success;
    BKNI_AcquireMutex(gWVSelectKeyMutex);
    rc = drm_WVOemCrypto_P_Do_SelectKey_V13(session, key_id, key_id_length, wvRc);
    BKNI_ReleaseMutex(gWVSelectKeyMutex);
    return rc;
}

DrmRC drm_WVOemCrypto_P_Do_SelectKey_V13(const uint32_t session,
                                              const uint8_t* key_id,
                                              uint32_t key_id_length,
                                              int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
#else
    NEXUS_KeySlotAllocateSettings keyslotSettings;
    NEXUS_KeySlotInformation keyslotInfo;
#endif
    BSAGElib_InOutContainer *container = NULL;
    uint32_t keySlotIdSelected;
    NEXUS_KeySlotHandle hKeySlotSelected = NULL;
    uint32_t keySlotID[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
    bool allocate_slot = false;
    uint32_t i;

    BDBG_ENTER(drm_WVOemCrypto_P_Do_SelectKey_V13);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(key_id == NULL)
    {
        BDBG_ERR(("%s - key_id buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:session =%d, key_id_len=%d, key central cache mode=%d",BSTD_FUNCTION,session,key_id_length, gCentralKeySlotCacheMode));

    gHostSessionCtx[session].session_id=session;

    BKNI_Memcpy(gHostSessionCtx[session].key_id, key_id, key_id_length);
    gHostSessionCtx[session].key_id_length =key_id_length;

    /* A central key cache should utilize the maximum amount of slots */
    if(gCentralKeySlotCacheMode && gKeySlotCacheAllocated < gKeySlotMaxAvail)
    {
        allocate_slot = true;
    }

    /* Session based key cache needs only a one time allocation per session */
    if(!gCentralKeySlotCacheMode)
    {
        if(gHostSessionCtx[session].num_key_slots < DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
        {
            allocate_slot = true;
        }
    }

    if(allocate_slot)
    {
        BKNI_AcquireMutex(gWVKeySlotMutex);
        for(i = 0; i < gKeySlotMaxAvail; i++)
        {
            if(gKeySlotCache[i].hSwKeySlot == NULL)
            {
                BDBG_MSG(("%s:Allocate keyslot for index %d",BSTD_FUNCTION, i));
#if (NEXUS_SECURITY_API_VERSION==1)
                NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
                keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
                keyslotSettings.client = NEXUS_SecurityClientType_eSage;
                gKeySlotCache[i].hSwKeySlot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
#else
                NEXUS_KeySlot_GetDefaultAllocateSettings(&keyslotSettings);
                keyslotSettings.useWithDma = true;
                keyslotSettings.owner = NEXUS_SecurityCpuContext_eSage;
                keyslotSettings.slotType   = NEXUS_KeySlotType_eIvPerBlock;
                gKeySlotCache[i].hSwKeySlot =  NEXUS_KeySlot_Allocate(&keyslotSettings);
#endif
                if(gKeySlotCache[i].hSwKeySlot == NULL)
                {
                    if(gHostSessionCtx[session].num_key_slots > 0 ||
                        (gCentralKeySlotCacheMode && gKeySlotCacheAllocated > 0))
                    {
                        /* Continue onwards if we allocated at least one key slot */
                        BDBG_WRN(("%s - Unable to allocate keyslot at index %d, continuing with %d keyslots allocated", BSTD_FUNCTION, i, gKeySlotCacheAllocated));
                        break;
                    }
                    else
                    {
                        BKNI_ReleaseMutex(gWVKeySlotMutex);
                        BDBG_ERR(("%s - Error allocating keyslot at index %d, %d keyslots allocated", BSTD_FUNCTION, i, gKeySlotCacheAllocated));
                        rc = BERR_INVALID_PARAMETER;
                        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                        goto ErrorExit;
                    }
                }

                BDBG_MSG(("%s: ======Allocated nexus key slot for index %d, keyslot handle=%p =======",
                    BSTD_FUNCTION, i, (void *)gKeySlotCache[i].hSwKeySlot));
#if (NEXUS_SECURITY_API_VERSION==1)
                NEXUS_Security_GetKeySlotInfo(gKeySlotCache[i].hSwKeySlot, &keyslotInfo);
                gKeySlotCache[i].keySlotID = keyslotInfo.keySlotNumber;
                BDBG_MSG(("%s - keyslotID[%d] Keyslot number = '%u'", BSTD_FUNCTION, i, keyslotInfo.keySlotNumber));
#else
                NEXUS_KeySlot_GetInformation( gKeySlotCache[i].hSwKeySlot, &keyslotInfo);
                gKeySlotCache[i].keySlotID = keyslotInfo.slotNumber;
                BDBG_MSG(("%s - keyslotID[%d] Keyslot number = '%u'", BSTD_FUNCTION, i, keyslotInfo.slotNumber));
#endif
                gHostSessionCtx[session].key_slot_ptr[gHostSessionCtx[session].num_key_slots] = &gKeySlotCache[i];
                gHostSessionCtx[session].num_key_slots++;

                gKeySlotCacheAllocated++;

                if(!gCentralKeySlotCacheMode && gHostSessionCtx[session].num_key_slots >= DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
                {
                    /* Cap the amount of slots to allocate in session key cache mode */
                    break;
                }
            }
        }
        BKNI_ReleaseMutex(gWVKeySlotMutex);
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
           BDBG_ERR(("%s - Error accessing key slot", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)", BSTD_FUNCTION, key_id_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        container->blocks[0].len = key_id_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, key_id, key_id_length);
        dump(container->blocks[0].data.ptr,key_id_length,"key id in sage mem:");
    }

    /* Provide allocated key slots */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)",
            BSTD_FUNCTION, sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[1].len = sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT;

    BKNI_Memcpy(container->blocks[1].data.ptr, keySlotID,
        sizeof(keySlotID[0]) * gHostSessionCtx[session].num_key_slots);
    dump(container->blocks[1].data.ptr, sizeof(keySlotID[0]) * gHostSessionCtx[session].num_key_slots,"key slot ID in sage mem:");

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = key_id_length;
    container->basicIn[2] = gHostSessionCtx[session].num_key_slots;

    /* Set to an invalid value to see if key slot was selected */
    container->basicOut[3] = INVALID_KEYSLOT_ID;

    BKNI_AcquireMutex(gWVKeySlotMutex);

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eSelectKey_V13, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:Sage process command is sucesss, next extract the status of the command",BSTD_FUNCTION));

    *wvRc = container->basicOut[2];
    BDBG_MSG(("%s:* WVRC=%d",BSTD_FUNCTION,*wvRc));

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Command was sent successfully to loadkey but actual operation failed (0x%08x, 0x%08x)", BSTD_FUNCTION, container->basicOut[0],
            container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if (container->basicOut[3] == INVALID_KEYSLOT_ID)
    {
        /* If key slot was not selected but a successful operation, an AES decryption key was not selected.
         * Key slot is expected to not be selected in this scenario.
         */
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        goto IgnoreKeySlot;
    }

    keySlotIdSelected = container->basicOut[3];

    for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
    {
        if(gKeySlotCache[i].hSwKeySlot != NULL && keySlotIdSelected == gKeySlotCache[i].keySlotID)
        {
           hKeySlotSelected = gKeySlotCache[i].hSwKeySlot;
           break;
        }
    }
    if(i == DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Error unknown keyslot %d selected", BSTD_FUNCTION, keySlotIdSelected));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Ensure key slot is unique within the session list */
    for(i = 0; i < gNumSessions; i++)
    {
        if(gHostSessionCtx[i].drmCommonOpStruct.keyConfigSettings.keySlot == hKeySlotSelected)
        {
            /* Set session key slot pointer to NULL as it was overwritten in cache.
             * Select key operation must be called again prior to decryption */
            gHostSessionCtx[i].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
        }
    }
    /* Assign the key slot handle to the session */
    gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = hKeySlotSelected;

#if (NEXUS_SECURITY_API_VERSION==1)
    BDBG_MSG(("%s - Selected by Sage: Keyslot number = '%u'", BSTD_FUNCTION, keyslotInfo.keySlotNumber));
#else
    BDBG_MSG(("%s - Selected by Sage: Keyslot number = '%u'", BSTD_FUNCTION, keyslotInfo.slotNumber));
#endif

    BKNI_ReleaseMutex(gWVKeySlotMutex);

    /* Set the cipher mode */
    gHostSessionCtx[session].cipher_mode = container->basicOut[1];
    gHostSessionCtx[session].key_select_count++;
    gHostSessionCtx[session].btp_sage_buffer_ptr = NULL;

IgnoreKeySlot:
ErrorExit:

    BDBG_MSG(("%s: free container block 0:",BSTD_FUNCTION));
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

    BDBG_MSG(("%s:set the return value based on wvRc",BSTD_FUNCTION));
    if (*wvRc!= 0)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_P_Do_SelectKey_V13);

   return rc;
}

static void  DRM_WVOemcrypto_P_Indication_CB(SRAI_ModuleHandle module, uint32_t arg, uint32_t id, uint32_t value)
{
    uint32_t i;

    BSTD_UNUSED(module);
    BSTD_UNUSED(arg);

    switch(id)
    {
        case DrmWVOEMCrypto_NotificationId_eKeySlotInvalidated:
        {
            BDBG_MSG(("%s - Key Slot invalidated indication event received, session ID: %d", BSTD_FUNCTION, value));
            if(gWVKeySlotMutex)
            {
                BKNI_AcquireMutex(gWVKeySlotMutex);
                if(gHostSessionCtx != NULL && value < gNumSessions)
                {
                    gHostSessionCtx[value].key_slot_error_notification = true;
                }
                BKNI_ReleaseMutex(gWVKeySlotMutex);
            }
            break;
        }
        case DrmWVOEMCrypto_NotificationId_eKeySlotsFlushed:
        {
            if(gWVKeySlotMutex)
            {
                BKNI_AcquireMutex(gWVKeySlotMutex);
                for(i = 0; i < gNumSessions; i++)
                {
                    gHostSessionCtx[i].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
                }
                BKNI_ReleaseMutex(gWVKeySlotMutex);
            }
            break;
        }
        default:
        {
            BDBG_WRN(("%s - Unknown notification id(0x%x)", __FUNCTION__, id));
        }
    }
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
        BDBG_WRN(("%s: No DMA job pending", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_WaitForEvent(gWVClrEvent, BKNI_INFINITE);

    nexus_rc = NEXUS_DmaJob_GetStatus(gWVClrJob, &status);
    if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: DmaJob_GetStatus err=%d", BSTD_FUNCTION, nexus_rc));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    if (status.currentState != NEXUS_DmaJobState_eComplete ) {
        BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed", BSTD_FUNCTION ));
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
    uint32_t i;

    if (gWvClrNumDmaBlocks == 0)
    {
        /* Return with warning if there is nothing to transfer */
        BDBG_WRN(("%s: No valid data to transfer", BSTD_FUNCTION));
        goto ErrorExit;
    }

    if (gWVClrQueued || gWVClrDma != NULL || gWVClrJob != NULL)
    {
        BDBG_WRN(("%s: DMA did not finish, forcing wait", BSTD_FUNCTION));
        drm_rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
        if (drm_rc != Drm_Success)
        {
            BDBG_ERR(("%s: Failed to verify DMA transfer complete", BSTD_FUNCTION));
            goto ErrorExit;
        }
    }

    if (gWVClrEvent == NULL)
    {
        BKNI_CreateEvent(&gWVClrEvent);
        if (gWVClrEvent == NULL)
        {
            BDBG_ERR(("%s: Failed to create event", BSTD_FUNCTION));
            drm_rc = Drm_Err;
            goto ErrorExit;
        }
    }

    gWVClrDma = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    if (gWVClrDma == NULL)
    {
        BDBG_ERR(("%s: Failed to NEXUS_Dma_Open !!!", BSTD_FUNCTION));
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
        BDBG_ERR(("%s: Failed to NEXUS_DmaJob_Create !!!", BSTD_FUNCTION));
        drm_rc = Drm_NexusErr;
        goto ErrorExit;
    }

    blockSettings = &gWvClrDmaJobBlockSettingsList[gWvClrNumDmaBlocks - 1];

    /* start DMA transfer when last subsample or beyond MAX */
    BDBG_MSG(("%s  nb_blks=%d",  BSTD_FUNCTION, gWvClrNumDmaBlocks));

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
        BDBG_ERR(("%s: error in dma transfer, err:%d", BSTD_FUNCTION, nexus_rc));
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
    NEXUS_DmaJobBlockSettings *blockSettings;
    DrmRC rc = Drm_Success;

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
            BSTD_FUNCTION, gWvClrNumDmaBlocks, data_addr, data_length, isLastSubsample));

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
            BDBG_ERR(("%s: Unable to initiate DMA transfer",BSTD_FUNCTION));
            goto ErrorExit;
        }

        if (block_wait && gWVClrQueued)
        {
            rc = drm_WVOemCrypto_P_Wait_Clear_Transfer();
            if (rc != Drm_Success)
            {
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", BSTD_FUNCTION));
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
        BDBG_ERR(("%s: error, DMA block info pointer is NULL", BSTD_FUNCTION));
        return Drm_Err;
    }

    blockInfo = gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock;

    /* Mark the end of the scatter/gather list */
    blockInfo[dmaBlockIdx-1].sg_end = true;

    BDBG_MSG(("%s: session[%d], nb_blks=%d", BSTD_FUNCTION, session, dmaBlockIdx));

    /*start M2M transfer*/
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_M2mOperation(&gHostSessionCtx[session].drmCommonOpStruct, true, true);
#else
    rc = DRM_Common_TL_M2mOperation_TA(Common_Platform_Widevine, &gHostSessionCtx[session].drmCommonOpStruct, true, true);
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Call to 'DRM_Common_TL_M2mOperation' failed 0x%x", BSTD_FUNCTION, rc));
        return rc;
    }

    BDBG_MSG(("%s - Decrypted %d dma blocks", BSTD_FUNCTION, dmaBlockIdx));
    return rc;
}


static DrmRC drm_WVOemCrypto_P_DecryptDMA_SG(uint8_t *data_addr,
                                             uint8_t *out_addr,
                                             uint32_t data_length,
                                             uint32_t enc_subsample_flags,
                                             uint32_t *block_offset,
                                             uint32_t session,
                                             bool secure)
{
    uint8_t *pDecryptBuf = NULL;
    bool isFirstEncSubsample = false;
    bool isLastEncSubsample = false;
    DmaBlockInfo_t *blockInfo;
    uint8_t **ppNonSecSrc;
    uint8_t **ppNonSecDst;
    uint32_t *pNonSecLen;
    uint32_t dmaBlockIdx;
    DrmRC rc = Drm_Success;
    unsigned int i;

    isFirstEncSubsample = enc_subsample_flags & WV_OEMCRYPTO_FIRST_SUBSAMPLE;
    isLastEncSubsample = enc_subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE;

    if(*block_offset >= PADDING_SZ_16_BYTE)
    {
        BDBG_ERR(("%s: Invalid block offset value of %d", BSTD_FUNCTION, *block_offset));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Allocate the dma block memory if it doesn't exist yet */
    blockInfo = gWvEncDmaBlockInfoList[session].dma_block_info;
    if (blockInfo == NULL) {
        blockInfo = (DmaBlockInfo_t *)SRAI_Memory_Allocate(sizeof(DmaBlockInfo_t) * MAX_SG_DMA_BLOCKS, SRAI_MemoryType_Shared);
        if (blockInfo == NULL)
        {
            BDBG_ERR(("%s: error allocating dma blockinfo memory", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }
        gWvEncDmaBlockInfoList[session].dma_block_info = blockInfo;
    }

    if(!secure)
    {
        ppNonSecSrc = gWvEncDmaBlockInfoList[session].non_sec_src;
        if(ppNonSecSrc == NULL)
        {
            ppNonSecSrc = (uint8_t **)BKNI_Malloc(sizeof(uint8_t *) * MAX_SG_DMA_BLOCKS);
            if(ppNonSecSrc == NULL)
            {
                BDBG_ERR(("%s: error allocating non secure dest memory", BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }
            BKNI_Memset(ppNonSecSrc, 0, (sizeof(uint8_t *) * MAX_SG_DMA_BLOCKS));
            gWvEncDmaBlockInfoList[session].non_sec_src = ppNonSecSrc;
        }

        ppNonSecDst = gWvEncDmaBlockInfoList[session].non_sec_dst;
        if(ppNonSecDst == NULL)
        {
            ppNonSecDst = (uint8_t **)BKNI_Malloc(sizeof(uint8_t *) * MAX_SG_DMA_BLOCKS);
            if(ppNonSecDst == NULL)
            {
                BDBG_ERR(("%s: error allocating non secure dest memory", BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }
            BKNI_Memset(ppNonSecDst, 0, (sizeof(uint8_t *) * MAX_SG_DMA_BLOCKS));
            gWvEncDmaBlockInfoList[session].non_sec_dst = ppNonSecDst;
        }

        pNonSecLen = gWvEncDmaBlockInfoList[session].non_sec_len;
        if(pNonSecLen == NULL)
        {
            pNonSecLen = (uint32_t *)BKNI_Malloc(sizeof(uint32_t) * MAX_SG_DMA_BLOCKS);
            if(pNonSecLen == NULL)
            {
                BDBG_ERR(("%s: error allocating non secure dest memory", BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }
            BKNI_Memset(pNonSecLen, 0, (sizeof(uint32_t) * MAX_SG_DMA_BLOCKS));
            gWvEncDmaBlockInfoList[session].non_sec_len = pNonSecLen;
        }
    }

    dmaBlockIdx = gHostSessionCtx[session].drmCommonOpStruct.num_dma_block;
    BDBG_MSG(("%s  session=%d, data_addr=0x%08x, len=%d, isFirstEncSubsample=%d, isLastEncSubsample=%d",
        BSTD_FUNCTION, session, data_addr,data_length, isFirstEncSubsample, isLastEncSubsample ));

    if (data_addr != NULL && out_addr != NULL && data_length > 0)
    {
        if (!secure)
        {
            /* Allocate buffer memory */
            pDecryptBuf = SRAI_Memory_Allocate(data_length + (16 - (data_length % 16)), SRAI_MemoryType_Shared);
            if(pDecryptBuf == NULL)
            {
                BDBG_ERR(("%s: %d Out of memory\n", BSTD_FUNCTION, __LINE__));
                rc = Drm_Err;
                goto ErrorExit;
            }

            /* Copy over encrypted data to buffer */
            BKNI_Memcpy(pDecryptBuf, data_addr, data_length);
        }

        if (dmaBlockIdx == 0)
        {
            if (gHostSessionCtx[session].btp_sage_buffer_ptr != NULL)
            {
                /* BTP block */
                blockInfo[dmaBlockIdx].pDstData = gHostSessionCtx[session].btp_sage_buffer_ptr;
                blockInfo[dmaBlockIdx].pSrcData = gHostSessionCtx[session].btp_sage_buffer_ptr;
                blockInfo[dmaBlockIdx].uiDataSize = BTP_SIZE;
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

        if (!secure)
        {
            blockInfo[dmaBlockIdx].pDstData = pDecryptBuf;
            blockInfo[dmaBlockIdx].pSrcData = pDecryptBuf;
        }
        else
        {
            blockInfo[dmaBlockIdx].pDstData = out_addr;
            blockInfo[dmaBlockIdx].pSrcData = data_addr;
        }

        blockInfo[dmaBlockIdx].uiDataSize = data_length;
        blockInfo[dmaBlockIdx].sg_start = (isFirstEncSubsample && *block_offset == 0);
        blockInfo[dmaBlockIdx].sg_end = false; /* End marker set on transfer execution */

        if (!secure)
        {
            if(ppNonSecSrc[dmaBlockIdx] != NULL)
            {
                /* Free previous memory if somehow it was not freed prior */
                SRAI_Memory_Free(ppNonSecSrc[dmaBlockIdx]);
            }
            ppNonSecSrc[dmaBlockIdx] = pDecryptBuf;
            ppNonSecDst[dmaBlockIdx] = out_addr;
            pNonSecLen[dmaBlockIdx] = data_length;
        }

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
            gHostSessionCtx[session].drmCommonOpStruct.pDmaBlock = gWvEncDmaBlockInfoList[session].dma_block_info;
            rc = drm_WVOemCrypto_P_Decrypt_Transfer(session);

            if(!secure)
            {
                /* Copy and free all allocated buffers */
                for(i = 0; i < dmaBlockIdx; i++)
                {
                    if(ppNonSecSrc[i] != NULL && ppNonSecDst[i] != NULL)
                    {
                        BKNI_Memcpy(ppNonSecDst[i], ppNonSecSrc[i], pNonSecLen[i]);
                    }

                    if(ppNonSecSrc[i] != NULL)
                    {
                        SRAI_Memory_Free(ppNonSecSrc[i]);
                        ppNonSecSrc[i] = NULL;
                    }

                    if(ppNonSecDst[i] != NULL)
                    {
                        ppNonSecDst[i] = NULL;
                    }

                    if(pNonSecLen[i] != 0)
                    {
                        pNonSecLen[i] = 0;
                    }
                }
            }
            gHostSessionCtx[session].drmCommonOpStruct.num_dma_block = 0;
        }
    }
ErrorExit:
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
                                     bool force_transfer,
                                     int *wvRc)
{
    BSAGElib_InOutContainer *container = NULL;

    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    *wvRc = SAGE_OEMCrypto_SUCCESS;
    bool isSecureDecrypt = (buffer_type == Drm_WVOEMCrypto_BufferType_Secure);
    bool call_decrypt_verify = false;
    time_t current_time;
    bool hold_keyslot = false;
    uint32_t i = 0;
    bool keyslot_found = false;

    BDBG_ENTER(drm_WVOemCrypto_P_DecryptPatternBlock);
    BDBG_MSG(("%s: Input data len=%d,is_encrypted=%d, sf:%d, secure:%d",
                BSTD_FUNCTION, data_length,is_encrypted,subsample_flags, isSecureDecrypt));

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - data buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    dump(data_addr,data_length,"encrypted data:");

    if(iv == NULL)
    {
        BDBG_ERR(("%s - iv buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(!force_transfer && isSecureDecrypt && (data_length % AES_BLOCK_SIZE) != 0)
    {
        /* Force transfer to prevent secure buffer mis-alignment */
        force_transfer = true;
    }

    if(!force_transfer && gEventDrivenVerify)
    {
        current_time = time(NULL);
        if(gHostSessionCtx[session].drmCommonOpStruct.num_dma_block > 0 &&
            current_time >= gHostSessionCtx[session].decrypt_verify_time + WV_DECRYPT_VERIFY_INTERVAL)
        {
            /* Force a transfer if an encrypted portion is present and we've met the deadline */
            BDBG_ERR(("%s - Forcing transfer to meet deadline", BSTD_FUNCTION));
            force_transfer = true;
        }
    }

    if (force_transfer)
    {
        /* Force a transfer by overriding subsample flags */
        subsample_flags |= WV_OEMCRYPTO_LAST_SUBSAMPLE;
    }

    if (!is_encrypted)
    {
        if (isSecureDecrypt)
        {
            if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(out_buffer, data_addr, data_length, subsample_flags, false) != Drm_Success)
            {
                BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",BSTD_FUNCTION));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }
        }
        else
        {
            BKNI_Memcpy(out_buffer,data_addr, data_length);
        }

        if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
        {
            /* Last subsample is clear. Transfer the encrypted block list */
             if(drm_WVOemCrypto_P_DecryptDMA_SG(NULL, NULL, 0,
                subsample_flags, &block_offset, session, isSecureDecrypt) != Drm_Success)
            {
                BDBG_ERR(("%s: decryption failed for SG DMA",BSTD_FUNCTION));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }
        }
    }
    else
    {
        if(gHostSessionCtx[session].cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
        {
             /* Verify block offset is 0 when cipher mode is CBC */
             if(block_offset != 0)
             {
                 BDBG_ERR(("%s: Invalid block offset provided",BSTD_FUNCTION));
                 rc = Drm_Err;
                 *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                 goto ErrorExit;
             }
        }

        /* Allocate BTP buffer if needed */
        if (gHostSessionCtx[session].btp_sage_buffer_ptr == NULL)
        {
            /* Search for available BTP.  If not found, allocate. */
            for(i = 0; i < gKeySlotMaxAvail; i++)
            {
                if(gKeySlotCache[i].hSwKeySlot != NULL)
                {
                    if(gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot == gKeySlotCache[i].hSwKeySlot)
                    {
                        keyslot_found = true;
                        break;
                    }
                }
            }

            if(!keyslot_found)
            {
                BDBG_ERR(("%s: Unable to find selected key slot ID (%u)", BSTD_FUNCTION, gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot));
                *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                rc = Drm_Err;
                goto ErrorExit;
            }

            if(gKeySlotCache[i].btp_sage_buffer == NULL)
            {
                if(gEventDrivenVerify)
                {
                    gKeySlotCache[i].btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_Shared);
                }
                else
                {
                    gKeySlotCache[i].btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_SagePrivate);
                }

                if(gKeySlotCache[i].btp_sage_buffer == NULL)
                {
                    BDBG_ERR(("%s: Out of memory for BTP (%u bytes)", BSTD_FUNCTION, BTP_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                /* We must fill the BTP by calling decryption verify */
                call_decrypt_verify = true;
            }
            gHostSessionCtx[session].btp_sage_buffer_ptr = gKeySlotCache[i].btp_sage_buffer;
        }

        /* Mark if this is the first encrypted subsample */
        if(gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            subsample_flags |= WV_OEMCRYPTO_FIRST_SUBSAMPLE;
        }

        BKNI_AcquireMutex(gWVKeySlotMutex);

        /* Verify key slot handle is present */
        if(gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot == NULL)
        {
            BKNI_ReleaseMutex(gWVKeySlotMutex);

            hold_keyslot = true;

            /* Call again to select key to obtain a keyslot handle */
            BKNI_AcquireMutex(gWVSelectKeyMutex);
            if(gWvOemCryptoParamSettings.api_version <= 13)
            {
                rc = drm_WVOemCrypto_P_Do_SelectKey_V13(session, gHostSessionCtx[session].key_id,
                    gHostSessionCtx[session].key_id_length, wvRc);
            }
            else
            {
                rc = drm_WVOemCrypto_P_Do_SelectKey(session, gHostSessionCtx[session].key_id,
                    gHostSessionCtx[session].key_id_length, gHostSessionCtx[session].cipher_mode, wvRc);
            }
            if(rc != Drm_Success)
            {
                BDBG_ERR(("%s: No valid key slot handle available", BSTD_FUNCTION));
                goto ErrorExit;
            }
            call_decrypt_verify = true;

            BKNI_AcquireMutex(gWVKeySlotMutex);
        }

        if(gEventDrivenVerify)
        {
            if(gHostSessionCtx[session].key_slot_error_notification)
            {
                gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
                gHostSessionCtx[session].key_slot_error_notification = false;
                call_decrypt_verify = true;
            }

            if(!call_decrypt_verify && gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
            {
                /* Required to call SAGE for decrypt verification every second or when new key selected */
                current_time = time(NULL);
                if(gHostSessionCtx[session].force_decrypt_verify ||
                    gHostSessionCtx[session].key_select_count > WV_KEY_SELECT_COUNT_MAX ||
                    current_time >= gHostSessionCtx[session].decrypt_verify_time + WV_DECRYPT_VERIFY_INTERVAL)
                {
                    gHostSessionCtx[session].key_select_count = 0;
                    gHostSessionCtx[session].force_decrypt_verify = false;
                    call_decrypt_verify = true;
                }
            }
        }
        else
        {
            if(gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
            {
                gHostSessionCtx[session].key_select_count = 0;
                call_decrypt_verify = true;
            }
        }

        BKNI_ReleaseMutex(gWVKeySlotMutex);

        /* Load the IV on the first block of a DMA */
        if(call_decrypt_verify)
        {
            container = SRAI_Container_Allocate();
            if(container == NULL)
            {
                BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
                    BDBG_ERR(("%s: Out of memory for IV (%u bytes)", BSTD_FUNCTION, WVCDM_KEY_IV_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                container->blocks[0].len = WVCDM_KEY_IV_SIZE;
                BKNI_Memcpy(container->blocks[0].data.ptr, iv, WVCDM_KEY_IV_SIZE);
            }

            /* Map to parameters into srai_inout_container */
            container->basicIn[0] = session;
            container->basicIn[1] = WVCDM_KEY_IV_SIZE;
            container->basicIn[2] = buffer_type;

            container->basicIn[3]= BTP_SIZE;
            container->blocks[1].data.ptr = gHostSessionCtx[session].btp_sage_buffer_ptr;
            container->blocks[1].len = BTP_SIZE;

            /* This command loads the IV and validates key control */
            sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDecryptCENC, container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error in DrmWVOEMCrypto_CommandId_eDecryptCENC", BSTD_FUNCTION));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            *wvRc = container->basicOut[2];
            if (container->basicOut[0] != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Command was sent successfully to load IV in decryptCENC but actual operation failed (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
                rc = Drm_Err;
                goto ErrorExit;
            }

            BDBG_MSG(("%s:extract wv error code",BSTD_FUNCTION));
            if(*wvRc != SAGE_OEMCrypto_SUCCESS)
            {
                BDBG_ERR(("%s:WVRC error",BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }

            if(gEventDrivenVerify)
            {
                gHostSessionCtx[session].ext_iv_offset = container->basicOut[1];
                gHostSessionCtx[session].decrypt_verify_time = time(NULL);
            }
        }
        else if(gEventDrivenVerify && gHostSessionCtx[session].drmCommonOpStruct.num_dma_block == 0)
        {
            /* Fill in BTP information */
            BKNI_Memcpy(&gHostSessionCtx[session].btp_sage_buffer_ptr[20 +(gHostSessionCtx[session].ext_iv_offset * 16)], &iv[8], 8);
            BKNI_Memcpy(&gHostSessionCtx[session].btp_sage_buffer_ptr[20 +(gHostSessionCtx[session].ext_iv_offset * 16) + 16], &iv[0], 8);
        }

        BKNI_AcquireMutex(gWVKeySlotMutex);

        /* Now kickoff dma */
        if(drm_WVOemCrypto_P_DecryptDMA_SG((uint8_t *)data_addr, out_buffer,
            data_length, subsample_flags, &block_offset, session, isSecureDecrypt) != Drm_Success)
        {
            BKNI_ReleaseMutex(gWVKeySlotMutex);
            BDBG_ERR(("%s: decryption failed for SG DMA",BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
            goto ErrorExit;
        }

        BKNI_ReleaseMutex(gWVKeySlotMutex);

        if(isSecureDecrypt)
        {
            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(NULL, NULL, 0, subsample_flags, false) != Drm_Success)
                {
                    BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",BSTD_FUNCTION));
                    rc = Drm_Err;
                    *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                    goto ErrorExit;
                }
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
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", BSTD_FUNCTION));
            }
        }
        BKNI_ReleaseMutex(gWVClrMutex);
    }

ErrorExit:

    if(hold_keyslot)
    {
        BKNI_ReleaseMutex(gWVSelectKeyMutex);
    }

    BDBG_MSG(("%s: Free the container",BSTD_FUNCTION));

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

    if(*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        *out_sz = 0;
        if(out_buffer && !isSecureDecrypt)
        {
            BKNI_Memset(out_buffer, 0x0, data_length);
        }
        rc = Drm_Err;
    }
    else
    {
        *out_sz = data_length;
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
    uint32_t i = 0;
    bool keyslot_found = false;

    BDBG_ENTER(drm_WVOemCrypto_DecryptCTR_V10);
    BDBG_MSG(("%s: Input data len=%d,is_encrypted=%d, sf:%d, secure:%d",
                BSTD_FUNCTION, data_length,is_encrypted,subsample_flags, isSecureDecrypt));

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - data buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    dump(data_addr,data_length,"encrypted data:");

    if(iv == NULL)
    {
        BDBG_ERR(("%s - iv buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer is NULL", BSTD_FUNCTION));
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
                BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",BSTD_FUNCTION));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                goto ErrorExit;
            }
        }
        else
        {
            BKNI_Memcpy(out_buffer,data_addr, data_length);
        }

        if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
        {
             /* Last subsample is clear. Transfer the encrypted block list */
             if(drm_WVOemCrypto_P_DecryptDMA_SG(NULL, NULL, 0,
                 subsample_flags, &block_offset, session, isSecureDecrypt) != Drm_Success)
             {
                 BDBG_ERR(("%s: decryption failed for SG DMA",BSTD_FUNCTION));
                 rc = Drm_Err;
                 *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                 goto ErrorExit;
             }
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
                BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
                    BDBG_ERR(("%s: Out of memory for IV (%u bytes)", BSTD_FUNCTION, WVCDM_KEY_IV_SIZE));
                    *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                    rc = Drm_Err;
                    goto ErrorExit;
                }
                container->blocks[0].len = WVCDM_KEY_IV_SIZE;
                BKNI_Memcpy(container->blocks[0].data.ptr, iv, WVCDM_KEY_IV_SIZE);
            }

            /* Allocate BTP buffer if needed */
            if (gHostSessionCtx[session].btp_sage_buffer_ptr == NULL)
            {
                /* Search for available BTP.  If not found, allocate. */
                for(i = 0; i < gKeySlotMaxAvail; i++)
                {
                    if(gKeySlotCache[i].hSwKeySlot != NULL)
                    {
                        if(gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot == gKeySlotCache[i].hSwKeySlot)
                        {
                            keyslot_found = true;
                            break;
                        }
                    }
                }

                if(!keyslot_found)
                {
                    BDBG_ERR(("%s: Unable to find selected key slot ID (%u)", BSTD_FUNCTION, gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot));
                    *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                    rc = Drm_Err;
                    goto ErrorExit;
                }

                if(gKeySlotCache[i].btp_sage_buffer == NULL)
                {
                     if(gEventDrivenVerify)
                     {
                         gKeySlotCache[i].btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_Shared);
                     }
                     else
                     {
                         gKeySlotCache[i].btp_sage_buffer = SRAI_Memory_Allocate(BTP_SIZE, SRAI_MemoryType_SagePrivate);
                     }

                     if(gKeySlotCache[i].btp_sage_buffer == NULL)
                     {
                         BDBG_ERR(("%s: Out of memory for BTP (%u bytes)", BSTD_FUNCTION, BTP_SIZE));
                         *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
                         rc = Drm_Err;
                         goto ErrorExit;
                     }
                }
                else
                {
                    /* Point to stored btp buffer */
                    gHostSessionCtx[session].btp_sage_buffer_ptr = gKeySlotCache[i].btp_sage_buffer;
                }
            }

            /* Map to parameters into srai_inout_container */
            container->basicIn[0] = session;
            container->basicIn[1] = WVCDM_KEY_IV_SIZE;
            container->basicIn[2] = buffer_type;

            container->basicIn[3]= BTP_SIZE;
            container->blocks[1].data.ptr = gHostSessionCtx[session].btp_sage_buffer_ptr;
            container->blocks[1].len = BTP_SIZE;

            /* This command loads the IV and validates key control */
            sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDecryptCTR_V10, container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error in DrmWVOEMCrypto_CommandId_eDecryptCENC", BSTD_FUNCTION));
                rc = Drm_Err;
                *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
                goto ErrorExit;
            }

            *wvRc = container->basicOut[2];
            if (container->basicOut[0] != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Command was sent successfully to load IV in decryptCENC but actual operation failed (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
                rc = Drm_Err;
                goto ErrorExit;
            }

            BDBG_MSG(("%s:extract wv error code",BSTD_FUNCTION));
            if(*wvRc != SAGE_OEMCrypto_SUCCESS)
            {
                BDBG_ERR(("%s:WVRC error",BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }

        /* Now kickoff dma */
        if(drm_WVOemCrypto_P_DecryptDMA_SG((uint8_t *)data_addr, out_buffer,
            data_length, subsample_flags, &block_offset, session, isSecureDecrypt) != Drm_Success)
        {
            BDBG_ERR(("%s: decryption failed for SG DMA",BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
            goto ErrorExit;
        }


        if(isSecureDecrypt)
        {
            if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
            {
                if (drm_WVOemCrypto_P_CopyBuffer_Secure_SG(NULL, NULL, 0, subsample_flags, false) != Drm_Success)
                {
                    BDBG_ERR(("%s: Operation failed for copy buffer SG Secure Buffer Type",BSTD_FUNCTION));
                    rc = Drm_Err;
                    *wvRc = SAGE_OEMCrypto_ERROR_DECRYPT_FAILED;
                    goto ErrorExit;
                }
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
                BDBG_ERR(("%s: Failed to verify DMA transfer complete", BSTD_FUNCTION));
            }
        }
        BKNI_ReleaseMutex(gWVClrMutex);
    }

ErrorExit:
    BDBG_MSG(("%s: Free the container",BSTD_FUNCTION));

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
        if(out_buffer && !isSecureDecrypt)
        {
            BKNI_Memset(out_buffer, 0x0, data_length);
        }
        *out_sz = 0;
    }
    else
    {
        *out_sz = data_length;
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
    uint8_t future_block_iv[WVCDM_KEY_IV_SIZE];
    Drm_WVOemCryptoNativeHandle *native_hdl = (Drm_WVOemCryptoNativeHandle *)out_buffer;
    bool native_hdl_locked = false;
    size_t dma_align = PADDING_SZ_16_BYTE;
    bool force_transfer = false;

    BDBG_ENTER(drm_WVOemCrypto_DecryptCENC);

    *out_sz = 0;
    BKNI_Memcpy(block_iv, &iv[0], WVCDM_KEY_IV_SIZE);

    if ((buffer_type == Drm_WVOEMCrypto_BufferType_Secure) &&
        (NEXUS_GetAddrType((const void *)src_ptr) == NEXUS_AddrType_eUnknown))
    {
       NEXUS_ClientConfiguration cc;
       NEXUS_MemoryAllocationSettings ms;
       NEXUS_Platform_GetClientConfiguration(&cc);
       NEXUS_Memory_GetDefaultAllocationSettings(&ms);
       ms.heap = cc.heap[1]; // NXCLIENT_FULL_HEAP
       gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex+1].size = (data_length + (dma_align-1)) & ~(dma_align-1);
       NEXUS_Memory_Allocate(gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex+1].size,
                             &ms,
                             (void **)&gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex+1].buffer);
       if (gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex+1].buffer == NULL)
       {
          BDBG_ERR(("%s: Failed to allocate bounce buffer %d (sz=%u)", BSTD_FUNCTION,
                    gWVDecryptBounceBufferIndex+1,
                    gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex+1].size));
          rc = Drm_MemErr;
          goto ErrorExit;
       }

       gWVDecryptBounceBufferIndex += 1;
       BKNI_Memcpy(gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex].buffer, src_ptr, data_length);
       src_ptr = (uint8_t *)gWVDecryptBounceBuffer[gWVDecryptBounceBufferIndex].buffer;
    }

    if (buffer_type == Drm_WVOEMCrypto_BufferType_Secure)
    {
       if (subsample_flags & WV_OEMCRYPTO_FIRST_SUBSAMPLE)
       {
          if (NEXUS_GetAddrType((const void *)dst_ptr) == NEXUS_AddrType_eUnknown)
          {
             NEXUS_Error nx_rc;
             NEXUS_BaseObjectId nx_oid = 0;
             // if we are passed in a native handle containing a nexus memory handle, we expect an opaque
             // handle redirection; used by android.
             nx_rc = NEXUS_Platform_GetIdFromObject((void *)native_hdl->data[0], &nx_oid);
             if (!nx_rc && nx_oid)
             {
                void *pMemory = NULL, *sMemory = NULL;
                nx_rc = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)native_hdl->data[0], &pMemory);
                if (nx_rc || pMemory == NULL)
                {
                   BDBG_ERR(("%s: unable to lock secure buffer (%p:%x) handle", BSTD_FUNCTION, out_buffer, native_hdl->data[0]));
                   rc = Drm_NexusErr;
                   goto ErrorExit;
                }
                native_hdl_locked = true;
                nx_rc = NEXUS_MemoryBlock_Lock(((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer, &sMemory);
                if (nx_rc || sMemory == NULL)
                {
                   BDBG_ERR(("%s: unable to lock srai buffer (%p:%x:%" PRIx64 ") handle", BSTD_FUNCTION,
                             out_buffer, native_hdl->data[0], ((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer));
                   rc = Drm_NexusErr;
                   goto ErrorExit;
                }
                dst_ptr = sMemory;
                NEXUS_MemoryBlock_Unlock(((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer);
                // keep copy of the base native handle memory pointer.
                gWVDecryptBufferNativeHandle = (uint8_t *)native_hdl;
                gWVDecryptBufferSecure = dst_ptr;
             }
          }
          // have been passed in a valid nexus address, go with it, skip any processing.
          else
          {
             gWVDecryptBufferNativeHandle = NULL;
             gWVDecryptBufferSecure = NULL;
          }
       }
       // native handle funky arithmetic.
       else if (gWVDecryptBufferNativeHandle != NULL)
       {
          size_t offset = dst_ptr - gWVDecryptBufferNativeHandle;
          dst_ptr = gWVDecryptBufferSecure + offset;
          BDBG_MSG(("%s: next sample destination (%p:%p) -> (%p:%x)", BSTD_FUNCTION,
                    gWVDecryptBufferSecure, gWVDecryptBufferNativeHandle, dst_ptr, offset));
       }
       // better be a valid nexus address.
       else
       {
          if (NEXUS_GetAddrType((const void *)dst_ptr) == NEXUS_AddrType_eUnknown)
          {
             BDBG_ERR(("%s: next sample: incorrect destination (%p)", BSTD_FUNCTION, dst_ptr));
             rc = Drm_NexusErr;
             goto ErrorExit;
          }
       }
    }

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
                if (gWVDecryptBounceBufferIndex >= MAX_SG_DMA_BLOCKS)
                {
                    /* Force all queued buffers to be transferred to prevent exceeding bounds */
                    force_transfer = true;
                }
            }
        }

        if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
        {
            BKNI_Memcpy(future_block_iv, src_ptr + blockDataLength - WVCDM_KEY_IV_SIZE, WVCDM_KEY_IV_SIZE);
        }

        rc = drm_WVOemCrypto_P_DecryptPatternBlock(session, src_ptr, blockDataLength,
            block_encrypted, buffer_type, block_iv, block_offset, dst_ptr, &block_out_sz,
            block_subsample_flags, force_transfer, wvRc);

        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s: Failed to decrypt subsample subpattern block", BSTD_FUNCTION));
            goto ErrorExit;
        }

        *out_sz += block_out_sz;

        /* Handle IV update for mid subsample transfers */
        if (block_encrypted && pPattern->skip > 0)
        {
            if (cipher_mode == Drm_WVOemCrypto_CipherMode_CBC)
            {
                BKNI_Memcpy(block_iv, future_block_iv, WVCDM_KEY_IV_SIZE);
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

    if (rc != Drm_Success || force_transfer || (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE))
    {
       if (gWVDecryptBounceBufferIndex >= 0)
       {
           for (int i = 0; (i <= gWVDecryptBounceBufferIndex) && (i < MAX_SG_DMA_BLOCKS) ; i++)
           {
              if (gWVDecryptBounceBuffer[i].buffer)
              {
                 NEXUS_Memory_Free(gWVDecryptBounceBuffer[i].buffer);
                 gWVDecryptBounceBuffer[i].buffer = NULL;
                 gWVDecryptBounceBuffer[i].size = 0;
              }
           }
           gWVDecryptBounceBufferIndex = -1;
       }
    }

    if ((subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE))
    {
       gWVDecryptBufferNativeHandle = NULL;
       gWVDecryptBufferSecure = NULL;
    }

    if (native_hdl_locked)
    {
       NEXUS_MemoryBlock_Unlock((NEXUS_MemoryBlockHandle)native_hdl->data[0]);
    }

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
        BDBG_ERR(("%s - keybox buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
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

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eInstallKeybox, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Installing key box", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to install keybox but actual operation failed (0x%08x)", BSTD_FUNCTION, sage_rc));
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
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eIsKeyboxValid, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error validating key box in sage. sage command failed", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s:Command was sent successfully to validate keybox but actual operation failed (0x%08x), wvRc = %d",
                            BSTD_FUNCTION, container->basicOut[0], *wvRc));
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
[in/out] idLength  on input, size of the callers device ID buffer. On output, the number of bytes
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
        BDBG_MSG(("%s - deviceID buffer is NULL", BSTD_FUNCTION));
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s: Out of memory for device ID (%u bytes)", BSTD_FUNCTION, SAGE_WVKBOX_DEVID_LEN));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = *idLength;

    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetDeviceID, container);
    *wvRc=container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Getting DeviceID with wvRC=%d", BSTD_FUNCTION,*wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }


    *idLength = container->basicOut[1];
    BDBG_MSG(("%s: idlength=%d",BSTD_FUNCTION,*idLength));


    if (*wvRc!= SAGE_OEMCrypto_SUCCESS)
    {
        rc =  Drm_Err;
        goto ErrorExit;
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to GetDeviceID but actual operation failed (0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    else
    {
        if(deviceID != NULL)
        {
            BKNI_Memcpy( deviceID, container->blocks[0].data.ptr, *idLength);
            BDBG_MSG(("%s: Device is %s",BSTD_FUNCTION,deviceID ));
        }
        else
        {
            BDBG_WRN(("%s:deviceID buffer is NULL!!!",BSTD_FUNCTION));
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

    BDBG_MSG(("%s: Exiting funciton",BSTD_FUNCTION));
    BDBG_LEAVE(drm_WVOemCrypto_GetDeviceID);

    return rc;
}



/********************************************************************************************************************
Description:
Decrypt and return the Key Data field from the Keybox.

Parameters:
[out] keyData pointer
to the buffer to hold the Key Data field from the Keybox
[in/out] keyDataLength  on input, the allocated buffer size. On output, the number of bytes in
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
        BDBG_ERR(("%s - keydata  is NULL ", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(keyDataLength == NULL)
    {
        BDBG_ERR(("%s - keyDataLength is NULL ", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container ", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(*keyDataLength != 0)
    {
        BDBG_MSG(("----------------%s: keyDataLength is %d",BSTD_FUNCTION,*keyDataLength));
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(*keyDataLength, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data (%u bytes)", BSTD_FUNCTION, *keyDataLength));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = *keyDataLength;
        BKNI_Memcpy(container->blocks[0].data.ptr, keyData, *keyDataLength);
    }
    else
    {
        BDBG_MSG(("%s: ------------Input keydata len is zero. lets get the needed size and set short buffer error",BSTD_FUNCTION));
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(SAGE_WVKBOX_KEYDATA_LEN, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data buffer (%u bytes)", BSTD_FUNCTION, SAGE_WVKBOX_KEYDATA_LEN));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = SAGE_WVKBOX_KEYDATA_LEN;
    }



    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetKeyData, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Getting key data", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:SRAI_Module_ProcessCommand completed ",BSTD_FUNCTION));

    /* if success, extract status from container */


    *wvRc = container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to GetKeyData but actual operation failed (0x%08x), wvRc=%d", BSTD_FUNCTION, container->basicOut[0],*wvRc));
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
        BDBG_MSG(("%s:copying %d bytes of keydata to host memory",BSTD_FUNCTION,*keyDataLength));
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

#if (NEXUS_SECURITY_API_VERSION==1)
{
    uint32_t tempSz=0;
    uint32_t ii = 0;

    BDBG_ENTER(drm_WVOemCrypto_GetRandom);

    if(dataLength >NEXUS_MAX_RANDOM_NUMBER_LENGTH)
    {
        BDBG_MSG(("%s - size greater than the HW limit, calling multiple times", BSTD_FUNCTION));
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
            /*BDBG_MSG(("%s - tempSz is %d", BSTD_FUNCTION, tempSz));*/
        }

        /*BDBG_MSG(("%s - sz is %d,random buffer is at 0x%x,  i:%d", BSTD_FUNCTION, tempSz, randomData+ii,ii));*/
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
}
#else
    BDBG_ENTER(drm_WVOemCrypto_GetRandom);

    rc = DRM_Common_GenerateRandomNumber(dataLength,randomData);
    if(rc!=Drm_Success){
        goto ErrorExit;
    }
#endif
ErrorExit:
    BDBG_LEAVE(drm_WVOemCrypto_GetRandom);
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
[out] wrappedKeybox  Pointer to wrapped keybox
[out] wrappedKeyboxLength  Pointer to the length of the wrapped keybox in bytes
[in] transportKey  Optional. AES transport key. If provided, the keybox parameter was
previously encrypted with this key. The keybox will be decrypted with the transport key using
AESCBC and a null IV.
[in] transportKeyLength  Optional. Number of bytes in the transportKey, if used.

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
        BDBG_ERR(("%s - keybox buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(wrappedKeybox!=NULL)
    {
        BDBG_ERR(("%s - wrappedKeybox buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(transportKey == NULL)
    {
        BDBG_ERR(("%s - transportKey is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(keyBoxLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(keyBoxLength, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for key data buffer (%u bytes)", BSTD_FUNCTION, SAGE_WVKBOX_KEYDATA_LEN));
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
            BDBG_ERR(("%s: Out of memory for wrapped keybox (%u bytes)", BSTD_FUNCTION, *wrappedKeyBoxLength));
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
            BDBG_ERR(("%s: Out of memory for transport key buffer (%u bytes)", BSTD_FUNCTION, transportKeyLength));
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[2].data.ptr,transportKey, transportKeyLength);
        container->blocks[2].len = transportKeyLength;
    }



    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eWrapKeybox, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling wrap key box", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to WrapKeybox but actual operation failed (0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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
            BSTD_FUNCTION,*wrapped_rsa_key_length,enc_rsa_key_length,message_length,signature_length ));

    if(message == NULL)
    {
        BDBG_ERR(("%s - message buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature ==NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(nonce == NULL)
    {
        BDBG_ERR(("%s - nonce is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(enc_rsa_key == NULL)
    {
        BDBG_ERR(("%s - enc_rsa_key buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(enc_rsa_key_iv == NULL)
    {
        BDBG_ERR(("%s - enc_rsa_key_iv buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container memory", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(message_length != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for message buffer (%u bytes)", BSTD_FUNCTION, message_length));
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
            BDBG_ERR(("%s: Out of memory for signature buffer (%u bytes)", BSTD_FUNCTION, signature_length));
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
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", BSTD_FUNCTION, enc_rsa_key_length));
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
        BDBG_ERR(("%s: Out of memory for rsa iv buffer (16 bytes)", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[3].data.ptr, enc_rsa_key_iv, 16);
    container->blocks[3].len = 16;

    if(wrapped_rsa_key == NULL)
    {
        BDBG_MSG(("%s - wrapped_rsa_key is null (SHORT_BUFFER)", BSTD_FUNCTION));
        container->blocks[4].data.ptr =NULL;
        container->blocks[4].len = 0;
    }
    else
    {
        BDBG_MSG(("%s - wrapped_rsa_key is not null, wrapped_rsa_key_length is %d ", BSTD_FUNCTION, *wrapped_rsa_key_length));
        container->blocks[4].data.ptr = SRAI_Memory_Allocate(*wrapped_rsa_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[4].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", BSTD_FUNCTION, *wrapped_rsa_key_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[4].len = *wrapped_rsa_key_length;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = *nonce;
    BDBG_MSG(("%s: nonce=%d",BSTD_FUNCTION,*nonce));


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eRewrapDeviceRSAKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling RewrapDeviceRSAKey", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =container->basicOut[2];
    *wrapped_rsa_key_length = container->basicOut[1];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        if(*wvRc == SAGE_OEMCrypto_ERROR_SHORT_BUFFER)
            BDBG_MSG(("%s - Command was sent successfully, and SHORT_BUFFER was returned", BSTD_FUNCTION));
        else
            BDBG_ERR(("%s - Command was sent successfully to RewrapDeviceRSAKey but actual operation failed (0x%08x), wvRC=%d", BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

     BDBG_MSG(("%s: sage returned wrapped key length as %d",BSTD_FUNCTION,container->basicOut[1]));

    /*copy the wrapped keybox back to the app buffer*/
     if(wrapped_rsa_key!=NULL){
        BKNI_Memcpy(wrapped_rsa_key, container->blocks[4].data.ptr, *wrapped_rsa_key_length);
     }

     if(*wvRc!=0)
     {
        BDBG_ERR(("%s:rewrap Device RSA key sage cmnd returned error rc =%d",BSTD_FUNCTION,*wvRc));
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
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(wrapped_rsa_key_length!=0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(wrapped_rsa_key_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s: Out of memory for rsa key buffer (%u bytes)", BSTD_FUNCTION, wrapped_rsa_key_length));
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr,wrapped_rsa_key, wrapped_rsa_key_length);
        container->blocks[0].len = wrapped_rsa_key_length;
    }

    container->basicIn[0] = session;
    container->basicIn[1] = wrapped_rsa_key_length;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadDeviceRSAKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    *wvRc =  container->basicOut[2];

    if(container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command sent to SAGE but actual operation failed (0x%08x), wvRc = %d", BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
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
        BDBG_ERR(("%s - message buffer is NULL or message length is invalid (%u)", BSTD_FUNCTION, message_length));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate shared memory buffers */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s: Out of memory for message buffer (%u bytes)", BSTD_FUNCTION, message_length));
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
                BDBG_ERR(("%s: Out of memory for signature buffer (%u bytes)", BSTD_FUNCTION, *signature_length));
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

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGenerateRSASignature, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling GenerateRSASignature", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *signature_length = container->basicOut[1] ;
    *wvRc =  container->basicOut[2];

    BDBG_MSG(("%s:return signature size is '%d' (wvRc = 0x%08x) ",BSTD_FUNCTION,*signature_length, (*wvRc)));

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        if (*wvRc == SAGE_OEMCrypto_ERROR_SHORT_BUFFER)
        {
            BDBG_MSG(("%s - Command was sent successfully, and SHORT_BUFFER was returned", BSTD_FUNCTION));
        }
        else
        {
            BDBG_ERR(("%s - Command was sent successfully to generate rsa signature but actual operation failed (0x%08x), wvRc = %d", BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
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
        BDBG_ERR(("%s - enc_session_key buffer is NULL", BSTD_FUNCTION));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(mac_key_context == NULL)
    {
        BDBG_ERR(("%s - mac_key_context buffer is NULL", BSTD_FUNCTION));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    if( enc_key_context == NULL)
    {
        BDBG_ERR(("%s -  enc_key_context is NULL", BSTD_FUNCTION));
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted session key", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for mac key context", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted mac key context (%u bytes)", BSTD_FUNCTION, enc_key_context_length));
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


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDeriveKeysFromSessionKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling Derive keys from Session", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc =  container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x),wvRC = %d", BSTD_FUNCTION, container->basicOut[0], *wvRc));
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
        BDBG_ERR(("%s - Error allocating SRAI container.", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetHDCPCapability, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error processing SAGE command.", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error in SAGE command. (0x%08x), wvRC = %d", BSTD_FUNCTION, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(*wvRc != 0)
    {
        BDBG_ERR(("%s - WV Error in SAGE command (0x%08x).", BSTD_FUNCTION, *wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *current = (uint32_t)container->basicOut[1];
    *maximum = (uint32_t)container->basicOut[3];

    BDBG_MSG(("%s - current: %d, max: %d.", BSTD_FUNCTION, *current, *maximum));

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
        BDBG_ERR(("%s - in_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if( iv == NULL)
    {
        BDBG_ERR(("%s -  iv is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if( buffer_length % AES_BLOCK_SIZE != 0 )
    {
        BDBG_MSG(("%s: buffers size bad.",BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for buffer (%u bytes)", BSTD_FUNCTION, buffer_length));
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
        BDBG_ERR(("%s - Error allocating memory for key/iv (%u bytes)", BSTD_FUNCTION, WVCDM_KEY_IV_SIZE));
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
        BDBG_ERR(("%s - Error allocating memory for large buffer (%u bytes)", BSTD_FUNCTION, buffer_length));
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
            BDBG_MSG(("%s: algo not supported",BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Encrypt, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling Generic_Encrypt", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc =  container->basicOut[2];

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to Generic_Encrypt but actual operation failed (0x%08x), wvRC = %d", BSTD_FUNCTION, container->basicOut[0],container->basicOut[2]));
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
        BDBG_ERR(("%s - in_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if( iv == NULL)
    {
        BDBG_ERR(("%s -  iv is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if(out_buffer == NULL)
    {
        BDBG_ERR(("%s - out_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit1;
    }

    if( buffer_length % AES_BLOCK_SIZE != 0 )
    {
        BDBG_MSG(("%s: buffers size bad.",BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        BDBG_MSG(("%s: buffers size bad.",BSTD_FUNCTION));
        goto ErrorExit1;
    }

    switch(algorithm)
    {

        case 0: /*aes cbc no padding OEMCrypto_AES_CBC_128_NO_PADDING*/
            break;
        case 1: /*OEMCrypto_HMAC_SHA256: HMAC SHA is not valid for encrypt operation hence send err*/
        default:
            BDBG_MSG(("%s: algo not supported",BSTD_FUNCTION));
            rc = Drm_Err;
            *wvRc=SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            goto ErrorExit1;
    }



    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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


    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Decrypt, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error processing Generic_decrypt command", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:extract status from container",BSTD_FUNCTION));
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    BDBG_MSG(("%s:extract WVRC status from container",BSTD_FUNCTION));
    *wvRc =  container->basicOut[2];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x), wvRC = %d", BSTD_FUNCTION, sage_rc,container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(*wvRc!=0)
    {
        rc =Drm_Err;
        goto ErrorExit;
    }
    BDBG_MSG(("%s:extract outbuffer from container",BSTD_FUNCTION));
    /*extract the outBuffer*/
    BKNI_Memcpy(out_buffer, container->blocks[2].data.ptr, buffer_length);

ErrorExit:
    BDBG_MSG(("%s:free container blks",BSTD_FUNCTION));
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

    BDBG_MSG(("%s:free container ",BSTD_FUNCTION));

ErrorExit1:
    BDBG_MSG(("%s:at ErrorExit1 ...",BSTD_FUNCTION));
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
        BDBG_ERR(("%s - in_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    switch(algorithm)
    {
    case 1:
        break;
    case 0:
    default:
        BDBG_MSG(("%s:Bad algo!!!!!!!!!!!!!!!",BSTD_FUNCTION));
        rc =Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit1;

    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
       BDBG_MSG(("%s:allocating buffer for signature",BSTD_FUNCTION));
        container->blocks[1].data.ptr = SRAI_Memory_Allocate( 32, SRAI_MemoryType_Shared);
        container->blocks[1].len = 32;
   }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = 2; /*for SAGE_Crypto_ShaVariant_eSha256*/

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Sign, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    BDBG_MSG(("%s:extract the wvrc",BSTD_FUNCTION));
    *wvRc =  container->basicOut[2];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully generic sign but actual operation failed (0x%08x), wvRC = %d", BSTD_FUNCTION, sage_rc, container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    else
    {
        BDBG_MSG(("%s: sage command sucessfully returned",BSTD_FUNCTION));
    }

    BDBG_MSG(("%s:extract the signature length",BSTD_FUNCTION));
    *signature_length = container->basicOut[1] ;

    BDBG_MSG(("%s:print the returned signature length",BSTD_FUNCTION));
    BDBG_MSG(("%s:sage returned signature length =%d",BSTD_FUNCTION,*signature_length));

    BDBG_MSG(("%scheck wvRc",BSTD_FUNCTION));

    if(*wvRc!=0)
    {
        rc =Drm_Err;
        goto ErrorExit;
    }

    if(signature!=NULL)
    {
        BDBG_MSG(("%s:copy signature",BSTD_FUNCTION));
        /*extract the outBuffer*/
        BKNI_Memcpy(signature, container->blocks[1].data.ptr,  32);
    }
ErrorExit:

    if(container!=NULL)
    {
        BDBG_MSG(("%s:free containser ",BSTD_FUNCTION));
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
        BDBG_ERR(("%s - in_buffer buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }


    if(signature == NULL)
    {
        BDBG_ERR(("%s - signature buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
        BDBG_MSG(("%s:Bad algo!!!!!!!!!!!!!!!",BSTD_FUNCTION));
        rc =Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = 2; /*algorithm;*/

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGeneric_Verify, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to DeriveKeysFromSessionKey but actual operation failed (0x%08x), wvRC = %d ", BSTD_FUNCTION, container->basicOut[0],container->basicOut[2]));
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
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", BSTD_FUNCTION, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[0].len = MAX_USAGE_TABLE_SIZE;
    BDBG_MSG(("%s -  %p -> length = '%u'", BSTD_FUNCTION, container->blocks[0].data.ptr, container->blocks[0].len));

    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current Epoch time = %u <<<<<<", BSTD_FUNCTION, (unsigned)current_epoch_time));

    /* send command to SAGE */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eUpdateUsageTable, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eUpdateUsageTable));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Possible encrypted Usage Table returned from SAGE */
    if(!gBigUsageTableFormat && container->basicOut[2] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[0].data.ptr, container->blocks[0].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
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
 * DRM_WVOemCrypto_DeactivateUsageEntry_V12
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
DrmRC DRM_WVOemCrypto_DeactivateUsageEntry_V12(uint8_t *pst,
                                           uint32_t pst_length,
                                           int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_epoch_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_DeactivateUsageEntry_V12);

    BDBG_ASSERT(wvRc); /* this is a programming error */

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", BSTD_FUNCTION, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Allocate memory for PST */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for PST (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", BSTD_FUNCTION, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[1].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", BSTD_FUNCTION, current_epoch_time));

    /* send command to SAGE */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry_V12, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry_V12));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if successful look at basicOut[x]?? */
    *wvRc = container->basicOut[2];

    /* Only if entry found and deleted do we overwrite the Usage Table on Rootfs */
    if(!gBigUsageTableFormat && container->basicOut[1] == OVERWRITE_USAGE_TABLE_ON_ROOTFS)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[1].data.ptr, container->blocks[1].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwriting Usage Table in rootfs (%s), wvRC = %d", BSTD_FUNCTION, strerror(errno),container->basicOut[2]));
            goto ErrorExit;
        }
    }

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x)",
                BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry_V12, container->basicOut[0]));
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


    BDBG_LEAVE(DRM_WVOemCrypto_DeactivateUsageEntry_V12);

    return rc;
}



/*********************************************************************************
 * DRM_WVOemCrypto_ReportUsage
 *
 * Increment Usage Tables generation number, sign, encrypt, and save the Usage Table.
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
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", BSTD_FUNCTION, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /* verify 'sessionContext' is valid or on SAGE side? or both? */

    /* if requesting size only then we don't need to send request to SAGE? */
    if(buffer == NULL || (*buffer_length) < sizeof(WvOEMCryptoPstReport))
    {
        BDBG_MSG(("%s - buffer is NULL or invalid input size (%u), therefore this is a size request (%u + %u)",
                BSTD_FUNCTION, (*buffer_length), sizeof(WvOEMCryptoPstReport), pst_length));
        (*buffer_length) = sizeof(WvOEMCryptoPstReport) + pst_length;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container->basicIn[0] = sessionContext;

    /* allocate memory for PST */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for PST (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_ERR(("%s - Error allocating memory for buffer (%u bytes)", BSTD_FUNCTION, (*buffer_length)));
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
        BDBG_ERR(("%s - Error allocating memory for returned Usage Table", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[2].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    container->basicIn[1] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", BSTD_FUNCTION, current_epoch_time));

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eReportUsage, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eReportUsage));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];

    BDBG_MSG(("%s - Copying (*buffer_length) = %u   *** container->blocks[1].len (%u bytes) wvRC =  %d", BSTD_FUNCTION, (*buffer_length), container->blocks[1].len, container->basicOut[2]));
    BKNI_Memcpy(buffer, container->blocks[1].data.ptr, container->blocks[1].len);

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eReportUsage, container->basicOut[0]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* At this point the session should already be associated with the given entry */

    /* If standard usage table, ALWAYS overwrite Usage Table, regardless if table changed */
    if(!gBigUsageTableFormat)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[2].data.ptr, container->blocks[2].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
            goto ErrorExit;
        }
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
 * Verifies the signature of the given message using the sessions mac_key[server]
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
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", BSTD_FUNCTION, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - Message buffer is NULL (%p) or length is 0 (%u)", BSTD_FUNCTION, message, message_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length != HMAC_SHA256_SIGNATURE_SIZE)
    {
        BDBG_ERR(("%s - Signature buffer is NULL (%p) or length is not 32 bytes (%u)", BSTD_FUNCTION, signature, signature_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }


    /* verify 'sessionContext' is valid or on SAGE side? or both? */

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for digest of PST (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_ERR(("%s - Error allocating memory for Message", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for Signature", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for Signature", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[3].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    BDBG_MSG(("%s - current Epoch time = '%u' <<<<<<", BSTD_FUNCTION, (unsigned)current_epoch_time));
    container->basicIn[1] = current_epoch_time;

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDeleteUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeleteUsageEntry));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d ",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeleteUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* not clear in the spec but only overwrite if entry found? or always (assuming always) */
    if(!gBigUsageTableFormat)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[3].data.ptr, container->blocks[3].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
            goto ErrorExit;
        }
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
    BDBG_MSG(("%s - Deleting tables from rootfs", BSTD_FUNCTION));
    if(remove(USAGE_TABLE_FILE_PATH) != 0){
        BDBG_ERR(("%s - error deleting '%s'from rootfs (%s)", BSTD_FUNCTION, USAGE_TABLE_FILE_PATH, strerror(errno)));
    }

    if(remove(USAGE_TABLE_BACKUP_FILE_PATH) != 0){
        BDBG_ERR(("%s - error deleting '%s'from rootfs (%s)", BSTD_FUNCTION, USAGE_TABLE_BACKUP_FILE_PATH, strerror(errno)));
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error in allocating memory for encrypted Usage Table (%u bytes on return)", BSTD_FUNCTION, MAX_USAGE_TABLE_SIZE));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    container->blocks[0].len = MAX_USAGE_TABLE_SIZE;

    current_epoch_time = time(NULL);
    container->basicIn[0] = current_epoch_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld'", BSTD_FUNCTION, current_epoch_time));

    /*
     * send command to SAGE to wipe Usage Table memory
     * */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDeleteUsageTable, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeleteUsageTable));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRC = %d ", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeleteUsageTable, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }


    /* overwrite before or after analyzing basicOut[0] */
    if(!gBigUsageTableFormat)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[0].data.ptr, container->blocks[0].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwriting Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
            goto ErrorExit;
        }
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
    BDBG_MSG(("%s - Attempting to read Usage Table at filepath '%s'", BSTD_FUNCTION, USAGE_TABLE_FILE_PATH));

    /* Verify backup file accessible */
    if(access(USAGE_TABLE_BACKUP_FILE_PATH, R_OK|W_OK) != 0)
    {
        BDBG_MSG(("%s - '%s' not detected or file is not read/writeable (errno = %s)", BSTD_FUNCTION, USAGE_TABLE_BACKUP_FILE_PATH, strerror(errno)));
        bUsageTableBackupExists = false;
        /* Continue onwards as main usage table may still be accessible */
    }
    else
    {
        bUsageTableBackupExists = true;
    }

    if(access(USAGE_TABLE_FILE_PATH, R_OK|W_OK) != 0)
    {
        BDBG_MSG(("%s - '%s' not detected or file is not read/writeable (errno = %s)", BSTD_FUNCTION, USAGE_TABLE_FILE_PATH, strerror(errno)));
        if(!bUsageTableBackupExists)
        {
            BDBG_MSG(("%s - Main and backup usage tables are not read/writeable (errno = %s)", BSTD_FUNCTION, strerror(errno)));
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
            BDBG_ERR(("%s - '%s' Error determine file size of bin file", BSTD_FUNCTION, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        if(filesize <= SHA256_DIGEST_SIZE || filesize > (MAX_USAGE_TABLE_SIZE + SHA256_DIGEST_SIZE))
        {
            BDBG_ERR(("%s - '%s' Invalid file size %u bytes", BSTD_FUNCTION, pActiveUsageTableFilePath, filesize));
            goto UseBackup;
        }

        DRM_Common_MemoryAllocate(&usage_table_buff, filesize);
        if(usage_table_buff == NULL)
        {
            BDBG_ERR(("%s - Error allocating '%u' bytes", BSTD_FUNCTION, filesize));
            rc = Drm_MemErr;
            goto ErrorExit;
        }

        fptr = fopen(pActiveUsageTableFilePath, "rb");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - %s Error opening drm bin file (%s)", BSTD_FUNCTION, pActiveUsageTableFilePath, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        read_size = fread(usage_table_buff, 1, filesize, fptr);
        if(read_size != filesize)
        {
            BDBG_ERR(("%s - %s Error reading Usage Table file size (%u != %u)", BSTD_FUNCTION, pActiveUsageTableFilePath, read_size, filesize));
            goto UseBackup;
        }

        /*
         * Verify integrity before sending to SAGE
         * */
        rc = DRM_Common_SwSha256(usage_table_buff, digest, filesize-SHA256_DIGEST_SIZE);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error calculating SHA of '%s'", BSTD_FUNCTION, pActiveUsageTableFilePath));
            goto UseBackup;
        }

        if(BKNI_Memcmp(digest, &usage_table_buff[filesize-SHA256_DIGEST_SIZE], SHA256_DIGEST_SIZE) != 0)
        {
            BDBG_ERR(("%s - Error comparing SHA of '%s'", BSTD_FUNCTION, pActiveUsageTableFilePath));
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
            BDBG_ERR(("%s - Error, unable to read a valid usage table.", BSTD_FUNCTION));
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
                             BSTD_FUNCTION, (*pUsageTableSharedMemorySize), filesize-SHA256_DIGEST_SIZE));
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
        BDBG_ERR(("%s - pointer to encrypted Usage Table buffer is NULL or length (%u) is invalid", BSTD_FUNCTION, encryptedUsageTableLength));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /*
     * Calculate SHA 256 diget
     * */
    rc = DRM_Common_SwSha256(pEncryptedUsageTable, digest, encryptedUsageTableLength);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calculating SHA", BSTD_FUNCTION));
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

        BDBG_MSG(("%s - Overwriting file '%s'", BSTD_FUNCTION, pActiveUsageTableFilePath));

        fptr = fopen(pActiveUsageTableFilePath, "w+b");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening Usage Table file (%s) in 'w+b' mode.  '%s'", BSTD_FUNCTION, pActiveUsageTableFilePath, strerror(errno)));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        /*
         * Write actual Usage Table data
         * */
        write_size = fwrite(pEncryptedUsageTable, 1, encryptedUsageTableLength, fptr);
        if(write_size != encryptedUsageTableLength)
        {
            BDBG_ERR(("%s - Error writing drm bin file size to rootfs (%u != %u)", BSTD_FUNCTION, write_size, encryptedUsageTableLength));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        write_size = fwrite(digest, 1, SHA256_DIGEST_SIZE, fptr);
        if(write_size != SHA256_DIGEST_SIZE)
        {
            BDBG_ERR(("%s - Error appending digest to Usage Table file (%ss) (%u != %u)", BSTD_FUNCTION, pActiveUsageTableFilePath, write_size, SHA256_DIGEST_SIZE));
            rc = Drm_FileErr;
            goto ErrorExit;
        }

        fd = fileno(fptr);
#ifdef ANDROID
        /* File system performance must be sufficient enough to sync at this point */
        fflush(fptr);
        fsync(fd);
#endif
        /* close file descriptor */
        fclose(fptr);
        fptr = NULL;
    }/* end of for */


ErrorExit:
    if(fptr != NULL)
    {
        fclose(fptr);
        fptr = NULL;
    }
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

static DrmRC DRM_WVOemCrypto_P_Query_Mgn_Initialized(void)
{
    DrmRC rc = Drm_Err;
    BERR_Code sage_rc = BERR_NOT_INITIALIZED;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED;
    uint32_t retries = 0;

    BDBG_ENTER(DRM_WVOemCrypto_P_Query_Mgn_Initialized);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    while(wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eIsMgnInitialized, container);
        if (sage_rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }

        /* If command sent, extract status from container */
        sage_rc = container->basicOut[0];
        wvRc = container->basicOut[2];

        retries++;
        if(retries >= MAX_INIT_QUERY_RETRIES)
        {
            BDBG_ERR(("%s - Unable to initialize MGN after %d queries", BSTD_FUNCTION, retries));
            break;
        }
        BKNI_Sleep(5);
    }

ErrorExit:

    if(wvRc == SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Success;
        gAntiRollbackHw = container->basicOut[1];
        if(!gAntiRollbackHw)
        {
            BDBG_WRN(("Anti-rollback HW unavailable"));
        }
    }

    BDBG_LEAVE(DRM_WVOemCrypto_P_Query_Mgn_Initialized);
    return rc;
}

static bool DRM_WVOemCrypto_P_Query_No_Write_Pending(void)
{
    bool rc = false;
    BERR_Code sage_rc = BERR_NOT_INITIALIZED;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED;
    uint32_t retries = 0;

    BDBG_ENTER(DRM_WVOemCrypto_P_Query_Write_Pending);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    while(wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eNoWritePending, container);
        if (sage_rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
            rc = Drm_Err;
            goto ErrorExit;
        }

        /* If command sent, extract status from container */
        sage_rc = container->basicOut[0];
        wvRc = container->basicOut[2];

        retries++;
        if(retries >= MAX_INIT_QUERY_RETRIES)
        {
            BDBG_ERR(("%s - Unable to clear pending write after %d queries", BSTD_FUNCTION, retries));
            break;
        }
        BKNI_Sleep(10);
    }

ErrorExit:

    if(wvRc == SAGE_OEMCrypto_SUCCESS)
    {
        rc = Drm_Success;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_P_Query_Write_Pending);
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
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetNumberOfOpenSessions, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent sucessfully but failed to GetNumberOfOpenSessions with err wvRC = %d", BSTD_FUNCTION,*wvRc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*noOfOpenSessions) = container->basicOut[1];

    BDBG_MSG(("%s: Number current open sessions = %d",BSTD_FUNCTION,container->basicOut[1] ));


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
        BDBG_ERR(("%s - Error allocating container in GetMaxNumberOfSessions", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetMaxNumberOfSessions, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - command sent successfully, but actual operation failed with err -(wvRC = %d)", BSTD_FUNCTION,*wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }



    /* if success, extract index from container */
    (*noOfMaxSessions) = container->basicOut[1];

    BDBG_MSG(("%s: Max no. of sessions supported = %d",BSTD_FUNCTION,container->basicOut[1] ));


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
    uint8_t *src_ptr = (uint8_t*)data_addr;
    uint8_t *dst_ptr = destination;
    Drm_WVOemCryptoNativeHandle *native_hdl = (Drm_WVOemCryptoNativeHandle *)destination;
    bool native_hdl_locked = false;
    size_t dma_align = PADDING_SZ_16_BYTE;

    if(data_addr == NULL)
    {
        BDBG_ERR(("%s - source buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if( data_length == 0)
    {
        BDBG_ERR(("%s copy buffer size is zero !!!!!!",BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if ((buffer_type == Drm_WVOEMCrypto_BufferType_Secure) &&
        (NEXUS_GetAddrType((const void *)src_ptr) == NEXUS_AddrType_eUnknown))
    {
       if (gWVCopyBounceBufferIndex+1 >= MAX_SG_DMA_BLOCKS)
       {
          BDBG_ERR(("%s: maximum bounce buffer count %d reached", BSTD_FUNCTION, gWVCopyBounceBufferIndex+1));
          // do not clean up already allocated bounce buffer.  this should never happen.
          rc = Drm_MemErr;
          goto ErrorExit;
       }

       NEXUS_ClientConfiguration cc;
       NEXUS_MemoryAllocationSettings ms;
       NEXUS_Platform_GetClientConfiguration(&cc);
       NEXUS_Memory_GetDefaultAllocationSettings(&ms);
       ms.heap = cc.heap[1]; // NXCLIENT_FULL_HEAP
       gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].size = (data_length + (dma_align-1)) & ~(dma_align-1);
       NEXUS_Memory_Allocate(gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].size,
                             &ms,
                             (void **)&gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].buffer);
       if (gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].buffer == NULL)
       {
          BDBG_ERR(("%s: Failed to allocate bounce buffer %d (sz=%u)", BSTD_FUNCTION,
                    gWVCopyBounceBufferIndex+1,
                    gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].size));
          gWVCopyBounceBuffer[gWVCopyBounceBufferIndex+1].size = 0;
          for (int i = 0; (i <= gWVCopyBounceBufferIndex) && (i < MAX_SG_DMA_BLOCKS) ; i++)
          {
             if (gWVCopyBounceBuffer[i].buffer)
             {
                NEXUS_Memory_Free(gWVCopyBounceBuffer[i].buffer);
                gWVCopyBounceBuffer[i].buffer = NULL;
                gWVCopyBounceBuffer[i].size = 0;
             }
          }
          gWVCopyBounceBufferIndex = -1;
          rc = Drm_MemErr;
          goto ErrorExit;
       }
       gWVCopyBounceBufferIndex += 1;
       BKNI_Memcpy(gWVCopyBounceBuffer[gWVCopyBounceBufferIndex].buffer, src_ptr, data_length);
       src_ptr = (uint8_t *)gWVCopyBounceBuffer[gWVCopyBounceBufferIndex].buffer;
    }

    dump(data_addr,data_length,"srcdata:");

    if(destination == NULL)
    {
        BDBG_ERR(("%s - destination buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }
    BDBG_MSG(("%s: Copying the buffer isSecureDecrypt %d....",BSTD_FUNCTION, isSecureDecrypt));

    if (buffer_type == Drm_WVOEMCrypto_BufferType_Secure)
    {
       if (subsample_flags & WV_OEMCRYPTO_FIRST_SUBSAMPLE)
       {
          if (NEXUS_GetAddrType((const void *)dst_ptr) == NEXUS_AddrType_eUnknown)
          {
             NEXUS_Error nx_rc;
             NEXUS_BaseObjectId nx_oid = 0;
             // if we are passed in a native handle containing a nexus memory handle, we expect an opaque
             // handle redirection; used by android.
             nx_rc = NEXUS_Platform_GetIdFromObject((void *)native_hdl->data[0], &nx_oid);
             if (!nx_rc && nx_oid)
             {
                void *pMemory = NULL, *sMemory = NULL;
                nx_rc = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)native_hdl->data[0], &pMemory);
                if (nx_rc || pMemory == NULL)
                {
                   BDBG_ERR(("%s: unable to lock secure buffer (%p:%x) handle", BSTD_FUNCTION, destination, native_hdl->data[0]));
                   rc = Drm_NexusErr;
                   goto ErrorExit;
                }
                native_hdl_locked = true;
                nx_rc = NEXUS_MemoryBlock_Lock(((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer, &sMemory);
                if (nx_rc || sMemory == NULL)
                {
                   BDBG_ERR(("%s: unable to lock srai buffer (%p:%x:%" PRIx64 ") handle", BSTD_FUNCTION,
                             destination, native_hdl->data[0], ((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer));
                   rc = Drm_NexusErr;
                   goto ErrorExit;
                }
                dst_ptr = sMemory;
                NEXUS_MemoryBlock_Unlock(((Drm_WVOemCryptoOpaqueHandle *)pMemory)->hBuffer);
                // keep copy of the base native handle memory pointer.
                gWVCopyBufferNativeHandle = (uint8_t *)native_hdl;
                gWVCopyBufferSecure = dst_ptr;
             }
          }
          // have been passed in a valid nexus address, go with it, skip any processing.
          else
          {
             gWVDecryptBufferNativeHandle = NULL;
             gWVDecryptBufferSecure = NULL;
          }
       }
       // native handle funky arithmetic.
       else if (gWVCopyBufferNativeHandle != NULL)
       {
          size_t offset = dst_ptr - gWVCopyBufferNativeHandle;
          dst_ptr = gWVCopyBufferSecure + offset;
          BDBG_MSG(("%s: next sample destination (%p:%p) -> (%p:%x)", BSTD_FUNCTION,
                    gWVCopyBufferSecure, gWVCopyBufferNativeHandle, dst_ptr, offset));
       }
       // better be a valid nexus address.
       else
       {
          if (NEXUS_GetAddrType((const void *)dst_ptr) == NEXUS_AddrType_eUnknown)
          {
             BDBG_ERR(("%s: next sample: incorrect destination (%p)", BSTD_FUNCTION, dst_ptr));
             rc = Drm_NexusErr;
             goto ErrorExit;
          }
       }
    }

    if (!isSecureDecrypt)
    {
       BKNI_Memcpy(dst_ptr, src_ptr, data_length);
    }
    else
    {
        rc = drm_WVOemCrypto_P_CopyBuffer_Secure_SG(dst_ptr, src_ptr,
            data_length, subsample_flags, true);
    }

    if ((subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE) &&
        (gWVCopyBounceBufferIndex >= 0))
    {
        for (int i = 0; (i <= gWVCopyBounceBufferIndex) && (i < MAX_SG_DMA_BLOCKS) ; i++)
        {
           if (gWVCopyBounceBuffer[i].buffer)
           {
              NEXUS_Memory_Free(gWVCopyBounceBuffer[i].buffer);
              gWVCopyBounceBuffer[i].buffer = NULL;
              gWVCopyBounceBuffer[i].size = 0;
           }
        }
        gWVCopyBounceBufferIndex = -1;
    }

    if (subsample_flags & WV_OEMCRYPTO_LAST_SUBSAMPLE)
    {
       gWVCopyBufferNativeHandle = NULL;
       gWVCopyBufferSecure = NULL;
    }

ErrorExit:

    if (native_hdl_locked)
    {
       NEXUS_MemoryBlock_Unlock((NEXUS_MemoryBlockHandle)native_hdl->data[0]);
    }

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
        BDBG_ERR(("%s - key_id buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if ((key_control_block_length == NULL)
          || (*key_control_block_length < WVCDM_KEY_CONTROL_SIZE))
    {
        BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
        *key_control_block_length = WVCDM_KEY_CONTROL_SIZE;
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER ;
        goto ErrorExit;
    }

    *key_control_block_length = WVCDM_KEY_CONTROL_SIZE;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container in QuerykeyControl", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", BSTD_FUNCTION, key_id_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
        container->blocks[0].len = key_id_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, key_id, key_id_length);
    }

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(*key_control_block_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for Message (%u bytes)", BSTD_FUNCTION, *key_control_block_length));
            rc = Drm_Err;
            goto ErrorExit;
        }
    /* allocate memory for returning the keycontrol block*/
    container->blocks[1].len = WVCDM_KEY_CONTROL_SIZE;
    BKNI_Memset(container->blocks[1].data.ptr, 0x0, *key_control_block_length);

    container->basicIn[0] = session;
    container->basicIn[1] = *key_control_block_length;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eQueryKeyControl, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
     *wvRc =    container->basicOut[2];
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to QueryControlBlock but actual operation failed (0x%08x), wvRC = %d ", BSTD_FUNCTION, sage_rc, container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }
    if(*wvRc != 0)
    {
        BDBG_ERR(("%s - failed with WvRc= (0x%08x)", BSTD_FUNCTION, *wvRc));
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
        BDBG_ERR(("%s - Invalid context. Provider Session Token is NULL (%p) or length is 0 (%u)", BSTD_FUNCTION, pst, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for digest of PST (%u bytes)", BSTD_FUNCTION, pst_length));
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
        BDBG_ERR(("%s - Error allocating memory for Signature", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[1].len = MAX_USAGE_TABLE_SIZE;

    /* fetch current epoch time */
    current_epoch_time = time(NULL);
    BDBG_MSG(("%s - current Epoch time = '%lu' <<<<<<", BSTD_FUNCTION, current_epoch_time));
    container->basicIn[0] = current_epoch_time;

    /*
     * send command to SAGE
     * */
    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command '%u' to SAGE", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    *wvRc = container->basicOut[2];

    /* if success, extract status from container */
    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eForceDeleteUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(*wvRc != 0)
    {
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* not clear in the spec but only overwrite if entry found? or always (assuming always) */
    if(!gBigUsageTableFormat)
    {
        rc = DRM_WvOemCrypto_P_OverwriteUsageTable(container->blocks[1].data.ptr, container->blocks[1].len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error overwrite Usage Table in rootfs (%s)", BSTD_FUNCTION, strerror(errno)));
            goto ErrorExit;
        }
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
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eSecurityPatchLevel, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eSecurityPatchLevel, container->basicOut[0], container->basicOut[2]));
        rc =Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from container */
    (*security_patch_level) = container->basicOut[1];

    BDBG_MSG(("%s: Security patch level = %d",BSTD_FUNCTION,container->basicOut[1] ));


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Security_Patch_Level);

   return rc;
}

DrmRC DRM_WVOemCrypto_Create_Usage_Table_Header(uint8_t *header_buffer, uint32_t *header_buffer_length, int* wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_Create_Usage_Table_Header);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(gSsdSupported)
    {
        rc = DRM_WVOemCrypto_P_Query_Mgn_Initialized();
        if(rc != Drm_Success)
        {
            /* We can continue onwards on failed initialization but anti rollback not available */
            BDBG_WRN(("%s - Unable to initialize the master generation number", BSTD_FUNCTION));
            rc = Drm_Success;
        }
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    if(header_buffer != NULL && *header_buffer_length)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(*header_buffer_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, *header_buffer_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, header_buffer, *header_buffer_length);

        current_time = time(NULL);
        container->basicIn[0] = current_time;
        BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));
    }
    else
    {
        /* We expect to fail with a short buffer error.  Send command to SAGE to retrieve required length. */
        container->blocks[0].data.ptr = NULL;
    }

    container->blocks[0].len = *header_buffer_length;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eCreateUsageTableHeader, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(*header_buffer_length < (uint32_t)container->basicOut[1])
    {
        BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
        *header_buffer_length = container->basicOut[1];
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    *header_buffer_length = container->basicOut[1];
    gLastHeaderLen = container->basicOut[1];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eCreateUsageTableHeader, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BKNI_Memcpy(header_buffer, container->blocks[0].data.ptr, *header_buffer_length);

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

    BDBG_LEAVE(DRM_WVOemCrypto_Create_Usage_Table_Header);

   return rc;
}

DrmRC DRM_WVOemCrypto_Load_Usage_Table_Header(const uint8_t *header_buffer, uint32_t header_buffer_length, int* wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_Load_Usage_Table_Header);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(gSsdSupported)
    {
        rc = DRM_WVOemCrypto_P_Query_Mgn_Initialized();
        if(rc != Drm_Success)
        {
            /* We can continue onwards on failed initialization but anti rollback not available */
            BDBG_WRN(("%s - Unable to initialize the master generation number", BSTD_FUNCTION));
            rc = Drm_Success;
        }
    }
    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(header_buffer == NULL || header_buffer_length == 0)
    {
        BDBG_ERR(("%s - Invalid buffer or buffer length", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(header_buffer_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, header_buffer_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = header_buffer_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, header_buffer, header_buffer_length);

    current_time = time(NULL);
    container->basicIn[0] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadUsageTableHeader, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadUsageTableHeader, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    gLastHeaderLen = header_buffer_length;

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

    BDBG_LEAVE(DRM_WVOemCrypto_Load_Usage_Table_Header);

   return rc;
}

DrmRC DRM_WVOemCrypto_Create_New_Usage_Entry(uint32_t session, uint32_t *usage_entry_number, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Create_New_Usage_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eCreateNewUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eCreateNewUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    *usage_entry_number = (uint32_t)container->basicOut[1];

    /* Invalidate the header buffer length cache due to new entry */
    gLastHeaderLen = 0;

 ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Create_New_Usage_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Load_Usage_Entry(uint32_t session, uint32_t usage_entry_number, const uint8_t *buffer, uint32_t buffer_length, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Load_Usage_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(buffer == NULL || buffer_length == 0)
    {
        BDBG_ERR(("%s - Invalid buffer or buffer length", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(buffer_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, buffer_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[0].len = buffer_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, buffer, buffer_length);

    container->basicIn[0] = session;
    container->basicIn[1] = usage_entry_number;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Invalidate the header buffer length cache due to new entry */
    gLastHeaderLen = 0;

    gLastEntryLen = buffer_length;

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

    BDBG_LEAVE(DRM_WVOemCrypto_Load_Usage_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Update_Usage_Entry(uint32_t session, uint8_t* header_buffer, uint32_t *header_buffer_length,
                                         uint8_t *entry_buffer, uint32_t *entry_buffer_length, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_Load_New_Usage_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(gLastHeaderLen > 0 && gLastEntryLen > 0)
    {
        if(*header_buffer_length < gLastHeaderLen || *entry_buffer_length < gLastEntryLen)
        {
            BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
            *header_buffer_length = gLastHeaderLen;
            *entry_buffer_length = gLastEntryLen;
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
            goto ErrorExit;
        }
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    if(header_buffer != NULL && *header_buffer_length)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(*header_buffer_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, *header_buffer_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, header_buffer, *header_buffer_length);
    }
    else
    {
        container->blocks[0].data.ptr = NULL;
    }

    container->blocks[0].len = *header_buffer_length;

    /*
     * allocate memory for entry buffer and copy to container
     * */
    if(entry_buffer != NULL && *entry_buffer_length)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(*entry_buffer_length, SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, *entry_buffer_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, entry_buffer, *entry_buffer_length);
    }
    else
    {
        container->blocks[1].data.ptr = NULL;
    }

    container->blocks[1].len = *entry_buffer_length;

    container->basicIn[0] = session;

    current_time = time(NULL);
    container->basicIn[1] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eUpdateUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    /* Check for short buffer*/
    if(*header_buffer_length < (uint32_t)container->basicOut[1])
    {
        BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
        *header_buffer_length = container->basicOut[1];
        *entry_buffer_length = container->basicOut[3];
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    if(*entry_buffer_length < (uint32_t)container->basicOut[3])
    {
        BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
        *header_buffer_length = container->basicOut[1];
        *entry_buffer_length = container->basicOut[3];
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    *header_buffer_length = container->basicOut[1];
    *entry_buffer_length = container->basicOut[3];

    gLastHeaderLen = container->basicOut[1];
    gLastEntryLen = container->basicOut[3];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eUpdateUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    BKNI_Memcpy(header_buffer, container->blocks[0].data.ptr, *header_buffer_length);
    BKNI_Memcpy(entry_buffer, container->blocks[1].data.ptr, *entry_buffer_length);

    /* We must verify decryption after an update to an entry*/
    gHostSessionCtx[session].force_decrypt_verify = true;

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
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Load_New_Usage_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Deactivate_Usage_Entry(uint32_t session, uint8_t *pst, uint32_t pst_length, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Deactivate_Usage_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid PST or PST length", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);
    container->blocks[0].len = pst_length;

    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eDeactivateUsageEntry, container->basicOut[0], container->basicOut[2]));
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

    BDBG_LEAVE(DRM_WVOemCrypto_Deactivate_Usage_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Shrink_Usage_Table_Header(uint32_t new_entry_count, uint8_t *header_buffer, uint32_t *header_buffer_length, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;

    BDBG_ENTER(DRM_WVOemCrypto_Shrink_Usage_Table_Header);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    if(header_buffer != NULL && *header_buffer_length > 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(*header_buffer_length, SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, *header_buffer_length));
            *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
            rc = Drm_Err;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, header_buffer, *header_buffer_length);
    }
    else
    {
        container->blocks[0].data.ptr = NULL;
    }

    container->blocks[0].len = *header_buffer_length;

    container->basicIn[0] = new_entry_count;

    current_time = time(NULL);
    container->basicIn[1] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eShrinkUsageTableHeader, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    /* Check for short buffer*/
    if(*header_buffer_length < (uint32_t)container->basicOut[1])
    {
        BDBG_MSG(("%s : OEMCrypto_ERROR_SHORT_BUFFER", BSTD_FUNCTION));
        *header_buffer_length = container->basicOut[1];
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_SHORT_BUFFER;
        goto ErrorExit;
    }

    *header_buffer_length = container->basicOut[1];
    gLastHeaderLen = container->basicOut[1];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eShrinkUsageTableHeader, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Copy over header */
    BKNI_Memcpy(header_buffer, (uint8_t*)container->blocks[0].data.ptr, *header_buffer_length);

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

    BDBG_LEAVE(DRM_WVOemCrypto_Shrink_Usage_Table_Header);

   return rc;
}

DrmRC DRM_WVOemCrypto_Move_Entry(uint32_t session, uint32_t new_index, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Move_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container->basicIn[0] = session;
    container->basicIn[1] = new_index;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eMoveEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eMoveEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }
 ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Move_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Copy_Old_Usage_Entry(uint32_t session, const uint8_t *pst, uint32_t pst_length, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_Copy_Old_Usage_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid PST or PST length", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for header buffer and copy to container
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pst, pst_length);
    container->blocks[0].len = pst_length;

    container->basicIn[0] = session;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eCopyOldUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eCopyOldUsageEntry, container->basicOut[0], container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Invalidate the header buffer length cache due to new entry */
    gLastHeaderLen = 0;

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

    BDBG_LEAVE(DRM_WVOemCrypto_Copy_Old_Usage_Entry);

   return rc;
}

DrmRC DRM_WVOemCrypto_Create_Old_Usage_Entry(uint64_t time_since_license_received,
                                             uint64_t time_since_first_decrypt,
                                             uint64_t time_since_last_decrypt,
                                             uint8_t status,
                                             uint8_t *server_mac_key,
                                             uint8_t *client_mac_key,
                                             const uint8_t *pst,
                                             uint32_t pst_length,
                                             int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    time_t current_time = 0;
    WvOEMCryptoPstReport *pReport;

    BDBG_ENTER(DRM_WVOemCrypto_Create_Test_Entry);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(pst == NULL || pst_length == 0)
    {
        BDBG_ERR(("%s - Invalid PST or PST length", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    /*
     * allocate memory for report
     * */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(sizeof(WvOEMCryptoPstReport), SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, sizeof(WvOEMCryptoPstReport)));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    pReport = (WvOEMCryptoPstReport *)container->blocks[0].data.ptr;
    BKNI_Memset(pReport, 0x0, sizeof(WvOEMCryptoPstReport));

    pReport->status = status;
    pReport->seconds_since_license_received = time_since_license_received;
    pReport->seconds_since_first_decrypt = time_since_first_decrypt;
    pReport->seconds_since_last_decrypt = time_since_last_decrypt;

    container->blocks[0].len = sizeof(WvOEMCryptoPstReport);

    /*
     * allocate memory for server mac key
     * */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(WVCDM_MAC_KEY_SIZE, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, WVCDM_MAC_KEY_SIZE));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[1].data.ptr, server_mac_key ,WVCDM_MAC_KEY_SIZE);
    container->blocks[1].len = WVCDM_MAC_KEY_SIZE;

    /*
     * allocate memory for client mac key
     * */
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(WVCDM_MAC_KEY_SIZE, SRAI_MemoryType_Shared);
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, WVCDM_MAC_KEY_SIZE));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[2].data.ptr, client_mac_key ,WVCDM_MAC_KEY_SIZE);
    container->blocks[2].len = WVCDM_MAC_KEY_SIZE;

    /*
     * allocate memory for pst
     * */
    container->blocks[3].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
    if(container->blocks[3].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for header buffer (%u bytes)", BSTD_FUNCTION, pst_length));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[3].data.ptr, pst, pst_length);
    container->blocks[3].len = pst_length;

    current_time = time(NULL);
    container->basicIn[0] = current_time;
    BDBG_MSG(("%s - current EPOCH time ld = '%ld' ", BSTD_FUNCTION, current_time));

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eCreateOldUsageEntry, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }

    /* If command sent, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];

    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command '%u' was sent successfully but SAGE specific error occured (0x%08x), wvRc = %d",
                    BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eCreateOldUsageEntry, container->basicOut[0], container->basicOut[2]));
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
        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if(container->blocks[2].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        if(container->blocks[3].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_Copy_Old_Usage_Entry);

   return rc;
}

bool DRM_WVOemCrypto_IsAntiRollbackHwPresent(void)
{
    return gAntiRollbackHw;
}

DrmRC DRM_WVOemCrypto_GetAnalogOutputFlags(uint32_t *output_flags, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(DRM_WVOemCrypto_GetAnalogOutputFlags);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container in GetAnalogOutputFlags", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc =SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eGetAnalogOutputFlags, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INIT_FAILED ;
        goto ErrorExit;
    }
    /* if success, extract status from container */
    sage_rc = container->basicOut[0];
    *wvRc = container->basicOut[2];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - command sent successfully, but actual operation failed with err -(wvRC = %d)", BSTD_FUNCTION,*wvRc));
        rc =Drm_Err;
        goto ErrorExit;
    }



    /* if success, extract index from container */
    (*output_flags) = container->basicOut[1];

    BDBG_MSG(("%s: analog output flags = 0x%x",BSTD_FUNCTION,container->basicOut[1] ));


ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_WVOemCrypto_GetAnalogOutputFlags);

   return rc;
}

DrmRC DRM_WVOemCrypto_LoadEntitledContentKeys(uint32_t session,
                                              uint32_t num_keys,
                                              void* key_array,
                                              int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    uint32_t key_object_shared_block_length = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoSageEntitledContentKeyObject* pKeyObj = NULL;
    Drm_WVOemCryptoEntitledContentKeyObject* keyArray = (Drm_WVOemCryptoEntitledContentKeyObject*)key_array;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t key_object_length = 0;

    BDBG_ENTER(DRM_WVOemCrypto_LoadEntitledContentKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate for sending key object data*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageEntitledContentKeyObject)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageEntitledContentKeyObject)) * num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[0].len = ((sizeof(Drm_WVOemCryptoSageEntitledContentKeyObject)) * num_keys);

    /* allocate for sending key object meta data*/
    key_array_sz = getSizeOfEntitledContentKeyObjectArray(keyArray, num_keys);
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(key_array_sz, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[1].data.ptr, 0xff, key_array_sz);

    /*
     * Fill in Key Array
     * */
    for(i = 0, key_object_shared_block_length = 0; i < num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d' entitlement key id length is %d",
            BSTD_FUNCTION, i, key_object_shared_block_length, keyArray[i].entitlement_key_id_length));

        /* copy entitlement key id  */
        BKNI_Memcpy(container->blocks[1].data.ptr + key_object_shared_block_length,
            keyArray[i].entitlement_key_id,
            keyArray[i].entitlement_key_id_length);

        key_object_shared_block_length += keyArray[i].entitlement_key_id_length;

        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d' content key id length is %d",
            BSTD_FUNCTION, i, key_object_shared_block_length, keyArray[i].content_key_id_length));

        /* copy content key id  */
        BKNI_Memcpy(container->blocks[1].data.ptr + key_object_shared_block_length,
            keyArray[i].content_key_id,
            keyArray[i].content_key_id_length);

        key_object_shared_block_length += keyArray[i].content_key_id_length;

        BDBG_MSG(("copy the key data iv at offset %d",key_object_shared_block_length));

        /* copy key_data_iv */
        BKNI_Memcpy(&container->blocks[1].data.ptr[key_object_shared_block_length],
                    keyArray[i].content_key_data_iv,
                    WVCDM_KEY_IV_SIZE);

        key_object_shared_block_length += WVCDM_KEY_IV_SIZE;

        BDBG_MSG(("copy the key data at offset %d",key_object_shared_block_length));

        /* copy key_data */
        BKNI_Memcpy(&container->blocks[1].data.ptr[key_object_shared_block_length],
                    keyArray[i].content_key_data,
                    keyArray[i].content_key_data_length);

        key_object_shared_block_length += keyArray[i].content_key_data_length;
    }

    /* update shared block length */
    if(key_array_sz != key_object_shared_block_length)
    {
        BDBG_ERR(("%s - previous length of key array size '%u' is no longer equal to '%u'",
            BSTD_FUNCTION, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[1].len = key_object_shared_block_length;

    pKeyObj = (Drm_WVOemCryptoSageEntitledContentKeyObject*)container->blocks[0].data.ptr;

    key_object_length = container->blocks[1].len / num_keys;

    for(i = 0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].entitlement_key_id = (uint32_t)&container->blocks[1].data.ptr[(i*key_object_length)];

        pKeyObj[i].entitlement_key_id_length = keyArray[i].entitlement_key_id_length;

        pKeyObj[i].content_key_id = (uint32_t)&container->blocks[1].data.ptr[(i*key_object_length)
                                                                  + keyArray[i].entitlement_key_id_length];

        pKeyObj[i].content_key_id_length = keyArray[i].content_key_id_length;

        pKeyObj[i].content_key_data_iv = (uint32_t)&container->blocks[1].data.ptr[(i*key_object_length)
                                                               + keyArray[i].entitlement_key_id_length
                                                               + keyArray[i].content_key_id_length];

        pKeyObj[i].content_key_data = (uint32_t)&container->blocks[1].data.ptr[(i*key_object_length)
                                                             + keyArray[i].entitlement_key_id_length
                                                             + keyArray[i].content_key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].content_key_data_length = keyArray[i].content_key_data_length;
    }

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadEntitledContentKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadEntitledContentKeys));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(DRM_WVOemCrypto_LoadEntitledContentKeys);
    return rc;
}

DrmRC drm_WVOemCrypto_SelectKey(const uint32_t session,
                                const uint8_t* key_id,
                                uint32_t key_id_length,
                                uint32_t cipher_mode,
                                int *wvRc)
{
    DrmRC rc = Drm_Success;
    BKNI_AcquireMutex(gWVSelectKeyMutex);
    rc = drm_WVOemCrypto_P_Do_SelectKey(session, key_id, key_id_length, cipher_mode, wvRc);
    BKNI_ReleaseMutex(gWVSelectKeyMutex);
    return rc;
}

DrmRC drm_WVOemCrypto_P_Do_SelectKey(const uint32_t session,
                                     const uint8_t* key_id,
                                     uint32_t key_id_length,
                                     uint32_t cipher_mode,
                                     int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
#else
    NEXUS_KeySlotAllocateSettings keyslotSettings;
    NEXUS_KeySlotInformation keyslotInfo;
#endif
    BSAGElib_InOutContainer *container = NULL;
    uint32_t keySlotIdSelected;
    NEXUS_KeySlotHandle hKeySlotSelected = NULL;
    uint32_t keySlotID[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
    bool allocate_slot = false;
    uint32_t i;

    BDBG_ENTER(drm_WVOemCrypto_P_Do_SelectKey);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(key_id == NULL)
    {
        BDBG_ERR(("%s - key_id buffer is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:session =%d, key_id_len=%d, key central cache mode=%d",BSTD_FUNCTION,session,key_id_length, gCentralKeySlotCacheMode));

    gHostSessionCtx[session].session_id=session;

    BKNI_Memcpy(gHostSessionCtx[session].key_id, key_id, key_id_length);
    gHostSessionCtx[session].key_id_length =key_id_length;

    /* A central key cache should utilize the maximum amount of slots */
    if(gCentralKeySlotCacheMode && gKeySlotCacheAllocated < gKeySlotMaxAvail)
    {
        allocate_slot = true;
    }

    /* Session based key cache needs only a one time allocation per session */
    if(!gCentralKeySlotCacheMode)
    {
        if(gHostSessionCtx[session].num_key_slots < DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
        {
            allocate_slot = true;
        }
    }

    if(allocate_slot)
    {
        BKNI_AcquireMutex(gWVKeySlotMutex);
        for(i = 0; i < gKeySlotMaxAvail; i++)
        {
            if(gKeySlotCache[i].hSwKeySlot == NULL)
            {
                BDBG_MSG(("%s:Allocate keyslot for index %d",BSTD_FUNCTION, i));
#if (NEXUS_SECURITY_API_VERSION==1)
                NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
                keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
                keyslotSettings.client = NEXUS_SecurityClientType_eSage;
                gKeySlotCache[i].hSwKeySlot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
#else
                NEXUS_KeySlot_GetDefaultAllocateSettings(&keyslotSettings);
                keyslotSettings.useWithDma = true;
                keyslotSettings.owner = NEXUS_SecurityCpuContext_eSage;
                keyslotSettings.slotType   = NEXUS_KeySlotType_eIvPerBlock;
                gKeySlotCache[i].hSwKeySlot =  NEXUS_KeySlot_Allocate(&keyslotSettings);
#endif
                if(gKeySlotCache[i].hSwKeySlot == NULL)
                {
                    if(gHostSessionCtx[session].num_key_slots > 0 ||
                        (gCentralKeySlotCacheMode && gKeySlotCacheAllocated > 0))
                    {
                        /* Continue onwards if we allocated at least one key slot */
                        BDBG_WRN(("%s - Unable to allocate keyslot at index %d, continuing with %d keyslots allocated", BSTD_FUNCTION, i, gKeySlotCacheAllocated));
                        break;
                    }
                    else
                    {
                        BKNI_ReleaseMutex(gWVKeySlotMutex);
                        BDBG_ERR(("%s - Error allocating keyslot at index %d, %d keyslots allocated", BSTD_FUNCTION, i, gKeySlotCacheAllocated));
                        rc = BERR_INVALID_PARAMETER;
                        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
                        goto ErrorExit;
                    }
                }

                BDBG_MSG(("%s: ======Allocated nexus key slot for index %d, keyslot handle=%p =======",
                    BSTD_FUNCTION, i, (void *)gKeySlotCache[i].hSwKeySlot));
#if (NEXUS_SECURITY_API_VERSION==1)
                NEXUS_Security_GetKeySlotInfo(gKeySlotCache[i].hSwKeySlot, &keyslotInfo);
                gKeySlotCache[i].keySlotID = keyslotInfo.keySlotNumber;
                BDBG_MSG(("%s - keyslotID[%d] Keyslot number = '%u'", BSTD_FUNCTION, i, keyslotInfo.keySlotNumber));
#else
                NEXUS_KeySlot_GetInformation( gKeySlotCache[i].hSwKeySlot, &keyslotInfo);
                gKeySlotCache[i].keySlotID = keyslotInfo.slotNumber;
                BDBG_MSG(("%s - keyslotID[%d] Keyslot number = '%u'", BSTD_FUNCTION, i, keyslotInfo.slotNumber));
#endif
                gHostSessionCtx[session].key_slot_ptr[gHostSessionCtx[session].num_key_slots] = &gKeySlotCache[i];
                gHostSessionCtx[session].num_key_slots++;

                gKeySlotCacheAllocated++;

                if(!gCentralKeySlotCacheMode && gHostSessionCtx[session].num_key_slots >= DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT)
                {
                    /* Cap the amount of slots to allocate in session key cache mode */
                    break;
                }
            }
        }
        BKNI_ReleaseMutex(gWVKeySlotMutex);
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
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
           BDBG_ERR(("%s - Error accessing key slot", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)", BSTD_FUNCTION, key_id_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        container->blocks[0].len = key_id_length;
        BKNI_Memcpy(container->blocks[0].data.ptr, key_id, key_id_length);
        dump(container->blocks[0].data.ptr,key_id_length,"key id in sage mem:");
    }

    /* Provide allocated key slots */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating mem to container block 0 (%u bytes)",
            BSTD_FUNCTION, sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[1].len = sizeof(keySlotID[0]) * DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT;

    BKNI_Memcpy(container->blocks[1].data.ptr, keySlotID,
        sizeof(keySlotID[0]) * gHostSessionCtx[session].num_key_slots);
    dump(container->blocks[1].data.ptr, sizeof(keySlotID[0]) * gHostSessionCtx[session].num_key_slots,"key slot ID in sage mem:");

    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = key_id_length;
    container->basicIn[2] = gHostSessionCtx[session].num_key_slots;
    container->basicIn[3] = cipher_mode;

    /* Set to an invalid value to see if key slot was selected */
    container->basicOut[3] = INVALID_KEYSLOT_ID;

    BKNI_AcquireMutex(gWVKeySlotMutex);

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eSelectKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:Sage process command is sucesss, next extract the status of the command",BSTD_FUNCTION));

    *wvRc = container->basicOut[2];
    BDBG_MSG(("%s:* WVRC=%d",BSTD_FUNCTION,*wvRc));

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Command was sent successfully to loadkey but actual operation failed (0x%08x, 0x%08x)", BSTD_FUNCTION, container->basicOut[0],
            container->basicOut[2]));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if (container->basicOut[3] == INVALID_KEYSLOT_ID)
    {
        /* If key slot was not selected but a successful operation, an AES decryption key was not selected.
         * Key slot is expected to not be selected in this scenario.
         */
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        goto IgnoreKeySlot;
    }

    keySlotIdSelected = container->basicOut[3];

    for(i = 0; i < DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT; i++)
    {
        if(gKeySlotCache[i].hSwKeySlot != NULL && keySlotIdSelected == gKeySlotCache[i].keySlotID)
        {
           hKeySlotSelected = gKeySlotCache[i].hSwKeySlot;
           break;
        }
    }
    if(i == DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT)
    {
        BKNI_ReleaseMutex(gWVKeySlotMutex);
        BDBG_ERR(("%s - Error unknown keyslot %d selected", BSTD_FUNCTION, keySlotIdSelected));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* Ensure key slot is unique within the session list */
    for(i = 0; i < gNumSessions; i++)
    {
        if(gHostSessionCtx[i].drmCommonOpStruct.keyConfigSettings.keySlot == hKeySlotSelected)
        {
            /* Set session key slot pointer to NULL as it was overwritten in cache.
             * Select key operation must be called again prior to decryption */
            gHostSessionCtx[i].drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
        }
    }
    /* Assign the key slot handle to the session */
    gHostSessionCtx[session].drmCommonOpStruct.keyConfigSettings.keySlot = hKeySlotSelected;

#if (NEXUS_SECURITY_API_VERSION==1)
    BDBG_MSG(("%s - Selected by Sage: Keyslot number = '%u'", BSTD_FUNCTION, keyslotInfo.keySlotNumber));
#else
    BDBG_MSG(("%s - Selected by Sage: Keyslot number = '%u'", BSTD_FUNCTION, keyslotInfo.slotNumber));
#endif

    BKNI_ReleaseMutex(gWVKeySlotMutex);

    /* Set the cipher mode */
    gHostSessionCtx[session].cipher_mode = cipher_mode;
    gHostSessionCtx[session].key_select_count++;
    gHostSessionCtx[session].btp_sage_buffer_ptr = NULL;

IgnoreKeySlot:
ErrorExit:

    BDBG_MSG(("%s: free container block 0:",BSTD_FUNCTION));
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

    BDBG_MSG(("%s:set the return value based on wvRc",BSTD_FUNCTION));
    if (*wvRc!= 0)
    {
        rc = Drm_Err;
    }

    BDBG_LEAVE(drm_WVOemCrypto_P_Do_SelectKey);

   return rc;
}

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
                               const uint8_t* srm_requirement,
                               uint32_t       license_type,
                               int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    uint32_t i = 0;
    uint32_t key_object_shared_block_length = 0;
    uint32_t key_array_sz = 0;
    Drm_WVOemCryptoSageKeyObject* pKeyObj = NULL;
    Drm_WVOemCryptoKeyObject* keyArray = (Drm_WVOemCryptoKeyObject*)key_array;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadKeys);

    *wvRc = SAGE_OEMCrypto_SUCCESS;

    if(message == NULL || message_length == 0)
    {
        BDBG_ERR(("%s - message buffer (%p) is NULL or message length (%u)", BSTD_FUNCTION, message, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(signature == NULL || signature_length == 0)
    {
        BDBG_ERR(("%s - signature buffer (%p) is NULL or length is 0 (%u)", BSTD_FUNCTION, signature, signature_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    if(key_array == NULL)
    {
        BDBG_ERR(("%s - key_array is NULL", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INVALID_CONTEXT;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating SRAI container", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(message_length, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for message data (%u bytes)", BSTD_FUNCTION, message_length));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    container->blocks[0].len = message_length;
    BKNI_Memcpy(container->blocks[0].data.ptr, message, message_length);


    container->blocks[1].data.ptr = SRAI_Memory_Allocate(signature_length, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for signature data", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC IV", BSTD_FUNCTION));
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
            BDBG_ERR(("%s - Error allocating memory for encrypted MAC key", BSTD_FUNCTION));
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
        BDBG_ERR(("%s - Error allocating memory for key object data (%u bytes)", BSTD_FUNCTION, key_array_sz));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    BKNI_Memset(container->blocks[4].data.ptr, 0xff, key_array_sz);


    /* allocate for sending key object data*/
    container->blocks[5].data.ptr = SRAI_Memory_Allocate(((sizeof(Drm_WVOemCryptoSageKeyObject)) * num_keys), SRAI_MemoryType_Shared);
    if (container->blocks[5].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating memory for key objects (%u bytes)", BSTD_FUNCTION, ((sizeof(Drm_WVOemCryptoSageKeyObject)) * num_keys)));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    container->blocks[5].len = ((sizeof(Drm_WVOemCryptoSageKeyObject)) * num_keys);

    /*
     * Allocate memory for PST (if applicable)
     * */
    if(pst_length > 0)
    {
        container->blocks[6].data.ptr = SRAI_Memory_Allocate(pst_length, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for PST digest data (%u bytes)", BSTD_FUNCTION, pst_length));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[6].data.ptr, pst, pst_length);
        container->blocks[6].len = pst_length ;
    }

    if(srm_requirement)
    {
        container->blocks[7].data.ptr = SRAI_Memory_Allocate(WVCDM_KEY_CONTROL_SIZE, SRAI_MemoryType_Shared);
        if(container->blocks[6].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating memory for SRM requirement data (%u bytes)", BSTD_FUNCTION, WVCDM_KEY_CONTROL_SIZE));
            rc = Drm_Err;
            *wvRc = SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES;
            goto ErrorExit;
        }

        BKNI_Memcpy(container->blocks[7].data.ptr, srm_requirement, WVCDM_KEY_CONTROL_SIZE);
        container->blocks[7].len = WVCDM_KEY_CONTROL_SIZE;
    }

    /*
     * Fill in Key Array
     * */
    for(i=0,key_object_shared_block_length=0; i < num_keys; i++)
    {
        BDBG_MSG(("%s:loop=%d,key_object_shared_block_length ='%d'   key id length is %d",BSTD_FUNCTION, i,key_object_shared_block_length,keyArray[i].key_id_length));
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
                    WVCDM_KEY_CONTROL_SIZE);

        key_object_shared_block_length += WVCDM_KEY_CONTROL_SIZE;
    }

    /* update shared block length */
    if(key_array_sz != key_object_shared_block_length)
    {
        BDBG_ERR(("%s - previous length of key array size  '%u' is no longer equal to '%u'", BSTD_FUNCTION, key_array_sz, key_object_shared_block_length));
        rc = Drm_Err;
        goto ErrorExit;
    }
    container->blocks[4].len = key_object_shared_block_length;



    pKeyObj = (Drm_WVOemCryptoSageKeyObject*)container->blocks[5].data.ptr;

    for (i=0; i < num_keys; i++)
    {
        BDBG_MSG(("Loop %d",i));
        pKeyObj[i].key_id = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys];

        pKeyObj[i].key_id_length = keyArray[i].key_id_length;

        pKeyObj[i].key_data_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                               + keyArray[i].key_id_length];

        pKeyObj[i].key_data = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                             + keyArray[i].key_id_length
                                                             + WVCDM_KEY_IV_SIZE];

        pKeyObj[i].key_data_length = keyArray[i].key_data_length;

        pKeyObj[i].key_control_iv = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                   + keyArray[i].key_id_length
                                                                   + WVCDM_KEY_IV_SIZE
                                                                   + keyArray[i].key_data_length];

        pKeyObj[i].key_control = (uint32_t)&container->blocks[4].data.ptr[(i*container->blocks[4].len)/num_keys
                                                                + keyArray[i].key_id_length
                                                                + (2*WVCDM_KEY_IV_SIZE)
                                                                + keyArray[i].key_data_length];
    }


    /* map to parameters into srai_inout_container */
    container->basicIn[0] = session;
    container->basicIn[1] = num_keys;
    container->basicIn[2] = license_type;

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadKeys, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters (command id = %u)", BSTD_FUNCTION, DrmWVOEMCrypto_CommandId_eLoadKeys));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
    }

    if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Load Key command failed due to SAGE issue (basicOut[0] = 0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
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

DrmRC drm_WVOemCrypto_LoadTestKeybox(uint8_t *keybox, uint32_t keyBoxLength, int *wvRc)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(drm_WVOemCrypto_LoadTestKeybox);

    if(keybox == NULL)
    {
        BDBG_ERR(("%s - keybox buffer is NULL", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_KEYBOX_INVALID;
        rc = Drm_Err;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error loading key parameters", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
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

    sage_rc = SRAI_Module_ProcessCommand(gWVmoduleHandle, DrmWVOEMCrypto_CommandId_eLoadTestKeybox, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error Installing key box", BSTD_FUNCTION));
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    sage_rc = container->basicOut[0];

    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully to load test keybox but actual operation failed (0x%08x)", BSTD_FUNCTION, sage_rc));
        *wvRc = container->basicOut[2];
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from container */
    *wvRc = container->basicOut[2];
    if (*wvRc != SAGE_OEMCrypto_SUCCESS)
    {
        BDBG_ERR(("%s - widevine return code (0x%08x)", BSTD_FUNCTION, *wvRc));
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

    BDBG_LEAVE(drm_WVOemCrypto_LoadTestKeybox);

    return rc;
}

DrmRC DRM_WVOemCrypto_LoadCasECMKeys(uint32_t session,
                                     uint16_t program_id,
                                     void* even_key,
                                     void* odd_key,
                                     int *wvRc)
{
    DrmRC rc = Drm_Success;
    Drm_WVOemCryptoEntitledContentKeyObject key_array[2];
    uint32_t num_keys = 0;

    BSTD_UNUSED(program_id);

    /* Check for at least one valid key */
    if(even_key == NULL && odd_key == NULL)
    {
        BDBG_ERR(("%s - No keys available", BSTD_FUNCTION));
        rc = Drm_Err;
        *wvRc = SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE;
        goto ErrorExit;
    }

    if(even_key)
    {
        BKNI_Memcpy(&key_array[num_keys], even_key, sizeof(Drm_WVOemCryptoEntitledContentKeyObject));
        num_keys++;
    }

    if(odd_key)
    {
        BKNI_Memcpy(&key_array[num_keys], odd_key, sizeof(Drm_WVOemCryptoEntitledContentKeyObject));
        num_keys++;
    }

    rc = DRM_WVOemCrypto_LoadEntitledContentKeys(session, num_keys, (void*)key_array, wvRc);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Failed to load ECM keys, wvRc=%d", BSTD_FUNCTION, *wvRc));
        goto ErrorExit;
    }

ErrorExit:
    return rc;
}
