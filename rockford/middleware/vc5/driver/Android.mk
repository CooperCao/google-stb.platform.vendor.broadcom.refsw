# Copyright (C) 2016 Broadcom Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
V3D_DRIVER_TOP := $(ROCKFORD_TOP)/middleware/vc5
LOCAL_PATH := $(V3D_DRIVER_TOP)

LOCAL_C_INCLUDES := \
	$(ROCKFORD_TOP)/middleware/vc5/driver/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/interface/khronos/include/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/vcos/pthreads/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/vcos/generic/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/vcos/include/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/vcos/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/v3d_platform/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/v3d_platform/bcg_abstract \
	$(ROCKFORD_TOP)/middleware/vc5/driver/v3d_platform/bcg_abstract/egl \
	$(ROCKFORD_TOP)/middleware/vc5/driver/middleware/khronos/glsl/ \
	$(ROCKFORD_TOP)/middleware/vc5/platform/android \
	$(ROCKFORD_TOP)/middleware/vc5/platform/common \
	${ANDROID_TOP}/system/core/libsync/include

LOCAL_CFLAGS := \
	-fpic -DPIC \
	-std=c99 \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DV3D_TECH_VERSION=3 \
	-DV3D_REVISION=2 \
	-DGFX_DEFAULT_UIF_PAGE_SIZE=4096 \
	-DGFX_DEFAULT_UIF_NUM_BANKS=8 \
	-DGFX_DEFAULT_UIF_XOR_ADDR=16 \
	-DGFX_DEFAULT_DRAM_MAP_MODE=2 \
	-DEGL_SERVER_SMALLINT \
	-DKHRN_LIBRARY_INIT \
	-DKHRN_GLES31_DRIVER=0 \
	-DGLSL_310_SUPPORT=0 \
	-DV3D_PLATFORM_SIM=0

LOCAL_CFLAGS += ${V3D_ANDROID_DEFINES}
LOCAL_LDFLAGS := ${V3D_ANDROID_LD}

PROFILING = 0
# Set FULL_DEBUG to build without the optimizer
V3D_FULL_DEBUG = n

ifeq ($(V3D_FULL_DEBUG),y)

# Show a BIG warning about debug mode
$(info ****************************************************)
$(info *****          D E B U G   B U I L D)
$(info *****)
$(info ***** You are building the V3D driver in debug mode.)
$(info ***** This will have a LARGE impact on performance.)
$(info ***** You must build in release mode for correct)
$(info ***** performance of the V3D driver.)
$(info ****************************************************)

ifneq ($(PROFILING),0)
LOCAL_CFLAGS += -Os -DNDEBUG
else
LOCAL_CFLAGS += -O0
endif

LOCAL_CFLAGS += -g -funwind-tables
LOCAL_LDFLAGS += -g -export-dynamic

else

ifeq ($(V3D_DEBUG),y)

# V3D_DEBUG=y : In Android this is set for eng and userdebug builds by default.
# We interpret this to mean add debug information, but still optimise.
# Set V3D_FULL_DEBUG=y if you want full, unoptimised debug data.

LOCAL_CFLAGS += -Os -DNDEBUG -g -funwind-tables
LOCAL_LDFLAGS += -g -export-dynamic

else

# Full release build - no debug info

LOCAL_CFLAGS += -Os -DNDEBUG

ifeq ($(PROFILING),0)
LOCAL_CFLAGS += -fvisibility=hidden
# Strip
LOCAL_LDFLAGS += -s
else
LOCAL_CFLAGS += -g
LOCAL_LDFLAGS += -g -export-dynamic
endif

endif
endif

# Remove unwanted warnings
LOCAL_CFLAGS += \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-Wno-override-init \
	-Wno-sign-compare \
	-Wno-pointer-arith \
	-Wno-clobbered \
	-Wno-error=address

