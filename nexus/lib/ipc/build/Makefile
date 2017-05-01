############################################################################
#  Copyright (C) 2011-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
###########################################################################

NEXUS_TOP := $(shell cd ../../..; pwd)
BSEAV := ${NEXUS_TOP}/../BSEAV
MAGNUM := ${NEXUS_TOP}/../magnum
ifdef B_REFSW_ARCH
CUSTOM_FLAGS := 1
endif

# include cross-compiler definitions
ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
NEXUS_PLATFORM ?= $(PLATFORM)
ifeq ($(NEXUS_PLATFORM),)
$(error NEXUS_PLATFORM is not defined)
endif
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif


ifndef CUSTOM_FLAGS
CFLAGS += $(NEXUS_CFLAGS) 
else
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -DBDBG_DEBUG_BUILD=1  -g -O
endif
CFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
   
   
ODIR = ${B_REFSW_OBJ_ROOT}/nexus/lib/ipc/build/$(B_REFSW_ARCH)
OS = ${B_REFSW_OS}

include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc

#CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
#CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
SRCS += $(notdir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))
vpath %.c $(dir $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES)))

CFLAGS += -I.. -I${ODIR}
vpath %.c ..

SERVER_SRCS := bipc_server.c bipc_util.c bipc_server_test.c ${SRCS} 
CLIENT_SRCS := bipc_client.c bipc_util.c bipc_client_test.c ${SRCS} 

INTERFACES =  bsspk_decoder.h bsac_session.h
CLIENT_IPC = ${ODIR}/ipc_client.h
SERVER_IPC = ${ODIR}/ipc_server.h

SERVER = ${ODIR}/server
CLIENT = ${ODIR}/client

ifeq (${OS},linuxuser)
LDFLAGS += -lpthread
endif

all: ${ODIR}/flag apps

.NOTPARALLEL: 

apps: ${SERVER}	${CLIENT}

${ODIR}/flag:
	@${MKDIR} ${ODIR}
	@${TOUCH} $@

${SERVER_IPC} ${CLIENT_IPC}: ${INTERFACES} ${bipc_build.pl}
	perl -I ${NEXUS_TOP}/build/tools/abiverify -I ${NEXUS_TOP}/build/tools/common bipc_build.pl ${SERVER_IPC} ${CLIENT_IPC} ${INTERFACES}

${SERVER}: ${SERVER_SRCS} ${SERVER_IPC}
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $(filter-out %.h, $^) $(CFLAGS) $(LDFLAGS)

${CLIENT}: ${CLIENT_SRCS} ${CLIENT_IPC}
	@echo $(CC) -o $@
	$(Q_)$(CC) -o $@ $(filter-out %.h, $^) $(CFLAGS) $(LDFLAGS)

debug: apps
	echo "set args ${CLIENT}" >.gdbinit
	echo "r" >>.gdbinit
	gdb ${SERVER}
	rm .gdbinit

test: apps
	${SERVER} ${CLIENT}

clean:
	${RM} -rf ${ODIR}



