/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

#include <inttypes.h>
#include "basp.h"
#include "basp_priv.h"

#include "bchp_asp_arcss_host_msg_xl.h" /*TODO: This will be removed once reg structure is restructured. */
#include "bchp_asp_arcss_host_h2fw_l2.h"
#include "bchp_asp_arcss_host_fw2h_l2.h"


BDBG_MODULE(BASP_MSGQUEUES);

BDBG_OBJECT_ID_DECLARE(BASP_P_Device);

BDBG_OBJECT_ID(BASP_P_Msgqueue);


#define BASP_MSGQUEUE_BUFFER_ALLIGNMENT 4

typedef enum BASP_MsgqueueMessageState
{
    BASP_MsgqueueMessageState_eNoMessage = 0,                   /* Msgqueue is empty (initial value). */
    BASP_MsgqueueMessageState_eWaitingForPreprocess,            /* Message is waiting to be pre-processed by base module. */
    BASP_MsgqueueMessageState_eWaitingForReadByApp,             /* Message is waiting for GetReadData() to be called. */
    BASP_MsgqueueMessageState_eWaitingForReadCompleteByApp,     /* Message is waiting for ReadComplete() to be called. */
    BASP_MsgqueueMessageState_eMax
} BASP_MsgqueueMessageState;

typedef struct BASP_P_Platform_MsgFifo
{
    uint64_t    ui64Base;
    uint64_t    ui64End;
    uint64_t    ui64Read;
    uint64_t    ui64Write;
}BASP_P_Platform_MsgFifo;

typedef struct BASP_P_Msgqueue
{
    BDBG_OBJECT(BASP_P_Msgqueue)
    BASP_Handle                     hAsp;            /* Handle of the ASP that this Msgqueue belongs to. */
    const char                     *pName;           /* Printable queue name (null-terminated). */
    const BASP_P_Platform_MsgFifo  *pMsgFifo;        /* Pointer to the struct that holds the register addresses for this queue. */
    uint32_t                       queueSize;        /* Size of the queue in bytes. */
    size_t                         queueMsgSize;     /* Size of queue's messages in bytes. */
    BASP_P_Buffer_Handle           hBuffer;          /* Handle of the queue's data buffer. */
    uint8_t                        *pBuffer;         /* Virtual address of the queue's data buffer. */
    uint64_t                       deviceOffset;     /* Physical address of the queue's data buffer. */
    BLST_S_ENTRY(BASP_P_Msgqueue)  nextMsgqueue;     /* BLST link field for ASP's list of Msgqueues. */
    uint32_t                       nextMessageCount; /* Counter for BASP_MessageHeader.ui32MessageCounter. */
    BASP_MsgqueueMessageState      topMessageState;  /* State of top/first/oldest message in queue. */
    size_t                         topMessageSize;   /* Size of top/first/oldest message in queue. */
    BASP_Fw2Pi_Message             *pTopMessage ;    /* Address of top/first/oldest message in queue. */
} BASP_P_Msgqueue;

typedef struct BASP_P_MsgqueuePointers
{
    uint64_t        base;
    uint64_t        end;
    uint64_t        read;
    uint64_t        write;
} BASP_P_MsgqueuePointers;


/* Populate the structure with the addresses of the registers that hold the queue pointers
 * for each of the four message queues. */
struct
{
    BASP_P_Platform_MsgFifo     sFw2HostMsgFifo;
    BASP_P_Platform_MsgFifo     sHost2FwMsgFifo;
    BASP_P_Platform_MsgFifo     sHost2FwRaFifo; /*!< This is for fw response for reassembled packets. */
    BASP_P_Platform_MsgFifo     sFw2HostRaFifo; /*!< This is for host to fw reassembled packets. */
} sMsgRegisters =
{
         /* sFw2HostMsgFifo */
    {
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_HOST_BASE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_HOST_END_ADDR,  /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_HOST_READ_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
       BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_HOST_WRITE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
    },

    /* sHost2FwMsgFifo */
    {
        BCHP_ASP_ARCSS_HOST_MSG_XL_HOST_TO_FW_BASE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_HOST_TO_FW_END_ADDR,  /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_HOST_TO_FW_READ_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_HOST_TO_FW_WRITE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
    },

    /* sHost2FwRaFifo */
    {
        BCHP_ASP_ARCSS_HOST_MSG_XL_RA_TO_FW_BASE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_RA_TO_FW_END_ADDR,  /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_RA_TO_FW_READ_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_RA_TO_FW_WRITE_ADDR,/* bchp_asp_arcss_host_msg_xl.h */
    },

    /* sFw2HostRaFifo */
    {
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_RA_BASE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_RA_END_ADDR,  /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_RA_READ_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
        BCHP_ASP_ARCSS_HOST_MSG_XL_FW_TO_RA_WRITE_ADDR, /* bchp_asp_arcss_host_msg_xl.h */
    },

};

/******************************************************************************/
/* Read a Msgqueue's pointer registers into the caller's structure. */
/******************************************************************************/
void BASP_Msgqueue_ReadRegisters_isrsafe(BASP_MsgqueueHandle hMsgqueue, BASP_P_MsgqueuePointers  *pQueuePointers)
{
    BASP_Handle hAsp;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    hAsp = hMsgqueue->hAsp;

    pQueuePointers->base  = BREG_Read64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Base);
    pQueuePointers->end   = BREG_Read64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64End);
    pQueuePointers->read  = BREG_Read64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Read);
    pQueuePointers->write = BREG_Read64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Write);
}

/******************************************************************************/
/* Determine the available space in a Msgqueue. */
/******************************************************************************/
uint32_t BASP_Msgqueue_Availability(BASP_P_MsgqueuePointers *pQueuePointers)
{
    if (pQueuePointers->write >= pQueuePointers->read)
    {
        return (pQueuePointers->end + 4 - pQueuePointers->write);
    }
    else
    {
        return (pQueuePointers->read - pQueuePointers->write);
    }
}

/******************************************************************************/
/* Determine the available space in a Msgqueue. */
/******************************************************************************/
uint32_t BASP_Msgqueue_Fullness_isrsafe(BASP_P_MsgqueuePointers *pQueuePointers)
{
    if (pQueuePointers->write >= pQueuePointers->read)
    {
        return (pQueuePointers->write - pQueuePointers->read);
    }
    else
    {
        return (pQueuePointers->end + 4 - pQueuePointers->read);
    }
}

