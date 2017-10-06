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
BASE_FW_IMAGE := ag-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-mscheventlog

TARGET_OPTIONS_config_sdio_release	:= sdio-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_sdio_debug	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err
TARGET_OPTIONS_config_sdio_mfgtest	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-txbf-dump
TARGET_OPTIONS_config_sdio_mfgtest_ate := $(TARGET_OPTIONS_config_sdio_mfgtest)-ate

TARGET_OPTIONS_config_pcie_base    := pcie-ag-splitbuf-pktctx-proptxstatus-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-txpwrcap-err-swdiv-ecounters-hchk-bssinfo-bdo-chanim-wlassocnbr-mscheventlog
TARGET_OPTIONS_config_pcie_release	:= pcie-$(TARGET_OPTIONS_config_pcie_base)-clm_min
TARGET_OPTIONS_config_pcie_debug	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err

# mfgtst target
TARGET_OPTIONS_config_pcie_mfgtest	:= pcie-$(BASE_FW_IMAGE)-assert-err-mfgtest-seqcmds-txbf-dump
TARGET_OPTIONS_config_pcie_mfgtest_ate	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ate

TARGET_OPTIONS_config_pcie_mfgtest_vasip	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-vasip
TARGET_OPTIONS_config_pcie_mfgtest_idma	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-idma
TARGET_OPTIONS_config_pcie_mfgtest_ifrm	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ifrm
TARGET_OPTIONS_config_pcie_mfgtest_d2hmsi	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-d2hmsi
TARGET_OPTIONS_config_pcie_mfgtest_dma1	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma1
TARGET_OPTIONS_config_pcie_mfgtest_dma2	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma2
TARGET_OPTIONS_config_pcie_mfgtest_dma1_dma2	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma1-dma2
