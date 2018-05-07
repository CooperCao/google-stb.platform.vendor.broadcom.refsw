############################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
# See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
#############################################################################
ifeq ($(REFSW_TOP),)
#$(warning REFSW_TOP is not defined! setting to ../../)
REFSW_TOP := $(shell cd ../../../../;pwd)
endif

NEXUS_TOP ?= $(realpath $(REFSW_TOP)/nexus)
BSEAV_TOP ?= $(realpath $(REFSW_TOP)/BSEAV)
MAGNUM_TOP ?= $(realpath $(REFSW_TOP)/magnum)
B_REFSW_OBJ_ROOT ?= $(REFSW_TOP)/$(B_REFSW_OBJ_DIR)

BSEAV := $(BSEAV_TOP)
MAGNUM := $(MAGNUM_TOP)

BME_USE_CCACHE ?= $(shell which ccache 2> /dev/null)

# Assume Nexus is already built
ifneq ($(MAKECMDGOALS),clean)
include $(B_REFSW_OBJ_ROOT)/nexus/bin/include/platform_app.inc
endif
include drm.inc

# include Nexus defines
NEXUS_CXX_FLAGS = $(filter-out -std=c89 -Wstrict-prototypes -pedantic, $(NEXUS_CFLAGS))

# NXCLIENT flags
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
NEXUS_CXX_FLAGS += $(filter-out $(NEXUS_CFLAGS) -pedantic, $(NXCLIENT_CFLAGS))
NEXUS_CXX_LDFLAGS += $(NXCLIENT_LDFLAGS) -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/common_drm/

# default to build in release
BME_BUILD_TYPE ?= Release

ifeq ($(B_REFSW_ARCH), arm-linux)
LIB_CMNDRM=cmndrm
else ifeq ($(B_REFSW_ARCH), aarch64-linux)
LIB_CMNDRM=cmndrm64
else ifeq ($(B_REFSW_ARCH), mipsel-linux)
LIB_CMNDRM=cmndrm
endif

SYSROOT=$(B_REFSW_OBJ_ROOT)/BSEAV/lib/bme
TARGET_DIR=$(B_REFSW_OBJ_ROOT)/target/usr/local
TARGET_INC_DIR=$(TARGET_DIR)/include/bme
TARGET_BIN_DIR=$(TARGET_DIR)/bin/bme
TARGET_LIB_DIR=$(TARGET_DIR)/lib

# construct DRM flags
ifeq ($(BME_ENABLE_COMMONDRM), y)
BME_LDFLAGS += -l$(LIB_CMNDRM) -lbcrypt -ldrmrootfs
BME_LDFLAGS += $(COMMON_DRM_LDFLAGS) -lssl -lcrypto
NEXUS_CFLAGS += $(addprefix -I, $(COMMON_CRYPTO_INCLUDES))
endif
ifeq ($(BME_ENABLE_PLAYREADY), y)
BME_LDFLAGS += -lcmndrmprdy $(addprefix -l, $(LIB_PLAYREADYPK25))
NEXUS_CXX_LDFLAGS += -L$(PR_PREBUILT_LIBDIR)
endif
NEXUS_CXX_LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/target/usr/local/lib

ifeq ($(SAGE_SUPPORT), y)
SAGE_LDFLAGS := -lsrai
BME_LDFLAGS += $(SAGE_LDFLAGS) -lcmndrm_tl

ifeq ($(BME_ENABLE_WIDEVINE), y)
include $(REFSW_TOP)/BSEAV/opensource/curl/curl.inc
include $(REFSW_TOP)/BSEAV/opensource/zlib/zlib.inc
include $(REFSW_TOP)/BSEAV/opensource/protobuf/protobuf.inc # only needed for WV
include $(BSEAV_TOP)/lib/security/common_drm/lib/cmndrm_dir.inc

WIDEVINE_LDFLAGS = -Wl,--whole-archive $(PROTOBUF_STATIC_LDFLAGS) \
    $(WVCDM_PREBUILT_LIB) \
    $(OEMCRYPTO_LIB) \
    $(CURL_LIB_FOLDER)/libcurl.a \
    $(ZLIB_STATIC_LDFLAGS) \
    -Wl,--no-whole-archive -Wl,--exclude-libs,libprotobuf.a
endif

SRAI_INC_PATH := -I$(BSEAV_TOP)/lib/security/sage/srai/include \
    -I$(BSEAV_TOP)/../magnum/syslib/sagelib/include
NEXUS_CFLAGS += $(SRAI_INC_PATH)
else # SAGE_SUPPORT
ifeq ($(BME_ENABLE_WIDEVINE), y)
$(warning Widevine CDM CENC3x can only work with SAGE enabled.)
$(error Please enable SAGE or disable Widevine)
endif
endif # SAGE_SUPPORT

