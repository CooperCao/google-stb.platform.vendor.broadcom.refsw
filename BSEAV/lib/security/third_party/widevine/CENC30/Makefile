#/***************************************************************************
# * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
# *
# *  This program is the proprietary software of Broadcom and/or its licensors,
# *  and may only be used, duplicated, modified or distributed pursuant to the terms and
# *  conditions of a separate, written license agreement executed between you and Broadcom
# *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# *  no license (express or implied), right to use, or waiver of any kind with respect to the
# *  Software, and Broadcom expressly reserves all rights in and to the Software and all
# *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
# *
# *  Except as expressly set forth in the Authorized License,
# *
# *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# *  and to use this information only in connection with your use of Broadcom integrated circuit products.
# *
# *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# *  USE OR PERFORMANCE OF THE SOFTWARE.
# *
# *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# *  ANY LIMITED REMEDY.
# *
# *
# *
# * Module Description:  Widevine oemcrypto app interface implementation
# *
# * Revision History:
# *
# *
# *
# **************************************************************************/
# Make file for building the porting kit on Linux with or without Stagecraft
# Adjust paths as needed for your environment.
#
# Required thirdparty libraries: openssl-1.0.0 curl
# NOTE -- an OpenSSL v1.0.1+ library will have additional functionality as AES128 CTR mode is better supported.
#


NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(URSR_TOP),)
URSR_TOP = $(shell pwd)/../../../../../..
$(warning URSR_TOP is not defined. Set it as default: $(URSR_TOP))
endif

NEXUS_TOP=$(URSR_TOP)/nexus
BSEAV_TOP=$(URSR_TOP)/BSEAV
B_REFSW_OBJ_ROOT = $(URSR_TOP)/obj.$(NEXUS_PLATFORM)

ifeq ($(COMMON_DRM_TOP),)
COMMON_DRM_TOP := $(BSEAV_TOP)/lib/security/common_drm
$(warning COMMON_DRM_TOP is not defined! Set it as default: $(COMMON_DRM_TOP))
endif

WIDEVINE_CENC_DIR := $(shell pwd)
ifeq ($(B_REFSW_DEBUG),y)
BUILD_TYPE ?= debug
FLAGS += -g
else
ifeq ($(B_REFSW_DEBUG),)
BUILD_TYPE ?= debug
FLAGS += -g
else
BUILD_TYPE ?= release
BUILD_MODE ?= release
FLAGS += -O2
endif
endif

BRCM_IMPL ?= y
STANDALONE ?= n
NXCLIENT_SUPPORT ?= y
USE_CURL ?= y
BUILD_CDM_TESTS ?= n

# Check oemcrypto/test and oemcrypto/mock/include
BUILD_OEMCRYPTO = y
ifeq ($(shell ls "$(WIDEVINE_CENC_DIR)/oemcrypto" 2>/dev/null|grep -m1 -o test), test)
    ifeq ($(shell ls "$(WIDEVINE_CENC_DIR)/oemcrypto/mock" 2>/dev/null|grep -m1 -o include), include)
        BUILD_OEMCRYPTO_TEST = y
    endif
endif

 BUILD_CDM_TESTS = n

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(BSEAV_TOP)/lib/openssl/openssl.inc
include $(COMMON_DRM_TOP)/common_drm.inc
include $(BSEAV_TOP)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc
include $(WIDEVINE_CENC_DIR)/oemcrypto_libdir.inc
-include $(BSEAV_TOP)/lib/protobuf/protobuf_ver.inc
-include $(BSEAV_TOP)/lib/curl/curl_ver.inc
ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif

