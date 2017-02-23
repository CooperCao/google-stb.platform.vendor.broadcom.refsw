/******************************************************************************
 *    Broadcom Proprietary and Confidential. (c)2008-2010 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include "init.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif
#include "nexus_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(X) (void)X

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef SINGLE_PROCESS
eInitResult InitPlatform(bool secure)
{
   NEXUS_Error                         err = NEXUS_NOT_SUPPORTED;
   NEXUS_PlatformSettings              platform_settings;
   NEXUS_PlatformConfigCapabilities    cap;
   NEXUS_MemoryConfigurationSettings   memory_config_settings;

   NEXUS_Platform_GetDefaultSettings(&platform_settings);
   platform_settings.openFrontend = false;
   NEXUS_GetDefaultMemoryConfigurationSettings(&memory_config_settings);

#if NEXUS_HAS_SAGE
   if (secure)
   {
      memory_config_settings.videoDecoder[0].secure = NEXUS_SecureVideo_eBoth;
      NEXUS_GetPlatformConfigCapabilities(&platform_settings, &memory_config_settings, &cap);
      if (!cap.secureGraphics[0].valid) return -1;
      platform_settings.heap[cap.secureGraphics[0].heapIndex].size = 64 * 1024 * 1024;
      platform_settings.heap[cap.secureGraphics[0].heapIndex].memcIndex = cap.secureGraphics[0].memcIndex;
      platform_settings.heap[cap.secureGraphics[0].heapIndex].heapType = NEXUS_HEAP_TYPE_SECURE_GRAPHICS;
      platform_settings.heap[cap.secureGraphics[0].heapIndex].memoryType =
         NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED |
         NEXUS_MEMORY_TYPE_MANAGED |
         NEXUS_MEMORY_TYPE_SECURE;
   }
#endif

   err = NEXUS_Platform_MemConfigInit(&platform_settings, &memory_config_settings);
   if (err != NEXUS_SUCCESS)
      return eInitFailed;

#if NEXUS_HAS_SAGE
   if (secure)
   {
      NEXUS_PlatformConfiguration platform_config;
      NEXUS_MemoryStatus memory_status_0, memory_status_1;
      NEXUS_Platform_GetConfiguration(&platform_config);
      /* Ensure secure graphics heap is also GFD0 accessible. In a multiprocess system, clients blit to large offscreen secure graphics heap
         and NSC copies to GFD0/GFD1 secure graphics. For this example, we keep it simple. */
      err = NEXUS_Heap_GetStatus(NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE), &memory_status_0);
      BDBG_ASSERT(!err);
      err = NEXUS_Heap_GetStatus(NEXUS_Platform_GetFramebufferHeap(0), &memory_status_1);
      BDBG_ASSERT(!err);
      if (memory_status_0.memcIndex != memory_status_1.memcIndex)
      {
         printf("app does not support multiple secure graphics heaps\n");
         return eInitFailed;
      }
      BDBG_ASSERT(platform_config.heap[cap.secureGraphics[0].heapIndex] == NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE));
   }
#endif

   return eInitSuccess;
}

#endif /* SINGLE_PROCESS */

#ifndef SINGLE_PROCESS
eInitResult InitPlatform(bool secure)
{
   NEXUS_Error err = NEXUS_NOT_SUPPORTED;
   BSTD_UNUSED(secure);

   /* We are multiprocess mode Nexus, so join with the existing instance */
#ifdef NXCLIENT_SUPPORT
   err = NxClient_Join(NULL);
#else
   err = NEXUS_Platform_Join();
#endif

   if (err != NEXUS_SUCCESS)
   {
      printf("Failed to join an existing server\n");
      return eInitFailed;
   }

   return eInitSuccess;
}
#endif /* !SINGLE_PROCESS */

eInitResult InitPlatformAndDefaultDisplay(NEXUS_DISPLAYHANDLE *handle, float *aspect, uint32_t w, uint32_t h, bool secure)
{
   eInitResult res = InitPlatform(secure);

   if (res == eInitSuccess)
   {
#ifdef SINGLE_PROCESS
      NEXUS_GraphicsSettings  graphics_settings;

      /* We are the primary process, so open the display */
      *handle = OpenDisplay(0, w, h, secure);
      InitCompositeOutput(*handle, w, h);
      InitComponentOutput(*handle);
      InitHDMIOutput(*handle);

      if (*handle != 0)
      {
         NEXUS_Display_GetGraphicsSettings(*handle, &graphics_settings);
         *aspect = (float)graphics_settings.position.width / graphics_settings.position.height;
      }
      else
         *aspect = (float)w / (float)h;
#else
      *aspect = (float)w / (float)h;
      *handle = 0;
#endif
   }

   return res;
}

