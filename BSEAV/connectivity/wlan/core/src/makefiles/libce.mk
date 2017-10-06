
REQUIRE_WINCE=1

define c-obj-command
    $(CC) -c $(C$(TTYPE)FLAGS) -I$(OUTDIR_$(TTYPE)) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
endef

C_DEFINES +=-DUNICODE -D_UNICODE -DUNDER_CE -DWIN32 -DSTRICT 
C_DEFINES +=-DINTERNATIONAL -DL0409 -DINTLMSG_CODEPAGE=1252

ifeq ($(PROCESSOR), MIPS)
C_DEFINES +=-DMIPS -DMIPSII -D_MIPS_ -DR4000 
endif
ifeq ($(PROCESSOR), ARM)
ifeq ($(WINCEVER), 500)
C_DEFINES +=-DARMV4 -DARM -D_ARM_ /QRinterwork-return /QRxscale
else
C_DEFINES +=-DARMV4 -DARM -D_ARM_
endif
endif
ifeq ($(PROCESSOR), X86)
C_DEFINES +=-D_X86_ -Dx86
endif

ifeq ($(WINCEVER), 500)
C_DEFINES +=-D_WIN32_WCE=500
_WINCE_VERSION=5.00
else
C_DEFINES +=-D_WIN32_WCE=420
_WINCE_VERSION=4.20
endif

COPTFLAGS  = -nologo -W3 -Ox -Os -Zi  $(C_DEFINES) $(C_DEFINES_$(TTYPE))
CDBGFLAGS  = -nologo -W3 -Od -Zi -DDEBUG=1 -DDBG=1 -DBCMDBG -Fd$(@D)/ $(C_DEFINES) $(C_DEFINES_$(TTYPE))

ifeq ($(WINCEFASTCAP),1)
ifeq ($(PROCESSOR), X86)
else
COPTFLAGS += -fastcap
CDBGFLAGS += -fastcap
endif
endif
ifeq ($(WINCECALLCAP),1)
ifeq ($(PROCESSOR), X86)
else
COPTFLAGS += -callcap
CDBGFLAGS += -callcap
endif
endif

INCLUDES += $(_PUBLICROOT)/speech/sdk/inc;$(_PUBLICROOT)/directx/sdk/inc;$(_PUBLICROOT)/wceappsfe/sdk/inc;$(_PUBLICROOT)/wceshellfe/sdk/inc;$(_PUBLICROOT)/shellsdk/sdk/inc;$(_PUBLICROOT)/rdp/sdk/inc;$(_PUBLICROOT)/servers/sdk/inc;$(_PUBLICROOT)/ie/sdk/inc;$(_PUBLICROOT)/dcom/sdk/inc;$(_WINCEROOT)/sdk/CE/inc;./Resource/0409

CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(INCLUDES)))
CEBLDTMP = "$(subst $(space),,_cebuild_$(TTYPE)_$(TARGETNAME)_$(TARGETTYPE).tmp)"

goo $(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPENDENCIES) $(DEFFILE)
	#
	# first create the .lib...
	#
	@echo -subsystem:windowsce,$(_WINCE_VERSION) > $(CEBLDTMP)
	@$(foreach dep,$(DOS_DEPS),echo $(dep) >> $(CEBLDTMP);)
	$(LIBCMD) -OUT:$(OUTDIR_$(TTYPE))/$(TARGETNAME).lib @$(CEBLDTMP)
	rm $(CEBLDTMP)
