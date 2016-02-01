/***************************************************************************
 *     (c)2008-2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#ifndef NEXUS_VIDEO_INPUT_VBI_H__
#define NEXUS_VIDEO_INPUT_VBI_H__

#include "nexus_video_input.h"
#include "nexus_vbi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************************************
Not all NEXUS_VideoInput's are supported. Only AnalogVideoDecoder and 656 input are supported.

Incoming teletext and closed caption data is queued in VideoInput. If the queue
overflows, an ERR message will be written to the console.

These interfaces are not used for routing data from the VideoInput directly to the VEC.
That routing happens internal to Nexus. See NEXUS_DisplayVbiSettings for configuration options.

The VBI Read functions are used if the application wants to capture and process the data (e.g. for
graphics rendering).
******************************************/

/**
Summary:
VBI control for the VideoInput
**/
typedef struct NEXUS_VideoInputVbiSettings
{
    NEXUS_CallbackDesc wssChanged;  /* The WSS value has changed. Call NEXUS_VideoInput_GetWss to get the new value. */
    NEXUS_CallbackDesc cgmsChanged; /* The CGMS value has changed. Call NEXUS_VideoInput_GetCgms to get the new value. */
    NEXUS_CallbackDesc vpsChanged;  /* The VPS value has changed. Call NEXUS_VideoInput_GetVps to get the new value. */
    NEXUS_CallbackDesc closedCaptionDataReady; /* New closed caption data has arrived. */
    NEXUS_CallbackDesc teletextDataReady; /* New teletext data has arrived. */
    NEXUS_CallbackDesc gemStarDataReady; /* New gemstar data has arrived. */

    bool closedCaptionEnabled;      /* Capture closed caption data from the NEXUS_VideoInput for NEXUS_VideoInput_ReadClosedCaption function */
    bool teletextEnabled;           /* Capture teletext data from the NEXUS_VideoInput for NEXUS_VideoInput_ReadTeletext function */
    bool gemStarEnabled;            /* Capture GemStar data from the NEXUS_VideoInput for NEXUS_VideoInput_ReadGemStar function */

    unsigned closedCaptionBufferSize; /* Number of NEXUS_ClosedCaptionData entries to buffer */
    unsigned teletextBufferSize;      /* Number of NEXUS_TeletextLine entries to buffer */
    unsigned gemStarBufferSize;       /* Number of NEXUS_GemStarData entries to buffer */

    struct {
        unsigned baseLineTop;    /* The video line number corresponding to the first bit in the following bit mask. */
        uint16_t lineMaskTop;    /* Bit mask that indicates which video lines shall carry Gemstar data in top video fields. */
        unsigned baseLineBottom; /* The video line number corresponding to the first bit in the following bit mask. Must be greater than 256. */
        uint16_t lineMaskBottom; /* Bit mask that indicates which video lines shall carry Gemstar data in bottom video fields. */
    } gemStar;

    struct {
        uint16_t vline576i;      /* The video line number to use for 576I/PAL decoding. */
    } wss;

    struct {
        uint16_t ccLineTop;      /* The line number at which CloseCaption is present*/
        uint16_t ccLineBottom;
    } cc;
} NEXUS_VideoInputVbiSettings;

/**
Summary:
Get current VBI settings
**/
void NEXUS_VideoInput_GetVbiSettings(
    NEXUS_VideoInputHandle handle,
    NEXUS_VideoInputVbiSettings *pSettings /* [out] */
    );

/**
Summary:
Set new VBI settings
**/
NEXUS_Error NEXUS_VideoInput_SetVbiSettings(
    NEXUS_VideoInputHandle handle,
    const NEXUS_VideoInputVbiSettings *pSettings
    );

/**
Summary:
Read an array of lines of queued teletext data from the video input
**/
NEXUS_Error NEXUS_VideoInput_ReadTeletext(
    NEXUS_VideoInputHandle handle,
    NEXUS_TeletextLine *pLines, /* attr{nelem=numLines;nelem_out=pNumLinesRead;reserved=4} [out] array of NEXUS_TeletextLine entries that VideoInput can populate */
    size_t numLines,            /* maximum number of entries that can be written to pLines */
    size_t *pNumLinesRead       /* [out] actual number of entries written to pLines */
    );

/**
Summary:
Flush all buffered teletext data
**/
void NEXUS_VideoInput_FlushTeletext(
    NEXUS_VideoInputHandle videoInput
    );

/**
Summary:
Read an array of queued closed caption data from the video input
**/
NEXUS_Error NEXUS_VideoInput_ReadClosedCaption(
    NEXUS_VideoInputHandle handle,
    NEXUS_ClosedCaptionData *pEntries,  /* attr{nelem=numEntries;nelem_out=pNumEntriesRead;reserved=4} [out] array of NEXUS_ClosedCaptionData entries that VideoInput can populate */
    size_t numEntries,                  /* maximum number of entries that can be written to pEntries */
    size_t *pNumEntriesRead             /* [out] actual number of entries written to pEntries */
    );

/**
Summary:
Flush all buffered closed caption data
**/
void NEXUS_VideoInput_FlushClosedCaption(
    NEXUS_VideoInputHandle videoInput
    );

/**
Summary:
Read an array of queued GemStar data from the video input
**/
NEXUS_Error NEXUS_VideoInput_ReadGemStar(
    NEXUS_VideoInputHandle handle,
    NEXUS_GemStarData *pEntries,        /* attr{nelem=numEntries;nelem_out=pNumEntriesRead;reserved=4} [out] array of NEXUS_GemStarData entries that VideoInput can populate */
    size_t numEntries,                  /* maximum number of entries that can be written to pEntries */
    size_t *pNumEntriesRead             /* [out] actual number of entries written to pEntries */
    );

/**
Summary:
Flush all buffered GemStar data
**/
void NEXUS_VideoInput_FlushGemStar(
    NEXUS_VideoInputHandle videoInput
    );

/**
Summary:
Read the current WSS value from the video input

Description:
See NEXUS_Display_SetWss for data format.
**/
NEXUS_Error NEXUS_VideoInput_GetWss(
    NEXUS_VideoInputHandle handle,
    uint16_t *pWssData /* [out] A value of 0xFFFF means no data is present. */
    );


/**
Summary:
Flush all buffered WSS data
**/
void NEXUS_VideoInput_FlushWss(
    NEXUS_VideoInputHandle videoInput
    );

/**
Summary:
Read the current CGMS value from the video input

Description:
See NEXUS_Display_SetCgms for data format.
**/
NEXUS_Error NEXUS_VideoInput_GetCgms(
    NEXUS_VideoInputHandle handle,
    uint32_t *pCgmsData /* [out] */
    );


/**
Summary:
Flush all buffered CGMS data
**/
void NEXUS_VideoInput_FlushCgms(
    NEXUS_VideoInputHandle videoInput
    );

/**
Summary:
Read the current VPS value from the video input
**/
NEXUS_Error NEXUS_VideoInput_GetVps(
    NEXUS_VideoInputHandle handle,
    NEXUS_VpsData *pData /* [out] */
    );


/**
Summary:
Flush all buffered VPS data
**/
void NEXUS_VideoInput_FlushVps(
    NEXUS_VideoInputHandle videoInput
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_INPUT_VBI_H__ */
