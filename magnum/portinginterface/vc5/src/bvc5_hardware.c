/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/

#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_registers_priv.h"
#include "bvc5_null_priv.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#ifdef BVC5_HARDWARE_REAL
#include "bchp_sun_top_ctrl.h"
#include "bchp_clkgen.h"
#ifdef V3D_HAS_BPCM
#include "bchp_v3d_bpcm.h"
#endif
#include "bchp_v3d_gmp.h"
#endif

#include "bmma_system.h"

BDBG_MODULE(BVC5_P);

#define USE_GMP_MONITORING 0  /* Not on by default */

/* TODO : This will come from the build system based on what RDB includes it finds */
/* TODO : It might also be possible to read a config register                      */
#define BVC5_P_NUM_CORES   1

/* These bits are defined in bcm_sched_job.h */
#define TFU_OUTPUT_DISABLE_MAIN_TEXTURE (1<<0)
#define TFU_OUTPUT_USECOEF (1<<1)

#define TFU_INPUT_FLIPY (1<<0)
#define TFU_INPUT_SRGB (1<<1)

static void BVC5_P_HardwareResetV3D(BVC5_Handle hVC5);

uint32_t BVC5_P_ReadRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg
)
{
#if defined(BVC5_HARDWARE_NONE)
   return BVC5_P_NullReadRegister(hVC5, uiCoreIndex, uiReg);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   return BVC5_P_SimpenroseReadRegister(hVC5, uiCoreIndex, uiReg);
#else
   return BREG_Read32(hVC5->hReg, uiReg + hVC5->psCoreStates[uiCoreIndex].uiRegOffset);
#endif
}

void BVC5_P_WriteRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg,
   uint32_t     uiValue
)
{
#if defined(BVC5_HARDWARE_NONE)
   BVC5_P_NullWriteRegister(hVC5, uiCoreIndex, uiReg, uiValue);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   BVC5_P_SimpenroseWriteRegister(hVC5, uiCoreIndex, uiReg, uiValue);
#else
   BREG_Write32(hVC5->hReg, uiReg + hVC5->psCoreStates[uiCoreIndex].uiRegOffset, uiValue);
#endif
}

/* BVC5_P_ReadRegister_isr
 *
 * Used in ISR context only
 */
static uint32_t BVC5_P_ReadRegister_isr(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg
)
{
#if defined(BVC5_HARDWARE_NONE)
   return BVC5_P_NullReadRegister(hVC5, uiCoreIndex, uiReg);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   return BVC5_P_SimpenroseReadRegister(hVC5, uiCoreIndex, uiReg);
#else
   return BREG_Read32_isr(hVC5->hReg, uiReg + hVC5->psCoreStates[uiCoreIndex].uiRegOffset);
#endif
}

/* BVC5_P_WriteRegister_isr
 *
 * Used in ISR context only
 */
static void BVC5_P_WriteRegister_isr(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg,
   uint32_t     uiValue
)
{
#if defined(BVC5_HARDWARE_NONE)
   BVC5_P_NullWriteRegister(hVC5, uiCoreIndex, uiReg, uiValue);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   BVC5_P_SimpenroseWriteRegister(hVC5, uiCoreIndex, uiReg, uiValue);
#else
   BREG_Write32_isr(hVC5->hReg, uiReg + hVC5->psCoreStates[uiCoreIndex].uiRegOffset, uiValue);
#endif
}

void BVC5_P_ClearL3Cache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   )
{
   /* L3 cache reset */
   uint32_t uiCacheControl = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_GCA_CACHE_CTRL);
   uiCacheControl &= ~(BCHP_MASK(V3D_GCA_CACHE_CTRL, FLUSH));
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_GCA_CACHE_CTRL, uiCacheControl | BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, FLUSH, 1));

   /* while ((BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_GCA_CACHE_STATUS) & BCHP_V3D_GCA_CACHE_STATUS_IDLE_MASK) == 0)
      ; */

   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_GCA_CACHE_CTRL, uiCacheControl | BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, FLUSH, 0));
}

void BVC5_P_FlushTextureCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
)
{
   /* Clean dirty lines and invalidate */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLSTA, 0);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLEND, ~0);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL, 1); /* Flush */
}

void BVC5_P_CleanTextureCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
)
{
   /* Clean dirty lines only */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLSTA, 0);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLEND, ~0);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL, 5); /* Clean */
   while ((BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL) & 1) != 0)
      ;
}

void BVC5_P_ClearL2Cache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
)
{
   uint32_t     uiReg;

   uiReg = BCHP_FIELD_DATA(V3D_CTL_L2CACTL, L2CCLR, 1) |
           BCHP_FIELD_DATA(V3D_CTL_L2CACTL, L2CENA, 1);
   /* This is a read-only cache, so invalidating is harmless */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2CACTL, uiReg);
}

void BVC5_P_ClearSlicesCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
)
{
   uint32_t     uiReg;

   BDBG_ASSERT(!BVC5_P_HardwareCacheClearBlocked(hVC5, uiCoreIndex));

   uiReg = BCHP_FIELD_DATA(V3D_CTL_SLCACTL, TVCCS0_to_TVCCS3, 0xF) |
           BCHP_FIELD_DATA(V3D_CTL_SLCACTL, TDCCS0_to_TDCCS3, 0xF) |
           BCHP_FIELD_DATA(V3D_CTL_SLCACTL, UCCS0_to_UCCS3, 0xF) |
           BCHP_FIELD_DATA(V3D_CTL_SLCACTL, ICCS0_to_ICCS3, 0xF);
   /* All slice caches contain only read data */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_SLCACTL, uiReg);
}

void BVC5_P_UpdateShadowCounters(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   if (hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
      /* capture the counters before taking away the power, only bother if any are on */
      if (hVC5->sPerfCounters.uiPCTREShadow)
      {
         uint32_t c;
         for (c = 0; c < BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE; c++)
            hVC5->sPerfCounters.uiPCTRShadows[c] += BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_PCTR_PCTR0 + (8 * c));
      }

      if (hVC5->sPerfCounters.uiActiveBwCounters > 0)
      {
         hVC5->sPerfCounters.uiMemBwCntShadow += BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_GCA_MEM_BW_CNT);
      }
   }
}

static void BVC5_P_WaitForAXIIdle(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   )
{
   uint32_t uiQuiet;

   /* Wait until V3D has no active or pending AXI transactions */
   uiQuiet = BCHP_FIELD_DATA(V3D_GMP_GMPCS, AWBURST, 1) |
             BCHP_FIELD_DATA(V3D_GMP_GMPCS, AWOST, 1)   |
             BCHP_FIELD_DATA(V3D_GMP_GMPCS, AWBWAIT, 1) |
             BCHP_FIELD_DATA(V3D_GMP_GMPCS, AROST, 1)   |
             BCHP_FIELD_DATA(V3D_GMP_GMPCS, ARDWAIT, 1) |
             BCHP_FIELD_DATA(V3D_GMP_GMPCS, LDBUSY, 1);

   while (BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCS) & uiQuiet)
   {
      BKNI_Printf("AXI WAS NOT IDLE\n");
      continue;
   }
}

