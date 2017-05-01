############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
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
include ../../prdy_libdir.inc
endif
else
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
#$(warning GCC_GTEQ_485 =$(GCC_GTEQ_485))
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

all:  $(APPS)
	@$(MAKE) copy


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

ifeq ($(SAGE_VERSION), 2x)
PRDY_LIB_PATH := ${BSEAV_TOP}/thirdparty/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/$(SAGE_VERSION)
else
PRDY_LIB_PATH := ${BSEAV_TOP}/thirdparty/playready/2.5/bin/${ARCH_DIR_PRDY}/lib
endif

PLAYREADY_LIB_PATH :=
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))

LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy

ifeq ($(SAGE_SUPPORT),y)
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
LDFLAGS += -L${PRDY_LIB_PATH} -lplayreadypk64_host
PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk64_host.so  ${NEXUS_BIN_DIR}
else
LDFLAGS += -L${PRDY_LIB_PATH} -lplayreadypk_host
PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
endif
else
ifeq "$(GCC_GTEQ_485)" "1"
LDFLAGS +=  -L${PRDY_LIB_PATH}/eglibc -lplayreadypk
else
LDFLAGS +=  -L${PRDY_LIB_PATH} -lplayreadypk
endif
PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk.so  ${NEXUS_BIN_DIR}
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

CFLAGS += -I$(BSEAV_TOP)/lib/utils
CFLAGS += -I$(BSEAV_TOP)/lib/media
CFLAGS += -I${BSEAV_TOP}/lib/media/build

# Implicit rule for building local apps
$(APPS): do_prep
	@$(MAKE) $(TARGET_OBJS) && \
	echo [Link... $@] && \
	$(CC) -o ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@ ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@.o $(LDFLAGS)

do_prep:
	@$(MAKE) prep_openssl && \
	$(MAKE) prep_nexus && \
	$(MAKE) prep_drmrootfs && \
	$(MAKE) prep_bcrypt && \
	$(MAKE) prep_srai && \
	$(MAKE) prep_cmndrmprdy && \
	$(MAKE) prep_cmndrm_tl

prep_openssl:
	@echo [..... prep_openssl] && \
	$(MAKE) -C $(B_OPENSSL_BUILD_DIR) > /dev/null

prep_srai:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... prep_srai] && \
	$(MAKE) -C ../../sage/srai re
endif

prep_cmndrm_tl:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... Common DRM target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null
endif

prep_cmndrmprdy:
	@echo [..... Common DRM Playready target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm playready && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null

prep_nexus:
ifeq ($(NXCLIENT_SUPPORT),y)
	@if [ ! -e ${NEXUS_BIN_DIR}/nxserver ]; then \
		echo [ building nxclient .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
else
	@if [ ! -e ${NEXUS_BIN_DIR}/libnexus.so ]; then \
		echo [ building nexus .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi
endif

prep_drmrootfs:
	@echo [Compiling drmrootfs dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > /dev/null

prep_bcrypt:
	@echo [Compiling Bcrypt dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > /dev/null

${target}/%.o: ${target}/%.c
	@echo [Compiling  with $(CC).... $< ] && \
	${CC} ${CFLAGS} -c -o  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/$@ $<

copy:
	@echo [Copying executables...]
	cp -f $(INSTALLED_APPS) $(NEXUS_BIN_DIR)
	@echo [Copy Common DRM and all dependencies...]
	cp -f -P ${COMMON_DRM_BIN_DEPS_LIST} $(NEXUS_BIN_DIR)
ifeq ($(filter $(target), playready playready_svp urr_toggling), $(target))
	@echo [Copy $(target) dependencies. $(CMNDRM_PRDY_LIBDIR)..]
	cp -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR)/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT),y)
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
	cp -f ${PRDY_LIB_PATH}/libplayreadypk64_host.so  ${NEXUS_BIN_DIR}
else
	cp -f ${PRDY_LIB_PATH}/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
endif
else
ifeq "$(GCC_GTEQ_485)" "1"
	cp -f ${PRDY_LIB_PATH}/eglibc/libplayreadypk.so  ${NEXUS_BIN_DIR}
else
	cp -f ${PRDY_LIB_PATH}/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
endif
endif
