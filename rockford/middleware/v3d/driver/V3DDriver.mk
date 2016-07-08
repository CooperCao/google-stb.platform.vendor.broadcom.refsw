# make the v3d - non proxy mode
#
BUILD_DYNAMIC ?= 1

# default build mode - release

PROFILING?=0

ifeq ($(VERBOSE),)
Q := @
endif

$(info === Building V3D driver ===)

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

ifndef B_REFSW_CROSS_COMPILE
B_REFSW_CROSS_COMPILE = $(B_REFSW_ARCH)-
endif

CC = $(B_REFSW_CROSS_COMPILE)gcc
C++ = $(B_REFSW_CROSS_COMPILE)g++

GCCGTEQ_40800 := $(shell expr `$(CC) -dumpversion | awk 'BEGIN { FS = "." }; { printf("%d%02d%02d", $$1, $$2, $$3) }'` \>= 40800)

ifeq ($(findstring mips, ${B_REFSW_ARCH}), mips)
CFLAGS += \
	-mips32
endif

ifeq ($(findstring arm, ${B_REFSW_ARCH}), arm)
CFLAGS += \
	-mfpu=neon
endif

CFLAGS += \
	-fpic -DPIC \
	-I. \
	-I./interface/vcos/pthreads \
	-DKHAPI="__attribute__((visibility(\"default\")))" \
	-DSPAPI="__attribute__((visibility(\"default\")))" \
	-DFASTMEM_USE_MALLOC \
	-DASSERT_ON_ALLOC_FAIL \
	-DV3D_LEAN \
	-DMUST_SET_ALPHA \
	-DBCG_VC4_FAST_ATOMICS \
	-DBCG_MULTI_THREADED \
	-Wunused-parameter \
	-Wsign-compare \
	-Wclobbered \
	-Wmissing-braces \
	-Wparentheses

ifeq ("$(GCCGTEQ_40800)", "1")
CFLAGS += \
	-Wmaybe-uninitialized
endif

ifeq ($(NO_OPENVG),1)
CFLAGS += \
	-DNO_OPENVG
endif

ifeq ($(NO_REMOTE_LOGGING),1)
	# Do nothing
else
	CFLAGS += -DREMOTE_API_LOGGING -DTIMELINE_EVENT_LOGGING
endif

ifeq ($(WITH_DEFRAG),1)
#   Add this to CFLAGS to compile memory defragmentation into code.
#   There is a performance penalty, but this can be useful for code that runs
#   for a very long time.
#
   CFLAGS += -DBCG_VC4_DEFRAG
endif

#   Add this define to change the scheduling of the low latency thread in the 3d driver
#
#   -DSCHED_OVERRIDE=SCHED_RR

#   Add this define to change the priority the of the thread in the given scheme
#
#   -DPRIORITY_OVERRIDE=95

ifeq ($(BUILD_DYNAMIC),1)
CFLAGS += -shared
endif

ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
# BIG ENDIAN
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG -DBIG_ENDIAN_CPU
else
# LITTLE ENDIAN
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -DLITTLE_ENDIAN_CPU
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
CFLAGS += -Os -DNDEBUG
LDFLAGS += --export-dynamic
else
CFLAGS += -O0
endif

