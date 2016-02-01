#############################################################################
#    (c)2010-2015 Broadcom Corporation
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
#
#############################################################################A


NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
MAGNUM_TOP := $(shell cd ../../../../magnum; pwd)

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/Makefile

# NB bhsm inc files now export ZEUS_VERSION

############################################################################
#
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
ifeq ($(ANDROID_BUILD),y)
    LIB_BASENAME        := nexus_security_android
else
    LIB_BASENAME        := nexus_security
endif

D_ALL_MODULE_DIR	:= $(filter-out include%, $(shell find . -maxdepth 2 -type d))

ifeq ($(ANDROID_BUILD),y)
    ANDROID_DIR := android
endif

############
# Set F_SRC_EXCLUDES to the basenames of C/C++ files you would like to
# exclude from compilation.  By default, all *.c and *.cpp files will
# be compiled.
#####
F_SRC_EXCLUDES   = nexus_security_ipc_server.lo
F_SRC_EXCLUDES  += %nexus_security_thunks.o
F_SRC_EXCLUDES  += %nexus_security_thunks.lo



############
# Set F_PUBLIC_INCS to the basenames of include files that are to be
# used by other libraries or applications.  These files will be
# installed into .../${BCHP_VER}/include/
#####
F_PUBLIC_INCS    	+= $(sort $(notdir $(shell find ${D_ALL_MODULE_DIR} -name '*.h')))



############
# If your library depends on another shared lib, you should add that
# lib here (and any additional lib directories to search).
#####
LDFLAGS				+=

ifeq ($(ANDROID_BUILD),y)
    LDFLAGS += -EL
    CFLAGS += -EL
endif

############
# Directory where to install the headers
#####
D_FOR_INC_INSTALL	:=


############
# You probably shouldn't have to modify the text below this point.
#####
BHSM_OBJS := $(filter $(NEXUS_OBJ_DIR)/MAGNUM/bhsm%, $(MAGNUM_OBJECTS))

F_PUBLIC_LIBS    	+= lib${LIB_BASENAME}.a
ZEUS_DIR                := $(ZEUS_VERSION)$(ZEUS_DIR_SUPPLEMENTARY_TAG)
OS_DIR                 	:= $(B_REFSW_OS)
ifeq ($(NEXUS_MODE),proxy)
    OS_DIR               := $(B_REFSW_OS).$(NEXUS_MODE)
endif

ZEUS_LIB_DIR             := $(ZEUS_DIR)/$(OS_DIR)

ifneq ($(B_REFSW_DEBUG),y)
    SEC_LIB_MODE := retail
else
    SEC_LIB_MODE := debug
endif

ifeq ($(ANDROID_BUILD),y)
    D_FOR_LIB_INSTALL := $(ZEUS_LIB_DIR)/$(SEC_LIB_MODE)/$(ANDROID_DIR)
else
    D_FOR_LIB_INSTALL := $(ZEUS_LIB_DIR)/$(SEC_LIB_MODE)
endif

_NEXUS_SECURITY_OBJECTS := $(filter-out %_driver.o, $(NEXUS_SECURITY_OBJECTS))
_NEXUS_SECURITY_OBJECTS := $(filter-out %_export.o, $(_NEXUS_SECURITY_OBJECTS))

F_OBJS 		   	:= $(_NEXUS_SECURITY_OBJECTS) ${BHSM_OBJS}
F_OBJS 			:= $(filter-out %${F_SRC_EXCLUDES}, $(F_OBJS))

F_INSTALLED_LIBS	:= $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_PUBLIC_LIBS})

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^
############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
sec_all: ${F_PUBLIC_LIBS}


veryclean:
	@ls *.a  | xargs rm -f
	@ls *.so | xargs rm -f
	@rm -f ${F_OBJS}
	@rm -f ${F_INSTALLED_LIBS}
	@rm -rf $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/bin${NEXUS_BIN_DIR_SUFFIX}/

clean:
	@rm -f ${F_OBJS}


#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
lib${LIB_BASENAME}.so: ${F_OBJS}
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-dy,-lstdc++
	${STRIP_COMMAND}

lib${LIB_BASENAME}.a: ${F_OBJS}
	@echo "[Link...... $(notdir $@)]"
	@ [ ! -f $@ ] || rm $@
	@${AR} rc  $@  ${F_OBJS}
	${STRIP_COMMAND}

idirs:
	@ [ -d ${ZEUS_DIR} ] || mkdir -p ${ZEUS_DIR}
	@ [ -d ${ZEUS_LIB_DIR} ] || mkdir -p ${ZEUS_LIB_DIR}
	@ [ -d ${D_FOR_LIB_INSTALL} ] || mkdir -p ${D_FOR_LIB_INSTALL}

sec_install: sec_all idirs ${F_INSTALLED_LIBS}

${D_FOR_LIB_INSTALL}/%.a : %.a
	@echo "[Install... $(notdir $@)]"
	@install -m 0755 $< $@

${D_FOR_LIB_INSTALL}/%.so : %.so
	install -m 0755 $< $@

${F_PREBUILT_OBJS} : %.o : %.obj
	${BDVD_PPRINT.cp}
	@cp $^ $@
