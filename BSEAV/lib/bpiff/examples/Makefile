############################################################
#    (c)2012-2014 Broadcom Corporation
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

NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
BSEAV_TOP := $(shell cd ../../../../BSEAV; pwd)

#APPS = encode_piff playback_piff playback_piff_PR_drm
#SRCS = encode_piff.c playback_piff.c playback_piff_PR_drm.c
APPS = encode_playback_piff
SRCS = encode_playback_piff.c
OBJS = ${SRCS:.c=.o}

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc

ifeq ($(APPS),)
$(error Must define APPS)
endif

ifneq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
$(error unsupported Playready VERSION = ${MSDRM_PRDY_SDK_VERSION})
endif

ifeq ($(B_REFSW_DEBUG),n)
PIFF_INSTALL	:= lib/retail
else
PIFF_INSTALL	:= lib/debug
endif

include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc

APP_BINS = $(APPS)

.PHONY: api clean

all: $(APP_BINS) copy

$(APP_BINS): 

LDFLAGS := ${COMMON_DRM_LDFLAGS}
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus
LDFLAGS += -L$(BSEAV_TOP)/lib/drmrootfs/lib/${B_REFSW_OS} -ldrmrootfs
ifeq ($(SAGE_SUPPORT), y)
LDFLAGS := -L$(BSEAV_TOP)/lib/security/common_drm_tl/lib/${CMNDRM_DIR} -lcmndrm_tl
LDFLAGS += -L$(BSEAV_TOP)/lib/security/sage/srai/lib -lsrai
LDFLAGS += -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk_host
else
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/lib/${CMNDRM_DIR} -lcmndrm
LDFLAGS += -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib -lplayreadypk
endif
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy
LDFLAGS += $(B_OPENSSL_LDFLAGS)
LDFLAGS += -L$(BSEAV_TOP)/lib/security/bcrypt/lib -lbcrypt
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

.c.o:
	@echo [Compile... $<]
	$(CC) $(CFLAGS) -c -o $<
#	${Q_}$(CC) -c $< $(CFLAGS)

copy:
	@echo [Copying proper lib version...]
	cp -f $(BSEAV_TOP)/lib/bpiff/libbpiff.so $(NEXUS_BIN_DIR)/
	cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.* $(NEXUS_BIN_DIR)/
	cp -f $(BSEAV_TOP)/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR)/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT), y)
	cp -f $(BSEAV_TOP)/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
else
	cp -f $(BSEAV_TOP)/lib/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
	@echo [Copying executables...]
	cp $(APPS) $(NEXUS_BIN_DIR)/
