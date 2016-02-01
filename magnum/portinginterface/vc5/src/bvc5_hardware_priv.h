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
#ifndef BVC5_HARDWARE_H__
#define BVC5_HARDWARE_H__

#define BVC5_P_MAX_TFU_FIFO      32
#define BVC5_P_DUMMY_BIN_BYTES   (32 * 1024)

/* HARDWARE

   This module contains the routines that speak directly to the hardware.

 */

#include "bvc5_jobq_priv.h"
#include "bvc5_internal_job_priv.h"
#include "bvc5_hardware_unit_priv.h"

#define BVC5_P_HW_QUEUE_STAGES   2
#define BVC5_P_HW_QUEUE_RUNNING  0
#define BVC5_P_HW_QUEUE_QUEUED   1

/* BVC5_P_BinnerState

   The software state of the binner unit(s)

 */
typedef struct BVC5_P_BinnerState
{
   BVC5_P_InternalJob   *psJob;
   uint32_t              uiPrevAddr;
}
BVC5_P_BinnerState;

/* BVC5_P_RenderState

   The software state of the render unit(s)

 */
typedef struct BVC5_P_RenderState
{
   BVC5_P_InternalJob   *psJob[BVC5_P_HW_QUEUE_STAGES];
   uint32_t              uiPrevAddr;
   uint32_t              uiCapturedRFC;
   uint32_t              uiLastRFC;
}
BVC5_P_RenderState;

/* BVC5_P_TFUState

   The software state of the TFU unit

 */
typedef struct BVC5_P_TFUState
{
   /* TFU hardware has a FIFO of jobs, but we only ever use the first entry to allow
    * for fair scheduling across processes */
   BVC5_P_InternalJob   *psJob;
}
BVC5_P_TFUState;

/* BVC5_P_CoreState

   The software state of a core

 */
typedef struct BVC5_P_CoreState
{
   BVC5_P_BinnerState   sBinnerState;
   BVC5_P_RenderState   sRenderState;

   int32_t              uiRegOffset;   /* Add to core 0 register address to get address for this core */

   /* Power management */
   uint32_t             uiPowerOnCount;
   uint32_t             bPowered;

   /* Watchdog */
   uint32_t             uiTimeoutCount;
}
BVC5_P_CoreState;

/* BVC5_P_HardwareIsUnitAvailable

   Checks if a unit is available

 */
bool BVC5_P_HardwareIsUnitAvailable(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex,
   BVC5_P_HardwareUnitType eHardwareType
);

/* BVC5_P_HardwareIsUnitStalled

   Return word indicating which, if any, units have stalled for
   given core.

 */
BVC5_P_HardwareUnitType BVC5_P_HardwareIsUnitStalled(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
);

/* BVC5_P_HardwarePowerAcquire

   Increment power on counter.  If counter > 0, core will is powered on (unless in standby)

 */
void BVC5_P_HardwarePowerAcquire(
   BVC5_Handle hVC5,
   uint32_t    uiCores
);

/* BVC5_P_HardwarePowerRelease

   Decrement power on counter.  If counter == 0, core will be powered off after a pause.

 */
void BVC5_P_HardwarePowerRelease(
   BVC5_Handle hVC5,
   uint32_t    uiCores
);

/* Issue the power up sequence */
void BVC5_P_HardwareBPCMPowerUp(
   BVC5_Handle hVC5
);

/* Issue the power down sequence */
void BVC5_P_HardwareBPCMPowerDown(
   BVC5_Handle hVC5
);

/* BVC5_P_HardwareResetWatchdog

 */
void BVC5_P_HardwareResetWatchdog(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
);

/* BVC5_P_HardwareIssueBinnerJob

   Submit a job to the binner.  Increments power-on count.
   Returns false if bin memory could not be acquired.

 */
bool BVC5_P_HardwareIssueBinnerJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
);

/* BVC5_P_HardwareIssueRenderJob

   Submit a job to the renderer.  Increments power-on count.

 */
