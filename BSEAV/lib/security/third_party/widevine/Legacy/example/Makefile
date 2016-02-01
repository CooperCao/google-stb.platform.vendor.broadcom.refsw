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

# FlashLite build scripts make assumptions about version of bin-utils (make, tar, etc) so, change path to point into the alternative location of such utils
TOOLS_BIN?=/tools/bin
CURL_VER?=curl-7.21.2
export PATH := ${TOOLS_BIN}:${PATH}


NEXUS_TOP = $(NEXUS)
BSEAV_TOP = $(NEXUS)/../BSEAV
PLAYBACKDEVICE_DIR = $(BSEAV_TOP)/lib/playbackdevice
WIDEVINE_DIR = $(BSEAV_TOP)/lib/security/third_party/widevine
EXAMPLE_DIR=$(WIDEVINE_DIR)/Legacy/example
B_LIB_TOP = $(addsuffix /lib,$(NEXUS))

include $(BSEAV_TOP)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

include $(NEXUS)/platforms/$(PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/security/common_drm/lib/cmndrm_dir.inc
include $(BSEAV_TOP)/lib/openssl/openssl.inc




CXXFLAG = -Wall -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CXXFLAG += -mips32 -D_GNU_SOURCE=1


# Include build rules
CXXFLAG += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CXXFLAG += $(addprefix -I,$(B_LIB_TOP)/os/include) $(addprefix -I,$(B_LIB_TOP)/os/include/linuxuser)
CXXFLAG += -I${BSEAV_TOP}/lib/security/common_drm/include


PLAYBACKDEVICE_INCLUDE_PATH = $(BSEAV_TOP)/lib/playbackdevice/src
WIDEVINE_LIB_PATH = $(WIDEVINE_DIR)/Legacy/lib
WIDEVINE_INCLUDE_PATH = $(WIDEVINE_DIR)/Legacy/include

CXXFLAG += $(addprefix -I,$(WIDEVINE_INCLUDE_PATH)) $(addprefix -I,$(CURL_INCLUDE_PATH)) $(addprefix -I,$(LINUX)/include.mipsel-uclibc.97425)
CXXFLAG += $(addprefix -I,$(LINUX)/include.mipsel-uclibc.$(PLATFORM)) $(addprefix -I,$(PLAYBACKDEVICE_INCLUDE_PATH))
CXXFLAG += -DPLAYBACKDEVICE -DBCMCARD_7425 -DBROADCOM_HARDWARE_CRYPTO



LDFLAGS = -ldl -L $(PLAYBACKDEVICE_DIR)/bin -lPlaybackDevice -lnexusMgr -lpthread -lm -lpthread -L$(NEXUS_BIN_DIR)/ -lnexus -lcrypto -lssl -lxml2 -lb_os
LDFLAGS += -L $(WIDEVINE_LIB_PATH) -lWVStreamControlAPI -lWVPlaybackAPI -lcurl
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/lib/${CMNDRM_DIR}/ -lcmndrm
LDFLAGS += -L$(BSEAV_TOP)/lib/security/bcrypt/${BCRYPT_LIBDIR} -lbcrypt
LDFLAGS += -L$(BSEAV_TOP)/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
LDFLAGS += $(B_OPENSSL_LDFLAGS)

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


all: prep_openssl prep_libb_os widevine_example

prep_openssl:
	echo [..... prep_openssl target];
	$(Q_)$(MAKE) -C $(B_OPENSSL_BUILD_DIR)

prep_libb_os:
	@if [ ! -f "${NEXUS_TOP}/bin/libb_os.so" ]; then \
		echo "${NEXUS_TOP}/bin/libb_os.* not found, calling make in ${NEXUS_TOP}/lib/os"; \
		make -C ${NEXUS_TOP}/lib/os; \
	fi

clean: widevine_example-clean

lib-environment:
	@if [ -z "${PLATFORM}"  ] || \
	   [ -z "${BCHP_VER}"  ] || \
	   [ -z "${MAGNUM}"  ] || \
	   [ -z "${NEXUS}"  ]; then \
		echo ; echo ;\
		echo "I detected that one of the following environment variables is missing:"; \
		echo "PLATFORM"=${PLATFORM}; \
		echo "BCHP_VER"=${BCHP_VER}; \
		echo "NEXUS"=${NEXUS}; \
		echo "EXAMPLE_DIR"=${EXAMPLE_DIR}; \
		echo "You should define ALL of these if you expect this build to work"; \
		exit 1; \
	fi

widevine_example: lib-environment
	@echo "==========>Building Playback Device Example application"
	@echo "NEXUS =" $(NEXUS)
	@echo "CURR_DIR =" $(CURR_DIR)
	@echo "PLAYBACKDEVICE_INCLUDE_PATH =" $(PLAYBACKDEVICE_INCLUDE_PATH)
	@echo "CURL_INCLUDE_PATH =" $(CURL_INCLUDE_PATH)
	@echo "B_LIB_TOP  =" $(B_LIB_TOP)
	@echo "NEXUS_MGR_PATH =" $(NEXUS_MGR_PATH)
	@echo "NEXUS_LIB_PATH =" $(NEXUS_LIB_PATH)
	@echo "EXAMPLE_DIR"=${EXAMPLE_DIR};
	@echo "commondrm_DIR"=${CMNDRM_DIR};


	@${CXX} ${CXXFLAG} -I../. -c app_wvplayback.cpp -o app_wvplayback.o
	@${CXX} -o $(EXAMPLE_DIR)/TestWVPlayback app_wvplayback.o ${LDFLAGS}

widevine_example-clean:
	rm -rf $(BSEAV_TOP)/lib/security/third_party/widevine/example/*.o
	rm -rf $(BSEAV_TOP)/lib/security/third_party/widevine/example/TestWVPlayback
	rm -rf $(NEXUS_BIN_DIR)/TestWVPlayback

install:all
	cp -f $(BSEAV_TOP)/lib/security/third_party/widevine/example/TestWVPlayback  $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/security/third_party/widevine/example/tvtest.sh $(NEXUS_BIN_DIR)
	cp -f $(WIDEVINE_LIB_PATH)/*.so* $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/security/common_drm/lib/${CMNDRM_DIR}/libcmndrm.so $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/security/bcrypt/${BCRYPT_LIBDIR}/libbcrypt.so $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so $(NEXUS_BIN_DIR)
	cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto* $(NEXUS_BIN_DIR)
	cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libssl* $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/playbackdevice/bin/*.so $(NEXUS_BIN_DIR)
	cp -f $(BSEAV_TOP)/lib/playbackdevice/bin/*.so $(NEXUS_BIN_DIR)