CFLAGS += -g
ifeq ($(findstring arm, ${B_REFSW_ARCH}), arm)
CFLAGS += -funwind-tables
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
	interface/khronos/common/khrn_int_util.c \
	interface/khronos/common/khrn_int_parallel.c \
	interface/khronos/common/khrn_int_image.c \
	interface/khronos/common/khrn_int_hash.c \
	interface/khronos/common/khrn_client_vector.c \
	interface/khronos/common/khrn_client_pointermap.c \
	interface/khronos/common/khrn_client_cache.c \
	interface/khronos/common/khrn_client.c \
	interface/khronos/common/khrn_options.c \
	interface/khronos/common/khrn_api_interposer.c \
	interface/khronos/common/abstract/khrn_client_platform_abstract.c \
	interface/khronos/egl/egl_client_surface.c \
	interface/khronos/egl/egl_client_context.c \
	interface/khronos/egl/egl_client_config.c \
	interface/khronos/egl/egl_client.c \
	interface/khronos/egl/egl_client_get_proc.c \
	interface/khronos/ext/egl_android_ext.c \
	interface/khronos/ext/gl_oes_framebuffer_object.c \
	interface/khronos/ext/gl_oes_egl_image_client.c \
	interface/khronos/ext/egl_khr_sync_client.c \
	interface/khronos/ext/egl_khr_lock_surface_client.c \
	interface/khronos/ext/egl_khr_image_client.c \
	interface/khronos/ext/egl_brcm_flush_client.c \
	interface/khronos/ext/egl_brcm_driver_monitor_client.c \
	interface/khronos/ext/egl_brcm_image_update_control_client.c \
	interface/khronos/ext/ext_gl_oes_query_matrix.c \
	interface/khronos/ext/ext_gl_oes_draw_texture.c \
	interface/khronos/ext/ext_gl_debug_marker.c \
	interface/khronos/ext/ext_gl_multisample_render_to_texture.c \
	interface/khronos/glxx/glxx_client.c \
	middleware/khronos/common/2708/khrn_render_state_4.c \
	middleware/khronos/common/2708/khrn_nmem_4.c \
	middleware/khronos/common/2708/khrn_interlock_4.c \
	middleware/khronos/common/2708/khrn_image_4.c \
	middleware/khronos/common/2708/khrn_hw_4.c \
	middleware/khronos/common/2708/khrn_fmem_4.c \
	middleware/khronos/common/2708/khrn_copy_buffer_4.c \
	middleware/khronos/common/2708/khrn_prod_4.c \
	middleware/khronos/common/2708/khrn_tfconvert_4.c \
	middleware/khronos/common/khrn_tformat.c \
	middleware/khronos/common/khrn_misc.c \
	middleware/khronos/common/khrn_math.c \
	middleware/khronos/common/khrn_map_64.c \
	middleware/khronos/common/khrn_map.c \
	middleware/khronos/common/khrn_interlock.c \
	middleware/khronos/common/khrn_image.c \
	middleware/khronos/common/khrn_fleaky_map.c \
	middleware/khronos/common/khrn_color.c \
	middleware/khronos/common/khrn_bf_dummy.c \
	middleware/khronos/common/khrn_workarounds.c \
	middleware/khronos/egl/abstract_server/egl_platform_abstractserver.c \
	middleware/khronos/egl/abstract_server/egl_platform_abstractpixmap.c \
	middleware/khronos/egl/egl_disp.c \
	middleware/khronos/egl/egl_server.c \
	middleware/khronos/ext/gl_oes_query_matrix.c \
	middleware/khronos/ext/gl_oes_egl_image.c \
	middleware/khronos/ext/gl_oes_draw_texture.c \
	middleware/khronos/ext/egl_brcm_driver_monitor.c \
	middleware/khronos/ext/egl_brcm_image_update_control.c \
	middleware/khronos/ext/egl_khr_image.c \
	middleware/khronos/gl11/2708/gl11_shader_4.c \
	middleware/khronos/gl11/2708/gl11_shadercache_4.c \
	middleware/khronos/gl11/2708/gl11_support_4.c \
	middleware/khronos/gl11/gl11_matrix.c \
	middleware/khronos/gl11/gl11_server.c \
	middleware/khronos/gl20/2708/gl20_shader_4.c \
	middleware/khronos/gl20/2708/gl20_support_4.c \
	middleware/khronos/gl20/gl20_shader.c \
	middleware/khronos/gl20/gl20_server.c \
	middleware/khronos/gl20/gl20_program.c \
	middleware/khronos/glsl/2708/glsl_allocator_4.c \
	middleware/khronos/glsl/2708/glsl_fpu_4.c \
	middleware/khronos/glsl/2708/glsl_qdisasm_4.c \
	middleware/khronos/glsl/2708/glsl_scheduler_4.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_bcg_sched.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_registers.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_depth_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_print_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_validate_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_sanitize_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_optimize_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_analyze_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_flatten_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_reghint_visitor.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.c \
	middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.c \
	middleware/khronos/glsl/prepro/glsl_prepro_directive.c \
	middleware/khronos/glsl/prepro/glsl_prepro_eval.c \
	middleware/khronos/glsl/prepro/glsl_prepro_expand.c \
	middleware/khronos/glsl/prepro/glsl_prepro_macro.c \
	middleware/khronos/glsl/prepro/glsl_prepro_token.c \
	middleware/khronos/glsl/glsl_symbols.c \
	middleware/khronos/glsl/glsl_stringbuilder.c \
	middleware/khronos/glsl/glsl_mendenhall.c \
	middleware/khronos/glsl/glsl_map.c \
	middleware/khronos/glsl/glsl_intern.c \
	middleware/khronos/glsl/glsl_header.c \
	middleware/khronos/glsl/glsl_globals.c \
	middleware/khronos/glsl/glsl_fastmem.c \
	middleware/khronos/glsl/glsl_errors.c \
	middleware/khronos/glsl/glsl_extensions.c \
	middleware/khronos/glsl/glsl_dataflow_visitor.c \
	middleware/khronos/glsl/glsl_dataflow_print.c \
	middleware/khronos/glsl/glsl_dataflow.c \
	middleware/khronos/glsl/glsl_const_functions.c \
	middleware/khronos/glsl/glsl_compiler.c \
	middleware/khronos/glsl/glsl_builders.c \
	middleware/khronos/glsl/glsl_ast_visitor.c \
	middleware/khronos/glsl/glsl_ast.c \
	middleware/khronos/glsl/glsl_ast_print.c \
	middleware/khronos/glsl/lex.yy.c \
	middleware/khronos/glsl/y.tab.c \
	middleware/khronos/glxx/2708/glxx_shader_4.c \
	middleware/khronos/glxx/2708/glxx_inner_4.c \
	middleware/khronos/glxx/2708/glxx_hw_4.c \
	middleware/khronos/glxx/2708/glxx_attr_sort_4.c \
	middleware/khronos/glxx/glxx_texture.c \
	middleware/khronos/glxx/glxx_shared.c \
	middleware/khronos/glxx/glxx_renderbuffer.c \
	middleware/khronos/glxx/glxx_framebuffer.c \
	middleware/khronos/glxx/glxx_buffer.c \
	middleware/khronos/glxx/glxx_server.c \
	middleware/khronos/glxx/glxx_server_cr.c \
	middleware/khronos/glxx/glxx_tweaker.c \
	vcfw/rtos/abstract/rtos_abstract_mem.c \
	interface/vcos/pthreads/vcos_pthreads.c \
	interface/vcos/generic/vcos_mem_from_malloc.c \
	interface/vcos/generic/vcos_generic_named_sem.c \
	interface/vcos/generic/vcos_abort.c

