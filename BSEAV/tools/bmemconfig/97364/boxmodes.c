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
#if (BCHP_VER == BCHP_VER_A0)
    case 1:
    {
        /*
            1x 16-bit DDR3-2133
            1080p30/60i 8-bit AVC / 10-bit HEVC main decode
            1080p30/60i 8-bit AVC / 10-bit HEVC PIP decode
            MAIN + PIP to UHD display (through up-scaling)
            No SD display
            No transcode
         */
        pSettings->boxModeId = boxMode;
        pSettings->boxModeDescription          = "Display:HD Video(1080p30/60i 8-bit HEVC):HD Main/PIP + No Xcode";

        pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
        pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
        pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

        pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
        pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[1].usage = "vid decoder 1";
        /* HD ONLY  NO SD*/
        pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
        pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[0].pipPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[0].usageMain = "Main Display, Main Window";
        pSettings->display[0].usagePip = "Main Display, PIP window";

        /* No Transcode
        pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[1].usageMain        = "Encoder 0 Display";
        */
        pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
        pSettings->graphics[0].heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics[0].used = true;
        pSettings->graphics[0].usage = "M2MC, primary display FB";
        /* No Transcode
        pSettings->transcoders[0].videoDecoder = 2;
        pSettings->transcoders[0].audioDecoder = 1;
        */
        pSettings->frontend.used               = true;
        pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->frontend.usage              = "Integrated Frontend";
        pSettings->frontend.usageSizeBytes     = 152 * 1024; /* need to double check with FE team */

        pSettings->graphics3d.used = true;
        pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics3d.usage = "3D Graphics";

        rc = 0;
        break;
    }
    case 2:
    {
        /*
           1x 16-bit DDR3-1866 - Allows high-temperature refresh
           1080p30/60i 8-bit HEVC or 1080p30/60i 8-bit AVC decode
           MAIN only (no PIP)
           UHD + SD display
           No transcode
         */
        pSettings->boxModeId = boxMode;
        pSettings->boxModeDescription          = "Display:HD Video(1080p30/60i HEVC 8-bit):HD Main/ No PIP + SD + No Xcode";

        pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
        pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
        pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

        /* HD + SD */
        pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
        pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;

        pSettings->display[0].usageMain = "Main Display, Main Window";

        pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;

        pSettings->display[0].usageMain = "Secondary Display, Main Window";

        /* No Transcode

        pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[1].usageMain        = "Encoder 0 Display";

        pSettings->transcoders[0].videoDecoder = 1;
        pSettings->transcoders[0].audioDecoder = 1;

        pSettings->transcoders[1].videoDecoder = 2;
        pSettings->transcoders[1].audioDecoder = 2;

        */

        pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
        pSettings->graphics[0].heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics[0].used = true;
        pSettings->graphics[0].usage = "M2MC, Primary Display FB";


        pSettings->frontend.used               = true;
        pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->frontend.usage              = "Integrated Frontend";
        pSettings->frontend.usageSizeBytes     = 152 * 1024; /* To be confirmed by FE team */

        pSettings->graphics3d.used = true;
        pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics3d.usage = "3D Graphics";

        rc = 0;
        break;
    }
#endif
    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
