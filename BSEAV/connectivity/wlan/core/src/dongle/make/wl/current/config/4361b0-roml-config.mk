# Config makefile that maps config based target names to features.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$

# Variables that map config names to features - 'TARGET_OPTIONS_config_[bus-type]_xxx'.
BASE_FW_IMAGE := ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-cbndstate-mscheventlog

TARGET_OPTIONS_config_sdio_release	:= sdio-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_sdio_debug	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err
TARGET_OPTIONS_config_sdio_mfgtest	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-txbf-dump
TARGET_OPTIONS_config_sdio_mfgtest_ate	:= $(TARGET_OPTIONS_config_sdio_mfgtest)-ate
TARGET_OPTIONS_config_sdio_mfgtest_sr	:= $(TARGET_OPTIONS_config_sdio_mfgtest)-sr

TARGET_OPTIONS_config_pcie_baseimage   := pcie-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_pcie_release   := pcie-$(BASE_FW_IMAGE)-sr-lpc-pwropt-txbf-wl11u-mfp-wnm-betdls-amsdutx5g-okc-ccx-ve-clm_ss_mimo-txpwr-rcc-fmc-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-shif-idsup-ndoe-fpl2g-wepso-chanim-bgdfs-ve-wbtext
TARGET_OPTIONS_config_pcie_debug	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err
TARGET_OPTIONS_config_pcie_fullfeature	:= $(TARGET_OPTIONS_config_pcie_release)-apf-rssimon-netx-aibss-relmcast-swprbrsp-natoe-wbtext
TARGET_OPTIONS_config_pcie_debug_qt	:= $(TARGET_OPTIONS_config_pcie_debug)-qt
TARGET_OPTIONS_config_pcie_mfgtest	:= pcie-$(BASE_FW_IMAGE)-assert-err-mfgtest-seqcmds-txbf-dump
TARGET_OPTIONS_config_pcie_mfgtest_ate	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ate
TARGET_OPTIONS_config_pcie_mfgtest_sr	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-sr

TARGET_OPTIONS_config_pcie_aibss_debug	:= $(TARGET_OPTIONS_config_pcie_release)-aibss-relmcast
TARGET_OPTIONS_config_pcie_natoe	:= $(TARGET_OPTIONS_config_pcie_release)-natoe
