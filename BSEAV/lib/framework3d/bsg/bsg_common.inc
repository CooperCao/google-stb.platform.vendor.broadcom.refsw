#############################################################################
#    (c)2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
#############################################################################

CPP_COMMON_FILES += \
	bsg_animation_list.cpp \
	bsg_animation_sequence.cpp \
	bsg_application.cpp \
	bsg_application_options.cpp \
	bsg_bound.cpp \
	bsg_camera.cpp \
	bsg_carousel.cpp \
	bsg_catmullrom.cpp \
	bsg_constraint.cpp \
	bsg_context.cpp \
	bsg_cube_images.cpp \
	bsg_dither.cpp \
	bsg_effect.cpp \
	bsg_effect_generator.cpp \
	bsg_effect_semantics.cpp \
	bsg_exception.cpp \
	bsg_fbo.cpp \
	bsg_font.cpp \
	bsg_font_texture_packer.cpp \
	bsg_gather_visitor.cpp \
	bsg_geometry.cpp \
	bsg_gl_intercept.cpp \
	bsg_gl_program.cpp \
	bsg_gl_texture.cpp \
	bsg_gl_uniform.cpp \
	bsg_hud.cpp \
	bsg_image.cpp \
	bsg_image_astc.cpp \
	bsg_image_pkm.cpp \
	bsg_image_png.cpp \
	bsg_image_raw.cpp \
	bsg_key_events.cpp \
	bsg_mouse_events.cpp \
	bsg_library.cpp \
	bsg_lod_group.cpp \
	bsg_lod_range.cpp \
	bsg_material.cpp \
	bsg_matrix.cpp \
	bsg_obj_reader.cpp \
	bsg_pass.cpp \
	bsg_pass_state.cpp \
	bsg_parse_utils.cpp \
	bsg_platform.cpp \
	bsg_print.cpp \
	bsg_quaternion.cpp \
	bsg_sampler_semantics.cpp \
	bsg_scene_node.cpp \
	bsg_semantic_data.cpp \
	bsg_surface.cpp \
	bsg_text.cpp \
	bsg_time.cpp \
	bsg_transform.cpp \
	bsg_fonts.cpp \
	bsg_utf8.cpp \
	bsg_task.cpp \
	bsg_shape.cpp \
	bsg_quad_render.cpp \
	bsg_render_options.cpp \
	bsg_pixel.cpp

H_COMMON_FILES += \
	bsg_common.h \
	bsg_animatable.h \
	bsg_animation_list.h \
	bsg_animation_sequence.h \
	bsg_animator.h \
	bsg_application.h \
	bsg_application_options.h \
	bsg_axis.h \
	bsg_bound.h \
	bsg_box.h \
	bsg_camera.h \
	bsg_carousel.h \
	bsg_catmullrom.h \
	bsg_circle.h \
	bsg_circular_index.h \
	bsg_color.h \
	bsg_compiler_quirks.h \
	bsg_constraint.h \
	bsg_context.h \
	bsg_cube_images.h \
	bsg_dither.h \
	bsg_effect.h \
	bsg_effect_generator.h \
	bsg_effect_semantics.h \
	bsg_exception.h \
	bsg_fbo.h \
	bsg_font.h \
	bsg_font_texture_packer.h \
	bsg_gather_visitor.h \
	bsg_geometry.h \
	bsg_gl.h \
	bsg_gl_intercept.h \
	bsg_gl_buffer.h \
	bsg_gl_program.h \
	bsg_gl_texture.h \
	bsg_gl_uniform.h \
	bsg_hud.h \
	bsg_image.h \
	bsg_image_astc.h \
	bsg_image_pkm.h \
	bsg_image_png.h \
	bsg_image_raw.h \
	bsg_key_events.h \
	bsg_mouse_events.h \
	bsg_library.h \
	bsg_lod_group.h \
	bsg_lod_range.h \
	bsg_material.h \
	bsg_math.h \
	bsg_matrix.h \
	bsg_maybe.h \
	bsg_no_copy.h \
	bsg_number.h \
	bsg_obj_reader.h \
	bsg_parse_utils.h \
	bsg_pass.h \
	bsg_pass_state.h \
	bsg_plane.h \
	bsg_platform.h \
	bsg_print.h \
	bsg_quaternion.h \
	bsg_render_options.h \
	bsg_sampler_semantics.h \
	bsg_scene_node.h \
	bsg_scene_node_callback.h \
	bsg_semantic_data.h \
	bsg_surface.h \
	bsg_text.h \
	bsg_time.h \
	bsg_trackers.h \
	bsg_transform.h \
	bsg_vector.h \
	bsg_video.h \
	bsg_video_decoder.h \
	bsg_visitor.h \
	bsg_fonts.h \
	bsg_utf8.h \
	bsg_task.h \
	bsg_shape.h \
	bsg_quad_render.h \
	bsg_circle.h \
	bsg_glapi.h \
	bsg_pixel.h

INC_FILES += \
	bsg_common.inc
