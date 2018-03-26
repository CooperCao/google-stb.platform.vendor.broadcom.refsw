############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
############################################################

# Built everything nexus
# all targets _must_
# * support out-of-source build
# * support parallel build
# * have proper dependency test

THIS_MFILE_DIR_FUNC := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
MY_TOP := $(call THIS_MFILE_DIR_FUNC)

B_REFSW_TOP:= $(abspath $(realpath ${MY_TOP}/../../../../..))

B_REFSW_CACHE_DIR ?= $(realpath /projects/stbdev/open-source)
export B_REFSW_CACHE_DIR

# For release target, default the download location
# For normal build, install.mk will set the download location (usually obj directory)
SAGE_BINARY_TOP  ?= ${B_REFSW_TOP}/BSEAV/lib/security/sage/bin

ifneq ($(NEXUS_PLATFORM),)
NEXUS_TOP:=$(B_REFSW_TOP)/nexus
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif


MYCHIP=${BCHP_CHIP}${BCHP_VER}
# Some chip versions need to be normalized since SAGE binaries are not distinct
ifeq (${MYCHIP},7278B1)
MYCHIP = 7278B0
endif

ALLCHIPS=7250B0 7260A0 7268B0 7271B0 7364C0 7366C0 7439B0 7445D0 7445E0 7278B0 7255A0 7260B0


VERIFY_SCRIPT=bash ${B_REFSW_TOP}/BSEAV/tools/build/binaries/verify_binaries.sh
THIS_DIR=BSEAV/lib/security/sage/bin

SSF_PROD_BINS_META=${MY_TOP}/ssf_prod_meta
SSF_DEV_BINS_META=${MY_TOP}/ssf_dev_meta
SAGEBL_PROD_BINS_META=${MY_TOP}/sbl_prod_meta
SAGEBL_DEV_BINS_META=${MY_TOP}/sbl_dev_meta

# Where the downloaded SAGE binaries are to be located
SAGE_FW := ${SAGE_BINARY_TOP}

VERIFY_ALLCHIP_BINARIES=$(addprefix verify_binaries_,${ALLCHIPS})
VERIFY_ALLCHIP_SBL_BINS=$(addprefix verify_sbl_binaries_,${ALLCHIPS})
VERIFY_ALLCHIP_SSF_BINS=$(addprefix verify_ssf_binaries_,${ALLCHIPS})
# All chips which have all TAs (7278B0 is not included)
VERIFY_ALLCHIP_TA_BINS=$(addprefix verify_ta_binaries_,$(filter-out 7278B0, ${ALLCHIPS}))

ANTIROLLBACK_DEV_BINS_META=${MY_TOP}/antirollback_dev_meta
ANTIROLLBACK_PROD_BINS_META=${MY_TOP}/antirollback_prod_meta
VERIFY_ALLCHIP_ANTIROLLBACK_BINS=$(addprefix verify_ar_,${ALLCHIPS})

BP3_DEV_BINS_META=${MY_TOP}/bp3_dev_meta
BP3_PROD_BINS_META=${MY_TOP}/bp3_prod_meta
VERIFY_ALLCHIP_BP3_BINS=$(addprefix verify_bp3_,${ALLCHIPS})

DTCPIP_DEV_BINS_META=${MY_TOP}/dtcpip_dev_meta
DTCPIP_PROD_BINS_META=${MY_TOP}/dtcpip_prod_meta
VERIFY_ALLCHIP_DTCPIP_BINS=$(addprefix verify_dtcp_,${ALLCHIPS})

EDRM_DEV_BINS_META=${MY_TOP}/edrm_dev_meta
EDRM_PROD_BINS_META=${MY_TOP}/edrm_prod_meta
VERIFY_ALLCHIP_EDRM_BINS=$(addprefix verify_edrm_,${ALLCHIPS})

HDCP22_DEV_BINS_META=${MY_TOP}/hdcp22_dev_meta
HDCP22_PROD_BINS_META=${MY_TOP}/hdcp22_prod_meta
VERIFY_ALLCHIP_HDCP22_BINS=$(addprefix verify_hdcp_,${ALLCHIPS})

