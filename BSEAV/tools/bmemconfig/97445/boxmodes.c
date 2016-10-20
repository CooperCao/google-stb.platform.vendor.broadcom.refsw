/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/

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
/************* 97445 Box modes *****************/
        case 1:
        {
            pSettings->boxModeId = boxMode;
            /*brief description on the box mode, this stirng is shown in the UI*/
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080p60->720p30(Max)";

            /* video decoder to window mapping */
            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string shows up in the usage column*/
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";
            /* not very common */
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            /* video decoder to window mapping */
            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            /* property of this display, primary display  */
            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            /*source of picture buffer heap */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            /* property of this display, sd display  */
            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            /* picture buffer heap , this is row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            /* this display is used for transcode */
            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            /* this is application specific memory */
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            /* show this in the graphics page */
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB: ";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[3].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Miracast FB, M2MC";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 3:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Triple up to 1080p30(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";
            pSettings->videoDecoder[1].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].secondaryUsage                = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";
            pSettings->videoDecoder[2].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].secondaryUsage                = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";
            pSettings->videoDecoder[3].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].secondaryUsage                = "Video Decoder 3";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            pSettings->display[4].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[4].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[4].usageMain        = "Encoder 2 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "No Graphics";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "No Graphics";

            pSettings->graphics[4].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[4].used            = false;
            pSettings->graphics[4].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[4].usage           = "No Graphics";


            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[2].audioDecoder = 3;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC1_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 1000:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Quad up to 1080p30(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->videoDecoder[4].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[4].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[4].usage                         = "Video Decoder 4";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Transcode 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Transcode 1 Display";

            pSettings->display[4].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[4].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[4].usageMain        = "Transcode 2 Display";

            pSettings->display[5].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[5].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[5].usageMain        = "Transcode 3 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC,Secondary Display FB";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = false;
            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = false;
            pSettings->graphics[4].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[4].used            = false;
            pSettings->graphics[5].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[5].used            = false;

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[3].videoDecoder = 4;

            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[2].audioDecoder = 3;
            pSettings->transcoders[3].audioDecoder = 4;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC1_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 7:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Headless: No local Display,Transcode:Upto six 1080p60 to 720p30(Max)***";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->videoDecoder[4].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[4].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[4].usage                         = "Video Decoder 4";

            pSettings->videoDecoder[5].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[5].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[5].usage                         = "Video Decoder 5";

            pSettings->display[0].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Transcode 0 Display";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Transcode 1 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Transcode 2 Display";

            pSettings->display[4].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[4].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[4].usageMain        = "Transcode 3 Display";

            pSettings->display[5].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[5].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[5].usageMain        = "Transcode 4 Display";

            pSettings->display[6].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[6].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[6].usageMain        = "Transcode 5 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "Trancode 0 FB";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = true;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC0_MAIN_HEAP;
            pSettings->graphics[2].usage           = "Trancode 1 FB";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = true;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Trancode 2 FB";

            pSettings->graphics[4].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[4].used            = true;
            pSettings->graphics[4].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[4].usage           = "Trancode 3 FB";

            pSettings->graphics[5].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[5].used            = true;
            pSettings->graphics[5].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[5].usage           = "Trancode 4 FB";

            pSettings->graphics[6].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[6].used            = true;
            pSettings->graphics[6].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[6].usage           = "Trancode 5 FB";

            pSettings->transcoders[0].videoDecoder = 0;
            pSettings->transcoders[1].videoDecoder = 1;
            pSettings->transcoders[2].videoDecoder = 2;
            pSettings->transcoders[3].videoDecoder = 3;
            pSettings->transcoders[4].videoDecoder = 4;
            pSettings->transcoders[5].videoDecoder = 5;
            pSettings->transcoders[0].audioDecoder = 0;
            pSettings->transcoders[1].audioDecoder = 1;
            pSettings->transcoders[2].audioDecoder = 2;
            pSettings->transcoders[3].audioDecoder = 3;
            pSettings->transcoders[4].audioDecoder = 4;
            pSettings->transcoders[5].audioDecoder = 5;

            pSettings->graphics3d.used             = false;
            rc = 0;
            break;
        }
        case 8:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription = "Headless: No local Display,Transcode:one 4kP60Hevc10 to 1080p30Avc8bit; two 1080p60Hevc10bit to 1080p30Avc8bit";

            pSettings->videoDecoder[0].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[0].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                = "Video Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Video Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Video Decoder 2";

            pSettings->display[0].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[0].mainPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                 = "Transcode 0 Display";

            pSettings->display[2].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                 = "Transcode 1 Display";

            pSettings->display[3].property                  = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                 = "Transcode 2 Display";

            pSettings->graphics[0].property                 = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[0].used                     = true;
            pSettings->graphics[0].heapIdx                  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage                    = "Trancode 0 FB";

            pSettings->graphics[2].property                 = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used                     = true;
            pSettings->graphics[2].heapIdx                  = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[2].usage                    = "Trancode 1 FB";

            pSettings->graphics[3].property                 = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used                     = true;
            pSettings->graphics[3].heapIdx                  = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[3].usage                    = "Trancode 2 FB";

            pSettings->transcoders[0].videoDecoder          = 0;
            pSettings->transcoders[1].videoDecoder          = 1;
            pSettings->transcoders[2].videoDecoder          = 2;
            pSettings->transcoders[0].audioDecoder          = 0;
            pSettings->transcoders[1].audioDecoder          = 1;
            pSettings->transcoders[2].audioDecoder          = 2;

            pSettings->graphics3d.used                      = false;
            rc = 0;
            break;
        }
        case 9:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080p60->1080p30 8AVC";

            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";

            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB: ";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Miracast FB, M2MC";

            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 12:
        {
            pSettings->boxModeId = boxMode;
            /*brief description on the box mode, this stirng is shown in the UI*/
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080i60->720p30(Max)";

            /* video decoder to window mapping */
            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string shows up in the usage column*/
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";
            /* not very common */
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            /* video decoder to window mapping */
            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            /* property of this display, primary display  */
            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            /*source of picture buffer heap */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            /* property of this display, sd display  */
            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            /* picture buffer heap , this is row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            /* this display is used for transcode */
            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            /* this is application specific memory */
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            /* show this in the graphics page */
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB: ";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[2].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "No Graphics with transcode";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "No Graphics with transcode";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 2;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC1_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 13:
        {
            pSettings->boxModeId = boxMode;
            /*brief description on the box mode, this stirng is shown in the UI*/
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:single up to 1080p60";

            /* video decoder to window mapping */
            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string shows up in the usage column*/
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";
            /* not very common */
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            /* video decoder to window mapping */
            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            /* property of this display, primary display  */
            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            /*source of picture buffer heap */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            /* property of this display, sd display  */
            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            /* picture buffer heap , this is row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            /* this display is used for transcode */
            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            /* this is application specific memory */
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            /* show this in the graphics page */
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB: ";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[2].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";

            /* information for the encoder page*/
            /* index of video decoder for this trancode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 14:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:HD Output, Video:HD Main/no PIP,Transcode:Triple up to 1080p30(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Encoder 0 Display";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Encoder 1 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 2 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "Graphics with encode0";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = true;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Graphics with encode1";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = true;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Graphics with encode2";

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[2].audioDecoder = 3;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 15:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD Output, Video:Dual HD main/HD PIP,Transcode:Single HDMI input transcoding up to 1080p60(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 ";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Encoder 0 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 16:
        {
            pSettings->boxModeId = boxMode;
            /*brief description on the box mode, this stirng is shown in the UI*/
            pSettings->boxModeDescription          = "Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080i60->1080p30(Max)";

            /* video decoder to window mapping */
            pSettings->videoDecoder[0].property    = Memconfig_VideoDecoderProperty_eMain;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string shows up in the usage column*/
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0";
            /* not very common */
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            /* video decoder to window mapping */
            pSettings->videoDecoder[1].property    = Memconfig_VideoDecoderProperty_ePip;
            /*video picture buffer heap information, obtained from RTS document*/
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            /* video decoder used for transcode  */
            pSettings->videoDecoder[2].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property    = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            /* property of this display, primary display  */
            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            /*source of picture buffer heap */
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            /* property of this display, sd display  */
            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            /* picture buffer heap , this is row number in the table */
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            /* this string is displayed in the usage column */
            pSettings->display[1].usageMain        = "Secondary Display Main Window";

            /* this display is used for transcode */
            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            /* row number in the table */
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            /* usage column string */
            pSettings->display[2].usageMain        = "Encoder 0 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 1 Display";

            /* this is application specific memory */
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            /* show this in the graphics page */
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB: ";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "M2MC, Secondary Display FB: ";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[3].used            = false;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Miracast FB, M2MC";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            /* do not show in the graphics page , adding here as an example */
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Miracast FB, M2MC";

            /* information for the encoder page*/
            /* index of video decoder for this transcode */
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 2;

            pSettings->transcoders[1].videoDecoder = 3;
            pSettings->transcoders[1].audioDecoder = 3;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 17:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:None, Video:UHD Main/no PIP Headless,Transcode:Quad up to 1080p30/720p60/1080i60(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window/Transcode 0 Display ";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Transcode 1 Display";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Transcode 2 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Transcode 3 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = false;
            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].used            = false;
            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = false;
            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = false;

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[3].videoDecoder = 4;

            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[2].audioDecoder = 3;
            pSettings->transcoders[3].audioDecoder = 4;

            pSettings->graphics3d.used             = false;
            rc = 0;
            break;
        }
        case 18:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:HD Output, Video:HD Main/no PIP,Transcode:Triple up to 1080p30(Max)";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";

            pSettings->videoDecoder[2].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                         = "Video Decoder 2";

            pSettings->videoDecoder[3].property                      = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[3].pictureBufferHeapIdx          = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[3].usage                         = "Video Decoder 3";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->display[1].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain        = "Encoder 0 Display";

            pSettings->display[2].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain        = "Encoder 1 Display";

            pSettings->display[3].property         = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain        = "Encoder 2 Display";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "Graphics with encode0";

            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].used            = true;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "Graphics with encode1";

            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].used            = true;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC2_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "Graphics with encode2";

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[2].videoDecoder = 3;
            pSettings->transcoders[0].audioDecoder = 1;
            pSettings->transcoders[1].audioDecoder = 2;
            pSettings->transcoders[2].audioDecoder = 3;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC2_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }


/************* 97252 Box modes *****************/
		case 2:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD/SD, Video:UHD Main/HD PIP, Transcode: None";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Vid Decoder 0 Chroma for HEVC";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usagePip = "Display Primary-PIP";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usagePip = "Display Secondary-PIP";

            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage    = "Primary Display FB";
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage    = "M2MC,3D, Secondary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 4:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD(2160p50)/SD, Video:UHD Main/HD PIP, Transcode:Single 1080i50->720p25(Max)";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Vid Decoder 0 Chroma for HEVC";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Vid Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usagePip = "Display Primary-PIP";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usagePip = "Display Secondary-PIP";
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                = "Display Transcode";

            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "Primary Display FB";
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "3D,M2MC,Secondary Display FB";
            pSettings->graphics[1].used            = true;
            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "No Graphics with Transcode";
            pSettings->graphics[2].used            = false;
            pSettings->transcoders[0].videoDecoder = 2;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }
		case 5:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD/SD, Video:UHD Main/HD NO-PIP, Transcode:1080i60->720p30(Max)";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Vid Decoder 0 Chroma for HEVC";

			pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                = "Display Transcode";

            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "Primary Display FB";
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "3D,M2MC,Secondary Display FB";
            pSettings->graphics[2].used            = false;
            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "No Graphics with Transcode";

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC1_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }
		case 6:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD/SD, Video:HD Main/HD NO-PIP, Dual Transcode:1080p60->720p60(Max)";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->videoDecoder[2].property             = Memconfig_VideoDecoderProperty_eTranscode;
            pSettings->videoDecoder[2].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[2].usage                = "Vid Decoder 2";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[2].property = Memconfig_DisplayProperty_eTranscode;
			pSettings->display[3].property = Memconfig_DisplayProperty_eTranscode;

            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
			pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[2].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[2].usageMain                = "Display Transcode1";
			pSettings->display[3].mainPictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[3].usageMain                = "Display Transcode2";

            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "Primary Display FB";
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "3D,M2MC,Secondary Display FB";
            pSettings->graphics[2].used            = false;
            pSettings->graphics[2].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[2].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[2].usage           = "No Graphics with transcode";
            pSettings->graphics[3].used            = false;
            pSettings->graphics[3].property        = Memconfig_DisplayProperty_eTranscode;
            pSettings->graphics[3].heapIdx         = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[3].usage           = "No Graphics with transcode";

            pSettings->transcoders[0].videoDecoder = 1;
            pSettings->transcoders[0].audioDecoder = 1;
			pSettings->transcoders[1].videoDecoder = 2;
            pSettings->transcoders[1].audioDecoder = 2;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }
        case 10:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD/SD, Video:UHD Main/(HD Main + HD PIP), Transcode: None";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Vid Decoder 0 Chroma for HEVC";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usagePip = "Display Primary-PIP";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usagePip = "Display Secondary-PIP";

            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[0].usage    = "Primary Display FB";
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC1_GRAPHICS_HEAP;
            pSettings->graphics[1].usage    = "M2MC,3D, Secondary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC1_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }
        case 1001:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:UHD/SD, Video:HD Main/HD NO-PIP,Transcode: None";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";

            pSettings->graphics[0].used     = true;
            pSettings->graphics[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage    = "M2MC,3D, Primary Display FB";
            pSettings->graphics[1].used     = true;
            pSettings->graphics[1].property = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx  = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].usage    = "M2MC,3D, Secondary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }

    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
