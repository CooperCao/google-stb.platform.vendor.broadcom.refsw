/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * [File Description:]
 *
 ***************************************************************************/
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */
#include "bint.h"

#include "bvce_platform.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BVCE_PLATFORM);

BERR_Code
BVCE_Platform_P_WriteRegisterList(
         BREG_Handle hReg,
         const BVCE_Platform_P_RegisterSetting *astRegisterList,
         uint32_t uiCount
         )
{
   uint32_t i;
   uint64_t uiRegValue;

   BDBG_ENTER( BVCE_P_WriteRegisterList );

   for ( i = 0; i < uiCount; i++ )
   {
#if BCHP_REGISTER_HAS_64_BIT
      if ( true == astRegisterList[i].bIs64Bit )
      {
         uiRegValue = BREG_Read64(
                  hReg,
                  astRegisterList[i].uiAddress
                  );
      }
      else
      {
#endif
      uiRegValue = BREG_Read32(
               hReg,
               astRegisterList[i].uiAddress
               );
#if BCHP_REGISTER_HAS_64_BIT
      }
#endif

      uiRegValue &= ~astRegisterList[i].uiMask;
      uiRegValue |= astRegisterList[i].uiValue & astRegisterList[i].uiMask;
#if BCHP_REGISTER_HAS_64_BIT
      if ( true == astRegisterList[i].bIs64Bit )
      {
         BREG_Write64(
                  hReg,
                  astRegisterList[i].uiAddress,
                  uiRegValue
                  );
      }
      else
      {
#endif
      BREG_Write32(
               hReg,
               astRegisterList[i].uiAddress,
               uiRegValue
               );
#if BCHP_REGISTER_HAS_64_BIT
      }
#endif

#if BDBG_DEBUG_BUILD
      {
        uint64_t uiRegValueActual;

#if BCHP_REGISTER_HAS_64_BIT
        if ( true == astRegisterList[i].bIs64Bit )
        {
           uiRegValueActual = BREG_Read64(
                 hReg,
                 astRegisterList[i].uiAddress
           );
        }
        else
        {
#endif
        uiRegValueActual = BREG_Read32(
              hReg,
              astRegisterList[i].uiAddress
        );
#if BCHP_REGISTER_HAS_64_BIT
        }
#endif

        BDBG_MSG(("@0x%08x <-- "BDBG_UINT64_FMT" ("BDBG_UINT64_FMT") - %s",
                  astRegisterList[i].uiAddress,
                  BDBG_UINT64_ARG(uiRegValue),
                  BDBG_UINT64_ARG(uiRegValueActual),
                  astRegisterList[i].szDescription
                  ));
      }
#endif
   }

   BDBG_LEAVE( BVCE_P_WriteRegisterList );

   return BERR_TRACE( BERR_SUCCESS );
}

void
BVCE_Platform_P_DumpRegisterList(
         BREG_Handle hReg,
         const BVCE_Platform_P_RegisterList *astRegisterList
         )
{
   unsigned i;

   BDBG_ENTER( BVCE_Platform_P_DumpRegisterRangeList );

   for ( i = 0; i < astRegisterList->uiCount; i++ )
   {
      unsigned uiAddress = astRegisterList->astRegisters[i].uiAddress + astRegisterList->iInstanceOffset;
      uint64_t uiRegValue;

#if !BDBG_DEBUG_BUILD
      BSTD_UNUSED( uiAddress );
      BSTD_UNUSED( hReg );
#endif
#if BCHP_REGISTER_HAS_64_BIT
     if ( true == astRegisterList->astRegisters[i].bIs64Bit )
     {
        uiRegValue = BREG_Read64( hReg, uiAddress );
     }
     else
     {
#endif
      uiRegValue = BREG_Read32( hReg, uiAddress );
#if BCHP_REGISTER_HAS_64_BIT
     }
#endif
      BDBG_LOG(("@%08x = "BDBG_UINT64_FMT" (%s)", uiAddress, BDBG_UINT64_ARG(uiRegValue), astRegisterList->astRegisters[i].szName));
   }

   BDBG_LEAVE( BVCE_Platform_P_DumpRegisterRangeList );
}

