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
#############################################################################
ifdef COMSPEC
# Any DOS environment
BSEAV := $(shell cd ../../../BSEAV && cd)
NEXUS_TOP := $(shell cd ../../../nexus && cd)
else
BSEAV := $(shell cd ../../../BSEAV; pwd)
NEXUS_TOP := $(shell cd ../../../nexus; pwd)
endif

B_LIB_TOP=$(NEXUS_TOP)/lib
LIB=$(NEXUS_TOP)/../BSEAV/lib

PLATFORM ?= $(NEXUS_PLATFORM)
ifndef NEXUS_PLATFORM
$(error PLATFORM is not defined)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bin

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

# NEXUS flags
CFLAGS += -g $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# NXCLIENT flags
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)

#LDFLAGS += $(NXCLIENT_LDFLAGS) -lpthread

nxclient:
	make -C $(NEXUS_TOP)/nxclient/build

app: nxclient
	$(CC) -o $@ app.c $(CFLAGS) $(LDFLAGS)

#########
#### Figure out ARCH
#########
ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mips-linux
export B_REFSW_ARCH
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

OS ?= $(B_REFSW_OS)
ARCH ?= $(B_REFSW_ARCH)
DEBUG ?= $(B_REFSW_DEBUG)

THEAPPS = tr69c_stb
THELIBS = libtr69c.so

ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif


.PHONY: all apps clean

OBJDIR=$(B_REFSW_OBJ_ROOT)/BSEAV/app/tr69c

all: $(APPS)

$(OBJDIR)/exists:
	mkdir -p $(OBJDIR)
	@touch $@

$(APPS): $(NXCLIENT_SHARED_LIB)

$(NXCLIENT_SHARED_LIB):
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -L$(BINDIR) -lpthread -lm
endif

LDFLAGS += $(NXCLIENT_LDFLAGS) -lpthread


# Always build with debug
#CFLAGS += -g

CFLAGS += -I./bcmLibIF
CFLAGS += -I./inc
CFLAGS += -I./lib
CFLAGS += -I./main
CFLAGS += -I./nanoxml
CFLAGS += -I./SOAPParser
CFLAGS += -I./standard
CFLAGS += -I./webproto

ifeq ($(PLAYBACK_IP_SUPPORT),y)
CFLAGS += -I$(B_LIB_TOP)/playback_ip/include
CFLAGS += -DPLAYBACK_IP_SUPPORT=1
endif

ifeq ($(OPENSSL_SUPPORT),y)
OPENSSLDIR = $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/BSEAV/opensource/openssl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
CFLAGS += -I$(OPENSSLDIR)/include -DUSE_SSL
LDFLAGS += -lssl -lcrypto -L$(OPENSSLDIR)/lib
endif

ifeq ($(XML_DOC_SUPPORT),y)
CFLAGS += -DXML_DOC_SUPPORT
endif

ifeq ($(TR135_SUPPORT),y)
CFLAGS += -I./bcmLibIF/tr135
CFLAGS += -I./standard/tr135
CFLAGS += -DTR135_SUPPORT
SRCS +=	./bcmLibIF/tr135/bcmStbCapabilities.c \
		./bcmLibIF/tr135/bcmStbComponents.c \
		./bcmLibIF/tr135/bcmStbServiceMonitoring.c \
		./bcmLibIF/tr135/bcmStbService.c \
		./bcmLibIF/tr135/bcmTR135Objs.c \
		./bcmLibIF/tr135/bcmStbMisc.c \
		./standard/tr135/stbcapabilitiesparams.c \
		./standard/tr135/stbcomponentsparams.c \
		./standard/tr135/stbmiscparams.c \
		./standard/tr135/stbservicemonitoringparams.c \
		./standard/tr135/stbSpectrumAnalyzerParams.c \
		./standard/tr135/stbSpectrumAnalyzerHandlers.c
endif

ifeq ($(SPECTRUM_ANALYZER),y)
CFLAGS += -DSPECTRUM_ANALYZER
endif

ifeq ($(TR181_SUPPORT),y)
CFLAGS += -I./bcmLibIF/tr181
CFLAGS += -I./standard/tr181
CFLAGS += -I./moca/moca2 -finline-functions
CFLAGS += -DTR181_SUPPORT -Dinline=""
LDFLAGS += -L./moca/moca2
ifeq ($(B_REFSW_ARCH), arm-linux)
LDFLAGS += -lmoca-arm
else
LDFLAGS += -lmoca-mips
endif
SRCS +=	./bcmLibIF/tr181/bcmMoca.c \
		./bcmLibIF/tr181/bcmMocaInterface.c \
		./standard/tr181/mocaInterfaceParams.c \
		./moca/moca2/moca2.c \
		./bcmLibIF/tr181/bcmEthernet.c \
		./bcmLibIF/tr181/bcmEthernetInterface.c \
		./bcmLibIF/tr181/ethernet.c \
		./standard/tr181/ethernetParams.c
endif

SRCS += ./main/main.c \
		./main/utils.c \
		./main/event.c \
		./main/md5.c \
		./main/informer.c \
		./main/acsListener.c \
		./main/httpDownload.c \
		./lib/tr69clib_priv.c \
		./bcmLibIF/bcmWrapper.c \
		./nanoxml/nanoxml.c \
		./SOAPParser/RPCState.c \
		./SOAPParser/xmlParserSM.c \
		./SOAPParser/xmlTables.c \
		./SOAPParser/CPEframework.c \
		./webproto/protocol.c \
		./webproto/www.c \
		./webproto/wget.c \
		./standard/standardparams.c \
		./standard/rootparams.c \
		./standard/baselineparams.c \
		./standard/baselinehandlers.c

OBJS = $(foreach file, $(SRCS), $(OBJDIR)/$(notdir $(patsubst %.c, %.o, $(file))))

vpath %.c $(foreach file, $(SRCS), $(dir $(patsubst %.c, %.o, $(file))))

$(OBJDIR)/%.o: %.c ${OBJDIR}/exists
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)

libtr69c.so: lib/tr69clib.c
	@echo [Building... $@]
	$(Q_)$(CC) -shared -fPIC -o $(NEXUS_BIN_DIR)/libtr69c.so lib/tr69clib.c -lc $(CFLAGS)

tr69c_stb$(APPSUFFIX): $(OBJS) $(VXOBJS)
	@echo [Link... $@]

# Link with correct library (-lnexus_client) when building with NEXUS_MODE=client, strip out lnexus
ifeq ($(NEXUS_MODE),client)
	$(Q_)$(CC) -o $(NEXUS_BIN_DIR)/$@ $(filter %.o, $^) $(filter-out -lnexus, $(LDFLAGS)) $(OPENSSLLIBS)
else
	$(Q_)$(CC) -o $(NEXUS_BIN_DIR)/$@ $(filter %.o, $^) $(LDFLAGS) $(OPENSSLLIBS)
endif
#	cp -f $@ ${FONTS} ${BINDIR}

install: all
clean:
	-$(RM) -r $(OBJDIR) $(NEXUS_BIN_DIR)/$(THEAPPS) $(NEXUS_BIN_DIR)/libtr69c.so
