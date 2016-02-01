#############################################################################
# (c) 2003-2014 Broadcom Corporation
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

#
# Driver options
#

BUILDDIR 	= $(shell /bin/pwd)
LIBNAME	 = libsi

BSEAV ?= $(shell cd ../../../../../BSEAV; pwd)

THIS_DIR := BSEAV/cable/lib/si/build
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
#B_REFSW_OBJ_ROOT ?= ${BSEAV_TOP}/../${B_REFSW_OBJ_DIR}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_TOP}/${B_REFSW_OBJ_DIR}

#
# Important toolchain and linux directories (make sure these are correct for your environment)
#

#include Tools.make

#
# Useful directory definitions
#

SI_DIR		= $(BSEAV)/cable/lib/si


TARGET		=	$(SI_OBJ_DIR)/$(LIBNAME).a

SI_VPATH 	= $(SI_DIR)/dbg \
 		  $(SI_DIR) \
 		  $(SI_DIR)/mpeg2_ts_parse \
 		  $(SI_DIR)/api \
		  $(SI_DIR)/aeit \
 		  $(SI_DIR)/aett \
 		  $(SI_DIR)/os/linux \
 		  $(SI_DIR)/lvct \
 		  $(SI_DIR)/mgt \
 		  $(SI_DIR)/svct \
 		  $(SI_DIR)/rrt \
 		  $(SI_DIR)/nit \
 		  $(SI_DIR)/ntt \
 		  $(SI_DIR)/stt  \
 		  $(SI_DIR)/ea


SI_INCLUDE=$(patsubst %,-I%,$(subst :, ,$(SI_VPATH)))

#
# Build Options
#

DEBUG 		= n
SI_OBJ_DIR	= $(B_REFSW_OBJ_ROOT)/BSEAV/cable/lib/si

VPATH = $(SI_VPATH)

# 		  si_lvct.c

#
# Source files
#
SI_SRC = si_dbg.c	\
 		  si_main.c \
		channellist.c \
		si_api_chanmap.c \
 		  si_util.c \
 		  si_aeit.c \
 		  si_aett.c \
 		  si_lvct.c \
 		  si_svct.c \
 		  si_svct_vcm.c \
 		  si_mgt.c \
 		  si_nit.c \
 		  si_nit_cds.c \
 		  si_nit_mms.c \
 		  si_ntt.c \
 		  si_ntt_sns.c \
 		  si_rrt.c \
 		  si_stt.c \
 		  si_ea.c \
		psip.c \
		psip_common.c \
		psip_dccsct.c \
		psip_dcct.c \
		psip_descriptor.c \
		psip_decode_tree_01.c \
		psip_decode_tree_02.c \
		psip_eit.c \
		psip_ett.c \
		psip_mgt.c \
		psip_mss.c \
		psip_rrt.c \
		psip_stt.c \
		psip_vct.c \
	ts_priv.o \
	ts_psi.o \
	ts_pmt.o \
	ts_pat.o \
	tspsimgr.o

#
# Optional compile and build options support
#

SOFTWARE_RS_MPOD ?= n
ifeq ($(SOFTWARE_RS_MPOD),y)
CFLAGS += -DSOFTWARE_RS_MPOD
ROCKFORD_TOP = $(BSEAV)/../rockford
CFLAGS += -I$(BSEAV)/cable/lib/mpod/include
endif

ifneq ($(STD_INC),)
	CFLAGS += -I$(STD_INC)
	CPPFLAGS += -I$(STD_INC)
endif

ifneq ($(GCC_INC),)
	CFLAGS += -I$(GCC_INC)
	CPPFLAGS += -I$(GCC_INC)
endif

ifeq ($(DEBUG),y)
#	SI_DEBUG_FLAGS = -g -DSI_DEBUG
	SI_DEBUG_FLAGS = -O2 -DSI_DEBUG
else
	SI_DEBUG_FLAGS = -O2
endif

CFLAGS += -DLINUX $(SI_DEBUG_FLAGS)

#
# Include directories
#
CFLAGS += $(SI_INCLUDE)

NEXUS_TOP = $(BSEAV)/../nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
# This is the minimum needed to compile and link with Nexus
CFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
#CFLAGS += -I$(NEXUS_TOP)/base/include -I$(NEXUS_TOP)/modules/frontend/common/include

#
# obj files
#
SI_OBJS = $(SI_SRC:.c=.o)
SI_TEST_OBJ = $(SI_TEST_SRC:.c=.o)


vpath %.h $(SI_VPATH)
vpath %.c $(SI_VPATH)
vpath %.o $(SI_OBJ_DIR)

all: $(TARGET) $(addprefix $(SI_OBJ_DIR)/, $(SI_TEST_OBJ))

$(TARGET): $(addprefix $(SI_OBJ_DIR)/, $(SI_OBJS))
	$(AR)  -cvr $@ $?

$(SI_OBJ_DIR)/%.o: %.c
	${Q_}mkdir -p $(SI_OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# include individual depend files
-include $(SI_OBJ_DIR)/*.d

# clean rule calls makefile in driver directory for both PCI=y
# and settop to make sure all platform-specific object files
# get removed.

clean:
	$(RM) $(TARGET) -f $(SI_OBJ_DIR)/*.o $(SI_OBJ_DIR)/*.d *~ core



