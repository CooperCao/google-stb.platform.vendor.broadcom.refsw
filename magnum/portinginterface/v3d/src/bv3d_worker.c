/***************************************************************************
 *     (c)2012-2013 Broadcom Corporation
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
#include "bv3d.h"
#include "bv3d_priv.h"
#include "bv3d_worker_priv.h"
#include "bv3d_binmem_priv.h"
#include "bkni_multi.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if (BCHP_CHIP == 7445)
#include "bchp_clkgen.h"
#endif

#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#include "bchp_v3d_qps.h"
#include "bchp_v3d_vpm.h"
#include "bchp_v3d_gca.h"
#include "bchp_v3d_cle.h"
#include "bchp_v3d_ptb.h"
#include "bchp_v3d_pctr.h"

#if ((BCHP_CHIP == 7425) && (BCHP_VER >= BCHP_VER_B0))
#include "bchp_v3d_top_gr_bridge.h"
#elif (BCHP_CHIP == 7435) || \
      (BCHP_CHIP == 7445) || \
      (BCHP_CHIP == 7145) || \
      (BCHP_CHIP == 7364) || \
      (BCHP_CHIP == 7366) || \
      (BCHP_CHIP == 7439) || \
      (BCHP_CHIP == 74371) || \
      (BCHP_CHIP == 7586) || \
      (BCHP_CHIP == 11360)
#include "bchp_v3d_top_gr_bridge.h"
#else
#include "bchp_gfx_gr.h"
#endif

BDBG_MODULE(BV3D_P);

static void BV3D_P_TuneQPURatios(BV3D_Handle hV3d);
static void BV3D_P_SetQPURatios(BV3D_Handle hV3d, uint32_t uiNumVertQPUS, uint32_t uiNumFragQPUS, uint32_t uiBinPct, uint32_t uiRdrPct);

void BV3D_P_GetTimeNow(BV3D_EventTime *t)
{
   uint64_t now;
   BV3D_P_GetTime_isrsafe(&now);
   t->uiSecs = now / 1000000;
   t->uiMicrosecs = now - ((uint64_t)t->uiSecs * 1000000);
}

/*********************************************************************************/
static void BV3D_P_ResetBinner(BV3D_Handle hV3d)
{
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT0CS, 1 << 15);      /* Reset */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT0CA, 0);            /* Set current and end address */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT0EA, 0);
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_BFC,   1);            /* Clear binning mode flush count to clear BMACTIVE in PCS */

   /* Give the PTB some bogus memory to chew on to clear OOM */
   BREG_Write32(hV3d->hReg, BCHP_V3D_PTB_BPOA, BV3D_P_BinMemGetOverspill(hV3d->hBinMemManager));
   BREG_Write32(hV3d->hReg, BCHP_V3D_PTB_BPOS, 8192);
   BREG_Write32(hV3d->hReg, BCHP_V3D_PTB_BPOS, 0);

   while (BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_PCS) & (1 << 8))
      continue;
}

/*********************************************************************************/
static void BV3D_P_ResetRender(BV3D_Handle hV3d)
{
   /* TODO make an abandon bin instruction */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT1CS, 1 << 15);   /* Reset */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT1CA, 0);         /* Set current and end address */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT1EA, 0);
   BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_RFC,   1);         /* Clear binning mode flush count to clear RMACTIVE in PCS */
}

/***************************************************************************/
static void BV3D_P_DebugDumpJob(BV3D_Handle hV3d, BV3D_Job *job)
{
   if (job)
   {
      void  (*pCallback)(uint32_t, void *);
      void  *pContext;
      uint32_t uiClientPID;
      /* grab the PID of the failing process */
      BV3D_P_CallbackMapGet(hV3d->hCallbackQ, job->uiClientId, &uiClientPID, &pContext, &pCallback);

      BKNI_Printf("    Client         = %d (PID = %d)\n", job->uiClientId, uiClientPID);
      BKNI_Printf("    CurrentInstr   = %d\n", job->uiCurrentInstr);
      BKNI_Printf("    InstrsLeft     = %d\n", job->uiInstrCount);
      BKNI_Printf("    BinMemory      = %p\n", job->uiBinMemory);
      BKNI_Printf("    UserVPM        = %d\n", job->uiUserVPM);
      BKNI_Printf("    Abandon        = %d\n", job->uiAbandon);
      BKNI_Printf("    OutOfMemory    = %d\n", job->uiOutOfMemory);
      BKNI_Printf("    Overspill      = %d\n", job->uiOverspill);
      BKNI_Printf("    CollectTime    = %d\n", job->bCollectTimeline ? 1 : 0);
      BKNI_Printf("    CallbackParam  = %d\n", job->uiNotifyCallbackParam);
      BKNI_Printf("    CallbackSeqNum = %d\n", job->uiNotifySequenceNum);
   }
}

/***************************************************************************/
static void BV3D_P_DebugDump(BV3D_Handle hV3d)
{
   BKNI_Printf("\n***********************************************************\n");
   BKNI_Printf("GPU Recovery Dump:\n\n");

   BKNI_Printf("InterruptReason   = %d\n", hV3d->interruptReason);
   BKNI_Printf("PowerOnCount      = %d\n", hV3d->powerOnCount);
   BKNI_Printf("ReallyPoweredOn   = %d\n", hV3d->reallyPoweredOn ? 1 : 0);
   BKNI_Printf("isStandby         = %d\n", hV3d->isStandby ? 1 : 0);
   BKNI_Printf("QuiescentTimeMs   = %d\n", hV3d->quiescentTimeMs);
   BKNI_Printf("TimeoutCount      = %d\n", hV3d->timeoutCount);
   BKNI_Printf("PrevBinAddress    = %p\n", hV3d->prevBinAddress);
   BKNI_Printf("PrevRenderAddress = %p\n", hV3d->prevRenderAddress);
   BKNI_Printf("WaitQSize         = %d\n", BV3D_P_WaitQSize(hV3d->hWaitQ));
   BKNI_Printf("UserVPM           = %d\n", hV3d->uiUserVPM);
   BKNI_Printf("NextClientId      = %d\n", hV3d->uiNextClientId);
   BKNI_Printf("ScheduleFirst     = %d\n", hV3d->uiScheduleFirst);
   BKNI_Printf("PerfMonitoring    = %d\n", hV3d->bPerfMonitorActive ? 1 : 0);
   BKNI_Printf("PerfMonHwBank     = %d\n", hV3d->uiPerfMonitorHwBank);
   BKNI_Printf("PerfMonMemBank    = %d\n", hV3d->uiPerfMonitorMemBank);

   BKNI_Printf("\n");

   BV3D_P_BinMemDebugDump(hV3d->hBinMemManager);

   BKNI_Printf("\n");

   BKNI_Printf("Current Bin Job\n");
   if (!BV3D_P_InstructionIsClear(&hV3d->sBin))
   {
      BKNI_Printf("Arg1 = %08X, Arg2 = %08X, CallbackParam = %08X\n", hV3d->sBin.uiArg1, hV3d->sBin.uiArg2,
                                                                      hV3d->sBin.uiCallbackParam);
      BV3D_P_DebugDumpJob(hV3d, hV3d->sBin.psJob);
   }

   BKNI_Printf("Current Render Job\n");
   if (!BV3D_P_InstructionIsClear(&hV3d->sRender))
   {
      BKNI_Printf("Arg1 = %08X, Arg2 = %08X, CallbackParam = %08X\n", hV3d->sRender.uiArg1, hV3d->sRender.uiArg2,
                                                                      hV3d->sRender.uiCallbackParam);
      BV3D_P_DebugDumpJob(hV3d, hV3d->sRender.psJob);
   }

   BKNI_Printf("Current User Job\n");
   if (!BV3D_P_InstructionIsClear(&hV3d->sUser))
   {
      BKNI_Printf("Arg1 = %08X, Arg2 = %08X, CallbackParam = %08X\n", hV3d->sUser.uiArg1, hV3d->sUser.uiArg2,
                                                                      hV3d->sUser.uiCallbackParam);
      BV3D_P_DebugDumpJob(hV3d, hV3d->sUser.psJob);
   }

   BKNI_Printf("\n"
      "INTCTL    %08X,  INTENA   %08X\n"
      "CT0CA     %08X,  CT0EA    %08X,  CT0CS  %08X\n"
      "CT1CA     %08X,  CT1EA    %08X,  CT1CS  %08X\n"
      "CT00RA0   %08X,  CT0LC    %08X,  CT0PC  %08X\n"
      "CT01RA0   %08X,  CT1LC    %08X,  CT1PC  %08X\n"
      "PCS       %08X,  BFC      %08X,  RFC    %08X\n"
      "BPCA      %08X,  BPCS     %08X,  BPOA   %08X,  BPOS   %08X\n",
      BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_INTCTL),   BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_INTENA),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0CA),    BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0EA),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0CS),    BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1CA),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1EA),    BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1CS),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT00RA0),  BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0LC),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0PC),    BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT01RA0),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1LC),    BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1PC),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_PCS),      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_BFC),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_RFC),      BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPCA),
      BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPCS),     BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPOA),
      BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPOS)
      );

   BKNI_Printf(
      "VPMBASE   %08X,  VPACNTL  %08X\n"
      "L2CACTL   %08X,  SLCACTL  %08X\n"
      "DBQITC    %08X,  DBQITE   %08X\n"
      "SCRATCH   %08X\n"
      "IDENT0    %08X,  IDENT1   %08X,  IDENT2 %08X,  IDENT3 %08X\n"
      "ERRSTAT   %08X,  DBGE     %08X,  FDBG0  %08X,  FDBGB  %08X\n"
      "FDBGR     %08X,  FDBGS    %08X,  BXCF   %08X\n\n",
      BREG_Read32(hV3d->hReg, BCHP_V3D_VPM_VPMBASE),  BREG_Read32(hV3d->hReg, BCHP_V3D_VPM_VPACNTL),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_L2CACTL),  BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_SLCACTL),
      BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_DBQITC),   BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_DBQITE),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_SCRATCH),  BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT0),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT1),   BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT2),
      BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT3),   BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_ERRSTAT),
      BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_DBGE),     BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_FDBG0),
      BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_FDBGB),    BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_FDBGR),
      BREG_Read32(hV3d->hReg, BCHP_V3D_DBG_FDBGS),    BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BXCF)
      );
}

