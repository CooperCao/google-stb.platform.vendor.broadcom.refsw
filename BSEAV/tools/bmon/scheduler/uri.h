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

#ifndef __SCHEDULER_URI__
#define __SCHEDULER_URI__

#include <limits.h>
#include "bmon_json.h"
#include "blst_slist.h"
#include "scheduler.h"

#define INFINITE  INT_MAX

/***************************************************************************
 * Summary:
 * URI list typedef
 ***************************************************************************/
typedef BLST_S_HEAD (listUri, bmon_scheduler_uri_t)   bmon_scheduler_uri_list_t;

/***************************************************************************
 * Summary:
 * URI which contains the original URI string, collection interval,
 * duration, and associated timer id.  bCollectFirst == true, will collect the data
 * (indicated by the strUri string) when this URI is first added to the
 * scheduler.  after the initial data retrieval, the timer based on the
 * intervalCollect will be started.  if bCollectFirst == false, then the
 * intervalCollect timer will be started, but there will be NO initial
 * data collection when added to the scheduler.
 ***************************************************************************/
typedef struct bmon_scheduler_uri_t
{
    BLST_S_ENTRY(bmon_scheduler_uri_t) node;
    bmon_scheduler_configHandle   hConfig;         /* config handle this uri originiates from */
    char                          strUri[1024];    /* text URI */
    int                           intervalCollect; /* msecs */
    int                           durationMax;     /* msecs */
    B_SchedulerTimerId            timerCollect;    /* timer id */
    bool                          bCollectFirst;   /* trigger then start timer */
} bmon_scheduler_uri_t;

/***************************************************************************
 * Summary:
 * Initialize a URI struct
 ***************************************************************************/
void uriGetDefaultSettings(bmon_scheduler_uri_t * pUri);

/***************************************************************************
 * Summary:
 * Create a new URI based on given cJSON object
 ***************************************************************************/
bmon_scheduler_uri_t * uriCreate(bmon_scheduler_configHandle hConfig, cJSON * pJsonUri);

/***************************************************************************
 * Summary:
 * Cancel timers associated with the given URI
 ***************************************************************************/
void uriCancelTimers(
        B_SchedulerHandle      hScheduler,
        bmon_scheduler_uri_t * pUri
        );

/***************************************************************************
 * Summary:
 * Free given URI
 ***************************************************************************/
void uriFree(bmon_scheduler_uri_t * pUri);

/***************************************************************************
 * Summary:
 * Free given URI list
 ***************************************************************************/
void uriFreeList(bmon_scheduler_uri_list_t * pList);

/***************************************************************************
 * Summary:
 * Print contents of given URI
 ***************************************************************************/
void uriPrint(
        bmon_scheduler_uri_t * pUri,
        bool                   bForce
        );

#endif /* __SCHEDULER_URI__ */