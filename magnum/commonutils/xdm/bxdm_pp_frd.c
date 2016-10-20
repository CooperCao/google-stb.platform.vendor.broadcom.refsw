/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"                /* Dbglib */

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_frd.h"


BDBG_MODULE(BXDM_PPFRD);
BDBG_FILE_MODULE(BXDM_PPFRD);

static void BXDM_PPFRD_S_AddDeltaPTS_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_PPFP_P_DataType *pstDeltaPTS
   )
{

   if ( hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount < BXDM_PPFRD_P_MAX_DELTAPTS_SAMPLES )
   {
      /* Increment the PTS count */
      hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount++;
   }
   else
   {
      /* Subtract old entry from the running sum */
      BXDM_PPFP_P_FixPtSub_isr(
         &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
         &hXdmPP->stDMState.stDecode.stFRDStats.astDeltaPTS[hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex],
         &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum
         );
   }

#if BXDM_DEBUG_LOW_PRIORITY
   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] BXDM_PPFRD_S_AddDeltaPTS_isr(%d.%d)[%d]",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pstDeltaPTS->uiWhole,
                              pstDeltaPTS->uiFractional,
                              hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount ));
#endif

   /* Replace the old entry with the new entry */
   hXdmPP->stDMState.stDecode.stFRDStats.astDeltaPTS[hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex] = *pstDeltaPTS;

   /* Update running sum */
   BXDM_PPFP_P_FixPtAdd_isr(
      &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
      &hXdmPP->stDMState.stDecode.stFRDStats.astDeltaPTS[hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex],
      &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum);

   /* Increment PTS index */
   hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex++;
   if ( hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex >= BXDM_PPFRD_P_MAX_DELTAPTS_SAMPLES )
   {
      hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSIndex = 0;
   }
}

static void BXDM_PPFRD_S_AddNumElements_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   const uint32_t uiNumElements
   )
{
   if ( hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCount < BXDM_PPFRD_P_MAX_NUMELEMENTS_SAMPLES )
   {
      /* Increment the PTS count */
      hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCount++;
   }
   else
   {
      /* Subtract old entry from the running sum */
      hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsRunningSum -= hXdmPP->stDMState.stDecode.stFRDStats.auiNumElements[hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex];
   }

#if BXDM_DEBUG_LOW_PRIORITY
   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] BXDM_PPFRD_S_AddNumElements_isr(%d)[%d]",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              uiNumElements,
                              hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCount));
#endif

   /* Replace the old entry with the new entry */
   hXdmPP->stDMState.stDecode.stFRDStats.auiNumElements[hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex] = uiNumElements;

   /* Update running sum */
   hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsRunningSum += hXdmPP->stDMState.stDecode.stFRDStats.auiNumElements[hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex];

   /* Increment PTS index */
   hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex++;
   if ( hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex >= BXDM_PPFRD_P_MAX_NUMELEMENTS_SAMPLES )
   {
      hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsIndex = 0;
   }
}

#define BXDM_PPFRD_P_DISCONTINUITY_DELTA_PTS_THRESHOLD 45000

BERR_Code BXDM_PPFRD_P_AddPTS_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   uint32_t uiPTS,
   bool bPTSValid,
   uint32_t uiNumElements
   )
{
   BXDM_PPFP_P_DataType stDeltaPTS;
   uint32_t i;

#if BXDM_DEBUG_LOW_PRIORITY
   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] BXDM_PPFRD_P_AddPTS_isr(%d, %d)",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              uiPTS,
                              bPTSValid ));
