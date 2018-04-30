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
B_REFSW_DEBUG ?= y
PIXMAN_VER?=0.32.4
PIXMAN_NAME:=pixman-$(PIXMAN_VER)
PIXMAN_SOURCE_TARBALL:=$(PIXMAN_NAME).tar.gz
PIXMAN_SOURCE_DIRECTORY:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/opensource/pixman)
PIXMAN_SOURCE_PATH:=${PIXMAN_SOURCE_DIRECTORY}/${PIXMAN_NAME}

PIXMAN_TAR_PATH:=${PIXMAN_SOURCE_DIRECTORY}/${PIXMAN_NAME}
PIXMAN_CFLAGS?= -O2 -fPIC
PIXMAN_ARCH := ${B_REFSW_ARCH}
PIXMAN_LIB_FOLDER:=${PIXMAN_TAR_PATH}/${PIXMAN_DIR}/.libs
PIXMAN_INSTALL_TARGET := $(abspath $(B_REFSW_OBJ_ROOT)/target)
PIXMAN_PREFIX=usr/local
PIXMAN_INSTALL_PREFIX := ${PIXMAN_INSTALL_TARGET}/${PIXMAN_PREFIX}
PIXMAN_CFLAGS:= -D_REENTRANT -I${PIXMAN_TAR_PATH}/${CAIRO_DIR}/include
PIXMAN_LDFLAGS:= -L$(PIXMAN_LIB_FOLDER) -lpixman-1

#  Where to store downloaded open-source files for local sharing
B_REFSW_CACHE_DIR ?= $(realpath /projects/stbdev/open-source)
