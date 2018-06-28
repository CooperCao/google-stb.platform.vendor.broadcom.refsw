/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_uri.h"

#define FPRINTF  NOFPRINTF /* NOFPRINTF to disable */

/***************************************************************************
 * Summary:
 * Create and initialize a cJSON root object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_Initialize(void)
{
    cJSON *     pObject    = NULL;
    int         rc         = 0;
    cJSON_Hooks cjsonHooks = { malloc, free };

    UNUSED(rc);
    BMON_TRACE("enter");

    cJSON_InitHooks(&cjsonHooks);

    pObject = cJSON_CreateObject();
    CHECK_PTR_ERROR_GOTO("Unable to allocate cJSON object", pObject, rc, -1, error);

error:
    BMON_TRACE("exit");
    return(pObject);
} /* json_Initialize */

/***************************************************************************
 * Summary:
 * Destroy the given cJSON root object tree
 ***************************************************************************/
void json_Uninitialize(cJSON ** pObject)
{
    BMON_TRACE("enter");
    if ((NULL == pObject) || (NULL == *pObject))
    {
        return;
    }

    cJSON_Delete(*pObject);
    *pObject = NULL;
    BMON_TRACE("exit");
} /* json_Uninitialize */

/***************************************************************************
 * Summary:
 * Generate a bmon JSON header and connect to given pObject
 *
 * Description:
 * This function will generate the plugin JSON header.  If an error occurs
 * during header generation, the given JSON object may have a partial
 * header. While we could clean up the partial data on error, this is more
 * of catastrophic failure - calling code will delete the given pObject
 * anyways.
 ***************************************************************************/

ssize_t formatTimeval(
        struct timeval * tv,
        char *           buf,
        size_t           sz
        )
{
    ssize_t     written = -1;
    struct tm * gm      = NULL;

    assert(NULL != tv);
    assert(NULL != buf);
    assert(0 < sz);

    gm = gmtime(&tv->tv_sec);

    if (gm)
    {
        written = (ssize_t)strftime(buf, sz, "%Y-%m-%dT%H:%M:%S", gm);
        if ((written > 0) && ((size_t)written < sz))
        {
            int w = snprintf(buf+written, sz-(size_t)written, ".%06dZ", tv->tv_usec);
            written = (w > 0) ? written + w : -1;
        }
    }
    return(written);
} /* formatTimeval */