KEYMASTER_DEV_BINS_META=${MY_TOP}/keymaster_dev_meta
KEYMASTER_PROD_BINS_META=${MY_TOP}/keymaster_prod_meta
VERIFY_ALLCHIP_KEYMASTER_BINS=$(addprefix verify_keymaster_,${ALLCHIPS})

MANUFACTURING_DEV_BINS_META=${MY_TOP}/manufacturing_dev_meta
MANUFACTURING_PROD_BINS_META=${MY_TOP}/manufacturing_prod_meta
VERIFY_ALLCHIP_MANUFACTURING_BINS=$(addprefix verify_mfg_,${ALLCHIPS})

NETFLIX_DEV_BINS_META=${MY_TOP}/netflix_dev_meta
NETFLIX_PROD_BINS_META=${MY_TOP}/netflix_prod_meta
VERIFY_ALLCHIP_NETFLIX_BINS=$(addprefix verify_nf_,${ALLCHIPS})

PLAYREADY_25_DEV_BINS_META=${MY_TOP}/playready_25_dev_meta
PLAYREADY_25_PROD_BINS_META=${MY_TOP}/playready_25_prod_meta
VERIFY_ALLCHIP_PLAYREADY_25_BINS=$(addprefix verify_pr25_,${ALLCHIPS})

PLAYREADY_30_DEV_BINS_META=${MY_TOP}/playready_30_dev_meta
PLAYREADY_30_PROD_BINS_META=${MY_TOP}/playready_30_prod_meta
VERIFY_ALLCHIP_PLAYREADY_30_BINS=$(addprefix verify_pr30_,${ALLCHIPS})

SECURE_LOG_DEV_BINS_META=${MY_TOP}/secure_log_dev_meta
SECURE_LOG_PROD_BINS_META=${MY_TOP}/secure_log_prod_meta
# Optional: secure_log binary metadata is not required
VERIFY_MYCHIP_SECURE_LOG_BINS :=
ifneq ($(wildcard ${SECURE_LOG_PROD_BINS_META}.${MYCHIP}.txt),)
VERIFY_MYCHIP_SECURE_LOG_BINS := verify_secure_log_${MYCHIP}
endif


SECURE_VIDEO_DEV_BINS_META=${MY_TOP}/secure_video_dev_meta
SECURE_VIDEO_PROD_BINS_META=${MY_TOP}/secure_video_prod_meta
VERIFY_ALLCHIP_SECURE_VIDEO_BINS=$(addprefix verify_svp_,${ALLCHIPS})

SSD_DEV_BINS_META=${MY_TOP}/ssd_dev_meta
SSD_PROD_BINS_META=${MY_TOP}/ssd_prod_meta
VERIFY_ALLCHIP_SSD_BINS=$(addprefix verify_ssd_,${ALLCHIPS})

UTILITY_DEV_BINS_META=${MY_TOP}/utility_dev_meta
UTILITY_PROD_BINS_META=${MY_TOP}/utility_prod_meta
VERIFY_ALLCHIP_UTILITY_BINS=$(addprefix verify_util_,${ALLCHIPS})

WIDEVINE_DEV_BINS_META=${MY_TOP}/widevine_dev_meta
WIDEVINE_PROD_BINS_META=${MY_TOP}/widevine_prod_meta
VERIFY_ALLCHIP_WIDEVINE_BINS=$(addprefix verify_wv_,${ALLCHIPS})


.PHONY : all verify_mychip_binaries ${VERIFY_ALLCHIP_BINARIES} ${VERIFY_ALLCHIP_SBL_BINS} ${VERIFY_ALLCHIP_SSF_BINS} distclean release
.PHONY : ${VERIFY_ALLCHIP_TA_BINS} ${VERIFY_ALLCHIP_ANTIROLLBACK_BINS} ${VERIFY_ALLCHIP_BP3_BINS} ${VERIFY_ALLCHIP_DTCPIP_BINS}
.PHONY : ${VERIFY_ALLCHIP_EDRM_BINS} ${VERIFY_ALLCHIP_HDCP22_BINS} ${VERIFY_ALLCHIP_MANUFACTURING_BINS} ${VERIFY_ALLCHIP_NETFLIX_BINS}
.PHONY : ${VERIFY_ALLCHIP_PLAYREADY_25_BINS} ${VERIFY_ALLCHIP_PLAYREADY_30_BINS} ${VERIFY_ALLCHIP_SECURE_VIDEO_BINS}
.PHONY : ${VERIFY_ALLCHIP_UTILITY_BINS} ${VERIFY_ALLCHIP_WIDEVINE_BINS} ${VERIFY_ALLCHIP_KEYMASTER_BINS} ${VERIFY_ALLCHIP_SSD_BINS}

