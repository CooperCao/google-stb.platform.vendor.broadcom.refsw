# make the v3d - non proxy mode
#
BUILD_DYNAMIC ?= 1

# default build mode - release

PROFILING?=0

ifeq ($(VERBOSE),)
Q := @
endif

$(info === Building V3D driver ===)

ifndef B_REFSW_CROSS_COMPILE
B_REFSW_CROSS_COMPILE = $(B_REFSW_ARCH)-
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

CFLAGS += \
	-fpic -DPIC \
	-std=c99 \
	-mcpu=cortex-a15 \
	-mfpu=neon \
	-I. \
	-Ivcos \
	-Ivcos/include \
	-Ivcos/pthreads \
	-Iv3d_platform \
	-Iv3d_platform/bcg_abstract \
	-Iv3d_platform/bcg_abstract/egl \
	-Iinterface/khronos/include \
	-Imiddleware/khronos/glsl \
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

ifeq ($(BUILD_DYNAMIC),1)
CFLAGS += -shared
endif

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
	interface/khronos/common/khrn_client_pointermap.c \
	interface/khronos/common/khrn_int_hash.c \
	interface/khronos/common/khrn_int_util.c \
	interface/khronos/common/khrn_options.c \
	interface/khronos/glxx/gl11_client.c \
	interface/khronos/glxx/glxx_client_skin.c \
	interface/khronos/tools/dglenum/dglenum.c \
	middleware/khronos/common/khrn_fmem.c \
	middleware/khronos/common/khrn_fmem_pool.c \
	middleware/khronos/common/khrn_fmem_debug_info.c \
	middleware/khronos/common/khrn_counters.c \
	middleware/khronos/common/khrn_event_monitor.c \
	middleware/khronos/common/khrn_render_state.c \
	middleware/khronos/common/khrn_blob.c \
	middleware/khronos/common/khrn_image.c \
	middleware/khronos/common/khrn_image_plane.c \
	middleware/khronos/common/khrn_interlock.c \
	middleware/khronos/common/khrn_map.c \
	middleware/khronos/common/khrn_res_interlock.c \
	middleware/khronos/common/khrn_mem.c \
	middleware/khronos/common/khrn_process.c \
	middleware/khronos/common/khrn_process_debug.c \
	middleware/khronos/common/khrn_tformat.c \
	middleware/khronos/common/khrn_uintptr_vector.c \
	middleware/khronos/common/khrn_synclist.c \
	middleware/khronos/common/khrn_fence.c \
	middleware/khronos/common/khrn_timeline.c \
	middleware/khronos/common/khrn_record.c \
	middleware/khronos/egl/egl_attrib_list.c \
	middleware/khronos/egl/egl_config.c \
	middleware/khronos/egl/egl_context_base.c \
	middleware/khronos/egl/egl_context.c \
	middleware/khronos/egl/egl_context_gl.c \
	middleware/khronos/egl/egl_display.c \
	middleware/khronos/egl/egl_map.c \
	middleware/khronos/egl/egl_pbuffer_surface.c \
	middleware/khronos/egl/egl_process.c \
	middleware/khronos/egl/egl_surface_base.c \
	middleware/khronos/egl/egl_surface.c \
	middleware/khronos/egl/egl_thread.c \
	middleware/khronos/egl/egl_image.c \
	middleware/khronos/egl/egl_image_texture.c \
	middleware/khronos/egl/egl_image_renderbuffer.c \
	middleware/khronos/egl/egl_image_framebuffer.c \
	middleware/khronos/egl/egl_sync.c \
	middleware/khronos/egl/egl_synckhr.c \
	middleware/khronos/egl/egl_proc_address.c \
	middleware/khronos/egl/egl_platform.c \
	middleware/khronos/ext/egl_brcm_perf_counters.c \
	middleware/khronos/ext/egl_brcm_event_monitor.c \
	middleware/khronos/ext/gl_brcm_base_instance.c \
	middleware/khronos/ext/gl_brcm_multi_draw_indirect.c \
	middleware/khronos/ext/gl_brcm_texture_1d.c \
	middleware/khronos/ext/gl_brcm_polygon_mode.c \
	middleware/khronos/ext/gl_brcm_provoking_vertex.c \
	middleware/khronos/ext/gl_ext_draw_elements_base_vertex.c \
	middleware/khronos/ext/gl_ext_robustness.c \
	middleware/khronos/ext/gl_oes_draw_texture.c \
	middleware/khronos/ext/gl_oes_map_buffer.c \
	middleware/khronos/ext/gl_oes_matrix_palette.c \
	middleware/khronos/ext/gl_oes_query_matrix.c \
	middleware/khronos/ext/gl_khr_debug.c \
	middleware/khronos/ext/gl_khr_debug_msgs.c \
	middleware/khronos/gl11/gl11_shader.c \
	middleware/khronos/gl11/gl11_vshader.c \
	middleware/khronos/gl11/gl11_fshader.c \
	middleware/khronos/gl11/gl11_shadercache.c \
	middleware/khronos/gl11/gl11_matrix.c \
	middleware/khronos/gl11/gl11_server.c \
	middleware/khronos/gl11/gl11_draw.c \
	middleware/khronos/gl11/gl_oes_framebuffer_object.c \
	middleware/khronos/gl20/gl20_program.c \
	middleware/khronos/gl20/gl20_server.c \
	middleware/khronos/gl20/gl20_shader.c \
	middleware/khronos/glsl/glsl_unique_index_queue.c \
	middleware/khronos/glsl/glsl_uniform_layout.c \
	middleware/khronos/glsl/glsl_symbols.c \
	middleware/khronos/glsl/glsl_symbol_table.c \
	middleware/khronos/glsl/glsl_stringbuilder.c \
	middleware/khronos/glsl/glsl_stackmem.c \
	middleware/khronos/glsl/glsl_source.c \
	middleware/khronos/glsl/glsl_shader_interfaces.c \
	middleware/khronos/glsl/glsl_scoped_map.c \
	middleware/khronos/glsl/glsl_safemem.c \
	middleware/khronos/glsl/glsl_program.c \
	middleware/khronos/glsl/glsl_precision.c \
	middleware/khronos/glsl/glsl_map.c \
	middleware/khronos/glsl/glsl_layout.c \
	middleware/khronos/glsl/glsl_layout.auto.c \
	middleware/khronos/glsl/glsl_ir_shader.c \
	middleware/khronos/glsl/glsl_ir_program.c \
	middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c \
	middleware/khronos/glsl/glsl_intrinsic_ir_builder.c \
	middleware/khronos/glsl/glsl_intrinsic.c \
	middleware/khronos/glsl/glsl_intern.c \
	middleware/khronos/glsl/glsl_globals.c \
	middleware/khronos/glsl/glsl_file_utils.c \
	middleware/khronos/glsl/glsl_fastmem.c \
	middleware/khronos/glsl/glsl_extensions.c \
	middleware/khronos/glsl/glsl_errors.c \
	middleware/khronos/glsl/glsl_dataflow_visitor.c \
	middleware/khronos/glsl/glsl_dataflow_simplify.c \
	middleware/khronos/glsl/glsl_dataflow_print.c \
	middleware/khronos/glsl/glsl_dataflow_builder.c \
	middleware/khronos/glsl/glsl_dataflow_cse.c \
	middleware/khronos/glsl/glsl_dataflow.c \
	middleware/khronos/glsl/glsl_linker.c \
	middleware/khronos/glsl/glsl_compiler.c \
	middleware/khronos/glsl/glsl_check.c \
	middleware/khronos/glsl/glsl_builders.c \
	middleware/khronos/glsl/glsl_binary_shader.c \
	middleware/khronos/glsl/glsl_binary_program.c \
	middleware/khronos/glsl/glsl_backflow_visitor.c \
	middleware/khronos/glsl/glsl_backflow_print.c \
	middleware/khronos/glsl/glsl_backflow.c \
	middleware/khronos/glsl/glsl_backflow_combine.c \
	middleware/khronos/glsl/glsl_backend.c \
	middleware/khronos/glsl/glsl_ast_visitor.c \
	middleware/khronos/glsl/glsl_ast_print.c \
	middleware/khronos/glsl/glsl_ast.c \
	middleware/khronos/glsl/glsl_arenamem.c \
	middleware/khronos/glsl/glsl_alloc_tracker.c \
	middleware/khronos/glsl/glsl_stdlib.auto.c \
	middleware/khronos/glsl/glsl_primitive_types.auto.c \
	middleware/khronos/glsl/glsl_basic_block.c \
	middleware/khronos/glsl/glsl_basic_block_builder.c \
	middleware/khronos/glsl/glsl_basic_block_flatten.c \
	middleware/khronos/glsl/glsl_basic_block_print.c \
	middleware/khronos/glsl/glsl_nast.c \
	middleware/khronos/glsl/glsl_nast_builder.c \
	middleware/khronos/glsl/glsl_nast_print.c \
	middleware/khronos/glsl/glsl_dominators.c \
	middleware/khronos/glsl/glsl_scheduler.c \
	middleware/khronos/glsl/glsl_quals.c \
	middleware/khronos/glsl/glsl_parser.c \
	middleware/khronos/glsl/glsl_lexer.c \
	middleware/khronos/glsl/glsl_numbers.c \
	middleware/khronos/glsl/glsl_version.c \
	middleware/khronos/glsl/prepro/glsl_prepro_directive.c \
	middleware/khronos/glsl/prepro/glsl_prepro_eval.c \
	middleware/khronos/glsl/prepro/glsl_prepro_expand.c \
	middleware/khronos/glsl/prepro/glsl_prepro_token.c \
	middleware/khronos/glsl/prepro/glsl_prepro_macro.c \
	middleware/khronos/glxx/gl31_stubs.c \
	middleware/khronos/glxx/gl32_stubs.c \
	middleware/khronos/glxx/glxx_hw.c \
	middleware/khronos/glxx/glxx_inner.c \
	middleware/khronos/glxx/glxx_shader_ops.c \
	middleware/khronos/glxx/glxx_shader.c \
	middleware/khronos/glxx/glxx_hw_tile_list.c \
	middleware/khronos/glxx/glxx_hw_clear.c \
	middleware/khronos/glxx/glxx_hw_render_state.c \
	middleware/khronos/glxx/glxx_buffer.c \
	middleware/khronos/glxx/glxx_ds_to_color.c \
	middleware/khronos/glxx/glxx_draw.c \
	middleware/khronos/glxx/glxx_extensions.c \
	middleware/khronos/glxx/glxx_framebuffer.c \
	middleware/khronos/glxx/glxx_renderbuffer.c \
	middleware/khronos/glxx/glxx_server.c \
	middleware/khronos/glxx/glxx_server_debug.c \
	middleware/khronos/glxx/glxx_textures.c \
	middleware/khronos/glxx/glxx_server_texture.c \
	middleware/khronos/glxx/glxx_server_buffer.c \
	middleware/khronos/glxx/glxx_query.c \
	middleware/khronos/glxx/glxx_server_query.c \
	middleware/khronos/glxx/glxx_server_get.c \
	middleware/khronos/glxx/glxx_server_framebuffer.c \
	middleware/khronos/glxx/glxx_server_transform_feedback.c \
	middleware/khronos/glxx/glxx_server_vao.c \
	middleware/khronos/glxx/glxx_server_sampler.c \
	middleware/khronos/glxx/glxx_server_sync.c \
	middleware/khronos/glxx/glxx_server_program_interface.c \
	middleware/khronos/glxx/glxx_shader_cache.c \
	middleware/khronos/glxx/glxx_shared.c \
	middleware/khronos/glxx/glxx_compressed_paletted_texture.c \
	middleware/khronos/glxx/glxx_texture.c \
	middleware/khronos/glxx/glxx_texture_utils.c \
	middleware/khronos/glxx/glxx_log.c \
	middleware/khronos/glxx/glxx_fencesync.c \
	middleware/khronos/glxx/glxx_tmu_blit.c \
	middleware/khronos/glxx/glxx_tlb_blit.c \
	middleware/khronos/glxx/glxx_utils.c \
	middleware/khronos/glxx/glxx_compute.c \
	middleware/khronos/glxx/glxx_texlevel_param.c \
	helpers/desc_map/desc_map.c \
	helpers/gfx/gfx_util_morton.c \
	helpers/gfx/gfx_util_hrsize.c \
	helpers/gfx/gfx_util_file.c \
	helpers/gfx/gfx_util.c \
	helpers/gfx/gfx_options.c \
	helpers/gfx/gfx_lfmt_translate_v3d.c \
	helpers/gfx/gfx_lfmt_translate_gl.c \
	helpers/gfx/gfx_lfmt_fmt_detail.c \
	helpers/gfx/gfx_lfmt_desc.c \
	helpers/gfx/gfx_lfmt_desc_maps.c \
	helpers/gfx/gfx_lfmt.c \
	helpers/gfx/gfx_ez.c \
	helpers/gfx/gfx_bufstate.c \
	helpers/gfx/gfx_buffer_v3d_tfu_srgb_conversions.c \
	helpers/gfx/gfx_buffer_uif_config.c \
	helpers/gfx/gfx_buffer_translate_v3d.c \
	helpers/gfx/gfx_buffer_compare.c \
	helpers/gfx/gfx_buffer_bc.c \
	helpers/gfx/gfx_buffer.c \
	helpers/gfx/gfx_buffer_bstc.c \
	helpers/gfx/gfx_buffer_desc_gen.c \
	helpers/gfx/gfx_buffer_slow_conv_compr.c \
	helpers/gfx/gfx_buffer_slow_conv_xform.c \
	helpers/gfx/gfx_lfmt_block.c \
	helpers/gfx/gfx_args.c \
	helpers/gfx/gfx_buffer_slow_conv.c \
	helpers/v3d/v3d_cl.c \
	helpers/v3d/v3d_gen.c \
	helpers/v3d/v3d_printer.c \
	helpers/v3d/v3d_cl_compr.c \
	helpers/v3d/v3d_config.c \
	helpers/v3d/v3d_tfu.c \
	helpers/v3d/v3d_tmu.c \
	v3d_platform/v3d_imgconv.c \
	v3d_platform/v3d_imgconv_c.c \
	v3d_platform/v3d_imgconv_neon.c \
	v3d_platform/v3d_imgconv_extra_neon.c \
	v3d_platform/v3d_imgconv_gfx_blit.c \
	v3d_platform/v3d_scheduler.c \
	v3d_platform/v3d_imgconv_tfu.c \
	v3d_platform/v3d_imgconv_tlb.c \
	v3d_platform/v3d_parallel.c \
	v3d_platform/bcg_abstract/gmem_abstract.c \
	v3d_platform/bcg_abstract/gmem_talloc.c \
	v3d_platform/bcg_abstract/sched_abstract.c \
	v3d_platform/bcg_abstract/egl/egl_platform_abstract.c \
	v3d_platform/bcg_abstract/egl/egl_window_surface_abstract.c \
	v3d_platform/bcg_abstract/egl/egl_surface_common_abstract.c \
	v3d_platform/bcg_abstract/egl/egl_pixmap_surface_abstract.c \
	v3d_platform/bcg_abstract/egl/egl_image_native_buffer_abstract.c \
	vcos/generic/vcos_logcat.c \
	vcos/generic/vcos_properties.c \
	vcos/generic/vcos_init.c \
	vcos/generic/vcos_generic_safe_string.c \
	vcos/pthreads/vcos_pthreads.c \
	tools/v3d/qpu_float/qpu_float.c \
	tools/v3d/sfu/sfu.c

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/libv3ddriver.so
else
all: $(LIBDIR)/libv3ddriver.a
endif

