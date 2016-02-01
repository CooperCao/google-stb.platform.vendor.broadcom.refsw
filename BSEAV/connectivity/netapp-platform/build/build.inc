############################################################################
#    (c)2015 Broadcom Corporation
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
############################################################################
ifndef NETAPP_TOP
$(error NETAPP_TOP not defined and needs to point to the NetApp VOB root directory)
endif

ifeq (${VERBOSE},y)
Q_					=
SQUASH_OUTPUT		=
#For parallel make, the following can be helpful to separate build logs
#SQUASH_OUTPUT		= > build_outputt.$$$$.log 2>&1
else
Q_					=@
SQUASH_OUTPUT		= > /dev/null 2>&1
endif

ifndef MAKE
MAKE				= make
endif

###############################################################################
# 					B U I L D   E N V I R O N M E N T
# Figure out how we're going to build NetApp (cross compile or for i386)
################################################################################
ifndef LINUX
LINUXNAME       :=`uname -r`
LINUX           :=/lib/modules/${LINUXNAME}/build
SKIP_CROSS_SET  = y
endif

KERNELVERSION           := $(shell cat ${LINUX}/include/config/kernel.release)
LINUXVER                := $(shell echo ${KERNELVERSION} | sed -e 's/-/ /' | sed 's/\s.*//')

KCONFIG_FILE 			:= ${LINUX}/.config
-include ${KCONFIG_FILE}


ifndef SKIP_CROSS_SET

ifneq (${B_REFSW_ARCH},)
CROSS_HOST              = $(B_REFSW_ARCH)
endif
ifeq (${CROSS_HOST},)
$(error LINUX is set but not CROSS_HOST!)
endif

ifneq ($(findstring mips,${CROSS_HOST}), )
ifeq (${CONFIG_CPU_BIG_ENDIAN},y)
NETAPP_ENDIAN           = be
ENDIAN_FLAGS           += -EB
else
NETAPP_ENDIAN           = le
ENDIAN_FLAGS           += -EL
endif
endif

ifneq ($(findstring arm,${CROSS_HOST}), )
CFLAGS                 += -marm
endif
endif
CROSS_COMPILE           = ${CROSS_HOST}-


ifndef NETAPP_OUTPUT_DIR
NETAPP_OUTPUT_DIR       := $(NETAPP_TOP)/../../../obj.$(NEXUS_PLATFORM)/BSEAV/bin
endif

NETAPPPLATFORM_OBJ_DIR := $(NETAPP_OUTPUT_DIR)/../netapp-platform
NETAPP_PREBUILT_DIR     = ${NETAPP_TOP}/prebuilt
NETAPP_OUTPUT_LIB_DIR   = ${NETAPP_OUTPUT_DIR}/lib
NETAPP_OUTPUT_FW_DIR    = ${NETAPP_OUTPUT_DIR}/firmware/brcm
NETAPP_TARGET_FW_DIR    = firmware/brcm
NETAPP_OUTPUT_INC_DIR   = ${NETAPP_OUTPUT_DIR}/include
NETAPP_OUTPUT_BIN_DIR   = ${NETAPP_OUTPUT_DIR}/bin
NETAPP_OUTPUT_MODULES_DIR= ${NETAPP_OUTPUT_LIB_DIR}/modules
NETAPP_OUTPUT_AP_DIR 	?= ${NETAPP_OUTPUT_DIR}/ap
NETAPP_OUTPUT_AP_DIR_NIC?= ${NETAPP_OUTPUT_AP_DIR}/nic
NETAPP_OUTPUT_AP_DIR_FD?= ${NETAPP_OUTPUT_AP_DIR}/fd
NETAPP_PREBUILT_TARBALL = ${NETAPP_PREBUILT_DIR}/NetApp_${KERNELVERSION}_${NETAPP_ENDIAN}.tbz
HAS_PREBUILT            = $(wildcard ${NETAPP_PREBUILT_TARBALL})

ifndef BUILD_TIME
BUILD_TIME = $(shell date +%Y%m%d_%H%M)
endif

ifndef APPLIBS_TARGET_DIR
NETAPP_TARGET_PLATFORM  = ${NEXUS_PLATFORM}${BCHP_VER_LOWER}.
endif

ifeq ($(B_REFSW_DEBUG),y)
CFLAGS +=-g
endif
CFLAGS                  += -fPIC -I${NETAPP_OUTPUT_INC_DIR}
CPPFLAGS                += -Wall -Wno-write-strings
################################################################################

################################################################################
# Check for External OpenSSL, if found we use that rather than the WLAN version
###############################################################################
EXTERNAL_OPENSSL_LIB := $(wildcard ${NETAPP_OUTPUT_LIB_DIR}/libcrypto.so)
EXTERNAL_OPENSSL_INC := $(wildcard ${NETAPP_OUTPUT_INC_DIR}/openssl)
ifneq (${EXTERNAL_OPENSSL_INC},)
EXTERNAL_OPENSSL=1
EXTERNAL_LIBS=-lz
EXTERNAL_OPENSSL_BASE=${NETAPP_OUTPUT_DIR}
else
EXTERNAL_OPENSSL=0
endif

################################################################################
#                   Check for NEXUS Support
################################################################################
ifndef NEXUS_TOP
NEXUS_TOP= $(wildcard $(NETAPP_TOP)/../../../nexus)

ifeq ($(NEXUS_TOP),)
NEXUS_TOP= $(wildcard $(NETAPP_TOP)/../nexus)
endif

ifneq ($(NEXUS_TOP),)
export NEXUS_TOP:=$(NEXUS_TOP)
endif
endif

CFLAGS					+= ${ENDIAN_FLAGS}
CC 						= ${CROSS_COMPILE}gcc
CXX 					= ${CROSS_COMPILE}g++
LD 						= ${CROSS_COMPILE}ld
STRIP 					= ${CROSS_COMPILE}strip
AR						= ${CROSS_COMPILE}ar
OBJCOPY					= ${CROSS_COMPILE}objcopy

ifneq (${MULTI_BUILD},n)
HOST_NPROCS := $(shell grep '^processor' /proc/cpuinfo | wc -l)
HOST_LOAD   = $(shell uptime | sed -e 's/^.*load average: //' -e 's/[.].*//')
#MAKE_J_ARG  = $(shell if [ ${HOST_NPROCS} -gt ${HOST_LOAD} ] ; then let "X = ${HOST_NPROCS} - ${HOST_LOAD}" && echo -n "-j $$X"  ; fi)
#MAKE_J_ARG  = $(shell hload=${HOST_LOAD}; if [ ${HOST_NPROCS} -gt $$hload ] ; then let "X = ${HOST_NPROCS} - $$hload" && echo -n "-j $$X"  ; fi)
endif

$(NETAPPPLATFORM_OBJ_DIR)/%.o : %.c
	@printf "[${CC}] <------------- %s \n" $<
	${Q_} ${CC}  ${CONLYFLAGS} $(LOCAL_CFLAGS) ${CFLAGS} ${CPPFLAGS} -c $<  -o $@

$(NETAPPPLATFORM_OBJ_DIR)/%.o : %.cpp
	@printf "[${CXX}] <------------- %s \n" $<
	${Q_} ${CXX} $(LOCAL_CFLAGS) ${CFLAGS} ${CPPFLAGS} -c $<  -o $@

%.bin: %.o
	@printf "[${OBJCOPY}] <--------- %s \n" $<
	$(Q_)$(CC) $(CFLAGS) -c -o $^ $(patsubst %.bin,%.c,$@);
	$(Q_)$(OBJCOPY) -j .data -O binary $^ $@