/******************************************************************************/
/* Create a new Msgqueue of the specified type. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_Create(
    BASP_Handle hAsp,
    BASP_MsgqueueType msgqueueType,
    uint32_t queueSize,
    BASP_MsgqueueHandle *pHandle /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BASP_MsgqueueHandle hMsgqueue;
    const BASP_P_Platform_MsgFifo     *pMsgFifo = NULL;
    const char *pName;
    size_t queueMsgSize = 0;

    BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(0 != queueSize);

    *pHandle = NULL;

    /* Make sure the msgqueue type is valid */
    switch(msgqueueType)
    {
    case BASP_MsgqueueType_eFwToHost:
        pMsgFifo = &sMsgRegisters.sFw2HostMsgFifo;
        pName = "FwToHost";
        queueMsgSize = sizeof (BASP_Fw2Pi_Message);
        break;
    case BASP_MsgqueueType_eHostToFw:
        pMsgFifo = &sMsgRegisters.sHost2FwMsgFifo;
        pName = "HostToFw";
        queueMsgSize = sizeof (BASP_Pi2Fw_Message);
        break;
    case BASP_MsgqueueType_eRaToFw:
        pMsgFifo = &sMsgRegisters.sHost2FwRaFifo;
        pName = "RaToFw";
        queueMsgSize = sizeof (BASP_Ra2Fw_Message);
        break;
    case BASP_MsgqueueType_eFwToRa:
        pMsgFifo = &sMsgRegisters.sFw2HostRaFifo;
        queueMsgSize = sizeof (BASP_Fw2Ra_Message);
        pName = "FwToRa";
        break;
    default:
        BDBG_ERR(("BASP_MsgqueueType: %u is invalid or not supported", msgqueueType ));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
        break;
    }

    hMsgqueue = BKNI_Malloc(sizeof(BASP_P_Msgqueue));

    if ( NULL == hMsgqueue )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset(hMsgqueue, 0, sizeof(*hMsgqueue));
    BDBG_OBJECT_SET(hMsgqueue, BASP_P_Msgqueue);

    BLST_S_INSERT_HEAD(&hAsp->msgqueueList, hMsgqueue, nextMsgqueue);

    hMsgqueue->pMsgFifo     = pMsgFifo;
    hMsgqueue->pName        = pName;
    hMsgqueue->queueMsgSize = queueMsgSize;
    hMsgqueue->hAsp         = hAsp;
    hMsgqueue->topMessageState = BASP_MsgqueueMessageState_eNoMessage;

    /* Allocate memory for the msgqueue buffer.  The physical address of the MMA memory
     * will be locked to insure it doesn't get shuffled (physically) by the MMA while
     *  the ASP has the physical addresses of the queue. */
    errCode = BASP_P_AllocateDeviceMemory(hAsp->aspCommMemory.hAllocator,    /* Use this allocator */
                                          &hMsgqueue->hBuffer,                  /* Put allocated buffer handle here */
                                          queueSize,                            /* Allocate this many bytes */
                                          BASP_MSGQUEUE_BUFFER_ALLIGNMENT);     /* Aligned like this */
    if ( BERR_SUCCESS != errCode )
    {
        BDBG_ERR(("%s:Error allocating Msgqueue buffer", BSTD_FUNCTION));
        return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
    }

    hMsgqueue->deviceOffset = BASP_P_Buffer_GetDeviceOffset_isrsafe( hMsgqueue->hBuffer );
    hMsgqueue->queueSize = queueSize;

    /* Lock the Msgqueue buffer's virtual address, and keep it locked to make sure it doesn't get
     * shuffled by the MMA. */
    hMsgqueue->pBuffer = BASP_P_Buffer_LockAddress(hMsgqueue->hBuffer);

    /* Now write the initial values to the Msgqueue's registers (that we share with the ASP). */
    {
        uint64_t queueStart = hMsgqueue->deviceOffset;
        uint64_t queueEnd = queueStart + queueSize - 4;

        BREG_Write64_isrsafe(hAsp->handles.hReg, pMsgFifo->ui64Base,  queueStart);
        BREG_Write64_isrsafe(hAsp->handles.hReg, pMsgFifo->ui64End,   queueEnd);
        BREG_Write64_isrsafe(hAsp->handles.hReg, pMsgFifo->ui64Read,  queueStart);
        BREG_Write64_isrsafe(hAsp->handles.hReg, pMsgFifo->ui64Write, queueStart);
    }

    BDBG_LOG(("hMsgqueue=%p", (void*)hMsgqueue));

    *pHandle = hMsgqueue;   /* Pass the handle back to our caller. */

error:
    return errCode;
}


/******************************************************************************/
/* Destroy the Msgqueue. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_Destroy(BASP_MsgqueueHandle hMsgqueue)
{
    BASP_Handle hAsp;

    BDBG_ENTER( BASP_Msgqueue_Destroy );
    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    hAsp = hMsgqueue->hAsp;

    /* Zero out the Msgqueue pointer registers so it's obvious that the
     * queues are shut down. */
    BREG_Write64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Base,  0);
    BREG_Write64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64End,   0);
    BREG_Write64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Read,  0);
    BREG_Write64_isrsafe(hAsp->handles.hReg, hMsgqueue->pMsgFifo->ui64Write, 0);

    /* Free up the Msgqueue's device memory. */
    if (hMsgqueue->hBuffer)
    {
        BASP_P_FreeDeviceMemory( hMsgqueue->hBuffer);
        hMsgqueue->hBuffer = NULL;
    }

    /* Unlink this Msgqueue from the Msgqueue list. */
    BLST_S_REMOVE(&hAsp->msgqueueList, hMsgqueue, BASP_P_Msgqueue, nextMsgqueue);

    /* Mark the object as destroyed. */
    BDBG_OBJECT_DESTROY(hMsgqueue, BASP_P_Msgqueue);

    /* And finally free the object's memory. */
    BKNI_Free(hMsgqueue);

    BDBG_LEAVE( BASP_Msgqueue_Destroy );
    return BERR_SUCCESS;
}


/******************************************************************************/
/* Print the Msgqueue's pointers to the BDBG log. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_Dump(BASP_MsgqueueHandle hMsgqueue)
{
    BERR_Code errCode = BERR_SUCCESS;
    char * msgqueueName = "";
    BASP_P_MsgqueuePointers  queuePointers;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    if      (hMsgqueue->pMsgFifo == &sMsgRegisters.sFw2HostMsgFifo)  msgqueueName = "sFw2HostMsgFifo";
    else if (hMsgqueue->pMsgFifo == &sMsgRegisters.sHost2FwMsgFifo)  msgqueueName = "sHost2FwMsgFifo";
    else if (hMsgqueue->pMsgFifo == &sMsgRegisters.sHost2FwRaFifo)   msgqueueName = "sHost2FwRaFifo";
    else if (hMsgqueue->pMsgFifo == &sMsgRegisters.sFw2HostRaFifo)   msgqueueName = "sFw2HostRaFifo";
    else msgqueueName = "<unknown>";

    BASP_Msgqueue_ReadRegisters_isrsafe(hMsgqueue, &queuePointers);

    BDBG_LOG(("%s: Msgqueue:%s  hAsp:%p hBuffer:%p pMsgFifo:%p queueSize:%" PRIu32 " deviceOffset:" BASP_X64_FMT ,
            BSTD_FUNCTION,
            msgqueueName,
            (void*)hMsgqueue->hAsp,
            (void*)hMsgqueue->hBuffer,
            (void*)hMsgqueue->pMsgFifo,
            hMsgqueue->queueSize,
            BASP_X64_ARG(hMsgqueue->deviceOffset)  ));

    BDBG_LOG(("%s: Msgqueue:%s  Start(0x" BASP_X64_FMT "):" BASP_X64_FMT" End(0x" BASP_X64_FMT "):" BASP_X64_FMT" Read(0x" BASP_X64_FMT "):" BASP_X64_FMT" Write(0x" BASP_X64_FMT "):" BASP_X64_FMT,
            BSTD_FUNCTION,
            msgqueueName,
            BASP_X64_ARG(hMsgqueue->pMsgFifo->ui64Base),  BASP_X64_ARG(queuePointers.base),
            BASP_X64_ARG(hMsgqueue->pMsgFifo->ui64End),   BASP_X64_ARG(queuePointers.end),
            BASP_X64_ARG(hMsgqueue->pMsgFifo->ui64Read),  BASP_X64_ARG(queuePointers.read),
            BASP_X64_ARG(hMsgqueue->pMsgFifo->ui64Write), BASP_X64_ARG(queuePointers.write) ));

    return errCode;
}


/******************************************************************************/
/* Write a message into the specified Msgqueue. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_Write(BASP_MsgqueueHandle hMsgqueue, void *pMsgToSend)
{
    BERR_Code errCode = BERR_SUCCESS;

    BASP_Handle     hAsp;
    BREG_Handle     hReg;
    size_t          offset = 0;
    size_t          msgLen = 0;
    BASP_MessageHeader      *pMsgInMsgqueue = NULL;
    BASP_P_MsgqueuePointers  queuePointers;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);
    hAsp = hMsgqueue->hAsp;
    hReg = hAsp->handles.hReg;

    /* All messages to the ASP must be a specific size. Get the message length for this Msgqueue. */
    msgLen = hMsgqueue->queueMsgSize;

    /* Read the pointer registers into our structure. */
    BASP_Msgqueue_ReadRegisters_isrsafe(hMsgqueue, &queuePointers);

    /* Is there enough space in the Msgqueue to hold this message? */
    if (BASP_Msgqueue_Availability(&queuePointers) < msgLen)
    {
        return BERR_TRACE( BERR_NOT_AVAILABLE );
    }

    /* Yes, copy the message to where the Msgqueue's write pointer
     * is pointing. */
    offset = queuePointers.write - queuePointers.base;
    pMsgInMsgqueue = (BASP_MessageHeader*)(hMsgqueue->pBuffer + offset);
    BKNI_Memcpy(pMsgInMsgqueue, pMsgToSend, msgLen);

    /* Update the message counter and put it in the message header. */
    pMsgInMsgqueue->ui32MessageCounter = ++(hMsgqueue->nextMessageCount);

    /* TODO: Shouldn't be calling _isr function from non-_isr function! */
    BASP_P_Buffer_FlushCache_isr(hMsgqueue->hBuffer, hMsgqueue->pBuffer + offset, msgLen);

    /* Print the message to the debug log. */
    BASP_Msgqueue_Log_isrsafe(hMsgqueue, "Sending ASP Message From Host to Firmware", pMsgInMsgqueue);

    /* Update the queue's write pointer.  But if there's not enough room for
     * another message, wrap the write pointer around to the start of the buffer. */
    queuePointers.write += msgLen;
    if (BASP_Msgqueue_Availability(&queuePointers) < msgLen)
    {
        queuePointers.write = queuePointers.base;
    }

    /* Write the updated write pointer back to it's register. */
    BREG_Write64_isrsafe(hReg, hMsgqueue->pMsgFifo->ui64Write, queuePointers.write);

    /* Set H2FW interrupt */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_HOST_H2FW_L2_HOST_SET,
                 BCHP_FIELD_DATA(ASP_ARCSS_HOST_H2FW_L2_HOST_SET, ARCSS_HOST00_INTR, 1));

    return errCode;
}


