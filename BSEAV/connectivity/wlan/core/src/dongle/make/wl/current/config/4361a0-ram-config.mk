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
BASE_FW_IMAGE := pcie-ag-p2p-mchan-pno-aoe-pktfilter-keepalive-pktctx-proptxstatus-ampduhostreorder-clm_min-noclminc-srampmac-powoff-cbndstate-mscheventlog
BASE_FW_IMAGE2 := pcie-ag-p2p-mchan-pktctx-proptxstatus-ampduhostreorder-clm_min-noclminc-powoff-cbndstate-mscheventlog

TARGET_OPTIONS_config_pcie_release	:= $(BASE_FW_IMAGE)-splitrx-err-logtrace-betdls-epa-ddchk
TARGET_OPTIONS_config_pcie_debug	:= $(TARGET_OPTIONS_config_pcie_release)-assert
TARGET_OPTIONS_config_pcie_natoe	:= $(TARGET_OPTIONS_config_pcie_release)-apsta-natoe-dbgnatoe
TARGET_OPTIONS_config_pcie_vsdb_tdls_tput    := $(BASE_FW_IMAGE2)-tput-dualpktamsdu-betdls-epa
#MFG
TARGET_OPTIONS_config_pcie_mfgtest      := $(BASE_FW_IMAGE)-mfgtest-seqcmds-logtrace-assert-err-epa-pwrstats
TARGET_OPTIONS_config_pcie_mfgtest_ipa  := $(BASE_FW_IMAGE)-mfgtest-seqcmds-logtrace-assert-err-pwrstats-ipa
#TARGET_OPTIONS_config_pcie_mfgtest_ate	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ate

#IBSS
#TARGET_OPTIONS_config_pcie_aibss_debug	:= $(TARGET_OPTIONS_config_pcie_release)-aibss-relmcast

#Throughput
TARGET_OPTIONS_config_pcie_tput         := $(BASE_FW_IMAGE2)-logtrace-err-tput-dualpktamsdu-dbgam-dbgrsdb-epa
TARGET_OPTIONS_config_pcie_tput_ipa     := $(BASE_FW_IMAGE)-logtrace-err-tput-dualpktamsdu-dbgam-dbgrsdb
TARGET_OPTIONS_config_pcie_tput_mode4	:= $(TARGET_OPTIONS_config_pcie_tput)-splitrxmode4
TARGET_OPTIONS_config_pcie_mbo          := $(BASE_FW_IMAGE)-assert-err-ve-okc-mbo
