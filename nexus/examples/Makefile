############################################################
#	  (c)2003-2015 Broadcom Corporation
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
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
############################################################
# Nexus examples
B_THIS_DIR:=nexus/examples
PHONY := all clean nexus clean_nexus api clean_api clean_apps install nexus_headers
.PHONY: ${PHONY}
.NOTPARALLEL: api apps

THEAPPS = \
	boot \
	decode \
	dynamic_heap \
	power_down \
	memconfig \
	move_allocs \
	heaps

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/examples

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

include $(NEXUS_TOP)/examples/Makefile_prologue.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/examples

all: apps

apps: $(APPS) subdirs

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)
CLIENT_LDFLAGS := $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_SERVER_LDFLAGS)
CLIENT_LDFLAGS := $(ANDROID_CLIENT_LDFLAGS)
endif


nexus_print_options nexus_headers:
	$(MAKE) -C $(NEXUS_TOP)/build $@

SUBDIRS := 3dtv audio cec dvr encoder frontend graphics hdmi_input hdmi_output io legacy_3dtv lipsync multiprocess security transport video

subdirs: $(if ${B_NEXUS_API_BUILD_COMPLETED}, , api)
	@for i in $(SUBDIRS); do $(MAKE) -C ${NEXUS_TOP}/examples/$$i B_NEXUS_API_BUILD_COMPLETED=1 B_REFSW_REAL_MAKE= apps || exit 1; done

clean_subdirs:
	${MAKE} -C ${NEXUS_TOP}/examples/multiprocess B_REFSW_REAL_MAKE= clean_apps
	@for i in $(SUBDIRS); do $(MAKE) -C ${NEXUS_TOP}/examples/$$i B_REFSW_REAL_MAKE= clean_apps; done

# This cleans nexus and local apps
clean: clean_api clean_apps

# Clean api except on binary version
ifeq ($(NEXUS_PREBUILT_BINARY),y)
nexus api:
clean_nexus clean_api:
else
nexus api:
	$(MAKE) -C $(NEXUS_TOP)/build
clean_nexus clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean
endif

clean_apps: clean_subdirs
	-$(RM) $(APPS) *.d *.o *.out

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(patsubst %.o,%.c,$(EXTRA_OBJS)) $(if ${B_NEXUS_API_BUILD_COMPLETED}, , api)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	cp -f $@ ${NEXUS_BIN_DIR}

-include *.d

install: all
	cp -f ${APPS} ${NEXUS_BIN_DIR}

endif # B_REFSW_REAL_MAKE
