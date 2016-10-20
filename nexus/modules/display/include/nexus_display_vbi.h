/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_DISPLAY_VBI_H__
#define NEXUS_DISPLAY_VBI_H__

#include "nexus_display.h"
#include "nexus_vbi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************************************
These API's control routing of VBI information to a VEC.

Data can be automatically routing from a video input directly to the VEC. Once the app has made the connection, no further interaction is needed.

Data can also be manually written to the VEC.
******************************************/

/*
Summary:
DCS types used in NEXUS_Display_SetDcs
*/
typedef enum NEXUS_DisplayDcsType
{
    NEXUS_DisplayDcsType_off, /* deprecated. use NEXUS_DisplayDcsType_eOff instead. */
    NEXUS_DisplayDcsType_eOff = NEXUS_DisplayDcsType_off,
    NEXUS_DisplayDcsType_on,  /* deprecated. use NEXUS_DisplayDcsType_eOn1 instead. */
    NEXUS_DisplayDcsType_eOn1 = NEXUS_DisplayDcsType_on,
    NEXUS_DisplayDcsType_eOn2,
    NEXUS_DisplayDcsType_eOn3,
    NEXUS_DisplayDcsType_eMax
} NEXUS_DisplayDcsType;

/**
Summary:
VBI encoding control for the Display

Description:
If you want to have automatic routing from the VideoInput to the Display, you must set both xxxEnabled to true and xxxRouting to true.
For VideoDecoder inputs, enabling routing will automatically enable userdata parsing.

NEXUS_Display_WriteXxx functions are used for streaming VBI data (i.e. data that the VBI encoder should not repeat for each field.)
NEXUS_Display_SetXxx functions are used for non-streaming VBI data (i.e. data that the VBI encoder should repeat for each field.)

If you want to use Write/Set functions, you must set xxxEnabled to true and the corresponding xxxRouting to false.
If your app calls both Write/Set as well as enables automatic routing, there will be a collision of data. Results are undefined.

For displays that are not connected to VEC's (e.g. DVO), this interface does nothing.
**/
typedef struct NEXUS_DisplayVbiSettings
{
    NEXUS_VideoInputHandle vbiSource;  /* Select which source is used for automatic routing.
                                    This feature was added for PIP systems (i.e. where they may be more than one source assigned to a display.)
                                    However, you must set this even if you are a single window system. */

    bool closedCaptionRouting;   /* Automatically route closed caption data from vbiSource to this display. closedCaptionEnabled must also be true. */
    bool teletextRouting;        /* deprecated and unused */
    bool wssRouting;             /* deprecated and unused */
    bool cgmsRouting;            /* deprecated and unused */
    bool vpsRouting;             /* deprecated and unused */

    bool teletextEnabled;           /* Encode teletext data in the VEC */
    bool closedCaptionEnabled;      /* Encode closed caption data in the VEC */
    bool wssEnabled;                /* Encode WSS data in the VEC */
    bool cgmsEnabled;               /* Encode CGMS data in the VEC */
    bool vpsEnabled;                /* Encode VPS data in the VEC. Only supported for PAL and SECAM. */
    bool gemStarEnabled;            /* Encode Gemstar data to the VEC. */
    bool amolEnabled;               /* Encode AMOL data to the VEC. */

    bool macrovisionEnabled;        /* unused */
    bool dcsEnabled;                /* unused */

    struct {
        unsigned baseLineTop;       /* The video line number corresponding to the first bit in the following bit mask. */
        uint16_t lineMaskTop;       /* Bit mask that indicates which video lines shall carry Gemstar data in top video fields. */
        unsigned baseLineBottom;    /* The video line number corresponding to the first bit in the following bit mask. Must be greater than 256. */
        uint16_t lineMaskBottom;    /* Bit mask that indicates which video lines shall carry Gemstar data in bottom video fields. */
    } gemStar;

    struct {
        NEXUS_AmolType type;
    } amol;
} NEXUS_DisplayVbiSettings;

/**
Summary:
Get current VBI settings
**/
void NEXUS_Display_GetVbiSettings(
    NEXUS_DisplayHandle handle,
    NEXUS_DisplayVbiSettings *pSettings /* [out] */
    );

