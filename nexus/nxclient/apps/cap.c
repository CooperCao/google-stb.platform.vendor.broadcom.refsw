/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_display.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_transport_capabilities.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(cap);

/*
print system capabilities for use in scripts
all output is a single integer, no newlines
for booleans, 1 is true, 0 is false
*/

static void print_usage(void)
{
    printf(
    "Usage: cap OPTIONS\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -video_windows             # of video windows on main display\n"
    "  -video_encoders            # of video encoders\n"
    "  -video_decoders            # of video decoders\n"
    "  -rec_pumps                 # of record pumps\n"
    );
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned result = 0;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-video_windows")) {
            NEXUS_DisplayCapabilities displayCap;

            NEXUS_GetDisplayCapabilities(&displayCap);
            result = displayCap.display[0].numVideoWindows;
            break;
        }
        else if (!strcmp(argv[curarg], "-video_encoders")) {
#if NEXUS_HAS_VIDEO_ENCODER
            NEXUS_VideoEncoderCapabilities videoEncoderCap;
            unsigned i;

            NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);
            for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
                if (videoEncoderCap.videoEncoder[i].supported) result++;
            }
#endif
            break;
        }
        else if (!strcmp(argv[curarg], "-video_decoders")) {
#if NEXUS_HAS_VIDEO_DECODER
            NEXUS_VideoDecoderCapabilities videoDecoderCap;

            NEXUS_GetVideoDecoderCapabilities(&videoDecoderCap);
            result=videoDecoderCap.numVideoDecoders;
#endif
            break;
        }
        else if (!strcmp(argv[curarg], "-rec_pumps")) {
#if NEXUS_HAS_TRANSPORT
            NEXUS_TransportCapabilities transportCap;

            NEXUS_GetTransportCapabilities(&transportCap);
            result=transportCap.numRecpumps;
#endif
            break;
        }
        curarg++;
    }

    if (curarg == argc) {
        print_usage();
    }
    else {
        /* don't print newline */
        printf("%u", result);
    }

    NxClient_Uninit();
    return 0;
}
