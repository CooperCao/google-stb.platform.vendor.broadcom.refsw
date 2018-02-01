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

EXES = zigbee_ha_onoff_controller

OBJDIR = obj.$(NEXUS_PLATFORM)

COMMON_OBJS = \
    $(OBJDIR)/bbMailService.o \
    $(OBJDIR)/zigbee_rpc.o \
    $(OBJDIR)/zigbee_api.o \
    $(OBJDIR)/zigbee_rpc_client.o \
    $(OBJDIR)/zigbee_socket_client.o \
    $(OBJDIR)/bbSysPayload.o

ZIGBEE_HA_ONOFF_CONTROLLER_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_onoff_controller.o \
    $(COMMON_OBJS)

ZIGBEE_HA_PERMIT_JOIN_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_permit_join.o \
    $(COMMON_OBJS)

ZIGBEE_HA_DUMP_DESCRIPTOR_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_dump_descriptor.o \
    $(COMMON_OBJS)

ZIGBEE_HA_SET_ATTRIBUTE_REPORT_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_set_attribute_report.o \
    $(COMMON_OBJS)

ZIGBEE_HA_REJOIN_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_rejoin.o \
    $(COMMON_OBJS)

ZIGBEE_HA_DOORLOCK_CONTROLLER_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_doorlock_controller.o \
    $(COMMON_OBJS)

ZIGBEE_HA_RESET_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_reset.o \
    $(COMMON_OBJS)

ZIGBEE_HA_IAS_CIE_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_ias_cie.o \
    $(COMMON_OBJS)

ZIGBEE_HA_STB_COMMISSIONING_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_stb_commissioning.o \
    $(COMMON_OBJS)

ZIGBEE_HA_NORMAL_OPERATION_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_normal_operation.o \
    $(COMMON_OBJS)

ZIGBEE_HA_ED_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_ed.o \
    $(COMMON_OBJS)

ZIGBEE_HA_COEX_ENABLE_APP_OBJS = \
    $(OBJDIR)/zigbee_ha_coex_enable.o \
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
#C99_FLAGS += -m32               # 32-bit version
C99_FLAGS += -Wenum-compare

PRJ_FLAGS += _HOST_           # Build for Broadcom STB host.
PRJ_FLAGS += __i386__
PRJ_FLAGS += CLIENT
PRJ_FLAGS += MAILBOX_UNIT_TEST
PRJ_FLAGS += _DEBUG_
PRJ_FLAGS += _DEBUG_COMPLEX_
PRJ_FLAGS += _DEBUG_LOG_
PRJ_FLAGS += _RF4CE_
PRJ_FLAGS += RF4CE_TARGET
PRJ_FLAGS += _ZBPRO_
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += _DEBUG_CONSOLELOG_=2
PRJ_FLAGS += _DEBUG_FILELINE_
PRJ_FLAGS += _MAILBOX_WRAPPERS_TEST_ENGINE_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_MAC_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_NWK_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_APS_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_ZDO_=2
PRJ_FLAGS += _MAILBOX_WRAPPERS_TC_=2
PRJ_FLAGS += _MAILBOX_WRAPPERS_ZCL_=2
PRJ_FLAGS += _MAILBOX_WRAPPERS_PROFILE_=2
PRJ_FLAGS += _MAILBOX_WRAPPERS_ZHA_=2
PRJ_FLAGS += _MAILBOX_INTERFACE_=1
PRJ_FLAGS += _MAC_BAN_TABLE_SIZE_=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += USE_RF4CE_PROFILE_GDP=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC2
PRJ_FLAGS += _ZHA_PROFILE_CIE_DEVICE_IMPLEMENTATION_
PRJ_FLAGS += MULTIPLE_ATTRIBUTES_READ
PRJ_FLAGS += _PHY_TEST_HOST_INTERFACE_
__PRJ_FLAGS = $(addprefix -D, $(PRJ_FLAGS))
__PRJ_INCS  = $(addprefix -I, $(INC_PATHS))

CFLAGS += $(__PRJ_INCS) $(C99_FLAGS) $(__PRJ_FLAGS)

#LDFLAGS += -m32
LDFLAGS += -std=gnu99
LDFLAGS += -g

all:   prebuild $(OBJDIR) $(OBJDIR)/zigbee_ha_set_attribute_report_app $(OBJDIR)/zigbee_ha_stb_commissioning_app $(OBJDIR)/zigbee_ha_reset_app $(OBJDIR)/zigbee_ha_ed_app $(OBJDIR)/zigbee_ha_coex_enable_app

#	$(LD) $(OBJS) -lpthread

clean:
	$(SILENT)rm -rf $(OBJDIR)
	$(SILENT)rm -rf ./bbMailService.c
	$(SILENT)rm -rf ./obj.$(NEXUS_PLATFORM)

$(OBJDIR)/zigbee_ha_onoff_controller_app:$(ZIGBEE_HA_ONOFF_CONTROLLER_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_permit_join_app:$(ZIGBEE_HA_PERMIT_JOIN_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_dump_descriptor_app:$(ZIGBEE_HA_DUMP_DESCRIPTOR_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_set_attribute_report_app:$(ZIGBEE_HA_SET_ATTRIBUTE_REPORT_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_rejoin_app:$(ZIGBEE_HA_REJOIN_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_doorlock_controller_app:$(ZIGBEE_HA_DOORLOCK_CONTROLLER_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_reset_app:$(ZIGBEE_HA_RESET_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_ias_cie_app: $(ZIGBEE_HA_IAS_CIE_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_stb_commissioning_app: $(ZIGBEE_HA_STB_COMMISSIONING_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_normal_operation_app: $(ZIGBEE_HA_NORMAL_OPERATION_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_ed_app: $(ZIGBEE_HA_ED_APP_OBJS)
	$(SILENT)$(CC) $^ -lpthread -o $@ $(LDFLAGS)

$(OBJDIR)/zigbee_ha_coex_enable_app: $(ZIGBEE_HA_COEX_ENABLE_APP_OBJS)
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

$(OBJDIR)/bbMailService.o: bbMailService.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbMailClient.o: $(STACK_PATH)/common/Mailbox/src/bbMailClient.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbMailServer.o: $(STACK_PATH)/common/Mailbox/src/bbMailServer.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbMailAdapter.o: $(STACK_PATH)/common/Mailbox/src/bbMailAdapter.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/bbSysPayload.o: $(STACK_PATH)/common/System/src/bbSysPayload.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(STACK_PATH)/common/System/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(HOST_PATH)/rpc/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

$(OBJDIR)/%.o: $(HOST_PATH)/common/src/%.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS)  $^ -c -o $@

%: %.c
	@echo --- C99: $@
	$(SILENT)$(CC) $(CFLAGS) $^ -o $@
#	$(CC) $(CFLAGS) $^ -o $@ -lpthread

### eof Makefile #########################################################################
