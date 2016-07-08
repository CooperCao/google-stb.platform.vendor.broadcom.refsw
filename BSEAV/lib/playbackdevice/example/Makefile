############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
############################################################

# FlashLite build scripts make assumptions about version of bin-utils (make, tar, etc) so, change path to point into the alternative location of such utils
TOOLS_BIN?=/tools/bin
CURL_VER?=curl-7.21.2
export PATH := ${TOOLS_BIN}:${PATH}


NEXUS_TOP = $(NEXUS)
BSEAV_TOP = $(NEXUS_TOP)/../BSEAV
CURR_DIR= $(BSEAV_TOP)/lib/playbackdevice/example
#NEXUS_LIB_PATH = $(NEXUS)/platforms/$(NEXUS_PLATFORM)/bin/linuxuser.mipsel-uclibc.release

ifeq ($(DEBUG), n)
   NEXUS_LIB_PATH = $(NEXUS)/platforms/$(NEXUS_PLATFORM)/bin/linuxuser.${ARCH}.release
else
   NEXUS_LIB_PATH = $(NEXUS)/platforms/$(NEXUS_PLATFORM)/bin/linuxuser.${ARCH}.debug
endif

# Include platform build objects
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

#ifeq ($(ARCH), mips-uclibc)
#FL_CXX := mips-uclibc-g++
#else
FL_CXX := mipsel-linux-g++
#endif

B_LIB_TOP = $(addsuffix /lib,$(NEXUS))

#CXXFLAG += -g -DBDBG_DEBUG_BUILD=1
#CXXFLAG += -g

#CXXFLAG = -Wall -Werror -fno-exceptions -fno-rtti -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CXXFLAG = -Wall -fno-exceptions -fno-rtti -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CXXFLAG += -mips32 -D_GNU_SOURCE=1 -D_DEBUG  -O1

# Include build rules
CXXFLAG += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Include build rules
CXXFLAG += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CXXFLAG += $(addprefix -I,$(B_LIB_TOP)/os/include) $(addprefix -I,$(B_LIB_TOP)/os/include/linuxuser) $(addprefix -I,$(BSEAV_TOP)/lib/security/common_drm/include)

NEXUS_MGR_PATH = $(addsuffix /nexusMgr,$(CURR_DIR)/..)
CXXFLAG += $(addprefix -I,$(NEXUS_MGR_PATH))

CURL_INCLUDE_PATH = $(NEXUS)/../AppLibs/opensource/libcurl/$(CURL_VER)/include
PLAYBACKDEVICE_INCLUDE_PATH = $(CURR_DIR)/../src
#CXXFLAG += $(addprefix -I,$(CURL_INCLUDE_PATH)/curl) $(addprefix -I,$(CURL_INCLUDE_PATH)) $(addprefix -I,$(LINUX)/include.mipsel-uclibc.935230)
CXXFLAG += $(addprefix -I,$(CURL_INCLUDE_PATH)/curl) $(addprefix -I,$(CURL_INCLUDE_PATH)) $(addprefix -I,$(LINUX)/include.mipsel-uclibc.$(NEXUS_PLATFORM)) $(addprefix -I,$(PLAYBACKDEVICE_INCLUDE_PATH))

CXXFLAG += -DPLAYBACKDEVICE

LDFLAGS = -ldl -L $(CURR_DIR)/../bin -lnexusMgr -lPlaybackDevice -lpthread -lm -lnexus -lb_os -lcurl -lcrypto -lssl
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/lib/${BCHP_CHIP}/ -lcmndrm
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR) -lbcrypt


ifeq ($(findstring nexus,$(SC_PLATFORM)),nexus)
PLAT_DIR = nexus
else
PLAT_DIR = directFB
endif

ifeq ($(ARCH), mips-uclibc)
ENDIAN_DIR = be
else
ENDIAN_DIR = le
endif

ifeq ($(NEXUSMGR_LIBDIR),)
NEXUSMGR_LIBDIR = ../../../../bin
export NEXUSMGR_LIBDIR
endif

all: prep_drmrootfs prep_bcrypt playbackdevice_example

clean: playbackdevice_example-clean

prep_drmrootfs:
	@echo [Compiling drmrootfs dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install

prep_bcrypt:
	@echo [Compiling Bcrypt dependency ....] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > /dev/null

lib-environment:
	@if [ -z "${BCHP_VER}"  ] || \
	   [ -z "${NEXUS}"  ]; then \
		echo ; echo ;\
		echo "I detected that one of the following environment variables is missing:"; \
		echo "BCHP_VER"=${BCHP_VER}; \
		echo "NEXUS"=${NEXUS}; \
		echo "You should define ALL of these if you expect this build to work"; \
		exit 1; \
	fi

playbackdevice_example: lib-environment
	@echo "==========>Building Playback Device Example application"
	@echo "NEXUS =" $(NEXUS)
#	@echo "CURR_DIR =" $(CURR_DIR)
	@echo "PLAYBACKDEVICE_INCLUDE_PATH =" $(PLAYBACKDEVICE_INCLUDE_PATH)
	@echo "CURL_INCLUDE_PATH =" $(CURL_INCLUDE_PATH)
	@echo "B_LIB_TOP =" $(B_LIB_TOP)
	@echo "NEXUS_MGR_PATH =" $(NEXUS_MGR_PATH)
	@echo "NEXUS_LIB_PATH =" $(NEXUS_LIB_PATH)
#	@echo "CXXFLAG =" $(CXXFLAG)

	@${FL_CXX} ${CXXFLAG} -I../. -c data_reader.cpp -o data_reader.o
	${FL_CXX} ${CXXFLAG} -I../. -c app_playback_device.cpp -o app_playback_device.o
	@${FL_CXX} -o $(CURR_DIR)/app_playback_device data_reader.o app_playback_device.o ${LDFLAGS}

playbackdevice_example-clean:
	rm -rf *.o
	rm -rf app_playback_device
