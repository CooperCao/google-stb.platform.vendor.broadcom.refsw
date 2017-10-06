
############################################################################
#
#   Flags for building EXE16 targets
#
############################################################################
REQUIRE_W98DDK = 1
REQUIRE_VC152 = 1
REQUIRE_MSDEV = 1

TARGET=$(TARGETNAME).exe

C_DEFINES += -D_X86_ -D_MMX_
CDBGFLAGS  = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /GD /Fd$(@D)/$(TARGETNAME).pdb $(C_DEFINES)
COPTFLAGS  = /nologo /W3 /ALw /D "NDEBUG" /GD $(C_DEFINES)

LDFLAGS_OPT = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LDFLAGS_DBG = $(LDFLAGS_OPT) /CO 

LIBVPATH += $(MSDEV.UNC)/vc98/lib:$(MSDEV.UNC)/vc98/mfc/lib
LIBVPATH += $(W98DDK.UNC)/lib/win98 $(VC152.UNC)/lib $(VC152.UNC)/mfc/lib 
LIBS = $(TARGETLIBS)

RCFLAGS_OPT = -dNDEBUG $(C_DEFINES) 
RCFLAGS_DBG = -d_DEBUG $(C_DEFINES) 

LIB=$(subst /,\\,$(W98DDK)/lib/win98\;$(VC152)/lib\;$(VC152)/mfc/lib)


NEWPATH += $(VC152.UNC)/bin
NEWPATH += $(W98DDK.UNC)/bin/win98

ifdef ECLOUD_BUILD_ID
  CMDSHELL := $(subst \,/,$(COMSPEC))
  CMDSTART := $(CMDSHELL) /c start /min /separate
else # ECLOUD_BUILD_ID
  CMDSHELL :=
  CMDSTART :=
endif # ECLOUD_BUILD_ID

CCPATH=$(VC152.UNC)/bin:$(W98DDK.UNC)/bin/win98
# %.obj :  %.c
define c-obj-command
PATH=$(CCPATH); \
    $(CMDSTART) $(CC) -c $(C$(TTYPE)FLAGS) -I$(OUTDIR_$(TTYPE)) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
endef

# %.obj :  %.cpp
define cpp-obj-command
PATH=$(CCPATH); \
    $(CMDSTART) $(CC) -c $(C$(TTYPE)FLAGS) -I$(OUTDIR_$(TTYPE)) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
endef

# %.obj :  %.asm
define asm-obj-command
PATH=$(CCPATH); \
$(CMDSTART) $(AS) -c $(AFLAGS) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
endef

# %.res :  %.rc
define rc-res-command
export INCLUDE="$(OUTDIR_$(TTYPE));$(MSINCLUDE)"; \
PATH=$(CCPATH); \
$(CMDSTART) $(RC) -r $(RCFLAGS) -fo$@ $<
endef

ifneq ($(strip $(SOURCES.MSG)),)
MSGINCLUDE=$(OUTDIR_$(TTYPE));
endif

MSINCLUDE=$(strip $(MSGINCLUDE)).;$(strip $(INCLUDES));$(W98DDK)/inc/win98/inc16;$(VC152)/include;$(VC152)/mfc/include;$(W98DDK)/inc/win98

CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))
RESFLAGS=$(C_DEFINES)

MAPFILE=$(@D)/$(TARGETNAME).map

NTICE_OPT=

ifdef	NTICE
NTICE_DBG= echo "Building SoftICE symbols for $@"; \
"$(NTICE)/nmsym.exe" $(SI_FLAGS) /OUT:$(@D)/$(TARGETNAME).nms\
-SOURCE:. $@
else
NTICE_DBG=
endif


############################################################################
#
#   Rules for building EXE16 targets
#
############################################################################

FOO:=link.$(shell echo $$$$)


$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPS_OBJ) $(LIBS) $(DEFFILE)
	@echo "Linking EXE - $@"
	@echo "$(subst /,\\,$(foreach dir,$(DEPS_OBJ),$(dir) + ))" >$(FOO)
	@echo >>$(FOO)
	@echo "$(subst /,\\,$@)" >>$(FOO)
	@echo "$(subst /,\\,$(MAPFILE))" >>$(FOO)
	@echo "$(LIBS)" >>$(FOO)
	@echo "$(DEFFILE);" >>$(FOO)
	@echo ::::::::::::::::::::::::::::::::::::::::
	@cat $(FOO)
	@echo ::::::::::::::::::::::::::::::::::::::::
	export LIB=$(LIB); \
	PATH=$(CCPATH); \
	$(LD) $(LDFLAGS_$(TTYPE)) @$(FOO)
	rm -f $(FOO)
	$(NTICE_$(TTYPE))	

ifneq ($(SOURCES.RES), )

$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPS_RES)
ifdef ECLOUD_BUILD_ID
	PATH=$(CCPATH); \
	$(CMDSHELL) /c "start /min /separate $(RC) $(RESFLAGS) $^ $@" && \
	implib /nologo /nowep $(@:%.exe=%.lib) $@
else # ECLOUD_BUILD_ID
	PATH=$(CCPATH); \
	$(RC) $(RESFLAGS) $^ $@ && \
	implib /nologo /nowep $(@:%.exe=%.lib) $@
endif # ECLOUD_BUILD_ID

endif # $(SOURCES.RES) != NULL