void TermPlatform(NEXUS_DISPLAYHANDLE handle)
{
   /* Close the Nexus display */
#ifdef SINGLE_PROCESS
   if (handle != 0)
   {
#if NEXUS_NUM_HDMI_OUTPUTS
      NEXUS_HdmiOutputSettings      hdmiSettings;
      NEXUS_PlatformConfiguration   platform_config;
      NEXUS_Platform_GetConfiguration(&platform_config);

      NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
      hdmiSettings.hotplugCallback.callback = 0;
      hdmiSettings.hotplugCallback.context = 0;
      hdmiSettings.hotplugCallback.param = 0;
      NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
#endif
      NEXUS_Display_Close(handle);
   }

   NEXUS_Platform_Uninit();
#else
   UNUSED(handle);
#ifdef NXCLIENT_SUPPORT
   NxClient_Uninit();
#endif
#endif
}

/* If format == 0, w & h are used to determine requested video format */
NEXUS_DisplayHandle OpenDisplay(NEXUS_VideoFormat format, uint32_t w, uint32_t h, bool secure)
{
   NEXUS_DisplayHandle     display = NULL;
   NEXUS_DisplaySettings   display_settings;
   NEXUS_GraphicsSettings  graphics_settings;

   /* Bring up display */
   NEXUS_Display_GetDefaultSettings(&display_settings);

   if (format != 0)
   {
      display_settings.format = format;
   }
   else
   {
      if (w <= 720 && h <= 480)
         display_settings.format = NEXUS_VideoFormat_eNtsc;
      else if (w <= 1280 && h <= 720)
         display_settings.format = NEXUS_VideoFormat_e720p;
      else
         display_settings.format = NEXUS_VideoFormat_e1080p;
   }
   display_settings.displayType = NEXUS_DisplayType_eAuto;

   display = NEXUS_Display_Open(0, &display_settings);
   if (!display)
      printf("NEXUS_Display_Open() failed\n");

   NEXUS_Display_GetGraphicsSettings(display, &graphics_settings);
   graphics_settings.horizontalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
   graphics_settings.verticalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
   graphics_settings.secure = secure;

   /* Disable blend with video plane */
   graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eOne;
   graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eZero;

   NEXUS_Display_SetGraphicsSettings(display, &graphics_settings);

   return display;
}

void InitComponentOutput(NEXUS_DisplayHandle display)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.component[0])
      NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platform_config.outputs.component[0]));
#else
   UNUSED(display);
#endif
}

void InitCompositeOutput(NEXUS_DisplayHandle display, uint32_t w, uint32_t h)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;

   if (w <= 720 && h <=480)
   {
      NEXUS_Platform_GetConfiguration(&platform_config);

      if (platform_config.outputs.composite[0])
         NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platform_config.outputs.composite[0]));
   }
#else
   UNUSED(display);
   UNUSED(w);
   UNUSED(h);
#endif
}

#if NEXUS_NUM_HDMI_OUTPUTS
typedef struct DisplaySessionHandles
{
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_DisplayHandle    display;
} DisplaySessionHandles;

static void hotplug_callback(void *pParam, int iParam)
{
   NEXUS_HdmiOutputStatus status;
   DisplaySessionHandles *displayHandles = (DisplaySessionHandles*)pParam;
   NEXUS_HdmiOutputHandle hdmi = displayHandles->hdmi;
   NEXUS_DisplayHandle    display = displayHandles->display;

   UNUSED(iParam);

   NEXUS_HdmiOutput_GetStatus(hdmi, &status);
   fprintf(stderr, "HDMI hotplug event: %s\n", status.connected?"connected":"not connected");

   /* the app can choose to switch to the preferred format, but it's not required. */
   if (status.connected)
   {
      NEXUS_DisplaySettings displaySettings;
      NEXUS_Display_GetSettings(display, &displaySettings);
      if (!status.videoFormatSupported[displaySettings.format])
      {
         fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
         displaySettings.format = status.preferredVideoFormat;
         NEXUS_Display_SetSettings(display, &displaySettings);
      }
   }
}
#endif

void InitHDMIOutput(NEXUS_DisplayHandle display)
{
#if NEXUS_NUM_HDMI_OUTPUTS
   static DisplaySessionHandles  displayHandles;
   NEXUS_HdmiOutputSettings      hdmiSettings;
   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.hdmi[0])
   {
      NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platform_config.outputs.hdmi[0]));
      /* Install hotplug callback -- video only for now */
      displayHandles.hdmi = platform_config.outputs.hdmi[0];
      displayHandles.display = display;

      NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
      hdmiSettings.hotplugCallback.callback = hotplug_callback;
      hdmiSettings.hotplugCallback.context = &displayHandles;
      NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);

      /* Force a hotplug to switch to a supported format if necessary */
      hotplug_callback(&displayHandles, 0);
   }
#else
   UNUSED(display);
#endif
}

#ifdef __cplusplus
}
#endif
