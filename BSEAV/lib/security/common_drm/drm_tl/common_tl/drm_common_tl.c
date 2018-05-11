/***************************************************************************
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
 ***************************************************************************/
#include <errno.h>
#include <string.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bcrypt.h"
#include "bcrypt_rsa_sw.h"
#include "bcrypt_sha256_sw.h"

#include "drm_metadata_tl.h"
#include "drm_types.h"
#include "drm_common_priv.h"
#include "drm_common_tl.h"
#include "drm_common_swcrypto_types.h"

#include "nexus_base_os.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include "nexus_platform_client.h"
#include "nexus_random_number.h"

#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_otpmsp.h"
#else
#include "nexus_otp_msp.h"
#include "nexus_otp_key.h"
#include "nexus_otp_msp_indexes.h"
#endif

#include "nexus_base_os.h"

#include "bsagelib_types.h"
#include "bsagelib_drm_types.h"
#include "sage_srai.h"

#ifdef USE_UNIFIED_COMMON_DRM
#include "drm_playback_tl.h"
#endif
#include "drm_types.h"

#define DEFAULT_DRM_BIN_FILESIZE (64*1024)
#define DRM_COMMON_OVERWRITE_BIN_FILE (1)
#define SRAI_PlatformId_eCommonDrm (0)
#define MAX_DMA_BLOCKS DRM_COMMON_TL_MAX_DMA_BLOCKS

static int DrmCommon_TL_Counter = 0;
#ifndef USE_UNIFIED_COMMON_DRM
static int Platform_RefCount[Common_Platform_Max] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif

static BKNI_MutexHandle drmCommonTLMutex[Common_Platform_Max] = {0,0,0,0,0,0,0,0};

static uint8_t *drm_bin_file_buff[Common_Platform_Max] = {0,0,0,0,0,0,0,0};
static drm_bin_header_t drm_bin_header[Common_Platform_Max] = {0,0,0,0,0,0,0,0};

static SRAI_PlatformHandle platformHandle[Common_Platform_Max] = {0,0,0,0,0,0,0,0};
static BSAGElib_State platform_status[Common_Platform_Max] = {0,0,0,0,0,0,0,0};

static bool bUseExternalHeapTA[Common_Platform_Max] = {0,0,0,0,0,0,0,0};;

static DrmRC DRM_Common_TL_URR_Toggle(void);

static CommonDrmPlatformType_e DRM_Common_P_drmType_to_PlatformIndex(BSAGElib_BinFileDrmTypes_e drmType);
static uint32_t DRM_Common_P_PlatformIndex_to_PlatformID (CommonDrmPlatformType_e platformIndex);

BDBG_MODULE(drm_common_tl);

static CommonDrmPlatformType_e DRM_Common_P_drmType_to_PlatformIndex(BSAGElib_BinFileDrmTypes_e drmType)
{
    CommonDrmPlatformType_e platformIndex = Common_Platform_Common;

    switch(drmType)
    {
        case BSAGElib_BinFileDrmType_eEdrm:
                platformIndex = Common_Platform_eDrm;
            break;
        case BSAGElib_BinFileDrmType_eWidevine:
                platformIndex = Common_Platform_Widevine;
            break;
        case BSAGElib_BinFileDrmType_eDtcpIp:
                platformIndex = Common_Platform_DtcpIp;
            break;
        case BSAGElib_BinFileDrmType_eAdobeAxcess:
                platformIndex = Common_Platform_AdobeAxcess;
            break;
        case BSAGElib_BinFileDrmType_ePlayready:
                platformIndex = Common_Platform_PlayReady_25;
            break;
        case BSAGElib_BinFileDrmType_eNetflix:
                platformIndex = Common_Platform_Netflix;
            break;
        default:
                platformIndex = Common_Platform_Common;
            break;
    }
    BDBG_MSG(("%s: platformIndex = %d",BSTD_FUNCTION,platformIndex));
    return platformIndex;
}

static uint32_t DRM_Common_P_PlatformIndex_to_PlatformID (CommonDrmPlatformType_e platformIndex)
{
    uint32_t platformID = BSAGE_PLATFORM_ID_COMMONDRM;

    switch (platformIndex)
    {
        case Common_Platform_eDrm:
            platformID = BSAGE_PLATFORM_ID_EDRM;
            break;
        case Common_Platform_Widevine:
            platformID = BSAGE_PLATFORM_ID_WIDEVINE;
            break;
        case Common_Platform_DtcpIp:
            platformID = BSAGE_PLATFORM_ID_DTCP_IP;
            break;
        case Common_Platform_AdobeAxcess:
            platformID = BSAGE_PLATFORM_ID_ADOBE_DRM;
            break;
        case Common_Platform_PlayReady_25:
            platformID = BSAGE_PLATFORM_ID_PLAYREADY_25;
            break;
        case Common_Platform_Netflix:
            platformID = BSAGE_PLATFORM_ID_NETFLIX;
            break;
        default:
            platformID = BSAGE_PLATFORM_ID_COMMONDRM;
            break;
    }
    BDBG_MSG(("%s: platformID = 0x%x",BSTD_FUNCTION,platformID));
    return platformID;
}

