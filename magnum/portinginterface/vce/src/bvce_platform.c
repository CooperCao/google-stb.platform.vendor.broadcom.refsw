/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
   uint32_t uiRegValue;

   BDBG_ENTER( BVCE_P_WriteRegisterList );

   for ( i = 0; i < uiCount; i++ )
   {
      uiRegValue = BREG_Read32(
               hReg,
               astRegisterList[i].uiAddress
               );

      uiRegValue &= ~astRegisterList[i].uiMask;
      uiRegValue |= astRegisterList[i].uiValue & astRegisterList[i].uiMask;

      BREG_Write32(
               hReg,
               astRegisterList[i].uiAddress,
               uiRegValue
               );

#if BDBG_DEBUG_BUILD
      {
        uint32_t uiRegValueActual = BREG_Read32(
                                                hReg,
                                                astRegisterList[i].uiAddress
                                                );

        BDBG_MSG(("@0x%08x <-- 0x%08x (0x%08x) - %s",
                  astRegisterList[i].uiAddress,
                  uiRegValue,
                  uiRegValueActual,
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

#if !BDBG_DEBUG_BUILD
      BSTD_UNUSED( uiAddress );
      BSTD_UNUSED( hReg );
#endif
      BDBG_LOG(("@%08x = %08x (%s)", uiAddress, BREG_Read32( hReg, uiAddress ), astRegisterList->astRegisters[i].szName));
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
            BAVC_ScanType eInputType = pstBoxConfig->stVce.stInstance[0].eDefaultInputType;
            pstChStartEncodeSettings->eInputType = eInputType;
            pstChStartEncodeSettings->stBounds.stDimensions.stMax.uiWidth = pstBoxConfig->stVce.stInstance[0].stBounds[BAVC_ScanType_eProgressive].uiWidth;
            pstChStartEncodeSettings->stBounds.stDimensions.stMax.uiHeight = pstBoxConfig->stVce.stInstance[0].stBounds[BAVC_ScanType_eProgressive].uiHeight;
            pstChStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiWidth = pstBoxConfig->stVce.stInstance[0].stBounds[BAVC_ScanType_eInterlaced].uiWidth;
            pstChStartEncodeSettings->stBounds.stDimensions.stMaxInterlaced.uiHeight = pstBoxConfig->stVce.stInstance[0].stBounds[BAVC_ScanType_eInterlaced].uiHeight;
            pstChStartEncodeSettings->stBounds.stFrameRate.eMin = pstBoxConfig->stVce.stInstance[0].stBounds[eInputType].stFrameRate.eMin;
            pstChStartEncodeSettings->stBounds.stFrameRate.eMax = pstBoxConfig->stVce.stInstance[0].stBounds[eInputType].stFrameRate.eMax;
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
            BAVC_ScanType eInputType = pstBoxConfig->stVce.stInstance[0].eDefaultInputType;
            pstChEncodeSettings->stFrameRate.eFrameRate = pstBoxConfig->stVce.stInstance[0].stBounds[eInputType].stFrameRate.eDefault;
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
            BAVC_ScanType eInputType = pstBoxConfig->stVce.stInstance[0].eDefaultInputTypeMemory;
            pstChMemoryBoundsSettings->eInputType = eInputType;
            pstChMemoryBoundsSettings->stDimensions.stMax.uiWidth = pstBoxConfig->stVce.stInstance[0].stBounds[eInputType].uiWidth;
            pstChMemoryBoundsSettings->stDimensions.stMax.uiHeight = pstBoxConfig->stVce.stInstance[0].stBounds[eInputType].uiHeight;
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
            bool bOverride = false;
            unsigned uiOldWidth = *puiWidth;
            unsigned uiOldHeight = *puiHeight;

            if ( (*puiWidth > pstBoxConfig->stVce.stInstance[0].stBounds[eScanType].uiWidth )
                 || (*puiWidth == 0 ) )
            {
               *puiWidth = pstBoxConfig->stVce.stInstance[0].stBounds[eScanType].uiWidth;
               bOverride = true;
            }
            if ( ( *puiHeight > pstBoxConfig->stVce.stInstance[0].stBounds[eScanType].uiHeight )
                 || ( *puiHeight == 0 ) )
            {
               *puiHeight = pstBoxConfig->stVce.stInstance[0].stBounds[eScanType].uiHeight;
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

      BKNI_Free( pstBoxConfig );
   }
}

#else
#endif
