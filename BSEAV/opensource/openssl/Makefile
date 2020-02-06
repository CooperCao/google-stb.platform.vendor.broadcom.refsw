#############################################################################
# Copyright (C) 2019 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################

SHELL := /bin/bash

THIS_DIR := BSEAV/opensource/openssl
B_REFSW_TOP := $(subst /${THIS_DIR},,${CURDIR})

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus



# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))

include $(NEXUS_TOP)/../BSEAV/opensource/openssl/openssl.inc

POD_DIR := $(B_REFSW_OBJ_ROOT)/BSEAV/cable/lib/mpod

B_OPENSSL_TOOLCHAIN_ARCH ?= $(notdir $(B_REFSW_CROSS_COMPILE))

B_OPENSSL_TAR_PATH:=${B_OPENSSL_SOURCE_DEST}
B_OPENSSL_TAR_FILENAME:=$(B_OPENSSL_NAME).tar.gz
B_OPENSSL_LIB_BUILD_CFLAGS?=
B_OPENSSL_LIB_BUILD_LDFLAGS?=

ifeq ($(findstring 1.0.2,$(B_OPENSSL_VER)),1.0.2)
B_OPENSSL_CONFIG_NO_RIPEMD := no-ripemd
B_OPENSSL_INSTALL_PREFIX := INSTALL_PREFIX
B_OPENSSL_CONFIG_NO_ASYNC :=
ifeq ($(findstring beta1,$(B_OPENSSL_VER)),beta1)
B_OPENSSL_CONFIG_ENTRY := "${B_OPENSSL_TOOLCHAIN_ARCH}"  ,"gcc :-fPIC -DLENDIAN -DTERMIO -O3 -fomit-frame-pointer -Wall :: -D_REENTRANT ::-ldl: BN_LLONG RC4_CHAR RC4_INDEX DES_INT DES_UNROLL DES_RISC2 :::::::::::::::::dlfcn:linux-shared:-fPIC:-shared:.so.\\$$(SHLIB_MAJOR).\\$$(SHLIB_MINOR)",
else #not beta1
ifeq ($(findstring beta1,$(B_OPENSSL_VER)),beta2)
B_OPENSSL_CONFIG_ENTRY := "${B_OPENSSL_TOOLCHAIN_ARCH}"  ,"gcc :-fPIC -DLENDIAN -DTERMIO -O3 -fomit-frame-pointer -Wall :: -D_REENTRANT ::-ldl: BN_LLONG RC4_CHAR RC4_INDEX DES_INT DES_UNROLL DES_RISC2 :::::::::::::::::dlfcn:linux-shared:-fPIC:-shared:.so.\\$$(SHLIB_MAJOR).\\$$(SHLIB_MINOR)",
else #not beta2
#newer versions have one more config item
B_OPENSSL_CONFIG_ENTRY := "${B_OPENSSL_TOOLCHAIN_ARCH}"  ,"gcc :-fPIC -DLENDIAN -DTERMIO -O3 -fomit-frame-pointer -Wall :: -D_REENTRANT ::-ldl: BN_LLONG RC4_CHAR RC4_INDEX DES_INT DES_UNROLL DES_RISC2 ::::::::::::::::::dlfcn:linux-shared:-fPIC:-shared:.so.\\$$(SHLIB_MAJOR).\\$$(SHLIB_MINOR)",
endif
endif
else #not 1.0.2
ifeq ($(findstring 1.1,$(B_OPENSSL_VER)),1.1)
#1.1.1 doesn't use B_OPENSSL_CONFIG_ENTRY.
B_OPENSSL_CONFIG_NO_RIPEMD :=
B_OPENSSL_INSTALL_PREFIX := DESTDIR
ifneq ($(findstring 4.5.,$(shell $(B_REFSW_CROSS_COMPILE)gcc -dumpversion)),)
B_OPENSSL_CONFIG_NO_ASYNC :=no-async
else
B_OPENSSL_CONFIG_NO_ASYNC :=
endif
else
B_OPENSSL_CONFIG_ENTRY := "${B_OPENSSL_TOOLCHAIN_ARCH}"  ,"gcc :-fPIC -DLENDIAN -DTERMIO -O3 -fomit-frame-pointer -Wall :: -D_REENTRANT ::-ldl: BN_LLONG RC4_CHAR RC4_INDEX DES_INT DES_UNROLL DES_RISC2 :::::::::::::::::dlfcn:linux-shared:-fPIC:-shared:.so.\\$$(SHLIB_MAJOR).\\$$(SHLIB_MINOR)",
B_OPENSSL_CONFIG_NO_RIPEMD := no-ripemd
B_OPENSSL_INSTALL_PREFIX := INSTALL_PREFIX
B_OPENSSL_CONFIG_NO_ASYNC :=
endif
endif
.PHONY: clean all bin-target target-clean

ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
    $(warning B_REFSW_OPENSSL_IS_EXTERNAL=${B_REFSW_OPENSSL_IS_EXTERNAL} Skipping build of OpenSSL...)
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)

