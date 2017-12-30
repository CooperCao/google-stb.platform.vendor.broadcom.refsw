############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
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
# Module Description:
#
###########################################################################

ifneq ($(VERBOSE),)
Q_=
endif

CDEP_FLAG = -MMD

ifeq ($(ODIR),)
ODIR = .
endif


# We need ODIR_FLAG to serialize between directory creation and building object files into this directory
# ODIR itself can't be used because it would change with every object file written into the directory
ifeq ($(ODIR),.)
ODIR_FLAG=
else
ODIR_FLAG = ${ODIR}/flag
endif

# COPT_FLAGS is being set to allow objects in *_OPT_OBJS to be built with specialized flags.  As of the time 
# of implementation, it is restricted to the p3d, opengles, grc, and vdc objects.  Not all areas of settopapi 
# can safely be optimized to this level.

# Implicit C Compile Rule
ifeq ($(SYSTEM),linux)
ifeq ($(BCHP_CHIP),7038)
$(ODIR)/%.o : COPT_FLAGS=$(if $(findstring $@,$(ST_OPT_OBJS)),-O3 -mips32,)
endif
endif
$(ODIR)/%.o : %.c ${ODIR_FLAG}
		@echo [Compile... $(notdir $<)]
		${Q_}$(CC) ${CDEP_FLAG} ${CFLAGS_STATIC} $(CFLAGS) $(COPT_FLAGS) $(if $(findstring $@,$(ST_NOPROF_OBJS)),,${CFLAGS_BPROFILE}) -c $< -o $@

ifeq ($(SYSTEM),linux)
ifeq ($(BCHP_CHIP),7038)
$(ODIR)/%.so : COPT_FLAGS=$(if $(findstring $@,$(SH_OPT_OBJS)),-O3 -mips32,)
endif
endif
$(ODIR)/%.so : %.c ${ODIR_FLAG}
		@echo [Compile... $(notdir $<)]
		${Q_}$(CC) ${CDEP_FLAG} ${CFLAGS_SHARED} $(CFLAGS) $(COPT_FLAGS) $(if $(findstring $@,$(SH_NOPROF_OBJS)),,${CFLAGS_BPROFILE}) -c $< -o $@

# Implicit C++ Compile Rule
$(ODIR)/%.o : %.cpp ${ODIR_FLAG}
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) ${CDEP_FLAG} ${CPPFLAGS_STATIC} $(CPPFLAGS) -c $< -o $@

# Implicit C++ Compile Rule
$(ODIR)/%.so : %.cpp ${ODIR_FLAG}
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) ${CDEP_FLAG} ${CPPFLAGS_SHARED} $(CPPFLAGS) -c $< -o $@

# Determine whether or not to print rm operations
ifneq ($(VERBOSE),)
RMOPTS := -v
else
RMOPTS :=
endif

ifdef APP
APP_OBJS = $(SRCS:%.c=${ODIR}/%.o)
APP_OPT_OBJS = $(OPTIMIZE_SRCS:%.c=${ODIR}/%.o)

ifeq ($(SYSTEM),linuxkernel)
ifeq ($(LINUX_VER_GE_2_6),y)
#OBJS += umdrv.mod.o
CFLAGS += -DKBUILD_MODNAME=${APP}
MOD_EXT = ko
else
MOD_EXT = o
endif
APP_IMAGE = ${ODIR}/${APP}.${MOD_EXT}
else
APP_IMAGE = ${ODIR}/${APP}
endif

application: $(OTHER_MAKES) ${ODIR} ${APP_IMAGE}

ifneq ($(SYM_SRC),)
SYM_OBJ = $(SYM_SRC:%.c=$(ODIR)/%.o)
SYM_INC = $(SYM_SRC:%.c=$(ODIR)/%.inc) 

OBJS += ${SYM_OBJ}

