/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "utility_platform.h"

#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "drm_metadata_tl.h"

BDBG_MODULE(utility_tl);

/* helper macro */
#define GET_UINT32_FROM_BUF(pBuf) \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) | \
     ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) | \
     ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  | \
     ((uint8_t *)(pBuf))[3])

/* definitions */
#define DEFAULT_DRM_BIN_FILESIZE (64*1024)
#define OVERWRITE_BIN_FILE (1)

static int Utility_ModuleCounter = 0;
static BKNI_MutexHandle utilityMutex = NULL;

static uint8_t *drm_bin_file_buff = NULL;

static SRAI_PlatformHandle platformHandle = NULL;

static uint32_t Utility_P_CheckDrmBinFileSize(void);
static BERR_Code Utility_P_GetFileSize(const char * filename, uint32_t *filesize);


BERR_Code
Utility_ModuleInit(Utility_ModuleId_e module_id,
                   const char * drm_bin_filename,
                   BSAGElib_InOutContainer *container,
                   SRAI_ModuleHandle *moduleHandle)
{
    BERR_Code rc = BERR_SUCCESS;
    FILE * fptr = NULL;
    uint32_t read_size = 0;
    uint32_t filesize_from_header = 0;
    uint32_t filesize = 0;
    uint32_t write_size = 0;
    BERR_Code sage_rc = BERR_SUCCESS;
    SRAI_ModuleHandle tmpModuleHandle = NULL;

    BDBG_ENTER(Utility_ModuleInit);

    if(utilityMutex == NULL)
    {
        if(BKNI_CreateMutex(&utilityMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling create mutex", __FUNCTION__));
            rc = BERR_OS_ERROR;
            goto End;
        }
    }
    BKNI_AcquireMutex(utilityMutex);

    if (moduleHandle == NULL)
    {
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    (*moduleHandle) = NULL;


    if(platformHandle == NULL)
    {
        BSAGElib_State platform_status;
        sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_UTILITY, &platform_status, &platformHandle);
        if (sage_rc != BERR_SUCCESS)
        {
            platformHandle = NULL; /* sanity reset */
            BDBG_ERR(("%s - Error calling platform_open", __FUNCTION__));
	        rc = sage_rc;
	        goto ErrorExit;
	    }

        BDBG_MSG(("%s - SRAI_Platform_Open(%u, %p, %p) returned %p", __FUNCTION__, BSAGE_PLATFORM_ID_UTILITY, &platform_status, &platformHandle, platformHandle));

		if(platform_status == BSAGElib_State_eUninit)
		{
	        BDBG_WRN(("%s - platform_status == BSAGElib_State_eUninit ************************* (platformHandle = 0x%08x)", __FUNCTION__, platformHandle));
		    sage_rc = SRAI_Platform_Init(platformHandle, NULL);
	        if (sage_rc != BERR_SUCCESS)
	        {
	            BDBG_ERR(("%s - Error calling platform init", __FUNCTION__));
	            rc = sage_rc;
	            goto ErrorExit;
	        }
	    }
	    else{
	        BDBG_WRN(("%s - Platform already initialized *************************", __FUNCTION__));
	    }
    }

    Utility_ModuleCounter++;

    /* if module uses a bin file - read it and add it to the container */
    if(drm_bin_filename != NULL)
    {
        if (container == NULL)
        {
            rc = BSAGE_ERR_CONTAINER_REQUIRED;
            BDBG_ERR(("%s - container is required to hold bin file", __FUNCTION__));
            goto ErrorExit;
        }

        /* first verify that it has not been already used by the calling module */
        if(container->blocks[0].data.ptr != NULL)
        {
            BDBG_ERR(("%s - Shared block[0] reserved for all DRM modules with bin file to pass to Sage.", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto ErrorExit;
        }

        BDBG_MSG(("%s - DRM bin filename '%s'", __FUNCTION__, drm_bin_filename));
        /*
         * 1) allocate drm_bin_file_buff
         * 2) read bin file
         * 3) check size
         * */
        rc = Utility_P_GetFileSize(drm_bin_filename, &filesize);
        if(rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error determine file size of bin file", __FUNCTION__));
            goto ErrorExit;
        }

        drm_bin_file_buff = SRAI_Memory_Allocate(filesize, SRAI_MemoryType_Shared);
        if(drm_bin_file_buff == NULL)
        {
            BDBG_ERR(("%s - Error allocating '%u' bytes", __FUNCTION__, filesize));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }

        fptr = fopen(drm_bin_filename, "rb");
        if(fptr == NULL)
        {
            BDBG_ERR(("%s - Error opening drm bin file (%s)", __FUNCTION__, drm_bin_filename));
            rc = BERR_OS_ERROR;
            goto ErrorExit;
        }

        read_size = fread(drm_bin_file_buff, 1, filesize, fptr);
        if(read_size != filesize)
        {
            BDBG_ERR(("%s - Error reading drm bin file size (%u != %u)", __FUNCTION__, read_size, filesize));
            rc = BERR_OS_ERROR;
            goto ErrorExit;
        }

        /* close file and set to NULL */
        if(fclose(fptr) != 0)
        {
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", __FUNCTION__, drm_bin_filename, strerror(errno)));
            rc = BERR_OS_ERROR;
            goto ErrorExit;
        }

        fptr = NULL;

        /* verify allocated drm_bin_file_buff size with size in header */
        filesize_from_header = Utility_P_CheckDrmBinFileSize();
        if(filesize_from_header != filesize)
        {
            BDBG_ERR(("%s - Error validating file size in header (%u != %u)", __FUNCTION__, filesize_from_header, filesize));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }

        BDBG_MSG(("%s - Error validating file size in header (%u ?=? %u)", __FUNCTION__, filesize_from_header, filesize));

        /* All index '0' shared blocks will be reserved for drm bin file data */

        container->blocks[0].len = filesize_from_header;

        container->blocks[0].data.ptr = SRAI_Memory_Allocate(filesize_from_header, SRAI_MemoryType_Shared);
        BDBG_MSG(("%s - Allocating SHARED MEMORY of '%u' bytes for shared block[0] (address %p)", __FUNCTION__, filesize_from_header, container->blocks[0].data.ptr));
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating SRAI memory", __FUNCTION__));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr, drm_bin_file_buff, filesize_from_header);

        BDBG_MSG(("%s - Copied '%u' bytes into SRAI container (address %p)", __FUNCTION__, filesize_from_header, container->blocks[0].data.ptr));
    }

    /* All modules will call SRAI_Module_Init */
    BDBG_MSG(("%s - ************************* (platformHandle = 0x%08x)", __FUNCTION__, platformHandle));
    sage_rc = SRAI_Module_Init(platformHandle, module_id, container, &tmpModuleHandle);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Module_Init", __FUNCTION__));
        rc = sage_rc;
        goto ErrorExit;
    }
    BDBG_MSG(("%s - SRAI_Module_Init(%p, %u, %p, %p) returned %p",
              __FUNCTION__, platformHandle, module_id, container, &tmpModuleHandle, moduleHandle));

    /* Extract DRM bin file manager response from basic[0].  Free memory if failed */
    sage_rc = container->basicOut[0];
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - SAGE error processing DRM bin file", __FUNCTION__));
        rc = sage_rc;
        goto ErrorExit;
    }

    /* Overwrite the drm bin file in rootfs, free up the buffer since a copy will exist on the sage side */
    if(drm_bin_filename != NULL)
    {
        if(container->basicOut[1] == OVERWRITE_BIN_FILE)
        {
            BDBG_MSG(("%s - Overwriting file '%s'", __FUNCTION__, drm_bin_filename));

            if(fptr != NULL)
            {
                BDBG_ERR(("%s - File pointer already opened, invalid state.  '%s'", __FUNCTION__, drm_bin_filename));
                rc = BERR_OS_ERROR;
                goto ErrorExit;
            }

            /* Overwrite drm bin file once bounded */
            fptr = fopen(drm_bin_filename, "w+b");
            if(fptr == NULL)
            {
                BDBG_ERR(("%s - Error opening DRM bin file (%s) in 'w+b' mode.  '%s'", __FUNCTION__, drm_bin_filename, strerror(errno)));
                rc = BERR_OS_ERROR;
                goto ErrorExit;
            }

            write_size = fwrite(container->blocks[0].data.ptr, 1, filesize_from_header, fptr);
            if(write_size != filesize)
            {
                BDBG_ERR(("%s - Error writing drm bin file size to rootfs (%u != %u)", __FUNCTION__, write_size, filesize));
                rc = BERR_OS_ERROR;
                goto ErrorExit;
            }
            fclose(fptr);
            fptr = NULL;
        }
        else{
            BDBG_MSG(("%s - No need to overwrite file '%s'", __FUNCTION__, drm_bin_filename));
        }
    }

    /* success */
    *moduleHandle = tmpModuleHandle;

ErrorExit:

    if(container && container->blocks[0].data.ptr != NULL){
        SRAI_Memory_Free(container->blocks[0].data.ptr);
        container->blocks[0].data.ptr = NULL;
    }

    if(drm_bin_file_buff != NULL){
        SRAI_Memory_Free(drm_bin_file_buff);
        drm_bin_file_buff = NULL;
    }

    if(fptr != NULL){
        fclose(fptr);
        fptr = NULL;
    }

    BKNI_ReleaseMutex(utilityMutex);

    if (rc != BERR_SUCCESS){
        Utility_ModuleUninit(tmpModuleHandle);
    }

End:
    BDBG_LEAVE(Utility_ModuleInit);
    return rc;
}

BERR_Code
Utility_ModuleUninit(SRAI_ModuleHandle moduleHandle)
{
    BDBG_ENTER(Utility_ModuleUninit);

    if(utilityMutex != NULL){
        BKNI_AcquireMutex(utilityMutex);
    }

    if(moduleHandle != NULL){
        BDBG_MSG(("%s - SRAI_Module_Uninit(%p)", __FUNCTION__, moduleHandle));
        SRAI_Module_Uninit(moduleHandle);
    }

    /* sanity check */

    /* if there's one DRM module left calling Uninit, clean everything up
     * Otherwise skip the clean up and decrement the counter. Is this handled by SRAI?*/
    if(Utility_ModuleCounter == 1)
    {
        BDBG_MSG(("%s - Cleaning up Utility TL only parameters ***************************", __FUNCTION__));
        if (platformHandle)
        {
            BDBG_MSG(("%s - SRAI_Platform_Close(%p)", __FUNCTION__, platformHandle));
            SRAI_Platform_Close(platformHandle);
            platformHandle = NULL;
        }

        BKNI_ReleaseMutex(utilityMutex);
        BDBG_MSG(("%s - BKNI_DestroyMutex(%p)", __FUNCTION__, utilityMutex));
        BKNI_DestroyMutex(utilityMutex);
        utilityMutex = NULL;
    }
    else if(Utility_ModuleCounter <= 0)
    {
        BDBG_WRN(("%s - Utility_ModuleCounter value is invalid ('%d').  Possible bad thread exit", __FUNCTION__, Utility_ModuleCounter));
    }
    /* else: remaining modules, do not uninit global variables */

    Utility_ModuleCounter--;

    if (utilityMutex != NULL)
    {
        BKNI_ReleaseMutex(utilityMutex);
        BDBG_MSG(("%s - BKNI_ReleaseMutex(%p)", __FUNCTION__, utilityMutex));
    }

    BDBG_LEAVE(Utility_ModuleUninit);

    return BERR_SUCCESS;
}



uint32_t
Utility_P_CheckDrmBinFileSize(void)
{
    uint32_t tmp_file_size = 0;
    drm_bin_header_t *pHeader = (drm_bin_header_t *)drm_bin_file_buff;

    tmp_file_size = GET_UINT32_FROM_BUF(pHeader->bin_file_size);
    if(tmp_file_size > DEFAULT_DRM_BIN_FILESIZE)
    {
        if(drm_bin_file_buff != NULL){
            SRAI_Memory_Free(drm_bin_file_buff);
            drm_bin_file_buff = NULL;
        }
        drm_bin_file_buff = SRAI_Memory_Allocate(tmp_file_size, SRAI_MemoryType_Shared);

        BKNI_Memset(drm_bin_file_buff, 0x00, tmp_file_size);
    }

    BDBG_MSG(("%s - tmp_file_size=%u", __FUNCTION__, tmp_file_size));

    return tmp_file_size;
}



BERR_Code Utility_P_GetFileSize(const char * filename, uint32_t *filesize)
{
    BERR_Code rc = BERR_SUCCESS;
    FILE *fptr = NULL;
    int pos = 0;

    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    pos = fseek(fptr, 0, SEEK_END);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error seeking to end of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    pos = ftell(fptr);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error determining position of file pointer of file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    /* check vs. max SAGE SRAM size */
    if(pos > DEFAULT_DRM_BIN_FILESIZE)
    {
        BDBG_ERR(("%s - Invalid file size detected for of file '%s'.  (%u)", __FUNCTION__, filename, pos));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    (*filesize) = pos;

ErrorExit:

    if(fptr != NULL)
    {
        /* error closing?!  weird error case not sure how to handle */
        if(fclose(fptr) != 0){
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", __FUNCTION__, filename, strerror(errno)));
            rc = BERR_OS_ERROR;
        }
    }

    BDBG_MSG(("%s - Exiting function (%u bytes)", __FUNCTION__, (*filesize)));

    return rc;
}
