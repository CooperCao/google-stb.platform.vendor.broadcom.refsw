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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
#
############################################################################
# Disable unused features in Android Environment
ifeq ($(ANDROID_ENABLE_DHD),y)
NETAPP_BRIDGING=n
NETAPP_VOICE_RECOGNITION=n
NETAPP_HOTPLUG=n
NETAPP_ZEROCONF=n
NETAPP_IEEE1905=n
NETAPP_DATABASE=n
NETAPP_SSDP=n
NETAPP_DHCP=n
NETAPP_NEXUS=n
endif


################################################################################
# 					B L U E T O O T H   	S U P O R T
# Enable Bluetooth if the Bluetooth source code is available
################################################################################
ifneq (${NETAPP_BLUETOOTH},n)
BT_CHIPSET			= broadcom
BT_DIR				= ${NETAPP_TOP}/bluetooth
BT_CHIPSET_PKG_TYPE	= .tgz
#BT package for 15.2
#BT_PKG_VERSION		?= bsa0106_01.52.00-netapp-v1
#BT package for 15.3
#BT_PKG_VERSION		?= bsa0107_00.06.00-netapp-v1
#BT package for 15.4
BT_PKG_VERSION		?= bsa0107_00.12.00-netapp-v1

BT_CHIPSET_DIR      := ${BT_DIR}/${BT_CHIPSET}

BT_CHIPSET_PKG		= $(NETAPPPLATFORM_OBJ_DIR)/${BT_PKG_VERSION}
HAS_BLUETOOTH_BUILT	:= $(wildcard ${NETAPP_OUTPUT_LIB_DIR}/libbsa.so)
BLUETOOTH_SRC		:= ${BT_CHIPSET_DIR}/${BT_PKG_VERSION}${BT_CHIPSET_PKG_TYPE}
HAS_BLUETOOTH_SRC	:= $(wildcard ${BLUETOOTH_SRC})

ifneq (,${HAS_BLUETOOTH_SRC})
HAS_BLUETOOTH			= ${HAS_BLUETOOTH_SRC}
else
ifneq (,${HAS_BLUETOOTH_BUILT})
HAS_BLUETOOTH			= ${HAS_BLUETOOTH_BUILT}
endif #HAS_BLUETOOTH_BUILT
endif #HAS_BLUETOOTH_SRC
endif #NETAPP_BLUETOOTH

ifneq (, ${HAS_BLUETOOTH})
BSA_DIR             = ${NETAPP_OUTPUT_INC_DIR}/${BT_PKG_VERSION}/3rdparty/embedded/bsa_examples/linux/libbsa/include
BTHID_DIR           = ${NETAPP_OUTPUT_INC_DIR}/${BT_PKG_VERSION}/3rdparty/embedded/brcm/linux/bthid
CFLAGS              += -I${BSA_DIR} -I${BTHID_DIR}
NETAPP_SHARED_LIBS  += bsa
else
NETAPP_BLUETOOTH=n
endif


################################################################################
# 							W I F I  	S U P P O R T
# Enable WiFi if the WiFi source code is available
################################################################################
ifneq (${NETAPP_WIFI},n)
WIFI_DIR                := ${NETAPP_TOP}/wlan
ifeq ($(NETAPP_WIFI_DRIVER),)
NETAPP_WIFI_DRIVER := dhd
endif
WIFI_CHIPSET            ?= broadcom
WIFI_CHIPSET_DIR        := ${WIFI_DIR}/${WIFI_CHIPSET}
WIFI_PRIVATE_DIR		?= ${WIFI_DIR}/private
WIFI_PUBLIC_DIR			?= ${WIFI_DIR}/public

# KIRIN (Very Old and not tested)
KIRIN_DRIVER_VERSION ?= kirin_rel_5_102_98_67
ifeq (${NETAPP_WIFI_DRIVER}, kirin)
NETAPP_WIFI_CHIP    = 43236b
WIFI_DRIVER_VERSION ?= ${KIRIN_DRIVER_VERSION}
CFLAGS              += -DKIRIN_WIFI_DRIVER
NETAPP_WIFI_WFD=n
endif


