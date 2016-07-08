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
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/core/vcos/pthreads/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/core/vcos/generic/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/core/vcos/include/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/core/vcos/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/khrn/egl/platform/bcg_abstract/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/platform/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/platform/bcg_abstract/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/khrn/egl/bcg_abstract/egl/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/khrn/glsl/ \
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/khrn/include/ \
	$(ROCKFORD_TOP)/middleware/vc5/platform/android/ \
	$(ROCKFORD_TOP)/middleware/vc5/platform/common/ \
	${ANDROID_TOP}/system/core/libsync/include/ \
	$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)

LOCAL_CFLAGS := \
	-fpic -DPIC \
	-std=c99 \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DGFX_DEFAULT_UIF_PAGE_SIZE=4096 \
	-DGFX_DEFAULT_UIF_NUM_BANKS=8 \
	-DGFX_DEFAULT_UIF_XOR_ADDR=16 \
	-DGFX_DEFAULT_DRAM_MAP_MODE=2 \
	-DEGL_SERVER_SMALLINT \
	-DEMBEDDED_SETTOP_BOX=1 \
	-DKHRN_LIBRARY_INIT \
	-DKHRN_GLES31_DRIVER=1 \
	-DKHRN_GLES32_DRIVER=0 \
	-DGLSL_310_SUPPORT=1 \
	-DV3D_PLATFORM_SIM=0 \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable

ifeq ($(TARGET_2ND_ARCH),arm)
LOCAL_CFLAGS_arm64 += ${V3D_ANDROID_DEFINES_1ST_ARCH}
LOCAL_LDFLAGS_arm64 := ${V3D_ANDROID_LD_1ST_ARCH}
LOCAL_CFLAGS_arm += ${V3D_ANDROID_DEFINES_2ND_ARCH}
LOCAL_LDFLAGS_arm := ${V3D_ANDROID_LD_2ND_ARCH}
else
LOCAL_CFLAGS_arm += ${V3D_ANDROID_DEFINES_1ST_ARCH}
LOCAL_LDFLAGS_arm := ${V3D_ANDROID_LD_1ST_ARCH}
endif

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
	-Wno-clobbered