ifeq ($(SYSTEM),linuxkernel)
${SYM_OBJ} : ${SYM_SRC} $(filter-out ${SYM_OBJ},${OBJS}) ${APP_OBJS}
	@echo [Symbols... $(notdir $<)]
	${Q_}# compile  empty sym-table and link with it 
	${Q_}${RM) ${SYM_INC}
	${Q_}echo '/* */' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(LD) ${LDFLAGS} $(OBJS) ${APP_OBJS}  ${LIBS} -o ${APP_IMAGE}.sym 
	${Q_}# compile with real sym-table but possibly wrong offsets
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${APP_IMAGE}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(LD) ${LDFLAGS} $(OBJS) ${APP_OBJS}  ${LIBS} -o ${APP_IMAGE}.sym 
	${Q_}# build real symtable and compile with it
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${APP_IMAGE}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}${MV} ${SYM_INC} $(SYM_INC:%.inc=%.sym)
else
${SYM_OBJ} : ${SYM_SRC} $(filter-out ${SYM_OBJ},${OBJS}) ${APP_OBJS}
	@echo [Symbols... $(notdir $<)]
	${Q_}# compile  empty sym-table and link with it 
	${Q_}${RM) ${SYM_INC}
	${Q_}echo '/* */' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(CC) $(OBJS) ${APP_OBJS} $(LDFLAGS) ${LIBS} -o ${APP_IMAGE}.sym
	${Q_}# compile with real sym-table but possibly wrong offsets
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${APP_IMAGE}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(CC) $(OBJS) ${APP_OBJS} $(LDFLAGS) ${LIBS} -o ${APP_IMAGE}.sym
	${Q_}# build real symtable and compile with it
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${APP_IMAGE}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_STATIC} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}${MV} ${SYM_INC} $(SYM_INC:%.inc=%.sym)
endif

endif

# liveMedia library is compiled with C++, must link with C++
ifeq ($(LIVEMEDIA_SUPPORT),y)
    B_LINKER := $(CXX)
else
    B_LINKER := $(CC)
endif

$(APP_IMAGE): ${LIBS} $(OBJS) ${APP_OBJS}
	@echo [Linking... $(notdir $@)]
ifeq ($(SYSTEM),linuxkernel)
	${Q_}$(LD) ${LDFLAGS} $(OBJS) ${APP_OBJS} ${LIBS} -o $@ 
else
	${Q_}$(B_LINKER) $(OBJS) ${APP_OBJS} $(LDFLAGS) ${LIBS} -o $@
endif

target-clean:
	${Q_}${RM} ${RMOPTS} ${APP_IMAGE}

endif

ifdef LIB
SH_LIB = ${ODIR}/lib$(LIB).so
SH_MAP = ${ODIR}/lib$(LIB).map
ST_LIB = ${ODIR}/lib$(LIB).a

.PHONY: shared static
shared: $(OTHER_MAKES) ${ODIR} ${SH_LIB} 
static: $(OTHER_MAKES) ${ODIR} ${ST_LIB} 

SH_OBJS = $(SRCS:%.c=${ODIR}/%.so)
ST_OBJS = $(SRCS:%.c=${ODIR}/%.o)

ifneq ($(SYM_SRC),)
SYM_OBJ = $(SYM_SRC:%.c=$(ODIR)/%.so)
SYM_INC = $(SYM_SRC:%.c=$(ODIR)/%.inc) 

SH_OBJS += ${SYM_OBJ}

