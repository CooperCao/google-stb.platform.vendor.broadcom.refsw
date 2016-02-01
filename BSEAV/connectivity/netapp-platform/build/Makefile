############################################################################
#    (c)2015 Broadcom Corporation
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
# $brcm_Revision: 14 $
# $brcm_Date: 2/3/12 4:18p $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: /NetApp/build/Makefile $
#
# 14   2/3/12 4:18p steven
# SW7231-638: NetApp Version 6.2
#
# 10   1/25/12 9:31a steven
# SW7346-638: Auto-Generated NetApp Release Candidate
#
# 10   1/25/12 9:31a steven
# SW7346-638: Auto-Generated NetApp Release Candidate
#
############################################################################
#
# DESCRIPTION: Makefile for building netapp-platform
#
############################################################################
NETAPP_TOP 		= ${shell pwd}/..
NETAPP_LIBS_DIR = ${NETAPP_TOP}/libs
CURRENT_DATE	= $(shell date +%Y%m%d)
include ${NETAPP_TOP}/build/build.inc
include ${NETAPP_TOP}/build/modules.inc

TOOLCHAIN_PATH ?= $(dir $(shell which ${CC}))


# Fetch the source internal to Broadcom, external need to get it from DocSafe
INTERNAL_PKG_SERVER		:= projects.irv.broadcom.com
INTERNAL_DRIVER_DIR             := http://${INTERNAL_PKG_SERVER}/applibs/netapp/platform-drivers
INTERNAL_THIRDPARTY_DIR	:= http://${INTERNAL_PKG_SERVER}/applibs/netapp/thirdparty
BLUETOOTH_SRC_PKG_SERVER:= ${INTERNAL_DRIVER_DIR}/${BT_PKG_VERSION}${BT_CHIPSET_PKG_TYPE}
NFC_SRC_PKG_SERVER		:= ${INTERNAL_DRIVER_DIR}/${NFC_PKG_VERSION}${NFC_CHIPSET_PKG_TYPE}
WIFI_SRC_PKG_SERVER		:= ${INTERNAL_DRIVER_DIR}/${WIFI_DRIVER_VERSION}.tgz
THIRDPARTY_PKG_SERVER	:= ${INTERNAL_THIRDPARTY_DIR}/${THIRDPARTY_PKG_VERSION}
SERVER_ALIVE            := $(shell nslookup -timeout=1 ${INTERNAL_PKG_SERVER} | grep Address && echo "1s" || echo "")
NETAPP_DEP_LIBS			:= $(shell cd ../libs; ls)

ifneq (${WIFI_DRIVER_VERSION_AP1},)
	WIFI_SRC_SERVER_AP1		?= ${INTERNAL_DRIVER_DIR}/${WIFI_DRIVER_VERSION_AP1}.tgz
endif
ifneq (${WIFI_DRIVER_VERSION_AP2},)
	WIFI_SRC_SERVER_AP2		?= ${INTERNAL_DRIVER_DIR}/${WIFI_DRIVER_VERSION_AP2}.tgz
endif
ifneq (${WIFI_APP_VERSION_AP},)
	WIFI_SRC_SERVER_AP	?= ${INTERNAL_DRIVER_DIR}/${WIFI_APP_VERSION_AP}.tgz
endif
# Hide symbols that we don't want exposed inside of NetApp-platform
LOCAL_CFLAGS 			+=-fvisibility=hidden -c

ifeq (${NETAPP_WIFI},n)
NETAPP_DEP_LIBS	:= $(filter-out usb, ${NETAPP_DEP_LIBS})
else
ifeq (${NETAPP_USB},n)
NETAPP_DEP_LIBS	:= $(filter-out usb, ${NETAPP_DEP_LIBS})
endif
endif


NETAPP_DEP_LIBS	:= $(filter-out common.inc, ${NETAPP_DEP_LIBS})

