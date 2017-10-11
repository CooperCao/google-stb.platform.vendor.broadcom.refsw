#
# Makefile for tools/misc
#
# Copyright (c) 1999, Epigram, Inc.
#
# $Id$

SRCBASE = ../..

ifeq ($(TARGETENV),win32)
  # If we are compiling on win32 environment, 
  # and we haven't got the proper environment set up, 
  # goahead and source build_env for the user.
  # For this to work you must have already set TARGETENV to win32.
  #
  CL=$(firstword $(wildcard $(patsubst %,%/CL.EXE,$(subst :, ,$(PATH))) $(patsubst %,%/cl.exe,$(subst :, ,$(PATH)))))
  ifeq ($(CL),)
    NEED_BUILD_ENV=1
  endif
  SHELL=bash.exe
endif # TARGETENV=win32

# If compiler is not found in path then NEED_BUILD_ENV is set
# to source proper build environment and then rerun the same make target again

ifneq ($(NEED_BUILD_ENV),)

default:
	source $(SRCBASE)/tools/build/build_env.sh && $(MAKE) 

%:
	source $(SRCBASE)/tools/build/build_env.sh && $(MAKE) $@

else # NEED_BUILD_ENV is not defined (i.e we can now build)

# need to include first to pick up TARGETENV dependent vars
include $(SRCBASE)/Makerules

LCOPTS	= -O
ifeq ($(TARGETENV), win32)
  LCOPTS	+= -MT
endif # TARGETENV

ifeq ($(TARGETENV), win32)
  LLDLIBS	= wsock32.lib advapi32.lib user32.lib
  #LLDFLAGS = /DEBUG /PDB:NONE
  BCMSTDLIB = bcmstdlib$(OBJEXT)
else # non windows
  ifneq ($(TARGETENV), freebsd)
    LLDLIBS	= -liberty 
  endif
  LLDLIBS += -lm
  ifeq ($(TARGETENV), sun4)
    LLDLIBS	+= -lxnet
  endif
  ifeq ($(TARGETENV), linux)
    CFLAGS	+= -m32
    CFLAGS	+= -DWLAWDL
    LDFLAGS	+= -m32
  endif
  ifeq ($(TARGETENV), linuxmips)
    CFLAGS	+= -static
    LDFLAGS	+= -static
  endif
endif # TARGETENV

ifeq ($(TARGETENV), macos)
   CFLAGS  += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk
endif

TARGETS = addcrc$(EXEEXT) addhdr$(EXEEXT) bin2c$(EXEEXT) ciscreate$(EXEEXT) epi_ping$(EXEEXT) \
	  epi_ttcp$(EXEEXT) lzma$(EXEEXT) now$(EXEEXT) nvserial$(EXEEXT) nvdump$(EXEEXT) swap$(EXEEXT) \
	  symcmp$(EXEEXT) symsize$(EXEEXT) trx$(EXEEXT) lzma_4k$(EXEEXT) \
	  mkcore$(EXEEXT) logprint$(EXEEXT)

ifeq ($(TARGETENV), win32)
	TARGETS += epi_exec$(EXEEXT)
	SCRIPT_TARGETS = ttcp_dispatch.bat
else
	TARGETS += mkinrd$(EXEEXT) mkip$(EXEEXT)
endif

all: $(TARGETS)

showenv:
	@echo "INFO: CL.EXE         = $(CL)"
	@echo "INFO: TARGETENV      = $(TARGETENV)"
	@echo "INFO: HOSTENV        = $(HOSTENV)"
	@echo "INFO: NEED_BUILD_ENV = $(NEED_BUILD_ENV)"

RELEASE_TARGETS	= $(TARGETS) $(SCRIPT_TARGETS)

release: $(RELEASE_TARGETS)
	if [ ! -d $(RELEASE_DIR)/$(RELEASE_TOOLS_DIR) ]; then \
		mkdir -p $(RELEASE_DIR)/$(RELEASE_TOOLS_DIR); \
	fi;
	$(INSTALL) $(RELEASE_TARGETS) $(RELEASE_DIR)/$(RELEASE_TOOLS_DIR)

check_all:

clean:
	rm -f $(TARGETS) *.base *.exe *~ *# *.dll *.jnk *.o *.obj foo *.a

%$(OBJEXT): %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%$(EXEEXT): %$(OBJEXT)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

#shutils$(OBJEXT): ../shared/utils.c
#	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I../shared ../shared/utils.c $(CC_TARGET)

