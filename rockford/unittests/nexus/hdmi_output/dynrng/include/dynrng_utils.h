/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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

#ifndef DYNRNG_UTILS_H__
#define DYNRNG_UTILS_H__

#include "nexus_types.h"
#include "nexus_video_types.h"
#include <stdlib.h>

typedef struct UTILS_StringMap * UTILS_StringMapHandle;

typedef struct UTILS_StringIntMapEntry
{
    const char * key;
    unsigned value;
} UTILS_StringIntMapEntry;

extern const char * const UTILS_STRING_UNKNOWN;
char * UTILS_Trim(char * s);

const char * UTILS_GetVideoCodecName(NEXUS_VideoCodec codec);
const char * UTILS_GetAudioCodecName(NEXUS_AudioCodec codec);
const char * UTILS_GetTransportTypeName(NEXUS_TransportType type);
const char * UTILS_GetColorSpaceName(NEXUS_ColorSpace space);
NEXUS_ColorSpace UTILS_ParseColorSpace(const char * spaceStr);
const char * UTILS_GetVideoFormatName(NEXUS_VideoFormat format);
NEXUS_VideoFormat UTILS_ParseVideoFormat(const char * formatStr);

void UTILS_DestroyStringMap(UTILS_StringMapHandle map);
UTILS_StringMapHandle UTILS_CreateStringMap(void);
int UTILS_PutStringMapValue(UTILS_StringMapHandle map, const char * key, const char * value);
const char * UTILS_GetStringMapValue(UTILS_StringMapHandle map, const char * key);
char * UTILS_SetString(char * oldStr, const char * newStr);
const char * UTILS_GetTableName(const UTILS_StringIntMapEntry * names, unsigned value);
unsigned UTILS_ParseTableAlias(const UTILS_StringIntMapEntry * aliases, const char * alias);

#endif /* DYNRNG_UTILS_H__ */
