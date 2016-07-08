/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
 *
 * FILENAME: $Workfile: $
 *
 * DESCRIPTION:
 *   This header describes private types and API for the ZDO Channel Manager component.
 *
 * $Revision: $
 * $Date: $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_ZDO_CHANNEL_MANAGER_H
#define _ZBPRO_ZDO_CHANNEL_MANAGER_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoConfig.h"
#include "bbZbProZdoCommon.h"
#include "bbZbProZdoSapTypesMgmtNwkUpdate.h"

/************************* DEFINITIONS **************************************************/
typedef struct _ZbProZdoChannelManagerRecord_t
{
    SYS_Time_t              timestamp;
    ZBPRO_ZDO_NwkAddr_t     address;
    Bool8_t                 isBusy;
} ZbProZdoChannelManagerRecord_t;

typedef struct _ZbProZdoChannelManagerDescr_t
{
    SYS_FSM_Descriptor_t                fsm;
    ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t   zdpUpdateReq;
    ZbProZdoChannelManagerRecord_t      history[ZBPRO_ZDO_CHANNEL_CHANGE_REPORTS_AMOUNT];
    PHY_ED_t                            energy[8 * sizeof(PHY_ChannelMask_t)];
    SYS_TimeoutSignal_t                 timer;
    ZBPRO_ZDO_NwkAddr_t                 candidateAddr;
    uint8_t                             times;
} ZbProZdoChannelManagerDescr_t;

void zbProZdoChannelManagerReset(void);
void zbProZdoChannelManagerUpdateInd(ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t *const indParams);
void zbProZdoCMHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

#endif /* _ZBPRO_ZDO_CHANNEL_MANAGER_H */
