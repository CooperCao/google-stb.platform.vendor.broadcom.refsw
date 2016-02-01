/******************************************************************************
 * (c) 2006-2015 Broadcom Corporation
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
 *
 *****************************************************************************/


#ifndef BDSP_RAAGA_FWINTERFACE_PRIV_H_
#define BDSP_RAAGA_FWINTERFACE_PRIV_H_

#include "bdsp_raaga_types.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_audio_task.h"
#include "bdsp_raaga_cmdresp_priv.h"
#include "bdsp_raaga_fw_settings.h"
#include "bdsp_raaga_fw_status.h"


#define BDSP_RAAGA_P_FIFO_BASE_OFFSET    0
#define BDSP_RAAGA_P_FIFO_END_OFFSET     4
#define BDSP_RAAGA_P_FIFO_READ_OFFSET    12 /*8*/
#define BDSP_RAAGA_P_FIFO_WRITE_OFFSET   8  /*12*/


#define BDSP_RAAGA_P_INVALID_TASK_ID            ((unsigned int)(-1))


/*************************************************************************
Summary:
       Data structure details for the message queue in the system memory

Description:
    Parmeters passed in this structure:-
        Handle for the heap of memory  to be allocated
        Base and End Addresses of the message queue (local copy)
        Read pointer and write pointer addresses (local copy)required for operations like writing a message and reading a message
        The message queue attribute address containing the base address of the message queue needed to be passed to the shared copy in DRAM
        The MSB toggle bits for both the read and write pointers to determine wrap around conditions in the message queue
***************************************************************************/
typedef struct BDSP_Raaga_P_MsgQueue
{
    BMEM_Handle hHeap;
    BREG_Handle hRegister;
    raaga_dramaddr  ui32BaseAddr;    /* phys address */
    raaga_dramaddr  ui32EndAddr;     /* phys address */
    raaga_dramaddr  ui32ReadAddr;     /* phys address */
    raaga_dramaddr  ui32WriteAddr;    /* phys address */
    int32_t  i32FifoId;     /* Fifo Id for this message queue */
    uint32_t ui32DspOffset; /* DSP Register Offset */
    void *pBaseAddr;        /* Virtual address of the buffer provided*/
    uint32_t ui32Size;      /* Size of the Buffer */
} BDSP_Raaga_P_MsgQueue;

typedef struct BDSP_Raaga_P_MsgQueue *BDSP_Raaga_P_MsgQueueHandle;

/***************************************************************************
Summary:
    This enum hold the type of message passed
***************************************************************************/
typedef enum  BDSP_Raaga_P_MsgType
{
    BDSP_Raaga_P_MsgType_eSyn,
    BDSP_Raaga_P_MsgType_eAsyn
}BDSP_Raaga_P_MsgType;

/***************************************************************************
Summary:
    Data structure details of the message queue parameters in the system memory

Description:
    Parmeters passed:-
        Base address(virtual) of the message queue
        Size of the message queue
        Address of the attribute structure address of the message queue

***************************************************************************/
typedef struct BDSP_Raaga_P_MsgQueueParams
{
    void  *pBaseAddr; /* Virtual Address */
    uint32_t uiMsgQueueSize;
    int32_t  i32FifoId;  /* Fifo Id for this message queue */
} BDSP_Raaga_P_MsgQueueParams;

typedef struct BDSP_Raaga_P_RdbQueueParams
{
    int32_t     startIndexOfFreeFifo; /* Start Id of Fifo */
    size_t      uiMsgQueueSize;
    int32_t     i32FifoId;  /* Fifo Id for this message queue */
} BDSP_Raaga_P_RdbQueueParams;

BERR_Code BDSP_Raaga_P_CreateMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    );

BERR_Code BDSP_Raaga_P_InitMsgQueue(
    BDSP_Raaga_P_MsgQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    );

BERR_Code BDSP_Raaga_P_DestroyMsgQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,
    BREG_Handle                 hRegister,          /* [in] */
    uint32_t                    ui32DspOffset       /* [in] */
    );

BERR_Code BDSP_Raaga_P_CreateRdbQueue(
    BDSP_Raaga_P_RdbQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    );

BERR_Code BDSP_Raaga_P_InitRdbQueue(
    BDSP_Raaga_P_RdbQueueParams    *psMsgQueueParams ,  /* [in]*/
    BMEM_Handle                     hHeap,              /* [in] */
    BREG_Handle                     hRegister,          /* [in] */
    uint32_t                        ui32DspOffset,      /* [in] */
    BDSP_Raaga_P_MsgQueueHandle    *hMsgQueue           /* [out]*/
    );