CJSON_PUBLIC(cJSON *) json_GenerateHeader(
        cJSON *          pObject,
        const char *     strPluginName,
        const char *     strPluginDescription,
        struct timeval * pTv,
        const char *     pluginVersion
        )
{
    int              rc          = 0;
    cJSON *          pRetJSON    = NULL;
    cJSON *          pObjectData = NULL;
    struct timeval * pMyTv       = NULL;
    struct timeval   myTv;
    char             strTime[32];
    char             strPcTime[32];
    double           dTime=0.0;

    assert(NULL != pObject);
    assert(NULL != strPluginName);

    BMON_TRACE("enter");

    /* assign pointer to valid tv struct */
    pMyTv = (NULL != pTv) ? pTv : &myTv;

    /* format string timestamp and datetime data */
    {
        char * pRet = NULL;

        if (NULL == pTv)
        {
            /* no time given to get current time */
            rc = gettimeofday(pMyTv, NULL);
            CHECK_ERROR_GOTO("Failure gettimeofday", rc, error);
        }
#if 0
        snprintf(strTime, sizeof(strTime) - 1, "%ld.%06ld", (unsigned long int)pMyTv->tv_sec, (unsigned long int)pMyTv->tv_usec);

        pRet = ctime_r(&(pMyTv->tv_sec), strPcTime);
        CHECK_PTR_ERROR_GOTO("Failure ctime_r", pRet, rc, -1, error);

        /* ctime() returns a string that has a newline at the end of it so trim it */
        strPcTime[strlen(strPcTime) - 1] = '\0';
#else
        formatTimeval(pMyTv,strPcTime,sizeof(strPcTime));
#endif
        dTime = (double)pMyTv->tv_sec + (double)pMyTv->tv_usec/1000000;
    }

    pRetJSON = cJSON_AddStringToObject(pObject, "version", pluginVersion);
    CHECK_PTR_ERROR_GOTO("Unable to create JSON string", pRetJSON, rc, -1, error);
    pRetJSON = cJSON_AddStringToObject(pObject, "name", strPluginName);
    CHECK_PTR_ERROR_GOTO("Unable to create JSON string", pRetJSON, rc, -1, error);
#if 0
    pRetJSON = cJSON_AddStringToObject(pObject, "description", strPluginDescription);
    CHECK_PTR_ERROR_GOTO("Unable to create JSON string", pRetJSON, rc, -1, error);
#endif
    pRetJSON = cJSON_AddStringToObject(pObject, "datetime", strPcTime);
    CHECK_PTR_ERROR_GOTO("Unable to create JSON string", pRetJSON, rc, -1, error);
    pRetJSON = cJSON_AddNumberToObject(pObject, "timestampSec", dTime);
    CHECK_PTR_ERROR_GOTO("Unable to create JSON string", pRetJSON, rc, -1, error);
    pObjectData = cJSON_AddArrayToObject(pObject, "data");
    CHECK_PTR_ERROR_GOTO("Unable to create JSON object", pRetJSON, rc, -1, error);

error:
    BMON_TRACE("exit");
    return(pObjectData);
} /* json_GenerateHeader */

/***************************************************************************
 * Summary:
 * Prune the given object tree of all objects without values and children.
 * This can occur when a container object's children get filtered out
 * by way of the "exclude" query string.
 ***************************************************************************/
static void json_PruneEmptyObjects(
        cJSON * object,
        cJSON * parent
        )
{
    cJSON * element = NULL;
    cJSON * next    = NULL;

    BMON_TRACE("enter");

    if (NULL == object)
    {
        return;
    }

    /* prune children recursively */
    for (element = (object != NULL) ? (object)->child : NULL; element != NULL; )
    {
        /* save next prior to json_PruneEmptyObjects() call in case element gets deleted */
        next = element->next;

        /* recursion */
        json_PruneEmptyObjects(element, object);

        element = next;
    }

    /* remove cjson objects with no children */
    if ((NULL != object) &&
        (NULL == object->child) &&
        ((cJSON_IsObject(object)) || (cJSON_IsArray(object))))
    {
        FPRINTF(stderr, "delete empty object:[%s] from parent:[%s]\n", object->string, parent->string);
        cJSON_Delete(cJSON_DetachItemViaPointer(parent, object));
    }

    BMON_TRACE("exit");
} /* json_PruneEmptyObjects */

/***************************************************************************
 * Summary:
 * Print the JSON tree pointed to by pObject to stdout.
 * Exporting FORMATTED_JSON="y" will pretty print the JSON output
 ***************************************************************************/
int json_Print(
        cJSON *   pObject,
        char *    buffer,
        const int length
        )
{
    int    rc                   = 0;
    char * settingFormattedJSON = getenv("FORMATTED_JSON");
    bool   bFormattedJSON       = (NULL != settingFormattedJSON) && (0 == strncmp(settingFormattedJSON, "y", 1)) ? true : false;

    BMON_TRACE("enter");

    /* filtering rules (exclude queries) may result in empty objects
     * so we will traverse tree depth first and remove any empty objects
     */
    json_PruneEmptyObjects(pObject, NULL);

    rc = (true == cJSON_PrintPreallocated(pObject, buffer, length, bFormattedJSON) ? 0 : -1);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    BMON_TRACE("exit");
    return(rc);
} /* json_Print */

/***************************************************************************
 * Summary:
 * Query filter implementation for "include" queries.
 ***************************************************************************/