/**
Summary:
Set new VBI settings
**/
NEXUS_Error NEXUS_Display_SetVbiSettings(
    NEXUS_DisplayHandle handle,
    const NEXUS_DisplayVbiSettings *pSettings
    );

/**
Summary:
Write an array of lines of teletext data for output on the VEC

Description:
NEXUS_TeletextLine.lineNumber allows multiple fields of teletext data to be sent in one call.
If the lineNumber for a NEXUS_TeletextLine is less than the lineNumber of the previous line, then that data
is transmitted on the next field.

This function is non-blocking. It will queue as many lines as possible. Check pNumLinesWritten
for the number consumed. The app must call this function again for any values that were not consumed.
If Nexus is able to send one line on a field, it will send every line for that field.

One set of NEXUS_TeletextLines will be consumed with every outgoing field or frame. Nexus
does not provide a callback when more space in available. The application should pace itself.

Every call to NEXUS_Display_WriteTeletext will begin writing lines to a new field.

If you call this, NEXUS_DisplayVbiSettings.teletextRouting should be false and NEXUS_DisplayVbiSettings.teletextEnabled should be true.
**/
NEXUS_Error NEXUS_Display_WriteTeletext(
    NEXUS_DisplayHandle handle,
    const NEXUS_TeletextLine *pLines,   /* attr{nelem=numLines;reserved=4} array of NEXUS_TeletextLine entries to output */
    size_t numLines,                    /* number of NEXUS_TeletextLine entries pointed to by pLines */
    size_t *pNumLinesWritten            /* [out] number of NEXUS_TeletextLine entries written */
    );

/**
Summary:
Write an array of closed caption data for output on the VEC

Description:
This function is non-blocking. It will queue as many values as possible. Check pNumEntriesWritten
for the number consumed. The app must call this function again for any values that were not consumed.

One NEXUS_ClosedCaptionData structure will be consumed with every outgoing field or frame. Nexus
does not provide a callback when more space in available. The application should pace itself.

If you call this, NEXUS_DisplayVbiSettings.closedCaptionRouting should be false and NEXUS_DisplayVbiSettings.closedCaptionEnabled should be true.
**/
NEXUS_Error NEXUS_Display_WriteClosedCaption(
    NEXUS_DisplayHandle handle,
    const NEXUS_ClosedCaptionData *pEntries,    /* attr{nelem=numEntries;reserved=4} array of NEXUS_ClosedCaptionData entries to output */
    size_t numEntries,                          /* number of NEXUS_ClosedCaptionData entries pointed to by pEntries */
    size_t *pNumEntriesWritten                  /* [out] number of NEXUS_ClosedCaptionData entries written */
    );

/**
Summary:
Write an array of GemStar data for output on the VEC

Description:
This function is non-blocking. It will queue as many values as possible. Check pNumEntriesWritten
for the number consumed. The app must call this function again for any values that were not consumed.

One NEXUS_GemStarData structure will be consumed with every outgoing field or frame. Nexus
does not provide a callback when more space in available. The application should pace itself.

If you call this, NEXUS_DisplayVbiSettings.gemStarEnabled should be true.
**/
NEXUS_Error NEXUS_Display_WriteGemStar(
    NEXUS_DisplayHandle handle,
    const NEXUS_GemStarData *pEntries,          /* attr{nelem=numEntries;reserved=4} array of NEXUS_GemStarData entries to output */
    size_t numEntries,                          /* number of NEXUS_GemStarData entries pointed to by pEntries */
    size_t *pNumEntriesWritten                  /* [out] number of NEXUS_GemStarData entries written */
    );

/**
Summary:
Set a new WSS value to be output on the VEC

Description:
Calling this function once will result in a persistent WSS value being encoded on all top fields or frames
coming from this display.

For each bit of WSS data, one of the following sequences is output, depending on the value of the bit:
0 = 000 111
1 = 111 000
These sequences are output one bit every WSS (5 MHz) cycle (i.e. 6 WSS cycles for every bit of WSS data).

If you call this, NEXUS_DisplayVbiSettings.wssRouting should be false and NEXUS_DisplayVbiSettings.wssEnabled should be true.
**/
NEXUS_Error NEXUS_Display_SetWss(
    NEXUS_DisplayHandle handle,
    uint16_t wssData
    );

