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
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "b_os_lib.h"
#include "blst_slist.h"

#include "scheduler.h"
#include "uri.h"
#include "rule.h"
/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_uri.h"
#include "plugin_launcher.h"

#define FPRINTF           NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */


/* types */
typedef struct bmon_scheduler_config_t
{
    BLST_S_ENTRY(bmon_scheduler_config_t) node;
    char                         strJsonConfig[2048]; /* JSON string of configuration settings */
    bmon_scheduler_uri_list_t    listUri;             /* list of URIs/timers parsed from strJsonConfig */
    bmon_scheduler_rule_list_t   listRule;            /* list of Rules/timers parsed from strJsonConfig */
} bmon_scheduler_config_t;

typedef BLST_S_HEAD (_listConfig, bmon_scheduler_config_t)   bmon_scheduler_config_list_t;

/* file globals */
static B_SchedulerHandle            _hScheduler       = NULL;
static B_ThreadHandle               _hSchedulerThread = NULL;
static B_MutexHandle                _hMutex           = NULL;
static bool                         _bRunning         = false;
static bmon_scheduler_settings_t    _settings;
static bmon_scheduler_config_list_t _listConfig;

/***************************************************************************
 * Summary:
 * CALLBACK called by scheduler when a URI collection timer expires.
 * the proper plugin must be called and data collected.
 * collected data can be relayed using callback if necessary.
 ***************************************************************************/
static void callbackCollectUri(void * pContext)
{
    bmon_scheduler_uri_t * pUri = (bmon_scheduler_uri_t *)pContext;
    int                    rc   = 0;

    UNUSED(rc);

    assert(NULL != pUri);
    assert(NULL != _settings.callbackFunc);
    assert(NULL != _settings.payloadBuffer);

    FPRINTF(stderr, "TTTTTTTT TIMER COLLECT EXPIRED:\n");
    uriPrint(pUri, !strcmp("fprintf", TO_STRING(FPRINTF)) ? true : false);

    /* clear/reset timer */
    {
        /* timer fired so clear out timer id */
        pUri->timerCollect = NULL;

        /* decrement duration count */
        if (INFINITE != pUri->durationMax)
        {
            pUri->durationMax -= pUri->intervalCollect;
        }

        /* reset timer */
        if (0 < pUri->durationMax)
        {
            pUri->timerCollect = B_Scheduler_StartTimer(_hScheduler, _hMutex, pUri->intervalCollect, callbackCollectUri, pUri);
            CHECK_PTR_ERROR_GOTO("Failure starting collect timer", pUri->timerCollect, rc, -1, error);
        }
    }

    {
        plugin_launcher_t settingsLauncher;
        int               sizeData = 0;

        B_Os_Memset(&settingsLauncher, 0, sizeof(plugin_launcher_t));

        /* init plugin_launcher settings */
        strncpy(settingsLauncher.s_user_agent, "bmon_scheduler/"BMON_SCHEDULER_VERSION, strlen("bmon_scheduler/"BMON_SCHEDULER_VERSION));
        strncpy(settingsLauncher.s_remote_ip_address, "127.0.0.1", strlen("127.0.0.1"));

        {
            char strUriEncoded[2048];

            B_Os_Memset(strUriEncoded, 0, sizeof(strUriEncoded));
            rc = bmon_uri_encode(pUri->strUri, strUriEncoded, sizeof(strUriEncoded));
            CHECK_ERROR_GOTO("Failure encoding URI", rc, error);

            strncpy(settingsLauncher.uri, strUriEncoded, sizeof(settingsLauncher.uri));
        }

        /* clear/set our payload buffer */
        B_Os_Memset(_settings.payloadBuffer, 0, _settings.payloadSize);
        settingsLauncher.p_payload    = _settings.payloadBuffer;
        settingsLauncher.size_payload = _settings.payloadSize;

        /* execute plugin indicated by URI */
        sizeData = plugin_launcher(&settingsLauncher);
        if (0 == sizeData)
        {
            /* plugin_launcher PARENT failed to fork() and execve() the plugin */
            sizeData = snprintf(settingsLauncher.p_payload, settingsLauncher.size_payload, "{\"status\":\"plugin_launcher(%s) returned 0 bytes\"}\n", settingsLauncher.uri);
            goto error;
        }
        else
        if (0 > sizeData)
        {
            /*
             * plugin_launcher CHILD failed to execve()
             * Something unexpected when wrong in the CHILD thread handling the plugin. We are never supposed to get here.
             */
            FPRINTF(stderr, "Plugin Launcher Failure: After plugin_launch ... sizeData (%d) ... (%s)\n", sizeData, settingsLauncher.p_payload);
            sizeData = snprintf(settingsLauncher.p_payload, settingsLauncher.size_payload, "{\"status\":\"plugin_launcher(%s) returned %d bytes\"}\n", settingsLauncher.uri, sizeData);
            goto error;
        }

        TRIM_TRAILING_NEWLINE(_settings.payloadBuffer);

        /* call callback */
        _settings.callbackFunc(pUri->hConfig, _settings.payloadBuffer, sizeData, _settings.callbackParam, _settings.callbackData);
    }

error:
    return;
} /* callbackCollectUri */

