/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *   This module controls and returns the User Data coming in the stream
 * and captured by the decoder.
 *
 ***************************************************************************/
#include "bstd.h"                /* standard types */
#include "bavc.h"                /* for userdata */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_userdata.h"
#include "bxvd_vdec_info.h"

BDBG_MODULE(BXVD_USERDATA);

/* Default settings. */
static const BXVD_Userdata_Settings s_stUserdataDefaultSettings =
{
   (4 * 1024),   /* default 4 kbyte user data size */
   BXVD_P_USERDATA_QUEUE_MAX,
   BXVD_P_USERDATA_ITEM_SIZE
};

/* Initialize the userdata read queue */
BERR_Code BXVD_P_Userdata_QueueInitialize(QUEUE_MGR *queue, BXVD_Userdata_Settings stUDSettings)
{
   int i;

   BDBG_ENTER(BXVD_P_Userdata_QueueInitialize);

#ifdef BXVD_USERDATA_EXTRA_DEBUG
   BKNI_Printf("<<< maxDataSize: %d >>>\n", stUDSettings.maxDataSize);
   BKNI_Printf("<<< maxQueueDepth: %d >>>\n", stUDSettings.maxQueueDepth);
   BKNI_Printf("<<< maxQueueItemSize: %d >>>\n", stUDSettings.maxQueueItemSize);
#endif

   queue->queue_data = (struct data *)BKNI_Malloc(stUDSettings.maxQueueDepth*sizeof(struct data));
   if (queue->queue_data == NULL)
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

   BKNI_Memset(queue->queue_data, 0, stUDSettings.maxQueueDepth*sizeof(struct data));

   for (i = 0; i < stUDSettings.maxQueueDepth; i++)
   {
      queue->queue_data[i].uUserData = (unsigned char *)BKNI_Malloc(stUDSettings.maxQueueItemSize*sizeof(unsigned char));

      /* SWSTB-5460: handle BKNI_Malloc failures */
      if (queue->queue_data[i].uUserData == NULL)
         return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

      BKNI_Memset(queue->queue_data[i].uUserData, 0, stUDSettings.maxQueueItemSize);
   }

   queue->ulQueueDepth = 0;
   queue->ulReadPtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulWritePtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulNextPtr = BXVD_P_USERDATA_QUEUE_START;

   BDBG_LEAVE(BXVD_P_Userdata_QueueInitialize);
   return BERR_SUCCESS;
}


/* Clear the userdata queue. Just calls queue initialize */
BERR_Code BXVD_P_Userdata_QueueClear(QUEUE_MGR *queue, BXVD_Userdata_Settings stUDSettings)
{
   int i;
   queue->ulQueueDepth = 0;
   queue->ulReadPtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulWritePtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulNextPtr = BXVD_P_USERDATA_QUEUE_START;

   for (i = 0; i < stUDSettings.maxQueueDepth; i++)
      BKNI_Memset(queue->queue_data[i].uUserData, 0, stUDSettings.maxQueueItemSize);

   return BERR_SUCCESS;
}

BERR_Code BXVD_P_Userdata_QueueDestroy(QUEUE_MGR *queue, BXVD_Userdata_Settings stUDSettings)
{
   int i;

   queue->ulQueueDepth = 0;
   queue->ulReadPtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulWritePtr = BXVD_P_USERDATA_QUEUE_START;
   queue->ulNextPtr = BXVD_P_USERDATA_QUEUE_START;

   for (i = 0; i < stUDSettings.maxQueueDepth; i++)
   {
      /* SWSTB-5460: not all the entries will be valid if BKNI_Malloc
       * failed in BXVD_P_Userdata_QueueInitialize. */
      if (queue->queue_data[i].uUserData)
         BKNI_Free(queue->queue_data[i].uUserData);
   }

   if (queue->queue_data)
   {
      BKNI_Free(queue->queue_data);
      queue->queue_data = NULL;
   }

   return BERR_SUCCESS;
}

/*
 * SWSTB-5460: revamp cleanup to better handle failures at open time.
 */
BERR_Code BXVD_Userdata_Destroy(BXVD_Userdata_Handle hUserData)
{
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ENTER(BXVD_Userdata_Close);
   BDBG_ASSERT(hUserData);

   BXVD_P_Userdata_QueueDestroy(&hUserData->queue,  hUserData->sUserdataSettings);

   /*
    * Release all allocated buffers
    */
   BKNI_Free(hUserData->pBfr);
   BKNI_Free(hUserData);

   hUserData = NULL;

   BDBG_LEAVE(BXVD_Userdata_Close);
   return eStatus;
}


