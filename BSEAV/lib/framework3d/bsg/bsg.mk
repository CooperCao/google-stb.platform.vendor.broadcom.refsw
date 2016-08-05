#############################################################################
# Broadcom Proprietary and Confidential. (c)2012-2013 Broadcom
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
#
#############################################################################

BSG_DIR          := $(shell pwd)

include common.inc

# Paths
BSEAV_DIR        := $(shell cd $(BSG_DIR)/../../..            ; pwd)
OPENSOURCE       := $(NEXUS_TOP)/../BSEAV/lib
NEXUS_TOP        ?= $(shell cd $(BSEAV_DIR)/../nexus          ; pwd)
ROCKFORD_TOP     ?= $(shell cd $(BSEAV_DIR)/../rockford       ; pwd)
V3D_DIR          ?= $(shell cd $(ROCKFORD_TOP)/middleware/$(V3D_PREFIX)/driver ; pwd)
V3D_PLATFORM_DIR ?= $(shell cd $(V3D_DIR)/../platform         ; pwd)
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_DIR}/../${B_REFSW_OBJ_DIR}

BSG_OBJ_DIR      ?= $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bsg
FREETYPE_OBJ_DIR ?= $(B_REFSW_OBJ_ROOT)/BSEAV/lib/freetype

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
V3D_OBJ := obj_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
V3D_OBJ := obj_$(NEXUS_PLATFORM)_release
endif

V3DDRIVER_LIB_TARGET := ${NEXUS_BIN_DIR}
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/v3d/$(V3D_OBJ)
NXPL_LIB_TARGET := ${NEXUS_BIN_DIR}
NXPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/platform/nexus/$(V3D_OBJ)

include bsg_common.inc
include bsg_nexus.inc

# Object files
COMMON_OBJECTS   = $(addprefix $(BSG_OBJ_DIR)/, $(addsuffix .o, $(basename $(CPP_COMMON_FILES))))
PLATFORM_OBJECTS = $(addprefix $(BSG_OBJ_DIR)/, $(addsuffix .o, $(basename $(CPP_PLATFORM_FILES))))
ALL_OBJECTS      = $(COMMON_OBJECTS) $(PLATFORM_OBJECTS)

# Target
TARGET := $(NEXUS_BIN_DIR)/libbsg.so

# Third party library locations
PNG              ?= $(OPENSOURCE)/libpng
ZLIB             ?= $(OPENSOURCE)/zlib
FREETYPE         ?= $(BSG_DIR)/../freetype-2.3.12

# Includes for most BSG files
INCLUDES       += $(V3D_DIR)/interface/khronos/include                \
                  $(V3D_DIR)/../platform/nexus                        \
                  $(V3D_DIR)/platform/nexus                           \
                  $(V3D_DIR)/libs/platform/bcg_abstract               \
                  $(V3D_DIR)/libs/core/vcos/include                   \
                  $(V3D_DIR)/libs/core/vcos/pthreads                  \
                  $(V3D_DIR)                                          \
                  $(FREETYPE)/include                                 \
                  $(PNG)                                              \
                  $(ZLIB)

PLATFORM_INCLUDES += $(V3D_PLATFORM_DIR)/nexus			\
                     $(NEXUS_TOP)/../BSEAV/lib/media            \
                     $(NEXUS_TOP)/../BSEAV/lib/utils

# C++ compiler flags
CXXFLAGS += $(addprefix -I,$(INCLUDES))
CXXFLAGS += -fPIC

# Includes and defines needed by Nexus client code
NEXUS_CXXFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS))
NEXUS_CXXFLAGS += $(addprefix -I,$(PLATFORM_INCLUDES))

NEXUS_CXXFLAGS += $(addprefix -D,$(NEXUS_APP_DEFINES))

.PHONY: info nexus_lib v3d_lib nxpl_lib zlib png freetype

all:	info nxpl_lib png freetype $(TARGET)

info:
	@echo "*******************************************************"; \
	echo "Building BSG with the following settings:"; \
	echo "                                                       "; \
	echo "BSG_DIR          = $(BSG_DIR)"; \
	echo "OPENSOURCE       = $(OPENSOURCE)"; \
	echo "NEXUS_TOP        = $(NEXUS_TOP)"; \
	echo "NEXUS_BIN_DIR    = $(NEXUS_BIN_DIR)"; \
	echo "ROCKFORD_TOP     = $(ROCKFORD_TOP)"; \
	echo "V3D_DIR          = $(V3D_DIR)"; \
	echo "V3D_PLATFORM_DIR = $(V3D_PLATFORM_DIR)"; \
	echo "CXX              = $(CXX)"; \
	echo "NXCLIENT_SUPPORT = $(NXCLIENT_SUPPORT)";\
	echo "                                                       "; \
	echo "*******************************************************"

