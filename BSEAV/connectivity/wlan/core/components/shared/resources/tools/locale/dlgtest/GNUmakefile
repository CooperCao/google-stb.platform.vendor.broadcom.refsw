# GNUmakefile : Makefile for dialog text truncation checker
$
#
# $Id: GNUmakefile,v 1.20 2008-10-14 22:49:40 $
#

TARGETNAME=dlgtest
TARGETTYPE=EXE
USE_MFC=1
REQUIRE_MSDEV := 1
SRCBASE       := ../../..
## If dlgtest step fails, do not exit the build.
EXIT_ON_ERROR := 1
OEM_LIST      ?= bcm dell hp
NULL          := /dev/null

DLGTEST  := $(shell cygpath -a $(SRCBASE)/tools/locale/dlgtest/dlgtest.exe)
## Default list of dialog apps to be tested
# CONFIG_NAME and OEM_NAME are replaced when rundlgtest target is invoked from release makefile

# xp/2k built objects to be dialog tested.
DLG_APPS  = $(SRCBASE)/components/BCMLogon/CONFIG_NAME/OEM_NAME/BCMLogon.dll
DLG_APPS += $(SRCBASE)/wl/cpl/wltray/CONFIG_NAME/OEM_NAME/wltray.exe
DLG_APPS += $(SRCBASE)/wl/cpl/neptune/cpl/CONFIG_NAME/OEM_NAME/bcmwlcpl.cpl

# vista built objects to be dialog tested.
DLG_APPS += $(SRCBASE)/wl/cpl/components/sup/eaphost/ttls/CONFIG_NAME/OEM_NAME/bcmttls.dll
DLG_APPS += $(SRCBASE)/wl/cpl/vista/ClientApp/CONFIG_NAME/OEM_NAME/wltray.exe
DLG_APPS += $(SRCBASE)/wl/cpl/vista/ControlPanel/CONFIG_NAME/OEM_NAME/bcmwlcpl.cpl

ifndef CONFIG_NAME
  CONFIG_NAME = Debug
endif

include $(SRCBASE)/makefiles/env.mk

# env defined NEWPATH...
empty:=
space:= $(empty) $(empty)
export PATH:=$(subst $(space),:,$(strip $(NEWPATH))):$(PATH)

CC  = cl
RC  = rc
LD  = link
LIBCMD = lib

OBJDIRS = checked free

UNICONV		:= 	$(SRCBASE)/tools/locale/free/uniconv.exe

.PHONY: all FORCE clean dlgtest

# avoid KB148652 - Errors When Link C Run-Time Libraries Are Linked Before MFC Libraries
#

LIBS =

LIBPATH =

#
# This is our default set of pre-processor flags.
# We modify these flags depending on whether we are compiling for a
# debug build or an optimized build.
#

C_DEFINES = /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /D "__UNICODE" /D "HASHELP" 

free/%   : C_DEFINES += /D "NDEBUG"
checked/%  : C_DEFINES += /D "DEBUG" /D "_DEBUG" /D "DBG=1"

#
# This is our default set of compiler flags.
# We modify these flags depending on whether we are compiling for a
# debug build or an optimized build.
#
CFLAGS = /nologo

#
# Optimized and debug builds get different compiler flags.
#
free/%    : CFLAGS += /MD /W3 /Zi /GX /O2
checked/%      : CFLAGS += /MD /W3 /Zi /GX /O2

INCLUDES=

INCLUDES += /I "./"

INCLUDES += /I "$(MSSDK)/include"
INCLUDES += /I "$(MSDEV)/VC98/INCLUDE"
INCLUDES += /I "$(MSDEV)/VC98/MFC/INCLUDE"
INCLUDES += /I "$(MSDEV)/VC98/ATL/INCLUDE"

CPPFLAGS = $(INCLUDES)

SOURCES = DlgTest.cpp 
SOURCES.OBJ	= $(patsubst %.cpp,%.obj,$(filter %.cpp,$(SOURCES)))

all:: checked/dlgtest.exe free/dlgtest.exe

