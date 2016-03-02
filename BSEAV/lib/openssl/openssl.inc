#############################################################################
#    (c)2010-2016 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTLIBCURLLAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
#############################################################################

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif


#  Define the OpenSSL's Makefile directory... probably the same dir that this file is in.
B_OPENSSL_BUILD_DIR := $(abspath $(NEXUS_TOP)/../BSEAV/lib/openssl)

B_OPENSSL_VER?=1.0.1r
B_OPENSSL_NAME?=openssl-${B_OPENSSL_VER}

B_OPENSSL_SOURCE_DEST:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/lib/openssl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX))
B_OPENSSL_SOURCE_PATH:=${B_OPENSSL_SOURCE_DEST}/${B_OPENSSL_NAME}
B_OPENSSL_TOP_INSTALL_DIR ?= $(abspath ${B_OPENSSL_SOURCE_PATH}/..)

#  Define the dirs where the Broadcom openssl wrapper makefile installs things.
B_OPENSSL_LIB_INSTALL_DIR ?= ${B_OPENSSL_TOP_INSTALL_DIR}/lib
B_OPENSSL_INC_INSTALL_DIR ?= ${B_OPENSSL_TOP_INSTALL_DIR}/include
B_OPENSSL_BIN_INSTALL_DIR ?= ${B_OPENSSL_TOP_INSTALL_DIR}/bin

#  Define the dirs where the real (open-source) OpenSSL makefile installs things.
B_OPENSSL_BASE_FOLDER:=${B_OPENSSL_SOURCE_PATH}/${NEXUS_PLATFORM}/usr/local
B_OPENSSL_LIB_FOLDER:=${B_OPENSSL_BASE_FOLDER}/lib
B_OPENSSL_INC_FOLDER:=${B_OPENSSL_BASE_FOLDER}/include
B_OPENSSL_PKG_FOLDER:=${B_OPENSSL_BASE_FOLDER}/lib/pkgconfig
B_OPENSSL_BIN_FOLDER:=${B_OPENSSL_BASE_FOLDER}/bin

#  The version filename that we create during the build.
B_OPENSSL_VER_FILENAME:=${B_OPENSSL_SOURCE_PATH}/../../openssl_ver.inc

#  CFLAGS and LDFLAGS
B_OPENSSL_CFLAGS:= -I${B_OPENSSL_INC_INSTALL_DIR}
B_OPENSSL_LDFLAGS:= -L${B_OPENSSL_LIB_INSTALL_DIR}/ -lssl -lcrypto

#  Where to store downloaded open-source files for local sharing
B_REFSW_CACHE_DIR ?= $(realpath /projects/stbdev/open-source)
