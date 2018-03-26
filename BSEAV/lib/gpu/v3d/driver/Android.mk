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
V3D_DRIVER_TOP := $(BSEAV_TOP)/lib/gpu/v3d
LOCAL_PATH := $(V3D_DRIVER_TOP)
LOCAL_PATH := $(subst ${ANDROID}/,,$(LOCAL_PATH))

LOCAL_C_INCLUDES := \
	$(V3D_DRIVER_TOP)/driver \
	$(V3D_DRIVER_TOP)/driver/interface/khronos/include \
	$(V3D_DRIVER_TOP)/driver/interface/vcos/pthreads \
	$(V3D_DRIVER_TOP)/driver/interface/vcos/generic \
	$(BSEAV_TOP)/lib/gpu/tools/gpumon_hook \
	$(V3D_DRIVER_TOP)/platform/android \
	$(V3D_DRIVER_TOP)/platform/nexus

ifeq ($(BOARD_VNDK_VERSION),current)
LOCAL_HEADER_LIBRARIES := liblog_headers
endif

LOCAL_CFLAGS := \
	-fpic -DPIC \
	-std=c99 \
	-DKHAPI="__attribute__((visibility(\"default\")))" \
	-DSPAPI="__attribute__((visibility(\"default\")))" \
	-DFASTMEM_USE_MALLOC \
	-DASSERT_ON_ALLOC_FAIL \
	-DV3D_LEAN \
	-DMUST_SET_ALPHA \
	-DREMOTE_API_LOGGING \
	-DTIMELINE_EVENT_LOGGING \
	-D_XOPEN_SOURCE=600 \
	-D_GNU_SOURCE \
	-DANDROID \
	-DLOGD=ALOGD \
	-DLOGE=ALOGE \
	-DNO_OPENVG

LOCAL_CFLAGS += ${V3D_ANDROID_DEFINES}
LOCAL_CPPFLAGS += -std=c++11
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
LOCAL_STRIP_MODULE := false
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
LOCAL_LDFLAGS += -g --export-dynamic
endif

endif
endif

ifeq ($(KHRN_AUTOCLIF),1)
LOCAL_CFLAGS += -DKHRN_AUTOCLIF
endif

# Remove unwanted warnings
LOCAL_CFLAGS += \
	-Wno-unused-parameter \
	-Wno-maybe-uninitialized \
	-Wno-sign-compare \
	-Wno-missing-braces \
	-Wno-clobbered \
	-Wno-parentheses \
	-Wno-pointer-arith \
	-Wno-error=address \
	-Wno-format \
	-Wno-address \
	-Wno-unknown-pragmas \
	-Wno-implicit-function-declaration