#endif

   if ( true == bPTSValid )
   {
      /* We have a valid PTS */
      if ( true == hXdmPP->stDMState.stDecode.stFRDStats.bLastPTSValid )
      {
         /* We have another valid PTS, so we can compute and insert
          * the delta PTS value(s) into the dPTS queue */
         if ( uiPTS > hXdmPP->stDMState.stDecode.stFRDStats.uiLastPTS )
         {
            /* SW7425-4389: if using the SW STC to run in reverse, the PTS values
             * should be running in reverse as well.  Flip the subtraction.
             */
            if ( true == pLocalState->bUsingSwStcToRunInReverse )
            {
               stDeltaPTS.uiWhole = hXdmPP->stDMState.stDecode.stFRDStats.uiLastPTS - uiPTS;
            }
            else
            {
            stDeltaPTS.uiWhole = uiPTS - hXdmPP->stDMState.stDecode.stFRDStats.uiLastPTS;
            }

            stDeltaPTS.uiFractional = 0;
            BXDM_PPFP_P_FixPtDiv_isr(&stDeltaPTS,
                                 hXdmPP->stDMState.stDecode.stFRDStats.uiPicturesSinceLastValidPTS,
                                 &stDeltaPTS);

            /* Handle PTS discontinuity scenario */
            if ( stDeltaPTS.uiWhole < BXDM_PPFRD_P_DISCONTINUITY_DELTA_PTS_THRESHOLD )
            {
               for (i = 0; i < hXdmPP->stDMState.stDecode.stFRDStats.uiPicturesSinceLastValidPTS; i++)
               {
                  /* Add new deltaPTS to queue.  We add one deltaPTS entry
                   * per picture. */
                  BXDM_PPFRD_S_AddDeltaPTS_isr(
                     hXdmPP,
                     &stDeltaPTS);
               }
            }
            else
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] Warning, PTS discontinuity detected (dPTS=%d).  Ignoring deltaPTS during discontinuity.",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          stDeltaPTS.uiWhole ));
            }
         }
         else
         {
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] Warning, PTS wrap/discontinuity detected (%08x -> %08x).  Ignoring deltaPTS during wrap/discontinuity.",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          hXdmPP->stDMState.stDecode.stFRDStats.uiLastPTS,
                                          uiPTS ));
         }
      }

      hXdmPP->stDMState.stDecode.stFRDStats.uiLastPTS = uiPTS;
      hXdmPP->stDMState.stDecode.stFRDStats.bLastPTSValid = true;
      hXdmPP->stDMState.stDecode.stFRDStats.uiPicturesSinceLastValidPTS = uiNumElements;
   }
   else
   {
      hXdmPP->stDMState.stDecode.stFRDStats.uiPicturesSinceLastValidPTS += uiNumElements;
   }

   /* Add new deltaPTS to queue.  We add one deltaPTS entry
    * per picture. */
   BXDM_PPFRD_S_AddNumElements_isr(
      hXdmPP,
      uiNumElements);

   return BERR_SUCCESS;
}

typedef struct BXDM_PPFRD_P_DeltaPtsToFrameRateMap
{
   BXDM_PPFP_P_DataType stDeltaPTS;
   BAVC_FrameRateCode eFrameRate;
} BXDM_PPFRD_P_DeltaPtsToFrameRateMap;

/* The table below is used to map the average delta PTS to a frame rate.
 * The range for a given frame rate is:
 * - low end:  45000/(frame_rate) - 45 (1 millisecond)
 * - high end: the preceding frame's rates low end
 *
 * The exceptions to this are the non-whole number rates; 7.493, 14.985, 23.976,
 * 29.97, 59.95.  In these cases, the low end is just a few ticks less
 * than 45000/(frame_rate)
 */
