############################################################
#     (c)2003-2014 Broadcom Corporation
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
#   This file defines basic library rules for nexus systems
#
# Revision History:
#
# Created: 03/05/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

# Include basic OS flags
include $(NEXUS_TOP)/build/os/$(B_REFSW_OS)/os_flags.inc

# Default for toolchain version
ifndef B_REFSW_TOOLCHAIN_VER
B_REFSW_TOOLCHAIN_VER=4.4.5
endif

# Determine what libraries to build
ifndef B_BUILD_LIBS
# Build all by default
B_BUILD_LIBS := $(B_LIBS)
endif

# Make sure we have a name for the final library
ifndef B_LIB_OUTPUT_NAME
$(error You must define B_LIB_OUTPUT_NAME)
endif

# Create baseline CFLAGS
B_LIB_CFLAGS := $(NEXUS_CFLAGS)

# Handle DEBUG vs. RELEASE builds
ifeq ($(B_REFSW_DEBUG),n)
B_LIB_CFLAGS += -Os
else
B_LIB_CFLAGS += -g -Os
endif

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
B_LIB_OBJ_DIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/$(B_LIB_OUTPUT_NAME)

# Handle static vs. shared library differences
ifneq ($(findstring $(B_REFSW_OS),linuxuser vxworks),)
# Only linuxuser supports shared libraries, do shared by default
ifdef B_LIB_STATIC
# Static Library
B_LIB_OBJ_SUFFIX := o
B_LIB_STATIC_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).a
else
# Shared (PIC) Library
ifeq ($(B_REFSW_OS),vxworks)
B_LIB_SHARED_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).out
B_LIB_STATIC_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).a
else
B_LIB_OBJ_SUFFIX := lo
B_LIB_CFLAGS += -DPIC -fpic
B_LIB_SHARED_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).so
B_LIB_STATIC_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).a
endif
endif
else
# Always static
B_LIB_OBJ_SUFFIX := o
B_LIB_STATIC_LIB := $(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).a
endif

# Expand per-library variables and nexus variables into correct variables
B_LIB_SOURCE_DIRS := $(sort $(foreach lib, $(B_BUILD_LIBS), $(dir $($(lib)_SOURCES))))
B_LIB_SOURCES := $(foreach lib, $(B_BUILD_LIBS), $($(lib)_SOURCES))
B_LIB_CFLAGS += $(addprefix -D,$(sort $(foreach lib, $(B_LIBS),$($(lib)_DEFINES))))
B_LIB_CFLAGS += $(addprefix -I,$(sort $(foreach lib, $(B_LIBS),$($(lib)_PUBLIC_INCLUDES))))
B_LIB_CFLAGS += $(addprefix -I,$(sort $(foreach lib, $(B_LIBS),$($(lib)_PRIVATE_INCLUDES))))
B_LIB_CFLAGS += $(addprefix -I, $(NEXUS_APP_INCLUDES))
B_LIB_CFLAGS += $(addprefix -I, $(NEXUS_APP_INCLUDE_PATHS))
B_LIB_CFLAGS += $(addprefix -D, $(NEXUS_APP_DEFINES))

#
# Specify additional external include dirs
#
B_LIB_CFLAGS += $(addprefix -I,$(B_REFSW_EXTRA_INCLUDE_DIRS))

# Create list of build objects
B_LIB_OBJECTS := $(addprefix $(B_LIB_OBJ_DIR)/, $(patsubst %.c,%.$(B_LIB_OBJ_SUFFIX),$(notdir $(B_LIB_SOURCES))))
B_LIB_OBJECTS := $(patsubst %.cpp,%.$(B_LIB_OBJ_SUFFIX),$(B_LIB_OBJECTS))

# Setup source directory structure
vpath %.c $(B_LIB_SOURCE_DIRS)
vpath %.cpp $(B_LIB_SOURCE_DIRS)

# Check for circular dependencies
define DEPENDENCY_template
$(foreach dep,$($(1)_DEPENDENCIES),$(if $(findstring $(1),$($(dep)_DEPENDENCIES)),$(error circular dependency for modules $(1) and $(dep))))
endef
$(foreach lib,$(B_LIBS),$(call DEPENDENCY_template,$(lib)))

# Final Destination
ifndef B_LIB_INSTALL_DIR
B_LIB_INSTALL_DIR := $(NEXUS_BIN_DIR)
endif

# Default Rule
all: install

# Install Binaries
install: $(B_LIB_SHARED_LIB) $(B_LIB_STATIC_LIB)
	@echo [Install... $(B_LIB_OUTPUT_NAME) library]
	$(Q_)$(MKDIR) $(B_LIB_INSTALL_DIR)
