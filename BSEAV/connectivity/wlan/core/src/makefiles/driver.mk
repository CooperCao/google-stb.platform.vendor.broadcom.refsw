############################################################################
#
#   Flags for building NT4 DRIVER targets
#
############################################################################

    REQUIRE_DDK = 1
    REQUIRE_MSDEV = 1

    TARGET=$(TARGETNAME).sys

    C_DEFINES += -D_X86_=1 -Di386=1  -DSTD_CALL -DCONDITION_HANDLING=1 \
	         -DNT_UP=1  -DNT_INST=0 -DWIN32=100 -D_NT1X_=100        \
	         -DWINNT=1 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN=1 \
	         -DDEVL=1 -D_DLL=1


    FDBG =  -DDEBUG  -DRDRDBG -DSRVDBG  -DFPO=0
    FOPT =  -DNDEBUG -DFPO=1

    COPTFLAGS  = -c -nologo -Zel -Zp8 -Gy -cbstring -W3 -Gz  -QIfdiv- -QIf  \
		 -Gi- -Gm- -EHsc- -GR- -GF   -Oxs -Oy                 \
		 -FI$(DDK)/inc/warning.h $(C_DEFINES)

    CDBGFLAGS  =  -c -nologo -Zel -Zp8 -Gy -cbstring -W3 -Gz  -QIfdiv- -QIf  \
		  -Gi- -Gm- -EHsc- -GR- -GF  -Z7 -Od -Oi -Oy-          \
	          -FI$(DDK)/inc/warning.h $(C_DEFINES)

    MSINCLUDE=i386;.;$(strip $(INCLUDES));$(DDK)/inc;$(MSDEV)/VC98/ATL/INCLUDE;$(MSDEV)/VC98/INCLUDE;$(MSDEV)/VC98/MFC/INCLUDE;

    CPPFLAGS = $(patsubst %,-I%,$(subst ;, ,$(MSINCLUDE)))

    RCFLAGS  = -l 409 -z "MS Sans Serif,Helv/MS Shell Dlg"  -r \
		$(C_DEFINES) $(CPPFLAGS)

    ifeq ($(strip $(SUBSYSTEM)), )
	SUBSYSTEM=native,4.00
    endif

    LDFLAGS =  -PDB:NONE -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d \
	       -OPT:REF -INCREMENTAL:NO -FORCE:MULTIPLE -RELEASE -FULLBUILD \
	       -IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 \
	       -NODEFAULTLIB \
	       -version:4.00 -osversion:4.00 -PDB:NONE -MERGE:.rdata=.text \
	       -optidata -driver -align:0x20 -subsystem:$(SUBSYSTEM) \
	       -base:0x10000 -entry:DriverEntry@8 

    LDFLAGS_OPT = $(LDFLAGS) -debug:notmapped,MINIMAL -debugtype:coff 
    LDFLAGS_DBG = $(LDFLAGS) -debug:notmapped,FULL -debugtype:both 

    LIBVPATH += $(MSDEV.UNC)/vc98/lib:$(MSDEV.UNC)/vc98/mfc/lib
    LIBS = $(DDK)/lib/i386/$(DDKBUILDENV)/int64.lib \
           $(DDK)/lib/i386/$(DDKBUILDENV)/ntoskrnl.lib \
           $(DDK)/lib/i386/$(DDKBUILDENV)/hal.lib \
	   $(TARGETLIBS)


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