static const BXDM_PPFRD_P_DeltaPtsToFrameRateMap sFrameRateLUT[] =
{
   { { 6051, 1 }, BAVC_FrameRateCode_eUnknown },   /* 6051 = (45000/7.493)    + 45     */
   { { 6005, 1 }, BAVC_FrameRateCode_e7_493 },     /* 6005 = (45000/(7500/1001) - 1    */
   { { 5955, 1 }, BAVC_FrameRateCode_e7_5, },      /* 5955 = (45000/7.5)      - 45     */ /*SWSTB-1401:*/
   { { 4503, 1 }, BAVC_FrameRateCode_e9_99, },     /* 4503 = (45000/9.99)     - 1      */ /*SWSTB-1401:*/
   { { 4455, 1 }, BAVC_FrameRateCode_e10 },        /* 4455 = (45000/10)       - 45     */
   { { 3755, 1 }, BAVC_FrameRateCode_e11_988 },    /* 3755 = (45000/11.98     - 1      */ /*SWSTB-1401:*/
   { { 3705, 1 }, BAVC_FrameRateCode_e12 },        /* 3705 = (45000/12)       - 45     */ /*SWSTB-1401:*/
   { { 3555, 1 }, BAVC_FrameRateCode_e12_5 },      /* 3555 = (45000/12.5)     - 45     */ /*SW7584-331:*/
   { { 3002, 1 }, BAVC_FrameRateCode_e14_985 },    /* 3002 = (45000/14.958)   - 1      */
   { { 2955, 1 }, BAVC_FrameRateCode_e15 },        /* 2955 = (45000/15)       - 45     */
   { { 2251, 1 }, BAVC_FrameRateCode_e19_98 },     /* 2325 = (45000/19.98)    - 1      */ /*SWSTB-378:*/
   { { 2205, 1 }, BAVC_FrameRateCode_e20 },        /* 2205 = (45000/20)       - 45     */
   { { 1875, 1 }, BAVC_FrameRateCode_e23_976 },    /* 1875 = (45000/23.976)   - ~2     */
   { { 1830, 1 }, BAVC_FrameRateCode_e24 },        /* 1830 = (45000/24)       - 45     */
   { { 1755, 1 }, BAVC_FrameRateCode_e25 },        /* 1755 = (45000/25)       - 45     */
   { { 1500, 1 }, BAVC_FrameRateCode_e29_97 },     /* 1500 = (45000/29.97)    - ~1.5   */
   { { 1455, 1 }, BAVC_FrameRateCode_e30 },        /* 1455 = (45000/30)       - 45     */
   { {  855, 1 }, BAVC_FrameRateCode_e50 },        /*  855 = (45000/50)       - 45     */
   { {  750, 1 }, BAVC_FrameRateCode_e59_94 },     /*  750 = (45000/59.94)    - ~1     */
   { {  705, 1 }, BAVC_FrameRateCode_e60 },        /*  705 = (45000/60)       - 45     */
   { {  405, 1 }, BAVC_FrameRateCode_e100 },       /*  405 = (45000/100)       - 45     */
   { {  374, 1 }, BAVC_FrameRateCode_e119_88 },    /*  374 = (45000/119.88)     - 1     */
   { {  330, 1 }, BAVC_FrameRateCode_e120 },       /*  330 = (45000/120)       - 45     */
   { {    0, 0 }, BAVC_FrameRateCode_eUnknown }
};

static const uint32_t sFrameRateStabiltyLUT[BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
   30, /* BAVC_FrameRateCode_eUnknown */
   24, /* BAVC_FrameRateCode_e23_976 */
   24, /* BAVC_FrameRateCode_e24 */
   25, /* BAVC_FrameRateCode_e25 */
   30, /* BAVC_FrameRateCode_e29_97 */
   30, /* BAVC_FrameRateCode_e30 */
   50, /* BAVC_FrameRateCode_e50 */
   60, /* BAVC_FrameRateCode_e59_94 */
   60, /* BAVC_FrameRateCode_e60 */
   15, /* BAVC_FrameRateCode_e14_985 */
   8,  /* BAVC_FrameRateCode_e7_493 */
   10, /* BAVC_FrameRateCode_e10 */
   15, /* BAVC_FrameRateCode_e15 */
   20, /* BAVC_FrameRateCode_e20 */
   13, /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
  100, /* BAVC_FrameRateCode_e100 */
  120, /* BAVC_FrameRateCode_e119.88 */
  120, /* BAVC_FrameRateCode_e120 */
   20, /* SWSTB-378: add support for BAVC_FrameRateCode_e19_98 */
    8, /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
   12, /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
   12, /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
   10, /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
};

