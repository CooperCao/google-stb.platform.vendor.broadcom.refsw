/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
******************************************************************************/

#include "dynrng_utils.h"
#include "dynrng_utils_priv.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char * const UTILS_STRING_UNKNOWN = "unknown";

char * UTILS_Trim(char * s)
{
    char * e = NULL;
    if (!s) return NULL;
    e = strchr(s, 0);
    e--;
    while (*s && isspace(*s)) s++;
    while ((e > s) && isspace(*e)) *e-- = 0;
    if (*s) return s;
    else return NULL; /* empty string */
}

static const UTILS_StringIntMapEntry videoCodecNames[] =
{
    { "h265", NEXUS_VideoCodec_eH265 },
    { "h264", NEXUS_VideoCodec_eH264 },
    { "mpeg2", NEXUS_VideoCodec_eMpeg2 },
    { NULL, NEXUS_VideoCodec_eMax }
};

const char * UTILS_GetVideoCodecName(NEXUS_VideoCodec codec)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = &videoCodecNames[0]; e->key; e++)
    {
        if (e->value == codec)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

static const UTILS_StringIntMapEntry audioCodecNames[] =
{
    { "aac", NEXUS_AudioCodec_eAac },
    { "ac3", NEXUS_AudioCodec_eAc3 },
    { "mpeg", NEXUS_AudioCodec_eMpeg },
    { NULL, NEXUS_AudioCodec_eMax }
};

const char * UTILS_GetAudioCodecName(NEXUS_AudioCodec codec)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = &audioCodecNames[0]; e->key; e++)
    {
        if (e->value == codec)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

static const UTILS_StringIntMapEntry transportTypeNames[] =
{
    { "ts", NEXUS_TransportType_eTs },
    { "mp4", NEXUS_TransportType_eMp4 },
    { "es", NEXUS_TransportType_eEs, },
    { NULL, NEXUS_TransportType_eMax },
};

const char * UTILS_GetTransportTypeName(NEXUS_TransportType type)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = &transportTypeNames[0]; e->key; e++)
    {
        if (e->value == type)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

static const UTILS_StringIntMapEntry colorSpaceNames[] =
{
    { "auto", NEXUS_ColorSpace_eAuto },
    { "422", NEXUS_ColorSpace_eYCbCr422 },
    { "420", NEXUS_ColorSpace_eYCbCr420 },
    { "444", NEXUS_ColorSpace_eYCbCr444 },
    { "rgb", NEXUS_ColorSpace_eRgb },
    { NULL, NEXUS_ColorSpace_eMax },
};

const char * UTILS_GetColorSpaceName(NEXUS_ColorSpace space)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = &colorSpaceNames[0]; e->key; e++)
    {
        if (e->value == space)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

NEXUS_ColorSpace UTILS_ParseColorSpace(const char * spaceStr)
{
    const UTILS_StringIntMapEntry * e;
    NEXUS_ColorSpace space = NEXUS_ColorSpace_eMax;

    for (e = &colorSpaceNames[0]; e->value != NEXUS_ColorSpace_eMax; e++)
    {
        if (!strcmp(e->key, spaceStr))
        {
            space = e->value;
            break;
        }
    }

    return space;
}

static const UTILS_StringIntMapEntry videoFormatNames[] =
{
    { "2160p30", NEXUS_VideoFormat_e3840x2160p30hz },
    { "2160p60", NEXUS_VideoFormat_e3840x2160p60hz },
    { "2160p50", NEXUS_VideoFormat_e3840x2160p50hz },
    { "1080p60", NEXUS_VideoFormat_e1080p60hz },
    { "1080p50", NEXUS_VideoFormat_e1080p50hz },
    { NULL, NEXUS_VideoFormat_eMax },
};

const char * UTILS_GetVideoFormatName(NEXUS_VideoFormat format)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = &videoFormatNames[0]; e->key; e++)
    {
        if (e->value== format)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

NEXUS_VideoFormat UTILS_ParseVideoFormat(const char * formatStr)
{
    const UTILS_StringIntMapEntry * e;
    NEXUS_VideoFormat format = NEXUS_VideoFormat_eUnknown;

    for (e = &videoFormatNames[0]; e->key; e++)
    {
        if (!strcmp(e->key, formatStr))
        {
            format = e->value;
            break;
        }
    }

    return format;
}

void UTILS_DestroyStringMap(UTILS_StringMapHandle map)
{
    UTILS_StringNameValuePair * p = NULL;
    if (map)
    {
        for (p = BLST_Q_FIRST(map); p; p = BLST_Q_FIRST(map))
        {
            BLST_Q_REMOVE_HEAD(map, link);
            if (p->key)
            {
                free(p->key);
            }
            if (p->value)
            {
                free(p->value);
            }
            free(p);
        }
        free(map);
    }
}

UTILS_StringMapHandle UTILS_CreateStringMap(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    UTILS_StringMapHandle map = NULL;

    map = malloc(sizeof(struct UTILS_StringMap));
    if (!map) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    memset(map, 0, sizeof(struct UTILS_StringMap));

    return map;

error:
    if (map)
    {
        UTILS_DestroyStringMap(map);
    }
    return NULL;
}

char * UTILS_SetString(char * oldStr, const char * newStr)
{
    unsigned len = 0;
    char * replStr = NULL;

    /* alloc replacement str first, in case it fails */
    if (newStr && (!oldStr || strcmp(oldStr, newStr)))
    {
        len = strlen(newStr);
        replStr = malloc(len + 1);
        if (!replStr) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto end; }
        memset(replStr, 0, len + 1);
        strncpy(replStr, newStr, len);
    }

    /* if replStr exists or newStr is null, free oldStr */
    if (oldStr && (replStr || !newStr))
    {
        free(oldStr);
        oldStr = NULL;
    }

    if (replStr)
    {
        oldStr = replStr;
    }

    /* otherwise leave oldStr alone */

end:
    return oldStr;
}