# this is used to filter out duplicates
BME_LDFLAGS_FILTERED = $(sort $(BME_LDFLAGS))

GYP_OPTIONS:= -DNEXUS_CFLAGS="$(NEXUS_CXX_FLAGS)" \
              -DNEXUS_LDFLAGS="$(NEXUS_CXX_LDFLAGS)" \
              -DLIB_PLAYREADYPK25=\"lib$(LIB_PLAYREADYPK25).so\" \
              -DLIB_PLAYREADYPK30=\"lib$(LIB_PLAYREADYPK30).so\" \
              -DBME_ENABLE_TV="$(BME_ENABLE_TV)" \
              -DBME_ENABLE_DIAL="$(BME_ENABLE_DIAL)" \
              -DBME_ENABLE_PLAYREADY="$(BME_ENABLE_PLAYREADY)" \
              -DPLAYREADY_VERSION_25="$(PLAYREADY_VERSION_25)" \
              -DPLAYREADY_VERSION_32="$(PLAYREADY_VERSION_32)" \
              -DBME_ENABLE_WIDEVINE="$(BME_ENABLE_WIDEVINE)" \
              -DSAGE_SUPPORT="$(SAGE_SUPPORT)" \
              -DSAGE_LDFLAGS="$(SAGE_LDFLAGS)" \
              -DBME_LDFLAGS="$(BME_LDFLAGS_FILTERED)"\
              -DWIDEVINE_LDFLAGS="$(WIDEVINE_LDFLAGS)"\
              -DCENC_DIR="$(CENC_DIR)" \
              -DBSEAV_TOP="$(BSEAV_TOP)" \
              -DLIBCURL_CFLAGS="$(CURL_CFLAGS)" \
              -Icommon.gypi

all: dial configure build_ninja install

GYP_GENERATORS=ninja

PREFIX_DIR=$(shell echo $(TARGET_DIR) | sed 's_/_\\/_g')
PKGCONFIG_FILE=$(SYSROOT)/bme-player.pc

$(SYSROOT)/gyp:
	@$(MKDIR) $(SYSROOT)
	@cd $(SYSROOT) ;\
	git clone https://chromium.googlesource.com/external/gyp

$(SYSROOT)/ninja:
	@cd $(SYSROOT) ;\
	wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip ;\
	unzip ninja-linux.zip

configure: $(SYSROOT)/gyp $(SYSROOT)/ninja
	@CC="$(BME_USE_CCACHE) $(CC)" \
	CXX="$(BME_USE_CCACHE) $(CXX)" \
	LD="$(LD)" \
	AR="$(AR)" \
	AS="$(AS)" \
	RANLIB="$(RANLIB)" \
	CC_host="gcc" \
	CXX_host="g++" \
	LD_host="g++" \
	AR_host="ar" \
	AS_host="as" \
	GYP_DEFINES="$(NEXUS_CXX_FLAGS)" \
	GYP_GENERATORS="$(GYP_GENERATORS)" \
	$(SYSROOT)/gyp/gyp media.gyp --depth=./ --generator-output=$(SYSROOT) $(GYP_OPTIONS);

build_ninja: configure
	@$(SYSROOT)/ninja $(NINJA_FLAG) -C $(SYSROOT)/out/$(BME_BUILD_TYPE)

install: build_ninja pkgconfig
	@$(MKDIR) $(TARGET_BIN_DIR)
	@$(MKDIR) $(TARGET_INC_DIR)
	@$(MKDIR) $(TARGET_LIB_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/lib/*.so $(TARGET_LIB_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/mediaplayer $(TARGET_BIN_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/playerTest $(TARGET_BIN_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/mosaicPlayer $(TARGET_BIN_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/pcmPlayer $(TARGET_BIN_DIR)
	@install $(SYSROOT)/out/$(BME_BUILD_TYPE)/texturePlayer $(TARGET_BIN_DIR)
	@install $(SYSROOT)/bme-player.pc $(TARGET_LIB_DIR)/pkgconfig
	@cp -r ./../player/include/* $(TARGET_INC_DIR)
	@cp -r ./../shared/*.h $(TARGET_INC_DIR)

.PHONY: all clean dial build install

clean:
	@echo "[Cleaning...]"
	@rm -rf $(SYSROOT)/*

pkgconfig:
	@sed -e 's/%prefix%/$(PREFIX_DIR)/' -e 's/%libs%/$(BME_LDFLAGS_FILTERED)/' <bme-player.pc > $(PKGCONFIG_FILE)

.PHONY: tools
