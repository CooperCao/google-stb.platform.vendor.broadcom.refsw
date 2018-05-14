############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# Module Description:
#   This file defines the OS-specific rules for building in linuxuser mode.
#
############################################################

############################################################
# Overview:
#
# this file should only be included by nexus.inc
# it is only used for internal nexus build
#
############################################################

# This will be the default rule unless the app overrode it.

nexus_install: nexus_all ${NEXUS_BIN_DIR}/exists $(EXTRA_INSTALL)
	@echo [Install... libraries]
ifdef NEXUS_SHARED_LIB
	@$(CP) $(NEXUS_SHARED_LIB) ${NEXUS_BIN_DIR}
endif
ifdef NEXUS_STATIC_LIB
	@$(CP) $(NEXUS_STATIC_LIB) ${NEXUS_BIN_DIR}
endif
ifneq ($(NEXUS_MODE),proxy)
	@$(CP) $(NEXUS_TOP)/build/nexus ${NEXUS_BIN_DIR}
	@$(CP) $(NEXUS_TOP)/build/nexus.client ${NEXUS_BIN_DIR}
	@chmod +x ${NEXUS_BIN_DIR}/nexus ${NEXUS_BIN_DIR}/nexus.client
endif

nexus_all: ${EXTRA_ALL} $(NEXUS_STATIC_LIB) $(NEXUS_SHARED_LIB) $(MAGNUM_LIB)

NEXUS_SYMBOLS_OBJ := $(NEXUS_SYMBOLS_SRC:%.c=$(NEXUS_OBJ_DIR)/MAGNUM/%.${NEXUS_OBJ_SUFFIX})
NEXUS_SYMBOLS_INC := $(NEXUS_SYMBOLS_SRC:%.c=$(NEXUS_OBJ_DIR)/MAGNUM/%.inc)

# Rule for magnum sources
$(filter-out ${NEXUS_SYMBOLS_OBJ}, ${MAGNUM_OBJECTS}): $(NEXUS_OBJ_DIR)/MAGNUM/%.$(NEXUS_OBJ_SUFFIX): %.c $(NEXUS_OBJ_DIR)/MAGNUM/exists
	@echo "[Compile... $(notdir $<) (magnum)]"
	$(Q_)$(CC) $(CDEP_FLAG) $(if $(filter $(notdir $<),$(MAGNUM_NOPROFILE_SRCS)),,${NEXUS_CFLAGS_BPROFILE}) $(if $(filter $(notdir $<),$(MAGNUM_NO_STRICT_ALIASING_SRCS)),-fno-strict-aliasing,) $(NEXUS_CFLAGS) -c $< -o $@
ifneq ($(filter $(B_REFSW_ARCH), arm-linux aarch64-linux),$(B_REFSW_ARCH))
	$(Q_)$(PERL) $(NEXUS_TOP)/build/os/ucos/fix_dep.pl $(dir $@) $(notdir $(@:.o=.d)) $(patsubst %.o,%.d,$@)
endif

# Ensure that exactly the same commands are used to built shared library in symbol table rules and real rules
ifeq ($(BPROFILE_SUPPORT),y)
# Don't use -Wl,-x linker options that causes exclusion of private (e.g. static functions) symbols from the symbol table
NEXUS_BUILD_SO := ${Q_}cd $(NEXUS_OBJ_DIR);$(CC) ${LDFLAGS} -shared -Wl,-soname,libnexus${NEXUS_LIB_SUFFIX}.so $(NEXUS_EXTRA_LDFLAGS) $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%, %, $(abspath ${NEXUS_OBJECTS} ${MAGNUM_OBJECTS} ${NEXUS_EXTRALIBS} ${MAGNUM_STATIC_LIBS}))
else
NEXUS_BUILD_SO := ${Q_}cd $(NEXUS_OBJ_DIR);$(CC) ${LDFLAGS} -shared  -Wl,-x -Wl,-soname,libnexus${NEXUS_LIB_SUFFIX}.so $(NEXUS_EXTRA_LDFLAGS) $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%, %, $(abspath ${NEXUS_OBJECTS} ${MAGNUM_OBJECTS} ${NEXUS_EXTRALIBS} ${MAGNUM_STATIC_LIBS}))
endif