checked/dlgtest.exe : checked/NUL \
					$(addprefix checked/,$(SOURCES.OBJ))
	@echo "Linking EXE - $@"
	@echo "$(LIBPATHS)" >>${TEMP}/LNK$${PPID}.tmp
	@echo "$(LIBS) -OUT:$@ " >>${TEMP}/LNK$${PPID}.tmp
	@echo "/subsystem:windows -MACHINE:I386" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/NOD:libc.lib /NOD:libcmt.lib /NOD:msvcrt.lib" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/IGNORE:4089  -DEBUG -PDB:$(@D)/" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/ENTRY:wWinMainCRTStartup" >>${TEMP}/LNK$${PPID}.tmp
	@$(foreach p,$(filter %.obj %.res,$^),echo $(p) >>${TEMP}/LNK$${PPID}.tmp; )
	@echo "["
	@cat ${TEMP}/LNK$${PPID}.tmp
	@echo "]"
	@LIB="$(MSSDK)/lib;$(MSDEV)/VC98/LIB;$(MSDEV)/VC98/MFC/LIB" \
	$(LD) @`cygpath -w $${TEMP}`/LNK$${PPID}.tmp
	@rm ${TEMP}/LNK$${PPID}.tmp
	@$(MAKE_SYMBOLS)

free/dlgtest.exe : free/NUL \
						   $(addprefix free/,$(SOURCES.OBJ))
	@echo "Linking EXE - $@"
	@echo "$(LIBPATHS)" >>${TEMP}/LNK$${PPID}.tmp
	@echo "$(LIBS) -OUT:$@ " >>${TEMP}/LNK$${PPID}.tmp
	@echo "/subsystem:windows" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/incremental:no" >>${TEMP}/LNK$${PPID}.tmp
	@echo "-MACHINE:I386" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/NOD:libc.lib /NOD:libcmt.lib" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/IGNORE:4089  -DEBUG -PDB:$(@D)/" >>${TEMP}/LNK$${PPID}.tmp
	@echo "/ENTRY:wWinMainCRTStartup" >>${TEMP}/LNK$${PPID}.tmp
	@$(foreach p,$(filter %.obj %.res,$^),echo $(p) >>${TEMP}/LNK$${PPID}.tmp; )
	@LIB="$(MSSDK)/lib;$(MSDEV)/VC98/LIB;$(MSDEV)/VC98/MFC/LIB" \
	$(LD) @`cygpath -w $${TEMP}`/LNK$${PPID}.tmp
	@rm ${TEMP}/LNK$${PPID}.tmp

#
# Utility target to create target directories on demand.
#
%/NUL :
	mkdir -p $(@D)

#
# Pattern rules for compiling .obj files from .c/.cpp sources.
#
define CC_COMMAND
	$(CC) $(CFLAGS) $(CPPFLAGS) $(C_DEFINES) -Fo"$(@D)/" -Fd"$(@D)/" -FD /c $<
endef

checked/%.obj : %.cpp
	$(CC_COMMAND)
free/%.obj : %.cpp
	$(CC_COMMAND)

# Inherit the brand/oem list from calling makefile

#pragma runlocal
rundlgtest:
	@echo "Running dialog test"; \
	dlgerror=0; dlgfails=""; \
	for dlgapp in $(foreach app,$(DLG_APPS),$(foreach oem,$(OEM_LIST),$(subst OEM_NAME,$(oem),$(subst CONFIG_NAME,$(CONFIG_NAME),$(app))))); do \
	    if [ -f "$${dlgapp}" ]; then \
		echo "Dialog Test -> $${dlgapp}"; \
		dlgappname=`basename $${dlgapp}`; \
		dlglogname="$${dlgappname}.dlgtest.log"; \
		dlglog="$${dlgapp}.dlgtest.log"; \
		pushd `dirname $${dlgapp}` > $(NULL); cp -p $(DLGTEST) .; \
		./dlgtest.exe -passfail $${dlgappname} > $${dlglogname}; \
		rm -f dlgtest.exe; popd > $(NULL); \
		if [ -s "$${dlglog}" ]; then \
			echo "APP NAME: $${dlgapp}" >> $(@)_errors.log; \
			cat $${dlglog} >> $(@)_errors.log; \
			echo -e "\n==================\n" >> $(@)_errors.log; \
			echo -e "\n$@ FAILED for $${dlgapp}\n"; \
			cat $${dlglog}; \
			rm -f $${dlglog}; \
			dlgerror=1; \
			dlgfails="$${dlgfails} $${dlgapp}"; \
		fi; \
	    fi; \
	done; \
	if [ "$${dlgerror}" == "1" -a "${EXIT_ON_ERROR}" == "0" ]; then \
		echo -e "\nSUMMARY: $@ FAILED for $${dlgfails}\n"; \
		echo -e "\nExiting\n"; \
		exit 1; \
	fi

clean: 
	rm -f $(foreach dir,$(OBJDIRS),$(dir)/*.obj)

clobber:  
	rm -rf $(foreach dir,$(OBJDIRS),$(dir))
