
#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

ifeq ($(BYPASS_RPC),y)
$(warning bypass RPC)
endif

# This needs to be adjusted to match the tool chain for the kernel being used.
#CROSS_COMPILE =/opt/toolchains/crosstools_hf-linux-2.6.18.0_gcc-4.2-10ts_uclibc-nptl-0.9.29-20070423_20080721/bin/mipsel-uclibc-
#CROSS_COMPILE =/opt/toolchains/stbgcc-4.4.5-2.0/bin/mips-linux-uclibc-
#CROSS_COMPILE =/opt/toolchains/stbgcc-4.5.4-2.5/bin/arm-linux-

# This is useful if cross-compiling. Taken from kernel Makefile (CC changed)
CROSS_COMPILE ?= /opt/toolchains/stbgcc-4.8-1.0/bin/arm-linux-
AS      =$(CROSS_COMPILE)as
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc
CPP     =$(CC) -E
AR      =$(CROSS_COMPILE)ar
NM      =$(CROSS_COMPILE)nm
STRIP   =$(CROSS_COMPILE)strip
OBJCOPY =$(CROSS_COMPILE)objcopy
OBJDUMP =$(CROSS_COMPILE)objdump

SILENT ?= @

CFLAGS += -g -I.

#ifeq ($(BYPASS_RPC),y)
#EXES = rf4ce_reset_and_pair_app
#else
#EXES = my_rf4ce_app rf4ce_reset_app rf4ce_pair_app rf4ce_reset_and_pair_app rf4ce_unpair_app zr2_pair_app rf4ce_set_fa_threshold_app rf4ce_get_fa_threshold_app rf4ce_set_discovery_lqi_threshold_app
#endif

OBJDIR = obj.$(NEXUS_PLATFORM)

#    $(OBJDIR)/$(EXES).o
ifeq ($(BYPASS_RPC),y)
COMMON_OBJS = \
    $(OBJDIR)/bbSysPayload.o
else
COMMON_OBJS = \
    $(OBJDIR)/bbMailService.o \
    $(OBJDIR)/zigbee_rpc.o \
    $(OBJDIR)/zigbee_api.o \
    $(OBJDIR)/zigbee_rpc_client.o \
    $(OBJDIR)/zigbee_socket_client.o \
    $(OBJDIR)/bbSysPayload.o
endif

RF4CE_APP_OBJS = \
    $(OBJDIR)/my_rf4ce_app.o \
    $(COMMON_OBJS)

RF4CE_RESET_OBJS = \
    $(OBJDIR)/rf4ce_reset.o \
    $(COMMON_OBJS)

RF4CE_RESET_AND_PAIR_OBJS = \
    $(OBJDIR)/rf4ce_reset_and_pair.o \
    $(COMMON_OBJS)

RF4CE_PAIR_OBJS = \
    $(OBJDIR)/rf4ce_pair.o \
    $(COMMON_OBJS)

ZRC2_PAIR_OBJS = \
    $(OBJDIR)/zrc2_pair.o \
    $(COMMON_OBJS)

RF4CE_UNPAIR_OBJS = \
    $(OBJDIR)/rf4ce_unpair.o \
    $(COMMON_OBJS)

RF4CE_SET_FA_THRESHOLD_OBJS = \
    $(OBJDIR)/rf4ce_set_fa_threshold.o \
    $(COMMON_OBJS)

RF4CE_GET_FA_THRESHOLD_OBJS = \
    $(OBJDIR)/rf4ce_get_fa_threshold.o \
    $(COMMON_OBJS)

RF4CE_SET_DISCOVERY_LQI_THRESHOLD_OBJS = \
    $(OBJDIR)/rf4ce_set_discovery_lqi_threshold.o \
    $(COMMON_OBJS)

RF4CE_SET_CHANNEL_OBJS = \
    $(OBJDIR)/rf4ce_set_single_channel.o \
    $(COMMON_OBJS)

RF4CE_PHY_TEST_OBJS = \
    $(OBJDIR)/rf4ce_phy_test.o \
    $(OBJDIR)/bbRF4CEPMProfiles.o \
    $(COMMON_OBJS)

##########################################################################################
# Path to the Project root directory relative to the make active directory.
PRJ_PATH = ../


##########################################################################################
# Paths to the Test Application and Stack root directories.
#APP_PATH = $(PRJ_PATH)src
STACK_PATH = $(PRJ_PATH)../../stack