#Falcon: Full Dongle
FALCON_DRIVER_VERSION 		?= falcon_rel_5_99_188_9
NETAPP_FALCON_WIFI_CHIPS	:= 43236b 43238b
ifeq (${NETAPP_WIFI_DRIVER}, falcon)
WIFI_DRIVER_VERSION ?= ${FALCON_DRIVER_VERSION}
CFLAGS              += -DFALCON_WIFI_DRIVER
NETAPP_FIRMWARE      = bcm43xx-firmware.bin
NETAPP_MFG_FIRMWARE  = bcm43xx-firmware_mfg.bin
NETAPP_NVRAM         = bcm43xx.nvm
NETAPP_WIFI_CHIP     ?= 43236b
NETAPP_WIFI_P2P=n
endif


# BISON: BMAC/NIC Driver
BISON_DRIVER_VERSION ?= bison04t_rel_7_14_124_60
NETAPP_BISON_WIFI_CHIPS   := 4360nic  43526b 43236b
ifeq (${NETAPP_WIFI_DRIVER}, bison)
WIFI_DRIVER_VERSION ?= ${BISON_DRIVER_VERSION}
CFLAGS              += -DBISON_WIFI_DRIVER
NETAPP_WIFI_CHIPS   := $(NETAPP_BISON_WIFI_CHIPS)
WIFI_CHIPSET_DIR        := ${WIFI_PRIVATE_DIR}
endif


# AARDVARK: BMAC/NIC Driver
AARDVARK_DRIVER_VERSION ?= aardvark01t_rel_6_37_14_105
NETAPP_AARDVARK_WIFI_CHIPS   := 4360nic 43143b0 43236b 43238b 43526b 43242a1 4360b
ifeq (${NETAPP_WIFI_DRIVER}, aardvark)
WIFI_DRIVER_VERSION ?= ${AARDVARK_DRIVER_VERSION}
CFLAGS              += -DAARDVARK_WIFI_DRIVER
NETAPP_WIFI_CHIPS   := $(NETAPP_AARDVARK_WIFI_CHIPS)
endif
ifeq (${NETAPP_WIFI_CHIP}, 43228nic)
AARDVARK_DRIVER_VERSION := aard37rc14_rel_6_37_4_8_PCIe
endif


DHD_DRIVER_VERSION 	    ?= dhd_rel_1_363_110_11
NETAPP_DHD_WIFI_CHIPS   += 4359b1 4366b1 43569a2 43570a2 43602a1
ifeq (${NETAPP_WIFI_DRIVER}, dhd)
WIFI_DRIVER_VERSION 	?= ${DHD_DRIVER_VERSION}
CFLAGS              	+= -DDHD_WIFI_DRIVER
NETAPP_WIFI_CHIPS   	:= $(NETAPP_DHD_WIFI_CHIPS)
WIFI_CHIPSET_DIR        := ${WIFI_PUBLIC_DIR}
endif



#NETAPP_FIRMWARE      = bcm43xx-firmware.bin
NETAPP_MFG_FIRMWARE  = bcm43xx-firmware_mfg.bin

WIFI_PCIE_SUPPORT=y
ifdef NETAPP_WIFI_CHIP
CHIPVER ?= $(NETAPP_WIFI_CHIP)
ifeq ($(findstring nic,$(NETAPP_WIFI_CHIP)),)
WIFI_PCIE_SUPPORT=n
endif
endif

PHOENIX_DRIVER_VERSION ?= phoenix2dhd-1-66_rel_6_10_166_3
ifeq (${NETAPP_WIFI_DRIVER}, phoenix2)
WIFI_DRIVER_VERSION ?= ${PHOENIX_DRIVER_VERSION}
CFLAGS              += -DPHOENIX2_WIFI_DRIVER
NETAPP_WIFI_WFD=n
endif


ALL_WIFI_DRIVERS      += $(addsuffix .tgz, $(DHD_DRIVER_VERSION) $(DHD_DRIVER_VERSION3) $(BISON_DRIVER_VERSION) $(AARDVARK_DRIVER_VERSION)\
						$(WIFI_AP_VERSION_FD) $(WIFI_AP_VERSION_NIC) $(WIFI_AP_VERSION_APP))