DrmRC
DRM_Common_TL_Initialize(DrmCommonInit_TL_t *pCommonTLSettings)
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *container = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;
    CommonDrmPlatformType_e platformIndex = Common_Platform_Common;
    uint32_t platformID = BSAGE_PLATFORM_ID_COMMONDRM;

    BDBG_ENTER(DRM_Common_TL_Initialize);
    BDBG_ASSERT(pCommonTLSettings != NULL);

    platformIndex = DRM_Common_P_drmType_to_PlatformIndex(pCommonTLSettings->drmType);

    if(drmCommonTLMutex[platformIndex] == 0)
    {
        if(BKNI_CreateMutex(&drmCommonTLMutex[platformIndex]) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling create mutex", BSTD_FUNCTION));
            goto ErrorExit;
        }
        BDBG_MSG(("%s: created mutex for platformIndex %d mutex %p handle %p",BSTD_FUNCTION,platformIndex,drmCommonTLMutex[platformIndex],&drmCommonTLMutex[platformIndex]));
    }
    BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);

    if(pCommonTLSettings->drmCommonInit.heap == NULL)
    {
        bUseExternalHeapTA[platformIndex] = false;
    }
    else
    {
        bUseExternalHeapTA[platformIndex] = true;
    }

    /* Call DRM_Common_BasicInitialize */
    /* TODO: revisit when making TL modules independent from libcmndrm */
    rc = DRM_Common_BasicInitialize(&pCommonTLSettings->drmCommonInit);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calling BasicInitialize", BSTD_FUNCTION));
        goto ErrorExit;
    }

    platformID = DRM_Common_P_PlatformIndex_to_PlatformID(platformIndex);

#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
    if(pCommonTLSettings->ta_bin_file_path != NULL){

        rc = DRM_Common_P_TA_Install(platformID, pCommonTLSettings->ta_bin_file_path);

        if(rc != Drm_Success){
            BDBG_WRN(("%s Installing Loadable TA %s - Error (rc %x) ", BSTD_FUNCTION, pCommonTLSettings->ta_bin_file_path, rc));
        }
        /* Set to Success as we support pre-loaded common drm TA */
        rc = Drm_Success;
    }else{
        /* Set to Success as we support pre-loaded common drm TA */
        rc = Drm_Success;
    }
#endif


    if (platformHandle[platformIndex] == NULL)
    {
        sage_rc = SRAI_Platform_Open(platformID, &platform_status[platformIndex], &platformHandle[platformIndex]);
        if (sage_rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling platform_open, pCommonTLSettings->ta_bin_file_path: %s",
                BSTD_FUNCTION, pCommonTLSettings->ta_bin_file_path));
            rc = Drm_Err;
            goto ErrorExit;
        }

        if(platform_status[platformIndex] == BSAGElib_State_eUninit)
        {
            BDBG_WRN(("%s - platform_status == BSAGElib_State_eUninit ************************* (platformHandle = %p)", BSTD_FUNCTION, (void *)platformHandle[platformIndex]));
            container = SRAI_Container_Allocate();
            if(container == NULL)
            {
                BDBG_ERR(("%s - Error fetching container", BSTD_FUNCTION));
                rc = Drm_Err;
                goto ErrorExit;
            }
            BDBG_MSG(("%s: allocated container for platformIndex %d at %p",BSTD_FUNCTION,platformIndex,container));
            sage_rc = SRAI_Platform_Init(platformHandle[platformIndex], container);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s: %d - Error calling platform init", BSTD_FUNCTION,__LINE__));
                rc = Drm_Err;
                goto ErrorExit;
            }
            /* does any info need to be extracted from container ??? */
        }
    }
    else
    {
        BDBG_WRN(("%s: Platform ID 0x%x already initialized *************************", BSTD_FUNCTION,platformID));
    }

    DrmCommon_TL_Counter++;
#ifndef USE_UNIFIED_COMMON_DRM
    Platform_RefCount[platformIndex]++;
#endif

ErrorExit:

    if (container != NULL) {
        BDBG_MSG(("%s: freeing container %p for platformIndex %d",BSTD_FUNCTION,container,platformIndex));
        SRAI_Container_Free(container);
    }

    BDBG_LEAVE(DRM_Common_TL_Initialize);

    BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
    return rc;
}
DrmRC
DRM_Common_TL_ModuleInitialize(
                            uint32_t module_id,
                            char * drm_bin_filename,
                            BSAGElib_InOutContainer *container,
                            SRAI_ModuleHandle *moduleHandle)
{
    DrmRC rc = Drm_Success;
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_Common,
        module_id, drm_bin_filename, container, moduleHandle);
    return rc;
}

