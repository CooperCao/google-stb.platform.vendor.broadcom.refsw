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
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

NEXUS_TOP = $(NEXUS)

##
#
# 2. Include platform build objects
#
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

#In case nexusMgr is built standalone
ifeq ($(NEXUSMGR_LIBDIR),)
NEXUSMGR_LIBDIR = .
export NEXUSMGR_LIBDIR
endif

ifeq ($(NEXUSMGR_OBJDIR),)
NEXUSMGR_OBJDIR:= obj
export NEXUSMGR_OBJDIR
endif

##
#
# 3. Include build rules
#
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))   -DPIC -fpic
CXXFLAGS = -Wall -Werror -fno-exceptions -fno-rtti -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC

ifeq ($(findstring nexus,$(SC_PLATFORM)),nexus)
CFLAGS += -D_SC_DIRECT_NEXUS_
CXXFLAGS += -D_SC_DIRECT_NEXUS_
endif

ifeq ($(DEBUG),y)
CFLAGS += -DBDBG_DEBUG_BUILD=1
else
CFLAGS += -DBDBG_DEBUG_BUILD=0
endif

CXXFLAGS += -mips32 -D_GNU_SOURCE=1 -D_DEBUG  -O1

all: nexusMgr
clean: nexusMgr-clean

nexusMgr: nexusMgr.c nexusMgr.h TestHarness.cpp TestHarness.h
	@echo "==========>Building NexusMgr"
	@if ! [ -e "${NEXUSMGR_LIBDIR}" ]; then \
		mkdir -p ${NEXUSMGR_LIBDIR}; \
	fi;
	@if ! [ -e "${NEXUSMGR_OBJDIR}" ]; then \
		mkdir -p ${NEXUSMGR_OBJDIR}; \
	fi;

	@${CC} ${CFLAGS} -c nexusMgr.c -o ${NEXUSMGR_OBJDIR}/nexusMgr.o
	@${CXX} ${CXXFLAGS} -c TestHarness.cpp -o ${NEXUSMGR_OBJDIR}/TestHarness.o
	@${CXX} -shared -L${SC_BCM_NEXUS_LIBDIR} -lnexus ${NEXUSMGR_OBJDIR}/nexusMgr.o ${NEXUSMGR_OBJDIR}/TestHarness.o -o ${NEXUSMGR_LIBDIR}/libnexusMgr.so


nexusMgr-clean:
	rm -f ${NEXUSMGR_OBJDIR}/nexusMgr.o
	rm -f ${NEXUSMGR_OBJDIR}/TestHarness.o
	@if [ -e "${NEXUSMGR_OBJDIR}" ]; then \
		rmdir --ignore-fail-on-non-empty ${NEXUSMGR_OBJDIR}; \
	fi;
	rm -rf ${NEXUSMGR_LIBDIR}/libnexusMgr.so
