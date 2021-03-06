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
#       This makefile contains the OS-independent rules for building
#       nexus modules.
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

############################################################
# Overview:
#
# This is an internal nexus Makefile include. It is used for processing the module .inc files and generated build rules.
# This file is included by nexus/platform/$(NEXUS_PLATFORM)/build/Makefile
#
# This file should never be included by applications.
# Applications should include nexus/platform/$(NEXUS_PLATFORM)/build/platform_app.inc into their Makefile.
# Please see nexus/build/nexus_defs.inc for a .inc which is common for internal nexus use and application use.
#
############################################################

ifeq ($(filter $(B_REFSW_OS), ucos ucos_ii ucos_iii), $(B_REFSW_OS))
NEXUS_BUILD_OS_DIR := $(NEXUS_TOP)/build/os/ucos
NEXUS_BASE_INCLUDE_DIR := $(NEXUS_TOP)/base/include/ucos
else
NEXUS_BUILD_OS_DIR := $(NEXUS_TOP)/build/os/$(B_REFSW_OS)
NEXUS_BASE_INCLUDE_DIR := $(NEXUS_TOP)/base/include/$(B_REFSW_OS)
endif

# Make sure files are included in the correct order
ifndef NEXUS_DEFS_INCLUDED
$(error nexus_defs.inc was not included first.)
endif

ifneq ($(B_REFSW_OS),vxworks)
BSTD_OMIT_DEFAULT_CONFIG := 1
endif

# Create NEXUS_SOURCE_DIRS from all modules in NEXUS_MODULES
NEXUS_SOURCE_DIRS := $(sort $(foreach module,$(NEXUS_MODULES),$(dir $(NEXUS_$(module)_SOURCES)) $(dir $(NEXUS_$(module)_LOCAL_SOURCES))))
# Add the nexus source dirs to the search path for .c files.
vpath %.c $(NEXUS_SOURCE_DIRS)

ifeq ($(BPROFILE_SUPPORT),y)
NEXUS_CFLAGS_BPROFILE = -DB_HAS_BPROFILE=1 -finstrument-functions
endif


# Create NEXUS CFLAGS from base include path and defines
NEXUS_DEFINES := $(sort $(foreach module, $(NEXUS_MODULES),$(NEXUS_$(module)_DEFINES)))
NEXUS_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))
NEXUS_CFLAGS += -I$(NEXUS_BASE_INCLUDE_DIR)/config $(addprefix -D,$(NEXUS_DEFINES))
NEXUS_CFLAGS += $(addprefix -I,$(NEXUS_EXTRA_INCLUDES))
NEXUS_CFLAGS += $(addprefix -I,$(NEXUS_PLATFORM_PUBLIC_INCLUDES))
NEXUS_USE_PRECOMPILED_HEADERS ?=y
NEXUS_P_WITH_PRECOMPILED_HEADERS := $(findstring y, ${NEXUS_USE_PRECOMPILED_HEADERS})
NEXUS_MAGNUM_PRECOMPILED_HEADER := ${MAGNUM}/basemodules/std/bstd.h
NEXUS_PRECOMPILED_HEADER := ${NEXUS_TOP}/base/include/nexus_base.h

# Include NEXUS OS-specific flags
include $(NEXUS_BUILD_OS_DIR)/os_flags_nexus.inc

ifeq ($(B_REFSW_OS),linuxkernel)
# The following source files are the only ones in Nexus/Magnum that have a direct dependency
# on the OS. The list should only be extended with careful consideration.
NEXUS_OS_ABSTRACTION := \
    PLATFORM/nexus_driver_procfs.c \
    PLATFORM/nexus_driver.c \
    PLATFORM/nexus_platform_os.c \
    PLATFORM/nexus_platform_cma.c \
    PLATFORM/qdivrem.c \
    BASE/nexus_base_os.c \
    $(foreach module, $(NEXUS_MODULES), $(module)/nexus_$(shell echo $(module)|tr '[:upper:]' '[:lower:]')_kernel_export.c)
NEXUS_OS_ABSTRACTION_OBJECTS := $(addprefix $(NEXUS_OBJ_DIR)/,$(patsubst %.c,%.$(NEXUS_OBJ_SUFFIX),$(NEXUS_OS_ABSTRACTION)))
MAGNUM_OS_ABSTRACTION := \
    MAGNUM/bkni.c \
    MAGNUM/bv3d_os.c \
    MAGNUM/bvc5_os.c \
    MAGNUM/brpc_socket.c \
    MAGNUM/bdbg_os_priv.c
