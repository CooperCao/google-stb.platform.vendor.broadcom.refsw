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
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <netdb.h> /* getservbyname */

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_uri.h"

/***************************************************************************
 * Summary:
 * Lookup the default port number for the given service/protocol
 ***************************************************************************/
static int lookupDefaultPort(
        const char * service,
        const char * protocol
        )
{
#ifndef NO_SOCKETS
    struct servent * s = getservbyname(service, protocol);
    if (s)
    {
        return(ntohs(s->s_port));
    }
    else
#endif /* ifndef NO_SOCKETS */
    return(0);
} /* lookupDefaultPort */

/***************************************************************************
 * Summary:
 * Replace oldCh character with newCh character in string str.
 * Returns the number of replaced characters.
 ***************************************************************************/
static int replace(
        char * str,
        char   oldch,
        char   newCh
        )
{
    int count = 0;

    BMON_TRACE("enter");

    if (str)
    {
        char * s = str;
        while (*s)
        {
            if (*s == oldch)
            {
                *s = newCh;
                count++;
            }
            s++;
        }
    }
    BMON_TRACE("exit");
    return(count);
} /* replace */

/***************************************************************************
 * Summary:
 * Parse the given strURI URI string and return parsed values in the
 * uriParsed struct.  Returns 0 on success, otherwise failure.
 ***************************************************************************/
int bmon_uri_parse(
        const char * strURI,
        bmon_uri *   uriParsed
        )
{
    char str[2048];
    int  rc = 0;

    assert(NULL != strURI);
    assert(NULL != uriParsed);

    BMON_TRACE("enter");
    memset(uriParsed, 0, sizeof(*uriParsed));

    bmon_uri_decode(strURI, str, sizeof(str));
    replace(str, '\\', '/');

    /* STEP_1: parse protocol/host/port if exists */

    if (strncmp(str, "file:", 5))
    {
        /* non "file://" protocol so look for host */
        char * host = (char *)strstr(str, "//");
        if (host)
        {
            char * slash = NULL;
            char * colon = NULL;

            if ((host > str) && (*(host - 1) == ':'))
            {
                strncpy(uriParsed->protocol, str, host - str - 1);
            }
            host += 2;

            slash = strchr(host, '/');
            if (slash)
            {
                strncpy(uriParsed->host, host, MIN(slash - host, (int)sizeof(uriParsed->host)));
                strncpy(uriParsed->query, slash, sizeof(uriParsed->query));
            }
            else
            {
                strncpy(uriParsed->host, host, sizeof(uriParsed->host));
                uriParsed->query[0] = '\0';
            }

            colon = strchr(uriParsed->host, ':');
            if (colon)
            {
                uriParsed->port = atoi(colon + 1);
                *colon          = '\0'; /* cut off port from host string */
            }
            else
            {
                uriParsed->port = lookupDefaultPort(uriParsed->protocol, "tcp");
            }
        }
        else
        {
            /* no protocol - copy to query string for STEP_2 */
            strncpy(uriParsed->query, str, sizeof(uriParsed->query));
        }
    }
    else
    {
        /* file: is the only protocol that doesn't have a server */
        char * path = (char *)strstr(str, "//");
        if (path)
        {
            path += 2;
            strncpy(uriParsed->query, path, sizeof(uriParsed->query));
        }

        strncpy(uriParsed->protocol, "file", sizeof(uriParsed->protocol));
    }

    /* STEP_2: at this point uriParsed->query contains the path, query, and fragment - parse them */

    if (0 < strlen(uriParsed->query))
    {
        char * qmark   = (char *)strchr(uriParsed->query, '?');
        char * hash    = (char *)strchr(uriParsed->query, '#');
        char * pathEnd = uriParsed->query + strlen(uriParsed->query);

        /* calc end of path */
        if (qmark)
        {
            pathEnd = qmark;
        }
        else
        if (hash)
        {
            pathEnd = hash;
        }

        /* extract the path */
        strncpy(uriParsed->path, uriParsed->query, MIN(pathEnd - uriParsed->query, (int)sizeof(uriParsed->path)));

        if (hash)
        {
            /* extract the fragment */
            strncpy(uriParsed->fragment, hash + 1, sizeof(uriParsed->fragment));
            *hash = '\0'; /* cut of fragment from query */
        }

        /* parse the query */
        if (qmark)
        {
            /* save query without leading '?' */
            TRIM_LEADING_CHARS(uriParsed->query, qmark + 1 - uriParsed->query);
        }
        else
        {
            /* we were using uriParsed->query to store path/query/fragment and there is no query
               so we must make sure it is empty */
            uriParsed->query[0] = '\0';
        }
    }

    BMON_TRACE("exit");
    return(rc);
} /* bmon_uri_parse */

