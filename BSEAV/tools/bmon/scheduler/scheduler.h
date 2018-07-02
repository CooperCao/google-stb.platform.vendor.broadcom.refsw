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

#ifndef __BMON_SCHEDULER_H__
#define __BMON_SCHEDULER_H__

#include "bmon_json.h"

#define BMON_SCHEDULER_VERSION  "1.0"

/***************************************************************************
 * Summary:
 * bmon scheduler configuration handle
 *
 * Description:
 * A bmon scheduler configuration is a group of URIs and Rules which
 * equates to a set of timers and data collection targets the scheduler
 * will operate on.  Configurations can be added and removed dynamically
 * using these handles.
 ***************************************************************************/
typedef struct bmon_scheduler_config_t   * bmon_scheduler_configHandle;

/***************************************************************************
 * Summary:
 * bmon scheduler data ready callback prototype.
 ***************************************************************************/
typedef void (* bmon_scheduler_callback)(
        bmon_scheduler_configHandle hConfig,
        char *                      payloadBuffer,
        size_t                      payloadSize,
        void *                      pParam,
        int                         nData
        );

/***************************************************************************
 * Summary:
 * Required settings to open bmon scheduler
 ***************************************************************************/
typedef struct bmon_scheduler_settings_t
{
    char *                    payloadBuffer;       /* caller supplied buffer which will be filled with plugin data */
    size_t                    payloadSize;         /* size of caller supplied payload buffer */
    bmon_scheduler_callback   callbackFunc;        /* callback to call when plugin data is available */
    void *                    callbackParam;       /* caller supplied param returned thru callback (optional) */
    int                       callbackData;        /* caller supplied data returned thru callback (optional) */
} bmon_scheduler_settings_t;

/***************************************************************************
 * Summary:
 * Initialize the bmon scheduler Library open settings
 ***************************************************************************/
void bmon_scheduler_getDefaultSettings(bmon_scheduler_settings_t * pSettings);

/***************************************************************************
 * Summary:
 * Initialize the bmon scheduler Library.
 *
 * Description:
 * Open scheduler and initialize. Returns 0 on success, -1 otherwise
 ***************************************************************************/
int bmon_scheduler_open(bmon_scheduler_settings_t * pSettings);

/***************************************************************************
 * Summary:
 * Add the given cJSON object based configuration to the bmon scheduler.
 *
 * Description:
 * This will set up the required timed data collection specified in the
 * configuration.  Adding a new configuration will not affect any pre-existing
 * configurations.  The returned handle can be later used to remove this
 * configuration from the scheduler.
 ***************************************************************************/
bmon_scheduler_configHandle bmon_scheduler_add(const cJSON * pJsonConfig);

/***************************************************************************
 * Summary:
 * Add the given JSON string based configuration to the bmon scheduler.
 *
 * Description:
 * This will set up the required timed data collection specified in the
 * configuration.  Adding a new configuration will not affect any pre-existing
 * configurations.  The returned handle can be later used to remove this
 * configuration from the scheduler.
 ***************************************************************************/
bmon_scheduler_configHandle bmon_scheduler_addString(const char * strJsonConfig);

/***************************************************************************
 * Summary:
 * Remove the configuration indicated by the given handle from the bmon
 * scheduler.
 *
 * Description:
 * All timed data collecion associated with the given configuration handle
 * will be stopped, and the configuration will be removed from the bmon
 * scheduler.
 ***************************************************************************/
void bmon_scheduler_remove(bmon_scheduler_configHandle pConfig);

/***************************************************************************
 * Summary:
 * Uninitialize the bmon scheduler library.
 *
 * Description:
 * Uninitialize and close the bmon scheduler.  All added configurations
 * are removed.
 ***************************************************************************/
void bmon_scheduler_close(void);

/***************************************************************************
 * Summary:
 * Helper function to create a cJSON based bmon scheduler configuration
 *
 * Description:
 * Creates cJSON header objects and returns pointer to root object.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) bmon_scheduler_configCreate(
        const char * strName,       /* name of config */
        const char * strDescription /* description of config */
        );

/***************************************************************************
 * Summary:
 * Helper function to create and add a URI to a cJSON based bmon scheduler
 * configuration.
 *
 * Description:
 * Creates cJSON URI object and attaches it to the given root object.
 * The given root object should be first created using
 * bmon_scheduler_configCreate().
 ***************************************************************************/
int bmon_scheduler_configAddUri(
        cJSON *      objectRoot,       /* parent JSON object */
        const char * strUri,           /* URI specifying data to retrieve */
        const double dIntervalCollect, /* seconds */
        const double dDurationMax,     /* seconds */
        const bool   bCollectFirst     /* URI data will be collected when first added to scheduler and then timer started */
        );

#if 0
/***************************************************************************
 * Summary:
 * Helper function to create and add a Rule to a cJSON based bmon scheduler
 * configuration.
 *
 * Description:
 * Creates cJSON Rule object and attaches it to the given root object.
 * A Rule consists of a name string, a URI object, a condition string, and
 * a list of Actions to take if the condition is met.  This function will
 * add all Rule parts except the Actions list.
 * The given root object should be first created using
 * bmon_scheduler_configCreate().  Returns a cJSON Rule object which can
 * be used to add Actions (see bmon_scheduler_configAddAction()).
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) bmon_scheduler_configAddRule(
        cJSON * objectRoot,            /* parent JSON object */
        const char * strRuleName,      /* name of the rule */
        const char * strUri,           /* URI specifying data to retrieve */
        const double dIntervalCollect, /* seconds */
        const double dDurationMax,     /* seconds */
        const char * strCondition      /* condition string which when satisfied, turns on any associated action URIs */
        );

/***************************************************************************
 * Summary:
 * Helper function to create and add an Action to a cJSON based bmon scheduler
 * configuration Rule.
 *
 * Description:
 * Creates cJSON Action object and attaches it to the given Rule object.
 * The given Rule object should be first created using
 * bmon_scheduler_configAddRule().  Returns a 0 on success, -1 otherwise.
 ***************************************************************************/
int bmon_scheduler_configAddAction(
        cJSON *      objectRule,       /* parent JSON object */
        const char * strUri,           /* URI specifying data to retrieve */
        const double nIntervalCollect, /* seconds */
        const double nDurationMax      /* seconds */
        );
#endif /* if 0 */

/***************************************************************************
 * Summary:
 * Helper function to save a generated JSON string to the given buffer.
 *
 * Description:
 * Translates the given cJSON root object to a JSON string and saves it to
 * the given buffer.
 ***************************************************************************/
int bmon_scheduler_configGenerate(
        cJSON * objectRoot, /* parent JSON object */
        char *  buffer,     /* buffer to store generated config JSON string */
        size_t  bufferSize  /* max size of given buffer */
        );

/***************************************************************************
 * Summary:
 * Helper function to destroy a given cJSON object tree.
 *
 * Description:
 * Destroys a cJSON object created with bmon_scheduler_configCreate(),
 * bmon_scheduler_configAddUri(), or bmon_scheduler_configAddRule().
 ***************************************************************************/
void bmon_scheduler_configDestroy(cJSON * objectRoot);

#endif /* __BMON_SCHEDULER_H__ */