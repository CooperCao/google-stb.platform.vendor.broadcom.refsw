# ------------------------------------
# Copyright Broadcom Corporation 2009 
# ------------------------------------

LOCAL_PATH:= $(call my-dir)
base := $(LOCAL_PATH)
# Change below to n if you want to build only wps cli
CONFIG_BUILD_COMPONENTS_UI :=y

# ---------------------------------
# Build IBinder Client Library
# ---------------------------------
ifeq ($(CONFIG_BUILD_COMPONENTS_UI), y)

include $(CLEAR_VARS)

SRCBASE := ../../../src
LOCALSRCBASE := $(SRCBASE)/wps/android/wlwpsclientlib

LOCAL_SRC_FILES:= \
       $(LOCALSRCBASE)/WLWPSClient.cpp \
       $(LOCALSRCBASE)/IWLWPSClient.cpp \
       $(LOCALSRCBASE)/IWLWPSService.cpp \

LOCAL_SHARED_LIBRARIES := \
	libui libcutils libutils libsonivox 

ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

LOCAL_MODULE:= libwlwpsclient

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif

LOCAL_C_INCLUDES := \
        $(base)/$(SRCBASE)/wps/android/include

LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

endif

# ---------------------------------
# Build IBinder Server Library
# ---------------------------------
ifeq ($(CONFIG_BUILD_COMPONENTS_UI), y)
include $(CLEAR_VARS)

SRCBASE := ../../../src
LOCALSRCBASE := $(SRCBASE)/wps/android/wlwpsserverlib

BUILD_BRCM_WPS := y

ifeq ($(BUILD_BRCM_WPS), y)

LOCAL_SRC_FILES:=               \
	$(LOCALSRCBASE)/WLWPSService.cpp \

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl -lpthread
endif

LOCAL_SHARED_LIBRARIES := \
     libcutils libutils libandroid_runtime libwlwps libnetutils 

LOCAL_STATIC_LIBRARIES := \
	libwlwpsclient 

LOCAL_CFLAGS:= -DTARGETENV_android -DLINUX -Dlinux -DBCMINTERNAL -DBCMDBG   -Wextra -DWPS_WIRELESS_ENROLLEE -D_TUDEBUGTRACE
ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif


LOCAL_C_INCLUDES :=  \
      $(base)/$(SRCBASE)/wps/android/include \
      $(base)/$(SRCBASE)/include \
      $(base)/$(SRCBASE)/common/include \
      $(base)/$(SRCBASE)/wps/common/include \
      $(base)/$(SRCBASE)/wps/linux/inc \

else
SUPPSRCBASE := $(SRCBASE)/../../wpa_supplicant
SUPPSRCBASEHEADER := $(SRCBASE)/../../wpa_supplicant

include $(SUPPSRCBASEHEADER)/wpa_supp/.config

ifndef CONFIG_OS
ifdef CONFIG_NATIVE_WINDOWS
CONFIG_OS=win32
else
CONFIG_OS=unix
endif
endif


L_CFLAGS:= -DTARGETENV_android -DLINUX -Dlinux -DBCMINTERNAL -DBCMDBG   -Wextra 
L_CFLAGS += -DCONFIG_CTRL_IFACE

OBJS_c = $(SUPPSRCBASE)/src/common/wpa_ctrl.c $(LOCALSRCBASE)/WLWPSService.cpp
OBJS_c += $(SUPPSRCBASE)/src/utils/os_$(CONFIG_OS).c

ifdef CONFIG_CTRL_IFACE
ifeq ($(CONFIG_CTRL_IFACE), y)
ifdef CONFIG_NATIVE_WINDOWS
CONFIG_CTRL_IFACE=named_pipe
else
CONFIG_CTRL_IFACE=unix
endif
endif