/***************************************************************************
 * Summary:
 * CALLBACK called by the scheduler when a Rule collection timer expires.
 * Rules are similar to URIs except they contain a condition which
 * if satisfied, can trigger addtional URIs to become active.
 ***************************************************************************/
static void callbackCollectRule(void * pContext)
{
    UNUSED(pContext);
    BMON_TRACE("enter");
    BMON_TRACE("exit");
    return;
} /* callbackCollectRule */

/***************************************************************************
 * Summary:
 * cancel all URI and Rule based timers
 ***************************************************************************/
static void cancelTimers(bmon_scheduler_config_t * pConfig)
{
    bmon_scheduler_uri_t *  pUri  = NULL;
    bmon_scheduler_rule_t * pRule = NULL;

    BMON_TRACE("enter");

    assert(NULL != pConfig);

    for (pUri = BLST_S_FIRST(&pConfig->listUri); pUri; pUri = BLST_S_NEXT(pUri, node))
    {
        uriCancelTimers(_hScheduler, pUri);
    }

    for (pRule = BLST_S_FIRST(&pConfig->listRule); pRule; pRule = BLST_S_NEXT(pRule, node))
    {
        ruleCancelTimers(_hScheduler, pRule);
    }

    BMON_TRACE("exit");
} /* cancelTimers */

/***************************************************************************
 * Summary:
 * empty all URI and Rule lists
 ***************************************************************************/
static void clearLists(bmon_scheduler_config_t * pConfig)
{
    BMON_TRACE("enter");

    assert(NULL != pConfig);

    /* free lists */
    uriFreeList(&pConfig->listUri);
    ruleFreeList(&pConfig->listRule);

    /* init lists */
    BLST_S_INIT(&pConfig->listUri);
    BLST_S_INIT(&pConfig->listRule);

    BMON_TRACE("exit");
}

/***************************************************************************
 * Summary:
 * start timers based on the given pUri.
 * this function is used for both URIs and Rules to start timers.
 ***************************************************************************/
static int scheduleTimers(
        bmon_scheduler_uri_t * pUri,
        B_TimerCallback        callbackCollect,
        void *                 pContext
        )
{
    int rc = 0;

    BMON_TRACE("entry");

    if (NULL == pUri)
    {
        return(-1);
    }

    if (0 < pUri->intervalCollect)
    {
        pUri->timerCollect = B_Scheduler_StartTimer(
                _hScheduler,
                _hMutex,
                (1 == pUri->bCollectFirst) ? 0 : pUri->intervalCollect,
                callbackCollect,
                pContext
                );
        CHECK_PTR_ERROR_GOTO("Failure starting collect timer", pUri->timerCollect, rc, -1, error);
    }

error:
    BMON_TRACE("exit");
    return(rc);
} /* scheduleTimers */

/***************************************************************************
 * Summary:
 * traverse cJSON tree to create/save URIs and Rules.
 * also start required timers.
 ***************************************************************************/