static bool queryTestInclude(
        const char * queryInclude,
        const char * path,
        const char * id,
        const bool   bStrict
        )
{
    bool   bRet     = false;
    char * pContext = NULL;
    char * pInclude = NULL;
    char   strQuery[512];

    assert(NULL != queryInclude);
    assert(NULL != id);

    BMON_TRACE("enter");

    strncpy(strQuery, queryInclude, sizeof(strQuery));

    /* test all '+' separated query strings individually by prepending path */
    pInclude = strtok_r(strQuery, "+\0", &pContext);
    while (NULL != pInclude)
    {
        char newPath[1536];

        /* prepend path */
        strncpy(newPath, path, sizeof(newPath));
        strncat(newPath, "/", 1);
        strncat(newPath, pInclude, sizeof(newPath));
        TRIM_LEADING_SLASH(newPath);

        /* check include query */
        {
            int nCompareLen = bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id));

            FPRINTF(stderr, "%s() ======> Testing include query path:%s\n", __FUNCTION__, newPath);
            if (0 == strncmp(id, newPath, nCompareLen))
            {
                FPRINTF(stderr, "%s()\t\t%s filter SUCCESS - INCLUDE query filter:[%s] id:[%s] len:%ld\n",
                        __FUNCTION__, bStrict ? "STRICT" : "LOOSE", newPath, id,
                        bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id)));
                bRet = true;
                goto done;
            }
        }

        FPRINTF(stderr, "%s()\t\t%s filter FAIL - INCLUDE query filter:[%s] id:[%s] len:%ld\n",
                __FUNCTION__, bStrict ? "STRICT" : "LOOSE", newPath, id,
                bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id)));
        pInclude = strtok_r(NULL, "+\0", &pContext);
    }
done:
    BMON_TRACE("exit");
    return(bRet);
} /* queryTestInclude */

/***************************************************************************
 * Summary:
 * Query filter implementation for "exclude" queries.
 ***************************************************************************/
static bool queryTestExclude(
        const char * queryExclude,
        const char * path,
        const char * id,
        const bool   bStrict
        )
{
    bool   bRet     = true;
    char * pContext = NULL;
    char * pExclude = NULL;
    char   strQuery[512];

    assert(NULL != queryExclude);
    assert(NULL != id);

    BMON_TRACE("enter");
    strncpy(strQuery, queryExclude, sizeof(strQuery));

    /* compare path first.  if path fails then return false.  otherwise test exclude query. */
    if (0 < strlen(path))
    {
        int nCompareLen = bStrict ? strlen(path) : MIN(strlen(path), strlen(id));

        FPRINTF(stderr, "%s() ======> Testing Path:%s\n", __FUNCTION__, path);
        if (0 != strncmp(id, path, nCompareLen))
        {
            FPRINTF(stderr, "%s()\t\t%s filter FAIL - PATH query filter:[%s] id:[%s] len:%d\n",
                    __FUNCTION__, bStrict ? "STRICT" : "LOOSE", path, id, nCompareLen);
            bRet = false;
            goto done;
        }

        FPRINTF(stderr, "%s()\t\t%s filter SUCCESS - PATH query filter:[%s] id:[%s] len:%d\n",
                __FUNCTION__, bStrict ? "STRICT" : "LOOSE", path, id, nCompareLen);
    }

    if (false == bStrict)
    {
        goto done;
    }

    /* test all '+' separated query strings individually by prepending path */
    pExclude = strtok_r(strQuery, "+\0", &pContext);
    while (NULL != pExclude)
    {
        char newPath[1536];
        int  nCompareLen = 0;

        /* prepend path */
        strncpy(newPath, path, sizeof(newPath));
        strncat(newPath, "/", 1);
        strncat(newPath, pExclude, sizeof(newPath));
        TRIM_LEADING_SLASH(newPath);

        /* check Exclude query */
        nCompareLen = bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id));

        FPRINTF(stderr, "%s() ======> Testing exclude query path:%s\n", __FUNCTION__, newPath);
        if (0 == strncmp(id, newPath, nCompareLen))
        {
            bRet = bStrict ? false : true;

            if (true == bStrict)
            {
                /* id is excluded so break */
                FPRINTF(stderr, "%s()\t\t%s filter %s - EXCLUDE query filter:[%s] id:[%s] len:%ld\n",
                        __FUNCTION__, bStrict ? "STRICT" : "LOOSE", bRet ? "SUCCESS" : "FAIL", newPath, id,
                        bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id)));
                break;
            }
        }

        FPRINTF(stderr, "%s()\t\t%s filter %s - EXCLUDE query filter:[%s] id:[%s] len:%ld\n",
                __FUNCTION__, bStrict ? "STRICT" : "LOOSE", bRet ? "SUCCESS" : "FAIL", newPath, id,
                bStrict ? strlen(newPath) : MIN(strlen(newPath), strlen(id)));
        pExclude = strtok_r(NULL, "+\0", &pContext);
    }
