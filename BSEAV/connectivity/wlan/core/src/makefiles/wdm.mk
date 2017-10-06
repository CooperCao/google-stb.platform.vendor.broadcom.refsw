############################################################################
#
#   Flags for building NDIS5 DRIVER targets
#
############################################################################

    REQUIRE_WDM = 1
    REQUIRE_MSDEV = 1

    TARGET=$(TARGETNAME).sys
	BASEDIR=$(WDMDDK)
#	TARGET_DIRECTORY=i386

    C_DEFINES += -D_X86_=1 -Di386=1  -DSTD_CALL -DCONDITION_HANDLING=1 \
	         -DNT_UP=1  -DNT_INST=0 -DWIN32=100 -D_NT1X_=100        \
	         -DWINNT=1 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D_WIN32_IE=0x0501 \
			 -DWIN32_LEAN_AND_MEAN=1 -DDEVL=1 -DNDEBUG -D_DLL=1

    FDBG =  -DDEBUG  -DRDRDBG -DSRVDBG -DFPO=0 -DDBG=1
    FOPT =  -DFPO=1

	FLAGS_BASE = /c -nologo /Zel /Zp8 /Gy -cbstring /W3 /Gz /QIfdiv- /QIf /QI0f /GB /Gi- \
			/Gm- /GX- /GR- /GF

    COPTFLAGS  = $(FLAGS_BASE) /Oxs /Oy -FI$(WDMDDK)/inc/warning.h $(C_DEFINES)

    CDBGFLAGS  =  $(FLAGS_BASE) -Z7 /Od /Oi /Oy- -FI$(WDMDDK)/inc/warning.h $(C_DEFINES)

    MSINCLUDE=i386;.;$(strip $(INCLUDES));$(WDMDDK)/inc;$(MSDEV)/VC98/ATL/INCLUDE;$(MSDEV)/VC98/INCLUDE;$(MSDEV)/VC98/MFC/INCLUDE;

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg"  -r \
		$(C_DEFINES) $(CPPFLAGS)

    ifeq ($(strip $(SUBSYSTEM)), )
	SUBSYSTEM=native,5.00
    endif

    LDFLAGS = -STACK:262144,4096 -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text \
		   -SECTION:INIT,d -OPT:REF -OPT:ICF -INCREMENTAL:NO -FULLBUILD \
		   -IGNORE:4001,4037,4039,4044,4065,4070,4078,4087,4089,4198 \
	       -FORCE:MULTIPLE -NOCOMMENT /release -NODEFAULTLIB -debug:FULL \
	       -debugtype:cv -version:5.00 -osversion:5.00 -optidata -driver \
		   -align:0x20 -subsystem:$(SUBSYSTEM) -base:0x10000 \
		   -entry:DriverEntry@8 

    LDFLAGS_OPT = $(LDFLAGS)
    LDFLAGS_DBG = $(LDFLAGS)

    LIBVPATH += $(MSDEV.UNC)/vc98/lib:$(MSDEV.UNC)/vc98/mfc/lib
	ifeq ($(DDKBUILDENV), "checked")
		DDK_LIB_PATH=$(WDMDDK)/libchk/i386
		LIBS = $(WDMDDK)/libchk/i386/int64.lib \
			   $(WDMDDK)/libchk/i386/ntoskrnl.lib \
			   $(WDMDDK)/libchk/i386/hal.lib \
		   $(TARGETLIBS)
	else
		DDK_LIB_PATH=$(WDMDDK)/libfre/i386
		LIBS = $(WDMDDK)/libfre/i386/int64.lib \
			   $(WDMDDK)/libfre/i386/ntoskrnl.lib \
			   $(WDMDDK)/libfre/i386/hal.lib \
		   $(TARGETLIBS)
	endif

    NTICE_OPT=
ifdef	NTICE
    NTICE_DBG= @echo "Building SoftICE symbols for $@"; \
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