all: install

#  Set up some make output filtering to get a medium amount of neatly formatted output (unless VERBOSE is enabled).
B_OPENSSL_EXTRACT_REDIR :=
B_OPENSSL_CONFIG_REDIR  :=
B_OPENSSL_COMPILE_REDIR :=
B_OPENSSL_INSTALL_REDIR :=

ifeq ($(B_REFSW_VERBOSE),)
	B_OPENSSL_EXTRACT_REDIR := 2>/dev/null  |  egrep --line-buffered  "===" ; test $${PIPESTATUS[0]}
	B_OPENSSL_CONFIG_REDIR := 2>/dev/null  |  													\
			egrep --line-buffered  "===|making depend in |making links in " |   		\
			sed -u 																		\
					-e  "s/^making depend in \(.*\)$$/[Depends... \1 (OpenSSL)]/ "		\
					-e  "s/^making links in \(.*\)$$/[Symlinking... \1 (OpenSSL)]/ "	\
			; test $${PIPESTATUS[0]}
	B_OPENSSL_COMPILE_REDIR := 2>/dev/null  |  											\
			egrep --line-buffered  "===|making all in " |   					\
			sed -u 																\
					-e  "s/^making all in \(.*\)$$/[Build... \1 (OpenSSL)]/ "	\
			; test $${PIPESTATUS[0]}
	B_OPENSSL_INSTALL_REDIR := &>/dev/null
endif


.source:
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@if [ ! -d "${B_OPENSSL_SOURCE_PATH}" ]; then \
		echo "OpenSSL source directory: $(abspath ${B_OPENSSL_SOURCE_PATH}) not found, extracting from tarball..."  ${B_OPENSSL_EXTRACT_REDIR}; \
		if [ ! -e "${B_OPENSSL_TAR_PATH}/${B_OPENSSL_TAR_FILENAME}" ]; then \
			echo "================ Downloading OPENSSL tarball"; \
			mkdir -p ${B_OPENSSL_TAR_PATH}; \
			B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR} $(NEXUS_TOP)/../BSEAV/tools/build/fetch.sh ${B_OPENSSL_BUILD_DIR}/${B_OPENSSL_NAME}.info ${B_OPENSSL_TAR_PATH}; \
		fi ;\
		if [ -e "${B_OPENSSL_TAR_PATH}/${B_OPENSSL_TAR_FILENAME}" ]; then \
			echo "================ Extracting OPENSSL tarball"; \
			mkdir -p ${B_OPENSSL_SOURCE_PATH}; \
			tar -xzf ${B_OPENSSL_TAR_PATH}/${B_OPENSSL_TAR_FILENAME} -C ${B_OPENSSL_SOURCE_PATH}/..; \
			if [ -e "openssl-${B_OPENSSL_VER}-fix_parallel_build-1.patch" ]; then \
				echo "applying patch openssl-${B_OPENSSL_VER}-fix_parallel_build-1.patch";\
				patch -t -d ${B_OPENSSL_SOURCE_PATH} -p1 < openssl-${B_OPENSSL_VER}-fix_parallel_build-1.patch ${B_OPENSSL_EXTRACT_REDIR}; \
			fi;\
			if [ -e "openssl-${B_OPENSSL_VER}-bcm_build_fixes-1.patch" ]; then \
				echo "applying patch openssl-${B_OPENSSL_VER}-bcm_build_fixes-1.patch";\
				patch -t -d ${B_OPENSSL_SOURCE_PATH} -p1 < openssl-${B_OPENSSL_VER}-bcm_build_fixes-1.patch ${B_OPENSSL_EXTRACT_REDIR}; \
			fi; \
			if [ "${CABLECARD_SUPPORT}" = "y" ]; then \
				if [ -e "openssl-${B_OPENSSL_VER}-mpod.patch" ]; then \
					echo "applying patch openssl-${B_OPENSSL_VER}-mpod.patch to OPENSSL source"; \
					patch -t -d ${B_OPENSSL_SOURCE_PATH} -p1 < openssl-${B_OPENSSL_VER}-mpod.patch ${B_OPENSSL_EXTRACT_REDIR}; \
				fi; \
			fi; \
		else \
			echo "Tarball ${B_OPENSSL_TAR_PATH}/${B_OPENSSL_TAR_FILENAME} is missing"; exit 1;\
		fi;\
	fi
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)