LOCAL_SRC_FILES := \
	driver/libs/khrn/common/khrn_int_hash.c \
	driver/libs/khrn/common/khrn_options.c \
	driver/libs/khrn/glxx/glxx_client_skin.c \
	driver/libs/util/dglenum/dglenum.c \
	driver/libs/util/log/log.c \
	driver/libs/khrn/common/khrn_fmem.c \
	driver/libs/khrn/common/khrn_fmem_pool.c \
	driver/libs/khrn/common/khrn_fmem_debug_info.c \
	driver/libs/khrn/common/khrn_counters.c \
	driver/libs/khrn/common/khrn_event_monitor.c \
	driver/libs/khrn/common/khrn_render_state.c \
	driver/libs/khrn/common/khrn_blob.c \
	driver/libs/khrn/common/khrn_image.c \
	driver/libs/khrn/common/khrn_image_plane.c \
	driver/libs/khrn/common/khrn_interlock.c \
	driver/libs/khrn/common/khrn_map.c \
	driver/libs/khrn/common/khrn_res_interlock.c \
	driver/libs/khrn/common/khrn_mem.c \
	driver/libs/khrn/common/khrn_process.c \
	driver/libs/khrn/common/khrn_process_debug.c \
	driver/libs/khrn/common/khrn_tformat.c \
	driver/libs/khrn/common/khrn_uintptr_vector.c \
	driver/libs/khrn/common/khrn_synclist.c \
	driver/libs/khrn/common/khrn_fence.c \
	driver/libs/khrn/common/khrn_timeline.c \
	driver/libs/khrn/common/khrn_record.c \
	driver/libs/khrn/egl/egl_attrib_list.c \
	driver/libs/khrn/egl/egl_config.c \
	driver/libs/khrn/egl/egl_context_base.c \
	driver/libs/khrn/egl/egl_context.c \
	driver/libs/khrn/egl/egl_context_gl.c \
	driver/libs/khrn/egl/egl_display.c \
	driver/libs/khrn/egl/egl_map.c \
	driver/libs/khrn/egl/egl_pbuffer_surface.c \
	driver/libs/khrn/egl/egl_process.c \
	driver/libs/khrn/egl/egl_surface_base.c \
	driver/libs/khrn/egl/egl_surface.c \
	driver/libs/khrn/egl/egl_thread.c \
	driver/libs/khrn/egl/egl_image.c \
	driver/libs/khrn/egl/egl_image_texture.c \
	driver/libs/khrn/egl/egl_image_renderbuffer.c \
	driver/libs/khrn/egl/egl_image_framebuffer.c \
	driver/libs/khrn/egl/egl_sync.c \
	driver/libs/khrn/egl/egl_synckhr.c \
	driver/libs/khrn/egl/egl_proc_address.c \
	driver/libs/khrn/egl/egl_platform.c \
	driver/libs/khrn/ext/egl_brcm_perf_counters.c \
	driver/libs/khrn/ext/egl_brcm_event_monitor.c \
	driver/libs/khrn/ext/gl_brcm_base_instance.c \
	driver/libs/khrn/ext/gl_brcm_multi_draw_indirect.c \
	driver/libs/khrn/ext/gl_brcm_texture_1d.c \
	driver/libs/khrn/ext/gl_brcm_polygon_mode.c \
	driver/libs/khrn/ext/gl_brcm_provoking_vertex.c \
	driver/libs/khrn/ext/gl_ext_draw_elements_base_vertex.c \
	driver/libs/khrn/ext/gl_ext_robustness.c \
	driver/libs/khrn/ext/gl_oes_draw_texture.c \
	driver/libs/khrn/ext/gl_oes_map_buffer.c \
	driver/libs/khrn/ext/gl_oes_matrix_palette.c \
	driver/libs/khrn/ext/gl_oes_query_matrix.c \
	driver/libs/khrn/ext/gl_khr_debug.c \
	driver/libs/khrn/ext/gl_khr_debug_msgs.c \
	driver/libs/khrn/gl11/gl11_client.c \
	driver/libs/khrn/gl11/gl11_shader.c \
	driver/libs/khrn/gl11/gl11_vshader.c \
	driver/libs/khrn/gl11/gl11_fshader.c \
	driver/libs/khrn/gl11/gl11_shadercache.c \
	driver/libs/khrn/gl11/gl11_matrix.c \
	driver/libs/khrn/gl11/gl11_server.c \
	driver/libs/khrn/gl11/gl11_draw.c \
	driver/libs/khrn/gl11/gl_oes_framebuffer_object.c \
	driver/libs/khrn/gl20/gl20_program.c \
	driver/libs/khrn/gl20/gl20_server.c \
	driver/libs/khrn/gl20/gl20_shader.c \
	driver/libs/khrn/glsl/glsl_unique_index_queue.c \
	driver/libs/khrn/glsl/glsl_uniform_layout.c \
	driver/libs/khrn/glsl/glsl_symbols.c \
	driver/libs/khrn/glsl/glsl_symbol_table.c \
	driver/libs/khrn/glsl/glsl_stringbuilder.c \
	driver/libs/khrn/glsl/glsl_stackmem.c \
	driver/libs/khrn/glsl/glsl_source.c \
	driver/libs/khrn/glsl/glsl_shader_interfaces.c \
	driver/libs/khrn/glsl/glsl_scoped_map.c \
	driver/libs/khrn/glsl/glsl_safemem.c \
	driver/libs/khrn/glsl/glsl_program.c \
	driver/libs/khrn/glsl/glsl_precision.c \
	driver/libs/khrn/glsl/glsl_map.c \
	driver/libs/khrn/glsl/glsl_layout.c \
	driver/libs/khrn/glsl/glsl_layout.auto.c \
	driver/libs/khrn/glsl/glsl_ir_shader.c \
	driver/libs/khrn/glsl/glsl_ir_program.c \
	driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c \
	driver/libs/khrn/glsl/glsl_intrinsic_ir_builder.c \
	driver/libs/khrn/glsl/glsl_intrinsic.c \
	driver/libs/khrn/glsl/glsl_intern.c \
	driver/libs/khrn/glsl/glsl_globals.c \
	driver/libs/khrn/glsl/glsl_file_utils.c \
	driver/libs/khrn/glsl/glsl_fastmem.c \
	driver/libs/khrn/glsl/glsl_extensions.c \
	driver/libs/khrn/glsl/glsl_errors.c \
	driver/libs/khrn/glsl/glsl_dataflow_visitor.c \
	driver/libs/khrn/glsl/glsl_dataflow_simplify.c \
	driver/libs/khrn/glsl/glsl_dataflow_print.c \
	driver/libs/khrn/glsl/glsl_dataflow_builder.c \
	driver/libs/khrn/glsl/glsl_dataflow_cse.c \
	driver/libs/khrn/glsl/glsl_dataflow.c \
	driver/libs/khrn/glsl/glsl_dataflow_image.c \
	driver/libs/khrn/glsl/glsl_linker.c \
	driver/libs/khrn/glsl/glsl_compiler.c \
	driver/libs/khrn/glsl/glsl_check.c \
	driver/libs/khrn/glsl/glsl_builders.c \
	driver/libs/khrn/glsl/glsl_binary_shader.c \
	driver/libs/khrn/glsl/glsl_binary_program.c \
	driver/libs/khrn/glsl/glsl_backflow_visitor.c \
	driver/libs/khrn/glsl/glsl_backflow_print.c \
	driver/libs/khrn/glsl/glsl_backflow.c \
	driver/libs/khrn/glsl/glsl_backflow_combine.c \
	driver/libs/khrn/glsl/glsl_backend.c \
	driver/libs/khrn/glsl/glsl_ast_visitor.c \
	driver/libs/khrn/glsl/glsl_ast_print.c \
	driver/libs/khrn/glsl/glsl_ast.c \
	driver/libs/khrn/glsl/glsl_arenamem.c \
	driver/libs/khrn/glsl/glsl_alloc_tracker.c \
	driver/libs/khrn/glsl/glsl_stdlib.auto.c \
	driver/libs/khrn/glsl/glsl_primitive_types.auto.c \
	driver/libs/khrn/glsl/glsl_basic_block.c \
	driver/libs/khrn/glsl/glsl_basic_block_builder.c \
	driver/libs/khrn/glsl/glsl_basic_block_flatten.c \
	driver/libs/khrn/glsl/glsl_basic_block_print.c \
	driver/libs/khrn/glsl/glsl_nast.c \
	driver/libs/khrn/glsl/glsl_nast_builder.c \
	driver/libs/khrn/glsl/glsl_nast_print.c \
	driver/libs/khrn/glsl/glsl_dominators.c \
	driver/libs/khrn/glsl/glsl_scheduler.c \
	driver/libs/khrn/glsl/glsl_quals.c \
	driver/libs/khrn/glsl/glsl_parser.c \
	driver/libs/khrn/glsl/glsl_lexer.c \
	driver/libs/khrn/glsl/glsl_numbers.c \
	driver/libs/khrn/glsl/glsl_version.c \
	driver/libs/khrn/glsl/prepro/glsl_prepro_directive.c \
	driver/libs/khrn/glsl/prepro/glsl_prepro_eval.c \
	driver/libs/khrn/glsl/prepro/glsl_prepro_expand.c \
	driver/libs/khrn/glsl/prepro/glsl_prepro_token.c \
	driver/libs/khrn/glsl/prepro/glsl_prepro_macro.c \
	driver/libs/khrn/glxx/gl32_stubs.c \
	driver/libs/khrn/glxx/glxx_ez.c \
	driver/libs/khrn/glxx/glxx_hw.c \
	driver/libs/khrn/glxx/glxx_inner.c \
	driver/libs/khrn/glxx/glxx_shader_ops.c \
	driver/libs/khrn/glxx/glxx_shader.c \
	driver/libs/khrn/glxx/glxx_hw_tile_list.c \
	driver/libs/khrn/glxx/glxx_hw_clear.c \
	driver/libs/khrn/glxx/glxx_hw_render_state.c \
	driver/libs/khrn/glxx/glxx_buffer.c \
	driver/libs/khrn/glxx/glxx_ds_to_color.c \
	driver/libs/khrn/glxx/glxx_draw.c \
	driver/libs/khrn/glxx/glxx_extensions.c \
	driver/libs/khrn/glxx/glxx_framebuffer.c \
	driver/libs/khrn/glxx/glxx_renderbuffer.c \
	driver/libs/khrn/glxx/glxx_server.c \
	driver/libs/khrn/glxx/glxx_server_barrier.c \
	driver/libs/khrn/glxx/glxx_server_debug.c \
	driver/libs/khrn/glxx/glxx_textures.c \
	driver/libs/khrn/glxx/glxx_server_texture.c \
	driver/libs/khrn/glxx/glxx_server_buffer.c \
	driver/libs/khrn/glxx/glxx_query.c \
	driver/libs/khrn/glxx/glxx_server_query.c \
	driver/libs/khrn/glxx/glxx_server_get.c \
	driver/libs/khrn/glxx/glxx_server_framebuffer.c \
	driver/libs/khrn/glxx/glxx_server_transform_feedback.c \
	driver/libs/khrn/glxx/glxx_server_vao.c \
	driver/libs/khrn/glxx/glxx_server_sampler.c \
	driver/libs/khrn/glxx/glxx_server_pipeline.c \
	driver/libs/khrn/glxx/glxx_server_sync.c \
	driver/libs/khrn/glxx/glxx_server_program_interface.c \
	driver/libs/khrn/glxx/glxx_shader_cache.c \
	driver/libs/khrn/glxx/glxx_shared.c \
	driver/libs/khrn/glxx/glxx_compressed_paletted_texture.c \
	driver/libs/khrn/glxx/glxx_texture.c \
	driver/libs/khrn/glxx/glxx_texture_utils.c \
	driver/libs/khrn/glxx/glxx_fencesync.c \
	driver/libs/khrn/glxx/glxx_tmu_blit.c \
	driver/libs/khrn/glxx/glxx_tlb_blit.c \
	driver/libs/khrn/glxx/glxx_utils.c \
	driver/libs/khrn/glxx/glxx_compute.c \
	driver/libs/khrn/glxx/glxx_texlevel_param.c \
	driver/libs/khrn/glxx/glxx_image_unit.c \
	driver/libs/util/desc_map/desc_map.c \
	driver/libs/util/gfx_util/gfx_util_morton.c \
	driver/libs/util/gfx_util/gfx_util_hrsize.c \
	driver/libs/util/gfx_util/gfx_util_file.c \
	driver/libs/util/gfx_util/gfx_util.c \
	driver/libs/util/gfx_util/gfx_util_term_col.c \
	driver/libs/util/gfx_util/gfx_util_wildcard.c \
	driver/libs/util/gfx_options/gfx_options.c \
	driver/libs/core/lfmt/lfmt_translate_v3d.c \
	driver/libs/core/lfmt/lfmt_fmt_detail.c \
	driver/libs/core/lfmt/lfmt.c \
	driver/libs/core/lfmt/lfmt_block.c \
	driver/libs/core/lfmt/lfmt_desc.c \
	driver/libs/core/lfmt/lfmt_desc_maps.c \
	driver/libs/core/lfmt_translate_gl/lfmt_translate_gl.c \
	driver/libs/core/gfx_buffer/gfx_buffer_v3d_tfu_srgb_conversions.c \
	driver/libs/core/gfx_buffer/gfx_buffer_uif_config.c \
	driver/libs/core/gfx_buffer/gfx_buffer_translate_v3d.c \
	driver/libs/core/gfx_buffer/gfx_buffer_compare.c \
	driver/libs/core/gfx_buffer/gfx_buffer_bc.c \
	driver/libs/core/gfx_buffer/gfx_buffer.c \
	driver/libs/core/gfx_buffer/gfx_buffer_bstc.c \
	driver/libs/core/gfx_buffer/gfx_buffer_desc_gen.c \
	driver/libs/core/gfx_buffer/gfx_buffer_slow_conv_compr.c \
	driver/libs/core/gfx_buffer/gfx_buffer_slow_conv_xform.c \
	driver/libs/core/gfx_buffer/gfx_buffer_slow_conv.c \
	driver/libs/util/gfx_args/gfx_args.c \
	driver/libs/core/v3d/v3d_cl.c \
	driver/libs/core/v3d/v3d_gen.c \
	driver/libs/core/v3d/v3d_ident.c \
	driver/libs/core/v3d/v3d_printer.c \
	driver/libs/core/v3d/v3d_cl_compr.c \
	driver/libs/core/v3d/v3d_tfu.c \
	driver/libs/core/v3d/v3d_tmu.c \
	driver/libs/platform/v3d_imgconv.c \
	driver/libs/platform/v3d_imgconv_c.c \
	driver/libs/platform/v3d_imgconv_neon.c \
	driver/libs/platform/v3d_imgconv_extra_neon.c \
	driver/libs/platform/v3d_imgconv_gfx_blit.c \
	driver/libs/platform/v3d_scheduler.c \
	driver/libs/platform/v3d_imgconv_tfu.c \
	driver/libs/platform/v3d_imgconv_tlb.c \
	driver/libs/platform/v3d_parallel.c \
	driver/libs/platform/bcg_abstract/gmem_abstract.c \
	driver/libs/platform/bcg_abstract/gmem_talloc.c \
	driver/libs/platform/bcg_abstract/sched_abstract.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_platform_abstract.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_window_surface_abstract.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_surface_common_abstract.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_pixmap_surface_abstract.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_image_native_buffer_abstract.c \
	driver/libs/core/vcos/pthreads/vcos_pthreads.c \
	driver/libs/core/vcos/generic/vcos_generic_safe_string.c \
	driver/libs/core/vcos/generic/vcos_init.c \
	driver/libs/core/vcos/generic/vcos_properties.c \
	driver/libs/sim/qpu_float/qpu_float.c \
	driver/libs/sim/sfu/sfu.c \
	platform/android/default_android.c \
	platform/android/display_android.c \
	platform/android/memory_android.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_native_fence_sync_android.c \
	platform/android/sched_android.c \
	platform/common/sched_nexus.c \
	platform/android/android_platform_library_loader.c

