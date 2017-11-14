/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_registers_priv.h"
#include "bvc5_null_priv.h"
#include "bvc5_hardware_debug.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
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

#if V3D_VER_AT_LEAST(3,3,0,0)
/* The MMU pagetable and illegal transaction safe page must be on a 4K
 * boundary and the programmed address is shifted to remove the LSBs
 */
#define MMU_ADDR_SHIFT (12)
/* The MMU address cap is programmed with a 256MB "page number", so the maximum
 * allowable address is shifted by 28
 */
#define MMU_ADDR_CAP_SHIFT (28)

/*
 * The top 32 bits of the virtual address causing an exception is reported in
 * the MMU VIO_ADDR register. However it is the internal representation of the
 * virtual address, whose width is version dependent, so we need to specify
 * how to shift the register value in order to report a sensible address for
 * the exception.
 */
#if V3D_VER_AT_LEAST(3,3,1,0)
/*
 * From 3.3.1 the MMU is 40bit physical capable, internally the virtual
 * address width is the same.
 */
#define MMU_VIO_ADDR_SHIFT (8)
#else
/*
 * On 3.3.0 the internal virtual addresses are actually 33bits wide, even
 * though only 32bit addesses are supported on input and output.
 */
#define MMU_VIO_ADDR_SHIFT (1)
#endif

#endif

static void BVC5_P_HardwareClean(BVC5_Handle hVC5, uint32_t uiCleans);
static void BVC5_P_HardwareResetV3D(BVC5_Handle hVC5);

#if V3D_VER_AT_LEAST(3,3,0,0)
static void BVC5_P_HardwareReadEventFifos_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex
   );
static void BVC5_P_HardwareReadEventFifosHub_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex
   );
#endif

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

uint32_t BVC5_P_ReadNonCoreRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiReg
   )
{
#if defined(BVC5_HARDWARE_NONE)
   return BVC5_P_NullReadRegister(hVC5, 0, uiReg);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   return BVC5_P_SimpenroseReadRegister(hVC5, 0, uiReg);
#else
   return BREG_Read32(hVC5->hReg, uiReg);
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

void BVC5_P_WriteNonCoreRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiReg,
   uint32_t     uiValue
   )
{
#if defined(BVC5_HARDWARE_NONE)
   BVC5_P_NullWriteRegister(hVC5, 0, uiReg, uiValue);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   BVC5_P_SimpenroseWriteRegister(hVC5, 0, uiReg, uiValue);
#else
   BREG_Write32(hVC5->hReg, uiReg, uiValue);
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

/* BVC5_P_ReadNonCoreRegister_isr
 *
 * Used in ISR context only
 */
static uint32_t BVC5_P_ReadNonCoreRegister_isr(
   BVC5_Handle  hVC5,
   uint32_t     uiReg
)
{
#if defined(BVC5_HARDWARE_NONE)
   return BVC5_P_NullReadRegister(hVC5, 0, uiReg);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   return BVC5_P_SimpenroseReadRegister(hVC5, 0, uiReg);
#else
   return BREG_Read32_isr(hVC5->hReg, uiReg);
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

/* BVC5_P_WriteNonCoreRegister_isr
 *
 * Used in ISR context only
 */
static void BVC5_P_WriteNonCoreRegister_isr(
   BVC5_Handle  hVC5,
   uint32_t     uiReg,
   uint32_t     uiValue
)
{
#if defined(BVC5_HARDWARE_NONE)
   BVC5_P_NullWriteRegister(hVC5, 0, uiReg, uiValue);
#elif defined(BVC5_HARDWARE_SIMPENROSE)
   BVC5_P_SimpenroseWriteRegister(hVC5, 0, uiReg, uiValue);
#else
   BREG_Write32_isr(hVC5->hReg, uiReg, uiValue);
#endif
}

#if V3D_VER_AT_LEAST(3,3,0,0)
static void BVC5_P_ClearMmuEntries(BVC5_Handle hVC5)
{
   uint32_t uiReg;

   /* The MMU(C) registers on v3.3 are subject to GFXH-1356 and must be read
    * atomically relative to the HUB isr handler reading the TFU and HUB INT
    * registers.
    */
   BKNI_EnterCriticalSection();
   uiReg = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMUC_CONTROL);

   if (uiReg & BCHP_FIELD_DATA(V3D_MMUC_CONTROL, ENABLE, 1))
   {
      uiReg |= BCHP_FIELD_DATA(V3D_MMUC_CONTROL, FLUSH, 1);
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMUC_CONTROL, uiReg);
      while(BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMUC_CONTROL) & BCHP_FIELD_DATA(V3D_MMUC_CONTROL, FLUSHING, 1));
   }

   uiReg = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_0_CTRL);

   if (uiReg & BCHP_FIELD_DATA(V3D_MMU_0_CTRL, ENABLE, 1))
   {
      uiReg |= BCHP_FIELD_DATA(V3D_MMU_0_CTRL, TLB_CLEAR, 1);
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_CTRL, uiReg);
      while(BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_0_CTRL) & BCHP_FIELD_DATA(V3D_MMU_0_CTRL, TLB_CLEARING, 1));
   }

#if !V3D_VER_AT_LEAST(4,0,2,0)
   uiReg = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL);

   if (uiReg & BCHP_FIELD_DATA(V3D_MMU_T_CTRL, ENABLE, 1))
   {
      uiReg |= BCHP_FIELD_DATA(V3D_MMU_T_CTRL, TLB_CLEAR, 1);
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL, uiReg);
      while(BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL) & BCHP_FIELD_DATA(V3D_MMU_T_CTRL, TLB_CLEARING, 1));
   }
#endif

   BKNI_LeaveCriticalSection();
}

static void BVC5_P_DisableMmu(BVC5_Handle hVC5)
{
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_CTRL, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_PT_PA_BASE, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_ADDR_CAP, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_ILLEGAL_ADR, 0);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_PT_PA_BASE, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_ADDR_CAP, 0);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_ILLEGAL_ADR, 0);
#endif
}

static void BVC5_P_EnableMmu(
   BVC5_Handle hVC5,
   uint32_t    uiPhys,
   uint32_t    uiMaxVirtualAddress
)
{
   const uint32_t uiCtrl =
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, ENABLE, 1) |
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, STATS_ENABLE, 1) |
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, PT_INVALID_EN, 1) |
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, PT_INVALID_ABORT_EN, 1) |
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, WRITE_VIOLATION_ABORT_EN, 1) |
      BCHP_FIELD_DATA(V3D_MMU_0_CTRL, CAP_EXCEEDED_ABORT_EN, 1);

   uint32_t uiIllegalAdr = (hVC5->uiMmuSafePageOffset >> MMU_ADDR_SHIFT) |
                           BCHP_FIELD_DATA(V3D_MMU_0_ILLEGAL_ADR, ENABLE, 1);
   uint32_t uiAddrCap;

   /*
    * Check the maximum virtual address is on a 256MB - 1 boundary
    */
   BDBG_ASSERT((uiMaxVirtualAddress & 0x0fffffff) == 0x0fffffff);

   uiAddrCap = uiMaxVirtualAddress >> MMU_ADDR_CAP_SHIFT;
   uiAddrCap |= BCHP_FIELD_DATA(V3D_MMU_0_ADDR_CAP, ENABLE, 1);

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_PT_PA_BASE, uiPhys);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_ILLEGAL_ADR, uiIllegalAdr);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_ADDR_CAP, uiAddrCap);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_0_CTRL, uiCtrl);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_PT_PA_BASE, uiPhys);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_ILLEGAL_ADR, uiIllegalAdr);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_ADDR_CAP, uiAddrCap);
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL, uiCtrl);
#endif
}
#endif

static bool BVC5_P_HardwareSetupMmu(
   BVC5_Handle hVC5,
   BVC5_P_InternalJob *pJob)
{
#if V3D_VER_AT_LEAST(3,3,0,0)
   const uint64_t uiBaseMask = BCHP_MASK(V3D_MMU_0_PT_PA_BASE, PAGE);

   uint32_t uiReg;
   uint32_t uiPhys;
   bool bSwitch;

   /*
    * Check we have no stray high bits set in the 64bit address
    */
   BDBG_ASSERT(((pJob->pBase->uiPagetablePhysAddr >> MMU_ADDR_SHIFT) & ~uiBaseMask) == 0);
   uiPhys = (uint32_t)((pJob->pBase->uiPagetablePhysAddr >> MMU_ADDR_SHIFT) & uiBaseMask);

   /* The MMU(C) registers on v3.3 are subject to GFXH-1356 and must be read
    * atomically relative to the HUB isr handler reading the TFU and HUB INT
    * registers.
    */
   BKNI_EnterCriticalSection();
   uiReg = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_0_PT_PA_BASE);
   BKNI_LeaveCriticalSection();

   bSwitch = (uiPhys != uiReg);
   if (bSwitch)
   {
      /* Always clean V3D caches on page table switch, the V3D virtual
       * addresses of anything dirty are about to become invalid or worse
       * point to some completely different part of physical memory
       */
      uint32_t uiCleans =  BVC5_CACHE_CLEAN_L2T | BVC5_CACHE_CLEAN_L1TD;
      /* uiCleans |= BVC5_CACHE_CLEAN_L3C; */
      BVC5_P_HardwareClean(hVC5, uiCleans);

      BVC5_P_ClearMmuEntries(hVC5);

      if (uiPhys != 0)
         BVC5_P_EnableMmu(hVC5, uiPhys, pJob->pBase->uiMmuMaxVirtAddr);
      else
         BVC5_P_DisableMmu(hVC5);

      /* Setup for all cache like things to be flushed now we have switched */
      {
         uint32_t uiCoreIndex;
         for (uiCoreIndex = 0; uiCoreIndex < hVC5->uiNumCores; ++uiCoreIndex)
            hVC5->psCoreStates[uiCoreIndex].uiCacheFlushes = BVC5_CACHE_FLUSH_ALL;
      }
   }
   else
   {
      BVC5_P_ClearMmuEntries(hVC5);
   }
   return bSwitch;
