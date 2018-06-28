###############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
SHELL := /bin/bash

###################################################################
#  Check for required variables.
###################################################################
$(if $(NEXUS_PLATFORM),,    $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,      $(error ERROR: B_REFSW_ARCH is not defined))
$(if $(BSEAV),,             $(error ERROR: BSEAV is not defined))

$(if $(B_REFSW_OBJ_DIR),,   $(error ERROR: B_REFSW_OBJ_DIR is not defined))
$(if $(B_REFSW_OBJ_ROOT),,  $(error ERROR: B_REFSW_OBJ_ROOT is not defined))

###################################################################
#  Define some debug aids.
###################################################################
# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${BSEAV}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
ifeq (${BMON_MAKEFILE_DEBUG},y)
    B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
    B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"
endif

# Define a debug function for printing a variable name and it's contents:
ifeq (${BMON_MAKEFILE_DEBUG},y)
BMON_PLUGIN_PRINTVAR =               \
    $(if $($1),                         \
        $(info )                        \
        $(info ---[debug]: $1:)         \
        $(foreach j,$($(1)),            \
            $(info $(j))                \
        )                               \
     ,                             		\
        $(info )                        \
        $(info ---[debug]: $1 is undefined (or empty))  \
    )
endif  # ifeq (${BMON_MAKEFILE_DEBUG},y)

###################################################################
#  Default rule:
###################################################################
.PHONY: all
all: 	build_with_subs
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

.PHONY: build_with_subs
build_with_subs:  ${BMON_THIS_PLUGIN_BUILD_WITH_SUBS_TARGET}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

.PHONY: build_without_subs
build_without_subs: install
	$(B_TRACE_RECIPE_BEGIN)
	@echo BMON plugin: $(B_THIS_PLUGIN) done.
	$(B_TRACE_RECIPE_END)

###################################################################
#  Nexus Includes:
###################################################################

ifeq (${BMON_THIS_PLUGIN_USES_NEXUS},y)

    # include cross-compiler definitions
    include ${NEXUS_TOP}/build/nexus_defs.inc
    include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

    # Use the target.inc file so we can submake the nxclient server.
    include ${NEXUS_TOP}/nxclient/server/b_nxserver_targets.inc

    # Set up Nexus' and NxClient's LDFLAGS and CFLAGS depending on the build flags.
    ifeq ($(NXCLIENT_SUPPORT),y)
        ifeq ($(NXCLIENT_CFLAGS),)
            include $(NEXUS_TOP)/nxclient/include/nxclient.inc
        endif
    else
        $(warning *** Error: NXCLIENT_SUPPORT is not enabled!)
        $(warning *** Error: bmon plugins must be used with NXCLIENT_SUPPORT!)
        $(error   Error: Giving up.)
    endif

    # Combine Nexus' defines and includes into the CFLAGS variable.
    BMON_PLUGINS_PRIVATE_INCLUDES += ${NEXUS_TOP}/utils

    BMON_PLUGINS_EXE_PRIVATE_CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
    BMON_PLUGINS_EXE_PRIVATE_CFLAGS += ${NXCLIENT_CFLAGS}
    BMON_PLUGINS_EXE_PRIVATE_LDFLAGS += ${NXCLIENT_LDFLAGS}
endif

BMON_PLUGINS_EXE_PRIVATE_CFLAGS  += ${BMON_THIS_PLUGIN_EXTRA_CFLAGS}
BMON_PLUGINS_EXE_PRIVATE_LDFLAGS += ${BMON_THIS_PLUGIN_EXTRA_LDFLAGS}

# Determine the name of the plugin's executable (same as plugin name). Also names for shared object and header file.
BMON_THIS_PLUGIN_EXE_FILENAME := ${B_THIS_PLUGIN}
BMON_THIS_PLUGIN_SO_FILENAME  := ${B_THIS_PLUGIN}.so
BMON_THIS_PLUGIN_HDR_FILENAME := ${B_THIS_PLUGIN}.h

# Define this plugin's in-source directory (probably $(CURDIR)).
BMON_THIS_PLUGIN_INSRC_DIR := $(BMON_PLUGINS_INSRC_DIR)/${B_THIS_PLUGIN}