/******************************************************************************/
/* Get a pointer to data that is available for reading. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_GetReadData_isr(BASP_MsgqueueHandle hMsgqueue, void **ppBuffer, size_t *pSize)
{
    BERR_Code errCode = BERR_SUCCESS;

    size_t          bytesInQueue = 0;
    BASP_P_MsgqueuePointers  queuePointers;

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    /* Read the pointer registers into our structure. */
    BASP_Msgqueue_ReadRegisters_isrsafe(hMsgqueue, &queuePointers);

    /* Is there anything in the queue? */
    bytesInQueue = BASP_Msgqueue_Fullness_isrsafe(&queuePointers);
    if (bytesInQueue == 0)
    {
        return BERR_NOT_AVAILABLE;
    }

    if (bytesInQueue > hMsgqueue->queueMsgSize)
    {
        bytesInQueue = hMsgqueue->queueMsgSize;
    }

    /* All messages to the ASP must be a specific size.  Check to be sure the size is proper. */
    if (bytesInQueue < hMsgqueue->queueMsgSize)
    {
        BDBG_WRN(("%s: Msgqueue:%s : Message size: %lu is incorrect! Should be %lu",
                    BSTD_FUNCTION, hMsgqueue->pName, (unsigned long)bytesInQueue, (unsigned long)hMsgqueue->queueMsgSize ));
    }

    /* Convert the Msgqueue's read pointer from a physical address to virtual, then
     * put it into the caller's variable. */
    *ppBuffer = hMsgqueue->pBuffer + (queuePointers.read - queuePointers.base);
    *pSize = bytesInQueue;

    /* This should invalidate the cache for the readable data, so the caller
     * won't read old cached data. */
    BASP_P_Buffer_FlushCache_isr(hMsgqueue->hBuffer, *ppBuffer, bytesInQueue);

    return errCode;
}


BERR_Code BASP_Msgqueue_GetReadData(BASP_MsgqueueHandle hMsgqueue, void **ppBuffer, size_t *pSize)
{
    BERR_Code   errCode;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    BKNI_EnterCriticalSection();
    errCode = BASP_Msgqueue_GetReadData_isr(hMsgqueue, ppBuffer, pSize);
    BKNI_LeaveCriticalSection();

    return errCode;
}


/******************************************************************************/
/* Indicate that read data can be discarded from the queue (read complete). */
/******************************************************************************/
BERR_Code BASP_Msgqueue_ReadComplete_isr(BASP_MsgqueueHandle hMsgqueue)
{
    BERR_Code errCode = BERR_SUCCESS;

    BASP_Handle     hAsp;
    BREG_Handle     hReg;
    size_t          bytesInQueue = 0;
    size_t          msgLen = 0;
    BASP_P_MsgqueuePointers  queuePointers;

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    hAsp = hMsgqueue->hAsp;
    hReg = hAsp->handles.hReg;;

    msgLen = hMsgqueue->queueMsgSize;

    /* Read the pointer registers into our structure. */
    BASP_Msgqueue_ReadRegisters_isrsafe(hMsgqueue, &queuePointers);

    /* Is there anything in the queue? */
    bytesInQueue = BASP_Msgqueue_Fullness_isrsafe(&queuePointers);
    if (bytesInQueue == 0)
    {
        return BERR_NOT_AVAILABLE; /* The Msgqueue has no data to discard! */
    }

    /* Advance read pointer, removing the message from the queue. */
    queuePointers.read += msgLen;

    /* Wrap the read pointer if appropriate. */
    bytesInQueue = BASP_Msgqueue_Fullness_isrsafe(&queuePointers);
    if (bytesInQueue > 0 && bytesInQueue < hMsgqueue->queueMsgSize)
    {
        queuePointers.read = queuePointers.base;
    }

    /* Write the updated read pointer back to it's register. */
    BREG_Write64_isrsafe(hReg, hMsgqueue->pMsgFifo->ui64Read, queuePointers.read);

    /* Set H2FW interrupt */
    BREG_Write32(hReg, BCHP_ASP_ARCSS_HOST_H2FW_L2_HOST_SET,
                 BCHP_FIELD_DATA(ASP_ARCSS_HOST_H2FW_L2_HOST_SET, ARCSS_HOST00_INTR, 1));

    hMsgqueue->topMessageState =  BASP_MsgqueueMessageState_eNoMessage;

    return errCode;
}


BERR_Code BASP_Msgqueue_ReadComplete(BASP_MsgqueueHandle hMsgqueue)
{
    BERR_Code   errCode;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    BKNI_EnterCriticalSection();
    errCode = BASP_Msgqueue_ReadComplete_isr(hMsgqueue);
    BKNI_LeaveCriticalSection();
    return errCode;
}


/******************************************************************************/
/* A new ASP message has arrived, tell somebody if appropriate. */
/******************************************************************************/
BERR_Code BASP_NewAspMessageNotify_isr( BASP_MsgqueueHandle hMsgqueue, BASP_Fw2Pi_Message *pMessage, size_t messageSize)
{
    BERR_Code               errCode = BERR_SUCCESS;
    BASP_Handle             hAsp;
    BASP_ChannelHandle      hChannel;
    uint32_t                messageChannel;
    BASP_MessageType        messageType;

    BKNI_ASSERT_ISR_CONTEXT();
    BSTD_UNUSED(messageSize);
    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);
    hAsp = hMsgqueue->hAsp;

    messageType = pMessage->MessageHeader.MessageType;
    messageChannel = pMessage->MessageHeader.ui32ChannelIndex;

    BDBG_MSG(("%s : %d : messageType=%u messageChannel=%u\n", BSTD_FUNCTION, __LINE__, messageType, messageChannel ));

    hMsgqueue->topMessageState = BASP_MsgqueueMessageState_eWaitingForReadByApp;

    if (messageType == BASP_MessageType_eFw2PiInitResp)
    {
        /* Process and delete the Init Response.  */
        if (pMessage->Message.ResponsePayload.InitResponse.ResponseStatus != BASP_ResponseStatus_eSuccess)
        {
            BDBG_ERR(("%s : %d : ASP Init message failed! Init Response=%d", BSTD_FUNCTION, __LINE__,
                      pMessage->Message.ResponsePayload.InitResponse.ResponseStatus ));
        }

        /* Print the ASP firmware and platform version info. */
        {
            uint32_t fwVersion  = pMessage->Message.ResponsePayload.InitResponse.aspFwVersion;
            uint32_t fwPlatform = pMessage->Message.ResponsePayload.InitResponse.aspFwPlatformVersion;

            BDBG_WRN(("ASP FW v%u.%u.%u.%u (0x%08x)  ASP Platform %u.%u.%u.%u (0x%08x)",
                            fwVersion>>24,
                            fwVersion>>16 & 0xff,
                            fwVersion>>8  & 0xff,
                            fwVersion     & 0xff,
                            fwVersion,
                            fwPlatform>>24,
                            fwPlatform>>16 & 0xff,
                            fwPlatform>>8  & 0xff,
                            fwPlatform     & 0xff,
                            fwPlatform ));
        }

        BASP_Msgqueue_ReadComplete_isr(hMsgqueue);
    }
    else if (messageType == BASP_MessageType_eFw2PiUnInitResp)
    {
        /* Process and delete the Uninit Response.  */

        /* TODO: Anything else to do for Uninit message from ASP? */
        BASP_Msgqueue_ReadComplete_isr(hMsgqueue);
    }
    else
    {
        /* Any other message should be for a channel, so find the channel
           handle and dispatch a callback (if one is registered).  */
        hChannel =  BASP_P_Channel_GetByChannelIndex_isr( hAsp, messageChannel );

        if (hChannel == NULL)
        {
            /* Message is for non-existant channel. */
            BDBG_WRN(("%s : %d Discarding orphan message for channelIndex=%u", BSTD_FUNCTION, __LINE__, messageChannel ));
            BASP_Msgqueue_ReadComplete_isr(hMsgqueue);
        }
        else
        {
            BASP_P_Channel_FireMessageReadyCallback_isr(hChannel);
        }
    }

    return errCode;
}