static int processConfig(
        bmon_scheduler_config_t * pConfig,
        const cJSON *             pJsonConfig
        )
{
    int rc = 0;

    BMON_TRACE("entry");

    assert(NULL != pConfig);
    assert(NULL != pJsonConfig);

    /* convert cJSON tree to URIs and Rules */
    {
        cJSON * pData = NULL;
        cJSON * pItem = NULL;

        /* clear out any previous URIs or Rules */
        clearLists(pConfig);

        /* walk the cJSON treee and add URIs/Rules to lists */
        pData = cJSON_GetObjectItem(pJsonConfig, "data");
        CHECK_PTR_ERROR_GOTO("Unable to find JSON data array", pData, rc, -1, error);

        cJSON_ArrayForEach(pItem, pData)
        {
            cJSON * pObject = pItem->child;

            if (NULL == pObject)
            {
                continue;
            }

            if (0 == strncmp(pObject->string, "element", strlen("element")))
            {
                bmon_scheduler_uri_t * pUri = NULL;

                pUri = uriCreate(pConfig, pObject);
                CHECK_PTR_ERROR_CONTINUE("Unable to create Uri object", pUri);

                BLST_S_INSERT_HEAD(&pConfig->listUri, pUri, node);

                rc = scheduleTimers(pUri, callbackCollectUri, pUri);
                CHECK_ERROR_GOTO("Failure scheduling URI timers", rc, error);
            }
            else
            if (0 == strncmp(pObject->string, "rule", strlen("rule")))
            {
                bmon_scheduler_rule_t * pRule = NULL;

                pRule = ruleCreate(pConfig, pObject);
                CHECK_PTR_ERROR_CONTINUE("Unable to create Rule object", pRule);

                BLST_S_INSERT_HEAD(&pConfig->listRule, pRule, node);

                rc = scheduleTimers(pRule->pUri, callbackCollectRule, pRule);
                CHECK_ERROR_GOTO("Failure scheduling Rule timers", rc, error);
            }
        }
    }

    goto done;
error:
    cancelTimers(pConfig);
    clearLists(pConfig);
done:
    BMON_TRACE("exit");
    return(rc);
} /* processConfig */

/***************************************************************************
 * Summary:
 * scheduler worker thread
 ***************************************************************************/
static void schedulerThread(void * pParam)
{
    BMON_TRACE("entry");
    UNUSED(pParam);

    assert(NULL != _hScheduler);

    _bRunning = true;

    /* B_Scheduler_Run() blocks until stopped */
    B_Scheduler_Run(_hScheduler);

    _bRunning = false;

    BMON_TRACE("exit");
}

/***************************************************************************
 * Summary:
 * Initialize the bmon scheduler Library open settings
 ***************************************************************************/
void bmon_scheduler_getDefaultSettings(bmon_scheduler_settings_t * pSettings)
{
    assert(NULL != pSettings);
    B_Os_Memset(pSettings, 0, sizeof(bmon_scheduler_settings_t));
}

/***************************************************************************
 * Summary:
 * Initialize the bmon scheduler Library.
 *
 * Description:
 * Open scheduler and initialize. Returns 0 on success, -1 otherwise
 ***************************************************************************/
int bmon_scheduler_open(bmon_scheduler_settings_t * pSettings)
{
    int     rc   = 0;
    B_Error berr = B_ERROR_SUCCESS;

    BMON_TRACE("entry");

    if ((NULL != _hScheduler) || (NULL != _hSchedulerThread))
    {
        FPRINTF(stderr, "Scheduler already opened\n");
        return(-1);
    }

    if (NULL == pSettings)
    {
        FPRINTF(stderr, "Scheduler given invalid settings\n");
        return(-1);
    }

    B_Os_Memset(&_settings, 0, sizeof(_settings));
    /* struct copy */
    _settings = *pSettings;

    BLST_S_INIT(&_listConfig);

    berr = B_Os_Init();
    CHECK_BOS_ERROR_GOTO("Failure initializing B_Os", rc, berr, error);

    _hMutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("Failure creating mutex", _hMutex, rc, -1, error);

    _hScheduler = B_Scheduler_Create(NULL);
    CHECK_PTR_ERROR_GOTO("Failure creating scheduler", _hScheduler, rc, -1, error);

    _hSchedulerThread = B_Thread_Create("BMON Scheduler Thread", schedulerThread, NULL, NULL);
    CHECK_PTR_ERROR_GOTO("Failure creating scheduler thread", _hSchedulerThread, rc, -1, error);

    /* yield so scheduler thread can start up */
    B_Thread_Sleep(1);

error:
    BMON_TRACE("exit");
    return(rc);
} /* bmon_scheduler_open */

