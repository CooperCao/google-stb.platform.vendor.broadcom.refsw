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

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)
endif

BSEAV ?= $(shell cd ../../..; pwd)
ROCKFORD ?= $(shell cd ../../../../rockford; pwd)
NEXUS ?= $(shell cd ../../../../nexus; pwd)
NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc
#
# Basic build option
#
OS=linux
APP = $(OBJ_DIR)/irbtest_nexus
CFLAGS += -DLINUX
SYSTEM = linux

# For linux builds, link to the correct libraries
ifeq ($(OS),linuxkernel)
LDFLAGS := ${NEXUS_LDFLAGS}
else
ifneq ($(findstring linux,$(OS)),)
LDFLAGS := -lnexus -L$(NEXUS_TOP)/bin
endif
endif

#ifeq ($(STATIC_TRINITY),y)
#LDFLAGS += -static
#endif

Q_=@
OBJ_DIR := $(PLATFORM)-$(OS)

ifeq ($(ARCH),mipsel-uclibc)
LIBC=uclibc
else
ifeq ($(ARCH),mipsel-linux)
LIBC=glibc
endif
endif

.PHONY: all clean


CFLAGS += -DBCM_BOARD_STR=\"$(PLATFORM)\"

#
# Main trinity files
#
TRINITY_DIRS = $(BSEAV)/app/trinity/src/xmp $(BSEAV)/app/trinity/src/test
TRINITY_INCLUDE_DIRS = $(addprefix -I,$(TRINITY_DIRS))

APP_OBJECTS = \
	input_xmp.o \
	input_xmp_os.o \
	irbtest_nexus.o


# XMP remote control
#ifeq ($(XMP2_SUPPORT),y)
CFLAGS += -DXMP2_SUPPORT
#endif


LDFLAGS += -L$(NEXUS)/bin -lb_os
TRINITY_INCLUDE_DIRS += -I$(NEXUS)/lib/os/include -I$(NEXUS)/lib/os/include/linuxuser

#nexus Kernel Mode builds
ifeq ($(KERNELMODE),y)
MODE=proxy
export MODE
endif


all: api oslib $(APP)


TRINITY_INCLUDE_DIRS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += \
	-MD \
	$(TRINITY_INCLUDE_DIRS) \
	$(MAGNUM_GENERIC_CHIP_DEFINES) \
	$(SYSTEM_FLAGS)

all: api oslib $(APP)


TRINITY_INCLUDE_DIRS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += \
        -MD \
        $(TRINITY_INCLUDE_DIRS) \
        $(MAGNUM_GENERIC_CHIP_DEFINES) \
        $(SYSTEM_FLAGS)

CFLAGS += -I$(NEXUS)/../magnum/basemodules/rpc/linuxuser
#CFLAGS += $(NEXUS_CFLAGS)
CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}


#CFLAGS += $(NEXUS_CFLAGS)
CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}

CFLAGS += -O2

LDFLAGS += -ldl -lpthread -lm

ifeq ($(DEBUG),y)
LDFLAGS += -g
CFLAGS += -DBDBG_DEBUG_BUILD=1
else
LDFLAGS += -s
endif


VPATH += $(TRINITY_DIRS)

CXXFLAGS += $(CFLAGS)

# Add C++ only options
CXXFLAGS +=	-fno-rtti -fno-exceptions


vpath %.h $(VPATH)
vpath %.cpp $(VPATH)
vpath %.c $(VPATH)
vpath %.o $(OBJ_DIR)

$(APP): $(OBJ_DIR) $(addprefix $(OBJ_DIR)/,$(APP_OBJECTS)) $(LIBS)
	@echo $(CXX) -o $@
	$(CXX) -o $@ $(addprefix $(OBJ_DIR)/,$(APP_OBJECTS)) $(LIBS) $(LDFLAGS)


%.o: %.cpp
	@echo '$(CXX) $<'
	$(Q_)$(CXX) $(CXXFLAGS) -c $< -o $(OBJ_DIR)/$@

%.o: %.c
	@echo '$(CC) $<'
	$(Q_)$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@

$(OBJ_DIR)/%.o: %.cpp
	@echo '$(CXX) $<'
	$(Q_)$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo '$(CC) $<'
	$(Q_)$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJ_DIR)/*.d

#
# always build trinity.o because it contains __DATE__
#
build_date:
	@$(RM) -f $(OBJ_DIR)/irbtest_nexus.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# This builds the Trinity App
irbtest_nexus: $(APP)

# This cleans the Trinity App
clean-irbtest_nexus:
	$(RM) -rf $(OBJ_DIR)

# This builds and cleans the Nexus api
api:
	$(MAKE) -C $(NEXUS_TOP)/build

clean-api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

# This builds and cleans NEXUS OS lib
oslib:
	$(MAKE) -C $(NEXUS_TOP)/lib/os

clean-oslib:
	$(MAKE) -C $(NEXUS_TOP)/lib/os clean

clean: clean-irbtest_nexus clean-api clean-oslib


-include $(OBJ_DIR)/*.d
