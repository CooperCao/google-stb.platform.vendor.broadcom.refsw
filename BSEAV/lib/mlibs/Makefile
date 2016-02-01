#############################################################################
# (c) 2003-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################


# ARCH allows you to put all the binaries into a subdirectory
# so that you can support multiple platforms at the same time.
# If ARCH is blank, then use ., which puts the binaries in
# the current directory.
#
# For instance, ARCH=mipsel-linux or ARCH=i386-linux
#
# However, ARCH does not change your compiler definitions.
# Instead you should define CXX and AR in your environment.
#

include ../../api/build/tools.mak
BSEAV = $(shell cd "../.." && ${PWD})
include $(BSEAV)/build/refsw_inc.mak
BWIN_SUPPORT = y

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

ODIR=$(B_REFSW_OBJ_ROOT)/BSEAV/lib/mlibs/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

MCOMMON_LIB=$(ODIR)/libmcommon.a
MNET_LIB=$(ODIR)/libmnet.a

MCOMMON_SRC=\
	murl.cpp mstring.cpp mgeom.cpp mstringlist.cpp \
	mvoidlist.cpp mregexp.cpp mstringhash.cpp

MNET_SRC=\
	mxmlelement.cpp mxmlparser.cpp

MCOMMON_OBJECTS=$(foreach src,$(MCOMMON_SRC:.cpp=.o),$(ODIR)/$(src))
MNET_OBJECTS=$(foreach src,$(MNET_SRC:.cpp=.o),$(ODIR)/$(src))

CXXFLAGS = -MD -O2 -fno-rtti -fno-exceptions
CXXFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)
CXXFLAGS += -fPIC

.PHONY: mcommon mnet

all: mcommon mnet

mcommon: dirs $(MCOMMON_LIB)

mnet: dirs $(MNET_LIB)

ifeq ($(OSTYPE),linux)
dirs:
	-@if [ ! -d $(ODIR) ]; then $(MKDIR) $(ODIR); fi
else
dirs:
	${Q_}$(MKDIR) $(ODIR)
endif

ifeq ($(BWIN_SUPPORT),y)
include $(BSEAV)/lib/bwin/include/bwin.mak
CXXFLAGS += ${BWIN_CFLAGS} -DBWIN_SUPPORT
endif

$(ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) -c -o $@ $< $(CXXFLAGS)

$(MCOMMON_LIB): $(MCOMMON_OBJECTS)
	@echo [Linking... $(notdir $@)]
	@$(AR) cr $@ $^
$(MNET_LIB): $(MNET_OBJECTS)
	@echo [Linking... $(notdir $@)]
	@$(AR) cr $@ $^

clean:
	-${Q_}$(RM) *.o *.a *.d
	-${Q_}$(RM) $(ODIR)/*.o $(ODIR)/*.a $(ODIR)/*.d

-include $(ODIR)/*.d