#if ( BVCE_PLATFORM_P_BOX_MODE_SUPPORT != 0 )

/* Taken from "Bit Twiddling Hacks" (http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable) */
const uint8_t BVCE_Platform_P_NumBitsLUT[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

void BVCE_Platform_P_GetTotalChannels(
   BBOX_Handle hBox,
   unsigned uiInstance,
   unsigned *puiTotalChannels
   )
{
   if ( NULL != puiTotalChannels )
   {
      BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
      *puiTotalChannels = BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS;

      if ( NULL != pstBoxConfig )
      {
         if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
         {
            if ( 0 != pstBoxConfig->stVce.uiBoxId )
            {
               unsigned i;
               unsigned uiTotalChannels = 0;

               for ( i = 0; i < BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES; i++ )
               {
                  if ( BVCE_Platform_P_NumBitsLUT[pstBoxConfig->stVce.stInstance[i].uiInstance] == uiInstance )
                  {
                     uiTotalChannels += BVCE_Platform_P_NumBitsLUT[pstBoxConfig->stVce.stInstance[i].uiChannels];
                  }
               }
               *puiTotalChannels = uiTotalChannels;
            }
         }

         BKNI_Free( pstBoxConfig );
      }
   }
}

void BVCE_Platform_P_GetTotalInstances(
   BBOX_Handle hBox,
   unsigned *puiTotalInstances
   )
{
   if ( NULL != puiTotalInstances )
   {
      BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );
      *puiTotalInstances = BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES;

      if ( NULL != pstBoxConfig )
      {
         if ( NULL != hBox )
         {
            if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
            {
               if ( 0 != pstBoxConfig->stVce.uiBoxId )
               {
                  unsigned i,j;
                  unsigned uiTotalInstances = 0;

                  for ( i = 0; i < BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES; i++ )
                  {
                     for ( j = 0; j < BBOX_VCE_MAX_INSTANCE_COUNT; j++ )
                     {
                        if ( ( i == pstBoxConfig->stVce.stInstance[j].uiInstance )
                             && ( 0 != pstBoxConfig->stVce.stInstance[j].uiChannels ) )
                        {
                           uiTotalInstances++;
                           break;
                        }
                     }
                  }
                  *puiTotalInstances = uiTotalInstances;
               }
            }
         }

         BKNI_Free( pstBoxConfig );
      }
   }
}


bool
BVCE_Platform_P_IsInstanceSupported(
   const BBOX_Handle hBox,
   unsigned uiInstance
   )
{
   bool bIsSupported = false;
   BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

   if ( NULL != pstBoxConfig )
   {
      if ( NULL != hBox )
      {
         if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
         {
            if ( 0 != pstBoxConfig->stVce.uiBoxId )
            {
               unsigned j;
               for ( j = 0; j < BBOX_VCE_MAX_INSTANCE_COUNT; j++ )
               {
                  if ( ( uiInstance == pstBoxConfig->stVce.stInstance[j].uiInstance )
                       && ( 0 != pstBoxConfig->stVce.stInstance[j].uiChannels ) )
                  {
                     bIsSupported = true;
                     break;
                  }
               }
            }
         }
      }

      BKNI_Free( pstBoxConfig );
   }

   return bIsSupported;
}

typedef struct BVCE_VideoFormatInfo
{
   BAVC_ScanType eDefaultInputType;
   BAVC_ScanType eDefaultInputTypeMemory;
   struct
   {
      unsigned uiWidth;
      unsigned uiHeight;
      struct
      {
         BAVC_FrameRateCode eDefault;
         BAVC_FrameRateCode eMin;
         BAVC_FrameRateCode eMax;
      } stFrameRate;
   } stBounds[2]; /* Bounds for each BAVC_ScanType. The 1st entry is the default settings */
} BVCE_VideoFormatInfo;

typedef struct BVCE_VideoFormatEntry
{
   BFMT_VideoFmt eVideoFormat;
   BVCE_VideoFormatInfo stInfo;
} BVCE_VideoFormatEntry;

