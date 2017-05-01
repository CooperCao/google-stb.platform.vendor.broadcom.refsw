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

static void BVC5_P_DumpJobType(BVC5_JobType etype)
{
   switch(etype)
   {
      case BVC5_JobType_eNull:
         BKNI_Printf("BVC5_JobType_eNull\n");
         break;
      case BVC5_JobType_eBin:
         BKNI_Printf("BVC5_JobType_eBin\n");
         break;
      case BVC5_JobType_eRender:
         BKNI_Printf("BVC5_JobType_eRender\n");
         break;
      case BVC5_JobType_eUser:
         BKNI_Printf("BVC5_JobType_eUser\n");
         break;
      case BVC5_JobType_eTFU:
         BKNI_Printf("BVC5_JobType_eTFU\n");
         break;
      case BVC5_JobType_eFenceWait:
         BKNI_Printf("BVC5_JobType_eFenceWait\n");
         break;
      case BVC5_JobType_eTest:
         BKNI_Printf("BVC5_JobType_eTest\n");
         break;
      case BVC5_JobType_eUsermode:
         BKNI_Printf("BVC5_JobType_eUsermode\n");
         break;
      default :
         BKNI_Printf("Unknown\n");
   }
}

static void BVC5_P_DumpInternalJob(BVC5_P_InternalJob *psJob)
{
   if (psJob)
   {
      uint32_t uiIndex;
      BVC5_JobBase *pJobBase = psJob->pBase;

      BDBG_ASSERT(pJobBase);

      BKNI_Printf("Job Type\t=\t");
      BVC5_P_DumpJobType(pJobBase->eType);

      BKNI_Printf("Completed Dependencies\t=\t%d\n", psJob->sRunDep_NotCompleted.uiNumDeps);
      for (uiIndex = 0; uiIndex < psJob->sRunDep_NotCompleted.uiNumDeps; ++uiIndex)
      {
         BKNI_Printf("\tDependency Id\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(psJob->sRunDep_NotCompleted.uiDep[uiIndex]));  /* Is that portable in Linux 64 bits? I read that it is not */
      }
      BKNI_Printf("Finalized Dependencies\t=\t%d\n", psJob->sRunDep_NotFinalized.uiNumDeps);
      for (uiIndex = 0; uiIndex < psJob->sRunDep_NotFinalized.uiNumDeps; ++uiIndex)
      {
         BKNI_Printf("\tDependency Id\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(psJob->sRunDep_NotFinalized.uiDep[uiIndex]));  /* Is that portable in Linux 64 bits? I read that it is not */
      }

      BKNI_Printf("ClientId\t=\t%d\n", psJob->uiClientId);
      BKNI_Printf("Abandon\t\t=\t%d\n", psJob->bAbandon ? 1 : 0);
      BKNI_Printf("Status\t\t=\t%d\n", psJob->eStatus);

      switch(pJobBase->eType)
      {
         case BVC5_JobType_eBin:
         {
            BVC5_JobBin *pJobBin = (BVC5_JobBin *)pJobBase;
            BKNI_Printf("Num Sub Jobs\t=\t%d\n", pJobBin->uiNumSubJobs);
            for (uiIndex=0; uiIndex < pJobBin->uiNumSubJobs; ++uiIndex)
               BKNI_Printf("\tStart = %#x, End = %#x\n", pJobBin->uiStart[uiIndex], pJobBin->uiEnd[uiIndex]);
         }
         break;
         case BVC5_JobType_eRender:
         {
            BVC5_JobRender *pJobRender = (BVC5_JobRender *)pJobBase;
            BKNI_Printf("Bin JobId\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(psJob->jobData.sRender.uiBinJobId));
            BKNI_Printf("Num Sub Jobs\t=\t%d\n", pJobRender->uiNumSubJobs);
            for (uiIndex=0; uiIndex < pJobRender->uiNumSubJobs; ++uiIndex)
               BKNI_Printf("\tStart = %#x, End = %#x\n", pJobRender->uiStart[uiIndex], pJobRender->uiEnd[uiIndex]);
         }
         break;
         case BVC5_JobType_eFenceWait:
         {
            BKNI_Printf("signaled\t\t=\t%d waitData\t\t=\t%p\n", psJob->jobData.sWait.signaled,
                  psJob->jobData.sWait.waitData);
         }
         break;
         case BVC5_JobType_eUsermode:
         {
            BVC5_JobUser *pJobUser = (BVC5_JobUser *)pJobBase;
            BKNI_Printf("Num Sub Jobs\t=\t%d\n", pJobUser->uiNumSubJobs);
            for (uiIndex=0; uiIndex < pJobUser->uiNumSubJobs; ++uiIndex)
               BKNI_Printf("\tuiPC = %#x, uiUnif = %#x\n", pJobUser->uiPC[uiIndex], pJobUser->uiUnif[uiIndex]);
         }
         break;
         default:
         {
            /* Nothing to do */
         }
      }
   }
}

static void BVC5_P_PrintBinJobDumpRunInfo(
   BVC5_Handle             hVC5,
   BVC5_P_InternalJob      *psJob
   )
{
   if (psJob != NULL)
   {
      BVC5_JobBin        *pJobBin = (BVC5_JobBin *)psJob->pBase;
      BVC5_P_InternalJob *psRenderJob = psJob->jobData.sBin.psInternalRenderJob;
      uint32_t           uiIndex;

      BKNI_Printf("Active bin job\n");

      for (uiIndex = 0; uiIndex < pJobBin->uiNumSubJobs; ++uiIndex)
         BKNI_Printf("  bin start = %#x  end = %#x\n", pJobBin->uiStart[uiIndex], pJobBin->uiEnd[uiIndex]);

      if (psRenderJob != NULL)
      {
         BVC5_P_BinMemArray *psBinMemArray = &psRenderJob->jobData.sRender.sBinMemArray;

         if (psBinMemArray && psBinMemArray->uiNumBinBlocks > 0)
         {
            BKNI_Printf("  tile_alloc_addr = 0x%08X, size = %d\n\n",
                        psBinMemArray->pvBinMemoryBlocks[0]->uiPhysOffset,
                        psBinMemArray->pvBinMemoryBlocks[0]->uiNumBytes);

            BKNI_Printf("The following command will convert the dump to a clif file (don't run on silicon):\n");

            BKNI_Printf("dump_run b " BDBG_UINT64_FMT " memdump.bin 0x%08X 0x%08X 0x%08X %u toclif\n",
                        BDBG_UINT64_ARG(hVC5->ulDbgHeapOffset), pJobBin->uiStart[0], pJobBin->uiEnd[0],
                        psBinMemArray->pvBinMemoryBlocks[0]->uiPhysOffset,
                        psBinMemArray->pvBinMemoryBlocks[0]->uiNumBytes);
         }
      }
   }
}

static void BVC5_P_PrintRenderJobDumpRunInfo(
   BVC5_Handle             hVC5,
   BVC5_P_InternalJob      *psJob
   )
{
   if (psJob != NULL)
   {
      BVC5_P_BinMemArray *psBinMemArray = &psJob->jobData.sRender.sBinMemArray;
      BVC5_JobRender     *pJobRender = (BVC5_JobRender *)psJob->pBase;
      uint32_t           uiIndex;

      BKNI_Printf("Active render job\n");

      for (uiIndex = 0; uiIndex < pJobRender->uiNumSubJobs; ++uiIndex)
         BKNI_Printf("  Rdr start = %#x  end = %#x\n", pJobRender->uiStart[uiIndex], pJobRender->uiEnd[uiIndex]);

      if (psBinMemArray && psBinMemArray->uiNumBinBlocks > 0)
      {
         BKNI_Printf("  render job tile_alloc_addr = 0x%08X, size = %d\n\n",
                     psBinMemArray->pvBinMemoryBlocks[0]->uiPhysOffset,
                     psBinMemArray->pvBinMemoryBlocks[0]->uiNumBytes);

         BKNI_Printf("The following command will convert the dump to a clif file (don't run on silicon):\n");

         BKNI_Printf("dump_run r " BDBG_UINT64_FMT " memdump.bin 0x%08X 0x%08X 0x%08X %u toclif\n",
                     BDBG_UINT64_ARG(hVC5->ulDbgHeapOffset), pJobRender->uiStart[0], pJobRender->uiEnd[0],
                     psBinMemArray->pvBinMemoryBlocks[0]->uiPhysOffset,
                     psBinMemArray->pvBinMemoryBlocks[0]->uiNumBytes);

      }
   }
}

void BVC5_P_DebugDump(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex
)
{
    uint32_t ct0cad = 0, ct1cad = 0;

    BKNI_Printf("\n***********************************************************\n");
    BKNI_Printf("GPU Recovery Dump for Core %d:\n\n", uiCoreIndex);

    BKNI_Printf("InterruptReason   = %d\n", hVC5->uiInterruptReason);
    BKNI_Printf("PowerOnCount      = %d\n", hVC5->psCoreStates[uiCoreIndex].uiPowerOnCount);
    BKNI_Printf("PoweredOn         = %d\n", hVC5->psCoreStates[uiCoreIndex].bPowered ? 1 : 0);
    BKNI_Printf("isStandby         = %d\n", hVC5->bInStandby ? 1 : 0);
    BKNI_Printf("TimeoutCount      = %d\n", hVC5->psCoreStates[uiCoreIndex].uiTimeoutCount);
    BKNI_Printf("Binner PrevAddr   = %#x\n",hVC5->psCoreStates[uiCoreIndex].sBinnerState.uiPrevAddr);
    BKNI_Printf("Render PrevAddr   = %#x\n",hVC5->psCoreStates[uiCoreIndex].sRenderState.uiPrevAddr);
    BKNI_Printf("NextClientId      = %d\n", hVC5->uiNextClientId);
    BKNI_Printf("CurrentClient     = %d\n", hVC5->sSchedulerState.uiCurrentClient);
    BKNI_Printf("PerfMonitoring    = %d\n", hVC5->sPerfCounters.bCountersActive ? 1 : 0);
    BKNI_Printf("ActiveHwCounters  = %d\n", hVC5->sPerfCounters.uiActiveHwCounters);

    BKNI_Printf("\n");
    BKNI_Printf("Bin Job:\n");
    if (hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob != NULL)
    {
      BKNI_Printf("JobId Running\t\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob->uiJobId));
      BVC5_P_DumpInternalJob(hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob);
    }

    BKNI_Printf("\n");
    BKNI_Printf("Current Render Job:\n");
    if (hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_RUNNING] != NULL)
    {
      BKNI_Printf("JobId Running\t\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_RUNNING]->uiJobId));
      BVC5_P_DumpInternalJob(hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_RUNNING]);
    }
    if (hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_QUEUED] != NULL)
    {
      BKNI_Printf("JobId Queued\t\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_QUEUED]->uiJobId));
      BVC5_P_DumpInternalJob(hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_QUEUED]);
    }

    BKNI_Printf("\n");
    BKNI_Printf("Current TFU Job:\n");
    if (hVC5->sTFUState.psJob != NULL)
      BKNI_Printf("JobId\t\t=\t" BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(hVC5->sTFUState.psJob->uiJobId));
    BVC5_P_DumpInternalJob(hVC5->sTFUState.psJob);

    BKNI_Printf("\n"
       "CTL_INT_STS     %08X,  INT_MSK_STS   %08X\n"
       "CT0CA           %08X,  CT0EA         %08X,  CT0CS      %08X\n"
       "CT1CA           %08X,  CT1EA         %08X,  CT1CS      %08X\n"
       "CT0RA0          %08X,  CT0LC         %08X,  CT0PC      %08X\n"
       "CT1RA0          %08X,  CT1LC         %08X,  CT1PC      %08X\n"
       "TFBC            %08X,  TFIT          %08X\n"
       "CT1CFG          %08X,  CT1TILECT     %08X,  CT1TSKIP   %08X,  CT1PTCT    %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_STS),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_INT_MSK_STS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0CA),      BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0EA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0CS),      BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1CA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1EA),      BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1CS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0RA0),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0LC),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0PC),      BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1RA0),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1LC),      BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1PC),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_TFBC),       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_TFIT),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1CFG),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1TILECT),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1TSKIP),   BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1PTCT)
       );

    BKNI_Printf(
       "CT0SYNC         %08X,  CT0QBA        %08X,  CT0QEA     %08X\n"
       "CT1SYNC         %08X,  CT1QBA        %08X,  CT1QEA     %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0SYNC),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QBA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QEA),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1SYNC),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBA),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QEA)
       );

#if BCHP_V3D_CLE_CT0CAD
    ct0cad = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0CAD);
#endif

#if BCHP_V3D_CLE_CT1CAD
    ct1cad = BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1CAD);
#endif

    BKNI_Printf(
       "CT0QMA          %08X,  CT0QMS        %08X\n"
       "CT1QCFG         %08X,  CT1QTSKIP     %08X\n"
       "CT1QBASE0       %08X,  CT1QBASE1     %08X,  CT1QBASE2  %08X,  CT1QBASE3  %08X\n"
       "CT1QBASE4       %08X,  CT1QBASE5     %08X,  CT1QBASE6  %08X,  CT1QBASE7  %08X\n"
       "CT0QSYNC        %08X,  CT0CAD        %08X\n"
       "CT1QSYNC        %08X,  CT1CAD        %08X\n"
       "PCS             %08X,  BFC           %08X,  RFC        %08X\n"
       "BPCA            %08X,  BPCS          %08X,  BPOA       %08X,  BPOS       %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMA),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QMS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QCFG),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QTSKIP),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE0),  BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE1),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE2),  BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE3),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE4),   BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE5),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE6),  BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QBASE7),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT0QSYNC),   ct0cad,
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_CT1QSYNC),   ct1cad,
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_PCS),        BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_BFC),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CLE_RFC),        BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPCA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPCS),       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BPOS)
       );

    BKNI_Printf(
       "VPMBASE         %08X,  VPACNTL       %08X\n"
       "L2CACTL         %08X,  SLCACTL       %08X\n"
       "SCRATCH         %08X,  BXCF          %08X\n"
       "IDENT0          %08X,  IDENT1        %08X,  IDENT2     %08X,  IDENT3     %08X\n"
       "HUBIDENT0       %08X,  HUBIDENT1     %08X,  HUBIDENT2  %08X,  HUBIDENT3  %08X\n"
       "ERRSTAT         %08X,  DBGE          %08X,  FDBG0      %08X,  FDBGB      %08X\n"
       "FDBGR           %08X,  FDBGS         %08X,  BXCF       %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_VPM_VPMBASE),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_VPM_VPACNTL),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_L2CACTL),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_SLCACTL),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_SCRATCH),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BXCF),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT0),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT1),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT2),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_CTL_IDENT3),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_HUB_CTL_IDENT0), BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_HUB_CTL_IDENT1),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_HUB_CTL_IDENT2), BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_HUB_CTL_IDENT3),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_ERRSTAT),
#if !V3D_VER_AT_LEAST(4,0,0,0)
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_DBGE),
#else
       0,
