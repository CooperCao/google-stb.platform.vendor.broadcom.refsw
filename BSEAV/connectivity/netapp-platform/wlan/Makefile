############################################################################
#    (c)2006-2015 Broadcom Corporation
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
# $brcm_Revision: 11 $
# $brcm_Date: 2/3/12 4:18p $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: /NetApp/netapp/wlan/Makefile $
#
# 11   2/3/12 4:18p steven
# SW7231-638: NetApp Version 6.2
#
# 10   1/30/12 3:50p steven
# SW7429-37: Fixes to bringup 7241 WiFi
#
# 11   12/9/11 4:44p steven
# SW7231-513: USB Hotplug Support in NetApp
#
# SW7231-513/1   12/9/11 3:36p steven
# SW7231-513: Finished working hotplug detection
#
# 9   12/1/11 8:56p steven
# SW7425-1404: Moved kernel modules and fixes to BT support
#
# 8   11/24/11 5:14p steven
# SW7425-1404: Bluetooh, WLAN Driver, and Multi-Wired Interface Support
#
# SW7425-1404/5   11/24/11 5:01p steven
# SW7425-1404: Don't build driver in parallel build
#
# SW7425-1404/4   11/24/11 4:02p steven
# SW7425-1404: Bluetooth, WLAN driver build, and multi-interface support
#
# SW7425-1404/1   10/7/11 5:33p steven
# SW7425-1404: initial pass fo BT support
#
# 7   10/4/11 3:01p steven
# SW7425-1399: Added support in the $(MAKE)file to load prebuilt by default
#
# 6   7/29/11 3:44p steven
# SW7425-749: Fixed $(MAKE)file rule
#
# 5   7/28/11 2:14p steven
# SW7425-749: Initial WoWL and P2P development checkin
#
# 4   7/7/11 10:56p steven
# SWBLURAY-26474: ndvd SRC package build fails
#
# 3   6/23/11 10:54a steven
# SW7420-1956: AppLibs $(MAKE) clean gives error
#
# 2   6/15/11 5:27p steven
# SW7425-714: Adding NetApp support to AppLibs
#
# 1   6/14/11 10:41a steven
# SW7425-714: Moved NetApp to AppLibs
#
############################################################################
#
# DESCRIPTION:
#   Makefile for building non-NetApp WiFi code
#
# !!!!!! IMPORTANT !!!!!!
# If your platform contains OpenSSL then set EXTERNAL_OPENSSL=1 and
# EXTERNAL_OPENSSL_BASE=<FULL path to openssl>. also if you need to link with
# another library like zlib then set EXTERNAL_LIBS
#
############################################################################
NETAPP_TOP = ${shell pwd}/..
include ${NETAPP_TOP}/build/build.inc
include ${NETAPP_TOP}/build/modules.inc

ifneq (,$(findstring mips,$(CC)))
	ARCH				:= mips
	TARGET_ARCH			:= $(ARCH)
    ifeq (${CONFIG_CPU_BIG_ENDIAN},y)
    TARGETARCH			:= mips_be
	TARGETMACH			:= mipseb
	TARGETENV			:= linuxmips_be
    else
    TARGETARCH			:= mips
	TARGETMACH			:= mipsel
	TARGETENV			:= linuxmips
    endif
else ifneq (,$(findstring arm,$(CC)))
	ARCH				:= arm
	TARGET_ARCH			:= armv7l
    ifeq (${CONFIG_CPU_BIG_ENDIAN},y)
    TARGETARCH			:= arm_be
	TARGETMACH			:= armeb
	TARGETENV			:= linuxarm_be
    else
    TARGETARCH			:= arm_le
	TARGETMACH			:= armle
	TARGETENV			:= linuxarm_le
    endif
else
	TARGETARCH	:=x86
	ARCH		:=x86
	TARGETENV	:= linux
	TARGET_ARCH	:= $(ARCH)
endif


################################################################################
#                              WIFI_CHIPSET:
# specify which WiFi driver to use
#vvvv###########################################################################
ifeq (${WIFI_CHIPSET}, broadcom)

ifeq ($(NETAPP_WIFI_DRIVER),eagle)
WL_DIR				:= ${WIFI_DRIVER_PKG}/sys/src/wl/exe
WL_MFG_DIR			:= ${WIFI_MFG_DRIVER_PKG}/sys/src/wl/exe
WIFI_DIR_PREFIX		:= /main
else
WL_DIR				:= ${WIFI_DRIVER_PKG}/src/wl/exe
WL_MFG_DIR			:= ${WIFI_MFG_DRIVER_PKG}/src/wl/exe
endif

