#############################################################################
#    (c)2009-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################

#Identify this library
#
B_LIBS += B_DTCP_IP_LIB

#
# Sanity check that NEXUS_TOP is defined
#
ifndef NEXUS_TOP
    $(error NEXUS_TOP is not defined.)
endif

# Identify object directory
B_DTCP_IP_LIB_OUTPUT_NAME := b_dtcp_ip
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
B_DTCP_IP_LIB_OBJ_DIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/$(B_DTCP_IP_LIB_OUTPUT_NAME)

#test for DTCP-IP sources
DTCP_IP_SOURCES_AVAILABLE:= $(shell test -e $(B_LIB_TOP)/dtcp_ip/src/b_dtcp_applib.c   && echo y)
DTCP_IP_STUB_SOURCES_AVAILABLE:= $(shell test -e $(B_LIB_TOP)/dtcp_ip/stubs/dtcp_stubs.c   && echo y)

#  Decide whether we should build the lib from real source, or from the stubs (that do nothing
#  but allow everything to build).

#  Start by assuming that we building the stubs (default).
B_DTCP_IP_BUILD_MODE := stubs
#  But if DTCP_IP_SUPPORT is enabled, build the real thing.
ifeq ($(DTCP_IP_SUPPORT),y)
    B_DTCP_IP_BUILD_MODE := source

    ifeq ($(SSL_SUPPORT),n)
        $(error SSL_SUPPORT is disabled, cannot enable DTCP_IP_SUPPORT)
    endif #SSL_SUPPORT

    ifneq ($(NEXUS_COMMON_CRYPTO_SUPPORT),y)
        $(error NEXUS_COMMON_CRYPTO_SUPPORT is disabled, cannot enable DTCP_IP_SUPPORT)
    endif #SSL_SUPPORT

    ifneq ($(DTCP_IP_HARDWARE_ENCRYPTION),n)
        export DTCP_IP_HARDWARE_ENCRYPTION=y
    endif #DTCP_IP_HARDWARE_ENCRYPTION
    ifneq ($(DTCP_IP_HARDWARE_DECRYPTION),n)
        export DTCP_IP_HARDWARE_DECRYPTION=y
    endif #DTCP_IP_HARDWARE_DECRYPTION

endif

#  If we want to build from source, make sure the source is there, fall back to stubs if not.
ifeq ($(B_DTCP_IP_BUILD_MODE),source)
    ifneq ($(DTCP_IP_SOURCES_AVAILABLE),y)
        $(warning Missing sources in $(B_LIB_TOP)/dtcp_ip/src... Building with stubs...)
        B_DTCP_IP_BUILD_MODE := stubs
    endif
endif

#  If we want to build from stubs, make sure the stub source is there, if not then give up.
ifeq ($(B_DTCP_IP_BUILD_MODE),stubs)
    ifneq ($(DTCP_IP_STUB_SOURCES_AVAILABLE),y)
        $(error Missing dtcp_ip stub sources in $(B_LIB_TOP)/dtcp_ip/stubs... Cannot continue.)
    endif
endif

#
#Define source files in this library
#
DTCP_TOP_DIR := $(B_LIB_TOP)/dtcp_ip

ifeq (${B_DTCP_IP_BUILD_MODE},stubs)
B_DTCP_IP_LIB_SOURCES = $(DTCP_TOP_DIR)/stubs/dtcp_stubs.c  # for building stub version of dtcp_ip library.
else
B_DTCP_IP_LIB_SOURCES = $(DTCP_TOP_DIR)/src/b_dtcp_ake.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ake_msg.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_applib.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_applib_priv.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_content.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_exch_key.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ip_ake.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ip_ake_msg.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ip_stack.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ip_transport.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_ip_transport_priv.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_srm.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_stack.c \
                        $(DTCP_TOP_DIR)/src/b_dtcp_stream.c \
                        $(DTCP_TOP_DIR)/src/b_ecc_wrapper.c \
                        $(DTCP_TOP_DIR)/src/b_socket_wrapper.c
