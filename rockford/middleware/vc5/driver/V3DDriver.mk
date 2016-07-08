# make the v3d - non proxy mode
#
BUILD_DYNAMIC ?= 1

NEXUS_TOP ?= ../../../../nexus
MAGNUM_TOP ?= $(NEXUS_TOP)/../magnum
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# default build mode - release

PROFILING?=0

ifeq ($(VERBOSE),)
Q := @
endif

$(info === Building V3D driver ===)

PYTHON2GTEQ_20705 := $(shell expr `python2 --version 2>&1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$2, $$3, $$4) }'` \>= 20705)
ifneq ("$(PYTHON2GTEQ_20705)", "1")
$(error python2 >= 2.7.5 must be available on build machine)
endif

GPERFGTEQ_30004 := $(shell expr `gperf -v | head -1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$3, $$4, $$5) }'` \>= 30004)
ifneq ("$(GPERFGTEQ_30004)", "1")
$(error gperf >= 3.0.4 must be available on build machine)
endif

FLEXGTEQ_20535 := $(shell expr `flex --version | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$2, $$3, $$4) }'` \>= 20535)
ifneq ("$(FLEXGTEQ_20535)", "1")
$(error flex >= 2.5.35 must be available on build machine)
endif

BISONGTEQ_20401 := $(shell expr `bison --version | head -1 | awk 'BEGIN { FS = "\\\.|\\\ " } { printf("%d%2.2d%2.2d\n", $$4, $$5, $$6) }'` \>= 20401)
ifneq ("$(BISONGTEQ_20401)", "1")
$(error bison >= 2.4.1 must be available on build machine)
endif

ifndef B_REFSW_CROSS_COMPILE
B_REFSW_CROSS_COMPILE = $(B_REFSW_ARCH)-
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

# 32-bit ARM
ifeq ($(findstring arm, ${B_REFSW_ARCH}), arm)
CFLAGS += \
	-mcpu=cortex-a15 \
	-mfpu=neon
endif

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-I$(MAGNUM_TOP)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	-I. \
	-I./libs/core/vcos/include \
	-I./libs/core/vcos/pthreads \
	-I./libs/platform/bcg_abstract \
	-I./libs/khrn/glsl \
	-I./libs/khrn/include \
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

ifeq ($(BUILD_DYNAMIC),1)
CFLAGS += -shared
endif

CFLAGS += -Wall -Wpointer-arith

# Add any customer specific cflags from the command line
CFLAGS += $(V3D_EXTRA_CFLAGS)

LDFLAGS = -lpthread -ldl

# Add any customer specific ldflags from the command line
LDFLAGS += $(V3D_EXTRA_LDFLAGS)

ifeq ($(V3D_DEBUG),y)

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
CFLAGS += -Os -g -DNDEBUG
LDFLAGS += --export-dynamic
else
CFLAGS += -O0 -g -fvisibility=hidden
endif

LDFLAGS += -g
OBJDIR ?= obj_$(NEXUS_PLATFORM)_debug
LIBDIR ?= lib_$(NEXUS_PLATFORM)_debug

else

CFLAGS += -Os -DNDEBUG

ifeq ($(PROFILING),0)
CFLAGS += -fvisibility=hidden
# Strip
LDFLAGS += -s
else
CFLAGS += -g
LDFLAGS += -g --export-dynamic
endif

OBJDIR ?= obj_$(NEXUS_PLATFORM)_release
LIBDIR ?= lib_$(NEXUS_PLATFORM)_release
endif

