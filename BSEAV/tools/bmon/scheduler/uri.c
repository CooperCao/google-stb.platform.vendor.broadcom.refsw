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

#include "uri.h"

#define FPRINTF  NOPRINTF /* NOFPRINTF to disable */

/***************************************************************************
 * Summary:
 * Initialize a URI struct
 ***************************************************************************/
void uriGetDefaultSettings(bmon_scheduler_uri_t * pUri)
{
    B_Os_Memset(pUri, 0, sizeof(bmon_scheduler_uri_t));
    pUri->durationMax = INFINITE;
}

/***************************************************************************
 * Summary:
 * Create a new URI based on given cJSON object
 ***************************************************************************/
bmon_scheduler_uri_t * uriCreate(
        bmon_scheduler_configHandle hConfig,
        cJSON *                     pJsonUri
        )
{
    int                    rc    = 0;
    bmon_scheduler_uri_t * pUri  = NULL;
    cJSON *                pItem = NULL;

    UNUSED(rc);
    assert(NULL != hConfig);

    FPRINTF(stderr, "TRACE entry %s()\n", __FUNCTION__);

    if (NULL == pJsonUri)
    {
        return(NULL);
    }

    pUri = B_Os_Malloc(sizeof(bmon_scheduler_uri_t));
    CHECK_PTR_ERROR_GOTO("Out of memory in uriCreate()", pUri, rc, -1, error);
    uriGetDefaultSettings(pUri);

    pUri->hConfig = hConfig;

    pItem = cJSON_GetObjectItem(pJsonUri, "uri");
    if (NULL != pItem)
    {
        strncpy(pUri->strUri, cJSON_GetStringValue(pItem), sizeof(pUri->strUri));
    }

    pItem = cJSON_GetObjectItem(pJsonUri, "intervalCollect");
    if (NULL != pItem)
    {
        pUri->intervalCollect = SECS_TO_MSECS(pItem->valuedouble);
    }

    pItem = cJSON_GetObjectItem(pJsonUri, "durationMax");
    if (NULL != pItem)
    {
        pUri->durationMax = SECS_TO_MSECS(pItem->valuedouble);
    }

    pItem = cJSON_GetObjectItem(pJsonUri, "collectFirst");
    if (NULL != pItem)
    {
        pUri->bCollectFirst = pItem->valueint;
    }

    uriPrint(pUri, false);
    goto done;
error:
    BFRE(pUri);
done:
    FPRINTF(stderr, "TRACE exit %s()\n", __FUNCTION__);
    return(pUri);
} /* uriCreate */

/***************************************************************************
 * Summary:
 * Cancel timers associated with the given URI
 ***************************************************************************/
void uriCancelTimers(
        B_SchedulerHandle      hScheduler,
        bmon_scheduler_uri_t * pUri
        )
{
    if (NULL == pUri)
    {
        return;
    }

    if (NULL != pUri->timerCollect)
    {
        /* zero duration so timer will not restart */
        pUri->durationMax = 0;
        B_Scheduler_CancelTimer(hScheduler, pUri->timerCollect);
        pUri->timerCollect = NULL;
    }
} /* uriCancelTimers */

/***************************************************************************
 * Summary:
 * Free given URI
 ***************************************************************************/
void uriFree(bmon_scheduler_uri_t * pUri)
{
    if (NULL == pUri)
    {
        return;
    }

    BFRE(pUri);
}

/***************************************************************************
 * Summary:
 * Free given URI list
 ***************************************************************************/
void uriFreeList(bmon_scheduler_uri_list_t * pList)
{
    if (NULL == pList)
    {
        return;
    }

    while (!BLST_S_EMPTY(pList))
    {
        bmon_scheduler_uri_t * pUri = NULL;

        pUri = BLST_S_FIRST(pList);
        BLST_S_REMOVE_HEAD(pList, node);
        uriFree(pUri);
    }
}

/***************************************************************************
 * Summary:
 * Print contents of given URI
 ***************************************************************************/
void uriPrint(
        bmon_scheduler_uri_t * pUri,
        bool                   bForce
        )
{
    if (NULL == pUri)
    {
        return;
    }

    if (true == bForce)
    {
        fprintf(stderr, "uri:%s\tintervalCollect:%d\tdurationMax:%d\n",
                pUri->strUri, pUri->intervalCollect, pUri->durationMax);
    }
    else
    {
        FPRINTF(stderr, "uri:%s\tintervalCollect:%d\tdurationMax:%d\n",
                pUri->strUri, pUri->intervalCollect, pUri->durationMax);
    }
} /* uriPrint */