/***************************************************************************
 * Summary:
 * Uninitialize the bmon scheduler library.
 *
 * Description:
 * Uninitialize and close the bmon scheduler.  All added configurations
 * are removed.
 ***************************************************************************/
void bmon_scheduler_close()
{
    int                       rc      = 0;
    B_Error                   berr    = B_ERROR_SUCCESS;
    bmon_scheduler_config_t * pConfig = NULL;

    UNUSED(rc);

    BMON_TRACE("entry");

    if (NULL == _hScheduler)
    {
        /* already closed */
        return;
    }

    while (NULL != (pConfig = BLST_S_FIRST(&_listConfig)))
    {
        bmon_scheduler_remove(pConfig);
    }

    if (NULL != _hScheduler)
    {
        B_Scheduler_Stop(_hScheduler);

        /* Block until thread exits */
        while ((volatile bool)(_bRunning) == true)
        {
            FPRINTF(stderr, "Waiting for thread to exit...\n");
            B_Thread_Sleep(10);
        }
    }

    if (NULL != _hSchedulerThread)
    {
        B_Thread_Destroy(_hSchedulerThread);
        _hSchedulerThread = NULL;
    }

    if (NULL != _hScheduler)
    {
        B_Scheduler_Destroy(_hScheduler);
        _hScheduler = NULL;
    }

    if (NULL != _hMutex)
    {
        B_Mutex_Destroy(_hMutex);
        _hMutex = NULL;
    }

    berr = B_Os_Uninit();
    CHECK_BOS_ERROR_GOTO("Failure uninitializing B_Os", rc, berr, error);

error:
    BMON_TRACE("exit");
} /* bmon_scheduler_close */

/***************************************************************************
 * Summary:
 * create bmon scheduler config and initialize
 ***************************************************************************/
static bmon_scheduler_config_t * configCreate(void)
{
    int rc                            = 0;
    bmon_scheduler_config_t * pConfig = NULL;

    UNUSED(rc);

    pConfig = B_Os_Malloc(sizeof(bmon_scheduler_config_t));
    CHECK_PTR_ERROR_GOTO("Failure allocating config", pConfig, rc, -1, error);

    B_Os_Memset(pConfig->strJsonConfig, 0, sizeof(pConfig->strJsonConfig));

    /* init lists */
    BLST_S_INIT(&pConfig->listUri);
    BLST_S_INIT(&pConfig->listRule);

error:
    return(pConfig);
} /* configCreate */

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
bmon_scheduler_configHandle bmon_scheduler_add(const cJSON * pJsonConfig)
{
    int rc                            = 0;
    bmon_scheduler_config_t * pConfig = NULL;

    B_Mutex_Lock(_hMutex);
    BMON_TRACE("entry");

    pConfig = configCreate();
    CHECK_PTR_ERROR_GOTO("Failure allocating config", pConfig, rc, -1, error);

    rc = processConfig(pConfig, pJsonConfig);
    CHECK_ERROR_GOTO("Failure processing JSON config", rc, error);

    BLST_S_INSERT_HEAD(&_listConfig, pConfig, node);

    goto done;
error:
    BFRE(pConfig);
done:
    BMON_TRACE("exit");
    B_Mutex_Unlock(_hMutex);
    return(pConfig);
} /* bmon_scheduler_add */

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
bmon_scheduler_configHandle bmon_scheduler_addString(const char * strJsonConfig)
{
    int     rc                          = 0;
    cJSON * pJsonConfig                 = NULL;
    bmon_scheduler_configHandle hConfig = NULL;

    UNUSED(rc);

    BMON_TRACE("entry");
    assert(NULL != strJsonConfig);

    pJsonConfig = cJSON_Parse(strJsonConfig);
    CHECK_PTR_ERROR_GOTO("Failure parsing JSON string", pJsonConfig, rc, -1, error);

    hConfig = bmon_scheduler_add(pJsonConfig);
    CHECK_PTR_ERROR_GOTO("Failure adding JSON config", hConfig, rc, -1, error);

    cJSON_Delete(pJsonConfig);
    pJsonConfig = NULL;

error:
    BMON_TRACE("exit");
    return(hConfig);
} /* bmon_scheduler_addString */

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
void bmon_scheduler_remove(bmon_scheduler_configHandle hConfig)
{
    B_Mutex_Lock(_hMutex);
    BMON_TRACE("entry");
    assert(NULL != hConfig);

    cancelTimers(hConfig);
    BLST_S_REMOVE(&_listConfig, hConfig, bmon_scheduler_config_t, node);
    clearLists(hConfig);

    BFRE(hConfig);

    BMON_TRACE("exit");
    B_Mutex_Unlock(_hMutex);
}