#else
   BSTD_UNUSED(hVC5);
   BSTD_UNUSED(pJob);
   return false;
#endif
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
            hVC5->sPerfCounters.uiPCTRShadows[c] +=
               BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PCTR_PCTR0 + ((BCHP_V3D_PCTR_PCTR1 - BCHP_V3D_PCTR_PCTR0) * c));
      }

#if !V3D_VER_AT_LEAST(4,0,2,0)
      if (hVC5->sPerfCounters.uiActiveBwCounters > 0)
      {
         hVC5->sPerfCounters.uiMemBwCntShadow += BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_GCA_MEM_BW_CNT);
      }
#endif
   }
}

#if V3D_VER_AT_LEAST(3,3,0,0)
static void BVC5_P_WaitForGCABridgeFIFOEmpty_isr(
   BVC5_Handle hVC5
   )
{
#if !V3D_VER_AT_LEAST(4,0,2,0)
   uint32_t uiReg;
   /*
    * Note: On an unloaded system the window for failure is very small and
    * just doing the first read/write of the control register is sufficient
    * to for the bridge to always report as idle.
    */
   uiReg = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_CTRL);

   /* Wait for MCP0 */
   uiReg &= ~BCHP_MASK(V3D_GCA_AXI_BRIDGE_CTRL, BRIDGE_STATUS_SELECT);
   BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_CTRL, uiReg);
   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_LO) != 0x0)
      ;

   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_HI) != 0x0)
      ;

#ifdef BCHP_S_MEMC_1_REG_START
   /* Wait for MCP1 */
   uiReg &= ~BCHP_MASK(V3D_GCA_AXI_BRIDGE_CTRL, BRIDGE_STATUS_SELECT);
   uiReg |= BCHP_FIELD_DATA(V3D_GCA_AXI_BRIDGE_CTRL, BRIDGE_STATUS_SELECT, 0x1);
   BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_CTRL, uiReg);
   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_LO) != 0x0)
      ;
   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_HI) != 0x0)
      ;
#endif

#ifdef BCHP_S_MEMC_2_REG_START
   /* Wait for MCP2 */
   uiReg &= ~BCHP_MASK(V3D_GCA_AXI_BRIDGE_CTRL, BRIDGE_STATUS_SELECT);
   uiReg |= BCHP_FIELD_DATA(V3D_GCA_AXI_BRIDGE_CTRL, BRIDGE_STATUS_SELECT, 0x2);
   BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_CTRL, uiReg);
   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_LO) != 0x0)
      ;
   while (BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_GCA_AXI_BRIDGE_STATUS_HI) != 0x0)
      ;
#endif
#else
   BSTD_UNUSED(hVC5);
#endif
}
#endif

static void BVC5_P_WaitForAXIIdle(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   )
{
   /* Wait until V3D has no active or pending AXI transactions */
#if V3D_VER_AT_LEAST(3,3,0,0)
   uint32_t uiQuiet;

   /* The V3D GCA in 7268/7271/7260 incorrectly signals to the rest of the
    * subsystem that all write activity has completed when that may
    * not be the case when burst splitting is enabled.
    *
    * Waiting for the GCA AXI bridge FIFOs for each MEMC to become
    * empty has been suggested by the hardware team as a solution.
    */
   if (!hVC5->sOpenParams.bNoBurstSplitting)
      BVC5_P_WaitForGCABridgeFIFOEmpty_isr(hVC5);

   /* Ask the GMP to stop all transactions cleanly */
   /* This is only called before a V3D or GMP reset, so we don't need to re-enable the AXI transactions later */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_CFG, BCHP_FIELD_DATA(V3D_GMP_CFG, STOP_REQ, 1));

   /* Wait until it has stopped, and there is no table load in progress */
   uiQuiet = BCHP_MASK(V3D_GMP_0_STATUS, WR_CNT) |
             BCHP_MASK(V3D_GMP_0_STATUS, RD_CNT) |
             BCHP_MASK(V3D_GMP_0_STATUS, CFG_BUSY);
   while (BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_STATUS) & uiQuiet)
   {
      BKNI_Printf("AXI WAS NOT IDLE\n");
      continue;
   }
#else
   uint32_t uiQuiet;

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
#endif
}

void BVC5_P_HardwarePLLEnable(
   BVC5_Handle hVC5
   )
{
#ifdef BCHP_PWR_SUPPORT
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH
   BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH);
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D
   BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM
   BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
#endif
#else
   BSTD_UNUSED(hVC5);
#endif
}

void BVC5_P_HardwarePLLDisable(
   BVC5_Handle hVC5
   )
{
#ifdef BCHP_PWR_SUPPORT
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM
   BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D
   BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH
   BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH);
#endif
#else
   BSTD_UNUSED(hVC5);
#endif
}

void BVC5_P_HardwareBPCMPowerUp(
   BVC5_Handle hVC5
   )
{
#ifdef V3D_HAS_BPCM
#if V3D_VER_AT_LEAST(3,3,0,0)
   #define BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL BCHP_ZONE0_FS_PWR_CONTROL
   #define V3D_BPCM_PWR_ZONE_0(X) ZONE0_FS_PWR_##X
#else
   #define V3D_BPCM_PWR_ZONE_0(X) V3D_BPCM_PWR_ZONE_0_##X
#endif
   #define BPCM_MASK(X, Y) BCHP_MASK(X, Y)
   /* Power-up V3D */
   uint32_t uiSuccess, uiMask;
   uint32_t uiZoneControl;

   uiZoneControl = BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_DPG_CNTL_EN, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_UP_REQ, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_BLK_RST_ASSERT, 1) |
      BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_MEM_PWR_CNTL_EN, 1);

   BREG_Write32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL, uiZoneControl);

   uiZoneControl = BREG_Read32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL);

   uiSuccess = BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_MEM_PWR_STATE) |
      BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_DPG_PWR_STATE) |
      BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_ON_STATE);

   uiMask = BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_ISO_STATE) |
      BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_MEM_PWR_STATE) |
      BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_DPG_PWR_STATE) |
      BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_ON_STATE);

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
   if (BCHP_GET_FIELD_DATA(uiZoneControl, V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_ON_STATE))
   {
      uint32_t uiSuccess, uiMask;

      uiZoneControl = BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_DPG_CNTL_EN, 1) |
         BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_DN_REQ, 1) |
         BCHP_FIELD_DATA(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_MEM_PWR_CNTL_EN, 1);

      BREG_Write32(hVC5->hReg, BCHP_V3D_BPCM_PWR_ZONE_0_CONTROL, uiZoneControl);

      uiSuccess = BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_ISO_STATE) |
         BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_OFF_STATE);

      uiMask = BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_ISO_STATE) |
         BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_MEM_PWR_STATE) |
         BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_DPG_PWR_STATE) |
         BPCM_MASK(V3D_BPCM_PWR_ZONE_0(CONTROL), ZONE_PWR_OFF_STATE);

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
      uint32_t uiCleans;

      BVC5_P_UpdateShadowCounters(hVC5, uiCoreIndex);

      BKNI_EnterCriticalSection();

      /* ATOMIC with IRQ which can read this value.  Every other access is in
         same thread */
      __sync_fetch_and_and(&hVC5->psCoreStates[uiCoreIndex].bPowered, 0);

      BKNI_LeaveCriticalSection();

      /* Always clean V3D caches on shutdown. */
      uiCleans =  BVC5_CACHE_CLEAN_L2T;
#if V3D_VER_AT_LEAST(3,3,0,0)
      uiCleans |= BVC5_CACHE_CLEAN_L1TD;
#endif
      /* uiCleans |= BVC5_CACHE_CLEAN_L3C; */
      BVC5_P_HardwareClean(hVC5, uiCleans);

      /* We have to wait for AXI idle before resetting (or powering off),
       * to ensure any write transactions have fully completed.
       */
      BVC5_P_WaitForAXIIdle(hVC5, uiCoreIndex);

      BVC5_P_HardwareResetV3D(hVC5);

      if (hVC5->sOpenParams.bUsePowerGating)
         BVC5_P_HardwareBPCMPowerDown(hVC5);

      if (hVC5->sPerfCounters.bCountersActive)
         BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiPoweredOffStartTime);

      BVC5_P_HardwarePLLDisable(hVC5);
   }
}

