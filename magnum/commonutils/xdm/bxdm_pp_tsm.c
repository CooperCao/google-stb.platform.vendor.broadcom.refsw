/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bxdm_pp_qm.h"
#include "bxdm_pp_tsm.h"
#include "bxdm_pp_vtsm.h"
#include "bxdm_pp_clip.h"
#include "bxdm_pp_fic.h"
#include "bxdm_pp_callback_priv.h"
#include "bxdm_pp_jrc.h"

BDBG_MODULE(BXDM_PPTSM);
BDBG_FILE_MODULE(BXDM_PPTSM);
BDBG_FILE_MODULE(BXDM_PPQM);
BDBG_FILE_MODULE(BXDM_PPVTSM);

/*******************************************************************************
 *
 * Local constant definitions.
 *
 ******************************************************************************/

 /* SW7445-608: treat content within a range of heights as 1080. */
 #define BXDM_PPTSM_P_1080_LOWER_LIMIT 736
 #define BXDM_PPTSM_P_1080_UPPER_LIMIT 1088

/* Lookup Tables */

/* The following is a LUT for the delta PTS given a
 * BXDM_PictureProvider_P_ClockRate, BXDM_PictureProvider_P_PictureType, and BAVC_FrameRateCode.
 * It is calculated assuming the fractional component's denominator is
 * 65536 */