# The OpenSSL makefile has problems generating dependencies if $(CC) isn't "gcc", so strip off
# the "ccache" prefix (if it's there) and remove the toolchain prefix.
ifeq ($(B_REFSW_CCACHE),)
    B_OPENSSL_CC:=${CC}
else
    B_OPENSSL_CC:=$(strip $(subst $(B_REFSW_CCACHE),,$(CC)))
endif
B_OPENSSL_CC:=$(strip $(subst $(B_OPENSSL_TOOLCHAIN_ARCH),,$(notdir $(B_OPENSSL_CC))))

.configure: .source
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@if [ ! -e "${B_OPENSSL_SOURCE_PATH}/.configured" ]; then \
		echo "================ Configuring OPENSSL"; \
		if [ -d ${B_OPENSSL_SOURCE_PATH}/Configurations ]; then \
			cp ${B_OPENSSL_BUILD_DIR}/20-b_refsw.conf ${B_OPENSSL_SOURCE_PATH}/Configurations; \
		else \
			sed -ie 's/^\(\#\#\#\# Variety of LINUX\).*/\1 \n${B_OPENSSL_CONFIG_ENTRY}/' ${B_OPENSSL_SOURCE_PATH}/Configure; \
		fi; \
		cd ${B_OPENSSL_SOURCE_PATH}; \
		CC="$(B_OPENSSL_CC)" \
		CFLAGS="${B_OPENSSL_LIB_BUILD_CFLAGS}" \
		LDFLAGS="${B_OPENSSL_LIB_BUILD_LDFLAGS}" \
		./Configure \
		enable-tls \
		no-zlib \
		threads \
		no-idea \
		no-mdc2 \
		no-rc5 \
		${B_OPENSSL_CONFIG_NO_RIPEMD} \
		${B_OPENSSL_CONFIG_NO_ASYNC} \
		shared \
		--cross-compile-prefix=$(B_OPENSSL_TOOLCHAIN_ARCH) \
		${B_OPENSSL_TOOLCHAIN_ARCH} \
		-pthread \
		--prefix=/usr/local; \
		${MAKE} -C ${B_OPENSSL_SOURCE_PATH} depend; \
		touch ${B_OPENSSL_SOURCE_PATH}/.configured; \
		echo "================ OPENSSL Configured"; \
	fi  ${B_OPENSSL_CONFIG_REDIR}
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)

MAKE_SUFFIX:=APPS=