void BVC5_P_RestorePerfCounters(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex,
   bool        bWriteSelectorsAndEnables
)
{
   if (hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PCTR_PCTRC, 0xFFFFFFFF);
#if !V3D_VER_AT_LEAST(4,0,2,0)
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_GCA_PM_CTRL, 1);     /* Holds the counters in reset until de-asserted */
#endif

      /* re-write the performance monitor selectors and enables */
      if (bWriteSelectorsAndEnables && hVC5->sPerfCounters.uiPCTREShadow)
      {
         uint32_t c;
#if V3D_VER_AT_LEAST(4,0,2,0)
         for (c = 0; c < BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE; c+=4)
         {
            uint32_t pctrs = ((hVC5->sPerfCounters.uiPCTRSShadow[c + 3] & 0x7F) << 24) |
                             ((hVC5->sPerfCounters.uiPCTRSShadow[c + 2] & 0x7F) << 16) |
                             ((hVC5->sPerfCounters.uiPCTRSShadow[c + 1] & 0x7F) <<  8) |
                             ((hVC5->sPerfCounters.uiPCTRSShadow[c + 0] & 0x7F)      );

            BVC5_P_WriteRegister(hVC5, uiCoreIndex,
               BCHP_V3D_PCTR_0_SRC_0_3 + ((BCHP_V3D_PCTR_0_SRC_4_7 - BCHP_V3D_PCTR_0_SRC_0_3) * c), pctrs);
         }
#else
         for (c = 0; c < BVC5_P_PERF_COUNTER_MAX_HW_CTRS_ACTIVE; c++)
            BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PCTR_PCTRS0 + (c * 8), hVC5->sPerfCounters.uiPCTRSShadow[c]);
#endif

         BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PCTR_PCTRE, hVC5->sPerfCounters.uiPCTREShadow);
      }

#if !V3D_VER_AT_LEAST(4,0,2,0)
      if (hVC5->sPerfCounters.uiActiveBwCounters > 0)
      {
         /* Must also clear the bottom two bits to enable the counters */
         BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_GCA_PM_CTRL, hVC5->sPerfCounters.uiGCAPMSelShadow & (~3));
      }
#endif
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

   if (hVC5->bSecure)
      return false;

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

   BMMA_FlushCache(hVC5->hGMPTable, ptr, size);

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

#if V3D_VER_AT_LEAST(3,3,0,0)
   /* Reset the GMP */

#if V3D_VER_AT_LEAST(4,0,2,0)
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_CFG, BCHP_FIELD_DATA(V3D_GMP_CFG, GMPRST, 1));
#else
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_STATUS, BCHP_FIELD_DATA(V3D_GMP_STATUS, GMPRST, 1));
#endif

   /* Enable the GMP violation interrupt bit */
   uiMask = BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, OUTOMEM, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FLDONE, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FRDONE, 1) |
            BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, GMPV, 1);
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_CLR, uiMask);

   /* Set the address of the protection table */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_TABLE_ADDR, (uint32_t)hVC5->uiGMPOffset);

   /* Load the entire table */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_CLEAR_LOAD, 0xFFFFFFFF);

   /* Turn on protection - no need to wait for table load to finish */
   BVC5_P_WriteRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_CFG, BCHP_FIELD_DATA(V3D_GMP_CFG, PROTENABLE, 1));

#else
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
#endif
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
#if V3D_VER_AT_LEAST(3,3,0,0)
   uint32_t type;
   uint32_t addr;
   uint32_t id;
   uint32_t write;

   addr  = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_VIO_ADDR);
   type  = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_GMP_VIO_TYPE);
   id    = BCHP_GET_FIELD_DATA(type, V3D_GMP_VIO_TYPE, VIOID);
   write = BCHP_GET_FIELD_DATA(type, V3D_GMP_VIO_TYPE, VIO_WR);

   BVC5_P_PrintGMPViolation_isr(addr, id, write ? true : false);
#else
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
#endif

   /* Reset the GMP */
   BVC5_P_SetupGMP_isr(hVC5, uiCoreIndex);
}

void BVC5_P_HardwareSetDefaultRegisterState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   uint32_t uiIntMask;

   /* Disable and clear interrupts from this core */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_SET, 0xFFFFFFFF);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_CLR, 0xFFFFFFFF);

   uiIntMask = BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, OUTOMEM, 1) |
               BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FLDONE, 1) |
               BCHP_FIELD_DATA(V3D_CTL_INT_MSK_CLR_INT, FRDONE, 1);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_CLR, uiIntMask);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   {
      uint32_t uiCachedMask, uiHighPriMask;

      /* Enable GCA cache */
      uiCachedMask = 1 << V3D_AXI_ID_CLE | 1 << V3D_AXI_ID_PTB | 1 << V3D_AXI_ID_PSE;
#if !V3D_VER_AT_LEAST(3,3,0,0)
      uiCachedMask |= 1 << V3D_AXI_ID_VCD;
#endif

      /* Send cached, L2T and L2C traffic down the high priority path. */
      uiHighPriMask = uiCachedMask | 1 << V3D_AXI_ID_L2T;
#if !V3D_VER_AT_LEAST(3,3,0,0)
      uiHighPriMask |= 1 << V3D_AXI_ID_L2C;
#endif

#if !V3D_VER_AT_LEAST(4,0,2,0)
      BVC5_P_WriteNonCoreRegister(hVC5,
         BCHP_V3D_GCA_CACHE_ID,
         BCHP_FIELD_DATA(V3D_GCA_CACHE_ID, CACHE_ID_EN, uiCachedMask));

      BVC5_P_WriteNonCoreRegister(hVC5,
         BCHP_V3D_GCA_LOW_PRI_ID,
         BCHP_FIELD_DATA(V3D_GCA_LOW_PRI_ID, LOW_PRI_ID_EN, uiHighPriMask)
       | BCHP_FIELD_DATA(V3D_GCA_LOW_PRI_ID, INVERT_FUNCTION, 1));
#endif
   }
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (hVC5->sOpenParams.bNoBurstSplitting || !V3D_VER_AT_LEAST(3,3,1,0))
      BVC5_P_WriteRegister(hVC5, 0, BCHP_V3D_GCA_SCB_8_0_CMD_SPLIT_CTL,
                   BCHP_FIELD_DATA(V3D_GCA_SCB_8_0_CMD_SPLIT_CTL, SCB_8_0_CMD_SPLIT_DIS, 1));
#endif

   /* Set up OVRTMUOUT mode (this doesn't work in v3dv3.2) */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_MISCCFG, 1);

   /* Workaround GFXH-1383: restore register at address 0 to default value. */
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_HUB_CTL_AXICFG,
      (BCHP_V3D_HUB_CTL_AXICFG_QOS_DEFAULT << BCHP_V3D_HUB_CTL_AXICFG_QOS_SHIFT)
    | (BCHP_V3D_HUB_CTL_AXICFG_MAXLEN_DEFAULT << BCHP_V3D_HUB_CTL_AXICFG_MAXLEN_SHIFT) );

   /* Turn on the MMU Cache once here as it is shared by all MMUs */
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_MMUC_CONTROL,
                BCHP_FIELD_DATA(V3D_MMUC_CONTROL, ENABLE, 1));
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

#if V3D_VER_AT_LEAST(3,3,0,0)
   /* Enable & clear the cycle counters in the core and the TFU*/
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CCNTCS,
                        BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CCNT_EN,  1) |
                        BCHP_FIELD_DATA(V3D_CLE_CCNTCS, FIFO_CLR, 1));

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_CCCS,
                        BCHP_FIELD_DATA(V3D_TFU_CCCS, CCNT_EN,  1) |
                        BCHP_FIELD_DATA(V3D_TFU_CCCS, FIFO_CLR, 1));
#endif

   /* Always flush the whole L2T. */
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLSTA, 0);
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TFLEND, ~0);

   __sync_fetch_and_and(&hVC5->uiInterruptReason, 0);
   __sync_fetch_and_and(&hVC5->uiTFUInterruptReason, 0);
}

static void BVC5_P_HardwareResetV3D(
   BVC5_Handle hVC5
)
{
#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* Request shutdown and wait for GCA so all outstanding SCB/MCP traffic
    * is terminated prior to sw_init. Any such pending traffic, including on
    * the MCP side will be lost by this action. If we are resetting after a
    * core lockup then that is what we want, if we are resetting as part of
    * a power management shutdown then we must already have waited for all
    * write traffic to have made it out of the subsystem before we do this.
    */
   BREG_Write32(hVC5->hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN,
                             BCHP_FIELD_DATA(V3D_GCA_SAFE_SHUTDOWN, SAFE_SHUTDOWN_EN,  1));

   while (BREG_Read32(hVC5->hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN_ACK) != 0x3)
      ;
#endif

   BREG_Write32(hVC5->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
   BREG_Write32(hVC5->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));
}

static void BVC5_P_MarkAllJobsFlushedV3D(
   BVC5_Handle hVC5
)
{
   /* Record on ready jobs that cache flushes have happened on all cores. */
   uint32_t uiCoreIndex;
   for (uiCoreIndex = 0; uiCoreIndex < hVC5->uiNumCores; ++uiCoreIndex)
   {
      BVC5_P_MarkJobsFlushedV3D(hVC5, uiCoreIndex);
   }
}

/* This should only be called when you know the core is powered - e.g. from a lockup condition */
void BVC5_P_HardwareResetCoreAndState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   BVC5_P_HardwareResetV3D(hVC5);
   BVC5_P_HardwareSetDefaultRegisterState(hVC5, uiCoreIndex);
}

