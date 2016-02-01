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
############################################################

NEXUS_TOP = $(NEXUS)

# Include platform build objects
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

# Include build rules
CFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I $(NEXUS)/lib/os/include
CFLAGS += -I $(NEXUS)/lib/os/include/linuxuser
CFLAGS += -I $(NEXUS_MGR_DIR)
CFLAGS += -g
CFLAGS += -fno-rtti

@if [ "$DEBUG" = "y" ]; then \
    CFLAGS += -DBDBG_DEBUG_BUILD=1; \
fi;

ifeq ($(PLAYBACKDEVICE_STAND_ALONE_APPLICATION), y)
    CFLAGS += -DPLAYBACKDEVICE_STAND_ALONE_APP 
endif

CFLAGS += -DNEXUS
CFLAGS += -D'BSTD_CPU_ENDIAN=1234'
CFLAGS += -DPLAYBACKDEVICE

CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -D_LARGEFILE_SOURCE
CFLAGS += -D_LARGEFILE64_SOURCE
CFLAGS += -DBCHP_ENDIAN=le
CFLAGS += -DLINUX

ifeq ($(PLAYBACKDEVICE_STAND_ALONE_APPLICATION), y)
B_LIB_TOP = $(addsuffix /lib,$(NEXUS))
NEXMGR_INCLUDE_PATH = $(addsuffix /nexusMgr,$(NEXUS_MGR_DIR))
CFLAGS += $(addprefix -I,$(B_LIB_TOP)/os/include) $(addprefix -I,$(B_LIB_TOP)/os/include/linuxuser)
CFLAGS += $(addprefix -I,$(NEXMGR_INCLUDE_PATH))
endif

all: PlaybackDevice

lib-environment:
	@if [ -z "${PLATFORM}"  ] || \
	   [ -z "${NEXUS_MGR_DIR}"  ] || \
	   [ -z "${BCHP_VER}"  ] || \
	   [ -z "${MAGNUM}"  ] || \
	   [ -z "${PLAYBACKDEVICE_STAND_ALONE_APPLICATION}"  ] || \
	   [ -z "${NEXUS}"  ]; then \
		echo ; echo ;\
		echo "I detected that one of the following environment variables is missing:"; \
		echo "PLATFORM"=${PLATFORM}; \
		echo "BCHP_VER"=${BCHP_VER}; \
		echo "NEXUS"=${NEXUS}; \
		echo "NEXUS_MGR_DIR"=${NEXUS_MGR_DIR}; \
		echo "PLAYBACKDEVICE_STAND_ALONE_APPLICATION"=${PLAYBACKDEVICE_STAND_ALONE_APPLICATION}; \
		echo "You should define ALL of these if you expect this build to work"; \
		exit 1; \
	fi

PlaybackDevice: lib-environment
	@echo "==========>Building PlaybackDevice library"
	@echo "CFLAGS = " $(CFLAGS)
	@echo "B_LIB_TOP = " $(B_LIB_TOP)
	@echo "EDK_LIBDIR = " $(EDK_LIBDIR)
	@echo "NEXUS_MGR_DIR = " $(NEXUS_MGR_DIR)
	@echo "NEXMGR_INCLUDE_PATH="$(NEXMGR_INCLUDE_PATH)

	@if ! [ -e "${EDK_LIBDIR}" ]; then \
		mkdir -p ${EDK_LIBDIR}; \
	fi;

	@${CC} ${CFLAGS} -shared -DPIC -fpic -L${SC_BCM_NEXUS_LIBDIR} -lnexus PlaybackDevice.cpp EventQueue.cpp -o ${EDK_LIBDIR}/libPlaybackDevice.so;

clean:
	rm -rf ${EDK_LIBDIR}/libPlaybackDevice.so