void *BXVD_P_ConvertUDOff2Addr_isr(BXVD_Userdata_Handle hUserData,
                                   BXVD_P_PHY_ADDR fwUserDataAddr)
{
   return (void*)BXVD_P_OFFSET_TO_VA(hUserData->hXvdCh, fwUserDataAddr);
}


/***************************************************************************
 *  {secret}
 *  BXVD_P_Userdata_QueueInsert_isr
 *  Adds data to the specified circular queue
 */
BERR_Code BXVD_P_Userdata_QueueInsert_isr(QUEUE_MGR *queue,
                                          int protocol,
                                          void *userDataPtr,
                                          unsigned long lUserDataSize,
                                          unsigned long ulFlags,
                                          unsigned long ulPulldown,
                                          unsigned long ulPTS,
                                          uint32_t uiDecodePictureId,
                                          BXVD_Userdata_Settings stUDSettings)
{
   BDBG_ENTER(BXVD_P_Userdata_QueueInsert);

   /* Make sure the queue pointers are valid */
   if ((queue->ulWritePtr < BXVD_P_USERDATA_QUEUE_START) ||
       (queue->ulWritePtr >= stUDSettings.maxQueueDepth))
   {
      return BERR_TRACE(BXVD_ERR_QUEUE_CORRUPTED);
   }

   /* Fill in the queue's next pointer */
   queue->ulNextPtr = queue->ulWritePtr+1;

   /* Wrap around */
   if (queue->ulNextPtr == stUDSettings.maxQueueDepth)
   {
      queue->ulNextPtr = BXVD_P_USERDATA_QUEUE_START;
   }

   /* Check for queue overflow */
   if (queue->ulNextPtr == queue->ulReadPtr)
   {
      return BERR_TRACE(BXVD_ERR_QUEUE_FULL);
   }

   /* Write value to queue */
   queue->queue_data[queue->ulWritePtr].protocol = protocol;
   queue->queue_data[queue->ulWritePtr].ulFlags = ulFlags;
   queue->queue_data[queue->ulWritePtr].ulPulldown = ulPulldown;
   queue->queue_data[queue->ulWritePtr].ulPTS = ulPTS;
   queue->queue_data[queue->ulWritePtr].uiDecodePictureId = uiDecodePictureId;

   BKNI_Memset_isr((unsigned char *)(queue->queue_data[queue->ulWritePtr].uUserData),
                   0x0,
                   stUDSettings.maxQueueItemSize);

#if 0
   BKNI_Printf("lUserDataSize: %ld\n", lUserDataSize);
#endif

   if (lUserDataSize > (unsigned long)stUDSettings.maxQueueItemSize)
   {

      BDBG_WRN(("lUserSataSize(%lu) > maxQueueItemSize(%d) Truncating to maxQueueItemSize and copying with bErrorBufferOverflow set to true",
                lUserDataSize, stUDSettings.maxQueueItemSize));

      BKNI_Memcpy_isr(queue->queue_data[queue->ulWritePtr].uUserData,
                      userDataPtr,
                      stUDSettings.maxQueueItemSize);

      queue->ulWritePtr = queue->ulNextPtr;
      queue->ulQueueDepth++;
      return BERR_TRACE(BXVD_ERR_USERDATA_ITEM_TOO_LARGE);
   }
   else
   {
      BKNI_Memcpy_isr(queue->queue_data[queue->ulWritePtr].uUserData,
                      userDataPtr,
                      lUserDataSize);
   }

   queue->ulWritePtr = queue->ulNextPtr;
   queue->ulQueueDepth++;

   return BERR_SUCCESS;
}

/***************************************************************************
 *  {secret}
 *  BXVD_P_Userdata_QueueRemove_isr
 *  Remove next entry from specified circular queue.
 */
