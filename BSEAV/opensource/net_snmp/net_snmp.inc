###############################################################################
#   Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
    DEBUG_SUFFIX=debug
else
    DEBUG_SUFFIX=release
endif

NET_SNMP_VER?=5.6.2.1

NET_SNMP_NAME:=net_snmp-$(NET_SNMP_VER)
NET_SNMP_NAME2:=net-snmp-$(NET_SNMP_VER)
NET_SNMP_SOURCE_TARBALL:=$(NET_SNMP_NAME2).tar.gz
NET_SNMP_SOURCE_DIRECTORY:=$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/opensource/net_snmp/$(B_REFSW_ARCH).$(DEBUG_SUFFIX))
NET_SNMP_SOURCE_PATH:=${NET_SNMP_SOURCE_DIRECTORY}/${NET_SNMP_NAME}
NET_SNMP_TAR_PATH:=${NET_SNMP_SOURCE_DIRECTORY}
NET_SNMP_BUILD_DIR:=$(abspath $(NEXUS_TOP)/../BSEAV/opensource/net_snmp)
NET_SNMP_DIR:=$(NET_SNMP_NAME2)
NET_SNMP_LIB_FOLDER:=${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}/snmplib
NET_SNMP_EXE_FOLDER:=${NET_SNMP_TAR_PATH}/${NET_SNMP_NAME2}/src

NET_SNMP_CFLAGS:= -D_REENTRANT -I${NET_SNMP_TAR_PATH}/${NET_SNMP_DIR}/include
NET_SNMP_LDFLAGS:= -L$(NET_SNMP_LIB_FOLDER) -lnet-snmp

#  Where to store downloaded open-source files for local sharing
B_REFSW_CACHE_DIR ?= $(realpath /projects/stbdev/open-source)