ifeq (${NETAPP_WIFI_DRIVER}, falcon)
LIBUSB_NEEDED = libusb
endif

CPPFLAGS				+= -fPIC
LIB_BASENAME        	:= netapp-platform
F_PUBLIC_INCS    		:= $(foreach d,../include,$(wildcard ${d}/*.h ))

LDFLAGS					+= -L${NETAPP_OUTPUT_LIB_DIR} -L.
SRC_DIRS				:= . ../src
INC_DIRS 				+= . ../src ../include


NETAPP_WLAN_BREADCRUMB	= $(NETAPPPLATFORM_OBJ_DIR)/netapp_wlan_breadcrumb
NETAPP_BT_BREADCRUMB	= $(NETAPPPLATFORM_OBJ_DIR)/netapp_bt_breadcrumb

install: download
	${Q_}$(MAKE) all
	${Q_}$(MAKE) make-include
	@echo "==== NetApp-Platform Build Completed!"


################################################################################
# 							W I F I  	S U P O R T
#Enable WiFi if the WiFi source code is available
################################################################################
ifneq (, ${HAS_WIFI})
LOCAL_CFLAGS			+= -DWIFI_DRIVER_VERSION="\"${WIFI_DRIVER_VERSION}\""
ifneq ($(findstring 43566, $(NETAPP_WIFI_CHIP)),)
LOCAL_CFLAGS			+= -DNETAPP_WIFI_CHIP=43566
endif

wlan: libs
ifneq (${HAS_WIFI_SRC},)
	$(Q_)if [ ! -f $(NETAPP_WLAN_BREADCRUMB) ]; then \
		echo "======== Building WiFi "; \
		$(MAKE) -C ../wlan examples && touch $(NETAPP_WLAN_BREADCRUMB); \
	fi
else
endif
ap: dirs download
ifeq (${NETAPP_WIFI_AP},y)
		echo "======== Building WiFi AP "; \
		$(MAKE) -C ../wlan ap
endif

# No WiFi
else
wlan:

endif # NO_WIFI
###############################################################################


################################################################################
# 					B L U E T O O T H   	S U P O R T
#Enable Bluetooth if the Bluetooth source code is available
################################################################################
ifneq (, ${HAS_BLUETOOTH})
BSA_DIR				= ${BT_CHIPSET_PKG}/3rdparty/embedded/bsa_examples/linux/libbsa/include
BTHID_DIR			= ${BT_CHIPSET_PKG}/3rdparty/embedded/brcm/linux/bthid
CFLAGS				+= -I${BSA_DIR} -I${BTHID_DIR}
BT_MODULES			+= bthid btusb
LOCAL_CFLAGS 		+= -DBT_MODULES="\"${BT_MODULES}\""
CFLAGS				+= -DNO_BT_HEADLESS_MODE -DBLE_INCLUDED=TRUE -DBTA_GATT_INCLUDED=TRUE

# enable auto pair by default for now, will remove with proximity or legacy pairing later
#CPPFLAGS			+= -DBT_HID_AUTO_PAIR

bluetooth: libs
ifneq (, ${HAS_BLUETOOTH_SRC})
	$(Q_)if [ ! -f $(NETAPP_BT_BREADCRUMB) ]; then \
		echo "======== Building Bluetooth "; \
		$(MAKE) -C ../bluetooth examples && touch $(NETAPP_BT_BREADCRUMB); \
	fi
endif

# No Bluetooth
else
EXCLUDED_SRCS	+= 	\
	netapp_bt.c		\

CFLAGS			+= -DNO_BLUETOOTH
CFLAGS			+= -DNO_BT_HEADLESS_MODE

bluetooth:

endif # NO_BLUETOOTH

###############################################################################


################################################################################

############################################################################
# These variable settings are all dependent on the above values.
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
STATIC_LINKER_STR 	= -Wl,-l
STATIC_LDFLAGS		+= $(addprefix ${STATIC_LINKER_STR}, ${NETAPP_STATIC_LIBS})

## NB: ARM build fails with --whole-archive for all, but needs it for next 3 libs
ifneq (${HAS_WIFI},)
STATIC_LDFLAGS		+= -Wl,--whole-archive
ifeq (${NETAPP_NEEDS_WPS}, y)
ifneq (${EXTERNAL_OPENSSL},1)
STATIC_LDFLAGS      += -Wl,-l${BCMCRYPTO}
endif
endif

ifneq (${NETAPP_WIFI_P2P},n)
STATIC_LDFLAGS		+= -Wl,-l$(BCMP2P) -Wl,-l$(BCMP2PSIG)
endif
ifneq (${NETAPP_WIFI_WFD},n)
STATIC_LDFLAGS		+= -Wl,-l$(WFDCAPDIE)
endif
STATIC_LDFLAGS		+= -Wl,--no-whole-archive
else
NETAPP_WIFI_WFD		=n
NETAPP_WIFI_WPS		=n
NETAPP_WIFI_P2P		=n
endif


F_PUBLIC_MAKE		= ${LIB_BASENAME}.inc

F_SRCS = $(filter-out \
           $(addprefix %/, ${EXCLUDED_SRCS}), \
               $(foreach d,${SRC_DIRS},$(wildcard ${d}/*.c ${d}/*.cpp)))

F_OBJS_COMPILED 	+= $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, ${F_SRCS}))
F_OBJS              += ${F_OBJS_PRECOMPILED} ${F_OBJS_COMPILED}
F_OBJS				:= $(addprefix $(NETAPPPLATFORM_OBJ_DIR)/,$(notdir $(F_OBJS)))
CPPFLAGS       		+= $(addprefix -I, ${INC_DIRS})
F_INSTALLED_LIBS	+= $(addprefix ${NETAPP_OUTPUT_LIB_DIR}/, $(notdir ${F_PUBLIC_LIBS}))
F_INSTALLED_INCS	+= $(addprefix ${NETAPP_OUTPUT_INC_DIR}/, $(notdir ${F_PUBLIC_INCS}))
F_INSTALLED_MAKE	= ${NETAPP_OUTPUT_INC_DIR}/${F_PUBLIC_MAKE}
CPPFLAGS       		+= -I${NETAPP_OUTPUT_INC_DIR}

bundle:
ifdef NETAPP_TARGET_TARBALL
	@echo "Creating $(NETAPP_TARGET_TARBALL)"
	@cd $(NETAPP_STAGE_DIR); tar -cz --exclude=include --exclude=doc --exclude=man --exclude=*.a -f ../${NETAPP_TARGET_TARBALL}  usr/
	@echo "*******************************************************************************"
	@echo "* Minimal Bundle created at $(NETAPP_TARGET_TARBALL)"
	@echo "*******************************************************************************"
endif

help: download
	${Q_}$(MAKE) help-menu

help-menu:
	@echo ""
	@echo "************************************************************************"
	@echo "                         NetApp-Platform"
	@echo "************************************************************************"
	@echo "Options:"
ifneq (${NETAPP_WIFI},n)
	@echo "    WiFi Support:            ENABLED  (NETAPP_WIFI=y)"
ifneq (${NETAPP_WIFI_P2P},n)
	@echo "        WiFi Direct (P2P):   ENABLED  (NETAPP_WIFI_P2P=y)"
else
	@echo "        WiFi Direct (P2P):   DISABLED (NETAPP_WIFI_P2P=n)"
endif
ifneq (${NETAPP_WIFI_WPS},n)
	@echo "        WiFi WPS:            ENABLED  (NETAPP_WIFI_WPS=y)"
else
	@echo "        WiFi (WPS):          DISABLED (NETAPP_WIFI_WPS=n)"
endif
else
	@echo "    WiFi Support:            DISABLED (NETAPP_WIFI=$(NETAPP_WIFI))"
endif
ifneq (${NETAPP_BLUETOOTH},n)
	@echo "    Bluetooth Support:       ENABLED  (NETAPP_BLUETOOTH=y)"
else
	@echo "    Bluetooth Support:       DISABLED (NETAPP_BLUETOOTH=n)"
endif
	@echo ""
	@echo "Info:"
ifndef LINUX
	@echo "ERR:LINUX:               NOT SET!"
	@exit -1
else
ifeq (,${KCONFIG_FILE})
	@echo "LINUX=${LINUX} KCONFIG_FILE=${KCONFIG_FILE} ERR:LINUX: Your Kernel has not been built or ${LINUX}/.config is not found"
	@exit -1
else
	@echo "    LINUX:                 ${LINUX}"
endif
endif
ifdef NEXUS_PLATFORM
	@echo "    NEXUS_PLATFORM:        ${NEXUS_PLATFORM}"
endif
	@echo "    NETAPP_OUTPUT_DIR:     ${NETAPP_OUTPUT_DIR}"
ifdef NETAPP_DISABLE_WIRED_SUPPORT
	@echo "    DISABLE_WIRED_SUPPORT: ${NETAPP_DISABLE_WIRED_SUPPORT}"
endif
ifeq (${EXTERNAL_OPENSSL},1)
	@echo "    Crypto Library:        External"
else
	@echo "    Crypto Library:        Internal"
endif
	@echo "    Kernel Version:        ${KERNELVERSION}"
	@echo "    Toolchain:             ${TOOLCHAIN_PATH}"
ifneq (${NETAPP_WIFI},n)
ifeq (${WIFI_MFG_SRC_PKG},)
	@if ! [ -e ${WIFI_SRC_PKG} ] ; then \
		if ! [ -e ${NETAPP_PREBUILT_TARBALL} ] ; then \
			echo "    *************** W A R N I N G ************************" && \
			echo "    ${WIFI_DRIVER_VERSION} NOT FOUND!!!!" && \
			echo "    Please place source in netapp/wlan/broadcom or binaries tarball in /prebuilt." && \
			echo "    *************** W A R N I N G ************************";\
		fi;\
	else \
		echo "    NETAPP_WIFI_DRIVER:    ${NETAPP_WIFI_DRIVER}";\
		echo "    WiFi Version:          ${WIFI_DRIVER_VERSION}";\
	fi
else
	@echo "    NETAPP_WIFI_DRIVER:    ${NETAPP_WIFI_DRIVER}"
	@echo "    WiFi Version:          ${WIFI_DRIVER_VERSION}_mfg"
endif
ifneq (${NETAPP_WIFI_CHIP},)
	@echo "    NETAPP_WIFI_CHIP:      $(NETAPP_WIFI_CHIP)"
else
	@echo "    Supported WiFi Chips (USB and/or PCIe):"
	@echo "      dhd:     ${NETAPP_DHD_WIFI_CHIPS}"
	@echo "      eagle:   ${NETAPP_EAGLE_WIFI_CHIPS}"
	@echo "      bison:   ${NETAPP_BISON_WIFI_CHIPS}"
	@echo "      aardvark:${NETAPP_AARDVARK_WIFI_CHIPS}"
	@echo "      falcon:  ${NETAPP_FALCON_WIFI_CHIPS} (DEPRECIATED)"
	@echo "      (set explicitly driver with NETAPP_WIFI_DRIVER=<dhd|bison|eagle|falcon|aardvark>)"
endif
ifneq ($(wildcard ${NETAPP_WIFI_NVRAM_FILE}),)
	@echo "    NETAPP_WIFI_NVRAM_FILE:${NETAPP_WIFI_NVRAM_FILE}"
endif
ifneq ($(wildcard ${NETAPP_WIFI_NVRAM_C_FILE}),)
	@echo "    NETAPP_WIFI_NVRAM_C_FILE:${NETAPP_WIFI_NVRAM_C_FILE}"
endif
endif
ifneq (${NETAPP_BLUETOOTH},n)
	@if ! [ -e ${BLUETOOTH_SRC} ] ; then \
		if ! [ -e ${NETAPP_PREBUILT_TARBALL} ] ; then \
			echo "    *************** W A R N I N G ************************" && \
			echo "    ${BT_PKG_VERSION} NOT FOUND!" && \
			echo "    Please place source in netapp/bluetooth/broadcom or binaries tarball in /prebuilt." && \
			echo "    *************** W A R N I N G ************************"; \
		fi;\
	else \
		echo "    BT_PKG_VERSION:        ${BT_PKG_VERSION}";\
	fi
endif
ifneq (${NETAPP_NFC},n)
	@if ! [ -e ${NFC_SRC} ] ; then \
		if ! [ -e ${NETAPP_PREBUILT_TARBALL} ] ; then \
			echo "    *************** W A R N I N G ************************" && \
			echo "    ${NFC_PKG_VERSION} NOT FOUND!" && \
			echo "    Please place source in netapp/nfc/broadcom or binaries tarball in /prebuilt." && \
			echo "    *************** W A R N I N G ************************"; \
		fi;\
	else \
		echo "    NFC Version:           ${NFC_PKG_VERSION}";\
	fi
endif
	@echo "************************************************************************"



${WIFI_CHIPSET_DIR}:
	@mkdir -p ${WIFI_CHIPSET_DIR}

ifneq ($(WIFI_PRIVATE_DIR),$(WIFI_CHIPSET_DIR))
${WIFI_PRIVATE_DIR}:
	@mkdir -p ${WIFI_PRIVATE_DIR}
endif

ifneq ($(WIFI_PUBLIC_DIR),$(WIFI_CHIPSET_DIR))
${WIFI_PUBLIC_DIR}:
	@mkdir -p ${WIFI_PUBLIC_DIR}
endif

${BT_CHIPSET_DIR}:
	@mkdir -p ${BT_CHIPSET_DIR}

${NFC_CHIPSET_DIR}:
	@mkdir -p ${NFC_CHIPSET_DIR}

download-all: download
	${Q_}for driver in ${ALL_WIFI_DRIVERS}; do \
		if [[ $$driver = "${DHD_DRIVER_VERSION}.tgz" ]] \
			|| [[ $$driver = "${WIFI_AP_VERSION_FD}.tgz" ]] ; then \
			if ! [ -e ${WIFI_PUBLIC_DIR}/$$driver ] ; then \
				echo "Downloading $$driver..." && \
				cd ${WIFI_PUBLIC_DIR} && \
				wget ${INTERNAL_DRIVER_DIR}/$$driver;\
			fi; \
		elif [[ $$driver = "${BISON_DRIVER_VERSION}.tgz" ]] \
			|| [[ $$driver = "${WIFI_AP_VERSION_NIC}.tgz" ]] \
			|| [[ $$driver = "${WIFI_AP_VERSION_APP}.tgz" ]] ; then \
			if ! [ -e ${WIFI_PRIVATE_DIR}/$$driver ] ; then \
				echo "Downloading $$driver..." && \
				cd ${WIFI_PRIVATE_DIR} && \
				wget ${INTERNAL_DRIVER_DIR}/$$driver;\
			fi; \
		elif ! [ -e ${WIFI_DIR}/${WIFI_CHIPSET}/$$driver ] ; then \
			echo "Downloading $$driver..." && \
			mkdir -p ${WIFI_DIR}/${WIFI_CHIPSET} && \
			cd ${WIFI_DIR}/${WIFI_CHIPSET} && \
			wget ${INTERNAL_DRIVER_DIR}/$$driver;\
		fi; \
	done

download: ${BT_CHIPSET_DIR} ${WIFI_CHIPSET_DIR} ${WIFI_PRIVATE_DIR} ${WIFI_PUBLIC_DIR}
ifeq (${HAS_BLUETOOTH},)
ifneq ($(BLUETOOTH_SRC),)
ifeq (${HAS_BLUETOOTH_SRC},)
ifneq (${SERVER_ALIVE},)
	@echo "==== Downloading Bluetooth ${BT_PKG_VERSION}..."
	${Q_}wget ${BLUETOOTH_SRC_PKG_SERVER} -O ${BLUETOOTH_SRC} || rm -f ${BLUETOOTH_SRC}
endif # SERVER_ALIVE
endif # HAS_BLUETOOTH_SRC
endif # BLUETOOTH_SRC
endif # HAS_BLUETOOTH
ifeq (${HAS_WIFI},)
ifeq (${HAS_WIFI_SRC},)
ifneq ($(WIFI_SRC_PKG),)
ifneq (${SERVER_ALIVE},)
	@echo "==== Downloading WIFI ${WIFI_DRIVER_VERSION}..."
	${Q_}wget ${WIFI_SRC_PKG_SERVER} -O ${WIFI_SRC_PKG} || rm -f ${WIFI_SRC_PKG}
endif # SERVER_ALIVE
endif # WIFI_SRC_PKG
endif # HAS_WIFI_SRC
endif # HAS_WIFI

ifneq (${SERVER_ALIVE},)
ifneq (${WIFI_SRC_SERVER_AP1},)
ifeq ($(wildcard ${WIFI_SRC_PKG_AP1}),)
	@echo "==== Downloading WIFI AP1 ${WIFI_SRC_SERVER_AP1}..."
	${Q_}wget ${WIFI_SRC_SERVER_AP1} -O ${WIFI_SRC_PKG_AP1} || rm -f ${WIFI_SRC_PKG_AP1}
endif # WIFI_SRC_PKG_AP1
endif # WIFI_SRC_SERVER_AP1

ifneq (${WIFI_SRC_SERVER_AP2},)
ifeq ($(wildcard ${WIFI_SRC_PKG_AP2}),)
	@echo "==== Downloading WIFI AP2 ${WIFI_SRC_SERVER_AP2}..."
	${Q_}wget ${WIFI_SRC_SERVER_AP2} -O ${WIFI_SRC_PKG_AP2} || rm -f ${WIFI_SRC_PKG_AP2}
endif # WIFI_SRC_PKG_AP2
endif # WIFI_SRC_SERVER_AP2

ifneq (${WIFI_SRC_SERVER_AP},)
ifeq ($(wildcard ${WIFI_APP_PKG_AP}),)
	@echo "==== Downloading WIFI AP ${NETAPP_WIFI_APP_AP}..."
	${Q_}wget ${WIFI_SRC_SERVER_AP} -O ${WIFI_SRC_PKG_AP} || rm -f ${WIFI_SRC_PKG_AP}
endif # WIFI_SRC_PKG_AP
endif # WIFI_SRC_SERVER_AP

endif # SERVER_ALIVE



${NETAPP_PREBUILT_TARBALL}:  dirs ${LIBUSB_NEEDED} wlan bluetooth
	${Q_}cd ${NETAPP_OUTPUT_DIR}/../../; tar -cjf ${NETAPP_PREBUILT_TARBALL} .
	${Q_}$(MAKE) distclean ${SQUASH_OUTPUT}
	@echo "==== Finished Creating $@"

# needed for Falcon driver
ifeq (${NETAPP_WIFI_DRIVER},falcon)
libusb:
	${Q_}$(MAKE) -C ../libs/usb install
else
libusb:
endif


prebuilt: ${NETAPP_PREBUILT_TARBALL}


dirs: help
	@echo "==== Starting NetApp-Platform Build"
	${Q_}if ! [ -d ${NETAPPPLATFORM_OBJ_DIR}    ] ; then 	mkdir -p ${NETAPPPLATFORM_OBJ_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_FW_DIR}      ] ; then 	mkdir -p ${NETAPP_OUTPUT_FW_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_LIB_DIR}     ] ; then 	mkdir -p ${NETAPP_OUTPUT_LIB_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_INC_DIR}     ] ; then 	mkdir -p ${NETAPP_OUTPUT_INC_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_BIN_DIR}     ] ; then 	mkdir -p ${NETAPP_OUTPUT_BIN_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_MODULES_DIR} ] ; then 	mkdir -p ${NETAPP_OUTPUT_MODULES_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_PREBUILT_DIR}       ] ; then 	mkdir -p ${NETAPP_PREBUILT_DIR}; fi
ifeq (${NETAPP_WIFI_AP},y)
	${Q_}if ! [ -d ${NETAPP_OUTPUT_AP_DIR}    ] ; then    mkdir -p ${NETAPP_OUTPUT_AP_DIR}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_AP_DIR_NIC}    ] ; then    mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}; fi
	${Q_}if ! [ -d ${NETAPP_OUTPUT_AP_DIR_FD}    ] ; then    mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}; fi
endif

libs: $(NETAPP_DEP_LIBS) dirs

${NETAPP_DEP_LIBS}: dirs
	$(Q_)$(MAKE) -C ${NETAPP_LIBS_DIR}/$@ install


distclean-libs:
	${Q_}for lib in ${NETAPP_DEP_LIBS}; do \
		if [ -d ${NETAPP_LIBS_DIR}/$$lib ] ; then \
			$(MAKE) -C ${NETAPP_LIBS_DIR}/$$lib distclean; \
		fi; \
	done

clean-libs:
	${Q_}for lib in ${NETAPP_DEP_LIBS}; do \
		if [ -d ${NETAPP_LIBS_DIR}/$$lib ] ; then \
			$(MAKE) -C ${NETAPP_LIBS_DIR}/$$lib clean; \
		fi; \
	done


clean: clean-netapp clean-libs


distclean: distclean-libs distclean-netapp
	@if [ -d $(NETAPPPLATFORM_OBJ_DIR)   ] ; then rm -rf  ${NETAPPPLATFORM_OBJ_DIR};  fi

clean-to-origin: distclean
	${Q_}rm -rf ${NETAPP_PREBUILT_DIR} ${NETAPP_TOP}/../*.bz2


#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##
############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

clean-netapp:
ifneq (, ${HAS_WIFI_SRC})
	@echo "======== Cleaning WiFi"
	${Q_}-$(MAKE) -C ../wlan clean WIFI_CHIPSET=${WIFI_CHIPSET}
endif
ifneq (, ${HAS_BLUETOOTH_SRC})
	@echo "======== Cleaning Bluetooth"
	${Q_}-$(MAKE) -C ../bluetooth clean
endif
	${Q_}if [ -e $(NETAPP_WLAN_BREADCRUMB) ] ; then \
		rm $(NETAPP_WLAN_BREADCRUMB); \
	fi
	${Q_}if [ -e $(NETAPP_BT_BREADCRUMB) ] ; then \
		rm $(NETAPP_BT_BREADCRUMB); \
	fi

distclean-netapp: clean
ifneq (, ${HAS_WIFI_SRC})
	@echo "======== Distclean WiFi"
	${Q_}-$(MAKE) -C ../wlan distclean WIFI_CHIPSET=${WIFI_CHIPSET}
endif
	@echo "======== Distclean Bluetooth"
	${Q_}-$(MAKE) -C ../bluetooth distclean
	@echo "======== Distclean NetApp-Platform"
	${Q_}rm -f ${F_INSTALLED_LIBS} ${F_INSTALLED_MAKE} ${F_INSTALLED_INCS}

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

prebuilt: dirs
ifneq (${NETAPP_WIFI},n)
ifeq (${HAS_WIFI_SRC},)
ifeq (${HAS_WIFI_BUILT},)
ifeq (${HAS_PREBUILT},)
	@echo "==== Prebuilt ${NETAPP_PREBUILT_TARBALL} Not Found!"
else
	@echo "==== Unpacking ${NETAPP_PREBUILT_TARBALL}"
	${Q_}cd ${NETAPP_OUTPUT_DIR}/../../; tar xjf ${NETAPP_PREBUILT_TARBALL}
endif
endif
endif
endif


all: dirs libs bluetooth wlan

LDFLAGS_OUT += $(STATIC_LDFLAGS) ${NETAPP_LDFLAGS} ${LIBCURL_LDFLAGS} -lpthread -lrt

# generate netapp.inc
make-include:
	${Q_}if [ -f ${F_INSTALLED_MAKE} ] ; then rm -f ${F_INSTALLED_MAKE}; fi
	@echo "======== Generating ${F_PUBLIC_MAKE}"
	@echo "CC=${CC}" >> ${F_INSTALLED_MAKE}
	@echo "CXX=${CXX}" >> ${F_INSTALLED_MAKE}
	@echo "LD=${LD}" >> ${F_INSTALLED_MAKE}
	@echo "STRIP=${STRIP}" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_LDFLAGS+=${LDFLAGS_OUT}" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_CFLAGS+=${CPPFLAGS} ${CFLAGS}" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_WIFI=$(subst ,n,$(HAS_WIFI))" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_WIFI_P2P=$(NETAPP_WIFI_P2P)" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_WIFI_WPS=$(NETAPP_WIFI_WPS)" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_BLUETOOTH=$(subst ,n,$(HAS_BLUETOOTH))" >> ${F_INSTALLED_MAKE}
	@echo "BT_PKG_VERSION=\"$(BT_PKG_VERSION)\"" >> ${F_INSTALLED_MAKE}
	@echo "NFC_PKG_VERSION=\"$(NFC_PKG_VERSION)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_WIFI_DRIVER=\"$(NETAPP_WIFI_DRIVER)\"" >> ${F_INSTALLED_MAKE}
	@echo "WIFI_DRIVER_VERSION=\"$(WIFI_DRIVER_VERSION)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_WIFI_CHIP=$(NETAPP_WIFI_CHIP)" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_DHD_WIFI_CHIPS=\"$(NETAPP_DHD_WIFI_CHIPS)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_EAGLE_WIFI_CHIPS=\"$(NETAPP_EAGLE_WIFI_CHIPS)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_BISON_WIFI_CHIPS=\"$(NETAPP_BISON_WIFI_CHIPS)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_AARDVARK_WIFI_CHIPS=\"$(NETAPP_AARDVARK_WIFI_CHIPS)\"" >> ${F_INSTALLED_MAKE}
	@echo "NETAPPPLATFORM_OBJ_DIR=$(NETAPPPLATFORM_OBJ_DIR)" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_BSA_LIB_DIR=$(BSA_DIR)" >> ${F_INSTALLED_MAKE}
	@echo "WIFI_DRIVER_PKG=$(WIFI_DRIVER_PKG)" >> ${F_INSTALLED_MAKE}
	@echo "NETAPP_TARGET_FW_DIR=$(NETAPP_TARGET_FW_DIR)" >> ${F_INSTALLED_MAKE}


${F_INSTALLED_INCS}: ${NETAPP_OUTPUT_INC_DIR}/%.h : ../include/%.h
	${Q_}install  -m 0664 $< $@

############################################################################
#                             DEP FILES, PHONY
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

.PHONY: help install all clean distclean prebuilt docs netapp download wlan help-menu bluetooth libs
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##
vpath %.h ${INC_DIRS}
vpath %.c ${SRC_DIRS}
vpath %.cpp ${SRC_DIRS}
