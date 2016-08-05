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
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_97445);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH265);
#if BCHP_VER >= BCHP_VER_E0
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eVp9);
#endif
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
#else
    BSTD_UNUSED(pSettings);
#endif
#if NEXUS_HAS_VIDEO_DECODER
    switch (g_pPreInitState->boxMode) {
    case 15:
        /* linked decoders */
        pSettings->videoDecoder[0].mosaic.maxNumber = 1;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[1].mosaic.maxNumber = 1;
        pSettings->videoDecoder[1].mosaic.maxWidth =1920;
        pSettings->videoDecoder[1].mosaic.maxHeight=1088;
        break;
    default:
        break;
    }
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    switch(boxMode)
    {
    /**** For 7445 ****/
    case 1:
    case 3:
    case 7:
    case 8:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 1000:
    default:
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size =  208*1024*1024; /*decoder FW+general,xpt playback,audio other general purpose */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 168*1024 *1024; /* CABACs(28)for 3 decoders + RAVE CDB(6+15) */
        pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = (0x00400000*2);
        pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].size = 5*1024*1024;  /* RDC heap plus margin for possible VCE fw/debug/output descriptors heap */
        break;

    /**** For 7252/7448/7449 ****/
    case 2:
    case 4:
    case 5:
    case 6:
    case 10:
    case 1001:
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size =  192*1024*1024; /*decoder FW+general,xpt playback,audio other general purpose */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 124*1024 *1024; /* CABACs(28)for 3 decoders + RAVE CDB(6+15) */
        pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 4*1024*1024;  /* RDC heap plus margin */
        break;
    }

    switch(boxMode)
    {
    /**** For 7445 ****/
    case 3:
    case 12:
    case 1000:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 512*1024*1024; /*gfd 0 on memc 1 */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 20*1024*1024;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            break;
    case 7:
    case 8:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 16*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 256*1024*1024; /* gfd 0 on memc 2*/
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 14:
    case 18:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 200*1024*1024; /*gfd 0/4/5/6 on memc 0 */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 15:
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 512*1024*1024; /*gfd 0 on memc 2 */
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 13:
    case 16:
    default:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 0;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 512*1024*1024; /* gfd on memc 2*/
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;

    /**** For 7252/7448/7449 ****/
    case 2:
    case 4:
    case 6:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 164*1024*1024; /*192+124+148 heap should be < 528 */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 384*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 5:
    case 10:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 0;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 512*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 1001:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 160*1024*1024;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 0;
            break;
    }

    if(boxMode == 7) {
        pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].size = 16*1024*1024;
    }

    if(boxMode == 10) {
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 0;  /* Saving 4M by making RDC heap plus margin to 0 as RDC takes memory from MEMC0_MAIN_HEAP and no VCE descriptors needed for box mode #10 */
    }

    return;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    switch (g_pPreInitState->boxMode) {

    /**** For 7445 ****/
    default:
    case 1:
            BDBG_WRN(("*** 97445 BoxMode 1:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080p60->720p30(Max)***"));
        break;
    case 3:
            BDBG_WRN(("*** 97445 BoxMode 3:Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Triple up to 1080p30(Max)***"));
        break;
    case 7:
            BDBG_WRN(("*** 97445 BoxMode 7:Display:None, Video:no Main/no PIP Headless,Transcode:Six up to 720p30(Max)***"));
        break;
    case 8:
            BDBG_WRN(("*** 97445 BoxMode 8:Display:None, Video:no Main/no PIP Headless,Transcode:Triple up to 1080p30(Max)***"));
        break;
    case 12:
            BDBG_WRN(("*** 97445 BoxMode 12:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080i60->720p30(Max)***"));
        break;
    case 13:
            BDBG_WRN(("*** 97445 BoxMode 13:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:single up to 1080p60***"));
        break;
	case 14:
            BDBG_WRN(("*** 97445 BoxMode 14:Display:HD Output, Video:HD Main/no PIP,Transcode:Triple (One up to 1080p60 (Max) and Dual up to 1080p30(Max))***"));
        break;
    case 15:
            BDBG_WRN(("*** 97445 BoxMode 15:Display:UHD Output, Video:Dual HD main/HD PIP,Transcode:Single HDMI input transcoding up to 1080p60(Max)***"));
        break;
    case 16:
            BDBG_WRN(("*** 97445 BoxMode 16:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080i60->1080p30(Max)***"));
        break;
    case 17:
            BDBG_WRN(("*** 97445 BoxMode 17:Display:None, Video:UHD Main/no PIP Headless,Transcode:Quad up to 1080p30/720p60/1080i60(Max)***"));
        break;
    case 18:
            BDBG_WRN(("*** 97445 BoxMode 18:Display:HD Output, Video:HD Main/no PIP,Transcode:Triple (One up to 1080p60 (Max) and Dual up to 1080p30(Max))***"));
        break;
    case 1000:
            BDBG_WRN(("*** 97445 TEMP BoxMode 1000:Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Quad up to 1080p30(Max)***"));
        break;

    /**** For 7252/7448/7449 ****/
    case 2:
#if NEXUS_USE_7449_VMS_SFF
            BDBG_WRN(("*** 97449D0 BoxMode 2:Display:UHD/SD, Video:UHD Main/HD PIP***"));
#else
            BDBG_WRN(("*** 97252D0 BoxMode 2:Display:UHD/SD, Video:UHD Main/HD PIP***"));
#endif
        break;
    case 4:
            BDBG_WRN(("*** 97252D0 BoxMode 4:Display:UHD/SD, Video:UHD Main/HD PIP, Transcode:1080i50->720p25(Max)***"));
        break;
    case 5:
            BDBG_WRN(("*** 97252D0 BoxMode 5:Display:UHD/SD, Video:UHD Main/HD NO-PIP, Transcode:1080i60->720p30(Max)***"));
        break;
    case 6:
            BDBG_WRN(("*** 97252D0 BoxMode 6:Display:UHD/SD, Video:HD Main/HD NO-PIP, Dual Transcode:1080p60->720p60(Max)***"));
        break;
    case 10:
            BDBG_WRN(("*** 97252D0 BoxMode 10:Display:UHD/SD, Video:UHD Main/(HD Main + HD PIP) ***"));
        break;
    case 1001:
            BDBG_WRN(("*** 97252D0 Temp BoxMode 1001:Display:UHD/SD, Video:HD Main/HD NO-PIP,NO Transcode ***"));
        break;

    }

