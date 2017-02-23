#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################
ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../../../nexus && cd)
else
NEXUS_TOP := $(shell cd ../../../nexus; pwd)
endif
LIB=$(NEXUS_TOP)/../BSEAV/lib

NEXUS_PLATFORM ?= $(PLATFORM)
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

ifdef APP
APPS=$(APP)
else
THEAPPS = jpeg_slideshow show_jpeg
endif

ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif

.PHONY: api clean libjpeg config-libjpeg

all: libjpeg $(APPS)
$(APPS): api

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -L$(B_REFSW_OBJ_ROOT)/nexus/bin -lnexus -lpthread
endif

api:
	$(MAKE) -C $(NEXUS_TOP)/build

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

LIBJPEG_ODIR=$(B_REFSW_OBJ_ROOT)/BSEAV/opensource/jpeg-6b
CFLAGS += -I$(LIBJPEG_ODIR)
LDFLAGS += -L$(LIBJPEG_ODIR) -ljpeg

# Always build with debug
CFLAGS += -g

ifeq ($(NEXUS_DTV_PLATFORM),y)
SRCS = b_hw_jpeg.c
else
SRCS = b_soft_jpeg.c
endif
OBJS = $(patsubst %.c,%.o,$(SRCS))

%.o: %.c
	@echo [Compile... $<]
	$(Q_)$(CC) -c -o $@ $^ $(CFLAGS)

show_jpeg: show_jpeg.o $(OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ $(NEXUS_TOP)/bin

jpeg_slideshow: jpeg_slideshow.o $(OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ $(NEXUS_TOP)/bin

ifeq ($(SYSTEM),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
endif

install: all

# There is no separate configure step for bwin, so handle it with make,
# but don't configure if Makefile already exists.
libjpeg: $(LIBJPEG_ODIR)/libjpeg.a

# configure libjpeg if Makefile doesn't exist
config-libjpeg:
#speed of machine vs. granularity of creation/modification creates the need for a little sleep here
	@if [ ! -e $(LIBJPEG_ODIR)/Makefile ]; then \
		echo [Config.... libjpeg]; \
		cd $(LIBJPEG_ODIR); RANLIB=$(RANLIB) AR="$(AR) rc" CC=$(CC) sh ./configure 1> /dev/null; \
		sleep 1; \
	fi

$(LIBJPEG_ODIR)/libjpeg.a: config-libjpeg
	@if ! $(MAKE) -q -C $(LIBJPEG_ODIR) libjpeg.a; then \
		echo [Build..... libjpeg]; \
		$(MAKE) -C $(LIBJPEG_ODIR) libjpeg.a; \
	fi

clean-libjpeg:
	@if [ -e $(LIBJPEG_ODIR)/Makefile ]; then \
		$(MAKE) -C $(LIBJPEG_ODIR) distclean; \
	fi