INC_PATHS := $(shell ls -d ./include $(STACK_PATH)/*/include $(STACK_PATH)/*/*/include $(STACK_PATH)/*/*/*/include 2>/dev/null)
#INC_PATHS += $(STACK_PATH)

ifneq ($(BYPASS_RPC),y)
INC_PATHS += $(PRJ_PATH)/rpc/include
endif
INC_PATHS += $(PRJ_PATH)/rf4ce_registration/include
INC_PATHS += $(PRJ_PATH)/ha_registration/include
INC_PATHS += $(PRJ_PATH)/common/include
#INC_PATHS += $(STACK_PATH)/common/System/include
#INC_PATHS += $(STACK_PATH)/common/NVM/include/
#INC_PATHS += $(STACK_PATH)/common/HAL/i386-utest/include
#INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/MSO/include
#INC_PATHS += $(STACK_PATH)/common/External/include
#INC_PATHS += $(STACK_PATH)/common/Mailbox/include
#INC_PATHS += $(STACK_PATH)/common/RPC/include
#INC_PATHS += $(STACK_PATH)/common/HAL/include
##INC_PATHS += $(STACK_PATH)/common/HAL/SoC/include
#INC_PATHS += $(STACK_PATH)/common/HAL/i386-utest/include
#INC_PATHS += $(STACK_PATH)/IEEE/MAC/include
#INC_PATHS += $(STACK_PATH)/IEEE/PHY/include
#INC_PATHS += $(STACK_PATH)/common/Security/include
#INC_PATHS += $(STACK_PATH)/RF4CE/NWK/include
#INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/ProfileManager/include
#INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/GDP/include
#INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/ZRC/include
#INC_PATHS += $(STACK_PATH)/ZbPro/NWK/include
#INC_PATHS += $(STACK_PATH)/ZbPro/APS/include
#INC_PATHS += $(STACK_PATH)/ZbPro/SSP/include
#INC_PATHS += $(STACK_PATH)/ZbPro/TC/include
#INC_PATHS += $(STACK_PATH)/ZbPro/ZDO/include
#INC_PATHS += $(STACK_PATH)/ZbPro/ZCL/include
#INC_PATHS += $(STACK_PATH)/ZbPro/Profiles/ZHA/include
INC_PATHS += ../../../../../../BSEAV/linux/driver/zigbee/

C99_FLAGS += -g                 # Debug level: emit full debug information.
C99_FLAGS += -std=gnu99
C99_FLAGS += -fpack-struct=4
C99_FLAGS += -fshort-enums
#C99_FLAGS += -m32               # 32-bit version

PRJ_FLAGS += _HOST_           # Build for Broadcom STB host.
PRJ_FLAGS += __i386__

    _DEBUG_HARNESSLOG_=2 \
    _DEBUG_FILELINE_ \
    _MAILBOX_WRAPPERS_TEST_ENGINE_=1 \
    _MAILBOX_WRAPPERS_MAC_=1 \
    _MAILBOX_WRAPPERS_NWK_=1 \
    _MAILBOX_WRAPPERS_PROFILE_=2 \
    _MAILBOX_INTERFACE_=1 \
    _RF4CE_ \
    RF4CE_TARGET \
    USE_RF4CE_PROFILE_ZRC1 \
    _ZBPRO_ \
    _MAC_BAN_TABLE_SIZE_=1

PRJ_FLAGS += CLIENT
PRJ_FLAGS += MAILBOX_UNIT_TEST
PRJ_FLAGS += _DEBUG_
PRJ_FLAGS += _DEBUG_COMPLEX_
PRJ_FLAGS += _DEBUG_LOG_
PRJ_FLAGS += _RF4CE_
PRJ_FLAGS += RF4CE_TARGET
#PRJ_FLAGS += _ZBPRO_
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += _DEBUG_CONSOLELOG_=2
PRJ_FLAGS += _DEBUG_FILELINE_
PRJ_FLAGS += _MAILBOX_WRAPPERS_TEST_ENGINE_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_MAC_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_NWK_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_PROFILE_=2
PRJ_FLAGS += _MAILBOX_INTERFACE_=1
PRJ_FLAGS += _MAC_BAN_TABLE_SIZE_=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += USE_RF4CE_PROFILE_GDP=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC2
PRJ_FLAGS += _PHY_TEST_HOST_INTERFACE_

