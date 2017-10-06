
############################################################################
#
#   Flags for building DLL16 targets
#
############################################################################
    REQUIRE_W98DDK = 1
    REQUIRE_VC152 = 1
    REQUIRE_MSDEV = 1

    TARGET=$(TARGETNAME).dll

    C_DEFINES += -D_X86_ -D_MMX_
    CDBGFLAGS  = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /FR /GD /Fd$(@D) $(C_DEFINES)
    COPTFLAGS  = /nologo /W3 /ALw /D "NDEBUG"  /FR /GD $(C_DEFINES)

    LDFLAGS_OPT = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
    LDFLAGS_DBG = $(LDFLAGS_OPT) /CO 

    LIBVPATH += $(MSDEV.UNC)/vc98/lib:$(MSDEV.UNC)/vc98/mfc/lib
    LIBVPATH += $(W98DDK.UNC)/lib/win98 $(VC152.UNC)/lib $(VC152.UNC)/mfc/lib 
    LIBS = $(TARGETLIBS)

    RCFLAGS_OPT = -dNDEBUG $(C_DEFINES) 
    RCFLAGS_DBG = -d_DEBUG $(C_DEFINES) 

    LIB=$(subst /,\\,$(W98DDK)/lib/win98\;$(VC152)/lib\;$(VC152)/mfc/lib)

    NEWPATH += $(VC152.UNC)/bin

    ifneq ($(strip $(SOURCES.MSG)),)
	MSGINCLUDE=$(OUTDIR_$(TTYPE));
    endif

    MSINCLUDE=$(strip $(MSGINCLUDE)).;$(strip $(INCLUDES));$(W98DDK)/inc/win98/inc16;$(VC152)/include;$(VC152)/mfc/include

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))
    RESFLAGS=$(C_DEFINES)




############################################################################
#
#   Rules for building DLL16 targets
#
############################################################################

FOO:=link.$(shell echo $$$$)


$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPS_OBJ) $(LIBS) $(DEFFILE)
	@echo "Linking DLL - $@"
	@echo "$(subst /,\\,$(foreach dir,$(DEPS_OBJ),$(dir) + ))" >$(FOO)
	@echo >>$(FOO)
	@echo "$(subst /,\\,$@)" >>$(FOO)
	@echo "nul" >>$(FOO)
	@echo "$(LIBS)" >>$(FOO)
	@echo "$(DEFFILE);" >>$(FOO)
	@echo ::::::::::::::::::::::::::::::::::::::::
	@cat $(FOO)
	@echo ::::::::::::::::::::::::::::::::::::::::
	export LIB=$(LIB); \
	$(LD) $(LDFLAGS_$(TTYPE)) @$(FOO)
	rm -f $(FOO)
	$(NTICE_$(TTYPE))	

ifneq ($(SOURCES.RES), )

$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPS_RES)
	$(RC) $(RESFLAGS) $^ $@ 
	implib /nologo /nowep $(@:%.dll=%.lib) $@

endif # $(SOURCES.RES) != NULL
