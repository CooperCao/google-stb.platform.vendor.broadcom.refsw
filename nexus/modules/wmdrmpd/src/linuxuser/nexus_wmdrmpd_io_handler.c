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
*   API name: wmdrmpd
*    Specific APIs related to Microsoft Windows Media DRM PD
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_wmdrmpd_module.h"
#include <stdio.h>

BDBG_MODULE(nexus_wmdrmpd_io_handler);

void NEXUS_WmDrmPd_P_ProcessIoRequest(
    const NEXUS_WmDrmPdIoRequest *pRequest, 
    NEXUS_WmDrmPdIoResponse *pResponse /* [out] */
    )
{
    FILE *pFile;
    ssize_t size;
    off_t offset;
    int ret;

    pFile = pRequest->pFileHandle;

    /* Assume success until proven otherwise */
    pResponse->errCode = NEXUS_SUCCESS;
    switch ( pRequest->operation )
    {
    case NEXUS_WmDrmPdIoOperation_eOpen:
        BDBG_MSG(("Open: '%s' mode:'%s", pRequest->params.open.fileName, pRequest->params.open.mode));
        pFile = fopen(pRequest->params.open.fileName, pRequest->params.open.mode);
        BDBG_MSG(("Open Returned FILE %#x", pFile));
        pResponse->params.open.pFileHandle = pFile;
        if ( NULL == pFile )
        {
            pResponse->errCode = NEXUS_SUCCESS;
        }
        break;
    case NEXUS_WmDrmPdIoOperation_eClose:
        BDBG_MSG(("Close: %#x", pFile));
        fclose(pFile);
        break;
    case NEXUS_WmDrmPdIoOperation_eRead:
        BDBG_MSG(("Read: %#x, %u, 1, %#x", pRequest->pBuffer, (unsigned)pRequest->length, pFile));
        size = fread(pRequest->pBuffer, 1, pRequest->length, pFile);
        BDBG_MSG(("Read Returned %d ", (int)size));
        if ( size < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
            pResponse->params.read.length = 0;
        }
        else
        {
            pResponse->params.read.length = size;
        }
        break;
    case NEXUS_WmDrmPdIoOperation_eWrite:
        BDBG_MSG(("Write: %#x, %u, 1, %#x", pRequest->pBuffer, (unsigned)pRequest->length, pFile));
        size = fwrite(pRequest->pBuffer, 1, pRequest->length, pFile);
        BDBG_MSG(("Write Returned %d", (int)size));
        if ( size < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
            pResponse->params.write.length = 0;
        }
        else
        {
            pResponse->params.write.length = size;
        }        
        break;
    case NEXUS_WmDrmPdIoOperation_eSeek:
        BDBG_MSG(("Seek: %#x, %u, %d", pFile, (unsigned)pRequest->params.seek.offset, pRequest->params.seek.whence));
        offset = fseek(pFile, pRequest->params.seek.offset, pRequest->params.seek.whence);
        BDBG_MSG(("Seek Returned %d", (int)offset));
        if ( offset < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
        }
        else
        {
            pResponse->params.seek.offset = offset;
        }
        break;
    case NEXUS_WmDrmPdIoOperation_eTell:
        BDBG_MSG(("Tell: %#x", pFile));
        offset = ftell(pFile);
        BDBG_MSG(("Tell Returned %d", (int)offset));
        if ( offset < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
        }
        else
        {
            pResponse->params.seek.offset = offset;
        }
        break;
    case NEXUS_WmDrmPdIoOperation_eFlush:
        BDBG_MSG(("Flush: %#x", pFile));
        ret = fflush(pFile);
        BDBG_MSG(("Flush returned %d", ret));
        if ( ret < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
        }
        break;
    case NEXUS_WmDrmPdIoOperation_eUnlink:
        BDBG_MSG(("unlink: %#x", pRequest->params.unlink.fileName));
        ret = unlink(pRequest->params.unlink.fileName);
        BDBG_MSG(("unlink returned %d", ret));
        if ( ret < 0 )
        {
            pResponse->errCode = BERR_OS_ERROR;
        }
        break;

    default:
        break;
    }
}