void BVC5_P_HardwareBPCMPowerUp(
   BVC5_Handle hVC5
   )
{
#ifdef V3D_HAS_BPCM
   /* Power-up V3D */
   uint32_t uiSuccess, uiMask;
   uint32_t uiZoneControl;

   uiZoneControl = BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_DPG_CNTL_EN, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_UP_REQ, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_BLK_RST_ASSERT, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_MEM_PWR_CNTL_EN, 1);

   BREG_Write32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL, uiZoneControl);

   uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);

   uiSuccess = BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_MEM_PWR_STATE) |
      BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_DPG_PWR_STATE) |
      BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_ON_STATE);

   uiMask = BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_ISO_STATE) |
      BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_MEM_PWR_STATE) |
      BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_DPG_PWR_STATE) |
      BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_ON_STATE);

   uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);

   while ((uiZoneControl & uiMask) != uiSuccess)
      uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);
#else
   BSTD_UNUSED(hVC5);
#endif
}

void BVC5_P_HardwareBPCMPowerDown(
   BVC5_Handle hVC5
   )
{
#ifdef V3D_HAS_BPCM
   /* Power down V3D */
   uint32_t uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);

   /* Check PWR_ON_STATE bit */
   if (BCHP_GET_FIELD_DATA(uiZoneControl, V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_ON_STATE))
   {
      uint32_t uiSuccess, uiMask;

      uiZoneControl = BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_DPG_CNTL_EN, 1) |
         BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_DN_REQ, 1) |
         BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_MEM_PWR_CNTL_EN, 1);

      BREG_Write32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL, uiZoneControl);

      uiSuccess = BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_ISO_STATE) |
         BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_OFF_STATE);

      uiMask = BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_ISO_STATE) |
         BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_MEM_PWR_STATE) |
         BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_DPG_PWR_STATE) |
         BCHP_MASK(V3D_BPCM_PWR_ZONE_0_CONTROL, ZONE_PWR_OFF_STATE);

      uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);
      while ((uiZoneControl & uiMask) != uiSuccess)
         uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);
   }
#else
   BSTD_UNUSED(hVC5);
#endif
}

static void BVC5_P_HardwareTurnOff(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   if (hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
      BVC5_P_UpdateShadowCounters(hVC5, uiCoreIndex);

      BKNI_EnterCriticalSection();

      /* ATOMIC with IRQ which can read this value.  Every other access is in
         same thread */
      __sync_fetch_and_and(&hVC5->psCoreStates[uiCoreIndex].bPowered, 0);

      BKNI_LeaveCriticalSection();

      /* In theory we are supposed to wait for AXI idle before resetting (or powering off).
       * We've never found this to be necessary however, so we'll avoid a potential spin by
       * not doing this. The code is here for reference.
      BVC5_P_WaitForAXIIdle(hVC5, uiCoreIndex);
      */

      BVC5_P_HardwareResetV3D(hVC5);

#ifdef BCHP_PWR_SUPPORT
      BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
      BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
#endif

      if (hVC5->sOpenParams.bUsePowerGating)
         BVC5_P_HardwareBPCMPowerDown(hVC5);

      if (hVC5->sPerfCounters.bCountersActive)
         BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiPoweredOffStartTime);
   }
}

void BVC5_P_RestorePerfCounters(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   if (hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_PCTR_PCTRC, 0xFFFF);
      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_GCA_PM_CTRL, 1);     /* Holds the counters in reset until de-asserted */

      /* re-write the performance monitor values/re-enable */
      if (hVC5->sPerfCounters.uiPCTREShadow)
      {
         uint32_t c;
         for (c = 0; c < BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE; c++)
            BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_PCTR_PCTRS0 + (c * 8), hVC5->sPerfCounters.uiPCTRSShadow[c]);
         BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_PCTR_PCTRE, hVC5->sPerfCounters.uiPCTREShadow);
      }

      if (hVC5->sPerfCounters.uiActiveBwCounters > 0)
      {
         /* Must also clear the bottom two bits to enable the counters */
         BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_GCA_PM_CTRL, hVC5->sPerfCounters.uiGCAPMSelShadow & (~3));
      }
   }
}

static bool BVC5_P_InitGMPTable(
   BVC5_Handle hVC5,
   uint32_t    startOfViolationRange,
   uint32_t    endOfViolationRange
   )
{
   BMMA_AllocationSettings pSettings;
   uint8_t                *ptr;
   const uint32_t          size       = 8 * 1024;
   const uint32_t          byteRegion = 512 * 1024;
   int32_t                 sByte, eByte;

   if (hVC5->hGMPTable != NULL)
      return true;

   BMMA_GetDefaultAllocationSettings(&pSettings);
   hVC5->hGMPTable = BMMA_Alloc(hVC5->hMMAHeap, size, 256, &pSettings);
   if (hVC5->hGMPTable == NULL)
      return false;

   ptr = BMMA_Lock(hVC5->hGMPTable);
   if (ptr == NULL)
   {
      BMMA_Free(hVC5->hGMPTable);
      return false;
   }

   hVC5->uiGMPOffset = BMMA_LockOffset(hVC5->hGMPTable);
   if (hVC5->uiGMPOffset == 0)
   {
      BMMA_Free(hVC5->hGMPTable);
      return false;
   }

   /* Fill the GMP table to protect nothing */
   BKNI_Memset(ptr, 0xFF, size);

   /* Note: we only fill the table a 512KB granularity. This can be changed to 128KB granularity
    * with some work to set the bits correctly. */
   sByte = startOfViolationRange / byteRegion;
   eByte = endOfViolationRange   / byteRegion;

   /* Fill the 512K regions from start to end for the protected region */
   BKNI_Memset(ptr + sByte, 0x0, eByte - sByte + 1);

   BMEM_Heap_FlushCache(hVC5->hHeap, 0, ~0);

   BMMA_Unlock(hVC5->hGMPTable, ptr);

   return true;
}

static void BVC5_P_SetupGMP_isr(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   )
{
   uint32_t  uiMask;

   if (!BVC5_P_InitGMPTable(hVC5, 0, 2 * 1024 * 1024 - 1))  /* Check first 2MB of memory for bad access */
      return;  /* Failed */

   /* Wait until the GMP can accept a reset */
   BVC5_P_WaitForAXIIdle(hVC5, uiCoreIndex);

   /* Reset the GMP */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCS, BCHP_FIELD_DATA(V3D_GMP_GMPCS, GMPRST, 1));

   /* Enable the GMP violation interrupt bit */
   uiMask = BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, OUTOMEM, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FLDONE, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FRDONE, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, GMPV, 1);
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_CLR, uiMask);

   /* Set the address of the protection table */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPPTA, (uint32_t)hVC5->uiGMPOffset);

   /* Load the entire table */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCLD, BCHP_FIELD_DATA(V3D_GMP_GMPCLD, CLDLBITS, 0xFFFFFFFF));

   while (BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCS) & BCHP_FIELD_DATA(V3D_GMP_GMPCS, LDBUSY, 1))
      continue;

   /* Turn it all on */

   /* Protection must only be enabled or disabled when in a force stall (FSTALL of GMPCFG set to 1)
    * and no AXI requests are present, no AXI responses are still pending and no AXI out buses are stalled
    * (AWQBUSY, AWOST, AWBWAIT, ARQBUSY, AROST, ARDWAIT of GMPCS all 0) and protection bits are not being
    * loaded(LDBUSY of GMPCS 0). When this point has been reached there are no incomplete transactions or
    * pending responses on either side of the GMP.
    */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCFG, BCHP_FIELD_DATA(V3D_GMP_GMPCFG, FSTALL, 1));

   BVC5_P_WaitForAXIIdle(hVC5, uiCoreIndex);

   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCFG,
                            BCHP_FIELD_DATA(V3D_GMP_GMPCFG, ENABLE, 1) |
                            BCHP_FIELD_DATA(V3D_GMP_GMPCFG, FSTALL, 0));
}