SOURCES = \
	libs/khrn/common/khrn_int_hash.c \
	libs/khrn/common/khrn_options.c \
	libs/khrn/glxx/glxx_client_skin.c \
	libs/util/dglenum/dglenum.c \
	libs/util/log/log.c \
	libs/khrn/common/khrn_fmem.c \
	libs/khrn/common/khrn_fmem_pool.c \
	libs/khrn/common/khrn_fmem_debug_info.c \
	libs/khrn/common/khrn_counters.c \
	libs/khrn/common/khrn_event_monitor.c \
	libs/khrn/common/khrn_render_state.c \
	libs/khrn/common/khrn_blob.c \
	libs/khrn/common/khrn_image.c \
	libs/khrn/common/khrn_image_plane.c \
	libs/khrn/common/khrn_interlock.c \
	libs/khrn/common/khrn_map.c \
	libs/khrn/common/khrn_res_interlock.c \
	libs/khrn/common/khrn_mem.c \
	libs/khrn/common/khrn_process.c \
	libs/khrn/common/khrn_process_debug.c \
	libs/khrn/common/khrn_tformat.c \
	libs/khrn/common/khrn_uintptr_vector.c \
	libs/khrn/common/khrn_synclist.c \
	libs/khrn/common/khrn_fence.c \
	libs/khrn/common/khrn_timeline.c \
	libs/khrn/common/khrn_record.c \
	libs/khrn/egl/egl_attrib_list.c \
	libs/khrn/egl/egl_config.c \
	libs/khrn/egl/egl_context_base.c \
	libs/khrn/egl/egl_context.c \
	libs/khrn/egl/egl_context_gl.c \
	libs/khrn/egl/egl_display.c \
	libs/khrn/egl/egl_map.c \
	libs/khrn/egl/egl_pbuffer_surface.c \
	libs/khrn/egl/egl_process.c \
	libs/khrn/egl/egl_surface_base.c \
	libs/khrn/egl/egl_surface.c \
	libs/khrn/egl/egl_thread.c \
	libs/khrn/egl/egl_image.c \
	libs/khrn/egl/egl_image_texture.c \
	libs/khrn/egl/egl_image_renderbuffer.c \
	libs/khrn/egl/egl_image_framebuffer.c \
	libs/khrn/egl/egl_sync.c \
	libs/khrn/egl/egl_synckhr.c \
	libs/khrn/egl/egl_proc_address.c \
	libs/khrn/egl/egl_platform.c \
	libs/khrn/ext/egl_brcm_perf_counters.c \
	libs/khrn/ext/egl_brcm_event_monitor.c \
	libs/khrn/ext/gl_brcm_base_instance.c \
	libs/khrn/ext/gl_brcm_multi_draw_indirect.c \
	libs/khrn/ext/gl_brcm_texture_1d.c \
	libs/khrn/ext/gl_brcm_polygon_mode.c \
	libs/khrn/ext/gl_brcm_provoking_vertex.c \
	libs/khrn/ext/gl_ext_draw_elements_base_vertex.c \
	libs/khrn/ext/gl_ext_robustness.c \
	libs/khrn/ext/gl_oes_draw_texture.c \
	libs/khrn/ext/gl_oes_map_buffer.c \
	libs/khrn/ext/gl_oes_matrix_palette.c \
	libs/khrn/ext/gl_oes_query_matrix.c \
	libs/khrn/ext/gl_khr_debug.c \
	libs/khrn/ext/gl_khr_debug_msgs.c \
	libs/khrn/gl11/gl11_client.c \
	libs/khrn/gl11/gl11_shader.c \
	libs/khrn/gl11/gl11_vshader.c \
	libs/khrn/gl11/gl11_fshader.c \
	libs/khrn/gl11/gl11_shadercache.c \
	libs/khrn/gl11/gl11_matrix.c \
	libs/khrn/gl11/gl11_server.c \
	libs/khrn/gl11/gl11_draw.c \
	libs/khrn/gl11/gl_oes_framebuffer_object.c \
	libs/khrn/gl20/gl20_program.c \
	libs/khrn/gl20/gl20_server.c \
	libs/khrn/gl20/gl20_shader.c \
	libs/khrn/glsl/glsl_unique_index_queue.c \
	libs/khrn/glsl/glsl_uniform_layout.c \
	libs/khrn/glsl/glsl_symbols.c \
	libs/khrn/glsl/glsl_symbol_table.c \
	libs/khrn/glsl/glsl_stringbuilder.c \
	libs/khrn/glsl/glsl_stackmem.c \
	libs/khrn/glsl/glsl_source.c \
	libs/khrn/glsl/glsl_shader_interfaces.c \
	libs/khrn/glsl/glsl_scoped_map.c \
	libs/khrn/glsl/glsl_safemem.c \
	libs/khrn/glsl/glsl_program.c \
	libs/khrn/glsl/glsl_precision.c \
	libs/khrn/glsl/glsl_map.c \
	libs/khrn/glsl/glsl_layout.c \
	libs/khrn/glsl/glsl_layout.auto.c \
	libs/khrn/glsl/glsl_ir_shader.c \
	libs/khrn/glsl/glsl_ir_program.c \
	libs/khrn/glsl/glsl_intrinsic_lookup.auto.c \
	libs/khrn/glsl/glsl_intrinsic_ir_builder.c \
	libs/khrn/glsl/glsl_intrinsic.c \
	libs/khrn/glsl/glsl_intern.c \
	libs/khrn/glsl/glsl_globals.c \
	libs/khrn/glsl/glsl_file_utils.c \
	libs/khrn/glsl/glsl_fastmem.c \
	libs/khrn/glsl/glsl_extensions.c \
	libs/khrn/glsl/glsl_errors.c \
	libs/khrn/glsl/glsl_dataflow_visitor.c \
	libs/khrn/glsl/glsl_dataflow_simplify.c \
	libs/khrn/glsl/glsl_dataflow_print.c \
	libs/khrn/glsl/glsl_dataflow_builder.c \
	libs/khrn/glsl/glsl_dataflow_cse.c \
	libs/khrn/glsl/glsl_dataflow.c \
	libs/khrn/glsl/glsl_dataflow_image.c \
	libs/khrn/glsl/glsl_linker.c \
	libs/khrn/glsl/glsl_compiler.c \
	libs/khrn/glsl/glsl_check.c \
	libs/khrn/glsl/glsl_builders.c \
	libs/khrn/glsl/glsl_binary_shader.c \
	libs/khrn/glsl/glsl_binary_program.c \
	libs/khrn/glsl/glsl_backflow_visitor.c \
	libs/khrn/glsl/glsl_backflow_print.c \
	libs/khrn/glsl/glsl_backflow.c \
	libs/khrn/glsl/glsl_backflow_combine.c \
	libs/khrn/glsl/glsl_backend.c \
	libs/khrn/glsl/glsl_ast_visitor.c \
	libs/khrn/glsl/glsl_ast_print.c \
	libs/khrn/glsl/glsl_ast.c \
	libs/khrn/glsl/glsl_arenamem.c \
	libs/khrn/glsl/glsl_alloc_tracker.c \
	libs/khrn/glsl/glsl_stdlib.auto.c \
	libs/khrn/glsl/glsl_primitive_types.auto.c \
	libs/khrn/glsl/glsl_basic_block.c \
	libs/khrn/glsl/glsl_basic_block_builder.c \
	libs/khrn/glsl/glsl_basic_block_flatten.c \
	libs/khrn/glsl/glsl_basic_block_print.c \
	libs/khrn/glsl/glsl_nast.c \
	libs/khrn/glsl/glsl_nast_builder.c \
	libs/khrn/glsl/glsl_nast_print.c \
	libs/khrn/glsl/glsl_dominators.c \
	libs/khrn/glsl/glsl_scheduler.c \
	libs/khrn/glsl/glsl_quals.c \
	libs/khrn/glsl/glsl_parser.c \
	libs/khrn/glsl/glsl_lexer.c \
	libs/khrn/glsl/glsl_numbers.c \
	libs/khrn/glsl/glsl_version.c \
	libs/khrn/glsl/prepro/glsl_prepro_directive.c \
	libs/khrn/glsl/prepro/glsl_prepro_eval.c \
	libs/khrn/glsl/prepro/glsl_prepro_expand.c \
	libs/khrn/glsl/prepro/glsl_prepro_token.c \
	libs/khrn/glsl/prepro/glsl_prepro_macro.c \
	libs/khrn/glxx/gl32_stubs.c \
	libs/khrn/glxx/glxx_ez.c \
	libs/khrn/glxx/glxx_hw.c \
	libs/khrn/glxx/glxx_inner.c \
	libs/khrn/glxx/glxx_shader_ops.c \
	libs/khrn/glxx/glxx_shader.c \
	libs/khrn/glxx/glxx_hw_tile_list.c \
	libs/khrn/glxx/glxx_hw_clear.c \
	libs/khrn/glxx/glxx_hw_render_state.c \
	libs/khrn/glxx/glxx_buffer.c \
	libs/khrn/glxx/glxx_ds_to_color.c \
	libs/khrn/glxx/glxx_draw.c \
	libs/khrn/glxx/glxx_extensions.c \
	libs/khrn/glxx/glxx_framebuffer.c \
	libs/khrn/glxx/glxx_renderbuffer.c \
	libs/khrn/glxx/glxx_server.c \
	libs/khrn/glxx/glxx_server_barrier.c \
	libs/khrn/glxx/glxx_server_debug.c \
	libs/khrn/glxx/glxx_textures.c \
	libs/khrn/glxx/glxx_server_texture.c \
	libs/khrn/glxx/glxx_server_buffer.c \
	libs/khrn/glxx/glxx_query.c \
	libs/khrn/glxx/glxx_server_query.c \
	libs/khrn/glxx/glxx_server_get.c \
	libs/khrn/glxx/glxx_server_framebuffer.c \
	libs/khrn/glxx/glxx_server_transform_feedback.c \
	libs/khrn/glxx/glxx_server_vao.c \
	libs/khrn/glxx/glxx_server_sampler.c \
	libs/khrn/glxx/glxx_server_pipeline.c \
	libs/khrn/glxx/glxx_server_sync.c \
	libs/khrn/glxx/glxx_server_program_interface.c \
	libs/khrn/glxx/glxx_shader_cache.c \
	libs/khrn/glxx/glxx_shared.c \
	libs/khrn/glxx/glxx_compressed_paletted_texture.c \
	libs/khrn/glxx/glxx_texture.c \
	libs/khrn/glxx/glxx_texture_utils.c \
	libs/khrn/glxx/glxx_fencesync.c \
	libs/khrn/glxx/glxx_tmu_blit.c \
	libs/khrn/glxx/glxx_tlb_blit.c \
	libs/khrn/glxx/glxx_utils.c \
	libs/khrn/glxx/glxx_compute.c \
	libs/khrn/glxx/glxx_texlevel_param.c \
	libs/khrn/glxx/glxx_image_unit.c \
	libs/util/desc_map/desc_map.c \
	libs/util/gfx_util/gfx_util_morton.c \
	libs/util/gfx_util/gfx_util_hrsize.c \
	libs/util/gfx_util/gfx_util_file.c \
	libs/util/gfx_util/gfx_util.c \
	libs/util/gfx_util/gfx_util_term_col.c \
	libs/util/gfx_util/gfx_util_wildcard.c \
	libs/util/gfx_options/gfx_options.c \
	libs/core/lfmt/lfmt_translate_v3d.c \
	libs/core/lfmt/lfmt_fmt_detail.c \
	libs/core/lfmt/lfmt.c \
	libs/core/lfmt/lfmt_block.c \
	libs/core/lfmt/lfmt_desc.c \
	libs/core/lfmt/lfmt_desc_maps.c \
	libs/core/lfmt_translate_gl/lfmt_translate_gl.c \
	libs/core/gfx_buffer/gfx_buffer_v3d_tfu_srgb_conversions.c \
	libs/core/gfx_buffer/gfx_buffer_uif_config.c \
	libs/core/gfx_buffer/gfx_buffer_translate_v3d.c \
	libs/core/gfx_buffer/gfx_buffer_compare.c \
	libs/core/gfx_buffer/gfx_buffer_bc.c \
	libs/core/gfx_buffer/gfx_buffer.c \
	libs/core/gfx_buffer/gfx_buffer_bstc.c \
	libs/core/gfx_buffer/gfx_buffer_desc_gen.c \
	libs/core/gfx_buffer/gfx_buffer_slow_conv_compr.c \
	libs/core/gfx_buffer/gfx_buffer_slow_conv_xform.c \
	libs/core/gfx_buffer/gfx_buffer_slow_conv.c \
	libs/util/gfx_args/gfx_args.c \
	libs/core/v3d/v3d_cl.c \
	libs/core/v3d/v3d_gen.c \
	libs/core/v3d/v3d_ident.c \
	libs/core/v3d/v3d_printer.c \
	libs/core/v3d/v3d_cl_compr.c \
	libs/core/v3d/v3d_tfu.c \
	libs/core/v3d/v3d_tmu.c \
	libs/platform/v3d_imgconv.c \
	libs/platform/v3d_imgconv_c.c \
	libs/platform/v3d_imgconv_neon.c \
	libs/platform/v3d_imgconv_extra_neon.c \
	libs/platform/v3d_imgconv_gfx_blit.c \
	libs/platform/v3d_scheduler.c \
	libs/platform/v3d_imgconv_tfu.c \
	libs/platform/v3d_imgconv_tlb.c \
	libs/platform/v3d_parallel.c \
	libs/platform/bcg_abstract/gmem_abstract.c \
	libs/platform/bcg_abstract/gmem_talloc.c \
	libs/platform/bcg_abstract/sched_abstract.c \
	libs/khrn/egl/platform/bcg_abstract/egl_platform_abstract.c \
	libs/khrn/egl/platform/bcg_abstract/egl_window_surface_abstract.c \
	libs/khrn/egl/platform/bcg_abstract/egl_surface_common_abstract.c \
	libs/khrn/egl/platform/bcg_abstract/egl_pixmap_surface_abstract.c \
	libs/khrn/egl/platform/bcg_abstract/egl_image_native_buffer_abstract.c \
	libs/core/vcos/pthreads/vcos_pthreads.c \
	libs/core/vcos/generic/vcos_generic_safe_string.c \
	libs/core/vcos/generic/vcos_init.c \
	libs/core/vcos/generic/vcos_properties.c \
	libs/sim/qpu_float/qpu_float.c \
	libs/sim/sfu/sfu.c

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/libv3ddriver.so
else
all: $(LIBDIR)/libv3ddriver.a
endif