static BERR_Code BXVD_P_Userdata_QueueRemove_isr(QUEUE_MGR *queue,
                                                 int *protocol,
                                                 uint8_t **udp,
                                                 unsigned long *ulFlags,
                                                 unsigned long *ulPulldown,
                                                 unsigned long *ulPTS,
                                                 uint32_t *uiDecodePictureId,
                                                 BXVD_Userdata_Settings stUDSettings)
{
   BDBG_ENTER(BXVD_P_Userdata_QueueRemove_isr);

   /* Make sure there is data in the queue */
   if (queue->ulReadPtr == queue->ulWritePtr)
   {
      return BXVD_ERR_QUEUE_EMPTY;
   }


   /* Check the queue pointers for validity */
   if ((queue->ulWritePtr < BXVD_P_USERDATA_QUEUE_START) ||
       (queue->ulWritePtr >= stUDSettings.maxQueueDepth))
   {
      return BERR_TRACE(BXVD_ERR_QUEUE_CORRUPTED);
   }

   /* Return the userdata information from the queue */
   *protocol = queue->queue_data[queue->ulReadPtr].protocol;
   *udp = queue->queue_data[queue->ulReadPtr].uUserData;
   *ulFlags = queue->queue_data[queue->ulReadPtr].ulFlags;
   *ulPulldown = queue->queue_data[queue->ulReadPtr].ulPulldown;
   *ulPTS = queue->queue_data[queue->ulReadPtr].ulPTS;
   *uiDecodePictureId = queue->queue_data[queue->ulReadPtr].uiDecodePictureId;

   /* Increment the userdata read pointer and decrement the depth */
   queue->ulReadPtr++;
   queue->ulQueueDepth--;

   /* Check for wrap around */
   if (queue->ulReadPtr == stUDSettings.maxQueueDepth)
   {
      queue->ulReadPtr = BXVD_P_USERDATA_QUEUE_START;
   }

   BDBG_LEAVE(BXVD_P_Userdata_QueueRemove_isr);
   return BERR_SUCCESS;
}

/***************************************************************************
 *  {secret}
 * BXVD_P_Userdata_EnqueueDataPointer
 */
BERR_Code BXVD_P_Userdata_EnqueueDataPointer_isr(BXVD_ChannelHandle hXvdCh,
                                                 int protocol,
                                                 unsigned long p_UserData,
                                                 unsigned long ulFlags,
                                                 unsigned long ulPulldown,
                                                 unsigned long ulPTS,
                                                 uint32_t uiDecodePictureId)
{
   BERR_Code rc = BERR_SUCCESS;
   void *userDataPtr;
   UD_HDR *pHdrInfo = (UD_HDR *)NULL;

   BXVD_P_PHY_ADDR p_UserDataTemp;

   BDBG_ENTER(BXVD_P_Userdata_EnqueueDataPointer_isr);

   /* Initialize global error to success */
   if (hXvdCh->pUserData)
      hXvdCh->pUserData->errForwardError = BERR_SUCCESS;

   /*
    * If the global user data context pointer isn't initialized, return
    * an error.
    */
   if (hXvdCh->pUserData == 0)
   {
      BXVD_DBG_MSG(hXvdCh, ("BXVD_P_Userdata_EnqueueDataPointer: not initialized"));
      return BXVD_ERR_USERDATA_UNINITED;
   }

   /*
    * If the user data callback is disabled, return an error. This is actually
    * more of a warning than an indication of something wrong in the userdata
    * subsystem.
    */
   if (hXvdCh->pUserData->bCallbackEnabled == false)
   {
      BXVD_DBG_MSG(hXvdCh, ("BXVD_P_Userdata_EnqueueDataPointer: callback disabled"));
      return BXVD_ERR_USERDATA_DISABLED;
   }

   /* If no userdata callback is installed, return the buffer and signal an
    * error.
    */
   if (hXvdCh->pUserData->fUserdataCallback_isr == NULL)
   {
      BXVD_DBG_MSG(hXvdCh, ("BXVD_P_Userdata_EnqueueDataPointer: callback not installed"));
      return BXVD_ERR_USERDATA_DISABLED;
   }

   /*
    * If the user data pointer is NULL return a no data error.
    */
   if (p_UserData == 0)
   {
      BXVD_DBG_MSG(hXvdCh, ("BXVD_P_Userdata_EnqueueDataPointer: no data"));
      return BERR_TRACE(BXVD_ERR_USERDATA_NONE);
   }


   /*
    * Get the protocol type and user data pointer from DM and convert it to
    * a virtual address before enqueueing.
    */
#if BXVD_P_CORE_40BIT_ADDRESSABLE
   p_UserDataTemp = p_UserData + hXvdCh->stDecodeFWBaseAddrs.FWContextBase;
#else
   p_UserDataTemp = p_UserData;
#endif

   userDataPtr = BXVD_P_ConvertUDOff2Addr_isr(hXvdCh->pUserData, p_UserDataTemp);

   if (userDataPtr == NULL)
      return BERR_TRACE(BXVD_ERR_USERDATA_INVALID);


   /* Loop through user data following the next pointer until the last
    * (or a single) packet is found. We call the application UD callback
    * each time a packet is found, converted and copied.
    */
   do
   {
      /* Extract the header information */
      pHdrInfo = (UD_HDR *)userDataPtr;

      BMMA_FlushCache_isr(hXvdCh->hFWGenMemBlock, userDataPtr, sizeof(UD_HDR));

      BMMA_FlushCache_isr(hXvdCh->hFWGenMemBlock, (uint8_t *)userDataPtr + sizeof(UD_HDR), pHdrInfo->size);

      /* SWSTB-3950: don't copy non user data onto the user data queue. */

      if ( !( pHdrInfo->type & BXVD_P_PPB_METADATA_PRESENT ) )
      {
         /*
          * Enqueue the data. The uiDecodePictureId member was added for transcode
          * userdata support. Jira: SW7425-1780
          */
         rc = BXVD_P_Userdata_QueueInsert_isr(&((hXvdCh->pUserData)->queue),
                                              protocol,
                                              userDataPtr,
                                              ((sizeof(UD_HDR)+((pHdrInfo->size+3))) & ~3), /* Make sure we copy long words, endianess issue */
                                              ulFlags,
                                              ulPulldown,
                                              ulPTS,
                                              uiDecodePictureId,
                                              hXvdCh->pUserData->sUserdataSettings);
         if (rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(hXvdCh, ("Could not enqueue user data packet"));
            hXvdCh->pUserData->errForwardError = rc;
            goto doCallback;
            /*return rc;*/
         }

      }

      /* Get the next user data packet, if any */
      if (pHdrInfo->next)
      {

#if BXVD_P_CORE_40BIT_ADDRESSABLE
         p_UserDataTemp = pHdrInfo->next + hXvdCh->stDecodeFWBaseAddrs.FWContextBase;
#else
         p_UserDataTemp = pHdrInfo->next;
#endif

         userDataPtr = BXVD_P_ConvertUDOff2Addr_isr(hXvdCh->pUserData, p_UserDataTemp);
         if (userDataPtr == NULL)
            return BERR_TRACE(BXVD_ERR_USERDATA_INVALID);
      }

   } while (pHdrInfo->next);

doCallback:

   /* Invoke application UD read callback */
   hXvdCh->pUserData->fUserdataCallback_isr(hXvdCh->pUserData->pParm1, hXvdCh->pUserData->parm2);

   BDBG_LEAVE(BXVD_P_Userdata_EnqueueDataPointer_isr);
   return rc;
}

