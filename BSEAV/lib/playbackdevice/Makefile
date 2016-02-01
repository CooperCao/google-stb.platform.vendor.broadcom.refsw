############################################################
#     (c)2008-2011 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
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
############################################################

BSEAV_TOP := $(shell cd ../../../BSEAV; pwd)

# Directory Paths
ifeq ($(PLAYBACKDEVICE_STAND_ALONE_APPLICATION),y)
NEXUSMGR_SRC_PATH:= $(BSEAV_TOP)/lib/playbackdevice/nexusMgr
endif 

CFLAGS += -I $(NEXUS)/lib/os/include
CFLAGS += -I $(NEXUS_MGR_DIR)


all: nexusMgr playbackdevice

clean: lib-clean

export NEXUSMGR_SRC_PATH

lib-environment:
	@if [ -z "${PLATFORM}"  ] || \
	   [ -z "${BCHP_VER}"  ] || \
	   [ -z "${SC_PLATFORM}"  ] || \
	   [ -z "${NEXUS}"  ]; then \
		echo ; echo ;\
		echo "I detected that one of the following environment variables is missing:"; \
		echo "PLATFORM"=${PLATFORM}; \
		echo "BCHP_VER"=${BCHP_VER}; \
		echo "NEXUS"=${NEXUS}; \
		echo "SC_PLATFORM(=nexus)"=${SC_PLATFORM}; \
		echo "You should define ALL of these if you expect this build to work"; \
		exit 1; \
	fi
	@if ! which mipsel-linux-gcc ; then \
		echo "path ="${PATH};\
		echo ; echo ;\
		echo "I detected your path variable does not have a mipsel-linux-gcc available"; \
		echo "You should add the toolchain bin path to your PATH variable"; \
		echo "start compiling.........";\
		exit 1; \
	fi;

.PHONY: nexusMgr playbackdevice

ifeq ($(PLAYBACKDEVICE_STAND_ALONE_APPLICATION),y)
nexusMgr: lib-environment
	${MAKE} -C $(NEXUSMGR_SRC_PATH)
endif 
playbackdevice: lib-environment
	${MAKE} ${CFLAGS} -C $(BSEAV_TOP)/lib/playbackdevice/src

lib-clean: clean_playbackdevice clean_nexusmgr
	@echo "lib-clean done ..."

clean_playbackdevice:
	@echo "clean_playbackdevice: in progress"

	${MAKE} ${CFLAGS} -C $(BSEAV_TOP)/lib/playbackdevice/src clean

clean_nexusmgr:
	@echo "clean_nexusmgr: in progress"
	${MAKE} -C $(NEXUSMGR_SRC_PATH) clean