DrmRC
DRM_Common_TL_ModuleInitialize_TA(CommonDrmPlatformType_e platIndex,
                            uint32_t module_id,
                            char * drm_bin_filename,
                            BSAGElib_InOutContainer *container,
                            SRAI_ModuleHandle *moduleHandle)
{
    DrmRC rc = Drm_Success;
    SRAI_Module_InitSettings init_settings;

    init_settings.indicationCallback = NULL;
    init_settings.indicationCallbackArg = 0;

    rc = DRM_Common_TL_ModuleInitialize_TA_Ext(platIndex,
        module_id, drm_bin_filename, container, moduleHandle, &init_settings);
    return rc;
}

DrmRC
DRM_Common_TL_ModuleInitialize_TA_Ext(CommonDrmPlatformType_e platIndex,
                                      uint32_t module_id,
                                      char * drm_bin_filename,
                                      BSAGElib_InOutContainer *container,
                                      SRAI_ModuleHandle *moduleHandle,
                                      SRAI_Module_InitSettings *init_settings)
{
    DrmRC rc = Drm_Success;
    FILE * fptr = NULL;
    uint32_t filesize = 0;
    uint32_t read_size = 0;
    uint32_t filesize_from_header = 0;
    uint32_t write_size = 0;
    BERR_Code sage_rc = BERR_SUCCESS;
    NEXUS_Error nexerr = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings memSettings;
    CommonDrmPlatformType_e platformIndex;

    BDBG_ENTER(DRM_Common_TL_ModuleInitialize_TA);

    BDBG_ASSERT(container != NULL);
    BDBG_ASSERT(moduleHandle != NULL);

    BDBG_ASSERT(((platIndex < Common_Platform_Max) && ( platIndex >= Common_Platform_Common)));
    platformIndex = platIndex;
    BDBG_MSG(("%s platform %d module %d container %p moduleHandle %p ",BSTD_FUNCTION,platformIndex,module_id,container,moduleHandle));

    BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);

    /* if module uses a bin file - read it and add it to the container */
    if(drm_bin_filename != NULL)
    {
        BDBG_MSG(("%s - DRM bin filename '%s'", BSTD_FUNCTION, drm_bin_filename));
        /*
         * 1) allocate drm_bin_file_buff
         * 2) read bin file
         * 3) check size
         * */
        rc = DRM_Common_P_GetFileSize(drm_bin_filename, &filesize);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error determine file size of bin file", BSTD_FUNCTION));
            goto ErrorExit;
        }

        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        if (bUseExternalHeapTA[platformIndex] == true)
        {
            nexerr = NEXUS_Memory_Allocate(filesize, &memSettings,(void **) &drm_bin_file_buff[platformIndex]);
        }
        else
        {
            nexerr = NEXUS_Memory_Allocate(filesize, NULL,(void **) &drm_bin_file_buff[platformIndex]);
        }

        if(nexerr != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Error allocating buffer err %u  bUseExternalHeap (%u)", BSTD_FUNCTION, nexerr, bUseExternalHeapTA[platformIndex]));
        }
        BDBG_MSG(("%s allocated file size %u drm_bin_file_buff[%d] %p", BSTD_FUNCTION, filesize, platformIndex, drm_bin_file_buff[platformIndex]));

        if(drm_bin_file_buff[platformIndex] == NULL)
        {
            BDBG_ERR(("%s - Error allocating '%u' bytes", BSTD_FUNCTION, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_MemErr;
            goto ErrorExit;
        }

        fptr = fopen(drm_bin_filename, "rb");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening drm bin file (%s)", BSTD_FUNCTION, drm_bin_filename));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        read_size = fread(drm_bin_file_buff[platformIndex], 1, filesize, fptr);
        if(read_size != filesize)
        {
            BDBG_ERR(("%s - Error reading drm bin file size (%u != %u)", BSTD_FUNCTION, read_size, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        /* close file and set to NULL */
        if(fclose(fptr) != 0)
        {
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", BSTD_FUNCTION, drm_bin_filename, strerror(errno)));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        fptr = NULL;

        /* verify allocated drm_bin_file_buff size with size in header */
        BKNI_Memcpy((uint8_t*)&drm_bin_header[platformIndex], drm_bin_file_buff[platformIndex], sizeof(drm_bin_header_t));
        filesize_from_header = GET_UINT32_FROM_BUF(drm_bin_header[platformIndex].bin_file_size);
        BDBG_MSG(("%s file size from header %u - line = %u", BSTD_FUNCTION, filesize_from_header, __LINE__));

        if(filesize_from_header > DEFAULT_DRM_BIN_FILESIZE)
        {
            BDBG_MSG(("%s - bin file size too large - line = %u", BSTD_FUNCTION, __LINE__));
            NEXUS_Memory_Free(drm_bin_file_buff[platformIndex]);
            drm_bin_file_buff[platformIndex] = NULL;
            if (bUseExternalHeapTA[platformIndex] == true)
            {
                nexerr = NEXUS_Memory_Allocate(filesize_from_header, &memSettings,(void **) &drm_bin_file_buff[platformIndex]);
            }
            else
            {
                nexerr = NEXUS_Memory_Allocate(filesize_from_header, NULL,(void **) &drm_bin_file_buff[platformIndex]);
            }
            if (nexerr == NEXUS_SUCCESS) {
                BDBG_MSG(("%s setting drm bin file buff to 0s",BSTD_FUNCTION));
                BKNI_Memset(drm_bin_file_buff[platformIndex], 0x00, filesize_from_header);
            }
            else
            {
                BDBG_MSG(("%s failed to allocate mem for invalid drm bin file size nexerr=%d",BSTD_FUNCTION,nexerr));
                (*moduleHandle) = NULL;
                rc = Drm_SraiModuleError;
                goto ErrorExit;
            }
        }

        if(filesize_from_header != filesize)
        {
            BDBG_ERR(("%s - Error validating file size in header (%u != %u)", BSTD_FUNCTION, filesize_from_header, filesize));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        /* All index '0' shared blocks will be reserved for drm bin file data */

        /* first verify that it has not been already used by the calling module */
        if(container->blocks[0].data.ptr != NULL)
        {
            BDBG_ERR(("%s - Shared block[0] reserved for all DRM modules with bin file to pass to Sage.", BSTD_FUNCTION));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }

        container->blocks[0].len = filesize_from_header;

        container->blocks[0].data.ptr = SRAI_Memory_Allocate(filesize_from_header, SRAI_MemoryType_Shared);
        BDBG_MSG(("%s - Allocating SHARED MEMORY of '%u' bytes for shared block[0] (address %p)", BSTD_FUNCTION, filesize_from_header, container->blocks[0].data.ptr));
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating SRAI memory", BSTD_FUNCTION));
            (*moduleHandle) = NULL;
            rc = Drm_SraiModuleError;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, drm_bin_file_buff[platformIndex], filesize_from_header);

        BDBG_MSG(("%s - Copied '%u' bytes into SRAI container (address %p)", BSTD_FUNCTION, filesize_from_header, container->blocks[0].data.ptr));
    }

    /* All modules will call SRAI_Module_Init */
    BDBG_MSG(("%s - ************************* (platformHandle = %p)", BSTD_FUNCTION, (void *)platformHandle[platformIndex]));
    sage_rc = SRAI_Module_Init_Ext(platformHandle[platformIndex], module_id, container, moduleHandle, init_settings);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Module_Init", BSTD_FUNCTION));
        (*moduleHandle) = NULL;
        rc = Drm_SraiModuleError;
        goto ErrorExit;
    }

    /* Extract DRM bin file manager response from basic[0].  Free memory if failed */
    sage_rc = container->basicOut[0];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - SAGE error processing DRM bin file", BSTD_FUNCTION));

        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        (*moduleHandle) = NULL;
        rc = Drm_SraiModuleError;

        goto ErrorExit;
    }

#ifndef CMNDRM_SKIP_BINFILE_OVERWRITE
    /* Overwrite the drm bin file in rootfs, free up the buffer since a copy will exist on the sage side */
    if(drm_bin_filename != NULL)
    {
        if(container->basicOut[1] == DRM_COMMON_OVERWRITE_BIN_FILE)
        {
            BDBG_MSG(("%s - Overwriting file '%s'", BSTD_FUNCTION, drm_bin_filename));

            if(fptr != NULL)
            {
                BDBG_ERR(("%s - File pointer already opened, invalid state.  '%s'", BSTD_FUNCTION, drm_bin_filename));
                rc = Drm_Err;
                goto ErrorExit;
            }

            /* Overwrite drm bin file once bounded */
            fptr = fopen(drm_bin_filename, "w+b");
            if(fptr == NULL)
            {
                BDBG_ERR(("%s - Error opening DRM bin file (%s) in 'w+b' mode.  '%s'", BSTD_FUNCTION, drm_bin_filename, strerror(errno)));
                rc = Drm_Err;
                goto ErrorExit;
            }

            write_size = fwrite(container->blocks[0].data.ptr, 1, filesize_from_header, fptr);
            if(write_size != filesize)
            {
                BDBG_ERR(("%s - Error writing drm bin file size to rootfs (%u != %u)", BSTD_FUNCTION, write_size, filesize));
                (*moduleHandle) = NULL;
                rc = Drm_SraiModuleError;
                goto ErrorExit;
            }
            fclose(fptr);
            fptr = NULL;
        }
        else{
            BDBG_MSG(("%s - No need to overwrite file '%s'", BSTD_FUNCTION, drm_bin_filename));
        }
    }
#endif

ErrorExit:

    /* if shared block[0] is not null, free since there was an error processing (i.e. can't trust the data) or the file was copied*/
    if(container->blocks[0].data.ptr != NULL){
        BDBG_MSG(("%s: freeing container->blocks[0].data.ptr %p",BSTD_FUNCTION, container->blocks[0].data.ptr));
        SRAI_Memory_Free(container->blocks[0].data.ptr);
        container->blocks[0].data.ptr = NULL;
    }

    if(drm_bin_file_buff[platformIndex] != NULL){
        BDBG_MSG(("%s: freeing drm_bin_file_buff[%d] %p",BSTD_FUNCTION,platformIndex,drm_bin_file_buff[platformIndex]));
        NEXUS_Memory_Free(drm_bin_file_buff[platformIndex]);
        drm_bin_file_buff[platformIndex] = NULL;
    }

    if(fptr != NULL){
        BDBG_MSG(("%s: Freeing fptr %p",BSTD_FUNCTION,fptr));
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(DRM_Common_TL_ModuleInitialize_TA);

    BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
    return rc;
}

DrmRC
DRM_Common_TL_ModuleFinalize(SRAI_ModuleHandle moduleHandle)
{
    DrmRC rc = Drm_Success;
    rc = DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Common, moduleHandle);
    return rc;
}

DrmRC
DRM_Common_TL_ModuleFinalize_TA(CommonDrmPlatformType_e platIndex, SRAI_ModuleHandle moduleHandle)
{
    CommonDrmPlatformType_e platformIndex;
    BDBG_ASSERT(((platIndex < Common_Platform_Max) && ( platIndex >= Common_Platform_Common)));
    platformIndex = platIndex;
    BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);

    BDBG_ENTER(DRM_Common_TL_ModuleFinalize_TA);

    SRAI_Module_Uninit(moduleHandle);

    BDBG_LEAVE(DRM_Common_TL_ModuleFinalize_TA);

    BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
    return Drm_Success;
}


static DrmRC DRM_Common_TL_URR_Toggle(void)
{
    DrmRC drmRc = Drm_Success;
#ifdef USE_UNIFIED_COMMON_DRM
    DrmPlaybackHandle_t playbackHandle = NULL;

    drmRc = DRM_Playback_Initialize(&playbackHandle);
    if (drmRc != Drm_Success || playbackHandle == NULL) {
        BDBG_ERR(("%s: DRM_Playback_Initialize returned error %d", BSTD_FUNCTION, drmRc));
        goto toggle_error_exit;
    }
    drmRc = DRM_Playback_Stop(playbackHandle);
    if (drmRc != Drm_Success) {
        BDBG_ERR(("%s: DRM_Playback_Stop returned error %d", BSTD_FUNCTION, drmRc));
        /* Fall through */
    }

    drmRc = DRM_Playback_Finalize(playbackHandle);
    if (drmRc != Drm_Success) {
         BDBG_ERR(("%s: DRM_Playback_Finalize returned error %d",BSTD_FUNCTION, drmRc));
    }
#endif

toggle_error_exit:
    return drmRc;
}
DrmRC DRM_Common_TL_Finalize()
{
    DrmRC rc = Drm_Success;
    rc = DRM_Common_TL_Finalize_TA(Common_Platform_Common);
    return rc;
}

DrmRC DRM_Common_TL_Finalize_TA(CommonDrmPlatformType_e platIndex)
{
    DrmRC rc = Drm_Success;
    uint32_t platformID = BSAGE_PLATFORM_ID_COMMONDRM;
    CommonDrmPlatformType_e platformIndex;

    BDBG_ASSERT(((platIndex < Common_Platform_Max) && ( platIndex >= Common_Platform_Common)));
    platformIndex = platIndex;

    BDBG_ASSERT(drmCommonTLMutex[platformIndex] != NULL);
    BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);

    BDBG_MSG(("%s - Entered function (init = '%d')", BSTD_FUNCTION, DrmCommon_TL_Counter));
#ifndef USE_UNIFIED_COMMON_DRM
    BDBG_MSG(("%s - Platform_RefCount[%d]=%d", BSTD_FUNCTION, platformIndex, Platform_RefCount[platformIndex]));
#endif

    platformID = DRM_Common_P_PlatformIndex_to_PlatformID(platformIndex);

    /* sanity check */
    if (DrmCommon_TL_Counter <= 0)
    {
        BDBG_WRN(("%s - DrmCommon_TL_Counter value is invalid ('%d').  Possible bad thread exit", BSTD_FUNCTION, DrmCommon_TL_Counter));
    }
#ifndef USE_UNIFIED_COMMON_DRM
    if (Platform_RefCount[platformIndex] <= 0)
    {
        BDBG_WRN(("%s - Platform_RefCount[%d] value is invalid ('%d').  Possible bad thread exit", BSTD_FUNCTION, platformIndex, Platform_RefCount[platformIndex]));
    }
#endif
    /* if there's one DRM module left calling DRM_Common_Finalize, clean everything up
     * Otherwise skip the clean up and decrement the counter */
    if (DrmCommon_TL_Counter == 1)
    {
        /* Call to URR Toggle. DRM_Common_TL_Finalize() will be called within its own context so we must
        * release the mutex.  The counter will increment again preventing an infinite loop. */
        BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);

  BDBG_MSG(("%s - Start URR toggle", BSTD_FUNCTION));
        rc = DRM_Common_TL_URR_Toggle();
        if (rc != Drm_Success) {
            BDBG_ERR(("%s - Error performing URR toggle", BSTD_FUNCTION));
        }
  BDBG_MSG(("%s - URR toggle completed", BSTD_FUNCTION));

        BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);
    }

