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
#ifndef NEXUS_WMDRMPD_IO_H_
#define NEXUS_WMDRMPD_IO_H_

/***************************************************************************
Summary: 
WMDRMPD I/O Handle
***************************************************************************/
typedef struct NEXUS_WmDrmPdIo *NEXUS_WmDrmPdIoHandle;

/***************************************************************************
Summary: 
WMDRMPD I/O Settings
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoSettings
{
    unsigned heapIndex;                 /* Optional heap to allocate internal buffers.  Must be CPU accessible. */
    NEXUS_CallbackDesc jobCallback;     /* Callback fires when a new I/O job is queued */
} NEXUS_WmDrmPdIoSettings;

/***************************************************************************
Summary: 
Get Default WMDRMPD I/O Settings
***************************************************************************/
void NEXUS_WmDrmPdIo_GetDefaultSettings(
    NEXUS_WmDrmPdIoSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary: 
Create a WMDRMPD I/O instance
***************************************************************************/
NEXUS_WmDrmPdIoHandle NEXUS_WmDrmPdIo_Create( /* attr{destructor=NEXUS_WmDrmPdIo_Destroy} */
    const NEXUS_WmDrmPdIoSettings *pSettings    /* attr{null_allowed=y} Optional.  Pass NULL for default settings. */
    );

/***************************************************************************
Summary: 
Destroy a WMDRMPD I/O instance
***************************************************************************/
void NEXUS_WmDrmPdIo_Destroy(
    NEXUS_WmDrmPdIoHandle handle
    );

/***************************************************************************
Summary: 
WMDRMPD I/O Job Operations
***************************************************************************/
typedef enum NEXUS_WmDrmPdIoOperation
{
    NEXUS_WmDrmPdIoOperation_eNone,
    NEXUS_WmDrmPdIoOperation_eOpen,
    NEXUS_WmDrmPdIoOperation_eClose,
    NEXUS_WmDrmPdIoOperation_eRead,
    NEXUS_WmDrmPdIoOperation_eWrite,
    NEXUS_WmDrmPdIoOperation_eSeek,
    NEXUS_WmDrmPdIoOperation_eTell,
    NEXUS_WmDrmPdIoOperation_eFlush,
    NEXUS_WmDrmPdIoOperation_eUnlink,
    NEXUS_WmDrmPdIoOperation_eMax
} NEXUS_WmDrmPdIoOperation;

/***************************************************************************
Summary: 
WMDRMPD I/O Job Request Definitions
***************************************************************************/
#define NEXUS_WMDRMPDIO_MAX_FNAME (64)
#define NEXUS_WMDRMPDIO_MAX_MODE  (8)

/***************************************************************************
Summary: 
WMDRMPD I/O Job Request
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoRequest
{
    NEXUS_WmDrmPdIoOperation operation; /* Requested Operation */
    unsigned requestId;                 /* Request ID */
    void *pFileHandle;                  /* File Handle (applicable to all operations except open) */
    void *pBuffer;                      /* attr{memory=cached} I/O Buffer (applicable to read/write) */
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
} NEXUS_WmDrmPdIoRequest;

/***************************************************************************
Summary: 
Get Next WMDRMPD I/O Job Request
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPdIo_GetRequest(
    NEXUS_WmDrmPdIoHandle handle,
    NEXUS_WmDrmPdIoRequest *pRequest    /* [out] */
    );

/***************************************************************************
Summary: 
WMDRMPD I/O Job Response
***************************************************************************/
typedef struct NEXUS_WmDrmPdIoResponse
{
    NEXUS_WmDrmPdIoOperation operation; /* Requested Operation */
    unsigned requestId;     /* Request ID being acknowledged */
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
} NEXUS_WmDrmPdIoResponse;

/***************************************************************************
Summary: 
Initialize WMDRMPD I/O Response Structure
***************************************************************************/
void NEXUS_WmDrmPdIo_GetDefaultIoResponse(
    NEXUS_WmDrmPdIoHandle handle,
    const NEXUS_WmDrmPdIoRequest *pRequest, /* Request Parameters */
    NEXUS_WmDrmPdIoResponse *pResponse      /* [out] */
    );

/***************************************************************************
Summary: 
Send WMDRMPD I/O Response
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPdIo_SendResponse(
    NEXUS_WmDrmPdIoHandle handle,
    const NEXUS_WmDrmPdIoResponse *pResponse
    );

#endif /* #ifndef NEXUS_WMDRMPD_IO_H_ */