ifeq ($(BYPASS_RPC),y)
PRJ_FLAGS += BYPASS_RPC
endif

__PRJ_FLAGS = $(addprefix -D, $(PRJ_FLAGS))
__PRJ_INCS  = $(addprefix -I, $(INC_PATHS))

CFLAGS += $(__PRJ_INCS) $(C99_FLAGS) $(__PRJ_FLAGS)

#LDFLAGS += -m32
LDFLAGS += -std=gnu99
LDFLAGS += -g -Xlinker --no-enum-size-warning
ifeq ($(BYPASS_RPC),y)
LDFLAGS += -L../obj.$(NEXUS_PLATFORM) -L../rf4ce_registration -lzigbee -lsqlite3
endif

all:   prebuild $(OBJDIR)
	@$(MAKE) $(OBJDIR)/my_rf4ce_app $(OBJDIR)/rf4ce_reset_app $(OBJDIR)/rf4ce_pair_app $(OBJDIR)/rf4ce_unpair_app $(OBJDIR)/rf4ce_reset_and_pair_app $(OBJDIR)/zrc2_pair_app $(OBJDIR)/rf4ce_set_discovery_lqi_threshold_app $(OBJDIR)/rf4ce_set_fa_threshold_app $(OBJDIR)/rf4ce_get_fa_threshold_app $(OBJDIR)/rf4ce_set_channel_app $(OBJDIR)/rf4ce_phy_test_app

#	$(LD) $(OBJS) -lpthread

clean:
	rm -rf $(OBJDIR)/*
	rm -rf ./bbMailClientTable.c
	rm -rf ./bbMailServerTable.c
	rm -rf ./bbSysPayload.c
	rm -rf ./bbRF4CEPMProfiles.c
#	rm -f *.o *.so
#	rm -f *.i *.s

$(OBJDIR)/my_rf4ce_app: $(RF4CE_APP_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_reset_app: $(RF4CE_RESET_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_reset_and_pair_app: $(RF4CE_RESET_AND_PAIR_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_pair_app: $(RF4CE_PAIR_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_unpair_app: $(RF4CE_UNPAIR_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zrc2_pair_app: $(ZRC2_PAIR_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_fa_threshold_app: $(RF4CE_SET_FA_THRESHOLD_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_get_fa_threshold_app: $(RF4CE_GET_FA_THRESHOLD_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_discovery_lqi_threshold_app: $(RF4CE_SET_DISCOVERY_LQI_THRESHOLD_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_channel_app: $(RF4CE_SET_CHANNEL_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_phy_test_app: $(RF4CE_PHY_TEST_OBJS)
	$(CC) $^ -lpthread -o $@ $(LDFLAGS)

prebuild:
	@rm -rf ./bbMailClientTable.c
	#@cp ../../../stack/common/Mailbox/src/bbMailService.c ./bbMailService.c
	#@sed   's/SERVER_TABLE_ENTRY(\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\))/SERVER_TABLE_ENTRY(\1, \2, \3, \4, 0)/' ../../../stack/common/Mailbox/src/bbMailServerTable.c > ./bbMailServerTable.c.bak
	#@sed   's/SERVER_TABLE_ENTRY(\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\), *\([a-zA-Z_0-9\.]*\))/SERVER_TABLE_ENTRY(\1, \2, \3, \4, 0, \6)/' ./bbMailServerTable.c.bak > ./bbMailServerTable.c
	@rm -rf ./bbMailServerTable.c.bak
	#@cp ../../../stack/common/Mailbox/src/bbMailClientTable.c ./bbMailClientTable.c
	@cp ../../../stack/common/System/src/bbSysPayload.c ./bbSysPayload.c
	@cp ../../../stack/RF4CE/Profiles/ProfileManager/src/bbRF4CEPMProfiles.c ./bbRF4CEPMProfiles.c
	@echo "=== prebuild completed ==="

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbMailClientTable.o: bbMailClientTable.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbMailServerTable.o: bbMailServerTable.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbSysPayload.o: bbSysPayload.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbRF4CEPMProfiles.o: bbRF4CEPMProfiles.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: ../common/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -c -o $@

#ifneq ($(BYPASS_RPC),y)
$(OBJDIR)/%.o: ../rpc/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -c -o $@
#endif

$(OBJDIR)/%.o: ../../../stack/common/System/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -c -o $@

%: %.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -o $@
#	$(CC) $(CFLAGS) $^ -o $@ -lpthread
