############################################################
#	  (c)2012 Broadcom Corporation
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
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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

APP = profile
CFLAGS += -W -Wall
CFLAGS += -fstrict-aliasing -frename-registers
CFLAGS += -Os  
SRCS += profile_app.c profile_idct.c 
#SRCS += bunit_alu.c
SRCS += bunit_fpu.c

MAGNUM=../../../../magnum

B_REFSW_OS ?= linuxuser

include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/commonutils/lst/blst.inc

MAGNUM_INCLUDES := $(sort $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
MAGNUM_CFLAGS := $(addprefix -D,$(MAGNUM_DEFINES)) $(addprefix -I,$(MAGNUM_INCLUDES))
CFLAGS += -I. $(MAGNUM_CFLAGS)

vpath %.c ../ $(MAGNUM)/basemodules/kni/dbg $(MAGNUM)/basemodules/kni/$(B_REFSW_OS)

LDFLAGS += -lpthread 

CFLAGS_BPROFILE = -finstrument-functions -DB_HAS_BPROFILE=1
#for now compile symbol table only for BPROFILE
CFLAGS += -DB_HAS_SYMTABLE
CFLAGS += -I.. -Istd
CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -DBDBG_DEBUG_BUILD=1
CFLAGS += ${CFLAGS_BPROFILE} 
CFLAGS += ${OTHER_CFLAGS}
ifeq ($(findstring $(PLATFORM),97425), $(PLATFORM))
CFLAGS += -DBCHP_CHIP=7425
CFLAGS += -I../../../linux/driver/usermode
CFLAGS += -DBPROFILE_CFG_PERF_COUNTER=2
SRCS+= profile_extra.c
endif
ifeq ($(findstring $(PLATFORM),97405), $(PLATFORM))
CFLAGS += -DBCHP_CHIP=7405
endif


SRCS += bsymtable.c
SRCS += bprofile.c bprofile_data.c btrc.c bperf_counter.c
SRCS += bdbg.c bdbg_os_priv.c bdbg_fifo.c bdbg_output.c bkni.c
SYM_SRC = bsymbols.c

OBJS = $(SRCS:%.c=%.o)
APP_IMAGE = ./profile_app

all: application

test: application
	${APP_IMAGE}  ${ARG}

debug: application
	echo "set args ${ARG}" >.gdbinit
	echo "r" >>.gdbinit
	gdb ${APP_IMAGE}
	rm .gdbinit


install: application	
	cp ${APP_IMAGE} ${DESTDIR}



SYM_OBJ = $(SYM_SRC:%.c=%.o)
SYM_INC = $(SYM_SRC:%.c=%.inc)

${SYM_OBJ} : ${SYM_SRC} ${OBJS}
	# compile  empty sym-table and link with it 
	rm -f ${SYM_INC}
	echo '/* */' >${SYM_INC}
	$(CC) $(CFLAGS) -I. -c $< -o $@
	$(CC) $(OBJS) ${SYM_OBJ} $(LDFLAGS) ${LIBS} -o ${APP_IMAGE}.sym
	# compile with real sym-table but possibly wrong offsets
	rm ${SYM_INC}
	nm -f bsd -n --defined-only ${APP_IMAGE}.sym|awk '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	$(CC) $(CFLAGS) -I. -c $< -o $@
	$(CC) $(OBJS) ${SYM_OBJ} $(LDFLAGS) ${LIBS} -o ${APP_IMAGE}.sym
	# build real symtable and compile with it
	${RM) ${SYM_INC}
	nm -f bsd -n --defined-only ${APP_IMAGE}.sym|awk '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	$(CC) $(CFLAGS) -I. -c $< -o $@
	mv ${SYM_INC} $(SYM_INC:%.inc=%.sym)

%.o : %.c 
	$(CC) $(CFLAGS) -c $< -o $@


${APP_IMAGE}: ${OBJS} ${SYM_OBJ}
	$(CC) $(OBJS) ${SYM_OBJ} $(LDFLAGS) ${LIBS} -o $@

application: ${APP_IMAGE}

clean:
	rm -vf ${OBJS} ${SYM_OBJ} ${APP_IMAGE}