/***************************************************************************
 * Summary:
 * Find tag from the uri string and return 1 if found, 0 otherwise
 ***************************************************************************/
int bmon_uri_find_tag(
        const char * str,
        char *       tag
        )
{
    char   strQuery[1024];
    char * pQuery  = NULL;
    char * pEquals = NULL;

    assert(NULL != str);
    assert(NULL != tag);

    memset(strQuery, 0, sizeof(strQuery));

    bmon_uri_decode(str, strQuery, sizeof(strQuery));
    pQuery = strQuery;

    pEquals = strchr(pQuery, '?');
    if (pEquals)
    {
        pQuery = pEquals+1;

        while ((NULL != pQuery) && (pEquals = strchr(pQuery, '=')))
        {
            char strTag[128];

            /* parse tag */
            memset(strTag, 0, sizeof(strTag));
            strncpy(strTag, pQuery, pEquals - pQuery);
            if (strcmp(tag, strTag) == 0)
            {
                return(1);
            }
            else
            {
                pQuery = strchr(pQuery, '&');
                if (NULL != pQuery)
                {
                    pQuery++;
                }
                continue;
            }
        }
    }

    return(0);
} /* bmon_uri_find_tag */

/***************************************************************************
 * Summary:
 * Find value of the tag. return 1 if found, 0 otherwise
 ***************************************************************************/
int bmon_uri_find_tagvalue(
        const char * str,
        const char * tag,
        char *       val,
        size_t       valSize
        )
{
    char   strQuery[1024];
    char * pQuery  = NULL;
    char * pEquals = NULL;

    assert(NULL != str);
    assert(NULL != tag);
    assert(NULL != val);
    assert(0 < valSize);

    memset(strQuery, 0, sizeof(strQuery));

    bmon_uri_decode(str, strQuery, sizeof(strQuery));
    pQuery = strQuery;

    pEquals = strchr(pQuery, '?');
    if (pEquals)
    {
        pQuery = pEquals+1;

        while ((NULL != pQuery) && (pEquals = strchr(pQuery, '=')))
        {
            char strTag[128];

            memset(strTag, 0, sizeof(strTag));
            strncpy(strTag, pQuery, pEquals - pQuery);
            if (strcmp(tag, strTag) == 0)
            {
                goto findval;
            }
            else
            {
                pQuery = strchr(pQuery, '&');
                if (NULL != pQuery)
                {
                    pQuery++;
                }
                continue;
            }
        }
    }

    return(0);

findval:
    memset(val, 0, valSize);
    pQuery  = pEquals + 1;
    pEquals = strchr(pQuery, '&');
    if (pEquals)
    {
        strncpy(val, pQuery, MIN((size_t)(pEquals - pQuery), valSize));
        pEquals++;
    }
    else
    {
        strncpy(val, pQuery, valSize);
        pQuery = NULL;
    }

    return(1);
} /* bmon_uri_find_tagvalue */

/***************************************************************************
 * Summary:
 * Decode hex encoded unsafe URI characters
 ***************************************************************************/
int bmon_uri_decode(
        const char * str,
        char *       result,
        size_t       resultSize
        )
{
    int          rc = 0;
    char         buf[3];
    const char * next;

    assert(NULL != str);
    assert(NULL != result);
    assert(0 < resultSize);

    BMON_TRACE("enter");

    memset(result, 0, resultSize);
    buf[2] = 0;

    while ((next = strchr(str, '%')) && next[1] && next[2])
    {
        int ch;

        strncat(result, str, next-str);
        next++; /* skip % */
        memcpy(buf, next, 2);
        ch = strtoul(buf, NULL, 16);

        if (strlen(result) >= resultSize)
        {
            /* out of space in result buffer */
            rc = -1;
            break;
        }
        strncat(result, (char *)&ch, 1);
        str = next + 2; /* skip 2 digits */
    }

    strncat(result, str, resultSize - strlen(result));

    BMON_TRACE("exit");
    return(rc);
} /* bmon_uri_decode */

