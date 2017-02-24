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
V3D_DRIVER_TOP := $(BSEAV_TOP)/lib/gpu/vc5
LOCAL_PATH := $(subst ${ANDROID}/,,$(LOCAL_PATH))

LOCAL_C_INCLUDES := \
        $(V3D_DRIVER_TOP)/driver/ \
        $(V3D_DRIVER_TOP)/driver/interface/khronos/include/ \
        $(V3D_DRIVER_TOP)/driver/vcos/pthreads/ \
        $(V3D_DRIVER_TOP)/driver/vcos/generic/ \
        $(V3D_DRIVER_TOP)/driver/vcos/include/ \
        $(V3D_DRIVER_TOP)/driver/vcos/ \
        $(V3D_DRIVER_TOP)/driver/v3d_platform/ \
        $(V3D_DRIVER_TOP)/driver/v3d_platform/bcg_abstract \
        $(V3D_DRIVER_TOP)/driver/v3d_platform/bcg_abstract/egl \
        $(V3D_DRIVER_TOP)/driver/middleware/khronos/glsl/ \
        $(V3D_DRIVER_TOP)/platform/android \
        $(V3D_DRIVER_TOP)/platform/common \

LOCAL_CFLAGS := \
        -fpic -DPIC -DBCG_ABSTRACT_PLATFORM \
        -Dkhronos_EXPORTS \
        -D_POSIX_C_SOURCE=200112 \
        -D_GNU_SOURCE \
        -DHAVE_ZLIB

LOCAL_CFLAGS += ${V3D_ANDROID_DEFINES}
LOCAL_LDFLAGS := ${V3D_ANDROID_LD}

PROFILING = 0
ifeq ($(V3D_DEBUG),y)

ifneq ($(PROFILING),0)
LOCAL_CFLAGS += -Os -g -DNDEBUG
else
LOCAL_CFLAGS += -O0 -g -fvisibility=hidden
endif

LOCAL_LDFLAGS += -g

else

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

## CAUTION: Using higher optimsation levels causes a SEGV when getting state
#LOCAL_CFLAGS += -O0 -fPIC -DPIC -fvisibility=hidden
LOCAL_CPPFLAGS := \
	-O0 \
	-DANDROID \
	-DHAVE_SYS_UIO_H \
	-fno-rtti \
	-fno-exceptions \
	-fno-use-cxa-atexit \
	-g -funwind-tables \
	-std=c++0x

LOCAL_SRC_FILES := \
	gpumon_hook.cpp \
	api.cpp \
	archive.cpp \
	circularbuffer.cpp \
	packet.cpp \
	platform.cpp \
	remote.cpp

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libnexus
LOCAL_SHARED_LIBRARIES += libnxwrap
LOCAL_SHARED_LIBRARIES += libnxclient

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgpumon_hook

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_C_INCLUDES := $(subst ${ANDROID}/,,$(LOCAL_C_INCLUDES))

include $(BUILD_SHARED_LIBRARY)