#ifdef USE_UNIFIED_COMMON_DRM
    if (DrmCommon_TL_Counter == 1)
#else
    if (Platform_RefCount[platformIndex] == 1)
#endif
    {
        BDBG_MSG(("%s - Cleaning up Common DRM TL only parameters ***************************", BSTD_FUNCTION));
        if (platformHandle[platformIndex]) {
            SRAI_Platform_Close(platformHandle[platformIndex]);
            platformHandle[platformIndex] = NULL;
        }

        /* Finalize and decrement the counter after we finish the URR toggle */
        DRM_Common_Finalize();
        DrmCommon_TL_Counter--;
#ifndef USE_UNIFIED_COMMON_DRM
        Platform_RefCount[platformIndex]--;
#endif

        SRAI_Platform_UnInstall(platformID);

        BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
        BKNI_DestroyMutex(drmCommonTLMutex[platformIndex]);
        drmCommonTLMutex[platformIndex] = 0;
    }
    else
    {
        DRM_Common_Finalize();
        if (DrmCommon_TL_Counter > 0) {
            DrmCommon_TL_Counter--;
        }
#ifndef USE_UNIFIED_COMMON_DRM
        if (Platform_RefCount[platformIndex] > 0) {
            Platform_RefCount[platformIndex]--;
        }
#endif

        BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
    }

    BDBG_MSG(("%s - Exiting function (init = '%d')", BSTD_FUNCTION, DrmCommon_TL_Counter));