/***************************************************************************/
void BV3D_P_PowerOn(
   BV3D_Handle hV3d
   )
{
   BDBG_ENTER(BV3D_P_PowerOn);

#ifdef BCHP_PWR_SUPPORT
   if (!hV3d->reallyPoweredOn)
   {
      /* power on */
      BDBG_MSG(("Power On"));
      BCHP_PWR_AcquireResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
      BCHP_PWR_AcquireResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
      hV3d->reallyPoweredOn = true;

#ifdef V3D_HANDLED_VIA_L2
      if (hV3d->callback_intctl)
         BINT_EnableCallback(hV3d->callback_intctl);
#endif
   }
#endif

   /* NOTE: This is also used by the lockup detector, so don't move it inside the ifdef above */
   hV3d->powerOnCount++;

   BDBG_LEAVE(BV3D_P_PowerOn);
}

/***************************************************************************/
void BV3D_P_PowerOff(
   BV3D_Handle hV3d
   )
{
   BDBG_ENTER(BV3D_P_PowerOff);

   /* This just adjusts the notional power count. The worker thread timeouts will eventually
      trigger a real power off when enough time has elapsed. */
   BDBG_ASSERT(hV3d->powerOnCount > 0);
   hV3d->powerOnCount--;

   BDBG_LEAVE(BV3D_P_PowerOff);
}

/***************************************************************************/
void BV3D_P_ReallyPowerOff(
   BV3D_Handle hV3d
   )
{
#ifdef BCHP_PWR_SUPPORT
   /* power off */
   if (hV3d->reallyPoweredOn)
   {
#ifdef V3D_HANDLED_VIA_L2
      if (hV3d->callback_intctl)
         BINT_DisableCallback(hV3d->callback_intctl);
#endif

      BDBG_MSG(("Power Off"));
      BCHP_PWR_ReleaseResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D);
      BCHP_PWR_ReleaseResource(hV3d->hChp, BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM);
      hV3d->reallyPoweredOn = false;
   }
#else
   BSTD_UNUSED(hV3d);
#endif
}

/***************************************************************************/

void BV3D_P_SupplyBinner(
   BV3D_Handle hV3d,
   uint32_t    uiAddr)
{
   BDBG_ASSERT(uiAddr != 0);

   if (uiAddr == 0)
      return;

   /* Program binner overspill registers */
   BREG_Write32(hV3d->hReg, BCHP_V3D_PTB_BPOA, uiAddr);
   BREG_Write32(hV3d->hReg, BCHP_V3D_PTB_BPOS, BV3D_P_BinMemGetChunkSize(hV3d->hBinMemManager));

   /* the BPOS write doesn't take effect immediately: wait for the oom bit to go low... */
   while (BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_PCS) & (1 << 8))
      continue;
}

/***************************************************************************/
void BV3D_P_ResetPerfMonitorHWCounters(
   BV3D_Handle hV3d
)
{
   BV3D_P_PowerOn(hV3d);

   BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRC, 0xFFFF);

   BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, 1);
   BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, 0);

   BV3D_P_PowerOff(hV3d);
}

/***************************************************************************/
void BV3D_P_GatherPerfMonitor(
   BV3D_Handle hV3d
)
{
   uint32_t i;

   BV3D_P_PowerOn(hV3d);

   if (hV3d->bPerfMonitorActive)
   {
      if (hV3d->uiPerfMonitorHwBank != 0)
      {
         for (i = 0; i < 16; i++)
            hV3d->sPerfData.uiHwCounters[i] += BREG_Read32(hV3d->hReg, BCHP_V3D_PCTR_PCTR0 + (i * 8));
      }

      if (hV3d->uiPerfMonitorMemBank != 0)
      {
         hV3d->sPerfData.uiMemCounters[0] += BREG_Read32(hV3d->hReg, BCHP_V3D_GCA_V3D_BW_CNT);
         hV3d->sPerfData.uiMemCounters[1] += BREG_Read32(hV3d->hReg, BCHP_V3D_GCA_MEM_BW_CNT);
      }

      /* Reset the h/w counters now we've captured them */
      BV3D_P_ResetPerfMonitorHWCounters(hV3d);
   }

   BV3D_P_PowerOff(hV3d);
}

/***************************************************************************/
void BV3D_P_InitPerfMonitor(
   BV3D_Handle hV3d
)
{
   BV3D_P_PowerOn(hV3d);

   /* Setup the counters we will capture*/
   if (hV3d->uiPerfMonitorHwBank == 1)
   {
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS0,  13);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS1,  14);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS2,  15);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS3,  16);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS4,  17);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS5,  18);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS6,  19);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS7,  20);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS8,  21);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS9,  22);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS10, 23);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS11, 24);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS12, 25);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS13, 28);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS14, 29);
   }
   else if (hV3d->uiPerfMonitorHwBank == 2)
   {
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS0,  0);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS1,  1);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS2,  2);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS3,  3);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS4,  4);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS5,  5);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS6,  6);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS7,  7);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS8,  8);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS9,  9);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS10, 10);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS11, 11);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS12, 12);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS13, 26);
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRS14, 27);
   }

   /* Enable counters */
   if (hV3d->bPerfMonitorActive && hV3d->uiPerfMonitorHwBank != 0)
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRE, 0x80007FFF);
   else
      BREG_Write32(hV3d->hReg, BCHP_V3D_PCTR_PCTRE, 0);

   if (hV3d->uiPerfMonitorMemBank == 1)
   {
      BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, 1);
      BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, 0);
   }
   else if (hV3d->uiPerfMonitorMemBank == 2)
   {
      BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, 1 | (1 << 2));
      BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_PM_CTRL, (1 << 2));
   }

   BV3D_P_PowerOff(hV3d);
}

