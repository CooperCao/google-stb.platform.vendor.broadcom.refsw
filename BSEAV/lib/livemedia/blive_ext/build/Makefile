############################################################
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
############################################################

B_THIS_DIR:=BSEAV/lib/livemedia/blive_ext/build
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

#  Sanity check... Make sure that $(NEXUS_TOP) exists and is accessible.
$(if $(realpath ${NEXUS_TOP}),,$(error Cannot find (or access) NEXUS_TOP: "$(NEXUS_TOP)"))

#  More Nexus definitions...
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

#  More sanity checks... By now, the following variables should be defined... fail if not.
$(if ${B_REFSW_ARCH},,$(error B_REFSW_ARCH is not defined))
$(if ${B_REFSW_OBJ_DIR},,$(error B_REFSW_OBJ_DIR is not defined))
$(if ${B_REFSW_OBJ_ROOT},,$(error B_REFSW_OBJ_ROOT is not defined))

include ../include/blive_ext.mak
include ../../livemedia.inc

include ../../../../api/build/tools.mak
BSEAV = ${NEXUS_TOP}/../BSEAV

include $(BSEAV)/build/refsw_inc.mak

BLIVE_EXT_ODIR := ${BLIVE_EXT_LIBDIR}
BLIVE_EXT_LIB = $(BLIVE_EXT_ODIR)/libblive_ext.a

.PHONY: check_environment clean
all: $(BLIVE_EXT_LIB)

ifeq ($(VERBOSE),)
MAKEFLAGS += -s
endif

ifeq ($(ANDROID_BUILD),y)
CFLAGS += $(NEXUS_CFLAGS)
endif

ifeq ($(SYSTEM),vxworks)
CFLAGS += -I$(WIND_BASE)/target/h
endif
CFLAGS += -W -Wall
CFLAGS += -I$(BLIVE_EXT_DIR)/include -I$(BLIVE_EXT_DIR)/src \
	-I$(BSEAV)/api/include \
	-I$(BSEAV)/lib/rtp \
    ${B_LIVEMEDIA_CFLAGS}

CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS) -fPIC
CXXFLAGS += $(CFLAGS)

vpath %.cpp $(BLIVE_EXT_DIR)/src

BLIVE_EXT_OBJS = bsap.o bscheduler.o

$(BLIVE_EXT_ODIR)/%.o: %.cpp         |     check_environment
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) -c $< -o $@

$(BLIVE_EXT_LIB): $(addprefix $(BLIVE_EXT_ODIR)/,$(BLIVE_EXT_OBJS))
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@ $^
	${Q_}$(RANLIB) $@

clean:
	-${Q_}$(RM) -rf $(BLIVE_EXT_ODIR)

check_environment:  $(BLIVE_EXT_ODIR)

$(BLIVE_EXT_ODIR):
	${Q_}$(MKDIR) "$@"

include $(BSEAV)/api/build/rules.mak