/******************************************************************************/
/* Check for arrival of a new message in a Msgqueue. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_NewMessageCheck_isr(BASP_MsgqueueHandle hMsgqueue)
{
    BERR_Code               errCode = BERR_SUCCESS;

    size_t                   bytesInQueue = 0;
    size_t                   messageSize;
    BASP_Fw2Pi_Message       *pMessage = NULL;
    BASP_P_MsgqueuePointers  queuePointers;

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    /* Do this in a loop in case the messages are being processed and deleted in the
     * MessageNotify_isr() function or the callbacks that it makes.  */
    for (;;)
    {
        BDBG_MSG(("%s : %d Checking for new message from ASP...", BSTD_FUNCTION, __LINE__ ));

        /* If the top message is already being processed, there's nothing for us to do now. */
        if (hMsgqueue->topMessageState != BASP_MsgqueueMessageState_eNoMessage)
        {
            BDBG_MSG(("%s : %d Top message is still busy, returning...\n", BSTD_FUNCTION, __LINE__ ));
            errCode = BERR_SUCCESS;
            break;
        }

        /* Read the pointer registers into our structure. */
        BASP_Msgqueue_ReadRegisters_isrsafe(hMsgqueue, &queuePointers);

        /* Is there anything in the queue? */
        bytesInQueue = BASP_Msgqueue_Fullness_isrsafe(&queuePointers);
        if (bytesInQueue == 0)
        {
            BDBG_MSG(("%s : %d No messages in queue, returning...\n", BSTD_FUNCTION, __LINE__ ));
            errCode = BERR_NOT_AVAILABLE;
            break;
        }

        hMsgqueue->topMessageState = BASP_MsgqueueMessageState_eWaitingForPreprocess;

        if (bytesInQueue > hMsgqueue->queueMsgSize)
        {
            bytesInQueue = hMsgqueue->queueMsgSize;
        }

        /* All messages to the ASP must be a specific size.  Check to be sure the size is proper. */
        if (bytesInQueue < hMsgqueue->queueMsgSize)
        {
            BDBG_WRN(("%s: Msgqueue:%s : Message size: %lu is incorrect! Should be %lu",
                        BSTD_FUNCTION, hMsgqueue->pName, (unsigned long)bytesInQueue, (unsigned long)hMsgqueue->queueMsgSize ));
        }

        /* Convert the Msgqueue's read pointer from a physical address to virtual, then
         * put it into the caller's variable. */
        pMessage = (BASP_Fw2Pi_Message*)((uint8_t*)hMsgqueue->pBuffer + (queuePointers.read - queuePointers.base));
        messageSize = bytesInQueue;

        /* This should invalidate the cache for the readable data, so the caller
         * won't read old cached data. */
        BASP_P_Buffer_FlushCache_isr(hMsgqueue->hBuffer, pMessage, bytesInQueue);

        /* Log the message. */
        BASP_Msgqueue_Log_isrsafe(hMsgqueue, "Received ASP Message From Firmware To Host", pMessage);

        /* Save the message's address and size for use by subsequent APIs (ReadData and ReadComplete). */
        hMsgqueue->topMessageSize  = messageSize;
        hMsgqueue->pTopMessage = pMessage;
        hMsgqueue->topMessageState = BASP_MsgqueueMessageState_eWaitingForReadByApp;

        /* If the message is an ASP-type of message (as opposed to a stream-mux type), handle it here. */
        BASP_NewAspMessageNotify_isr(hMsgqueue, pMessage, messageSize);
    }

    return errCode;
}


void
BASP_P_Msgqueue_FW2H_Receive_isr(
         void *pContext,
         int iParam
         )
{
   BASP_Handle hAsp = (BASP_Handle) pContext;
   BASP_MsgqueueHandle   hMsgqueue;

   BSTD_UNUSED(iParam);
   BKNI_ASSERT_ISR_CONTEXT();
   BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);

   hMsgqueue = hAsp->hMsgqueueFwToHost;
   BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

   BDBG_MSG(("%s : %d Started interrupt handking. iParam=%u\n", BSTD_FUNCTION, __LINE__, iParam ));
   BASP_Msgqueue_NewMessageCheck_isr(hMsgqueue);
   BDBG_MSG(("%s : %d Finished interrupt handking. iParam=%u\n", BSTD_FUNCTION, __LINE__, iParam ));
}