/* from p197, HTML: The Definitive Guide, 3rd ed.
 * added space, remove /?:=
 */
static char * g_unsafe = (char *)";@&<>\"#%{}|\\^~[]` ";

/***************************************************************************
 * Summary:
 * Encode unsafe URI characters as hex
 ***************************************************************************/
int bmon_uri_encode(
        const char * str,
        char *       result,
        size_t       resultSize
        )
{
    int  rc = 0;
    char buf[4];

    assert(NULL != str);
    assert(NULL != result);
    assert(0 < resultSize);

    BMON_TRACE("enter");

    memset(result, 0, resultSize);

    while (*str)
    {
        int n = strcspn(str, g_unsafe);
        if (n)
        {
            if (strlen(result) >= resultSize)
            {
                rc = -1;
                break;
            }
            strncat(result, str, n);
            str += n;
            if (!*str)
            {
                break;
            }
        }

        snprintf(buf, sizeof(buf), "%%%02x", str[0]);
        strncat(result, buf, resultSize - strlen(result));
        str++;
    }

    BMON_TRACE("exit");
    return(rc);
} /* bmon_uri_encode */

/***************************************************************************
 * Summary:
 * Combine path and query and fill given string buffer
 ***************************************************************************/
void bmon_uri_getPathQuery(
        const bmon_uri * pUri,
        char *     strPathQuery,
        size_t     sizePathQuery
        )
{
    assert(NULL != strPathQuery);
    assert(NULL != pUri);

    bmon_uri_decode(pUri->path, strPathQuery, sizePathQuery);

    if (0 < strlen(pUri->query))
    {
        strncat(strPathQuery, "?", 1);
        strncat(strPathQuery, pUri->query, sizePathQuery - strlen(strPathQuery) - strlen(pUri->query));
    }
} /* bmon_uri_getPathQuery */

/*#define BMON_URI_TEST_CODE*/
#ifdef BMON_URI_TEST_CODE
int main()
{
    char     strURI[]        = "this is a test ";
    char     strURIencoded[] = "this%20is%20a%20test%20";
    char     result[64];
    bmon_uri uriParsed;
    int      i = 0;

    BMON_TRACE("enter");

    result[0] = '\0';
    bmon_uri_decode(strURIencoded, result, sizeof(result));
    printf("bmon_uri_decode(%s)\n", result);
    result[0] = '\0';
    bmon_uri_encode(strURI, result, sizeof(result));
    printf("bmon_uri_encode(%s)\n", result);

    char strTest[][256] = {
        "?include=value1+value2",
        "\\folder1\\folder2?include=value1+value2&exclude=value3#fragment_data",
        "gopher://10.1.0.1\\folder1\\folder2",
        "http://10.1.0.1\\folder1\\folder2#fragment_data",
        "telnet://broadcom@10.1.0.1\\#fragment_data",
        "http://broadcom@10.1.0.1\\?tag1=value1&tag2=value2&tag3=value3",
        "ftp://broadcom@10.1.0.1\\folder1\\folder2?tag1=value1&tag2=value2&tag3=value3",
        "http://10.1.0.1:8080\\folder1\\folder2?tag1=value1&tag2=value2&tag3=value3#fragment_data",
        "file://\\folder1\\folder2?tag1=value1&tag2=value2&tag3=value3#fragment_data",
        "file://\\folder1\\folder2"
    };

    for (i = 0; i < sizeof(strTest) / sizeof(strTest[0]); i++)
    {
        printf("\ntest URI:\t[%s]\n", strTest[i]);
        bmon_uri_parse(strTest[i], &uriParsed);
        printf("protocol:\t[%s]\nhost:\t\t[%s]\nport:\t\t[%d]\npath:\t\t[%s]\nquery:\t\t[%s]\nqueryInclude:\t[%s]\nqueryExclude:\t[%s]\nfragment:\t[%s]\n",
                uriParsed.protocol, uriParsed.host, uriParsed.port, uriParsed.path, uriParsed.query, uriParsed.queryInclude, uriParsed.queryExclude, uriParsed.fragment);
    }

    BMON_TRACE("exit");
} /* main */

#endif /* ifdef BMON_URI_TEST_CODE */
