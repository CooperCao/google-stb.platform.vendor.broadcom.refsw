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
	$(ROCKFORD_TOP)/middleware/vc5/driver/libs/core/vcos/posix/ \
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

# Bind references to global functions to the definitions within the khronos
# library. This in particular means eglGetProcAddress will always return the
# khronos library function pointers, even if a GL wrapper library is in
# LD_PRELOAD.
LOCAL_LDFLAGS += -Wl,-Bsymbolic-functions

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

include $(V3D_DRIVER_TOP)/driver/common.mk

LOCAL_SRC_FILES := \
	$(addprefix driver/, $(COMMON_SRC_FILES)) \
	platform/android/default_android.c \
	platform/android/display_android.c \
	platform/android/memory_android.c \
	driver/libs/khrn/egl/platform/bcg_abstract/egl_native_fence_sync_android.c \
	platform/android/sched_android.c \
	platform/common/sched_nexus.c \
	platform/android/android_platform_library_loader.c

# definition order matters here for intermediate (generated) modules.
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libGLES_nexus
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
intermediates := $(call local-generated-sources-dir)
GENERATED_SRC_FILES := $(addprefix $(intermediates)/driver/libs/khrn/glsl/, $(COMMON_GENERATED_SRC_FILES))
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
$(hide) \
	python $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_primitive_types.py \
		-I $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl \
		-O $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/scalar_types.table \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/sampler_types.table \
		$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/image_types.table;
endef

define generated_src_dir_exists
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/util/dglenum
endef

$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table : $(glsl_primitive_types_deps)
	$(glsl_primitive_types_gen)


$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.c \
$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.h \
$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_type_index.auto.h : \
	$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table
		@

$(intermediates)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c : $(LOCAL_PATH)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.gperf > $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_intrinsic_lookup.auto.c

$(intermediates)/driver/libs/khrn/glsl/glsl_layout.auto.c : $(LOCAL_PATH)/driver/libs/khrn/glsl/glsl_layout.gperf
	$(generated_src_dir_exists)
	gperf $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_layout.gperf > $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_layout.auto.c


define textures_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
$(hide) \
	python \
	$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_texture_functions.py \
	-O $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl;
endef

$(intermediates)/driver/libs/khrn/glsl/textures.auto.props : $(LOCAL_PATH)/driver/libs/khrn/glsl/scripts/build_texture_functions.py
	$(textures_auto_gen)

$(intermediates)/driver/libs/khrn/glsl/textures.auto.glsl : \
	$(intermediates)/driver/libs/khrn/glsl/textures.auto.props
		@

# sources for stdlib from source tree.
STDLIB_SOURCES := \
	stdlib/atomics.glsl \
	stdlib/common.glsl \
	stdlib/derivatives.glsl \
	stdlib/exponential.glsl \
	stdlib/extensions.glsl \
	stdlib/geometry.glsl \
	stdlib/geom_shade.glsl \
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
	$(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/textures.auto.glsl \
	$(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/textures.auto.props \
	$(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table

# full set of sources for stdlib.
STDLIB_SOURCES += $(STDLIB_SOURCES_EXTRA)

glsl_stdlib_deps := \
	$(intermediates)/driver/libs/khrn/glsl/textures.auto.glsl \
	$(intermediates)/driver/libs/khrn/glsl/glsl_primitive_types.auto.table

define glsl_stdlib_auto_gen
@mkdir -p $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl
$(hide) python \
	$(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/scripts/build_stdlib.py \
	-I $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl \
	-O $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl \
	$(STDLIB_SOURCES);

endef

$(intermediates)/driver/libs/khrn/glsl/glsl_stdlib.auto.c : $(glsl_stdlib_deps)
	$(glsl_stdlib_auto_gen)

$(intermediates)/driver/libs/khrn/glsl/glsl_stdlib.auto.h : \
	$(intermediates)/driver/libs/khrn/glsl/glsl_stdlib.auto.c
		@

$(intermediates)/driver/libs/khrn/glsl/glsl_parser.output : $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_parser.y
	$(generated_src_dir_exists)
	bison -d -o $(GENERATED_SRC_DIR)/driver/libs/khrn/glsl/glsl_parser.c $(V3D_DRIVER_TOP)/driver/libs/khrn/glsl/glsl_parser.y

$(intermediates)/driver/libs/khrn/glsl/glsl_parser.c \
$(intermediates)/driver/libs/khrn/glsl/glsl_parser.h : \
	$(intermediates)/driver/libs/khrn/glsl/glsl_parser.output
		@

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