done:
    BMON_TRACE("exit");
    return(bRet);
} /* queryTestExclude */

/***************************************************************************
 * Summary:
 * Filter implementation which supports both loose and strict filtering.
 * See filterStrict() and filterLoose() for more info.
 ***************************************************************************/
static bool filterTest(
        const char * filter,
        const char * id,
        const bool   bStrict
        )
{
    int      rc          = 0;
    bool     bRet        = false;
    size_t   nCompareLen = 0;
    bmon_uri uriParsed;
    char     strVal[512];

    BMON_TRACE("enter");
    memset(strVal, 0, sizeof(strVal));

    if (NULL == filter)
    {
        FPRINTF(stderr, "%s()\t\tNULL filter = SUCCESS (id:%s)\n", __FUNCTION__, id);
        return(true);
    }
    if (0 == strncmp(filter, "/", 2))
    {
        FPRINTF(stderr, "%s()\t\t'/' filter = SUCCESS (id:%s)\n", __FUNCTION__, id);
        return(true);
    }

    memset(&uriParsed, 0, sizeof(bmon_uri));

    rc = bmon_uri_parse(filter, &uriParsed);
    CHECK_ERROR_GOTO("Failure parsing URI", rc, error);

    FPRINTF(stderr, "%s()\t\turi:[%s]\n", __FUNCTION__, filter);

    /* check "include" query */
    if (bmon_uri_find_tagvalue(filter, "include", strVal, sizeof(strVal)))
    {
        bRet = queryTestInclude(strVal, uriParsed.path, id, bStrict);

        FPRINTF(stderr, "%s()\t\t%s filter %s! - INCLUDE query filter:[%s] id:[%s] len:%ld\n",
                __FUNCTION__, "STRICT", bRet ? "SUCCESS" : "FAIL", uriParsed.path, id, strlen(uriParsed.path));
    }
    else /* check "exclude" path */
    if (bmon_uri_find_tagvalue(filter, "exclude", strVal, sizeof(strVal)))
    {
        /* use "exclude" query */
        bRet = queryTestExclude(strVal, uriParsed.path, id, bStrict);

        FPRINTF(stderr, "%s()\t\t%s filter %s! - EXCLUDE query filter:[%s] id:[%s] len:%ld\n",
                __FUNCTION__, "STRICT", bRet ? "SUCCESS" : "FAIL", uriParsed.path, id, strlen(uriParsed.path));
    }
    else /* check path */
    {
        nCompareLen = bStrict ? strlen(uriParsed.path) : MIN(strlen(uriParsed.path), strlen(id));

        FPRINTF(stderr, "%s() ======> Testing path only:%s\n", __FUNCTION__, uriParsed.path);
        if (0 == strncmp(id, uriParsed.path, nCompareLen))
        {
            bRet = true;
        }

        FPRINTF(stderr, "%s()\t\t%s filter %s! - filter:[%s] id:[%s] len:%ld\n",
                __FUNCTION__, "STRICT", bRet ? "SUCCESS" : "FAIL", uriParsed.path, id, nCompareLen);
    }

    goto done;
error:
    FPRINTF(stderr, "%s() error!\n", __FUNCTION__);
done:
    BMON_TRACE("exit");
    return(bRet);
} /* filterTest */

