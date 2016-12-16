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
/************* 97271 Box modes *****************/
        case 1:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/no PIP/Multi-PIP,Transcode:None; LPDDR4-3200";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 2:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/no PIP/Multi-PIP (4+0); Transcode:None; LPDDR4-3200";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 3:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/PIP,Transcode:None; LPDDR4-2133";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";
            pSettings->videoDecoder[1].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].secondaryUsage                = "Video Decoder 1";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 4:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription                            = "Display:HD/SD; Video:HD Main/HD PIP; Transcode:None; LPDDR4-1600";
            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Vid Decoder 0";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Vid Decoder 0 Chroma for HEVC";

            pSettings->videoDecoder[1].property             = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                = "Vid Decoder 1";

            pSettings->display[0].property = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[1].property = Memconfig_DisplayProperty_eSecondary;

            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain                = "Display Primary-MAIN";
            pSettings->display[0].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usagePip = "Display Primary-PIP";
            pSettings->display[1].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usageMain                = "Display Secondary-MAIN";
            pSettings->display[1].pipPictureBufferHeapIdx  = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[1].usagePip = "Display Secondary-PIP";

            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "Primary Display FB";
            pSettings->graphics[1].used            = true;
            pSettings->graphics[1].property        = Memconfig_DisplayProperty_eSecondary;
            pSettings->graphics[1].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[1].usage           = "3D,M2MC,Secondary Display FB";
            pSettings->graphics[1].used            = true;

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";
            rc = 0;
            break;
        }
        case 5:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/PIP,Transcode:None; LPDDR4-3200";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";
            pSettings->videoDecoder[1].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].secondaryUsage                = "Video Decoder 1";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 6:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/PIP,Transcode:None; LPDDR4-3733";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->videoDecoder[1].property                      = Memconfig_VideoDecoderProperty_ePip;
            pSettings->videoDecoder[1].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].usage                         = "Video Decoder 1";
            pSettings->videoDecoder[1].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[1].secondaryUsage                = "Video Decoder 1";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
        case 1000:
        {
            pSettings->boxModeId = boxMode;
            pSettings->boxModeDescription          = "Display:UHD; Video:UHD Main/no PIP/Multi-PIP (3+0); Transcode:None; LPDDR4-2667";

            pSettings->videoDecoder[0].property                      = Memconfig_VideoDecoderProperty_eMain;
            pSettings->videoDecoder[0].pictureBufferHeapIdx          = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].usage                         = "Video Decoder 0 Luma";
            pSettings->videoDecoder[0].secondaryPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->videoDecoder[0].secondaryUsage                = "Video Decoder 0 Chroma";

            pSettings->display[0].property         = Memconfig_DisplayProperty_ePrimary;
            pSettings->display[0].mainPictureBufferHeapIdx = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            pSettings->display[0].usageMain        = "Primary Display Main Window";

            pSettings->graphics[0].property        = Memconfig_DisplayProperty_ePrimary;
            pSettings->graphics[0].used            = true;
            pSettings->graphics[0].heapIdx         = NEXUS_MEMC0_GRAPHICS_HEAP;
            pSettings->graphics[0].usage           = "M2MC, 3D, Primary Display FB";

            pSettings->graphics3d.used             = true;
            pSettings->graphics3d.heapIdx          = NEXUS_MEMC0_GRAPHICS_HEAP; /* should match the 3D Primary graphics heap index */
            pSettings->graphics3d.usage            = "V3D";

            rc = 0;
            break;
        }
    } /* switch */

    return( rc );
} /* Memconfig_GetBoxModeDefaultSettings */