static void BVC5_P_PrintGMPViolation_isr(
   uint32_t    addr,
   uint32_t    id,
   bool        write
   )
{
   BKNI_Printf("**** V3D GMP %s VIOLATION AT 0x%08X FROM ", write ? "WRITE" : "READ", addr);

   if (id == 0xff)
      BKNI_Printf("GMP");
   else
   {
      switch (id >> 5)
      {
      case 0:  BKNI_Printf("L2C"); break;
      case 1:  BKNI_Printf("CLE"); break;
      case 2:  BKNI_Printf("PTB"); break;
      case 3:  BKNI_Printf("PSE"); break;
      case 4:  BKNI_Printf("VCD"); break;
      case 5:  BKNI_Printf("VDW"); break;
      case 6:  BKNI_Printf("L2T"); break;
      case 7:  BKNI_Printf("TLB"); break;
      case 8:  BKNI_Printf("TFU"); break;
      }
   }
   BKNI_Printf(" ****\n");
}

static void BVC5_P_HandleGMPViolation_isr(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   )
{
   uint32_t status;
   uint32_t addr;
   uint32_t pvrt;
   uint32_t id;

   status = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPCS);

   if (status & BCHP_FIELD_DATA(V3D_GMP_GMPCS, RDVIO, 1))
   {
      addr = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPRVA);
      pvrt = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPRVT);
      id   = BCHP_GET_FIELD_DATA(pvrt, V3D_GMP_GMPRVT, RDVIOID);

      BVC5_P_PrintGMPViolation_isr(addr, id, false);
   }

   if (status & BCHP_FIELD_DATA(V3D_GMP_GMPCS, WRVIO, 1))
   {
      addr = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPWVA);
      pvrt = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_GMPWVT);
      id = BCHP_GET_FIELD_DATA(pvrt, V3D_GMP_GMPWVT, WRVIOID);

      BVC5_P_PrintGMPViolation_isr(addr, id, true);
   }

   /* Reset the GMP */
   BVC5_P_SetupGMP_isr(hVC5, uiCoreIndex);
}

static void BVC5_P_SetDefaultRegisterState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   uint32_t uiIntMask;
   uint32_t uiCacheId;
   uint32_t uiLowPriId;

   /* Disable and clear interrupts from this core */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_SET, 0xFFFFFFFF);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR, 0xFFFFFFFF);

   uiIntMask = BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, OUTOMEM, 1) |
               BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FLDONE, 1) |
               BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FRDONE, 1);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_CLR, uiIntMask);

   /* Client IDs : 0=L2C, 1=CLE, 2=PTB, 3=PSE, 4=VC4, 5=VDW, 6=L2T, 7=TLB */
   uiCacheId = BCHP_FIELD_DATA(V3D_GCA_CACHE_ID, CACHE_ID_EN, 0x41) | /* Bypass the L3C for L2C & L2T data */
               BCHP_FIELD_DATA(V3D_GCA_CACHE_ID, INVERT_FUNCTION, 1);
   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_GCA_CACHE_ID, uiCacheId);

   uiLowPriId = BCHP_FIELD_DATA(V3D_GCA_LOW_PRI_ID, LOW_PRI_ID_EN, 0x7F) | /* High-priority for all except TLB and TFU */
                BCHP_FIELD_DATA(V3D_GCA_LOW_PRI_ID, INVERT_FUNCTION, 1);
   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_GCA_LOW_PRI_ID, uiLowPriId);

#if !((BCHP_CHIP == 7250) || (BCHP_CHIP == 7439))
   /* Set up OVRTMUOUT mode (this doesn't work in v3dv3.2) */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_MISCCFG, 1);
#endif

#if USE_GMP_MONITORING
   BVC5_P_SetupGMP_isr(hVC5, uiCoreIndex);
#endif

   {
      BVC5_P_RenderState  *renderState = BVC5_P_HardwareGetRenderState(hVC5, uiCoreIndex);

      /* Clear the render frame count to 0 */
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_RFC, 1);
      renderState->uiLastRFC = 0;
   }
}

static void BVC5_P_HardwareResetV3D(
   BVC5_Handle hVC5
)
{
   /* Request shutdown and wait for GCA so SCB/MCP traffic is correctly handled prior to sw_init */
   BREG_Write32(hVC5->hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN,
                             BCHP_FIELD_DATA(V3D_GCA_SAFE_SHUTDOWN, SAFE_SHUTDOWN_EN,  1));

   while (BREG_Read32(hVC5->hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN_ACK) != 0x3)
      ;

   BREG_Write32(hVC5->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
   BREG_Write32(hVC5->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));
}

/* This should only be called when you know the core is powered - e.g. from a lockup condition */
void BVC5_P_HardwareResetCoreAndState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   BVC5_P_HardwareResetV3D(hVC5);
   BVC5_P_SetDefaultRegisterState(hVC5, uiCoreIndex);
}

static void BVC5_P_HardwareTurnOn(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   if (!hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
#ifdef BCHP_PWR_SUPPORT
      BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
      BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
#endif

      if (hVC5->sOpenParams.bUsePowerGating)
      {
         /* Power-up V3D */
         BVC5_P_HardwareBPCMPowerUp(hVC5);
      }

      if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiPoweredOffStartTime != 0)
      {
         uint64_t now;
         BVC5_P_GetTime_isrsafe(&now);
         hVC5->sPerfCounters.uiPoweredCumOffTime += now - hVC5->sPerfCounters.uiPoweredOffStartTime;
         hVC5->sPerfCounters.uiPoweredOffStartTime = 0;
      }

      /* Also reset V3D after turning on power. */
      BVC5_P_HardwareResetV3D(hVC5);

      /* ATOMIC with IRQ which can read this value.  Every other access is in
         same thread */
      __sync_fetch_and_or(&hVC5->psCoreStates[uiCoreIndex].bPowered, 1);

      BVC5_P_SetDefaultRegisterState(hVC5, uiCoreIndex);
      BVC5_P_RestorePerfCounters(hVC5, uiCoreIndex);
      BVC5_P_HardwareResetWatchdog(hVC5, uiCoreIndex);
   }
}