/***************************************************************************
 * Summary:
 * Very simple filter check. returns true for pass, false for fail.
 * note that filterStrict("/abc/d", "/abc/def") will pass
 * but filterStrict("/abc/def", "/abd/d") will NOT pass
 * NULL filter or "/" filter will always pass
 ***************************************************************************/
static bool filterStrict(
        const char * filter,
        const char * id
        )
{
    return(filterTest(filter, id, true));
} /* filterStrict */

/***************************************************************************
 * Summary:
 * Very simple filter check.  returns true for pass, false for fail.
 * note that filter and id are trimmed to the shortest length before
 * comparison so both CHECK_FILTER_GOTO("/abc/def", "/abc/d") and
 * CHECK_FILTER_GOTO("/abc/d", "/abc/def") will pass.
 * NULL filter or "/" filter will always pass
 ***************************************************************************/
static bool filterLoose(
        const char * filter,
        const char * id
        )
{
    return(filterTest(filter, id, false));
} /* filterLoose */

/***************************************************************************
 * Summary:
 * Given the root object and the current object, this function traverses
 * the JSON tree and generates a path which is saved in strPath.
 **************************************************************************/
static int json_FindPath(
        const cJSON * root,
        const cJSON * object,
        char *        strPath,
        size_t        sizePath
        )
{
    cJSON * element = NULL;

    assert(NULL != strPath);

    if (root == object)
    {
        /* found the intended object - path is generated as we unravel
         * the recursion
         */
        memset(strPath, 0, sizePath);
        return(0);
    }
    else
    if (NULL == object)
    {
        return(-1);
    }

    cJSON_ArrayForEach(element, root)
    {
        /* recursive call to continue tree traversal */
        if (0 == json_FindPath(element, object, strPath, sizePath))
        {
            unsigned lenPath    = strlen(strPath);
            unsigned lenElement = (NULL != element->string) ? strlen(element->string) : 0;

            /* found element */
            if (0 < lenElement)
            {
                /* prefix element->string to strPath - do in place as we recurse back.*/

                if (0 < lenPath)
                {
                    /* shift extra space for '/' */
                    lenElement++;
                }

                if (lenPath + lenElement < sizePath)
                {
                    unsigned i = 0;

                    /* shift strPath current contents to make room for object->string */
                    for (i = lenPath + lenElement; i >= lenElement; i--)
                    {
                        strPath[i] = strPath[i - lenElement];
                    }

                    /* copy object->string into strPath */
                    memcpy(strPath, element->string, lenElement);

                    if (0 < lenPath)
                    {
                        /* add slash between current element and previous path */
                        strPath[i] = '/';
                    }
                }
            }

            return(0);
        }
    }

    return(-1);
} /* json_FindPath */

/***************************************************************************
 * Summary:
 * Plugins are given an filter which is basically a partial URI.
 * This filter is compared to the given id (a path in JSON tree) to
 * determine where to add the data to the output JSON tree. Note that
 * json_AddXXXX() APIs will auto check filters so there is little need
 * to call this function on its own.  However, if the plugin wishes to
 * test a filter ONCE prior to adding a large number of array elements
 * (assuming there is no need to check the filter for every element of the
 * array), then each array element can be added using NO_FILTER for the
 * filter parameter of the json_AddXXXX().
 * Returns true if passes filter,
 * false otherwise.
 ***************************************************************************/