all: verify_mychip_binaries

sbl: verify_sbl_binaries_${MYCHIP}

sagesw: verify_ssf_binaries_${MYCHIP} verify_ta_binaries_${MYCHIP} ${VERIFY_MYCHIP_SECURE_LOG_BINS}

verify_mychip_binaries :  verify_binaries_${MYCHIP} ${VERIFY_MYCHIP_SECURE_LOG_BINS}

ifeq ($(B_REFSW_VERBOSE),)
SAGE_QUIET_BINARIES ?= >/dev/null
endif


${VERIFY_ALLCHIP_BINARIES} : verify_binaries_% : verify_sbl_binaries_% verify_ssf_binaries_% verify_ta_binaries_%
	${Q_}echo "All $* SAGE binaries verified matching metadata." ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_SBL_BINS} : verify_sbl_binaries_% : ${SAGEBL_PROD_BINS_META}.%.txt ${SAGEBL_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${SAGEBL_PROD_BINS_META}.$*.txt $*/sage_bl.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${SAGEBL_DEV_BINS_META}.$*.txt $*/dev/sage_bl_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_SSF_BINS} : verify_ssf_binaries_% : ${SSF_PROD_BINS_META}.%.txt ${SSF_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${SSF_PROD_BINS_META}.$*.txt $*/sage_framework.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${SSF_DEV_BINS_META}.$*.txt $*/dev/sage_framework_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_TA_BINS} : verify_ta_binaries_% : verify_ar_% verify_bp3_% verify_dtcp_% verify_edrm_% verify_hdcp_% verify_keymaster_% verify_mfg_% verify_nf_% verify_pr25_% verify_pr30_% verify_svp_% verify_ssd_% verify_util_% verify_wv_%

verify_ta_binaries_7278B0 : verify_ta_binaries_% : verify_ar_% verify_bp3_% verify_dtcp_% verify_edrm_% verify_hdcp_% verify_mfg_% verify_nf_% verify_pr25_% verify_pr30_% verify_svp_% verify_util_% verify_wv_%