/***************************************************************************/
void BV3D_P_ResetCore(
   BV3D_Handle hV3d,
   uint32_t    vpmBase
)
{
   uint32_t uiIdent2;
   bool bDefaultBigEndian;
   bool bEndianSwappable;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
   bool bWantBigEndian = true;
#else
   bool bWantBigEndian = false;
#endif
   BCHP_FeatureData featureData;

   BDBG_ENTER(BV3D_P_ResetCore);

   BDBG_MSG(("Reset Core : VPM = %d", vpmBase));

   BV3D_P_PowerOn(hV3d);

   /* Gather any active counters prior to reset */
   BV3D_P_GatherPerfMonitor(hV3d);

   /* SW7445-1174.  Suffers from spurious IRQ's.
      Stop the IRQ handler from functioning during reset to prevent GISB timeout */
   /* Do this for all chips and kernels, as missing a case could cause failure, whereas doing it will always be OK */
   BKNI_EnterCriticalSection();

#if ((BCHP_CHIP==7425) && (BCHP_VER>=BCHP_VER_B0)) || (BCHP_CHIP==7435) || (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || \
     (BCHP_CHIP==7364) || (BCHP_CHIP==7366) || (BCHP_CHIP == 7439) || (BCHP_CHIP == 74371) || (BCHP_CHIP == 7586)
   BREG_Write32(hV3d->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
   BREG_Write32(hV3d->hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));
#elif (BCHP_CHIP==7231) || (BCHP_CHIP==7584) || (BCHP_CHIP==75845)
   BREG_Write32(hV3d->hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
   BREG_Write32(hV3d->hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));
#elif (BCHP_CHIP==11360)
   BV3D_P_OsSoftReset(hV3d);
#else
   BREG_Write32(hV3d->hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_TOP_CLK_108_SW_INIT, ASSERT));
   BREG_Write32(hV3d->hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_TOP_CLK_108_SW_INIT, DEASSERT));
#endif

   BKNI_LeaveCriticalSection();

   BCHP_GetFeature(hV3d->hChp, BCHP_Feature_eProductId, &featureData);

   if (hV3d->uiNumSlices == 0)
   {
      uint32_t          uiIdent1;

      uiIdent1 = BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT1);
      hV3d->uiNumSlices = (uiIdent1 >> 4) & 0xF;

      if (featureData.data.productId == 0x7250)
         hV3d->uiNumSlices--;
   }

   uiIdent2 = BREG_Read32(hV3d->hReg, BCHP_V3D_CTL_IDENT2);
   bDefaultBigEndian = ((uiIdent2 >> 20) & 0x1) != 0;
   bEndianSwappable = ((uiIdent2 >> 21) & 0x1) != 0;

   /* Setup the endian handling (when it's available) */
   /*  coverity[dead_error_condition] */
   if (bWantBigEndian)
   {
      if (!bDefaultBigEndian && !bEndianSwappable)
      {
         BDBG_ERR(("This V3D variant does not support big-endian"));
         BDBG_ASSERT(0);
      }
#ifdef BCHP_V3D_CTL_ENDSWP
      else if (bEndianSwappable)
         BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_ENDSWP, 1);
#endif
   }
   else
   {
      if (bDefaultBigEndian && !bEndianSwappable)
      {
         BDBG_ERR(("This V3D variant does not support little-endian"));
         BDBG_ASSERT(0);
      }
#ifdef BCHP_V3D_CTL_ENDSWP
      else if (bEndianSwappable)
         BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_ENDSWP, 0);
#endif
   }

   BREG_Write32(hV3d->hReg, BCHP_V3D_VPM_VPMBASE, vpmBase);

   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTDIS, ~0);

   /* clear interrupts */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTCTL, ~0);
   BREG_Write32(hV3d->hReg, BCHP_V3D_DBG_DBQITC, ~0); /* clear them just incase */
   BREG_Write32(hV3d->hReg, BCHP_V3D_DBG_DBQITE, ~0); /* make sure qpu interrupts are enabled */
   /* enable DBG block or user shader IRQs don't work */
   BREG_Write32(hV3d->hReg, BCHP_V3D_DBG_DBCFG, 1);

   BV3D_P_ClearV3dCaches(hV3d);

   /* reset error bit and counts... */
   BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SRQCS,
      (1 << BCHP_V3D_QPS_SRQCS_QPURQERR_SHIFT) |
      (1 << BCHP_V3D_QPS_SRQCS_QPURQCM_SHIFT) |
      (1 << BCHP_V3D_QPS_SRQCS_QPURQCC_SHIFT));

   /* Set up overspill memory for binner to prevent immediate out-of-memory condition
    * The h/w will start writing here (a quirk), we will not use this data and it will be
    * overwritten if OOM is triggered.
    */
   BV3D_P_BinMemReset(hV3d->hBinMemManager);
   BV3D_P_ResetBinner(hV3d);
   BV3D_P_ResetRender(hV3d);

   /* Set the top two bits in the SCB remap to reflect the heap address (only do it if required) */
   if (hV3d->uiHeapOffset >> 30)
   {
      uint32_t ctrl;
      ctrl = BREG_Read32(hV3d->hReg, BCHP_V3D_GCA_CACHE_CTRL);
      ctrl &= ~(BCHP_MASK(V3D_GCA_CACHE_CTRL, MEM_MSB_REMAP) | BCHP_MASK(V3D_GCA_CACHE_CTRL, MEM_MSB_REMAP_EN));
      ctrl |= (BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, MEM_MSB_REMAP, hV3d->uiHeapOffset >> 30) |
               BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, MEM_MSB_REMAP_EN, 1));
      BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_CACHE_CTRL, ctrl);
   }

#if (((BCHP_CHIP == 7445) && (BCHP_VER == BCHP_VER_D0)) || \
     ((BCHP_CHIP == 7145) && (BCHP_VER >= BCHP_VER_B0)) || \
     ((BCHP_CHIP == 7366) && (BCHP_VER == BCHP_VER_B0)))
   BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_LOW_PRI_ID, 0xA40000);
#endif

#if (BCHP_CHIP == 7445)
   if (hV3d->uiMdiv != 0)
   {
      uint32_t mask = BCHP_CLKGEN_PLL_MOCA_PLL_CHANNEL_CTRL_CH_3_MDIV_CH3_MASK;
      uint32_t value = BCHP_FIELD_DATA(CLKGEN_PLL_MOCA_PLL_CHANNEL_CTRL_CH_3, MDIV_CH3, hV3d->uiMdiv);
      BREG_AtomicUpdate32(hV3d->hReg, BCHP_CLKGEN_PLL_MOCA_PLL_CHANNEL_CTRL_CH_3, mask, value);
      hV3d->uiMdiv = 0; /* only program once, which could allow it to be changed by BBS */
   }
#endif

   if (hV3d->uiNumSlices < 2)
      hV3d->bDisableAQA = true;

   if (featureData.data.productId == 0x7250)
   {
      hV3d->uiCurQpuSched0 = 0xFFFF0000;
      hV3d->bDisableAQA = true;
   }

   BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SQRSV0, hV3d->uiCurQpuSched0);
   BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SQRSV1, hV3d->uiCurQpuSched1);

   /* Init performance counter banks */
   BV3D_P_InitPerfMonitor(hV3d);

   /* Re-enable interrupts */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTCTL, ~0);
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTENA, 7); /* TODO: use a proper symbols -- don't want overspill taken interrupt */

   /* Reset the lockup detection state */
   hV3d->timeoutCount = 0;

   BV3D_P_PowerOff(hV3d);

   BDBG_LEAVE(BV3D_P_ResetCore);
}

/***************************************************************************/
void BV3D_P_ClearV3dCaches(
   BV3D_Handle hV3d
)
{
   uint32_t uiCacheControl;
   BDBG_ENTER(BV3D_P_ClearV3dCaches);

   /* L2 cache in v3d */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_L2CACTL, 1 << 2);
   /* slices cache control */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_SLCACTL, ~0);

   /* L3 cache reset */
   uiCacheControl = BREG_Read32(hV3d->hReg, BCHP_V3D_GCA_CACHE_CTRL);
   uiCacheControl &= ~(BCHP_MASK(V3D_GCA_CACHE_CTRL, FLUSH));
   BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_CACHE_CTRL, uiCacheControl | BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, FLUSH, 1));
   BREG_Write32(hV3d->hReg, BCHP_V3D_GCA_CACHE_CTRL, uiCacheControl | BCHP_FIELD_DATA(V3D_GCA_CACHE_CTRL, FLUSH, 0));

   BDBG_LEAVE(BV3D_P_ClearV3dCaches);
}

/***************************************************************************/
/* AllUnitsIdle
   Are all h/w units idle? */