/***************************************************************************
 * Get userdata default settings. Currently this is only the default
 * userdata buffer size (4K).
 ***************************************************************************/
BERR_Code BXVD_Userdata_GetDefaultSettings(BXVD_Userdata_Settings *pDefSettings)
{
   BDBG_ENTER(BXVD_Userdata_GetDefaultSettings);
   BDBG_ASSERT(pDefSettings);

   *pDefSettings = s_stUserdataDefaultSettings;

   BDBG_LEAVE(BXVD_Userdata_GetDefaultSettings);
   return BERR_SUCCESS;
}

/***************************************************************************
 * Open an instance of the userdata module.
 ***************************************************************************/
BERR_Code BXVD_Userdata_Open(BXVD_ChannelHandle            hXvdCh,
                 BXVD_Userdata_Handle         *phUserData,
                 const BXVD_Userdata_Settings *pDefSettings)
{
   BERR_Code               eStatus = BERR_SUCCESS;
   BXVD_P_UserDataContext *pUserdata = NULL;

   BDBG_ENTER(BXVD_Userdata_Open);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(phUserData);
   BSTD_UNUSED(pDefSettings);

   /* Return null handle if we fail to create one */
   *phUserData = NULL;

   /* Allocate user data handle */
   pUserdata = (BXVD_P_UserDataContext*)(BKNI_Malloc(sizeof(BXVD_P_UserDataContext)));

   if(!pUserdata)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
   /* Clear out the context and set defaults. */
   BKNI_Memset((void*)pUserdata, 0x0, sizeof(BXVD_P_UserDataContext));

   /* Set the handle type */
   pUserdata->eHandleType = BXVD_P_HandleType_Userdata;

   /* Take in default settings. */
   pUserdata->sUserdataSettings = (pDefSettings) ?
      *pDefSettings : s_stUserdataDefaultSettings;

   /* Initialize userdata parameters */
   pUserdata->hXvdCh = hXvdCh;
   pUserdata->bCallbackEnabled = false;
   pUserdata->fUserdataCallback_isr = NULL;
   pUserdata->errForwardError = BERR_SUCCESS;

   /* Allocate the userdata work buffer */
   pUserdata->pBfr = BKNI_Malloc(pUserdata->sUserdataSettings.maxDataSize);
   BDBG_ASSERT((unsigned long)pUserdata->pBfr%4==0);
   if(!pUserdata->pBfr)
   {
      BKNI_Free(pUserdata);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* All done. Return the new Channel context to user and set the global
    * user data handle
    */
   *phUserData = hXvdCh->pUserData = (BXVD_Userdata_Handle)pUserdata;

   /* Initialize the userdata queue */
   eStatus = BXVD_P_Userdata_QueueInitialize(&(hXvdCh->pUserData->queue), pUserdata->sUserdataSettings);

   /* SWSTB-5460: cleanup if a BKNI_Malloc failed in BXVD_P_Userdata_QueueInitialize */
   if ( eStatus != BERR_SUCCESS )
   {
      BXVD_DBG_ERR(hXvdCh, ("%s: BKNI_Malloc failed", BSTD_FUNCTION ));
      BXVD_Userdata_Destroy( hXvdCh->pUserData );
      *phUserData = NULL;
   }

#ifdef BXVD_FLATTEN_USERDATA
   BXVD_DBG_MSG(hXvdCh, ("Userdata will be delivered as coalesced packets"));
#else
   BXVD_DBG_MSG(hXvdCh, ("Userdata will be delivered as single packets"));
#endif

   BDBG_LEAVE(BXVD_Userdata_Open);
   return eStatus;
}

/***************************************************************************
* Close a previously opened userdata instance
****************************************************************************/
BERR_Code BXVD_Userdata_Close(BXVD_Userdata_Handle hUserData)
{
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ENTER(BXVD_Userdata_Close);
   BDBG_ASSERT(hUserData);

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Clear internal state inside a critical section */
   BKNI_EnterCriticalSection();
   hUserData->bCallbackEnabled = false;
   hUserData->fUserdataCallback_isr = NULL;
   hUserData->hXvdCh->pUserData = NULL;
   BKNI_LeaveCriticalSection();

   BXVD_Userdata_Destroy( hUserData );

   BDBG_LEAVE(BXVD_Userdata_Close);
   return eStatus;
}

/***************************************************************************
    * Read user data. Non-isr version
****************************************************************************/
BERR_Code BXVD_Userdata_Read(BXVD_Userdata_Handle   hUserData,
                 BAVC_USERDATA_info    *pUserDataInfo)
{
   BERR_Code status;

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BKNI_EnterCriticalSection();
   status = BXVD_Userdata_Read_isr(hUserData, pUserDataInfo);
   BKNI_LeaveCriticalSection();
   return status;
}


/***************************************************************************
    * Read user data. ISR version
****************************************************************************/
BERR_Code BXVD_Userdata_Read_isr(BXVD_Userdata_Handle   hUserData,
                 BAVC_USERDATA_info *pUserDataInfo)
{
   int protocol;
   uint32_t uiDecodePictureId;
   unsigned long ulFlags, ulPulldown, ulPTS;
   BERR_Code       eStatus = BERR_SUCCESS;
   unsigned offset;
   uint8_t    *pDataBfr;
   unsigned       entries;
   uint8_t *userDataPtr;
#ifdef BXVD_FLATTEN_USERDATA
   bool            bMoreUserdata;
#endif

   UD_HDR *pHdr;

   BDBG_ENTER(BXVD_Userdata_Read);
   BDBG_ASSERT(hUserData);
   BDBG_ASSERT(pUserDataInfo);

   protocol = 0;
   userDataPtr = NULL;
   uiDecodePictureId = 0;
   ulFlags = ulPulldown = ulPTS = 0;

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /*
    * Clear overflow flag. We'll set it if a queue full condition is
    * detected below.
    */
   pUserDataInfo->bErrorBufferOverflow = false;

   /*
    * If there was an error detected in the enqueuing routine.
    * forward it to the user callback unless it was a queue overflow
    */
   if (hUserData->errForwardError == BXVD_ERR_QUEUE_FULL)
   {
      BDBG_ERR(("Queue full condition detected, bErrorBufferOverflow set"));
      pUserDataInfo->bErrorBufferOverflow = true;
   }
   else if (hUserData->errForwardError != BERR_SUCCESS)
   {
      BDBG_ERR(("Error detected in enqueuing routine"));
      return BERR_TRACE(hUserData->errForwardError);
   }

   /* Clear out the user data info structure */
   BKNI_Memset((void *)pUserDataInfo, 0, sizeof(*pUserDataInfo));

   /* Get the userdata from the queue */
   if (BXVD_P_Userdata_QueueRemove_isr(&(hUserData->queue),
                                       &protocol,
                                       &userDataPtr,
                                       &ulFlags,
                                       &ulPulldown,
                                       &ulPTS,
                                       &uiDecodePictureId,
                                       hUserData->sUserdataSettings)
       == BXVD_ERR_QUEUE_EMPTY)
   {
      return BXVD_ERR_USERDATA_NONE;
   }

   /*
    * Overflow condition is checked by DM, so we can set overflow flag
    * to false here.
    */
   pUserDataInfo->bErrorBufferOverflow = false;

   /* Set field polarity flag */
   pUserDataInfo->bTopFieldFirst =
      (ulFlags & BXVD_P_PPB_FLAG_BOTTOM_FIRST) ? false : true;

   /* Set repeat flag */
   pUserDataInfo->bRepeatFirstField =
      ((ulPulldown == BXVD_P_PPB_PullDown_eTopBottomTop) ||
       (ulPulldown == BXVD_P_PPB_PullDown_eBottomTopBottom) ||
       (ulPulldown == BXVD_P_PPB_PullDown_eFrameX2) ||
       (ulPulldown == BXVD_P_PPB_PullDown_eFrameX3) ||
       (ulPulldown == BXVD_P_PPB_PullDown_eFrameX4)) ? true : false;

   /*
    * Copy the user data buffer pointer to the info structure and make a
    * local copy for processing
    */
   pUserDataInfo->pUserDataBuffer = hUserData->pBfr;
   pDataBfr = (uint8_t *)hUserData->pBfr;

   /* SWSTB-3092: check if pDataBfr is NULL */
   if ( NULL == pDataBfr )
   {
      BDBG_WRN(("%s: pDataBfr is NULL", BSTD_FUNCTION ));
      eStatus = BERR_TRACE(BXVD_ERR_USERDATA_INVALID);
      goto consume;
   }

   /* Set up parsing loop initial conditions */
   offset = 0;
#ifdef BXVD_FLATTEN_USERDATA
   bMoreUserdata = true;
   /* Parse and format the user data */
   while( bMoreUserdata )
   {
#endif
      /* Pass on the PTS and PTS valid   flag */
      pUserDataInfo->ui32PTS = ulPTS;
      pUserDataInfo->bPTSValid = (ulFlags&BXVD_P_PPB_FLAG_PTS_PRESENT)?true : false;

      /* Pass on the decode picture id */
      pUserDataInfo->ulDecodePictureId = uiDecodePictureId;

      if ( NULL == userDataPtr )
      {
         BDBG_WRN(("%s: userDataPtr is NULL", BSTD_FUNCTION ));
         eStatus = BERR_TRACE(BXVD_ERR_USERDATA_INVALID);
         goto consume;
      }

      /* Get Userdata info */
      pHdr = (UD_HDR *)userDataPtr;

      /*
       * Parse the user data.
       */
      /* Valid for mpeg2 only */
      if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_I)
         pUserDataInfo->ePicCodingType = BAVC_USERDATA_PictureCoding_eI;
      else if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_P)
         pUserDataInfo->ePicCodingType = BAVC_USERDATA_PictureCoding_eP;
      else
         pUserDataInfo->ePicCodingType = BAVC_USERDATA_PictureCoding_eB;

      /* Get userdata type */
      if (BXVD_IS_AVC(protocol))
      {
         pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eSEI;
      }
      else if (BXVD_IS_MPEG(protocol))
      {
         if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_SEQ)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eSeq;
         else if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_GOP)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eGOP;
         else
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_ePicture;
      }
      else if (BXVD_IS_AVS(protocol))
      {
         pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eFrame;
      }
      else   /* VC1 */
      {
         if (pHdr->type & BXVD_P_PPB_VC1_USERDATA_TYPE_SEQ)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eSeq;
         else if (pHdr->type & BXVD_P_PPB_VC1_USERDATA_TYPE_ENTRYPOINT)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eEntryPoint;
         else if (pHdr->type & BXVD_P_PPB_VC1_USERDATA_TYPE_FLD)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eField;
         else if (pHdr->type & BXVD_P_PPB_VC1_USERDATA_TYPE_FRM)
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eFrame;
         else
            pUserDataInfo->eUserDataType = BAVC_USERDATA_Type_eSlice;
      }

      /* mpeg2 and avc uses the same it fields defs */
      if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_TOP)
         pUserDataInfo->eSourcePolarity = BAVC_Polarity_eTopField;
      else if (pHdr->type & BXVD_P_PPB_MPEG_USERDATA_TYPE_BOT)
         pUserDataInfo->eSourcePolarity = BAVC_Polarity_eBotField;
      else
         pUserDataInfo->eSourcePolarity = BAVC_Polarity_eFrame;

      /* Check the user data packet size */
      if(offset+4 > (unsigned)hUserData->sUserdataSettings.maxDataSize)
      {
         BDBG_WRN(("user data packet is too big %u+%u(%u)[%u]",
                   offset,
                   4,
                   offset+4,
                   hUserData->sUserdataSettings.maxDataSize));

         eStatus = BERR_TRACE(BXVD_ERR_USERDATA_USRBFROFL);
         goto consume;
      }

      /* Check buffer alignment */
      if ((unsigned long)pDataBfr % 4)
      {
         BDBG_WRN(("user data buffer is unaligned"));
         eStatus = BERR_TRACE(BXVD_ERR_USERDATA_INVALID);
         goto consume;

      }

      /* Create simulated data headers based on protocol */
      if (BXVD_IS_AVC(protocol))
      {
         /*     simulate NAL and SEI header */
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;

         pDataBfr[offset++] = 0x01;
         pDataBfr[offset++] = 0x06;

         if (pHdr->type == BXVD_P_PPB_H264_USERDATA_TYPE_REGISTERED)
         {
            pDataBfr[offset++] = (uint8_t) BXVD_USERDATA_H264_TYPE_REGISTERED;
         }
         else if (pHdr->type == BXVD_P_PPB_H264_USERDATA_TYPE_FRAME_PACK)
         {
            pDataBfr[offset++] = (uint8_t) BXVD_USERDATA_H264_TYPE_FRAME_PACK;
         }
         else
         {
            pDataBfr[offset++] = (uint8_t) BXVD_USERDATA_H264_TYPE_UNREGISTERED;
         }

         pDataBfr[offset++] = (uint8_t)pHdr->size;
      }
      else if (protocol == BAVC_VideoCompressionStd_eVC1)
      {
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x01;
         pDataBfr[offset++] = 0x1E;
      }
      else
      {
         /* <MPEG-2> An extra 0 is prepended to keep data aligned to 32 bits */
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x00;
         pDataBfr[offset++] = 0x01;
      }

      /* Check the user data packet size again after header creation */
      if(offset+pHdr->size >
         (unsigned)hUserData->sUserdataSettings.maxDataSize)
      {
         BDBG_WRN(("user data packet is too big %u+%u(%u)[%u]",
                   offset,
                   pHdr->size,
                   offset+pHdr->size,
                   hUserData->sUserdataSettings.maxDataSize));

         eStatus = BERR_TRACE(BXVD_ERR_USERDATA_USRBFROFL);
         goto consume;
      }

      /* Get number of 32 bit entries */
      entries = (pHdr->size+3)>>2;

      /* Copy segment of user data after verifying source and destination pointers */
      BKNI_Memcpy(&pDataBfr[offset],
                  userDataPtr + sizeof(UD_HDR),
                  entries*4);

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
      { /* convert endianess */
         uint32_t data;
         unsigned i;
         for(i=0;i<entries;i++)
         {
            data = ((uint32_t *)(pDataBfr+offset))[i];
            data = ((data >> 24) & 0xFF) |
               ((data >> 8) & 0xFF00) |
               ((data & 0xFF00) << 8) |
               ((data & 0xFF) << 24);
            ((uint32_t *)(pDataBfr+offset))[i] = data;
         }
      }
