/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/

/* For debugging */
/* #define BUDP_P_GETUD_DUMP 1 */
#ifdef BUDP_P_GETUD_DUMP
static const char* BUDP_P_Getud_Filename = "userdata.getud";
#include <stdio.h>
#endif

#include "bstd.h"
#include "bdbg.h"
#include "budp.h"
#include "budp_jp3dparse.h"

BDBG_MODULE(BUDP);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static size_t FindJp3dStart_isr (
    const uint8_t* userdata, size_t length);

#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info, size_t offset);
#endif

/***************************************************************************
* Static data (tables, etc.)
***************************************************************************/


/***************************************************************************
* Implementation of "BUDP_JP3Dparse_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BUDP_JP3Dstart_isr (
    const BAVC_USERDATA_info* pUserdata_info,
    size_t         offset,
    size_t*  pBytesParsed
)
{
    size_t bytesParsedSub;
    uint32_t length;
    uint8_t* userdata;

    BDBG_ENTER(BUDP_JP3Dstart_isr);

    /* Check for obvious errors from user */
    if ((pUserdata_info == 0x0) ||
        (pBytesParsed   == 0x0)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Programming note:  all function parameters are now validated */

#ifdef BUDP_P_GETUD_DUMP
    dump_getud_isr (pUserdata_info, offset);
#endif

    /* Take care of a special case */
    userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    length   = pUserdata_info->ui32UserDataBufSize - offset;
    if (length < 8)
    {
        *pBytesParsed = length;
        return BERR_BUDP_NO_DATA;
    }

    /* jump past the first SEI userdata start code */
    bytesParsedSub = FindJp3dStart_isr (userdata, length);
    *pBytesParsed = bytesParsedSub;
    length -= bytesParsedSub;
    /* userdata += bytesParsedSub; */

    /* If we did not find a start code, bail out now */
    if (length == 0)
    {
        return BERR_BUDP_NO_DATA;
    }

    BDBG_LEAVE(BUDP_JP3Dstart_isr);
    return BERR_SUCCESS;
}


BERR_Code BUDP_JP3Dparse_isr (
    const BAVC_USERDATA_info* pUserdata_info,
    size_t         offset,
    size_t*  pBytesParsed,
    uint16_t*     sigtype
)
{
    uint32_t length;
    uint8_t* userdata;
    uint8_t val8;
    uint16_t val16;

    BDBG_ENTER(BUDP_JP3Dparse_isr);

    /* Check for obvious errors from user */
    if ((pUserdata_info == 0x0) ||
        (pBytesParsed   == 0x0) ||
        (sigtype        == 0x0)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Programming note:  all function parameters are now validated */

    /* Take care of a special case */
    userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    length   = pUserdata_info->ui32UserDataBufSize - offset;
    if (length < 4)
    {
        *pBytesParsed = length;
        return BERR_BUDP_NO_DATA;
    }

    /* Stereo_Video_Format_Signaling_Length */
    if ((*userdata++) != 3)
    {
        *pBytesParsed = 1;
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Reserved (bit) and Stereo_Video_Format_Signaling_Type */
    val8 = *userdata++;
    if ((val8 & 0x80) == 0)
    {
        *pBytesParsed = 2;
        return BERR_BUDP_PARSE_ERROR;
    }
    val8 &= 0x7F;
    switch (val8)
    {
    case 0x03:
    case 0x04:
    case 0x08:
        break;
    default:
        *pBytesParsed = 2;
        return BERR_BUDP_PARSE_ERROR;
        break;
    }
    /* The heart of the matter */
    *sigtype = val8;

    /* Reserved (2 bytes) */
    val16 = (*userdata++) << 8;
    val16 |= *userdata++;
    *pBytesParsed = 4;
    if (val16 != 0x04FF)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    BDBG_LEAVE(BUDP_JP3Dparse_isr);
    return BERR_SUCCESS;
}


static size_t FindJp3dStart_isr (
    const uint8_t* userdata, size_t length)
{
    uint8_t saved[8];
    size_t count = 0;

    /* Special case (failure) */
    if (length < 8)
        return length;

    /* Initialize */
    saved[1] = *userdata++;
    saved[2] = *userdata++;
    saved[3] = *userdata++;
    saved[4] = *userdata++;
    saved[5] = *userdata++;
    saved[6] = *userdata++;
    saved[7] = *userdata++;

    while (length >= 8)
    {
        /* Read in another byte */
        saved[0] = saved[1];
        saved[1] = saved[2];
        saved[2] = saved[3];
        saved[3] = saved[4];
        saved[4] = saved[5];
        saved[5] = saved[6];
        saved[6] = saved[7];
        saved[7] = *userdata++;

        if ((saved[0] == 0x00) &&
            (saved[1] == 0x00) &&
            (saved[2] == 0x01) &&
            (saved[3] == 0xb2) &&
            (saved[4] == 0x4a) &&
            (saved[5] == 0x50) &&
            (saved[6] == 0x33) &&
            (saved[7] == 0x44)   )
        {
            /* Found it! */
            break;
        }

        /* proceed to the next byte */
        --length;
        ++count;
    }

    if (length >= 8)
    {
        /* found the pattern before the end of stream */
        return count + 8;
    }
    else
    {
        /* Didn't find any start code */
        return count + 7;
    }
}
