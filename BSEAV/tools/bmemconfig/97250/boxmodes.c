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
        case 1:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD,Video(Dual 1080p30/60i 10-bit HEVC),Main/PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "video decoder 1";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx   = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[0].usagePip                  = "main disp, pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 2:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD+SD,Video(1080p30/60i 8-bit HEVC),Main/no PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[1].property                  = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                 = "sec disp, main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, main display FB";
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "m2mc, sec display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 3:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD,Video(1080p60 8-bit HEVC),Main/no PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, main display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 4:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD,Video(Dual 1080p60 8-bit HEVC),Main/no PIP,SD/576p xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "video decoder 1";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[1].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                 = "encoder 0 disp";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, main display FB";
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "m2mc, encode display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 5:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Headless,Video(1080p60 10-bit HEVC),720p30 xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "encoder 0 disp";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, encode display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 6:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD+SD,Video(1080p30/60i 10-bit HEVC),Main/no PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[1].property                  = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                 = "sec disp, main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, main display FB";
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "m2mc, sec display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 7:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD,Video(1080p30/60i 8-bit HEVC,480p30/60i 8-bit HEVC),Main/PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "video decoder 1";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx   = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[0].usagePip                  = "main disp, pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
        case 8:
        {
            pSettings->boxModeId            = boxMode;
            pSettings->boxModeDescription   = "Display:UHD+SD,Video(Multi-PIP decode 1080p30/60i 8-bit HEVC),Main/PIP,No xcode";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "video decoder 0";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "main disp, main window";
            pSettings->display[1].property                  = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                 = "sec disp, main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, main display FB";
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "m2mc, sec display FB";

            pSettings->frontend.used        = false;

            pSettings->graphics3d.used      = true;
            pSettings->graphics3d.heapIdx   = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage     = "3D Graphics";

            rc = 0;
            break;
        }
    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