#elif   BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
/* do nothing */
#else
#error   "Not supported"
#endif

      /* Adjust packet size */
      offset += pHdr->size;

      /* Align to 32 bits and pad with 0 */
      switch(offset%4)
      {
         case 1:
            pDataBfr[offset++] = 0x00;
            /* Fallthrough */
         case 2:
            pDataBfr[offset++] = 0x00;
            /* Fallthrough */
         case 3:
            pDataBfr[offset++] = 0x00;
            /* Fallthrough */
         default:
            break;
      }
#ifdef BXVD_FLATTEN_USERDATA
      /* Get the userdata from the queue */
      if (BXVD_P_Userdata_QueueRemove_isr(&(hUserData->queue),
                                          &protocol,
                                          &userDataPtr,
                                          &ulFlags,
                                          &ulPulldown,
                                          &ulPTS,
                                          &uiDecodePictureId,
                                          hUserData->sUserdataSettings)
      == BXVD_ERR_QUEUE_EMPTY)
      {
         bMoreUserdata = false;
      }
#ifdef BXVD_BREAK_ON_TYPE_CHANGE
      else
      {
         UD_HDR *tmp;
         tmp = (UD_HDR *)userDataPtr;
         if (tmp->type != pHdr->type)
         {
            BDBG_MSG(("tmp->type : pHdr->type", tmp->type, pHdr->type));
            break;
         }
      }
