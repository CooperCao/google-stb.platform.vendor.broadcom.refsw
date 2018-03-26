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
 *****************************************************************************/

#include "bmuxlib_asp_types.h"

#include "bmuxlib_asp_source_list_priv.h"

#include "bmuxlib_asp_ts_source_av_priv.h"
#include "bmuxlib_asp_ts_source_sys_priv.h"
#include "bmuxlib_asp_ts_source_user_priv.h"
#include "bmuxlib_asp_ts_memory_priv.h"

BDBG_MODULE(BMUXLIB_ASP_TS_CHANNEL);

static const BMUXlib_Source_List_Dispatch_t astSourceListDispatch[BMUXLIB_ASP_SOURCE_TYPE_MAX] =
{
   { (BMUXLIB_ASP_P_Source_Entry_Start) BMUXlib_ASP_TS_P_Source_AV_Start, (BMUXLIB_ASP_P_Source_Entry_Reconcile) BMUXlib_ASP_TS_P_Source_AV_Reconcile, (BMUXLIB_ASP_P_Source_Entry_DoMux) BMUXlib_ASP_TS_P_Source_AV_DoMux }, /* BMUXLIB_ASP_SOURCE_TYPE_INDEXED */
   { (BMUXLIB_ASP_P_Source_Entry_Start) BMUXib_ASP_TS_P_Source_Sys_Start, (BMUXLIB_ASP_P_Source_Entry_Reconcile) BMUXlib_ASP_TS_P_Source_Sys_Reconcile, (BMUXLIB_ASP_P_Source_Entry_DoMux) BMUXlib_ASP_TS_P_Source_Sys_DoMux }, /* BMUXLIB_ASP_SOURCE_TYPE_SYSTEMDATA */
   { (BMUXLIB_ASP_P_Source_Entry_Start) BMUXlib_ASP_P_Source_User_Start, (BMUXLIB_ASP_P_Source_Entry_Reconcile) BMUXlib_ASP_P_Source_User_Reconcile, (BMUXLIB_ASP_P_Source_Entry_DoMux) BMUXlib_ASP_P_Source_User_DoMux }, /* BMUXLIB_ASP_SOURCE_TYPE_USERDATA */
   { NULL, NULL, NULL }, /* BMUXLIB_ASP_SOURCE_TYPE_UNKNOWN */
};

void
BMUXlib_ASP_TS_P_Channel_Start(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle
   )
{
   /* Start each input */
   BMUXlib_Source_List_Handle hSourceListHandle = &hChannelHandle->stContext.stSourceList;
   uint32_t i;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Channel_Start );

   /* Setup Memory Context */
   hChannelHandle->stContext.stMemory.pstMemoryInterface = &hChannelHandle->stStartSettings.stMemory;
   hChannelHandle->stContext.stMemory.pstDMAInterface = &hChannelHandle->stStartSettings.stDMA;
   BMUXlib_ASP_TS_P_Memory_Packet_Start( &hChannelHandle->stContext.stMemory );

   /* Setup Output Context */
   hChannelHandle->stContext.stOutput.pstOutputInterface = &hChannelHandle->stStartSettings.stOutput;

   /* Setup Input List */
   hChannelHandle->stContext.stSourceList.uiNumInputs = 0;

   if ( BMUXLIB_ASP_TS_MAX_AV_SOURCE < hChannelHandle->stStartSettings.uiNumValidSource )
   {
      BDBG_ERR(("Too many AV Sources"));
      return;
   }

   for ( i = 0; i < hChannelHandle->stStartSettings.uiNumValidSource; i++ )
   {
      BMUXlib_Source_Entry_Context_t *pstSourceMetadata = &hChannelHandle->stContext.stSourceList.stInput[hChannelHandle->stContext.stSourceList.uiNumInputs];
      pstSourceMetadata->bActive = TRUE;
      pstSourceMetadata->eSourceType = BMUXLIB_ASP_SOURCE_TYPE_INDEXED;
      pstSourceMetadata->uiIndex = i;
      pstSourceMetadata->stTiming.bValid = FALSE;
      hChannelHandle->stContext.stSourceList.uiNumInputs++;
   }

#if BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE
   for ( i = 0; i < (uint32_t) ( hChannelHandle->stStartSettings.stUserData.uiNumValidUserData ); i++ )
   {
      BMUXlib_Source_Entry_Context_t *pstSourceMetadata = &hChannelHandle->stContext.stSourceList.stInput[hChannelHandle->stContext.stSourceList.uiNumInputs];
      pstSourceMetadata->bActive = TRUE;
      pstSourceMetadata->eSourceType = BMUXLIB_ASP_SOURCE_TYPE_USERDATA;
      pstSourceMetadata->uiIndex = i;
      pstSourceMetadata->stTiming.bValid = FALSE;
      hChannelHandle->stContext.stSourceList.uiNumInputs++;
   }
