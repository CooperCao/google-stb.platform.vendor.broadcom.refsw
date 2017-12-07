############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
# Module Description:
#   This file defines the rules to build the Nexus platform
#
############################################################

LINUX_OUT ?= $(LINUX)

ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux mipsel-uclibc mipsel-linux-uclibc mips-linux mips-uclibc mips-linux-uclibc mips-linux-gnu mips-linux-android mipsel-linux-android), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=mips
endif
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm
endif
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm64
endif

nexus_install: nexus_all ${NEXUS_BIN_DIR}/exists
	@echo [Install... nexus driver to $(realpath ${NEXUS_BIN_DIR})]
ifneq ($(NEXUS_DRIVER),)
	@$(CP) $(NEXUS_DRIVER) ${NEXUS_BIN_DIR}
else
ifdef NEXUS_STATIC_LIB
	@$(CP) $(NEXUS_STATIC_LIB) ${NEXUS_BIN_DIR}
endif
endif

ifneq ($(NEXUS_DRIVER),)
nexus_all: ${LINUX_INC} $(NEXUS_DRIVER) ${NEXUS_STATIC_LIB}
else
nexus_all: ${LINUX_INC} ${NEXUS_STATIC_LIB}
endif

${NEXUS_MAGNUM_PRECOMPILED_HEADER_H}.gch: ${NEXUS_MAGNUM_PRECOMPILED_HEADER} ${NEXUS_OBJ_DIR}/MAGNUM/exists
	@echo "[Compile... $(notdir $<)]"
	$(Q_)$(CC) $(CDEP_FLAG) $(NEXUS_CFLAGS) -c $< -o $@

${NEXUS_PRECOMPILED_HEADER_H}.gch: ${NEXUS_PRECOMPILED_HEADER} ${NEXUS_OBJ_DIR}/MAGNUM/exists
	@echo "[Compile... $(notdir $<)]"
	$(Q_)$(CC) $(CDEP_FLAG) $(NEXUS_CFLAGS) ${NEXUS_BASE_CFLAGS} -c $< -o $@

$(NEXUS_OBJ_DIR)/MAGNUM/$(patsubst %.c,%.$(NEXUS_OBJ_SUFFIX),${MAGNUM_NO_STRICT_ALIASING_SRCS}): NEXUS_CFLAGS+=-fno-strict-aliasing
# Rule for magnum sources
$(MAGNUM_OBJECTS): $(NEXUS_OBJ_DIR)/MAGNUM/%.$(NEXUS_OBJ_SUFFIX): %.c $(NEXUS_OBJ_DIR)/MAGNUM/exists $(if ${NEXUS_P_WITH_PRECOMPILED_HEADERS},${NEXUS_MAGNUM_PRECOMPILED_HEADER_H}.gch)
	@echo "[Compile... $(notdir $<) (magnum)]"
	${Q_}$(CC) $(CDEP_FLAG) $(if ${NEXUS_P_WITH_PRECOMPILED_HEADERS},-include ${NEXUS_MAGNUM_PRECOMPILED_HEADER_H}) $(NEXUS_CFLAGS) -c $< -o $@

MAGNUM_OS_ABSTRACTION_OBJECTS = $(addprefix $(NEXUS_OBJ_DIR)/,$(patsubst %.c,%.$(NEXUS_OBJ_SUFFIX),$(MAGNUM_OS_ABSTRACTION)))
$(MAGNUM_OS_ABSTRACTION_OBJECTS): $(NEXUS_OBJ_DIR)/MAGNUM/%.$(NEXUS_OBJ_SUFFIX): %.c $(NEXUS_OBJ_DIR)/MAGNUM/exists
	@echo "[Compile... $(notdir $<) (magnum os)]"
	${Q_}$(CC) $(CDEP_FLAG) $(NEXUS_CFLAGS) $(NEXUS_KERNEL_CFLAGS) -c $< -o $@

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux aarch64-linux ), ${B_REFSW_ARCH})
NEXUS_ASM_OBJECTS := $(NEXUS_OBJ_DIR)/nexus_driver_arm_eabi.o
ifeq (${B_REFSW_ARCH}, arm-linux)
NEXUS_ASM_OBJECTS += $(NEXUS_OBJ_DIR)/bcmdriver_arm_v7.o
else
NEXUS_ASM_OBJECTS += $(NEXUS_OBJ_DIR)/bcmdriver_arm_v8.o
endif
vpath %.S $(BSEAV)/linux/driver/usermode ${NEXUS_TOP}/platforms/common/src/linuxkernel
${NEXUS_ASM_OBJECTS} : $(NEXUS_OBJ_DIR)/%.o : %.S
	@echo "[Compile... $(notdir $<)]"
	${Q_}$(CC) $(NEXUS_CFLAGS) $(NEXUS_KERNEL_CFLAGS) -D__ASSEMBLY__  -o $@ -c $<