void BVC5_P_HardwareIssueRenderJob(
   BVC5_Handle          hVC5,
   uint32_t             uiCoreIndex,
   BVC5_P_InternalJob  *pJob
);

/* BVC5_P_HardwareIssueTFUJob

   Submit a job to the TFU.  Increments power-on count.

 */
void BVC5_P_HardwareIssueTFUJob(
   BVC5_Handle          hVC5,
   BVC5_P_InternalJob  *pJob
);

/* BVC5_P_HardwareGetBinnerState
 */
BVC5_P_BinnerState *BVC5_P_HardwareGetBinnerState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
);

/* BVC5_P_HardwareGetRenderState
 */
BVC5_P_RenderState *BVC5_P_HardwareGetRenderState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
);

/* BVC5_P_HardwareJobDone

   A job has completed.  Reset the hardware structures and decrement power-on count.

 */
void BVC5_P_HardwareJobDone(
   BVC5_Handle             hVC5,
   uint32_t                uiCoreIndex,
   BVC5_P_HardwareUnitType eHardwareType
);

/* BVC5_P_HardwareInitializeCoreStates

   Initialize the core states and core count

 */
void BVC5_P_HardwareInitializeCoreStates(
   BVC5_Handle hVC5
);

/* BVC5_P_HardwareGetInfo

   Read configuration information from ident registers

 */
void BVC5_P_HardwareGetInfo(
   BVC5_Handle  hVC5,
   BVC5_Info   *pInfo
);

/* BVC5_P_HardwareSupplyBinner

   Submit more memory to the binner

*/
void BVC5_P_HardwareSupplyBinner(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiPhysOffset,
   uint32_t     uiSize);

/* BVC5_P_CoreStateHasClient

   Test whether a client is active in the hardware (all cores)

*/
bool BVC5_P_HardwareCoreStateHasClient(
   BVC5_Handle hVC5,
   uint32_t    uiClientId
);

/* BVC5_P_HardwareAbandonJobs

   Abandons all jobs in specified core

*/
void BVC5_P_HardwareAbandonJobs(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
);


/* BVC5_P_HardwareIsCoreIdle

   Abandons all jobs in specified core

*/
bool BVC5_P_HardwareIsCoreIdle(
   BVC5_Handle    hVC5,
   uint32_t       uiCoreIndex
);

/* BVC5_P_HardwareIsIdle

   Are all cores idle?

*/
bool BVC5_P_HardwareIsIdle(
   BVC5_Handle    hVC5
);

/* BVC5_P_HardwareGetNumCores

   Report the number of cores in hardware

*/
uint32_t BVC5_P_HardwareGetNumCores(
   BVC5_Handle hVC5
);

/* BVC5_P_HardwareStandby

   Put VC5 into standby mode.  The core is powered down, but
   we remember the power status.

 */
void BVC5_P_HardwareStandby(
   BVC5_Handle hVC5
);

/* BVC5_P_HardwareResume

   Take VC5 out of standby mode.  All the formerly powered up
   cores are reactivated.

 */
void BVC5_P_HardwareResume(
   BVC5_Handle hVC5
);

uint32_t BVC5_P_ReadRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg
   );

void BVC5_P_WriteRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg,
   uint32_t     uiValue
   );

void BVC5_P_ClearL3Cache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   );

void BVC5_P_FlushTextureCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   );

void BVC5_P_CleanTextureCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   );

void BVC5_P_ClearL2Cache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   );

void BVC5_P_ClearSlicesCache(
   BVC5_Handle hVC5,
   uint32_t uiCoreIndex
   );

void BVC5_P_InterruptHandler_isr(
   void *pParm,
   int   iValue
   );

void BVC5_P_InterruptHandlerHub_isr(
   void *pParm,
   int   iValue
   );

void BVC5_P_RestorePerfCounters(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   );

void BVC5_P_UpdateShadowCounters(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   );

void BVC5_P_HardwareResetCoreAndState(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   );

/* For workaround GFXH-1181 */
bool BVC5_P_HardwareCacheClearBlocked(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex
   );

#endif /* BVC5_HARDWARE_H__ */