PROTOC_VER := $(shell protoc --version | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
ifeq ("$(PROTOC_VER)", "$(PROTOBUF_VER)")
    PROTOC_GENERATE := y
else
    $(warning protoc version $(PROTOC_VER) does not match protobuf $(PROTOBUF_VER))
    PROTOC_GENERATE := n
endif

# set target dir
INSTALL_PREFIX ?= usr/local
OUT_ROOT_DIR := $(WIDEVINE_CENC_DIR)/bin
ifeq ($(SAGE_SUPPORT),y)
WVCDM_LIBDIR :=$(WIDEVINE_CENC_DIR)/cdm/lib/$(ARCH_DIR)
OEMCRYPTO_LIBDIR :=$(WIDEVINE_CENC_DIR)/oemcrypto/lib/$(ARCH_DIR)
else
WVCDM_LIBDIR :=$(WIDEVINE_CENC_DIR)/cdm/lib/$(ARCH_DIR)/nonsage
OEMCRYPTO_LIBDIR :=$(WIDEVINE_CENC_DIR)/oemcrypto/lib/$(ARCH_DIR)/nonsage
endif

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

CFLAGS += -DPIC -fpic
CFLAGS += -Wl,--exclude-libs,ALL
CFLAGS += -Wno-variadic-macros
ifeq ($(ARCH_DIR),arm)
CFLAGS += -DARM_PLATFORM
endif
CFLAGS += $(B_OPENSSL_CFLAGS) \
	$(GTEST_CFLAGS) \
	$(PROTOBUF_CFLAGS) \
	$(CURL_CFLAGS)

$(warning CFLAGS=$(CFLAGS))

ifeq ($(NXCLIENT_SUPPORT),y)
CFLAGS += $(NXCLIENT_CFLAGS)
endif

CFLAGS += -Wall -fexceptions -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CFLAGS += -D_GNU_SOURCE=1
CFLAGS += $(FLAGS) $(NEXUS_CFLAGS)  $(addprefix -D,$(NEXUS_APP_DEFINES))

ifeq ($(BRCM_IMPL),y)
CFLAGS += $(COMMON_DRM_CFLAGS)
endif

CFLAGS := $(filter-out -std=c89 -Wstrict-prototypes, $(CFLAGS))



ifeq ($(COMPILE_WVSOURCE),y)
WVCDM_GENERATED_SRCS := \
	./core/src/device_files.pb.cc \
	./core/src/license_protocol.pb.cc

WVCDM_SRCS := $(WVCDM_GENERATED_SRCS) \
	./cdm/src/cdm.cpp \
	./cdm/src/lock.cpp \
	./cdm/src/log.cpp \
	./cdm/src/properties_ce.cpp \
	./core/src/buffer_reader.cpp \
	./core/src/cdm_engine.cpp \
	./core/src/cdm_session.cpp \
	./core/src/certificate_provisioning.cpp \
	./core/src/crypto_session.cpp \
	./core/src/device_files.cpp \
	./core/src/initialization_data.cpp \
	./core/src/license.cpp \
	./core/src/max_res_engine.cpp \
	./core/src/oemcrypto_adapter_static.cpp \
	./core/src/policy_engine.cpp \
	./core/src/privacy_crypto_openssl.cpp \
	./core/src/properties.cpp \
	./core/src/string_conversions.cpp
endif


ifeq ($(COMPILE_OEMCRYPTO_SOURCE),y)
OEMCRYPTO_SRCS :=  \
	./oemcrypto/mock/src/oemcrypto_mock.cpp \
    ./oemcrypto/mock/src/oemcrypto_engine_mock.cpp \
    ./oemcrypto/mock/src/oemcrypto_key_mock.cpp \
	./oemcrypto/mock/src/oemcrypto_usage_table_mock.cpp \
	./oemcrypto/mock/src/oemcrypto_engine_device_properties.cpp \
	./oemcrypto/mock/src/oemcrypto_logging.cpp \
    ./oemcrypto/mock/src/oemcrypto_keybox_mock.cpp \
    ./oemcrypto/mock/src/wvcrc.cpp \
	./oemcrypto/mock/src/oemcrypto_keybox_testkey.cpp

SYSDEP_SRCS := \
	./cdm/src/clock.cpp \
	./linux/src/lock.cpp \
	./linux/src/log.cpp \
	./cdm/src/properties_common.cpp \
	./core/src/properties.cpp \
	./core/src/string_conversions.cpp \
	./third_party/stringencoders/src/modp_b64w.cpp
endif

OEMCRYPTO_TEST_SRCS := \
	./oemcrypto/test/oemcrypto_test.cpp \
	./oemcrypto/test/oemcrypto_test_main.cpp

OEMCRYPTO_LOGGING_TEST_SRCS := \
	./oemcrypto/mock/test/oemcrypto_logging_test.cpp

CDM_API_TEST_SRCS := \
	./cdm/test/cdm_test.cpp \
	./cdm/test/cdm_test_main.cpp \
	./cdm/test/cdm_test_printers.cpp \
	./cdm/test/device_cert.cpp \
	./cdm/test/test_host.cpp \
	./core/test/license_request.cpp \
	./core/test/http_socket.cpp \
	./core/test/url_request.cpp

CDM_ENGINE_TEST_SRCS := \
	./core/test/base64_test.cpp \
	./core/test/cdm_engine_test.cpp \
	./core/test/cdm_session_unittest.cpp\
	./core/test/config_test_env.cpp \
	./core/test/device_files_unittest.cpp \
	./core/test/http_socket.cpp \
	./core/test/initialization_data_unittest.cpp \
	./core/test/license_request.cpp \
	./core/test/license_unittest.cpp \
	./core/test/max_res_engine_unittest.cpp \
	./core/test/policy_engine_unittest.cpp \
	./core/test/test_printers.cpp \
	./core/test/url_request.cpp


HTTP_SOCKET_TEST_SRCS := \
	./core/test/http_socket_test.cpp \
	./core/test/http_socket.cpp \
	./cdm/src/log.cpp

WV_PLAYBACK_TEST_SRCS := \
    ./wv_playback/wv_playback.cpp \
    ./wv_playback/mp4_parser.c \
    ./wv_playback/bmp4.c  \
    ./wv_playback/cenc_parser.c \

ifneq ($(USE_CURL),y)
WV_PLAYBACK_TEST_SRCS += \
	./core/test/http_socket.cpp \
	./core/test/url_request.cpp
endif

ifeq ($(BRCM_IMPL),y)
ifeq ($(COMPILE_OEMCRYPTO_SOURCE),y)
OEMCRYPTO_SRCS += \
	./oemcrypto/brcm/oemcrypto_brcm.cpp \
	./oemcrypto/brcm/oemcrypto_brcm.h
endif
endif

DEFINES :=  \
	CDM_IMPLEMENTATION \
	CERTI_PROVISIONING
#	DEBUG_DUMP \
#	DEBUG
#	NDEBUG

ifeq ($(BRCM_IMPL),y)
DEFINES += BRCM_IMPL
DEFINES += ENCRYPTED_KEYBOX
endif

ifeq ($(SAGE_SUPPORT),y)
DEFINES += BRCM_SAGE
endif

ifeq ($(STANDALONE),y)
DEFINES += OEC_TEST_STANDALONE
endif

DEFINES := $(addprefix -D, $(DEFINES))

INCLUDES :=  \
	-I$(WIDEVINE_CENC_DIR) \
	-I$(WIDEVINE_CENC_DIR)/cdm/include \
	-I$(WIDEVINE_CENC_DIR)/linux/include \
	-I$(WIDEVINE_CENC_DIR)/core/include \
	-I$(WIDEVINE_CENC_DIR)/core/src \
	-I$(WIDEVINE_CENC_DIR)/oemcrypto/include \
	-I$(WIDEVINE_CENC_DIR)/oemcrypto/mock/include \
	-I$(WIDEVINE_CENC_DIR)/third_party/stringencoders/src \
	-I$(WIDEVINE_CENC_DIR)/cdm/test \
	-I$(WIDEVINE_CENC_DIR)/core/test

ifeq ($(BRCM_IMPL),y)
INCLUDES += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS}) \
	-I$(NEXUS_TOP)/include \
	-I$(BSEAV_TOP)/lib/security/common_crypto/include \
	-I$(BSEAV_TOP)/lib/security/common_drm/include \
	-I$(BSEAV_TOP)/lib/security/common_drm/include/priv \
	-I$(BSEAV_TOP)/lib/drmrootfs \
	-I$(WIDEVINE_CENC_DIR)/oemcrypto/brcm \
	-I$(BSEAV_TOP)/lib/security/sage/srai/include \
    -I$(URSR_TOP)/magnum/syslib/sagelib/include \
    -I$(URSR_TOP)/magnum/basemodules/dbg
