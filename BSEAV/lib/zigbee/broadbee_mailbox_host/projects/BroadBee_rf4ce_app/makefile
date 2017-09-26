##############################################################################
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
##############################################################################

### Suppress info messages from the make utility 'Entering/Leaving directory ...'
MAKEFLAGS += --no-print-directory

ifeq ($(BYPASS_RPC),y)
$(warning bypass RPC)
endif

# This needs to be adjusted to match the tool chain for the kernel being used.
#CROSS_COMPILE =/opt/toolchains/crosstools_hf-linux-2.6.18.0_gcc-4.2-10ts_uclibc-nptl-0.9.29-20070423_20080721/bin/mipsel-uclibc-
#CROSS_COMPILE =/opt/toolchains/stbgcc-4.4.5-2.0/bin/mips-linux-uclibc-
#CROSS_COMPILE =/opt/toolchains/stbgcc-4.5.4-2.5/bin/arm-linux-
CROSS_COMPILE ?= /opt/toolchains/stbgcc-4.8-1.0/bin/arm-linux-

# This is useful if cross-compiling. Taken from kernel Makefile (CC changed)
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
#EXES = rf4ce_reset_app rf4ce_pair_app rf4ce_reset_and_pair_app rf4ce_unpair_app rf4ce_set_fa_threshold_app rf4ce_get_fa_threshold_app rf4ce_set_discovery_lqi_threshold_app zrc2_pair_app
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

COMMON_OBJS += \
	$(OBJDIR)/zigbee_dbg.o

COMMON_OBJS += \
	$(OBJDIR)/rf4ce_common.o

RF4CE_GET_FW_REV_OBJS = \
	$(OBJDIR)/rf4ce_get_fw_rev.o \
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

RF4CE_PHY_TEST_OBJS = \
	$(OBJDIR)/rf4ce_phy_test.o \
	$(OBJDIR)/bbRF4CEPMProfiles.o \
	$(COMMON_OBJS)

RF4CE_SET_CHANNEL_OBJS = \
	$(OBJDIR)/rf4ce_set_single_channel.o \
	$(COMMON_OBJS)

PHY_TEST_OBJS = \
	$(OBJDIR)/phy_test.o \
	$(COMMON_OBJS)

ZRC2_PAIR_OBJS = \
	$(OBJDIR)/zrc2_pair.o \
	$(COMMON_OBJS)

##########################################################################################
# Path to the Host source directory relative to the make active directory.
HOST_PATH = ../../host


##########################################################################################
# Paths to the test application Project and Stack root directories.
PRJ_PATH = ./.
STACK_PATH = ../../stack

INC_PATHS := $(shell ls -d ./include $(STACK_PATH)/*/include $(STACK_PATH)/*/*/include $(STACK_PATH)/*/*/*/include 2>/dev/null)
#INC_PATHS += $(STACK_PATH)
INC_PATHS += $(HOST_PATH)/rpc/include
INC_PATHS += $(HOST_PATH)/rf4ce_registration/include
INC_PATHS += $(HOST_PATH)/ha_registration/include
INC_PATHS += $(HOST_PATH)/common/include

C99_FLAGS += -g                 # Debug level: emit full debug information.
C99_FLAGS += -std=gnu99
C99_FLAGS += -fpack-struct=4
C99_FLAGS += -fshort-enums
#C99_FLAGS += -m32               # 32-bit version

PRJ_FLAGS += _HOST_           # Build for Broadcom STB host.
PRJ_FLAGS += __i386__
PRJ_FLAGS += MAILBOX_UNIT_TEST
PRJ_FLAGS += _MAILBOX_WRAPPERS_TEST_ENGINE_=1
PRJ_FLAGS += _DEBUG_
PRJ_FLAGS += _DEBUG_COMPLEX_
PRJ_FLAGS += _DEBUG_LOG_
PRJ_FLAGS += _DEBUG_FILELINE_
PRJ_FLAGS += _MAILBOX_WRAPPERS_MAC_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_NWK_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_PROFILE_=2
PRJ_FLAGS += _MAILBOX_INTERFACE_=1
PRJ_FLAGS += _RF4CE_
PRJ_FLAGS += RF4CE_TARGET
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += USE_RF4CE_PROFILE_GDP=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC2
PRJ_FLAGS += _MAC_BAN_TABLE_SIZE_=1
PRJ_FLAGS += _PHY_TEST_HOST_INTERFACE_
PRJ_FLAGS += _USE_ASYNC_UART_

ifeq ($(BYPASS_RPC),y)
PRJ_FLAGS += BYPASS_RPC
endif

CLIENT_PRJ_FLAGS = $(PRJ_FLAGS)
CLIENT_PRJ_FLAGS += CLIENT
CLIENT_PRJ_FLAGS += _DEBUG_CONSOLELOG_=2

SERVER_PRJ_FLAGS = $(PRJ_FLAGS)
SERVER_PRJ_FLAGS += SERVER
SERVER_PRJ_FLAGS += _DEBUG_HARNESSLOG_=2

__PRJ_FLAGS = $(addprefix -D, $(CLIENT_PRJ_FLAGS))
__PRJ_INCS  = $(addprefix -I, $(INC_PATHS))

