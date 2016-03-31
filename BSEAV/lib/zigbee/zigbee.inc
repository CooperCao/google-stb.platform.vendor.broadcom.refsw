###############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
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
###############################################################################


##########################################################################################
# Path to the Project root directory relative to the make active directory.
PRJ_PATH = $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide

##########################################################################################
# Paths to the Test Application and Stack root directories.
#APP_PATH = $(PRJ_PATH)src
STACK_PATH = $(PRJ_PATH)../../stack

INC_PATHS += $(STACK_PATH)
INC_PATHS += $(PRJ_PATH)/rpc/include
INC_PATHS += $(PRJ_PATH)/rf4ce_registration/include
INC_PATHS += $(STACK_PATH)/common/include
INC_PATHS += $(STACK_PATH)/common/System/include
INC_PATHS += $(STACK_PATH)/common/HAL/i386-utest/include
INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/MSO/include
INC_PATHS += $(STACK_PATH)/common/Mailbox/include
INC_PATHS += $(STACK_PATH)/common/RPC/include
INC_PATHS += $(STACK_PATH)/common/HAL/include
INC_PATHS += $(STACK_PATH)/common/HAL/SoC/include
INC_PATHS += $(STACK_PATH)/IEEE/MAC/include
INC_PATHS += $(STACK_PATH)/IEEE/PHY/include
INC_PATHS += $(STACK_PATH)/common/Security/include
INC_PATHS += $(STACK_PATH)/RF4CE/NWK/include
INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/ProfileManager/include
INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/GDP/include
INC_PATHS += $(STACK_PATH)/RF4CE/Profiles/ZRC/include
INC_PATHS += $(STACK_PATH)/ZbPro/NWK/include
INC_PATHS += $(STACK_PATH)/ZbPro/APS/include
INC_PATHS += $(STACK_PATH)/ZbPro/SSP/include
INC_PATHS += $(STACK_PATH)/ZbPro/TC/include
INC_PATHS += $(STACK_PATH)/ZbPro/ZDO/include
INC_PATHS += $(STACK_PATH)/ZbPro/ZCL/include
INC_PATHS += $(BSEAV)/linux/driver/zigbee
INC_PATHS += $(STACK_PATH)/ZbPro/Profiles/ZHA/include

ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Mailbox/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/RPC/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/System/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/HAL/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/HAL/i386-utest/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/RF4CE/Profiles/MSO/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Security/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/IEEE/MAC/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/IEEE/PHY/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/RF4CE/NWK/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/RF4CE/Profiles/ProfileManager/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/RF4CE/Profiles/GDP/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/RF4CE/Profiles/ZRC/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/NWK/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/APS/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/SSP/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/TC/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/ZDO/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/ZCL/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/ZbPro/Profiles/ZHA/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/NVM/include

ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/rpc/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/common/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/rf4ce_registration/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/ha_registration/include

PRJ_FLAGS += _HOST_           # Build for Broadcom STB host.
PRJ_FLAGS += __i386__ \
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

PRJ_FLAGS += RF4CE_SUPPORT=1
PRJ_FLAGS += CLIENT
PRJ_FLAGS += MAILBOX_UNIT_TEST
#PRJ_FLAGS += _DEBUG_
#PRJ_FLAGS += _DEBUG_COMPLEX_
#PRJ_FLAGS += _DEBUG_LOG_
PRJ_FLAGS += _RF4CE_
PRJ_FLAGS += RF4CE_TARGET
PRJ_FLAGS += _ZBPRO_
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
#PRJ_FLAGS += _DEBUG_CONSOLELOG_=2
#PRJ_FLAGS += _DEBUG_FILELINE_
PRJ_FLAGS += _MAILBOX_WRAPPERS_TEST_ENGINE_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_MAC_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_NWK_=1
PRJ_FLAGS += _MAILBOX_WRAPPERS_PROFILE_=2
PRJ_FLAGS += _MAILBOX_INTERFACE_=1
PRJ_FLAGS += _MAC_BAN_TABLE_SIZE_=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC1
PRJ_FLAGS += USE_RF4CE_PROFILE_GDP=1
PRJ_FLAGS += USE_RF4CE_PROFILE_ZRC2

CFLAGS += $(addprefix -D, $(PRJ_FLAGS))
CFLAGS += $(addprefix -I, $(INC_PATHS))

VPATH += $(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/System/src
VPATH += $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/rpc/src
VPATH += $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/common/src

LDFLAGS += -std=gnu99 -Xlinker --no-enum-size-warning

APP_OBJECTS += \
    bbMailClientTable.o \
    bbMailServerTable.o \
    zigbee_rpc.o \
    zigbee_api.o \
    zigbee_rpc_client.o \
    zigbee_socket_client.o \
    bbSysPayload.o

RF4CE_STACK_C_FLAGS := -std=gnu99 -fpack-struct=4 -fshort-enums

$(ATLAS_ODIR)/bbMailClientTable.o:$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Mailbox/src/bbMailClientTable.c
#	@cp $(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Mailbox/src/bbMailClientTable.c ./
	@echo [Compile... bbMailClientTable.o]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
$(ATLAS_ODIR)/bbMailServerTable.o:$(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Mailbox/src/bbMailServerTable.c
	@echo [Compile... bbMailServerTable.o]
	@sed  's/SERVER_TABLE_ENTRY(\(.*\), *\(.*\), *\(.*\), *\(.*\), *\(.*\), */SERVER_TABLE_ENTRY(\1, \2, \3, \4, 0),/' $(BSEAV)/lib/zigbee/broadbee_mailbox_host/stack/common/Mailbox/src/bbMailServerTable.c > ./bbMailServerTable.c
	$(Q_)$(CC) -c -o $@ ./bbMailServerTable.c $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
	@sed 's|bbMailServerTable\.c|$(BSEAV)\/lib\/zigbee\/broadbee_mailbox_host\/stack\/common\/Mailbox\/src\/bbMailServerTable\.c|' $(ATLAS_ODIR)/bbMailServerTable.d > $(ATLAS_ODIR)/bbMailServerTable.d.tmp
	@mv $(ATLAS_ODIR)/bbMailServerTable.d.tmp $(ATLAS_ODIR)/bbMailServerTable.d
	@rm -rf ./bbMailServerTable.c
$(ATLAS_ODIR)/zigbee_api.o:zigbee_api.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
$(ATLAS_ODIR)/zigbee_rpc.o:zigbee_rpc.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
$(ATLAS_ODIR)/zigbee_rpc_client.o:zigbee_rpc_client.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
$(ATLAS_ODIR)/zigbee_socket_client.o:zigbee_socket_client.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
$(ATLAS_ODIR)/bbSysPayload.o:bbSysPayload.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
#$(ATLAS_ODIR)/bbSysDbg.o:bbSysDbg.c
#	@echo [Compile... $(notdir $<)]
#	$(Q_)$(CC) -c -o $@ $< $(filter-out -pedantic,$(CFLAGS)) $(RF4CE_STACK_C_FLAGS)