BERR_Code BDSP_Raaga_P_DestroyRdbQueue(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue,
    BREG_Handle                    hRegister,          /* [in] */
    uint32_t                       ui32DspOffset      /* [in] */
    );

BERR_Code BDSP_Raaga_P_SendCommand(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Raaga_P_Command         *psCommand  /*[in]*/ ,
    void *pTaskHandle      /*[in] Task handle */
    );

BERR_Code BDSP_Raaga_P_SendCommand_isr(
    BDSP_Raaga_P_MsgQueueHandle    hMsgQueue   /*[in]*/,
    const BDSP_Raaga_P_Command         *psCommand  /*[in]*/,
    void    *pTaskHandle       /*[in] Task handle */);

/***************************************************************************
Summary:
    Gets a message from the message queue and writes in into the message buffer

Description:
    Sanity check is done to check if the read and write pointers haven't been corrupted in the shared copy.
    Checks if a message is present. If no message is there in the message queue BUFFER_EMPTY error is returned
    MESSAGE_SIZE/4 number of words are read from the msg buffer into the message queue
    Read Pointers are updated both in the shared and the local copy

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BDSP_Raaga_P_GetMsg(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,
    void *pMsgBuf,
    BDSP_Raaga_P_MsgType eMgsType
    );

/***************************************************************************
Summary:
    Gets a message from the message queue and writes in into the message buffer

Description:
    This is the isr version of BDSP_Raaga_P_GetMsg.

Returns:
    BERR_SUCCESS else error

**************************************************************************/
BERR_Code BDSP_Raaga_P_GetMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,
    void *pMsgBuf,
    BDSP_Raaga_P_MsgType eMgsType
    );

BERR_Code BDSP_Raaga_P_SetAlgorithmSettings(
    BMEM_Handle             hHeap,
    BDSP_Algorithm          eAlgorithm,
    void                    *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t                uiConfigBufSize,    /* [in] Config Buf Size */
    const void             *pSettingsBuffer,
    size_t                  settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_GetAlgorithmSettings(
    BMEM_Handle             hHeap,
    BDSP_Algorithm          eAlgorithm,
    void                    *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t                uiConfigBufSize,    /* [in] Config Buf Size */
    void                   *pSettingsBuffer,
    size_t                  settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_SetFrameSyncTsmStageConfigParams(
    BMEM_Handle     hHeap,
    BDSP_Algorithm eAlgorithm,
    void            *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t        uiConfigBufSize,     /* [in] Config Buf Size */
    const void      *pSettingsBuffer,
    size_t          settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_SetFrameSyncTsmStageConfigParams_isr(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    const void         *pSettingsBuffer,
    size_t              settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_GetFrameSyncTsmStageConfigParams(
    BMEM_Handle     hHeap,
    BDSP_Algorithm eAlgorithm,
    void            *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t        uiConfigBufSize,     /* [in] Config Buf Size */
    void           *pSettingsBuffer,
    size_t          settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_GetFrameSyncTsmStageConfigParams_isr(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pConfigBuf,    /* [in] Config Buf Address */
    uint32_t            uiConfigBufSize,     /* [in] Config Buf Size */
    void               *pSettingsBuffer,
    size_t              settingsBufferSize
    );

BERR_Code BDSP_Raaga_P_GetAlgorithmStatus(
    BMEM_Handle         hHeap,
    BDSP_Algorithm      eAlgorithm,
    void                *pStatusBuf,    /* [in] Config Buf Address */
    uint32_t            uiStatusBufSize,    /* [in] Config Buf Size */
    void               *pStatusBuffer,      /*[out]*/
    size_t              statusBufferSize
    );

BERR_Code BDSP_Raaga_P_GetAsyncMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle hMsgQueue,  /*[in]*/
    void                       *pMsgBuf,   /*[in]*/
    unsigned int               *puiNumMsgs /*[out]*/
    );

BERR_Code BDSP_Raaga_P_GetVideoMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle  hMsgQueue,/*[in]*/
    uint32_t *pMsgBuf,
    bool bReadUpdate
    );

BERR_Code BDSP_Raaga_P_WriteVideoMsg_isr(
    BDSP_Raaga_P_MsgQueueHandle   hMsgQueue/*[in]*/,
    void *pMsgBuf, /*[in]*/
    unsigned int uiBufSize/*[in]*/
    );

#endif