bool json_CheckFilter(
        const cJSON * object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name
        )
{
    char id[2048] = "\0";

    BMON_TRACE("enter");

    if (NULL != objectRoot)
    {
        char path[1024];

        json_FindPath(objectRoot, object, path, sizeof(path));
        snprintf(id, sizeof(id), "%s/%s", path, name);
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    BMON_TRACE("exit");
    return(filterStrict(filter, id));
} /* json_CheckFilter */

/***************************************************************************
 * Summary:
 * Add a new JSON object with given name to the given object,
 * if given jsonPath passes the given filter.  This new object is
 * typically used as a container for other objects.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddObject(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterLoose(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* array elements will be wrapped with a arrayItem object */
        arrayItem = cJSON_CreateObject();
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
        cJSON_AddItemToArray(object, arrayItem);
        pRet = cJSON_AddObjectToObject(arrayItem, name);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), pRet, rc, -1, error);
    }
    else
    {
        pRet = cJSON_AddObjectToObject(object, name);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddObject */

/***************************************************************************
 * Summary:
 * Add a new JSON array with given name to the given object, if given
 * jsonPath passes the given filter. JSON arrays can contain multiple
 * different json types, but will only store the data value once added to
 * the array.  In order to simplify our most common use case where we want
 * arrays to contain tag/value pairs, jsonAddString(), jsonAddNumber(),
 * jsonAddBool() will all automatically create an intermediate container
 * object to preserve tags.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddArray(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterLoose(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* array elements will be wrapped with a arrayItem object */
        arrayItem = cJSON_CreateObject();
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
        cJSON_AddItemToArray(object, arrayItem);
        pRet = cJSON_AddArrayToObject(arrayItem, name);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON array"), pRet, rc, -1, error);
    }
    else
    {
        pRet = cJSON_AddArrayToObject(object, name);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON array"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddArray */

/***************************************************************************
 * Summary:
 * add a new empty JSON array element to the given array.  note that this
 * function does not require a filter as it will be a generic unnamed
 * cJSON object.  its primary purpose is to serve as a container for
 * multiple objects in an array node.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddArrayElement(
        cJSON * object
        )
{
    cJSON * pArrayItem = cJSON_CreateObject();

    if (NULL != pArrayItem)
    {
        cJSON_AddItemToArray(object, pArrayItem);
    }

    return(pArrayItem);
} /* json_AddArrayElement */

/***************************************************************************
 * Summary:
 * add a new JSON string to the given object, if the given jsonPath passed
 * the given filter.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddString(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name,
        const char *  string
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterStrict(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* if user does not want array elements to have label before the value */
        if (name == NULL)
        {
            /* array elements will not be wrapped */
            arrayItem = cJSON_CreateString(string);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON arrayItem object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem); /* returns void */
        }
        else
        {
            /* array elements will be wrapped with a arrayItem object */
            arrayItem = cJSON_CreateObject();
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem);
            pRet = cJSON_AddStringToObject(arrayItem, name, string);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON string"), pRet, rc, -1, error);
        }
    }
    else
    {
        pRet = cJSON_AddStringToObject(object, name, string);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON string"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddString */

/***************************************************************************
 * Summary:
 * add a new JSON number to the given object, if the given jsonPath passed
 * the given filter.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddNumber(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name,
        const double  number
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterStrict(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* if user does not want array elements to have label before the value */
        if (name == NULL)
        {
            /* array elements will not be wrapped */
            arrayItem = cJSON_CreateNumber(number);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON arrayItem object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem); /* returns void */
        }
        else
        {
            /* array elements will be wrapped with a arrayItem object */
            arrayItem = cJSON_CreateObject();
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem);
            pRet = cJSON_AddNumberToObject(arrayItem, name, number);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON number"), pRet, rc, -1, error);
        }
    }
    else
    {
        pRet = cJSON_AddNumberToObject(object, name, number);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON number"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddNumber */

/***************************************************************************
 * Summary:
 * add a new JSON boolean to the given object, if the given jsonPath passed
 * the given filter.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddBool(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name,
        const bool    bValue
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterStrict(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* if user does not want array elements to have label before the value */
        if (name == NULL)
        {
            /* array elements will not be wrapped */
            arrayItem = cJSON_CreateBool(bValue);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON arrayItem object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem); /* returns void */
        }
        else
        {
            /* array elements will be wrapped with a arrayItem object */
            arrayItem = cJSON_CreateObject();
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem);
            pRet = cJSON_AddBoolToObject(arrayItem, name, bValue);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON bool"), pRet, rc, -1, error);
        }
    }
    else
    {
        pRet = cJSON_AddBoolToObject(object, name, bValue);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON bool"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddBool */

/***************************************************************************
 * Summary:
 * add a new JSON null to the given object, if the given jsonPath passed
 * the given filter.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddNull(
        cJSON *       object,
        const char *  filter,
        const cJSON * objectRoot,
        const char *  name
        )
{
    cJSON * pRet  = NULL;
    int     rc    = 0;
    bool    bPass = false;
    char    id[2048];

    UNUSED(rc);

    BMON_TRACE("enter");

    /* append given name to jsonPath if not NULL */
    if (NULL != objectRoot)
    {
        char jsonPath[1024];

        json_FindPath(objectRoot, object, jsonPath, sizeof(jsonPath));
        if (0 < strlen(jsonPath))
        {
            snprintf(id, sizeof(id), "%s/%s", jsonPath, name);
        }
        else
        {
            snprintf(id, sizeof(id), "%s", name);
        }
    }
    else
    {
        snprintf(id, sizeof(id), "%s", name);
    }

    bPass = filterStrict(filter, id);
    if (false == bPass)
    {
        FPRINTF(stderr, "%s()\t\tfilter:%s id:%s fail\n", __FUNCTION__, filter, id);
        goto done;
    }

    if (true == cJSON_IsArray(object))
    {
        cJSON * arrayItem = NULL;

        /* if user does not want array elements to have label before the value */
        if (name == NULL)
        {
            /* array elements will not be wrapped */
            arrayItem = cJSON_CreateNull();
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON arrayItem object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem); /* returns void */
        }
        else
        {
            /* array elements will be wrapped with a arrayItem object */
            arrayItem = cJSON_CreateObject();
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON object"), arrayItem, rc, -1, error);
            cJSON_AddItemToArray(object, arrayItem);
            pRet = cJSON_AddNullToObject(arrayItem, name);
            CHECK_PTR_ERROR_GOTO(("Unable to create JSON bool"), pRet, rc, -1, error);
        }
    }
    else
    {
        pRet = cJSON_AddNullToObject(object, name);
        CHECK_PTR_ERROR_GOTO(("Unable to create JSON bool"), pRet, rc, -1, error);
    }

    goto done;
error:
done:
    BMON_TRACE("exit");
    return(pRet);
} /* json_AddNull */

/***************************************************************************
 * Summary:
 * Generate a bmon JSON error objec and connect to given pObject
 *
 * Returns:
 * 0 on success, -1 otherwise
 ***************************************************************************/
int json_GenerateError(
        cJSON *          pObject,              /* parent object for header */
        BMON_ERROR_CODE  code,                 /* error code */
        const char *     message               /* cerbose error string*/
        )
{
    cJSON * objError = NULL;
    if(!pObject)
    {
        FPRINTF(stderr, "Invalid object");
        return -1;
    }
    objError = json_AddObject(pObject,NO_FILTER,pObject,"error");
    if(!objError)
    {
        FPRINTF(stderr, "Unable to add error object");
        return -1;
    }
    json_AddNumber(objError,NO_FILTER,pObject,"code",(const double)code);
    json_AddString(objError,NO_FILTER,pObject,"message",message);
    return 0;
}/* json_GenerateError */