void BVC5_P_HardwareInitializeCoreStates(
   BVC5_Handle hVC5
)
{
   uint32_t uiCoreIndex;

   hVC5->uiNumCores = BVC5_P_NUM_CORES;

   for (uiCoreIndex = 0; uiCoreIndex < hVC5->uiNumCores; ++uiCoreIndex)
   {
      BVC5_P_CoreState  *psCoreState   = &hVC5->psCoreStates[uiCoreIndex];

      BKNI_Memset(psCoreState, 0, sizeof(*psCoreState));

      /* Setup register offsets */
      if (uiCoreIndex == 0)
         hVC5->psCoreStates[uiCoreIndex].uiRegOffset = 0;
      else
         hVC5->psCoreStates[uiCoreIndex].uiRegOffset = 0; /* TODO : when multi-core RDB is available */
   }
}

bool BVC5_P_HardwareIsUnitAvailable(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex,
   BVC5_P_HardwareUnitType eHardwareType
)
{
   uint32_t uiPos = hVC5->sOpenParams.bNoQueueAhead ? BVC5_P_HW_QUEUE_RUNNING : BVC5_P_HW_QUEUE_QUEUED;

   switch (eHardwareType)
   {
   case BVC5_P_HardwareUnit_eBinner   : return hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob == NULL;
   case BVC5_P_HardwareUnit_eRenderer : return hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[uiPos] == NULL;
   case BVC5_P_HardwareUnit_eTFU      : return hVC5->sTFUState.psJob == NULL;
   }
   return true;
}

BVC5_P_HardwareUnitType BVC5_P_HardwareIsUnitStalled(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   /* TODO: multi-core */
   BVC5_P_CoreState        *psCoreState   = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_P_BinnerState      *psBinnerState = &psCoreState->sBinnerState;
   BVC5_P_RenderState      *psRenderState = &psCoreState->sRenderState;
   BVC5_P_HardwareUnitType  uiUnits       = 0;

   if (!hVC5->sOpenParams.bUseStallDetection)
      return 0;

   /* Is a bin job running? */
   if (psBinnerState->psJob != NULL)
   {
      uint32_t uiBinAddr  = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0CA);
      uint32_t uiBinEnd   = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0EA);
      uint32_t uiPcs      = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_PCS);
      bool     bBinActive = (BCHP_GET_FIELD_DATA(uiPcs, V3D_CLE_PCS, BMACTIVE) != 0);

      /* Has it stalled? */
      if (uiBinAddr == psBinnerState->uiPrevAddr)
      {
         bool stalled = false;

         if (uiBinAddr != uiBinEnd)
            stalled = true;

         if (uiBinAddr == uiBinEnd && bBinActive)
            stalled = true;

         if (stalled)
            uiUnits |= BVC5_P_HardwareUnit_eBinner;
      }

      /* Record for next time */
      psBinnerState->uiPrevAddr = uiBinAddr;
   }

   /* Is a render job running? */
   if (psRenderState->psJob[BVC5_P_HW_QUEUE_RUNNING] != NULL)
   {
      uint32_t uiRenderAddr  = BVC5_P_ReadRegister(hVC5, uiCoreIndex,  BCHP_V3D_CLE_CT1CA);
      uint32_t uiRenderEnd   = BVC5_P_ReadRegister(hVC5, uiCoreIndex,  BCHP_V3D_CLE_CT1EA);
      uint32_t uiPcs         = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_PCS);
      bool     bRdrActive    = (BCHP_GET_FIELD_DATA(uiPcs, V3D_CLE_PCS, RMACTIVE) != 0);

      if (uiRenderAddr == psRenderState->uiPrevAddr)
      {
         bool stalled = false;

         if (uiRenderAddr != uiRenderEnd)
            stalled = true;

         if (uiRenderAddr == uiRenderEnd && bRdrActive)
            stalled = true;

         if (stalled)
            uiUnits |= BVC5_P_HardwareUnit_eRenderer;
      }

      /* Record for next time */
      psRenderState->uiPrevAddr = uiRenderAddr;
   }

   /* TODO: Can TFU stall?  How to detect? */

   return uiUnits;
}

void BVC5_P_HardwarePowerAcquire(
   BVC5_Handle hVC5,
   uint32_t    uiCores
)
{
   uint32_t i;

   for (i = 0; i < hVC5->uiNumCores; ++i)
   {
      if ((uiCores & (1 << i)) != 0)
      {
         BVC5_P_CoreState  *psCoreState = &hVC5->psCoreStates[i];

         if (psCoreState->uiPowerOnCount == 0)
            BVC5_P_HardwareTurnOn(hVC5, i);

         psCoreState->uiPowerOnCount++;
      }
   }
}

void BVC5_P_HardwarePowerRelease(
   BVC5_Handle hVC5,
   uint32_t    uiCores
)
{
   uint32_t i;

   for (i = 0; i < hVC5->uiNumCores; ++i)
   {
      if ((uiCores & (1 << i)) != 0)
      {
         BVC5_P_CoreState  *psCoreState = &hVC5->psCoreStates[i];

         if (psCoreState->uiPowerOnCount > 0)
            psCoreState->uiPowerOnCount--;

         if (psCoreState->uiPowerOnCount == 0)
            BVC5_P_HardwareTurnOff(hVC5, i);
      }
   }
}

void BVC5_P_HardwareResetWatchdog(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   /* TODO: multi-core */
   BVC5_P_CoreState        *psCoreState   = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_P_BinnerState      *psBinnerState = &psCoreState->sBinnerState;
   BVC5_P_RenderState      *psRenderState = &psCoreState->sRenderState;

   psCoreState->uiTimeoutCount    = 0;
   psBinnerState->uiPrevAddr      = 0;
   psRenderState->uiPrevAddr      = 0;
}

static void BVC5_P_HardwareFakeInterrupt(
   BVC5_Handle hVC5,
   uint32_t    uiReason
)
{
   /* BKNI_Printf("Fake Interrupt %x\n", uiReason); */

   __sync_fetch_and_or(&hVC5->uiInterruptReason, uiReason);
   if (hVC5->uiInterruptReason != 0)
      BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
}

static void BVC5_P_HardwareFakeHubInterrupt(
   BVC5_Handle hVC5,
   uint32_t    uiReason
   )
{
   /* BKNI_Printf("Fake Hub Interrupt %x\n", uiReason); */

   __sync_fetch_and_or(&hVC5->uiTFUInterruptReason, uiReason);
   if (hVC5->uiTFUInterruptReason != 0)
      BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
}