endif


# Filter out all non c/cpp files
OEMCRYPTO_SRCS := $(filter %.c %.cpp, $(OEMCRYPTO_SRCS))
SYSDEP_SRCS    := $(filter %.c %.cpp, $(SYSDEP_SRCS))
WVCDM_SRCS     := $(filter %.c %.cpp %.cc, $(WVCDM_SRCS))
CDM_API_TEST_SRCS    := $(filter %.c %.cpp, $(CDM_API_TEST_SRCS))
CDM_ENGINE_TEST_SRCS := $(filter %.c %.cpp, $(CDM_ENGINE_TEST_SRCS))
OEMCRYPTO_TEST_SRCS  := $(filter %.c %.cpp, $(OEMCRYPTO_TEST_SRCS))
HTTP_SOCKET_TEST_SRCS := $(filter %.c %.cpp, $(HTTP_SOCKET_TEST_SRCS))
OEMCRYPTO_LOGGING_TEST_SRCS := $(filter %.c %.cpp, $(OEMCRYPTO_LOGGING_TEST_SRCS))
WV_PLAYBACK_TEST_SRCS    := $(filter %.c %.cpp, $(WV_PLAYBACK_TEST_SRCS))

# Make object files to use as dependencies
OEMCRYPTO_OBJS := $(patsubst %.cpp, %.o, $(OEMCRYPTO_SRCS))
OEMCRYPTO_OBJS := $(patsubst %.c, %.o, $(OEMCRYPTO_OBJS))
OEMCRYPTO_OBJS := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(OEMCRYPTO_OBJS))
SYSDEP_OBJS    := $(patsubst %.cpp, %.o, $(SYSDEP_SRCS))
SYSDEP_OBJS    := $(patsubst %.c, %.o, $(SYSDEP_OBJS))
SYSDEP_OBJS    := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(SYSDEP_OBJS))
WVCDM_OBJS     := $(patsubst %.cpp, %.o, $(WVCDM_SRCS))
WVCDM_OBJS     := $(patsubst %.cc, %.o, $(WVCDM_OBJS))
WVCDM_OBJS     := $(patsubst %.c, %.o, $(WVCDM_OBJS))
WVCDM_OBJS     := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(WVCDM_OBJS))
CDM_API_TEST_OBJS      := $(patsubst %.cpp, %.o, $(CDM_API_TEST_SRCS))
CDM_API_TEST_OBJS      := $(patsubst %.c, %.o, $(CDM_API_TEST_OBJS))
CDM_API_TEST_OBJS      := $(patsubst ./%, $(OUT_ROOT_DIR)/%,$(CDM_API_TEST_OBJS))
CDM_ENGINE_TEST_OBJS   := $(patsubst %.cpp, %.o, $(CDM_ENGINE_TEST_SRCS))
CDM_ENGINE_TEST_OBJS   := $(patsubst %.c, %.o, $(CDM_ENGINE_TEST_OBJS))
CDM_ENGINE_TEST_OBJS   := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(CDM_ENGINE_TEST_OBJS))
HTTP_SOCKET_TEST_OBJS  := $(patsubst %.cpp, %.o, $(HTTP_SOCKET_TEST_SRCS))
HTTP_SOCKET_TEST_OBJS  := $(patsubst %.c, %.o, $(HTTP_SOCKET_TEST_OBJS))
HTTP_SOCKET_TEST_OBJS  := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(HTTP_SOCKET_TEST_OBJS))
OEMCRYPTO_TEST_OBJS    := $(patsubst %.cpp, %.o, $(OEMCRYPTO_TEST_SRCS))
OEMCRYPTO_TEST_OBJS    := $(patsubst %.c, %.o, $(OEMCRYPTO_TEST_OBJS))
OEMCRYPTO_TEST_OBJS    := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(OEMCRYPTO_TEST_OBJS))
OEMCRYPTO_LOGGING_TEST_OBJS   := $(patsubst %.cpp, %.o, $(OEMCRYPTO_LOGGING_TEST_SRCS))
OEMCRYPTO_LOGGING_TEST_OBJS   := $(patsubst %.c, %.o, $(OEMCRYPTO_LOGGING_TEST_OBJS))
OEMCRYPTO_LOGGING_TEST_OBJS   := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(OEMCRYPTO_LOGGING_TEST_OBJS))
WV_PLAYBACK_TEST_OBJS      := $(patsubst %.cpp, %.o, $(WV_PLAYBACK_TEST_SRCS))
WV_PLAYBACK_TEST_OBJS      := $(patsubst %.c, %.o, $(WV_PLAYBACK_TEST_OBJS))
WV_PLAYBACK_TEST_OBJS      := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(WV_PLAYBACK_TEST_OBJS))