WIFI_SRC_PKG            := ${WIFI_CHIPSET_DIR}/${WIFI_DRIVER_VERSION}.tgz
WIFI_DRIVER_PKG			:= $(NETAPPPLATFORM_OBJ_DIR)/$(strip ${WIFI_DRIVER_VERSION})

WIFI_DRIVER_PATCH		:= ${WIFI_DIR}/${WIFI_DRIVER_VERSION}.patch
ifeq ($(wildcard $(WIFI_DRIVER_PATCH)),)
WIFI_DRIVER_PATCH		:= ${WIFI_DIR}/${NETAPP_WIFI_DRIVER}.patch
endif
WIFI_DRIVER_STRIP_PATCH	:= ${WIFI_DIR}/${NETAPP_WIFI_DRIVER}_strip.patch
HAS_WIFI_SRC            :=$(wildcard ${WIFI_SRC_PKG})

WIFI_MFG_SRC_PKG		:= $(wildcard ${WIFI_DIR}/${WIFI_CHIPSET}/${WIFI_DRIVER_VERSION}_mfg.tgz)
ifneq (${WIFI_MFG_SRC_PKG},)
# we found a valid MFG driver source tarball
WIFI_MFG_DRIVER_PKG		:= $(addsuffix _mfg, ${WIFI_DRIVER_PKG})
WIFI_MFG_DRIVER_PATCH	=${WIFI_DIR}/${NETAPP_WIFI_DRIVER}_mfg.patch
endif

NETAPP_STRIP_WIFI_DRIVER ?=y

WIFI_AP_VERSION_NIC				?= BISON04T_REL_7_14_131_28
WIFI_AP_VERSION_FD				?= DHD_REL_1_363_45_48
WIFI_AP_VERSION_APP				?= BIS131RC25_REL_7_60_2
ifeq (${NETAPP_WIFI_AP},y)
NETAPP_WIFI_CHIP1 ?= ${NETAPP_WIFI_CHIP}
WIFI_DRIVER_VERSION_AP_4360		?= ${WIFI_AP_VERSION_NIC}
WIFI_DRIVER_VERSION_AP_43602	?= ${WIFI_AP_VERSION_FD}
WIFI_DRIVER_VERSION_AP_4366		?= ${WIFI_AP_VERSION_FD}
WIFI_DRIVER_VERSION_AP_43570	?= ${WIFI_AP_VERSION_NIC}
WIFI_APP_VERSION_AP				?= ${WIFI_AP_VERSION_APP}

NETAPP_WIFI_AP_FIRMWARE_4366	?= 4366b1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs.bin
NETAPP_WIFI_AP_FIRMWARE_43602	?= 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16.bin

NETAPP_WIFI_AP_BLDCFG_NIC	?= nodebug-apdef-stadef-extnvm-mfp-wmf-stb-stbap-armv7l
NETAPP_WIFI_AP_BLDCFG_FD	?= dhd-msgbuf-pciefd-mfp-wmf-psta-stbap-armv7l
NETAPP_WIFI_AP_BLDARGS += CROSS_COMPILE=${CROSS_COMPILE} STBLINUX=1 SHOWWLCONF=1 LINUX_VERSION=${LINUXVER} LINUXDIR=${LINUX}
NETAPP_WIFI_AP_APP_BLDARGS += CROSS_COMPILE=${CROSS_COMPILE} STBLINUX=1  LINUX_VERSION=${LINUXVER} EXTRA_LDFLAGS=-lgcc_s PLT=arm TOOLCHAIN=${TOOLCHAIN_AP}

ifneq ($(findstring 4366, $(NETAPP_WIFI_CHIP1)),)
	WIFI_DRIVER_VERSION_AP1	?= ${WIFI_DRIVER_VERSION_AP_4366}
	WIFI_DRIVER_AP1 		?= drivers_fd
	WIFI_CHIPSET1			?= 4366
	WIFI_FIRMWARE_AP1		?= ${NETAPP_WIFI_AP_FIRMWARE_4366}
	#WIFI_DRIVER_PATCH_AP1 	?= dhd.patch