#ifndef USE_UNIFIED_COMMON_DRM
    BDBG_MSG(("%s - Platform_RefCount[%d]=%d", BSTD_FUNCTION, platformIndex, Platform_RefCount[platformIndex]));
#endif
    return rc;
}

DrmRC DRM_Common_P_GetFileSize(char * filename, uint32_t *filesize)
{
    DrmRC rc = Drm_Success;
    FILE *fptr = NULL;
    int pos = 0;

    BDBG_ENTER(DRM_Common_P_GetFileSize);

    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    pos = fseek(fptr, 0, SEEK_END);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error seeking to end of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    pos = ftell(fptr);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error determining position of file pointer of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /* check vs. arbitrary large file size */
    if(pos >= 2*1024*1024)
    {
        BDBG_ERR(("%s - Invalid file size detected for of file '%s'.  (%u)", BSTD_FUNCTION, filename, pos));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    (*filesize) = pos;

ErrorExit:

    BDBG_MSG(("%s - Exiting function (%u bytes)", BSTD_FUNCTION, (*filesize)));

    if(fptr != NULL)
    {
        /* error closing?!  weird error case not sure how to handle */
        if(fclose(fptr) != 0){
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
            rc = Drm_Err;
        }
    }

    return rc;
}
#ifdef USE_UNIFIED_COMMON_DRM
DrmRC DRM_Common_TL_M2mOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct, bool bSkipCacheFlush, bool bExternalIV )
#else
DrmRC DRM_Common_TL_M2mOperation_TA(CommonDrmPlatformType_e platIndex, DrmCommonOperationStruct_t *pDrmCommonOpStruct, bool bSkipCacheFlush, bool bExternalIV )
#endif
{
    NEXUS_DmaJobBlockSettings jobBlkSettings[MAX_DMA_BLOCKS];

    CommonCryptoJobSettings jobSettings;
    unsigned int i = 0;
    unsigned int j = 0;
    DrmRC drmRc = Drm_Success;
    CommonDrmPlatformType_e platformIndex;

    /* The mutex is still protecting the DRM Common Handle (resp CommonCrypto handle) table */
    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
#ifdef USE_UNIFIED_COMMON_DRM
    platformIndex = Common_Platform_Common;
#else
    BDBG_ASSERT(((platIndex < Common_Platform_Max) && ( platIndex > Common_Platform_Common)));
    platformIndex = platIndex;
#endif

    /* Sanity check */
    if ( (pDrmCommonOpStruct == NULL) || (pDrmCommonOpStruct->pDmaBlock == NULL) ||
         (pDrmCommonOpStruct->num_dma_block > MAX_DMA_BLOCKS) ||
         ( bExternalIV && pDrmCommonOpStruct->num_dma_block < 1 ))
    {
        BDBG_ERR(("%s - invalid paramters for M2M operation", BSTD_FUNCTION));
        drmRc = Drm_CryptoDmaErr;
        goto ErrorExit;
    }

    BDBG_ASSERT(drmCommonTLMutex[platformIndex] != NULL);
    BKNI_AcquireMutex(drmCommonTLMutex[platformIndex]);

    DRM_Common_Handle drmHnd;
    drmRc = DRM_Common_GetHandle(&drmHnd);;

    if (drmRc == Drm_Success)
    {
        CommonCrypto_GetDefaultJobSettings(&jobSettings);

        /* Data format used when encrypting/decryption data. */
        /*jobSettings.dataFormat; *//* Set to NEXUS_DmaDataFormat_eBlock in CommonCrypto_GetDefaultJobSettings ?! */
        /* Key slot handle to use during the DMA transfer.  NULL(default) if not encrypting or decrypting data*/
        jobSettings.keySlot = pDrmCommonOpStruct->keyConfigSettings.keySlot;

        DmaBlockInfo_t* pDmaBlock = pDrmCommonOpStruct->pDmaBlock;

        if ( bExternalIV )
        {
            BDBG_MSG(("%s - btp 0x%08x size=%d", BSTD_FUNCTION, pDmaBlock->pSrcData, pDmaBlock->uiDataSize));

            /* External IV BTP data in first dma block */
            NEXUS_DmaJob_GetDefaultBlockSettings(&jobBlkSettings[0]);
            jobBlkSettings[0].pSrcAddr                   = pDmaBlock->pSrcData;
            jobBlkSettings[0].pDestAddr                  = pDmaBlock->pDstData;
            jobBlkSettings[0].blockSize                  = pDmaBlock->uiDataSize;
            jobBlkSettings[0].resetCrypto                = true;
            jobBlkSettings[0].scatterGatherCryptoStart   = true;
            jobBlkSettings[0].scatterGatherCryptoEnd     = true;
            jobBlkSettings[0].securityBtp                = true;
            if (bSkipCacheFlush) {
                jobBlkSettings[0].cached = true;
            }
            else
            {
                NEXUS_FlushCache(jobBlkSettings[0].pSrcAddr, jobBlkSettings[0].blockSize);
            }
            pDmaBlock++;
            j=1;
        }

        for ( i = j; i < pDrmCommonOpStruct->num_dma_block; i++)
        {
            BDBG_MSG(("%s - blkidx=%d, src=0x%08x, des=0x%08x, size=%d, start=%d,end=%d", BSTD_FUNCTION, i, pDmaBlock->pSrcData, pDmaBlock->pDstData, pDmaBlock->uiDataSize, pDmaBlock->sg_start,pDmaBlock->sg_end ));
            NEXUS_DmaJob_GetDefaultBlockSettings (&jobBlkSettings[i]);
            jobBlkSettings[i].pSrcAddr = pDmaBlock->pSrcData;
            jobBlkSettings[i].pDestAddr = pDmaBlock->pDstData;
            jobBlkSettings[i].blockSize = pDmaBlock->uiDataSize;
            jobBlkSettings[i].scatterGatherCryptoStart = pDmaBlock->sg_start;
            jobBlkSettings[i].scatterGatherCryptoEnd = pDmaBlock->sg_end;
            if (bSkipCacheFlush) {
                jobBlkSettings[i].cached = true;
            }
            else
            {
                NEXUS_FlushCache(jobBlkSettings[i].pSrcAddr, jobBlkSettings[i].blockSize);
            }
            pDmaBlock++;
        }

        jobBlkSettings[j].resetCrypto = true;

        BDBG_MSG(("%s - dmaXfer [%d] blocks", BSTD_FUNCTION, pDrmCommonOpStruct->num_dma_block ));
        if (CommonCrypto_DmaXfer(drmHnd->cryptHnd,
                                 &jobSettings,
                                 jobBlkSettings,
                                 pDrmCommonOpStruct->num_dma_block) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Error with M2M DMA operation", BSTD_FUNCTION));
            drmRc = Drm_CryptoDmaErr;
            goto ErrorExit;
        }

        if (!bSkipCacheFlush) {
            for (i = j; i < pDrmCommonOpStruct->num_dma_block; i++)
            {
                NEXUS_FlushCache(jobBlkSettings[i].pDestAddr, jobBlkSettings[i].blockSize);
            }
        }
    }

ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonTLMutex[platformIndex]);
    return drmRc;
}