/******************************************************************************/
/* Print the specified message to the debug log. */
/******************************************************************************/
BERR_Code BASP_Msgqueue_Log_isrsafe(BASP_MsgqueueHandle hMsgqueue, const char *pHeading, void *pMessage)
{
    BERR_Code errCode = BERR_SUCCESS;
    BASP_MessageHeader    *pMsgHeader = NULL;

    BDBG_OBJECT_ASSERT(hMsgqueue, BASP_P_Msgqueue);

    pMsgHeader = pMessage;

    /* First, log the message header. */
    BDBG_MSG(("---------- Start: %s ---------" , pHeading));
    BDBG_MSG(("Header"));
    BDBG_MSG(("  MessageType:        %u" , pMsgHeader->MessageType));
    BDBG_MSG(("  ui32MessageCounter: %u" , pMsgHeader->ui32MessageCounter));
    BDBG_MSG(("  ResponseType:       %u (%s)" ,
            pMsgHeader->ResponseType,
            pMsgHeader->ResponseType==BASP_ResponseType_eNoneRequired    ? "eNoneRequired"    :
            pMsgHeader->ResponseType==BASP_ResponseType_eAckRequired     ? "eAckRequired"     :
            pMsgHeader->ResponseType==BASP_ResponseType_eRespRequired    ? "eRespRequired"    :
            pMsgHeader->ResponseType==BASP_ResponseType_eAckRespRequired ? "eAckRespRequired" :
            pMsgHeader->ResponseType==BASP_ResponseType_eInvalid         ? "eInvalid"         :
                                                                           "<undefined>" ));
    BDBG_MSG(("  ui32ChannelIndex:   %u" , pMsgHeader->ui32ChannelIndex));

    /* Now, log the message body, which depends on the message type. */
    switch (pMsgHeader->MessageType)
    {
    case BASP_MessageType_ePi2FwInit:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body                                Pi2FwInit"));
        BDBG_MSG(("                                    ========="));
        BDBG_MSG(("  ui32NumMaxChannels:               %u"     , pMsg->MessagePayload.Init.ui32NumMaxChannels));
        BDBG_MSG(("  ui32MemDmaMcpbBaseAddressLo:      0x%08x" , pMsg->MessagePayload.Init.ui32MemDmaMcpbBaseAddressLo));
        BDBG_MSG(("  ui32MemDmaMcpbBaseAddressHi:      0x%08x" , pMsg->MessagePayload.Init.ui32MemDmaMcpbBaseAddressHi));

        BDBG_MSG(("  ui32XptMcpbBaseAddressLo:         0x%08x" , pMsg->MessagePayload.Init.ui32XptMcpbBaseAddressLo));
        BDBG_MSG(("  ui32XptMcpbBaseAddressHi:         0x%08x" , pMsg->MessagePayload.Init.ui32XptMcpbBaseAddressHi));

        BDBG_MSG(("  ui32McpbStreamInDescType:         %u" , pMsg->MessagePayload.Init.ui32McpbStreamInDescType));

        BDBG_MSG(("  EdPktHeaderBuffer.ui32BaseAddrLo: 0x%08x"  , pMsg->MessagePayload.Init.EdPktHeaderBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  EdPktHeaderBuffer.ui32BaseAddrHi: 0x%08x"  , pMsg->MessagePayload.Init.EdPktHeaderBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  EdPktHeaderBuffer.ui32Size:       %u (0x%x)" , pMsg->MessagePayload.Init.EdPktHeaderBuffer.ui32Size, pMsg->MessagePayload.Init.EdPktHeaderBuffer.ui32Size));

        BDBG_MSG(("  StatusBuffer.ui32BaseAddrLo:      0x%08x"  , pMsg->MessagePayload.Init.StatusBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  StatusBuffer.ui32BaseAddrHi:      0x%08x"  , pMsg->MessagePayload.Init.StatusBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  StatusBuffer.ui32Size:            %u (0x%x)" , pMsg->MessagePayload.Init.StatusBuffer.ui32Size, pMsg->MessagePayload.Init.StatusBuffer.ui32Size));

        BDBG_MSG(("  DebugBuffer.ui32BaseAddrLo:       0x%08x"  , pMsg->MessagePayload.Init.DebugBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  DebugBuffer.ui32BaseAddrHi:       0x%08x"  , pMsg->MessagePayload.Init.DebugBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  DebugBuffer.ui32Size:             %u (0x%x)" , pMsg->MessagePayload.Init.DebugBuffer.ui32Size, pMsg->MessagePayload.Init.DebugBuffer.ui32Size));

        BDBG_MSG(("  HwInitInfo.ui32BaseAddrLo:        0x%08x"  , pMsg->MessagePayload.Init.HwInitInfo.ui32BaseAddrLo));
        BDBG_MSG(("  HwInitInfo.ui32BaseAddrHi:        0x%08x"  , pMsg->MessagePayload.Init.HwInitInfo.ui32BaseAddrHi));
        BDBG_MSG(("  HwInitInfo.ui32Size:              %u (0x%x)" , pMsg->MessagePayload.Init.HwInitInfo.ui32Size, pMsg->MessagePayload.Init.HwInitInfo.ui32Size));
        break;
    }

    case BASP_MessageType_eFw2PiInitResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiInitResp"));
        BDBG_MSG(("                  ============="));
        BDBG_MSG(("  ResponseStatus:       %u"     , pMsg->Message.ResponsePayload.InitResponse.ResponseStatus));
        BDBG_MSG(("  aspFwVersion:         0x%08x" , pMsg->Message.ResponsePayload.InitResponse.aspFwVersion));
        BDBG_MSG(("  aspFwPlatformVersion: 0x%08x" , pMsg->Message.ResponsePayload.InitResponse.aspFwPlatformVersion));
        break;
    }

    case BASP_MessageType_ePi2FwChannelStartStreamOut:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BASP_ConnectionControlBlock *pCcb = &pMsg->MessagePayload.ChannelStartStreamOut.ConnectionControlBlock;
        BASP_McpbStreamOutConfig *pMcpb = &pMsg->MessagePayload.ChannelStartStreamOut.McpbStreamOutConfig;

        BDBG_MSG(("Body                                     Pi2FwChannelStartStreamOut"));
        BDBG_MSG(("                                         =========================="));
        BDBG_MSG(("  ui32RetransmissionEnable:              %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32RetransmissionEnable));
        BDBG_MSG(("  ui32AspBypassEnabled:                  %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32AspBypassEnabled));
        BDBG_MSG(("  ui32SendHttpResponsePktEnable:         %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32SendHttpResponsePktEnable));
        BDBG_MSG(("  ui32SendRstPktOnRetransTimeOutEnable:  %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32SendRstPktOnRetransTimeOutEnable));
        BDBG_MSG(("  ui32CongestionFlowControlEnable:       %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32CongestionFlowControlEnable));
        BDBG_MSG(("  ui32DrmEnabled:                        %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32DrmEnabled));
        BDBG_MSG(("  ui32PcpPayloadSize:                    %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32PcpPayloadSize));
        BDBG_MSG(("  ui32HttpType:                          %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32HttpType));
        BDBG_MSG(("  ui32FullChunkHeaderSize:               %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32FullChunkHeaderSize));
        BDBG_MSG(("  ui32ChunkSize:                         %u"        , pMsg->MessagePayload.ChannelStartStreamOut.ui32ChunkSize));

        BDBG_MSG(("  ReceivePayloadBuffer.ui32BaseAddrLo:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.ReceivePayloadBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  ReceivePayloadBuffer.ui32BaseAddrHi:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.ReceivePayloadBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  ReceivePayloadBuffer.ui32Size:         %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamOut.ReceivePayloadBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamOut.ReceivePayloadBuffer.ui32Size));

        BDBG_MSG(("  EthernetHeaderBuffer.ui32BaseAddrLo:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.EthernetHeaderBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  EthernetHeaderBuffer.ui32BaseAddrHi:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.EthernetHeaderBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  EthernetHeaderBuffer.ui32Size:         %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamOut.EthernetHeaderBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamOut.EthernetHeaderBuffer.ui32Size));

        BDBG_MSG(("  HttpResponseBuffer.ui32BaseAddrLo:     0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.HttpResponseBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  HttpResponseBuffer.ui32BaseAddrHi:     0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.HttpResponseBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  HttpResponseBuffer.ui32Size:           %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamOut.HttpResponseBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamOut.HttpResponseBuffer.ui32Size));

        BDBG_MSG(("  ReTransmissionBuffer.ui32BaseAddrLo:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.ReTransmissionBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  ReTransmissionBuffer.ui32BaseAddrHi:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamOut.ReTransmissionBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  ReTransmissionBuffer.ui32Size:         %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamOut.ReTransmissionBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamOut.ReTransmissionBuffer.ui32Size));

        BDBG_MSG((" ConnectionControlBlock.aui8DestMacAddr: %2x:%2x:%2x:%2x:%2x:%2x" ,
                    pCcb->aui8DestMacAddr[0],
                    pCcb->aui8DestMacAddr[1],
                    pCcb->aui8DestMacAddr[2],
                    pCcb->aui8DestMacAddr[3],
                    pCcb->aui8DestMacAddr[4],
                    pCcb->aui8DestMacAddr[5]
                 ));
        BDBG_MSG((" ConnectionControlBlock.aui8SrcMacAddr:  %2x:%2x:%2x:%2x:%2x:%2x"  ,
                    pCcb->aui8SrcMacAddr[0],
                    pCcb->aui8SrcMacAddr[1],
                    pCcb->aui8SrcMacAddr[2],
                    pCcb->aui8SrcMacAddr[3],
                    pCcb->aui8SrcMacAddr[4],
                    pCcb->aui8SrcMacAddr[5]
                 ));
        BDBG_MSG((" ConnectionControlBlock.ui32EtherType:                 0x%x"  , pCcb->ui32EtherType));
        BDBG_MSG((" ConnectionControlBlock.ui32IngressBrcmTag:            0x%x"  , pCcb->ui32IngressBrcmTag));
        BDBG_MSG((" ConnectionControlBlock.ui32EgressClassId:             0x%x"  , pCcb->ui32EgressClassId));
        BDBG_MSG((" ConnectionControlBlock.ui32IpVersion:                 0x%x"  , pCcb->ui32IpVersion));
        BDBG_MSG((" ConnectionControlBlock.ui32Dscp:                      0x%x"  , pCcb->ui32Dscp));
        BDBG_MSG((" ConnectionControlBlock.ui32Ecn:                       0x%x"  , pCcb->ui32Ecn));
        BDBG_MSG((" ConnectionControlBlock.ui32IpIdSel:                   0x%x"  , pCcb->ui32IpIdSel));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeToLive:                0x%x"  , pCcb->ui32TimeToLive));
        BDBG_MSG((" ConnectionControlBlock.ui32ProtocolType:              0x%x"  , pCcb->ui32ProtocolType));
        BDBG_MSG((" ConnectionControlBlock.ai32SrcIpAddr[0]:              0x%x"  , pCcb->ai32SrcIpAddr[0]));
        BDBG_MSG((" ConnectionControlBlock.ai32DestIpAddr[0]:             0x%x"  , pCcb->ai32DestIpAddr[0]));
        BDBG_MSG((" ConnectionControlBlock.ui32SrcPort:                   0x%x"  , pCcb->ui32SrcPort));
        BDBG_MSG((" ConnectionControlBlock.ui32DestPort:                  0x%x"  , pCcb->ui32DestPort));
        BDBG_MSG((" ConnectionControlBlock.ui32InitialSendSeqNumber:      0x%x"  , pCcb->ui32InitialSendSeqNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32InitialReceivedSeqNumber:  0x%x"  , pCcb->ui32InitialReceivedSeqNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32CurrentAckedNumber:        0x%x"  , pCcb->ui32CurrentAckedNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32RemoteWindowSize:          0x%x"  , pCcb->ui32RemoteWindowSize));
        BDBG_MSG((" ConnectionControlBlock.ui32WindowAdvConst:            0x%x"  , pCcb->ui32WindowAdvConst));
        BDBG_MSG((" ConnectionControlBlock.ui32LocalWindowScaleValue:     0x%x"  , pCcb->ui32LocalWindowScaleValue));
        BDBG_MSG((" ConnectionControlBlock.ui32RemoteWindowScaleValue:    0x%x"  , pCcb->ui32RemoteWindowScaleValue));
        BDBG_MSG((" ConnectionControlBlock.ui32SackEnable:                0x%x"  , pCcb->ui32SackEnable));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeStampEnable:           0x%x"  , pCcb->ui32TimeStampEnable));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeStampEchoValue:        0x%x"  , pCcb->ui32TimeStampEchoValue));
        BDBG_MSG((" ConnectionControlBlock.ui32MaxSegmentSize:            0x%x"  , pCcb->ui32MaxSegmentSize));
        BDBG_MSG((" ConnectionControlBlock.ui32KaTimeout:                 0x%x"  , pCcb->ui32KaTimeout));
        BDBG_MSG((" ConnectionControlBlock.ui32KaInterval:                0x%x"  , pCcb->ui32KaInterval));
        BDBG_MSG((" ConnectionControlBlock.ui32KaMaxProbes:               0x%x"  , pCcb->ui32KaMaxProbes));
        BDBG_MSG((" ConnectionControlBlock.ui32RetxTimeout:               0x%x"  , pCcb->ui32RetxTimeout));
        BDBG_MSG((" ConnectionControlBlock.ui32RetxMaxRetries:            0x%x"  , pCcb->ui32RetxMaxRetries));

        BDBG_MSG((" ui32StreamLabel:                                      %u"    , pMsg->MessagePayload.ChannelStartStreamOut.ui32StreamLabel));
        BDBG_MSG((" ui32RaveContextBaseAddressLo:                         0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.ui32RaveContextBaseAddressLo));
        BDBG_MSG((" ui32RaveContextBaseAddressHi:                         0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.ui32RaveContextBaseAddressHi));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32PacingType:                   %u"    , pMcpb->ui32PacingType));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32SrcTimestampType:             %u"    , pMcpb->ui32SrcTimestampType));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32ForceRestamping:              %u"    , pMcpb->ui32ForceRestamping));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32PcrPacingPid:                 0x%x"  , pMcpb->ui32PcrPacingPid));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32ParityCheckDisable:           %u"    , pMcpb->ui32ParityCheckDisable));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32AvgStreamBitrate:             %u"    , pMcpb->ui32AvgStreamBitrate));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32TimebaseIndex:                %u"    , pMcpb->ui32TimebaseIndex));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32PacketLen:                    %u"    , pMcpb->ui32PacketLen));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32ParserAllPassControl:         %u"    , pMcpb->ui32ParserAllPassControl));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32ParserStreamType:             %u"    , pMcpb->ui32ParserStreamType));
        BDBG_MSG((" McpbStreamOutCpMcpb.ui32McpbTmeuErrorBoundLate:       %u"    , pMcpb->ui32McpbTmeuErrorBoundLate));
        BDBG_MSG((" ui32SwitchSlotsPerEthernetPacket:                     %d"    , pMsg->MessagePayload.ChannelStartStreamOut.ui32SwitchSlotsPerEthernetPacket));
        BDBG_MSG((" ui32SwitchQueueNumber:                                %d"    , pMsg->MessagePayload.ChannelStartStreamOut.ui32SwitchQueueNumber));
        BDBG_MSG((" DtcpIpInfo.ui32ExchangeKeys[0]:                       0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32ExchangeKeys[0]));
        BDBG_MSG((" DtcpIpInfo.ui32ExchangeKeys[1]:                       0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32ExchangeKeys[1]));
        BDBG_MSG((" DtcpIpInfo.ui32ExchangeKeys[2]:                       0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32ExchangeKeys[2]));
        BDBG_MSG((" DtcpIpInfo.ui32C_A2:                                  0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32C_A2));
        BDBG_MSG((" DtcpIpInfo.ui32ExchangeKeyLabel:                      0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32ExchangeKeyLabel));
        BDBG_MSG((" DtcpIpInfo.ui32EmiModes:                              0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32EmiModes));
        BDBG_MSG((" DtcpIpInfo.ui32Nc[0]:                                 0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32Nc[0]));
        BDBG_MSG((" DtcpIpInfo.ui32Nc[1]:                                 0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32Nc[1]));
        BDBG_MSG((" DtcpIpInfo.ui32ASPKeySlot:                            0x%x"  , pMsg->MessagePayload.ChannelStartStreamOut.DtcpIpInfo.ui32ASPKeySlot));
        BDBG_MSG((" DtcpIpInfo.ui32PcpPayloadSize:                        %u"    , pMsg->MessagePayload.ChannelStartStreamOut.ui32PcpPayloadSize));
        BDBG_MSG((" DtcpIpInfo.ui32DrmEnabled:                            %u"    , pMsg->MessagePayload.ChannelStartStreamOut.ui32DrmEnabled));
#if 0 /* ******************** Temporary by Gary **********************/
            Still need to add these fields.

        BASP_DtcpIpInfoFromHost DtcpIpInfo;
#endif /* ******************** Temporary by Gary **********************/
        break;
    }

    case BASP_MessageType_eFw2PiChannelStartStreamOutResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiChannelStartStreamOutResp"));
        BDBG_MSG(("                  =============================="));
        BDBG_MSG(("  ResponseStatus: %u"     , pMsg->Message.ResponsePayload.StartStreamOutResponse.ResponseStatus));
        break;
    }

    case BASP_MessageType_ePi2FwChannelStop:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Pi2FwChannelStop"));
        BDBG_MSG(("                  ================"));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.ChannelStop.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiChannelStopResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body                      Fw2PiChannelStopResp"));
        BDBG_MSG(("                          ===================="));
        BDBG_MSG(("  ResponseStatus:         %u"     , pMsg->Message.ResponsePayload.StopResponse.ResponseStatus));
        BDBG_MSG(("  ui32CurrentAckedNumber: %u"     , pMsg->Message.ResponsePayload.StopResponse.ui32CurrentAckedNumber));
        BDBG_MSG(("  ui32CurrentSeqNumber:   %u"     , pMsg->Message.ResponsePayload.StopResponse.ui32CurrentSeqNumber));
        break;
    }

    case BASP_MessageType_ePi2FwChannelAbort:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Pi2FwChannelAbort"));
        BDBG_MSG(("                  ================="));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.ChannelAbort.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiChannelAbortResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body                      Fw2PiChannelAbortResp"));
        BDBG_MSG(("                          ====================="));
        BDBG_MSG(("  ResponseStatus:         %u"     , pMsg->Message.ResponsePayload.AbortResponse.ResponseStatus));
        BDBG_MSG(("  ui32CurrentAckedNumber: %u"     , pMsg->Message.ResponsePayload.AbortResponse.ui32CurrentAckedNumber));
        BDBG_MSG(("  ui32CurrentSeqNumber:   %u"     , pMsg->Message.ResponsePayload.AbortResponse.ui32CurrentSeqNumber));
        break;
    }

    case BASP_MessageType_ePi2FwChannelAbortWithRst:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Pi2FwChannelAbortWithRst"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.ChannelAbortWithRst.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiChannelAbortWithRstResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body                      Fw2PiChannelAbortWithRstResp"));
        BDBG_MSG(("                          ============================"));
        BDBG_MSG(("  ResponseStatus:         %u"     , pMsg->Message.ResponsePayload.AbortWithRstResponse.ResponseStatus));
        BDBG_MSG(("  ui32CurrentAckedNumber: %u"     , pMsg->Message.ResponsePayload.AbortWithRstResponse.ui32CurrentAckedNumber));
        BDBG_MSG(("  ui32CurrentSeqNumber:   %u"     , pMsg->Message.ResponsePayload.AbortWithRstResponse.ui32CurrentSeqNumber));
        break;
    }

    case BASP_MessageType_ePi2FwPerformanceGathering:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Pi2FwPerformanceGathering"));
        BDBG_MSG(("                  ========================="));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.PerformanceGathering.ui32Unused));
        break;
    }

#if 0
    /* This message seems to be removed from the FW API header. We will need to see if we need it to get to the periodic stats. */
    /* Or we are going to use read the stats directly from the DRAM once firmware writes to it. */
    case BASP_MessageType_eFw2PiPerformanceGatheringResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body                             Fw2PiPerformanceGatheringResp"));
        BDBG_MSG(("                                 ============================="));
        BDBG_MSG(("  ResponseStatus:                %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ResponseStatus));
        BDBG_MSG(("  ui32NumChannelsAlive:          %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32NumChannelsAlive));
        BDBG_MSG(("  ui32NumBytesSentPerChannelLo:  %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32NumBytesSentPerChannelLo));
        BDBG_MSG(("  ui32NumBytesSentPerChannelHi:  %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32NumBytesSentPerChannelHi));
        BDBG_MSG(("  ui32UnimacErrorCodes:          %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32UnimacErrorCodes));
        BDBG_MSG(("  ui32XonXoffStatus:             %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32XonXoffStatus));
        BDBG_MSG(("  ui32AnyBuffersOverflow:        %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32AnyBuffersOverflow));
        BDBG_MSG(("  ui32TransmitWindowSize:        %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32TransmitWindowSize));
        BDBG_MSG(("  ui32EthernetPacketCounts:      %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32EthernetPacketCounts));
        BDBG_MSG(("  ui32TotalNumEPKTSent:          %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32TotalNumEPKTSent));
        BDBG_MSG(("  ui32SwitchPauseStatus:         %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32SwitchPauseStatus));
        BDBG_MSG(("  ui32EpktStatus:                %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32EpktStatus));
        BDBG_MSG(("  ui32EdpktStatus:               %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32EdpktStatus));
        BDBG_MSG(("  ui32WindowPause:               %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32WindowPause));
        BDBG_MSG(("  ui32DMACommittedbytes:         %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32DMACommittedbytes));
        BDBG_MSG(("  ui32BufferDepth:               %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32BufferDepth));
        BDBG_MSG(("  ui32HwChannelProcessing:       %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32HwChannelProcessing));
        BDBG_MSG(("  ui32TimeStampValLo:            %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32TimeStampValLo));
        BDBG_MSG(("  ui32TimeStampValHi:            %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32TimeStampValHi));
        BDBG_MSG(("  ui32TimeStampEcho:             %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32TimeStampEcho));
        BDBG_MSG(("  ui32RoundTripTime:             %u"   , pMsg->Message.ResponsePayload.PerformanceGatherResponse.ui32RoundTripTime));
        break;
    }
#endif

    case BASP_MessageType_eFw2PiRstNotify:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiRstNotify"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.ChannelStop.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiFinNotify:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiFinNotify"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.ChannelStop.ui32Unused));
        break;
    }

    case BASP_MessageType_ePi2FwChannelStartStreamIn:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BASP_ConnectionControlBlock *pCcb = &pMsg->MessagePayload.ChannelStartStreamIn.ConnectionControlBlock;
        BDBG_MSG(("Body                                     Pi2FwChannelStartStreamIn"));
        BDBG_MSG(("                                         =========================="));
        BDBG_MSG(("  ui32AspBypassEnabled:                  %u"        , pMsg->MessagePayload.ChannelStartStreamIn.ui32AspBypassEnabled));
        BDBG_MSG(("  ui32SendRstPktOnRetransTimeOutEnable:  %u"        , pMsg->MessagePayload.ChannelStartStreamIn.ui32SendRstPktOnRetransTimeOutEnable));
        BDBG_MSG(("  ui32KeepAliveTimerEnable:              %u"        , pMsg->MessagePayload.ChannelStartStreamIn.ui32KeepAliveTimerEnable));
        BDBG_MSG(("  ui32DrmEnabled:                        %u"        , pMsg->MessagePayload.ChannelStartStreamIn.ui32DrmEnabled));
        BDBG_MSG(("  ui32PcpPayloadSize:                    %u"        , pMsg->MessagePayload.ChannelStartStreamIn.ui32PcpPayloadSize));
        BDBG_MSG(("  ReceivePayloadBuffer.ui32BaseAddrLo:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamIn.ReceivePayloadBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  ReceivePayloadBuffer.ui32BaseAddrHi:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamIn.ReceivePayloadBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  ReceivePayloadBuffer.ui32Size:         %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamIn.ReceivePayloadBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamIn.ReceivePayloadBuffer.ui32Size));

        BDBG_MSG(("  EthernetHeaderBuffer.ui32BaseAddrLo:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamIn.EthernetHeaderBuffer.ui32BaseAddrLo));
        BDBG_MSG(("  EthernetHeaderBuffer.ui32BaseAddrHi:   0x%08x"    , pMsg->MessagePayload.ChannelStartStreamIn.EthernetHeaderBuffer.ui32BaseAddrHi));
        BDBG_MSG(("  EthernetHeaderBuffer.ui32Size:         %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamIn.EthernetHeaderBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamIn.EthernetHeaderBuffer.ui32Size));

        BDBG_MSG((" MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrLo:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrLo));
        BDBG_MSG((" MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrHi:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrHi));
        BDBG_MSG((" MemDmaMcpbDramDescriptorBuffer.ui32Size:     %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamIn.MemDmaMcpbDramDescriptorBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamIn.MemDmaMcpbDramDescriptorBuffer.ui32Size));

        BDBG_MSG((" HttpRequestBuffer.ui32BaseAddrLo:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.HttpRequestBuffer.ui32BaseAddrLo));
        BDBG_MSG((" HttpRequestBuffer.ui32BaseAddrHi:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.HttpRequestBuffer.ui32BaseAddrHi));
        BDBG_MSG((" HttpRequestBuffer.ui32Size:     %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamIn.HttpRequestBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamIn.HttpRequestBuffer.ui32Size));

        BDBG_MSG((" ReTransmissionBuffer.ui32BaseAddrLo:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.ReTransmissionBuffer.ui32BaseAddrLo));
        BDBG_MSG((" ReTransmissionBuffer.ui32BaseAddrHi:  0x%08x" , pMsg->MessagePayload.ChannelStartStreamIn.ReTransmissionBuffer.ui32BaseAddrHi));
        BDBG_MSG((" ReTransmissionBuffer.ui32Size:     %u (0x%x)" , pMsg->MessagePayload.ChannelStartStreamIn.ReTransmissionBuffer.ui32Size, pMsg->MessagePayload.ChannelStartStreamIn.ReTransmissionBuffer.ui32Size));


        BDBG_MSG((" ConnectionControlBlock.aui8DestMacAddr: %2x:%2x:%2x:%2x:%2x:%2x" ,
                    pCcb->aui8DestMacAddr[0],
                    pCcb->aui8DestMacAddr[1],
                    pCcb->aui8DestMacAddr[2],
                    pCcb->aui8DestMacAddr[3],
                    pCcb->aui8DestMacAddr[4],
                    pCcb->aui8DestMacAddr[5]
                 ));
        BDBG_MSG((" ConnectionControlBlock.aui8SrcMacAddr:  %2x:%2x:%2x:%2x:%2x:%2x"  ,
                    pCcb->aui8SrcMacAddr[0],
                    pCcb->aui8SrcMacAddr[1],
                    pCcb->aui8SrcMacAddr[2],
                    pCcb->aui8SrcMacAddr[3],
                    pCcb->aui8SrcMacAddr[4],
                    pCcb->aui8SrcMacAddr[5]
                 ));
        BDBG_MSG((" ConnectionControlBlock.ui32EtherType:                 0x%x"  , pCcb->ui32EtherType));
        BDBG_MSG((" ConnectionControlBlock.ui32IngressBrcmTag:            0x%x"  , pCcb->ui32IngressBrcmTag));
        BDBG_MSG((" ConnectionControlBlock.ui32EgressClassId:             0x%x"  , pCcb->ui32EgressClassId));
        BDBG_MSG((" ConnectionControlBlock.ui32IpVersion:                 0x%x"  , pCcb->ui32IpVersion));
        BDBG_MSG((" ConnectionControlBlock.ui32Dscp:                      0x%x"  , pCcb->ui32Dscp));
        BDBG_MSG((" ConnectionControlBlock.ui32Ecn:                       0x%x"  , pCcb->ui32Ecn));
        BDBG_MSG((" ConnectionControlBlock.ui32IpIdSel:                   0x%x"  , pCcb->ui32IpIdSel));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeToLive:                0x%x"  , pCcb->ui32TimeToLive));
        BDBG_MSG((" ConnectionControlBlock.ui32ProtocolType:              0x%x"  , pCcb->ui32ProtocolType));
        BDBG_MSG((" ConnectionControlBlock.ai32SrcIpAddr[0]:              0x%x"  , pCcb->ai32SrcIpAddr[0]));
        BDBG_MSG((" ConnectionControlBlock.ai32DestIpAddr[0]:             0x%x"  , pCcb->ai32DestIpAddr[0]));
        BDBG_MSG((" ConnectionControlBlock.ui32SrcPort:                   0x%x"  , pCcb->ui32SrcPort));
        BDBG_MSG((" ConnectionControlBlock.ui32DestPort:                  0x%x"  , pCcb->ui32DestPort));
        BDBG_MSG((" ConnectionControlBlock.ui32InitialSendSeqNumber:      0x%x"  , pCcb->ui32InitialSendSeqNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32InitialReceivedSeqNumber:  0x%x"  , pCcb->ui32InitialReceivedSeqNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32CurrentAckedNumber:        0x%x"  , pCcb->ui32CurrentAckedNumber));
        BDBG_MSG((" ConnectionControlBlock.ui32RemoteWindowSize:          0x%x"  , pCcb->ui32RemoteWindowSize));
        BDBG_MSG((" ConnectionControlBlock.ui32WindowAdvConst:            0x%x"  , pCcb->ui32WindowAdvConst));
        BDBG_MSG((" ConnectionControlBlock.ui32LocalWindowScaleValue:     0x%x"  , pCcb->ui32LocalWindowScaleValue));
        BDBG_MSG((" ConnectionControlBlock.ui32RemoteWindowScaleValue:    0x%x"  , pCcb->ui32RemoteWindowScaleValue));
        BDBG_MSG((" ConnectionControlBlock.ui32SackEnable:                0x%x"  , pCcb->ui32SackEnable));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeStampEnable:           0x%x"  , pCcb->ui32TimeStampEnable));
        BDBG_MSG((" ConnectionControlBlock.ui32TimeStampEchoValue:        0x%x"  , pCcb->ui32TimeStampEchoValue));
        BDBG_MSG((" ConnectionControlBlock.ui32MaxSegmentSize:            0x%x"  , pCcb->ui32MaxSegmentSize));
        BDBG_MSG((" ConnectionControlBlock.ui32KaTimeout:                 0x%x"  , pCcb->ui32KaTimeout));
        BDBG_MSG((" ConnectionControlBlock.ui32KaInterval:                0x%x"  , pCcb->ui32KaInterval));
        BDBG_MSG((" ConnectionControlBlock.ui32KaMaxProbes:               0x%x"  , pCcb->ui32KaMaxProbes));
        BDBG_MSG((" ConnectionControlBlock.ui32RetxTimeout:               0x%x"  , pCcb->ui32RetxTimeout));
        BDBG_MSG((" ConnectionControlBlock.ui32RetxMaxRetries:            0x%x"  , pCcb->ui32RetxMaxRetries));

        BDBG_MSG((" ui32StreamLabel             :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32StreamLabel));
        BDBG_MSG((" ui32PlaybackChannelNumber   :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32PlaybackChannelNumber));
        BDBG_MSG((" ui32PidChannel              :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32PidChannel));
        BDBG_MSG((" ui32AvgStreamBitrate        :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32AvgStreamBitrate));
        BDBG_MSG((" ui32SwitchQueueNumber       :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32SwitchQueueNumber));
        BDBG_MSG((" ui32SwitchSlotsPerEthernetPacket        :                                %u" , pMsg->MessagePayload.ChannelStartStreamIn.ui32SwitchSlotsPerEthernetPacket));
        break;
    }
    case BASP_MessageType_eFw2PiChannelStartStreamInResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiChannelStartStreamInResp"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  ResponseStatus"
                  ""
                  ""
                  ":     0x%08x" , pMsg->Message.ResponsePayload.StartStreamInResponse.ResponseStatus));
        break;
    }

    case BASP_MessageType_ePi2FwUnInit:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Pi2FwUnInit"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.UnInit.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiUnInitResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body               Fw2PiUnInitResp"));
        BDBG_MSG(("                   ====================="));
        BDBG_MSG(("  ResponseStatus:  %u"     , pMsg->Message.ResponsePayload.UnInitResponse.ResponseStatus));
        break;
    }

    case BASP_MessageType_eFw2PiPayloadNotify:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body              Fw2PiPayloadNotify"));
        BDBG_MSG(("                  ========================"));
        BDBG_MSG(("  responseAddress hi:lo:     0x%08x:0x%08x" ,
                    pMsg->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrLo,
                    pMsg->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrLo));
        BDBG_MSG(("  responseSize:              %d" , pMsg->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32Size));
        break;
    }

    case BASP_MessageType_ePi2FwPayloadConsumed:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body               Pi2FwPayloadConsumed"));
        BDBG_MSG(("                   ====================="));
        BDBG_MSG(("  ui32NumberofBytesToSkip:     0x%08x" , pMsg->MessagePayload.PayloadConsumed.ui32NumberofBytesToSkip));
        break;
    }
    case BASP_MessageType_eFw2PiPayloadConsumedResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body               Pi2FwPayloadConsumedResp"));
        BDBG_MSG(("                   ====================="));
        BDBG_MSG(("  ResponseStatus:     0x%08x" , pMsg->Message.ResponsePayload.PayloadConsumedResponse.ResponseStatus));
        break;
    }

    case BASP_MessageType_ePi2FwGetDrmConst:
    {
        BASP_Pi2Fw_Message  *pMsg = pMessage;
        BDBG_MSG(("Body               Pi2FwGetDrmConst"));
        BDBG_MSG(("                   ====================="));
        BDBG_MSG(("  ui32Unused:     0x%08x" , pMsg->MessagePayload.GetDrmConst.ui32Unused));
        break;
    }

    case BASP_MessageType_eFw2PiGetDrmConstResp:
    {
        BASP_Fw2Pi_Message  *pMsg = pMessage;
        BDBG_MSG(("Body               Fw2PiGetDrmConstResp"));
        BDBG_MSG(("                   ====================="));
        BDBG_MSG(("  ResponseStatus:  %u"     , pMsg->Message.ResponsePayload.GetDrmConstMessageResponse.ResponseStatus));
        break;
    }

    case BASP_MessageType_ePi2FwFrameConsumed:
    case BASP_MessageType_eFw2PiFrameConsumedResp:
    case BASP_MessageType_ePi2FwGenericSgTableFeed:
    case BASP_MessageType_eFw2PiGenericSgTableFeedResp:

    case BASP_MessageType_ePi2FwInvalid:

    /* Message from ASP FW to Host due to events from network / client */
    case BASP_MessageType_eFw2PiFrameAvailable:
    case BASP_MessageType_eFw2PiFinComplete:
    case BASP_MessageType_eFw2PiGenericSgTableFeedCompletion:
    case BASP_MessageType_eFw2PiRtoNotify: /* FW reaches maximum retries */

    /* RA related */
    case BASP_MessageType_eRa2FwReassembledPacket:
    case BASP_MessageType_eRa2FwReassembledPacketResp:
    case BASP_MessageType_eFw2RaReassembledPacketCompletion:

    /* SAGE related */
    case BASP_MessageType_eSage2FwConstInfo:
    case BASP_MessageType_eFw2SageReqConst:
    case BASP_MessageType_eFw2SageReqConstResp:
    default:
    {
        BDBG_MSG(("Body"));
        BDBG_MSG(("  Sorry, I don't know how to display MessageType=%u" , pMsgHeader->MessageType));
        BDBG_MSG(("  Please add code to %s()" , BSTD_FUNCTION));
        break;
    }


    }
    BDBG_MSG(("----------   End: %s ---------" , pHeading));

    return errCode;
}
