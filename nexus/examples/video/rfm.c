/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
******************************************************************************/
#if NEXUS_HAS_RFM
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_composite_output.h"
#include "nexus_rfm.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>

int main(void)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_SurfaceHandle surface;
    int rc;

    rc = NEXUS_Platform_Init(NULL);
    if (rc) return rc;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (platformConfig.outputs.composite[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
#endif
    if (!platformConfig.outputs.rfm[0]) {
        fprintf(stderr, "rfm not available");
        return -1;
    }
    NEXUS_Display_AddOutput(display, NEXUS_Rfm_GetVideoConnector(platformConfig.outputs.rfm[0]));

    {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_GraphicsSettings graphicsSettings;
        unsigned x,y;
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        surfaceCreateSettings.width = 720;
        surfaceCreateSettings.height = 480;
        surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
        surface = NEXUS_Surface_Create(&surfaceCreateSettings);
        NEXUS_Surface_GetMemory(surface, &mem);
        for (y=0;y<surfaceCreateSettings.height;y++) {
            uint32_t *ptr = (uint32_t *)&((uint8_t*)mem.buffer)[y*mem.pitch];
            unsigned color = (0xFF<<24)|(y/20*0x1234);
            for (x=0;x<surfaceCreateSettings.width;x++) ptr[x] = color;
        }
        NEXUS_Surface_Flush(surface);
        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, surface);
        BDBG_ASSERT(!rc);
    }

    while (1) {
        NEXUS_RfmSettings rfmSettings;
        NEXUS_Rfm_GetSettings(platformConfig.outputs.rfm[0], &rfmSettings);

        printf("Displaying on channel %d. Press ENTER to toggle between channels 3 and 4.\n", rfmSettings.channel);
        getchar();

        rfmSettings.channel = rfmSettings.channel == 3 ? 4 : 3;
        rc = NEXUS_Rfm_SetSettings(platformConfig.outputs.rfm[0], &rfmSettings);
        BDBG_ASSERT(!rc);
    }

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This platform doesn't support rfm.\n");
    return 0;
}
#endif
