/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
 ***************************************************************************/
#include "b_playback.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
Sample app that uses bcmplayer to read and play an MPEG2 TS file w/ trick codes.
Could be the basis for a server-based stream player.
**/

int main(int argc, char **argv)
{
    b_playback_t p;
    struct b_playback_start_settings start_settings;
    struct b_playback_trick_mode trick_mode;
    FILE *outputfile;
    int rc;
    int i;

    if (argc < 5) {
        printf("Usage: b_playback_capture inputfile.ts inputfile.nav 0x11 outputfile.ts\n");
        return -1;
    }

    BKNI_Init();
    BDBG_Init();

    p = b_playback_create(NULL);
    BDBG_ASSERT(p);

    b_playback_get_default_start_settings(&start_settings);
    start_settings.datafile = argv[1];
    start_settings.indexfile = argv[2];
    start_settings.video_pid = strtoul(argv[3], NULL, 0);
    rc = b_playback_start(p, &start_settings);
    BDBG_ASSERT(!rc);

    outputfile = fopen(argv[4], "w");
    BDBG_ASSERT(outputfile);

    /* first pass w/ normal play */
    for (i=0;i<1000;i++) {
        void *buffer;
        unsigned size;
        rc = b_playback_get_buffer(p, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (size == 0) break;
        fwrite(buffer, size, 1, outputfile);
        rc = b_playback_read_complete(p, size);
        BDBG_ASSERT(!rc);
    }

    rc = b_playback_seek(p, 0, SEEK_SET);
    BDBG_ASSERT(!rc);

    b_playback_get_default_trick_mode(&trick_mode);
    trick_mode.rate = B_PLAYBACK_NORMAL_RATE * 10;
    rc = b_playback_trick_mode(p, &trick_mode);
    BDBG_ASSERT(!rc);

    /* second pass as trick mode */
    for (i=0;i<1000;i++) {
        void *buffer;
        unsigned size;
        rc = b_playback_get_buffer(p, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (size == 0) break;
        fwrite(buffer, size, 1, outputfile);
        rc = b_playback_read_complete(p, size);
        BDBG_ASSERT(!rc);
    }

    fclose(outputfile);
    b_playback_stop(p);
    b_playback_destroy(p);

    /* you can now decode the file. be sure to decode in vsync mode because of the embedded trick mode. */

    return 0;
}