middleware/khronos/glsl/glsl_primitive_types.auto.table \
middleware/khronos/glsl/glsl_primitive_type_index.auto.h \
middleware/khronos/glsl/glsl_primitive_types.auto.h \
middleware/khronos/glsl/glsl_primitive_types.auto.c : \
	middleware/khronos/glsl/scripts/build_primitive_types.py \
	middleware/khronos/glsl/scripts/scalar_types.table middleware/khronos/glsl/scripts/sampler_types.table \
	middleware/khronos/glsl/scripts/image_types.table
		$(Q)cd middleware/khronos/glsl && \
		python2 \
			scripts/build_primitive_types.py \
			-I $(CURDIR)/middleware/khronos/glsl \
			-O $(CURDIR)/middleware/khronos/glsl \
			scripts/scalar_types.table \
			scripts/sampler_types.table \
			scripts/image_types.table;

middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c : middleware/khronos/glsl/glsl_intrinsic_lookup.gperf
	$(Q)gperf middleware/khronos/glsl/glsl_intrinsic_lookup.gperf > middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c

middleware/khronos/glsl/glsl_layout.auto.c : middleware/khronos/glsl/glsl_layout.gperf
	$(Q)gperf middleware/khronos/glsl/glsl_layout.gperf > middleware/khronos/glsl/glsl_layout.auto.c