static void BVC5_P_HardwareTurnOn(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   if (!hVC5->psCoreStates[uiCoreIndex].bPowered)
   {
      BVC5_P_HardwarePLLEnable(hVC5);

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
      BVC5_P_MarkAllJobsFlushedV3D(hVC5);

      /* ATOMIC with IRQ which can read this value.  Every other access is in
         same thread */
      __sync_fetch_and_or(&hVC5->psCoreStates[uiCoreIndex].bPowered, 1);

      BVC5_P_HardwareSetDefaultRegisterState(hVC5, uiCoreIndex);
      BVC5_P_RestorePerfCounters(hVC5, uiCoreIndex, true);
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

bool BVC5_P_HardwareIsRendererAvailable(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   uint32_t uiPos = hVC5->sOpenParams.bNoQueueAhead ? BVC5_P_HW_QUEUE_RUNNING : BVC5_P_HW_QUEUE_QUEUED;

   return hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[uiPos] == NULL;
}

bool BVC5_P_HardwareIsBinnerAvailable(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
)
{
   return hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob == NULL;
}

bool BVC5_P_HardwareIsTFUAvailable(
   BVC5_Handle hVC5
)
{
   return hVC5->sTFUState.psJob == NULL;
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
   BVC5_BinBlockHandle    hBinBlock;
   BVC5_JobBin           *pBinJob;
   BVC5_P_InternalJob    *psRenderJob;
   BVC5_P_BinnerState    *pBinnerState = &hVC5->psCoreStates[uiCoreIndex].sBinnerState;

   BDBG_ASSERT(psJob != NULL);

   pBinJob = (BVC5_JobBin *)psJob->pBase;
   psRenderJob = psJob->jobData.sBin.psInternalRenderJob;

   if (!psJob->bAbandon && psRenderJob != NULL && pBinJob->uiNumSubJobs > 0)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      uint32_t uiTsAddr;
      BVC5_P_TileState *psTileState;
      BMMA_Heap_Handle hMMAHeap;
#endif
      BDBG_ASSERT(pBinJob->uiStart[uiCoreIndex] != 0);
      BDBG_ASSERT(pBinJob->uiEnd[uiCoreIndex] != 0);
      BDBG_ASSERT(pBinJob->uiStart[uiCoreIndex] != pBinJob->uiEnd[uiCoreIndex]);

#if V3D_VER_AT_LEAST(4,1,34,0)
      psTileState = (pBinJob->sBase.bSecure) ? &hVC5->sTileState[1] : &hVC5->sTileState[0];
      hMMAHeap = (pBinJob->sBase.bSecure && hVC5->hSecureMMAHeap) ? hVC5->hSecureMMAHeap : hVC5->hMMAHeap;

      if (pBinJob->uiTileStateSize > psTileState->uiTileStateSize)
      {
         if (!BVC5_P_AllocTileState(psTileState, hMMAHeap, pBinJob->uiTileStateSize))
         {
            BKNI_Printf("%s : FATAL : Unable to grow tile state\n", BSTD_FUNCTION);
            return false; /* Couldn't get enough tile state memory */
         }
      }
#endif

      hBinBlock = BVC5_P_BinMemArrayAdd(&psRenderJob->jobData.sRender.sBinMemArray,
                                        psJob->jobData.sBin.uiMinInitialBinBlockSize, &uiPhysical);

      if (hBinBlock == NULL)
         return false; /* Couldn't get enough bin memory */

      pBinnerState->psJob = psJob;

      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_BIN_JOBS_SUBMITTED, 1);

      BDBG_ASSERT(uiPhysical != 0);

      /* Clear BPOS to ensure we can't use left over bin memory */
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOS, 0);

      /* Setup MMU if active in the Job and adjust the bin memory physical
       * address if required to the fixed virtual mapping in the MMU
       */
      if (pBinJob->sBase.uiPagetablePhysAddr != 0)
         uiPhysical = BVC5_P_TranslateBinAddress(hVC5, uiPhysical, pBinJob->sBase.bSecure);

#if V3D_VER_AT_LEAST(4,1,34,0)
      uiTsAddr = BVC5_P_TranslateBinAddress(hVC5, psTileState->uiTileStateOffset, pBinJob->sBase.bSecure);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_0_CT0QTS, uiTsAddr | BCHP_FIELD_DATA(V3D_CLE_0_CT0QTS, CTQTSEN, 1));
#endif

      /* Setup the memory for this bin job and kick it off */
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMA, uiPhysical);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMS, hBinBlock->uiNumBytes);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QBA, pBinJob->uiStart[uiCoreIndex]);

      if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiBinnerIdleStartTime != 0)
      {
         uint64_t now;
         BVC5_P_GetTime_isrsafe(&now);
         hVC5->sPerfCounters.uiBinnerCumIdleTime += now - hVC5->sPerfCounters.uiBinnerIdleStartTime;
         hVC5->sPerfCounters.uiBinnerIdleStartTime = 0;
      }

#if V3D_VER_AT_LEAST(3,3,0,0)
      if (hVC5->sEventMonitor.bActive)
      {
         BVC5_P_EventInfo *psEventInfo;

         BKNI_EnterCriticalSection();
         psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sBinJobQueueCLE);
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, psEventInfo);
         BVC5_P_SendMessage(&hVC5->sEventMonitor.sBinJobQueueCLE, psEventInfo);

         psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sBinJobQueuePTB);
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, psEventInfo);
         BVC5_P_SendMessage(&hVC5->sEventMonitor.sBinJobQueuePTB, psEventInfo);
         BKNI_LeaveCriticalSection();
      }
#else
      {
      BVC5_P_EventInfo sBinJobEventInfo;
      BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, &sBinJobEventInfo);
      BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING,
                             BVC5_EventBegin, &sBinJobEventInfo, BVC5_P_GetEventTimestamp());
      }
#endif

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QEA, pBinJob->uiEnd[uiCoreIndex]);

      /* The binner is off and running, so now reallocate memory. */
      BVC5_P_BinMemArrayReplenishPool(&psRenderJob->jobData.sRender.sBinMemArray);
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
      uint32_t uiQCFG = BCHP_FIELD_DATA(V3D_CLE_CT1QCFG, CTQMCDIS, 1);

      BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_RENDER_JOBS_SUBMITTED, 1);

#if V3D_VER_AT_LEAST(3,3,0,0)
      /* Control empty-tile processing (TBL) */
      if (pRenderJob->uiEmptyTileMode != BVC5_EMPTY_TILE_MODE_NONE)
      {
         /* Macros to control empty-tile processing are missing. */
         uiQCFG |= 1u << 7; /* BCHP_FIELD_DATA(V3D_CLE_CT1QCFG, CTETFILT, 1); */
         if (pRenderJob->uiEmptyTileMode == BVC5_EMPTY_TILE_MODE_FILL)
            uiQCFG |= 1u << 6; /* BCHP_FIELD_DATA(V3D_CLE_CT1QCFG, CTETPROC, 1); */
      }
#endif

      /* We don't support multi-core yet */
      BDBG_ASSERT(pRenderJob->uiNumSubJobs == 1);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QCFG, uiQCFG);

      if (!psJob->jobData.sRender.bRenderOnlyJob)
      {
         uint32_t              physAddr;
         BVC5_BinBlockHandle   pvBinMem = BVC5_P_BinMemArrayGetBlock(&psJob->jobData.sRender.sBinMemArray, 0);
         if (pvBinMem != NULL)
         {
            physAddr = BVC5_P_BinBlockGetPhysical(pvBinMem);

            if (pRenderJob->sBase.uiPagetablePhysAddr != 0)
               physAddr = BVC5_P_TranslateBinAddress(hVC5, physAddr, pRenderJob->sBase.bSecure);

            /* BKNI_Printf("Renderer physical address %08x -> BCHP_V3D_CLE_CT1QBASE0\n", physAddr); */

            BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE0, physAddr);
         }
      }

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBA, pRenderJob->uiStart[uiCoreIndex]);

      if (hVC5->sPerfCounters.bCountersActive && hVC5->sPerfCounters.uiRendererIdleStartTime != 0)
      {
         uint64_t now;
         BVC5_P_GetTime_isrsafe(&now);
         hVC5->sPerfCounters.uiRendererCumIdleTime += now - hVC5->sPerfCounters.uiRendererIdleStartTime;
         hVC5->sPerfCounters.uiRendererIdleStartTime = 0;
      }

#if V3D_VER_AT_LEAST(3,3,0,0)
      if (hVC5->sEventMonitor.bActive)
      {
         BVC5_P_EventInfo *psEventInfo;

         BKNI_EnterCriticalSection();
         psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sRenderJobQueueCLE);
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, psEventInfo);
         BVC5_P_SendMessage(&hVC5->sEventMonitor.sRenderJobQueueCLE, psEventInfo);

         psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sRenderJobQueueTLB);
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, psEventInfo);
         BVC5_P_SendMessage(&hVC5->sEventMonitor.sRenderJobQueueTLB, psEventInfo);
         BKNI_LeaveCriticalSection();
      }

      if (hVC5->bCollectLoadStats)
      {
         BVC5_P_EventInfo *psEventInfo;

         BKNI_EnterCriticalSection();
         psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sRenderJobQueueCLELoadStats);
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, psEventInfo);
         BVC5_P_SendMessage(&hVC5->sEventMonitor.sRenderJobQueueCLELoadStats, psEventInfo);
         BKNI_LeaveCriticalSection();
      }