ifeq ($(USE_CURL),y)
CDM_API_DEFINES := -DUSE_CURL
CDM_API_DEFINES += -DSKIP_PEER_VERIFICATION
CDM_API_DEFINES += -DUNIT_TEST
CDM_API_DEFINES += -DCDM_TESTS
CDM_API_CFLAGS  := -I$(WIDEVINE_CENC_DIR)/third_party
CDM_API_CFLAGS += -I$(WIDEVINE_CENC_DIR)/third_party/gmock/include
CDM_API_LDFLAGS := -lcurl $(CURL_LDFLAGS)
endif

ifeq ($(SAGE_SUPPORT),y)
CDM_API_DEFINES += -DPLATFORM_REQUIRES_SECURE_BUFFERS=true -DPLATFORM_USES_CLEAR_BUFFERS=false
CDM_API_DEFINES += -DUSE_SECURE_VIDEO_PLAYBACK=1 -DUSE_SECURE_AUDIO_PLAYBACK=1
else
CDM_API_DEFINES += -DPLATFORM_REQUIRES_SECURE_BUFFERS=false -DPLATFORM_USES_CLEAR_BUFFERS=true
CDM_API_DEFINES += -DUSE_SECURE_VIDEO_PLAYBACK=0 -DUSE_SECURE_AUDIO_PLAYBACK=0
endif