bool BV3D_P_AllUnitsIdle(
   BV3D_Handle hV3d
)
{
   bool bIdle = BV3D_P_InstructionIsClear(&hV3d->sBin)    &&
                BV3D_P_InstructionIsClear(&hV3d->sRender) &&
                BV3D_P_InstructionIsClear(&hV3d->sUser);

   return bIdle;
}

/***************************************************************************/
/* AllQueuesEmpty
   Are all the client queues empty? */
bool BV3D_P_AllQueuesEmpty(
   BV3D_Handle hV3d
)
{
   void *pNext;
   bool bEmpty = true;

   BV3D_IQHandle hIQ;

   hIQ = BV3D_P_IQMapFirst(hV3d->hIQMap, &pNext);
   while (bEmpty && hIQ)
   {
      bEmpty = bEmpty && (BV3D_P_IQGetSize(hIQ) == 0);
      hIQ = BV3D_P_IQMapNext(hV3d->hIQMap, &pNext);
   }

   return bEmpty;
}

/***************************************************************************/
/* IsIdle */
bool BV3D_P_IsIdle(
   BV3D_Handle hV3d
)
{
   return BV3D_P_AllUnitsIdle(hV3d) && BV3D_P_AllQueuesEmpty(hV3d);
}

/***************************************************************************/
void BV3D_P_DispatchWaiting(
   BV3D_Handle hV3d
)
{
   BV3D_Job *psJob = BV3D_P_WaitQTop(hV3d->hWaitQ);

   /* Reset the hardware */
   if (psJob != NULL)
   {
      /* reset the hardware using new VPM settings */
      if (psJob->uiUserVPM != hV3d->uiUserVPM)
         BV3D_P_ResetCore(hV3d, psJob->uiUserVPM);

      hV3d->uiUserVPM = psJob->uiUserVPM;
   }
 
   /* Only pop jobs with the same new VPM settings */
   while (psJob != NULL && psJob->uiUserVPM == hV3d->uiUserVPM)
   {
      BV3D_P_UnrollJob(hV3d, psJob);
      BV3D_P_WaitQPop(hV3d->hWaitQ);
      psJob = BV3D_P_WaitQTop(hV3d->hWaitQ);
   }
}

/***************************************************************************/
bool V3D_P_CanWaitProceed(
   BV3D_Handle hV3d,
   BV3D_Instruction *psInstruction
)
{
   BV3D_Job *psJob = psInstruction->psJob;
   uint32_t uiSig  = psInstruction->uiArg1;

   /* What are we waiting for? */
   bool bWaitBin     = (uiSig & BV3D_Signaller_eBinSig)    != 0;
   bool bWaitRender  = (uiSig & BV3D_Signaller_eRenderSig) != 0;
   bool bWaitUser    = (uiSig & BV3D_Signaller_eUserSig)   != 0;

   /* We only need to wait for instructions from our matching clientIds -- a wait can only be held up by instructions from the
      same client. */
   bool  bBinDone    = !bWaitBin    || (BV3D_P_InstructionIsClear(&hV3d->sBin)    || hV3d->sBin.psJob->uiClientId    != psJob->uiClientId);
   bool  bRenderDone = !bWaitRender || (BV3D_P_InstructionIsClear(&hV3d->sRender) || hV3d->sRender.psJob->uiClientId != psJob->uiClientId);
   bool  bUserDone   = !bWaitUser   || (BV3D_P_InstructionIsClear(&hV3d->sUser)   || hV3d->sUser.psJob->uiClientId   != psJob->uiClientId);

   return bBinDone && bRenderDone && bUserDone;
}

/***************************************************************************/
void BV3D_P_InstructionDone(
   BV3D_Handle hV3d,
   BV3D_Job    *psJob
)
{
   if (psJob == NULL)
      return;

   psJob->uiInstrCount -= 1;

   /* Have all the instructions for this job completed? */
   if (psJob->uiInstrCount == 0)
   {
      /* Release bin memory attached to this job */
      BV3D_P_BinMemReleaseByJob(hV3d->hBinMemManager, psJob);

      /* Free up the job */
      BKNI_Free(psJob);
   }
}

/***************************************************************************/
void BV3D_P_DoClientCallback(BV3D_Handle hV3d, BV3D_Instruction *psInstruction, uint32_t *callbackParam, 
                             uint64_t seqNum, bool sync)
{
   void  (*pCallback)(uint32_t, void *);
   void  *pContext;
   uint32_t uiClientPID;

   BV3D_P_NotifyQPush(hV3d->hNotifyQ,
      psInstruction->psJob->uiClientId,
      *callbackParam,
      sync,
      psInstruction->psJob->uiOutOfMemory,
      seqNum,
      psInstruction->psJob->bCollectTimeline ? &psInstruction->psJob->sTimelineData : NULL);

   BV3D_P_CallbackMapGet(hV3d->hCallbackQ, psInstruction->psJob->uiClientId, &uiClientPID, &pContext, &pCallback);

   pCallback(psInstruction->psJob->uiClientId, pContext);

   /* Callback has been done -- mark it so to prevent re-issuing */
   *callbackParam = 0;
}

/***************************************************************************/
void BV3D_P_HardwareDone(
   BV3D_Handle      hV3d,
   BV3D_Instruction *psInstruction
)
{
   uint64_t uiNowUs;

   BDBG_ENTER(BV3D_P_HardwareDone);

   /* This would be a bogus interrupt */
   if (BV3D_P_InstructionIsClear(psInstruction))
      return;

   BV3D_P_GetTime_isrsafe(&uiNowUs);

   if (psInstruction == &hV3d->sBin)
      hV3d->uiCumBinTimeUs += (uiNowUs - hV3d->uiBinStartTimeUs);

   if (psInstruction == &hV3d->sRender)
      hV3d->uiCumRenderTimeUs += (uiNowUs - hV3d->uiRenderStartTimeUs);

   if (hV3d->bCollectLoadStats || psInstruction->psJob->bCollectTimeline)
   {
      if (psInstruction == &hV3d->sRender)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sRenderEnd);
   }

   if (psInstruction->psJob->bCollectTimeline)
   {
      /* Record the end times */
      if (psInstruction == &hV3d->sBin)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sBinEnd);
      else if (psInstruction == &hV3d->sUser)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sUserEnd);
   }

   if (hV3d->bCollectLoadStats && psInstruction == &hV3d->sRender)
   {
      uint64_t renderTime = ((uint64_t)psInstruction->psJob->sTimelineData.sRenderEnd.uiSecs * 1000000 +
                             psInstruction->psJob->sTimelineData.sRenderEnd.uiMicrosecs) -
                            ((uint64_t)psInstruction->psJob->sTimelineData.sRenderStart.uiSecs * 1000000 +
                             psInstruction->psJob->sTimelineData.sRenderStart.uiMicrosecs);

      /* Update the load stats */
      BV3D_P_CallbackMapUpdateStats(hV3d->hCallbackQ, psInstruction->psJob->uiClientId, renderTime);
   }

   /* Is there a callback?  If so issue notification to client and call-back. */
   if (psInstruction->uiCallbackParam != 0)
   {
      BDBG_MSG(("Issuing HW CALLBACK(p=%d) to client %d", psInstruction->uiCallbackParam, psInstruction->psJob->uiClientId));
      BV3D_P_DoClientCallback(hV3d, psInstruction, &psInstruction->uiCallbackParam, 
                              psInstruction->psJob->uiSequence, false);
   }

   /* Is there a notify callback pending? */
   if (psInstruction->psJob->uiNotifyCallbackParam)
   {
      BDBG_MSG(("Issuing HW NOTIFY CALLBACK(p=%d) to client %d", psInstruction->psJob->uiNotifyCallbackParam, psInstruction->psJob->uiClientId));
      BV3D_P_DoClientCallback(hV3d, psInstruction, &psInstruction->psJob->uiNotifyCallbackParam, 
                              psInstruction->psJob->uiNotifySequenceNum, false);
   }

   BV3D_P_InstructionDone(hV3d, psInstruction->psJob);

   /* Unit is finished, so empty the instruction record */
   BV3D_P_InstructionClear(psInstruction);

   /* Power on/off is reference counted, so match this with the power on in submit */
   BV3D_P_PowerOff(hV3d);

   BDBG_LEAVE(BV3D_P_HardwareDone);
}