int UTILS_SetStringMapEntry(UTILS_StringNameValuePair * pair, const char * value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!pair) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    pair->value = UTILS_SetString(pair->value, value);

end:
    return rc;
}

void UTILS_DestroyStringMapEntry(UTILS_StringNameValuePair * pair)
{
    if (pair)
    {
        if (pair->key)
        {
            free(pair->key);
        }
        if (pair->value)
        {
            free(pair->value);
        }
        free(pair);
    }
}

UTILS_StringNameValuePair * UTILS_CreateStringMapEntry(const char * key, const char * value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned len = 0;
    UTILS_StringNameValuePair * pair = NULL;

    pair = malloc(sizeof(UTILS_StringNameValuePair));
    if (!pair) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    memset(pair, 0, sizeof(UTILS_StringNameValuePair));

    len = strlen(key);
    pair->key = malloc(len + 1);
    if (!pair->key) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    memset(pair->key, 0, len + 1);
    strncpy(pair->key, key, len);

    rc = UTILS_SetStringMapEntry(pair, value);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    return pair;

error:
    if (pair)
    {
        UTILS_DestroyStringMapEntry(pair);
    }
    return NULL;
}

int UTILS_PutStringMapValue(UTILS_StringMapHandle map, const char * key, const char * value)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    UTILS_StringNameValuePair * p = NULL;

    p = UTILS_GetStringMapEntry(map, key);
    if (p)
    {
        rc = UTILS_SetStringMapEntry(p, value);
        if (rc) { rc = BERR_TRACE(rc); goto end; } /* don't destroy the entry */
    }
    else
    {
        p = UTILS_CreateStringMapEntry(key, value);
        if (!p) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error_create; }
        BLST_Q_INSERT_HEAD(map, p, link);
    }

end:
    return rc;

error_create:
    if (p)
    {
        UTILS_DestroyStringMapEntry(p);
    }
    goto end;
}

UTILS_StringNameValuePair * UTILS_GetStringMapEntry(UTILS_StringMapHandle map, const char * key)
{
    UTILS_StringNameValuePair * p = NULL;
    UTILS_StringNameValuePair * pair = NULL;

    for (p = BLST_Q_FIRST(map); p; p = BLST_Q_NEXT(p, link))
    {
        if (!strcmp(p->key, key))
        {
            pair = p;
            break;
        }
    }

    return pair;
}

const char * UTILS_GetStringMapValue(UTILS_StringMapHandle map, const char * key)
{
    UTILS_StringNameValuePair * pair = NULL;
    const char * value = NULL;

    pair = UTILS_GetStringMapEntry(map, key);

    if (pair)
    {
        value = pair->value;
    }

    return value;
}

const char * UTILS_GetTableName(const UTILS_StringIntMapEntry * names, unsigned value)
{
    const UTILS_StringIntMapEntry * e;
    const char * name = NULL;

    for (e = names; e->key; e++)
    {
        if (e->value == value)
        {
            name = e->key;
            break;
        }
    }

    return name ? name : UTILS_STRING_UNKNOWN;
}

unsigned UTILS_ParseTableAlias(const UTILS_StringIntMapEntry * aliases, const char * alias)
{
    const UTILS_StringIntMapEntry * e;
    unsigned value = 0;

    for (e = aliases; e->key; e++)
    {
        if (!strcmp(e->key, alias))
        {
            value = e->value;
            break;
        }
    }

    if (!e->key)
    {
        value = e->value;
    }

    return value;
}
