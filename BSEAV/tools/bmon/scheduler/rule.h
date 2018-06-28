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

#ifndef __SCHEDULER_RULE__
#define __SCHEDULER_RULE__

#include "bmon_json.h"
#include "blst_slist.h"
#include "uri.h"

/***************************************************************************
 * Summary:
 * Rule list typedef
 ***************************************************************************/
typedef BLST_S_HEAD (listRule, bmon_scheduler_rule_t) bmon_scheduler_rule_list_t;

/***************************************************************************
 * Summary:
 * Rule which contains the name, URI, condition, and action list
 ***************************************************************************/
typedef struct bmon_scheduler_rule_t
{
    BLST_S_ENTRY(bmon_scheduler_rule_t) node;
    char                        strName[128];       /* name of this rule */
    bmon_scheduler_uri_t *      pUri;               /* URI object indicating data collection */
    char                        strCondition[256];  /* logical condition when true, enables action URIs */
    bmon_scheduler_uri_list_t   listAction;         /* list of URIs which activate when strCondition is satisfied */
} bmon_scheduler_rule_t;

/***************************************************************************
 * Summary:
 * Initialize a Rule struct
 ***************************************************************************/
void ruleGetDefaultSettings(bmon_scheduler_rule_t * pRule);

/***************************************************************************
 * Summary:
 * Create a new Rule based on given cJSON object
 ***************************************************************************/
bmon_scheduler_rule_t * ruleCreate(
        bmon_scheduler_configHandle hConfig,
        cJSON *                     pJsonRule
        );

/***************************************************************************
 * Summary:
 * Cancel timers associated with the given Rule
 ***************************************************************************/
void ruleCancelTimers(
        B_SchedulerHandle       hScheduler,
        bmon_scheduler_rule_t * pRule
        );

/***************************************************************************
 * Summary:
 * Free given Rule
 ***************************************************************************/
void ruleFree(bmon_scheduler_rule_t * pRule);

/***************************************************************************
 * Summary:
 * Free given Rule list
 ***************************************************************************/
void ruleFreeList(bmon_scheduler_rule_list_t * pList);

/***************************************************************************
 * Summary:
 * Print contents of given Rule
 ***************************************************************************/
void rulePrint(
        bmon_scheduler_rule_t * pRule,
        bool                    bForce
        );

#endif /* __SCHEDULER_URI__ */