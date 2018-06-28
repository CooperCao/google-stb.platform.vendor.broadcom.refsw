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

#ifndef __BMON_JSON_H__
#define __BMON_JSON_H__

#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"

#define NO_FILTER  NULL

/***************************************************************************
 * Summary:
 * Create and initialize a cJSON root object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_Initialize(void);

/***************************************************************************
 * Summary:
 * Destroy the given cJSON root object tree
 ***************************************************************************/
void json_Uninitialize(cJSON ** pObject);

/***************************************************************************
 * Summary:
 * Generate a bmon JSON header and connect to given pObject
 *
 * Returns:
 * pointer to created header cJSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_GenerateHeader(
        cJSON *          pObject,              /* parent object for header */
        const char *     strPluginName,        /* name string */
        const char *     strPluginDescription, /* description string */
        struct timeval * pTv,                  /* time val */
        const char *     pluginVersion         /* version string */
        );

/***************************************************************************
 * Summary:
 * Generate a bmon JSON error objec and connect to given pObject
 *
 * Returns:
 * 0 on success, -1 otherwise
 ***************************************************************************/
typedef enum
{
        BMON_CALLER_ERR_BAD_REQUEST=400,       /* return this error  for bad query */
        BMON_CALLER_ERR_RES_NOT_FOUND=404,     /* resource not found */
        BMON_PLUGIN_ERR=500                    /* internal server error at plugin */
}BMON_ERROR_CODE;
int json_GenerateError(
        cJSON *          pObject,              /* parent object for header */
        BMON_ERROR_CODE  code,                 /* error code */
        const char *     message               /* cerbose error string*/
        );


/***************************************************************************
 * Summary:
 * Print the JSON tree pointed to by pObject to stdout.
 * Exporting FORMATTED_JSON="y" will pretty print the JSON output
 *
 * Returns:
 * 0 on success, -1 otherwise
 ***************************************************************************/
int json_Print(
        cJSON *   object,   /* object to begin printing */
        char *    buffer,   /* buffer to save printed JSON to */
        const int length    /* size of buffer */
        );

/***************************************************************************
 * Summary:
 * Plugins are given an filter which is basically a partial URI.
 * This filter is compared to the given object (a path in JSON tree) to
 * determine where to add the data to the output JSON tree. Note that
 * json_AddXXXX() APIs will auto check filters so there is little need
 * to call this function on its own.  However, if a plugin wishes to
 * test a filter ONCE prior to adding a large number of array elements
 * (assuming there is no need to check the filter for every element of the
 *
 * Returns:
 * true if pass, false otherwise
 ***************************************************************************/
bool json_CheckFilter(
        const cJSON * object,     /* parent object to check filter with */
        const char *  filter,     /* string filter */
        const cJSON * objectRoot, /* root object at top of JSON tree ("data" object in most cases) */
        const char *  name        /* pObject JSON path + name is compared to given filter */
        );

/***************************************************************************
 * Summary:
 * Add a new JSON object with given name to the given object,
 * if given object/name passes the given filter.  This new object is
 * typically used as a container for other objects.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddObject(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* string filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name         /* object JSON path + name is compared to the given filter */
        );

/***************************************************************************
 * Summary:
 * Add a new JSON array with given name to the given object, if given
 * jsonPath passes the given filter. JSON arrays can contain multiple
 * different json types, but will only store the data value once added to
 * the array.  In order to simplify our most common use case where we want
 * arrays to contain tag/value pairs, jsonAddString(), jsonAddNumber(),
 * jsonAddBool() will all automatically create an intermediate container
 * object to preserve tags.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddArray(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* string filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name         /* object JSON path + name is compared to the given filter */
        );

/***************************************************************************
 * Summary:
 * add a new empty JSON array element to the given array.  note that this
 * function does not require a filter as it will be a generic unnamed
 * cJSON object.  its primary purpose is to serve as a container for
 * multiple objects in an array node.
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddArrayElement(
        cJSON * object
        );

/***************************************************************************
 * Summary:
 * add a new JSON string to the given object, if the given object/name passes
 * the given filter.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddString(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* tring filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name,        /* object JSON path + name is compared to the given filter */
        const char *  string       /* string to add */
        );

/***************************************************************************
 * Summary:
 * add a new JSON number to the given object, if the given object/name passes
 * the given filter.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddNumber(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* string filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name,        /* object JSON path + name is compared to the given filter */
        const double  number       /* number to add */
        );

/***************************************************************************
 * Summary:
 * add a new JSON boolean to the given object, if the given object/name passes
 * the given filter.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddBool(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* string filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name,        /* object JSON path + name is compared to the given filter */
        const bool    bValue       /* boolean to add */
        );

/***************************************************************************
 * Summary:
 * add a new JSON null to the given object, if the given object/name passes
 * the given filter.
 *
 * Returns:
 * new JSON object
 ***************************************************************************/
CJSON_PUBLIC(cJSON *) json_AddNull(
        cJSON *       object,      /* parent object to check filter with */
        const char *  filter,      /* string filter */
        const cJSON * objectRoot,  /* root object at top fo JSON tree ("data" object in most cases) */
        const char *  name         /* object JSON path + name is compared to the given filter */
        );
#endif /* __BMON_JSON_H__ */