#else
      /* Job is issued straight away so record start.  In the other case, we record the start when the previous job
         has finished */
      if (bUseRunning)
      {
         BVC5_P_EventInfo sRenderJobEventInfo;
         BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, psJob, &sRenderJobEventInfo);
         BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK, BVC5_P_EVENT_MONITOR_RENDERING,
                                BVC5_EventBegin, &sRenderJobEventInfo, BVC5_P_GetEventTimestamp());
         if (hVC5->bCollectLoadStats)
            BVC5_P_GetTime_isrsafe(&psJob->uiRenderStart);
      }
#endif

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

      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUCOEF3, uiVal);

      /* COEF2 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF2, AGR, pTFUJob->sCustomCoefs.uiGR) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF2, ARR, pTFUJob->sCustomCoefs.uiRR);

      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUCOEF2, uiVal);

      /* COEF1 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF1, AGC, pTFUJob->sCustomCoefs.uiGC) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF1, ABC, pTFUJob->sCustomCoefs.uiBC);

      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUCOEF1, uiVal);

      /* COEF0 */
      uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, AY, pTFUJob->sCustomCoefs.uiY) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, ARC, pTFUJob->sCustomCoefs.uiRC) |
              BCHP_FIELD_DATA(V3D_TFU_TFUCOEF0, USECOEF, 1);

      /* Also enable custom coeffs */
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUCOEF0, uiVal);
   }

   /* Output Size */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIOS, XSIZE, pTFUJob->sOutput.uiWidth) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOS, YSIZE, pTFUJob->sOutput.uiHeight);

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUIOS, uiVal);

   BDBG_ASSERT((pTFUJob->sOutput.uiAddress & ~BCHP_V3D_TFU_TFUIOA_OADDR_MASK) == 0);

   /* Output address */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OADDR, pTFUJob->sOutput.uiAddress >> BCHP_V3D_TFU_TFUIOA_OADDR_SHIFT) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OFORMAT, pTFUJob->sOutput.uiByteFormat) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, OBIGEND, pTFUJob->sOutput.uiEndianness) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIOA, DIMTW, pTFUJob->sOutput.uiFlags & TFU_OUTPUT_DISABLE_MAIN_TEXTURE ? 1 : 0);

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUIOA, uiVal);

   /* Input stride */
   uiVal = BCHP_FIELD_DATA(V3D_TFU_TFUIIS, STRIDE0, pTFUJob->sInput.uiRasterStride) |
           BCHP_FIELD_DATA(V3D_TFU_TFUIIS, STRIDE1, pTFUJob->sInput.uiChromaStride);

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUIIS, uiVal);

#if V3D_VER_AT_LEAST(3,3,0,0)
   /* U-Plane address */
   uiVal = pTFUJob->sInput.uiUPlaneAddress;
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUIUA, uiVal);
#endif

   /* Chroma address */
   uiVal = pTFUJob->sInput.uiChromaAddress;
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUICA, uiVal);

   /* Raster address */
   uiVal = pTFUJob->sInput.uiAddress;
   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUIIA, uiVal);

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

#if V3D_VER_AT_LEAST(3,3,0,0)
   if (hVC5->sEventMonitor.bActive)
   {
      BVC5_P_EventInfo *psEventInfo;

      BKNI_EnterCriticalSection();
      psEventInfo = BVC5_P_GetMessage(&hVC5->sEventMonitor.sQueueTFU);
      BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/true, psJob, psEventInfo);
      BVC5_P_SendMessage(&hVC5->sEventMonitor.sQueueTFU, psEventInfo);
      BKNI_LeaveCriticalSection();
   }
#else
   {
      BVC5_P_EventInfo sFifoTFUEventInfo;
      BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/true, psJob, &sFifoTFUEventInfo);
      BVC5_P_AddTFUJobEvent(hVC5, BVC5_EventBegin, &sFifoTFUEventInfo, BVC5_P_GetEventTimestamp());
   }
#endif

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_TFUICFG, uiVal);

   BVC5_P_SchedPerfCounterAdd_isr(hVC5, BVC5_P_PERF_TFU_JOBS_SUBMITTED, 1);
}

bool BVC5_P_SwitchSecurityMode(
   BVC5_Handle          hVC5,
   bool                 bSecure
)
{
   if (hVC5->bSecure != bSecure)
   {
      /* nothing is available for index in the functions above, so fix to 0 for
         now */
      const uint32_t uiCoreIndex = 0;

      BVC5_P_HardwarePowerAcquire(hVC5, 1 << uiCoreIndex);

      /* Mustn't change if there is anything in the hardware */
      if (!BVC5_P_HardwareIsIdle(hVC5))
      {
         BVC5_P_HardwarePowerRelease(hVC5, 1 << uiCoreIndex);
         return false;
      }

      hVC5->bSecure = bSecure;

      BKNI_EnterCriticalSection();  /* Ensure no spurious ISR is running when we set bToggling */
      hVC5->bToggling = true;
      BKNI_LeaveCriticalSection();

      /* Change security mode */
      if (hVC5->sCallbacks.fpSecureToggleHandler)
         hVC5->sCallbacks.fpSecureToggleHandler(bSecure);

      hVC5->bToggling = false;

      /* Core will have been reset, so reconfigure it */
      /* TODO will need to handle multiple cores */
      BVC5_P_HardwareSetDefaultRegisterState(hVC5, 0);

      BVC5_P_HardwarePowerRelease(hVC5, 1 << uiCoreIndex);
   }

   return true;
}

uint32_t BVC5_P_HardwareDeferCacheFlush(BVC5_Handle hVC5, uint32_t uiFlushes, uint32_t uiJobMask)
{
   uint32_t uiCoreIndex;
   uint32_t uiCoreMask;

   /* no L3C yet */
   BDBG_ASSERT(!(uiFlushes & BVC5_CACHE_FLUSH_L3C));

   if (uiFlushes)
   {
      BDBG_MSG(("BVC5_P_HardwareDeferCacheFlush: %.8x %.8x", uiFlushes, uiJobMask));

      for (uiCoreIndex = 0; uiCoreIndex != hVC5->uiNumCores; ++uiCoreIndex)
      {
         hVC5->psCoreStates[uiCoreIndex].uiCacheFlushes |= uiFlushes;
      }
      if (uiFlushes & uiJobMask)
         return (1 << hVC5->uiNumCores) - 1;
   }


   /* Figure out core mask. */
   uiCoreMask = 0;
   for (uiCoreIndex = 0; uiCoreIndex != hVC5->uiNumCores; ++uiCoreIndex)
   {
      if (hVC5->psCoreStates[uiCoreIndex].uiCacheFlushes & uiJobMask)
         uiCoreMask |= 1 << uiCoreIndex;
   }
   return uiCoreMask;
}

static void BVC5_P_HardwareCoreFlush(BVC5_Handle hVC5, uint32_t uiCoreIndex)
{
   uint32_t uiFlushes = hVC5->psCoreStates[uiCoreIndex].uiCacheFlushes;
   if (!uiFlushes)
      return;
   hVC5->psCoreStates[uiCoreIndex].uiCacheFlushes = 0;

   BDBG_MSG(("BVC5_P_HardwareCoreFlush: %x, %.8x", uiCoreIndex, uiFlushes));

   BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId, BVC5_EventBegin,
                        true, true, true, BVC5_P_GetEventTimestamp());

   /* Flush from the outside in ... */

   /* The GCA only exists in single core configurations 3.3 and earlier,
    * so just handle as part of core flush for simplicity. */
 #if !V3D_VER_AT_LEAST(4,0,2,0)
   if (uiFlushes & BVC5_CACHE_CLEAR_GCA)
   {
      BDBG_ASSERT(hVC5->uiNumCores == 1);
      BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_GCA_CACHE_CTRL, BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, FLUSH, 1));

      if (!V3D_VER_AT_LEAST(3,3,0,0))
      {
         /* while ((BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_GCA_CACHE_STATUS) & BCHP_V3D_GCA_CACHE_STATUS_IDLE_MASK) == 0)
         ; */

         BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_GCA_CACHE_CTRL, 0);
      }
   }
 #endif

   /* Redundant requests to flush/clean will be safely ignored if a flush is still in progress. */
   if (uiFlushes & BVC5_CACHE_FLUSH_L2T)
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL, 1 /* Flush */);

 #if !V3D_VER_AT_LEAST(3,3,0,0)
   if (uiFlushes & BVC5_CACHE_CLEAR_L2C)
   {
      uint32_t uiVal = BCHP_FIELD_DATA(V3D_CTL_L2CACTL, L2CCLR, 1)
                     | BCHP_FIELD_DATA(V3D_CTL_L2CACTL, L2CENA, 1);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2CACTL, uiVal);

   }
 #endif

   if (uiFlushes & (BVC5_CACHE_CLEAR_SIC | BVC5_CACHE_CLEAR_SUC | BVC5_CACHE_CLEAR_L1TD | BVC5_CACHE_CLEAR_L1TC))
   {
      uint32_t uiVal = 0;
      if (uiFlushes & BVC5_CACHE_CLEAR_SIC)  uiVal |= BCHP_FIELD_DATA(V3D_CTL_SLCACTL, ICCS0_to_ICCS3, 0xF);
      if (uiFlushes & BVC5_CACHE_CLEAR_SUC)  uiVal |= BCHP_FIELD_DATA(V3D_CTL_SLCACTL, UCCS0_to_UCCS3, 0xF);
      if (uiFlushes & BVC5_CACHE_CLEAR_L1TD) uiVal |= BCHP_FIELD_DATA(V3D_CTL_SLCACTL, TDCCS0_to_TDCCS3, 0xF);
      if (uiFlushes & BVC5_CACHE_CLEAR_L1TC) uiVal |= BCHP_FIELD_DATA(V3D_CTL_SLCACTL, TVCCS0_to_TVCCS3, 0xF);
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_SLCACTL, uiVal);

      BDBG_ASSERT(!(uiFlushes & BVC5_CACHE_CLEAR_SUC) || !BVC5_P_HardwareCacheClearBlocked(hVC5, uiCoreIndex));
   }

   /* Record on ready bin/render jobs that cache flushes have happened on this core. */
   BVC5_P_MarkJobsFlushedV3D(hVC5, uiCoreIndex);

   BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId++, BVC5_EventEnd,
                        true, true, true, BVC5_P_GetEventTimestamp());
}

