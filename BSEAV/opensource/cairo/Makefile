###############################################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
###############################################################################

THIS_DIR := BSEAV/opensource/cairo
SHELL:=/bin/bash
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OPENSOURCE ?= ${ROOT_DIR}/../
B_REFSW_TOP ?= $(subst /${THIS_DIR},,$(ROOT_DIR))

WORKING_DIR := $(B_REFSW_TOP)/$(THIS_DIR)
BSEAV_TOP := $(B_REFSW_TOP)/BSEAV
NEXUS_TOP := $(B_REFSW_TOP)/nexus
# include cross-compiler definitions
#
B_REFSW_OBJ_DIR ?= obj.$(NEXUS_PLATFORM)
B_REFSW_OBJ_ROOT ?= $(B_REFSW_TOP)/$(B_REFSW_OBJ_DIR)
include $(B_REFSW_OBJ_ROOT)/nexus/bin/include/platform_app.inc
include ${NEXUS_TOP}/nxclient/include/nxclient.inc
include ${BSEAV_TOP}/opensource/cairo/cairo.inc

FETCH = $(realpath $(BSEAV_TOP))/tools/build/fetch.sh
SQUASH_OUTPUT = > /dev/null

CXXFLAGS = -O3
NEXUS_CONFIG_CFLAGS:=$(NXCLIENT_CFLAGS) $(NEXUS_CFLAGS)
NEXUS_CONFIG_CFLAGS:=$(subst -Wstrict-prototypes,,${NEXUS_CONFIG_CFLAGS})
NEXUS_CONFIG_CFLAGS:=$(subst -pedantic,,${NEXUS_CONFIG_CFLAGS})
NEXUS_CONFIG_CFLAGS:=$(subst -std=c89,,${NEXUS_CONFIG_CFLAGS})
NEXUS_CONFIG_CFLAGS:=$(NEXUS_CONFIG_CFLAGS) -I$(CAIRO_INSTALL_TARGET)/../nexus/bin/include
NEXUS_CONFIG_LDFLAGS:=$(NEXUS_CLIENT_LD_LIBRARIES)

glesv2_CFLAGS = -I$(realpath $(BSEAV_TOP)/lib/gpu/v3d/driver/interface/khronos/include/)
glesv2_LIBS = -L${CAIRO_INSTALL_TARGET}/usr/local/lib -lv3ddriver
egl_CFLAGS = $(glesv2_CFLAGS)
egl_LIBS = $(glesv2_LIBS)

CAIRO_VERSION = 1.14.4
SOURCE_FOLDER = ./cairo-$(CAIRO_VERSION)
BUILD_FOLDER = ./build_cairo

CAIRO_SOURCE_TARBALL = $(SOURCE_FOLDER).tar.xz

CONFIGURE_SCRIPT = $(SOURCE_FOLDER)/configure

CONFIGURE_ARGS = \
	--prefix=$(CAIRO_INSTALL_PREFIX) \
	--host=$(CAIRO_ARCH) \
	--enable-glesv2=yes \
	--enable-egl=yes \
	--enable-nexus=yes \
	--disable-xcb \
	--disable-xlib \
	--disable-gobject \
	--enable-shared \
	--disable-valgrind \
	--enable-ps=no \
	--enable-pdf=no \
	--enable-svg=no \
	--disable-xlib-xrender  \
	--disable-win32 \
	--with-sysroot=$(CAIRO_INSTALL_TARGET)/usr/local

CONFIGURE_ENV = \
	CC=$(CC) \
	CFLAGS="-O2 -fPIC -I$(CAIRO_INSTALL_TARGET)/usr/local/include" \
	LDFLAGS=-L$(CAIRO_INSTALL_TARGET)/usr/local/lib \
	glesv2_CFLAGS="$(glesv2_CFLAGS)" \
	glesv2_LIBS="$(glesv2_LIBS)" \
	egl_CFLAGS="$(egl_CFLAGS)" \
	egl_LIBS="$(egl_LIBS)" \
	nexus_CFLAGS="$(NEXUS_CONFIG_CFLAGS)" \
	ac_env_nexus_CFLAGS_value="$(NEXUS_CONFIG_CFLAGS)" \
	nexus_LDFLAGS="$(NEXUS_CONFIG_LDFLAGS)" \
	ac_env_nexus_LIBS_value="$(NEXUS_CONFIG_LDFLAGS)" \
	PKG_CONFIG_PATH=$(CAIRO_INSTALL_TARGET)/usr/local/lib/pkgconfig

BUILD=$(BUILD_FOLDER)/.build

SRC_URI = "http://cairographics.org/releases/$(CAIRO_SOURCE_TARBALL)"

.PHONY: clean

all: $(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/.libs/libcairo.so

$(CAIRO_TAR_PATH)/$(CAIRO_SOURCE_TARBALL):
	@mkdir -p ${CAIRO_SOURCE_PATH}
	@$(FETCH) cairo-$(CAIRO_VER).info $(CAIRO_TAR_PATH)

$(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/.libs/libcairo.so: $(CAIRO_TAR_PATH)/$(CAIRO_SOURCE_TARBALL)
	@echo "================ Compiling Cairo"
	@tar -xf ${CAIRO_TAR_PATH}/${CAIRO_SOURCE_TARBALL} -C ${CAIRO_TAR_PATH}
	@cd ${CAIRO_TAR_PATH}/${CAIRO_NAME}; \
	cp -a $(WORKING_DIR)/broadcom_files/$(CAIRO_VERSION)/* .;\
	patch -p1 < $(WORKING_DIR)/broadcom_files/cairo-$(CAIRO_VER).patch ;\
	NOCONFIGURE=y ./autogen.sh;\
	$(CONFIGURE_ENV) ../$(CONFIGURE_SCRIPT) $(CONFIGURE_ARGS) ${SQUASH_OUTPUT}
	@${MAKE} ${MAKE_OPTIONS} -C ${CAIRO_TAR_PATH}/${CAIRO_NAME} ${MAKE_SUFFIX} ${SQUASH_OUTPUT}
	@sed -i 's/ glesv2//g' $(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/cairo.pc
	@sed -i 's/ glesv2//g' $(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/cairo-glesv2.pc
	@sed -i 's/ egl//g' $(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/cairo.pc
	@sed -i 's/ egl//g' $(CAIRO_TAR_PATH)/$(CAIRO_NAME)/src/cairo-egl.pc
	@${MAKE} ${MAKE_OPTIONS} -C ${CAIRO_TAR_PATH}/${CAIRO_NAME} install ${MAKE_SUFFIX} ${SQUASH_OUTPUT}
	@echo "================ Cairo Compiled!"

clean:
	@rm -rf $(OPENSOURCE_NAME)_ver.inc
	@rm -rf ${CAIRO_TAR_PATH}/${CAIRO_NAME}
	@rm -rf .$(CAIRO_NAME)
	@echo "================ Done"