#endif

#if BMUXLIB_ASP_TS_MAX_SYSTEMDATA_SOURCE
   /* Add the system data channel at the end of the list because it relies on data from the A/V sources */
   {
      BMUXlib_Source_Entry_Context_t *pstSourceMetadata = &hChannelHandle->stContext.stSourceList.stInput[hChannelHandle->stContext.stSourceList.uiNumInputs];
      pstSourceMetadata->bActive = TRUE;
      pstSourceMetadata->eSourceType = BMUXLIB_ASP_SOURCE_TYPE_SYSTEMDATA;
      pstSourceMetadata->stTiming.bValid = FALSE;
      hChannelHandle->stContext.stSourceList.uiNumInputs++;
   }
#endif

   /* Setup Source Context(s) */
   for ( i = 0; i < hSourceListHandle->uiNumInputs; i++ )
   {
      BMUXlib_Source_Entry_Handle hSourceEntryHandle = &hChannelHandle->stContext.stSourceList.stInput[i];
      astSourceListDispatch[hSourceEntryHandle->eSourceType].fStart( hChannelHandle, hSourceEntryHandle );
   }


   BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE(hChannelHandle, BMUXLIB_ASP_CHANNEL_STATE_STARTED);

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Channel_Start );
}

bool_t
BMUXlib_ASP_TS_P_Channel_Reconcile(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle
   )
{
   uint32_t i;
   bool_t bAllActiveInputsHaveData = TRUE;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Channel_Reconcile );

   switch ( BMUXLIB_ASP_TS_P_CHANNEL_GET_STATE( hChannelHandle ) )
   {
      case BMUXLIB_ASP_CHANNEL_STATE_STARTED:
      {
         BMUXlib_ASP_P_Source_List_Reset( &hChannelHandle->stContext.stSourceList );

         /* Check if all A/V inputs are done */
         {
            bool_t bAVInputStillActive = FALSE;

            for ( i = 0; i < hChannelHandle->stContext.stSourceList.uiNumInputs; i++ )
            {
               BMUXlib_Source_Entry_Handle hSourceEntryHandle = &hChannelHandle->stContext.stSourceList.stInput[i];

               if ( BMUXLIB_ASP_SOURCE_TYPE_INDEXED == hSourceEntryHandle->eSourceType )
               {
                  bAVInputStillActive |= hSourceEntryHandle->bActive;
               }
            }

            if ( FALSE == bAVInputStillActive )
            {
               BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE(hChannelHandle, BMUXLIB_ASP_CHANNEL_STATE_FINISHING);
               break;
            }
         }

         for ( i = 0; i < hChannelHandle->stContext.stSourceList.uiNumInputs; i++ )
         {
            BMUXlib_Source_Entry_Handle hSourceEntryHandle = &hChannelHandle->stContext.stSourceList.stInput[i];
            bool_t bInputHasData = FALSE;

            if ( TRUE == hSourceEntryHandle->bActive )
            {
               if ( BMUXLIB_ASP_SOURCE_TYPE_SYSTEMDATA == hSourceEntryHandle->eSourceType )
               {
                  if ( TRUE == BMUXlib_ASP_P_Source_List_IsEmpty( &hChannelHandle->stContext.stSourceList ) )
                  {
                     BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE( hChannelHandle, BMUXLIB_ASP_CHANNEL_STATE_FINISHED );
                     bAllActiveInputsHaveData = FALSE;
                     break;
                  }
                  else if ( FALSE == bAllActiveInputsHaveData )
                  {
                     break;
                  }
               }
               bInputHasData = astSourceListDispatch[hSourceEntryHandle->eSourceType].fReconcile( hChannelHandle, hSourceEntryHandle );

               /* For now, ignore is no data is available on userdata[0] (system data) */
               /* TODO: Add NULL stuffing in HOST userdata insertion code */
               if ( ( FALSE == bInputHasData )
                    && ( BMUXLIB_ASP_SOURCE_TYPE_USERDATA == hSourceEntryHandle->eSourceType )
                    && ( 0 == hSourceEntryHandle->uiIndex ) ) continue;

               bAllActiveInputsHaveData &= bInputHasData;

               /* Add this source to the sorted list */
               BMUXlib_ASP_P_Source_List_Insert( &hChannelHandle->stContext.stSourceList, hSourceEntryHandle );
            }
         }
         break;
      }

      case BMUXLIB_ASP_CHANNEL_STATE_FINISHING:
         if ( hChannelHandle->stContext.stOutput.uiRead == hChannelHandle->stContext.stOutput.uiWrite )
         {
            BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE(hChannelHandle, BMUXLIB_ASP_CHANNEL_STATE_FINISHED);
         }
         break;

      default:
         break;
   }

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Channel_Reconcile );
   return bAllActiveInputsHaveData;
}

