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
*   API name: wmdrmpd_io_priv
*    Private I/O APIs intended to be called by the wmdrm_core module.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_WMDRMPD_IO_PRIV_H_
#define NEXUS_WMDRMPD_IO_PRIV_H_

#include "nexus_wmdrmpd_io.h"

/***************************************************************************
Summary: 
WMDRMPD I/O Job Handle
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoJob *NEXUS_WmDrmPdIoJobHandle;

/***************************************************************************
Summary: 
WMDRMPD I/O Job Settings
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoJobSettings
{
    NEXUS_WmDrmPdIoOperation operation;
    BKNI_EventHandle completionEvent;   /* Completion Event.  This will be set when the job completes */
    void *pFileHandle;                  /* File Handle (applicable to all operations except open) */
    void *pBuffer;                      /* I/O Buffer (applicable to read/write) */
    unsigned length;                    /* Length of request in bytes */
    union
    {
        struct
        {
            char fileName[NEXUS_WMDRMPDIO_MAX_FNAME];
            char mode[NEXUS_WMDRMPDIO_MAX_MODE];
        } open;
        struct
        {
            char fileName[NEXUS_WMDRMPDIO_MAX_FNAME];
        } unlink;
        struct
        {
            int whence;
            int offset;
        } seek;
    } params;    
} NEXUS_WmDrmPdIoJobSettings;

/***************************************************************************
Summary: 
Get Default WMDRMPD I/O Job Settings
***************************************************************************/
void NEXUS_WmDrmPdIoJob_GetDefaultSettings_priv(
    NEXUS_WmDrmPdIoJobSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary: 
Create a WMDRMPD I/O Job
***************************************************************************/
NEXUS_WmDrmPdIoJobHandle NEXUS_WmDrmPdIoJob_Create_priv(
    NEXUS_WmDrmPdIoHandle ioHandle,
    const NEXUS_WmDrmPdIoJobSettings *pSettings
    );

/***************************************************************************
Summary: 
Destroy a WMDRMPD I/O Job
***************************************************************************/
void NEXUS_WmDrmPdIoJob_Destroy_priv(
    NEXUS_WmDrmPdIoJobHandle jobHandle
    );

/***************************************************************************
Summary: 
WMDRMPD I/O Job Status
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoJobStatus
{
    bool pending;
    NEXUS_WmDrmPdIoOperation operation; /* Requested Operation */
    NEXUS_Error errCode;    /* Status of the job request */
    union
    {
        struct
        {
            void *pFileHandle;  /* Handle returned from open if successful */
        } open;
        struct
        {
            unsigned length;
        } read, write;
        struct
        {
            int offset;
        } seek, tell;
    } params;
} NEXUS_WmDrmPdIoJobStatus;

/***************************************************************************
Summary: 
Get WMDRMPD I/O Job Status
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPdIoJob_GetStatus_priv(
    NEXUS_WmDrmPdIoJobHandle jobHandle,
    NEXUS_WmDrmPdIoJobStatus *pStatus   /* [out] */
    );

#endif /* #ifndef NEXUS_WMDRMPD_IO_PRIV_H_ */

