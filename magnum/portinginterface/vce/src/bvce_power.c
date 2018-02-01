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
 * [File Description:]
 *
 ***************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "bkni.h"           /* kernel interface */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bvce.h"
#include "bvce_priv.h"

#include "bvce_power.h"

BDBG_MODULE(BVCE_POWER);

#ifdef BCHP_PWR_SUPPORT
static bool
BVCE_Power_S_IsArcAsleep(
   BVCE_Handle hVce,
   unsigned uiArcInstance
   )
{
   if ( ( true == hVce->bBooted )
        && ( false == hVce->bWatchdogOccurred ) )
   {
      uint32_t uiValue;
      /* Check to see if ARC is asleep */
      if ( 0 != hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stSleep.uiAddress )
      {
         uiValue = BREG_Read32( hVce->handles.hReg, hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stSleep.uiAddress );
         /* ARC is asleep if the sleep bit is 1 */
         if ( ( hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stSleep.uiValue & hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stSleep.uiMask ) != ( uiValue & hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stSleep.uiMask ) )
         {
            return false;
         }
      }

      /* Check to see if the Watchdog is enabled */
      if ( 0 != hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stWatchdog.uiAddress )
      {
         uiValue = BREG_Read32( hVce->handles.hReg, hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stWatchdog.uiAddress );
         /* Watchdog is enabled if the watchdog enable bit is 1 */
         if ( ( hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stWatchdog.uiValue & hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stWatchdog.uiMask ) == ( uiValue & hVce->stPlatformConfig.stPower.stCore[uiArcInstance].stWatchdog.uiMask ) )
         {
            return false;
         }
      }
   }
   return true;
}

#endif

void
BVCE_Power_P_AcquireResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      )
{
   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Power_P_AcquireResource );

#ifdef BCHP_PWR_SUPPORT
   if ( 0 != hVce->stPlatformConfig.stPower.astResource[eType].id )
   {
      if ( 0 == hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount )
      {
         /* SW7445-3345: if supported, scale up the clock frequency */
         if ( BVCE_Power_Type_eClock == eType )
         {
            unsigned uiMaxClockRate;
            if ( BERR_SUCCESS == BCHP_PWR_GetMaxClockRate( hVce->handles.hChp, hVce->stPlatformConfig.stPower.astResource[eType].id, &uiMaxClockRate ) )
            {
               BCHP_PWR_SetClockRate( hVce->handles.hChp, hVce->stPlatformConfig.stPower.astResource[eType].id, uiMaxClockRate );
            }
         }

         BCHP_PWR_AcquireResource(
               hVce->handles.hChp,
               hVce->stPlatformConfig.stPower.astResource[eType].id
               );
      }
   }
#endif

   if ( ( 0 == hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount )
        && ( BVCE_Power_Type_eClock == eType ) )
   {
      BVCE_P_EnableInterrupts( hVce, true );
   }

   hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount++;

   BDBG_LEAVE( BVCE_Power_P_AcquireResource );
}

void
BVCE_Power_P_ReleaseResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      )
{
   BDBG_ENTER( BVCE_Power_P_ReleaseResource );

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ASSERT( 0 != hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount );
   hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount--;

#ifdef BCHP_PWR_SUPPORT
   if ( ( 0 == hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount )
        && ( BVCE_Power_Type_eClock == eType ) )
   {
      BVCE_P_EnableInterrupts( hVce, false );
   }

   if ( 0 != hVce->stPlatformConfig.stPower.astResource[eType].id )
   {
      if ( 0 == hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount )
      {
         /* Halt the ARC before shutting down clock */
         if ( BVCE_Power_Type_eClock == eType )
         {
            unsigned i;

            /* Wait for ARCs to be asleep */
            for ( i = 0; i < BVCE_PLATFORM_P_NUM_ARC_CORES; i++ )
            {
               while ( false == BVCE_Power_S_IsArcAsleep( hVce, i ) )
               {
                  BKNI_Sleep(10);
               }
            }
         }

         BCHP_PWR_ReleaseResource(
               hVce->handles.hChp,
               hVce->stPlatformConfig.stPower.astResource[eType].id
               );

         /* SW7445-3345: if supported, scale down the clock frequency */
         if ( BVCE_Power_Type_eClock == eType )
         {
            unsigned uiMinClockRate;
            if ( BERR_SUCCESS == BCHP_PWR_GetMinClockRate( hVce->handles.hChp, hVce->stPlatformConfig.stPower.astResource[eType].id, &uiMinClockRate ) )
            {
               BCHP_PWR_SetClockRate( hVce->handles.hChp, hVce->stPlatformConfig.stPower.astResource[eType].id, uiMinClockRate );
            }
         }
      }
   }
#endif

   BDBG_LEAVE( BVCE_Power_P_ReleaseResource );
}

unsigned
BVCE_Power_P_QueryResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      )
{
   unsigned uiReferenceCount = 0;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Power_P_QueryResource );

   uiReferenceCount = hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount;

   BDBG_LEAVE( BVCE_Power_P_QueryResource );

   return uiReferenceCount;
}

void
BVCE_Power_P_ReleaseAllResources(
      BVCE_Handle hVce
      )
{
   BVCE_Power_Type eType;

   BDBG_OBJECT_ASSERT(hVce, BVCE_P_Context);
   BDBG_ENTER( BVCE_Power_P_ReleaseAllResources );

   BVCE_P_EnableInterrupts( hVce, false );

   for ( eType = 0; eType < BVCE_Power_Type_eMax; eType++ )
   {
#ifdef BCHP_PWR_SUPPORT
      if ( ( 0 != hVce->stPlatformConfig.stPower.astResource[eType].id )
            && ( 0 != hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount ) )
      {
         BCHP_PWR_ReleaseResource(
               hVce->handles.hChp,
               hVce->stPlatformConfig.stPower.astResource[eType].id
               );
      }
#endif

      hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount = 0;
   }

   BDBG_LEAVE( BVCE_Power_P_ReleaseAllResources );
}