static void BVC5_P_HardwareCoreClean(BVC5_Handle hVC5, uint32_t uiCoreIndex, uint32_t uiCleans)
{
   if (uiCleans)
      BDBG_MSG(("BVC5_P_HardwareCoreClean: %x, %.8x", uiCoreIndex, uiCleans));

   /* Clean from inside out. */

 #if V3D_VER_AT_LEAST(3,3,0,0)
   if (uiCleans & BVC5_CACHE_CLEAN_L1TD)
   {
      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL, (1 << 8)); /* l1t write combiner flush */
      while ((BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL) & (1 << 8)) != 0)
         ;
   }
 #endif

   /* Redundant requests to flush/clean will be safely ignored if a flush is still in progress. */
   if (uiCleans & BVC5_CACHE_CLEAN_L2T)
   {
      BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId, BVC5_EventBegin,
                                 false, false, /*clearL2T=*/true, BVC5_P_GetEventTimestamp());

      BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL, 5 /* Clean */);
      while ((BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2TCACTL) & 1) != 0)
         ;

      BVC5_P_AddFlushEvent(hVC5, hVC5->sEventMonitor.uiSchedTrackNextId++, BVC5_EventEnd,
                                 false, false, /*clearL2T=*/true, BVC5_P_GetEventTimestamp());
   }
}

static void BVC5_P_HardwareClean(BVC5_Handle hVC5, uint32_t uiCleans)
{
   /* Process cleans now.
    * TODO for multicore:
    *  - Overlap L2T cleans between cores.
    *  - Clean only cores that might have dirty caches.
    */
   uint32_t uiCoreIndex;
   for (uiCoreIndex = 0; uiCoreIndex != hVC5->uiNumCores; ++uiCoreIndex)
   {
      BVC5_P_HardwareCoreClean(hVC5, uiCoreIndex, uiCleans);
   }

   /* No L3C to clean yet */
   BDBG_ASSERT(!(uiCleans & BVC5_CACHE_CLEAN_L3C));
}

static void BVC5_P_HardwarePrepareForJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
)
{
   bool bMmuPTSwitched;
   /* Power should be on at this point, but we need to acquire it for the
      duration of this job.  This will be released when the job finishes
      in BVC5_P_HardwareJobDone()
    */
   BDBG_ASSERT(hVC5->psCoreStates[uiCoreIndex].uiPowerOnCount > 0);

   BVC5_P_HardwarePowerAcquire(hVC5, 1 << uiCoreIndex);

   bMmuPTSwitched = BVC5_P_HardwareSetupMmu(hVC5, pJob);

   if (bMmuPTSwitched || pJob->uiNeedsCacheFlush & (1 << uiCoreIndex))
   {
      pJob->uiNeedsCacheFlush &= ~(1 << uiCoreIndex);
      BVC5_P_HardwareCoreFlush(hVC5, uiCoreIndex);
   }

}

bool BVC5_P_HardwareIssueBinnerJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
)
{
   BVC5_P_HardwarePrepareForJob(hVC5, uiCoreIndex, pJob);

   if (BVC5_P_IssueBinJob(hVC5, uiCoreIndex, pJob))
      return true;

   /* Release the power as the job was not issued */
   BVC5_P_HardwarePowerRelease(hVC5, 1 << uiCoreIndex);
   return false;
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

void BVC5_P_HardwareProcessBarrierJob(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *pJob
)
{
   uint32_t uiCoreIndex;
   BVC5_JobBarrier *pBarrierJob = (BVC5_JobBarrier *)pJob->pBase;

   for (uiCoreIndex = 0; uiCoreIndex != hVC5->uiNumCores; ++uiCoreIndex)
      BVC5_P_HardwarePowerAcquire(hVC5, 1 << uiCoreIndex);

   /* Clean now. */
   BVC5_P_HardwareClean(hVC5, pBarrierJob->sBase.uiCacheOps & BVC5_CACHE_CLEAN_ALL);

   /* Defer cache flushes. */
   BVC5_P_HardwareDeferCacheFlush(hVC5, pBarrierJob->sBase.uiCacheOps, BVC5_CACHE_FLUSH_ALL);

   for (uiCoreIndex = 0; uiCoreIndex != hVC5->uiNumCores; ++uiCoreIndex)
      BVC5_P_HardwarePowerRelease(hVC5, 1 << uiCoreIndex);

   /* Signal the barrier job has completed. */
   {
      BVC5_ClientHandle hClient = BVC5_P_ClientMapGet(hVC5, hVC5->hClientMap, pJob->uiClientId);
      BVC5_P_ClientJobRunningToCompleted(hVC5, hClient, pJob);
   }
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
         BVC5_P_BinnerState *pState = BVC5_P_HardwareGetBinnerState(hVC5, uiCoreIndex);
         BVC5_P_InternalJob *pJob = pState->psJob;

         pState->psJob      = NULL;
         pState->uiPrevAddr = 0;

         if (hVC5->sPerfCounters.bCountersActive)
            BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiBinnerIdleStartTime);

         BVC5_P_HardwareCoreClean(hVC5, uiCoreIndex, pJob->pBase->uiCacheOps & BVC5_CACHE_CLEAN_ALL);
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
#if !V3D_VER_AT_LEAST(3,3,0,0)
            BVC5_P_InternalJob   *pRunning = pState->psJob[BVC5_P_HW_QUEUE_RUNNING];

            {
            BVC5_P_EventInfo sEventInfo;
            BVC5_P_PopulateEventInfo(hVC5, hVC5->sOpenParams.bGPUMonDeps, /*bCopyTFU=*/false, pRunning, &sEventInfo);
            BVC5_P_AddCoreJobEvent(hVC5, uiCoreIndex, BVC5_P_EVENT_MONITOR_CORE_RENDER_TRACK, BVC5_P_EVENT_MONITOR_RENDERING,
                                   BVC5_EventBegin, &sEventInfo, BVC5_P_GetEventTimestamp());
            }
            if (hVC5->bCollectLoadStats)
               BVC5_P_GetTime_isrsafe(&pRunning->uiRenderStart);
#endif
         }
         else
         {
            if (hVC5->sPerfCounters.bCountersActive)
               BVC5_P_GetTime_isrsafe(&hVC5->sPerfCounters.uiRendererIdleStartTime);
         }

         BVC5_P_HardwareCoreClean(hVC5, uiCoreIndex, pJob->pBase->uiCacheOps & BVC5_CACHE_CLEAN_ALL);
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
   *pCur++ = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_HUB_CTL_IDENT0);
   *pCur++ = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_HUB_CTL_IDENT1);
   *pCur++ = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_HUB_CTL_IDENT2);
   *pCur++ = BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_HUB_CTL_IDENT3);

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
#if V3D_VER_AT_LEAST(3,3,0,0)
   int         i;
#endif

   BSTD_UNUSED(iValue);

   /* We are not interested in these interrupts */
   if (hVC5->bToggling)
      return;

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

#if V3D_VER_AT_LEAST(3,3,0,0)
      for (i = 0; i < BVC5_P_HW_QUEUE_STAGES; i++)
         BVC5_P_HardwareReadEventFifos_isr(hVC5, uiCoreIndex);
#endif
      __sync_fetch_and_or(&hVC5->uiInterruptReason, uiIntStatus);

      BKNI_SetEvent_isr(hVC5->hSchedulerWakeEvent);
   }
}