endif

ifdef NEXUS_STATIC_LIB
$(NEXUS_STATIC_LIB): $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS) ${NEXUS_ASM_OBJECTS} $(MAGNUM_OS_ABSTRACTION_OBJECTS)
	@echo "[Archive... $(notdir $@)]"
	$(Q_)cd $(NEXUS_OBJ_DIR);$(AR) rcs $(abspath $@) $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%,%,$(abspath $^))
endif

# Rule for nexus driver
$(NEXUS_OBJ_DIR)/nexus.o_shipped: $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS) $(MAGNUM_OS_ABSTRACTION_OBJECTS) $(NEXUS_EXTRALIBS) $(NEXUS_ASM_OBJECTS)
	@echo [Link...... nexus.ko]
	${Q_}rm -f $@.ld && for f in $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%,%,$(abspath $^)) ; do echo $(abspath ${NEXUS_OBJ_DIR})/$$f >>$@.ld; done
	${Q_}cd $(LINUX);$(LD) -Map $(abspath ${NEXUS_DRIVER}.map) ${NEXUS_LDFLAGS} $(NEXUS_EXTRA_LDFLAGS) --strip-debug -r @$(abspath $@.ld) -o $(abspath $@)

$(NEXUS_DRIVER): $(NEXUS_OBJ_DIR)/nexus.o_shipped $(NEXUS_DRIVER_EXTRADEPS)
ifeq ($(LINUX_VER_GE_2_6_31),y)
	${Q_}# build a dynamic Kbuild file
	@echo "obj-m += nexus.o" >$(NEXUS_OBJ_DIR)/Kbuild
	@echo "nexus-y += nexus.o_shipped" >>$(NEXUS_OBJ_DIR)/Kbuild
ifneq ($(NEXUS_DRIVER_KBUILD_ARGS),)
	${Q_}cat $(NEXUS_DRIVER_KBUILD_ARGS) > $(NEXUS_OBJ_DIR)/Module.symvers
endif
	$(MAKE) -C $(LINUX_OUT) M=$(realpath $(NEXUS_OBJ_DIR)) ARCH=$(B_REFSW_LINUX_ARCH) modules PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)
else
	${@_}# do not use kbuild for 2.6.18
	@$(CP) $^ $@
endif

nexus_clean: ${OTHER_CLEANS}
	-$(Q_)$(RM) -r $(NEXUS_DRIVER) ${NEXUS_STATIC_LIB} $(NEXUS_OBJ_DIR) $(NEXUS_SYNCTHUNK_DIR) $(NEXUS_BUILD_DIR)/module_vars_*.inc $(NEXUS_BUILD_DIR)/module_rules_*.inc
ifdef NEXUS_STATIC_LIB
	-$(Q_)$(RM) ${NEXUS_BIN_DIR}/$(notdir ${NEXUS_STATIC_LIB})
endif

# Include dependencies
NEXUS_DEP_FILES := $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
ifneq ($(NEXUS_DEP_FILES),)
-include $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
endif

ifeq ($(filter $(B_REFSW_LINUX_ARCH), arm64 arm ), $(B_REFSW_LINUX_ARCH))
####################################
# get ARM Linux CFLAGS/LDFLAGS from Kbuild

# Linux requires absolute path
KBUILD_OBJ_DIR := ${shell $(MKDIR) $(NEXUS_OBJ_DIR); cd $(NEXUS_OBJ_DIR); pwd}

$(KBUILD_OBJ_DIR)/Kbuild_flags : $(BSEAV)/linux/driver/usermode/Kbuild $(KBUILD_OBJ_DIR)/exists
	$(Q_)$(CP) $(BSEAV)/linux/driver/usermode/Kbuild $(KBUILD_OBJ_DIR)
	${Q_}$(MAKE) -C $(LINUX_OUT) M=$(KBUILD_OBJ_DIR) modules ARCH=$(B_REFSW_LINUX_ARCH) PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE) BCM_PRINT_KBUILD_ENV_ONLY=1 2>$@ >/dev/null

-include $(KBUILD_OBJ_DIR)/Kbuild_flags
endif