#if ( BVCE_P_CORE_MAJOR < 3 )
static const BVCE_VideoFormatEntry BVCE_VideoFormatLUT[] =
{
   { 0,
      { BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
         {
            { 0, 0, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e30 }, }, /* Bounds[eInterlaced] */
            { 0, 0, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e60 }, }, /* Bounds[eProgressive] */
         }
      }
   },
   { BFMT_VideoFmt_e720p_25Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e25, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e50, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e720p_30Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e720p,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e60, }, },  /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p_25Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e50, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e50, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p_30Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e7_493, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
};
#else
static const BVCE_VideoFormatEntry BVCE_VideoFormatLUT[] =
{
   { 0,
      { BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */\
         {
            { 0, 0, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30 }, }, /* Bounds[eInterlaced] */
            { 0, 0, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60 }, }, /* Bounds[eProgressive] */
         }
      }
   },
   { BFMT_VideoFmt_e720p_25Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e25, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e720p_30Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e720p,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eProgressive, /* Instance[0] */
         {
            { 720, 576, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1280, 720, { BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60, }, },  /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p_25Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e25, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e50, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p_30Hz,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
   { BFMT_VideoFmt_e1080p,
      {
         BAVC_ScanType_eProgressive, BAVC_ScanType_eInterlaced, /* Instance[0] */
         {
            { 1920, 1088, { BAVC_FrameRateCode_e30, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e30, }, }, /* Bounds[eInterlaced] */
            { 1920, 1088, { BAVC_FrameRateCode_e60, BAVC_FrameRateCode_e14_985, BAVC_FrameRateCode_e60, }, }, /* Bounds[eProgressive] */
         }
      },
   },
};
#endif

static
const BVCE_VideoFormatInfo*
BVCE_Platform_S_GetVideoFormatInfo( const BFMT_VideoFmt eVideoFormat )
{
   unsigned i;

   for ( i = 0; i < sizeof(BVCE_VideoFormatLUT)/sizeof(BVCE_VideoFormatEntry); i++ )
   {
      if ( eVideoFormat == BVCE_VideoFormatLUT[i].eVideoFormat )
      {
         return &BVCE_VideoFormatLUT[i].stInfo;
      }
   }

   return NULL;
}

void
BVCE_Platform_P_OverrideChannelDefaultStartEncodeSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
   )
{
   BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

   if ( NULL != pstBoxConfig )
   {
      if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
      {
         if ( 0 != pstBoxConfig->stVce.uiBoxId )
         {
            const BVCE_VideoFormatInfo *pstVideoFormatInfo = BVCE_Platform_S_GetVideoFormatInfo( pstBoxConfig->stVce.stInstance[0].eVideoFormat );
            BDBG_ASSERT( pstVideoFormatInfo );
            {
               BAVC_ScanType eInputType = pstVideoFormatInfo->eDefaultInputType;
               pstChStartEncodeSettings->eInputType = eInputType;
               pstChStartEncodeSettings->stBounds.stDimensions.stMax.uiWidth = pstVideoFormatInfo->stBounds[BAVC_ScanType_eProgressive].uiWidth;
               pstChStartEncodeSettings->stBounds.stDimensions.stMax.uiHeight = pstVideoFormatInfo->stBounds[BAVC_ScanType_eProgressive].uiHeight;
               pstChStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiWidth = pstVideoFormatInfo->stBounds[BAVC_ScanType_eInterlaced].uiWidth;
               pstChStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiHeight = pstVideoFormatInfo->stBounds[BAVC_ScanType_eInterlaced].uiHeight;
               pstChStartEncodeSettings->stBounds.stFrameRate.eMin = pstVideoFormatInfo->stBounds[eInputType].stFrameRate.eMin;
               pstChStartEncodeSettings->stBounds.stFrameRate.eMax = pstVideoFormatInfo->stBounds[eInputType].stFrameRate.eMax;
            }
         }
      }

      BKNI_Free( pstBoxConfig );
   }
}