SOURCES_VG = \
	interface/khronos/vg/vg_int_mat3x3.c \
	interface/khronos/vg/vg_client.c \
	middleware/khronos/vg/2708/vg_tess_4.c \
	middleware/khronos/vg/2708/vg_shader_md_4.c \
	middleware/khronos/vg/2708/vg_shader_fd_4.c \
	middleware/khronos/vg/2708/vg_segment_lengths_4.c \
	middleware/khronos/vg/2708/vg_path_4.c \
	middleware/khronos/vg/2708/vg_hw_4.c \
	middleware/khronos/vg/2708/vg_bf_4.c \
	middleware/khronos/vg/vg_tess.c \
	middleware/khronos/vg/vg_set.c \
	middleware/khronos/vg/vg_server.c \
	middleware/khronos/vg/vg_segment_lengths.c \
	middleware/khronos/vg/vg_scissor.c \
	middleware/khronos/vg/vg_ramp.c \
	middleware/khronos/vg/vg_path.c \
	middleware/khronos/vg/vg_paint.c \
	middleware/khronos/vg/vg_mask_layer.c \
	middleware/khronos/vg/vg_image.c \
	middleware/khronos/vg/vg_hw.c \
	middleware/khronos/vg/vg_font.c \
	middleware/khronos/vg/vg_bf.c

ifneq ($(NO_OPENVG),1)
SOURCES += ${SOURCES_VG}
endif

ifeq ($(BUILD_DYNAMIC),1)
all: $(LIBDIR)/libv3ddriver.so
else
all: $(LIBDIR)/libv3ddriver.a
endif

middleware/khronos/glsl/y.tab.c \
middleware/khronos/glsl/y.tab.h : \
	middleware/khronos/glsl/glsl_shader.y
		$(Q)bison -d -o middleware/khronos/glsl/y.tab.c middleware/khronos/glsl/glsl_shader.y

middleware/khronos/glsl/lex.yy.c : middleware/khronos/glsl/glsl_shader.l
	$(Q)flex -L -o middleware/khronos/glsl/lex.yy.c --never-interactive middleware/khronos/glsl/glsl_shader.l

AUTO_FILES = \
	middleware/khronos/glsl/lex.yy.c \
	middleware/khronos/glsl/y.tab.c

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
