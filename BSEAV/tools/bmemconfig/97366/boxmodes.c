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
        case 0:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:HD/SD,Video:HD Main/PIP";

            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage = "vid deocder 0";
            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage = "vid decoder 1";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain = "main disp-main window";
            pSettings->display[0].usagePip = "main disp pip window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain = "sd disp main window";
            pSettings->display[1].usagePip = "sd disp pip window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used = true;
            pSettings->graphics[0].usage = "m2mc, primary display FB";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used = true;
            pSettings->graphics[1].usage = "secondary display FB";

            pSettings->frontend.used               = true;
            pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage              = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes     = 152 * 1024;

            pSettings->graphics3d.used = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage = "3D Graphics";

            rc = 0;
            break;
        }
#elif (BCHP_VER >= BCHP_VER_B0)
    case 1:
    {
        pSettings->boxModeId = boxMode;
        pSettings->boxModeDescription          = "Display:HD Video(1080p60 HEVC):HD Main/PIP + 1 Xcode";

        pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
        pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
        pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

        pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
        pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[1].usage = "vid decoder 1";
        /* HD ONLY */
        pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
        pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->display[0].pipPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->display[0].usageMain = "Main Display, Main Window";
        pSettings->display[0].usagePip = "Main Display, PIP window";

        /* this display is used for transcode */
        pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
        /* row number in the table */
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        /* usage column string */
        pSettings->display[1].usageMain        = "Encoder 0 Display";

        pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
        pSettings->graphics[0].heapIdx = NEXUS_MEMC1_GRAPHICS_HEAP;
        pSettings->graphics[0].used = true;
        pSettings->graphics[0].usage = "M2MC, primary display FB";

        /* single transcode index 2 and assume audio decoder 1 */
        pSettings->transcoders[0].videoDecoder = 2;
        pSettings->transcoders[0].audioDecoder = 1;

        pSettings->frontend.used               = true;
        pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->frontend.usage              = "Integrated Frontend";
        pSettings->frontend.usageSizeBytes     = 152 * 1024;

        pSettings->graphics3d.used = true;
        pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics3d.usage = "3D Graphics";

        rc = 0;
        break;
    }
    case 2:
    {
        pSettings->boxModeId = boxMode;
        pSettings->boxModeDescription          = "Display:HD Video(1080p60 HEVC):HD Main/ No PIP + 2 Xcode";

        pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
        pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
        pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

        /* HD ONLY */
        pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
        pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->display[0].pipPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->display[0].usageMain = "Main Display, Main Window";

        /* this display is used for transcode 0 */
        pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
        /* row number in the table */
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        /* usage column string */
        pSettings->display[1].usageMain        = "Encoder 0 Display";

        /* this display is used for transcode 1 */
        pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
        /* row number in the table */
        pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        /* usage column string */
        pSettings->display[2].usageMain        = "Encoder 1 Display";

        pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
        pSettings->graphics[0].heapIdx = NEXUS_MEMC1_GRAPHICS_HEAP;
        pSettings->graphics[0].used = true;
        pSettings->graphics[0].usage = "M2MC, Primary Display FB";

        /* Dual transcode index 2,3 and assume audio decoder 1,2 */
        pSettings->transcoders[0].videoDecoder = 1;
        pSettings->transcoders[0].audioDecoder = 1;

        pSettings->transcoders[1].videoDecoder = 2;
        pSettings->transcoders[1].audioDecoder = 2;

        pSettings->frontend.used               = true;
        pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->frontend.usage              = "Integrated Frontend";
        pSettings->frontend.usageSizeBytes     = 152 * 1024;

        pSettings->graphics3d.used = true;
        pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics3d.usage = "3D Graphics";

        rc = 0;
        break;
    }
    case 3:
    {
        pSettings->boxModeId = boxMode;
        pSettings->boxModeDescription          = "Display:HD Video(3840x2160p60 10-bit HEVC):HD/SD Main";

        pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
        pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
        pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

        /* HD + SD now but no PiP */
        pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
        pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[0].usageMain = "Main Display, Main Window";

        /* SD display  */
        pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
        pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
        pSettings->display[1].usageMain        = "Secondary Display, Main Window";
        /* Graphics 0 */
        pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
        pSettings->graphics[0].heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
        pSettings->graphics[0].used = true;
        pSettings->graphics[0].usage = "M2MC, Primary Display FB";

        /* NO Transcode */
        pSettings->frontend.used               = true;
        pSettings->frontend.heapIdx            = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->frontend.usage              = "Integrated Frontend";
        pSettings->frontend.usageSizeBytes     = 152 * 1024;

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
