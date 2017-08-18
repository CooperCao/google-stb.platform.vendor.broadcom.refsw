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
BASE_FW_IMAGE := ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-clm_min-noclminc-dbgrsdb-dbgmac-srampmac-utrace-dbguc-logtrace-rapsta-powoff-rsdb_apscan-heapcheck-sr-srpwr-cbndstate-mscheventlog
BASE_FW_IMAGE2 := ag-p2p-mchan-pktctx-proptxstatus-ampduhostreorder-dbgrsdb-dbgmac-srampmac-utrace-dbguc-powoff-heapcheck-sr-srpwr-cbndstate-mscheventlog
BASE_FW_IMAGE_NOP2P := ag-mcnx-pktctx-proptxstatus-ampduhostreorder-dbgrsdb-cbndstate-mscheventlog
BASE_FW_IMAGE3 := $(BASE_FW_IMAGE)-okc-wepso-pfn-olpc-apcs-pspretend-rcc-fmc-p2poelo-dpm-hthrot-phycal-pwrstats-txpwr-dualpktamsdu
CONFIG_PCIE_BASE_FW_IMAGE := pcie-$(BASE_FW_IMAGE)-ndoe-okc-wepso-pfn-hs20sta-olpc-apcs-pspretend-rcc-fmc-p2poelo-dpm-hthrot-phycal-pwrstats-txpwr-dualpktamsdu-scan-assoc-assert

TARGET_OPTIONS_config_pcie_release_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-linkstat-wnm-txbf-murx-rmon-btcdyn-ipa
TARGET_OPTIONS_config_pcie_release		:= $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-linkstat-wnm-txbf-murx-rmon-btcdyn-epa


TARGET_OPTIONS_config_pcie_release1	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-apf-hchk-gscan-linkstat-rmon-mfp-epa
TARGET_OPTIONS_config_pcie_release1_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-apf-hchk-gscan-linkstat-rmon-mfp-ipa

TARGET_OPTIONS_config_pcie_release_mode4	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-splitrxmode4-epa
TARGET_OPTIONS_config_pcie_release_mode4_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-splitrxmode4-ipa

TARGET_OPTIONS_config_pcie_bgdfs	:= pcie-$(BASE_FW_IMAGE)-bgdfs-dbgdfs-epa
TARGET_OPTIONS_config_pcie_bgdfs_ipa	:= pcie-$(BASE_FW_IMAGE)-bgdfs-dbgdfs-ipa

TARGET_OPTIONS_config_pcie_nop2p	:= pcie-$(BASE_FW_IMAGE_NOP2P)-phycal-txpwr-dualpktamsdu-clm_min-noclminc-logtrace-logtrace_pcie-epa
TARGET_OPTIONS_config_pcie_nop2p_ipa	:= pcie-$(BASE_FW_IMAGE_NOP2P)-phycal-txpwr-dualpktamsdu-clm_min-noclminc-logtrace-logtrace_pcie-ipa

TARGET_OPTIONS_config_pcie_tput		:= pcie-$(BASE_FW_IMAGE2)-phycal-txpwr-dualpktamsdu-tput-epa-clm_min-noclminc-logtrace-logtrace_pcie
TARGET_OPTIONS_config_pcie_tput_ipa		:= pcie-$(BASE_FW_IMAGE2)-phycal-txpwr-dualpktamsdu-tput-ipa-clm_min-noclminc-logtrace-logtrace_pcie_dbgam

TARGET_OPTIONS_config_pcie_tput_mu	:= $(TARGET_OPTIONS_config_pcie_tput)-txbf-murx-dbgmu
TARGET_OPTIONS_config_pcie_tput_mu_ipa	:= $(TARGET_OPTIONS_config_pcie_tput_ipa)-txbf-murx-dbgmu

TARGET_OPTIONS_config_pcie_tdls	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-betdls-assert-epa
TARGET_OPTIONS_config_pcie_tdls_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-betdls-assert-ipa

TARGET_OPTIONS_config_pcie_odt	:=  $(CONFIG_PCIE_BASE_FW_IMAGE)-betdls-linkstat-wnm-rmon-epa

# For SoftAP, STA, p2p, RSDB, VSDB
TARGET_OPTIONS_config_pcie_utf	:= pcie-$(BASE_FW_IMAGE3)-ndoe-hs20sta-idsup-assert

# TDLS
TARGET_OPTIONS_config_pcie_utf1	:= pcie-$(BASE_FW_IMAGE3)-betdls-idsup-assert