#################################################
# Create the nxclient server (if needed)
#################################################
nexus_lib:
ifeq ($(NXCLIENT_SUPPORT),y)
	$(MAKE) -C $(NEXUS_TOP)/nxclient NEXUS_BIN_DIR=$(NEXUS_BIN_DIR)
else
	$(MAKE) -C $(NEXUS_TOP)/build NEXUS_BIN_DIR=$(NEXUS_BIN_DIR)
endif

#################################################
# Create the V3D driver
#################################################

v3d_lib: nexus_lib
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)

# Create the platform layer
nxpl_lib: v3d_lib
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y

#################################################
# Build libz
#################################################
zlib:	$(B_REFSW_OBJ_ROOT)/BSEAV/lib/zlib/$(B_REFSW_ARCH)/libz.a

$(B_REFSW_OBJ_ROOT)/BSEAV/lib/zlib/$(B_REFSW_ARCH)/libz.a:
	$(MAKE) -C $(ZLIB)

#################################################
# Build libpng
#################################################
png:	$(B_REFSW_OBJ_ROOT)/BSEAV/lib/libpng/libpng.a zlib

$(B_REFSW_OBJ_ROOT)/BSEAV/lib/libpng/libpng.a:
	$(MAKE) -C $(PNG)

#################################################
# Build libfreetype
#################################################

$(FREETYPE)/.bsg_configured:
	cd $(FREETYPE); \
	chmod -f +x configure builds/unix/configure; \
	./configure --host=$(B_REFSW_ARCH); \
	touch .bsg_configured

freetype:	$(NEXUS_BIN_DIR)/libfreetype.so

$(NEXUS_BIN_DIR)/libfreetype.so:	$(FREETYPE)/.bsg_configured
	echo $(FREETYPE)
	echo $(MAKE)
	mkdir -p $(FREETYPE_OBJ_DIR)
	$(MAKE) -C $(FREETYPE) OBJ_DIR=$(FREETYPE_OBJ_DIR)
	mkdir -p $(NEXUS_BIN_DIR)
	cp -f $(FREETYPE_OBJ_DIR)/.libs/*.so* $(NEXUS_BIN_DIR)

#################################################
# Make the documentation
#################################################
docs:
	doxygen Doxyfile
	-rm -rf $(BSG_DIR)/../doc/bsg_api_docs
	mv -f doxygen_docs $(BSG_DIR)/../doc/bsg_api_docs

#################################################
# Create and install the BSG library
#################################################
$(TARGET):      $(ALL_OBJECTS) freetype png nxpl_lib
	@echo Linking $(TARGET)
	@$(CXX) -shared -o $(TARGET) $(ALL_OBJECTS)

#################################################
# Clean rules
#################################################
clean_bsg:
	@echo "Removing BSG objects and library"
	@rm -rf $(ALL_OBJECTS) 2> /dev/null
	@rm -rf $(BSG_OBJ_DIR) 2> /dev/null
	@rm -rf $(NEXUS_BIN_DIR)/$(TARGET) 2> /dev/null

clean: clean_bsg
	@echo "Cleaning V3D Driver and platform"
	@$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	@$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) clean
ifeq ($(NXCLIENT_SUPPORT),y)
	$(MAKE) -C $(NEXUS_TOP)/nxclient NEXUS_BIN_DIR=$(NEXUS_BIN_DIR) clean
else
	$(MAKE) -C $(NEXUS_TOP)/build NEXUS_BIN_DIR=$(NEXUS_BIN_DIR) clean
endif
	@$(MAKE) -C $(ZLIB) clean
	@$(MAKE) -C $(PNG) clean
ifneq ($(wildcard $(FREETYPE)/.bsg_configured),)
	@$(MAKE) -C $(FREETYPE) OBJ_DIR=$(FREETYPE_OBJ_DIR) clean
endif
	@rm -f $(NEXUS_BIN_DIR)/libfreetype.*
	@rm -f $(FREETYPE)/.bsg_configured $(FREETYPE)/config.mk

#################################################
# Generic rules
#################################################

# Files that need all the Nexus flags for platform specific code
$(PLATFORM_OBJECTS): $(BSG_OBJ_DIR)/%o: %cpp
	@mkdir -p $(BSG_OBJ_DIR)
	@echo Compiling $^
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(NEXUS_CXXFLAGS) -c $^ -o $@

# Files that do not need nexus flags
$(COMMON_OBJECTS): $(BSG_OBJ_DIR)/%o: %cpp
	@mkdir -p $(BSG_OBJ_DIR)
	@echo Compiling $^
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -c $^ -o $@
