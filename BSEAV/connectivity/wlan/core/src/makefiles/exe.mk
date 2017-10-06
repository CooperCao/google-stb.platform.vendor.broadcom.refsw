
############################################################################
#
#   Flags for building EXE targets
#
############################################################################

TARGET=$(TARGETNAME).exe

ifeq ($(strip $(SUBSYSTEM)), )
	SUBSYSTEM=console
endif

ifndef VSVERSION
VSVERSION=VC98
endif

ifndef MACHINE
MACHINE=IX86
endif

ifeq ($(VSVERSION),VS2003)
ifeq ($(MACHINE),AMD64)
    REQUIRE_VS2003AMD64 = 1
else
    REQUIRE_VS2003 = 1
endif
    COMMON_FLAGS = -nologo -W3 -EHsc $(C_DEFINES) -Fd$@.pdb
    CDBGFLAGS = $(COMMON_FLAGS) -MLd -Od -GS -RTCsuc -Zi -DDEBUG 
    COPTFLAGS = $(COMMON_FLAGS) -ML -O1
    MSINCLUDE=.;$(VS2003.INCLUDE);$(strip $(INCLUDES));
    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    LDFLAGS = -nologo -machine:$(MACHINE) -subsystem:$(SUBSYSTEM)
    LDFLAGS += $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")
    LDFLAGS_OPT=$(LDFLAGS)
    LDFLAGS_DBG=$(LDFLAGS) -DEBUG -PDB:$(@D)/

    LIBS = $(TARGETLIBS)

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg" -r $(C_DEFINES)
else  # VSVERSION
    REQUIRE_MSDEV = 1

    LDFLAGS += -nologo -MACHINE:i386 \
	     $(LIBPATHS) \
	     -subsystem:$(SUBSYSTEM) -MACHINE:i386  \
	     -MAP:$(@:.exe=.map) $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")

    LDFLAGS_OPT += $(LDFLAGS)
    LDFLAGS_DBG += $(LDFLAGS) -DEBUG -PDB:$(@D)/

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg"  -r \
		$(C_DEFINES)

    LIBS = $(TARGETLIBS)

    ifneq ($(strip $(USE_MFC)),)
	LIBVPATH += $(MSDEV.UNC)/vc98/mfc/lib
	C_DEFINES +=  -D_MT -D_DLL
	COPTFLAGS  += -MD 
	CDBGFLAGS  += -MDd
    endif

    ifeq ($(REQUIRE_SDK60), 1)
	COPTFLAGS  += -nologo -W3 -EHsc -O2 $(C_DEFINES)
	CDBGFLAGS  += -nologo -W3 -EHsc -Z7 -Od -RTC1  -DDEBUG $(C_DEFINES)
    	C_DEFINES  += -D_CRT_SECURE_NO_DEPRECATE
    else
	COPTFLAGS  += -nologo -W3 -GX -O2 $(C_DEFINES)
	CDBGFLAGS  += -nologo -W3 -GX -Z7 -Od -GZ  -DDEBUG $(C_DEFINES)
    endif

    MSINCLUDE=.;$(strip $(INCLUDES));$(MSDEV.INCLUDE)

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    NTICE_OPT=
ifdef	NTICE
ifndef NTICE_DBG
    NTICE_DBG= @echo "Building SoftICE symbols for $@"; \
	"$(NTICE)/nmsym.exe" $(SI_FLAGS) \
	-SOURCE:. $@
endif
else
    NTICE_DBG=
endif
endif # VSVERSION



############################################################################
#
#   Rules for building EXE targets 
#
############################################################################

$(OUTDIR_$(TTYPE))/$(TARGET) : $(DEPENDENCIES) $(DEFFILE) 
	@echo "Linking Executable - $@ for $(MACHINE)"
	$(LD) $(LDFLAGS_$(TTYPE)) -OUT:$@ $(DOS_DEPS) $(LIBS) 
	$(NTICE_$(TTYPE))
