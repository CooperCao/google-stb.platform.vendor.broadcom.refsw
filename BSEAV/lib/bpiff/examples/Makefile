############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#
############################################################

NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
BSEAV_TOP := $(shell cd ../../../../BSEAV; pwd)

#APPS = encode_piff playback_piff playback_piff_PR_drm
#SRCS = encode_piff.c playback_piff.c playback_piff_PR_drm.c
APPS = encode_playback_piff
SRCS = encode_playback_piff.c
OBJS = ${SRCS:.c=.o}

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
include $(BSEAV_TOP)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

ifeq ($(APPS),)
$(error Must define APPS)
endif

ifneq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
$(error unsupported Playready VERSION = ${MSDRM_PRDY_SDK_VERSION})
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
PIFF_INSTALL	:= lib/debug
else
PIFF_INSTALL	:= lib/retail
endif

include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

APP_BINS = $(APPS)

.PHONY: api clean prep_drmrootfs

all: do_prep $(APP_BINS) copy

$(APP_BINS):

LDFLAGS := ${COMMON_DRM_LDFLAGS}
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus${NEXUS_LIB_SUFFIX}
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
ifeq ($(SAGE_SUPPORT), y)
LDFLAGS := -L$(BSEAV_TOP)/lib/security/common_drm_tl/lib/${CMNDRM_DIR} -lcmndrm_tl
LDFLAGS += -L$(BSEAV_TOP)/lib/security/sage/srai/lib -lsrai
LDFLAGS += -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk_host
else
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/lib/${CMNDRM_DIR} -lcmndrm
LDFLAGS += -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk
endif
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy
LDFLAGS += $(B_OPENSSL_LDFLAGS)
LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR)/libbcrypt.so -lbcrypt
LDFLAGS += -L$(BSEAV_TOP)/lib/bpiff -lbpiff

#$(error XXX $(LDFLAGS))

# Always build with debug

# common crypto include path

CFLAGS += -I$(BSEAV_TOP)/lib/utils
CFLAGS += -I$(BSEAV_TOP)/lib/media
CFLAGS += -I${BSEAV_TOP}/lib/media/build
CFLAGS += -I${BSEAV_TOP}/lib/piff

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -g

# bpiff encoder
CFLAGS += -I$(BSEAV_TOP)/lib/bpiff/include
CFLAGS += ${COMMON_DRM_CFLAGS}
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media -I$(NEXUS_TOP)/../BSEAV/lib/utils -I$(NEXUS_TOP)/../BSEAV/lib/asf

#$(error XXX $(CFLAGS))

# This cleans nexus and local apps
clean:
	-$(RM) $(APP_BINS) *.d *.o *.out;

# Implicit rule for building local apps
#$(APPS): prep_openssl $(OBJS)
#	@echo [Link... $@]
#	@echo $(NEXUS_TOP)
#	@echo $(LDFLAGS)
#	${Q_} $(CC) -o $@ $@.o $(LDFLAGS)
#
#prep_openssl:
#	$(MAKE) -C $(B_OPENSSL_BUILD_DIR)

do_prep:
	$(MAKE) prep_drmrootfs && \
	$(MAKE) prep_bcrypt && \
	$(MAKE) prep_cmndrmprdy && \
	$(MAKE) prep_bpiff

prep_drmrootfs:
	@echo [Compiling drmrootfs dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install

prep_bcrypt:
	@echo [Compiling Bcrypt dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > /dev/null

prep_cmndrmprdy:
	@echo [..... Common DRM Playready target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm playready && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null

prep_bpiff:
	@echo [Compiling bpiff dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/bpiff all > /dev/null

.c.o:
	@echo [Compile... $<]
	$(CC) $(CFLAGS) -c -o $<
#	${Q_}$(CC) -c $< $(CFLAGS)

copy:
	@echo [Copying proper lib version...]
	cp -f $(BSEAV_TOP)/lib/bpiff/libbpiff.so $(NEXUS_BIN_DIR)/
	cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.* $(NEXUS_BIN_DIR)/
	cp -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR)/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT), y)
	cp -f $(BSEAV_TOP)/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
else
	cp -f $(BSEAV_TOP)/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
	@echo [Copying executables...]
	cp $(APPS) $(NEXUS_BIN_DIR)/