libs/khrn/glsl/glsl_primitive_types.auto.table \
libs/khrn/glsl/glsl_primitive_type_index.auto.h \
libs/khrn/glsl/glsl_primitive_types.auto.h \
libs/khrn/glsl/glsl_primitive_types.auto.c : \
	libs/khrn/glsl/scripts/build_primitive_types.py \
	libs/khrn/glsl/scripts/scalar_types.table libs/khrn/glsl/scripts/sampler_types.table \
	libs/khrn/glsl/scripts/image_types.table
		$(Q)cd libs/khrn/glsl && \
		python2 \
			scripts/build_primitive_types.py \
			-I $(CURDIR)/libs/khrn/glsl \
			-O $(CURDIR)/libs/khrn/glsl \
			scripts/scalar_types.table \
			scripts/sampler_types.table \
			scripts/image_types.table;

libs/khrn/glsl/glsl_intrinsic_lookup.auto.c : libs/khrn/glsl/glsl_intrinsic_lookup.gperf
	$(Q)gperf libs/khrn/glsl/glsl_intrinsic_lookup.gperf > libs/khrn/glsl/glsl_intrinsic_lookup.auto.c

libs/khrn/glsl/glsl_layout.auto.c : libs/khrn/glsl/glsl_layout.gperf
	$(Q)gperf libs/khrn/glsl/glsl_layout.gperf > libs/khrn/glsl/glsl_layout.auto.c

