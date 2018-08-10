############################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
# See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
#############################################################################
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/security/common_drm/lib/cmndrm_dir.inc
include $(BSEAV_TOP)/lib/security/common_crypto/common_crypto.inc
include $(BSEAV_TOP)/opensource/curl/curl.inc

COMMONDRM_LIBDIR=${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/$(CMNDRM_DIR)

COMMONDRM_LIBSO=libcmndrm.so
ifeq ($(B_REFSW_ARCH), mipsel-linux)
  ARCH_DIR ?= mips
  #Adding eglibc support for mips platforms. if the tool chain version is greater than 4.8.5
  GCC_GTEQ_485 := $(shell expr `PATH=$(PATH) $(CC) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40805)
  ifeq "$(GCC_GTEQ_485)" "1"
    ifeq (,$(findstring eglibc,$(COMMONDRM_LIBDIR)))
      COMMONDRM_LIBDIR:=$(COMMONDRM_LIBDIR)/eglibc
    endif
  endif
else
  ARCH_DIR ?= arm
endif

DRMROOTFS_LIBDIR:=${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs

export URSR_TOP=$(realpath ${NEXUS_TOP}/..)
export COMMON_DRM_TOP=$(realpath ${BSEAV_TOP}/lib/security/common_drm)

PLAYREADY_TOP=$(realpath ${BSEAV_TOP}/thirdparty/playready)

# Enable CommonDRM security in Nexus by default, user can override
ifneq ($(BME_ENABLE_COMMONDRM), n)
  export NEXUS_COMMON_CRYPTO_SUPPORT=y
  export BME_ENABLE_COMMONDRM=y
endif

#Sage support
ifeq ($(SAGE_SUPPORT),y)
  SAGE_SECURE_MODE_DEFAULT := 1
  export SAGE_SECURE_MODE ?= $(SAGE_SECURE_MODE_DEFAULT)
  SAGE_BINARY_PATH := $(BSEAV_TOP)/lib/security/sage/bin/$(BCHP_CHIP)$(BCHP_VER)
else
  export SAGE_SUPPORT=n
endif

# PlayReady
ifeq ($(BME_ENABLE_PLAYREADY), y)
  export MSDRM_PRDY_SUPPORT=y
  export MSDRM_PD_HWDECRYPT=y
  export PRDY_ROBUSTNESS_ENABLE=y
  ifeq ($(PLAYREADY_VERSION_32), y)
    export MSDRM_PRDY_SDK_VERSION_3_0=y
  else
    export PLAYREADY_VERSION_25=y
    export MSDRM_PRDY_SDK_VERSION=2.5
  endif
  export PRDY_NEXUS_IO=y
endif

#Widevine
ifeq ($(BME_ENABLE_WIDEVINE), y)
    export WIDEVINE_CDM_VERSION=3.5
    export WIDEVINE_OEMCRYPTO_VERSION=13
    CENC_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/third_party/widevine/CENC3x/ce_cdm-$(WIDEVINE_CDM_VERSION)
    WVCDM_LIBNAME = libwvcdm.a
    WIDEVINE_LIBNAME  = libwvcdm.a
    WVCDM_TARGET = $(TARGET_LIB_DIR)/$(WVCDM_LIBNAME)
    WVCDM_PREBUILT_LIB = $(CENC_DIR)/bin/$(WVCDM_LIBNAME)
    LDFLAGS += -L$(TARGET_LIB_DIR) -lprotobuf -lcurl
endif # BME_ENABLE_WIDEVINE

-include $(PLAYREADY_TOP)/3.0/prdy30_libdir.inc
ifneq ($(PRDY30_LIBDIR),)
  PLAYREADY30_SDK_LIB_PATH := $(realpath $(BSEAV_TOP)/$(PRDY30_LIBDIR))
else
  PLAYREADY30_SDK_LIB_PATH := $(PLAYREADY_TOP)/3.0/bin/$(B_REFSW_ARCH)/lib
endif

REFSW_GE_18_1 := $(shell echo "$(NEXUS_PLATFORM_VERSION_MAJOR).$(NEXUS_PLATFORM_VERSION_MINOR) >= 18.1" | bc)
ifeq ($(REFSW_GE_18_1),1)
  ifneq ($(B_REFSW_ARCH),mipsel-linux)
    ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),5)
        PRDY25_LIBDIR = thirdparty/playready/2.5/bin/$(ARCH_DIR)/Zeus5x/lib
    else
        PRDY25_LIBDIR = thirdparty/playready/2.5/bin/$(ARCH_DIR)/Zeus4x/lib
    endif
  endif
endif
ifneq ($(PRDY25_LIBDIR),)
  PLAYREADY25_SDK_LIB_PATH := $(realpath $(BSEAV_TOP)/$(PRDY25_LIBDIR))
else
  PLAYREADY25_SDK_LIB_PATH :=$(PLAYREADY_TOP)/2.5/bin/$(ARCH_DIR)/lib
endif

ifeq ($(BME_ENABLE_PLAYREADY), y)
  ifeq ($(B_REFSW_ARCH), mipsel-linux)
    #Adding eglibc support for mips platforms. if the tool chain version is greater than 4.8.5
    GCC_GTEQ_485 := $(shell expr `PATH=$(PATH) $(CC) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40805)
    ifeq "$(GCC_GTEQ_485)" "1"
      ifeq (,$(findstring eglibc,$(PLAYREADY25_SDK_LIB_PATH)))
        PLAYREADY25_SDK_LIB_PATH := $(PLAYREADY25_SDK_LIB_PATH)/eglibc
      endif
    endif
  endif

  include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
  CMNDRM_PRDY_LIB := libcmndrmprdy.so
  CMNDRM_PRDY_TARGET := $(TARGET_LIB_DIR)/$(CMNDRM_PRDY_LIB)
endif

ifeq ($(SAGE_SUPPORT), y)
  ifeq ($(BME_ENABLE_WIDEVINE), y)
    OEMCRYPTO_LIBNAME = liboemcrypto_tl.a
    LDFLAGS += wvcdm.a oemcrypto_tl.a
  endif # BME_ENABLE_WIDEVINE

  ifeq ($(BME_ENABLE_PLAYREADY), y)
    ifeq ($(PLAYREADY_VERSION_32), y)
        LIB_PLAYREADYPK30 := playready30pk
        PR_PREBUILT_LIBS += $(PLAYREADY30_SDK_LIB_PATH)/lib$(LIB_PLAYREADYPK30).so
        PR_PREBUILT_LIBDIR := $(PLAYREADY30_SDK_LIB_PATH)
    endif
    ifeq ($(PLAYREADY_VERSION_25), y)
      ifeq ($(B_REFSW_ARCH), aarch64-linux)
          LIB_PLAYREADYPK25 := playreadypk64_host
      else
          LIB_PLAYREADYPK25 := playreadypk_host
      endif
      PR_PREBUILT_LIBS += $(PLAYREADY25_SDK_LIB_PATH)/lib$(LIB_PLAYREADYPK25).so
      PR_PREBUILT_LIBDIR := $(PLAYREADY25_SDK_LIB_PATH)
    endif
  endif # BME_ENABLE_PLAYREADY
else
  # These are libraries to use if SAGE is not enabled
  ifeq ($(BME_ENABLE_PLAYREADY)$(PLAYREADY_VERSION_25), yy)
    LIB_PLAYREADYPK25 := playreadypk
    PR_PREBUILT_LIBS += $(PLAYREADY25_SDK_LIB_PATH)/lib$(LIB_PLAYREADYPK25).so
    PR_PREBUILT_LIBDIR := $(PLAYREADY25_SDK_LIB_PATH)
  endif # BME_ENABLE_PLAYREADY

  ifeq ($(BME_ENABLE_WIDEVINE), y)
    WIDEVINE_LIBNAME  = libwvcdm.a
    OEMCRYPTO_LIBNAME = liboemcrypto.a
    LDFLAGS += -lwvcdm -loemcrypto
  endif # BME_ENABLE_WIDEVINE
endif # SAGE_SUPPORT

PR_TARGET := $(addprefix $(TARGET_LIB_DIR)/,$(notdir ${PR_PREBUILT_LIBS}))

ifeq ($(BME_ENABLE_WIDEVINE), y)
  OEMCRYPTO_TARGET = $(TARGET_LIB_DIR)/$(OEMCRYPTO_LIBNAME)
  OEMCRYPTO_LIB := ${CENC_DIR}/bin/${OEMCRYPTO_LIBNAME}
  WIDEVINE_TARGET = $(TARGET_LIB_DIR)/$(WIDEVINE_LIBNAME)
endif

# MS12 options
ifeq ($(B_REFSW_ARCH), arm-linux)
export BDSP_MS12_SUPPORT=b
else ifeq ($(B_REFSW_ARCH), aarch64-linux)
export BDSP_MS12_SUPPORT=b
else ifeq ($(B_REFSW_ARCH), mipsel-linux)
export BDSP_MS12_SUPPORT=c
endif

print-%  : ; @echo $($*)
