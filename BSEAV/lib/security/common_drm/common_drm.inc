#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
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
#############################################################################

ifeq ($(NEXUS_TOP),)
$(error NEXUS_TOP is not defined! Make it point to where nexus, magnum, BSEAV exist in your view)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
COMMON_DRM_TOP ?= ${NEXUS_TOP}/../BSEAV/lib/security/common_drm

# Include .inc files from Common DRM, bcrypt and drmrootfs
include $(NEXUS_TOP)/../BSEAV/lib/security/common_drm/lib/cmndrm_dir.inc
include $(NEXUS_TOP)/../BSEAV/lib/security/bcrypt/bcrypt_libdir.inc
include $(NEXUS_TOP)/../BSEAV/lib/drmrootfs/drmrootfs_libdir.inc
include $(NEXUS_TOP)/../BSEAV/opensource/openssl/openssl.inc

# COMMON_DRM_INCLUDES defines the include paths without the -I
COMMON_DRM_INCLUDES = $(NEXUS_TOP)/../BSEAV/lib/security/common_drm/include \
                      $(NEXUS_TOP)/../BSEAV/lib/security/common_drm/include/priv \
                      $(NEXUS_TOP)/../BSEAV/lib/security/common_crypto/include \
                      $(NEXUS_TOP)/../BSEAV/lib/security/bcrypt/include \
                      $(NEXUS_TOP)/../BSEAV/lib/drmrootfs
ifeq ($(DTCP_IP_SUPPORT),y)
	COMMON_DRM_INCLUDES +=	$(NEXUS_TOP)/../BSEAV/lib/security/third_party/dtcp_ip
endif
ifeq ($(SAGE_SUPPORT),y)
	COMMON_DRM_INCLUDES +=	$(NEXUS_TOP)/../BSEAV/lib/security/common_drm/include/tl
endif

# Add -I in front of each of the COMMON_DRM_INCLUDES members + define other -D flags if necessary
COMMON_DRM_CFLAGS = $(addprefix -I, $(COMMON_DRM_INCLUDES))
COMMON_DRM_CFLAGS += $(B_OPENSSL_CFLAGS)

# Add common LDFLAGS
COMMON_DRM_LDFLAGS := -lbcrypt -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/${BCRYPT_LIBDIR} \
		      -ldrmrootfs -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} \
		      $(B_OPENSSL_LDFLAGS)


COMMON_DRM_BIN_DEPS_LIST = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so \
			   $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR)/libbcrypt.so \
			   $(B_OPENSSL_LIB_INSTALL_DIR)/lib*.*

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
	COMMON_DRM_BIN_DEPS_LIST += ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm64.so
else
	COMMON_DRM_BIN_DEPS_LIST += ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm.so
endif

# Add SAGE-specific LDFLAGS and append SRAI to the list of bin file dependencies
ifeq ($(SAGE_SUPPORT),y)
COMMON_DRM_LDFLAGS += -lsrai -L$(NEXUS_TOP)/../BSEAV/lib/security/sage/srai/lib
$(shell $(make -C $(NEXUS_TOP)/../BSEAV/lib/security/sage/srai re))
COMMON_DRM_BIN_DEPS_LIST += ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai/libsrai.so \
                            ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm_tl.so
endif

# Determine which non-SAGE common_drm components are present
COMMON_DRM_ADOBE := $(shell test -e $(COMMON_DRM_TOP)/drm/adobe/drm_adobe.c && test -e $(COMMON_DRM_TOP)/drm/pmc/pmc_adobe.c && test -e $(COMMON_DRM_TOP)/drm/pmc/pmc_core.c && echo y)
COMMON_DRM_OCAP := $(shell test -e $(COMMON_DRM_TOP)/drm/ocap/drm_ocap.c && echo y)
COMMON_DRM_DTCPIP := $(shell test -e $(COMMON_DRM_TOP)/drm/dtcp_ip/drm_dtcp_ip.c && echo y)
COMMON_DRM_NETFLIX := $(shell test -e $(COMMON_DRM_TOP)/drm/netflix/drm_netflix.c && echo y)
COMMON_DRM_WIDEVINE := $(shell test -e $(COMMON_DRM_TOP)/drm/widevine_cenc/drm_widevine_cenc.c && test -e $(COMMON_DRM_TOP)/drm/cenc_key_region/drm_cenc_key_region.c && echo y)

#$(warning Detected non-SAGE CommonDRM components for ADOBE=$(COMMON_DRM_ADOBE) OCAP=$(COMMON_DRM_OCAP) DTCPIP=$(COMMON_DRM_DTCPIP) NETFLIX=$(COMMON_DRM_NETFLIX) WIDEVINE=$(COMMON_DRM_WIDEVINE) )
