############################################################################
#
#   Flags for building NDIS5 DRIVER targets
#
############################################################################

    REQUIRE_WDM2600 = 1
    DDK_OS=wxp

    TARGET=$(TARGETNAME).sys
	WDMDDK=$(WDM2600DDK)
	WDMDDK.UNC=$(WDM2600DDK.UNC)
	BASEDIR=$(WDMDDK)

    C_DEFINES += -D_X86_=1 -Di386=1  -DSTD_CALL -DCONDITION_HANDLING=1   -DNT_INST=0 -DWIN32=100 -D_NT1X_=100 -DWINNT=1 -D_WIN32_WINNT=0x0500 /DWINVER=0x0500 -D_WIN32_IE=0x0600 -DWIN32_LEAN_AND_MEAN=1 -DDEVL=1 -D__BUILDMACHINE__=WinDDK  -DNDEBUG -D_DLL=1 

# these two are used in src/GNUmakefile.inc in the form of F$(TTYPE)
    FDBG =   -DDEBUG  -DDBG=1 -DFPO=0 -DBCMDBG -DBCMDBG_MEM
    FOPT =   -DNDEBUG -DFPO=1 

    FLAGS_BASE = -nologo /c /Zel /Zp8 /Gy -cbstring /W3 /WX /Gz  /QIfdiv- /QIf /G6 /Gi- /Gm- /GX-  /GR- /GF -Z7 \
	         -FI$(WDMDDK)/inc/$(DDK_OS)/warning.h 

    COPTFLAGS  = $(FLAGS_BASE) /Oxs /Oy  $(C_DEFINES)

    CDBGFLAGS  =  $(FLAGS_BASE) /Od /Oi /Oy- $(C_DEFINES)

    MSINCLUDE=i386;$(strip $(INCLUDES));$(WDMDDK)/inc/$(DDK_OS);$(WDMDDK)/inc/ddk/$(DDK_OS);$(WDMDDK)/inc/ddk/wdm/$(DDK_OS);$(WDMDDK)/inc/crt;

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg"  -r \
		$(C_DEFINES) $(CPPFLAGS)

    ifeq ($(strip $(SUBSYSTEM)), )
	SUBSYSTEM=native,5.00
    endif

    LDFLAGS_DBG = -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -OPT:ICF \
		  -IGNORE:4010,4037,4039,4065,4070,4078,4087,4089,4198,4221 -INCREMENTAL:NO \
		  -FULLBUILD /release -NODEFAULTLIB /WX -debug:FULL -debugtype:cv -version:5.1 \
		  -osversion:5.1 /opt:nowin98 -STACK:0x40000,0x1000 -driver -base:0x10000 \
		  -align:0x80 -subsystem:native,5.1 -entry:DriverEntry@8 -map

    LDFLAGS_OPT = -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -OPT:ICF \
		  -IGNORE:4010,4037,4039,4065,4070,4078,4087,4089,4198,4221 -INCREMENTAL:NO \
		  -FULLBUILD /release -NODEFAULTLIB /WX -debug:FULL -debugtype:cv -version:5.1 \
		  -osversion:5.1 /opt:nowin98 -STACK:0x40000,0x1000 -driver -base:0x10000 \
		  -align:0x80 -subsystem:native,5.1 -entry:DriverEntry@8 -map

    # LIBVPATH += $(MSDEV.UNC)/vc98/lib:$(MSDEV.UNC)/vc98/mfc/lib

    LIBS = $(WDMDDK)/lib/$(DDK_OS)/i386/ntoskrnl.lib \
	   $(WDMDDK)/lib/$(DDK_OS)/i386/hal.lib \
	   $(WDMDDK)/lib/$(DDK_OS)/i386/wmilib.lib \
		   $(TARGETLIBS)

    DDK_LIB_PATH=$(WDMDDK)/lib/$(DDK_OS)/i386


     NEWPATH += $(WDMDDK.UNC)/bin/x86/

    NTICE_OPT=
ifdef	NTICE
    NTICE_DBG= echo "Building SoftICE symbols for $@"; \
	"$(NTICE)/nmsym.exe" $(SI_FLAGS) \
	-SOURCE:. $@
else
    NTICE_DBG=
endif


############################################################################
#
#   Rules for building NT DRIVER targets
#
############################################################################

$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPS_OBJ) $(DEPS_RES) $(DEFFILE)
	@echo "Linking Executable - $@ for i386"
	$(LD) $(LDFLAGS_$(TTYPE)) -OUT:$@ \
		$(DEPS_OBJ) $(DEPS_RES) $(LIBS)
	$(NTICE_$(TTYPE))