static const BXDM_PPFP_P_DataType sDeltaPtsLUT[BXDM_PictureProvider_P_ClockRate_eMax][BXDM_PictureProvider_P_PictureType_eMax][BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
   /* BXDM_PictureProvider_P_ClockRate_eMPEG2 (45Khz) */
   {
      /* BXDM_PictureProvider_P_PictureType_eProgressive */
      {
         { 1500,     0},   /* BAVC_FrameRateCode_eUnknown */
         { 1876, 57344},   /* BAVC_FrameRateCode_e23_976 */
         { 1875,     0},   /* BAVC_FrameRateCode_e24 */
         { 1800,     0},   /* BAVC_FrameRateCode_e25 */
         { 1501, 32768},   /* BAVC_FrameRateCode_e29_97 */
         { 1500,     0},   /* BAVC_FrameRateCode_e30 */
         {  900,     0},   /* BAVC_FrameRateCode_e50 */
         {  750, 49152},   /* BAVC_FrameRateCode_e59_94 */
         {  750,     0},   /* BAVC_FrameRateCode_e60 */
         { 3003,     0},   /* BAVC_FrameRateCode_e14_985 */
         { 6006,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 4500,     0},   /* BAVC_FrameRateCode_e10 */
         { 3000,     0},   /* BAVC_FrameRateCode_e15 */
         { 2250,     0},   /* BAVC_FrameRateCode_e20 */
         { 3600,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  450,     0},   /* BAVC_FrameRateCode_e100 */
         {  375, 24576},   /* BAVC_FrameRateCode_e119_88 */
         {  375,     0},   /* BAVC_FrameRateCode_e120 */
         { 2252, 16384},   /* BAVC_FrameRateCode_e19_98 */
         { 6000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 3750,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 3753, 49152},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 4504, 32768},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      },

      /* BXDM_PictureProvider_P_PictureType_eInterlaced */
      {
         {  750,     0},   /* BAVC_FrameRateCode_eUnknown */
         {  938, 28672},   /* BAVC_FrameRateCode_e23_976 */
         {  937, 32768},   /* BAVC_FrameRateCode_e24 */
         {  900,     0},   /* BAVC_FrameRateCode_e25 */
         {  750, 49152},   /* BAVC_FrameRateCode_e29_97 */
         {  750,     0},   /* BAVC_FrameRateCode_e30 */
         {  450,     0},   /* BAVC_FrameRateCode_e50 */
         {  375, 24576},   /* BAVC_FrameRateCode_e59_94 */
         {  375,     0},   /* BAVC_FrameRateCode_e60 */
         { 1501, 32768},   /* BAVC_FrameRateCode_e14_985 */
         { 3003,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 2250,     0},   /* BAVC_FrameRateCode_e10 */
         { 1500,     0},   /* BAVC_FrameRateCode_e15 */
         { 1125,     0},   /* BAVC_FrameRateCode_e20 */
         { 1800,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  225,     0},   /* BAVC_FrameRateCode_e100 */
         {  187, 45056},   /* BAVC_FrameRateCode_e119_88 */
         {  187, 32768},   /* BAVC_FrameRateCode_e120 */
         { 1126,  8192},   /* BAVC_FrameRateCode_e19_98 */
         { 3000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 1875,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 1876, 57344},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 2252, 16384},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      }
   },

   /* BXDM_PictureProvider_P_ClockRate_eDirecTV (27 Mhz) */
   {
      /* BXDM_PictureProvider_P_PictureType_eProgressive */
      {
         {  900000,     0},   /* BAVC_FrameRateCode_eUnknown */
         { 1126125,     0},   /* BAVC_FrameRateCode_e23_976 */
         { 1125000,     0},   /* BAVC_FrameRateCode_e24 */
         { 1080000,     0},   /* BAVC_FrameRateCode_e25 */
         {  900900,     0},   /* BAVC_FrameRateCode_e29_97 */
         {  900000,     0},   /* BAVC_FrameRateCode_e30 */
         {  540000,     0},   /* BAVC_FrameRateCode_e50 */
         {  450450,     0},   /* BAVC_FrameRateCode_e59_94 */
         {  450000,     0},   /* BAVC_FrameRateCode_e60 */
         { 1801800,     0},   /* BAVC_FrameRateCode_e14_985 */
         { 3603600,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 2700000,     0},   /* BAVC_FrameRateCode_e10 */
         { 1800000,     0},   /* BAVC_FrameRateCode_e15 */
         { 1350000,     0},   /* BAVC_FrameRateCode_e20 */
         { 2160000,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  270000,     0},   /* BAVC_FrameRateCode_e100 */
         {  225225,     0},   /* BAVC_FrameRateCode_e119_88 */
         {  225000,     0},   /* BAVC_FrameRateCode_e120 */
         { 1351350,     0},   /* BAVC_FrameRateCode_e19_98 */
         { 3600000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 2250000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 2252250,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 2702700,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      },

      /* BXDM_PictureProvider_P_PictureType_eInterlaced */
      {
         {  450000,     0},   /* BAVC_FrameRateCode_eUnknown */
         {  563062, 32768},   /* BAVC_FrameRateCode_e23_976 */
         {  562500,     0},   /* BAVC_FrameRateCode_e24 */
         {  540000,     0},   /* BAVC_FrameRateCode_e25 */
         {  450450,     0},   /* BAVC_FrameRateCode_e29_97 */
         {  450000,     0},   /* BAVC_FrameRateCode_e30 */
         {  270000,     0},   /* BAVC_FrameRateCode_e50 */
         {  225225,     0},   /* BAVC_FrameRateCode_e59_94 */
         {  225000,     0},   /* BAVC_FrameRateCode_e60 */
         {  900900,     0},   /* BAVC_FrameRateCode_e14_985 */
         { 1801800,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 1350000,     0},   /* BAVC_FrameRateCode_e10 */
         {  900000,     0},   /* BAVC_FrameRateCode_e15 */
         {  675000,     0},   /* BAVC_FrameRateCode_e20 */
         { 1080000,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  135000,     0},   /* BAVC_FrameRateCode_e100 */
         {  112612, 32768},   /* BAVC_FrameRateCode_e119_88 */
         {  112500,     0},   /* BAVC_FrameRateCode_e120 */
         {  675675,     0},   /* BAVC_FrameRateCode_e19_98 */
         { 1800000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 1125000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 1126125,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 1351350,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      }
   }
}; /* end of sDeltaPtsLUT[] */


/* SW7405-5231: only use whole values when in CRC mode.
 * There was an occasional dropped picture during the CRC testing for AVD.  These sometimes
 * occurred when the Delivery Queue under flowed and then two pictures were delivered on a
 * subsequent vsync.  Due to the handling of the fractional component of the virtual PTS
 * and STC, both pictures would pass the TSM test.  As a result, the first picture would
 * be dropped.  This problem goes away when whole numbers are used.   In CRC mode, the
 * delta PTS is used to interpolate the virtual STC.  The goal is to scan out a new picture
 * element on every vsync, the source and display rates don't really matter.  We probably
 * could just use '1' for the delta PTS for all content.
 */
static const BXDM_PPFP_P_DataType sCRCDeltaPtsLUT[BXDM_PictureProvider_P_PictureType_eMax][BXDM_PictureProvider_P_MAX_FRAMERATE] =
{
      /* BXDM_PictureProvider_P_PictureType_eProgressive */
      {
         { 1500,     0},   /* BAVC_FrameRateCode_eUnknown */
         { 1875,     0},   /* BAVC_FrameRateCode_e23_976 */
         { 1875,     0},   /* BAVC_FrameRateCode_e24 */
         { 1800,     0},   /* BAVC_FrameRateCode_e25 */
         { 1500,     0},   /* BAVC_FrameRateCode_e29_97 */
         { 1500,     0},   /* BAVC_FrameRateCode_e30 */
         {  900,     0},   /* BAVC_FrameRateCode_e50 */
         {  750,     0},   /* BAVC_FrameRateCode_e59_94 */
         {  750,     0},   /* BAVC_FrameRateCode_e60 */
         { 3003,     0},   /* BAVC_FrameRateCode_e14_985 */
         { 6006,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 4500,     0},   /* BAVC_FrameRateCode_e10 */
         { 3000,     0},   /* BAVC_FrameRateCode_e15 */
         { 2250,     0},   /* BAVC_FrameRateCode_e20 */
         { 3600,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  450,     0},   /* BAVC_FrameRateCode_e100 */
         {  375,     0},   /* BAVC_FrameRateCode_e119_88 */
         {  375,     0},   /* BAVC_FrameRateCode_e120 */
         { 2252,     0},   /* BAVC_FrameRateCode_e19_98 */
         { 6000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 3750,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 3753,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 4504,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      },

      /* BXDM_PictureProvider_P_PictureType_eInterlaced */
      {
         {  750,     0},   /* BAVC_FrameRateCode_eUnknown */
         {  938,     0},   /* BAVC_FrameRateCode_e23_976 */
         {  937,     0},   /* BAVC_FrameRateCode_e24 */
         {  900,     0},   /* BAVC_FrameRateCode_e25 */
         {  750,     0},   /* BAVC_FrameRateCode_e29_97 */
         {  750,     0},   /* BAVC_FrameRateCode_e30 */
         {  450,     0},   /* BAVC_FrameRateCode_e50 */
         {  375,     0},   /* BAVC_FrameRateCode_e59_94 */
         {  375,     0},   /* BAVC_FrameRateCode_e60 */
         { 1501,     0},   /* BAVC_FrameRateCode_e14_985 */
         { 3003,     0},   /* BAVC_FrameRateCode_e7_493 */
         { 2250,     0},   /* BAVC_FrameRateCode_e10 */
         { 1500,     0},   /* BAVC_FrameRateCode_e15 */
         { 1125,     0},   /* BAVC_FrameRateCode_e20 */
         { 1800,     0},   /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
         {  225,     0},   /* BAVC_FrameRateCode_e100 */
         {  188,     0},   /* BAVC_FrameRateCode_e119_88 */
         {  187,     0},   /* BAVC_FrameRateCode_e120 */
         { 1126,     0},   /* BAVC_FrameRateCode_e19_98 */
         { 3000,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
         { 1875,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
         { 1876,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
         { 2252,     0},   /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
      }

}; /* end of sDeltaPtsLUT[] */



/* sNumElementsLUT maps the pulldown value to a number of elements */
static const uint32_t sNumElementsLUT[BXDM_Picture_PullDown_eMax] =
{
   2, /* Invalid */
   1, /* BXDM_Picture_PullDown_eTop */
   1, /* BXDM_Picture_PullDown_eBottom */
   2, /* BXDM_Picture_PullDown_eTopBottom */
   2, /* BXDM_Picture_PullDown_eBottomTop */
   3, /* BXDM_Picture_PullDown_eTopBottomTop */
   3, /* BXDM_Picture_PullDown_eBottomTopBottom */
   2, /* BXDM_Picture_PullDown_eFrameX2 */
   3, /* BXDM_Picture_PullDown_eFrameX3 */
   1, /* BXDM_Picture_PullDown_eFrameX1 */
   4, /* BXDM_Picture_PullDown_eFrameX4 */
}; /* end of sNumElementsLUT[] */

/* sInitialPolarityLUT maps the pulldown value to the polarity of the
 * first element.  If bReverseFields is enabled, then the table is
 * inverted.
 * SW7425-4434: the only pulldown's that need to be inverted are TB
 * and BT. TBT and BTB are the same in forward and reverse.
 */
static const BAVC_Polarity sInitialPolarityLUT[2][BXDM_Picture_PullDown_eMax] =
{
   /* bReverseFields = false */
   {
      BAVC_Polarity_eTopField,    /* Invalid */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eTop */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eBottom */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eTopBottom */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eBottomTop */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eTopBottomTop */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eBottomTopBottom */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX2 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX3 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX1 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX4 */
   },

   /* bReverseFields = true */
   {
      BAVC_Polarity_eBotField,    /* Invalid */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eTop */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eBottom */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eTopBottom */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eBottomTop */
      BAVC_Polarity_eTopField,    /* BXDM_Picture_PullDown_eTopBottomTop */
      BAVC_Polarity_eBotField,    /* BXDM_Picture_PullDown_eBottomTopBottom */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX2 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX3 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX1 */
      BAVC_Polarity_eFrame,       /* BXDM_Picture_PullDown_eFrameX4 */
   }
}; /* end of sInitialPolarityLUT[] */

/* sPictureTypeLUT maps the pulldown information to whether the FRAME
 * is interlaced or progressive for the purposes of calculating the
 * PTS delta */
static const BXDM_PictureProvider_P_PictureType sPictureTypeLUT[BXDM_Picture_PullDown_eMax] =
{
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* Invalid */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eTop */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eBottom */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eTopBottom */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eBottomTop */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eTopBottomTop */
   BXDM_PictureProvider_P_PictureType_eInterlaced,    /* BXDM_Picture_PullDown_eBottomTopBottom */
   BXDM_PictureProvider_P_PictureType_eProgressive,   /* BXDM_Picture_PullDown_eFrameX2 */
   BXDM_PictureProvider_P_PictureType_eProgressive,   /* BXDM_Picture_PullDown_eFrameX3 */
   BXDM_PictureProvider_P_PictureType_eProgressive,   /* BXDM_Picture_PullDown_eFrameX1 */
   BXDM_PictureProvider_P_PictureType_eProgressive,   /* BXDM_Picture_PullDown_eFrameX4 */
}; /* end of sPictureTypeLUT[] */

/* sProgressiveFrameLUT maps the pulldown information to whether a
 * FRAME is interlaced or progressive for the purposes of field
 * inversion correction. TRUE indicates progressive. */
static const bool sProgressiveFrameLUT[BXDM_Picture_PullDown_eMax] =
{
   false,   /* Invalid */
   false,   /* BXDM_Picture_PullDown_eTop */
   false,   /* BXDM_Picture_PullDown_eBottom */
   false,   /* BXDM_Picture_PullDown_eTopBottom */
   false,   /* BXDM_Picture_PullDown_eBottomTop */
   true,    /* BXDM_Picture_PullDown_eTopBottomTop */
   true,    /* BXDM_Picture_PullDown_eBottomTopBottom */
   true,    /* BXDM_Picture_PullDown_eFrameX2 */
   true,    /* BXDM_Picture_PullDown_eFrameX3 */
   true,    /* BXDM_Picture_PullDown_eFrameX1 */
   true     /* BXDM_Picture_PullDown_eFrameX4 */
}; /* end of sProgressiveFrameLUT[] */

/* sProgressiveStreamLUT maps the pulldown information to whether a
 * STREAM is interlaced or progressive for the purposes of reporting
 * to the application. TRUE indicates progressive. */
static const bool sProgressiveStreamLUT[BXDM_Picture_PullDown_eMax] =
{
   false,   /* Invalid */
   false,   /* BXDM_Picture_PullDown_eTop */
   false,   /* BXDM_Picture_PullDown_eBottom */
   false,   /* BXDM_Picture_PullDown_eTopBottom */
   false,   /* BXDM_Picture_PullDown_eBottomTop */
   false,   /* BXDM_Picture_PullDown_eTopBottomTop */
   false,   /* BXDM_Picture_PullDown_eBottomTopBottom */
   true,    /* BXDM_Picture_PullDown_eFrameX2 */
   true,    /* BXDM_Picture_PullDown_eFrameX3 */
   true,    /* BXDM_Picture_PullDown_eFrameX1 */
   true     /* BXDM_Picture_PullDown_eFrameX4 */
}; /* end of sProgressiveStreamLUT[] */

/* When in CRC mode, display each picture element once and only once.
 * Remap pulldown information that repeats a picture elemnent to
 * a non-repeating variant.
 */
#if 0 /* SWSTB-461: this doesn't appear to be used anymore. 12/21/15 */
static const uint32_t sRemapPullDownLUT[ BXDM_Picture_PullDown_eMax ] =
{
   0,                                   /* invalid */
   BXDM_Picture_PullDown_eTop,            /* unchanged */
   BXDM_Picture_PullDown_eBottom,         /* unchanged */
   BXDM_Picture_PullDown_eTopBottom,      /* unchanged */
   BXDM_Picture_PullDown_eBottomTop,      /* unchanged */
   BXDM_Picture_PullDown_eTopBottom,      /* _eTopBottomTop -> _eTopBottom */
   BXDM_Picture_PullDown_eBottomTop,      /* _eBottomTopBottom -> _eBottomTop */
   BXDM_Picture_PullDown_eFrameX1,        /* _eFrameX2 -> _eFrameX1 */
   BXDM_Picture_PullDown_eFrameX1,        /* _eFrameX3 -> _eFrameX1 */
   BXDM_Picture_PullDown_eFrameX1,        /* unchanged */
   BXDM_Picture_PullDown_eFrameX1         /* _eFrameX4 -> _eFrameX1 */

}; /* end of sRemapPullDownLUT[] */
#endif

/*******************************************************************************
 *
 * Local function prototypes.
 *
 ******************************************************************************/

static BXDM_PictureProvider_TSMResult BXDM_P_PPTSM_S_CompareStcAndPts_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   bool bEvaluateActualPts
   );

static void BXDM_P_PPTSM_S_ActualTSMResultHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   );

static void BXDM_P_PPTSM_S_FinalTSMResultHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   );

/*******************************************************************************
 *
 * Local functions.
 *
 ******************************************************************************/


/*******************************************************************************
 *
 * Public TSM  functions.
 *
 ******************************************************************************/

/*
 * SW7425-764: map BXDM_PictureProvider_MonitorRefreshRate enums to BFMT_Vert
 */
static void BXDM_PPTSM_S_BFMTVert_Enum_Lookup_isr(
   BXDM_PictureProvider_MonitorRefreshRate eMonitorRefreshRate,
   BFMT_Vert * peBFMTRefreshRate
   )
{
   switch( eMonitorRefreshRate )
   {
      case BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz:
         *peBFMTRefreshRate = BFMT_Vert_e7_493Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz:
         *peBFMTRefreshRate = BFMT_Vert_e7_5Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz:
         *peBFMTRefreshRate = BFMT_Vert_e9_99Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e10Hz:
         *peBFMTRefreshRate = BFMT_Vert_e10Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz:
         *peBFMTRefreshRate = BFMT_Vert_e11_988Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e12Hz:
         *peBFMTRefreshRate = BFMT_Vert_e12Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz:
         *peBFMTRefreshRate = BFMT_Vert_e12_5Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz:
         *peBFMTRefreshRate = BFMT_Vert_e14_985Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e15Hz:
         *peBFMTRefreshRate = BFMT_Vert_e15Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz:
         *peBFMTRefreshRate = BFMT_Vert_e19_98Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e20Hz:
         *peBFMTRefreshRate = BFMT_Vert_e20Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e23_976Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e24Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e24Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e25Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e25Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e29_97Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e30Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e30Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e48Hz:
         *peBFMTRefreshRate = BFMT_Vert_e48Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e50Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e50Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e59_94Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e60Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e60Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e100Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e100Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e119_88Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_e120Hz:
         *peBFMTRefreshRate =   BFMT_Vert_e120Hz;
         break;

      case BXDM_PictureProvider_MonitorRefreshRate_eUnknown:
      default:
         *peBFMTRefreshRate =   BFMT_Vert_eInvalid;
         break;
   }
}



static void BXDM_PPTSM_S_ApplyCDTOverride_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_Picture_PullDown *pePullDown
   )
{
   bool b50HzPlusProgressiveSource = false;  /* SW7405-4883: scan out 50+ Hz progressive content as progressive */

   BDBG_ASSERT(pePullDown);
   BSTD_UNUSED(pLocalState);

   /*
    * SW7425-764: save the monitor refresh rate for this picture. Will be passed to VDC in the MFD structure.
    */
   BXDM_PPTSM_S_BFMTVert_Enum_Lookup_isr(
               hXdmPP->stDMConfig.eMonitorRefreshRate,
               &(pstPicture->stPicParms.stDisplay.stStatic.eBFMTRefreshRate)
               );

   /* SW7405-5549: save a copy of the monitor refresh rate.  Currently this is only used to determine if
    * BXDM_PPTSM_S_ApplyCDTOverride_isr needs to be called again from BXDM_PPTSM_P_EvaluateTsmState_isr.
    */
   pstPicture->stPicParms.stDisplay.stStatic.eMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;

   /* Set scan mode and repeat modes based on the default cases.
    * Then override as needed to handle the special cases
    */
   switch ( *pePullDown )
   {
      case BXDM_Picture_PullDown_eTopBottom:
      case BXDM_Picture_PullDown_eBottomTop:
      case BXDM_Picture_PullDown_eTopBottomTop:
      case BXDM_Picture_PullDown_eBottomTopBottom:
         pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eInterlaced;
         pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;

         if ( pstPicture->pstUnifiedPicture->stBufferInfo.eSourceFormat == BXDM_Picture_SourceFormat_eProgressive )
         {
            pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         }
         else
         {
            pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;
         }
         break;

      case BXDM_Picture_PullDown_eTop:
      case BXDM_Picture_PullDown_eBottom:
         pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eInterlaced;
         pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;
         pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;
         break;

      case BXDM_Picture_PullDown_eFrameX1:
      case BXDM_Picture_PullDown_eFrameX2:
      case BXDM_Picture_PullDown_eFrameX3:
      case BXDM_Picture_PullDown_eFrameX4:
         pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eProgressive;
         pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         break;

      default:
         BXVD_DBG_WRN(hXdmPP, ("%x: [%02x.%03x] Unknown Pulldown: %d",
                                    hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                    BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                    pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                    *pePullDown
                                    ));
   }

   switch ( pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
   {
      /* Handle 60i->60p source override */
      case BAVC_FrameRateCode_e50:
      case BAVC_FrameRateCode_e59_94:
      case BAVC_FrameRateCode_e60:
      case BAVC_FrameRateCode_e100:
      case BAVC_FrameRateCode_e119_88:
      case BAVC_FrameRateCode_e120:
         if ( BXDM_PictureProvider_P_ScanMode_eProgressive != pstPicture->stPicParms.stDisplay.stStatic.eScanMode )
         {
            BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] Source Polarity Override: 60i->60p",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

            pstPicture->stPicParms.uiStateBits |= BXDM_PPDBG_State_60iTo60p;

            pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eProgressive;
            pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
            pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         }
         else
         {
            b50HzPlusProgressiveSource = true;  /* SW7405-4883: progressive content with a frame rate >= 50 Hz */
         }
         break;

      default:
         break;
   }

   /* Handle Progressive Source --> Interlaced Scanout Overrides */
   if ( BXDM_PictureProvider_P_ScanMode_eProgressive == pstPicture->stPicParms.stDisplay.stStatic.eScanMode )
   {
      bool b1080Content = false;
      /* Handle 1080p->1080i source override for non-1080p pass-thru cases
       *
       * SW7420-666: Originally the comparison below was to 720.  However there are some progressive
       * Allegro streams incorrectly coded with a frame rate of 60i and a source height of 736.
       * These streams are actually 60p.  The logic in the preceding block sets the scan out parameters
       * correctly to progressive.  However the code in this block was incorrectly setting the scan mode
       * back to interlaced based on the height of 736.  As it turns out, the height was clipped to
       * 720 in BXDM_PPOUT_S_SetClipping.  Whether clipped or not, a source height of 736 can be scanned
       * out as progressive.
       *
       * SW7405-4883: if the video decoder is delivering 1080p source with a frame rate of 50Hz or greater,
       * the assumption is that the entire system can support this content.  With regard to this block of
       * code, we're assuming that VDC has enough memory bandwidth and the stream does NOT need to be
       * scanned out as interlaced.
       *
       * SW7445-608: 2K content was incorrectly being treated as 1080 and scanned out as interlaced.
       * Treat content within a range of heights as 1080.  Any content taller than this will not
       * be overridden, the scan out will be based strictly on the pulldown.
       */
      b1080Content = ( pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight > BXDM_PPTSM_P_1080_LOWER_LIMIT );
      b1080Content &= ( pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight <= BXDM_PPTSM_P_1080_UPPER_LIMIT );

      if ( true == b1080Content
          && false == b50HzPlusProgressiveSource )
      {
         bool bForceInterlacedScanout = false;  /* SWSTB-1769: to simplify the 1080p override logic. */

         /* We have a 1080p source */
         switch ( hXdmPP->stDMConfig.eMonitorRefreshRate )
         {
            case BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e24Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e25Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e30Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e48Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e15Hz:
            case BXDM_PictureProvider_MonitorRefreshRate_e20Hz:
               /* Do not override 23.97,24,25,29.97, or 30Hz displays */
               /* SW7445-2997: do not override 12.5, 14.985, 15 or 20 Hz displays. */
               break;

            default:
               /* SWSTB-1769: simplify the 1080p override logic.
                * Scan out the stream as interlaced if the monitor refresh
                * rate is > 48 Hz and either of the following is true:
                * - progressive content and the frame rate <= 30 Hz
                * - interlaced content and the frame rate > 30 Hz.
                *
                * It turns out we can only get here if one of the preceding it true. The progressive
                * content case is handled by "b50HzPlusProgressiveSource" being false.  The
                * interlaced content case is handled by the 60i->60p override logic up above. */

               bForceInterlacedScanout = true;
               break;

         }

         if ( true == bForceInterlacedScanout
               && BXDM_PictureProvider_1080pScanMode_eDefault == hXdmPP->stDMConfig.e1080pScanMode )
         {
            /* We still need to force an interlaced scanout for this content
             * since by default 1080p scanout at > 48Hz is not supported */
            pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eInterlaced;

            if ( ( BXDM_PictureProvider_PulldownMode_eUseEncodedFormat != hXdmPP->stDMConfig.e1080pPulldownMode )
                 && ( BXDM_Picture_PullDown_eFrameX1 == *pePullDown ) )
            {
               /* We have an HD progressive frame (1080p) but no
                * compatible display, so we need to force it to be
                * interlaced (1080i) for two reasons:
                *  1) Most flavors of VDC does not have the bandwidth to
                *     display 1080p60
                *  2) Many early streams were incorrectly encoded as 1080p
                *     when they were really 1080i */
               BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] Source Polarity Override: 1080p->1080i",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

               pstPicture->stPicParms.uiStateBits |= BXDM_PPDBG_State_1080pTo1080i;

               pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;

               switch ( hXdmPP->stDMConfig.e1080pPulldownMode )
               {
                  case BXDM_PictureProvider_PulldownMode_eTopBottom:
                     *pePullDown = BXDM_Picture_PullDown_eTopBottom;
                     break;
                  case BXDM_PictureProvider_PulldownMode_eBottomTop:
                     *pePullDown = BXDM_Picture_PullDown_eBottomTop;
                     break;
                  default:
                     BXVD_DBG_ERR(hXdmPP, ("%x: [%02x.%03x] Unsupported 1080p Progressive Override Mode (%d)",
                                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                                         hXdmPP->stDMConfig.e1080pPulldownMode
                                                         ));
               }
            }
         }


      }
      else if ( ( 480 == pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight )
               || ( 576 == pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight ) )
      {
         /* Handle 480p30->480i30 source override */
         if ( ! /* Do not override 480p23.97/24 --> 23.97/24Hz Display (1:1 pulldown) */
                ( ( ( ( BAVC_FrameRateCode_e23_976 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                    || ( BAVC_FrameRateCode_e24 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD ) )
                  && ( ( BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz == hXdmPP->stDMConfig.eMonitorRefreshRate )
                       || ( BXDM_PictureProvider_MonitorRefreshRate_e24Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) ) )
                /* SW7445-2997: Do not override 480p12.5 --> 12.5Hz Display (1:1 pulldown) */
                || ( ( BAVC_FrameRateCode_e12_5 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                     &&  ( BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) )
                /* SW7445-2997: Do not override 480p 14.985/15 --> 14.985/25Hz Display (1:1 pulldown) */
                || ( ( ( BAVC_FrameRateCode_e14_985 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                    || ( BAVC_FrameRateCode_e15 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD ) )
                  && ( ( BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz == hXdmPP->stDMConfig.eMonitorRefreshRate )
                       || ( BXDM_PictureProvider_MonitorRefreshRate_e15Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) ) )
                /*  SW7445-2997: Do not override 480p20 --> 20Hz Display (1:1 pulldown) */
                || ( ( ( BAVC_FrameRateCode_e20 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                       || ( BAVC_FrameRateCode_e19_98 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD ) )
                     &&  ( BXDM_PictureProvider_MonitorRefreshRate_e20Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) )
                /* Do not override 480p25 --> 25Hz Display (1:1 pulldown) */
                || ( ( BAVC_FrameRateCode_e25 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                     &&  ( BXDM_PictureProvider_MonitorRefreshRate_e25Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) )
                /* Do not override 480p29.97/30 --> 29.97/30 Hz Display (1:1 pulldown) */
                || ( ( ( BAVC_FrameRateCode_e29_97 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                    || ( BAVC_FrameRateCode_e30 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD ) )
                  && ( ( BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz == hXdmPP->stDMConfig.eMonitorRefreshRate )
                       || ( BXDM_PictureProvider_MonitorRefreshRate_e30Hz == hXdmPP->stDMConfig.eMonitorRefreshRate ) ) )
                /* Do not override 480p50/59.94/60 ever */
                || ( BAVC_FrameRateCode_e50 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                || ( BAVC_FrameRateCode_e59_94 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                || ( BAVC_FrameRateCode_e60 == pstPicture->stPicParms.stTSM.stStatic.eFrameRatePreFRD )
                )
             )
         {
            if ( BXDM_PictureProvider_PulldownMode_eUseEncodedFormat != hXdmPP->stDMConfig.e480pPulldownMode )
            {
               pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eInterlaced;

               switch (*pePullDown)
               {
                  case BXDM_Picture_PullDown_eFrameX1:
                     /* We have a (480/576)p(30/29.97) frame, so we need to treat
                      * is as interlaced because many DVDs are mis-encoded as
                      * progressive.  Here's Darren Neuman's explanation:
                      *
                      * For 480p24 and 480p30, we found on DVD's that this content
                      * was really intended to display on a 480i display, so
                      * scanning out interlaced makes a lot of sense. Often the
                      * content is mis-marked.
                      *
                      * Scanning out interlaced then goes into our MAD
                      * deinterlacer, which can sort out the fields/frames and
                      * construct the correct picture.
                      */
                     BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] Source Polarity Override: 480p->480i",
                                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

                     pstPicture->stPicParms.uiStateBits |= BXDM_PPDBG_State_480pTo480i;

                     pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eField;

                     switch ( hXdmPP->stDMConfig.e480pPulldownMode )
                     {
                        case BXDM_PictureProvider_PulldownMode_eTopBottom:
                           *pePullDown = BXDM_Picture_PullDown_eTopBottom;
                           break;
                        case BXDM_PictureProvider_PulldownMode_eBottomTop:
                           *pePullDown = BXDM_Picture_PullDown_eBottomTop;
                           break;
                        default:
                           BXVD_DBG_ERR(hXdmPP, ("%x: [%02x.%03x] Unsupported 480p Progressive Override Mode (%d)",
                                                                  hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                                  BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                                  pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                                                  hXdmPP->stDMConfig.e480pPulldownMode
                                                                  ));
                     }
                     break;
                  default:
                     break;
               }
            }
         }
      }        /* end of if ( 480 == uiHeight || 576 == uiHeight )                        */
   }           /* end of if ( BXDM_PictureProvider_P_ScanMode_eProgressive == eScanMode ) */

   else  /* ( BXDM_PictureProvider_P_ScanMode_eInterlaced == eScanMode ) */
   {
      /* SW7405-5683: Delivering interlaced content to a display running at <= 30 Hz is not supported by the system.
       * The most common case would be 1080i60 to a 30p display. To get the best possible results, the FIC is disabled
       * and the display mode is forced to eSingleField for interlaced content when the monitor rate is <= 30 Hz.
       * SW7445-2997: added 12.5 -> 20 Hz display rates.
       */
      switch( hXdmPP->stDMConfig.eMonitorRefreshRate )
      {
         case BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e15Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e20Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e24Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e25Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz:
         case BXDM_PictureProvider_MonitorRefreshRate_e30Hz:
            pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode = true;
            pstPicture->stPicParms.uiStateBits |= BXDM_PPDBG_State_ForceSingleFieldMode;
            break;

         default:
            break;
      }

   }

   {
      /* SW7445-403: only apply the override to MPEG content */
      bool bMPEGProtocol = ( BAVC_VideoCompressionStd_eMPEG1 == pstPicture->pstUnifiedPicture->stProtocol.eProtocol );
      bMPEGProtocol |= ( BAVC_VideoCompressionStd_eMPEG2 == pstPicture->pstUnifiedPicture->stProtocol.eProtocol );
      bMPEGProtocol |= ( BAVC_VideoCompressionStd_eMPEG2DTV == pstPicture->pstUnifiedPicture->stProtocol.eProtocol );
      bMPEGProtocol |= ( BAVC_VideoCompressionStd_eMPEG2_DSS_PES == pstPicture->pstUnifiedPicture->stProtocol.eProtocol );

      /* Handle 240i30->240p30 source override */
      if ( ( BXDM_PictureProvider_P_ScanMode_eProgressive != pstPicture->stPicParms.stDisplay.stStatic.eScanMode )
           && ( BXDM_PictureProvider_240iScanMode_eForceProgressive == hXdmPP->stDMConfig.e240iScanMode )
           && ( ( 240 == pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight )
                || ( 288 == pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight ) )
           && ( true == bMPEGProtocol )
         )
      {
         /* We have a (240/288)i(30/29.97) non-AVC frame, so we need
          * to treat it as progressive because some streams are
          * mis-encoded. Here's Darren Neuman's explanation:
          *
          * Basically the content on MPEG-1 and VCD used 352x240
          * progressive for video. The transition to MPEG-2 caused
          * confusion to the encoders at the time, and they
          * mis-labeled some streams 240i for some streams, but we
          * found the content was actually progressive in almost every
          * case we could find. It was just experimental evidence that
          * showed better performance scaning out progressive.
          */
         BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] Source Polarity Override: 240i->240p (non-AVC)",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                          pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

         pstPicture->stPicParms.uiStateBits |= BXDM_PPDBG_State_240iTo240p;

         pstPicture->stPicParms.stDisplay.stStatic.eScanMode = BXDM_PictureProvider_P_ScanMode_eProgressive;
         pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
         pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
      }
   }

   /* Handle 24i to 24p override */
   if ( ( ( BAVC_FrameRateCode_e23_976 == pstPicture->stPicParms.stTSM.stStatic.eFrameRate )
          || ( BAVC_FrameRateCode_e24 == pstPicture->stPicParms.stTSM.stStatic.eFrameRate )
          || ( true == pstPicture->stPicParms.stDisplay.stStatic.b32FilmSource ) )
        && ( ( BXDM_PictureProvider_P_RepeatMode_eFrame != pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode )
             || ( BXDM_PictureProvider_P_RepeatMode_eFrame != pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode ) ) )
   {
      BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] 3:2 Source Format Override",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

      pstPicture->stPicParms.stDisplay.stDynamic.eRateConversionRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
      pstPicture->stPicParms.stDisplay.stDynamic.eTrickPlayRepeatMode = BXDM_PictureProvider_P_RepeatMode_eFrame;
   }
}

#define BXDM_PPTSM_P_FRAME_RATE_NORMALIZATION_FACTOR 1000

typedef struct BXDM_PPTSM_P_FrameRateMap
{
      uint32_t uiNormalizedFrameRate;
      BAVC_FrameRateCode eFrameRateCode;
} BXDM_PPTSM_P_FrameRateMap;

static const BXDM_PPTSM_P_FrameRateMap sFrameRateLUT[] =
{
   { 0, BAVC_FrameRateCode_eUnknown },
   { 7492, BAVC_FrameRateCode_e7_493 },   /* round down, this is really 7.49250 */
   { 7500, BAVC_FrameRateCode_e7_5 }, /* SWSTB-1401: add support for BAVC_FrameRateCode_e7_5 */
   { 9990, BAVC_FrameRateCode_e9_99 }, /* SWSTB-1401: add support for BAVC_FrameRateCode_e9_99 */
   { 10000, BAVC_FrameRateCode_e10 },
   { 11988, BAVC_FrameRateCode_e11_988 }, /* SWSTB-1401: add support for BAVC_FrameRateCode_e11_988 */
   { 12000, BAVC_FrameRateCode_e12 }, /* SWSTB-1401: add support for BAVC_FrameRateCode_e12 */
   { 12500, BAVC_FrameRateCode_e12_5 }, /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
   { 14985, BAVC_FrameRateCode_e14_985 },
   { 15000, BAVC_FrameRateCode_e15 },
   { 19980, BAVC_FrameRateCode_e19_98 }, /* SWSTB-378: add support for BAVC_FrameRateCode_e19_98 */
   { 20000, BAVC_FrameRateCode_e20 },
   { 23976, BAVC_FrameRateCode_e23_976 },
   { 24000, BAVC_FrameRateCode_e24 },
   { 25000, BAVC_FrameRateCode_e25 },
   { 29970, BAVC_FrameRateCode_e29_97 },
   { 30000, BAVC_FrameRateCode_e30 },
   { 50000, BAVC_FrameRateCode_e50 },
   { 59940, BAVC_FrameRateCode_e59_94 },
   { 60000, BAVC_FrameRateCode_e60 },
   {100000, BAVC_FrameRateCode_e100 },
   {119880, BAVC_FrameRateCode_e119_88 },
   {120000, BAVC_FrameRateCode_e120 },
   {120001, BAVC_FrameRateCode_eUnknown }
};

static
BAVC_FrameRateCode
BXDM_PPTSM_S_LookupFrameRate_isr(
         const BXDM_Picture_FrameRate *pstFrameRate
         )
{
   BAVC_FrameRateCode eFrameRateCode = BAVC_FrameRateCode_eUnknown;
   uint32_t uiNormalizedFrameRate;
   uint32_t uiIndex = 0;
   uint32_t uiMaxIndex = sizeof( sFrameRateLUT ) / sizeof( BXDM_PPTSM_P_FrameRateMap );

   if ( ( NULL == pstFrameRate )
        || ( false == pstFrameRate->bValid )
        || ( 0 == pstFrameRate->stRate.uiNumerator )
        || ( 0 == pstFrameRate->stRate.uiDenominator )
      )
   {
      return BAVC_FrameRateCode_eUnknown;
   }

   uiNormalizedFrameRate = (BXDM_PPTSM_P_FRAME_RATE_NORMALIZATION_FACTOR * pstFrameRate->stRate.uiNumerator) / pstFrameRate->stRate.uiDenominator;

   for (uiIndex = 0; uiIndex < uiMaxIndex; uiIndex++ )
   {
      if ( uiNormalizedFrameRate < sFrameRateLUT[uiIndex].uiNormalizedFrameRate ) break;
      eFrameRateCode = sFrameRateLUT[uiIndex].eFrameRateCode;
   }

   return eFrameRateCode;
}

void BXDM_PPTSM_P_PtsCalculateParameters_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   )
{
   BXDM_PictureProvider_P_Picture_Params *   pstPicParms;
   BXDM_PictureProvider_P_PictureType ePictureType;
   BAVC_Polarity ePolarity;
   BXDM_Picture_PullDown ePullDown;
   BAVC_FrameRateCode eFrameRateFRD;

   uint32_t    i;
   bool        bTreatAsSingleElement = false;

   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( pLocalState );

   BDBG_ENTER(BXDM_PPTSM_P_PtsCalculateParameters_isr);

   BDBG_ASSERT( pstPicture ) ;

   /* Initialize the local variables. */
   pstPicParms = &( pstPicture->stPicParms );

   /* Dereference the PPB state to make it easier to access further on in the code.
    * By saving it in the picture context, we will also be able look at the state of preceding pictures.
    */

   /* SWDEPRECATED-1003: If specified use the frame rate override.
    * Otherwise use the value from the stream.
    */
   if ( true == hXdmPP->stDMConfig.stFrameRateOverride.bValid )
   {
      BXDM_Picture_FrameRate stFrameRate;
      stFrameRate.bValid = hXdmPP->stDMConfig.stFrameRateOverride.bValid;
      stFrameRate.stRate = hXdmPP->stDMConfig.stFrameRateOverride.stRate;

      pstPicParms->stTSM.stStatic.eFrameRate = BXDM_PPTSM_S_LookupFrameRate_isr(&stFrameRate);

      bTreatAsSingleElement = hXdmPP->stDMConfig.stFrameRateOverride.bTreatAsSingleElement;

      pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eOverride;
   }
   else
   {
      pstPicParms->stTSM.stStatic.eFrameRate = BXDM_PPTSM_S_LookupFrameRate_isr(&pstPicture->pstUnifiedPicture->stFrameRate);

      pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eCoded;
   }

   pstPicParms->stTSM.stStatic.eFrameRateXVD = pstPicParms->stTSM.stStatic.eFrameRate;

   /* Handle unknown/unsupported coded frame rates */
   if ( ( pstPicParms->stTSM.stStatic.eFrameRate == BAVC_FrameRateCode_eUnknown ) ||
        ( pstPicParms->stTSM.stStatic.eFrameRate >= BXDM_PictureProvider_P_MAX_FRAMERATE ) )
   {
      if ( hXdmPP->stDMState.stDecode.eLastUnsupportedFrameRate != pstPicParms->stTSM.stStatic.eFrameRate )
      {
         /* SW7435-970: Don't output warning if the picture buffer is invalid
            (occurs for the EOS marker on the end of transcode) */
         if ( pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
         {
            if ( BAVC_FrameRateCode_eUnknown == hXdmPP->stDMConfig.eDefaultFrameRate )
            {
               BXVD_DBG_WRN(hXdmPP, ("%x: [%02x.%03x] Overriding unsupported frame rate of %d to default of eUnknown",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                                pstPicParms->stTSM.stStatic.eFrameRate
                                                ));
            }
            else
            {
               BDBG_MODULE_MSG( BXDM_PPTSM, ("%x: [%02x.%03x] Overriding unsupported frame rate of %d to default of %d",
                                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                                pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                                pstPicParms->stTSM.stStatic.eFrameRate,
                                                hXdmPP->stDMConfig.eDefaultFrameRate ));
            }
         }
         hXdmPP->stDMState.stDecode.eLastUnsupportedFrameRate = pstPicParms->stTSM.stStatic.eFrameRate;
      }

      pstPicParms->stTSM.stStatic.eFrameRate =  hXdmPP->stDMConfig.eDefaultFrameRate;

      pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eDefault;
   }

   pstPicParms->stTSM.stStatic.eFrameRatePreFRD = pstPicParms->stTSM.stStatic.eFrameRate;

   if ( false == hXdmPP->stDMConfig.bCRCMode )
   {
      if ( BAVC_FrameRateCode_eUnknown == pstPicParms->stTSM.stStatic.eFrameRate )
      {
         /* If the frame rate is still unknown, then we use the Frame
          * Rate Detection (FRD) logic to get a STABLE frame rate based
          * on the PTS values that we've seen in the past */
         BXDM_PPFRD_P_GetFrameRate_isr(
            hXdmPP,
            pLocalState->eClockRate,
            hXdmPP->stDMConfig.eFrameRateDetectionMode,
            &pstPicParms->stTSM.stStatic.eFrameRate);

         pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eCalculated;

      }

      pstPicParms->stTSM.stStatic.eFrameRateXVD = pstPicParms->stTSM.stStatic.eFrameRate;

      if ( pstPicParms->stTSM.stStatic.eFrameRateXVD == BAVC_FrameRateCode_eUnknown )
      {
         /* PR47456: Determine frame rate using PTS values.  For XVD
          * internal purposes, we really don't care how accurate the
          * frame rate is, so we use the FAST frame rate detection
          * logic */
         BXDM_PPFRD_P_GetFrameRate_isr(
            hXdmPP,
            pLocalState->eClockRate,
            BXDM_PictureProvider_FrameRateDetectionMode_eFast,
            &pstPicParms->stTSM.stStatic.eFrameRateXVD);

         pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eCalculated;
      }
   }

   eFrameRateFRD = pstPicParms->stTSM.stStatic.eFrameRateXVD;

   if ( pstPicParms->stTSM.stStatic.eFrameRateXVD == BAVC_FrameRateCode_eUnknown )
   {
      pstPicParms->stTSM.stStatic.eFrameRateType = BXDM_PictureProvider_P_FrameRateType_eForced;

      /* Set internal XVD default frame rate based on video_height.
       *   - PAL content should assume 25fps.
       *   - All other content should assume 30fps
       */
      if ( 576 == pstPicture->pstUnifiedPicture->stBufferInfo.stSource.uiHeight )
      {
         pstPicParms->stTSM.stStatic.eFrameRateXVD = BAVC_FrameRateCode_e25;
      }
      else
      {
         pstPicParms->stTSM.stStatic.eFrameRateXVD = BAVC_FrameRateCode_e30;
      }
   }

   /* If in CRC mode, use the "frame" bits of the "flags" variable to
    * override the pulldown, source format and progressive sequence flag.
    */

   /* SW7445-586: use accessor method to retrieve pulldwon */
   BXDM_PPQM_P_GetPicturePulldown_isr( pstPicture, &ePullDown );

   /* Determine the pulldown */
   if ((ePullDown > BXDM_Picture_PullDown_eFrameX4) ||
       (ePullDown < BXDM_Picture_PullDown_eTop))
   {
      BXDM_Picture_PullDown eOriginalPullDown = ePullDown;

      ePullDown = BXDM_Picture_PullDown_eTopBottom;

      if ( hXdmPP->stDMState.stDecode.eLastUnsupportedPullDown != eOriginalPullDown )
      {
         BXVD_DBG_WRN(hXdmPP, ("%x: [%02x.%03x] Pulldown Override: Unsupported(%d->%d)",
                                             hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                             BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                             pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                             eOriginalPullDown, ePullDown
                                             ));
         hXdmPP->stDMState.stDecode.eLastUnsupportedPullDown = eOriginalPullDown;
      }
   }

   /* SW7405-5549: save a copy of the pulldown.  Currently this is only used when
    * BXDM_PPTSM_S_ApplyCDTOverride_isr needs to be called again from BXDM_PPTSM_P_EvaluateTsmState_isr.
    */
   pstPicParms->stTSM.stStatic.ePullDownRangeChecked = ePullDown;

   /* SW7405-2942:: move BXDM_PPFRD_P_AddPTS_isr after range checking ePullDown */
   BXDM_PPFRD_P_AddPTS_isr(
      hXdmPP,
      pLocalState,
      pstPicture->pstUnifiedPicture->stPTS.uiValue,
      pstPicture->pstUnifiedPicture->stPTS.bValid,
      sNumElementsLUT[ePullDown]);

   if ( false == hXdmPP->stDMConfig.bCRCMode )
   {
      BXDM_PPTSM_S_ApplyCDTOverride_isr( hXdmPP, pLocalState, pstPicture, &ePullDown);
   }

   /* SWSTB-1805: Treat the picture as single element when single field mode is being forced.
    * Single field mode is forced when interlaced content is being scanned out at 30 Hz or less. */
   bTreatAsSingleElement |= pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode;

   /* Determine the number of elements based on pulldown */
   pstPicParms->stTSM.stStatic.uiNumElements = sNumElementsLUT[ePullDown];

   /* SWDEPRECATED-1003: effectively force FrameX1 when predicting the PTS of the next picture. */
   if ( true == bTreatAsSingleElement )
   {
      pstPicParms->stTSM.stStatic.uiNumElements = 1;
   }

   /* Determine the polarity of each element based on pulldown */
   ePolarity = sInitialPolarityLUT[hXdmPP->stDMConfig.bReverseFields][ePullDown];
   for( i=0; i < pstPicParms->stTSM.stStatic.uiNumElements; i++ )
   {
      BXDM_PPQM_P_SetElementPolarity_isr( pstPicture, i, ePolarity );

      if (ePolarity == BAVC_Polarity_eTopField)
      {
         ePolarity = BAVC_Polarity_eBotField;
      }
      else if (ePolarity == BAVC_Polarity_eBotField)
      {
         ePolarity = BAVC_Polarity_eTopField;
      }
   }

   /* Determine the picture type based on pulldown */
   ePictureType = sPictureTypeLUT[ePullDown];


   /* Reporting for progressive bits */
   if (sProgressiveFrameLUT[ePullDown])
   {
      pstPicParms->uiStateBits |= BXDM_PPDBG_State_ProgressiveFramePulldown;
   }

   if (sProgressiveStreamLUT[ePullDown])
   {
      pstPicParms->uiStateBits |= BXDM_PPDBG_State_ProgressiveStreamPulldown;
   }

   if ( BXDM_Picture_SourceFormat_eProgressive == pstPicture->pstUnifiedPicture->stBufferInfo.eSourceFormat )
   {
      pstPicParms->uiStateBits |= BXDM_PPDBG_State_ProgressiveSourceFormat;
   }

   if ( BXDM_Picture_Sequence_eProgressive == pstPicture->pstUnifiedPicture->stPictureType.eSequence )
   {
      pstPicParms->uiStateBits |= BXDM_PPDBG_State_ProgressiveSequence;
   }

   /* Calculate the thresholds */
   pstPicParms->stTSM.stStatic.uiVeryEarlyThreshold = hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooEarlyThreshold;
   pstPicParms->stTSM.stStatic.uiVeryLateThreshold = hXdmPP->stDMConfig.stTSMThresholdSettings.uiTooLateThreshold;
   pstPicParms->stTSM.stStatic.uiPtsVsyncDiffThreshold = hXdmPP->stDMConfig.stTSMThresholdSettings.uiDeltaStcPtsDiffThreshold;

   if (pLocalState->eClockRate == BXDM_PictureProvider_P_ClockRate_eDirecTV)
   {
      /* DirectTV uses a 27Mhz clock (vs MPEG2's 45Khz) so we need to
       * scale the thresholds accordingly */
      pstPicParms->stTSM.stStatic.uiVeryEarlyThreshold *= 600;
      pstPicParms->stTSM.stStatic.uiVeryLateThreshold *= 600;
   }

   /* Determine the the delta PTS based on the clock rate, picture type, and frame rate */
   pstPicParms->stTSM.stStatic.stPTSDelta = sDeltaPtsLUT[pLocalState->eClockRate][ePictureType][pstPicParms->stTSM.stStatic.eFrameRateXVD];

   /* SW7405-5231: only use whole values when in CRC mode.
    * There was an occasional dropped picture during the CRC testing for AVD.  These sometimes
    * occurred when the Delivery Queue under flowed and then two pictures were delivered on a
    * subsequent vsync.  Due to the handling of the fractional component of the virtual PTS
    * and STC, both pictures would pass the TSM test.  As a result, the first picture would
    * be dropped.  This problem goes away when whole numbers are used.   In CRC mode, the
    * delta PTS is used to interpolate the virtual STC.  The goal is to scan out a new picture
    * element on every vsync, the source and display rates don't really matter.  We probably
    * could just use '1' for the delta PTS for all content.
    */
   if ( true == hXdmPP->stDMConfig.bCRCMode )
   {
      pstPicParms->stTSM.stStatic.stPTSDelta = sCRCDeltaPtsLUT[ePictureType][pstPicParms->stTSM.stStatic.eFrameRateXVD];
   }


   /* SWDEPRECATED-1003: effectively force FrameX1 when predicting the PTS of the next picture. */
   if ( true == bTreatAsSingleElement )
   {
      pstPicParms->stTSM.stStatic.stPTSDelta = sDeltaPtsLUT[pLocalState->eClockRate][BXDM_PictureProvider_P_PictureType_eProgressive][pstPicParms->stTSM.stStatic.eFrameRateXVD];
   }

   /* SW3548-2418: PTS Jitter Correction Logic
    *
    * The following cases are disabled for now:
    * TODO: Handle sparse PTS
    * TODO: Handle DirectTV clock timebase
    *
    */
    /* FIXME: This condition does NOT effectively test for coded PTS every picture, it only
       tests for coded PTS for *THIS* picture.  Ideally, it should compensate for missing pictures
       by counting them, and adjusting dPTS accordingly */
   if ( ( true == pstPicture->pstUnifiedPicture->stPTS.bValid )  /* Coded PTS every picture */
        && ( BAVC_FrameRateCode_eUnknown != eFrameRateFRD ) /* Frame rate is unknown */
      )
   {
      uint32_t uiJitterCorrectedValue;

      BXDM_PPJRC_P_AddValue_isrsafe(
               hXdmPP->stDMState.stChannel.hJrc,
               pstPicture->pstUnifiedPicture->stPTS.uiValue,
               &pstPicParms->stTSM.stStatic.stPTSDelta,
               pstPicParms->stTSM.stStatic.uiNumElements,
               &uiJitterCorrectedValue
               );

      pstPicParms->stTSM.stStatic.iPTSJitterCorrection = uiJitterCorrectedValue - pstPicture->pstUnifiedPicture->stPTS.uiValue;
   }
   else
   {
      BXDM_PPJRC_P_Reset_isrsafe(hXdmPP->stDMState.stChannel.hJrc);
   }

#if 0
   BKNI_Printf("[eClockRate = %d, ePictureType = %d, eFrameRate = %d] --> deltaPTS=(%d, %d)\n",
               pLocalState->eClockRate, ePictureType, pstPicParms->stTSM.stStatic.eFrameRateXVD;,
               pstPicParms->stTSM.stStatic.stPTSDelta.uiWhole,
               pstPicParms->stTSM.stStatic.stPTSDelta.uiFractional);
#endif

   BDBG_LEAVE(BXDM_PPTSM_P_PtsCalculateParameters_isr);

   return;
} /* end of BXDM_PPTSM_P_PtsCalculateParameters_isr() */


void BXDM_PPTSM_P_PtsInterpolate_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_PictureProvider_P_Picture_Context* pstPrevPicture
   )
{
   BXDM_PPFP_P_DataType stPTSTemp;
   BXDM_PPFP_P_DataType stPTSOfNextPPB;
   uint32_t       i;

   BXDM_PictureProvider_PTSType eTempPtsType;

   BDBG_ENTER(BXDM_PPTSM_P_PtsInterpolate_isr);

   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( pLocalState );

   /* If the PPB contains a coded PTS, use that one.
    * Else grab one from the previous PPB.
    */

   if ( true == pstPicture->pstUnifiedPicture->stPTS.bValid )
   {
      stPTSTemp.uiWhole = pstPicture->pstUnifiedPicture->stPTS.uiValue;
      stPTSTemp.uiFractional = 0;

      pstPicture->stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eCoded;
   }
   else
   {
      BXDM_PPQM_P_GetPredictedPtsWithFrac_isr( pstPrevPicture, BXDM_PictureProvider_P_PTSIndex_eActual, &stPTSTemp );

      BXDM_PPQM_P_GetPtsType_isr( pstPrevPicture, &eTempPtsType );

      if ( BXDM_PictureProvider_PTSType_eCoded == eTempPtsType
            || BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS == eTempPtsType
          )
      {
         pstPicture->stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS;
      }
      else
      {
         pstPicture->stPicParms.stTSM.stStatic.ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
      }

   }

   /* TODO: add a debug statement here */
   if ( BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB <= pstPicture->stPicParms.stTSM.stStatic.uiNumElements )
   {
      pstPicture->stPicParms.stTSM.stStatic.uiNumElements = BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB;
   }

   /* Now calculate the PTS value for each element associated with this PPB.
    * In addition, the predicted PTS for the next PPB will be generated.
    */
   stPTSOfNextPPB = stPTSTemp;

   for( i=0; i < pstPicture->stPicParms.stTSM.stStatic.uiNumElements; i++ )
   {
      BXDM_PPQM_P_SetPtsWithFrac_isr( pstPicture, BXDM_PictureProvider_P_PTSIndex_eActual, i, stPTSTemp );

      /* SW7425-4389: interpolate the PTS values in reverse when the
       * SW STC is running in reverse. */

      if ( true == pLocalState->bUsingSwStcToRunInReverse )
      {
         BXDM_PPFP_P_FixPtSub_isr(
            &stPTSTemp,
            &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
            &stPTSTemp
            );

         BXDM_PPFP_P_FixPtSub_isr(
            &stPTSOfNextPPB,
            &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
            &stPTSOfNextPPB
            );
      }
      else
      {
         BXDM_PPFP_P_FixPtAdd_isr(
            &stPTSTemp,
            &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
            &stPTSTemp
            );

         BXDM_PPFP_P_FixPtAdd_isr(
            &stPTSOfNextPPB,
            &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
            &stPTSOfNextPPB
            );

      }

   }

   BXDM_PPQM_P_SetPredictedPtsWithFrac_isr( pstPicture, BXDM_PictureProvider_P_PTSIndex_eActual, stPTSOfNextPPB );

   BDBG_LEAVE(BXDM_PPTSM_P_PtsInterpolate_isr);

   return;

} /* BXDM_PPTSM_P_PtsInterpolate_isr() */

static void BXDM_P_PPTSM_S_ActualTSMResultHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   )
{

   /* SWSTB-6858: used to store the eStcInvalidForceWait state */
   pstPicture->stPicParms.stTSM.stDynamic.bForceWait = false;

   if ( BXDM_PictureProvider_DisplayMode_eTSM == hXdmPP->stDMConfig.eDisplayMode )
   {
      if ( true == hXdmPP->stDMConfig.bPlayback
           && false == hXdmPP->stDMConfig.bSTCValid
         )
      {
         /* PR50235: If we're:
          *  1) playing back from disk AND
          *  2) in TSM mode AND
          *  3) are evaluating the actual PTS AND
          *  4) STC is invalid AND
          * Then we may want to override the TSM result
          */
         switch (pstPicture->stPicParms.stTSM.stDynamic.eTsmResult)
         {
            case BXDM_PictureProvider_TSMResult_eDrop:
               /* We never want to override a drop state */
               break;

            case BXDM_PictureProvider_TSMResult_ePass:
               if ( true == hXdmPP->stDMState.stDecode.bAutoValidateStc )
               {
                  /* In this case, even though the STC is reported to be
                   * invalid, the STC seems OK, so we auto-validate the STC
                   * and use the actual TSM result */
                  BXDM_PictureProvider_SetSTCValid_isr(
                           hXdmPP,
                           true
                           );
                  break;
               }
            default:
               /* All other states we want to override to wait until the
                * app sets the STC valid */
               pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;

               /* Only flag this wait if the picture is being selected in TSM mode and it is not
                * being repeated (i.e. it hasn't already been displayed.)
                */
               if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode
                    && false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
               {
                  BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eStcInvalidForceWait );

                  /* SWSTB-6858: need to save this state to prevent a reset of the vPTS. */
                  pstPicture->stPicParms.stTSM.stDynamic.bForceWait = true;
               }

         }
      }

      /* PR50235: We turn off auto validation of the STC if it is already
       * valid */
      if ( true == hXdmPP->stDMConfig.bSTCValid )
      {
         hXdmPP->stDMState.stDecode.bAutoValidateStc = false;
      }

      /* SW7425-1721: Execute the TSM result callback prior to determining any additional picture
       * or system state.  Previously this callback was being executed after the FIC evaluation,
       * this made it impossible to get field accurate transitions with the TSM result callback.
       */
      if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
      {
         /* Execute the TSM Result callback. */
         BXDM_PPCB_P_ExecuteSingleCallback_isr( hXdmPP, pLocalState, BXDM_PictureProvider_Callback_eTSMResult );

         /* Set picture selection mode/TSM result based on picture
          * handling mode and TSM Result */
         switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
         {
            case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:
               pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eTSM;
               break;

            case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:
               pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
               break;

            default:
               break;
         }

      }

      if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
      {
         pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement = pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement;

         BXDM_PPQM_P_GetElementPolarity_isr(
            pstPicture,
            pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
            &pstPicture->stPicParms.stDisplay.stDynamic.eSourcePolarity);

         /* Perform FIC adjustment */
         BXDM_PPFIC_P_CalculateFieldInversionCorrectionOffset_isr(
            hXdmPP,
            pLocalState,
            pstPicture);
      }

      if ( BXDM_PictureProvider_TSMResult_ePass == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
      {
         /*  Now that a picture has passed the TSM check, unconditionally stop prerolling. */
         hXdmPP->stDMState.stDecode.bPreRolling = false;
      }
   }
   else
   {
      /* Conditionally generate the ASTM TSM pass callback. */
      if ( ( true == hXdmPP->stDMConfig.bAstmMode )
           && ( BXDM_PictureProvider_TSMResult_ePass == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
         )
      {
         pLocalState->bASTMModeTSMPass = true;
      }
   }

   if ( BXDM_PictureProvider_DisplayMode_eTSM == hXdmPP->stDMConfig.eDisplayMode )
   {
      if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
      {
         BXDM_PPCLIP_P_ClipTimeTSMTransitionHandler_isr(
            hXdmPP,
            pLocalState,
            pstPicture,
            &pstPicture->stPicParms.stTSM.stDynamic.eTsmResult);
      }

      if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
      {
         /* We only generate callbacks if this is the first time we're
          * evaluating the picture.  We don't want to generate callbacks if
          * we're redisplaying a picture.  E.g. if we're in a time bases
          * slideshow, the current picture may eventually be late, but we
          * shouldn't generate an error at that point.
          *
          * SW7425-1721: eTooLate/eTooEarly are only considered a PTS error if "ePictureHandlingMode"
          * is the default.  If "ePictureHandlingMode" has been set to a different value, the assumption
          * is that the application has evaluated the PTS and determined the disposition of the picture.
          * The PTSError callback is redundant in this case.  In addition, executing the callback might
          * confuse matters.
          *
          * Similarly for bTSMPass (BXDM_PictureProvider_Callback_eFirstPTSPassed/BXVD_Interrupt_eFirstPTSPassed)
          * the assumption is that when the application is using the TSM result callback, it has intimate knowledge
          * of the system state is does not require the callback.
          */
         pLocalState->bPtsError = false;

         if ( BXDM_PictureProvider_PictureHandlingMode_eDefault == pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode
               && BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode
            )
         {
            if ( ( BXDM_PictureProvider_TSMResult_eTooLate == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
                 || ( BXDM_PictureProvider_TSMResult_eTooEarly == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
               )
            {
               pLocalState->bPtsError = true;

               /* Get PTS for element */
               BXDM_PPQM_P_GetPts_isr(
                  pstPicture,
                  BXDM_PictureProvider_P_PTSIndex_eActual,
                  0,
                  &hXdmPP->stDMState.stDecode.stErrorPts.ui32RunningPTS);

               BXDM_PPQM_P_GetPtsType_isr(
                  pstPicture,
                  &hXdmPP->stDMState.stDecode.stErrorPts.ePTSType);

               hXdmPP->stDMState.stDecode.stErrorPts.ui32EffectivePTS = pstPicture->stPicParms.stTSM.stDynamic.uiEffectivePTS[BXDM_PictureProvider_P_PTSIndex_eActual];
            }

            /*
             * Conditionally generate the TSM pass callback.
             */
            if ( BXDM_PictureProvider_TSMResult_ePass == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
            {
               pLocalState->bTSMPass = true;
            }
         }

         /* PR52424: If we're in playback, and we see a picture that is
          * TooLate, we want to display the first TooLate picture in vTSM
          * mode but still generate the PTSError to give the app a chance
          * to recover */
         if ( ( true == hXdmPP->stDMConfig.bPlayback )
                 && ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode ) )
         {
            if ( BXDM_PictureProvider_TSMResult_eTooLate == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
            {
               if ( ( BXDM_PictureProvider_PictureHandlingMode_eDefault == pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
                    && ( false == hXdmPP->stDMState.stDecode.bTooLateGimmeDone )
                  )
               {
                  /* SW7425-1074: The mulligan logic should use "eWait" to give the middleware/application
                   * a chance to reseed the STC.  Previously "ePictureHandlingMode" was set to "eIgnorePTS".
                   * This resulted in the picture selection mode being changed to "vsync".
                   * If the STC is reseeded on this vsync, the virtual STC will also be loaded with the new value.
                   * However the virtual PTS of this picture is based on the preceding picture.  Depending on how
                   * far back in time the STC (and virtual STC) is set, it could take a long time for this
                   * picture to mature.  Better to push up the decision to switch to vsync mode rather
                   * then forcing it here.
                   */
                  pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;
                  BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eMulliganLogicForceWait );
                  hXdmPP->stDMState.stDecode.bTooLateGimmeDone = true;
               }
            }
            else
            {
               hXdmPP->stDMState.stDecode.bTooLateGimmeDone = false;
            }
         }

      }
   }
}

void BXDM_P_PPTSM_S_FinalTSMResultHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   )
{
   bool bDecodeErrorDrop = false;

   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement = pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement;

      BXDM_PPQM_P_GetElementPolarity_isr(
         pstPicture,
         pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
         &pstPicture->stPicParms.stDisplay.stDynamic.eSourcePolarity);

      /* Perform FIC adjustment */
      BXDM_PPFIC_P_CalculateFieldInversionCorrectionOffset_isr(
         hXdmPP,
         pLocalState,
         pstPicture);
   }

   /* PR56051: the failure occurred while pictures were being dropped for error
    * conditions AND the delivery  queue ran dry.  On the next vsync, the
    * TSM logic would re-evaluate the current picture.  This re-evaluation was
    * causing the Display Manager to prematurely exit the error dropping state.
    * The fix is to qualify "hXdmPP->stDMConfig.eErrorHandlingMode"
    * with "true == bFirstEvaluation"
    *
    * SWSTB-439: adds a new error level with a range of 0% to 100%. Drop a picture
    * if the percentage of macro blocks with an error is greater than or equal
    * to this threshold. Evaluated when eErrorHandlingMode is ePicture or ePrognostic.
    *
    * Note: we can come through here before a picture has been received,
    * hence the need to check pstUnifiedPicture.*/

   if ( NULL != pstPicture->pstUnifiedPicture )
   {
      bool bPictureHasAnError;

      /* SWSTB-439: if the error level is non-zero, use "uiPercentError" from the picture structure
       * to determine if the picture should be dropped.  Otherwise use the error flag. */

      if ( 0 != hXdmPP->stDMConfig.uiErrorThreshold )
      {
         bPictureHasAnError = ( pstPicture->pstUnifiedPicture->stError.uiPercentError >= hXdmPP->stDMConfig.uiErrorThreshold );
      }
      else
      {
         bPictureHasAnError = pstPicture->pstUnifiedPicture->stError.bThisPicture;
      }

      if ( BXDM_PictureProvider_ErrorHandlingMode_ePicture == hXdmPP->stDMConfig.eErrorHandlingMode
           && false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated
         )
      {
         if ( true == bPictureHasAnError )
         {
            bDecodeErrorDrop = true;
         }
      }
      else if ( BXDM_PictureProvider_ErrorHandlingMode_ePrognostic == hXdmPP->stDMConfig.eErrorHandlingMode
                && false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
      {
         /* Exit "drop  until next RAP" mode if,
          * - the picture does NOT contain a decode error
          * - the picture IS a RAP
          * - and "bDropUntilNextRAP" is true
          *   (this is redundant, but might be useful for debugging)
          */
         if ( false == bPictureHasAnError
              && true == pstPicture->pstUnifiedPicture->stPictureType.bRandomAccessPoint
              && true == hXdmPP->stDMState.stDecode.bDropUntilNextRAP
            )
         {
            hXdmPP->stDMState.stDecode.bDropUntilNextRAP = false;
         }

         /*
          * Drop the picture?
          */
         if ( true == bPictureHasAnError
              || true == pstPicture->pstUnifiedPicture->stError.bPreviousRefPic
            )
         {
            bDecodeErrorDrop = true;
         }
         else if ( true == hXdmPP->stDMState.stDecode.bDropUntilNextRAP )
         {
            /* When in "bDropUntilNextRAP" mode, don't
             * drop error free I pictures.
             */
            if ( BXDM_Picture_Coding_eI != pstPicture->pstUnifiedPicture->stPictureType.eCoding )
            {
               bDecodeErrorDrop = true;
            }
         }

         /* Enter "drop until next RAP" mode if,
          * - the picture contains a decode error
          * - and it is a reference picture.
          */
         if ( ( true == bPictureHasAnError )
               && ( pstPicture->pstUnifiedPicture->stPictureType.bReference ) )
         {
            hXdmPP->stDMState.stDecode.bDropUntilNextRAP = true;
         }

       }
    }

   /* Override the TSM result if the decode error flag is set */
   if ( true == bDecodeErrorDrop )
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eDrop;

      /* bump "uiDecodeErrorDropCount" if,
       * - evaluating the real PTS (not the virtual)
       * - a new picture (TODO: is this needed?)
       */
      if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
      {
         hXdmPP->stDMState.stDecode.uiDecodeErrorDropCount++;
      }
   }

   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      BXDM_PPCLIP_P_ClipTimeTSMTransitionHandler_isr(
         hXdmPP,
         pLocalState,
         pstPicture,
         &pstPicture->stPicParms.stTSM.stDynamic.eTsmResult);
   }

   /* PR52247: In the case of picture-less clips (i.e. only contains a
    * single dummy PPB), we won't have a valid picture displayed, so
    * we need to ensure the clip callback trigger handler is still
    * executed */
   BXDM_PPCLIP_P_ClipTimeCallbackTriggerHandler_isr( hXdmPP, pLocalState, pstPicture );

   if ( BXDM_PictureProvider_PictureHandlingMode_eDrop == pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eDrop;
   }
   else if ( BXDM_PictureProvider_PictureHandlingMode_eWait == pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode
            &&  BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode  )
   {
      /* SW7635-51: give the middleware/application the option to hold off processing a picture.
       * The TSMResult callback is only executed when eSelectionMode is eTSM.   There is the potential
       * for a deadlock if ePictureHandlingMode is set to eWait on one vsync and then eSelectionMode changes
       * to eVirtual on the next vsync.  This change could happen if the system were put into vsync mode,
       * a decoder trick mode began or there was a PCR discontinuity.  To avoid this, only override the TSM
       * result if eSelectionMode is eTSM.
       */
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;
      BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eTSMResultCallbackForceWait );
   }

}

void BXDM_PPTSM_P_EvaluateTsmState_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture
   )
{
   BDBG_ENTER(BXDM_PPTSM_P_EvaluateTsmState_isr);

   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eEvaluateTsmState );

   /* SW7405-5549: if the monitor refresh rate changed after a picture was validated
    * (in BXDM_PPQM_S_ValidatePictureContext) call BXDM_PPTSM_S_ApplyCDTOverride_isr again to
    * ensure that the picture is scanned out properly.  This can occur if the refresh rate
    * is changed while the system is paused.
    */
   if ( false == hXdmPP->stDMConfig.bCRCMode
       && true == pstPicture->bValidated
       && pstPicture->stPicParms.stDisplay.stStatic.eMonitorRefreshRate != hXdmPP->stDMConfig.eMonitorRefreshRate
      )
   {
      BXDM_Picture_PullDown ePullDown = pstPicture->stPicParms.stTSM.stStatic.ePullDownRangeChecked;

      BXDM_PPTSM_S_ApplyCDTOverride_isr( hXdmPP, pLocalState, pstPicture, &ePullDown );
   }


   /* Handle removal delay */
   if ( 0 != hXdmPP->stDMState.stDecode.uiRemovalDelay )
   {
      if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
      {
         BXDM_PPDBG_P_SelectionLog_isr( hXdmPP, BXDM_PPDBG_Selection_eDelay );
         pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;

         hXdmPP->stDMState.stDecode.uiRemovalDelay--;
      }
      return;
   }

   /* Remember this pictures last dStcPts */
   pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceDisplayed = pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated;


   /* SWSTB-6858: to prevent a bogus reset of the vPTS in BXDM_PPVTSM_P_VirtualPtsInterpolate_isr.
    * This reset was causing two pictures to pass on a single vsync.
    * If iStcPtsDifferenceDisplayed is too large, the reset logic assumes that the delivery queue
    * has underflowed. In this scenario, iStcPtsDifferenceDisplayed is not valid because the STC
    * was not valid on the previous vsync, so it is set to 0 here.  This failure occured when:
    * On vsync "n":
    * - the delivery queue contained data AND the STC was not valid
    * - the TSM result for the picture was force to be WAIT.
    * On vsync "n+1":
    * - the clip start callback fired
    * - the STC which had been loaded on the previous vsync, was now marked as invalid
    * - XDM was paused by setting the playback rate to 0.  */

   if ( true == pstPicture->stPicParms.stTSM.stDynamic.bForceWait )
   {
      pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceDisplayed = 0;
      BDBG_MODULE_MSG( BXDM_PPVTSM, ("%x:[%02x.%03x] bForceWait is true, resetting iStcPtsDifferenceDisplayed",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF ));

      BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eVTSM, true,
                                 "%x:[%02x.%03x] bForceWait is true, resetting iStcPtsDifferenceDisplayed",
                                 hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstPicture->stPicParms.uiPPBIndex & 0xFFF );
   }


   /*
    * Always perform the TSM evaluation for the real STC and PTS.
    */
   BXDM_P_PPTSM_S_CompareStcAndPts_isr(
       hXdmPP,  pLocalState,  pstPicture, true );

   BXDM_P_PPTSM_S_ActualTSMResultHandler_isr( hXdmPP, pLocalState, pstPicture );

   /*
    * Conditionally perform the TSM evaluation for the "virtual" STC and PTS.
    */
   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      /* SWSTB-6858: bForceWait is only applicable when eSelectionMode == eTSM. */
      pstPicture->stPicParms.stTSM.stDynamic.bForceWait = false;

      BXDM_P_PPTSM_S_CompareStcAndPts_isr( hXdmPP,  pLocalState,  pstPicture, false );

      /* SW7445-491: add support for the TSM result callback when in vsync mode.
       * This logic can either be here or in BXDM_P_PPTSM_S_FinalTSMResultHandler_isr. It just
       * needs to be executed after the preceding call to BXDM_P_PPTSM_S_CompareStcAndPts_isr.
       * Since we are only supporting the "eDefault" and "eDrop" modes, we don't need to
       * worry about how this logic might interact with the FIC logic.
       */
      if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == hXdmPP->stDMConfig.eDisplayMode )
      {
         if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
         {
            /* Execute the TSM Result callback. */
            BXDM_PPCB_P_ExecuteSingleCallback_isr( hXdmPP, pLocalState, BXDM_PictureProvider_Callback_eTSMResult );

            /* When in vsync mode, only ePictureHandlingMode's of eDefault and eDrop are supported.
             * Force all other modes to eDefault. */
            switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
            {
               case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:
               case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:
               case BXDM_PictureProvider_PictureHandlingMode_eWait:
                  pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode = BXDM_PictureProvider_PictureHandlingMode_eDefault;

                  BDBG_MODULE_MSG( BXDM_PPQM, (" %x:[%02x.xxx] ePictureHandlingMode of %d is not support in vsync mode, forcing eDefault",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode ));

                  BXDM_PPDFIFO_P_QueString_isr( hXdmPP, BXDM_Debug_MsgType_eQM, true,
                                       " %x:[%02x.xxx] ePictureHandlingMode of %d is not support in vsync mode, forcing eDefault",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                       pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode );
                  break;

               default:
                  break;
            }
         }
      }

   }

   BXDM_P_PPTSM_S_FinalTSMResultHandler_isr( hXdmPP, pLocalState, pstPicture );

   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eEvaluateTsmState );

   BDBG_LEAVE(BXDM_PPTSM_P_EvaluateTsmState_isr);

   return;
} /* BXDM_PPTSM_P_EvaluateTsmState_isr() */

