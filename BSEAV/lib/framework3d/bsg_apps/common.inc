#############################################################################
#    (c)2012-2016 Broadcom Corporation
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
#############################################################################

BSG_DIR          ?= $(shell cd ../../bsg; pwd)

include $(BSG_DIR)/common.inc

# For spyhook_replat only
ifeq ($(V3D_PREFIX),vc5)
HOOK_NAME = gpumon_hook
else
HOOK_NAME = spyhook
endif

B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}
BSG_OBJ_DIR      ?= $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bsg/$(TARGET)

OBJECTS   = $(addprefix $(BSG_OBJ_DIR)/, $(addsuffix .o, $(basename $(SOURCES))))

# Set up paths
APP_DIR          := $(shell pwd)
TARGET_DIR       := $(NEXUS_BIN_DIR)/bsg
TARGET_APP       := $(TARGET_DIR)/$(TARGET)

# Includes for most BSG files
INCLUDES       += $(BSG_DIR)                            \
                  $(V3D_DIR)/interface/khronos/include

LDFLAGS += -L$(NEXUS_BIN_DIR) -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/libpng -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/zlib/$(B_REFSW_ARCH)
LDFLAGS += -lbsg -lrt -lm -lpng -lz -lfreetype -lnxpl -lv3ddriver -ldl
LDFLAGS += -lnexus$(NEXUS_LIB_SUFFIX)

ifeq ($(WEBCPU_MODE),y)
LDFLAGS += -lnexus_webcpu
endif

ifeq ($(V3D_PREFIX),vc5)
CXXFLAGS += -DV3D_TECH_VERSION=3
INCLUDES += $(V3D_DIR)/../
endif

.PHONY:	all clean clean_bsg info bsg install

all:	info bsg $(BSG_OBJ_DIR)/$(TARGET) install

info:
	@echo "************************************************************"; \
	echo "Building $(TARGET) with the following settings:             "; \
	echo "                                                            "; \
	echo "BSG_DIR          = $(BSG_DIR)"; \
	echo "OPENSOURCE       = $(OPENSOURCE)"; \
	echo "NEXUS_TOP        = $(NEXUS_TOP)"; \
	echo "NEXUS_BIN_DIR    = $(NEXUS_BIN_DIR)"; \
	echo "ROCKFORD_TOP     = $(ROCKFORD_TOP)"; \
	echo "V3D_DIR          = $(V3D_DIR)"; \
	echo "V3D_PLATFORM_DIR = $(V3D_PLATFORM_DIR)"; \
	echo "TARGET_DIR       = $(TARGET_DIR)"; \
	echo "TARGET_APP       = $(TARGET_APP)"; \
	echo "WEBCPU_MODE      = $(WEBCPU_MODE)"; \
	echo "                                                            "; \
	echo "************************************************************"


# Create the app
$(info Objects = $(OBJECTS))
$(BSG_OBJ_DIR)/$(TARGET):	$(OBJECTS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $(BSG_OBJ_DIR)/$(TARGET)

B_REFSW_DEBUG ?= y
ifneq ($(B_REFSW_DEBUG),y)
	$(STRIP) $(BSG_OBJ_DIR)/$(TARGET)
endif

bsg:
	echo "Making bsg"
	$(MAKE) -f bsg.mk -C $(BSG_DIR)

# Install
install:	bsg $(BSG_OBJ_DIR)/$(TARGET)
	mkdir -p $(TARGET_APP); \
	chmod 777 $(TARGET_APP); \
	cp -f $(BSG_OBJ_DIR)/$(TARGET) $(TARGET_APP); \
	test "$(DATA)" != "" && cp -f $(DATA) $(TARGET_APP); \
	mkdir -p $(TARGET_APP)/resources; \
	cp -f resources/* $(TARGET_APP)/resources; \
	mkdir -p $(TARGET_APP)/../common/resources; \
	cp -f ../common/resources/* $(TARGET_APP)/../common/resources

clean_app:
	@echo "Cleaning application ..."
	-@rm -f $(OBJECTS) $(BSG_OBJ_DIR)/$(TARGET) 2> /dev/null
	-@rm -rf $(TARGET_APP)

clean:	clean_app
	-@rm -rf $(TARGET_DIR)/common
	-@rm -rf $(TARGET_DIR)
	$(MAKE) -C $(BSG_DIR) -f bsg.mk clean

# Files that are common
$(OBJECTS): $(BSG_OBJ_DIR)/%o: %cpp
	mkdir -p $(BSG_OBJ_DIR)
	mkdir -p $(BSG_OBJ_DIR)/../common
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $^ -o $@