endif
ifneq ($(findstring 4366, $(NETAPP_WIFI_CHIP2)),)
	WIFI_DRIVER_VERSION_AP2	?= ${WIFI_DRIVER_VERSION_AP_4366}
	WIFI_DRIVER_AP2 		?= drivers_fd
	WIFI_CHIPSET2			?= 4366
	WIFI_FIRMWARE_AP2		?= ${NETAPP_WIFI_AP_FIRMWARE_4366}
	WIFI_DRIVER_PATCH_AP2 	?= dhd.patch
endif
ifneq ($(findstring 43602, $(NETAPP_WIFI_CHIP1)),)
	WIFI_DRIVER_VERSION_AP1	?= ${WIFI_DRIVER_VERSION_AP_43602}
	WIFI_DRIVER_AP1 ?= drivers_fd
	WIFI_CHIPSET1			?= 43602
	WIFI_FIRMWARE_AP1		?= ${NETAPP_WIFI_AP_FIRMWARE_43602}
	#WIFI_DRIVER_PATCH_AP1 ?= dhd.patch
else
ifneq ($(findstring 4360, $(NETAPP_WIFI_CHIP1)),)
	WIFI_DRIVER_VERSION_AP1	?= ${WIFI_DRIVER_VERSION_AP_4360}
	WIFI_DRIVER_AP1 ?= drivers_nic
	WIFI_CHIPSET1			?= 4360
	#WIFI_DRIVER_PATCH_AP1 ?= bison.patch
endif
endif
ifneq ($(findstring 43602, $(NETAPP_WIFI_CHIP2)),)
	WIFI_DRIVER_VERSION_AP2	?= ${WIFI_DRIVER_VERSION_AP_43602}
	WIFI_DRIVER_AP2 ?= drivers_fd
	WIFI_CHIPSET2			?= 43602
	WIFI_FIRMWARE_AP2		?= ${NETAPP_WIFI_AP_FIRMWARE_43602}
	#WIFI_DRIVER_PATCH_AP2 ?= dhd.patch
else
ifneq ($(findstring 4360, $(NETAPP_WIFI_CHIP2)),)
	WIFI_DRIVER_VERSION_AP2	?= ${WIFI_DRIVER_VERSION_AP_4360}
	WIFI_DRIVER_AP2 ?= drivers_nic
	WIFI_CHIPSET2			?= 4360
	#WIFI_DRIVER_PATCH_AP2 ?= bison.patch
endif
endif
ifneq ($(findstring 43570, $(NETAPP_WIFI_CHIP1)),)
	WIFI_DRIVER_VERSION_AP1	?= ${WIFI_DRIVER_VERSION_AP_43570}
	WIFI_DRIVER_AP1 ?= drivers_nic
	WIFI_CHIPSET1			?= 43570
	WIFI_FIRMWARE_AP1		?= ${NETAPP_WIFI_AP_FIRMWARE_43570}
	#WIFI_DRIVER_PATCH_AP1 ?= bison.patch
endif
ifneq ($(findstring 43570, $(NETAPP_WIFI_CHIP2)),)
	WIFI_DRIVER_VERSION_AP2	?= ${WIFI_DRIVER_VERSION_AP_43570}
	WIFI_DRIVER_AP2 ?= drivers_nic
	WIFI_CHIPSET2			?= 43570
	WIFI_FIRMWARE_AP2		?= ${NETAPP_WIFI_AP_FIRMWARE_43570}
	#WIFI_DRIVER_PATCH_AP2 ?= bison.patch
endif
ifeq (${WIFI_DRIVER_AP1},drivers_fd)
WIFI_SRC_PKG_AP1            := ${WIFI_PUBLIC_DIR}/${WIFI_DRIVER_VERSION_AP1}.tgz
else
WIFI_SRC_PKG_AP1            := ${WIFI_PRIVATE_DIR}/${WIFI_DRIVER_VERSION_AP1}.tgz
endif
WIFI_DRIVER_PKG_AP1			:= $(NETAPPPLATFORM_OBJ_DIR)/$(strip ${WIFI_DRIVER_VERSION_AP1})
ifeq (${WIFI_DRIVER_AP2},drivers_fd)
WIFI_SRC_PKG_AP2            := ${WIFI_PUBLIC_DIR}/${WIFI_DRIVER_VERSION_AP2}.tgz
else
WIFI_SRC_PKG_AP2            := ${WIFI_PRIVATE_DIR}/${WIFI_DRIVER_VERSION_AP2}.tgz
endif
WIFI_DRIVER_PKG_AP2			:= $(NETAPPPLATFORM_OBJ_DIR)/$(strip ${WIFI_DRIVER_VERSION_AP2})
WIFI_SRC_PKG_AP             := ${WIFI_PRIVATE_DIR}/${WIFI_APP_VERSION_AP}.tgz
WIFI_APP_PKG_AP			    := $(NETAPPPLATFORM_OBJ_DIR)/$(strip ${WIFI_APP_VERSION_AP})
endif

