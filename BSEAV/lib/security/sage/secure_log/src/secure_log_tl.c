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

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_otpmsp.h"

#include "secure_log_ids.h"
#include "bsagelib_types.h"
#include "sage_srai.h"
#include "secure_log_ids.h"
#include "secure_log_tl.h"

#include "nexus_base_os.h"

BDBG_MODULE(secure_log_tl);

#define SECURE_LOG_TA_NAME_PRODUCTION  "sage_ta_secure_log.bin"
#define SECURE_LOG_TA_NAME_DEVELOPMENT "sage_ta_secure_log_dev.bin"
#define SAGEBIN_DEFAULT_PATH           "."
#define OTP_MSP0_VALUE_ZS (0x02)
#define OTP_MSP1_VALUE_ZS (0x02)
#define OTP_MSP0_VALUE_ZB (0x3E)
#define OTP_MSP1_VALUE_ZB (0x3F)

/* Chipset type */
typedef enum
{
    ChipType_eZS = 0,
    ChipType_eZB,
    ChipType_eCustomer,
    ChipType_eCustomer1
} ChipType_e;

static SRAI_ModuleHandle moduleHandle = NULL;
static uint32_t ConfRingBuffSize = 0;

static int Secure_Log_ModuleCounter = 0;
static BKNI_MutexHandle secure_logMutex = NULL;

static SRAI_PlatformHandle platformHandle = NULL;

static BERR_Code Secure_Log_P_GetFileSize(const char * , uint32_t *);
static BERR_Code Secure_Log_P_TA_Install(char *);
static ChipType_e Secure_Log_P_GetChipType(void);
static BERR_Code Secure_Log_ModuleUninit(SRAI_ModuleHandle);
static BERR_Code Secure_Log_ModuleInit(Secure_Log_ModuleId_e,BSAGElib_InOutContainer *, SRAI_ModuleHandle *);


static BERR_Code
Secure_Log_ModuleInit(Secure_Log_ModuleId_e module_id,
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
#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
    char * ta_bin_filename;
#endif
    BDBG_ENTER(Secure_Log_ModuleInit);

    if(secure_logMutex == NULL)
    {
        if(BKNI_CreateMutex(&secure_logMutex) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error calling create mutex", BSTD_FUNCTION));
            rc = BERR_OS_ERROR;
            goto End;
        }
    }
    BKNI_AcquireMutex(secure_logMutex);

    if (moduleHandle == NULL)
    {
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    (*moduleHandle) = NULL;


    if(platformHandle == NULL)
    {
#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)

        if(Secure_Log_P_GetChipType() == ChipType_eZS)
            ta_bin_filename = SECURE_LOG_TA_NAME_DEVELOPMENT;
        else
            ta_bin_filename = SECURE_LOG_TA_NAME_PRODUCTION;

        sage_rc = Secure_Log_P_TA_Install(ta_bin_filename);

        if (sage_rc != BERR_SUCCESS)
        {
            BDBG_WRN(("%s - Could not Install TA %s: Make sure you have the TA binary", BSTD_FUNCTION, ta_bin_filename));
            rc = sage_rc;
        }
#endif
        BSAGElib_State platform_status;
        sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_SECURE_LOGGING, &platform_status, &platformHandle);
        if (sage_rc != BERR_SUCCESS)
        {
            platformHandle = NULL; /* sanity reset */
            BDBG_ERR(("%s - Error calling platform_open", BSTD_FUNCTION));
            rc = sage_rc;
            goto ErrorExit;
        }

        BDBG_MSG(("%s - SRAI_Platform_Open(%u, %p, %p) returned %p", BSTD_FUNCTION, BSAGE_PLATFORM_ID_SECURE_LOGGING, (void *)&platform_status, (void *)&platformHandle, (void *)platformHandle));

        if(platform_status == BSAGElib_State_eUninit)
        {
            BDBG_WRN(("%s - platform_status == BSAGElib_State_eUninit ************************* (platformHandle = 0x%08x)", BSTD_FUNCTION, (uint32_t)platformHandle));
            sage_rc = SRAI_Platform_Init(platformHandle, NULL);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error calling platform init", BSTD_FUNCTION));
                rc = sage_rc;
                goto ErrorExit;
            }
        }
        else{
            BDBG_WRN(("%s - Platform already initialized *************************", BSTD_FUNCTION));
        }
    }

    Secure_Log_ModuleCounter++;

    /* All modules will call SRAI_Module_Init */
    BDBG_MSG(("%s - ************************* (platformHandle = 0x%08x)", BSTD_FUNCTION, (uint32_t)platformHandle));
    sage_rc = SRAI_Module_Init(platformHandle, module_id, container, &tmpModuleHandle);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Module_Init", BSTD_FUNCTION));
        rc = sage_rc;
        goto ErrorExit;
    }
    BDBG_MSG(("%s - SRAI_Module_Init(%p, %u, %p, %p) returned %p",
              BSTD_FUNCTION, (void *)platformHandle, module_id, (void *)container, (void *)&tmpModuleHandle, (void *)moduleHandle));

    /* success */
    *moduleHandle = tmpModuleHandle;