${VERIFY_ALLCHIP_ANTIROLLBACK_BINS} : verify_ar_% : ${ANTIROLLBACK_PROD_BINS_META}.%.txt ${ANTIROLLBACK_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${ANTIROLLBACK_PROD_BINS_META}.$*.txt $*/sage_ta_antirollback.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${ANTIROLLBACK_DEV_BINS_META}.$*.txt $*/dev/sage_ta_antirollback_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_BP3_BINS} : verify_bp3_% : ${BP3_PROD_BINS_META}.%.txt ${BP3_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${BP3_PROD_BINS_META}.$*.txt $*/sage_ta_bp3.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${BP3_DEV_BINS_META}.$*.txt $*/dev/sage_ta_bp3_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_DTCPIP_BINS} : verify_dtcp_% : ${DTCPIP_PROD_BINS_META}.%.txt ${DTCPIP_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${DTCPIP_PROD_BINS_META}.$*.txt $*/sage_ta_dtcpip.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${DTCPIP_DEV_BINS_META}.$*.txt $*/dev/sage_ta_dtcpip_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_EDRM_BINS} : verify_edrm_% : ${EDRM_PROD_BINS_META}.%.txt ${EDRM_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${EDRM_PROD_BINS_META}.$*.txt $*/sage_ta_edrm.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${EDRM_DEV_BINS_META}.$*.txt $*/dev/sage_ta_edrm_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_HDCP22_BINS} : verify_hdcp_% :  ${HDCP22_PROD_BINS_META}.%.txt ${HDCP22_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${HDCP22_PROD_BINS_META}.$*.txt $*/sage_ta_hdcp22.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${HDCP22_DEV_BINS_META}.$*.txt $*/dev/sage_ta_hdcp22_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_KEYMASTER_BINS} : verify_keymaster_% : ${KEYMASTER_PROD_BINS_META}.%.txt ${KEYMASTER_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${KEYMASTER_PROD_BINS_META}.$*.txt $*/sage_ta_keymaster.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${KEYMASTER_DEV_BINS_META}.$*.txt $*/dev/sage_ta_keymaster_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_MANUFACTURING_BINS} : verify_mfg_% : ${MANUFACTURING_PROD_BINS_META}.%.txt ${MANUFACTURING_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${MANUFACTURING_PROD_BINS_META}.$*.txt $*/sage_ta_manufacturing.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${MANUFACTURING_DEV_BINS_META}.$*.txt $*/dev/sage_ta_manufacturing_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_NETFLIX_BINS} : verify_nf_% : ${NETFLIX_PROD_BINS_META}.%.txt ${NETFLIX_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${NETFLIX_PROD_BINS_META}.$*.txt $*/sage_ta_netflix.bin ${SAGE_FW}  ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${NETFLIX_DEV_BINS_META}.$*.txt $*/dev/sage_ta_netflix_dev.bin ${SAGE_FW}  ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_PLAYREADY_25_BINS} : verify_pr25_% : ${PLAYREADY_25_PROD_BINS_META}.%.txt ${PLAYREADY_25_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${PLAYREADY_25_PROD_BINS_META}.$*.txt $*/sage_ta_playready_25.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${PLAYREADY_25_DEV_BINS_META}.$*.txt $*/dev/sage_ta_playready_25_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_PLAYREADY_30_BINS} : verify_pr30_% : ${PLAYREADY_30_PROD_BINS_META}.%.txt ${PLAYREADY_30_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${PLAYREADY_30_PROD_BINS_META}.$*.txt $*/sage_ta_playready_30.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${PLAYREADY_30_DEV_BINS_META}.$*.txt $*/dev/sage_ta_playready_30_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_MYCHIP_SECURE_LOG_BINS} : verify_secure_log_% : ${SECURE_LOG_PROD_BINS_META}.%.txt ${SECURE_LOG_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${SECURE_LOG_PROD_BINS_META}.$*.txt $*/sage_ta_secure_log.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${SECURE_LOG_DEV_BINS_META}.$*.txt $*/dev/sage_ta_secure_log_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}


${VERIFY_ALLCHIP_SECURE_VIDEO_BINS} : verify_svp_% : ${SECURE_VIDEO_PROD_BINS_META}.%.txt ${SECURE_VIDEO_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${SECURE_VIDEO_PROD_BINS_META}.$*.txt $*/sage_ta_secure_video.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${SECURE_VIDEO_DEV_BINS_META}.$*.txt $*/dev/sage_ta_secure_video_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_SSD_BINS} : verify_ssd_% : ${SSD_PROD_BINS_META}.%.txt ${SSD_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${SSD_PROD_BINS_META}.$*.txt $*/sage_ta_ssd.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${SSD_DEV_BINS_META}.$*.txt $*/dev/sage_ta_ssd_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_UTILITY_BINS} : verify_util_% : ${UTILITY_PROD_BINS_META}.%.txt ${UTILITY_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${UTILITY_PROD_BINS_META}.$*.txt $*/sage_ta_utility.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${UTILITY_DEV_BINS_META}.$*.txt $*/dev/sage_ta_utility_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

${VERIFY_ALLCHIP_WIDEVINE_BINS} : verify_wv_% : ${WIDEVINE_PROD_BINS_META}.%.txt ${WIDEVINE_DEV_BINS_META}.%.txt
	@echo [Binary... $@]
	${Q_}${VERIFY_SCRIPT} ${WIDEVINE_PROD_BINS_META}.$*.txt $*/sage_ta_widevine.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}
	${Q_}${VERIFY_SCRIPT} ${WIDEVINE_DEV_BINS_META}.$*.txt $*/dev/sage_ta_widevine_dev.bin ${SAGE_FW} ${SAGE_QUIET_BINARIES}

distclean:
	${RM} -r ${SAGE_FW}/7*

release: ${VERIFY_ALLCHIP_BINARIES}
	@echo "All SAGE binaries downloaded for release successfully."
