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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "b_os_lib.h"
#include "bmon_defines.h"

#include "rule.h"

#define FPRINTF  NOFPRINTF /* NOFPRINTF to disable */

/***************************************************************************
 * Summary:
 * Initialize a Rule struct
 ***************************************************************************/
void ruleGetDefaultSettings(bmon_scheduler_rule_t * pRule)
{
    B_Os_Memset(pRule, 0, sizeof(bmon_scheduler_rule_t));
    BLST_S_INIT(&pRule->listAction);
}

/***************************************************************************
 * Summary:
 * Create a new Rule based on given cJSON object
 ***************************************************************************/
bmon_scheduler_rule_t * ruleCreate(
        bmon_scheduler_configHandle hConfig,
        cJSON *                     pJsonRule
        )
{
    int rc                          = 0;
    bmon_scheduler_rule_t * pRule   = NULL;
    cJSON *                 pItem   = NULL;
    cJSON *                 pAction = NULL;

    UNUSED(rc);

    fprintf(stderr, "TRACE entry %s()\n", __FUNCTION__);

    if (NULL == pJsonRule)
    {
        return(NULL);
    }

    pRule = B_Os_Malloc(sizeof(bmon_scheduler_rule_t));
    CHECK_PTR_ERROR_GOTO("Out of memory in uriCreate()", pRule, rc, -1, error);
    ruleGetDefaultSettings(pRule);

    pItem = cJSON_GetObjectItem(pJsonRule, "name");
    if (NULL != pItem)
    {
        strncpy(pRule->strName, cJSON_GetStringValue(pItem), sizeof(pRule->strName));
    }

    pItem = cJSON_GetObjectItem(pJsonRule, "element");
    if (NULL != pItem)
    {
        pRule->pUri = uriCreate(hConfig, pItem);
    }

    pItem = cJSON_GetObjectItem(pJsonRule, "condition");
    if (NULL != pItem)
    {
        strncpy(pRule->strCondition, cJSON_GetStringValue(pItem), sizeof(pRule->strCondition));
    }

    pAction = cJSON_GetObjectItem(pJsonRule, "action");
    if (NULL != pAction)
    {
        bmon_scheduler_uri_t * pBmonUri = NULL;
        cJSON_ArrayForEach(pItem, pAction)
        {
            cJSON * pObject = pItem->child;

            if ((NULL == pObject) || (0 != strncmp(pObject->string, "element", strlen("element"))))
            {
                continue;
            }

            pBmonUri = uriCreate(hConfig, pObject);
            if (NULL != pBmonUri)
            {
                BLST_S_INSERT_HEAD(&(pRule->listAction), pBmonUri, node);
            }
        }
    }

    rulePrint(pRule, false);
    goto done;
error:
    ruleFree(pRule);
    pRule = NULL;
done:
    fprintf(stderr, "TRACE exit %s()\n", __FUNCTION__);
    return(pRule);
} /* ruleCreate */

/***************************************************************************
 * Summary:
 * Cancel timers associated with the given Rule
 ***************************************************************************/
void ruleCancelTimers(
        B_SchedulerHandle       hScheduler,
        bmon_scheduler_rule_t * pRule
        )
{
    bmon_scheduler_uri_t * pUri = NULL;

    if (NULL == pRule)
    {
        return;
    }

    uriCancelTimers(hScheduler, pRule->pUri);

    for (pUri = BLST_S_FIRST(&pRule->listAction); pUri; pUri = BLST_S_NEXT(pUri, node))
    {
        uriCancelTimers(hScheduler, pUri);
    }
}

/***************************************************************************
 * Summary:
 * Free given Rule
 ***************************************************************************/
void ruleFree(bmon_scheduler_rule_t * pRule)
{
    if (NULL == pRule)
    {
        return;
    }

    uriFree(pRule->pUri);
    uriFreeList(&(pRule->listAction));
    BFRE(pRule);
}

/***************************************************************************
 * Summary:
 * Free given Rule list
 ***************************************************************************/
void ruleFreeList(bmon_scheduler_rule_list_t * pList)
{
    if (NULL == pList)
    {
        return;
    }

    while (!BLST_S_EMPTY(pList))
    {
        bmon_scheduler_rule_t * pRule = NULL;

        pRule = BLST_S_FIRST(pList);
        BLST_S_REMOVE_HEAD(pList, node);
        ruleFree(pRule);
    }
}

/***************************************************************************
 * Summary:
 * Print contents of given Rule
 ***************************************************************************/
void rulePrint(
        bmon_scheduler_rule_t * pRule,
        bool                    bForce
        )
{
    bmon_scheduler_uri_t * pUri = NULL;

    if (NULL == pRule)
    {
        return;
    }

    fprintf(stderr, "rule:%s\n", pRule->strName);
    uriPrint(pRule->pUri, bForce);
    fprintf(stderr, "strCondition:%s\n", pRule->strCondition);

    for (pUri = BLST_S_FIRST(&pRule->listAction); NULL != pUri; pUri = BLST_S_NEXT(pUri, node))
    {
        uriPrint(pUri, bForce);
    }
} /* rulePrint */