BERR_Code BXDM_PPFRD_P_GetFrameRate_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_ClockRate eClockRate,
   BXDM_PictureProvider_FrameRateDetectionMode eFrameRateDetectionMode,
   BAVC_FrameRateCode *peFrameRate
   )
{
   BXDM_PPFP_P_DataType stAverageDeltaPTS;
   uint32_t uiFrameRateIndex = 0;

   *peFrameRate = BAVC_FrameRateCode_eUnknown;
   stAverageDeltaPTS.uiWhole = 0;
   stAverageDeltaPTS.uiFractional = 0;

   if ( BXDM_PictureProvider_FrameRateDetectionMode_eOff != eFrameRateDetectionMode )
   {
      /* Calculate the average deltaPTS */
      if ( hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount > 0 )
      {
         if ( BXDM_PictureProvider_P_ClockRate_eDirecTV == eClockRate )
         {
            /* Scale down by 600 if we have a DirecTV clock rate */
            BXDM_PPFP_P_FixPtDiv_isr(
               &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
               hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount*600,
               &stAverageDeltaPTS
               );
         }
         else
         {
            BXDM_PPFP_P_FixPtDiv_isr(
               &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
               hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
               &stAverageDeltaPTS
               );
         }

         /* The average delta PTS we've computed so far is the avg dPTS
          * per _field_.  We need to convert it to the avg dPTS per
          * _frame_.  We assume that 2 interlaced fields equals 1 frame.
          * So, we calculate the following:
          *
          * (avg dPTS)/picture = (avg dPTS/field)*(floor(avg fields/picture))
          *
          * This equation accounts for 3:2 pulldown content which has 2.5
          * fields/picture.
          */
         BXDM_PPFP_P_FixPtMult_isr(
            &stAverageDeltaPTS,
            hXdmPP->stDMState.stDecode.stFRDStats.uiNumElementsRunningSum/hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCount,
            &stAverageDeltaPTS);

         /* Map the average deltaPTS to a known frame rate */
         while ( ( stAverageDeltaPTS.uiWhole < sFrameRateLUT[uiFrameRateIndex].stDeltaPTS.uiWhole )
                 || ( ( stAverageDeltaPTS.uiWhole == sFrameRateLUT[uiFrameRateIndex].stDeltaPTS.uiWhole )
                      && ( stAverageDeltaPTS.uiFractional < sFrameRateLUT[uiFrameRateIndex].stDeltaPTS.uiFractional ) ) )
         {
            uiFrameRateIndex++;
         }

         /* Determine which frame rate type to return */
         switch ( eFrameRateDetectionMode )
         {
            case BXDM_PictureProvider_FrameRateDetectionMode_eStable:
            {
               /* Keep track of how many pictures the frame rate is stable for */
               if (sFrameRateLUT[uiFrameRateIndex].eFrameRate == hXdmPP->stDMState.stDecode.stFRDStats.eLastCalculatedFrameRate)
               {
                  hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCalculatedFrameRateWasStable++;
               }
               else
               {
                  hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCalculatedFrameRateWasStable = 0;
               }
               hXdmPP->stDMState.stDecode.stFRDStats.eLastCalculatedFrameRate = sFrameRateLUT[uiFrameRateIndex].eFrameRate;

               if ( hXdmPP->stDMState.stDecode.stFRDStats.uiNumPicturesCalculatedFrameRateWasStable >= sFrameRateStabiltyLUT[sFrameRateLUT[uiFrameRateIndex].eFrameRate] )
               {
                  hXdmPP->stDMState.stDecode.stFRDStats.eLastReportedStableFrameRate = sFrameRateLUT[uiFrameRateIndex].eFrameRate;
               }

               *peFrameRate = hXdmPP->stDMState.stDecode.stFRDStats.eLastReportedStableFrameRate;
            }
            break;

            case BXDM_PictureProvider_FrameRateDetectionMode_eFast:
               *peFrameRate = sFrameRateLUT[uiFrameRateIndex].eFrameRate;
               break;

            default:
               *peFrameRate = BAVC_FrameRateCode_eUnknown;
         }
      }
      else
      {
         /* We don't have enough data to compute a frame rate */
         *peFrameRate = BAVC_FrameRateCode_eUnknown;
      }
   }

#if BXDM_DEBUG_LOW_PRIORITY
   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eFRD, "%x:[%02x.xxx] BXDM_PPFRD_P_GetFrameRate_isr((%d.%d)/%d = %d.%d --> %d [%d])",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum.uiWhole,
                              hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum.uiFractional,
                              hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
                              stAverageDeltaPTS.uiWhole,
                              stAverageDeltaPTS.uiFractional,
                              *peFrameRate,
                              eFrameRateDetectionMode ));
#endif

   return BERR_SUCCESS;
}