static bool BVC5_P_IssueBinJob(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex,
   BVC5_P_InternalJob     *psJob
)
{
   uint32_t               uiPhysical;
   BVC5_BinBlockHandle    pvAddr;
   BVC5_JobBin           *pBinJob;
   BVC5_P_InternalJob    *psRenderJob;
   BVC5_P_BinnerState    *pBinnerState = &hVC5->psCoreStates[uiCoreIndex].sBinnerState;

   BDBG_ASSERT(psJob != NULL);

   pBinJob = (BVC5_JobBin *)psJob->pBase;

   BDBG_ASSERT(pBinJob->uiStart[uiCoreIndex] != 0);
   BDBG_ASSERT(pBinJob->uiEnd[uiCoreIndex] != 0);
   BDBG_ASSERT(pBinJob->uiStart[uiCoreIndex] != pBinJob->uiEnd[uiCoreIndex]);

   psRenderJob = psJob->jobData.sBin.psInternalRenderJob;

   if (!psJob->bAbandon && psRenderJob != NULL)
   {
      pvAddr = BVC5_P_BinMemArrayAdd(&psRenderJob->jobData.sRender.sBinMemArray, hVC5->hBinPool,
                                     psJob->jobData.sBin.uiMinInitialBinBlockSize, &uiPhysical);

      if (pvAddr == NULL)
         return false;     /* Couldn't get enough bin memory */

      pBinnerState->psJob = psJob;

      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_JOBS_SUBMITTED, 1);

      BDBG_ASSERT(uiPhysical != 0);

      /* Clear BPOS to ensure we can't use left over bin memory */
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOS, 0);

      /* Setup the memory for this bin job and kick it off */
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMA, uiPhysical);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMS, pvAddr->uiNumBytes);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QBA, pBinJob->uiStart[uiCoreIndex]);

      if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiBinnerIdleStartTime != 0)
      {
         uint64_t now;
         BVC5_P_GetTime_isrsafe(&now);
         hVC5->sPerfCounters.uiBinnerCumIdleTime += now - hVC5->sPerfCounters.uiBinnerIdleStartTime;
         hVC5->sPerfCounters.uiBinnerIdleStartTime = 0;
      }

      BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING,
                             BVC5_EventBegin, psJob);

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QEA, pBinJob->uiEnd[uiCoreIndex]);

      /* The binner is off and running, so now reallocate memory. */
      BVC5_P_BinPoolReplenish(hVC5->hBinPool);
   }
   else
   {
      pBinnerState->psJob = psJob;

      BVC5_P_HardwareFakeInterrupt(hVC5, BCHP_FIELD_DATA(V3D_CTL_INT_STS_INT, FLDONE, 1));
   }

   return true;
}

static void BVC5_P_IssueRenderJob(
   BVC5_Handle           hVC5,
   uint32_t              uiCoreIndex,
   BVC5_P_InternalJob   *psJob
)
{
   BVC5_JobRender       *pRenderJob;
   BVC5_P_RenderState   *pRenderState = &hVC5->psCoreStates[uiCoreIndex].sRenderState;
   bool                  bUseRunning;

   BDBG_ASSERT(psJob != NULL);

   pRenderJob = (BVC5_JobRender *)psJob->pBase;

   bUseRunning = pRenderState->psJob[BVC5_P_HW_QUEUE_RUNNING] == NULL;

   if (bUseRunning)
      pRenderState->psJob[BVC5_P_HW_QUEUE_RUNNING] = psJob;
   else
      pRenderState->psJob[BVC5_P_HW_QUEUE_QUEUED] = psJob;

   BDBG_ASSERT(pRenderJob->uiStart[uiCoreIndex] != 0);
   BDBG_ASSERT(pRenderJob->uiEnd[uiCoreIndex] != 0);
   BDBG_ASSERT(pRenderJob->uiStart[uiCoreIndex] != pRenderJob->uiEnd[uiCoreIndex]);

   if (!psJob->bAbandon)
   {
      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_RENDER_JOBS_SUBMITTED, 1);

      /* We don't support multi-core yet */
      BDBG_ASSERT(pRenderJob->uiNumSubJobs == 1);

      /* TODO is this needed in single core? */
      if (pRenderJob->uiNumSubJobs == 1)
         BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QCFG, BCHP_FIELD_DATA(V3D_CLE_CT1QCFG, CTQMCDIS, 1));

      if (!psJob->jobData.sRender.bRenderOnlyJob)
      {
         uint32_t              physAddr;
         BVC5_BinBlockHandle   pvBinMem = BVC5_P_BinMemArrayGetBlock(&psJob->jobData.sRender.sBinMemArray, 0);

         BDBG_ASSERT(pvBinMem != NULL);
         physAddr = BVC5_P_BinBlockGetPhysical(pvBinMem);
         BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE0, physAddr);
      }

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBA, pRenderJob->uiStart[uiCoreIndex]);

      if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiRendererIdleStartTime != 0)
      {
         uint64_t now;
         BVC5_P_GetTime_isrsafe(&now);
         hVC5->sPerfCounters.uiRendererCumIdleTime += now - hVC5->sPerfCounters.uiRendererIdleStartTime;
         hVC5->sPerfCounters.uiRendererIdleStartTime = 0;
      }

      /* Job is issued straight away so record start.  In the other case, we record the start when the previous job
         has finished */
      if (bUseRunning)
      {
         BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK, BVC5_P_EVENT_MONITOR_RENDERING,
                                BVC5_EventBegin, psJob);
      }

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QEA, pRenderJob->uiEnd[uiCoreIndex]);
   }
   else
   {
      __sync_add_and_fetch(&pRenderState->uiCapturedRFC, 1);
      BVC5_P_HardwareFakeInterrupt(hVC5, BCHP_FIELD_DATA(V3D_CTL_INT_STS_INT, FRDONE, 1));
   }
}

static void BVC5_P_IssueTFUJob(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *psJob
)
{
   BVC5_P_TFUState   *psTFUState = &hVC5->sTFUState;
   BVC5_JobTFU       *pTFUJob    = (BVC5_JobTFU *)psJob->pBase;

   uint32_t           uiVal;

   BDBG_ASSERT(psJob->pBase->eType == BVC5_JobType_eTFU);

   psTFUState->psJob = psJob;

   /* If using custom coefficients, set them up */
   if (pTFUJob->sOutput.uiFlags & TFU_OUTPUT_USECOEF)
   {
      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_TFU_JOBS_SUBMITTED, 1);

      /* COEF3 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF3, ABB, pTFUJob->sCustomCoefs.uiBB) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF3, AGB, pTFUJob->sCustomCoefs.uiGB);

      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUCOEF3, uiVal);

      /* COEF2 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF2, AGR, pTFUJob->sCustomCoefs.uiGR) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF2, ARR, pTFUJob->sCustomCoefs.uiRR);

      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUCOEF2, uiVal);

      /* COEF1 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF1, AGC, pTFUJob->sCustomCoefs.uiGC) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF1, ABC, pTFUJob->sCustomCoefs.uiBC);

      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUCOEF1, uiVal);

      /* COEF0 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, AY, pTFUJob->sCustomCoefs.uiY) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, ARC, pTFUJob->sCustomCoefs.uiRC) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, USECOEF, 1);

      /* Also enable custom coeffs */
      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUCOEF0, uiVal);
   }

   /* Output Size */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIOS, XSIZE, pTFUJob->sOutput.uiWidth) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOS, YSIZE, pTFUJob->sOutput.uiHeight);

   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUIOS, uiVal);

   BDBG_ASSERT((pTFUJob->sOutput.uiAddress & ~BCHP_V3D_TFU_TFUIOA_OADDR_MASK) == 0);

   /* Output address */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OADDR, pTFUJob->sOutput.uiAddress >> BCHP_V3D_TFU_TFUIOA_OADDR_SHIFT) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OFORMAT, pTFUJob->sOutput.uiByteFormat) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OBIGEND, pTFUJob->sOutput.uiEndianness) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, DIMTW, pTFUJob->sOutput.uiFlags & TFU_OUTPUT_DISABLE_MAIN_TEXTURE ? 1 : 0);

   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUIOA, uiVal);

   /* Input stride */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIIS, STRIDE0, pTFUJob->sInput.uiRasterStride) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIIS, STRIDE1, pTFUJob->sInput.uiChromaStride);

   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUIIS, uiVal);

   /* Chroma address */
   uiVal = pTFUJob->sInput.uiChromaAddress;
   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUICA, uiVal);

   /* Raster address */
   uiVal = pTFUJob->sInput.uiAddress;
   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUIIA, uiVal);

   /* Configuration and submit conversion, enable interrupts */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUICFG, OPAD, pTFUJob->sOutput.uiVerticalPadding) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, IFORMAT, pTFUJob->sInput.uiByteFormat) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, IBIGEND, pTFUJob->sInput.uiEndianness) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, TTYPE, pTFUJob->sInput.uiTextureType) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, NUMMM, pTFUJob->sOutput.uiMipmapCount) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, SRGB, pTFUJob->sInput.uiFlags & TFU_INPUT_SRGB ? 1 : 0) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, FLIPY, pTFUJob->sInput.uiFlags & TFU_INPUT_FLIPY ? 1 : 0) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, RGBAORD, pTFUJob->sInput.uiComponentOrder) |
           BCHP_FIELD_DATA(V3D_TFU_TFUICFG, IOC, 1);

   if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiTFUIdleStartTime != 0)
   {
      uint64_t now;
      BVC5_P_GetTime_isrsafe(&now);
      hVC5->sPerfCounters.uiTFUCumIdleTime += now - hVC5->sPerfCounters.uiTFUIdleStartTime;
      hVC5->sPerfCounters.uiTFUIdleStartTime = 0;
   }

   BVC5_P_AddTFUJobEvent(hVC5, BVC5_EventBegin, psJob);

   BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_TFU_TFUICFG, uiVal);

   BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_TFU_JOBS_SUBMITTED, 1);
}

