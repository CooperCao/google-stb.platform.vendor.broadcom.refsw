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
BASE_FW_IMAGE := ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder

TARGET_OPTIONS_config_sdio_release	:= sdio-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_sdio_debug	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err
TARGET_OPTIONS_config_sdio_mfgtest	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-txbf
TARGET_OPTIONS_config_sdio_qt		:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-qt
TARGET_OPTIONS_config_sdio_mfgtest_ate := $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-ate
TARGET_OPTIONS_config_sdio_mfgtest_ate_qt := $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-ate-qt
TARGET_OPTIONS_config_sdio_mfgtest_qt := $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-qt

TARGET_OPTIONS_config_pcie_release	:= pcie-$(BASE_FW_IMAGE)-dlystat-rmon
TARGET_OPTIONS_config_pcie_debug	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err
TARGET_OPTIONS_config_pcie_debug_qt	:= $(TARGET_OPTIONS_config_pcie_debug)-qt
TARGET_OPTIONS_config_pcie_mfgtest	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-txbf
TARGET_OPTIONS_config_pcie_mfgtest_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-qt
TARGET_OPTIONS_config_pcie_mfgtest_qt_ate	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-qt-ate
TARGET_OPTIONS_config_pcie_mfgtest_vasip_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-vasip-qt
TARGET_OPTIONS_config_pcie_mfgtest_vasip_idma_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-vasip-idma-qt
TARGET_OPTIONS_config_pcie_mfgtest_vasip_ifrm_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-vasip-ifrm-qt
TARGET_OPTIONS_config_pcie_mfgtest_vasip_d2hmsi_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-vasip-d2hmsi-qt
TARGET_OPTIONS_config_pcie_qt		:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-qt
TARGET_OPTIONS_config_pcie_qt_ate		:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-qt-ate
TARGET_OPTIONS_config_pcie_qt_norsdb	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-qt-norsdb

TARGET_OPTIONS_config_pcie_olym_base	:= pcie-ag-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-txpwrcap-err-swdiv-ecounters-hchk-bssinfo-bdo-chanim-wlassocnbr
TARGET_OPTIONS_config_pcie_olym_release	:= $(TARGET_OPTIONS_config_pcie_olym_base)-clm_min
TARGET_OPTIONS_config_pcie_olym_debug	:= $(TARGET_OPTIONS_config_pcie_olym_release)-assert-err
# Exclude clm_min with -qt
TARGET_OPTIONS_config_pcie_olym_qt	:= $(TARGET_OPTIONS_config_pcie_olym_base)-assert-err-qt

TARGET_OPTIONS_config_pcie_ss_release	:= pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-hostpp-lpc-pwropt-txbf-wl11u-mfp-wnm-betdls-amsdutx5g-okc-ccx-ve-clm_ss_mimo-txpwr-rcc-fmc-noccxaka-sarctrl-proxd-gscan-linkstat-pwrstats-shif-idsup-ndoe-fpl2g-wepso-chanim-bgdfs
TARGET_OPTIONS_config_pcie_ss_debug	:= $(TARGET_OPTIONS_config_pcie_ss_release)-assert-err
TARGET_OPTIONS_config_pcie_ss_qt	:= $(TARGET_OPTIONS_config_pcie_ss_debug)-qt

TARGET_OPTIONS_config_pcie_aibss_debug	:= $(TARGET_OPTIONS_config_pcie_release)-aibss-relmcast
TARGET_OPTIONS_config_pcie_natoe	:= $(TARGET_OPTIONS_config_pcie_release)-natoe
