/******************************************************************************
 *    (c)2008-2010 Broadcom Corporation
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

#include <stdio.h>

#define UNUSED(X) (void)X

/* SIXTY_HZ needs to be defined for B552 LVDS->DVI converter support */
/* #define SIXTY_HZ */

#ifdef __cplusplus
extern "C"
{
#endif

void InitPlatform(void)
{
   NEXUS_PlatformSettings        platform_settings;
   NEXUS_Error                   err;

   /* Initialise the Nexus platform */
   NEXUS_Platform_GetDefaultSettings(&platform_settings);
#if NEXUS_DTV_PLATFORM
#ifdef SIXTY_HZ
   platform_settings.displayModuleSettings.panel.dvoLinkMode = NEXUS_PanelOutputLinkMode_eDualChannel1;
   platform_settings.displayModuleSettings.panel.lvdsColorMode = NEXUS_LvdsColorMode_e8Bit;
#endif
#endif
   platform_settings.openFrontend = false;

   err = NEXUS_Platform_Init(&platform_settings);
   if (err)
      printf("NEXUS_Platform_Init() failed\n");
}

/* If format == 0, w & h are used to determine requested video format */
NEXUS_DisplayHandle OpenDisplay(NEXUS_VideoFormat format, uint32_t w, uint32_t h)
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
#if !NEXUS_DTV_PLATFORM
   else
   {
      if (w <= 720 && h <= 480)
         display_settings.format = NEXUS_VideoFormat_eNtsc;
      else if (w <= 1280 && h <= 720)
         display_settings.format = NEXUS_VideoFormat_e720p;
      else
         display_settings.format = NEXUS_VideoFormat_e1080i; /* Change to 1080p when it works on 7422 */
   }
#else
   UNUSED(w);
   UNUSED(h);
#endif

   display = NEXUS_Display_Open(0, &display_settings);
   if (!display)
      printf("NEXUS_Display_Open() failed\n");

   NEXUS_Display_GetGraphicsSettings(display, &graphics_settings);
   graphics_settings.horizontalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
   graphics_settings.verticalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
   NEXUS_Display_SetGraphicsSettings(display, &graphics_settings);

   return display;
}

void InitPanelOutput(NEXUS_DisplayHandle display)
{
#if NEXUS_NUM_PANEL_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;

   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.panel[0])
   {
#ifdef SIXTY_HZ
      {
         NEXUS_PanelOutputSettings     panelOutputSettings;
         NEXUS_PanelOutput_GetSettings(platform_config.outputs.panel[0], &panelOutputSettings);
         panelOutputSettings.frameRateMultiplier = 1;
         NEXUS_PanelOutput_SetSettings(platform_config.outputs.panel[0], &panelOutputSettings);
      }
#endif

      NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platform_config.outputs.panel[0]));
   }
#else
   UNUSED(display);
#endif
}

void InitComponentOutput(NEXUS_DisplayHandle display)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS && !NEXUS_DTV_PLATFORM

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
#if NEXUS_NUM_COMPOSITE_OUTPUTS && !NEXUS_DTV_PLATFORM

   NEXUS_PlatformConfiguration   platform_config;

   if (w <= 720 && h <=480)
   {
      NEXUS_Platform_GetConfiguration(&platform_config);

      if (platform_config.outputs.composite[0])
         NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platform_config.outputs.composite[0]));
   }
#else
   UNUSED(display);
#endif
}

#if NEXUS_NUM_HDMI_OUTPUTS && !NEXUS_DTV_PLATFORM
static void hotplug_callback(void *pParam, int iParam)
{
   NEXUS_HdmiOutputStatus status;
   NEXUS_HdmiOutputHandle hdmi = pParam;
   NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

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
#if NEXUS_NUM_HDMI_OUTPUTS && !NEXUS_DTV_PLATFORM

   NEXUS_HdmiOutputSettings      hdmiSettings;
   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.hdmi[0])
   {
      NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platform_config.outputs.hdmi[0]));
      /* Install hotplug callback -- video only for now */
      NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
      hdmiSettings.hotplugCallback.callback = hotplug_callback;
      hdmiSettings.hotplugCallback.context = platform_config.outputs.hdmi[0];
      hdmiSettings.hotplugCallback.param = (int)display;
      NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);

      /* Force a hotplug to switch to a supported format if necessary */
      hotplug_callback(platform_config.outputs.hdmi[0], (int)display);
   }
#else
   UNUSED(display);
#endif
}

#ifdef __cplusplus
}
#endif