ifdef B_LIB_SHARED_LIB
	@$(CP) $(B_LIB_SHARED_LIB) $(B_LIB_INSTALL_DIR)
endif
ifneq ($(B_REFSW_ANDROID),y)
	@$(CP) $(B_LIB_STATIC_LIB) $(B_LIB_INSTALL_DIR)
endif

#
# We should only use the C++ linker if needed.  Otherwise, all applibs require libstdc++.
#
ifeq ($(findstring .cpp,$(B_LIB_SOURCES)),)
# No .cpp files present, use the C linker
B_LINKER := $(CC)
else
# .cpp files are present, use the C++ linker
B_LINKER := $(CXX)
endif

#
# Specify additional external library dirs and libraries
#
LDFLAGS += $(addprefix -L,$(B_REFSW_EXTRA_LIB_DIRS)) $(addprefix -l,$(B_REFSW_EXTRA_LIBS))

#
# GSTREAMER links dvr library to a source and sink library which expect
# dvr library to be linked to b_os and nexus to act as standalone library
#

ifeq ($(GSTREAMER_DVR_ENABLE),y)
ifeq ($(filter ${B_LIB_OUTPUT_NAME}, b_os), ${B_LIB_OUTPUT_NAME})
LDFLAGS += -L$(NEXUS_BIN_DIR) -lnexus
endif
ifeq ($(filter ${B_LIB_OUTPUT_NAME}, b_dvr), ${B_LIB_OUTPUT_NAME})
LDFLAGS += -L$(NEXUS_BIN_DIR) -lnexus -lb_os
endif
endif

#
# Android specifics
#
ifeq ($(B_REFSW_ANDROID),y)

B_LIB_LDFLAGS = $(LDFLAGS) ${NEXUS_LDFLAGS} -L${NEXUS_BIN_DIR} -lnexus${NEXUS_LIB_SUFFIX}

$(B_LIB_SHARED_LIB): $(B_LIB_OBJECTS)
	@echo "[Link...... $(notdir $@)]"
	${Q_}$(B_LINKER) -Wl,--cref -Wl,-x -o $@ -Wl,-soname,lib$(B_LIB_OUTPUT_NAME).so $^ ${B_LIB_LDFLAGS}

else

$(B_LIB_SHARED_LIB): $(B_LIB_OBJECTS)
	@echo "[Link...... $(notdir $@)]"
ifeq ($(B_REFSW_OS),vxworks)
	${Q_}$(B_LINKER) ${LDFLAGS} -Wl,-Map,$(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).map -Wl,--cref -Wl,-x -o $@ -Wl,-soname,lib$(B_LIB_OUTPUT_NAME).out $^
else
	${Q_}$(B_LINKER) ${LDFLAGS} -shared -Wl,-Map,$(B_LIB_OBJ_DIR)/lib$(B_LIB_OUTPUT_NAME).map -Wl,--cref -Wl,-x -o $@ -Wl,-soname,lib$(B_LIB_OUTPUT_NAME).so $^
endif

endif # B_REFSW_ANDROID

$(B_LIB_STATIC_LIB): $(B_LIB_OBJECTS)
	@echo "[Archive... $(notdir $@)]"
	-$(Q_)$(AR) rcs $@ $^

$(B_LIB_OBJ_DIR)/%.$(B_LIB_OBJ_SUFFIX): %.c
	-$(Q_)$(MKDIR) $(B_LIB_OBJ_DIR) >/dev/null 2>&1
	@echo "[Compile... $(notdir $<) ($(B_LIB_OUTPUT_NAME))]"
	$(Q_)$(CC) $(CDEP_FLAG) $(B_LIB_CFLAGS) -c $< -o $@

$(B_LIB_OBJ_DIR)/%.$(B_LIB_OBJ_SUFFIX): %.cpp
	-$(Q_)$(MKDIR) $(B_LIB_OBJ_DIR) >/dev/null 2>&1
	@echo "[Compile... $(notdir $<) ($(B_LIB_OUTPUT_NAME))]"
	$(Q_)$(CXX) $(CDEP_FLAG) $(filter-out $(B_LIB_C_ONLY_FLAGS),$(B_LIB_CFLAGS)) $(B_LIB_CPPFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(B_LIB_INSTALL_DIR)/$(notdir $(B_LIB_SHARED_LIB)) $(B_LIB_INSTALL_DIR)/$(notdir $(B_LIB_STATIC_LIB)) $(B_LIB_OBJ_DIR)/*

# Include dependencies
B_LIB_DEP_FILES := $(wildcard $(B_LIB_OBJ_DIR)/*.d)
ifneq ($(B_LIB_DEP_FILES),)
-include $(B_LIB_DEP_FILES)
endif

