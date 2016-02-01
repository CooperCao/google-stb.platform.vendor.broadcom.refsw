############################################################
#     (c)2003-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################
# Nexus examples

PHONY := all nexus api clean clean_apps install
.PHONY: ${PHONY}

THEAPPS = \
	playback \
	message \
	record \
	decode \
	audio_decode \
	audio_playback \
	video

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(NEXUS_USE_97445_DBS),y)
NEXUS_FRONTEND_4538=y
endif

ifeq ($(B_REFSW_REAL_MAKE),)
ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd .. && cd)
else
NEXUS_TOP := $(shell cd ../; pwd)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/utils

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
ifeq ($(B_REFSW_OS),linuxkernel)
APPSUFFIX = .ko
APPS = $(addsuffix ${APPSUFFIX}, $(THEAPPS))
else
APPS = $(THEAPPS)
endif
endif




OBJDIR=./obj
vpath %.c ${NEXUS_TOP}/utils
all: $(APPS)

$(OBJDIR)/exists:
	mkdir -p $(OBJDIR)
	@touch $@

ifneq ($(NEXUS_PREBUILT_BINARY),y)
$(APPS): $(if ${B_NEXUS_API_BUILD_COMPLETED}, , api)
endif

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_LDFLAGS)
endif

# This builds the Nexus api
nexus api:
	$(MAKE) -C $(NEXUS_TOP)/build

# This cleans nexus and local apps
clean: clean_apps
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_apps:
	-$(RM) -r $(APPS) $(OBJDIR)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
LDFLAGS += ${NEXUS_EXTRA_LDFLAGS}

# Always build with debug
CFLAGS += -g

# media probe (only needed for playback and record)
BSEAV = $(NEXUS_TOP)/../BSEAV
include $(BSEAV)/lib/bfile/bfile.inc
ifeq ($(MEDIA_AVI_SUPPORT),y)
include $(BSEAV)/lib/avi/bavi.inc
endif
ifeq ($(MEDIA_ASF_SUPPORT),y)
include $(BSEAV)/lib/asf/basf.inc
endif
ifeq ($(MEDIA_RMFF_SUPPORT),y)
include $(BSEAV)/lib/media/rmff/brmff.inc
endif
include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES) $(BFILE_MEDIA_INCLUDES)) -I$(BSEAV)/lib/mpeg2_ts_parse -I$(BSEAV)/lib/tspsi
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))

DECODE_SRCS += ./cmdline_args.c ./namevalue.c ./fileio_custom.c ./frontend.c ./decoder_bitrate.c

DECODE_OBJS = $(foreach file, $(DECODE_SRCS), $(OBJDIR)/$(notdir $(patsubst %.c, %.o, $(file))))

vpath %.c $(foreach file, $(DECODE_SRCS) $(MESSAGE_SRCS), $(dir $(patsubst %.c, %.o, $(file))))

$(OBJDIR)/%.o: %.c ${OBJDIR}/exists
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS) ${CDEP_FLAG}

# this is not implicit because we don't link in $(DECODE_OBJS)
message: $(OBJDIR)/message.o $(VXOBJS)
	@echo [Link...... $@]
ifeq ($(B_REFSW_OS),linuxkernel)
	${Q_}$(LD) ${LDFLAGS} --strip-debug -r $(filter %.o, $^) ${NEXUS_BIN_DIR}/libnexus.a -o $@
else
	$(Q_)$(CC) -o $@ $(filter %.o, $^) $(LDFLAGS)
endif
	cp $@ ${NEXUS_BIN_DIR}

# cancel this implicit rule so we can do separate compile & link w/ implicit rules
%: %.c

%: $(OBJDIR)/%.o $(DECODE_OBJS) $(VXOBJS)
	@echo [Link...... $@]
ifeq ($(B_REFSW_OS),linuxkernel)
	${Q_}$(LD) ${LDFLAGS} --strip-debug -r $(filter %.o, $^) ${NEXUS_BIN_DIR}/libnexus.a -o $@
else
	$(Q_)$(CC) -o $@ $(filter %.o, $^) $(LDFLAGS)
endif
	cp $@ ${NEXUS_BIN_DIR}

ifeq ($(B_REFSW_OS),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS)
endif

install: all

-include ${OBJDIR}/*.d

endif # B_REFSW_REAL_MAKE
