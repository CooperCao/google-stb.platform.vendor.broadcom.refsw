/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
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
 *
 *****************************************************************************/
#ifndef PLATFORM_H__
#define PLATFORM_H__ 1

#include "platform_types.h"

PlatformHandle platform_open(const char * appName);
void platform_close(PlatformHandle platform);

void platform_get_default_model(PlatformModel * pModel);
void platform_get_default_picture_info(PlatformPictureInfo * pInfo);
const char * platform_get_dynamic_range_name(PlatformDynamicRange eotf);
const char * platform_get_colorimetry_name(PlatformColorimetry colorimetry);
const char * platform_get_color_space_name(PlatformColorSpace colorSpace);
const char * platform_get_capability_name(PlatformCapability cap);
PlatformDynamicRange platform_get_dynamic_range_from_path(const char * path);

PlatformDisplayHandle platform_display_open(PlatformHandle platform);
void platform_display_close(PlatformDisplayHandle display);
const PlatformPictureInfo * platform_display_get_picture_info(PlatformDisplayHandle display);
void platform_display_print_hdmi_drm_settings(PlatformDisplayHandle display, const char * tag);
void platform_display_print_hdmi_status(PlatformDisplayHandle display);
void platform_display_set_hdmi_drm_dynamic_range(PlatformDisplayHandle display, PlatformDynamicRange eotf);
void platform_display_set_hdmi_colorimetry(PlatformDisplayHandle display, PlatformColorimetry colorimetry);
bool platform_display_hdmi_is_connected(PlatformDisplayHandle display);
void platform_display_wait_for_display_settings_application(PlatformDisplayHandle display);

void platform_plm_set_vid_point(unsigned inputIndex, unsigned rectIndex, unsigned pointIndex, double slopeMantissa, int slopeExponent, double x, double y);
void platform_plm_get_vid_point(unsigned inputIndex, unsigned rectIndex, unsigned pointIndex, double * pSlopeMantissa, int * pSlopeExponent, double * pX, double * pY);
void platform_plm_set_gfx_point(unsigned inputIndex, unsigned rectIndex, unsigned pointIndex, double slopeMantissa, int slopeExponent, double x, double y);
void platform_plm_get_gfx_point(unsigned inputIndex, unsigned rectIndex, unsigned pointIndex, double * pSlopeMantissa, int * pSlopeExponent, double * pX, double * pY);

PlatformGraphicsHandle platform_graphics_open(PlatformHandle platform, const char * fontPath, unsigned fbWidth, unsigned fbHeight);
void platform_graphics_close(PlatformGraphicsHandle gfx);
void platform_graphics_fill(PlatformGraphicsHandle gfx, const PlatformRect * rect, unsigned color);
void platform_graphics_blend(PlatformGraphicsHandle gfx, const PlatformRect * rect, unsigned color);
void platform_graphics_clear(PlatformGraphicsHandle gfx, const PlatformRect * pRect);
void platform_graphics_get_default_text_rendering_settings(PlatformTextRenderingSettings * pSettings);
void platform_graphics_render_text(PlatformGraphicsHandle gfx, const char * text, const PlatformTextRenderingSettings * pSettings);
unsigned platform_graphics_get_text_height(PlatformGraphicsHandle gfx);
unsigned platform_graphics_get_text_width(PlatformGraphicsHandle gfx, const char * text);
const PlatformRect * platform_graphics_get_fb_rect(PlatformGraphicsHandle gfx);
void platform_graphics_submit(PlatformGraphicsHandle gfx);
void platform_graphics_render_picture(PlatformGraphicsHandle gfx, PlatformPictureHandle pic, const PlatformRect * pRect);
void platform_graphics_scale_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect);
void platform_graphics_move_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect);
void platform_graphics_render_video(PlatformGraphicsHandle gfx, const PlatformRect * pRect);

PlatformPictureHandle platform_picture_create(PlatformHandle platform, const char * picturePath);
void platform_picture_destroy(PlatformPictureHandle pic);
const PlatformPictureInfo * platform_picture_get_info(PlatformPictureHandle pic);
const PlatformPictureFormat * platform_picture_get_format(PlatformPictureHandle pic);
void platform_picture_get_dimensions(PlatformPictureHandle pic, unsigned * pWidth, unsigned * pHeight);
const char * platform_picture_get_path(PlatformPictureHandle pic);

extern const int PLATFORM_TRICK_RATE_1X;
PlatformMediaPlayerHandle platform_media_player_create(PlatformHandle platform, PlatformCallback streamInfoCallback, void * streamInfoContext);
void platform_media_player_destroy(PlatformMediaPlayerHandle player);
void playform_media_player_start(PlatformMediaPlayerHandle player);
void playform_media_player_stop(PlatformMediaPlayerHandle player);
void playform_media_player_trick(PlatformMediaPlayerHandle player, int rate);
void platform_media_player_get_picture_info(PlatformMediaPlayerHandle player, PlatformPictureInfo * pInfo);

PlatformReceiverHandle platform_receiver_open(PlatformHandle platform, PlatformCallback hotplugCallback, void * hotplugContext);
void platform_receiver_close(PlatformReceiverHandle rx);
void platform_receiver_start(PlatformReceiverHandle rx);
const PlatformReceiverModel * platform_receiver_supports_picture(PlatformReceiverHandle rx, const PlatformPictureInfo * pInfo);
PlatformCapability platform_receiver_supports_dynamic_range(PlatformReceiverHandle rx, PlatformDynamicRange eotf);
PlatformCapability platform_receiver_supports_colorimetry(PlatformReceiverHandle rx, PlatformColorimetry eotf);
PlatformCapability platform_receiver_supports_color_space(PlatformReceiverHandle rx, PlatformColorSpace color_space);
PlatformCapability platform_receiver_supports_format(PlatformReceiverHandle rx, const PlatformPictureFormat * pFormat);
PlatformCapability platform_receiver_supports_color_depth(PlatformReceiverHandle rx, unsigned color_depth);

PlatformInputHandle platform_input_open(PlatformHandle platform, PlatformInputMethod method);
void platform_input_close(PlatformInputHandle input);
void platform_input_set_event_handler(PlatformInputHandle input, PlatformInputEvent event, PlatformCallback callback, void * callbackContext, int param);
bool platform_input_try(PlatformInputHandle input);

PlatformSchedulerHandle platform_get_scheduler(PlatformHandle platform);
PlatformListenerHandle platform_scheduler_add_listener(PlatformSchedulerHandle scheduler, PlatformCallback callback, void * pCallbackContext);
void platform_scheduler_remove_listener(PlatformSchedulerHandle scheduler, PlatformListenerHandle listener);
void platform_scheduler_wake(PlatformSchedulerHandle scheduler);

#endif /* PLATFORM_H__ */