############################################

CFLAGS                  += -DWIFI_CHIP="\"${WIFI_CHIP}\"" -DWIFI_CHIPS="\"${WIFI_CHIPS}\""
CFLAGS                  += -DNETAPP_MFG_FIRMWARE_PATH="\"${NETAPP_TARGET_FW_DIR}/${NETAPP_MFG_FIRMWARE}\""

ifneq (${HAS_WIFI_SRC},)
HAS_WIFI				:= ${HAS_WIFI_SRC}
else
ifneq (,${HAS_WIFI_BUILT})
HAS_WIFI				:= ${HAS_WIFI_BUILT}
endif
endif

ifneq (${HAS_WIFI},)
WPSCLI                  := wpscli
WPSAPI                  := wpsapi
WPSENR                  := wpsenr
WPSCOM                  := wpscom
WPSAP                   := wpsap
BCMP2P                  := bcmp2p
BCMP2PSIG               := bcmp2psig
BCMCRYPTO               := bcmcrypto
WFDCAPDIE               := wfdcapdie

ifeq (${WIFI_MFG_SRC_PKG},)
ifneq (${NETAPP_WIFI_WPS}, n)
NETAPP_NEEDS_WPS    = y
NETAPP_NEEDS_WPSCLI = y
endif

ifneq (${NETAPP_WIFI_P2P},n)
NETAPP_NEEDS_WPS    = y
NETAPP_NEEDS_WPSCLI = y
CPPFLAGS            += -DSTBLINUX=1
else
NETAPP_WIFI_WFD     = n
endif

ifneq (${NETAPP_WIFI_INVITE},n)
NETAPP_NEEDS_WPS    = y
NETAPP_NEEDS_WPSCLI = y
endif

ifeq (${NETAPP_NEEDS_WPSCLI}, y)
NETAPP_SHARED_LIBS       += ${WPSCLI}
endif

ifeq (${NETAPP_NEEDS_WPS}, y)
NETAPP_SHARED_LIBS      += ${WPSENR} ${WPSCOM} ${WPSAPI} ${WPSAP}
ifeq (${EXTERNAL_OPENSSL},1)
NETAPP_SHARED_LIBS      += crypto
else
NETAPP_STATIC_LIBS      += ${BCMCRYPTO}
endif
endif

ifneq (${NETAPP_WIFI_P2P},n)
NETAPP_STATIC_LIBS       += ${BCMP2P} ${BCMP2PSIG}
endif

ifneq (${NETAPP_WIFI_WFD},n)
NETAPP_STATIC_LIBS       += ${WFDCAPDIE}
endif
else
NETAPP_WIFI_WPS=n
NETAPP_WIFI_P2P=n
NETAPP_WIFI_WFD=n
endif #MFG driver
endif #HAS_WIFI
NETAPP_TARGET_MODULES_LIST	+= \
	bcmdhd.ko \
	dhd_pcie.ko
NETAPP_TARGET_BIN_LIST 		+= \
	dhd \
	wl \
	sromvar
endif #WIFI=y

NETAPP_SHARED_LINKER_STR= -Wl,-L$(NETAPP_OUTPUT_LIB_DIR),-dy,-l
NETAPP_LDFLAGS      	+= $(addprefix ${NETAPP_SHARED_LINKER_STR}, ${NETAPP_SHARED_LIBS})
NETAPP_CFLAGS       	= -I${NETAPP_TOP}/include ${CPPFLAGS} ${CFLAGS}
