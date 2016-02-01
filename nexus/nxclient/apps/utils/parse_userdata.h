/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
 *****************************************************************************/
#ifndef PARSE_USERDATA_H__
#define PARSE_USERDATA_H__

#include "nexus_types.h"
#include "nexus_simple_video_decoder.h"

/**
NOTE: This API is only example code. It is subject to change and 
is not supported as a standard reference software deliverable.
**/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buserdata *buserdata_t;

typedef struct buserdata_settings
{
    NEXUS_UserDataFormat formatFilter;
    bool preferredUserDataFormat;        /* Whatever format is set by formatFilter is treated as the preferred format, but
                                            until that format is detected, the parser will operate in NEXUS_UserDataFormat_eAny mode.
                                            When the preferred format is detected, the parser will switch to the preferred mode until decode is stopped. */
    unsigned bufferSize;
} buserdata_settings;

void buserdata_get_default_settings(buserdata_settings *psettings);
buserdata_t buserdata_create(const buserdata_settings *psettings);
void buserdata_destroy(buserdata_t handle);

/* 
local function which consumes and parses data from NEXUS_SimpleVideoDecoder_ReadUserDataBuffer.
because there is no persistent storage, you must pass enough memory to read all entries, otherwise they are dropped.
*/
int buserdata_parse(
    buserdata_t handle,
    NEXUS_SimpleVideoDecoderHandle videoDecoder, 
    NEXUS_ClosedCaptionData *pClosedCaption, /* size is determined by totalEntries*sizeof(NEXUS_ClosedCaption) */
    unsigned totalEntries,
    unsigned *pNumRead /* returns number of entries read. */
    );

#ifdef __cplusplus
}
#endif

#endif