void BVC5_P_HardwarePrepareForJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_ClientHandle hClient;

   BVC5_P_HardwarePowerAcquire(hVC5, 1 << uiCoreIndex);

   hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);
   if ((int32_t)(hClient->uiFlushCpuCacheDone - pJob->uiFlushCpuCacheReq) < 0)
   {
      /* All outstanding CPU cache flush requests will be done. */
      hClient->uiFlushCpuCacheDone = hClient->uiFlushCpuCacheReq;

      /* Flush entire CPU cache */
      BVC5_P_AddEvent(hVC5, BVC5_P_EVENT_MONITOR_SCHED_TRACK,
                        hVC5->sEventMonitor.uiSchedTrackNextId,
                        BVC5_P_EVENT_MONITOR_FLUSH_CPU, BVC5_EventBegin);

      /* resolves to a flush all */
      BMEM_Heap_FlushCache(hVC5->hHeap, 0, ~0);

      BVC5_P_AddEvent(hVC5, BVC5_P_EVENT_MONITOR_SCHED_TRACK,
                        hVC5->sEventMonitor.uiSchedTrackNextId++,
                        BVC5_P_EVENT_MONITOR_FLUSH_CPU, BVC5_EventEnd);
   }

   if (!pJob->bFlushedV3D)
   {
      /* Anything other than only TFU access likely requires all the caches flushing. */
      if (pJob->pBase->uiSyncFlags & (BVC5_SYNC_V3D & ~BVC5_SYNC_TFU))
      {
         /* Clean all VC5 caches. Outside in. */
         BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId, BVC5_EventBegin,
                              true, true, true);

         BVC5_P_ClearL3Cache(hVC5, uiCoreIndex);
         BVC5_P_ClearL2Cache(hVC5, uiCoreIndex);
         BVC5_P_ClearSlicesCache(hVC5, uiCoreIndex);
         BVC5_P_FlushTextureCache(hVC5, uiCoreIndex);

         BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId++, BVC5_EventEnd,
                              true, true, true);

         BVC5_P_MarkJobsFlushedV3D(hVC5);
      }
      pJob->bFlushedV3D = true;
   }
}

bool BVC5_P_HardwareIssueBinnerJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_P_HardwarePrepareForJob(hVC5, uiCoreIndex, pJob);
   return BVC5_P_IssueBinJob(hVC5, uiCoreIndex, pJob);
}

void BVC5_P_HardwareIssueRenderJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_P_HardwarePrepareForJob(hVC5, uiCoreIndex, pJob);
   BVC5_P_IssueRenderJob(hVC5, uiCoreIndex, pJob);
}

void BVC5_P_HardwareIssueTFUJob(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *pJob
)
{
   /* TODO -- core 0 should be something indicating the TFU in multi-core */
   BVC5_P_HardwarePrepareForJob(hVC5, 0, pJob);
   BVC5_P_IssueTFUJob(hVC5, pJob);
}

BVC5_P_BinnerState *BVC5_P_HardwareGetBinnerState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   return &hVC5->psCoreStates[uiCoreIndex].sBinnerState;
}

BVC5_P_RenderState *BVC5_P_HardwareGetRenderState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   return &hVC5->psCoreStates[uiCoreIndex].sRenderState;
}

void BVC5_P_HardwareJobDone(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex,
   BVC5_P_HardwareUnitType eHardwareType
)
{
   switch (eHardwareType)
   {
   case BVC5_P_HardwareUnit_eBinner   :
      {
         BVC5_P_BinnerState   *pState = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);

         pState->psJob      = NULL;
         pState->uiPrevAddr = 0;

         if (hVC5->sPerfCounters.bCountersActive)
            BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiBinnerIdleStartTime);
      }
      break;

   case BVC5_P_HardwareUnit_eRenderer :
      {
         BVC5_P_RenderState *pState = BVC5_P_HardwareGetRenderState(hVC5, uiCoreIndex);
         BVC5_P_InternalJob *pJob = pState->psJob[BVC5_P_HW_QUEUE_RUNNING];

         pState->psJob[BVC5_P_HW_QUEUE_RUNNING] = pState->psJob[BVC5_P_HW_QUEUE_QUEUED];
         pState->psJob[BVC5_P_HW_QUEUE_QUEUED] = NULL;
         pState->uiPrevAddr = 0;

         if (pState->psJob[BVC5_P_HW_QUEUE_RUNNING] != NULL)
         {
            BVC5_P_InternalJob   *pRunning = pState->psJob[BVC5_P_HW_QUEUE_RUNNING];

            BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK, BVC5_P_EVENT_MONITOR_RENDERING,
                                   BVC5_EventBegin, pRunning);
         }
         else
         {
            if (hVC5->sPerfCounters.bCountersActive)
               BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiRendererIdleStartTime);
         }

         /* We need to ensure any dirty writes are flushed back to
          * memory now, before reporting the job as completed or the power gating
          * powers down the hardware and the data is lost (SWVC5-168).
          */
         if (pJob->pBase->uiSyncFlags & BVC5_SYNC_TMU_DATA_WRITE)
         {
            BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId, BVC5_EventBegin,
                                 false, false, /*clearL2T=*/true);

            BVC5_P_CleanTextureCache(hVC5, uiCoreIndex);

            BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId++, BVC5_EventEnd,
                                 false, false, /*clearL2T=*/true);
         }
      }
      break;

   case BVC5_P_HardwareUnit_eTFU      :
      {
         BVC5_P_TFUState  *pState  = &hVC5->sTFUState;

         /* Clear out */
         pState->psJob = NULL;

         if (hVC5->sPerfCounters.bCountersActive)
            BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiTFUIdleStartTime);
      }
      break;
   }

   BVC5_P_HardwareResetWatchdog(hVC5, uiCoreIndex);
   BVC5_P_HardwarePowerRelease(hVC5, 1 << uiCoreIndex);
}

