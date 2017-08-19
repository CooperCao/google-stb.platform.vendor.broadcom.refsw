#
#  Broadcom Proprietary and Confidential. Copyright (C) 2017,
#  All Rights Reserved.
#  
#  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
#  the contents of this file may not be disclosed to third parties, copied
#  or duplicated in any form, in whole or in part, without the prior
#  written permission of Broadcom.
#
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
#
#  <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id$
#

ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	dhd/exe/dhdu.c \
	dhd/exe/dhdu_linux.c \
	shared/bcmutils.c \
	shared/miniopt.c

LOCAL_MODULE := dhdarm_android
LOCAL_CFLAGS := -DSDTEST -DTARGETENV_android -Dlinux -DLINUX
LOCAL_CFLAGS += -DBCMDONGLEHOST
LOCAL_C_INCLUDES +=$(LOCAL_PATH)/include $(LOCAL_PATH)/../../../../kernel/include $(LOCAL_PATH)/../components/shared $(LOCAL_PATH)/shared/bcmwifi/include
LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_STATIC_LIBRARIES := libc

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug tests

include $(BUILD_EXECUTABLE)

#ifeq ($(ESTA_POSTMOGRIFY_REMOVAL), true) 

# Build WL Utility
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	shared/bcm_app_utils.c \
	shared/bcmutils.c \
	shared/bcmwifi/src/bcmwifi_channels.c \
	shared/bcmxtlv.c \
	shared/miniopt.c \
	wl/exe/wlu.c \
	wl/exe/wlu_client_shared.c \
	wl/exe/wlu_cmd.c \
	wl/exe/wlu_common.c \
	wl/exe/wlu_iov.c \
	wl/exe/wlu_linux.c \
	wl/exe/wlu_pipe.c \
	wl/exe/wlu_pipe_linux.c \
	wl/exe/wlu_rates_matrix.c \
	wl/exe/wlu_subcounters.c \
	wl/exe/wluc_ampdu.c \
	wl/exe/wluc_ampdu_cmn.c \
	wl/exe/wluc_ap.c \
	wl/exe/wluc_arpoe.c \
	wl/exe/wluc_bmac.c \
	wl/exe/wluc_bssload.c \
	wl/exe/wluc_btcx.c \
	wl/exe/wluc_cac.c \
	wl/exe/wluc_ht.c \
	wl/exe/wluc_interfere.c \
	wl/exe/wluc_keymgmt.c \
	wl/exe/wluc_keep_alive.c \
	wl/exe/wluc_led.c \
	wl/exe/wluc_lq.c \
	wl/exe/wluc_ltecx.c \
	wl/exe/wluc_mfp.c \
	wl/exe/wluc_obss.c \
	wl/exe/wluc_offloads.c \
	wl/exe/wluc_ota_test.c \
	wl/exe/wluc_phy.c \
	wl/exe/wluc_pkt_filter.c \
	wl/exe/wluc_prot_obss.c \
	wl/exe/wluc_relmcast.c \
	wl/exe/wluc_rrm.c \
	wl/exe/wluc_seq_cmds.c \
	wl/exe/wluc_scan.c \
	wl/exe/wluc_stf.c \
	wl/exe/wluc_toe.c \
	wl/exe/wluc_tpc.c \
	wl/exe/wluc_wds.c \
	wl/exe/wluc_wnm.c \
	wl/exe/wluc_wowl.c \
	wl/ppr/src/wlc_ppr.c

LOCAL_MODULE := wlarm_android
LOCAL_CFLAGS := -DBCMWPA2 -DTARGETENV_android -DLINUX -Dlinux
LOCAL_CFLAGS += -DRWL_WIFI -DRWL_SOCKET -DRWL_DONGLE
LOCAL_C_INCLUDES +=$(LOCAL_PATH)/include $(LOCAL_PATH)/../../../../kernel/include $(LOCAL_PATH)/../components/shared $(LOCAL_PATH)/shared/bcmwifi/include $(LOCAL_PATH)/wl/ppr/include
LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_STATIC_LIBRARIES := libc

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug tests
include $(BUILD_EXECUTABLE)

# Build WLM library
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	wl/exe/wlm.c \
	wl/exe/wlu.c \
	wl/exe/wlu_common.c \
	wl/exe/wlu_cmd.c \
	wl/exe/wlu_iov.c \
	wl/exe/wlu_linux.c \
	wl/exe/wlu_client_shared.c \
	wl/exe/wlu_pipe_linux.c \
	wl/exe/wlu_pipe.c \
	wl/exe/wlu_rates_matrix.c \
	wl/exe/wlu_subcounters.c \
	wl/ppr/src/wlc_ppr.c \
	shared/bcmutils.c \
	shared/bcmwifi/src/bcmwifi_channels.c \
	shared/bcm_app_utils.c \
	shared/miniopt.c

LOCAL_MODULE := wlmarm_android
LOCAL_CFLAGS := -DBCMWPA2 -DTARGETENV_android -Dlinux -DLINUX
LOCAL_CFLAGS += -DRWL_DONGLE -DRWL_SOCKET -DRWL_WIFI -DWLTEST -DWLMSO
LOCAL_C_INCLUDES +=$(LOCAL_PATH)/include $(LOCAL_PATH)/../../../../../kernel/include $(LOCAL_PATH)/../components/shared $(LOCAL_PATH)/shared/bcmwifi/include $(LOCAL_PATH)/wl/ppr/include
LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_STATIC_LIBRARIES := libc

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug tests
include $(BUILD_STATIC_LIBRARY)

#include $(BUILD_EXECUTABLE)
#endif
#endif /* !defined(ESTA_POSTMOGRIFY_REMOVAL) */
endif  # TARGET_SIMULATOR != true