ifeq ($(BRCM_IMPL),y)
ifeq ($(NXCLIENT_SUPPORT),y)
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif
endif

LDFLAGS_THIRDPARTY_LIB := $(PROTOBUF_LDFLAGS) $(GTEST_LDFLAGS)
ifeq ($(SAGE_SUPPORT),y)
LDFLAGS_OEMCRYPTO_CDM_LIB := -L$(WVCDM_LIBDIR)/ -lwvcdm -L$(OEMCRYPTO_LIBDIR) -loemcrypto_tl \
                             -L$(NEXUS_BIN_DIR) -lsrai
else
LDFLAGS_OEMCRYPTO_CDM_LIB := -L$(WVCDM_LIBDIR)/ -lwvcdm -L$(OEMCRYPTO_LIBDIR) -loemcrypto
endif

LDFLAGS += $(COMMON_DRM_LDFLAGS)
LDFLAGS += $(B_OPENSSL_LDFLAGS)
LDFLAGS += $(LDFLAGS_THIRDPARTY_LIB)

ifeq ($(BRCM_IMPL),y)
LDFLAGS += $(COMMON_DRM_LDFLAGS)
LDFLAGS += -L$(NEXUS_BIN_DIR) -lnexus
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR) -lbcrypt \
            -L$(BSEAV_TOP)/lib/drmrootfs/$(DRMROOTFS_LIBDIR) -ldrmrootfs
LDFLAGS += -L$(COMMON_DRM_TOP)/lib/$(CMNDRM_DIR) -lcmndrm
ifeq ($(SAGE_SUPPORT),y)
LDFLAGS += -L$(COMMON_DRM_TOP)/lib/$(CMNDRM_DIR) -lcmndrm_tl
endif
endif

.PHONY: clean all install prep_nexus prep_openssl prep_bcrypt oemcrypto oemcrypto_tl wvcdm oec_unittest cdm_api_unittest cdm_engine_unittest http_socket_unittest wv_playback

.SUFFIXES: .c .cpp

%.pb.cc: %.proto
	protoc --cpp_out=. $<

$(OUT_ROOT_DIR)/%.o: %.c
	@test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	${CC} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $(CDM_API_DEFINES) $(CDM_API_CFLAGS) $< -o $@

$(OUT_ROOT_DIR)/%.o: %.cc
	@test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	${CXX} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $(CDM_API_DEFINES) $(CDM_API_CFLAGS) $< -o $@

$(OUT_ROOT_DIR)/%.o: %.cpp
	@test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	${CXX} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $(CDM_API_DEFINES) $(CDM_API_CFLAGS) $< -o $@

all: prep_nexus prep_openssl copy-thirdparty-builds prep_bcrypt prep_cmndrm oemcrypto wvcdm oec_unittest cdm_api_unittest cdm_engine_unittest http_socket_unittest wv_playback copy