/************************************************************************
 * Function: BMUXLIB_ASP_P_MuxManagerChannelDoMux
 * Description:
 *   Main work of the ASP muxer is done here
 *
 * Parameters:
 *
 * Returns:
 *     none
 *
 ************************************************************************/

void
BMUXlib_ASP_TS_P_Channel_DoMux(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle
   )
{
   BDBG_ENTER( BMUXlib_ASP_TS_P_Channel_DoMux );

   {
      uint32_t uiEndESCR = 0;
      bool_t bEndESCRInitialized = FALSE;
      bool_t bActiveInputIsEmpty = FALSE;

      switch ( BMUXLIB_ASP_TS_P_CHANNEL_GET_STATE( hChannelHandle ) )
      {
         case BMUXLIB_ASP_CHANNEL_STATE_STARTED:
            /* Process the descriptors that are completed and reclaim any resources */
            BMUXlib_ASP_P_Output_Descriptor_ProcessCompleted( &hChannelHandle->stContext.stOutput );

            if ( FALSE == BMUXlib_ASP_TS_P_Channel_Reconcile( hChannelHandle ) )
            {
              /* Not all active inputs has data, yet */
              return;
            }

            while (1)
            {
               BMUXlib_Source_Entry_Handle hSourceEntry = NULL;

               /* Get the first source entry on the list */
               BMUXlib_ASP_P_Source_List_Pop( &hChannelHandle->stContext.stSourceList, &hSourceEntry );
               if ( NULL == hSourceEntry )
               {
                  /* The list is empty, so we're done in this MSP */
                  BDBG_MSG(("Source List Empty"));
                  break;
               }

               /* If we are on the source input we wait need to wait until the next MSP if:
                *    1) There are no more A/V inputs left
                *    2) One of the A/V inputs has gone empty
                */
               if ( ( BMUXLIB_ASP_SOURCE_TYPE_SYSTEMDATA == hSourceEntry->eSourceType )
                    && ( ( TRUE == BMUXlib_ASP_P_Source_List_IsEmpty( &hChannelHandle->stContext.stSourceList ) )
                         || ( TRUE == bActiveInputIsEmpty ) ) )
               {
                  break;
               }

               /* Calculate the end ESCR */
               {
                  uint32_t uiNewEndESCR = uiEndESCR;

                  /* We need to peek at the next input to calculate the endESCR */
                  BMUXlib_Source_Entry_Handle hNextSourceEntry = NULL;

                  BMUXlib_ASP_P_Source_List_Peek( &hChannelHandle->stContext.stSourceList, &hNextSourceEntry );

                  if ( NULL == hNextSourceEntry )
                  {
                     /* There IS NOT a next source, so we can process the rest of the current source */
                     if ( FALSE == bEndESCRInitialized )
                     {
                        uiNewEndESCR = hSourceEntry->stTiming.uiCurrentESCR + 0x7FFFFFFF;
                     }
                  }
                  else
                  {
                     /* There IS a next source, so we can process until the start of the next source */
                     uiNewEndESCR = hNextSourceEntry->stTiming.uiCurrentESCR;
                  }

                  if ( ( TRUE == bEndESCRInitialized ) && ( TRUE == bActiveInputIsEmpty ) )
                  {
                     /* Pick the smallest END ESCR to that we don't mux more than we can*/
                     if ( ( (int32_t) ( uiNewEndESCR - uiEndESCR ) ) < 0 )
                     {
                        uiEndESCR = uiNewEndESCR;
                     }
                  }
                  else
                  {
                     uiEndESCR = uiNewEndESCR;
                  }
                  bEndESCRInitialized = TRUE;

                  BDBG_MSG(("Source[%d][%d] escr: %08x endESCR: %08x pts:"BDBG_UINT64_FMT" - Process",
                     hSourceEntry->eSourceType,
                     hSourceEntry->uiIndex,
                     hSourceEntry->stTiming.uiCurrentESCR,
                     uiEndESCR,
                     BDBG_UINT64_ARG(hSourceEntry->stTiming.uiCurrentPTS)
                     ));
               }

               /* Process the current source */
               if ( TRUE == astSourceListDispatch[hSourceEntry->eSourceType].fDoMux( hChannelHandle, hSourceEntry, uiEndESCR ) )
               {
                  BDBG_MSG(("Source[%d][%d] escr: %08x endESCR: %08x pts:"BDBG_UINT64_FMT" - ReQueue",
                     hSourceEntry->eSourceType,
                     hSourceEntry->uiIndex,
                     hSourceEntry->stTiming.uiCurrentESCR,
                     uiEndESCR,
                     BDBG_UINT64_ARG(hSourceEntry->stTiming.uiCurrentPTS)
                     ));

                  /* There's more data in the current source, so re-queue it into the source list */
                  BMUXlib_ASP_P_Source_List_Insert( &hChannelHandle->stContext.stSourceList, hSourceEntry );
               }
               else
               {
                  /* There's no data left to process in the current source */
                  if ( TRUE == hSourceEntry->bActive )
                  {
                     BDBG_MSG(("Source[%d][%d] escr: %08x endESCR: %08x pts:"BDBG_UINT64_FMT" - Empty",
                        hSourceEntry->eSourceType,
                        hSourceEntry->uiIndex,
                        hSourceEntry->stTiming.uiCurrentESCR,
                        uiEndESCR,
                        BDBG_UINT64_ARG(hSourceEntry->stTiming.uiCurrentPTS)
                        ));

                     if (!( ( BMUXLIB_ASP_SOURCE_TYPE_USERDATA == hSourceEntry->eSourceType ) && ( 0 == hSourceEntry->uiIndex ) ) ) /* If it's not the system data userdata source (index 0) */
                     {
                        /* The current source is still active, so we can't proceed any further in this MSP */
                        bActiveInputIsEmpty = TRUE;
                     }
                  }
                  else
                  {
                     /* The current source is no longer active ((e.g. EOS was seen),
                      * so process the next input(s), if any */
                     BDBG_MSG(("Source[%d][%d] escr: %08x endESCR: %08x pts:"BDBG_UINT64_FMT" - Finished",
                        hSourceEntry->eSourceType,
                        hSourceEntry->uiIndex,
                        hSourceEntry->stTiming.uiCurrentESCR,
                        uiEndESCR,
                        BDBG_UINT64_ARG(hSourceEntry->stTiming.uiCurrentPTS)
                        ));

                     /* We're done with an input, so let's recalculate the endESCR in the next iteration */
                     bEndESCRInitialized = FALSE;
                  }
               }

               uiEndESCR = hSourceEntry->stTiming.uiCurrentESCR;
            }

            /* TODO: Move this */
            /* DMA user data read pointer array to DRAM */
            if ( NULL != hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset )
            {
               if ( TRUE == hChannelHandle->stStartSettings.stDMA.fIdle(
                     hChannelHandle->stStartSettings.stDMA.pvContext,
                     &hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset
                     ) )
               {
                  hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset = NULL;
               }
            }

            if ( TRUE == hChannelHandle->stContext.stUserDataCommon.bReadOffsetUpdated )
            {
               if ( NULL == hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset )
               {
                  /* DMA current read offsets to DRAM */
                  hChannelHandle->stStartSettings.stDMA.fDccm2Dram(
                    hChannelHandle->stStartSettings.stDMA.pvContext,
                    hChannelHandle->stStartSettings.stUserData.uiOffset + ( offsetof( BMUXlib_ASP_TS_Userdata_Host_Interface_t, uiReadOffset ) ),
                    hChannelHandle->stContext.stUserDataCommon.uiReadOffset,
                    sizeof( uint32_t ) * hChannelHandle->stStartSettings.stUserData.uiNumValidUserData,
                    &hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset
                  );

                  if ( NULL != hChannelHandle->stContext.stUserDataCommon.pDMATokenReadOffset )
                  {
                     hChannelHandle->stContext.stUserDataCommon.bReadOffsetUpdated = FALSE;
                  }
               }
            }

         break;

         case BMUXLIB_ASP_CHANNEL_STATE_FINISHING:
            /* Process the descriptors that are completed and reclaim any resources */
            BMUXlib_ASP_P_Output_Descriptor_ProcessCompleted( &hChannelHandle->stContext.stOutput );

            if ( FALSE == BMUXlib_ASP_TS_P_Channel_Reconcile( hChannelHandle ) )
            {
              /* Not all active inputs has data, yet */
              return;
            }

         default:
            break;
      }
   }

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Channel_DoMux );
}
