
############################################################################
#
#   Flags for building DOSEXE targets
#
############################################################################
    REQUIRE_DOS16 = 1
    REQUIRE_VC152 = 1

    TARGET=$(TARGETNAME).exe

    LIBS = $(TARGETLIBS)

    CDBGFLAGS = /nologo  /D_DEBUG /D_DOS /AL /G3 /Gs /W1 /Fd"DIAG.PDB" $(C_DEFINES)
    COPTFLAGS = /nologo /DNDEBUG /D_DOS /AL /G3 /Gs /W1 $(C_DEFINES)

    LIBVPATH += $(DOS16.UNC)/LIB/MSC80:$(DOS16.UNC)/LIB

    INCLUDEPATH=include;$(VC152)\include;$(DOS16)\include
    LIBPATH=$(VC152)\lib;$(DOS16)\lib;$(DOS16)\lib\msc80

    CCPATH=$(VC152.UNC)/bin:$(DOS16.UNC)/bin

    # %.obj :  %.c
    define c-obj-command
	PATH=$(CCPATH); \
	INCLUDE="$(INCLUDEPATH)" \
        $(CC) -c $(C$(TTYPE)FLAGS) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
    endef

    # %.obj :  %.cpp
    define cpp-obj-command
	PATH=$(CCPATH); \
	INCLUDE="$(INCLUDEPATH)" \
        $(CC) -c $(C$(TTYPE)FLAGS) $(CPPFLAGS) $(F$(TTYPE)) -Fo$@ $<
    endef


    define link-command
	@echo -QUIET -DBG -DPMI -STACK:4096 > $(TARGETNAME).glu
	@echo $(DEPS_OBJ) >> $(TARGETNAME).glu	
	@echo "e($(TARGETNAME))"  >> $(TARGETNAME).glu
	@echo "m($(TARGETNAME))"  >> $(TARGETNAME).glu
	@$(foreach lib,$(TARGETLIBS),echo $(lib)  >> $(TARGETNAME).glu;)
	PATH="$(DOS16.UNC)/bin"; \
	LIB="$(LIBPATH)"; \
	glu @$(TARGETNAME).glu; \
	splice $@ $(TARGETNAME).exp loader.exe
    endef



############################################################################
#
#   Rules for building DOSEXE targets 
#
############################################################################


$(OUTDIR_$(TTYPE))/$(TARGET) :: $(DEPENDENCIES) 
	@echo "Linking Library - $@ for i386"
	$(link-command)