DrmRC DRM_Common_P_TA_Install(uint32_t platformID, char * ta_bin_filename)
{
    DrmRC rc = Drm_Success;
    FILE * fptr = NULL;
    uint32_t file_size = 0;
    uint32_t read_size = 0;
    uint8_t *ta_bin_file_buff = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;

    BDBG_ENTER(DRM_Common_TL_P_Install);

    BDBG_MSG(("%s - DRM bin filename '%s'", BSTD_FUNCTION, ta_bin_filename));

    rc = DRM_Common_P_GetFileSize(ta_bin_filename, &file_size);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error determine file size of TA bin file", BSTD_FUNCTION));
        goto ErrorExit;
    }

    ta_bin_file_buff = SRAI_Memory_Allocate(file_size, SRAI_MemoryType_Shared);
    if(ta_bin_file_buff == NULL)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for loading TA bin file", BSTD_FUNCTION, file_size));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    BDBG_MSG(("%s: allocated ta_bin_file_buff at %p size %u",BSTD_FUNCTION,ta_bin_file_buff,file_size));
    fptr = fopen(ta_bin_filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening TA bin file (%s)", BSTD_FUNCTION, ta_bin_filename));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    read_size = fread(ta_bin_file_buff, 1, file_size, fptr);
    if(read_size != file_size)
    {
        BDBG_ERR(("%s - Error reading TA bin file size (%u != %u)", BSTD_FUNCTION, read_size, file_size));
        rc = Drm_FileErr;
        goto ErrorExit;
    }

    /* close file and set to NULL */
    if(fclose(fptr) != 0)
    {
        BDBG_ERR(("%s - Error closing TA bin file '%s'.  (%s)", BSTD_FUNCTION, ta_bin_filename, strerror(errno)));
        rc = Drm_FileErr;
        goto ErrorExit;
    }
    fptr = NULL;

    BDBG_MSG(("%s - TA 0x%x Install file %s size %u", BSTD_FUNCTION,platformID,ta_bin_filename,file_size));

    sage_rc = SRAI_Platform_Install(platformID, ta_bin_file_buff, file_size);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Platform_Install Error 0x%x", BSTD_FUNCTION, sage_rc ));
        rc = Drm_SraiModuleError;
        goto ErrorExit;
    }

