############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

NEXUS_TOP ?= nexus

# Locally define rules -- all is the default rule
all: install

# Include nexus definitions
include $(NEXUS_TOP)/build/nexus_defs.inc
NEXUS_PLATFORM_BUILDDIR=$(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc

####################################################

ifeq ($(NEXUS_P_IS_MULTIPASS),)
ifeq ($(B_REFSW_OS),linuxuser)
ifeq ($(NEXUS_MODE),proxy)
# kernel mode is a two pass build:
# OS=linuxuser MODE=proxy
# OS=linuxkernel MODE=driver
NEXUS_P_START_MULTIPASS=y
NEXUS_P_FIRST_PASS_NAME="driver"
NEXUS_P_FIRST_PASS_PARAMS=B_REFSW_OS=linuxkernel NEXUS_MODE=driver
NEXUS_P_SECOND_PASS_NAME="proxy"
NEXUS_P_SECOND_PASS_PARAMS=
endif
ifeq ($(NEXUS_MODE),)
ifneq ($(NEXUS_SERVER_SUPPORT),n)
NEXUS_MODE := client
endif
endif
ifeq ($(NEXUS_MODE),client)
ifeq ($(NEXUS_WEBCPU),)
# usermode multiprocess is a two pass build:
# OS=linuxuser NEXUS_MODE=client
# OS=linuxuser NEXUS_MODE=
NEXUS_P_START_MULTIPASS=y
NEXUS_P_FIRST_PASS_NAME="server"
NEXUS_P_FIRST_PASS_PARAMS=NEXUS_MODE=
NEXUS_P_SECOND_PASS_NAME="client"
NEXUS_P_SECOND_PASS_PARAMS=NEXUS_MODE=client
endif
endif
endif
endif

# Include platform modules
# allow outside override
ifeq ($(PLATFORM_MODULES_INC),)
PLATFORM_MODULES_INC=$(NEXUS_TOP)/platforms/common/build/platform_modules.inc
endif
NEXUS_INCLUDE_MODULE_RULES:=1
include $(PLATFORM_MODULES_INC)


####################################################
# List the nexus and magnum modules which are compiled into the proxy/client.
# This list is determined by the nexus architecture and does not vary per chip.
# It's easier to maintain a small inclusion list than a large exclusion list.

# NEXUS_CLIENT_MODULES is the list of nexus modules compiled into the client/proxy library.
# It is used nexus/build/os/linuxuser/module_rules.pl, et al.
NEXUS_CLIENT_MODULES := RECORD FILE PLAYBACK

# MAGNUM_CLIENT_MODULES is the list of magnum modules compiled into the client/proxy library.
# It is used in nexus/build/nexus.inc
MAGNUM_CLIENT_MODULES := BKNI BDBG BVLC BLST BMMA BMMA_POOL BVCE_DEBUG
ifeq ($(NEXUS_CONFIG_IMAGE),y)
	# TODO: use BXVD_IMG_ONLY and BRAP_IMG_ONLY for client/proxy-only
	MAGNUM_CLIENT_MODULES += BXVD BRAP BAPE_IMG BVCE BDSP BSID BHAB
endif
NEXUS_CLIENT_MODULES += FILE_MUX
MAGNUM_CLIENT_MODULES += BMUXLIB_FILE_ASF BMUXLIB_FILE_COMMON BMUXLIB_FILE_MP4 BMUXLIB_FILE_PES BMUXLIB_FILE_IVF

ifeq ($(NEXUS_MODE),client)
ifeq ($(NEXUS_WEBCPU),core1_server)
NEXUS_CLIENT_MODULES += GRAPHICS2D
# build the full implementations of the following magnum modules
MAGNUM_CLIENT_MODULES += BGRC BGRCLIB BFMT BREG BPXL BINT BPXL BMEM
endif
endif

# NOTE: platform_modules.inc must also exclude the modules in NEXUS_CLIENT_MODULES

####################################################

# Pull in astra for kernel builds
ifeq ($(NEXUS_MODE),driver)
ifeq ($(NEXUS_ASTRA_SUPPORT),y)
EXTRA_CLEAN += astra_drv_clean
NEXUS_DRIVER_EXTRADEPS += astra_drv_install
NEXUS_DRIVER_KBUILD_ARGS += KBUILD_EXTRA_SYMBOLS=$(realpath $(NEXUS_OBJ_DIR))/bcm_astra.symvers
endif
endif

ifeq ($(B_REFSW_OS),linuxuser)
ifneq ($(PLATFORM_CYGNUS),y)
ifeq ($(NEXUS_MODE),)
ifneq  (${B_REFSW_SYSTEM_MODE},CLIENT)
# user mode is a one pass build which also builds bcmdriver
EXTRA_CLEAN += bcmdriver_clean
EXTRA_INSTALL += bcmdriver_install
ifeq ($(NEXUS_ASTRA_SUPPORT),y)
EXTRA_CLEAN += astra_clean astra_drv_clean
EXTRA_INSTALL += astra_install astra_drv_install
NEXUS_EXTRALIBS += $(ASTRA_LIB)
endif
endif # client
endif # NEXUS_MODE
endif # cygnus
ifneq ($(NEXUS_MODE),client)
ifneq (${B_REFSW_SYSTEM_MODE},SERVER)
ifneq ($(NEXUS_LOGGER_EXTERNAL),y) # Defined by some systems such as android that have their own loggger implementation
EXTRA_CLEAN += logger_clean
EXTRA_INSTALL += logger_install
endif
endif
endif
ifeq ($(NEXUS_MODE),proxy)
ifeq ($(NEXUS_ASTRA_SUPPORT),y)
EXTRA_CLEAN += astra_clean
EXTRA_INSTALL += astra_install
NEXUS_EXTRALIBS += $(ASTRA_LIB)
endif
endif
endif



ifeq ($(SAGE_SUPPORT),y)
ifneq ($(NEXUS_MODE),client)
ifneq ($(NEXUS_MODE),proxy)
ifeq ($(shell test -e $(BSEAV)/lib/security/sage/bin/install.mak && echo y),y)
EXTRA_INSTALL += install_sage
include $(BSEAV)/lib/security/sage/bin/install.mak
endif
endif
endif
endif

ifeq ($(filter $(NEXUS_MODE),client),)
.PHONY: nexus_headers
ifeq ($(NEXUS_HEADERS),y)
# automatically run "make nexus_headers" with normal nexus build
EXTRA_ALL   += nexus_headers
EXTRA_CLEAN += clean_nexus_headers
endif

.PHONY: verify_module_api clean_verify_module_api
EXTRA_ALL   += verify_module_api
EXTRA_CLEAN += clean_verify_module_api
endif

ifneq ($(PLATFORM_CYGNUS),y)
ifneq  (${B_REFSW_SYSTEM_MODE},CLIENT)
ifeq ($(filter ${B_REFSW_ARCH},arm-linux aarch64-linux),${B_REFSW_ARCH})
EXTRA_CLEAN += wakeupdriver_clean
EXTRA_INSTALL += wakeupdriver_install
ifeq ($(RF4CE_SUPPORT),y)
EXTRA_CLEAN += zigbeedriver_clean zigbeeprocess_clean
EXTRA_INSTALL += zigbeedriver_install zigbeeprocess_install
endif
endif
endif
endif

# bcmdriver rules
.PHONY: bcmdriver
include ${NEXUS_TOP}/platforms/common/build/platform_version.inc
BCMDRIVER:=${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/usermode/$(B_REFSW_ARCH)/bcmdriver.ko
bcmdriver: ${BCMDRIVER}
${BCMDRIVER}: ${BSEAV}/linux/driver/usermode/bcm_driver.h ${BSEAV}/linux/driver/usermode/bcm_driver.c
	@echo "[Build.... bcmdriver]"
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/build B_REFSW_DEBUG=$(B_REFSW_DEBUG)  NEXUS_PLATFORM_VERSION_MAJOR=$(NEXUS_PLATFORM_VERSION_MAJOR) NEXUS_PLATFORM_VERSION_MINOR=$(NEXUS_PLATFORM_VERSION_MINOR) --no-print-directory


.PHONY: bcmdriver_clean bcmdriver_install
bcmdriver_install: bcmdriver
	@echo "[Install... bcmdriver]"
	${Q_}$(CP) ${BCMDRIVER} ${NEXUS_BIN_DIR}
bcmdriver_clean:
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/build B_REFSW_DEBUG=$(B_REFSW_DEBUG) clean --no-print-directory > /dev/null

# astra rules
.PHONY: astra astra_drv
ASTRA_LIB:=${B_REFSW_OBJ_ROOT}/astra/linux/libbcm_astra/libbcm_astra.a
ASTRA_DRV:=${B_REFSW_OBJ_ROOT}/astra/linux/bcm_astra/$(B_REFSW_ARCH)/bcm_astra.ko
ASTRA_SYMVERS:=${B_REFSW_OBJ_ROOT}/astra/linux/bcm_astra/$(B_REFSW_ARCH)/Module.symvers

astra_drv:
	@echo "[Build..... astra driver]"
	${Q_}$(MAKE) -C $(NEXUS_TOP)/platforms/common/build -f Makefile.bcm_astra --no-print-directory

astra: $(ASTRA_LIB)

$(ASTRA_LIB):
	@echo "[Build..... astra]"
	${Q_}$(MAKE) -C $(NEXUS_TOP)/platforms/common/build -f Makefile.libbcm_astra --no-print-directory

.PHONY: astra_clean astra_install astra_drv_clean astra_drv_install
astra_install: astra
	@echo "[Install... astra]"
	${Q_}$(CP) ${ASTRA_LIB} ${NEXUS_BIN_DIR}

astra_clean:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/platforms/common/build -f Makefile.libbcm_astra clean > /dev/null

astra_drv_install: astra_drv
	@echo "[Install... astra driver]"
	${Q_}$(CP) ${ASTRA_DRV} ${NEXUS_BIN_DIR}
	${Q_}$(CP) $(ASTRA_SYMVERS) ${NEXUS_OBJ_DIR}/bcm_astra.symvers

astra_drv_clean:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/platforms/common/build -f Makefile.bcm_astra clean > /dev/null

# wakeupdriver rules
.PHONY: wakeupdriver
WAKEUPDRIVER:=${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/wakeup/$(B_REFSW_ARCH)/wakeup_drv.ko
wakeupdriver: ${WAKEUPDRIVER}
${WAKEUPDRIVER}: $(BSEAV)/linux/driver/wakeup/wakeup_driver.c
	@echo "[Build.... wakeupdriver]"
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/wakeup B_REFSW_DEBUG=$(B_REFSW_DEBUG) --no-print-directory

.PHONY: wakeupdriver_clean wakeupdriver_install
wakeupdriver_install: wakeupdriver
	@echo "[Install... wakeupdriver]"
	${Q_}$(CP) ${WAKEUPDRIVER} ${NEXUS_BIN_DIR}
wakeupdriver_clean:
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/wakeup B_REFSW_DEBUG=$(B_REFSW_DEBUG) clean --no-print-directory > /dev/null

# Zigbee server process
.PHONY: zigbeeprocess
ZIGBEEPROCESS:=${B_REFSW_OBJ_ROOT}/BSEAV/lib/zigbee/$(B_REFSW_ARCH)/SoC_mailboxHost.elf
zigbeeprocess:
	@echo "[Build.... zigbeeprocess]"
	${Q_}$(MAKE) -C $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide B_REFSW_DEBUG=$(B_REFSW_DEBUG) --no-print-directory
	mkdir -p ${B_REFSW_OBJ_ROOT}/BSEAV/lib/zigbee/$(B_REFSW_ARCH)
	${Q_}$(CP) $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/obj.$(NEXUS_PLATFORM)/SoC_mailboxHost.elf ${ZIGBEEPROCESS}

.PHONY: zigbeeprocess_clean zigbeeprocess_install
zigbeeprocess_install: zigbeeprocess
	@echo "[Install... zigbeeprocess]"
	${Q_}$(CP) ${ZIGBEEPROCESS} ${NEXUS_BIN_DIR}
zigbeeprocess_clean:
	${Q_}$(MAKE) -C $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide B_REFSW_DEBUG=$(B_REFSW_DEBUG) clean --no-print-directory > /dev/null

# bcmdriver rules
.PHONY: zigbeedriver
ZIGBEEDRIVER:=${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/zigbee/$(B_REFSW_ARCH)/zigbee_drv.ko
zigbeedriver: ${ZIGBEEDRIVER}
${ZIGBEEDRIVER}: $(BSEAV)/linux/driver/zigbee/zigbee_driver.c
	@echo "[Build.... zigbeedriver]"
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/zigbee B_REFSW_DEBUG=$(B_REFSW_DEBUG) BCHP_CHIP=$(BCHP_CHIP) --no-print-directory

.PHONY: zigbeedriver_clean zigbeedriver_install
zigbeedriver_install: zigbeedriver
	@echo "[Install... zigbeedriver]"
	${Q_}$(CP) ${ZIGBEEDRIVER} ${NEXUS_BIN_DIR}
zigbeedriver_clean:
	${Q_}$(MAKE) -C $(BSEAV)/linux/driver/zigbee B_REFSW_DEBUG=$(B_REFSW_DEBUG) clean --no-print-directory > /dev/null

.PHONY: fw_img_sign_clean fw_signature_clean
fw_img_sign_clean:
	-${Q_}$(MAKE) -C $(BSEAV)/lib/security/fw_img_sign clean --no-print-directory > /dev/null 2>&1

ifeq ($(NEXUS_SECURITY_FW_SIGN),y)
EXTRA_CLEAN += fw_img_sign_clean
endif

fw_signature_clean:
	-${Q_}${RM} ${NEXUS_SECURITY_REGVER_SIGNATURES} ${NEXUS_SECURITY_REGVER_KEY}
EXTRA_CLEAN += fw_signature_clean

# generic install and clean rules
ifeq ($(NEXUS_P_START_MULTIPASS),y)
install:
	@echo "[Build..... $(NEXUS_P_FIRST_PASS_NAME)]"
	${Q_}$(MAKE) -f $(NEXUS_PLATFORM_BUILDDIR)/Makefile NEXUS_P_IS_MULTIPASS=y $(NEXUS_P_FIRST_PASS_PARAMS)
	@echo "[Build..... $(NEXUS_P_SECOND_PASS_NAME)]"
	${Q_}$(MAKE) -f $(NEXUS_PLATFORM_BUILDDIR)/Makefile NEXUS_P_IS_MULTIPASS=y $(NEXUS_P_SECOND_PASS_PARAMS)
clean:
	@echo "[Clean $(NEXUS_P_SECOND_PASS_NAME)]"
	${Q_}$(MAKE) -f $(NEXUS_PLATFORM_BUILDDIR)/Makefile NEXUS_P_IS_MULTIPASS=y $(NEXUS_P_SECOND_PASS_PARAMS) clean
	@echo "[Clean $(NEXUS_P_FIRST_PASS_NAME)]"
	${Q_}$(MAKE) -f $(NEXUS_PLATFORM_BUILDDIR)/Makefile NEXUS_P_IS_MULTIPASS=y $(NEXUS_P_FIRST_PASS_PARAMS) clean
else
install: nexus_install $(EXTRA_INSTALL)
clean: nexus_clean $(EXTRA_CLEAN)
endif

nexus_install $(EXTRA_INSTALL): prep_dir

.PHONY: prep_dir
prep_dir:
	${Q_}${MKDIR} ${NEXUS_BIN_DIR}

# Include Nexus build last -- This defines nexus_install, nexus_clean, etc.
include $(NEXUS_TOP)/build/nexus.inc

# security region verification
ifeq ($(NEXUS_SECURITY_FW_SIGN),y)

# this dependency ensures that the signature file is generated before trying to compile the security module
$(NEXUS_SECURITY_SOURCES): $(NEXUS_SECURITY_REGVER_SIGNATURES)

NEXUS_FW_SIGN_SRC = $(foreach module, $(NEXUS_FW_SIGN_MODULES), $($(module)_AUTH_SOURCES))
NEXUS_FW_SIGN_FAIL_MESSAGE = "Warning: Firmware region verification signing process failed. Using stub..."

${NEXUS_SECURITY_REGVER_SIGNATURES}: $(NEXUS_FW_SIGN_SRC)
	@echo "[Build..... fw_img_sign]"
	${Q_}$(MAKE) -C $(BSEAV)/lib/security/fw_img_sign sign --no-print-directory MAKEFLAGS= B_REFSW_OBJ_ROOT=$(abspath $(B_REFSW_OBJ_ROOT)) > /dev/null 2>&1 || (echo $(NEXUS_FW_SIGN_FAIL_MESSAGE); cp $(NEXUS_SECURITY_REGVER_SIGNATURES_STUB) $(NEXUS_SECURITY_REGVER_SIGNATURES); cp $(NEXUS_SECURITY_REGVER_KEY_STUB) $(NEXUS_SECURITY_REGVER_KEY))

${NEXUS_SECURITY_REGVER_KEY}: ${NEXUS_SECURITY_REGVER_SIGNATURES}
endif

$(NEXUS_GPIO_TABLE):
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/nexus_gpio_table_prologue.c >$@
	${Q_}$(PERL) $(NEXUS_TOP)/modules/gpio/generate_nexus_gpio_mapping.pl $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/${BCHP_VER_LOWER}/bchp_aon_pin_ctrl.h >>$@
	${Q_}$(PERL) $(NEXUS_TOP)/modules/gpio/generate_nexus_gpio_mapping.pl $(NEXUS_TOP)/../magnum/basemodules/chp/include/${BCHP_CHIP}/rdb/${BCHP_VER_LOWER}/bchp_sun_top_ctrl.h >>$@
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/nexus_gpio_table_epilogue.c >>$@
	${Q_}cat $(NEXUS_TOP)/modules/gpio/src/${BCHP_CHIP}/nexus_gpio_table_epilogue_${BCHP_CHIP}.c >>$@

.PHONY: logger_clean logger_install
logger_clean:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/utils/logger clean --no-print-directory

logger_install:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/utils/logger install --no-print-directory

nexus_headers: ${NEXUS_BIN_DIR}/exists
	${Q_}echo Copying header files and platform_app.inc to ${NEXUS_BIN_DIR}/include
	${Q_}${MAKE} -C $(NEXUS_TOP)/platforms/common/build -f Makefile.app NEXUS_TOP=$(abspath ${NEXUS_TOP}) NEXUS_PLATFORM=${NEXUS_PLATFORM} NEXUS_BIN_DIR=$(abspath ${NEXUS_BIN_DIR})

clean_nexus_headers:
	${Q_}$(RM) -rf ${NEXUS_BIN_DIR}/include

ifneq ($(MAKECMDGOALS),clean)
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
verify_module_api clean_verify_module_api:
	${Q_}$(MAKE) -f $(NEXUS_TOP)/build/Makefile.verify $@ B_REFSW_OBJ_ROOT=${B_REFSW_OBJ_ROOT} NEXUS_TOP=$(NEXUS_TOP) NEXUS_PLATFORM=$(NEXUS_PLATFORM)
endif
