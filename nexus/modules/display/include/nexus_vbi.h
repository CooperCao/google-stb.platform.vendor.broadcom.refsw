/***************************************************************************
 *     (c)2008-2011 Broadcom Corporation
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
#ifndef NEXUS_VBI_H__
#define NEXUS_VBI_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=****************************************
This file contains common data structures for VBI input and output.

The VBI data types that are currently supported in Nexus are:
- Closed Captioning
- Teletext
- WSS (Widescreen Signalling)
- CGMS (Copy generation management system - analog)
- VPS (Video Program System)
- GemStar

Other types will be added as needed.

VBI data can be read from VideoInput or written to a Display for output on a VEC.
See nexus_video_input_vbi.h and nexus_display_vbi.h.

See nexus_types.h for NEXUS_ClosedCaptionData.

******************************************/

/**
Summary:
Line size used in NEXUS_TeletextLine
**/
#define NEXUS_TELETEXT_LINESIZE 42

/**
Summary:
One line of teletext data

Description:
See NEXUS_VideoInput_ReadTeletext and NEXUS_Display_WriteTeletext
**/
typedef struct NEXUS_TeletextLine
{
    bool topField;          /* If true, top field. Otherwise, bottom field. */
    unsigned lineNumber;    /* lineNumber is only used to specify field breaks. It is not used for exact VBI line placement.
                               If you send an array of 10 NEXUS_TeletextLine entries with lineNumbers of 0,1,2,3,4,0,1,2,3,4, then
                               the first 0..4 series will be sent on one field, the second 0..4 series will be sent on the next.
                               This allows applications to send large blocks of TT data in one call. It also allows Nexus to remap
                               line numbers to avoid unencodable VBI lines or VBI line conflicts with other standards like VPS, WSS or CC. */
    uint8_t framingCode;
    uint8_t data[NEXUS_TELETEXT_LINESIZE];
} NEXUS_TeletextLine;

/**
Summary:
One field of VPS data

Description:
See NEXUS_VideoInput_ReadVps and NEXUS_Display_WriteVps
**/
typedef struct NEXUS_VpsData
{
    uint8_t byte05;
    uint8_t byte11;
    uint8_t byte12;
    uint8_t byte13;
    uint8_t byte14;
    uint8_t byte15;
} NEXUS_VpsData;

/**
Summary:
Array size used in NEXUS_GemStarData
**/
#define NEXUS_GEMSTAR_MAX_LINES 5

/**
Summary:
One entry of GemStar data

Description:
See NEXUS_VideoInput_ReadGemStar and NEXUS_Display_WriteGemStar.
See NEXUS_VideoInputVbiSettings.gemStar for decoding options for gemstar data.
See NEXUS_DisplayVbiSettings.gemStar for encoding options for gemstar data.
**/
typedef struct NEXUS_GemStarData
{
    bool topField;     /* If true, top field. Otherwise, bottom field. */

    uint32_t lineMask; /* A bitmask indicating the significance of the data[] array.
                          If the ith bit in ulDataLines is one, then video line i of the field will be encoded with gemstar data.
                          The gemstar data comes from data[] where data[0] is the first piece of data to be encoded, data[1] is the second piece of data to be encoded, etc.
                          Example: If lineMask is 1000000001010b, then data[0] going out on line 1, data[1] on line 3 and data[2] on line 12. */

    uint32_t data[NEXUS_GEMSTAR_MAX_LINES]; /* a packed array where each member corresponds to lineMask */

    uint32_t errorMask; /* Parity errors in the captured data[] lines. The Nth bit in errorMask corresponds to the Nth entry in data[].
                           There is no relationship between lineMask and errorMask.
                           Example If errorMask is 01010b, then data[1] has an error and data[3] has an error.
                           errorMask can never be greater than NEXUS_GEMSTAR_MAX_LINES.*/
 } NEXUS_GemStarData;

/**
Summary:
Automated Measurement of Lineup (AMOL) for Nielsen Media Research
**/
typedef enum NEXUS_AmolType
{
    NEXUS_AmolType_eI,           /* AMOL I             */
    NEXUS_AmolType_eII_Lowrate,  /* AMOL II, 1 Mbit/s. */
    NEXUS_AmolType_eII_Highrate, /* AMOL II, 2 Mbit/s. */
    NEXUS_AmolType_eMax
} NEXUS_AmolType;

/**
Summary:
AMOL data for one video field
**/
typedef struct NEXUS_AmolData
{
    bool topField;     /* If true, top field. Otherwise, bottom field. */
#define NEXUS_MAX_AMOL_DATA 24
    uint8_t data[NEXUS_MAX_AMOL_DATA]; /* typically 6 bytes for type I, 12 bytes for II_Lowrate and 24 bytes for II_Highrate */
    unsigned length; /* number of bytes in data[] */
} NEXUS_AmolData;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VBI_H__ */