ErrorExit:

    if(ta_bin_file_buff != NULL){
        SRAI_Memory_Free(ta_bin_file_buff);
        ta_bin_file_buff = NULL;
    }

    if(fptr != NULL){
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(DRM_Common_P_TA_Install);

    return rc;
}

#define OTP_MSP0_VALUE_ZS (0x02)
#define OTP_MSP1_VALUE_ZS (0x02)
#define OTP_MSP0_VALUE_ZB (0x3E)
#define OTP_MSP1_VALUE_ZB (0x3F)

ChipType_e DRM_Common_GetChipType()
{
    NEXUS_Error rc =  NEXUS_SUCCESS;
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_ReadMspParms     readMspParms;
    NEXUS_ReadMspIO        readMsp0;
    NEXUS_ReadMspIO        readMsp1;

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved233;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp0);

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved234;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp1);

    BDBG_MSG(("OTP MSP0 %d %d %d %d OTP MSP0 %d %d %d %d",readMsp0.mspDataBuf[0], readMsp0.mspDataBuf[1], readMsp0.mspDataBuf[2], readMsp0.mspDataBuf[3],
                                                          readMsp1.mspDataBuf[0], readMsp1.mspDataBuf[1], readMsp1.mspDataBuf[2], readMsp1.mspDataBuf[3]));

    if((readMsp0.mspDataBuf[3] == OTP_MSP0_VALUE_ZS) && (readMsp1.mspDataBuf[3] == OTP_MSP1_VALUE_ZS)){
        return ChipType_eZS;
    }
    return ChipType_eZB;