libs/khrn/glsl/textures.auto.props libs/khrn/glsl/textures.auto.glsl : \
	libs/khrn/glsl/scripts/build_texture_functions.py
		$(Q)cd libs/khrn/glsl && \
		python2 scripts/build_texture_functions.py;

STDLIB_SOURCES = \
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
	stdlib/restrictions.props \
	textures.auto.glsl \
	textures.auto.props \
	glsl_primitive_types.auto.table

libs/khrn/glsl/glsl_stdlib.auto.c libs/khrn/glsl/glsl_stdlib.auto.h : \
	$(addprefix libs/khrn/glsl/,$(STDLIB_SOURCES))
		$(Q)cd libs/khrn/glsl && \
		python2 \
			$(CURDIR)/libs/khrn/glsl/scripts/build_stdlib.py \
			-O $(CURDIR)/libs/khrn/glsl \
			-I $(CURDIR)/libs/khrn/glsl \
			-I $(CURDIR)/libs/khrn/glsl \
			$(STDLIB_SOURCES)

libs/khrn/glsl/glsl_parser.c \
libs/khrn/glsl/glsl_parser.output \
libs/khrn/glsl/glsl_parser.h : \
	libs/khrn/glsl/glsl_parser.y
		$(Q)bison -d -o libs/khrn/glsl/glsl_parser.c libs/khrn/glsl/glsl_parser.y

