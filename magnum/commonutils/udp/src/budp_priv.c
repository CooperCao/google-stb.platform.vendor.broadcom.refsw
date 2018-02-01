/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "budp.h"
#include "budp_bitread.h"
#include "budp_priv.h"

BDBG_MODULE(BUDP);

/***************************************************************************
* Implementation of private (static) functions
***************************************************************************/
#if (BUDP_P_GETUD_DUMP)
#include <stdio.h>
#include <stdlib.h>

void BUDP_P_dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info,
    size_t                    offset,
    const char*               pchGetUdFilename )
{
    unsigned int iByte;
    static FILE* fd = NULL;
    static unsigned int nPicture;
    uint8_t* userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    uint32_t length   = pUserdata_info->ui32UserDataBufSize - offset;

    /* Initialization */
    if (fd == NULL)
    {
        if ((fd = fopen (pchGetUdFilename, "w")) == 0)
        {
            fprintf (stderr, "ERROR: could not open %s for debug output\n",
                pchGetUdFilename);
            return;
        }
        fprintf (fd, "getud output format version 1\n");
        nPicture = 0;
    }

    fprintf (fd, "\nPic %u LOC %06lx TR %u\n", ++nPicture, 0UL, 0U);
    fprintf (fd, "PS %d TFF %d RFF %d\n",
        pUserdata_info->eSourcePolarity,
        pUserdata_info->bTopFieldFirst,
        pUserdata_info->bRepeatFirstField);
    fprintf (fd, "UDBYTES %u\n", length);
    for (iByte = 0 ; iByte < length ; ++iByte)
    {
        fprintf (fd, "%02x", userdata[iByte]);
        if ((iByte % 16) == 15)
            putc ('\n', fd);
        else
            putc (' ', fd);
    }
    if ((iByte % 16) != 15)
        putc ('\n', fd);
}
#endif

/***************************************************************************
 * This function finds the next userdata startcode 0x000001B2.  It
 * indicates the byte following this startcode by its return value.
 * If no startcode was found, it simply returns the length of the
 * input data.
 */
size_t BUDP_P_FindMpegUserdataStart_isr (
    BUDP_Bitread_Context* pReader, size_t length)
{
    size_t count = 0;
    uint8_t saved[4];

    /* Special case (failure) */
    if (length < 4)
        return length;

    /* Initialize */
    saved[1] = BUDP_Bitread_Byte_isr (pReader);
    saved[2] = BUDP_Bitread_Byte_isr (pReader);
    saved[3] = BUDP_Bitread_Byte_isr (pReader);

    while (length >= 4)
    {
        /* Read in another byte */
        saved[0] = saved[1];
        saved[1] = saved[2];
        saved[2] = saved[3];
        saved[3] = BUDP_Bitread_Byte_isr (pReader);

        if ((saved[0] == 0x00) &&
            (saved[1] == 0x00) &&
            (saved[2] == 0x01) &&
            (saved[3] == 0xB2)    )
        {
            /* Found it! */
            break;
        }

        /* proceed to the next byte */
        --length;
        ++count;
    }

    if (length >= 4)
    {
        /* found the pattern before the end of stream */
        return count + 4;
    }
    else
    {
        /* Didn't find any start code */
        return count + 3;
    }
}

/* End of file */
