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
#ifndef BV3D_WORKER_PRIV__H
#define BV3D_WORKER_PRIV__H

typedef enum BV3D_P_ISRValue
{
   BV3D_ISR_RENDER_DONE         = 1 << 0,
   BV3D_ISR_BIN_DONE            = 1 << 1,
   BV3D_ISR_BIN_OUT_OF_MEM      = 1 << 2,
   BV3D_ISR_BIN_USED_OVERSPILL  = 1 << 3,
   BV3D_ISR_USER_DONE           = 1 << 4
} BV3D_P_ISRValue;


/***************************************************************************/
void BV3D_P_GetTimeNow(
   BV3D_EventTime *t
);

/***************************************************************************/
void BV3D_P_Issue(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_PowerOn(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_PowerOff(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_SupplyBinner(
   BV3D_Handle hV3d,
   uint32_t    uiAddr
);

/***************************************************************************/
void BV3D_P_InitPerfMonitor(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_ResetPerfMonitorHWCounters(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_GatherPerfMonitor(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_ResetCore(
   BV3D_Handle hV3d,
   uint32_t    vpmBase
);

/***************************************************************************/
void BV3D_P_OnInterrupt_isr(
   BV3D_Handle hV3d,
   uint32_t    uiReason
);

/***************************************************************************/
void BV3D_P_ClearV3dCaches(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_AbandonJob(
   BV3D_Handle       hV3d,
   BV3D_Instruction  *instr
);

/***************************************************************************/
void BV3D_P_ReallyPowerOff(
   BV3D_Handle hV3d
);

/***************************************************************************/
void BV3D_P_OutOfBinMemory(
   BV3D_Handle hV3d,
   BV3D_Job    *psJob
);

/***************************************************************************/
void BV3D_P_InstructionDone(
   BV3D_Handle hV3d,
   BV3D_Job    *psJob
);

/***************************************************************************/
bool BV3D_P_IsIdle(
   BV3D_Handle hV3d
);

/***************************************************************************/
bool BV3D_P_AllUnitsIdle(
   BV3D_Handle hV3d
);

/***************************************************************************/

#endif /* BV3D_WORKER_PRIV__H */