ErrorExit:

    if(container && container->blocks[0].data.ptr != NULL){
        SRAI_Memory_Free(container->blocks[0].data.ptr);
        container->blocks[0].data.ptr = NULL;
    }

    if(fptr != NULL){
        fclose(fptr);
        fptr = NULL;
    }

    BKNI_ReleaseMutex(secure_logMutex);

    if (rc != BERR_SUCCESS){
        Secure_Log_ModuleUninit(tmpModuleHandle);
    }

End:
    BDBG_LEAVE(Secure_Log_ModuleInit);
    return rc;
}

static BERR_Code
Secure_Log_ModuleUninit(SRAI_ModuleHandle moduleHandle)
{
    BDBG_ENTER(Secure_Log_ModuleUninit);

    if(secure_logMutex != NULL){
        BKNI_AcquireMutex(secure_logMutex);
    }

    if(moduleHandle != NULL){
        BDBG_MSG(("%s - SRAI_Module_Uninit(%p)", BSTD_FUNCTION, (void *)moduleHandle));
        SRAI_Module_Uninit(moduleHandle);
    }

    /* sanity check */

    /* if there's one DRM module left calling Uninit, clean everything up
     * Otherwise skip the clean up and decrement the counter. Is this handled by SRAI?*/
    if(Secure_Log_ModuleCounter == 1)
    {
        BDBG_MSG(("%s - Cleaning up Secure_Log TL only parameters ***************************", BSTD_FUNCTION));
        if (platformHandle)
        {
            BDBG_MSG(("%s - SRAI_Platform_Close(%p)", BSTD_FUNCTION, (void *)platformHandle));
            SRAI_Platform_Close(platformHandle);
            platformHandle = NULL;
        }

        SRAI_Platform_UnInstall(BSAGE_PLATFORM_ID_SECURE_LOGGING);

        BKNI_ReleaseMutex(secure_logMutex);
        BDBG_MSG(("%s - BKNI_DestroyMutex(%p)", BSTD_FUNCTION, (void *)secure_logMutex));
        BKNI_DestroyMutex(secure_logMutex);
        secure_logMutex = NULL;
    }
    else if(Secure_Log_ModuleCounter <= 0)
    {
        BDBG_WRN(("%s - Secure_Log_ModuleCounter value is invalid ('%d').  Possible bad thread exit", BSTD_FUNCTION, Secure_Log_ModuleCounter));
    }
    /* else: remaining modules, do not uninit global variables */

    Secure_Log_ModuleCounter--;

    if (secure_logMutex != NULL)
    {
        BKNI_ReleaseMutex(secure_logMutex);
        BDBG_MSG(("%s - BKNI_ReleaseMutex(%p)", BSTD_FUNCTION, (void *)secure_logMutex));
    }

    BDBG_LEAVE(Secure_Log_ModuleUninit);

    return BERR_SUCCESS;
}