# Broadcom Userspace Library Definitions
WPSENR_APP			:= wpsenr_app
WPSAPI_TESTER		:= wpsapitester
P2PLIB_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/p2p/p2plib
WFI_DIR				= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/apps/wfi/linux
WPSCLI_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wps/wpscli
WPSAPI_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wps/wpsapi
WPSCOM_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wps/common
WPSENR_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wps/linux/enr
OUPUT_DIR	 		= ${NETAPPPLATFORM_OBJ_DIR}/${CC}
EPIVERS_DIR			= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/include
EPIVERS				= ${EPIVERS_DIR}/epivers.h
EPIVERS_MFG_DIR		= ${WIFI_MFG_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/include
EPIVERS_MFG			= ${EPIVERS_MFG_DIR}/epivers.h
F_PUBLIC_LIBS		+= ${NETAPP_SHARED_LIBS} ${NETAPP_STATIC_LIBS}
WFDCAPDIE_DIR		= ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/apps/wfd_capd/linux/wfd_capd_ie

# Fix for channelspec needed by Phoenix driver
ifeq (${NETAPP_WIFI_DRIVER}, phoenix2)
BCM_P2P_IOTYPECOMPAT	= BCM_P2P_IOTYPECOMPAT=1
BCM_P2P_IOTYPECOMPAT_DEF=-D${BCM_P2P_IOTYPECOMPAT}
endif


#check what libraries we need to include to build

ifeq (${NETAPP_NEEDS_WPSCLI}, y)
F_PUBLIC_INCS		+= wpscli_api.h
INC_DIRS			+= ${WPSCLI_DIR}/include
endif

WL					= wl${TARGETARCH}
DHD					= dhd${TARGETARCH}
WL_MFG				= wl${TARGETARCH}_mfg

############################
#  WiFi Library Defines
############################
# BWL Support
INC_DIRS			+= \
	${WIFI_CHIPSET} \
	${P2PLIB_DIR}/include \
	${EPIVERS_DIR} \
	${EPIVERS_MFG_DIR} \
	${OUPUT_DIR}

F_PUBLIC_INCS		+= epivers.h typedefs.h site_typedefs.h bcmdefs.h

# WiFi Direct (P2P)
ifneq (${NETAPP_WIFI_P2P},n)
INC_DIRS			+= ${P2PLIB_DIR}/include
F_PUBLIC_INCS 		+= BcmP2PAPI.h
#WIFI_EXAMPLES		+= bcmp2papp
ifneq (${NETAPP_WIFI_WFD},n)
INC_DIRS			+= ${WFDCAPDIE_DIR}/../../include
F_PUBLIC_INCS 		+= wfd_capd.h wfd_capdie.h wfadisp.h
endif
endif


# WPS 2.0 & Internal WPS library
ifneq (${NETAPP_WIFI_WPS},n)
INC_DIRS			+= ${WPSAPI_DIR}/common/include
F_PUBLIC_INCS		+= wps_sdk.h
endif

BLDTYPE 			?= release
else
$(error "WIFI_CHIPSET ${WIFI_CHIPSET} Not supported")
endif


############################################################################
#                              NETAPP WIFI TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
SRC_DIRS			+= .
INC_DIRS 			+= . ../
CPPFLAGS 			+= -DHAS_WIFI
SHARED_LDFLAGS		+= -Wl,-Bsymbolic,-dy,-lstdc++,-lpthread,-lm
LDFLAGS 			+= -L${NETAPP_OUTPUT_LIB_DIR} -L.
F_SRCS 				= $(filter-out \
				$(addprefix %/, ${EXCLUDED_SRCS}), \
			$(foreach d,${SRC_DIRS},$(wildcard ${d}/*.c ${d}/*.cpp)))
F_OBJS_COMPILED 	+= $(patsubst %.c,%.o, $(patsubst %.cpp,%.o, ${F_SRCS}))
F_OBJS              += ${F_OBJS_PRECOMPILED} ${F_OBJS_COMPILED}
CPPFLAGS       		+= $(addprefix -I, ${D_ALL_SUBDIRS} ${INC_DIRS})
F_INSTALL_LIBS		+= $(addsuffix .so, $(addprefix ${NETAPP_OUTPUT_LIB_DIR}/lib, ${NETAPP_SHARED_LIBS}))
F_INSTALL_LIBS		+= $(addsuffix .a, $(addprefix ${NETAPP_OUTPUT_LIB_DIR}/lib, ${NETAPP_STATIC_LIBS}))
F_INSTALLED_INCS	+= $(addprefix ${NETAPP_OUTPUT_INC_DIR}/, $(notdir ${F_PUBLIC_INCS}))

WIFI_DRIVER_INS		+= $(addprefix ${NETAPP_OUTPUT_MODULES_DIR}/, ${WIFI_MODULES})


WIFI_MODULES_OPTS	=  \
	SHARED_LIB=1 \
	EXTERNAL_OPENSSL=${EXTERNAL_OPENSSL} \
	EXTERNAL_OPENSSL_BASE=${EXTERNAL_OPENSSL_BASE} \
	EXTERNAL_LIBS="${EXTERNAL_LIBS}" \
	BLDTYPE=${BLDTYPE}\
	TARGETARCH=${TARGETARCH} \
	TARGETENV=${TARGETENV} \
	TARGETOS=unix\
	CC=${CC} \
	LD=${LD} \
	STBLINUX=1 \
	LIBUSB_PATH=${NETAPP_OUTPUT_DIR} \
	STRIP=${STRIP} \
	EXTERNAL_OPENSSL_INC=${EXTERNAL_OPENSSL_BASE}/include \
	LIBDIR=${OUPUT_DIR} \
	WCN_NET_SUPPORT=y \
	NETAPP_WIFI_DRIVER=${NETAPP_WIFI_DRIVER}

############################################################################
#                              WIFI_CHIPSET Builds
############################################################################
ifeq (${WIFI_CHIPSET}, broadcom)
# Miracast IE parsing
${WFDCAPDIE}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) -C ${WFDCAPDIE_DIR} BLDTYPE=${BLDTYPE} CC=$(CC) CFLAGS="-Wall -Wnested-externs -fPIC" TARGETARCH=${TARGETARCH} ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${WFDCAPDIE_DIR}/obj/${TARGETARCH}-${BLDTYPE}/lib${WFDCAPDIE}.a  ${NETAPP_OUTPUT_LIB_DIR}

#Sig part of P2P library (built by BCMP2P target)
${BCMP2PSIG}:


#WiFi Direct (P2P) Library
${BCMP2P}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${P2PLIB_DIR}/linux  BCM_P2P_ACRATES=1 BLDTYPE=debug \
		CFLAGS="-Wall -Wnested-externs -DWPS_WIRELESS_ENROLLEE -MD -Wformat -Wformat-security \
		-Wformat-nonliteral -DP2PAPI_ENABLE_DEBUG_SHOWKEY=1 -DP2PAPI_ENABLE_WPS=1 -DP2PAPI_ENABLE_MULTI_CHANNEL=1 ${BCM_P2P_IOTYPECOMPAT_DEF} \
		-DP2PAPI_ENABLE_DHCPD=0 -DP2PAPI_ENABLE_SERVICE_DISCOVERY=1 -DSTBLINUX=1 -DBCM_P2P_ACRATES -DP2PAPI_USE_IDAUTH=1 -DP2PAPI_USE_IDSUP=1 -g \
		-D_TUDEBUGTRACE -DBCMP2P -DWLP2P -DWIFI_ACT_FRAME -DWPSCLI_NO_WPS_LOOP -D_TUDEBUGTRACE -DDEBUG -D_REENTRANT -gstabs+ -fPIC -I${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/shared/bcmwifi/include" \
		CC=${CC} TARGETARCH=${TARGETARCH} BCMSTB=0 OBJDIR=obj/$(TARGETARCH)-debug-intsec ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${P2PLIB_DIR}/linux/obj/${TARGETARCH}-debug-intsec/lib${BCMP2PSIG}.a  ${NETAPP_OUTPUT_LIB_DIR}
	${Q_}install -m 0755 ${P2PLIB_DIR}/linux/obj/${TARGETARCH}-debug-intsec/lib${BCMP2P}.a  ${NETAPP_OUTPUT_LIB_DIR}


#WPS Test Library
${WPSAPI_TESTER}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) -C ${WPSAPI_DIR}/linux BLDTYPE=${BLDTYPE} TARGETARCH=${TARGETARCH} TARGETENV=${TARGETENV} TARGETOS=unix CC=${CC} LD=${LD} \
	STRIP=${STRIP} WCN_NET_SUPPORT=y ${BCM_P2P_IOTYPECOMPAT} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src ${SQUASH_OUTPUT}
ifeq (,$(filter kirin%, ${WIFI_DRIVER_VERSION}))
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${BLDTYPE}/${TARGETARCH}/${WPSAPI_TESTER} ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${BLDTYPE}/${TARGETARCH}/wpsenr ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${BLDTYPE}/${TARGETARCH}/wpsreg ${NETAPP_OUTPUT_BIN_DIR}
else
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${CC}/${WPSAPI_TESTER} ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${CC}/wpsenr ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSAPI_DIR}/linux/${CC}/wpsreg ${NETAPP_OUTPUT_BIN_DIR}
endif

${WPSENR_APP}: ${WIFI_DRIVER_PKG}
	${Q_}cd ${WPSENR_DIR} && \
		$(MAKE) CC=${CC} STRIP=${STRIP} OUPUT_DIR=${OUPUT_DIR} BLDTYPE=${BLDTYPE} \
		EXTERNAL_OPENSSL=${EXTERNAL_OPENSSL} V=1 SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src -f wps_enr_app.mk ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${WPSENR_DIR}/${CC}/wpsenr ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSENR_DIR}/${CC}/wpsreg ${NETAPP_OUTPUT_BIN_DIR}
	${Q_}install -m 0755 ${WPSENR_DIR}/${CC}/wpsapitester ${NETAPP_OUTPUT_BIN_DIR}

#WiFi Invite Library
${WFI}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WFI_DIR}/wfi_api ${WIFI_MODULES_OPTS} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src outdir ${SQUASH_OUTPUT}
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WFI_DIR}/wfi_api ${WIFI_MODULES_OPTS} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src wfiapi ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${WFI_DIR}/wfi_api/obj/${TARGETARCH}-${BLDTYPE}/lib${WFI}.so  ${NETAPP_OUTPUT_LIB_DIR}


#WPSCLI Library
${WPSCLI}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WPSCLI_DIR}/linux ${WIFI_MODULES_OPTS} NO_WPS_THREAD=1 ${BCM_P2P_IOTYPECOMPAT} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src lib${WPSCLI}.so ${SQUASH_OUTPUT}
	${Q_}install ${WPSCLI_DIR}/linux/obj/${TARGETARCH}-${BLDTYPE}/lib${WPSCLI}.so ${NETAPP_OUTPUT_LIB_DIR}


# WPSAPI Library
${WPSAPI}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
#	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WPSAPI_DIR}/linux ${WIFI_MODULES_OPTS} ${BCM_P2P_IOTYPECOMPAT} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src libs ${SQUASH_OUTPUT}
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WPSAPI_DIR}/linux ${WIFI_MODULES_OPTS} ${BCM_P2P_IOTYPECOMPAT} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${OUPUT_DIR}/lib${WPSAPI}.so ${NETAPP_OUTPUT_LIB_DIR}


# WPS Common Lirary
${WPSCOM}: ${WIFI_DRIVER_PKG} ${WPSENR}
	@echo "============ Building $@"
	${Q_}$(MAKE) -C ${WPSCOM_DIR} ${WIFI_MODULES_OPTS} CFLAGS+="-fPIC -DWCN_NET_SUPPORT ${BCM_P2P_IOTYPECOMPAT_DEF}" SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src -f wps_common_lib.mk ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${OUPUT_DIR}/lib${WPSCOM}.so ${NETAPP_OUTPUT_LIB_DIR}


# WPS Core SoftAP Library
${WPSAP}: ${WIFI_DRIVER_PKG} ${WPSCOM}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WPSCOM_DIR} ${WIFI_MODULES_OPTS} CFLAGS+="-fPIC -DWCN_NET_SUPPORT ${BCM_P2P_IOTYPECOMPAT_DEF}" SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src -f wps_ap_lib.mk ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${OUPUT_DIR}/lib${WPSAP}.so ${NETAPP_OUTPUT_LIB_DIR}

WPS_CFLAGS = -fPIC -DWCN_NET_SUPPORT ${BCM_P2P_IOTYPECOMPAT_DEF}
ifeq (${WPS_OVERLAP_CHECKS_UUID_BSSID},y)
WPS_CFLAGS += -DWPS_OVERLAP_CHECKS_UUID_BSSID
endif

# WPS ENR (enrollee, registrar, shared, etc....) Library
${WPSENR}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WPSCOM_DIR} ${WIFI_MODULES_OPTS} CFLAGS+="${WPS_CFLAGS}" SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src -f wps_enr_lib.mk ${SQUASH_OUTPUT}
	${Q_}install -m 0755 ${OUPUT_DIR}/lib${WPSENR}.so ${NETAPP_OUTPUT_LIB_DIR}
ifneq (${EXTERNAL_OPENSSL},1)
	${Q_}install -m 0755 ${OUPUT_DIR}/lib${BCMCRYPTO}.a ${NETAPP_OUTPUT_LIB_DIR}
endif

# Crypto library built by another WPS library
${BCMCRYPTO}:

wl: ${WL}
dhd: ${DHD}

${DHD}: ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/exe ASD=0 CC=${CC} \
	DEBUG=1 TARGETARCH=${TARGETARCH} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/exe/$@ ${NETAPP_OUTPUT_BIN_DIR}/dhd

${WL}: ${WIFI_DRIVER_PKG} ${EPIVERS}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C $(WL_DIR) ASD=0 CC=${CC} \
	DEBUG=1 TARGETARCH=${TARGETARCH} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}cp $(WL_DIR)/$@ ${NETAPP_OUTPUT_BIN_DIR}/wl

${WL_MFG}: ${WIFI_MFG_DRIVER_PKG} ${EPIVERS_MFG}
	@echo "============ Building $@"
	${Q_}$(MAKE) ${MAKE_J_ARG} -C $(WL_MFG_DIR) ASD=0 CC=${CC} \
	DEBUG=1 TARGETARCH=${TARGETARCH} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}cp $(WL_MFG_DIR)/${WL} ${NETAPP_OUTPUT_BIN_DIR}/wl

bcmp2papp:  ${WIFI_DRIVER_PKG}
	@echo "============ Building $@"
	${Q_}$(MAKE) -C ${P2PLIB_DIR}/linux/sampleapp ASD=0 CC=${CC} ${BCM_P2P_IOTYPECOMPAT} \
	DEBUG=1 TARGETARCH=${TARGETARCH} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}cp ${P2PLIB_DIR}/linux/sampleapp/obj/${TARGETARCH}-debug-intsec/$@ ${NETAPP_OUTPUT_BIN_DIR}

${EPIVERS}: ${WIFI_DRIVER_PKG}
	${Q_}if [ ! -e $(EPIVERS) ] ; then \
		$(MAKE) -C ${EPIVERS_DIR} ${SQUASH_OUTPUT};\
	fi

${EPIVERS_MFG}: ${WIFI_DRIVER_PKG} ${WIFI_MFG_DRIVER_PKG}
	${Q_}if [ ! -e $(EPIVERS_MFG) ] ; then \
		$(MAKE) -C ${EPIVERS_MFG_DIR} ${SQUASH_OUTPUT};\
	fi

# Kernel Mode Driver Build
ifdef LINUX
##################################
#  KIRIN  #
##################################
ifneq (,$(filter kirin%, ${WIFI_DRIVER_VERSION}))
### TGK - From this point on - all "mips"-es got replaced by $(ARM); OK, one left for ref
###WIFI_TARGET	= mips-apdef-stadef-high-dnglimage-media-p2p ### TGK
WIFI_TARGET	= $(ARCH)-apdef-stadef-high-dnglimage-media-p2p
drivers: ${WIFI_DRIVER_PKG}
	@echo "============ Building ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/wl/linux ${WIFI_TARGET} \
	CC=${CC} LD=${LD} STRIP=${STRIP} \
	LINUXDIR=${LINUX} \
	TARGETARCH=$(ARCH) LINUXVER=${LINUXVER} BRAND=linux-external-wl WLLXIW=0 \
	STBLINUX=1 BCM_DNGL_EMBEDIMAGE=1 \
	FIRMWARE="4322-bmac/roml-ag-nodis-media-p2p-mchan 43236b0-bmac/ag-nodis-media-p2p-mchan-pool" \
	NVRAMSTUBS=1 USBSHIM=1 WET=1\
	ARCH=${ARCH} \
	BRCM_WLAN_IFNAME=wln%d RPC_RETURN_WAIT_TIMEOUT_MSEC=30000 WL_DNGL_WD=0 ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/wl/linux/obj-${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}
endif


##################################
# FALCON  falcon_rel_5_99_188.xx #
##################################
ifneq (,$(filter falcon_rel%, ${WIFI_DRIVER_VERSION}))

ifneq (,$(findstring 43238,$(NETAPP_WIFI_CHIP)))
DNGL_FW_NAME="fake43238um_p260.nvm"
DNGL_IMAGE_NAME="${NETAPP_WIFI_CHIP}-roml/ag-idsup-af-wet"
else
DNGL_FW_NAME="fake43236usb_p532.nvm"
ifneq ($(NETAPP_WIFI_P2P),n)
DNGL_IMAGE_NAME="${NETAPP_WIFI_CHIP}-roml/ag-p2p-apsta-idsup-af-idauth-wowlpf"
else
DNGL_IMAGE_NAME="${NETAPP_WIFI_CHIP}-roml/ag-idsup-af"
endif
endif
WIFI_TARGET=dhd-cdc-usb-gpl
WIFI_CHIP_STRIPPED = $(filter-out %b,%,$(NETAPP_WIFI_CHIP))
DNGL_MFG_IMAGE_NAME="${NETAPP_WIFI_CHIP}-roml/ag-mfgtest-seqcmds"

ifeq (${CONFIG_CPU_BIG_ENDIAN},y)
WIFI_TARGET := ${WIFI_TARGET}-be
endif

drivers: ${WL}
	@echo "============ Building ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/dhd/linux ${WIFI_TARGET} \
	LINUXDIR=${LINUX} TARGETARCH=$(ARCH) WET=1 WLTEST=${WLTEST} \
	CROSS_COMPILE=${CROSS_COMPILE} \
	BRCM_WLAN_IFNAME=wln%d \
	ARCH=${ARCH} \
	LINUXVER=${LINUXVER} USBSHIM=1 WL_NVRAM_FILE=1 CC=${CC} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/usbdev/usbdl ${WIFI_MODULES_OPTS} ${SQUASH_OUTPUT}
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
	${Q_}cp ${WIFI_DRIVER_PKG}/firmware/${DNGL_IMAGE_NAME}/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/${NETAPP_FIRMWARE}
	${Q_}cp ${WIFI_DRIVER_PKG}/nvrams/${DNGL_FW_NAME} ${NETAPP_OUTPUT_FW_DIR}/${NETAPP_NVRAM}

drivers_mfg: ${EPIVERS_MFG} ${WL_MFG}
	@echo "============ Building ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux ${WIFI_TARGET} \
	LINUXDIR=${LINUX} TARGETARCH=$(ARCH) WET=1 WLTEST=${WLTEST} \
	CROSS_COMPILE=${CROSS_COMPILE} \
	BRCM_WLAN_IFNAME=wln%d \
	ARCH=${ARCH} \
	LINUXVER=${LINUXVER} USBSHIM=1 WL_NVRAM_FILE=1 CC=${CC} STRIP=${STRIP} ${SQUASH_OUTPUT}

	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/usbdev/usbdl ${WIFI_MODULES_OPTS} ${SQUASH_OUTPUT}
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ] ; then \
		cp ${WIFI_MFG_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ] ; then \
		cp ${WIFI_MFG_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -d ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER} ] ; then \
		cp ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}/dhd_mfg.ko; \
	fi
	${Q_}if [ -d ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION} ] ; then \
		cp ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}/dhd_mfg.ko; \
	fi
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/firmware/${DNGL_MFG_IMAGE_NAME}/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/${NETAPP_FALCON_MFG_FIRMWARE}
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/nvrams/${DNGL_FW_NAME} ${NETAPP_OUTPUT_FW_DIR}/${NETAPP_NVRAM}
endif


#############################################
# PHOENIX  phoenix2dhd-1-66_rel_6_10_166_xx #
#############################################
ifneq (,$(filter phoenix2dhd-1-66_rel_6_10_166%, ${WIFI_DRIVER_VERSION}))

### FIXME: Need to support multiple chips and mfg driver

ifneq (,$(findstring 43242,$(NETAPP_WIFI_CHIP)))
DNGL_IMAGE_NAME="${NETAPP_WIFI_CHIP}-roml/usb-ag-p2p-mchan-idauth-idsup-keepalive-vsdb-proptxstatus-nodis"
WIFI_TARGET=dhd-cdc-usb-reqfw-gpl
DNGL_FW_NAME="bcm943242usbref_p303.txt"
else
$error "WIFI_CHIPSET ${WIFI_CHIPSET} Not supported in ${WIFI_DRIVER_VERSION}"
endif

ifeq (${CONFIG_CPU_BIG_ENDIAN},y)
WIFI_TARGET := ${WIFI_TARGET}-be
endif

drivers: ${EPIVERS}
	@echo "============ Building ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/dhd/linux ${WIFI_TARGET} \
	LINUXDIR=${LINUX} TARGETARCH=mips WLTEST=${WLTEST} \
	CROSS_COMPILE=${CROSS_COMPILE} \
	BRCM_WLAN_IFNAME=wln%d \
	ARCH=${ARCH} \
	LINUXVER=${LINUXVER} USBSHIM=1 WL_NVRAM_FILE=1 CC=${CC} STRIP=${STRIP} ${SQUASH_OUTPUT}

	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
	${Q_}cp ${WIFI_DRIVER_PKG}/firmware/${DNGL_IMAGE_NAME}/rtecdc.bin.trx ${NETAPP_FIRMWARE_PATH}
	${Q_}cp ${WIFI_DRIVER_PKG}/nvrams/${DNGL_FW_NAME} ${NETAPP_OUTPUT_FW_DIR}

endif


##################################
# EAGLE (MEDIA) #
##################################
ifneq (,$(filter eagle%, ${WIFI_DRIVER_VERSION}))
WIFI_FIRMWARE		=
MFG_WIFI_TARGET		= $(ARCH)-media-high-mfgtest
DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"

ifneq (${NETAPP_WIFI_SECDMA},y)
WIFI_TARGET_PCIE		= armv7l-extnvm-apdef-stadef
else
WIFI_TARGET_PCIE		= armv7l-extnvm-apdef-stadef-secdma
endif
MFG_WIFI_TARGET_PCIE	= armv7l-extnvm-apdef-stadef-mfgtest-debug

ifeq ($(ATP_BUILD),y)
WIFI_TARGET    		= $(ARCH)-armv7l-reqfw-media-high-p2p-mchan-cfg80211
else
WIFI_TARGET    		= $(ARCH)-armv7l-reqfw-media-high-p2p-mchan
endif

DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"


WIFI_ARGS += CC=${CC} LD=${LD} STRIP=${STRIP} OBJCOPY=$(OBJCOPY) \
	LINUXDIR=${LINUX} \
	TARGETARCH=$(ARCH) LINUXVER=${LINUXVER} BRAND=linux-external-wl \
	STBLINUX=1 \
	FIRMWARE=${WIFI_FIRMWARE} \
	ARCH=${ARCH} \
	WET=1 \
	WLLXIW=0 \
	BRCM_WLAN_IFNAME=wlan%d RPC_RETURN_WAIT_TIMEOUT_MSEC=30000 WL_DNGL_WD=0 \
	NVRAM_TARGET_DIR=../${NETAPP_TARGET_FW_DIR}

ifeq (${WIFI_MFG_SRC_PKG},)
drivers: ${WL}
ifeq ($(findstring nic,$(NETAPP_WIFI_CHIP)),)
	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux ${WIFI_ARGS} ${WIFI_TARGET} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux/obj-${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/usbdev/usbdl ${WIFI_MODULES_OPTS} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}
ifeq (${WIFI_MFG_SRC_PKG},)
	@echo "============ Installing USB Firmware"
	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/4360b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx   ${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/43526b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/43236b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/43238b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/43143b0-bmac/g-p2p-mchan-media/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dongle/rte/wl/builds/43242a1-bmac/ag-nodis/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43242-firmware.bin
endif
endif
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux/obj-${WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_pcie.ko
endif
endif
else
drivers:
endif # MFG source package

drivers_mfg_prep:  ${EPIVERS_MFG} ${WL_MFG}
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${MFG_WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux/obj-${MFG_WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_mfg_pcie.ko
endif
endif
#	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
#	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} BCM_REQUEST_FW=1 ${MFG_WIFI_TARGET} ${SQUASH_OUTPUT}
#	@echo "============ Installing Manufacturing Firmware for ${NETAPP_WIFI_CHIPS}"
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/4360b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx   ${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43526b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43236b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43238b-bmac/ag-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware_mfg.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43143b0-bmac/g-mfgtest/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43242a1-bmac/ag-mfgtest/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43242-firmware.bin

drivers_mfg: drivers_mfg_prep
	${Q_}$(foreach file, $(shell ls ${WIFI_MFG_DRIVER_PKG}/src/wl/linux/obj-${MFG_WIFI_TARGET}-${LINUXVER}/*.ko), cp $(file) ${NETAPP_OUTPUT_MODULES_DIR}/$(notdir $(basename $(file)))_mfg.ko;)
endif # aardvark


##################################
# AARDVARK (MEDIA) #
##################################
ifneq (,$(filter aard%, ${WIFI_DRIVER_VERSION}))
WIFI_FIRMWARE		=
MFG_WIFI_TARGET		= $(ARCH)-media-high-mfgtest
DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"

ifneq (${NETAPP_WIFI_SECDMA},y)
WIFI_TARGET_PCIE		= $(ARCH)-apdef-stadef-p2p
else
WIFI_TARGET_PCIE		= $(ARCH)-apdef-stadef-p2p-secdma
endif
WIFI_ARGS_PCIE			+= BCMEXTNVM=1
MFG_WIFI_TARGET_PCIE	= $(ARCH)-apdef-stadef-mfgtest

ifeq ($(ATP_BUILD),y)
WIFI_TARGET    		= $(ARCH)-reqfw-media-high-p2p-mchan-cfg80211
else
WIFI_TARGET    		= $(ARCH)-reqfw-media-high-p2p-mchan
endif

DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"


WIFI_ARGS += CC=${CC} LD=${LD} STRIP=${STRIP} OBJCOPY=$(OBJCOPY) \
	LINUXDIR=${LINUX} \
	TARGETARCH=$(ARCH) LINUXVER=${LINUXVER} BRAND=linux-external-wl \
	STBLINUX=1 \
	FIRMWARE=${WIFI_FIRMWARE} \
	ARCH=${ARCH} \
	WET=1 \
	WLLXIW=0 \
	BRCM_WLAN_IFNAME=wlan%d RPC_RETURN_WAIT_TIMEOUT_MSEC=30000 WL_DNGL_WD=0 \
	NVRAM_TARGET_DIR=../${NETAPP_TARGET_FW_DIR}

ifeq (${WIFI_MFG_SRC_PKG},)
drivers: ${WL}
ifeq ($(findstring nic,$(NETAPP_WIFI_CHIP)),)
	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} ${WIFI_TARGET} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/wl/linux/obj-${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/usbdev/usbdl ${WIFI_MODULES_OPTS} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}
ifeq (${WIFI_MFG_SRC_PKG},)
	@echo "============ Installing USB Firmware"
	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/4360b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx   ${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/43526b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/43236b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/43238b-bmac/ag-nodis-p2p-mchan-media/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/43143b0-bmac/g-p2p-mchan-media/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/src/dongle/rte/wl/builds/43242a1-bmac/ag-nodis/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43242-firmware.bin
endif
endif
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/wl/linux/obj-${WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_pcie.ko
endif
endif
else
drivers:
endif # MFG source package

drivers_mfg_prep:  ${EPIVERS_MFG} ${WL_MFG}
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${MFG_WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/wl/linux/obj-${MFG_WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_mfg_pcie.ko
endif
endif
	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} BCM_REQUEST_FW=1 ${MFG_WIFI_TARGET} ${SQUASH_OUTPUT}
	@echo "============ Installing Manufacturing Firmware for ${NETAPP_WIFI_CHIPS}"
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/4360b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx   ${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43526b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43236b-bmac/ag-nodis-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43238b-bmac/ag-mfgtest/rtecdc.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware_mfg.bin
#	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43143b0-bmac/g-mfgtest/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/dongle/rte/wl/builds/43242a1-bmac/ag-mfgtest/rtecdc.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43242-firmware.bin

drivers_mfg: drivers_mfg_prep
	${Q_}$(foreach file, $(shell ls ${WIFI_MFG_DRIVER_PKG}/src/wl/linux/obj-${MFG_WIFI_TARGET}-${LINUXVER}/*.ko), cp $(file) ${NETAPP_OUTPUT_MODULES_DIR}/$(notdir $(basename $(file)))_mfg.ko;)
endif # AARDVARK


##################################
# BISON (BMAC/NIC) #
##################################
ifneq (,$(filter bison04t%, ${WIFI_DRIVER_VERSION}))

WIFI_FIRMWARE		=
MFG_WIFI_TARGET		= $(TARGET_ARCH)-media-high-mfgtest-debug
DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"

ifneq (${NETAPP_WIFI_SECDMA},y)
WIFI_TARGET_PCIE		?= $(TARGET_ARCH)-extnvm-apdef-stadef-p2p
MFG_WIFI_TARGET_PCIE	?= $(TARGET_ARCH)-extnvm-apdef-stadef-mfgtest-debug
else
WIFI_TARGET_PCIE		?= $(TARGET_ARCH)-extnvm-apdef-stadef-p2p-secdma
MFG_WIFI_TARGET_PCIE	?= $(TARGET_ARCH)-extnvm-apdef-stadef-mfgtest-debug-secdma
endif
WIFI_ARGS_PCIE			+= BCMEXTNVM=1

DNGL_MFG_IMAGE_NAME	="${NETAPP_WIFI_CHIP}-bmac/ag-mfgtest"
ifeq ($(ATP_BUILD),y)
WIFI_TARGET    		= $(ARCH)-reqfw-media-high-p2p-mchan-cfg80211
else
WIFI_TARGET    		= $(ARCH)-reqfw-media-high-p2p-mchan
endif

WIFI_ARGS += CC=${CC} LD=${LD} STRIP=${STRIP} OBJCOPY=$(OBJCOPY) \
	LINUXDIR=${LINUX} \
	TARGETARCH=$(TARGET_ARCH) LINUXVER=${LINUXVER} BRAND=linux-external-media \
	STBLINUX=1 \
	FIRMWARE=${WIFI_FIRMWARE} \
	WLLXIW=0 \
	USBSHIM=0\
	BRCM_WLAN_IFNAME=wlan%d RPC_RETURN_WAIT_TIMEOUT_MSEC=30000 WL_DNGL_WD=0 \
	NVRAM_TARGET_DIR=${NETAPP_TARGET_FW_DIR} \
        CHIPVER=${CHIPVER}

drivers: ${WL}
ifeq ($(findstring nic,$(NETAPP_WIFI_CHIP)),)
	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} ${WIFI_TARGET} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/wl/linux/obj-${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/usbdev/usbdl ${WIFI_MODULES_OPTS} ${SQUASH_OUTPUT}
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/mips/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/mips/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
ifeq (${WIFI_MFG_SRC_PKG},)
	@echo "============ Installing USB Firmware"
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4360b-bmac/ag-nodis-p2p-mchan-media.bin.trx 				${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43526b-bmac/ag-p2p-mchan-media.bin.trx 					${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43236b-bmac/ag-nodis-p2p-mchan-media.bin.trx 				${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43242a1-bmac/ag-nodis-p2p-mchan-media-srvsdb-vusb.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43143b0-bmac/g-nodis-p2p-mchan-media.bin.trx				${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
endif
endif
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_DRIVER_PKG}/src/wl/linux/obj-${WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_pcie.ko
endif
endif

drivers_mfg:  ${EPIVERS_MFG}
ifeq ($(CONFIG_PCI),y)
ifneq ($(WIFI_PCIE_SUPPORT),n)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} $(WIFI_ARGS_PCIE) ${MFG_WIFI_TARGET_PCIE} ${SQUASH_OUTPUT}
	${Q_}cp ${WIFI_MFG_DRIVER_PKG}/src/wl/linux/obj-${MFG_WIFI_TARGET_PCIE}-${LINUXVER}/wl.ko ${NETAPP_OUTPUT_MODULES_DIR}/wl_mfg_pcie.ko
endif
endif
#	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} MFG Driver(s) for ${LINUXVER}"
#	${Q_}$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ${WIFI_ARGS} BCM_REQUEST_FW=1 ${MFG_WIFI_TARGET} ${SQUASH_OUTPUT}
#	@echo "============ Installing Manufacturing Firmware for ${NETAPP_WIFI_CHIPS}"
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4360b-bmac/ag-nodis-p2p-mchan-media.bin.trx 				${NETAPP_OUTPUT_FW_DIR}/bcm4360-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43526b-bmac/ag-p2p-mchan-media.bin.trx 					${NETAPP_OUTPUT_FW_DIR}/bcm43526-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43236b-bmac/ag-nodis-p2p-mchan-media.bin.trx 				${NETAPP_OUTPUT_FW_DIR}/bcm43236-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43242a1-bmac/ag-nodis-p2p-mchan-media-srvsdb-vusb.bin.trx  ${NETAPP_OUTPUT_FW_DIR}/bcm43238-firmware.bin
#	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43143b0-bmac/g-nodis-p2p-mchan-media.bin.trx				${NETAPP_OUTPUT_FW_DIR}/bcm43143-firmware.bin
endif # BISON


##################################
#        D H D                   #
#   dhd 1_3x                    #
##################################
ifneq (,$(filter dhd_rel_1_%, ${WIFI_DRIVER_VERSION}))

ifeq ($(ANDROID_ENABLE_DHD),y)
WIFI_TARGET                     = dhd-cdc-usb-android-media-cfg80211-comp-mfp
WIFI_TARGET_PCIE                ?= dhd-msgbuf-pciefd-android-media-cfg80211-mfp
WIFI_MODULES_OPTS				+= \
	LIBS="-Wl,-L$(ANDROID_TOP)/out/target/product/bcm_platform/obj/lib,-dy,-lusb2 -Wl,-L$(ANDROID_TOP)/out/target/product/bcm_platform/obj/lib,-dy,-lusb-compat" \
	GLDFLAGS="-shared"
EXTERNAL_CFLAGS					= "-I$(LIBUSB_PATH) -marm -fPIC -mfloat-abi=hard"
else
ifeq (${NETAPP_WIFI_SECDMA},y)
WIFI_TARGET_SECDMA 				?= -secdma
endif
ifeq (${NETAPP_WIFI_CFG80211},y)
WIFI_TARGET_CFG80211				?= -cfg80211
endif
ifeq (,$(filter dhd_rel_1_2%, ${WIFI_DRIVER_VERSION}))
WIFI_TARGET_ARCH 				?= -$(TARGET_ARCH)
endif
ifneq ($(findstring 43566, $(NETAPP_WIFI_CHIP)),)
WIFI_TARGET                     ?= dhd-cdc-usb-gpl$(WIFI_TARGET_CFG80211)$(WIFI_TARGET_ARCH)
else
WIFI_TARGET                     = dhd-cdc-usb-comp-gpl$(WIFI_TARGET_CFG80211)$(WIFI_TARGET_ARCH)
endif
WIFI_TARGET_PCIE		?= dhd-msgbuf-pciefd-mfp$(WIFI_TARGET_SECDMA)$(WIFI_TARGET_CFG80211)$(WIFI_TARGET_ARCH)


endif

ifneq ($(findstring mfgtest, $(NETAPP_WIFI_FIRMWARE)),)
	NETAPP_WIFI_FIRMWARE_NAME ?= mfg_firmware
else
	NETAPP_WIFI_FIRMWARE_NAME ?= firmware
endif

drivers: ${WL} $(DHD)
	@echo "============ Building USB  ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/dhd/linux ${WIFI_TARGET} \
	LINUXDIR=${LINUX} TARGETARCH=$(ARCH) WET=1 WLTEST=${WLTEST} \
	CROSS_COMPILE=${CROSS_COMPILE} \
	BRCM_WLAN_IFNAME=wlan%d \
	ARCH=${ARCH} \
	LINUXVER=${LINUXVER} USBSHIM=1 WL_NVRAM_FILE=1 CC=${CC} STRIP=${STRIP} ${SQUASH_OUTPUT}
ifneq ($(ANDROID_ENABLE_DHD),y)
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/usbdev/usbdl ${WIFI_MODULES_OPTS} EXTERNAL_CFLAGS=$(EXTERNAL_CFLAGS) ${SQUASH_OUTPUT}
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/obj/${TARGETARCH}/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/usbdev/usbdl/bcmdl ${NETAPP_OUTPUT_BIN_DIR}; \
	fi
endif
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET}-${KERNELVERSION}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}; \
	fi
ifeq ($(NETAPP_WIFI_CHIP),43569a0)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a0-roml/usb-ag-mfgtest-pktctx-dmatxrc-txbf-vusb.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43569-mfg_firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a0-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-proptxstatus-vusb.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43569-firmware.bin
else
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a2-roml/usb-ag-mfgtest-pktctx-dmatxrc-txbf-vusb.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43569-mfg_firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43569-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a2-roml/usb-ag-mfgtest-pktctx-dmatxrc-txbf-vusb.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43566-mfg_firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx ${NETAPP_OUTPUT_FW_DIR}/bcm43566-firmware.bin
endif
ifeq ($(CONFIG_PCI),y)
	@echo "============ Building PCIe ${WIFI_DRIVER_VERSION} Driver(s) for ${LINUXVER}"
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG}/src/dhd/linux ${WIFI_TARGET_PCIE} \
	LINUXDIR=${LINUX} TARGETARCH=$(ARCH) CROSS_COMPILE=${CROSS_COMPILE} \
	BRCM_WLAN_IFNAME=wln%d \
	ARCH=${ARCH} \
	LINUXVER=${LINUXVER} CC=${CC} STRIP=${STRIP} ${SQUASH_OUTPUT}
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET_PCIE}-${LINUXVER} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET_PCIE}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_MODULES_DIR}/dhd_pcie.ko; \
	fi
	${Q_}if [ -d ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET_PCIE}-${KERNELVERSION} ] ; then \
		cp ${WIFI_DRIVER_PKG}/src/dhd/linux/${WIFI_TARGET_PCIE}-${KERNELVERSION}/dhd.ko ${NETAPP_OUTPUT_MODULES_DIR}/dhd_pcie.ko; \
	fi

#ifneq (,$(filter dhd_rel_1_2%, ${WIFI_DRIVER_VERSION}))
ifeq ($(NETAPP_WIFI_CHIP),43570a0)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43570a0-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-mfp-sr-proptxstatus-vsdb-pktfilter-wowlpf-ampduhostreorder-keepalive.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43570-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43570a0-roml/pcie-ag-splitrx-txbf-p2p-mfgtest-seqcmds-phydbg.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43570-mfg_firmware.bin
else
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43570-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43570a2-roml/pcie-ag-splitrx-txbf-p2p-mfgtest-seqcmds-phydbg.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43570-mfg_firmware.bin
endif
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-btcxgci48bits.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43602-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43602a1-roml/pcie-ag-mfgtest-pktctx-splitrx-amsdutx-txbf-p2p-mchan-sr-proptxstatus-btcxgci48bits.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43602-mfg_firmware.bin
ifeq ($(NETAPP_WIFI_CHIP),4359b0)
	${Q_}cp ${WIFI_DRIVER_PKG}/firmware/4359b0-roml/threadx-pcie-ag-p2p-pno-pktfilter-keepalive-mchan-wl11u-pktctx-ccx-ve-mfp-okc-splitrx-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder-aoe-ndoe-btcdyn-die5-norsdb-tdls-sstput-wfds-pwrofs-pwrstats-lpc-olpc/rtecdc.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4355-firmware.bin
else
	${Q_}cp ${WIFI_DRIVER_PKG}/firmware/4359b1-roml/pcie-wl11u-ve-mfp-okc-idsup-idauth-sr-die3-betdls-pwrofs-gscan-wepso-sarctrl-linkstat-wnm-pfn-hs20sta-ulb-pwrstats-lpc-olpc-mobfd-txbf-mimopscan-apcs-pspretend-rcc-slna-ccx-noccxaka-clm_ss_mimo-tbow-wapi.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4355-firmware.bin
endif
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-p2p-mchan-sr-txbf-mfp-proptxstatus-wowlpf-pktfilter-ampduhostreorder-keepalive-chkd2hdma-ringer-dmaindex16-mfgtest.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365c0-mfg_firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-ampduhostreorder-keepalive-chkd2hdma-ringer-dmaindex16.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365c0-firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366b1-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-mfp-proptxstatus-wowlpf-pktfilter-ampduhostreorder-keepalive-chkd2hdma-ringer-dmaindex16-mfgtest.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365-mfg_firmware.bin
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366b1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-wowlpf-pktfilter-ampduhostreorder-chkd2hdma-ringer-dmaindex16-keepalive.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365-firmware.bin

ifneq (${NETAPP_WIFI_FIRMWARE},)
	@echo "==${NETAPP_WIFI_FIRMWARE_NAME} $(NETAPP_WIFI_CHIP) 43570-$(findstring 43570, $(NETAPP_WIFI_CHIP)) 43602-$(findstring 43602, $(NETAPP_WIFI_CHIP)) \
		4365-$(findstring 4365, $(NETAPP_WIFI_CHIP)) 4366-$(findstring 4366, $(NETAPP_WIFI_CHIP))=="
ifneq ($(findstring 43570, $(NETAPP_WIFI_CHIP)),)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43570a2-roml/${NETAPP_WIFI_FIRMWARE}.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43570-${NETAPP_WIFI_FIRMWARE_NAME}.bin
endif
ifneq ($(findstring 43602, $(NETAPP_WIFI_CHIP)),)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/43602a1-roml/${NETAPP_WIFI_FIRMWARE}.bin ${NETAPP_OUTPUT_FW_DIR}/bcm43602-${NETAPP_WIFI_FIRMWARE_NAME}.bin
endif
ifneq ($(findstring 4365, $(NETAPP_WIFI_CHIP)),)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366b1-roml/${NETAPP_WIFI_FIRMWARE}.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365-${NETAPP_WIFI_FIRMWARE_NAME}.bin
endif
ifneq ($(findstring 4366, $(NETAPP_WIFI_CHIP)),)
	${Q_}-cp ${WIFI_DRIVER_PKG}/firmware/4366b1-roml/${NETAPP_WIFI_FIRMWARE}.bin ${NETAPP_OUTPUT_FW_DIR}/bcm4365-${NETAPP_WIFI_FIRMWARE_NAME}.bin
endif
endif

#endif # dhd_rel_1_2
endif # CONFIG_PCI
endif # dhd_rel_1_201

##################################
#        A P                   #
##################################
drivers_nic:
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router/emf/emf ${NETAPP_WIFI_AP_BLDARGS} WLARCH=arm TOOLCHAIN=${TOOLCHAIN_AP} LINUXVER=${LINUXVER}
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router/emf/igs ${NETAPP_WIFI_AP_BLDARGS} WLARCH=arm TOOLCHAIN=${TOOLCHAIN_AP} LINUXVER=${LINUXVER}
ifeq (${WIFI_DRIVER_AP1},drivers_nic)
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/wl/linux  ${NETAPP_WIFI_AP_BLDARGS} LINUXVER=${LINUXVER} \
		KBUILD_EXTRA_SYMBOLS=${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers \
		${NETAPP_WIFI_AP_BLDCFG_NIC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/wl/linux/obj-${NETAPP_WIFI_AP_BLDCFG_NIC}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_AP_DIR}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/wl/exe TARGETENV=linuxarm_le WLARCH=arm CC=${CC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/wl/exe/wlarm_le ${NETAPP_OUTPUT_AP_DIR}/wl
endif # WIFI_DRIVER_AP1
ifeq (${WIFI_DRIVER_AP2},drivers_nic)
ifneq (${WIFI_DRIVER_PKG_AP2},${WIFI_DRIVER_PKG_AP1})
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP2}/src/wl/linux  ${NETAPP_WIFI_AP_BLDARGS} LINUXVER=${LINUXVER} \
		KBUILD_EXTRA_SYMBOLS=${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers \
		${NETAPP_WIFI_AP_BLDCFG_NIC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP2}/src/wl/linux/obj-${NETAPP_WIFI_AP_BLDCFG_NIC}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_AP_DIR}
endif
endif # WIFI_DRIVER_AP2

drivers_fd:
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router/emf/emf ${NETAPP_WIFI_AP_BLDARGS} WLARCH=arm TOOLCHAIN=${TOOLCHAIN_AP} LINUXVER=${LINUXVER}
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router/emf/igs ${NETAPP_WIFI_AP_BLDARGS} WLARCH=arm TOOLCHAIN=${TOOLCHAIN_AP} LINUXVER=${LINUXVER}
ifeq (${WIFI_DRIVER_AP1},drivers_fd)
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/dhd/linux  ${NETAPP_WIFI_AP_BLDARGS} \
		KBUILD_EXTRA_SYMBOLS=${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers \
		${NETAPP_WIFI_AP_BLDCFG_FD} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/dhd/linux/${NETAPP_WIFI_AP_BLDCFG_FD}-${KERNELVERSION}/*.ko ${NETAPP_OUTPUT_AP_DIR}
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/dhd/exe CC=${CC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/dhd/exe/dhd ${NETAPP_OUTPUT_AP_DIR}/dhd
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/wl/exe TARGETENV=linuxarm_le WLARCH=arm CC=${CC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/wl/exe/wlarm_le ${NETAPP_OUTPUT_AP_DIR}/wl
endif # WIFI_DRIVER_AP1
ifeq (${WIFI_DRIVER_AP2},drivers_fd)
ifneq (${WIFI_DRIVER_PKG_AP2},${WIFI_DRIVER_PKG_AP1})
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP2}/src/dhd/linux  ${NETAPP_WIFI_AP_BLDARGS} \
		KBUILD_EXTRA_SYMBOLS=${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/Module.symvers \
		${NETAPP_WIFI_AP_BLDCFG_FD} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP2}/src/dhd/linux/${NETAPP_WIFI_AP_BLDCFG_FD}-${LINUXVER}/*.ko ${NETAPP_OUTPUT_AP_DIR}
ifneq (${WIFI_DRIVER_AP1},drivers_fd)
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/dhd/exe CC=${CC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/dhd/exe/dhd ${NETAPP_OUTPUT_AP_DIR}/dhd
	${Q_}$(MAKE) -C ${WIFI_DRIVER_PKG_AP1}/src/wl/exe TARGETENV=linuxarm_le WLARCH=arm CC=${CC} ${SQUASH_OUTPUT}
	cp ${WIFI_DRIVER_PKG_AP1}/src/wl/exe/wlarm_le ${NETAPP_OUTPUT_AP_DIR}/wl
endif # WIFI_DRIVER_AP1
endif # WIFI_DRIVER_PKG_AP2 - WIFI_DRIVER_PKG_AP1
endif # WIFI_DRIVER_PKG_AP2
ap_app:
ifneq (${WIFI_APP_PKG_AP},)
ifneq (,$(filter drivers_nic,$(WIFI_DRIVER_AP1) $(WIFI_DRIVER_AP2)))
	cp ${WIFI_APP_PKG_AP}/src/router/config/defconfig-stbap-dhd ${WIFI_APP_PKG_AP}/src/router/.config
	echo '\n' | make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_BLDARGS} oldconfig
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_APP_BLDARGS} apps
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_APP_BLDARGS} apps_install
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/etc
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/sbin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/usr/sbin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/www
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/bin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_NIC}/lib/modules
	cp ${WIFI_APP_PKG_AP}/src/shared/nvram/stbap_nvram.txt ${NETAPP_OUTPUT_AP_DIR_NIC}/etc/nvrams_ap_default.txt
ifneq (,$(filter drivers_nic,$(WIFI_DRIVER_AP1)))
	cat ${WIFI_APP_PKG_AP}/src/router/rc/stbap_init.sh | sed 's/WIFI_DRIVER_MODE=__@@__/WIFI_DRIVER_MODE=nic/' | sed 's/WIFI_CHIP=__@@__/WIFI_CHIP=${WIFI_CHIPSET1}/' > ${NETAPP_OUTPUT_AP_DIR_NIC}/sbin/stbap_init.sh
endif # WIFI_DRIVER_AP1
ifneq (,$(filter drivers_nic,$(WIFI_DRIVER_AP2)))
	cat ${WIFI_APP_PKG_AP}/src/router/rc/stbap_init.sh | sed 's/WIFI_DRIVER_MODE=__@@__/WIFI_DRIVER_MODE=nic/' | sed 's/WIFI_CHIP=__@@__/WIFI_CHIP=${WIFI_CHIPSET2}/' > ${NETAPP_OUTPUT_AP_DIR_NIC}/sbin/stbap_init.sh
endif # WIFI_DRIVER_AP2
	cp ${WIFI_APP_PKG_AP}/src/router/rc/stbhotplug ${NETAPP_OUTPUT_AP_DIR_NIC}/sbin
	cp -r ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/sbin/rc ${NETAPP_OUTPUT_AP_DIR_NIC}/sbin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/usr/sbin/* ${NETAPP_OUTPUT_AP_DIR_NIC}/usr/sbin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/bin/* ${NETAPP_OUTPUT_AP_DIR_NIC}/bin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/usr/lib/* ${NETAPP_OUTPUT_AP_DIR_NIC}/lib
	cp -r ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/www ${NETAPP_OUTPUT_AP_DIR_NIC}/
	cp  ${NETAPP_OUTPUT_AP_DIR}/wl.ko ${NETAPP_OUTPUT_AP_DIR_NIC}/lib/modules/
	cp ${WIFI_APP_PKG_AP}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko ${NETAPP_OUTPUT_AP_DIR_NIC}/lib/modules/
	cp ${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko ${NETAPP_OUTPUT_AP_DIR_NIC}/lib/modules/
	cp  ${NETAPP_OUTPUT_AP_DIR}/wl ${NETAPP_OUTPUT_AP_DIR_NIC}/usr/sbin/
	cp  ${NETAPP_TOP}/wlan/nvram/defaults/nvram.txt  ${NETAPP_OUTPUT_AP_DIR_NIC}/lib/modules/nvram.txt
endif # drivers_nic

ifneq (,$(filter drivers_fd,$(WIFI_DRIVER_AP1) $(WIFI_DRIVER_AP2)))
	cp  ${WIFI_APP_PKG_AP}/src/router/config/defconfig-stbap ${WIFI_APP_PKG_AP}/src/router/.config
	echo '\n' | make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_BLDARGS} oldconfig
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_APP_BLDARGS} apps
	${Q_}make -C ${WIFI_APP_PKG_AP}/src/router ${NETAPP_WIFI_AP_APP_BLDARGS} apps_install
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/etc
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/sbin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/usr/sbin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/www
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/bin
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/lib/modules
	mkdir -p ${NETAPP_OUTPUT_AP_DIR_FD}/lib/firmware/brcm
	cp  ${WIFI_APP_PKG_AP}/src/shared/nvram/stbap_nvram.txt ${NETAPP_OUTPUT_AP_DIR_FD}/etc/nvrams_ap_default.txt
ifneq (,$(filter drivers_fd,$(WIFI_DRIVER_AP1)))
	cat ${WIFI_APP_PKG_AP}/src/router/rc/stbap_init.sh \
		| sed 's/WIFI_DRIVER_MODE=__@@__/WIFI_DRIVER_MODE=fd/' \
		| sed 's/WIFI_CHIP=__@@__/WIFI_CHIP=${WIFI_CHIPSET1}/' \
		| sed '/WIFI_NVRAM_WL0_PATH=\/lib\/firmware\/brcm\/bcm9${WIFI_CHIPSET1}/c\\t\t\t\tWIFI_NVRAM_WL0_PATH=/lib/firmware/brcm/bcm${WIFI_CHIPSET1}.nvm' \
		| sed '/WIFI_NVRAM_WL1_PATH=\/lib\/firmware\/brcm\/bcm9${WIFI_CHIPSET1}/c\\t\t\t\tWIFI_NVRAM_WL1_PATH=/lib/firmware/brcm/bcm${WIFI_CHIPSET1}_wl1.nvm' \
		> ${NETAPP_OUTPUT_AP_DIR_FD}/sbin/stbap_init.sh
	cat  ${NETAPP_TOP}/wlan/nvram/defaults/bcm${WIFI_CHIPSET1}.nvm \
		| sed '/macaddr=/iwlunit=0' \
		| sed '/macaddr=/c\macaddr=00:90:4C:0E:60:22' \
		> ${NETAPP_OUTPUT_AP_DIR_FD}/lib/firmware/brcm/bcm${WIFI_CHIPSET1}.nvm
	cp  ${WIFI_DRIVER_PKG_AP1}/firmware/${WIFI_FIRMWARE_AP1}  ${NETAPP_OUTPUT_AP_DIR_FD}/lib/firmware/brcm/bcm${WIFI_CHIPSET1}-firmware.bin
endif # WIFI_DRIVER_AP1
ifneq (,$(filter drivers_fd,$(WIFI_DRIVER_AP2)))
	cat ${WIFI_APP_PKG_AP}/src/router/rc/stbap_init.sh \
		| sed 's/WIFI_DRIVER_MODE=__@@__/WIFI_DRIVER_MODE=fd/' \
		| sed 's/WIFI_CHIP=__@@__/WIFI_CHIP=${WIFI_CHIPSET2}/' \
		| sed '/WIFI_NVRAM_WL0_PATH=\/lib\/firmware\/brcm\/bcm9${WIFI_CHIPSET2}/c\\t\t\t\tWIFI_NVRAM_WL0_PATH=/lib/firmware/brcm/bcm${WIFI_CHIPSET2}.nvm' \
		| sed '/WIFI_NVRAM_WL1_PATH=\/lib\/firmware\/brcm\/bcm9${WIFI_CHIPSET2}/c\\t\t\t\tWIFI_NVRAM_WL1_PATH=/lib/firmware/brcm/bcm${WIFI_CHIPSET2}_wl1.nvm' \
		> ${NETAPP_OUTPUT_AP_DIR_FD}/sbin/stbap_init.sh
	cat  ${NETAPP_TOP}/wlan/nvram/defaults/bcm${WIFI_CHIPSET2}.nvm \
		| sed '/macaddr=/iwlunit=1' \
		| sed '/macaddr=/c\macaddr=00:90:4C:0E:60:33' \
		> ${NETAPP_OUTPUT_AP_DIR_FD}/lib/firmware/brcm/bcm${WIFI_CHIPSET2}_wl1.nvm
	cp -v ${WIFI_DRIVER_PKG_AP1}/firmware/${WIFI_FIRMWARE_AP2}  ${NETAPP_OUTPUT_AP_DIR_FD}/lib/firmware/brcm/bcm${WIFI_CHIPSET2}-firmware.bin
endif # WIFI_DRIVER_AP2
	cp  ${WIFI_APP_PKG_AP}/src/router/rc/stbhotplug ${NETAPP_OUTPUT_AP_DIR_FD}/sbin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/sbin/rc ${NETAPP_OUTPUT_AP_DIR_FD}/sbin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/usr/sbin/* ${NETAPP_OUTPUT_AP_DIR_FD}/usr/sbin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/bin/* ${NETAPP_OUTPUT_AP_DIR_FD}/bin
	cp  ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/usr/lib/* ${NETAPP_OUTPUT_AP_DIR_FD}/lib
	cp -r ${WIFI_APP_PKG_AP}/src/router/arm-glibc/target/www ${NETAPP_OUTPUT_AP_DIR_FD}/

	cp  ${NETAPP_OUTPUT_AP_DIR}/dhd.ko ${NETAPP_OUTPUT_AP_DIR_FD}/lib/modules/
	cp ${WIFI_APP_PKG_AP}/src/router/emf/emf/obj-emf-${LINUXVER}/emf.ko ${NETAPP_OUTPUT_AP_DIR_FD}/lib/modules/
	cp ${WIFI_APP_PKG_AP}/src/router/emf/igs/obj-igs-${LINUXVER}/igs.ko ${NETAPP_OUTPUT_AP_DIR_FD}/lib/modules/
	cp  ${NETAPP_OUTPUT_AP_DIR}/dhd ${NETAPP_OUTPUT_AP_DIR_FD}/usr/sbin/
	cp  ${NETAPP_OUTPUT_AP_DIR}/wl ${NETAPP_OUTPUT_AP_DIR_FD}/usr/sbin/
endif # drivers_fd
endif # WIFI_APP_PKG_AP

${OUPUT_DIR}:
	${Q_}mkdir -p ${OUPUT_DIR}/bcmcrypto
	${Q_}mkdir -p ${OUPUT_DIR}/sta
	${Q_}mkdir -p ${OUPUT_DIR}/enrollee
	${Q_}mkdir -p ${OUPUT_DIR}/registrar
	${Q_}mkdir -p ${OUPUT_DIR}/shared
	${Q_}mkdir -p ${OUPUT_DIR}/ap

endif # LINUX
endif # Broadcom Chipset
############################################################################
release-clean: clean
	${Q_}rm -rf ${WIFI_DRIVER_PKG} $(OUPUT_DIR) ${WIFI_MFG_DRIVER_PKG} ${WIFI_DRIVER_PKG_AP} ${WIFI_DRIVER_PKG_AP1} ${WIFI_DRIVER_PKG_AP2}

clean:
ifneq ($(wildcard ${WIFI_DRIVER_PKG}), )
	${Q_}if [ -e ${P2PLIB_DIR}/linux/sampleapp ] ; then \
		$(MAKE) -C ${P2PLIB_DIR}/linux/sampleapp ASD=0 CC=${CC} \
			DEBUG=1 TARGETARCH=${TARGETARCH} STRIP=${STRIP} clean ${SQUASH_OUTPUT};\
	fi
	${Q_}if [ -e ${WFDCAPDIE_DIR} ] ; then \
		$(MAKE) -C ${WFDCAPDIE_DIR} BLDTYPE=debug CC=$(CC) TARGETARCH=${TARGETARCH} clean ${SQUASH_OUTPUT};\
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux ] ; then \
		$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/linux TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/linux ] ; then \
		$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/linux TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/exe ] ; then \
		$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/dhd/exe TARGETARCH=${TARGETARCH} clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/exe ] ; then \
		$(MAKE) -C ${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src/wl/exe TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/wl/linux ] ; then \
		$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/linux TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_MFG_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux ] ; then \
		$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/dhd/linux TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_MFG_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/dhd/exe ] ; then \
		$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/dhd/exe TARGETARCH=${TARGETARCH} clean ${SQUASH_OUTPUT}; \
	fi
	${Q_}if [ -e ${WIFI_MFG_DRIVER_PKG}/src/wl/exe ] ; then \
		$(MAKE) -C ${WIFI_MFG_DRIVER_PKG}/src/wl/exe TARGETARCH=${TARGETARCH} SRCBASE=${WIFI_MFG_DRIVER_PKG}/src clean ${SQUASH_OUTPUT}; \
	fi

	${Q_}rm -rf *.marker
	${Q_}-$(MAKE) -C ${WPSCLI_DIR}/linux ${WIFI_MODULES_OPTS} SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src clean ${SQUASH_OUTPUT}
	${Q_}if [ -e ${WPSENR_DIR} ] ; then \
		cd ${WPSENR_DIR} && $(MAKE) CC=${CC} STRIP=${STRIP} OUPUT_DIR=${OUPUT_DIR} EXTERNAL_OPENSSL=${EXTERNAL_OPENSSL} V=1 SRCBASE=${WIFI_DRIVER_PKG}$(WIFI_DIR_PREFIX)/src -f wps_enr_app.mk ${SQUASH_OUTPUT} clean;\
	fi
	${Q_}rm -f ${F_OBJS_COMPILED} *.d
	${Q_}rm -rf ${OUPUT_DIR}
endif

ifneq ($(wildcard ${NETAPP_OUTPUT_AP_DIR}), )
	${Q_}rm -rf ${NETAPP_OUTPUT_AP_DIR}
endif

distclean: clean release-clean
	${Q_}rm -rf ${NETAPP_OUTPUT_FW_DIR} ${WIFI_DRIVER_INS}


$(WIFI_DRIVER_PKG):
	@echo "============ Extracting WiFi Driver tarball ${WIFI_DRIVER_VERSION}" ;
	${Q_}cd ${NETAPPPLATFORM_OBJ_DIR}; tar xf ${WIFI_SRC_PKG}
	${Q_}if [ -f ${WIFI_DRIVER_PATCH} ]; then \
		cd $(WIFI_DRIVER_PKG) && patch -p1 < ${WIFI_DRIVER_PATCH} ${SQUASH_OUTPUT}; \
	fi
ifeq ($(NETAPP_STRIP_WIFI_DRIVER),y)
	${Q_}if [ -f ${WIFI_DRIVER_STRIP_PATCH} ]; then \
		cd $(WIFI_DRIVER_PKG) && patch -p1 < ${WIFI_DRIVER_STRIP_PATCH} ${SQUASH_OUTPUT}; \
	fi
endif

$(WIFI_MFG_DRIVER_PKG):
	@echo "============ Extracting MFG WiFi Driver tarball ${WIFI_DRIVER_VERSION}";
	${Q_}cd ${NETAPPPLATFORM_OBJ_DIR}; tar xf ${WIFI_MFG_SRC_PKG}
	${Q_}if [ -f ${WIFI_MFG_DRIVER_PATCH} ]; then \
		cd $(WIFI_MFG_DRIVER_PKG) && patch -t -p1 < ${WIFI_MFG_DRIVER_PATCH} ${SQUASH_OUTPUT}; \
	fi

${WIFI_DRIVER_PKG_AP1}:
ifneq ($(wildcard ${WIFI_SRC_PKG_AP1}),)
		@echo "============ Extracting WiFi Driver tarball ${WIFI_DRIVER_VERSION_AP1}";
		${Q_}cd $(NETAPPPLATFORM_OBJ_DIR); tar xf ${WIFI_SRC_PKG_AP1}
endif # WIFI_SRC_PKG_AP1


${WIFI_DRIVER_PKG_AP2}:
ifneq ($(wildcard ${WIFI_SRC_PKG_AP2}),)
		@echo "============ Extracting WiFi Driver tarball ${WIFI_DRIVER_VERSION_AP2}";
		${Q_}cd $(NETAPPPLATFORM_OBJ_DIR); tar xf ${WIFI_SRC_PKG_AP2}
endif # WIFI_SRC_PKG_AP2
${WIFI_APP_PKG_AP}:
ifneq ($(wildcard ${WIFI_SRC_PKG_AP}),)
		@echo "============ Extracting WiFi AP tarball ${WIFI_APP_VERSION_AP}";
		${Q_}cd ${NETAPPPLATFORM_OBJ_DIR}; tar xf ${WIFI_SRC_PKG_AP}
endif # WIFI_SRC_PKG_AP
debug:
	@echo "TARGETENV=${TARGETENV}"
	@echo "WIFI_SRC_PKG=${WIFI_SRC_PKG} HAS_WIFI_BUILT=${HAS_WIFI_BUILT}"
	@echo "SRCBASE=${WIFI_DRIVER_PKG}/src"
	@echo "WIFI_MODULES_OPTS=${WIFI_MODULES_OPTS}"
	@echo "F_INSTALL_LIBS=${F_INSTALL_LIBS}"
	@echo "F_PUBLIC_LIBS=${F_PUBLIC_LIBS}"

ifneq (${WIFI_MFG_SRC_PKG},)
DRIVER_MFG_TARGET = drivers_mfg
endif


install: ${WIFI_DRIVER_PKG} ${WIFI_MFG_DRIVER_PKG} ${OUPUT_DIR} drivers ${DRIVER_MFG_TARGET} ${F_PUBLIC_LIBS}
	${Q_}$(MAKE) header-files ${SQUASH_OUTPUT}
ap: ${WIFI_DRIVER_PKG_AP1}  ${WIFI_DRIVER_PKG_AP2} ${WIFI_APP_PKG_AP} drivers_nic drivers_fd ap_app

header-files: ${F_INSTALLED_INCS}

examples: install ${WIFI_EXAMPLES}

${F_INSTALLED_INCS}: ${NETAPP_OUTPUT_INC_DIR}/%.h : %.h
	${Q_}install  -m 0664 $< $@

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


vpath %.h ${INC_DIRS}

############################################################################
#                             DEP FILES, PHONY
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
-include *.d
.PHONY: ${NETAPP_SHARED_LIBS} ${NETAPP_STATIC_LIBS} ${WPSENR_APP} all drivers unpack build distclean clean install
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##