GENERATED_SRC_FILES := \
	driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c \
	driver/libs/khrn/glsl/glsl_layout.auto.c \
	driver/libs/khrn/glsl/glsl_lexer.c \
	driver/libs/khrn/glsl/glsl_numbers.c \
	driver/libs/khrn/glsl/glsl_parser.c \
	driver/libs/khrn/glsl/glsl_primitive_types.auto.c \
	driver/libs/khrn/glsl/glsl_version.c \
	driver/libs/khrn/glsl/glsl_stdlib.auto.c

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libGLES_nexus
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
intermediates := $(call local-generated-sources-dir)
LOCAL_SRC_FILES := $(filter-out $(GENERATED_SRC_FILES), $(LOCAL_SRC_FILES))
GENERATED_SRC_FILES := $(addprefix $(intermediates)/, $(GENERATED_SRC_FILES))
LOCAL_GENERATED_SOURCES := $(GENERATED_SRC_FILES)
GENERATED_SRC_DIR := $(ANDROID_TOP)/$(intermediates)
LOCAL_C_INCLUDES += \
	$(intermediates)/driver/libs/util/dglenum \
	$(intermediates)/driver/libs/khrn/glsl

glsl_primitive_types_deps := \
	$(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/build_primitive_types.py \
	$(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/scalar_types.table \
	$(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/sampler_types.table \
	$(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/image_types.table

define glsl_primitive_types_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
$(hide) pushd $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_primitive_types.py -I . -O . \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/scalar_types.table \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/sampler_types.table \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/image_types.table && \
	popd
endef

define generated_src_dir_exists
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/util/dglenum
endef

$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.c \
$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.h \
$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table \
$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_type_index.auto.h : $(glsl_primitive_types_deps)
	$(glsl_primitive_types_gen)

$(intermediates)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c : $(LOCAL_PATH)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.gperf > $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c

$(intermediates)/driver/libs/khrn/glsl/glsl_layout.auto.c : $(LOCAL_PATH)/driver/libs/khrn/glsl/glsl_layout.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_layout.gperf > $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_layout.auto.c


define textures_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
$(hide)	pushd $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_texture_functions.py && \
	popd
endef

$(intermediates)/driver/libs/khrn/glsl/textures.auto.glsl \
$(intermediates)/driver/libs/khrn/glsl/textures.auto.props : $(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/build_texture_functions.py
	$(textures_auto_gen)

# sources for stdlib from source tree.
STDLIB_SOURCES := \
	stdlib/atomics.glsl \
	stdlib/common.glsl \
	stdlib/derivatives.glsl \
	stdlib/exponential.glsl \
	stdlib/extensions.glsl \
	stdlib/geometry.glsl \
	stdlib/hyperbolic.glsl \
	stdlib/image.glsl \
	stdlib/integer.glsl \
	stdlib/matrix.glsl \
	stdlib/packing.glsl \
	stdlib/synchronisation.glsl \
	stdlib/texture_gather.glsl \
	stdlib/trigonometry.glsl \
	stdlib/vector_relational.glsl \
	stdlib/common.inl \
	stdlib/derivatives.inl \
	stdlib/exponential.inl \
	stdlib/geometry.inl \
	stdlib/hyperbolic.inl \
	stdlib/integer.inl \
	stdlib/matrix.inl \
	stdlib/packing.inl \
	stdlib/trigonometry.inl \
	stdlib/vector_relational.inl \
	stdlib/texture.glsl \
	stdlib/globals.glsl \
	stdlib/stages.props \
	stdlib/v100_only.props \
	stdlib/v300_only.props \
	stdlib/v310_only.props \
	stdlib/v320_only.props \
	stdlib/extensions.props \
	stdlib/restrictions.props

STDLIB_SOURCES := $(addprefix $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/, $(STDLIB_SOURCES))

# sources for stdlib from generated tree (no prefix since already in the right location).
STDLIB_SOURCES_EXTRA := \
	textures.auto.glsl \
	textures.auto.props \
	glsl_primitive_types.auto.table

# full set of sources for stdlib.
STDLIB_SOURCES += $(STDLIB_SOURCES_EXTRA)

glsl_stdlib_deps := \
	$(intermediates)/driver/libs/khrn/glsl/textures.auto.glsl \
	$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table

define glsl_stdlib_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
$(hide) pushd $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/ && \
	python $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_stdlib.py -O . -I . $(STDLIB_SOURCES) && \
	popd

endef

$(intermediates)/driver/libs/khrn/glsl/glsl_stdlib.auto.h \
$(intermediates)/driver/libs/khrn/glsl/glsl_stdlib.auto.c : $(glsl_stdlib_deps)
	$(glsl_stdlib_auto_gen)

$(intermediates)/driver/libs/khrn/glsl/glsl_parser.h \
$(intermediates)/driver/libs/khrn/glsl/glsl_parser.c \
$(intermediates)/driver/libs/khrn/glsl/glsl_parser.output : $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_parser.y
	$(generated_src_dir_exists)
	bison -d -o $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_parser.c $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_parser.y

$(intermediates)/driver/libs/khrn/glsl/glsl_lexer.c : $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_lexer.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_lexer.c --never-interactive $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_lexer.l

$(intermediates)/driver/libs/khrn/glsl/glsl_numbers.c : $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_numbers.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_numbers.c --never-interactive $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_numbers.l

$(intermediates)/driver/libs/khrn/glsl/glsl_version.c : $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_version.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_version.c --never-interactive $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_version.l

dglenum_gen_deps := \
	$(LOCAL_PATH)/driver/libs/util/dglenum/dglenum_gen.py \
	$(LOCAL_PATH)/driver/libs/khrn/include/GLES3/gl3.h \
	$(LOCAL_PATH)/driver/libs/khrn/include/GLES3/gl3ext_brcm.h

$(intermediates)/driver/libs/util/dglenum/dglenum_gen.h : $(dglenum_gen_deps)
	$(generated_src_dir_exists)
	pushd $(V3D_DRIVER_TOP)/driver && python libs/util/dglenum/dglenum_gen.py > $(GENERATED_SRC_DIR)/driver/libs/util/dglenum/dglenum_gen.h && popd

$(LOCAL_PATH)/driver/libs/util/dglenum/dglenum.c : $(intermediates)/driver/libs/util/dglenum/dglenum_gen.h

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
