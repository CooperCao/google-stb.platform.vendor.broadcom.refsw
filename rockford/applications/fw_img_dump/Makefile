#############################################################################
# (c) 2005-2015 Broadcom Corporation
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


# Requires the use of the 'plat' script which defines the following:
#   PLATFORM (97038, 97398, ...)
#   BCHP_VER (B0, B1, C0, ...)
#   BCHP_7411_VER (C0, D0, ...) plus versions for other secondary chipsets
#   DEBUG
#   LINUX
#   ARCH
#   TOOLCHAIN

# Target O/S (can be one of the following: linuxuser vxworks)
TARGET_OS = linuxemu
B_REFSW_OS = $(TARGET_OS)

# Directories relative to location of makefile
MAGNUM_DIR = ../../../magnum
ROCKFORD_DIR = ../../../rockford
FRAMEWORK_DIR = .

# Name of application
APP = fw_img_dump

# Application object file list, include path, C flags, and link
# flags
APP_SOURCES= \
	./fw_img_dump.c

# Initial application settings
APP_INCLUDES =
APP_CFLAGS   =
APP_LDFLAGS  =

# Must define ENDIAN to be either big or little
ENDIAN = little

# Want to see details (y/n)?
VERBOSE = n

# Debug (y/n)?
DEBUG ?= y
ifeq ($(DEBUG),y)
APP_CFLAGS += -DBDBG_DEBUG_BUILD
endif

CFLAGS += -DLINUX

# Include object file list, include path, and C flags for common utilities and base modules
# (sets MAGNUM_SOURCES, MAGNUM_INCLUDES, MAGNUM_CFLAGS, and MAGNUM_MODULE_DIR)
include $(FRAMEWORK_DIR)/build/magnum.inc

# Include object file list, include path, and C flags for other magnum modules required
# by this application (adds to MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS).
# Here is where you also define module specific makefile defines

# At this point, user can add additional files, include paths, and C flags to
# MAGNUM_SOURCES, MAGNUM_INCLUDES, and MAGNUM_CFLAGS (optional).

# Include other common modules (adds to COMMON_SOURCES, COMMON_INCLUDES, and
# COMMON_CFLAGS)

# Specifies how magnum files and application files are built.
include $(FRAMEWORK_DIR)/build/tools.inc

include $(MAGNUM_DIR)/portinginterface/vce/bvce_auth.inc
include $(MAGNUM_DIR)/portinginterface/xvd/bxvd_auth.inc
include $(MAGNUM_DIR)/portinginterface/sid/bsid_auth.inc
include $(MAGNUM_DIR)/portinginterface/xpt/bxpt_auth.inc
include $(MAGNUM_DIR)/basemodules/dsp/bdsp_auth.inc

# Determine list of all magnum sources and includes
MAGNUM_SOURCES = $(foreach module, $(MAGNUM_MODULES), $(${module}_SOURCES))
MAGNUM_INCLUDES = $(foreach module, $(MAGNUM_MODULES), $(${module}_INCLUDES))

# For computing file dependencies
CDEP_FLAG = -MD

CFLAGS += -Wall -W -Wstrict-prototypes -pedantic -m32
CFLAGS += $(addprefix -I${FRAMEWORK_DIR}/, ${FRAMEWORK_INCLUDES})
CFLAGS += $(MAGNUM_CFLAGS) $(addprefix -I, ${MAGNUM_INCLUDES})
CFLAGS += $(COMMON_FLAGS) $(addprefix -I, ${COMMON_INCLUDES})
CFLAGS += $(APP_CFLAGS) $(addprefix -I, ${APP_INCLUDES})
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))

SRC += $(MAGNUM_SOURCES)

# All all magnum and framework directories to C file search path
vpath %.c $(dir $(SRC))
SRC_NODIR = $(notdir $(SRC))

# Determine prefix of objects and executable directories
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${FRAMEWORK_DIR}/../../../${B_REFSW_OBJ_DIR}
ifeq (${B_REFSW_DEBUG},y)
PREFIX_DIR = $(NEXUS_PLATFORM).$(BCHP_VER).debug
CFLAGS += -g
LDFLAGS += -g
else
PREFIX_DIR = $(NEXUS_PLATFORM).$(BCHP_VER)
endif

# Location of magnum and framework objects
CUR_PREFIX := $(subst $(realpath ${FRAMEWORK_DIR}/../../..)/,,$(CURDIR))
ODIR := $(abspath ${B_REFSW_OBJ_ROOT}/${CUR_PREFIX}/${PREFIX_DIR})

# Name of application that is parameter specific
APP_CHIP = $(ODIR)/$(APP)

all: builddate $(APP_CHIP)

OBJ = $(APP_SOURCES:%.c=$(ODIR)/%.o) $(addprefix $(ODIR)/, $(SRC_NODIR:%.c=%.o))
DEP = $(OBJ:%.o=%.d)

ifneq (${VERBOSE},y)
Q_ = @
endif

$(APP_CHIP): $(OBJ) $(ODIR)/exists
	@echo Linking $@
	${Q_}$(CC) -o $@ $(CFLAGS) $(OBJ) -lpthread $(LDFLAGS) -lm

$(ODIR)/%.o : %.c ${ODIR}/exists
	@echo Compiling $<
	${Q_}$(CC) ${CDEP_FLAG} ${CFLAGS_STATIC} $(CFLAGS) -c $< -o $@

$(ODIR)/exists:
	$(MKDIR) $@

#$(ODIR)/builddate.o : builddate.c $(ODIR)/gitstatus.h ${ODIR}/exists
#	@echo Compiling $<
#	${Q_}$(CC) ${CFLAGS_STATIC} $(CFLAGS) -I$(ODIR) -c $< -o $@

# Programming note: will need to handle failure of git before activating this.
#$(ODIR)/gitstatus.h: FORCE
#	@echo "Querying git status"
#	${Q_}git status -uno | sed -e 's/^/"/g' -e 's/$$/",/g' > $@

# builddate.o must be built every time because it contains __DATE__
builddate:
	$(Q_)$(RM) -f ${ODIR}/builddate.o ${ODIR}/gitlog.h

clean:
	${Q_} $(RM) $(OBJ) $(DEP) $(APP_CHIP)

.PHONY: builddate FORCE

-include $(DEP)

# End of file
