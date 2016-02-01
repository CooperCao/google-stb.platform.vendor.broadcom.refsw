############################################################################
#    (c)2013 Broadcom Corporation
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
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
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
# @author Steven Hartley (steven@broadcom.com)
#
# DO NOT MODIFY THIS FILE WITHOUT CONSULTATION WITH NETAPP TEAM
#
############################################################################
ifndef NETAPP_TOP
$(error NETAPP_TOP not defined and needs to point to the NetApp root directory)
endif

LIB_NAME := $(shell basename ${CURDIR})
include ${NETAPP_TOP}/build/build.inc

export CFLAGS	:= ${CFLAGS} -fPIC -D_REENTRANT -I${NETAPP_OUTPUT_INC_DIR}
export CXXFLAGS	:= ${CXXFLAGS} -fPIC
export LDFLAGS 	:= -L${NETAPP_OUTPUT_LIB_DIR}
export CROSS_HOST:=${CROSS_HOST}

# FIXME: --enable-shared, --enable-static are not recognized by some "configure"-s
CONFIGURE_OPTIONS +=	\
	--enable-shared \
	--enable-static \
	--host=${CROSS_HOST} \
	--prefix=${NETAPP_OUTPUT_DIR} \
	--exec-prefix=${NETAPP_OUTPUT_DIR}

LIB_PKG_TYPE 	?= $(wildcard *tar.gz)
LIB_PKG_NAME	?= $(basename $(basename $(shell ls ${LIB_PKG_TYPE})))
LIB_PKG         = $(NETAPPPLATFORM_OBJ_DIR)/$(LIB_PKG_NAME)
LIB_PATCH		= ${LIB_PKG_NAME}.patch
TAR_OPTION		?=zxf

install: ${PREP2_MARKER}

PREP0_MARKER := ${LIB_PKG}/.marker_prep0
PREP1_MARKER := ${LIB_PKG}/.marker_prep1
PREP2_MARKER := ${LIB_PKG}/.marker_prep2

${PREP0_MARKER}:
	@echo "======== Building ${LIB_NAME}..."
	@echo "============ ${LIB_NAME} Extracting Tarball(s)..."
	${Q_}rm -fr ${LIB_PKG}
	${Q_}for file in ${LIB_PKG_TYPE}; do \
		cd $(NETAPPPLATFORM_OBJ_DIR); tar ${TAR_OPTION} $(shell pwd)/$$file; \
	done
	${Q_}if [ -f ${LIB_PATCH} ]; then \
		cd $(LIB_PKG) && patch -p0 < $(shell pwd)/${LIB_PATCH} ${SQUASH_OUTPUT}; \
	fi
	@touch $@

${PREP1_MARKER}: ${PREP0_MARKER}
	@echo "============ ${LIB_NAME} Configuring..."
	${Q_}if [ -f $(LIB_PKG)/./configure ]; then \
		cd $(LIB_PKG); ./configure ${CONFIGURE_OPTIONS} ${SQUASH_OUTPUT}; \
	fi
	@touch $@

${PREP2_MARKER}: ${PREP1_MARKER}
	@echo "============ ${LIB_NAME} Building..."
	${Q_}cd ${LIB_PKG} && ${MAKE} ${MAKE_J_ARG} all CC=$(CC) NETAPP_OUTPUT_DIR=$(NETAPP_OUTPUT_DIR) ${SQUASH_OUTPUT}
	@touch $@


prep0: ${PREP0_MARKER}
prep1: ${PREP1_MARKER}
prep2: ${PREP2_MARKER}

install: ${PREP2_MARKER}
	@echo "============ ${LIB_NAME} Installing..."
	${Q_}cd ${LIB_PKG} && ${MAKE} ${MAKE_J_ARG} install CC=$(CC) NETAPP_OUTPUT_DIR=$(NETAPP_OUTPUT_DIR) ${SQUASH_OUTPUT}

clean:
	@echo "======== ${LIB_NAME} Cleanning..."
ifneq (, $(wildcard ${LIB_PKG}))
	${Q_}if [ -f ${PREP1_MARKER} ] ; then \
		cd ${LIB_PKG} && ${MAKE} clean ${SQUASH_OUTPUT};\
	fi
	${Q_}if [ -f ${PREP2_MARKER} ] ; then \
		rm ${PREP2_MARKER}; \
	fi
endif

distclean:
	@echo "======== ${LIB_NAME} Distcleanning..."
ifneq (, $(wildcard ${LIB_PKG}))
	${Q_}if [ -d ${NETAPP_OUTPUT_LIB_DIR}/${LIB_PKG} ] ; then \
		rm -rf ${NETAPP_OUTPUT_LIB_DIR}/${LIB_PKG}*; \
	fi
	${Q_}if [ -f ${PREP1_MARKER} ] ; then \
		cd ${LIB_PKG} && ${MAKE} distclean ${SQUASH_OUTPUT} || rm -rf ${LIB_PKG} ;\
	fi
	${Q_}rm -rf ${LIB_PKG}
endif