endif

ifeq ($(B_REFSW_ANDROID),y)
MAGNUM_OS_ABSTRACTION += \
    MAGNUM/bvc5_android_fences.c \
    MAGNUM/bvc5_android_bin_pool_alloc.c
endif

ifneq ($(MAKECMDGOALS),clean)
%/exists:
	-${Q_}${MKDIR} $@

# Create NEXUS_XXX_CFLAGS and NEXUS_XXX_OBJECTS using PERL
$(NEXUS_BUILD_DIR)/module_vars_${NEXUS_MODE}.inc:${NEXUS_BUILD_DIR}/exists ${NEXUS_SYNCTHUNK_DIR}/exists
	${Q_}$(PERL) $(NEXUS_TOP)/build/module_vars.pl $@ $(NEXUS_MODULES)

# Include result into makefile
-include $(NEXUS_BUILD_DIR)/module_vars_${NEXUS_MODE}.inc
endif

# Include Magnum Modules
NEXUS_MAGNUM_MODULES := $(foreach module,$(NEXUS_MODULES),$(NEXUS_$(module)_MAGNUM_MODULES))
include $(sort $(NEXUS_MAGNUM_MODULES))

# Include any extra modules requested by nexus .inc files as well.
MAGNUM_MODULES += $(sort $(NEXUS_EXTRA_MODULES))

# Expand Magnum Variables
MAGNUM_INCLUDES := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))

ifneq ($(filter ${NEXUS_MODE},client proxy),)
# For client/proxy environments, we only compile in a small subset of magnum modules
MAGNUM_SOURCES := $(patsubst %.o,%.c,$(sort $(foreach module, $(NEXUS_EXTRA_MODULES) ${MAGNUM_CLIENT_MODULES}, $($(module)_SOURCES))))
else
# Don't compile magnum modules that are accesses using proxy interface
MAGNUM_SOURCES := $(patsubst %.o,%.c,$(sort $(foreach module, ${MAGNUM_MODULES}, $($(module)_SOURCES))))
endif

MAGNUM_DEFINES := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
MAGNUM_CFLAGS := $(addprefix -D,$(MAGNUM_DEFINES)) $(addprefix -I,$(MAGNUM_INCLUDES))
MAGNUM_SOURCE_DIRS := $(foreach module,$(MAGNUM_MODULES),$(dir $($(module)_SOURCES)))
MAGNUM_OPTIONS := $(sort $(foreach module,$(MAGNUM_MODULES),$($(module)_OPTIONS)))
MAGNUM_NOPROFILE_SRCS += bdbg.c bdbg_output.c bdbg_os_priv.c bkni.c
ifeq ($(B_REFSW_OS), linuxemu)
NEXUS_CFLAGS += $(addprefix -I,$(NEXUS_PLATFORM_PRIVATE_INCLUDES))
MAGNUM_NOPROFILE_SRCS += breg_client.c
else
MAGNUM_NOPROFILE_SRCS += breg_mem.c
endif
MAGNUM_STATIC_LIBS := $(sort $(foreach module,$(MAGNUM_MODULES),$($(module)_STATIC_LIBS)))

#MAGNUM_NO_STRICT_ALIASING_SRCS lists files which will be compile with -fno-strict-aliasing.  strict-aliasing is a useful and
#important optimization, which is turned off by including a file in this list.  Don't add files to this list unless strict-aliasing
#warnings are caused by code outside our control (Linux kernel headers).  If at all possible, fix the condition that causes
#the warning.
MAGNUM_NO_STRICT_ALIASING_SRCS := brpc_socket.c

# Set other makes
NEXUS_OTHER_MAKES := $(sort $(foreach module, $(NEXUS_MODULES),$(NEXUS_$(module)_MAKES))) $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_MAKES)))