static BERR_Code Secure_Log_P_GetFileSize(const char * filename, uint32_t *filesize)
{
    BERR_Code rc = BERR_SUCCESS;
    FILE *fptr = NULL;
    int pos = 0;

    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    pos = fseek(fptr, 0, SEEK_END);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error seeking to end of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    pos = ftell(fptr);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error determining position of file pointer of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    (*filesize) = pos;

ErrorExit:

    if(fptr != NULL)
    {
        /* error closing?!  weird error case not sure how to handle */
        if(fclose(fptr) != 0){
            BDBG_ERR(("%s - Error closing drm bin file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
            rc = BERR_OS_ERROR;
        }
    }

    BDBG_MSG(("%s - Exiting function (%u bytes)", BSTD_FUNCTION, (*filesize)));

    return rc;
}

static BERR_Code Secure_Log_P_TA_Install(char * ta_bin_filename)
{
    BERR_Code rc = BERR_SUCCESS;
    FILE * fptr = NULL;
    uint32_t file_size = 0;
    uint32_t read_size = 0;
    uint8_t *ta_bin_file_buff = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;
    char *path = NULL;
    char ta_name[512];

    BDBG_MSG(("%s - TA bin filename '%s'", BSTD_FUNCTION, ta_bin_filename));

    path = (char *)NEXUS_GetEnv("SAGEBIN_PATH");
    if (path == NULL)
    {
       path = SAGEBIN_DEFAULT_PATH;
    }
    sprintf(ta_name, "%s/%s", path, ta_bin_filename);

    rc = Secure_Log_P_GetFileSize(ta_name, &file_size);
    if(rc != BERR_SUCCESS)
    {
        BDBG_LOG(("%s - Error determine file size of TA bin file", BSTD_FUNCTION));
        goto ErrorExit;
    }

    ta_bin_file_buff = SRAI_Memory_Allocate(file_size, SRAI_MemoryType_Shared);
    if(ta_bin_file_buff == NULL)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for loading TA bin file", BSTD_FUNCTION, file_size));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    fptr = fopen(ta_name, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening TA bin file (%s)", BSTD_FUNCTION, ta_name));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    read_size = fread(ta_bin_file_buff, 1, file_size, fptr);
    if(read_size != file_size)
    {
        BDBG_ERR(("%s - Error reading TA bin file size (%u != %u)", BSTD_FUNCTION, read_size, file_size));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }

    /* close file and set to NULL */
    if(fclose(fptr) != 0)
    {
        BDBG_ERR(("%s - Error closing TA bin file '%s'.  (%s)", BSTD_FUNCTION, ta_name, strerror(errno)));
        rc = BERR_OS_ERROR;
        goto ErrorExit;
    }
    fptr = NULL;

    BDBG_MSG(("%s - TA 0x%x Install file %s", BSTD_FUNCTION,BSAGE_PLATFORM_ID_SECURE_LOGGING, ta_name));

    sage_rc = SRAI_Platform_Install(BSAGE_PLATFORM_ID_SECURE_LOGGING, ta_bin_file_buff, file_size);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Platform_Install Error 0x%x", BSTD_FUNCTION, sage_rc ));
        rc = sage_rc;
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

    return rc;
}

static ChipType_e Secure_Log_P_GetChipType()
{

    NEXUS_ReadMspParms     readMspParms;
    NEXUS_ReadMspIO        readMsp0;
    NEXUS_ReadMspIO        readMsp1;
    NEXUS_Error rc =  NEXUS_SUCCESS;

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
}