/**** For 7445 ****/
#if NEXUS_USE_7445_SV
    BDBG_WRN(("*** Initializing 7445 SV Board ...***"));
#endif
#if NEXUS_USE_7445_VMS_SFF
    BDBG_WRN(("***Initializing 7445 VMS_SFF Board ...***"));
#endif
#if NEXUS_USE_7445_C
    BDBG_WRN(("***Initializing 7445 C Board ...***"));
#endif
#if NEXUS_USE_7445_DBS
    BDBG_WRN(("***Initializing 7445 DBS Board ...***"));
#endif
#if NEXUS_USE_7445_LCC
    BDBG_WRN(("***Initializing 7445 LCC Board ...***"));
#endif
#if NEXUS_USE_7445_AUTO
    BDBG_WRN(("***Initializing 7445 AUTO Board ...***"));
#endif
#if NEXUS_USE_7445_EXT24
    BDBG_WRN(("***Initializing 7445 EXT24 Board ...***"));
#endif


/**** For 7252/7448/7449 ****/
#if NEXUS_USE_7252_SV
    BDBG_WRN(("*** Initializing 7252 SV Board ...***"));
#elif NEXUS_USE_7252_VMS_SFF
    BDBG_WRN(("***Initializing 7252 VMS_SFF Board ...***"));
#elif NEXUS_USE_7252_C
    BDBG_WRN(("***Initializing 7252 C Board ...***"));
#elif NEXUS_USE_7252_LCC
    BDBG_WRN(("***Initializing 7252 LCC Board ...***"));
#elif NEXUS_USE_7449_VMS_SFF
    BDBG_WRN(("*** Initializing 7449 VMS_SFF Board ...***"));
#endif

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
