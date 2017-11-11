#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################

# build env
TZ_TOP := ../..
TZ_MOD := user/tzioc_tapp
TZ_APP := tzioc_tapp

include $(TZ_TOP)/build/build_env.mk

# build src
SRC_DIRS := .
INC_DIRS := .
LIB_DIRS := .
BIN_DIRS := $(TZ_OBJ_DIR)

# dependent libs
DEP_LIBS := libtzioc
INC_DIRS += $(TZ_TOP)/user/libtzioc
LIB_DIRS += $(TZ_OBJ_TOP)/user/libtzioc

include $(TZ_TOP)/build/build_src.mk

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I$(TZ_TOP)/kernel/api
INCLUDES    += -I$(TZ_TOP)/kernel/api/arch/$(ARCH_DIR)
INCLUDES    += -I$(TZ_TOP)/common/include

BASEFLAGS   := -O2 -g

WARNFLAGS   := -Wall
WARNFLAGS   += -Werror

OSFLAGS     := -D__TZOS__

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=gnu99
CFLAGS      += -D_POSIX_SOURCE

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS)
CPPFLAGS    += -std=c++11
CPPFLAGS    += -D_POSIX_SOURCE

LDFLAGS     := $(addprefix -L,$(LIB_DIRS))

# build rules
.PHONY: checkplatform checkdirs $(TZ_APP) $(DEP_LIBS)

all: $(TZ_APP)

$(TZ_APP): checkplatform checkdirs $(TZ_OBJ_DIR)/$(TZ_APP).elf

checkplatform:
	@echo Building for TZ_PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	@mkdir -p $@

$(TZ_OBJ_DIR)/$(TZ_APP).elf: $(DEP_LIBS) $(OBJS)
	$(TZ_ARMGNU)-g++ $(LDFLAGS) $(patsubst lib%,-l%,$(DEP_LIBS)) $(OBJS) -o $@

$(DEP_LIBS):
	$(MAKE) -C $(TZ_TOP)/user/$@

clean:
	$(RM) -r $(TZ_OBJ_DIR)

distclean: clean
	@for lib in $(DEP_LIBS); do \
		$(MAKE) -C $(TZ_TOP)/user/$$lib clean; \
	done

include $(TZ_TOP)/build/build_rules.mk