#if V3D_VER_AT_LEAST(3,3,0,0)
static void BVC5_P_ReportMmuException_isr(
   const char *pName,
   uint32_t    uiMmuCtrl,
   uint32_t    uiVIOAddr,
   uint32_t    uiAxiId,
   uint32_t    uiCap
   )
{
   uint64_t uiAddr = (uint64_t)uiVIOAddr << MMU_VIO_ADDR_SHIFT;

   BKNI_Printf("%s Access violation ctrl: %#x AXI ID: %u VIO ADDR: 0x%x%x\n", pName, uiMmuCtrl, uiAxiId, (uint32_t)(uiAddr >> 32), (uint32_t)(uiAddr & 0xffffffff));
   if (uiMmuCtrl & BCHP_FIELD_DATA(V3D_MMU_0_CTRL, PT_INVALID, 1))
      BKNI_Printf("   PT_INVALID");

   if (uiMmuCtrl & BCHP_FIELD_DATA(V3D_MMU_0_CTRL, WRITE_VIOLATION, 1))
      BKNI_Printf("   WRITE_VIOLATION");

   if (uiMmuCtrl & BCHP_FIELD_DATA(V3D_MMU_0_CTRL, CAP_EXCEEDED, 1))
      BKNI_Printf("   CAP_EXCEEDED (%#x)", uiCap);

   BKNI_Printf("\n");
}
#endif

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
#if V3D_VER_AT_LEAST(3,3,0,0)
   int         i;
#endif

   BSTD_UNUSED(iValue);

   /* We are not interested in these interrupts */
   if (hVC5->bToggling)
      return;

   /* system needs to be tolerant of spurious IRQ's.  Under heavy load, you can get dup's through
      here with the power off, leading to GISB timeouts */
   /* ATOMIC as written out of the IRQ context */
   if (__sync_fetch_and_and(&hVC5->psCoreStates[uiCoreIndex].bPowered, 0xFFFFFFFF))
   {
      uiEnable = ~BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_TFU_TFUINT_MSK_STS);

      uiIntStatus = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_TFU_TFUINT_STS) & uiEnable;

      if (BCHP_GET_FIELD_DATA(uiIntStatus, V3D_TFU_TFUINT_STS, INT_TFUC))
         BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_TFU_TFUINT_CLR,
                                         BCHP_FIELD_DATA(V3D_TFU_TFUINT_CLR, INT_TFUC, 1));

      __sync_fetch_and_or(&hVC5->uiTFUInterruptReason, uiIntStatus);

#if V3D_VER_AT_LEAST(3,3,0,0)
      uiEnable = BCHP_FIELD_DATA(V3D_HUB_CTL_INT_STS, INT_MMU_PTI, 1) |
                 BCHP_FIELD_DATA(V3D_HUB_CTL_INT_STS, INT_MMU_WRV, 1) |
                 BCHP_FIELD_DATA(V3D_HUB_CTL_INT_STS, INT_MMU_CAP, 1);

      uiIntStatus = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_HUB_CTL_INT_STS) & uiEnable;

      /*
       * GFXH1356: We need to read a non-INT HUB CTL register after reading
       * the interrupt status (TFU and HUB), otherwise future MMU(C) register
       * reads will randomly have incorrect bits set in them.
       *
       * Any reads of the MMU(C) registers in other thread contexts must be
       * placed in a critical section so they are atomic relative to this
       * code sequence.
       */
      {
         uint32_t uiGFXH1356;
         uiGFXH1356 = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_HUB_CTL_AXICFG);
         BSTD_UNUSED(uiGFXH1356);
      }

      if (uiIntStatus)
      {
         uint32_t uiMmuIntFlagsMask =
            BCHP_FIELD_DATA(V3D_MMU_0_CTRL, PT_INVALID, 1) |
            BCHP_FIELD_DATA(V3D_MMU_0_CTRL, WRITE_VIOLATION, 1) |
            BCHP_FIELD_DATA(V3D_MMU_0_CTRL, CAP_EXCEEDED, 1);
         uint32_t uiMmuCtrl;
         uint32_t uiAxiId;
         uint64_t uiVIOAddr;
         uint32_t uiCap;

         /*
          * Clear the HUB interrupts, we cannot do that from the MMUs
          */
         BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_HUB_CTL_INT_CLR, uiIntStatus);

         /*
          * Both MMUs are Or'd into the same HUB interrupt lines so we have to
          * find out which generated the interrupt
          */
         uiMmuCtrl = BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_MMU_0_CTRL);
         if (uiMmuCtrl & uiMmuIntFlagsMask)
         {
            uiAxiId = BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_MMU_0_VIO_ID);
            uiVIOAddr = BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_MMU_0_VIO_ADDR);
            uiCap = (BVC5_P_ReadRegister_isr(hVC5, 0, BCHP_V3D_MMU_0_ADDR_CAP));

            /*
             * Writing 0 to the flag bits in the MMU ctrl register clears them
             */
            BVC5_P_WriteRegister_isr(hVC5, 0, BCHP_V3D_MMU_0_CTRL, uiMmuCtrl & ~uiMmuIntFlagsMask);

            BVC5_P_ReportMmuException_isr("MMU 0", uiMmuCtrl, uiVIOAddr, uiAxiId, uiCap);
         }

#if !V3D_VER_AT_LEAST(4,0,2,0)
         uiMmuCtrl = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_MMU_T_CTRL);
         if (uiMmuCtrl & uiMmuIntFlagsMask)
         {
            uiAxiId = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_MMU_T_VIO_ID);
            uiVIOAddr = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_MMU_T_VIO_ADDR);
            uiCap = BVC5_P_ReadNonCoreRegister_isr(hVC5, BCHP_V3D_MMU_T_ADDR_CAP);

            BVC5_P_WriteNonCoreRegister_isr(hVC5, BCHP_V3D_MMU_T_CTRL, uiMmuCtrl & ~uiMmuIntFlagsMask);

            BVC5_P_ReportMmuException_isr("MMU T", uiMmuCtrl, uiVIOAddr, uiAxiId, uiCap);
         }
#endif
      }

      for (i = 0; i < BVC5_P_HW_QUEUE_STAGES; i++)
         BVC5_P_HardwareReadEventFifosHub_isr(hVC5, uiCoreIndex);
#endif

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

static bool BVC5_P_HardwareIsCoreIdle(
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

static bool BVC5_P_HardwareIsTFUIdle(
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

      /* on platforms with PLL_CH, hold it open using reference count for performance reasons.
         To allow for standby, it needs decrementing a 2nd time to allow sleep */
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH
      BCHP_PWR_ReleaseResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH);
#endif

      if (!hVC5->sOpenParams.bUsePowerGating)
      {
         BVC5_P_HardwareBPCMPowerDown(hVC5);
         BVC5_P_HardwarePLLDisable(hVC5);
      }

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

      /* on platforms with PLL_CH, hold it open using reference count for performance reasons. */
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH
      BCHP_PWR_AcquireResource(hVC5->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH);
#endif

      if (!hVC5->sOpenParams.bUsePowerGating)
      {
         /* make sure PLLs are running prior to hitting BPCM.  Note! do not
            call BVC5_P_HardwarePowerAcquire() as it'll in turn reset the HW
            which will not work as BPCM is not up yet */
         BVC5_P_HardwarePLLEnable(hVC5);
         BVC5_P_HardwareBPCMPowerUp(hVC5);
      }

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

bool BVC5_P_HardwareBinBlocked(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex,
   BVC5_P_InternalJob *pNewJob
)
{
   uint32_t i;

   BVC5_P_CoreState *pCoreState = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_JobBin *pBin = (BVC5_JobBin *)pNewJob->pBase;

   for (i = 0; i < BVC5_P_HW_QUEUE_STAGES; ++i)
   {
      BVC5_P_InternalJob *pRenderJob = pCoreState->sRenderState.psJob[i];
      if (pRenderJob != NULL)
      {
         BVC5_JobRender *pRender = (BVC5_JobRender *)pRenderJob->pBase;

         if ((pRender->uiFlags | pBin->uiFlags) & BVC5_NO_BIN_RENDER_OVERLAP)
            return true;
      }
   }
   return false;
}

bool BVC5_P_HardwareRenderBlocked(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex,
   BVC5_P_InternalJob *pNewJob
)
{
   BVC5_P_CoreState *pCoreState = &hVC5->psCoreStates[uiCoreIndex];
   BVC5_JobRender *pRender = (BVC5_JobRender *)pNewJob->pBase;

   {
      BVC5_P_InternalJob *pBinJob = pCoreState->sBinnerState.psJob;
      if (pBinJob != NULL)
      {
         BVC5_JobBin *pBin = (BVC5_JobBin *)pBinJob->pBase;

         if ((pRender->uiFlags | pBin->uiFlags) & BVC5_NO_BIN_RENDER_OVERLAP)
            return true;
      }
   }
   return false;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

typedef struct BVC5_P_PerformanceStats
{
   uint64_t begin;
   uint64_t end;
   bool valid;
} BVC5_P_PerformanceStats;

static void BVC5_P_LogData_isr(
   BVC5_Handle                hVC5,
   uint32_t                   uiCoreIndex,
   BVC5_P_PerformanceStats   *psStats,
   BVC5_P_JobQueue           *psQueue,
   int                        iTrack,
   int                        iEvent)
{
   if (psStats->valid)
   {
      BVC5_P_EventInfo *psEventInfo = BVC5_P_ReceiveMessage_isrsafe(psQueue);
      if (psEventInfo)
      {
         uint64_t uiDuration = psStats->end - psStats->begin;
         BVC5_P_AddCoreJobEvent_isr(hVC5, uiCoreIndex, iTrack, iEvent, BVC5_EventBegin, psEventInfo, psEventInfo->uiTimeStamp);
         BVC5_P_AddCoreJobEvent_isr(hVC5, uiCoreIndex, iTrack, iEvent, BVC5_EventEnd, psEventInfo, psEventInfo->uiTimeStamp + uiDuration);
         BVC5_P_ReleaseMessage_isrsafe(psQueue, psEventInfo);
      }
   }
}

static void BVC5_P_LogTFUData_isr(
   BVC5_Handle                hVC5,
   BVC5_P_PerformanceStats   *psStats,
   BVC5_P_JobQueue           *psQueue)
{
   if (psStats->valid)
   {
      BVC5_P_EventInfo *psEventInfo = BVC5_P_ReceiveMessage_isrsafe(psQueue);
      if (psEventInfo)
      {
         uint64_t uiDuration = psStats->end - psStats->begin;
         BVC5_P_AddTFUJobEvent_isr(hVC5, BVC5_EventBegin, psEventInfo, psEventInfo->uiTimeStamp);
         BVC5_P_AddTFUJobEvent_isr(hVC5, BVC5_EventEnd, psEventInfo, psEventInfo->uiTimeStamp + uiDuration);
         BVC5_P_ReleaseMessage_isrsafe(psQueue, psEventInfo);
      }
   }
}

static uint64_t BVC5_P_GetData_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   uint32_t             uiRegHI,
   uint32_t             uiRegLO
)
{
   uint64_t val;
   val = (uint64_t)BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, uiRegLO);
   val = val | ((uint64_t)BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, uiRegHI) << 32);
   return val / hVC5->sEventMonitor.uiCyclesPerUs;
}