ifeq ($(PROTOC_GENERATE),y)
prep_wvcdm_src: $(WVCDM_GENERATED_SRCS)
else
prep_wvcdm_src:
	@echo [Copy pre-generated pb source files]
	@cp core/src/proto_src/* core/src
endif

prep_nexus:
ifeq ($(NXCLIENT_SUPPORT),y)
	@if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/nxserver ]; then \
		echo [ building nxclient .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
else
	@if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/libnexus.so ]; then \
		echo [ building nexus .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi
endif

prep_bcrypt:
	@echo [..... prep_bcrypt target ]
	@if [ ! -d $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR) ]; then \
		echo [ building bcrypt .... ]; \
		$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install >/dev/null; \
	fi

prep_openssl:
	@echo [..... prep_openssl target];
	@if [ ! -d $(B_OPENSSL_LIB_INSTALL_DIR) ]; then \
		echo [ building openssl .... ]; \
		$(MAKE) -C $(B_OPENSSL_BUILD_DIR) install >/dev/null; \
	fi

prep_cmndrm:
	@echo [..... Common DRM target with j option];
	$(Q_)$(MAKE) -C $(COMMON_DRM_TOP) install > /dev/null

report:
	@echo "Reporting ..."

	@echo NEXUS_BIN_DIR = "$(NEXUS_BIN_DIR)"
	@echo WIDEVINE_CENC_DIR = "$(WIDEVINE_CENC_DIR)"
	@echo NEXUS_TOP = "$(NEXUS_TOP)"
	@echo OEMCRYPTO_TEST_OBJS = "$(OEMCRYPTO_TEST_OBJS)"
	@echo OEMCRYPTO_LOGGING_TEST_OBJS = "$(OEMCRYPTO_LOGGING_TEST_OBJS)"
	@echo OEMCRYPTO_SRCS = "$(OEMCRYPTO_SRCS)"
	@echo SYSDEP_OBJS= "$(SYSDEP_OBJS)"
	@echo WVCDM_OBJS = "$(WVCDM_OBJS)"
	@echo
	@echo INCLUDES = "$(INCLUDES)"
	@echo
	@echo CFLAGS = "$(CFLAGS)"
	@echo
	@echo LDFLAGS = "$(LDFLAGS)"
	@echo
	@echo bcrypt lib dir is   "$(BCRYPT_LIBDIR)	"
	@echo drmrootfs lib dir is "$(DRMROOTFS_LIBDIR)"
	@echo BRCM_IMPL = "$(BRCM_IMPL)"
	@echo nexus bin dir="$(NEXUS_BIN_DIR)"
	@echo STAND ALONE TEST = "$(STANDALONE)"
	@echo NXCLIENT_SUPPORT = "$(NXCLIENT_SUPPORT)"
	@echo
	@echo OEMCRYPTO_LIBDIR = "$(OEMCRYPTO_LIBDIR)"
	@echo WVCDM_LIBDIR = "$(WVCDM_LIBDIR)"
	@echo OUT_ROOT_DIR = "$(OUT_ROOT_DIR)"
	@echo CC = "${CC}"
	@echo CXX = "${CXX}"
	@echo ARCH_DIR = "${ARCH_DIR}"
	@echo B_OPENSSL_CFLAGS = "$(B_OPENSSL_CFLAGS)"
	@echo B_OPENSSL_LDFLAGS = "$(B_OPENSSL_LDFLAGS)"
	@echo B_OPENSSL_BUILD_DIR = "$(B_OPENSSL_BUILD_DIR)"
	@echo B_OPENSSL_LIB_INSTALL_DIR = "$(B_OPENSSL_LIB_INSTALL_DIR)"
	@echo

oemcrypto_tl:
	@echo "####################making oemcrypto tl for L1##################"
	@echo " "
	make -C $(WIDEVINE_CENC_DIR)/brcm_oemcrypto_L1 all
	make -C $(WIDEVINE_CENC_DIR)/brcm_oemcrypto_L1 install
	@echo " "
	@echo "######################################"

help:
	@echo "######################################"
	@echo " "
	@echo "First 'export URSR_TOP=<directory to the top of URSR source tree >'"
	@echo "Then  'make'"
	@echo " "
	@echo "######################################"

clean:
	@echo "Cleaning ..."
ifeq ($(COMPILE_OEMCRYPTO_SOURCE),y)
	rm -rf $(OEMCRYPTO_LIBDIR)
endif
ifeq ($(COMPILE_WVSOURCE),y)
	rm -rf $(WVCDM_LIBDIR)
endif
	rm -rf $(OUT_ROOT_DIR)
	rm -rf core/src/*.pb.h core/src/*.pb.cc

# This library is linked into the runtime (Flash/AIR) and drmtest
ifeq ($(COMPILE_WVSOURCE),y)
wvcdm: prep_wvcdm_src $(WVCDM_OBJS)
	@echo "Building $@ library ..."
	@mkdir -p $(OUT_ROOT_DIR)/final
ifeq ($(SAGE_SUPPORT),y)
	@$(CXX) $(FLAGS) $(DEFINES) $(WVCDM_OBJS)  -L$(OEMCRYPTO_LIBDIR) -loemcrypto_tl  -shared -o $(OUT_ROOT_DIR)/final/lib$@.so
else
	@$(CXX) $(FLAGS) $(DEFINES) $(WVCDM_OBJS)  -L$(OEMCRYPTO_LIBDIR) -loemcrypto  -shared -o $(OUT_ROOT_DIR)/final/lib$@.so
endif
	@$(AR) rc $(OUT_ROOT_DIR)/final/lib$@.a $(WVCDM_OBJS)
	@mkdir -p $(WVCDM_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/lib$@.* $(WVCDM_LIBDIR)/
else
wvcdm:
endif
	@echo "Copying $@ ..."
	@cp -f $(WVCDM_LIBDIR)/lib$@.so $(NEXUS_BIN_DIR)/


ifeq ($(COMPILE_OEMCRYPTO_SOURCE),y)
ifeq ($(SAGE_SUPPORT),y)
oemcrypto: prep_openssl prep_bcrypt $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS)
	@echo "Building sage version $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	@$(CXX) $(FLAGS) $(DEFINES) $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS) -shared -o $(OUT_ROOT_DIR)/final/lib$@_tl.so
	@$(AR) rc $(OUT_ROOT_DIR)/final/lib$@_tl.a $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS)
	@mkdir -p $(OEMCRYPTO_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/lib$@_tl.* $(OEMCRYPTO_LIBDIR)/
	@cp -f $(OUT_ROOT_DIR)/final/lib$@_tl.so $(NEXUS_BIN_DIR)/
else
oemcrypto: prep_openssl prep_bcrypt $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	@$(CXX) $(FLAGS) $(DEFINES) $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS) -shared -o $(OUT_ROOT_DIR)/final/lib$@.so
	@$(AR) rc $(OUT_ROOT_DIR)/final/lib$@.a $(OEMCRYPTO_OBJS) $(SYSDEP_OBJS)
	@mkdir -p $(OEMCRYPTO_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/lib$@.* $(OEMCRYPTO_LIBDIR)/
	@cp -f $(OUT_ROOT_DIR)/final/lib$@.so $(NEXUS_BIN_DIR)/
endif
else
ifeq ($(SAGE_SUPPORT),y)
oemcrypto: prep_openssl prep_bcrypt oemcrypto_tl
	@echo "Copying $@_tl ..."
	@cp -f $(OEMCRYPTO_LIBDIR)/lib$@_tl.so $(NEXUS_BIN_DIR)/
else
oemcrypto: prep_openssl prep_bcrypt
	@echo "Copying $@ ..."
	@cp -f $(OEMCRYPTO_LIBDIR)/lib$@.so $(NEXUS_BIN_DIR)/
endif
endif

# This library is only linked into the runtime (Flash/AIR)
oec_mock: $(OEMCRYPTO_OBJS)
	@echo "Building $@ library ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(AR) rc $(OUT_ROOT_DIR)/final/lib$@.a $(OEMCRYPTO_OBJS)

# Adjust the openssl and curl paths below according to your platform
ifeq ($(BUILD_OEMCRYPTO_TEST), n)
oec_unittest:
else
oec_unittest: copy-thirdparty-builds $(OEMCRYPTO_TEST_OBJS) $(oemcrypto) $(wvcdm)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(OEMCRYPTO_TEST_OBJS)  $(LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB)  -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)
endif

ifeq ($(BUILD_CDM_TESTS), n)
cdm_api_unittest:
cdm_engine_unittest:
http_socket_unittest:
else
cdm_api_unittest: copy-thirdparty-builds $(CDM_API_TEST_OBJS) $(oemcrypto) $(wvcdm)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(CDM_API_TEST_OBJS)  $(LDFLAGS) $(CDM_API_LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB) -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)

cdm_engine_unittest: copy-thirdparty-builds $(CDM_ENGINE_TEST_OBJS) $(oemcrypto) $(wvcdm)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(CDM_ENGINE_TEST_OBJS)  $(LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB) -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)

http_socket_unittest: copy-thirdparty-builds $(HTTP_SOCKET_TEST_OBJS)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(HTTP_SOCKET_TEST_OBJS)  $(LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB) -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)
endif
oemcrypto_logging_unittest: copy-thirdparty-builds $(OEMCRYPTO_LOGGING_TEST_OBJS)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(OEMCRYPTO_LOGGING_TEST_OBJS)  $(LDFLAGS)  -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)

ifeq ($(BUILD_CDM_TESTS), n)
wv_playback:
else
ifeq ($(NXCLIENT_SUPPORT),y)
wv_playback: copy-thirdparty-builds $(WV_PLAYBACK_TEST_OBJS)  $(oemcrypto) $(wvcdm)
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(WV_PLAYBACK_TEST_OBJS) $(LDFLAGS) $(CDM_API_LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB)  -o $(OUT_ROOT_DIR)/final/$@
	@cp -rf $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)
else
wv_playback:
endif
endif

copy: copy-security copy-thirdparty-builds copy-openssl

install: copy-prebuilt
	@echo [oemcrypto libs installed...]

copy-security:
ifeq ($(BUILD_OEMCRYPTO_TEST), y)
	@echo [Copying application...]
	@cp -f $(WIDEVINE_CENC_DIR)/bin/final/oec_unittest $(NEXUS_BIN_DIR)
endif
ifeq ($(SAGE_SUPPORT),y)
	@echo [Copying sage version Common DRM lib version...]
	@cp -f $(BSEAV_TOP)/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
	@cp -f $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
	@cp -f $(OEMCRYPTO_LIBDIR)/liboemcrypto_tl.so $(NEXUS_BIN_DIR)

else
	@echo [Copying Common DRM lib version...]
	@cp -f $(BSEAV_TOP)/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
endif
	@echo [Copying DRM rootfs lib version...]
	@cp -f $(BSEAV_TOP)/lib/drmrootfs/$(DRMROOTFS_LIBDIR)/libdrmrootfs.so $(NEXUS_BIN_DIR)
	@echo [Copying bcrypt lib version...]
	@cp -f $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR)/libbcrypt.so $(NEXUS_BIN_DIR)

copy-thirdparty-builds:
	@if [ ! -d $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-2.5.0/ ]; then \
		echo [ Building protobuf and gtest.... ]; \
		$(MAKE) -C $(BSEAV_TOP)/lib/protobuf/ all >/dev/null; \
	fi
	echo [Copying protobuf lib...]; \
	install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-2.5.0/$(NEXUS_PLATFORM)/lib/*.so* $(NEXUS_BIN_DIR)/; \
	echo [Copying gtest lib...] ; \
	install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-2.5.0/gtest/lib/.libs/*.so* $(NEXUS_BIN_DIR)/;

ifeq ($(USE_CURL),y)
	@if [ ! -d $(B_REFSW_OBJ_ROOT)/BSEAV/lib/curl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/curl--7.41.0/ ]; then \
		echo [ Building curl ... ]; \
		$(MAKE) -C $(BSEAV_TOP)/lib/curl/ all  >/dev/null; \
	fi
	install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/curl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/curl-7.41.0/lib/.libs/*.so* $(NEXUS_BIN_DIR)/;
endif

ifeq ($(SAGE_SUPPORT),y)
copy-prebuilt:
	@echo [Copying prebuilt liboemcrypto_tl.so & libwvcdm.so...]
	@echo $(OEMCRYPTO_LIBDIR)
	@echo $(WVCDN_LIBDIR)
	@mkdir -p $(OEMCRYPTO_LIBDIR)
	@mkdir -p $(WVCDM_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/liboemcrypto_tl.so oemcrypto/$(OEMCRYPTO_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/libwvcdm.so cdm/$(WVCDM_LIBDIR)
else
copy-prebuilt:
	@echo [Copying prebuilt liboemcrypto.so & libwvcdm.so...]
	@echo $(OEMCRYPTO_LIBDIR)
	@echo $(WVCDN_LIBDIR)
	@mkdir -p $(OEMCRYPTO_LIBDIR)
	@mkdir -p $(WVCDM_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/liboemcrypto.so oemcrypto/$(OEMCRYPTO_LIBDIR)
	@cp -f $(OUT_ROOT_DIR)/final/libwvcdm.so cdm/$(WVCDM_LIBDIR)
endif

copy-openssl:
	@echo [Copying openssl libs...]
	@cp -f $(B_OPENSSL_LIB_INSTALL_DIR)/lib* $(NEXUS_BIN_DIR)
