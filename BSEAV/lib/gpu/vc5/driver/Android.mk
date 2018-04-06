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

# defines V3D_VER_AT_LEAST which is required for feature definition
include $(BSEAV_TOP)/lib/gpu/vc5/driver/common.mk

V3D_DRIVER_TOP_ABS_PATH := $(BSEAV_TOP)/lib/gpu/vc5
V3D_DRIVER_TOP_REL_PATH := $(subst ${ANDROID}/,,$(V3D_DRIVER_TOP_ABS_PATH))

PYTHON_CMD := python
BISON_CMD  := ${BISON}
FLEX_CMD   := ${LEX}
GPERF_CMD  := ${GPERF_BCM}

ifeq ($(V3D_VER_AT_LEAST_4_1_34), 1)
HAS_VULKAN ?= 1
else
HAS_VULKAN ?= 0
endif

ifeq ($(HAS_VULKAN), 1)
$(info === Building V3D OpenGL ES and Vulkan drivers ===)
else
$(info === Building V3D OpenGL ES driver ===)
endif

define set_gl_vk_common_local_variables

	LOCAL_C_INCLUDES := \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/interface/khronos/include \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/posix \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/generic \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/include \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/egl/platform/bcg_abstract \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/platform \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/platform/bcg_abstract \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/egl/bcg_abstract/egl \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/glsl \
		$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/include \
		$(V3D_DRIVER_TOP_ABS_PATH)/platform/android \
		$(V3D_DRIVER_TOP_ABS_PATH)/platform/common \
		$(ANDROID_TOP)/system/core/libsync/include \
		$(ANDROID_TOP)/frameworks/native/libs/arect/include \
		$(ANDROID_TOP)/frameworks/native/libs/nativewindow/include \
		$(ANDROID_TOP)/frameworks/native/libs/nativebase/include \
		$(BSEAV_TOP)/../magnum/portinginterface/vc5/include \
		$(BSEAV_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
		$(BSEAV_TOP)/linux/driver/brcmv3d/include/uapi/drm \
		$(ANDROID_TOP)/system/core/liblog/include

	LOCAL_CFLAGS := \
		-fpic -DPIC \
		-fwrapv \
		-Dkhronos_EXPORTS \
		-D_POSIX_C_SOURCE=200112 \
		-D_GNU_SOURCE \
		-DHAVE_ZLIB \
		-DEMBEDDED_SETTOP_BOX=1 \
		-DKHRN_GLES32_DRIVER=0 \
		-DV3D_PLATFORM_SIM=0 \
		-DSECURE_SUPPORT=0 \
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
		ifneq ($(PROFILING),0)
			LOCAL_CFLAGS += -O3 -DNDEBUG
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

			LOCAL_CFLAGS += -O3 -DNDEBUG -g -funwind-tables
			LOCAL_LDFLAGS += -g -export-dynamic
		else
			# Full release build - no debug info
			LOCAL_CFLAGS += -O3 -DNDEBUG
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

	LOCAL_PRELINK_MODULE := false

	LOCAL_SHARED_LIBRARIES := libcutils
	LOCAL_SHARED_LIBRARIES += libdl
	LOCAL_SHARED_LIBRARIES += liblog
	LOCAL_SHARED_LIBRARIES += libnexus
	LOCAL_SHARED_LIBRARIES += libnxwrap
	LOCAL_SHARED_LIBRARIES += libnxclient
	LOCAL_SHARED_LIBRARIES += libsync
	LOCAL_SHARED_LIBRARIES += libc++

	# Bind references to global functions to the definitions within the khronos
	# library. This in particular means eglGetProcAddress will always return the
	# khronos library function pointers, even if a GL wrapper library is in
	# LD_PRELOAD.
	LOCAL_LDFLAGS += -Wl,-Bsymbolic-functions

endef

# ----------------------------------------
# Building generated code as a static lib
# To be shared between GLES and Vulkan
# ----------------------------------------

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# Android needs this to be set
LOCAL_PATH := $(V3D_DRIVER_TOP_REL_PATH)

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := vk_gl_intermediate
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
intermediates := $(call local-generated-sources-dir)
GENERATED_SRC_DIR := $(abspath ${intermediates})

GLSL_INTERMEDIATE_ABS_PATH := $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
DGLENUM_INTERMEDIATE_ABS_PATH := $(GENERATED_SRC_DIR)/driver/libs/util/dglenum
GLSL_INTERMEDIATE_REL_PATH := $(intermediates)/driver/libs/khrn/glsl
DGLENUM_INTERMEDIATE_REL_PATH := $(intermediates)/driver/libs/util/dglenum

LOCAL_C_INCLUDES := \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/interface/khronos/include \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/posix \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/generic \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos/include \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/core/vcos \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/egl/platform/bcg_abstract \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/platform \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/platform/bcg_abstract \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/egl/bcg_abstract/egl \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/glsl \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/khrn/include \
	$(BSEAV_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) \
	$(BSEAV_TOP)/../magnum/portinginterface/vc5/include

ifeq ($(BOARD_VNDK_VERSION),current)
LOCAL_HEADER_LIBRARIES := liblog_headers
endif

LOCAL_CFLAGS := \
	-fpic -DPIC \
	-fwrapv \
	-Dkhronos_EXPORTS \
	-D_POSIX_C_SOURCE=200112 \
	-D_GNU_SOURCE \
	-DHAVE_ZLIB \
	-DEMBEDDED_SETTOP_BOX=1 \
	-DKHRN_GLES32_DRIVER=0 \
	-DV3D_PLATFORM_SIM=0 \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable

LOCAL_CPPFLAGS := -std=gnu++0x


ifeq ($(TARGET_2ND_ARCH),arm)
	LOCAL_CFLAGS_arm64 += ${V3D_ANDROID_DEFINES_1ST_ARCH}
	LOCAL_LDFLAGS_arm64 := ${V3D_ANDROID_LD_1ST_ARCH}
	LOCAL_CFLAGS_arm += ${V3D_ANDROID_DEFINES_2ND_ARCH}
	LOCAL_LDFLAGS_arm := ${V3D_ANDROID_LD_2ND_ARCH}
else
	LOCAL_CFLAGS_arm += ${V3D_ANDROID_DEFINES_1ST_ARCH}
	LOCAL_LDFLAGS_arm := ${V3D_ANDROID_LD_1ST_ARCH}
endif

LOCAL_C_INCLUDES += \
	$(DGLENUM_INTERMEDIATE_REL_PATH) \
	$(GLSL_INTERMEDIATE_REL_PATH)

include $(V3D_DRIVER_TOP_ABS_PATH)/driver/common.mk

LOCAL_GENERATED_SOURCES := $(addprefix $(intermediates)/driver/libs/khrn/glsl/, $(COMMON_GENERATED_SRC_FILES) $(GLES_GENERATED_SRC_FILES))

LOCAL_EXPORT_C_INCLUDES := \
	$(DGLENUM_INTERMEDIATE_REL_PATH) \
	$(GLSL_INTERMEDIATE_REL_PATH)

V3D_DRIVER_LIBS_REL_PATH := $(V3D_DRIVER_TOP_REL_PATH)/driver/libs
V3D_DRIVER_LIBS_ABS_PATH := $(V3D_DRIVER_TOP_ABS_PATH)/driver/libs

include $(V3D_DRIVER_TOP_ABS_PATH)/driver/intermediates.mk

LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := $(subst ${ANDROID}/,,$(LOCAL_C_INCLUDES))

include $(BUILD_STATIC_LIBRARY)


# -----------------------
# Building GL ES driver
# -----------------------

include $(CLEAR_VARS)
# Android needs this to be set
LOCAL_PATH := $(V3D_DRIVER_TOP_REL_PATH)

$(eval $(call set_gl_vk_common_local_variables))

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
endif


# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libGLES_nexus
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_STATIC_LIBRARIES := vk_gl_intermediate

include $(V3D_DRIVER_TOP_ABS_PATH)/driver/common.mk

LOCAL_C_INCLUDES += \
	$(DGLENUM_INTERMEDIATE_REL_PATH) \
	$(GLSL_INTERMEDIATE_REL_PATH)

ifeq ($(BOARD_VNDK_VERSION),current)
LOCAL_HEADER_LIBRARIES := liblog_headers
endif

LOCAL_SRC_FILES := \
	$(addprefix driver/, $(COMMON_SRC_FILES) $(GLES_SRC_FILES)) \
	platform/android/default_android.c \
	platform/android/display_android.c \
	platform/android/memory_android.c \
	platform/common/memory_drm.c \
	platform/common/memory_convert.c \
	platform/common/display_helpers.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_native_fence_sync_android.c \
	platform/android/sched_android.c \
	platform/common/sched_nexus.c \
	platform/android/android_platform_library_loader.c \
	platform/common/perf_event.cpp

LOCAL_CPPFLAGS := -std=gnu++0x -fno-rtti -fno-exceptions

LOCAL_CFLAGS += -std=c99

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_RELATIVE_PATH := egl

LOCAL_C_INCLUDES := $(subst ${ANDROID}/,,$(LOCAL_C_INCLUDES))

include $(BUILD_SHARED_LIBRARY)


ifeq ($(HAS_VULKAN), 1)

# -----------------------
# Building Vulkan driver
# -----------------------

include $(CLEAR_VARS)
# Android needs this to be set
LOCAL_PATH := $(V3D_DRIVER_TOP_REL_PATH)

$(eval $(call set_gl_vk_common_local_variables))

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libbcmvulkan_icd
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_STATIC_LIBRARIES := vk_gl_intermediate

LOCAL_CFLAGS += -DVK_USE_PLATFORM_ANDROID_KHR=1 -frtti -fexceptions

LOCAL_C_INCLUDES += \
	$(DGLENUM_INTERMEDIATE_REL_PATH) \
	$(GLSL_INTERMEDIATE_REL_PATH) \
	$(V3D_DRIVER_TOP_ABS_PATH)/platform/common \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/vulkan/include \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/vulkan/driver \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/vulkan/driver/spirv \
	$(V3D_DRIVER_TOP_ABS_PATH)/driver/libs/vulkan/driver/platforms \
	$(ANDROID_TOP)/frameworks/native/vulkan/include

ifeq ($(BOARD_VNDK_VERSION),current)
LOCAL_HEADER_LIBRARIES := liblog_headers
endif

LOCAL_SRC_FILES := \
	$(addprefix driver/, $(COMMON_SRC_FILES) $(VULKAN_SRC_FILES)) \
	platform/android/display_android.c \
	platform/android/memory_android.c \
	platform/android/sched_android.c

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_RELATIVE_PATH := ./

LOCAL_C_INCLUDES := $(subst ${ANDROID}/,,$(LOCAL_C_INCLUDES))

include $(BUILD_SHARED_LIBRARY)

endif
