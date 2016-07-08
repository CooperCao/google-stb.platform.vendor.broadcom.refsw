/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
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