/***************************************************************************/

/* Out of memory
 *
 * The binner has run out of memory.
 *
 * The first thing we do is to supply the overspill block to the binner to get
 * it going again,
 *
 * Next we mark the overspill as being owned by the current job, then allocate
 * some more memory for next time.
 */
void BV3D_P_OutOfBinMemory(
   BV3D_Handle hV3d,
   BV3D_Job    *psJob
)
{
   BV3D_BinMemManagerHandle   hBinMemManager = hV3d->hBinMemManager;
   uint32_t                   uiOverspill    = BV3D_P_BinMemGetOverspill(hBinMemManager);

   BDBG_ENTER(BV3D_P_OutOfBinMemory);

   /* It can happen that OOM and bin done happen at the same time.
    * We make sure to capture the job so it doesn't disappear.
    */

   /* Kick off the binner again using the current overspill block */
   if (uiOverspill != 0 && (psJob == NULL || psJob->uiAbandon == 0))
   {
      BV3D_P_SupplyBinner(hV3d, uiOverspill);

      if (psJob != NULL)
      {
         /* Record overspill for this job -- may need it if we run out of memory
         * We should never launch a bin job unless there is at least one overspill
         * available, and we should never give the last one away.
         */
         psJob->uiOverspill = uiOverspill;

         /* Attach overspill to job and allocate new overspill */
         BV3D_P_BinMemOverspillUsed(hBinMemManager, psJob);
         BV3D_P_BinMemAllocOverspill(hBinMemManager);
      }
   }
   else
   {
      /* We came round again with no memory - it's the end of the line */
      if (hV3d->sBin.psJob != NULL)
      {
         BDBG_ASSERT(hV3d->sBin.psJob == psJob);
         BDBG_ASSERT(psJob->uiOverspill != 0);

         /* This job has failed, so we can't render it */
         psJob->uiAbandon = true;

         /* Keep supplying the same block until the binner finishes */
         BV3D_P_SupplyBinner(hV3d, psJob->uiOverspill);
      }
   }

   /* We have supplied memory, re-enable the interrupt */
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTCTL, BCHP_V3D_CTL_INTCTL_OFB_MASK);
   BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_INTENA, BCHP_V3D_CTL_INTCTL_OFB_MASK);

   BDBG_LEAVE(BV3D_P_OutOfBinMemory);
}

/***************************************************************************/

void BV3D_P_IssueBin(
   BV3D_Handle      hV3d,
   BV3D_Instruction *psInstruction
)
{
   BV3D_P_PowerOn(hV3d);

   hV3d->sBin = *psInstruction;
   
   if (psInstruction->psJob == NULL)
      return;

   if (psInstruction->psJob->uiAbandon)
   {
      BV3D_P_HardwareDone(hV3d, &hV3d->sBin); /* Fake completion of the task */
   }
   else
   {
      /* Kick the binner off */
      BV3D_P_ClearV3dCaches(hV3d);

      BDBG_MSG(("Issuing BIN job %p for client %d", psInstruction->psJob, psInstruction->psJob->uiClientId));

      BV3D_P_GetTime_isrsafe(&hV3d->uiBinStartTimeUs);

      if (psInstruction->psJob->bCollectTimeline && psInstruction->psJob->sTimelineData.sBinStart.uiSecs == 0)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sBinStart);

      BDBG_MSG(("Binner start = %p, end = %p", psInstruction->uiArg1, psInstruction->uiArg2));
      BDBG_MSG(("BPCA = %p, BPCS = %p\n", BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPCA), BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPCS)));

      /* If the binner has used EXACTLY the amount of bytes in the overspill buffer, we can get here with
         BPCS==0. If we start another bin job like this we will fail somewhere later, so ensure we give some more
         bin memory now */
      if (BREG_Read32(hV3d->hReg, BCHP_V3D_PTB_BPCS) == 0)
      {
         uint32_t uiOverspill = BV3D_P_BinMemGetOverspill(hV3d->hBinMemManager);
         BDBG_ASSERT(uiOverspill);

         BV3D_P_SupplyBinner(hV3d, uiOverspill);

         psInstruction->psJob->uiOverspill = uiOverspill;

         /* Attach overspill to job and allocate new overspill */
         BV3D_P_BinMemOverspillUsed(hV3d->hBinMemManager, psInstruction->psJob);
         BV3D_P_BinMemAllocOverspill(hV3d->hBinMemManager);
      }

      /* Point binner instruction counters at control list */
      BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT0CA, psInstruction->uiArg1);
      BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT0EA, psInstruction->uiArg2);
   }
}

/***************************************************************************/
void BV3D_P_IssueRender(
   BV3D_Handle      hV3d,
   BV3D_Instruction *psInstruction
)
{
   BV3D_P_PowerOn(hV3d);

   hV3d->sRender = *psInstruction;

   if (psInstruction->psJob->uiAbandon)
   {
      BV3D_P_HardwareDone(hV3d, &hV3d->sRender); /* Fake completion of the task */
   }
   else
   {
      /* Kick the renderer off */
      BV3D_P_ClearV3dCaches(hV3d);

      BDBG_MSG(("Issuing RENDER job %p for client %d", psInstruction->psJob, psInstruction->psJob->uiClientId));

      BV3D_P_GetTime_isrsafe(&hV3d->uiRenderStartTimeUs);

      if ((hV3d->bCollectLoadStats || psInstruction->psJob->bCollectTimeline) &&
           psInstruction->psJob->sTimelineData.sRenderStart.uiSecs == 0)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sRenderStart);

      BDBG_MSG(("Render start = %p, end = %p", psInstruction->uiArg1, psInstruction->uiArg2));

      BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT1CA, psInstruction->uiArg1);
      BREG_Write32(hV3d->hReg, BCHP_V3D_CLE_CT1EA, psInstruction->uiArg2);
   }
}

/***************************************************************************/
void BV3D_P_IssueUser(
   BV3D_Handle      hV3d,
   BV3D_Instruction *psInstruction
)
{
   BV3D_P_PowerOn(hV3d);

   hV3d->sUser = *psInstruction;

   if (psInstruction->psJob->uiAbandon)
   {
      BV3D_P_HardwareDone(hV3d, &hV3d->sUser); /* Fake completion of the task */
   }
   else
   {
      BV3D_P_ClearV3dCaches(hV3d);

      BDBG_MSG(("Issuing USER SHADER job %p for client %d", psInstruction->psJob, psInstruction->psJob->uiClientId));

      if (psInstruction->psJob->bCollectTimeline && psInstruction->psJob->sTimelineData.sUserStart.uiSecs == 0)
         BV3D_P_GetTimeNow(&psInstruction->psJob->sTimelineData.sUserStart);

      /* starts the shader */
      BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SRQUA, psInstruction->uiArg1);
      BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SRQPC, psInstruction->uiArg2);
   }
}

static bool BV3D_P_NoPendingRenderOrWait(
   BV3D_Handle hV3d,
   uint32_t    uiClientCount,
   uint32_t    *uiClients
   )
{
   uint32_t i;

   if (!BV3D_P_InstructionIsClear(&hV3d->sRender))
      return false;

   /* Find out if there are pending renders or waits from any clients */
   for (i = 0; i < uiClientCount; i++)
   {
      BV3D_IQHandle  hIQ  = BV3D_P_IQMapGet(hV3d->hIQMap, uiClients[i]);

      if (BV3D_P_IQGetSize(hIQ) > 0)
      {
         BV3D_Instruction *psInstruction = BV3D_P_IQTop(hIQ);

         if (psInstruction != NULL && psInstruction->eOperation == BV3D_Operation_eRenderInstr)
            return false;

         if (psInstruction != NULL && psInstruction->eOperation == BV3D_Operation_eWaitInstr && V3D_P_CanWaitProceed(hV3d, psInstruction))
            return false;
      }
   }

   return true;
}