BERR_Code
Secure_Log_TlInit(void)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(Secure_Log_TlInit);

    container = SRAI_Container_Allocate();
    BDBG_ASSERT(container);

    /* Initialize SAGE Secure Log module */
    rc = Secure_Log_ModuleInit(Secure_Log_ModuleId,
                           container,
                           &moduleHandle);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing Secure Log TL module (0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:

    if(container != NULL){
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(Secure_Log_TlInit);

    return rc;
}

BERR_Code
Secure_Log_TlUninit(void)
{
    BDBG_ENTER(Secure_Log_TlUninit);


    /* After sending UnInit command to SAGE, close the module handle (i.e. send DRM_Adobe_UnInit to SAGE) */
    if(moduleHandle != NULL){
        Secure_Log_ModuleUninit(moduleHandle);
        moduleHandle = NULL;
    }

    BDBG_LEAVE(Secure_Log_TlUninit);
    return BERR_SUCCESS;
}

/*
 * All the memory blocks are passed by application by allocating memory
 * using SRAI_Memory_Allocate API.
 */

BERR_Code
Secure_Log_TlConfigureBuffer(uint8_t *certificateBin,
                             uint32_t certificateSize,
                             uint32_t ringBuffSize)
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container = NULL;
    BDBG_ENTER(Secure_Log_TlConfigureBuffer);

    if(NULL == moduleHandle){
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s - Secure Log TL module not initialized",BSTD_FUNCTION));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    BDBG_ASSERT(container);

    container->blocks[0].len = certificateSize;
    container->blocks[0].data.ptr = (uint8_t* )certificateBin;

    container->basicIn[0] = ringBuffSize;

    rc = SRAI_Module_ProcessCommand(moduleHandle,
                                    Secure_Log_CommandId_eConfigureBuffer,
                                    container);

ErrorExit:
    BDBG_LEAVE(Secure_Log_TlConfigureBuffer);
    return rc;
}

BERR_Code
Secure_Log_TlAttach(uint32_t TA_Id)
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container = NULL;
    BDBG_ENTER(Secure_Log_TlAttach);

    if(NULL == moduleHandle){
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s - Secure Log TL module not initialized",BSTD_FUNCTION));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();

    BDBG_ASSERT(container);

    container->basicIn[0] = TA_Id;

    rc = SRAI_Module_ProcessCommand(moduleHandle,
                                    Secure_Log_CommandId_eAttach,
                                    container);

ErrorExit:
    BDBG_LEAVE(Secure_Log_TlAttach);
    return rc;
}

BERR_Code
Secure_Log_TlDetach(uint32_t TA_Id)
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container = NULL;
    BDBG_ENTER(Secure_Log_TlDetach);

    if(NULL == moduleHandle){
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s - Secure Log TL module not initialized",BSTD_FUNCTION));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();

    BDBG_ASSERT(container);

    container->basicIn[0] = TA_Id;

    rc = SRAI_Module_ProcessCommand(moduleHandle,
                                    Secure_Log_CommandId_eDetach,
                                    container);
ErrorExit:
    BDBG_LEAVE(Secure_Log_TlDetach);
    return rc;
}

/* GLRBufferSize should be equal to RingBuffSize,
 * for Buffer attached to the TA Id. All the memory blocks
 * are passed by application by allocating memory using SRAI_Memory_Allocate API.
 */

BERR_Code
Secure_Log_TlGetBuffer(Secure_Log_TlBufferContext *pSecureLogBuffCtx,
                        uint8_t *pGLRBufferAddr,
                        uint32_t GLRBufferSize,
                        uint32_t TA_Id)
{
    BERR_Code rc;
    BSAGElib_InOutContainer *container = NULL;
    BDBG_ENTER(Secure_Log_TlGetBuffer);

    if(NULL == moduleHandle){
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s - Secure Log TL module not initialized",BSTD_FUNCTION));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();

    BDBG_ASSERT(container);

    container->basicIn[0] = TA_Id;

    container->blocks[0].len = sizeof(Secure_Log_TlBufferContext);
    container->blocks[0].data.ptr = (uint8_t*) pSecureLogBuffCtx;

    container->blocks[1].len = GLRBufferSize;
    container->blocks[1].data.ptr = (uint8_t*)pGLRBufferAddr;

    rc = SRAI_Module_ProcessCommand(moduleHandle,
                                    Secure_Log_CommandId_eGetBuffer,
                                    container);
ErrorExit:
    BDBG_LEAVE(Secure_Log_TlGetBuffer);
    return rc;
}
