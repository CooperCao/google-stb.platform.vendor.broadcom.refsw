# Makefile
# $Id: Makefile,v 1.2 2007-07-31 05:36:03 $:
#

TARGETTYPE=DLL
REQUIRE_MSDEV=1
REQUIRE_WDM2600=1

MFG_DLL = nok_mfg.dll
MFG_DBG_DLL = nok_mfg_dbg.dll

SRC_TOP = ../../../../
MFG_TOP = $(SRC_TOP)/tools/mfgc
CLI_DIR = $(MFG_TOP)/nokia/cli/

REL_MFG_DIR = free
DEB_MFG_DIR = checked

include $(SRC_TOP)/makefiles/env.mk

CFLAGS +=

vpath %.c $(MFG_TOP)/nokia/src $(MFG_TOP)/src/util

# env defined NEWPATH...
empty:=
space:= $(empty) $(empty)
export PATH:=$(subst $(space),:,$(strip $(NEWPATH))):$(PATH)

DEPENDS_CC = gcc
CC  = cl
LD  = link

C_DEFINES  = -D "WIN32" -D "_WINDOWS" -D "_MBCS" -D "_USRDLL" -D"BUILD_DLL"
$(REL_MFG_DIR)/%.obj 	: C_DEFINES += -D NDEBUG
$(DEB_MFG_DIR)/%.obj   	: C_DEFINES += -D BCMINTERNAL -D BCMDBG

INCLUDES= $(SRC_TOP)/include $(MFG_TOP)/nokia/include
MSINCLUDE=.;$(strip $(INCLUDES));$(MSDEV.INCLUDE)
CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))
DEPENDS_CPPFLAGS = $(patsubst %,-I%,$(INCLUDES))

LIBPATHS=

LIBS = advapi32.lib
LIBS += iphlpapi.lib
LIBS += $(WDM2600DDK)/lib/w2k/i386/cfgmgr32.lib 
LIBS += ws2_32.lib
LIBS += user32.lib

$(REL_MFG_DIR)/%	: LIBS += $(SRC_TOP)/tools/epictrl/free/bcmctrl.lib
$(DEB_MFG_DIR)/%	: LIBS += $(SRC_TOP)/tools/epictrl/checked/bcmctrl.lib

SOURCES = nokia.c

SOURCES.OBJ	:= $(filter %.c,$(SOURCES))
SOURCES.OBJ	:= $(SOURCES.OBJ:%.c=%.obj)


all: mfgdll

#For use with this nok_cli in src/mfgc
mfgdll: epilibs $(DEB_MFG_DIR)/$(MFG_DLL) $(REL_MFG_DIR)/$(MFG_DLL)

epilibs:
	$(MAKE) -C $(SRC_TOP)/tools/epictrl

$(DEB_MFG_DIR)/$(MFG_DLL) : $(DEB_MFG_DIR)/NUL \
	$(addprefix $(DEB_MFG_DIR)/,$(SOURCES.OBJ)) \
	$(SRC_TOP)/tools/epictrl/checked/bcmctrl.lib
	@$(LINK_FILES)
	cp -u $(DEB_MFG_DIR)/$(MFG_DLL) $(CLI_DIR)/$(MFG_DBG_DLL)

# Make mfg dll for use with nok_cli in src/tools/mfg
$(REL_MFG_DIR)/$(MFG_DLL) : $(REL_MFG_DIR)/NUL \
	$(addprefix $(REL_MFG_DIR)/,$(SOURCES.OBJ)) \
	$(SRC_TOP)/tools/epictrl/free/bcmctrl.lib
	@$(LINK_FILES)
	cp -u $(REL_MFG_DIR)/$(MFG_DLL) $(CLI_DIR)/$(MFG_DLL)

Debug/% :: LINKFLAGS = /debug

# Define the command used to link all the objet files together into
# our final target.
#
define LINK_FILES
	echo "Linking DLL - $@"
	echo $(LIBS) $(LINKFLAGS)  /nologo /dll /pdb:"${@:%.dll=%.pdb}" /machine:I386 /NOD:libc.lib /NOD:libcmt.lib -OUT:$@ $(LIBPATHS) >$${TEMP}/LNK$${PPID}.tmp
	$(foreach p,$(filter %.obj %.res,$^),echo $(p) >>$${TEMP}/LNK$${PPID}.tmp; )
	echo "["
	cat $${TEMP}/LNK$${PPID}.tmp
	echo "]"
	LIB="$(MSSDK)/lib;$(MSDEV)/VC98/LIB;$(MSDEV)/VC98/MFC/LIB" \
	$(LD) @`cygpath -w $${TEMP}`/LNK$${PPID}.tmp
	rm $${TEMP}/LNK$${PPID}.tmp
endef

$(REL_MFG_DIR)/%.obj: %.c
	@echo "Compiling - $(notdir $<)"
	$(CC) /nologo /MD /W3 /EHsc /O2 /Fo"$(@D)/" /Fd"$(@D)/" /FD /c $(C_DEFINES) $(CPPFLAGS)   $<

$(DEB_MFG_DIR)/%.obj :  %.c
	@echo "Compiling - $(notdir $<)"
	$(CC) /nologo /MDd /W3 /EHsc /ZI /Od /Fo"$(@D)/" /Fd"$(@D)/" /FD /GZ  /c  $(C_DEFINES) $(CPPFLAGS) $<

$(REL_MFG_DIR)/NUL $(DEB_MFG_DIR)/NUL : % :
	echo $(@D)
	mkdir -p $(@D)


clean:
	rm -f *.obj
	rm -rf $(REL_MFG_DIR)
	rm -rf $(DEB_MFG_DIR)
	$(MAKE) -C $(SRC_TOP)/tools/epictrl clean

clobber: clean

.PHONY: all clean clobber install # debug free

ifeq (,$(filter clean clobber install,$(MAKECMDGOALS)))
-include $(SOURCES.OBJ:%.obj=.%.d)
endif