/***************************************************************************/
/* IssueInstr */
/* Schedule and issue instructions. */
/* Iterate over the client queues (using a round-robin scheme) looking for instructions that can be issued. */
/***************************************************************************/
void BV3D_P_IssueInstr(
   BV3D_Handle hV3d
)
{
   void           *pNext;
   uint32_t       uiClientCount, i, uiPrev;
   uint32_t       *uiClients;

   uiClientCount = BV3D_P_IQMapSize(hV3d->hIQMap);

   BDBG_ASSERT(uiClientCount > 0);

   uiClients = BKNI_Malloc(sizeof(uint32_t) * uiClientCount);

   /* start point = hV3d->uiScheduleFirst % size() */
   i = 0;
   uiClients[i++] = uiPrev = BV3D_P_IQMapFirstKey(hV3d->hIQMap, &pNext);

   while (uiPrev != ~0u)
   {
      uiPrev = BV3D_P_IQMapNextKey(hV3d->hIQMap, &pNext);
      if (uiPrev != ~0u)
         uiClients[i++] = uiPrev;
   }

   /* Iterate over the clients */
   for (i = 0; i < uiClientCount; i++)
   {
      uint32_t       indx = (i + hV3d->uiScheduleFirst) % uiClientCount;

      /* Round robin scheduling */
      BV3D_IQHandle  hIQ  = BV3D_P_IQMapGet(hV3d->hIQMap, uiClients[indx]);

      /* Can't do anything if waiting for a sync */
      if (!BV3D_P_IQGetWaiting(hIQ))
      {
         bool bAdvance = true;

         /* Issue an instruction if there is one and a slot is available */
         while (bAdvance && BV3D_P_IQGetSize(hIQ) > 0)
         {
            BV3D_Instruction *psInstruction = BV3D_P_IQTop(hIQ);

            switch (psInstruction->eOperation)
            {
               case BV3D_Operation_eBinInstr:
                  if (BV3D_P_InstructionIsClear(&hV3d->sBin))
                  {
                     /* Need a reasonable number of blocks before we should launch a bin job */
                     /* Even if there aren't, if there aren't any pending renders then we should launch the bin cos nothing is
                      * going to free up memory.  We might fail later.
                      */
                     /* At start of job make sure we have at least one overspill block */
                     if (BV3D_P_BinMemGetOverspill(hV3d->hBinMemManager) == 0)
                        BV3D_P_BinMemAllocOverspill(hV3d->hBinMemManager);

                     if (BV3D_P_BinMemGetOverspill(hV3d->hBinMemManager) &&
                         (BV3D_P_BinMemEnoughFreeBlocks(hV3d->hBinMemManager) ||
                          BV3D_P_NoPendingRenderOrWait(hV3d, uiClientCount, uiClients)))
                     {
                        BV3D_P_IssueBin(hV3d, psInstruction);
                     }
                     else
                        bAdvance = false;
                  } 
                  else
                     bAdvance = false;
                  break;

               case BV3D_Operation_eRenderInstr:
                  if (BV3D_P_InstructionIsClear(&hV3d->sRender))
                     BV3D_P_IssueRender(hV3d, psInstruction);
                  else
                     bAdvance = false;
                  break;

               case BV3D_Operation_eUserInstr:
                  if (BV3D_P_InstructionIsClear(&hV3d->sUser))
                     BV3D_P_IssueUser(hV3d, psInstruction);
                  else
                     bAdvance = false;
                  break;

               case BV3D_Operation_eSyncInstr:
                  {
                     if (psInstruction->uiCallbackParam != 0)
                     {
                        BV3D_P_IQSetWaiting(hIQ, true);

                        BDBG_MSG(("Issuing SYNC CALLBACK(p=%d) to client %d\n", psInstruction->uiCallbackParam, psInstruction->psJob->uiClientId));

                        BV3D_P_DoClientCallback(hV3d, psInstruction, &psInstruction->uiCallbackParam, 
                                                psInstruction->psJob->uiSequence, true);
                     }

                     bAdvance = !BV3D_P_IQGetWaiting(hIQ);

                     /* Has Sync finished? */
                     if (bAdvance)
                        BV3D_P_InstructionDone(hV3d, psInstruction->psJob);
                  }
                  break;

               case BV3D_Operation_eWaitInstr:

                  if (V3D_P_CanWaitProceed(hV3d, psInstruction))
                  {
                     if (psInstruction->uiCallbackParam != 0)
                     {
                        /* if (psInstruction->uiArg1 == (BV3D_Signaller_eBinSig | BV3D_Signaller_eRenderSig | BV3D_Signaller_eUserSig))
                        {
                           BKNI_Printf("??? %x\n", psInstruction->uiArg1);
                           BV3D_P_BinMemDebugDump(hV3d->hBinMemManager);
                        }*/

                        BDBG_MSG(("Issuing WAIT CALLBACK(p=%d) to client %d", psInstruction->uiCallbackParam, psInstruction->psJob->uiClientId));

                        BV3D_P_DoClientCallback(hV3d, psInstruction, &psInstruction->uiCallbackParam, 
                                                psInstruction->psJob->uiSequence, false);
                     }

                     bAdvance = true;

                     BV3D_P_InstructionDone(hV3d, psInstruction->psJob);
                  }
                  else
                     bAdvance = false;
                  break;
                  
               case BV3D_Operation_eNotifyInstr:
                  /* We need to notify the client when all render jobs from it are complete.
                     This differs from a wait-render in that it doesn't block later instructions,
                     so a later bin can still be issued for example. */
                  if (BV3D_P_InstructionIsClear(&hV3d->sRender) || 
                      psInstruction->psJob->uiClientId != hV3d->sRender.psJob->uiClientId)
                  {
                     /* The renderer is idle, or working for another client - so issue the notify
                        callback immediately */
                     BDBG_MSG(("Issuing NOTIFY CALLBACK(p=%d) to client %d", psInstruction->uiCallbackParam, psInstruction->psJob->uiClientId));

                     BV3D_P_DoClientCallback(hV3d, psInstruction, &psInstruction->uiCallbackParam, 
                                             psInstruction->psJob->uiSequence, false);

                     bAdvance = true;
                  }
                  else
                  {
                     hV3d->sRender.psJob->uiNotifyCallbackParam = psInstruction->uiCallbackParam;
                     hV3d->sRender.psJob->uiNotifySequenceNum   = psInstruction->psJob->uiSequence;
                     bAdvance = true;
                  }

                  BV3D_P_InstructionDone(hV3d, psInstruction->psJob);

                  break;

               case BV3D_Operation_eFenceInstr:
                  {
                     int fd = (int)(uintptr_t)psInstruction->uiArg1;
                     bool                    bSignalled = false;

                     BV3D_P_FenceArrayMutexAcquire(hV3d->hFences);

                     bSignalled = BV3D_P_FenceIsSignalled(hV3d->hFences, fd);
                     if (bSignalled)
                     {
                        /* There will not be any other waiters so free up the fence */
                        BV3D_P_FenceFree(hV3d->hFences, fd);
                        BV3D_P_InstructionDone(hV3d, psInstruction->psJob);
                     }
                     else
                     {
                        /* Poll on fence failed, just try again at next opportunity */
                        bAdvance = false;
                     }

                     BV3D_P_FenceArrayMutexRelease(hV3d->hFences);
                  }
                  break;

               case BV3D_Operation_eEndInstr:
               default:
                  break;
            }

            if (bAdvance)
               BV3D_P_IQPop(hIQ);
            else
            {
               /* Here, we only promote bins forward if the other queues also cant advance.
                  This prevents a single client hogging all the GPU time */
               bool bCandidateFound = false;
               void *pNext;
               BV3D_IQHandle hIQOtherQueues = BV3D_P_IQMapFirst(hV3d->hIQMap, &pNext);
               while (hIQOtherQueues)
               {
                  if (BV3D_P_IQGetSize(hIQOtherQueues) > 0)
                  {
                     BV3D_Instruction *psInstruction = BV3D_P_IQTop(hIQOtherQueues);
                     if (psInstruction->eOperation == BV3D_Operation_eBinInstr)
                     {
                        bCandidateFound = true;
                        break;
                     }
                  }
                  hIQOtherQueues = BV3D_P_IQMapNext(hV3d->hIQMap, &pNext);
               }

               if (!bCandidateFound)
               {
                  /* can't advance, so check for another potential bin / waitbin and move it to top of queue */
                  if (BV3D_P_InstructionIsClear(&hV3d->sBin) &&
                     BV3D_P_BinMemEnoughFreeBlocks(hV3d->hBinMemManager))
                  {
                     /* if found, set advance and try again */
                     bAdvance = BV3D_P_JobQFindBinOrWaitBinAndMoveToTop(hIQ);
                  }
               }
            }
         }
      }
   }

   BKNI_Free(uiClients);

   hV3d->uiScheduleFirst += 1;
}