static uint64_t BVC5_P_GetNonCoreData_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiRegHI,
   uint32_t             uiRegLO
)
{
   uint64_t val;
   /* Must read the LO part first as this latches the HI result */
   val = (uint64_t)BVC5_P_ReadNonCoreRegister_isr(hVC5, uiRegLO);
   val = val | ((uint64_t)BVC5_P_ReadNonCoreRegister_isr(hVC5, uiRegHI) << 32);
   return val / hVC5->sEventMonitor.uiCyclesPerUs;
}

#define GetStats_isr(res, counterRegS, counterRegE)\
do {\
   res.begin = BVC5_P_GetData_isr(hVC5, uiCoreIndex, BCHP_V3D_CLE_##counterRegS##HI, BCHP_V3D_CLE_##counterRegS##LO);\
   res.end = BVC5_P_GetData_isr(hVC5, uiCoreIndex, BCHP_V3D_CLE_##counterRegE##HI, BCHP_V3D_CLE_##counterRegE##LO);\
   res.valid = true;\
} while(0)

#define GetTFUStats_isr(res, counterRegS, counterRegE)\
do {\
   res.begin = BVC5_P_GetNonCoreData_isr(hVC5, BCHP_V3D_TFU_##counterRegS##HI, BCHP_V3D_TFU_##counterRegS##LO);\
   res.end = BVC5_P_GetNonCoreData_isr(hVC5, BCHP_V3D_TFU_##counterRegE##HI, BCHP_V3D_TFU_##counterRegE##LO);\
   res.valid = true;\
} while(0)

enum
{
   BVC5_CLE_BIN = 0,
   BVC5_PTB_BIN = 1,
   BVC5_CLE_RENDER = 2,
   BVC5_TLB_RENDER = 3,
   BVC5_PC_LAST = 4   /* LAST VALUE */
};

static void BVC5_P_HardwareReadEventFifos_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex
   )
{
   BVC5_P_PerformanceStats stats[BVC5_PC_LAST];
   BKNI_Memset(stats, 0, sizeof(stats));

   if (hVC5->bCollectLoadStats || hVC5->sEventMonitor.bActive)
   {
      uint32_t status = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_CLE_CCNTCS);

      /* Create a mask for all the fifo not-empty bits */
      const uint32_t hasDataMask =
         BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CBEND_NE, 1)    |
         BCHP_FIELD_DATA(V3D_CLE_CCNTCS, PBEND_NE, 1)    |
         BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CREND_NE, 1)    |
         BCHP_FIELD_DATA(V3D_CLE_CCNTCS, TREND_NE, 1);

      /* Be aware - 3.3.0 hardware has a bug in the counter fifos.
       * Reading from the CTRE fifo actually pops both CTRE and CTRS fifos and
       * clears both status bit if applicable. Reading CTRS doesn't clear any status bits.
       * To work around this, we ignore all start status bits and read both
       * start and end fifos when we see an end status bit.
       * This has the advantage that the same code will work when the h/w bug is fixed.
       */

      if ((status & BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CCNT_EN, 1)) &&
            (status & hasDataMask))
      {
         /* We have some event data to handle in the fifos and they are enabled */
         if (status & BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CBEND_NE, 1))
            GetStats_isr(stats[BVC5_CLE_BIN], CCBS, CCBE);

         if (status & BCHP_FIELD_DATA(V3D_CLE_CCNTCS, PBEND_NE, 1))
            GetStats_isr(stats[BVC5_PTB_BIN], CPBS, CPBE);

         if (status & BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CREND_NE, 1))
            GetStats_isr(stats[BVC5_CLE_RENDER], CCRS, CCRE);

         if (status & BCHP_FIELD_DATA(V3D_CLE_CCNTCS, TREND_NE, 1))
            GetStats_isr(stats[BVC5_TLB_RENDER], CTRS, CTRE);
      }

      if (hVC5->bCollectLoadStats && stats[BVC5_TLB_RENDER].valid)
      {
         BVC5_P_EventInfo *psEventInfo = BVC5_P_ReceiveMessage_isrsafe(&hVC5->sEventMonitor.sRenderJobQueueCLELoadStats);
         if (psEventInfo)
         {
            BVC5_P_ClientMapUpdateStats_isrsafe(hVC5->hClientMap, psEventInfo->uiClientId, stats[BVC5_TLB_RENDER].end - stats[BVC5_TLB_RENDER].begin);
            BVC5_P_ReleaseMessage_isrsafe(&hVC5->sEventMonitor.sRenderJobQueueCLELoadStats, psEventInfo);
         }
      }

      if (hVC5->sEventMonitor.bActive)
      {
         BVC5_P_LogData_isr(hVC5, uiCoreIndex, &stats[BVC5_CLE_BIN], &hVC5->sEventMonitor.sBinJobQueueCLE, BVC5_P_EVENT_MONITOR_CORE_CLE_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING);
         BVC5_P_LogData_isr(hVC5, uiCoreIndex, &stats[BVC5_PTB_BIN], &hVC5->sEventMonitor.sBinJobQueuePTB, BVC5_P_EVENT_MONITOR_CORE_PTB_BIN_TRACK, BVC5_P_EVENT_MONITOR_BINNING);
         BVC5_P_LogData_isr(hVC5, uiCoreIndex, &stats[BVC5_CLE_RENDER], &hVC5->sEventMonitor.sRenderJobQueueCLE, BVC5_P_EVENT_MONITOR_CORE_CLE_RDR_TRACK, BVC5_P_EVENT_MONITOR_RENDERING);
         BVC5_P_LogData_isr(hVC5, uiCoreIndex, &stats[BVC5_TLB_RENDER], &hVC5->sEventMonitor.sRenderJobQueueTLB, BVC5_P_EVENT_MONITOR_CORE_TLB_RDR_TRACK, BVC5_P_EVENT_MONITOR_RENDERING);
      }
   }
}

static void BVC5_P_HardwareReadEventFifosHub_isr(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex
   )
{
   if (hVC5->sEventMonitor.bActive)
   {
      /* Now do the same for the TFU counters */
      uint32_t status = BVC5_P_ReadRegister_isr(hVC5, uiCoreIndex, BCHP_V3D_TFU_CCCS);

      const uint32_t hasTFUDataMask =
         BCHP_FIELD_DATA(V3D_TFU_CCCS, TFU_END_NE, 1);

      if ((status & BCHP_FIELD_DATA(V3D_TFU_CCCS, CCNT_EN, 1)) &&
          (status & hasTFUDataMask))
      {
         BVC5_P_PerformanceStats stats;
         GetTFUStats_isr(stats, SCC, ECC);
         BVC5_P_LogTFUData_isr(hVC5, &stats, &hVC5->sEventMonitor.sQueueTFU);
      }
   }
}

void BVC5_P_HardwareClearEventFifos(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   )
{
   /* Enable & clear the cycle counters in the core and the TFU*/
   BVC5_P_WriteRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CCNTCS,
                        BCHP_FIELD_DATA(V3D_CLE_CCNTCS, CCNT_EN, 1) |
                        BCHP_FIELD_DATA(V3D_CLE_CCNTCS, FIFO_CLR, 1));

   BVC5_P_WriteNonCoreRegister(hVC5, BCHP_V3D_TFU_CCCS,
                               BCHP_FIELD_DATA(V3D_TFU_CCCS, CCNT_EN, 1) |
                               BCHP_FIELD_DATA(V3D_TFU_CCCS, FIFO_CLR, 1));
}
#endif

uint64_t BVC5_P_GetEventTimestamp(
   void
   )
{
   uint64_t val = 0;
   BVC5_P_GetTime_isrsafe(&val);
   return val;
}