#endif
   }
#endif
   /* Set the new packet size in user data info struct */
   pUserDataInfo->ui32UserDataBufSize = offset;

#ifdef BXVD_USERDATA_EXTRA_DEBUG
   {
      uint32_t x;
      BXVD_PTSInfo ptsInfo;
      BKNI_Printf("ui32UserDataBufSize = 0x%x (%d)\n", offset, offset);
      BKNI_Printf("pHdr->type = 0x%x (%d)\n", pHdr->type, pHdr->type);
      if (ulPTS == 0)
      {
     BXVD_GetPTS_isr(hUserData->hXvdCh, &ptsInfo);
     BKNI_Printf("Interpolated running PTS = 0x%x (%u) - ",
             ptsInfo.ui32RunningPTS,
             ptsInfo.ui32RunningPTS);
     BKNI_Printf("Interpolated effective PTS = 0x%x (%u)\n",
             ptsInfo.ui32EffectivePTS,
             ptsInfo.ui32EffectivePTS);

      }
      else
     BKNI_Printf("PTS = 0x%x (%d)\n", (unsigned)ulPTS, (unsigned)ulPTS);
      BKNI_Printf("pUserDataInfo->eUserDataType = 0x%x (%d)\n",
                  pUserDataInfo->eUserDataType,
                  pUserDataInfo->eUserDataType);
      for (x = 0; x < offset; x++)
      {
         if (!((x+1)%45))
            BKNI_Printf("<end>\n");
         BKNI_Printf("%2.2x ", pDataBfr[x]);
      }
      BKNI_Printf("<end>\n");
      BKNI_Printf("actual size: 0x%x (%d)\n", x, x);
      BKNI_Printf("------------------------------------------------------\n");
   }
