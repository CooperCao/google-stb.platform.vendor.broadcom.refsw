
############################################################################
#
#   Flags for building DLL targets (aka DYNLINK)
#
############################################################################

TARGET=$(TARGETNAME).dll

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
    CDBGFLAGS = $(COMMON_FLAGS) -MTd -LDd -Od -GS -RTCsuc -Zi -DDEBUG 
    COPTFLAGS = $(COMMON_FLAGS) -MT -LD -O1
    MSINCLUDE=.;$(VS2003.INCLUDE);$(strip $(INCLUDES));
    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    LDFLAGS = -nologo -DLL -machine:$(MACHINE) $(DEFFILE:%=-DEF:%)
    LDFLAGS += $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")
    LDFLAGS_OPT=$(LDFLAGS)
    LDFLAGS_DBG=$(LDFLAGS) -DEBUG -PDB:$(@D)/

    LIBS = $(TARGETLIBS)

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg" -r $(C_DEFINES)
else # VSVERSION
    REQUIRE_MSDEV = 1


    LDFLAGS_OPT += -nologo -DLL -MACHINE:i386 $(DEFFILE:%=-DEF:%) \
	      -MAP:$(@:.dll=.map) $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p")
    LDFLAGS_DBG += $(LDFLAGS_OPT) -DEBUG -PDB:$(@D)/

    NTICE_OPT=
ifdef	NTICE
ifndef NTICE_DBG
    NTICE_DBG= @echo "Building SoftICE symbols - $(@:%.dll=%.nms) for i386"; \
	"$(NTICE)/nmsym.exe" $(SI_FLAGS) \
	-SOURCE:. -SOURCE:$(MSDEV)/vc98/mfc/src \
	-SOURCE:$(MSDEV)/vc98/crt/src $@
endif
else
    NTICE_DBG=
endif

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg"  -r \
		$(C_DEFINES) $(CPPFLAGS)
    # RCFLAGS  =  $(CPPFLAGS)

    LIBS = $(TARGETLIBS)

    ifneq ($(strip $(USE_MFC)),)
	CRTFLAGS=-MD
    else
	CRTFLAGS=-MD -LD
    endif

    C_DEFINES += -D_X86_ -D_MMX_ -D_DLL=1 -D_MT

    ifeq ($(REQUIRE_SDK60), 1)
        COPTFLAGS  = -nologo -W3 -EHsc -O2 $(CRTFLAGS) $(C_DEFINES) $(C_DEFINES_$(TTYPE))
        CDBGFLAGS  = -nologo -W3 -EHsc -Z7 -Od $(CRTFLAGS)d -RTC1 -Fd$(@D)/ $(C_DEFINES) $(C_DEFINES_$(TTYPE))
    else
        COPTFLAGS  = -nologo -W3 -GX -O2 $(CRTFLAGS) $(C_DEFINES) $(C_DEFINES_$(TTYPE))
        CDBGFLAGS  = -nologo -W3 -GX -Z7 -Od $(CRTFLAGS)d -GZ -Fd$(@D)/ $(C_DEFINES) $(C_DEFINES_$(TTYPE))
    endif

    ifneq ($(strip $(SOURCES.MSG)),)
	MSGINCLUDE=$(OUTDIR_$(TTYPE));
    endif

    MSINCLUDE=.;$(strip $(INCLUDES));$(MSDEV.INCLUDE)

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))
endif # VSVERSION

############################################################################
#
#   Rules for building DLL, DYNLINK targets
#
############################################################################

$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPENDENCIES) $(DEFFILE)
	@echo "Linking DLL - $@"
	$(LD) $(LDFLAGS_$(TTYPE)) $(foreach p,$(LIBVPATH.W),"/LIBPATH:$p") -OUT:$@ $(DOS_DEPS) $(LIBS)
	$(NTICE_$(TTYPE))