.$(B_OPENSSL_NAME): .source .configure
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@echo "================ Compiling OpenSSL"
	@echo "[Making... all (OpenSSL)]"
	@${MAKE} ${MAKE_OPTIONS} -C ${B_OPENSSL_SOURCE_PATH} ${MAKE_SUFFIX}  ${B_OPENSSL_COMPILE_REDIR}
	@echo "[Making... install_sw (OpenSSL)]"
	@${MAKE} -C ${B_OPENSSL_SOURCE_PATH} install_sw ${B_OPENSSL_INSTALL_PREFIX}=${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}  ${MAKE_SUFFIX} ${B_OPENSSL_COMPILE_REDIR}
ifeq ($(findstring 1.1,$(B_OPENSSL_VER)),1.1)
	@echo "[Making... install_ssldirs (OpenSSL)]"
	@${MAKE} -C ${B_OPENSSL_SOURCE_PATH} install_ssldirs ${B_OPENSSL_INSTALL_PREFIX}=${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}  ${MAKE_SUFFIX} ${B_OPENSSL_COMPILE_REDIR}
endif
	@rm -rf ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_VER="$(B_OPENSSL_VER) >> ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_NAME="$(B_OPENSSL_NAME) >> ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_PATH="$(B_OPENSSL_TOP_INSTALL_DIR) >> ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_CFLAGS:="$(B_OPENSSL_CFLAGS) >> ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_LDFLAGS:="$(B_OPENSSL_LDFLAGS) >> ${B_OPENSSL_VER_FILENAME}
	@echo "B_OPENSSL_PKG_PATH:="$(B_OPENSSL_PKG_FOLDER) >> ${B_OPENSSL_VER_FILENAME}
	@echo "================ OpenSSL Compiled"
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)


#  For the "install" target, blindly copying all of the files (including the openssl header files)
#  will lead to the rebuilding of everything that includes those header files.  So here's a small
#  make function that only copies the files when they are different (or if they don't exist at the
#  destination).
copy_if_different = @SRCDIR=$(shell dirname '$(1)')  ;\
                     SRCFIL=$(shell basename '$(1)') ;\
                     DSTDIR='$(2)'                   ;\
						 for FILE in `find $$SRCDIR  \( -type f -o -type l \) -name "$$SRCFIL"  -printf "%P "`  ; do \
                         diff -q $$SRCDIR/$$FILE $$DSTDIR/$$FILE 2>/dev/null || (mkdir -p $$(dirname $$DSTDIR/$$FILE) && cp -v -dp $$SRCDIR/$$FILE  $$DSTDIR/$$FILE)  ; \
                     done


USE_COPY_IF_DIFFERENT := y


