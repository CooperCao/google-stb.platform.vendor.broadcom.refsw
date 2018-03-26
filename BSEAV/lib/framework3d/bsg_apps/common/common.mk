#############################################################################
# Broadcom Proprietary and Confidential. (c)2012 Broadcom.  All rights reserved.
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

# Get the platform locations sorted
ifndef NEXUS_PLATFORM
	$(error NEXUS_PLATFORM is not defined)
endif

ifndef BCHP_VER
	$(error BCHP_VER is not defined)
endif

BSEAV_DIR	:= $(shell cd ../../../..            ; pwd)
NEXUS_TOP	?= $(shell cd $(BSEAV_DIR)/../nexus  ; pwd)
BSG_DIR     := $(shell pwd)/../../bsg

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR ?= $(shell cd $(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver; pwd)

B_REFSW_OBJ_DIR  ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV_DIR}/../${B_REFSW_OBJ_DIR}

BSG_OBJ_DIR      ?= $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bsg_apps/common

CXXFLAGS += -O2 -DNDEBUG -std=c++0x
ifeq ($(V3D_PREFIX),vc5)
	CXXFLAGS += -DBSG_USE_ES3 -DBSG_VC5
endif
CXXFLAGS += -I$(BSG_DIR)
CXXFLAGS += -I$(V3D_DIR)/interface/khronos/include

all: $(BSG_OBJ_DIR)/bsg_common.a
.PHONY: all clean

SOURCES = bcm_backdrop.cpp \
			 bcm_guilloche.cpp \
			 bcm_help_menu.cpp \
			 bcm_wiggle.cpp

OBJECTS = $(addprefix $(BSG_OBJ_DIR)/, $(addsuffix .o, $(basename $(SOURCES))))
$(info $(OBJECTS))

$(OBJECTS): $(BSG_OBJ_DIR)/%.o: %.cpp
	@echo Compiling $^
	@mkdir -p $(BSG_OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $^ -o $@

$(BSG_OBJ_DIR)/bsg_common.a: $(OBJECTS)
	@$(AR) rcs $@ $?

clean:
	@rm -rf $(BSG_OBJ_DIR)