${NEXUS_SYMBOLS_OBJ} : ${NEXUS_SYMBOLS_SRC} $(filter-out ${NEXUS_SYMBOLS_OBJ},${MAGNUM_OBJECTS}) ${NEXUS_OBJECTS} ${NEXUS_EXTRALIBS} ${MAGNUM_STATIC_LIBS}
	@echo "[Symbols... $(notdir $<)]"
	${Q_}# compile  empty sym-table and link with it
	${Q_}${RM} ${NEXUS_SYMBOLS_INC}
	${Q_}echo '/* */' >${NEXUS_SYMBOLS_INC}
	${Q_}$(CC) -I$(dir $@) $(NEXUS_CFLAGS) -c $< -o $@
	${NEXUS_BUILD_SO} -o $(abspath ${NEXUS_SHARED_LIB}.sym)
	${Q_}# compile with real sym-table but possibly wrong offsets
	${Q_}${RM} ${NEXUS_SYMBOLS_INC}
	${Q_}${NM} -f bsd -n --defined-only ${NEXUS_SHARED_LIB}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${NEXUS_SYMBOLS_INC}
	${Q_}$(CC) -I$(dir $@) $(NEXUS_CFLAGS) -c $< -o $@
	${NEXUS_BUILD_SO} -o $(abspath $@)
	${Q_}# build real symtable and compile with it
	${Q_}${RM} ${NEXUS_SYMBOLS_INC}
	${Q_}${NM} -f bsd -n --defined-only ${NEXUS_SHARED_LIB}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${NEXUS_SYMBOLS_INC}
	${Q_}$(CC) -I$(dir $@) $(NEXUS_CFLAGS) -c $< -o $@
	${Q_}${MV} ${NEXUS_SYMBOLS_INC} $(NEXUS_SYMBOLS_INC:%.inc=%.sym)

ifdef NEXUS_STATIC_LIB
# Rule for magnum library
$(MAGNUM_LIB): $(MAGNUM_OBJECTS)
	@echo "[Archive... $(notdir $@)]"
	-$(Q_)cd $(NEXUS_OBJ_DIR);$(AR) rcs $(abspath $@) $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%,%,$(abspath $^))

# Rule for magnum library
$(NEXUS_STATIC_LIB): $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS) ${NEXUS_EXTRALIBS} ${MAGNUM_STATIC_LIBS}
	@echo "[Archive... $(notdir $@)]"
	-$(Q_)cd $(NEXUS_OBJ_DIR);$(AR) rcs $(abspath $@) $(patsubst $(abspath $(NEXUS_OBJ_DIR))/%,%,$(abspath $^))
endif

# Rule for shared nexus library
ifdef NEXUS_SHARED_LIB
$(NEXUS_SHARED_LIB): $(NEXUS_OBJECTS) $(MAGNUM_OBJECTS) $(NEXUS_EXTRALIBS) ${MAGNUM_STATIC_LIBS}
	@echo "[Link...... $(notdir $@)]"
	${NEXUS_BUILD_SO} -o $(abspath $@) $(NEXUS_LDFLAGS)
endif

nexus_clean:
	-$(Q_)$(RM) -r ${NEXUS_SHARED_LIB} $(NEXUS_STATIC_LIB) $(MAGNUM_LIB) $(NEXUS_OBJ_DIR) $(NEXUS_BUILD_DIR)/module_vars_*.inc $(NEXUS_BUILD_DIR)/module_rules_*.inc
ifneq ($(NEXUS_MODE),client)
	-$(Q_)$(RM) -r $(NEXUS_SYNCTHUNK_DIR)
endif
ifdef NEXUS_SHARED_LIB
	-$(Q_)$(RM) ${NEXUS_BIN_DIR}/$(notdir ${NEXUS_SHARED_LIB})
endif
ifdef NEXUS_STATIC_LIB
	-$(Q_)$(RM) ${NEXUS_BIN_DIR}/$(notdir ${NEXUS_STATIC_LIB})
endif

# Include dependencies
NEXUS_DEP_FILES := $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
ifneq ($(NEXUS_DEP_FILES),)
-include $(wildcard $(NEXUS_OBJ_DIR)/*/*.d)
endif