install: .$(B_OPENSSL_NAME)
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@echo "================ Installing OpenSSL"
	@mkdir -p ${B_OPENSSL_LIB_INSTALL_DIR}
	@mkdir -p ${B_OPENSSL_INC_INSTALL_DIR}
	@mkdir -p ${B_OPENSSL_BIN_INSTALL_DIR}/ssl

	@cp -af ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/lib/*.so*   ${B_OPENSSL_LIB_INSTALL_DIR}   ${B_OPENSSL_INSTALL_REDIR}
	@cp -af ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/lib/engines* ${B_OPENSSL_LIB_INSTALL_DIR}   ${B_OPENSSL_INSTALL_REDIR}
ifeq ($(USE_COPY_IF_DIFFERENT),y)
	$(call copy_if_different,${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/include/*,${B_OPENSSL_INC_INSTALL_DIR})  ${B_OPENSSL_INSTALL_REDIR}
else
	@cp -af ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/include/* ${B_OPENSSL_INC_INSTALL_DIR}    ${B_OPENSSL_INSTALL_REDIR}
endif
	@cp -af ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/bin/* ${B_OPENSSL_BIN_INSTALL_DIR}   ${B_OPENSSL_INSTALL_REDIR}
	@cp -af ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/ssl ${B_OPENSSL_BIN_INSTALL_DIR}/../   ${B_OPENSSL_INSTALL_REDIR}
	@echo "================ OpenSSL Installed"
ifeq ($(CABLECARD_SUPPORT),y)
	@mkdir -p $(B_REFSW_OBJ_ROOT)/BSEAV/cable/lib/mpod/crypto/include/internal
	@echo "================ Installing OPENSSL headers for POD"
	@cp -afLR ${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local/include ${POD_DIR}/crypto
ifeq ($(findstring 1.1,$(B_OPENSSL_VER)),1.1)
	@cp -afLR ${B_OPENSSL_SOURCE_PATH}/include/internal/ ${POD_DIR}/crypto/include/
	@cp -afLR ${B_OPENSSL_SOURCE_PATH}/crypto/include/internal/x509_int.h ${POD_DIR}/crypto/include/internal
endif
	@cp -afLR ${B_OPENSSL_SOURCE_PATH}/apps/apps.h ${POD_DIR}/crypto/include
	@echo "================ Installing OPENSSL apps objects for POD"
	@cp -af ${B_OPENSSL_SOURCE_PATH}/apps/apps.o   ${POD_DIR}/apps.o
ifeq ($(findstring 1.1,$(B_OPENSSL_VER)),1.1)
	@cp -af ${B_OPENSSL_SOURCE_PATH}/apps/opt.o ${POD_DIR}/opt.o
	@cp -af ${B_OPENSSL_SOURCE_PATH}/apps/bf_prefix.o ${POD_DIR}/bf_prefix.o
endif
ifeq ($(B_REFSW_ARCH),arm-linux)
	@echo "================ Installing OPENSSL libcrypto for ARM platforms POD"
ifeq ($(B_REFSW_TOOLCHAIN_VER_GE_4_8),y)
	@cp -af ${B_OPENSSL_SOURCE_PATH}/libcrypto.a ${POD_DIR}/libcrypto.a
else
	@cp -af ${B_OPENSSL_SOURCE_PATH}/libcrypto.a ${POD_DIR}/libcrypto_uclibc.a
endif
else
	@echo "================ Installing OPENSSL libcrypto for MIPS platforms POD"
	@cp -af ${B_OPENSSL_SOURCE_PATH}/libcrypto.a   ${POD_DIR}/libcrypto.${B_OPENSSL_VER}_uclibc.a
endif
	@echo "================ MPOD OpenSSL Installed"
endif # ifeq ($(CABLECARD_SUPPORT),y)
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)

target-clean:
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@echo "================ Removing target OpenSSL binaries"
	@rm -rf ${B_OPENSSL_LIB_INSTALL_DIR}/libssl.so*
	@rm -rf ${B_OPENSSL_LIB_INSTALL_DIR}/libcrypto.so*
	@rm -rf ${B_OPENSSL_LIB_INSTALL_DIR}/engines
	@rm -rf ${B_OPENSSL_INC_INSTALL_DIR}/openssl
	@rm -rf ${B_OPENSSL_INC_INSTALL_DIR}/../ssl
	@rm -rf ${B_OPENSSL_BIN_INSTALL_DIR}/openssl
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)

clean:
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@echo "================ Cleaning OpenSSL"
	@rm -rf ${B_OPENSSL_VER_FILENAME}
	@rm -rf ${B_OPENSSL_SOURCE_PATH}
	@rm -rf ${NEXUS_PLATFORM}
	@echo "================ Done"
ifeq ($(CABLECARD_SUPPORT),y)
ifeq ($(B_REFSW_ARCH),arm-linux)
ifeq ($(B_REFSW_TOOLCHAIN_VER_GE_4_8),y)
	@rm -rf ${POD_DIR}/libcrypto.a
	@rm -rf ${POD_DIR}/apps.o
	@rm -rf ${POD_DIR}/crypto/include
else
	@rm -rf ${POD_DIR}/libcrypto_uclibc.a
	@rm -rf ${POD_DIR}/apps_uclibc.o
	@rm -rf ${POD_DIR}/crypto/include
endif
else
	@rm -rf ${POD_DIR}/libcrypto.${B_OPENSSL_VER}_uclibc.a
	@rm -rf ${POD_DIR}/apps_uclibc.o
	@rm -rf ${POD_DIR}/crypto/include
endif # ifeq ($(B_REFSW_ARCH),arm-linux)
endif # ifeq ($(CABLECARD_SUPPORT),y)
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