# Remove pre-compiled files from the build
MAGNUM_PRECOMPILED_DIR := $(NEXUS_BUILD_DIR)/libmagnum.$(B_REFSW_OS).$(B_REFSW_ARCH)
MAGNUM_PRECOMPILED_SOURCES := $(patsubst %.$(NEXUS_OBJ_SUFFIX),%.c,$(notdir $(wildcard $(MAGNUM_PRECOMPILED_DIR)/*)))

# Check that all precompiled files are actually being requested by the build.  Otherwise, flags are probably mismatched somewhere.
$(foreach precomp,$(MAGNUM_PRECOMPILED_SOURCES),$(if $(filter $(precomp),$(notdir $(MAGNUM_SOURCES))),,$(warning precompiled file $(precomp) is not being used in your build)))

MAGNUM_OBJECTS := $(addprefix $(NEXUS_OBJ_DIR)/MAGNUM/,$(patsubst %.c,%.$(NEXUS_OBJ_SUFFIX),$(filter-out $(notdir $(MAGNUM_OS_ABSTRACTION)) $(MAGNUM_PRECOMPILED_SOURCES),$(notdir $(MAGNUM_SOURCES)))))
MAGNUM_OS_ABSTRACTION := $(filter $(addprefix MAGNUM/,$(notdir $(MAGNUM_SOURCES))), $(MAGNUM_OS_ABSTRACTION))
MAGNUM_PRECOMPILED_OBJECTS += $(wildcard $(MAGNUM_PRECOMPILED_DIR)/*)

# Add the magnum source dirs to the search path for .c files.
vpath %.c $(MAGNUM_SOURCE_DIRS)

# Add all magnum CFLAGS to NEXUS_CFLAGS
NEXUS_CFLAGS += $(MAGNUM_CFLAGS)

# Include os-dependencies and expand nexus rules
# Sanity Check that we have included an OS-dependent set of defines
ifndef NEXUS_OBJ_SUFFIX
$(error NEXUS_OBJ_SUFFIX has not been defined.  Stop.)
endif

ifndef NEXUS_OBJ_DIR
$(error NEXUS_OBJ_DIR has not been defined.  Stop.)
endif

ifndef NEXUS_ENDIAN
$(error NEXUS_ENDIAN has not been defined.  Stop.)
endif

reverse = $(if $(1),$(call reverse,$(wordlist 2,$(words $(1)),$(1)))) $(firstword $(1))

NEXUS_REVERSE_SORTED_MODULES = PLATFORM $(filter-out PLATFORM,$(call reverse, $(NEXUS_MODULES)))

ifneq ($(MAKECMDGOALS),clean)
$(NEXUS_BUILD_DIR)/module_rules_${NEXUS_MODE}.inc:${NEXUS_BUILD_DIR}/exists
	${Q_}$(PERL) $(NEXUS_BUILD_OS_DIR)/module_rules.pl $@ $(MODULE_RULES_OPTIONS) $(NEXUS_REVERSE_SORTED_MODULES)

-include $(NEXUS_BUILD_DIR)/module_rules_${NEXUS_MODE}.inc
endif

# Create NEXUS_OPTIONS from list of all options
NEXUS_OPTIONS := $(sort $(foreach module, $(NEXUS_MODULES),$(NEXUS_$(module)_OPTIONS)))

# Create single list of all nexus objects for convenience
NEXUS_OBJECTS := $(foreach module,$(NEXUS_MODULES),$(NEXUS_$(module)_OBJECTS) $(NEXUS_$(module)_OS_OBJECTS))

# Include OS-specific rules
include $(NEXUS_BUILD_OS_DIR)/os_rules.inc

# Debug rule to dump all options specified in NEXUS_OPTIONS
define OPTION_template
@echo $(1)=$(if $($(1)),$($(1)),"<value not set>")

endef

NEXUS_OPTIONS += NEXUS_PLATFORM NEXUS_MODE B_REFSW_OS B_REFSW_DEBUG B_REFSW_ARCH B_REFSW_SHAREABLE NEXUS_POWER_MANAGEMENT B_REFSW_FIRMWARE

.PHONY: nexus_print_options
nexus_print_options:
ifneq ($(NEXUS_OPTIONS),)
	@echo "---Nexus Options---"
	$(foreach option,$(sort $(NEXUS_OPTIONS)),$(call OPTION_template,$(option)))
endif
ifneq ($(MAGNUM_OPTIONS),)
	@echo "---Magnum Options---"
	$(foreach option,$(sort $(MAGNUM_OPTIONS)),$(call OPTION_template,$(option)))
endif

# NOTE: It's important that NEXUS_$(module)_PUBLIC_INCLUDES not be rolled up into a single, global include path inside nexus.inc.
# Having per-module include paths is required for perserving modularity by preventing unmanaged calls from one module to another.
# See platform_app.inc for a rolled up NEXUS_$(module)_PUBLIC_INCLUDES usable by applications to access the entire Nexus public API.