ifeq ($(CONFIG_CTRL_IFACE), unix)
L_CFLAGS += -DCONFIG_CTRL_IFACE_UNIX
endif
ifeq ($(CONFIG_CTRL_IFACE), udp)
L_CFLAGS += -DCONFIG_CTRL_IFACE_UDP
endif
ifeq ($(CONFIG_CTRL_IFACE), named_pipe)
L_CFLAGS += -DCONFIG_CTRL_IFACE_NAMED_PIPE
endif
OBJS += wpa_supp/ctrl_iface.c wpa_supp/ctrl_iface_$(CONFIG_CTRL_IFACE).c
endif

L_CFLAGS += -DCONFIG_NO_STDOUT_DEBUG

LOCAL_SHARED_LIBRARIES := libc libcutils libutils libandroid_runtime
LOCAL_CFLAGS := $(L_CFLAGS)
ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

LOCAL_SRC_FILES := $(OBJS_c)
LOCAL_C_INCLUDES := \
             $(SUPPSRCBASEHEADER)/src \
             /openssl/include \
             $(SUPPSRCBASEHEADER)/src/utils \
             $(SUPPSRCBASEHEADER)/src/common \
             $(SUPPSRCBASEHEADER)/src/radius \
             $(SUPPSRCBASEHEADER)/src/eap_server \
             $(SUPPSRCBASEHEADER)/src/eap_peer \
             $(SUPPSRCBASEHEADER)/src/eap_common \
             $(SUPPSRCBASEHEADER)/src/drivers \
             $(SUPPSRCBASEHEADER)/src/crypto \
             $(SUPPSRCBASEHEADER)/src/wps \
             $(SUPPSRCBASEHEADER)/src/tls \
             $(SUPPSRCBASEHEADER)/src/rsn_supp \
             $(SUPPSRCBASEHEADER)/src/l2_packet \
             $(SUPPSRCBASEHEADER)/src/eap_supp \
	     $(base)/$(SRCBASE)/wps/android/include \
	     $(base)/$(SRCBASE)/include \
	     $(base)/$(SRCBASE)/common/include \
	     $(base)/$(SRCBASE)/wps/common/include \
	     $(base)/$(SRCBASE)/wps/linux/inc \
	     $(base)/../../../../../kernel/include \

LOCAL_STATIC_LIBRARIES := libwlwpsclient 

endif

LOCAL_MODULE:= libwlwpsservice

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------
# Build JNI Library
# ---------------------------------
ifeq ($(CONFIG_BUILD_COMPONENTS_UI), y)

include $(CLEAR_VARS)

SRCBASE := ../../../src
LOCALSRCBASE := $(SRCBASE)/wps/android/wlwpsjnilib

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
  $(LOCALSRCBASE)/android_wl_wps_jni.cpp

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libnativehelper \
	libcutils \
	libutils \

LOCAL_STATIC_LIBRARIES := \
        libwlwpsclient \

# This is the target being built.
LOCAL_MODULE:= libwlwpsjni

      
# Also need the JNI headers.
LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
        $(base)/$(SRCBASE)/wps/android/include

# No special compiler flags.
LOCAL_CFLAGS +=
ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

# Don't prelink this library.  For more efficient code, you may want
# to add this library to the prelink map and set this to true. However,
# it's difficult to do this for applications that are not supplied as
# part of a system image.

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------
# Build Binder service 
# ---------------------------------

ifeq ($(CONFIG_BUILD_COMPONENTS_UI), y)

include $(CLEAR_VARS)

SRCBASE := ../../../src
LOCALSRCBASE := $(SRCBASE)/wps/android/wlwpsserver

LOCAL_SRC_FILES:= \
	$(LOCALSRCBASE)/main_wlwpsserver.cpp 

LOCAL_SHARED_LIBRARIES := \
	libwlwpsservice \

ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

LOCAL_C_INCLUDES := \
    $(base)/$(SRCBASE)/wps/android/include \
    $(base)/$(SRCBASE)/include \
    $(base)/$(SRCBASE)/common/include \
    $(base)/$(SRCBASE)/wps/linux/inc \
    $(base)/../../kernel/include \

LOCAL_MODULE:= wlwpsservice

include $(BUILD_EXECUTABLE)

endif

