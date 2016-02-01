/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bxvd_status.h"
#include "bxvd_status_priv.h"
#include "bint_stats.h"

BDBG_MODULE(BXVD_STATUS);

static void BXVD_Status_S_InterruptStatsReset(
   BXVD_StatusHandle hXvdStatus
   )
{
#ifdef BINT_STATS_ENABLE
   /* Reset XVD interrupt stats */
   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      BINT_Stats_Reset(hXvdStatus->hXvd->stDecoderContext.pCbAVC_MBX_ISR);
   }

   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      BINT_Stats_Reset(hXvdStatus->hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);
   }

   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)
   {
      BINT_Stats_Reset(hXvdStatus->hXvd->stDecoderContext.pCbAVC_Watchdog_ISR);
   }
#else
   BSTD_UNUSED(hXvdStatus);
#endif  
}

static void BXVD_Status_S_InterruptStatsPrint(
   BXVD_StatusHandle hXvdStatus
   )
{
#ifdef BINT_STATS_ENABLE
   BINT_Stats_CallbackStats *pMBXCallbackStats;
   BINT_Stats_CallbackStats *pStillPicRdyCallbackStats;
   BINT_Stats_CallbackStats *pWatchdogCallbackStats;
      
   /* Retrieve XVD interrupt stats */
   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      BINT_Stats_Get(hXvdStatus->hXvd->stDecoderContext.pCbAVC_MBX_ISR,
                     &pMBXCallbackStats);
   }

   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      BINT_Stats_Get(hXvdStatus->hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR,
                     &pStillPicRdyCallbackStats);
   }
            
   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)
   {
      BINT_Stats_Get(hXvdStatus->hXvd->stDecoderContext.pCbAVC_Watchdog_ISR,
                     &pWatchdogCallbackStats);
   }

   /* Print XVD interrupt stats */
   BKNI_EnterCriticalSection();
   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_MBX_ISR)
   {
      BDBG_MSG(("Device[%d]: Mailbox ISR = 0x%08x (0x%08x - 0x%08x) [0x%08x]",
                hXvdStatus->hXvd->uDecoderInstance,
                pMBXCallbackStats->ulTimeAvg,
                pMBXCallbackStats->ulTimeMin,
                pMBXCallbackStats->ulTimeMax,
                pMBXCallbackStats->ulCbHitCount));
   }

   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      BDBG_MSG(("Device[%d]: StillPictureReady ISR = 0x%08x (0x%08x - 0x%08x) [0x%08x]",
                hXvdStatus->hXvd->uDecoderInstance,
                pStillPicRdyCallbackStats->ulTimeAvg,
                pStillPicRdyCallbackStats->ulTimeMin,
                pStillPicRdyCallbackStats->ulTimeMax,
                pStillPicRdyCallbackStats->ulCbHitCount));
   }

   if (hXvdStatus->hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)
   {
      BDBG_MSG(("Device[%d]: Watchdog ISR = 0x%08x (0x%08x - 0x%08x) [0x%08x]",
                hXvdStatus->hXvd->uDecoderInstance,
                pWatchdogCallbackStats->ulTimeAvg,
                pWatchdogCallbackStats->ulTimeMin,
                pWatchdogCallbackStats->ulTimeMax,
                pWatchdogCallbackStats->ulCbHitCount));
   }
   BKNI_LeaveCriticalSection();
#else
   BSTD_UNUSED(hXvdStatus);
#endif
}

/* Create the XVD Status Handle */
BERR_Code BXVD_Status_Open(
   BXVD_Handle hXvd,
   BXVD_StatusHandle *phXvdStatus
   )
{
   BXVD_P_Status_Context *pXvdStatus = NULL;

   BDBG_ENTER(BXVD_Status_Open);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(phXvdStatus);
   
   /* Set handle to NULL in case the allocation fails */
   *phXvdStatus = NULL;

   pXvdStatus = (BXVD_P_Status_Context *) BKNI_Malloc(sizeof(BXVD_P_Status_Context));
   if (!pXvdStatus)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pXvdStatus, 0, sizeof(BXVD_P_Status_Context));
   
   pXvdStatus->hXvd = hXvd;

   *phXvdStatus = pXvdStatus;
   
   BDBG_LEAVE(BXVD_Status_Open);
   
   return BERR_TRACE(BERR_SUCCESS);
}

/* Destroy the XVD Status Handle */
BERR_Code BXVD_Status_Close(
   BXVD_StatusHandle hXvdStatus
   )
{
   BDBG_ENTER(BXVD_Status_Close);
   
   BDBG_ASSERT(hXvdStatus);
   
   /* Print Status */
   BXVD_Status_Print(hXvdStatus);
   
   /* Release the status context */
   BKNI_Free(hXvdStatus);
   
   BDBG_LEAVE(BXVD_Status_Close);
   
   return BERR_TRACE(BERR_SUCCESS);
}

/* Increment the number of times this channel has been opened */
BERR_Code BXVD_Status_OpenChannel(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   )
{
   BDBG_ENTER(BXVD_Status_OpenChannel);

   BDBG_ASSERT(hXvdStatus);
   BDBG_ASSERT(hXvdCh);
   
   hXvdStatus->auiOpenChannelCount[hXvdCh->ulChannelNum]++;
   
   BDBG_LEAVE(BXVD_Status_OpenChannel);

   return BERR_TRACE(BERR_SUCCESS);
}

/* Increment the number of times this channel has been closed */
BERR_Code BXVD_Status_CloseChannel(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   )
{
   BDBG_ENTER(BXVD_Status_CloseChannel);

   BDBG_ASSERT(hXvdStatus);
   BDBG_ASSERT(hXvdCh);
   
   hXvdStatus->auiCloseChannelCount[hXvdCh->ulChannelNum]++;
   
   BDBG_LEAVE(BXVD_Status_CloseChannel);

   return BERR_TRACE(BERR_SUCCESS);
}