#else
#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(5,1)
    unsigned msp0Index = 224;
    unsigned msp1Index = 225;
#else
    unsigned msp0Index = 233;
    unsigned msp1Index = 234;
#endif

    NEXUS_OtpMspRead readMsp0;
    NEXUS_OtpMspRead readMsp1;

    rc = NEXUS_OtpMsp_Read( msp0Index, &readMsp0);
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Error Reading MSP", BSTD_FUNCTION));
        return ChipType_eMax;
    }
    BDBG_LOG(("%s: otp msp0 = 0x%x, valid= 0x%x", __FUNCTION__,readMsp0.data, readMsp0.valid));
    readMsp0.data &= readMsp0.data & readMsp0.valid;
    BDBG_LOG(("%s: otp msp0 = 0x%x", __FUNCTION__,readMsp0.data));
    rc = NEXUS_OtpMsp_Read( msp1Index, &readMsp1);
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Error Reading MSP", BSTD_FUNCTION));
        return ChipType_eMax;
    }
    readMsp1.data &= readMsp1.data & readMsp1.valid;
    BDBG_LOG(("%s: otp msp1 = 0x%x", __FUNCTION__,readMsp1.data));

    if ((readMsp0.data  == OTP_MSP0_VALUE_ZS)
        && (readMsp1.data  == OTP_MSP1_VALUE_ZS)) {
        BDBG_LOG(("%s: chiptype is ZS",__FUNCTION__));
        return ChipType_eZS;
    }
    else
    {
        BDBG_LOG(("%s: chiptype is ZB",__FUNCTION__));
        return ChipType_eZB;
    }
#endif
}
