/***************************************************************************
 *     (c)2012 Broadcom Corporation
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
#ifndef BV3D_PRIV_H__
#define BV3D_PRIV_H__

#include "bkni.h"
#include "bkni_multi.h"

#include "bv3d_qmap_priv.h"
#include "bv3d_waitq_priv.h"
#include "bv3d_notifyq_priv.h"
#include "bv3d_callbackmap_priv.h"
#include "bv3d_binmem_priv.h"
#include "bv3d_fence_priv.h"
#include "bv3d_os_priv.h"

#if ((BCHP_CHIP == 7425) && (BCHP_VER >= BCHP_VER_B0))
#include "bchp_v3d_top_gr_bridge.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_gfx_gr.h"
/*#define BCHP_INT_ID_V3D_INTR  BCHP_INT_ID_CREATE(0, BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_V3D_CPU_INTR_SHIFT)*/

/* from bint_7425.c */
#define BCHP_INT_ID_V3D_INTCTL_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_CTL_INTCTL, 0)
#define BCHP_INT_ID_V3D_DBQITC_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_DBG_DBQITC, 0)

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
#include "bchp_hif_cpu_intr1.h"
/*#define BCHP_INT_ID_V3D_INTR  BCHP_INT_ID_CREATE(0, BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_V3D_CPU_INTR_SHIFT)*/

/* from bint_7435.c */
#define BCHP_INT_ID_V3D_INTCTL_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_CTL_INTCTL, 0)
#define BCHP_INT_ID_V3D_DBQITC_INTR              BCHP_INT_ID_CREATE(BCHP_V3D_DBG_DBQITC, 0)

#else
#include "bchp_v3d_pctr.h"
#include "bchp_gfx_gr.h"
#include "bchp_int_id_gfx_l2.h"
#define V3D_HANDLED_VIA_L2
#endif

typedef struct BV3D_P_Handle {
   BCHP_Handle         hChp;
   BREG_Handle         hReg;
   BMMA_Heap_Handle    hMma;
   BINT_Handle         hInt;

   int                 hFd;

   BINT_CallbackHandle  callback_intctl;
   BINT_CallbackHandle  callback_dbqitc;

   BKNI_EventHandle     workerCanTerminate;     /* signaled when the worker can shutdown                                */
   BKNI_EventHandle     wakeWorkerThread;
   BKNI_EventHandle     workerSync;             /* created by the caller and used to sync thread creation/destruction   */
   bool                 workerPresent;          /* used in termination                                                  */

   uint32_t             interruptReason;        /* Current interrupt conditions                                         */
   uint32_t             powerOnCount;           /* Reference count of power on/off                                      */
   bool                 reallyPoweredOn;        /* Set when the core is actually powered                                */
   bool                 isStandby;              /* Power down is done via a watchdog, dont let it power down when
                                                   already in standby                                                   */
   uint32_t             quiescentTimeMs;        /* How long has the hardware been quiet? Power down after a period      */

   uint32_t             timeoutCount;           /* Used for lockup detection                                            */
   uint32_t             prevBinAddress;         /* The last control list address the binner processed                   */
   uint32_t             prevRenderAddress;      /* The last control list address the renderer processed                 */

   BKNI_MutexHandle           hModuleMutex;     /* used to protect the worker queue and the posting of messages         */
   BV3D_IQMapHandle           hIQMap;           /* map of instruction queues based on clientId                          */
   BV3D_WaitQHandle           hWaitQ;           /* job queue for waiting jobs                                           */
   BV3D_NotifyQHandle         hNotifyQ;         /* when a job is complete it goes into the notification queue           */
   BV3D_CallbackMapHandle     hCallbackQ;       /* callbacks                                                            */
   BV3D_BinMemManagerHandle   hBinMemManager;   /* binmemory allocator object                                           */

   uint32_t             uiUserVPM;              /* Current VPM settings                                                 */

   uint32_t             uiNextClientId;         /* Counter for client ids                                               */

   uint32_t             uiScheduleFirst;        /* round robin for the scheduler                                        */

   BV3D_Instruction    sBin;                    /* current Bin instruction                                              */
   BV3D_Instruction    sRender;                 /* current Render instruction                                           */
   BV3D_Instruction    sUser;                   /* current User instruction                                             */

   BV3D_PerfMonitorData sPerfData;              /* Current performance data                                             */
   bool                 bPerfMonitorActive;     /* True if we are gathering data                                        */
   uint32_t             uiPerfMonitorHwBank;
   uint32_t             uiPerfMonitorMemBank;

   uint32_t             uiHeapOffset;           /* needs to be patched for SCB remap.  Get it once at BV3D_Open()       */

   bool                 bCollectLoadStats;      /* Do we want to gather load statistics for each client?                */
   uint32_t             uiNumSlices;            /* Number of slices                                                     */

   bool                 bDisableAQA;            /* Disable adaptive QPU assignment if true                              */
   uint64_t             uiCumBinTimeUs;         /* Cumulative bin time since last load check                            */
   uint64_t             uiBinStartTimeUs;       /* Start time of last bin job                                           */
   uint64_t             uiCumRenderTimeUs;      /* Cumulative render time since last load check                         */
   uint64_t             uiRenderStartTimeUs;    /* Start time of last render job                                        */
   uint64_t             sLastLoadCheckTimeUs;   /* Timestamp of last load check                                         */
   uint32_t             uiCurQpuSched0;         /* Shadow of the QPU0 scheduler register                                */
   uint32_t             uiCurQpuSched1;         /* Shadow of the QPU1 scheduler register                                */

   uint32_t             uiMdiv;                 /* calculated Mdiv for the requested frequency                          */

   /* Fences */
   BV3D_FenceArrayHandle     hFences;           /* holds the fence pool                                                 */
} BV3D_P_Handle;

#endif /* BV3D_PRIV_H__ */