#endif

  consume:

   BDBG_LEAVE(BXVD_Userdata_Read);
   return eStatus;
}

/***************************************************************************
 * Install the userdata read interrupt callback
 ***************************************************************************/
BERR_Code BXVD_Userdata_InstallInterruptCallback
(
   BXVD_Userdata_Handle    hUserData,
   BINT_CallbackFunc       xvdInterruptCallBack,
   void                   *pParm1,
   int                     parm2
   )
{
   BDBG_ENTER(BXVD_Userdata_InstallInterruptCallback);

   BDBG_ASSERT(hUserData);
   BDBG_ASSERT(xvdInterruptCallBack);
   BSTD_UNUSED(parm2);

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hUserData->fUserdataCallback_isr = xvdInterruptCallBack;
   hUserData->pParm1 = pParm1;
   hUserData->parm2  = parm2;

   hUserData->bCallbackEnabled = false;

   BDBG_LEAVE(BXVD_Userdata_InstallInterruptCallback);
   return BERR_SUCCESS;
}

/***************************************************************************
 * Uninstall the read interrupt callback
 ***************************************************************************/
BERR_Code BXVD_Userdata_UninstallInterruptCallback
(
   BXVD_Userdata_Handle    hUserData,
   BINT_CallbackFunc       xvdInterruptCallBack
   )
{
   BDBG_ENTER(BXVD_Userdata_UninstallInterruptCallback);

   BDBG_ASSERT(hUserData);
   BSTD_UNUSED(xvdInterruptCallBack);

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hUserData->bCallbackEnabled = false;

   hUserData->fUserdataCallback_isr = NULL;
   hUserData->pParm1 = 0;
   hUserData->parm2  = 0;

   BDBG_LEAVE(BXVD_Userdata_UninstallInterruptCallback);
   return BERR_SUCCESS;
}

/***************************************************************************
 * Enable userdata
 ***************************************************************************/
BERR_Code BXVD_Userdata_Enable
(
   BXVD_Userdata_Handle     hUserData,
   bool                     bEnable
   )
{
   BDBG_ENTER(BXVD_Userdata_Enable);
   BDBG_ASSERT(hUserData);

   /* Check handle type for correctness */
   if (hUserData->eHandleType != BXVD_P_HandleType_Userdata)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hUserData->bCallbackEnabled = bEnable;

   /* If user data is being enabled, clear the existing queue */
   if (bEnable == true)
   {
      BXVD_P_Userdata_QueueClear(&(hUserData->queue), hUserData->sUserdataSettings);
   }

   BDBG_LEAVE(BXVD_Userdata_Enable);
   return BERR_SUCCESS;
}
/* End of File */