# Build a list of all the source files needed for this plugin.
# Start with all source files in the plugin's directory.
BMON_THIS_PLUGIN_SRC_PATH_LIST := $(wildcard ${BMON_THIS_PLUGIN_INSRC_DIR}/*.c)

ifeq ($(BMON_THIS_PLUGIN_SRC_PATH_LIST),)
    $(info *** Warning, cannot find bmon plugin source files (*.c) in: "${BMON_THIS_PLUGIN_INSRC_DIR}".  Continuing anyway...)
endif

# Now add any "extra" source file passed from the plugin's Makefile.
BMON_THIS_PLUGIN_SRC_PATH_LIST += ${BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST}

# Build a list of directories where we need to install that executable.
BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST = $(abspath $(NEXUS_BIN_DIR)/plugins)
ifeq ($(shell test $(INSTALL_DIR) && echo y),y)
    ifeq ($(shell test -d $(INSTALL_DIR) && echo y),y)
        BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST += $(abspath $(INSTALL_DIR))
    else
        $(warning Warning: INSTALL_DIR (${INSTALL_DIR}) does not exist. Skipping...)
    endif
endif

# Now combine the plugin's executable (and shared object file)  with the list of install directories to
# get lists of final targets.
BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST = $(foreach dir,${BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST},${dir}/${BMON_THIS_PLUGIN_EXE_FILENAME})
BMON_THIS_PLUGIN_SO_INSTALL_PATH_LIST = $(foreach dir,${BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST},${dir}/${BMON_THIS_PLUGIN_SO_FILENAME})

# The plugin's executable (and shared object) will be copied to the install directories from the
# out-of-source build directory, so define that
BMON_THIS_PLUGIN_EXE_OOSRC_DIR ?= ${BMON_PLUGINS_OOSRC_DIR}/${B_THIS_PLUGIN}

# Here is the full paths to the executable and shared object in the out-of-source build directory.
BMON_THIS_PLUGIN_EXE_OOSRC_PATH = ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/${BMON_THIS_PLUGIN_EXE_FILENAME}
BMON_THIS_PLUGIN_SO_OOSRC_PATH  = ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/${BMON_THIS_PLUGIN_SO_FILENAME}

BMON_THIS_PLUGIN_HDR_INSTALL_PATH_LIST = $(foreach dir,${BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST},${dir}/include/${BMON_THIS_PLUGIN_HDR_FILENAME})
vpath %.h ${BMON_THIS_PLUGIN_INSRC_DIR}

# Build a list of all the source directories, then use vpath to tell
# make where to look for the source files.
BMON_THIS_PLUGIN_SRC_DIR_LIST := $(sort $(foreach f,$(BMON_THIS_PLUGIN_SRC_PATH_LIST),$(dir $(f))))
vpath %.c $(BMON_THIS_PLUGIN_SRC_DIR_LIST)

# Now define a list of object files that are needed to build the plugin executable.
# Start with an object file for each C file in the plugin's directory.
BMON_THIS_PLUGIN_OBJ_PATH_LIST += $(addprefix ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/, $(patsubst %.c,%.o,$(notdir $(BMON_THIS_PLUGIN_SRC_PATH_LIST))))

# Build a list of include directories that we'll need to compile the plugin's source files.
BMON_PLUGINS_PRIVATE_INCLUDES += ${BMON_THIS_PLUGIN_EXTRA_INCLUDE_DIR_LIST}
BMON_PLUGINS_PRIVATE_INCLUDES += ${BMON_THIS_PLUGIN_INSRC_DIR}
BMON_PLUGINS_PRIVATE_INCLUDES += ${BMON_PLUGINS_COMMON_INSRC_DIR}

BMON_PLUGINS_EXE_PRIVATE_DEFINES += BMON_PLUGIN

BMON_PLUGINS_EXE_PRIVATE_CFLAGS += $(addprefix -I,$(BMON_PLUGINS_PRIVATE_INCLUDES))   $(addprefix -D,$(BMON_PLUGINS_EXE_PRIVATE_DEFINES))

###################################################################
#  install target
###################################################################
#  The "install" target just lists all of the installed files as its prerequisites.
.PHONY: install install_exe

install: install_exe install_so  install_hdr
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

install_hdr:  ${BMON_THIS_PLUGIN_HDR_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

install_exe:  ${BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

install_so:  ${BMON_THIS_PLUGIN_SO_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

#  Determine the command prefix to use for cross-compile toolchain commands.
B_REFSW_CROSS_COMPILE ?= $(B_REFSW_ARCH)-

#  For each specified install directory, copy the executable from the build location in the out-of-source tree.
${BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST}:  ${BMON_THIS_PLUGIN_EXE_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $@) to $(dir $@) (BMON_PLUGINS_EXE)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}cp -dfp  ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/$(notdir $@) $@
	$(B_TRACE_RECIPE_END)

#  For each specified install directory, copy the shared object file from the build location in the out-of-source tree.
${BMON_THIS_PLUGIN_SO_INSTALL_PATH_LIST}:  ${BMON_THIS_PLUGIN_SO_OOSRC_PATH}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $@) to $(dir $@) (BMON_PLUGINS_SO)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}cp -dfp  ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/$(notdir $@) $@
	$(B_TRACE_RECIPE_END)

#  Build the plugin executable from it's corresponding object file and other common dependencies.
${BMON_THIS_PLUGIN_EXE_OOSRC_PATH} : ${BMON_THIS_PLUGIN_OBJ_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Link... $(notdir $<) (BMON_PLUGINS_EXE)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}$(B_REFSW_CROSS_COMPILE)gcc -o $@ ${BMON_THIS_PLUGIN_OBJ_PATH_LIST} ${BMON_PLUGINS_EXE_PRIVATE_CFLAGS} ${BMON_PLUGINS_EXE_PRIVATE_LDFLAGS}
	$(B_TRACE_RECIPE_END)


#  Build the plugin shared object from it's corresponding object file and other common dependencies.
${BMON_THIS_PLUGIN_SO_OOSRC_PATH} : ${BMON_THIS_PLUGIN_OBJ_PATH_LIST}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Link... $(notdir $<) (BMON_PLUGINS_SO)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}$(B_REFSW_CROSS_COMPILE)gcc -o $@ ${BMON_THIS_PLUGIN_OBJ_PATH_LIST} ${BMON_PLUGINS_EXE_PRIVATE_CFLAGS} ${BMON_PLUGINS_EXE_PRIVATE_LDFLAGS} -shared -Wl,-Map,${BMON_THIS_PLUGIN_SO_OOSRC_PATH}.map -Wl,--cref -Wl,-x -Wl,-soname,${BMON_THIS_PLUGIN_SO_FILENAME}
	$(B_TRACE_RECIPE_END)

#  Compile each plugin source into an object file.
${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}/%.o : %.c
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Compile... $(notdir $<) (BMON_PLUGINS_EXE)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}$(B_REFSW_CROSS_COMPILE)gcc -MMD  -fPIC   -c -o $@ ${BMON_PLUGINS_EXE_PRIVATE_CFLAGS}  ${BMON_PLUGINS_EXE_PRIVATE_LDFLAGS}  $<
	$(B_TRACE_RECIPE_END)

#  For each specified install directory, copy the header file from the in-source location to the out-of-source tree.
${BMON_THIS_PLUGIN_HDR_INSTALL_PATH_LIST}: ${BMON_THIS_PLUGIN_INSRC_DIR}/${BMON_THIS_PLUGIN_HDR_FILENAME}
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $@) to $(dir $@) (BMON_PLUGINS_EXE)]"
	${Q_}mkdir -p $(dir $@)
	${Q_}cp -dfp  $<  $@
	$(B_TRACE_RECIPE_END)


###################################################################
#  clean target
###################################################################
.PHONY: clean
clean: clean_with_subs

.PHONY: clean_with_subs
clean_with_subs: ${BMON_THIS_PLUGIN_CLEAN_WITH_SUBS_TARGET}

.PHONY: clean_all
clean_all: clean

.PHONY: clean_without_subs
clean_without_subs:  clean_exe clean_so clean_hdr
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

.PHONY: clean_exe
clean_exe:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... ${B_THIS_PLUGIN} (BMON_PLUGINS_EXE)]"
	$(Q_)$(RM) -rf ${BMON_THIS_PLUGIN_EXE_OOSRC_DIR}
	$(Q_)$(RM) -f ${BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_END)

.PHONY: clean_so
clean_so:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... ${B_THIS_PLUGIN} (BMON_PLUGINS_SO)]"
	$(Q_)$(RM) -rf ${BMON_THIS_PLUGIN_SO_OOSRC_DIR}
	$(Q_)$(RM) -f ${BMON_THIS_PLUGIN_SO_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_END)

.PHONY: clean_hdr
clean_hdr:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... ${B_THIS_PLUGIN} (BMON_PLUGINS_HDR)]"
	$(Q_)$(RM) -f ${BMON_THIS_PLUGIN_HDR_INSTALL_PATH_LIST}
	$(B_TRACE_RECIPE_END)

#  Print out a list of make variables for debugging.
ifeq (${BMON_MAKEFILE_DEBUG},y)
$(call BMON_PLUGIN_PRINTVAR ,NEXUS_PLATFORM)
$(call BMON_PLUGIN_PRINTVAR ,B_REFSW_ARCH)
$(call BMON_PLUGIN_PRINTVAR ,NEXUS_BIN_DIR)
$(call BMON_PLUGIN_PRINTVAR ,INSTALL_DIR)
$(call BMON_PLUGIN_PRINTVAR ,BSEAV)
$(call BMON_PLUGIN_PRINTVAR ,B_REFSW_OBJ_DIR)
$(call BMON_PLUGIN_PRINTVAR ,B_REFSW_OBJ_ROOT)
$(call BMON_PLUGIN_PRINTVAR ,B_REFSW_CROSS_COMPILE)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_FILENAME)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_INSRC_DIR)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_SRC_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_SRC_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_INSTALL_DIR_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_OOSRC_DIR)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_OOSRC_PATH)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_SRC_DIR_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_OBJ_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_EXE_INSTALL_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_SO_INSTALL_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,BMON_THIS_PLUGIN_HDR_INSTALL_PATH_LIST)
## $(call BMON_PLUGIN_PRINTVAR ,BMON_PLUGINS_EXE_PRIVATE_CFLAGS)
$(call BMON_PLUGIN_PRINTVAR ,BMON_PLUGINS_EXE_PRIVATE_LDFLAGS)
endif

BMON_THIS_PLUGIN_DEP_FILES = $(wildcard $(BMON_THIS_PLUGIN_EXE_OOSRC_DIR)/*.d)
ifneq ($(BMON_THIS_PLUGIN_DEP_FILES),)
-include $(BMON_THIS_PLUGIN_DEP_FILES)
endif