libs/khrn/glsl/glsl_lexer.c : libs/khrn/glsl/glsl_lexer.l
	$(Q)flex -L -o libs/khrn/glsl/glsl_lexer.c --never-interactive libs/khrn/glsl/glsl_lexer.l

libs/khrn/glsl/glsl_numbers.c : libs/khrn/glsl/glsl_numbers.l
	$(Q)flex -L -o libs/khrn/glsl/glsl_numbers.c --never-interactive libs/khrn/glsl/glsl_numbers.l

libs/khrn/glsl/glsl_version.c : libs/khrn/glsl/glsl_version.l
	$(Q)flex -L -o libs/khrn/glsl/glsl_version.c --never-interactive libs/khrn/glsl/glsl_version.l

libs/util/dglenum/dglenum_gen.h : \
		libs/util/dglenum/dglenum_gen.py \
		libs/khrn/include/GLES3/gl3.h \
		libs/khrn/include/GLES3/gl3ext_brcm.h
	$(Q)python2 libs/util/dglenum/dglenum_gen.py > libs/util/dglenum/dglenum_gen.h

AUTO_FILES = \
	libs/khrn/glsl/glsl_primitive_types.auto.table \
	libs/khrn/glsl/glsl_primitive_type_index.auto.h \
	libs/khrn/glsl/glsl_primitive_types.auto.h \
	libs/khrn/glsl/glsl_primitive_types.auto.c \
	libs/khrn/glsl/glsl_intrinsic_lookup.auto.c \
	libs/khrn/glsl/textures.auto.glsl \
	libs/khrn/glsl/textures.auto.props \
	libs/khrn/glsl/glsl_stdlib.auto.c \
	libs/khrn/glsl/glsl_stdlib.auto.h \
	libs/khrn/glsl/glsl_parser.c \
	libs/khrn/glsl/glsl_parser.h \
	libs/khrn/glsl/glsl_parser.output \
	libs/khrn/glsl/glsl_lexer.c \
	libs/khrn/glsl/glsl_numbers.c \
	libs/khrn/glsl/glsl_version.c \
	libs/khrn/glsl/glsl_layout.auto.c \
	libs/util/dglenum/dglenum_gen.h