void
BVCE_Platform_P_OverrideChannelDefaultEncodeSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_EncodeSettings *pstChEncodeSettings
   )
{
   BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

   if ( NULL != pstBoxConfig )
   {
      if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
      {
         if ( 0 != pstBoxConfig->stVce.uiBoxId )
         {
            const BVCE_VideoFormatInfo *pstVideoFormatInfo = BVCE_Platform_S_GetVideoFormatInfo( pstBoxConfig->stVce.stInstance[0].eVideoFormat );
            BDBG_ASSERT( pstVideoFormatInfo );
            {
               BAVC_ScanType eInputType = pstVideoFormatInfo->eDefaultInputType;
               pstChEncodeSettings->stFrameRate.eFrameRate = pstVideoFormatInfo->stBounds[eInputType].stFrameRate.eDefault;
            }
         }
      }

      BKNI_Free( pstBoxConfig );
   }
}

void
BVCE_Platform_P_OverrideChannelDefaultMemoryBoundsSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings
   )
{
   BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

   if ( NULL != pstBoxConfig )
   {
      if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
      {
         if ( 0 != pstBoxConfig->stVce.uiBoxId )
         {
            const BVCE_VideoFormatInfo *pstVideoFormatInfo = BVCE_Platform_S_GetVideoFormatInfo( pstBoxConfig->stVce.stInstance[0].eVideoFormat );
            BDBG_ASSERT( pstVideoFormatInfo );
            {
               BAVC_ScanType eInputType = pstVideoFormatInfo->eDefaultInputTypeMemory;
               pstChMemoryBoundsSettings->eInputType = eInputType;
               pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth = pstVideoFormatInfo->stBounds[eInputType].uiWidth;
               pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight = pstVideoFormatInfo->stBounds[eInputType].uiHeight;
            }
         }
      }

      BKNI_Free( pstBoxConfig );
   }
}

void
BVCE_Platform_P_OverrideChannelDimensionBounds(
   const BBOX_Handle hBox,
   BAVC_ScanType eScanType,
   unsigned *puiWidth,
   unsigned *puiHeight
   )
{
   BBOX_Config *pstBoxConfig = (BBOX_Config *) BKNI_Malloc( sizeof( BBOX_Config ) );

   if ( NULL != pstBoxConfig )
   {
      if ( BERR_SUCCESS == BBOX_GetConfig( hBox, pstBoxConfig ) )
      {
         if ( 0 != pstBoxConfig->stVce.uiBoxId )
         {
            const BVCE_VideoFormatInfo *pstVideoFormatInfo = BVCE_Platform_S_GetVideoFormatInfo( pstBoxConfig->stVce.stInstance[0].eVideoFormat );
            BDBG_ASSERT( pstVideoFormatInfo );
            {
               bool bOverride = false;
               unsigned uiOldWidth = *puiWidth;
               unsigned uiOldHeight = *puiHeight;

               if ( (*puiWidth > pstVideoFormatInfo->stBounds[eScanType].uiWidth )
                    || (*puiWidth == 0 ) )
               {
                  *puiWidth = pstVideoFormatInfo->stBounds[eScanType].uiWidth;
                  bOverride = true;
               }
               if ( ( *puiHeight > pstVideoFormatInfo->stBounds[eScanType].uiHeight )
                    || ( *puiHeight == 0 ) )
               {
                  *puiHeight = pstVideoFormatInfo->stBounds[eScanType].uiHeight;
                  bOverride = true;
               }

               if ( true == bOverride )
               {
                  BDBG_WRN(("Overriding max resolution to match box mode %d: %dx%d%c --> %dx%d%c",
                     pstBoxConfig->stVce.uiBoxId,
                     uiOldWidth, uiOldHeight, (eScanType == BAVC_ScanType_eInterlaced) ? 'i' : 'p',
                     *puiWidth, *puiHeight, (eScanType == BAVC_ScanType_eInterlaced) ? 'i' : 'p'
                     ));
               }
            }
         }
      }

      BKNI_Free( pstBoxConfig );
   }
}

#else
#endif
