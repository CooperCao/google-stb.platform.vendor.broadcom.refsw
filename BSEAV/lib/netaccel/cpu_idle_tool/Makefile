############################################################
#     (c)2003-2013 Broadcom Corporation
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
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
##########################################################


#If you want to get the system wide MIPS performance stats (i/d, L2, RAC caches miss, hit, # of cycles & instructions), 
# enable this define.
#EXTRA_CFLAGS += -DBMIPS_PERFC_ENABLE

#Also, if you want to get the MIPS performance stats for a specific kernel module or specific portion of base kernel, 
# you will need to also enabled this define.
#In addition, you will also need to modify kernel to invoke snapshot calls (Look in the README file).
# EXTRA_CFLAGS += -DPERFC_INSTRUMENT_KERNEL


ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif

ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux mipsel-uclibc mipsel-linux-uclibc mipsel-linux-android), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc mips-linux-android), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mips-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= arm-linux-
endif
endif
endif

#tools
CC = $(B_REFSW_CROSS_COMPILE)gcc

ifeq ($(BINDIR),)
BINDIR = ../../../../obj.$(NEXUS_PLATFORM)/nexus/bin
endif

obj-m += bcm_perf_drv.o

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), arm-linux)
bcm_perf_drv-objs += barm_idle_stats.o bmips_perf_stats.o bmips_perf_drv.o
else
bcm_perf_drv-objs += bmips_idle_stats.o bmips_perf_stats.o bmips_perf_drv.o
endif


ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), arm-linux)
EXTRA_CFLAGS += -DB_PERF_ARM_V7
else
ifeq ($(findstring $(PLATFORM),97420 97410 97409 93380sms 93380vms 97425 97422 97346 97435), $(PLATFORM))
    EXTRA_CFLAGS += -DB_PERF_BMIPS_5000
else
    EXTRA_CFLAGS += -DB_PERF_BMIPS_4380
endif
endif

EXTRA_FLAGS += -ggdb -fno-inline -O0 -Wall


all:bcm_perf_test
	make -C $(LINUXDIR) M=$(shell pwd) modules

clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean

install: all
	test -d "$(BINDIR)" || mkdir -p $(BINDIR) && cp -f  bcm_perf_drv.ko  $(BINDIR)

bcm_perf_test:bcm_perf_test.c
	$(CC) -Wall -o bcm_perf_test bcm_perf_test.c