LOCAL_SRC_FILES := \
	driver/interface/khronos/common/khrn_client_pointermap.c \
	driver/interface/khronos/common/khrn_int_hash.c \
	driver/interface/khronos/common/khrn_int_util.c \
	driver/interface/khronos/common/khrn_options.c \
	driver/interface/khronos/glxx/gl11_client.c \
	driver/interface/khronos/glxx/glxx_client_skin.c \
	driver/interface/khronos/tools/dglenum/dglenum.c \
	driver/middleware/khronos/common/khrn_fmem.c \
	driver/middleware/khronos/common/khrn_fmem_pool.c \
	driver/middleware/khronos/common/khrn_fmem_debug_info.c \
	driver/middleware/khronos/common/khrn_counters.c \
	driver/middleware/khronos/common/khrn_event_monitor.c \
	driver/middleware/khronos/common/khrn_render_state.c \
	driver/middleware/khronos/common/khrn_blob.c \
	driver/middleware/khronos/common/khrn_image.c \
	driver/middleware/khronos/common/khrn_image_plane.c \
	driver/middleware/khronos/common/khrn_interlock.c \
	driver/middleware/khronos/common/khrn_map.c \
	driver/middleware/khronos/common/khrn_res_interlock.c \
	driver/middleware/khronos/common/khrn_mem.c \
	driver/middleware/khronos/common/khrn_process.c \
	driver/middleware/khronos/common/khrn_process_debug.c \
	driver/middleware/khronos/common/khrn_tformat.c \
	driver/middleware/khronos/common/khrn_uintptr_vector.c \
	driver/middleware/khronos/common/khrn_synclist.c \
	driver/middleware/khronos/common/khrn_fence.c \
	driver/middleware/khronos/common/khrn_timeline.c \
	driver/middleware/khronos/common/khrn_record.c \
	driver/middleware/khronos/egl/egl_attrib_list.c \
	driver/middleware/khronos/egl/egl_config.c \
	driver/middleware/khronos/egl/egl_context_base.c \
	driver/middleware/khronos/egl/egl_context.c \
	driver/middleware/khronos/egl/egl_context_gl.c \
	driver/middleware/khronos/egl/egl_display.c \
	driver/middleware/khronos/egl/egl_map.c \
	driver/middleware/khronos/egl/egl_pbuffer_surface.c \
	driver/middleware/khronos/egl/egl_process.c \
	driver/middleware/khronos/egl/egl_surface_base.c \
	driver/middleware/khronos/egl/egl_surface.c \
	driver/middleware/khronos/egl/egl_thread.c \
	driver/middleware/khronos/egl/egl_image.c \
	driver/middleware/khronos/egl/egl_image_texture.c \
	driver/middleware/khronos/egl/egl_image_renderbuffer.c \
	driver/middleware/khronos/egl/egl_image_framebuffer.c \
	driver/middleware/khronos/egl/egl_sync.c \
	driver/middleware/khronos/egl/egl_synckhr.c \
	driver/middleware/khronos/egl/egl_proc_address.c \
	driver/middleware/khronos/egl/egl_platform.c \
	driver/middleware/khronos/ext/egl_brcm_perf_counters.c \
	driver/middleware/khronos/ext/egl_brcm_event_monitor.c \
	driver/middleware/khronos/ext/gl_brcm_base_instance.c \
	driver/middleware/khronos/ext/gl_brcm_multi_draw_indirect.c \
	driver/middleware/khronos/ext/gl_brcm_texture_1d.c \
	driver/middleware/khronos/ext/gl_brcm_polygon_mode.c \
	driver/middleware/khronos/ext/gl_brcm_provoking_vertex.c \
	driver/middleware/khronos/ext/gl_ext_draw_elements_base_vertex.c \
	driver/middleware/khronos/ext/gl_ext_robustness.c \
	driver/middleware/khronos/ext/gl_oes_draw_texture.c \
	driver/middleware/khronos/ext/gl_oes_map_buffer.c \
	driver/middleware/khronos/ext/gl_oes_matrix_palette.c \
	driver/middleware/khronos/ext/gl_oes_query_matrix.c \
	driver/middleware/khronos/ext/gl_khr_debug.c \
	driver/middleware/khronos/ext/gl_khr_debug_msgs.c \
	driver/middleware/khronos/gl11/gl11_shader.c \
	driver/middleware/khronos/gl11/gl11_vshader.c \
	driver/middleware/khronos/gl11/gl11_fshader.c \
	driver/middleware/khronos/gl11/gl11_shadercache.c \
	driver/middleware/khronos/gl11/gl11_matrix.c \
	driver/middleware/khronos/gl11/gl11_server.c \
	driver/middleware/khronos/gl11/gl11_draw.c \
	driver/middleware/khronos/gl11/gl_oes_framebuffer_object.c \
	driver/middleware/khronos/gl20/gl20_program.c \
	driver/middleware/khronos/gl20/gl20_server.c \
	driver/middleware/khronos/gl20/gl20_shader.c \
	driver/middleware/khronos/glsl/glsl_unique_index_queue.c \
	driver/middleware/khronos/glsl/glsl_uniform_layout.c \
	driver/middleware/khronos/glsl/glsl_symbols.c \
	driver/middleware/khronos/glsl/glsl_symbol_table.c \
	driver/middleware/khronos/glsl/glsl_stringbuilder.c \
	driver/middleware/khronos/glsl/glsl_stackmem.c \
	driver/middleware/khronos/glsl/glsl_source.c \
	driver/middleware/khronos/glsl/glsl_shader_interfaces.c \
	driver/middleware/khronos/glsl/glsl_scoped_map.c \
	driver/middleware/khronos/glsl/glsl_safemem.c \
	driver/middleware/khronos/glsl/glsl_program.c \
	driver/middleware/khronos/glsl/glsl_precision.c \
	driver/middleware/khronos/glsl/glsl_map.c \
	driver/middleware/khronos/glsl/glsl_layout.c \
	driver/middleware/khronos/glsl/glsl_layout.auto.c \
	driver/middleware/khronos/glsl/glsl_ir_shader.c \
	driver/middleware/khronos/glsl/glsl_ir_program.c \
	driver/middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c \
	driver/middleware/khronos/glsl/glsl_intrinsic_ir_builder.c \
	driver/middleware/khronos/glsl/glsl_intrinsic.c \
	driver/middleware/khronos/glsl/glsl_intern.c \
	driver/middleware/khronos/glsl/glsl_globals.c \
	driver/middleware/khronos/glsl/glsl_file_utils.c \
	driver/middleware/khronos/glsl/glsl_fastmem.c \
	driver/middleware/khronos/glsl/glsl_extensions.c \
	driver/middleware/khronos/glsl/glsl_errors.c \
	driver/middleware/khronos/glsl/glsl_dataflow_visitor.c \
	driver/middleware/khronos/glsl/glsl_dataflow_simplify.c \
	driver/middleware/khronos/glsl/glsl_dataflow_print.c \
	driver/middleware/khronos/glsl/glsl_dataflow_builder.c \
	driver/middleware/khronos/glsl/glsl_dataflow_cse.c \
	driver/middleware/khronos/glsl/glsl_dataflow.c \
	driver/middleware/khronos/glsl/glsl_linker.c \
	driver/middleware/khronos/glsl/glsl_compiler.c \
	driver/middleware/khronos/glsl/glsl_check.c \
	driver/middleware/khronos/glsl/glsl_builders.c \
	driver/middleware/khronos/glsl/glsl_binary_shader.c \
	driver/middleware/khronos/glsl/glsl_binary_program.c \
	driver/middleware/khronos/glsl/glsl_backflow_visitor.c \
	driver/middleware/khronos/glsl/glsl_backflow_print.c \
	driver/middleware/khronos/glsl/glsl_backflow.c \
	driver/middleware/khronos/glsl/glsl_backflow_combine.c \
	driver/middleware/khronos/glsl/glsl_backend.c \
	driver/middleware/khronos/glsl/glsl_ast_visitor.c \
	driver/middleware/khronos/glsl/glsl_ast_print.c \
	driver/middleware/khronos/glsl/glsl_ast.c \
	driver/middleware/khronos/glsl/glsl_arenamem.c \
	driver/middleware/khronos/glsl/glsl_alloc_tracker.c \
	driver/middleware/khronos/glsl/glsl_stdlib.auto.c \
	driver/middleware/khronos/glsl/glsl_primitive_types.auto.c \
	driver/middleware/khronos/glsl/glsl_basic_block.c \
	driver/middleware/khronos/glsl/glsl_basic_block_builder.c \
	driver/middleware/khronos/glsl/glsl_basic_block_flatten.c \
	driver/middleware/khronos/glsl/glsl_basic_block_print.c \
	driver/middleware/khronos/glsl/glsl_nast.c \
	driver/middleware/khronos/glsl/glsl_nast_builder.c \
	driver/middleware/khronos/glsl/glsl_nast_print.c \
	driver/middleware/khronos/glsl/glsl_dominators.c \
	driver/middleware/khronos/glsl/glsl_scheduler.c \
	driver/middleware/khronos/glsl/glsl_quals.c \
	driver/middleware/khronos/glsl/glsl_parser.c \
	driver/middleware/khronos/glsl/glsl_lexer.c \
	driver/middleware/khronos/glsl/glsl_numbers.c \
	driver/middleware/khronos/glsl/glsl_version.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_directive.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_eval.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_expand.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_token.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_macro.c \
	driver/middleware/khronos/glxx/gl31_stubs.c \
	driver/middleware/khronos/glxx/gl32_stubs.c \
	driver/middleware/khronos/glxx/glxx_hw.c \
	driver/middleware/khronos/glxx/glxx_inner.c \
	driver/middleware/khronos/glxx/glxx_shader_ops.c \
	driver/middleware/khronos/glxx/glxx_shader.c \
	driver/middleware/khronos/glxx/glxx_hw_tile_list.c \
	driver/middleware/khronos/glxx/glxx_hw_clear.c \
	driver/middleware/khronos/glxx/glxx_hw_render_state.c \
	driver/middleware/khronos/glxx/glxx_buffer.c \
	driver/middleware/khronos/glxx/glxx_ds_to_color.c \
	driver/middleware/khronos/glxx/glxx_draw.c \
	driver/middleware/khronos/glxx/glxx_extensions.c \
	driver/middleware/khronos/glxx/glxx_framebuffer.c \
	driver/middleware/khronos/glxx/glxx_renderbuffer.c \
	driver/middleware/khronos/glxx/glxx_server.c \
	driver/middleware/khronos/glxx/glxx_server_debug.c \
	driver/middleware/khronos/glxx/glxx_textures.c \
	driver/middleware/khronos/glxx/glxx_server_texture.c \
	driver/middleware/khronos/glxx/glxx_server_buffer.c \
	driver/middleware/khronos/glxx/glxx_query.c \
	driver/middleware/khronos/glxx/glxx_server_query.c \
	driver/middleware/khronos/glxx/glxx_server_get.c \
	driver/middleware/khronos/glxx/glxx_server_framebuffer.c \
	driver/middleware/khronos/glxx/glxx_server_transform_feedback.c \
	driver/middleware/khronos/glxx/glxx_server_vao.c \
	driver/middleware/khronos/glxx/glxx_server_sampler.c \
	driver/middleware/khronos/glxx/glxx_server_sync.c \
	driver/middleware/khronos/glxx/glxx_server_program_interface.c \
	driver/middleware/khronos/glxx/glxx_shader_cache.c \
	driver/middleware/khronos/glxx/glxx_shared.c \
	driver/middleware/khronos/glxx/glxx_compressed_paletted_texture.c \
	driver/middleware/khronos/glxx/glxx_texture.c \
	driver/middleware/khronos/glxx/glxx_texture_utils.c \
	driver/middleware/khronos/glxx/glxx_log.c \
	driver/middleware/khronos/glxx/glxx_fencesync.c \
	driver/middleware/khronos/glxx/glxx_tmu_blit.c \
	driver/middleware/khronos/glxx/glxx_tlb_blit.c \
	driver/middleware/khronos/glxx/glxx_utils.c \
	driver/middleware/khronos/glxx/glxx_compute.c \
	driver/middleware/khronos/glxx/glxx_texlevel_param.c \
	driver/helpers/desc_map/desc_map.c \
	driver/helpers/gfx/gfx_util_morton.c \
	driver/helpers/gfx/gfx_util_hrsize.c \
	driver/helpers/gfx/gfx_util_file.c \
	driver/helpers/gfx/gfx_util.c \
	driver/helpers/gfx/gfx_options.c \
	driver/helpers/gfx/gfx_lfmt_translate_v3d.c \
	driver/helpers/gfx/gfx_lfmt_translate_gl.c \
	driver/helpers/gfx/gfx_lfmt_fmt_detail.c \
	driver/helpers/gfx/gfx_lfmt_desc.c \
	driver/helpers/gfx/gfx_lfmt_desc_maps.c \
	driver/helpers/gfx/gfx_lfmt.c \
	driver/helpers/gfx/gfx_ez.c \
	driver/helpers/gfx/gfx_bufstate.c \
	driver/helpers/gfx/gfx_buffer_v3d_tfu_srgb_conversions.c \
	driver/helpers/gfx/gfx_buffer_uif_config.c \
	driver/helpers/gfx/gfx_buffer_translate_v3d.c \
	driver/helpers/gfx/gfx_buffer_compare.c \
	driver/helpers/gfx/gfx_buffer_bc.c \
	driver/helpers/gfx/gfx_buffer.c \
	driver/helpers/gfx/gfx_buffer_bstc.c \
	driver/helpers/gfx/gfx_buffer_desc_gen.c \
	driver/helpers/gfx/gfx_buffer_slow_conv_compr.c \
	driver/helpers/gfx/gfx_buffer_slow_conv_xform.c \
	driver/helpers/gfx/gfx_lfmt_block.c \
	driver/helpers/gfx/gfx_args.c \
	driver/helpers/gfx/gfx_buffer_slow_conv.c \
	driver/helpers/v3d/v3d_cl.c \
	driver/helpers/v3d/v3d_gen.c \
	driver/helpers/v3d/v3d_printer.c \
	driver/helpers/v3d/v3d_cl_compr.c \
	driver/helpers/v3d/v3d_config.c \
	driver/helpers/v3d/v3d_tfu.c \
	driver/helpers/v3d/v3d_tmu.c \
	driver/v3d_platform/v3d_imgconv.c \
	driver/v3d_platform/v3d_imgconv_c.c \
	driver/v3d_platform/v3d_imgconv_neon.c \
	driver/v3d_platform/v3d_imgconv_extra_neon.c \
	driver/v3d_platform/v3d_imgconv_gfx_blit.c \
	driver/v3d_platform/v3d_scheduler.c \
	driver/v3d_platform/v3d_imgconv_tfu.c \
	driver/v3d_platform/v3d_imgconv_tlb.c \
	driver/v3d_platform/v3d_parallel.c \
	driver/v3d_platform/bcg_abstract/gmem_abstract.c \
	driver/v3d_platform/bcg_abstract/gmem_talloc.c \
	driver/v3d_platform/bcg_abstract/sched_abstract.c \
	driver/v3d_platform/bcg_abstract/egl/egl_platform_abstract.c \
	driver/v3d_platform/bcg_abstract/egl/egl_window_surface_abstract.c \
	driver/v3d_platform/bcg_abstract/egl/egl_surface_common_abstract.c \
	driver/v3d_platform/bcg_abstract/egl/egl_pixmap_surface_abstract.c \
	driver/v3d_platform/bcg_abstract/egl/egl_image_native_buffer_abstract.c \
	driver/vcos/generic/vcos_logcat.c \
	driver/vcos/generic/vcos_properties.c \
	driver/vcos/generic/vcos_init.c \
	driver/vcos/generic/vcos_generic_safe_string.c \
	driver/vcos/pthreads/vcos_pthreads.c \
	driver/tools/v3d/qpu_float/qpu_float.c \
	driver/tools/v3d/sfu/sfu.c \
	platform/android/default_android.c \
	platform/android/display_android.c \
	platform/android/memory_android.c \
	platform/android/egl_native_fence_sync_android.c \
	platform/android/sched_android.c \
	platform/common/sched_nexus.c \
	driver/android_platform_library_loader.c

