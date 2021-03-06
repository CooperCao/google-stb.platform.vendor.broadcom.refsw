############################################################
#    (c)2012-2013 Broadcom Corporation
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
############################################################
ifeq ($(URSR_TOP),)
$(warning URSR_TOP is not defined. Establising relative paths to nexus and BSEAV)
NEXUS_TOP := $(shell cd ../../../../../nexus; pwd)
BSEAV_TOP := $(shell cd ../../../../../BSEAV; pwd)
else
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV
endif

$(warning target = $(target))

ifeq ($(target), )
$(error Makefile usage:  make target=<directory> (i.e. make target=widevine))
else
include ./$(target)/$(target)_app.inc
TARGET_DIR := BSEAV/lib/security/common_drm/drm
endif

APPS += $(DRM_APPS)
SRCS = $(DRM_APP_SRCS)
OBJS = ${SRCS:.c=.o}

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# create 'target' dir in out of source dir
$(shell mkdir -p ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target})

ifeq ($(OVERRIDE_USE_SECURE_REPO_DEPS),y)
include ../../common_drm.inc

ifeq ($(SAGE_SUPPORT),y)
include $(BSEAV_TOP)/lib/security/sage/srai/srai.inc
endif

ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
include ../../prdy_libdir.inc
endif
endif
else
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
endif
endif
endif

ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif


TARGET_OBJS = $(addprefix  ${target}/, ${OBJS})
APP_BINS = $(APPS)
#$(warning APP_BINS = $(APP_BINS))
INSTALLED_APPS = $(addprefix  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/, ${APPS})
CLEAN_APPS = $(addprefix  $(NEXUS_BIN_DIR)/, ${APPS})
#$(warning INSTALLED_APPS = ${INSTALLED_APPS})
#$(warning CLEAN_APPS = ${CLEAN_APPS})

all:  $(APPS) copy


# For linux builds, link to the correct libraries
LDFLAGS := ${COMMON_DRM_LDFLAGS}
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus

CFLAGS += $(DRM_APP_CFLAGS)
LDFLAGS += $(DRM_APP_LDFLAGS)

ifeq ($(SAGE_SUPPORT),y)
LDFLAGS += $(SRAI_LDFLAGS)
CFLAGS += -I$(BSEAV_TOP)/../magnum/syslib/sagelib/include
CFLAGS += -I$(BSEAV_TOP)/lib/security/sage/srai/include
endif

PLAYREADY_LIB_PATH :=
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy

ifeq ($(SAGE_SUPPORT),y)
LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk_host
PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
else
LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk
PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
else
$(error Unsupported Playready Version)
endif
endif

# This cleans nexus and local apps
clean:
	rm -rf ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}
	rm -f ${CLEAN_APPS}


# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

APP_INCLUDES += ${DRM_APP_INCLUDES}

# Always build with debug
CFLAGS += -g


# Common DRM dependent CFLAGS
CFLAGS += ${COMMON_DRM_CFLAGS}
CFLAGS += -I../include/priv

ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
CFLAGS += -I$(BSEAV_TOP)/lib/utils
CFLAGS += -I$(BSEAV_TOP)/lib/media
CFLAGS += -I${BSEAV_TOP}/lib/media/build
endif

# Implicit rule for building local apps
$(APPS): prep_openssl prep_nexus prep_bcrypt prep_srai prep_cmndrmprdy prep_cmndrm_tl $(TARGET_OBJS)
	@echo [Link... $@]
	${Q_} $(CC) -o ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@ ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@.o $(LDFLAGS)


prep_openssl:
	echo [..... prep_openssl with j option];
	$(Q_)$(MAKE) -j4 -C $(B_OPENSSL_BUILD_DIR) > /dev/null

prep_srai:
ifeq ($(SAGE_SUPPORT),y)
	echo [..... prep_srai];
	$(Q)$(MAKE) -C ../../sage/srai re
endif

prep_cmndrm_tl:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... Common DRM target];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null
endif

prep_cmndrmprdy:
ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
	@echo [..... Common DRM Playready target];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm playready install > /dev/null
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
	@echo [Compiling Bcrypt dependency ....];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > /dev/null


${target}/%.o: ${target}/%.c
	@ echo [Compiling  with $(CC).... $< ]
	@${CC} ${CFLAGS} -c -o  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/$@ $<


copy:
	@echo [Copying executables...]
	cp -f $(INSTALLED_APPS) $(NEXUS_BIN_DIR)
	@echo [Copy Common DRM and all dependencies...]
	cp -f -P ${COMMON_DRM_BIN_DEPS_LIST} $(NEXUS_BIN_DIR)
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
ifeq ($(MSDRM_PRDY_SDK_VERSION), 2.5)
	@echo [Copy $(target) dependencies...]
	cp -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR)/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT),y)
	cp -f ${BSEAV}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
else
	cp -f ${BSEAV}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
endif
endif