# nvserial needs an object built from the lzma_src directory
LzmaEnc$(OBJEXT) : lzma_src/C/LzmaEnc.c
		$(CC) -c  $^  $(CFLAGS) $(CPPFLAGS)  $(CC_TARGET)

LzmaDec$(OBJEXT) : lzma_src/C/LzmaDec.c
		$(CC) -c  $^  $(CFLAGS) $(CPPFLAGS)  $(CC_TARGET)

LzFind$(OBJEXT) : lzma_src/C/LzFind.c
		$(CC) -c  $^  $(CFLAGS) $(CPPFLAGS) $(CC_TARGET)

Alloc$(OBJEXT) : lzma_src/C/Alloc.c
		$(CC) -c  $^  $(CFLAGS) $(CPPFLAGS) $(CC_TARGET)

LzmaLib$(OBJEXT) : lzma_src/C/LzmaLib.c
		$(CC) -c  $^  $(CFLAGS) $(CPPFLAGS) $(CC_TARGET)


nvserial$(EXEEXT): nvserial$(OBJEXT) bcmutils$(OBJEXT) $(BCMSTDLIB) LzmaLib$(OBJEXT) Alloc$(OBJEXT)  LzmaEnc$(OBJEXT) LzFind$(OBJEXT) LzmaDec$(OBJEXT)  
	$(CC)  $^  $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

nvdump$(EXEEXT): nvdump$(OBJEXT) bcmutils$(OBJEXT) $(BCMSTDLIB) LzmaLib$(OBJEXT) Alloc$(OBJEXT)  LzmaEnc$(OBJEXT) LzFind$(OBJEXT) LzmaDec$(OBJEXT)  
	$(CC)  $^  $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

#if 0

watermark$(EXEEXT): watermark$(OBJEXT)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

watermark$(OBJEXT): watermark.c
	$(CC) -c -DBCMINTERNAL $(CFLAGS) $(CPPFLAGS) $< -o $@

fwtag$(EXEEXT): fwtag$(OBJEXT)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

fwtag$(OBJEXT): fwtag.c
	$(CC) -c -DBCMINTERNAL $(CFLAGS) $(CPPFLAGS) $< -o $@

#end /* BCMINTERNAL */

trx$(EXEEXT): trx$(OBJEXT) bcmutils$(OBJEXT) $(BCMSTDLIB)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

bcmutils$(OBJEXT): $(SRCBASE)/shared/bcmutils.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

bcmxtlv$(OBJEXT): $(SRCBASE)/shared/bcmxtlv.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

bcmstdlib$(OBJEXT): $(SRCBASE)/shared/bcmstdlib.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

addcrc$(EXEEXT): addcrc$(OBJEXT) bcmutils$(OBJEXT) $(BCMSTDLIB)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

#bin2c$(EXEEXT): bin2c$(OBJEXT) shutils$(OBJEXT)
bin2c$(EXEEXT): bin2c$(OBJEXT)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

lzma$(EXEEXT):
	make -C lzma_src/C/LzmaUtil -f makefile.gcc
	cp -f lzma_src/C/LzmaUtil/lzma lzma$(EXEEXT)

lzma_4k$(EXEEXT): lzma$(EXEEXT)
	cp -f lzma$(EXEEXT) lzma_4k$(EXEEXT)

mkcore$(EXEEXT): mkcore$(OBJEXT)
	$(CC) $^ -I$(SRCBASE)/include $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)

ifeq ($(TARGETENV), macos)
logprint: logprint.o logdump_decode.o bcmxtlv.o
	gcc -o logprint logprint.o logdump_decode.o bcmxtlv.o
else
logprint$(EXEEXT): logprint$(OBJEXT) logdump_decode$(OBJEXT) bcmxtlv$(OBJEXT)
	$(CC) $^ -I$(SRCBASE)/include $(LDFLAGS) $(LDLIBS) $(LINK_TARGET)
endif

ifneq ($(EXEEXT),)
# Rule to allow "gmake prog" to build prog.exe
# Used a static pattern rule because a simple implicit rule of the form
# %_tst : %_tst$(EXEEXT)
# will not work because the left-hand targets are phony.
#
TARG_BASENAMES = $(basename $(filter %$(EXEEXT),$(TARGETS)))
$(TARG_BASENAMES): %: %$(EXEEXT)
.PHONY : $(TARG_BASENAMES)
endif

endif # NEED_BUILD_ENV