middleware/khronos/glsl/textures.auto.props middleware/khronos/glsl/textures.auto.glsl : \
	middleware/khronos/glsl/scripts/build_texture_functions.py
		$(Q)cd middleware/khronos/glsl && \
		python2 scripts/build_texture_functions.py;

STDLIB_SOURCES = \
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
	stdlib/v310_only.props \
	textures.auto.glsl \
	textures.auto.props \
	glsl_primitive_types.auto.table

middleware/khronos/glsl/glsl_stdlib.auto.c middleware/khronos/glsl/glsl_stdlib.auto.h : \
	$(addprefix middleware/khronos/glsl/,$(STDLIB_SOURCES))
		$(Q)cd middleware/khronos/glsl && \
		python2 \
			$(CURDIR)/middleware/khronos/glsl/scripts/build_stdlib.py \
			-O $(CURDIR)/middleware/khronos/glsl \
			-I $(CURDIR)/middleware/khronos/glsl \
			-I $(CURDIR)/middleware/khronos/glsl \
			$(STDLIB_SOURCES)

middleware/khronos/glsl/glsl_parser.c \
middleware/khronos/glsl/glsl_parser.output \
middleware/khronos/glsl/glsl_parser.h : \
	middleware/khronos/glsl/glsl_parser.y
		$(Q)bison -d -o middleware/khronos/glsl/glsl_parser.c middleware/khronos/glsl/glsl_parser.y