/**
Summary:
Set a new CGMS-A value to be output on the VEC

Description:
Calling this function once will result in a persistent CGMS-A value being encoded on all top and bottom fields or frames coming from this display.

See hardware register database for data format information. Nexus simply passes the 32 bit value to hardware.

If you call this, NEXUS_DisplayVbiSettings.cgmsRouting should be false and NEXUS_DisplayVbiSettings.cgmsEnabled should be true.
**/
NEXUS_Error NEXUS_Display_SetCgms(
    NEXUS_DisplayHandle handle,
    uint32_t cgmsData
    );

/**
Summary:
Set CGMS-B data

Description:
This would typically be an array of 5 uint32_t's.

If you call this, NEXUS_DisplayVbiSettings.cgmsRouting should be false and NEXUS_DisplayVbiSettings.cgmsEnabled should be true.
**/
NEXUS_Error NEXUS_Display_SetCgmsB(
    NEXUS_DisplayHandle handle,
    const uint32_t *pCgmsData, /* attr{nelem=size} */
    unsigned size
    );

/**
Summary:
Set a new VPS value to be output on the VEC

Description:
Calling this function once will result in persistent VPS values being encoded on all top fields or frames
coming from this display.

If you call this, NEXUS_DisplayVbiSettings.vpsRouting should be false and NEXUS_DisplayVbiSettings.vpsEnabled should be true.
**/
NEXUS_Error NEXUS_Display_SetVps(
    NEXUS_DisplayHandle handle,
    const NEXUS_VpsData *pData
    );

/**
Summary:
Set a new macrovision value to be output on the VEC

Description:
Macrovision code is not included or enabled by default. If you have a Macrovision license and have
obtained the necessary source code, then compile with BVDC_MACROVISION=y set your compilation environment.

If a display format does not support macrovision, Nexus will print a warning but will not fail.
Macrovision will be re-applied after every display format change.
**/
NEXUS_Error NEXUS_Display_SetMacrovision(
    NEXUS_DisplayHandle handle,
    NEXUS_DisplayMacrovisionType type,
    const NEXUS_DisplayMacrovisionTables *pTable  /* attr{null_allowed=y} Optional macrovision tables if type == NEXUS_DisplayMacrovisionType_eCustom. */
    );

/**
Summary:
Set a new dcs value to be output on the VEC

Description:
DCS code is not included or enabled by default. If you have a DCS license and have obtained
the necessary source code, then compile with BVDC_DCS=y set in your compilation environment.
**/
NEXUS_Error NEXUS_Display_SetDcs(
    NEXUS_DisplayHandle handle,
    NEXUS_DisplayDcsType type
    );

/**
Summary:
Perform a DCS lock on VEC hardware

Description:
This function is provisional and may be subject to change.
**/
NEXUS_Error NEXUS_Display_DcsLock(
    NEXUS_DisplayHandle handle
    );
    
/**
Summary:
Write an array of AMOL for output on the VEC

Description:
This function is non-blocking. It will queue as many values as possible. Check pNumEntriesWritten
for the number consumed. The app must call this function again for any values that were not consumed.

One NEXUS_AmolData structure will be consumed with every outgoing field or frame. Nexus
does not provide a callback when more space in available. The application should pace itself.

If you call this, NEXUS_DisplayVbiSettings.amolEnabled should be true.
**/
NEXUS_Error NEXUS_Display_WriteAmol(
    NEXUS_DisplayHandle handle,
    const NEXUS_AmolData *pEntries,    /* attr{nelem=numEntries;reserved=4} array of NEXUS_AmolData entries to output */
    size_t numEntries,                 /* number of NEXUS_AmolData entries pointed to by pEntries */
    size_t *pNumEntriesWritten         /* [out] number of NEXUS_AmolData entries written */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_VBI_H__ */