GENERATED_SRC_FILES := \
	driver/middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c \
	driver/middleware/khronos/glsl/glsl_layout.auto.c \
	driver/middleware/khronos/glsl/glsl_lexer.c \
	driver/middleware/khronos/glsl/glsl_numbers.c \
	driver/middleware/khronos/glsl/glsl_parser.c \
	driver/middleware/khronos/glsl/glsl_primitive_types.auto.c \
	driver/middleware/khronos/glsl/glsl_version.c \
	driver/middleware/khronos/glsl/glsl_stdlib.auto.c

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libGLES_nexus
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
intermediates := $(call local-intermediates-dir)
LOCAL_SRC_FILES := $(filter-out $(GENERATED_SRC_FILES), $(LOCAL_SRC_FILES))
GENERATED_SRC_FILES := $(addprefix $(intermediates)/, $(GENERATED_SRC_FILES))
LOCAL_GENERATED_SOURCES := $(GENERATED_SRC_FILES)
GENERATED_SRC_DIR := $(ANDROID_TOP)/$(intermediates)
LOCAL_C_INCLUDES += \
	$(intermediates)/driver/interface/khronos/tools/dglenum \
	$(intermediates)/driver/middleware/khronos/glsl

glsl_primitive_types_deps := \
	$(LOCAL_PATH)/driver/middleware/khronos/glsl/scripts/build_primitive_types.py \
	$(LOCAL_PATH)/driver/middleware/khronos/glsl/scripts/scalar_types.table \
	$(LOCAL_PATH)/driver/middleware/khronos/glsl/scripts/sampler_types.table \
	$(LOCAL_PATH)/driver/middleware/khronos/glsl/scripts/image_types.table

