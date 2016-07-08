/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
******************************************************************************/
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
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD; Video:HD Main/No PIP";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 2:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/PIP; Xcode:Dual 1080p30->720p30";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->videoDecoder[3].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                = "Video Decoder 3";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->graphics[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used     = true;
            pSettings->graphics[2].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage    = "Encode 0 Graphics";

            pSettings->graphics[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used     = true;
            pSettings->graphics[3].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[3].usage    = "Encode 1 Graphics";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 3:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/PIP; Xcode:none";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 4:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/No PIP; Xcode:Dual 1080p50-10bitHevc->1080p25-8bitAvc";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";

            pSettings->display[1].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 5:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/PIP; Xcode:Single 1080p30 8-bit";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->graphics[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used     = true;
            pSettings->graphics[2].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage    = "Encode 0 Graphics";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 6:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/PIP; Xcode:None";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 7:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/PIP; Xcode:Dual 1080p25->720p25 8AVC";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->videoDecoder[3].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                = "Video Decoder 3";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 8:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/PIP; Xcode:Dual 1080p30->720p30";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->videoDecoder[3].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                = "Video Decoder 3";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 9:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/No PIP; Xcode:Dual 1080p50-10bitHevc->1080p25-8bitAvc";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain = "Encoder 0 Display";

            pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 10:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/3+1 1080p30 PIP; Xcode:None; DDR4-2400";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 12:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/no SD,Video:UHD Main/1080p25 PIP; Xcode:None; DDR3-2133";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 13:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/no PIP; Xcode:None; DDR3-2133/DDR4-2400";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 14:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/3+1 1080p30 PIP; Xcode:None; DDR3-1866";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 16:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/no PIP; Xcode:One 1080p60 10-bit->1080p30 8-bit";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            /* this display is used for transcode */
            pSettings->display[1].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[1].usageMain                = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 17:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:HD/SD,Video:HD Main/PIP; Xcode:One 1080p30->720p60";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain                = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            rc = 0;
            break;
        }
        case 18:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Headless; Xcode:Dual 4Kp30->1080p30";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* this display is used for transcode */
            pSettings->display[0].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[0].usageMain = "Encoder 0 Display";

            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage    = "Encode 0 Graphics";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].usage    = "Encode 1 Graphics";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 0;
            pSettings->transcoders[0].audioDecoder = 0;

            pSettings->transcoders[1].videoDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 19:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/no PIP; Xcode:One 1080p25->720p25";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            /* this display is used for transcode */
            pSettings->display[1].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[1].usageMain                = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Encoder 0 Graphics";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 20:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/3+1 1080p30 PIP; Xcode:None;";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "Primary Display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "Secondary Display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            rc = 0;
            break;
        }
        case 21:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/PIP; Xcode:One 1080p30->720p60; DDR4-2380";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "sd disp main window";
            pSettings->display[1].usagePip                 = "sd disp pip window";

            /* this display is used for transcode */
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain                = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 22:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Headless; Xcode:Dual 1080p30->1080p30";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            /* this display is used for transcode */
            pSettings->display[0].property                  = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[0].usageMain                 = "Encoder 0 Display";

            pSettings->display[2].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                 = "Encoder 1 Display";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 0;
            pSettings->transcoders[0].audioDecoder = 0;

            pSettings->transcoders[1].videoDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 23:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Display:UHD/SD,Video:UHD Main/no PIP; Xcode:One 1080p60 10-bit->1080p30 8-bit; DDR4-2400";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "main disp main window";
            pSettings->display[0].usagePip                 = "main disp pip window";

            /* this display is used for transcode */
            pSettings->display[1].property                 = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[1].usageMain                = "Encoder 0 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].usage    = "m2mc, primary display FB";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].usage    = "secondary display FB";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            rc = 0;
            break;
        }
        case 24:
        {
            pSettings->boxModeId          = boxMode;
            pSettings->boxModeDescription = "Headless; Xcode:Dual 1080p30->1080p30 w/ gfx; DDR3-2133";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eGraphicsPip;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->videoDecoder[3].property             = Memconfig_VideoDecoderProperty_eGraphicsPip;
            pSettings->videoDecoder[3].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                = "Video Decoder 3";

            /* this display is used for transcode */
            pSettings->display[0].property = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[0].usageMain = "Encoder 0 Display";

            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                = "Encoder 1 Display";

            pSettings->graphics[0].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage    = "Encode 0 Graphics";

            pSettings->graphics[1].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].usage    = "Encode 1 Graphics";

            pSettings->graphics[2].property = Memconfig_DisplayProperty_eGraphicsPip;
            pSettings->graphics[2].used     = true;
            pSettings->graphics[2].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage    = "Graphics PIP 0";

            pSettings->graphics[3].property = Memconfig_DisplayProperty_eGraphicsPip;
            pSettings->graphics[3].used     = true;
            pSettings->graphics[3].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[3].usage    = "Graphics PIP 1";

            pSettings->frontend.used           = true;
            pSettings->frontend.heapIdx        = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->frontend.usage          = "Integrated Frontend";
            pSettings->frontend.usageSizeBytes = 152 * 1024;

            pSettings->graphics3d.used    = true;
            pSettings->graphics3d.heapIdx = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics3d.usage   = "3D Graphics";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 0;
            pSettings->transcoders[0].audioDecoder = 0;

            pSettings->transcoders[1].videoDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 1;

            rc = 0;
            break;
        }
    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