LOCAL_SRC_FILES := \
	driver/interface/khronos/common/khrn_int_util.c \
	driver/interface/khronos/common/khrn_int_parallel.c \
	driver/interface/khronos/common/khrn_int_image.c \
	driver/interface/khronos/common/khrn_int_hash.c \
	driver/interface/khronos/common/khrn_client_vector.c \
	driver/interface/khronos/common/khrn_client_pointermap.c \
	driver/interface/khronos/common/khrn_client.c \
	driver/interface/khronos/common/khrn_options.c \
	driver/interface/khronos/common/abstract/khrn_client_platform_abstract.c \
	driver/interface/khronos/egl/egl_client_surface.c \
	driver/interface/khronos/egl/egl_client_context.c \
	driver/interface/khronos/egl/egl_client_config.c \
	driver/interface/khronos/egl/egl_client.c \
	driver/interface/khronos/egl/egl_client_get_proc.c \
	driver/interface/khronos/ext/egl_android_ext.c \
	driver/interface/khronos/ext/egl_khr_sync_client.c \
	driver/interface/khronos/ext/egl_khr_lock_surface_client.c \
	driver/interface/khronos/ext/egl_khr_image_client.c \
	driver/interface/khronos/ext/egl_brcm_driver_monitor_client.c \
	driver/middleware/khronos/common/2708/khrn_render_state_4.c \
	driver/middleware/khronos/common/2708/khrn_nmem_4.c \
	driver/middleware/khronos/common/2708/khrn_interlock_4.c \
	driver/middleware/khronos/common/2708/khrn_image_4.c \
	driver/middleware/khronos/common/2708/khrn_hw_4.c \
	driver/middleware/khronos/common/2708/khrn_fmem_4.c \
	driver/middleware/khronos/common/2708/khrn_prod_4.c \
	driver/middleware/khronos/common/2708/khrn_tfconvert_4.c \
	driver/middleware/khronos/common/khrn_tformat.c \
	driver/middleware/khronos/common/khrn_math.c \
	driver/middleware/khronos/common/khrn_interlock.c \
	driver/middleware/khronos/common/khrn_image.c \
	driver/middleware/khronos/common/khrn_fleaky_map.c \
	driver/middleware/khronos/common/khrn_color.c \
	driver/middleware/khronos/common/khrn_bf_dummy.c \
	driver/middleware/khronos/common/khrn_workarounds.c \
	driver/middleware/khronos/common/khrn_debug_helper.cpp \
	driver/middleware/khronos/common/khrn_mem.c \
	driver/middleware/khronos/common/khrn_map.c \
	driver/middleware/khronos/egl/abstract_server/egl_platform_abstractserver.c \
	driver/middleware/khronos/egl/abstract_server/egl_platform_abstractpixmap.c \
	driver/middleware/khronos/egl/egl_server.c \
	driver/middleware/khronos/ext/gl_oes_query_matrix.c \
	driver/middleware/khronos/ext/gl_oes_egl_image.c \
	driver/middleware/khronos/ext/gl_oes_draw_texture.c \
	driver/middleware/khronos/ext/gl_oes_framebuffer_object.c \
	driver/middleware/khronos/ext/ext_gl_multisample_render_to_texture.c \
	driver/middleware/khronos/ext/egl_brcm_driver_monitor.c \
	driver/middleware/khronos/ext/egl_khr_image.c \
	driver/middleware/khronos/ext/ext_gl_debug_marker.c \
	driver/middleware/khronos/gl11/2708/gl11_shader_4.c \
	driver/middleware/khronos/gl11/2708/gl11_shadercache_4.c \
	driver/middleware/khronos/gl11/2708/gl11_support_4.c \
	driver/middleware/khronos/gl11/gl11_matrix.c \
	driver/middleware/khronos/gl11/gl11_server.c \
	driver/middleware/khronos/gl20/2708/gl20_shader_4.c \
	driver/middleware/khronos/gl20/2708/gl20_support_4.c \
	driver/middleware/khronos/gl20/gl20_shader.c \
	driver/middleware/khronos/gl20/gl20_server.c \
	driver/middleware/khronos/gl20/gl20_program.c \
	driver/middleware/khronos/glsl/2708/glsl_allocator_4.c \
	driver/middleware/khronos/glsl/2708/glsl_fpu_4.c \
	driver/middleware/khronos/glsl/2708/glsl_qdisasm_4.c \
	driver/middleware/khronos/glsl/2708/glsl_scheduler_4.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_bcg_sched.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_registers.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_depth_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_print_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_validate_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_sanitize_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_optimize_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_analyze_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_flatten_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_reghint_visitor.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.c \
	driver/middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_directive.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_eval.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_expand.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_macro.c \
	driver/middleware/khronos/glsl/prepro/glsl_prepro_token.c \
	driver/middleware/khronos/glsl/glsl_symbols.c \
	driver/middleware/khronos/glsl/glsl_stringbuilder.c \
	driver/middleware/khronos/glsl/glsl_mendenhall.c \
	driver/middleware/khronos/glsl/glsl_map.c \
	driver/middleware/khronos/glsl/glsl_intern.c \
	driver/middleware/khronos/glsl/glsl_header.c \
	driver/middleware/khronos/glsl/glsl_globals.c \
	driver/middleware/khronos/glsl/glsl_fastmem.c \
	driver/middleware/khronos/glsl/glsl_errors.c \
	driver/middleware/khronos/glsl/glsl_extensions.c \
	driver/middleware/khronos/glsl/glsl_dataflow_visitor.c \
	driver/middleware/khronos/glsl/glsl_dataflow_print.c \
	driver/middleware/khronos/glsl/glsl_dataflow.c \
	driver/middleware/khronos/glsl/glsl_const_functions.c \
	driver/middleware/khronos/glsl/glsl_compiler.c \
	driver/middleware/khronos/glsl/glsl_builders.c \
	driver/middleware/khronos/glsl/glsl_ast_visitor.c \
	driver/middleware/khronos/glsl/glsl_ast.c \
	driver/middleware/khronos/glsl/glsl_ast_print.c \
	driver/middleware/khronos/glsl/lex.yy.c \
	driver/middleware/khronos/glsl/y.tab.c \
	driver/middleware/khronos/glxx/2708/glxx_shader_4.c \
	driver/middleware/khronos/glxx/2708/glxx_inner_4.c \
	driver/middleware/khronos/glxx/2708/glxx_hw_4.c \
	driver/middleware/khronos/glxx/2708/glxx_attr_sort_4.c \
	driver/middleware/khronos/glxx/glxx_texture.c \
	driver/middleware/khronos/glxx/glxx_shared.c \
	driver/middleware/khronos/glxx/glxx_renderbuffer.c \
	driver/middleware/khronos/glxx/glxx_framebuffer.c \
	driver/middleware/khronos/glxx/glxx_buffer.c \
	driver/middleware/khronos/glxx/glxx_server.c \
	driver/middleware/khronos/glxx/glxx_server_cr.c \
	driver/middleware/khronos/glxx/glxx_tweaker.c \
	driver/vcfw/rtos/abstract/rtos_abstract_mem.c \
	driver/vcfw/rtos/abstract/talloc.c \
	driver/interface/vcos/pthreads/vcos_pthreads.c \
	driver/interface/vcos/generic/vcos_mem_from_malloc.c \
	driver/interface/vcos/generic/vcos_generic_named_sem.c \
	driver/interface/vcos/generic/vcos_abort.c \
	driver/interface/vcos/android/vcos_log.c \
	driver/android_platform_library_loader.c \
	platform/android/default_RSO_android.c \
	platform/android/display_RSO_android.cpp \
	platform/common/memory_nexus.c \
	platform/common/hardware_nexus.cpp \
	platform/common/packet_rgba.c \
	platform/common/packet_yv12.c \
	platform/common/autoclif.c