middleware/khronos/glsl/glsl_lexer.c : middleware/khronos/glsl/glsl_lexer.l
	$(Q)flex -L -o middleware/khronos/glsl/glsl_lexer.c --never-interactive middleware/khronos/glsl/glsl_lexer.l

middleware/khronos/glsl/glsl_numbers.c : middleware/khronos/glsl/glsl_numbers.l
	$(Q)flex -L -o middleware/khronos/glsl/glsl_numbers.c --never-interactive middleware/khronos/glsl/glsl_numbers.l

middleware/khronos/glsl/glsl_version.c : middleware/khronos/glsl/glsl_version.l
	$(Q)flex -L -o middleware/khronos/glsl/glsl_version.c --never-interactive middleware/khronos/glsl/glsl_version.l

interface/khronos/tools/dglenum/dglenum_gen.c : \
		interface/khronos/tools/dglenum/dglenum_gen.py \
		interface/khronos/include/GLES3/gl3.h \
		interface/khronos/include/GLES3/gl3ext_brcm.h
	$(Q)python2 interface/khronos/tools/dglenum/dglenum_gen.py > interface/khronos/tools/dglenum/dglenum_gen.c

AUTO_FILES = \
	middleware/khronos/glsl/glsl_primitive_types.auto.table \
	middleware/khronos/glsl/glsl_primitive_type_index.auto.h \
	middleware/khronos/glsl/glsl_primitive_types.auto.h \
	middleware/khronos/glsl/glsl_primitive_types.auto.c \
	middleware/khronos/glsl/glsl_intrinsic_lookup.auto.c \
	middleware/khronos/glsl/textures.auto.glsl \
	middleware/khronos/glsl/textures.auto.props \
	middleware/khronos/glsl/glsl_stdlib.auto.c \
	middleware/khronos/glsl/glsl_stdlib.auto.h \
	middleware/khronos/glsl/glsl_parser.c \
	middleware/khronos/glsl/glsl_parser.h \
	middleware/khronos/glsl/glsl_parser.output \
	middleware/khronos/glsl/glsl_lexer.c \
	middleware/khronos/glsl/glsl_numbers.c \
	middleware/khronos/glsl/glsl_version.c \
	middleware/khronos/glsl/glsl_layout.auto.c \
	interface/khronos/tools/dglenum/dglenum_gen.c

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