void BVC5_P_HardwareGetInfo(
   BVC5_Handle hVC5,
   BVC5_Info   *pInfo
)
{
   uint32_t uiCoreIndex;
   uint32_t *pCur;

   /* Power on all cores */
   BVC5_P_HardwarePowerAcquire(hVC5, ~0u);

   BKNI_Memset((void *)pInfo, 0, sizeof(BVC5_Info));

   pCur = &pInfo->uiIdent[0];

   for (uiCoreIndex = 0; uiCoreIndex < hVC5->uiNumCores; uiCoreIndex++)
   {
      *pCur++ = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT0);
      *pCur++ = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT1);
      *pCur++ = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT2);
      *pCur++ = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT3);

#if defined(BVC5_HARDWARE_REAL) && (BCHP_CHIP==7250)
      /* GFXH-1272: These cores always show ASTC as present, even though it isn't. Fix it up */
      BCHP_SET_FIELD_DATA((*(pCur - 2)), V3D_CTL_IDENT2, WITH_ASTC, 0);
#endif
   }

   pCur = &pInfo->uiHubIdent[0];
   *pCur++ = BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_HUB_CTL_IDENT0);
   *pCur++ = BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_HUB_CTL_IDENT1);
   *pCur++ = BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_HUB_CTL_IDENT2);
   *pCur++ = BVC5_P_ReadRegister(hVC5, 0, BCHP_V3D_HUB_CTL_IDENT3);

   /* Power off all cores */
   BVC5_P_HardwarePowerRelease(hVC5, ~0u);
}

void BVC5_P_InterruptHandler_isr(
   void *pParm,
   int   iValue
)
{
   BVC5_Handle hVC5 = (BVC5_Handle)pParm;
   uint32_t    uiEnable;
   uint32_t    uiIntStatus;
   uint32_t    uiCoreIndex = 0; /* TODO */

   BSTD_UNUSED(iValue);

   /* system needs to be tolerant of spurious IRQ's.  Under heavy load, you can get dup's through
      here with the power off, leading to GISB timeouts */
   /* ATOMIC as written out of the IRQ context */
   if (__sync_fetch_and_and(&hVC5->psCoreStates[uiCoreIndex].bPowered, 0xFFFFFFFF))
   {
      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_INTERRUPTS, 1);

      /* HANDLE CORE INTERRUPTS */
      /* The mask will filter out some interrupts so it can't have been one of those */
      uiEnable    = ~BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_STS);

      /* Read status and mask off irrelevant bits */
      uiIntStatus =  BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_STS);
      uiIntStatus = uiIntStatus & uiEnable;

      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_CTL_INT_STS, INT_GMPV) != 0)
      {
         BVC5_P_HandleGMPViolation_isr(hVC5, uiCoreIndex);
         BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR, BCHP_FIELD_DATA(V3D_CTL_INT_CLR, INT_GMPV, 1));
      }

      /* Acknowledge render done interrupt and capture frame count */
      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_CTL_INT_STS, INT_FRDONE) != 0)
      {
         BVC5_P_RenderState *renderState = BVC5_P_HardwareGetRenderState(hVC5, uiCoreIndex);
         uint32_t            uiRFC;

         BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR, BCHP_FIELD_DATA(V3D_CTL_INT_CLR, INT_FRDONE, 1));

         /* This must be read after clearing interrupt condition.
            RFC can change at any time, so the value read here may have changed since the interrupt.
            If read before clearing interrupt we might miss an increment of RFC like this:
              -- s/w reads RFC
              -- h/w RFC increments
              -- s/w clear interrupt
              -- isr won't necessarily fire again and we have missed the increment
          */
         uiRFC = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CLE_RFC);

         /* shennanigans because we can't read and reset the RFC register atomically, so it must free run */
         __sync_fetch_and_add(&renderState->uiCapturedRFC, 0xFF & (uiRFC - renderState->uiLastRFC));
         renderState->uiLastRFC = uiRFC;
      }

      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_CTL_INT_STS_INT, OUTOMEM) != 0)
      {
         /* Need more overflow binning memory; will be handled in event loop */

         /* Clear the OOM bit now - we don't need to mask it as it's edge-triggered,
          * so it won't trigger until we've supplied some and it's run out again. */
         BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR,
                                  BCHP_FIELD_DATA(V3D_CTL_INT_CLR, INT_OUTOMEM, 1));
      }

      /* Acknowledge binner done interrupt */
      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_CTL_INT_STS, INT_FLDONE) != 0)
      {
         /* Clear the bin-done ISR condition */
         BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR,
                                  BCHP_FIELD_DATA(V3D_CTL_INT_CLR, INT_FLDONE, 1));

         /* See above if you need to add BFC handling */
      }

      __sync_fetch_and_or(&hVC5->uiInterruptReason, uiIntStatus);

      BKNI_SetEvent_isr(hVC5->hSchedulerWakeEvent);
   }
}

void BVC5_P_InterruptHandlerHub_isr(
   void *pParm,
   int   iValue
   )
{
   /* HANDLE TFU INTERRUPTS TODO: might provoke a different routine */
   BVC5_Handle hVC5 = (BVC5_Handle)pParm;
   uint32_t    uiEnable;
   uint32_t    uiIntStatus;
   uint32_t    uiCoreIndex = 0; /* TODO */

   BSTD_UNUSED(iValue);

   /* system needs to be tolerant of spurious IRQ's.  Under heavy load, you can get dup's through
      here with the power off, leading to GISB timeouts */
   /* ATOMIC as written out of the IRQ context */
   if (__sync_fetch_and_and(&hVC5->psCoreStates[uiCoreIndex].bPowered, 0xFFFFFFFF))
   {
      uiEnable = ~BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_TFU_TFUINT_MSK_STS);

      uiIntStatus = BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_TFU_TFUINT_STS) & uiEnable;

      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_TFU_TFUINT_STS, INT_TFUC))
         BVC5_P_WriteRegister_isr(hVC5, 0, BCHP_V3D_TFU_TFUINT_CLR, BCHP_FIELD_DATA(V3D_TFU_TFUINT_CLR, INT_TFUC, 1));

      __sync_fetch_and_or(&hVC5->uiTFUInterruptReason, uiIntStatus);

      BKNI_SetEvent_isr(hVC5->hSchedulerWakeEvent);
   }
}

