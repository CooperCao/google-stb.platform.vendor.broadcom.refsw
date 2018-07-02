/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#if BKNI_P_WAITFORGROUP_STATS
static void BKNI_GroupObj_Stats_Init(struct BKNI_GroupObj *group)
{
    group->stats.currentMembers = 0;
    group->stats.maxMembers = 0;
    group->stats.fastCalls = 0;
    group->stats.totalCalls = 0;
    group->stats.mean.sum = 0;
    group->stats.mean.count = 0;
    return;
}

static void BKNI_GroupObj_Stats_Uninit(struct BKNI_GroupObj *group)
{
    BSTD_UNUSED(group);
    return;
}

static void BKNI_GroupObj_Stats_Report(struct BKNI_GroupObj *group)
{
    unsigned mean_100 = 0;
    if(group->stats.mean.count) {
        mean_100 = (group->stats.mean.sum * 100)/group->stats.mean.count;
    }
    BDBG_LOG(("BKNI_WaitForGroup:%p maxMembers:%u calls:%u/%u nevents=%u.%02u", (void *)group, group->stats.maxMembers, group->stats.totalCalls, group->stats.fastCalls, mean_100/100, mean_100%100));
    return;
}

static void BKNI_GroupObj_Stats_AddEvent(struct BKNI_GroupObj *group)
{
    group->stats.currentMembers ++;
    if(group->stats.maxMembers < group->stats.currentMembers) {
        group->stats.maxMembers = group->stats.currentMembers;
    }
    return;
}

static void BKNI_GroupObj_Stats_RemoveEvent(struct BKNI_GroupObj *group)
{
    group->stats.currentMembers--;
    return;
}

static void BKNI_GroupObj_Stats_WaitEvents_Fast(struct BKNI_GroupObj *group)
{
    group->stats.fastCalls ++;
    return;
}

static void BKNI_GroupObj_Stats_WaitEvents_Result(struct BKNI_GroupObj *group, unsigned events)
{
    group->stats.mean.sum += events;
    group->stats.mean.count ++;
    group->stats.totalCalls ++;
    if(group->stats.mean.sum >= 16521216) { /* report before overflow */
        BKNI_GroupObj_Stats_Report(group);
        group->stats.mean.sum = events;
        group->stats.mean.count = 1;
    }
    return;
}
#else /* #if BKNI_P_WAITFORGROUP_STATS */
#define BKNI_GroupObj_Stats_Init(group)
#define BKNI_GroupObj_Stats_Uninit(group)
#define BKNI_GroupObj_Stats_Report(group)
#define BKNI_GroupObj_Stats_RemoveEvent(group)
#define BKNI_GroupObj_Stats_AddEvent(group)
#define BKNI_GroupObj_Stats_WaitEvents_Fast(group)
#define BKNI_GroupObj_Stats_WaitEvents_Result(group, events)
#endif /* #if BKNI_P_WAITFORGROUP_STATS */