${SYM_OBJ} : ${SYM_SRC} $(filter-out ${SYM_OBJ},${SH_OBJS})
	@echo [Symbols... $(notdir $<)]
	${Q_}# compile  empty sym-table and link with it 
	${Q_}${RM) ${SYM_INC}
	${Q_}echo '/* */' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_SHARED} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(CC) ${LDFLAGS} -shared -o ${SH_LIB}.sym -Wl,-soname,lib${LIB}.so ${SH_OBJS} ${LIBS} ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
	${Q_}# compile with real sym-table but possibly wrong offsets
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${SH_LIB}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_SHARED} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}$(CC) ${LDFLAGS} -shared  -o ${SH_LIB}.sym -Wl,-soname,lib${LIB}.so ${SH_OBJS} ${LIBS} ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
	${Q_}# build real symtable and compile with it
	${Q_}${RM) ${SYM_INC}
	${Q_}${NM} -f bsd -n --defined-only ${SH_LIB}.sym|${AWK} '/.* [Tt] .*/ {printf "B_SYM(0x%su,%s)\n",$$1,$$3}' >${SYM_INC}
	${Q_}$(CC) ${CFLAGS_SHARED} -I${ODIR} $(CFLAGS) $(COPT_FLAGS) -c $< -o $@
	${Q_}${MV} ${SYM_INC} $(SYM_INC:%.inc=%.sym)

endif

SH_OPT_OBJS = $(OPTIMIZE_SRCS:%.c=${ODIR}/%.so)
ST_OPT_OBJS = $(OPTIMIZE_SRCS:%.c=${ODIR}/%.o)
SH_NOPROF_OBJS = $(NOPROFILE_SRCS:%.c=${ODIR}/%.so)
ST_NOPROF_OBJS = $(NOPROFILE_SRCS:%.c=${ODIR}/%.o)


${SH_LIB}: ${SH_OBJS} $(LIBS)
	@echo [Linking... $(notdir $@)]
	${Q_}$(CC) ${LDFLAGS} -shared -Wl,-Map,${SH_MAP} -Wl,--cref -o $@ -Wl,-soname,lib${LIB}.so ${SH_OBJS} ${LIBS} ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
ifneq (${CMD_BUILD_DONE},)
	${Q_}${CMD_BUILD_DONE} ${SH_LIB} ${SH_MAP}
endif

${ST_LIB}: ${ST_OBJS} 
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@  $(ST_OBJS) ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
	${Q_}$(RANLIB) $@

target-clean:
	${Q_}${RM} ${RMOPTS} ${SH_LIB} ${ST_LIB}

endif

# Dependency file checking (created with gcc -M command)
-include $(ODIR)/*.d

# Clean Rules
# Use OTHER_CLEANS to extend the clean rule for app-specific stuff.
# It's ok to leave it undefined.

.PHONY: clean target-clean veryclean $(OTHER_CLEANS)
clean: target-clean $(OTHER_CLEANS)
ifdef OBJS
	${Q_}$(RM) ${RMOPTS} $(OBJS)
	${Q_}$(RM) ${RMOPTS} $(OBJS:%.o=%.d)
endif
ifdef APP_OBJS
	${Q_}$(RM) ${RMOPTS} $(APP_OBJS)
	${Q_}$(RM) ${RMOPTS} $(APP_OBJS:%.o=%.d)
endif
ifdef SH_OBJS
	${Q_}$(RM) ${RMOPTS} $(SH_OBJS)
	${Q_}$(RM) ${RMOPTS} $(SH_OBJS:%.so=%.d)
endif
ifdef ST_OBJS
	${Q_}$(RM) ${RMOPTS} $(ST_OBJS)
	${Q_}$(RM) ${RMOPTS} $(ST_OBJS:%.o=%.d)
endif
ifneq ($(ODIR_FLAG),)
	${Q_}$(RM) ${ODIR_FLAG}
endif

veryclean: clean
ifdef ODIR
	-@$(RM) ${RMOPTS} $(ODIR)/*
	-@$(RM) -r ${RMOPTS} $(ODIR)
endif

ifneq ($(ODIR),.)
${ODIR_FLAG}: ${ODIR}
endif

$(ODIR):
	${Q_}$(MKDIR) "$(ODIR)"
ifneq ($(ODIR_FLAG),)
	${Q_}${TOUCH} "${ODIR_FLAG}"
endif