/* Increment the number of times a decode has been started on this channel */
BERR_Code BXVD_Status_StartDecode(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   )
{
   BDBG_ENTER(BXVD_Status_StartDecode);

   BDBG_ASSERT(hXvdStatus);
   BDBG_ASSERT(hXvdCh);
   
   hXvdStatus->auiStartDecodeCount[hXvdCh->ulChannelNum]++;

   BXVD_Status_S_InterruptStatsReset(hXvdStatus);

   BDBG_LEAVE(BXVD_Status_StartDecode);

   return BERR_TRACE(BERR_SUCCESS);
}

/* Copy and Accumulate Channel Status */
BERR_Code BXVD_Status_StopDecode(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc;

   BXVD_ChannelStatus stCurrentChannelStatus;
   BXVD_ChannelStatus *pstChannelStatus = NULL;

   BDBG_ENTER(BXVD_Status_StopDecode);
   
   BDBG_ASSERT(hXvdStatus);
   BDBG_ASSERT(hXvdCh);

   pstChannelStatus = &hXvdStatus->astChannelStatus[hXvdCh->ulChannelNum];

   BDBG_ASSERT(pstChannelStatus);
   
   hXvdStatus->auiStopDecodeCount[hXvdCh->ulChannelNum]++;

   /* Get Channel Status for this channel */
   rc = BXVD_GetChannelStatus(hXvdCh,
                              &stCurrentChannelStatus);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Add up the stats from this stop decode to our running channel
    * status totals */

   
   pstChannelStatus->ulUnderflowCount += stCurrentChannelStatus.ulUnderflowCount;
   pstChannelStatus->ulPictureDeliveryCount += stCurrentChannelStatus.ulPictureDeliveryCount;
   pstChannelStatus->uiDisplayManagerDroppedCount += stCurrentChannelStatus.uiDisplayManagerDroppedCount;
   pstChannelStatus->uiDecoderDroppedCount += stCurrentChannelStatus.uiDecoderDroppedCount;
   pstChannelStatus->uiDisplayedParityFailureCount += stCurrentChannelStatus.uiDisplayedParityFailureCount;
   pstChannelStatus->uiDisplayedCount += stCurrentChannelStatus.uiDisplayedCount;
   pstChannelStatus->uiPicturesDecodedCount += stCurrentChannelStatus.uiPicturesDecodedCount;
   pstChannelStatus->uiVsyncUnderflowCount += stCurrentChannelStatus.uiVsyncUnderflowCount;
   pstChannelStatus->uiDecodeErrorCount += stCurrentChannelStatus.uiDecodeErrorCount;
   pstChannelStatus->uiVsyncCount += stCurrentChannelStatus.uiVsyncCount;

   BXVD_Status_S_InterruptStatsPrint(hXvdStatus);

   BDBG_LEAVE(BXVD_Status_StopDecode);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXVD_Status_Print(
   BXVD_StatusHandle hXvdStatus
   )
{
#if BDBG_DEBUG_BUILD
   uint32_t i;
   BXVD_ChannelStatus *pstChannelStatus = NULL;

   BDBG_ASSERT(hXvdStatus);
   
   BDBG_MSG(("Begin Printing Status"));   

   for (i = 0; i < BXVD_MAX_VIDEO_CHANNELS; i++)
   {
      pstChannelStatus = &hXvdStatus->astChannelStatus[i];
      if (hXvdStatus->auiOpenChannelCount[i])
      {
         BDBG_MSG(("Channel[%d][%02d]: uiOpenChannelCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, hXvdStatus->auiOpenChannelCount[i]));
         BDBG_MSG(("Channel[%d][%02d]: uiCloseChannelCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, hXvdStatus->auiCloseChannelCount[i]));
         BDBG_MSG(("Channel[%d][%02d]: uiStartDecodeCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, hXvdStatus->auiStartDecodeCount[i]));
         BDBG_MSG(("Channel[%d][%02d]: uiStopDecodeCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, hXvdStatus->auiStopDecodeCount[i]));
         
         BDBG_MSG(("Channel[%d][%02d]: ulUnderflowCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->ulUnderflowCount));
         BDBG_MSG(("Channel[%d][%02d]: ulPictureDeliveryCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->ulPictureDeliveryCount));
         BDBG_MSG(("Channel[%d][%02d]: uiDisplayManagerDroppedCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiDisplayManagerDroppedCount));
         BDBG_MSG(("Channel[%d][%02d]: uiDecoderDroppedCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiDecoderDroppedCount));
         BDBG_MSG(("Channel[%d][%02d]: uiDisplayedParityFailureCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiDisplayedParityFailureCount));
         BDBG_MSG(("Channel[%d][%02d]: uiDisplayedCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiDisplayedCount));
         BDBG_MSG(("Channel[%d][%02d]: uiPicturesDecodedCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiPicturesDecodedCount));
         BDBG_MSG(("Channel[%d][%02d]: uiVsyncUnderflowCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiVsyncUnderflowCount));
         BDBG_MSG(("Channel[%d][%02d]: uiDecodeErrorCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiDecodeErrorCount));
         BDBG_MSG(("Channel[%d][%02d]: uiVsyncCount = %d", hXvdStatus->hXvd->uDecoderInstance, i, pstChannelStatus->uiVsyncCount));         
      }
   }

   BDBG_MSG(("End Printing Status"));
#else
   BSTD_UNUSED(hXvdStatus);
#endif
   return BERR_TRACE(BERR_SUCCESS);
}