/***************************************************************************
 * Summary:
 * Helper function to create a cJSON based bmon scheduler configuration
 *
 * Description:
 * Creates cJSON header objects and returns pointer to root object.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) bmon_scheduler_configCreate(const char * strName, const char * strDescription)
{
    int     rc         = 0;
    cJSON * objectRoot = NULL;
    cJSON * objectData = NULL;

    assert(NULL != strName);
    assert(NULL != strDescription);

    UNUSED(rc);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, strName, strDescription, NULL, BMON_SCHEDULER_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    goto done;
error:
    json_Uninitialize(&objectRoot);
done:
    return(objectRoot);
}

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
        cJSON *      objectRoot,
        const char * strUri,
        const double dIntervalCollect,
        const double dDurationMax,
        const bool   bCollectFirst
        )
{
    int      rc         = 0;
    cJSON *  objectData = NULL;
    cJSON *  objectUri  = NULL;
    cJSON *  object     = NULL;
    bmon_uri uriParsed;
    char     strPathQuery[2048];

    assert(NULL != objectRoot);
    assert(NULL != strUri);
    assert(0.0 < dIntervalCollect);
    assert(0.0 <= dDurationMax);

    rc = bmon_uri_parse(strUri, &uriParsed);
    CHECK_ERROR_GOTO("unable to parse given URI", rc, error);

    bmon_uri_getPathQuery(&uriParsed, strPathQuery, sizeof(strPathQuery));

    objectData = cJSON_GetObjectItem(objectRoot, "data");
    CHECK_PTR_ERROR_GOTO("Failure find data JSON object", objectData, rc, -1, error);
    objectUri = json_AddObject(objectData, NULL, NULL, "element");
    CHECK_PTR_ERROR_GOTO("Failure adding JSON object", objectUri, rc, -1, error);
    object = json_AddString(objectUri, NULL, NULL, "uri", strPathQuery);
    CHECK_PTR_ERROR_GOTO("Failure adding JSON string", object, rc, -1, error);
    object = json_AddNumber(objectUri, NULL, NULL, "intervalCollect", dIntervalCollect);
    CHECK_PTR_ERROR_GOTO("Failure adding JSON number", object, rc, -1, error);

    if (0.0 < dDurationMax)
    {
        object = json_AddNumber(objectUri, NULL, NULL, "durationMax", dDurationMax);
        CHECK_PTR_ERROR_GOTO("Failure adding JSON number", object, rc, -1, error);
    }

    object = json_AddNumber(objectUri, NULL, NULL, "collectFirst", (true == bCollectFirst) ? 1 : 0);
    CHECK_PTR_ERROR_GOTO("Failure adding JSON boolean", object, rc, -1, error);
error:
    return(rc);
} /* bmon_scheduler_configAddUri */

/***************************************************************************
 * Summary:
 * Helper function to save a generated JSON string to the given buffer.
 *
 * Description:
 * Translates the given cJSON root object to a JSON string and saves it to
 * the given buffer.
 ***************************************************************************/
int bmon_scheduler_configGenerate(
        cJSON * objectRoot,
        char *  buffer,
        size_t  bufferSize
        )
{
    int rc = 0;

    rc = json_Print(objectRoot, buffer, bufferSize);
    CHECK_ERROR_GOTO("Failure generating JSON string to buffer", rc, error);

error:
    return(rc);
}

/***************************************************************************
 * Summary:
 * Helper function to destroy a given cJSON object tree.
 *
 * Description:
 * Destroys a cJSON object created with bmon_scheduler_configCreate(),
 * bmon_scheduler_configAddUri(), or bmon_scheduler_configAddRule().
 ***************************************************************************/
void bmon_scheduler_configDestroy(cJSON * objectRoot)
{
    assert(NULL != objectRoot);

    json_Uninitialize(&objectRoot);
}