.phony: OUTDIR
OUTDIR :
	$(Q)mkdir -p $(OBJDIR)

# $(1) = src
# $(2) = obj
define CCompileRule
OBJS += $(2)
$(2) : $(1)
	$(Q)echo Compiling $(1)
	$(Q)$(CC) -c $(CFLAGS) -o "$(2)" "$(1)"

endef

$(foreach src,$(SOURCES),$(eval $(call CCompileRule,$(src),$(OBJDIR)/$(basename $(notdir $(src))).o)))

# $(1) = src
# $(2) = d
# $(3) = obj
define DependRule_C
$(2) : $(1) | OUTDIR $(AUTO_FILES)
	$(Q)echo Making depends for $(1)
	$(Q)touch $(2).tmp
	$(Q)$(CC) -D__SSE__ -D__MMX__ -M -MQ $(3) -MF $(2).tmp -MM $(CFLAGS) $(1)
	$(Q)sed 's/D:/\/\/D/g' < $(2).tmp | sed 's/C:/\/\/C/g' > $(2)
	$(Q)rm -f $(2).tmp

PRE_BUILD_RULES += $(2)
# Don't know why, but a comment on this line is necessary

endef

ifneq ($(MAKECMDGOALS),clean)

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call DependRule_C,$(src),$(OBJDIR)/$(basename $(notdir $(src))).d,\
			$(OBJDIR)/$(basename $(notdir $(src))).o)))

$(foreach src,$(filter %.c,$(SOURCES)),$(eval -include $(OBJDIR)/$(basename $(notdir $(src))).d))

endif

$(LIBDIR)/libv3ddriver.so: $(PRE_BUILD_RULES) $(OBJS)
	$(Q)echo Linking ... libv3ddriver.so
	$(Q)mkdir -p $(LIBDIR)
	$(Q)$(CC) $(LDFLAGS) -shared -o $(LIBDIR)/libv3ddriver.so $(OBJS)

$(LIBDIR)/libv3ddriver.a: $(PRE_BUILD_RULES) $(OBJS)
	$(Q)echo Archiving ... libv3ddriver.a
	$(Q)mkdir -p $(LIBDIR)
	$(Q)ar -rcs $(LIBDIR)/libv3ddriver.a $(OBJS)

# clean out the dross
clean:
	$(Q)rm -f $(AUTO_FILES)
	$(Q)rm -f $(LIBDIR)/libv3ddriver.so *~ $(OBJS)
	$(Q)rm -f $(OBJDIR)/*.d
	$(Q)rm -f $(LIBDIR)/libv3ddriver.a
