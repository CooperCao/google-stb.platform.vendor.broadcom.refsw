/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: wmdrmpd_io
*    Specific APIs related to Microsoft Windows Media DRM PD I/O
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_wmdrmpd_core_module.h"
#include "nexus_wmdrmpd_io.h"
#include "priv/nexus_wmdrmpd_io_priv.h"

#define LOCK_IO()   NEXUS_Module_Lock(g_NEXUS_wmDrmPdCoreModuleData.settings.modules.wmdrmpdIo)
#define UNLOCK_IO() NEXUS_Module_Unlock(g_NEXUS_wmDrmPdCoreModuleData.settings.modules.wmdrmpdIo)

void *NEXUS_WmDrmPdCore_P_Fopen(const char *pFilename, const char *pMode)
{
    void *pFile=NULL;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eOpen;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    b_strncpy(jobSettings.params.open.fileName, pFilename, sizeof(jobSettings.params.open.fileName));
    b_strncpy(jobSettings.params.open.mode, pMode, sizeof(jobSettings.params.open.mode));
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                if ( errCode != BERR_OS_ERROR ) 
                {
                    (void)BERR_TRACE(errCode);
                }
            }
            else
            {
                pFile = jobStatus.params.open.pFileHandle;
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return pFile;
}

int NEXUS_WmDrmPdCore_P_Fclose(void *pFile)
{
    NEXUS_Error errCode=BERR_UNKNOWN;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eClose;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return (errCode)?-1:0;
}

size_t NEXUS_WmDrmPdCore_P_Fread(void *pBuf, size_t size, size_t count, void *pFile)
{
    size_t ret = (size_t)-1;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eRead;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    jobSettings.pBuffer = pBuf;
    jobSettings.length = size*count;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
            else
            {
                ret = jobStatus.params.read.length;
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return ret;
}


int NEXUS_WmDrmPdCore_P_Unlink(const char *pFilename)
{
    int ret = -1;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eUnlink;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    b_strncpy(jobSettings.params.unlink.fileName, pFilename, sizeof(jobSettings.params.unlink.fileName));
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return ret;
}


int NEXUS_WmDrmPdCore_P_Fseek(void *pFile, long int offset, int whence)
{
    int ret = -1;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eSeek;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    jobSettings.params.seek.offset = offset;
    jobSettings.params.seek.whence = whence;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
            else
            {
                ret = jobStatus.params.seek.offset;
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return ret;
}

int64_t NEXUS_WmDrmPdCore_P_Ftell(void *pFile)
{
    int64_t ret = -1;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eTell;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
            else
            {
                ret = jobStatus.params.tell.offset;
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return ret;
}

size_t NEXUS_WmDrmPdCore_P_Fwrite(void *pBuf, size_t size, size_t count, void *pFile)
{
    size_t ret = (size_t)-1;
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eWrite;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    jobSettings.pBuffer = pBuf;
    jobSettings.length = size*count;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
            else
            {
                ret = jobStatus.params.write.length;
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return ret;
}

int NEXUS_WmDrmPdCore_P_Fflush(void *pFile)
{
    NEXUS_Error errCode=BERR_UNKNOWN;
    NEXUS_WmDrmPdIoJobHandle hJob;
    NEXUS_WmDrmPdIoJobSettings jobSettings;
    NEXUS_WmDrmPdIoJobStatus jobStatus;

    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(NULL != NEXUS_WMDRMPD_GET_ACTIVE_IO());

    BKNI_ResetEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent);

    LOCK_IO();
    NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(&jobSettings);
    jobSettings.operation = NEXUS_WmDrmPdIoOperation_eFlush;
    jobSettings.completionEvent = g_NEXUS_wmDrmPdCoreModuleData.hIoEvent;
    jobSettings.pFileHandle = pFile;
    hJob = NEXUS_WmDrmPdIoJob_Create_priv(NEXUS_WMDRMPD_GET_ACTIVE_IO(), &jobSettings);
    UNLOCK_IO();
    if ( hJob )
    {
        errCode = BKNI_WaitForEvent(g_NEXUS_wmDrmPdCoreModuleData.hIoEvent, g_NEXUS_wmDrmPdCoreModuleData.settings.ioRequestTimeout);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
        else
        {
            LOCK_IO();
            errCode = NEXUS_WmDrmPdIoJob_GetStatus_priv(hJob, &jobStatus);
            UNLOCK_IO();
            if ( errCode || (errCode = jobStatus.errCode) )
            {
                (void)BERR_TRACE(errCode);
            }
        }
        LOCK_IO();
        NEXUS_WmDrmPdIoJob_Destroy_priv(hJob);
        UNLOCK_IO();
    }
    return (errCode)?-1:0;
}

