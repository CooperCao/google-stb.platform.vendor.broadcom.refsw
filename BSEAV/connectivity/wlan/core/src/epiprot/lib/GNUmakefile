# GNUmakefile : build all brands and target types (debug or release)
# of the control panel applet
#
# $Id: GNUmakefile,v 1.3 2006-06-18 10:43:25 $
#

SUBSYSTEM=windows
REQUIRE_MSDEV=1
REQUIRE_WDM2600=1

SRCBASE = ../..

include $(SRCBASE)/makefiles/env.mk

# env defined NEWPATH...
empty:=
space:= $(empty) $(empty)
export PATH:=$(subst $(space),:,$(strip $(NEWPATH))):$(PATH)

CC  = cl
RC  = rc
LD  = link
LIBCMD = lib

INCLUDES   = $(SRCBASE)/bcm/Include;$(SRCBASE)/include;$(SRCBASE)/common/include;$(SRCBASE)/../components/shared;$$(MSSDK)/include;$(MSDEV)/vc98/include;$(WDM2600DDK)/inc/wxp;

C_DEFINES  = -D "WIN32" -D BUILD_EPICTRL_DLL -D "WIN32_LEAN_AND_MEAN"

C_DEFINES += -D_X86_ -D_MMX_ -DBCMCCX
MSINCLUDE=.;$(strip $(INCLUDES))
CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

export LIB=$(MSDEV.LIBPATH);$(WDM2600DDK)/lib/wxp/i386

SOURCES    = reg2k.c regnt4.c ir.c irhelper.c driveloader.c

ifeq ($(WL_CMD), 1)
SOURCES    += ir_wmi.c
C_DEFINES  += -DWL_CMD
endif

SOURCES.OBJ	:= $(filter %.c,$(SOURCES))
SOURCES.OBJ	:= $(SOURCES.OBJ:.c=.obj)
SOURCES.RES	:= $(filter %.rc,$(SOURCES))
SOURCES.RES	:= $(SOURCES.RES:.rc=.res)

# calculate some dynamic variables that are useful in build rules.
#
DOS_DEPS= $(shell echo $(filter-out %.tlb %.TLB %.h %.H %.def %.DEF,$^) | sed 's%//\(.\)/%\1:/%g')

all: free/bcmctrl.lib checked/bcmctrl.lib

debug checked : checked/bcmctrl.lib

free release : free/bcmctrl.lib


# The flags supplied to the linker on debug build are different from
# those supplied for optimized (aka 'free', 'release', 'retail')
# builds. These next couple of target-specific variable definitions
# define the linker flags we want to use for each flavor of build that we
# support.
#
# avoid KB148652 - Errors When Link C Run-Time Libraries Are Linked Before MFC Libraries
#


checked/bcmctrl.lib : LIBFLAGS = -nologo -MACHINE:i386 \
				    $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p") 


free/bcmctrl.lib : LIBFLAGS = -nologo -MACHINE:i386 \
				   $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p") 
				     

# Define the command used to link all the objet files together into
# our final target.
#
define LINK_FILES
	@echo "Building LIB - $@"
	@echo Creating temporary file "$${TEMP}/LNK$${PPID}.tmp" with contents
	@echo $(LIBS) /NOD:LIBC.LIB $(LIBFLAGS) -OUT:$@ >$${TEMP}/LNK$${PPID}.tmp
	@$(foreach p,$(filter %.obj %.res,$(DOS_DEPS)),echo $(p) >>$${TEMP}/LNK$${PPID}.tmp; )
	@echo "["
	@cat $${TEMP}/LNK$${PPID}.tmp
	@echo "]"
	$(LIBCMD) @`cygpath -w $${TEMP}`/LNK$${PPID}.tmp
	@rm $${TEMP}/LNK$${PPID}.tmp
endef

free/bcmctrl.lib :: C_DEFINES += -D "NDEBUG" 
free/bcmctrl.lib :: free/NUL
free/bcmctrl.lib :: $(addprefix free/,$(SOURCES.OBJ)) \
				    $(addprefix free/,$(SOURCES.RES)) 
	$(LINK_FILES)

checked/bcmctrl.lib :: C_DEFINES += -D "_DEBUG" -D "BCMDBG"
checked/bcmctrl.lib :: checked/NUL
checked/bcmctrl.lib :: $(addprefix checked/,$(SOURCES.OBJ)) \
					   $(addprefix checked/,$(SOURCES.RES)) 
	$(LINK_FILES)


free/%.res : %.rc
	@echo "Compiling - $(notdir $(filter %.rc,$<))"
	export INCLUDE="$(dir $@);$(MSINCLUDE)"; \
	$(RC) -r $(RCFLAGS) -fo$@ $(filter %.rc,$<)

checked/%.res : %.rc
	@echo "Compiling - $(notdir $(filter %.rc,$<))"
	export INCLUDE="$(dir $@);$(MSINCLUDE)"; \
	$(RC) -r $(RCFLAGS) -fo$@ $(filter %.rc,$<)

free/%.obj :  %.c
	@echo "Compiling - $(notdir $<)"
	$(CC) -c -nologo -W3 -GX -Od $(C_DEFINES) $(CPPFLAGS)  -Fo$@ $<

checked/%.obj :  %.c
	@echo "Compiling - $(notdir $<)"
	$(CC) -c -nologo -W3 -GX -Z7 -Od -GZ -Fd$(@D)/ $(C_DEFINES) $(CPPFLAGS) -Fo$@ $<

#
# Utility target to create target directories on demand.
#
%/NUL :
	mkdir -p $(@D)

clean: 
	rm -f checked/*.obj free/*.obj
	rm -f checked/*.res free/*.res

clobber:  
	rm -rf checked free

.PHONY: all FORCE clean strings resources debug release free checked