/***************************************************************************/
/* Issue */
/* If there is work in the instruction queues, then issue the top ones to the hardware (if it is free) */
/* If there is no work, but there is stuff in the wait queue dispatch it to the instruction queues and reissue. */
void BV3D_P_Issue(
   BV3D_Handle hV3d
)
{
   BV3D_P_IssueInstr(hV3d);

   if (BV3D_P_AllUnitsIdle(hV3d))
   {
      /* Running benchmark with depth.txt has very slow earlyZ performance unless we have this
         core reset here, which triggers between each test. Why? Who knows? */
      BV3D_P_ResetCore(hV3d, hV3d->uiUserVPM);
   }

   if (BV3D_P_IsIdle(hV3d))
   {
      if (BV3D_P_WaitQSize(hV3d->hWaitQ) != 0)
      {
         BV3D_P_DispatchWaiting(hV3d);
         BV3D_P_IssueInstr(hV3d);
      }
   }
}

/********************************************************************************************
 * AbandonJob
 * If a h/w unit locks-up then we kill the whole job, but have to clock through all the
 * instructions so that all the callback etc. are sent to the client.
 */
void BV3D_P_AbandonJob(
   BV3D_Handle       hV3d,
   BV3D_Instruction  *instr
)
{
   BDBG_ENTER(BV3D_P_AbandonJob);

   if (instr != NULL)
   {
      BV3D_Job *psJob = instr->psJob;

      if (psJob != NULL)
      {
         psJob->uiAbandon = 1;

         /* technically not required as the core has been reset by now, so no isrs will
            arrive */
         BKNI_EnterCriticalSection();

         /* Fake up an interrupt to complete the instruction in this unit */
         switch (instr->eOperation)
         {
         case BV3D_Operation_eBinInstr    :
            BV3D_P_OnInterrupt_isr(hV3d, BCHP_V3D_CTL_INTCTL_EOB_MASK);
            break;
         case BV3D_Operation_eRenderInstr :
            BV3D_P_OnInterrupt_isr(hV3d, BCHP_V3D_CTL_INTCTL_EOF_MASK);
            break;
         case BV3D_Operation_eUserInstr   :
            BV3D_P_OnInterrupt_isr(hV3d, 0xFFFF000);
            break;
         default:
            break;
         }

         BKNI_LeaveCriticalSection();

         /* Free up the memory here as this job is abandoned */
         BV3D_P_BinMemReleaseByJob(hV3d->hBinMemManager, psJob);
      }
   }

   BDBG_LEAVE(BV3D_P_AbandonJob);
}

/********************************************************************************************/
/* Dispatches interrupts -- called outside ISR context. */
void BV3D_P_ProcessInterrupt(
   BV3D_Handle hV3d
)
{
   uint32_t capturedReason;
   BV3D_Job *binJob;

   BDBG_ENTER(BV3D_P_ProcessInterrupt);

   BKNI_AcquireMutex(hV3d->hModuleMutex);

   /* Reset the lockup detection state */
   hV3d->timeoutCount = 0;
   hV3d->quiescentTimeMs = 0;
   hV3d->prevBinAddress = 0;
   hV3d->prevRenderAddress = 0;

   /* Capture interruptReason with interrupts disabled */
   BKNI_EnterCriticalSection();
   capturedReason = hV3d->interruptReason;
   hV3d->interruptReason &= ~capturedReason;
   BKNI_LeaveCriticalSection();

   /* Handle renders first as they can potentially free
    * some memory
    */
   if ((capturedReason & BCHP_V3D_CTL_INTCTL_EOF_MASK) != 0)
   {
      BDBG_MSG(("RENDER job %p completed", hV3d->sRender.psJob));
      BV3D_P_HardwareDone(hV3d, &hV3d->sRender);
   }

   /* Capture the job before processing the binner interrupt
    * as sometimes we get both together
    * and don't want to attach the dead job to the overspill
    * chunk
    */
   binJob = hV3d->sBin.psJob;

   if ((capturedReason & BCHP_V3D_CTL_INTCTL_EOB_MASK) != 0)
   {
      BDBG_MSG(("BIN job %p completed", binJob));
      BV3D_P_HardwareDone(hV3d, &hV3d->sBin);
   }

   /* Binner out of memory
    */
   if ((capturedReason & BCHP_V3D_CTL_INTCTL_OFB_MASK) != 0)
      BV3D_P_OutOfBinMemory(hV3d, binJob);

   /* Low 4 bits of reason are for bin/render units */
   /* High 16-bits are for user shaders             */
   if ((capturedReason & 0xFFFF0000) != 0)
   {
      BDBG_MSG(("USER SHADER job %p completed", hV3d->sUser.psJob));
      BV3D_P_HardwareDone(hV3d, &hV3d->sUser);
   }

   /* The interrupt routine is the heartbeat of the system.  When we get any interrupt we will
      try to issue instructions to the hardware. */
   BV3D_P_Issue(hV3d);

   BKNI_ReleaseMutex(hV3d->hModuleMutex);

   BDBG_LEAVE(BV3D_P_ProcessInterrupt);
}

/********************************************************************************************/
/* Called either in real ISR context from the handler or locally to simulate a missing ISR. */
void BV3D_P_OnInterrupt_isr(
   BV3D_Handle hV3d,
   uint32_t    uiReason
   )
{
   hV3d->interruptReason |= uiReason;
   BKNI_SetEvent(hV3d->wakeWorkerThread);
}

/* We check every 20ms for lockups and power-down conditions */
#define WATCHDOG_TIMEOUT   20

/* After 40ms seconds of no activity, we power down the 3D core */
#define POWERDOWN_TIMEOUT  40

/*********************************************************************************/
/* The core has gone for 20ms without receiving an interrupt if this gets called */
static void BV3D_P_WatchdogTimeout(
   BV3D_Handle hV3d
)
{
   BKNI_AcquireMutex(hV3d->hModuleMutex);

   if (hV3d->isStandby == false)
   {
      /* We only care if the core is notionally powered on */
      if (hV3d->powerOnCount > 0)
      {
         /* The first thing we'll do is clear the slice caches to workaround bug GFXH16 */
         BREG_Write32(hV3d->hReg, BCHP_V3D_CTL_SLCACTL, 0xF0F);

         if (hV3d->timeoutCount > 50)
         {
            uint32_t binAddr    = BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0CA);
            uint32_t renderAddr = BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1CA);
            uint32_t binEnd     = BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT0EA);
            uint32_t renderEnd  = BREG_Read32(hV3d->hReg, BCHP_V3D_CLE_CT1EA);

            bool     binnerStall = hV3d->sBin.eOperation    != 0 && binEnd    != 0 && binAddr    == hV3d->prevBinAddress    && binAddr    != binEnd;
            bool     renderStall = hV3d->sRender.eOperation != 0 && renderEnd != 0 && renderAddr == hV3d->prevRenderAddress && renderAddr != renderEnd;

            if (binnerStall || renderStall)
            {
               if (binnerStall)
                  BDBG_WRN(("Binner job (%p) lockupaddr %x prev %x end %x", hV3d->sBin.psJob, binAddr, hV3d->prevBinAddress, binEnd));

               if (renderStall)
                  BDBG_WRN(("Render job (%p) lockup addr %x prev %x end %x", hV3d->sRender.psJob, renderAddr, hV3d->prevRenderAddress, renderEnd));

               BV3D_P_DebugDump(hV3d);

               BV3D_P_ResetCore(hV3d, 0);

               BV3D_P_AbandonJob(hV3d, &hV3d->sRender);
               BV3D_P_AbandonJob(hV3d, &hV3d->sBin);
            }

            hV3d->prevBinAddress    = binAddr;
            hV3d->prevRenderAddress = renderAddr;

            /* TODO - can we detect user shader deadlocks?? */
         }

         hV3d->timeoutCount++;
         hV3d->quiescentTimeMs = 0;
      }
      else
      {
         /* The core is notionally powered off. If it still is after 40ms, really power off. */
         hV3d->quiescentTimeMs += WATCHDOG_TIMEOUT;
         if (hV3d->quiescentTimeMs >= POWERDOWN_TIMEOUT)
         {
            BV3D_P_ReallyPowerOff(hV3d);
            hV3d->quiescentTimeMs = 0;
         }
      }
   }

   BKNI_ReleaseMutex(hV3d->hModuleMutex);
}

