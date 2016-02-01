############################################################################
#    (c)2006-2013 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: Makefile $
# $brcm_Revision: 4 $
# $brcm_Date: 12/9/11 4:44p $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: /AppLibs/broadcom/netapp/bluetooth/Makefile $
#
# 4   12/9/11 4:44p steven
# SW7231-513: USB Hotplug Support in NetApp
#
# SW7231-513/1   12/9/11 3:36p steven
# SW7231-513: Finished working hotplug detection
#
# 3   12/7/11 2:08p steven
# SW7231-514: Improvements to NetApp build to build standalone
#
# 2   12/1/11 8:56p steven
# SW7425-1404: Moved kernel modules and fixes to BT support
#
# 1   11/24/11 5:13p steven
# SW7425-1404: Bluetooh, WLAN Driver, and Multi-Wired Interface Support
#
# SW7425-1404/4   11/24/11 4:02p steven
# SW7425-1404: Bluetooth, WLAN driver build, and multi-interface support
#
# SW7425-1404/2   11/4/11 10:14a steven
# SW7425-1404: Backup of BT work
#
############################################################################
#
# DESCRIPTION:
#   Makefile for building non-NetApp Bluetooth code
#
############################################################################
NETAPP_TOP = ${shell pwd}/..
include ${NETAPP_TOP}/build/build.inc
include ${NETAPP_TOP}/build/modules.inc

ifneq (,$(findstring mips,$(CC)))
CPU:=mips
else ifneq (,$(findstring arm,$(CC)))
CPU:=arm
else
CPU:=x86
endif

BSA_USER_DIR		= ${BT_CHIPSET_PKG}/3rdparty/embedded/bsa_examples/linux
BSA_KERNEL_DIR		= ${BT_CHIPSET_PKG}/3rdparty/embedded/brcm/linux
BSA_SERVER			= bsa_server
BSA_SERVER_DIR		= ${BSA_USER_DIR}/server
LIBBSA_DIR			= ${BSA_USER_DIR}/libbsa
LIBBSA				= $(LIBBSA_DIR)/build/$(CPU)/sharedlib/libbsa.so
BSA_USB_DIR			= ${BSA_KERNEL_DIR}/btusb
BSA_BTDL_DIR		= ${BSA_KERNEL_DIR}/btdl/build
BSA_BTDL_INSTALLED	= ${NETAPP_OUTPUT_BIN_DIR}/btdl
BSA_USB_MOD			= btusb.ko
BSA_SERVER_INSTALLED= ${NETAPP_OUTPUT_BIN_DIR}/${BSA_SERVER}
LIBBSA_INSTALLED	= ${NETAPP_OUTPUT_LIB_DIR}/${LIBBSA}
USB_MOD_INSTALLED	= ${NETAPP_OUTPUT_MODULES_DIR}/${BSA_USB_MOD}
BSA_EXAMPLES		= app_manager app_hh app_avk app_av app_tm simple_app app_ble
#BSA_EXAMPLES		:= $(addsuffix /build, $(addprefix ${BSA_USER_DIR}/, ${BSA_EXAMPLES}))

### TGK: It would have been very nice (OK, just consistent! :-)) to avoid MIPSxxx vars...
BT_VARIABLES		= AR=${AR} ARCH=${CPU} CPU=${CPU} MIPSGCC=${CC} MIPSGCCLIBPATH= BSASHAREDLIB=TRUE ENABLE_BLE_AUDIO=TRUE ENABLE_BTHID=TRUE
### TGK: ... but it's easy to prech, the reallity is sightly different - adding ARMGCC var
###          (OK, temp. - both shall be removed soon for good?!)
BT_VARIABLES += ARMGCC=$(CC)

txpower:
	${Q_}install txpower.txt ${NETAPP_OUTPUT_BIN_DIR}

${BT_CHIPSET_PKG}:
	@echo "============ Extracting Bluetooth Tarball"
	${Q_}cd ${NETAPPPLATFORM_OBJ_DIR}; tar xf ${BLUETOOTH_SRC}
	${Q_}if [ -f ${BT_DIR}/bsa.patch ]; then	\
		cd 	${BT_CHIPSET_PKG}; patch -p1 < ${BT_DIR}/bsa.patch ${SQUASH_OUTPUT}; \
	fi


${BSA_SERVER_INSTALLED}: ${BT_CHIPSET_PKG}
	@echo "============ Building BSA Server"
	${Q_}$(MAKE) -C ${BSA_SERVER_DIR}/build ${BT_VARIABLES} all ${SQUASH_OUTPUT}
	${Q_}install ${BSA_SERVER_DIR}/build/${CPU}/${BSA_SERVER} ${NETAPP_OUTPUT_BIN_DIR}

${LIBBSA_INSTALLED}: ${BT_CHIPSET_PKG}
	@echo "============ Building LIB BSA (Client API)"
	${Q_}$(MAKE) -C ${LIBBSA_DIR}/build ${BT_VARIABLES} all ${SQUASH_OUTPUT}
	${Q_}install ${LIBBSA} ${NETAPP_OUTPUT_LIB_DIR}