#endif
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_FDBG0),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_FDBGB),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_FDBGR),    BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_ERROR_FDBGS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_PTB_BXCF)
       );

    BKNI_Printf(
       "SQRSV0          %08X,  SQRSV1        %08X,  SQCNTL     %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_QPS_SQRSV0),     BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_QPS_SQRSV1),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_QPS_SQCNTL)
       );

#ifdef BCHP_V3D_MMU_0_CTRL
    BKNI_Printf(
       "MMU_CTRL        %08X,  MMU_PTPABASE  %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_MMU_0_CTRL),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_MMU_0_PT_PA_BASE)
       );
#endif

    BKNI_Printf(
       "TFUCS           %08X,  TFUSU         %08X\n"
       "TFUICFG         %08X,  TFUIIA        %08X,  TFUICA     %08X,  TFUIIS     %08X\n"
       "TFUIOA          %08X,  TFUIOS        %08X\n"
       "TFUCOEF0        %08X,  TFUCOEF1      %08X,  TFUCOEF2   %08X,  TFUCOEF3   %08X\n"
       "TFUCRC          %08X,  TFUINT_STS    %08X,  TFUINT_MSK_STS  %08X\n"
       "TFUSYNC         %08X\n",
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCS),            BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUSU),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUICFG),          BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUIIA),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUICA),           BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUIIS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUIOA),           BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUIOS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCOEF0),         BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCOEF1),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCOEF2),         BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCOEF3),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUCRC),           BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUINT_STS),
       BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUINT_MSK_STS),   BVC5_P_ReadRegister(hVC5, uiCoreIndex, BCHP_V3D_TFU_TFUSYNC)
       );

