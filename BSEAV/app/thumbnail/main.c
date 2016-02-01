/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_surface.h"

#include "thumbnail.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(thumbnail);

NEXUS_VideoCodec getVideoCodec(const char *cur)
{
    if (!strcasecmp(cur, "avc") || !strcasecmp(cur, "h264")) {
        return NEXUS_VideoCodec_eH264;
    }
    else if (!strcasecmp(cur, "vc1sm")) {
        return NEXUS_VideoCodec_eVc1SimpleMain;
    }
    else if (!strcasecmp(cur, "vc1")) {
        return NEXUS_VideoCodec_eVc1;
    }
    else if (!strcasecmp(cur, "divx")) {
        return NEXUS_VideoCodec_eMpeg4Part2;
    }
    else if (!strcasecmp(cur, "divx311")) {
        return NEXUS_VideoCodec_eDivx311;
    }
    else if (!strcasecmp(cur, "mpeg")) {
        return NEXUS_VideoCodec_eMpeg2;
    }
    else {
        BDBG_ERR(("Unknown video codec: %s", cur));
        return NEXUS_VideoCodec_eMpeg2;
    }
}

NEXUS_TransportType getTransportType(const char *cur, const char *datafilename)
{
    if (strcasecmp(cur, "auto")) {
        if (!strcasecmp(cur, "asf")) {
            return NEXUS_TransportType_eAsf;
        }
        else if (!strcasecmp(cur, "avi")) {
            return NEXUS_TransportType_eAvi;
        }
        else if (!strcasecmp(cur, "mp4")) {
            return NEXUS_TransportType_eMp4;
        }
        else if (!strcasecmp(cur, "mkv")) {
            return NEXUS_TransportType_eMkv;
        }
        else if (!strcasecmp(cur, "pes")) {
            return NEXUS_TransportType_eMpeg2Pes;
        }
        else if (!strcasecmp(cur, "es")) {
            return NEXUS_TransportType_eEs;
        }
        else if (!strcasecmp(cur, "ts") || !strcasecmp(cur, "tts")) {
            return NEXUS_TransportType_eTs;
        }
        BDBG_ERR(("Unknown transport type: %s", cur));
        return NEXUS_TransportType_eTs;
    }
    else {
        if (strstr(datafilename, ".asf") || strstr(datafilename, ".wmv")) {
            return NEXUS_TransportType_eAsf;
        }
        else if (strstr(datafilename, ".avi") || strstr(datafilename, ".divx")) {
            return NEXUS_TransportType_eAvi;
        }
        else if (strstr(datafilename, ".mkv")) {
            return NEXUS_TransportType_eMkv;
        }
        else if (strstr(datafilename, ".mp4")) {
            return NEXUS_TransportType_eMp4;
        }
        return NEXUS_TransportType_eTs;
    }
}

NEXUS_TransportTimestampType getTimestampType(const char *cur)
{
    return !strcasecmp(cur, "tts")?NEXUS_TransportTimestampType_e32_Binary:NEXUS_TransportTimestampType_eNone;
}

int main(int argc, char **argv)
{
    if (argc < 6) {
        printf(
        "\n"
        "Usage: nexus thumbnail DATAFILE INDEXFILE VIDEOCODEC PID STREAMTYPE SPACING\n"
        "  DATAFILE = stream data\n"
        "  INDEXFILE = stream index (use ""same"" if the DATAFILE contains the index, ""none"" if no index)\n"
        "  VIDEOCODEC = mpeg,avc/h264,vc1,vc1sm,divx,divx311\n"
        "  PID = transport pid or ASF/AVI stream index\n"
        "  STREAMTYPE = ts,asf,avi,mkv,mp4,tts\n"
        "  SPACING = # in seconds (default 5)\n"
        "\n"
        );
        printf(
        "Samples:\n"
        "  nexus thumbnail /mnt/hd/mystream.mpg /mnt/hd/mystream.nav avc 0x1000 ts 30\n"
        "  nexus thumbnail /mnt/hd/mystream.divx same divx 1 auto 10\n"
        );
        exit(1);
    }

    g_data.display_format = NEXUS_VideoFormat_e720p;
    g_data.datafilename = argv[1];
    g_data.indexfilename = argv[2];
    if (!strcasecmp(g_data.indexfilename, "same")) {
        g_data.indexfilename = g_data.datafilename;
    }
    else if (!strcasecmp(g_data.indexfilename, "none")) {
        g_data.indexfilename = NULL;
    }
    g_data.videoCodec = getVideoCodec(argv[3]);
    g_data.pid = strtoul(argv[4], NULL, 0);
    g_data.transportType = getTransportType(argv[5], g_data.datafilename);
    g_data.timestampType = getTimestampType(argv[5]);
    if (argc > 6) {
        g_data.spacing = strtoul(argv[6], NULL, 0);
    }
    if (g_data.spacing == 0) g_data.spacing = 5;

    thumbnail_demo_init();

    thumbnail_demo_run();

    thumbnail_demo_uninit();

    return 0;
}


