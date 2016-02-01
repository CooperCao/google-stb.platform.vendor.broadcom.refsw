/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 ***************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(show_yuv);


static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent(context);
    return;
}

static NEXUS_Error convert_yuv420_yuv422(NEXUS_Graphics2DHandle graphics, BKNI_EventHandle checkpointEvent, NEXUS_SurfaceHandle yuv422, NEXUS_Addr yuv420, unsigned width, unsigned height, unsigned clip_width, unsigned clip_height)
{
    NEXUS_Error rc;
    static const BM2MC_PACKET_Blend combColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
        BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, false, BM2MC_PACKET_BlendFactor_eZero};
    static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eOne, false,
        BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
    BM2MC_PACKET_Plane planeY, planeCb, planeCr, planeYCbCr;
    void *buffer, *next;
    size_t size;

    planeY.address = yuv420;
    planeY.pitch = width;
    planeY.format = NEXUS_PixelFormat_eY8;
    planeY.width = clip_width;
    planeY.height = clip_height;

    planeCb.address = yuv420 + width*height;
    planeCb.pitch = width/2;
    planeCb.format = NEXUS_PixelFormat_eCb8;
    planeCb.width = clip_width/2;
    planeCb.height = clip_height/2;

    planeCr.address = yuv420 + width*height + (width/2)*(height/2);
    planeCr.pitch = width/2;
    planeCr.format = NEXUS_PixelFormat_eCr8;
    planeCr.width = clip_width/2;
    planeCr.height = clip_height/2;


    rc = NEXUS_Surface_LockPlane(yuv422, &planeYCbCr);
    BDBG_ASSERT(!rc);

    /* contributed by Shi-Long (Steven) Yang <syang@broadcom.com> */
    rc = NEXUS_Graphics2D_GetPacketBuffer(graphics, &buffer, &size, 1024);
    BDBG_ASSERT((!rc) && (size));

    next = buffer;
    {
        BM2MC_PACKET_PacketFilterEnable *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, FilterEnable, false );
        pPacket->enable = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketSourceFeeders *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, SourceFeeders, false );
        pPacket->plane0 = planeCb;
        pPacket->plane1 = planeCr;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketDestinationFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, DestinationFeeder, false );
        pPacket->plane = planeY;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane = planeYCbCr;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketBlend *pPacket = next;
        BM2MC_PACKET_INIT( pPacket, Blend, false );
        pPacket->color_blend = combColor;
        pPacket->alpha_blend = copyAlpha;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketScaleBlendBlit *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, ScaleBlendBlit, true);
        pPacket->src_rect.x = 0;
        pPacket->src_rect.y = 0;
        pPacket->src_rect.width = planeCb.width;
        pPacket->src_rect.height = planeCb.height;
        pPacket->out_rect.x = 0;
        pPacket->out_rect.y = 0;
        pPacket->out_rect.width = planeYCbCr.width;
        pPacket->out_rect.height = planeYCbCr.height;
        pPacket->dst_point.x = 0;
        pPacket->dst_point.y = 0;
        next = ++pPacket;
    }

    rc = NEXUS_Graphics2D_PacketWriteComplete(graphics, (uint8_t*)next - (uint8_t*)buffer);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(graphics, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, 5000);
        BDBG_ASSERT(!rc);
    }
    else {
        BDBG_ASSERT(!rc);
    }
    NEXUS_Surface_UnlockPlane(yuv422);
    return NEXUS_SUCCESS;
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    NEXUS_VideoWindowHandle window;
    NEXUS_SurfaceHandle yuv422_clip, yuv422;
    NEXUS_MemoryBlockHandle yuv422_mem;
    NEXUS_MemoryBlockHandle yuv420;
    void *yuv420_ptr;
    NEXUS_Addr yuv420_addr;
    size_t luma_size, chroma_size, frame_size;
    FILE *yuv;
    NEXUS_VideoInput videoInput;
    NEXUS_VideoImageInputHandle imageInput;
    unsigned frame_no;

    const char *filename = "akebono.png"; /* Sunset.jpg; change this to a jpeg of your choice */
    unsigned width = 1280;
    unsigned height = 720;
    unsigned clip_width, clip_height;

    if (argc > 1) {
        filename = argv[1];
    }
    if (argc > 2) {
        sscanf(argv[2], "%ux%u", &width, &height);
    }
    clip_width = width;
    clip_height = height;
    if (argc > 3) {
        sscanf(argv[3], "%ux%u", &clip_width, &clip_height);
    }

    luma_size = width * height;
    chroma_size = width/2 * height/2;
    frame_size = luma_size + 2*chroma_size;
    yuv = fopen(filename, "rb");
    if(yuv==NULL) {
        fprintf(stderr,"Can't open %s filename", filename);
        return -1;
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* HD display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    window = NEXUS_VideoWindow_Open(display, 0);
    BDBG_ASSERT(window);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    yuv422_mem = NEXUS_MemoryBlock_Allocate(NEXUS_Platform_GetFramebufferHeap(0), width*height*2, 0, NULL);
    BDBG_ASSERT(yuv422_mem);

    createSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
    createSettings.width  = width;
    createSettings.height = height;
    createSettings.pixelMemory = yuv422_mem;
    yuv422 = NEXUS_Surface_Create(&createSettings);
    BDBG_ASSERT(yuv422);

    createSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
    createSettings.width  = clip_width;
    createSettings.height = clip_height;
    createSettings.pitch = 2 * width;
    createSettings.pixelMemory = yuv422_mem;
    yuv422_clip = NEXUS_Surface_Create(&createSettings);
    BDBG_ASSERT(yuv422_clip);

    yuv420 = NEXUS_MemoryBlock_Allocate(NULL, frame_size, 0, NULL);
    BDBG_ASSERT(yuv420);
    rc = NEXUS_MemoryBlock_LockOffset(yuv420, &yuv420_addr);
    BDBG_ASSERT(rc == NEXUS_SUCCESS);
    rc = NEXUS_MemoryBlock_Lock(yuv420, &yuv420_ptr);
    BDBG_ASSERT(rc == NEXUS_SUCCESS);


    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context  = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    imageInput = NEXUS_VideoImageInput_Open(0, NULL);
    BDBG_ASSERT(imageInput);

    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
    BDBG_ASSERT(videoInput);

    rc = NEXUS_VideoWindow_AddInput(window, videoInput);
    BDBG_ASSERT(!rc);

    for(frame_no=0;;frame_no++) {
        int frc = fread(yuv420_ptr, 1, frame_size, yuv);
        BDBG_LOG(("reading frame %u (%d) (%ux%u)", frame_no, frc, width, height));
        if(frc != (int)frame_size) {
            break;
        }
        NEXUS_FlushCache(yuv420_ptr, frame_size);
        BDBG_LOG(("converting frame %u %ux%u", frame_no, width, height));
        rc = convert_yuv420_yuv422(gfx, checkpointEvent, yuv422, yuv420_addr, width, height, width, height);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        rc = NEXUS_VideoImageInput_SetSurface(imageInput, yuv422_clip);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        BDBG_LOG(("displaying frame %u %ux%u", frame_no, clip_width, clip_height));
        BKNI_Sleep(1000);
#if 1
        rc = NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
#endif
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
    }

    printf("Press any key to exit\n");
    getchar();

    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
    NEXUS_VideoWindow_RemoveInput(window, videoInput);
    NEXUS_VideoInput_Shutdown(videoInput);
    NEXUS_VideoImageInput_Close(imageInput);

    NEXUS_MemoryBlock_Unlock(yuv420);
    NEXUS_MemoryBlock_UnlockOffset(yuv420);
    NEXUS_MemoryBlock_Free(yuv420);

    NEXUS_MemoryBlock_Free(yuv422_mem);
    NEXUS_Surface_Destroy(yuv422_clip);
    NEXUS_Surface_Destroy(yuv422);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(yuv422);
    NEXUS_MemoryBlock_Unlock(yuv420);
    NEXUS_MemoryBlock_UnlockOffset(yuv420);
    NEXUS_MemoryBlock_Free(yuv420);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
