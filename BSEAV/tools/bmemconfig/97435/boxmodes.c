/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform_features.h"
#include "../boxmodes_defines.h"

/*
#define NEXUS_SAGE_SECURE_HEAP          0  optional secure heaps, can be left undefined if not used
#define NEXUS_MEMC0_MAIN_HEAP           1
#define NEXUS_MEMC1_GRAPHICS_HEAP       2
#define NEXUS_MEMC0_GRAPHICS_HEAP       3
#define NEXUS_VIDEO_SECURE_HEAP         4
#define NEXUS_MEMC1_MAIN_HEAP           5
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 6
#define NEXUS_MEMC1_PICTURE_BUFFER_HEAP 7
#define NEXUS_MEMC0_DRIVER_HEAP         8

*/

int Memconfig_GetBoxModeDefaultSettings(
    int                boxMode,
    Memconfig_BoxMode *pSettings
    )
{
    int rc = 0;

    BKNI_Memset( pSettings, 0, sizeof( Memconfig_BoxMode ));

    switch (boxMode) {
        default:
        {
            rc = -1;
            break;
        }
        case 1:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:HD/SD, Video:HD Main/No PIP,Transcode:Quad 1080p60->720p30(Max)";


            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";

            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->videoDecoder[4].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[4].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[4].usage                         = "Video Decoder 4";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Secondary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            pSettings->display[4].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[4].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[4].usageMain        = "Encoder 2 Display";

            pSettings->display[5].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[5].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[5].usageMain        = "Encoder 3 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB:";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = true;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = true;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Miracast FB, M2MC";

            pSettings->graphics[4].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[4].used            = true;
            pSettings->graphics[4].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[4].usage           = "Miracast FB, M2MC";

            pSettings->graphics[5].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[5].used            = true;
            pSettings->graphics[5].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[5].usage           = "Miracast FB, M2MC";


            pSettings->transcoders[0].videoDecoder = 1; /* index of the videoDecoder */
            pSettings->transcoders[1].videoDecoder = 2; /* = x where    pSettings->videoDecoder[x].property    = Memconfig_VideoDecoderProperty_eTranscode; */
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[3].videoDecoder = 4;
            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 4;

            rc = 0;
            break;
        }
        case 2:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:HD/SD, Video:HD Main/No PIP,Transcode:Single 1080p60->720p30(Max)";


            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";

            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Secondary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Encoder 0 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB:";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = true;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";


            pSettings->transcoders[0].videoDecoder = 1; /* index of the videoDecoder */
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