# ---------------------------------
# Build WPS API Library
# ---------------------------------

include $(CLEAR_VARS)

SRCBASE := ../../../src
BCMCRYPTO    := bcmcrypto
LOCAL_SRC_FILES:= \
   $(SRCBASE)/$(BCMCRYPTO)/aes.c \
   $(SRCBASE)/$(BCMCRYPTO)/rijndael-alg-fst.c \
   $(SRCBASE)/$(BCMCRYPTO)/dh.c \
   $(SRCBASE)/$(BCMCRYPTO)/bn.c \
   $(SRCBASE)/$(BCMCRYPTO)/sha256.c \
   $(SRCBASE)/$(BCMCRYPTO)/hmac_sha256.c \
   $(SRCBASE)/$(BCMCRYPTO)/random.c \
   $(SRCBASE)/wps/common/shared/wps_sslist.c \
   $(SRCBASE)/wps/common/shared/buffobj.c \
   $(SRCBASE)/wps/common/shared/reg_proto_utils.c \
   $(SRCBASE)/wps/common/shared/reg_proto_msg.c \
   $(SRCBASE)/wps/common/shared/tlv.c \
   $(SRCBASE)/wps/common/shared/state_machine.c \
   $(SRCBASE)/wps/common/shared/tutrace.c \
   $(SRCBASE)/wps/common/shared/wps_utils.c \
   $(SRCBASE)/wps/common/enrollee/enr_api.c \
   $(SRCBASE)/wps/common/shared/dev_config.c \
   $(SRCBASE)/wps/common/sta/sta_eap_sm.c \
   $(SRCBASE)/wps/common/enrollee/enr_reg_sm.c \
   $(SRCBASE)/wps/common/registrar/reg_sm.c \
   $(SRCBASE)/wps/linux/enr/wps_api.c \
   $(SRCBASE)/wps/linux/enr/wps_linux_hooks.c \
   $(SRCBASE)/wps/linux/enr/wl_wps.c \

LOCAL_SHARED_LIBRARIES := \
	libui libcutils libutils libsonivox 

LOCAL_CFLAGS:= -DTARGETENV_android -DLINUX -Dlinux -DBCMINTERNAL -DBCMDBG   -Wextra -DWPS_WIRELESS_ENROLLEE -D_TUDEBUGTRACE 
ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

LOCAL_MODULE:= libwlwps

LOCAL_LDLIBS += -lpthread

LOCAL_C_INCLUDES := \
      $(base)/$(SRCBASE)/include \
      $(base)/$(SRCBASE)/common/include \
      $(base)/$(SRCBASE)/wps/common/include \
      $(base)/$(SRCBASE)/wps/linux/inc \
      $(base)/$(SRCBASE)/include/bcmcrypto \
      $(base)/$(SRCBASE)/include/proto \
      $(base)/$(SRCBASE)/common/include/proto

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------
# Build WPS Executable
# ---------------------------------

include $(CLEAR_VARS)

SRCBASE := ../../../src

LOCAL_SRC_FILES:= \
 $(SRCBASE)/wps/linux/enr/wps_api_tester.c \

LOCAL_MODULE:= wpsapitester

LOCAL_LDLIBS += -lpthread

LOCAL_C_INCLUDES := \
      $(base)/$(SRCBASE)/include \
      $(base)/$(SRCBASE)/common/include \
      $(base)/$(SRCBASE)/wps/common/include \
      $(base)/$(SRCBASE)/wps/linux/inc \


LOCAL_CFLAGS:= -DTARGETENV_android -DLINUX -Dlinux -DBCMINTERNAL -DBCMDBG   -Wextra -DWPS_WIRELESS_ENROLLEE -D_TUDEBUGTRACE 

LOCAL_SHARED_LIBRARIES := libwlwps

ifeq ($(BUILD_ID), )
else
LOCAL_CFLAGS += -DANDROID_AFTERCUPCAKE
LOCAL_SHARED_LIBRARIES += libbinder
endif

include $(BUILD_EXECUTABLE)