static BXDM_PictureProvider_TSMResult BXDM_P_PPTSM_S_CompareStcAndPts_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   bool bEvaluateActualPts
   )
{
   uint32_t    uiEffectiveStc;
   BXDM_PPFP_P_DataType stPts[BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB];
   int32_t     iStcPtsDifferenceActual[BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB];
   int32_t     iStcPtsDifferenceEvaluated[BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB];
   int32_t     iStcPtsDifferenceEvaluatedDeltaAbs[BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB];
   BXDM_PictureProvider_P_PTSIndex ePTSIndex;
   uint32_t       i;
   BXDM_PictureProvider_PTSType ePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS;

   BDBG_ENTER(BXDM_P_PPTSM_S_CompareStcAndPts_isr);

   BDBG_ASSERT( pstPicture );

   /* If the context for this picture has not been validated, simply return from this routine.
    * Currently the only way to get in this state is;
    * - the stream loops when playbacking from disk
    * - AND this routine is called to redisplay the currently selected picture
    *
    * The preceding prevents spurious PTSError callbacks
    *
    * In addition if in live mode, don't perform TSM evaluation until the first valid PCR
    * offset has been received.  Prior to that, the TSM results will be random.
    * However, if the picture selection mode is "vsync", do perform the TSM evaluation
    * using the virtual STC and PTS values. ( PR53423 )
    */
   if ( false == pstPicture->bValidated
        || ( false == hXdmPP->stDMState.stDecode.bValidPcrOffsetReceived
             && true == bEvaluateActualPts
           )
      )
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eDrop;
      goto AllDone;
   }
   else
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_ePass;
   }

   /* SW7435-657: save this intermediate value in the event the global values in stDMConfig
    * is changed during XDM's execution, say for instance in the TSM result callback.
    * This value will be bound to the picture if it is selected for display.
    */
   BXDM_PPQM_P_GetSoftwarePcrOffset_isr(
         hXdmPP,
         pstPicture,
         &(pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation)
         );


   if ( true == bEvaluateActualPts )
   {
      uint32_t uiPCROffset;

      /* TSM Mode */
      ePTSIndex = BXDM_PictureProvider_P_PTSIndex_eActual;

      /*
       * Calculate the effective STC, i.e. add the PCR offset to the STC
       */
      BXDM_PPQM_P_GetCookedPcrOffset_isr( hXdmPP, pstPicture, &uiPCROffset );

      uiEffectiveStc = pLocalState->uiAdjustedStc + uiPCROffset;

      BXDM_PPQM_P_GetPtsType_isr( pstPicture, &ePtsType );
   }
   else
   {
      /* VSYNC Mode */
      ePTSIndex = BXDM_PictureProvider_P_PTSIndex_eVirtual;

      BXDM_PPVTSM_P_VirtualStcGet_isr( hXdmPP, &uiEffectiveStc );

      /* SW7435-657: the SW PCR offset has been decoupled from the vSTC, add it in now. */
      uiEffectiveStc += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;
   }

   /* SW7425-2536: save the effective STC. Currently only returned as
    * status in the "drop at decode" callbacks.
    */
   pstPicture->stPicParms.stTSM.stDynamic.uiEffectiveStc[ePTSIndex] = uiEffectiveStc;

   /* For this picture, find the element with smallest STC/PTS
    * diff that has matured (i.e. the smallest positive value)
    */
   for (i = 0; i < pstPicture->stPicParms.stTSM.stStatic.uiNumElements; i++)
   {
      /* Get PTS for element */
      BXDM_PPQM_P_GetPtsWithFrac_isr( pstPicture, ePTSIndex, i, &stPts[i] );

      /* SW7425-2255: once a picture has been selected for display, use the
       * offset bound to the picture.
       * SW7435-657: save this intermediate value (uiPtsOffsetUsedForEvaluation)
       * in the event the global values in stDMConfig is changed during XDM's execution,
       * say for instance in the TSM result callback.  This value will be bound to
       * the picture if it is selected for display.
       */
      BXDM_PPQM_P_GetPtsOffset_isr(
               hXdmPP,
               pstPicture,
               &(pstPicture->stPicParms.stTSM.stDynamic.uiPtsOffsetUsedForEvaluation)
               );

      /* Add PTSOffset for lip sync when */
      stPts[i].uiWhole += pstPicture->stPicParms.stTSM.stDynamic.uiPtsOffsetUsedForEvaluation;

      /* Add the JRC jitter offset, but only when evaluating the ACTUAL PTS values,
       * since the virtual PTS values are already de-jittered */
      if ( BXDM_PictureProvider_P_PTSIndex_eActual == ePTSIndex )
      {
         stPts[i].uiWhole += pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection;
      }

      /* Save the actual PTS/STC diff BEFORE we do apply the FICPTSOffset .
       * SW7425-1264: if the STC is running in reverse, flip the comparision,
       * i.e. subtract the STC from the PTS.
       */
      if ( true == pLocalState->bUsingSwStcToRunInReverse )
      {
         iStcPtsDifferenceActual[i] = stPts[i].uiWhole - uiEffectiveStc;

         /* Subtract FIC offset for Frame Rate Conversion */
         BXDM_PPFP_P_FixPtSub_isr(
            &stPts[i],
            &hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset,
            &stPts[i]);
      }
      else
      {
         iStcPtsDifferenceActual[i] = uiEffectiveStc - stPts[i].uiWhole;

         /* Add FIC offset for Frame Rate Conversion */
         BXDM_PPFP_P_FixPtAdd_isr(
            &stPts[i],
            &hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset,
            &stPts[i]);
      }

      /* SWBLURAY-18249: round the PTS value up instead of truncating.
       * The need for this was highlighted when playing a seamless clip.
       * The clip stop time was the expected first PTS of the next clip.
       * Since it was a seamless clip, the first picture was processed in
       * vsync mode, i.e. the virtual PTS was used.  When the vPTS was truncated,
       * the picture was passing one tick too soon and the clip stop time
       * was not reached.
       */
      if ( 0 != stPts[i].uiFractional
            && false == pLocalState->bUsingSwStcToRunInReverse )
      {
         stPts[i].uiWhole++;
         stPts[i].uiFractional = 0;
      }

      /* SW7425-1264: if the STC is running in reverse, subtract the STC from the PTS. */
      if ( true == pLocalState->bUsingSwStcToRunInReverse )
      {
         iStcPtsDifferenceEvaluated[i] = stPts[i].uiWhole - uiEffectiveStc;
      }
      else
      {
         iStcPtsDifferenceEvaluated[i] = uiEffectiveStc - stPts[i].uiWhole;
      }


      iStcPtsDifferenceEvaluatedDeltaAbs[i] = hXdmPP->stDMState.stDecode.iPreviousStcPtsDifferenceEvaluated - iStcPtsDifferenceEvaluated[i];

#if 0
      if (((BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) && bEvaluateActualPts) ||
          ((BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) && !bEvaluateActualPts))
      {

         BKNI_Printf("%x: [%d] stc: %lu - pts: %lu = %ld\n",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     i,
                     uiEffectiveStc,
                     stPts[i].uiWhole,
                     iStcPtsDifferenceEvaluated[i]);

         BKNI_Printf("%x: [%d] %ld - %ld = %ld\n",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     i,
                     hXdmPP->stDMState.stDecode.iPreviousStcPtsDifferenceEvaluated,
                     iStcPtsDifferenceEvaluated[i],
                     iStcPtsDifferenceEvaluatedDeltaAbs[i]
            );
      }
#endif

      if ( iStcPtsDifferenceEvaluatedDeltaAbs[i] < 0 )
      {
         iStcPtsDifferenceEvaluatedDeltaAbs[i] = -iStcPtsDifferenceEvaluatedDeltaAbs[i];
      }
   }

   /* Find the best element within the picture */
   /* We need to decide which element to start with.
    * If the picture has already been displayed, then we start with the last displayed element.
    * However, if it has not been displayed, then we start from the beginning.
    *
    * We need to make this distinction because even though the previous TSM result may have been ePass/eLate,
    * it may have been overridden to eWait.  This is likely in playback when the STC hasn't been validated, yet,
    * and a non-ePass TSM result is always overridden to eWait.  So, we always want to start from
    * the first element of the PPB.
    */
   if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
   {
      pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement = 0;
   }

   pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement = pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement;
   for (i = pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement; i < pstPicture->stPicParms.stTSM.stStatic.uiNumElements; i++)
   {
      /* Never select an element with a negative actual STC/PTS diff */
      if (iStcPtsDifferenceActual[i] < 0)
      {
         break;
      }

      /* PR47453: We pick the element to minimize the
       * |deltaPtsStcOffset| between vsyncs to account for any STC/PTS
       * jitter that we may have */

      /* PR51148: With the original jitter correction logic, it was
       * possible to lock to large dStcPts diff values after trick
       * modes.  E.g. the upper dStcPts diff wasn't bounded, so after
       * a trick mode, the jitter correction logic would try to
       * maintain an possibly larger dStcPts.  The goal of the jitter
       * correction TSM is to pick the best picture that results in an
       * effective dStcPts of between [-jitter, dPTS] with the most
       * stable dStcPts.  The picture selection logic has been updated
       * as follows:
       *
       *  1) A picture's evaluated dStcPts must be greater than
       *     -(jitter threshold) -- this is the lower bound
       *
       *  2) If the picture's evaluated dStcPts is greater than dPTS,
       *     always select the picture for display. -- this is the
       *     upper bound
       *
       *  3) if the picture evaluated dStcPts is less than one picture
       *     time, then we pick the picture that results in the
       *     smallest change in the dStcPts. -- this is the jitter
       *     correction portion
       *
       */
      if ( ( iStcPtsDifferenceEvaluated[i] >= 0 )
           && ( ( pLocalState->iStcPtsDifferenceEvaluatedBest[ePTSIndex] >= (int32_t) pstPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole )
                || ( pLocalState->iStcPtsDifferenceEvaluatedDeltaAbsMin[ePTSIndex] + 1 >= iStcPtsDifferenceEvaluatedDeltaAbs[i] )
                || ( iStcPtsDifferenceEvaluated[i] > (int32_t) pstPicture->stPicParms.stTSM.stStatic.uiVeryLateThreshold )

                /* SW7550-349: When in STC pause, we don't want jitter correction because it can cause
                 * issues with STC frame advance preventing the next picture from getting selected.
                 */
                || ( ( true == bEvaluateActualPts )
                      && ( BXDM_PictureProvider_P_STCTrickMode_ePause == pLocalState->eSTCTrickMode ) )
              )
         )
         {
         /* Save the smallest jitter compensated StcPtsDifference for
          * this vsync */
         pLocalState->iStcPtsDifferenceEvaluatedDeltaAbsMin[ePTSIndex] = iStcPtsDifferenceEvaluatedDeltaAbs[i];
         pLocalState->iStcPtsDifferenceEvaluatedBest[ePTSIndex] = iStcPtsDifferenceEvaluated[i];
         pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement = i;
      }
   }

   pstPicture->stPicParms.stTSM.stDynamic.uiEffectivePTS[ePTSIndex] = stPts[pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement].uiWhole;
   pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated = iStcPtsDifferenceEvaluated[pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement];
   pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual = iStcPtsDifferenceActual[pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement];
   pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluatedDeltaAbs = iStcPtsDifferenceEvaluatedDeltaAbs[pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement];

   /*
    * Now that the necessary values have been calculated, perform the TSM evaluation.
    */
   if ( ( BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS == ePtsType )
        && ( true == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
      )

   {
      /* We have an invalid PTS, we don't know when to display the
       * picture, so we just drop it */

      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eDrop;
   }
   else if ( true == BXDM_PPCLIP_P_ClipTimeTSMHandler_isr(
                hXdmPP,
                pLocalState,
                pstPicture,
                bEvaluateActualPts,
                &pstPicture->stPicParms.stTSM.stDynamic.eTsmResult
                ))
   {
      /* Done */
   }

   /* handle too late case */
   else if ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated > (int32_t)pstPicture->stPicParms.stTSM.stStatic.uiVeryLateThreshold )
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eTooLate;
   }
   /* handle too early case */
   else if ( -pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated > (int32_t)pstPicture->stPicParms.stTSM.stStatic.uiVeryEarlyThreshold )
   {
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eTooEarly;
   }
   /* PR47453: We only want to pass an element if it produces the
    * smallest |deltaPtsStcOffset| of all elements selected so far */
   else if ( ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual >= 0 )
             && ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluatedDeltaAbs == pLocalState->iStcPtsDifferenceEvaluatedDeltaAbsMin[ePTSIndex] )
             && ( pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated == pLocalState->iStcPtsDifferenceEvaluatedBest[ePTSIndex] ) )
   {
      /*
       * The picture has matured
       */
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_ePass;
   }
   else
   {
      /*
       * The picture is early.
       */
      pstPicture->stPicParms.stTSM.stDynamic.eTsmResult = BXDM_PictureProvider_TSMResult_eWait;
   }


 /* SW7445-1329: only compile for debug builds.
  * SWSTB-1380: when using the debug fifo, the following messages get queued up
  * every vsync.  This was having a negative impact on performance. Only
  * compile the code when we really need it. */