endif
#
#Define public include path for this library
#
ifndef (B_OPENSSL_INC_INSTALL_DIR)
    include $(B_LIB_TOP)/../../BSEAV/lib/openssl/openssl.inc
endif

B_DTCP_IP_LIB_PUBLIC_INCLUDES += $(DTCP_TOP_DIR)/include \
                        $(B_LIB_TOP)/os/include \
                        $(B_LIB_TOP)/os/include/linuxuser \
                        $(B_OPENSSL_INC_INSTALL_DIR)

#
#Define private include path(s) for this library
#

#
#Defines extra C definations, These are from original DTCP package.
#NOTE: the DTCP_IP_HARDWARE_DECRYPTION need to be defined, if you wan to use
#Broadcom's M2M hardware to decrypt DTCP content stream.
#
B_DTCP_IP_LIB_DEFINES += LINUX_BUILD _POSIX \
                                __cdecl="" \
                                __stdcall=""

B_DTCP_IP_LIB_DEFINES += B_HAS_DTCP_IP
B_DTCP_IP_LIB_DEFINES += DTCP_IP_SUPPORT

ifeq ($(DTCP_IP_HARDWARE_DECRYPTION), y)
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_HW_DECRYPTION
endif

ifeq ($(DTCP_IP_HARDWARE_ENCRYPTION), y)
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_HW_ENCRYPTION
endif

#ifeq ($(DTCP_IP_CLOSE_SOCKET_ON_AKE_OK), y)
# Make it default on!
B_DTCP_IP_LIB_DEFINES += CLOSE_SOCKET_ON_AKE_OK
#endif

ifeq ($(DTCP_IP_DO_NOT_CLEAN_HCI), y)
B_DTCP_IP_LIB_DEFINES += DO_NOT_CLEAR_DTCP_HCI
endif

ifneq ($(DTCP_IP_DATA_BRCM), n)
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_DATA_BRCM
endif #DTCP_IP_DATA_BRCM

# Using common DRM for content encryption/decryption
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT

#build with common DRM key provisioning by default.
ifneq (${B_DTCP_IP_BUILD_MODE},stubs)
ifeq ($(DTCP_IP_USE_EXT_VENDOR_LIB),y)
B_DTCP_IP_LIB_LDFLAGS +=  -L$(B_LIB_TOP)/../../BSEAV/lib/security/third_party/dtcp_ip -lb_dtcp_ip_vendor
else
B_DTCP_IP_LIB_SOURCES += $(B_LIB_TOP)/../../BSEAV/lib/security/third_party/dtcp_ip/drm_dtcp_ip_vendor.c
endif
endif

include  $(B_LIB_TOP)/../../BSEAV/lib/security/common_drm/common_drm.inc
B_DTCP_IP_LIB_PUBLIC_INCLUDES += ${COMMON_DRM_INCLUDES}

ifneq (${B_DTCP_IP_BUILD_MODE},stubs)
    B_DTCP_IP_LIB_LDFLAGS += ${COMMON_DRM_LDFLAGS}
endif

#build with legacy production key provisioning if requested.
ifeq ($(DTCP_IP_LEGACY_PRODUCTION_KEY), y)
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_LEGACY_PRODUCTION_KEY
B_DTCP_IP_LIB_SOURCES += $(DTCP_TOP_DIR)/src/decrypto.c


B_DTCP_IP_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../../nexus/modules/security/$(BCHP_CHIP)/include \
                                 $(B_LIB_TOP)/../../nexus/extensions/security/keyladder/$(BCHP_CHIP)/include
endif

B_DTCP_IP_LIB_CFLAGS += $(addprefix -I,$(B_DTCP_IP_LIB_PUBLIC_INCLUDES)) $(addprefix -D,$(B_DTCP_IP_LIB_DEFINES))
B_DTCP_IP_LIB_LDFLAGS += -L$(B_DTCP_IP_LIB_OBJ_DIR)  -l$(B_DTCP_IP_LIB_OUTPUT_NAME)