${BSA_EXAMPLES}: install
	@echo "================ Building BSA example - $@"
	${Q_}$(MAKE) $(MAKE_J_ARG) -C $(addsuffix /build, $(addprefix ${BSA_USER_DIR}/, $@)) ${BT_VARIABLES} all ${SQUASH_OUTPUT}
	${Q_}cp $(addsuffix /build, $(addprefix ${BSA_USER_DIR}/, $@))/${CPU}/$@ ${NETAPP_OUTPUT_BIN_DIR}

ifneq ($(ANDROID_ENABLE_DHD),y)
examples: ${BSA_EXAMPLES}
else
examples: btutils patchram
endif

clean-examples:
	@echo "============ Cleaning BSA Examples"
	${Q_}if [ -d ${BSA_USER_DIR} ] ; then \
		for example in ${BSA_EXAMPLES}; do \
			echo "================ Cleaning $$example" && \
			$(MAKE) -C $(addsuffix /build, $(addprefix ${BSA_USER_DIR}/, $$example)) ${BT_VARIABLES} clean ${SQUASH_OUTPUT}; \
		done \
	fi

ifeq (${LINUX},)
linux_warning:
	@echo "******************* W A R N I N G ****************************"
	@echo "LINUX is not set so you cannot build Bluetooth HID and USB Modules"
	@echo "******************* W A R N I N G ****************************"

${USB_MOD_INSTALLED}: linux_warning

else
${USB_MOD_INSTALLED}: ${BT_CHIPSET_PKG}
	@echo "============ Building USB Kernel Module"
	${Q_}$(MAKE) $(MAKE_J_ARG) -C ${BSA_USB_DIR} KDIR=${LINUX} ${BT_VARIABLES} CROSS_COMPILE=${CROSS_COMPILE} ${SQUASH_OUTPUT}
	${Q_}install ${BSA_USB_DIR}/${BSA_USB_MOD} ${NETAPP_OUTPUT_MODULES_DIR}

$(BSA_BTDL_INSTALLED): ${BT_CHIPSET_PKG}
	@echo "============ Building btdl "
	${Q_}$(MAKE) $(MAKE_J_ARG) -C ${BSA_BTDL_DIR} ${BT_VARIABLES} CROSS_COMPILE=${CROSS_COMPILE} ${SQUASH_OUTPUT}
	${Q_}install $(BSA_BTDL_DIR)/$(CPU)/btdl ${NETAPP_OUTPUT_BIN_DIR}

btutils: ${USB_MOD_INSTALLED} $(BSA_BTDL_INSTALLED)
endif

release-clean: clean
	${Q_}rm -rf ${BT_CHIPSET_PKG}

install: ${BT_CHIPSET_PKG} ${BSA_SERVER_INSTALLED} ${LIBBSA_INSTALLED} btutils patchram txpower
	${Q_}cd ${BT_CHIPSET}; \
	for directory in $$(find -type d); do \
		mkdir -p  "${NETAPP_OUTPUT_INC_DIR}/$$directory" ; \
	done
	${Q_}cd ${BT_CHIPSET}; \
	for file in $$(find -name *.h); do \
		cp -fp "$$file" "${NETAPP_OUTPUT_INC_DIR}/$$file"; \
	done

patchram:
	$(Q_)if [ -e patchram ] ; then \
		$(MAKE) -C patchram install;\
	fi

clean-examples:

clean-usb-module:
ifneq ($(wildcard ${BT_CHIPSET_PKG}), )
	@echo "============ Cleaning USB Kernel Module"
	-${Q_}$(MAKE) -C ${BSA_USB_DIR} clean ${SQUASH_OUTPUT}
endif

clean-bsa-client:
ifneq ($(wildcard ${BT_CHIPSET_PKG}), )
	@echo "============ Cleaning LIB BSA (Client API)"
	-${Q_}$(MAKE) -C ${LIBBSA_DIR}/build ${BT_VARIABLES} clean ${SQUASH_OUTPUT}
endif

clean-bsa-server:
ifneq ($(wildcard ${BT_CHIPSET_PKG}), )
	@echo "============ Cleaning BSA Server"
	-${Q_}$(MAKE) -C ${BSA_SERVER_DIR}/build ${BT_VARIABLES} clean ${SQUASH_OUTPUT}
endif

clean: clean-bsa-client clean-bsa-server clean-usb-module
	${Q_}-rm -rf ${BSA_SERVER_INSTALLED} ${LIBBSA_INSTALLED} ${USB_MOD_INSTALLED}

distclean: release-clean
	@echo "============ Distclean BSA Server"
	${Q_}rm -rf ${NETAPP_OUTPUT_INC_DIR}/${BT_PKG_VERSION} ${BSA_SERVER_INSTALLED} ${LIBBSA_INSTALLED} ${USB_MOD_INSTALLED} $(BSA_BTDL_INSTALLED) ${HAS_BLUETOOTH_BUILT}

############################################################################
#                             DEP FILES, PHONY
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
-include *.d
.PHONY: all unpack build distclean clean install btutils patchram txpower
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##