#if BDBG_DEBUG_BUILD && BXDM_DEBUG_LOW_PRIORITY

   if (((BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) && bEvaluateActualPts) ||
       ((BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) && !bEvaluateActualPts))
   {
      char cTsmResult;

      switch ( pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
      {
         case 0:  cTsmResult = 'e';    break;
         case 1:  cTsmResult = 'w';    break;
         case 2:  cTsmResult = 'p';    break;
         case 3:  cTsmResult = 'l';    break;
         case 4:  cTsmResult = 'd';    break;
         default: cTsmResult = 'u';    break;
      }

      if ( true == pLocalState->bUsingSwStcToRunInReverse )
      {
         BDBG_MODULE_MSG( BXDM_PPTSM, ("%x:[%02x.%03x] %c:%c pts:%08x - stc:%08x =%8d slt:%c elm:%d %s",
                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                       pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                       ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode ) ? 't' : 'v',
                       cTsmResult,
                       pstPicture->stPicParms.stTSM.stDynamic.uiEffectivePTS[ePTSIndex],
                       uiEffectiveStc,
                       pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated,
                       hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset.uiWhole ? '2' : '1',
                       pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement,
                       ( pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated ) ? "rp" : "  " ));
      }
      else
      {
         BDBG_MODULE_MSG( BXDM_PPTSM, ("%x:[%02x.%03x] %c:%c stc:%08x - pts:%08x =%8d slt:%c elm:%d %s",
                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                       pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                       ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode ) ? 't' : 'v',
                       cTsmResult,
                       uiEffectiveStc,
                       pstPicture->stPicParms.stTSM.stDynamic.uiEffectivePTS[ePTSIndex],
                       pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceEvaluated,
                       hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset.uiWhole ? '2' : '1',
                       pstPicture->stPicParms.stTSM.stDynamic.uiTSMSelectedElement,
                       ( pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated ) ? "rp" : "  " ));
      }
   }
#endif

   /* SW7425-1264: track which STC was selected, currently only used for debug.
    */
   if ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride )
   {
      pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc = true;
   }
   else
   {
      pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc = false;
   }


  AllDone:
   BDBG_LEAVE(BXDM_P_PPTSM_S_CompareStcAndPts_isr);

   return pstPicture->stPicParms.stTSM.stDynamic.eTsmResult;

} /* end of BXDM_P_PPTSM_S_CompareStcAndPts_isr() */
