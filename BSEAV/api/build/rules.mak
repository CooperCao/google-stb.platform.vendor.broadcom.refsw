############################################################################
#     Copyright (c) 2003-2010, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
ifneq ($(SYSTEM),vxworks)
ifeq ($(ODIR),.)
ODIR_FLAG=
else
ODIR_FLAG = ${ODIR}/flag
endif
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
ifeq ($(SYSTEM),vxworks)
ifneq ($(vxWorksVersion),6)
		-${Q_}$(MV) $(patsubst %.c,%.d,$(notdir $< )) ${ODIR}
endif
endif

ifeq ($(SYSTEM),linux)
ifeq ($(BCHP_CHIP),7038)
$(ODIR)/%.so : COPT_FLAGS=$(if $(findstring $@,$(SH_OPT_OBJS)),-O3 -mips32,)
endif
endif
$(ODIR)/%.so : %.c ${ODIR_FLAG}
		@echo [Compile... $(notdir $<)]
		${Q_}$(CC) ${CDEP_FLAG} ${CFLAGS_SHARED} $(CFLAGS) $(COPT_FLAGS) $(if $(findstring $@,$(SH_NOPROF_OBJS)),,${CFLAGS_BPROFILE}) -c $< -o $@
ifeq ($(SYSTEM),vxworks)
ifneq ($(vxWorksVersion),6)
		-${Q_}$(MV) $(patsubst %.c,%.d,$(notdir $< )) ${ODIR}
endif
endif


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

ifeq ($(SYSTEM),vxworks)
#ifneq ($(vxWorksVersion),6)
# For tests and utils we need a rule that doesn't include the ODIR (the objects build in the currect working directory).
# This is mainly for VxWorks 5.5 compiler (defaults to putting the .o and the .d in the current working directory.
# Implicit C Compile Rule
%.o : %.c
		@echo === Compiling $<
		${Q_}$(CC) ${CDEP_FLAG} ${CFLAGS_STATIC} $(CFLAGS) -c $< -o $@
#endif
endif

ifdef APP
APP_OBJS = $(SRCS:%.c=${ODIR}/%.o)
APP_OPT_OBJS = $(OPTIMIZE_SRCS:%.c=${ODIR}/%.o)

ifeq ($(SYSTEM),vxworks)
APP_IMAGE = ${ODIR}/${APP}.out
else
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
ifeq (${SYSTEM},vxworks)
SH_LIB = ${ODIR}/lib$(LIB).out
else
SH_LIB = ${ODIR}/lib$(LIB).so
endif
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
ifeq (${SYSTEM},vxworks)
	${Q_}$(CC) ${LDFLAGS} -nostdlib -r -Wl,-Map,${SH_MAP} -Wl,-EB -Wl,-X -o $@ -Wl,-soname,lib${LIB}.so ${SH_OBJS} ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
else
	${Q_}$(CC) ${LDFLAGS} -shared -Wl,-Map,${SH_MAP} -Wl,--cref -o $@ -Wl,-soname,lib${LIB}.so ${SH_OBJS} ${LIBS} ${CRYPTO_LDFLAGS} ${DIVX_DRM_LIBS}
ifneq (${CMD_BUILD_DONE},)
	${Q_}${CMD_BUILD_DONE} ${SH_LIB} ${SH_MAP}
endif
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

veryclean: clean
ifdef ODIR
ifeq ($(vxWorksVersion),6)
# VxWorks 6 command shell can't handle long commands (sic) -- need to do this in steps.
# By doing these deletes first, the wildcards below will have less files to delete and thus shorter command line.
	-@$(RM) ${RMOPTS} $(ODIR)/*.o
	-@$(RM) ${RMOPTS} $(ODIR)/*.d
	-@$(RM) ${RMOPTS} $(ODIR)/*.so
endif
	-@$(RM) ${RMOPTS} $(ODIR)/*
	-@$(RM) -r ${RMOPTS} $(ODIR)
endif

ifneq ($(SYSTEM),vxworks)
ifneq ($(ODIR),.)
${ODIR_FLAG}: ${ODIR}
endif
endif

$(ODIR):
	${Q_}$(MKDIR) "$(ODIR)"
ifneq ($(SYSTEM),vxworks)
	${Q_}${TOUCH} "${ODIR_FLAG}"
endif