GENERATED_SRC_FILES := \
	driver/middleware/khronos/glsl/lex.yy.c \
	driver/middleware/khronos/glsl/y.tab.c

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libGLES_nexus
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
intermediates := $(call local-intermediates-dir)
LOCAL_SRC_FILES := $(filter-out $(GENERATED_SRC_FILES), $(LOCAL_SRC_FILES))
GENERATED_SRC_FILES := $(addprefix $(intermediates)/, $(GENERATED_SRC_FILES))
LOCAL_GENERATED_SOURCES := $(GENERATED_SRC_FILES)
ifeq (,$(strip $(OUT_DIR_COMMON_BASE)))
GENERATED_SRC_DIR := $(ANDROID_TOP)/$(intermediates)
else
GENERATED_SRC_DIR := $(intermediates)
endif
LOCAL_C_INCLUDES += \
	$(intermediates)/driver/middleware/khronos/glsl \
	$(intermediates)/driver

define generated_src_dir_exists
@mkdir -p $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl
endef

$(intermediates)/driver/middleware/khronos/glsl/y.tab.h \
$(intermediates)/driver/middleware/khronos/glsl/y.tab.c : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_shader.y
	$(generated_src_dir_exists)
	bison -d -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/y.tab.c $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_shader.y

$(intermediates)/driver/middleware/khronos/glsl/lex.yy.c : $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_shader.l
	$(generated_src_dir_exists)
	flex -L -o $(GENERATED_SRC_DIR)/driver/middleware/khronos/glsl/lex.yy.c --never-interactive $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/glsl_shader.l

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libnexus
LOCAL_SHARED_LIBRARIES += libnxwrap
LOCAL_SHARED_LIBRARIES += libnxclient
ifeq ($(KHRN_AUTOCLIF),1)
LOCAL_STATIC_LIBRARIES += libautoclif
endif

ifeq ($(TARGET_2ND_ARCH),arm)
LOCAL_MODULE_RELATIVE_PATH := egl
else
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
endif

LOCAL_C_INCLUDES := $(subst ${ANDROID}/,,$(LOCAL_C_INCLUDES))

include $(BUILD_SHARED_LIBRARY)