void BVC5_P_HardwareSupplyBinner(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiPhysOffset,
   uint32_t     uiSize
)
{
   BDBG_ASSERT(uiPhysOffset != 0);

   /* Just supply the binner memory. No interrupts need masking in, as OOM is edge-triggered */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOA, uiPhysOffset);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOS, uiSize);
}

/* BVC5_P_CoreStateHasClient

   Is client active in any of the cores?

 */
bool BVC5_P_HardwareCoreStateHasClient(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
)
{
   uint32_t           i;
   uint32_t           uiNumCores    = hVC5->uiNumCores;
   BVC5_P_CoreState   *psCoreStates = hVC5->psCoreStates;
   BVC5_P_InternalJob *psTFUJob     = hVC5->sTFUState.psJob;

   for (i = 0; i < uiNumCores; ++i)
   {
      uint32_t j;

      BVC5_P_InternalJob  *binJob = psCoreStates[i].sBinnerState.psJob;

      if (binJob != NULL && binJob->uiClientId == uiClientId)
         return true;

      for (j = 0; j < BVC5_P_HW_QUEUE_STAGES; ++j)
      {
         BVC5_P_InternalJob  *renderJob = psCoreStates[i].sRenderState.psJob[j];

         if (renderJob != NULL && renderJob->uiClientId == uiClientId)
            return true;
      }
   }

   if (psTFUJob != NULL && psTFUJob->uiClientId == uiClientId)
      return true;

   return false;
}

void BVC5_P_HardwareAbandonJobs(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   BVC5_P_CoreState     *psCoreState   = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_P_BinnerState   *psBinnerState = &psCoreState->sBinnerState;
   BVC5_P_RenderState   *psRenderState = &psCoreState->sRenderState;
   uint32_t              uiReason      = 0;
   uint32_t              uiHubReason   = 0;

   BVC5_P_InternalJob   *psBinJob           = psBinnerState->psJob;
   BVC5_P_InternalJob   *psRunningRenderJob = psRenderState->psJob[BVC5_P_HW_QUEUE_RUNNING];
   BVC5_P_InternalJob   *psQueuedRenderJob  = psRenderState->psJob[BVC5_P_HW_QUEUE_QUEUED];
   BVC5_P_InternalJob   *psTFUJob           = hVC5->sTFUState.psJob;

   if (psBinJob != NULL)
   {
      uiReason |= BCHP_FIELD_DATA(V3D_CTL_INT_STS, INT_FLDONE, 1);
      psBinJob->bAbandon = true;
      psBinJob->eStatus  = BVC5_JobStatus_eERROR;

      if (psBinJob->jobData.sBin.psInternalRenderJob != NULL)
      {
         /* Abandon the attached render job */
         psBinJob->jobData.sBin.psInternalRenderJob->bAbandon = true;
         psBinJob->jobData.sBin.psInternalRenderJob->eStatus  = BVC5_JobStatus_eERROR;
      }
   }

   if (psRunningRenderJob != NULL || psQueuedRenderJob != NULL)
   {
      uiReason |= BCHP_FIELD_DATA(V3D_CTL_INT_STS, INT_FRDONE, 1);

      if (psRunningRenderJob != NULL)
      {
         psRunningRenderJob->bAbandon = true;
         psRunningRenderJob->eStatus  = BVC5_JobStatus_eERROR;
         __sync_add_and_fetch(&psRenderState->uiCapturedRFC, 1);
      }

      if (psQueuedRenderJob != NULL)
      {
         psQueuedRenderJob->bAbandon  = true;
         psQueuedRenderJob->eStatus   = BVC5_JobStatus_eERROR;
         __sync_add_and_fetch(&psRenderState->uiCapturedRFC, 1);
      }
   }

   if (psTFUJob != NULL)
   {
      uiHubReason |= BCHP_FIELD_DATA(V3D_TFU_TFUINT_STS, INT_TFUC, 1);
      psTFUJob->bAbandon = true;
      psTFUJob->eStatus  = BVC5_JobStatus_eERROR;
   }

   BVC5_P_HardwareFakeInterrupt(hVC5, uiReason);
   BVC5_P_HardwareFakeHubInterrupt(hVC5, uiHubReason);
}

bool BVC5_P_HardwareIsCoreIdle(
   BVC5_Handle    hVC5,
   uint32_t       uiCoreIndex
)
{
   BVC5_P_CoreState     *psCoreState   = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_P_BinnerState   *psBinnerState = &psCoreState->sBinnerState;
   BVC5_P_RenderState   *psRenderState = &psCoreState->sRenderState;

   return psBinnerState->psJob == NULL                          &&
          psRenderState->psJob[BVC5_P_HW_QUEUE_RUNNING] == NULL &&
          psRenderState->psJob[BVC5_P_HW_QUEUE_QUEUED]  == NULL;
}

bool BVC5_P_HardwareIsTFUIdle(
   BVC5_Handle    hVC5
   )
{
   return hVC5->sTFUState.psJob == NULL;
}

bool BVC5_P_HardwareIsIdle(
   BVC5_Handle    hVC5
)
{
   bool     bIsIdle = true;
   uint32_t i;

   for (i = 0; i < hVC5->uiNumCores && bIsIdle; ++i)
      bIsIdle = bIsIdle && BVC5_P_HardwareIsCoreIdle(hVC5, i);

   bIsIdle = bIsIdle && BVC5_P_HardwareIsTFUIdle(hVC5);

   return bIsIdle;
}

void BVC5_P_HardwareStandby(
   BVC5_Handle hVC5
)
{
   /* TODO multi-core -- is power down per core? */
   if (!hVC5->bInStandby)
   {
      BDBG_MSG(("VC5 enter standby"));

      BVC5_P_HardwarePowerRelease(hVC5, ~0u);

      hVC5->bInStandby = true;
   }
}

void BVC5_P_HardwareResume(
   BVC5_Handle hVC5
)
{
   if (hVC5->bInStandby)
   {
      BDBG_MSG(("VC5 leaving standby"));

      if (!hVC5->sOpenParams.bUseClockGating)
         BVC5_P_HardwarePowerAcquire(hVC5, ~0u);

      hVC5->bInStandby = false;
   }
}

/* For workaround GFXH-1181 */
bool BVC5_P_HardwareCacheClearBlocked(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   uint32_t i;

   BVC5_P_CoreState  *pCoreState = &hVC5->psCoreStates[uiCoreIndex];

   {
      BVC5_P_InternalJob   *pBinJob = pCoreState->sBinnerState.psJob;

      if (pBinJob != NULL)
      {
         BVC5_JobBin *pBin = (BVC5_JobBin *)pBinJob->pBase;

         if (pBin->uiFlags & BVC5_GFXH_1181)
            return true;
      }
   }

   for (i = 0; i < BVC5_P_HW_QUEUE_STAGES; ++i)
   {
      BVC5_P_InternalJob   *pRenderJob = pCoreState->sRenderState.psJob[i];

      if (pRenderJob != NULL)
      {
         BVC5_JobRender *pRender = (BVC5_JobRender *)pRenderJob->pBase;

         if (pRender->uiFlags & BVC5_GFXH_1181)
            return true;
      }
   }

   return false;
}