define glsl_primitive_types_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl
$(hide) pushd $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/build_primitive_types.py -I . -O . \
		$(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/scalar_types.table \
		$(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/sampler_types.table \
		$(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/image_types.table && \
	popd
endef

define generated_src_dir_exists
@mkdir -p $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl
@mkdir -p $(GENERATED_SRC_DIR)/driver/interface/khronos/tools/dglenum
endef

$(intermediates)/driver/middleware/khronos/glsl/glsl_primitive_types.auto.c \
$(intermediates)/driver/middleware/khronos/glsl/glsl_primitive_types.auto.h \
$(intermediates)/driver/middleware/khronos/glsl/glsl_primitive_types.auto.table \
$(intermediates)/driver/middleware/khronos/glsl/glsl_primitive_type_index.auto.h : $(glsl_primitive_types_deps)
	$(glsl_primitive_types_gen)

$(intermediates)/driver/middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c : $(LOCAL_PATH)/driver/middleware/khronos/glsl/glsl_intrinsic_lookup.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_intrinsic_lookup.gperf > $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c

$(intermediates)/driver/middleware/khronos/glsl/glsl_layout.auto.c : $(LOCAL_PATH)/driver/middleware/khronos/glsl/glsl_layout.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_layout.gperf > $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_layout.auto.c


define textures_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl
$(hide)	pushd $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/build_texture_functions.py && \
	popd
endef

$(intermediates)/driver/middleware/khronos/glsl/textures.auto.glsl \
$(intermediates)/driver/middleware/khronos/glsl/textures.auto.props : $(LOCAL_PATH)/driver/middleware/khronos/glsl/scripts/build_texture_functions.py
	$(textures_auto_gen)

# sources for stdlib from source tree.
STDLIB_SOURCES := \
	stdlib/common.glsl \
	stdlib/derivatives.glsl \
	stdlib/exponential.glsl \
	stdlib/geometry.glsl \
	stdlib/hyperbolic.glsl \
	stdlib/matrix.glsl \
	stdlib/packing.glsl \
	stdlib/synchronisation.glsl \
	stdlib/trigonometry.glsl \
	stdlib/vector_relational.glsl \
	stdlib/common.inl \
	stdlib/derivatives.inl \
	stdlib/exponential.inl \
	stdlib/geometry.inl \
	stdlib/hyperbolic.inl \
	stdlib/matrix.inl \
	stdlib/packing.inl \
	stdlib/trigonometry.inl \
	stdlib/vector_relational.inl \
	stdlib/texture.glsl \
	stdlib/globals.glsl \
	stdlib/fragment_only.props \
	stdlib/vertex_only.props \
	stdlib/compute_only.props \
	stdlib/v100_only.props \
	stdlib/v300_only.props \
	stdlib/v310_only.props

STDLIB_SOURCES := $(addprefix $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/, $(STDLIB_SOURCES))

# sources for stdlib from generated tree (no prefix since already in the right location).
STDLIB_SOURCES_EXTRA := \
	textures.auto.glsl \
	textures.auto.props \
	glsl_primitive_types.auto.table

# full set of sources for stdlib.
STDLIB_SOURCES += $(STDLIB_SOURCES_EXTRA)

glsl_stdlib_deps := \
	$(intermediates)/driver/middleware/khronos/glsl/textures.auto.glsl \
	$(intermediates)/driver/middleware/khronos/glsl/glsl_primitive_types.auto.table

define glsl_stdlib_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl
$(hide) pushd $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/scripts/build_stdlib.py -O . -I . $(STDLIB_SOURCES) && \
	popd

endef

$(intermediates)/driver/middleware/khronos/glsl/glsl_stdlib.auto.h \
$(intermediates)/driver/middleware/khronos/glsl/glsl_stdlib.auto.c : $(glsl_stdlib_deps)
	$(glsl_stdlib_auto_gen)

$(intermediates)/driver/middleware/khronos/glsl/glsl_parser.h \
$(intermediates)/driver/middleware/khronos/glsl/glsl_parser.c \
$(intermediates)/driver/middleware/khronos/glsl/glsl_parser.output : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_parser.y
	$(generated_src_dir_exists)
	bison -d -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_parser.c $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_parser.y

$(intermediates)/driver/middleware/khronos/glsl/glsl_lexer.c : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_lexer.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_lexer.c --never-interactive $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_lexer.l

$(intermediates)/driver/middleware/khronos/glsl/glsl_numbers.c : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_numbers.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_numbers.c --never-interactive $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_numbers.l

$(intermediates)/driver/middleware/khronos/glsl/glsl_version.c : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_version.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/glsl_version.c --never-interactive $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_version.l

dglenum_gen_deps := \
	$(LOCAL_PATH)/driver/interface/khronos/tools/dglenum/dglenum_gen.py \
	$(LOCAL_PATH)/driver/interface/khronos/include/GLES3/gl3.h \
	$(LOCAL_PATH)/driver/interface/khronos/include/GLES3/gl3ext_brcm.h

$(intermediates)/driver/interface/khronos/tools/dglenum/dglenum_gen.c : $(dglenum_gen_deps)
	$(generated_src_dir_exists)
	pushd $(V3D_DRIVER_TOP)/driver && python interface/khronos/tools/dglenum/dglenum_gen.py > $(GENERATED_SRC_DIR)/driver/interface/khronos/tools/dglenum/dglenum_gen.c && popd

$(LOCAL_PATH)/driver/interface/khronos/tools/dglenum/dglenum.c : $(intermediates)/driver/interface/khronos/tools/dglenum/dglenum_gen.c

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libnexus
LOCAL_SHARED_LIBRARIES += libnexuseglclient
LOCAL_SHARED_LIBRARIES += libnxclient
LOCAL_SHARED_LIBRARIES += libsync

ifeq ($(TARGET_2ND_ARCH),arm)
LOCAL_MODULE_RELATIVE_PATH := egl
else
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
endif

include $(BUILD_SHARED_LIBRARY)