# Below commented prod target is for reference
#TARGET_OPTIONS_config_pcie_prod         := pcie-ve-mfp-okc-idsup-sr-die3-betdls-wepso-linkstat-wnm-pfn-hs20sta-olpc-ag-keepalive-pktfilter-p2p-mchan-proptxstatus-ampduhostreorder-aoe-ndoe-pno-pktctx-apcs-pspretend-rcc-ccx-noccxaka-clm_ss_mimo-fmc-shif-rapsta-rscanf-roamol-p2poelo-dpm-hthrot-murx-pwrofs-pwrofs5g-sarctrl-pwrstats-lpc-txbf-imptxbf-txpwr-fpl2g-ocl-elnabyp
TARGET_OPTIONS_config_sdio_release	:= sdio-$(BASE_FW_IMAGE2)
TARGET_OPTIONS_config_sdio_debug	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err
TARGET_OPTIONS_config_sdio_mfgtest	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-txbf-dump
TARGET_OPTIONS_config_sdio_mfgtest_ate := $(TARGET_OPTIONS_config_sdio_mfgtest)-ate
TARGET_OPTIONS_config_sdio_mfgtest_ate_ipa := sdio-ag-proptxstatus-nocis-mfgtest-txbf-ate-ipa

TARGET_OPTIONS_config_pcie_mfgtest	:= pcie-$(BASE_FW_IMAGE2)-mfgtest-seqcmds-txbf-logtrace-assert-err-epa-pwrstats
TARGET_OPTIONS_config_pcie_mfgtest_ipa	:= pcie-$(BASE_FW_IMAGE2)-mfgtest-seqcmds-txbf-logtrace-assert-err-pwrstats-ipa
TARGET_OPTIONS_config_pcie_mfgtest_ate	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ate

TARGET_OPTIONS_config_pcie_proxd	:= pcie-$(BASE_FW_IMAGE2)-epa-proxd
TARGET_OPTIONS_config_pcie_proxd_ipa	:= pcie-$(BASE_FW_IMAGE2)-ipa-proxd

TARGET_OPTIONS_config_pcie_aibss_debug	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-aibss-relmcast-epa
TARGET_OPTIONS_config_pcie_aibss_debug_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-aibss-relmcast-ipa

TARGET_OPTIONS_config_pcie_release_ocl := $(CONFIG_PCIE_BASE_FW_IMAGE)-oclf-epa
TARGET_OPTIONS_config_pcie_release_ocl_ipa := $(CONFIG_PCIE_BASE_FW_IMAGE)-oclf-ipa

TARGET_OPTIONS_config_pcie_natoe        := $(CONFIG_PCIE_BASE_FW_IMAGE)-natoe-epa
TARGET_OPTIONS_config_pcie_natoe_ipa        := $(CONFIG_PCIE_BASE_FW_IMAGE)-natoe-ipa

TARGET_OPTIONS_config_pcie_wbtext_ccx       := $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-wnm-wbtext-dbgwbtext-ccx-noccxaka-epa
TARGET_OPTIONS_config_pcie_wbtext_ccx_ipa       := $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-wnm-wbtext-dbgwbtext-ccx-noccxaka-ipa

#nan
TARGET_OPTIONS_config_pcie_nan	:= $(TARGET_OPTIONS_config_pcie_nop2p)-assert-nan
TARGET_OPTIONS_config_pcie_nan_ipa	:= $(TARGET_OPTIONS_config_pcie_nop2p_ipa)-assert-nan

#packetid_audit
TARGET_OPTIONS_config_pcie_pktidaudit:= pcie-$(BASE_FW_IMAGE)-pktidaudit-epa
TARGET_OPTIONS_config_pcie_pktidaudit_ipa:= pcie-$(BASE_FW_IMAGE)-pktidaudit-ipa

#eLNA Bypass feature
TARGET_OPTIONS_config_pcie_mfgtest_elnabyp	:= pcie-$(BASE_FW_IMAGE2)-mfgtest-seqcmds-txbf-logtrace-assert-err-epa-pwrstats-elnabyp
TARGET_OPTIONS_config_pcie_mfgtest_elnabyp_ipa	:= pcie-$(BASE_FW_IMAGE2)-mfgtest-seqcmds-txbf-logtrace-assert-err-pwrstats-ipa-elnabyp
TARGET_OPTIONS_config_pcie_tput_elnabyp		:= pcie-$(BASE_FW_IMAGE2)-phycal-txpwr-dualpktamsdu-tput-epa-clm_min-noclminc-logtrace-logtrace_pcie-elnabyp
TARGET_OPTIONS_config_pcie_tput_elnabyp_ipa	:= pcie-$(BASE_FW_IMAGE2)-phycal-txpwr-dualpktamsdu-tput-ipa-clm_min-noclminc-logtrace-logtrace_pcie-elnabyp
TARGET_OPTIONS_config_pcie_release_elnabyp	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-linkstat-wnm-txbf-murx-rmon-btcdyn-epa-elnabyp
TARGET_OPTIONS_config_pcie_release_elnabyp_ipa	:= $(CONFIG_PCIE_BASE_FW_IMAGE)-ve-linkstat-wnm-txbf-murx-rmon-btcdyn-ipa-elnabyp