__SERVER_PRJ_FLAGS = $(addprefix -D, $(SERVER_PRJ_FLAGS))

CFLAGS += $(__PRJ_INCS) $(C99_FLAGS) $(__PRJ_FLAGS)

#LDFLAGS += -m32
LDFLAGS += -std=gnu99
LDFLAGS += -g -Xlinker --no-enum-size-warning
ifeq ($(BYPASS_RPC),y)
LDFLAGS += -L../BroadBee_mailboxHost/obj.$(NEXUS_PLATFORM) -L../../host/rf4ce_registration -lzigbee -lsqlite3
endif

all:   prebuild $(OBJDIR) zigbee_lib
ifeq ($(BYPASS_RPC),y)
	@$(MAKE) $(OBJDIR)/rf4ce_reset_app $(OBJDIR)/rf4ce_pair_app $(OBJDIR)/rf4ce_unpair_app $(OBJDIR)/rf4ce_reset_and_pair_app $(OBJDIR)/rf4ce_set_discovery_lqi_threshold_app $(OBJDIR)/rf4ce_set_fa_threshold_app $(OBJDIR)/rf4ce_get_fa_threshold_app $(OBJDIR)/rf4ce_phy_test_app $(OBJDIR)/rf4ce_get_fw_rev_app $(OBJDIR)/rf4ce_set_channel_app $(OBJDIR)/phy_test_app
else
	@$(MAKE) $(OBJDIR)/rf4ce_reset_app $(OBJDIR)/rf4ce_pair_app $(OBJDIR)/rf4ce_unpair_app $(OBJDIR)/rf4ce_reset_and_pair_app $(OBJDIR)/rf4ce_set_discovery_lqi_threshold_app $(OBJDIR)/rf4ce_set_fa_threshold_app $(OBJDIR)/rf4ce_get_fa_threshold_app $(OBJDIR)/rf4ce_phy_test_app $(OBJDIR)/rf4ce_get_fw_rev_app $(OBJDIR)/rf4ce_set_channel_app $(OBJDIR)/phy_test_app $(OBJDIR)/zrc2_pair_app
endif

#	$(LD) $(OBJS) -lpthread


zigbee_lib:
	$(SILENT)make "CDEFSFLAGS=$(__SERVER_PRJ_FLAGS)" -C ../BroadBee_mailboxHost

clean:
	$(SILENT)rm -rf $(OBJDIR)
	$(SILENT)rm -rf ./bbMailService.c
	$(SILENT)rm -rf ./obj.$(NEXUS_PLATFORM)
	make -C ../BroadBee_mailboxHost clean

$(OBJDIR)/rf4ce_get_fw_rev_app: $(RF4CE_GET_FW_REV_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_reset_app: $(RF4CE_RESET_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_reset_and_pair_app: $(RF4CE_RESET_AND_PAIR_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_pair_app: $(RF4CE_PAIR_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_unpair_app: $(RF4CE_UNPAIR_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_fa_threshold_app: $(RF4CE_SET_FA_THRESHOLD_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_get_fa_threshold_app: $(RF4CE_GET_FA_THRESHOLD_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_discovery_lqi_threshold_app: $(RF4CE_SET_DISCOVERY_LQI_THRESHOLD_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_phy_test_app: $(RF4CE_PHY_TEST_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/rf4ce_set_channel_app: $(RF4CE_SET_CHANNEL_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/phy_test_app: $(PHY_TEST_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zrc2_pair_app: $(ZRC2_PAIR_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

prebuild:
	@rm -rf $(PRJ_PATH)/bbMailService.c
	@cp $(STACK_PATH)/common/Mailbox/src/bbMailService.c $(PRJ_PATH)/bbMailService.c
	@sed 's/\.function[ ].*= (MailPublicFunction_t)function_name/\.function      = 0/g' $(STACK_PATH)/common/Mailbox/src/bbMailService.c > $(PRJ_PATH)/bbMailService.c
	@echo "" >> $(PRJ_PATH)/bbMailService.c
	@echo "" >> $(PRJ_PATH)/bbMailService.c
	@echo "void mailClientInit() {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "void mailServerInit() {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "void mailAdapterInit() {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "void mailAdapterDeinit() {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "void Mail_TestEngineSendHello() {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "void mailClientSerialize(const uint16_t fId, uint8_t *const req) {}" >> $(PRJ_PATH)/bbMailService.c
	@echo "MailPendingAPICall_t *mailClientFindEmptyPendingTableEntry(void) { return NULL; }" >> $(PRJ_PATH)/bbMailService.c
	@echo "=== prebuild completed ==="

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbSysPayload.o: $(STACK_PATH)/common/System/src/bbSysPayload.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbRF4CEPMProfiles.o: $(STACK_PATH)/RF4CE/Profiles/ProfileManager/src/bbRF4CEPMProfiles.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(STACK_PATH)/common/System/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(HOST_PATH)/common/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(HOST_PATH)/rpc/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

%: %.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -o $@
#	$(CC) $(CFLAGS) $^ -o $@ -lpthread

### eof Makefile #########################################################################