#ifdef BCHP_V3D_MMU_T_CTRL
    BKNI_Printf(
       "MMUT_CTRL       %08X,  MMUT_PTPABASE %08X\n",
       BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_T_CTRL),
       BVC5_P_ReadNonCoreRegister(hVC5, BCHP_V3D_MMU_T_PT_PA_BASE)
       );
#endif


    BKNI_Printf("\n***********************************************************\n");

    if (hVC5->sOpenParams.bMemDumpOnStall && !hVC5->bSecure)
    {
       /* If there is a bin job active - print its info so it can be debugged via dump_run */
       if (hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob)
          BVC5_P_PrintBinJobDumpRunInfo(hVC5, hVC5->psCoreStates[uiCoreIndex].sBinnerState.psJob);

       /* If there is a render job active - print its info so it can be debugged via dump_run */
       if (hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_RUNNING])
          BVC5_P_PrintRenderJobDumpRunInfo(hVC5, hVC5->psCoreStates[uiCoreIndex].sRenderState.psJob[BVC5_P_HW_QUEUE_RUNNING]);

       /* Drop the entire heap into a file */
       /* Can't dump the secure heap */
       BVC5_P_DebugDumpHeapContents(hVC5->ulDbgHeapOffset, hVC5->uDbgHeapSize, uiCoreIndex);
    }
}