static void BV3D_P_SetQPURatios(
   BV3D_Handle hV3d,
   uint32_t    uiNumVertQPUS,
   uint32_t    uiNumFragQPUS,
   uint32_t    uiBinPct,
   uint32_t    uiRdrPct
   )
{
   uint32_t sliceIndex = hV3d->uiNumSlices - 1;
   const uint32_t uiVertOrder[4][16] = {
         { 0, 1, 2, 3, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
         { 0, 4, 1, 5, 2, 6, 3, 7, 15, 15, 15, 15, 15, 15, 15, 15 },
         { 0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11, 15, 15, 15, 15 },
         { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 }
   };

   const uint32_t uiFragOrder[4][16] = {
         { 3, 2, 1, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },
         { 7, 3, 6, 2, 5, 1, 4, 0, 15, 15, 15, 15, 15, 15, 15, 15 },
         { 11, 7, 3, 10, 6, 2, 9, 5, 1, 8, 4, 0, 15, 15, 15, 15 },
         { 15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0 }
   };

   uint32_t uiTotalQPUs = hV3d->uiNumSlices * 4;   /* 4 qpus per slice */

   /* Sets all active QPUs to run any shader type */
   uint64_t uiQPUCtrl = ~(((uint64_t)1 << (uiTotalQPUs * 4)) - 1);   /* 4 bits per control */

   uint32_t uiVertsToDisable = uiTotalQPUs - uiNumVertQPUS;
   uint32_t uiFragsToDisable = uiTotalQPUs - uiNumFragQPUS;
   uint32_t i;
   bool     print = false;

   for (i = 0; i < uiVertsToDisable; i++)
      uiQPUCtrl |= ((uint64_t)8 << (uiVertOrder[sliceIndex][i] * 4));

   for (i = 0; i < uiFragsToDisable; i++)
      uiQPUCtrl |= ((uint64_t)2 << (uiFragOrder[sliceIndex][i] * 4));

   if (hV3d->uiCurQpuSched0 != (uint32_t)(uiQPUCtrl & 0xFFFFFFFF))
   {
      hV3d->uiCurQpuSched0 = (uint32_t)(uiQPUCtrl & 0xFFFFFFFF);
      BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SQRSV0, hV3d->uiCurQpuSched0);
      print = true;
   }

   if (hV3d->uiCurQpuSched1 != (uint32_t)(uiQPUCtrl >> 32))
   {
      hV3d->uiCurQpuSched1 = (uint32_t)(uiQPUCtrl >> 32);
      BREG_Write32(hV3d->hReg, BCHP_V3D_QPS_SQRSV1, hV3d->uiCurQpuSched1);
      print = true;
   }

   if (print)
   {
      BDBG_MSG(("Bin = %d%%   Render = %d%%", uiBinPct, uiRdrPct));
      BDBG_MSG(("%08X %08X", hV3d->uiCurQpuSched1, hV3d->uiCurQpuSched0));
      BDBG_MSG(("%d vert, %d frag", uiNumVertQPUS, uiNumFragQPUS));
   }
}

/***************************************************************************/
static void BV3D_P_TuneQPURatios(
   BV3D_Handle hV3d
   )
{
   uint64_t uiNowUs;
   uint64_t uiTotalUs;
   uint32_t uiBinPct;
   uint32_t uiRenderPct;
   uint32_t uiTotalQPUs = hV3d->uiNumSlices * 4;

   if (hV3d->bDisableAQA)
      return;

   BV3D_P_GetTime_isrsafe(&uiNowUs);

   if (hV3d->sLastLoadCheckTimeUs == 0)
   {
      hV3d->sLastLoadCheckTimeUs = uiNowUs;
      return;
   }

   uiTotalUs = uiNowUs - hV3d->sLastLoadCheckTimeUs;

   if (!BV3D_P_InstructionIsClear(&hV3d->sBin))
   {
      hV3d->uiCumBinTimeUs += (uiNowUs - hV3d->uiBinStartTimeUs);
      hV3d->uiBinStartTimeUs = uiNowUs;
   }

   if (!BV3D_P_InstructionIsClear(&hV3d->sRender))
   {
      hV3d->uiCumRenderTimeUs += (uiNowUs - hV3d->uiRenderStartTimeUs);
      hV3d->uiRenderStartTimeUs = uiNowUs;
   }

   uiBinPct    = (uint32_t)(hV3d->uiCumBinTimeUs * 100 / uiTotalUs);
   uiRenderPct = (uint32_t)(hV3d->uiCumRenderTimeUs * 100 / uiTotalUs);

   if (uiBinPct > 50)
   {
      if (uiBinPct > 90 && uiBinPct > uiRenderPct)
         BV3D_P_SetQPURatios(hV3d, uiTotalQPUs, uiTotalQPUs - 3, uiBinPct, uiRenderPct);
      else if (uiRenderPct > 90 && uiRenderPct > uiBinPct)
         BV3D_P_SetQPURatios(hV3d, uiTotalQPUs, uiTotalQPUs - 1, uiBinPct, uiRenderPct);
   }
   else
      BV3D_P_SetQPURatios(hV3d, uiTotalQPUs, uiTotalQPUs, uiBinPct, uiRenderPct);

   hV3d->uiCumBinTimeUs = 0;
   hV3d->uiCumRenderTimeUs = 0;
   hV3d->sLastLoadCheckTimeUs = uiNowUs;
}

/***************************************************************************/
/* The worker thread.
 * Handles dispatching of ISR's into non-ISR context.
 * Also looks for locked-up hardware units and manages the recovery process.
 */
void BV3D_Worker(
   void *p
)
{
   if (p != NULL)
   {
      uint64_t uiLastUs;
      /* if the parameters were created on the stack frame, then they are only
         guaranteed to exist until the SetEvent on sync */
      BV3D_Handle hV3d = ((BV3D_WorkerSettings *)p)->hV3d;

      BV3D_P_GetTime_isrsafe(&uiLastUs);

      hV3d->workerSync = ((BV3D_WorkerSettings *)p)->hSync;

      /* TODO : make this thread safe on startup */
      hV3d->workerPresent = true;

      /* Reset the lockup detection state */
      hV3d->timeoutCount = 0;
      hV3d->prevBinAddress = 0;
      hV3d->prevRenderAddress = 0;
      hV3d->quiescentTimeMs = 0;

      /* create signal object */
      BKNI_CreateEvent(&hV3d->wakeWorkerThread);

      /* termination case */
      BKNI_CreateEvent(&hV3d->workerCanTerminate);

      /* signal caller that it can now continue */
      BKNI_SetEvent(hV3d->workerSync);

      for (;;)
      {
         BERR_Code err;

         err = BKNI_WaitForEvent(hV3d->wakeWorkerThread, WATCHDOG_TIMEOUT);

         /* terminate loop */
         if (BKNI_WaitForEvent(hV3d->workerCanTerminate, 0) == BERR_SUCCESS)
            break;

         /* Process the event or timeout */
         if (err == BERR_TIMEOUT)
         {
            /* Timeout fired */
            BV3D_P_WatchdogTimeout(hV3d);
         }
         else if (err == BERR_SUCCESS)
         {
            /* We got an interrupt */
            BV3D_P_ProcessInterrupt(hV3d);
         }

         if (hV3d->reallyPoweredOn && !hV3d->bDisableAQA)
         {
            uint64_t uiNowUs;
            BV3D_P_GetTime_isrsafe(&uiNowUs);
            if (uiNowUs - uiLastUs > 25000)
            {
               BV3D_P_TuneQPURatios(hV3d);
               uiLastUs = uiNowUs;
            }
         }
      }

      BKNI_DestroyEvent(hV3d->wakeWorkerThread);
      BKNI_DestroyEvent(hV3d->workerCanTerminate);

      /* signal caller that it can now continue */
      BKNI_SetEvent(hV3d->workerSync);
   }
}

/***************************************************************************/
