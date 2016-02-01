#############################################################################
#
# Copyright (c) 2007-2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.  
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################
#  $Id$
#
#  Filename:       BfcLinux.mak
#  Author:         Mike Sieweke (based on VxWorks original by David Pullen)
#  Creation Date:  November 14, 2003
#
##############################################################################
#  Description:
#
#	Makefile for the GNU make utility, which has specific rules for building
#	BFC systems for Linux.
#
##############################################################################


# Some users may want to have the shell located somewhere besides /bin/sh -
# strange but true.  We'll allow them to specify which shell to use for the
# make job.

ifdef LINUX_MAKE_SHELL
    export SHELL := $(LINUX_MAKE_SHELL)
endif

##############################################################################
#
# Set up relative paths as macros; this makes it easier to relocate things as
# needed, and also makes it easier to read the rest of the stuff.

BFC_MAKE_DIR = $(BFC_ROOT_DIR)/Bfc/make

BFC_LINUX_DIR         = Bfc/linux

# TODO: NOT REQUIRED FOR LINUX CURRENTLY
# BFC_LINUX_PATCHES_DIR = $(BFC_LINUX_DIR)/patches/v2_0

BFC_BOOTLOADER_DIR   = $(BFC_ROOT_DIR)/Bfc/LibSupport/Bootloader

# A bunch of old-school V2 BSP stuff.  We don't have time to rework the BSP
# completely at this point, so we're leveraging the existing stuff as much as
# possible for now.  Over time, we hope to redo this.
#
# $(buildProcessor) is set by the top level build script.
BSP_ROOT            = cm_bsp_v2
BSP_COMMON          = $(BSP_ROOT)/bsp_common
BSP_COMMON_OS       = $(BSP_COMMON)/os/linux
BSP_COMMON_OS_TARGET = $(BSP_COMMON)/os/linux/bcm335x
BSP_COMMON_INC      = $(BSP_COMMON)/common_inc
BSP_COMMON_SRC      = $(BSP_COMMON)/common_src
BSP_BOARDHAL        = $(BSP_COMMON)/BoardHal

BSP_PLATFORM        = $(BSP_ROOT)/bsp_bcm9$(buildProcessor)
BSP_PLATFORM_COMMON = $(BSP_PLATFORM)/src
BSP_PLATFORM_OS     = $(BSP_PLATFORM)/os/linux


##############################################################################
#
# Set up the variables that are used as external constants by the component
# makefiles.

BFC_TARGET_OS = Linux
BFC_TARGET_PROCESSOR = MIPS


##############################################################################
#
# Tell GNU Make about the file extensions that it needs to pay attention to.
# I have to clear the default list first.

.SUFFIXES :
.SUFFIXES : .cpp .cxx .o .h .c .s .a .bin


##############################################################################
#
# Macros for the various tools we use, and their options.

# This is where we define the compiler tools and sets up the flags and
# other options for Linux.
include $(BFC_ROOT_DIR)/$(BFC_LINUX_DIR)/makeinc/defs.bsp


# Add dependency compiler flags. -MD generates the dependency file as a
# side-effect of the compilcation process. The -MT specifies the targets of the
# the rules generated by -MD (which should be the dependency file and object file).
ifeq ($(BFC_MAKE_DEPENDENCY_FILES),1)
    ADDED_DEP_FLAGS = -MD -MT $(*F).d -MT $(*F).o
endif


# Now we override compiler flags with our own.  CC_OPTIM is used for both
# C and C++, but there are separate "added flags" variables for each language.
ADDED_CFLAGS   += \
        -Wimplicit \
        -Wall \
        -Wno-missing-braces \
        $(ADDED_DEP_FLAGS)
ADDED_C++FLAGS += \
        -Wimplicit \
        -Wall \
        -Wno-deprecated \
        -Wno-missing-braces \
        -finline-limit=4800 \
        -fno-for-scope \
        -fno-rtti \
        $(ADDED_DEP_FLAGS)
CC_OPTIM       = -O2 -fno-exceptions -fno-unroll-loops -fno-for-scope -fno-rtti

# This allows us to temporarily add flags for a single build or a single file
# so we don't need to manually edit this file as often.  One useful flag is
# "-H", which can be added with this command:
#   $ ADD_FLAGS=-H makeapp <parameters>
# Multiple flags can be added by enclosing them in quotes:
#   $ ADD_FLAGS="-H -v" makeapp <parameters>
ifneq ($(ADD_FLAGS),)
    CC_OPTIM += $(ADD_FLAGS)
endif
# This was the original form, but we don't add flags just for debugging.
ifneq ($(DEBUG_FLAGS),)
    CC_OPTIM += $(DEBUG_FLAGS)
endif

# If we are building 'slim' or 'superslim', we can specify the NDEBUG define,
# which gets rid of debugging things like assert(), etc.
ifeq ($(BCM_REDUCED_IMAGE_SIZE),1)

    CC_OPTIM += -DNDEBUG

endif

# If we are building 'superslim', modify the build options so that we favor
# things that reduce code size.
ifeq ($(BFC_CONFIG_SUPERSLIM),1)

    # This should be like -O2, but disabling optimizations that increase code
    # size.
    CC_OPTIM += -Os

    # Other things I've tried to reduce code size, but that didn't pan out:
    #
    #  -O3 : The compiler crashes with this for some reason.  -Os works.
    #
    #  -finline-functions : Good lord, don't use this!  It bloats the code by 100k!
    #
    #  -finline-limit=10000 : This increases the compressed size a bit.
    #
    #  -fshort-enums : Increased code size a bit.
    #
    #  -fconserve-space : No impact, doesn't have any effect.
    #  -fmerge-all-constants
    #  -fno-enforce-eh-specs
    #  -fomit-frame-pointer
    #  -foptimize-sibling-calls
    #  -fno-keep-static-consts
    #  -fshared-data

endif

# PR12289 - if we are doing function profiling, set the build option.
ifeq ($(BFC_INCLUDE_FUNCTION_PROFILE_SUPPORT),1)

    CC_OPTIM += -finstrument-functions

endif

##############################################################################
#
# Top level targets for the build.  The first one handles the case where no
# target was specified; this is not allowed.

NOTHING :
	@ echo :
	@ echo No make target was specified!  Please specify a target to build...
	# -- NO TARGET TO MAKE, ABORTING --


clean :
	@ echo :
	@ echo Making output directory clean...
	-@ rm -f -- *

CLEAN : clean


##############################################################################
#
# Include the relevant component makefiles, adding them to the list of files,
# directories, options, etc., as appropriate for the system configuration.


# Get the BFC system configuration file.  This is specific to each
# application/system, and comes from the app's portable directory.
include $(APP_PORTABLE_DIR)/BfcSystemConfig.mak


# UNFINISHED - the eCos kernel has callouts to our code that, unfortunately,
# forces the HeapManager to be included.  Until we change the kernel to remove
# this dependency, we need to force the HeapManager to be included for eCos.
#
# TODO:  HEAP_MANAGER IS CURRENTLY NOT REQUIRED FOR LINUX
BFC_INCLUDE_HEAP_MANAGER_SUPPORT = 0

# BFC global makefile.  This has all the BFC component makefiles together.
include $(BFC_MAKE_DIR)/Bfc.mak

# If the top level makefile set things up properly, let's include the target-
# specific makefile to get drivers and other components.
ifeq ($(IncludeTargetSpecificMakefile), 1)
    include $(BFC_MAKE_DIR)/$(TargetSpecificMakefile)
endif

# Finally, include the application/system specific makefile.
include $(APP_PORTABLE_DIR)/BfcSystem.mak


# For eCos, we need to include a number of files for the BSP (Board Support
# Package).  This stuff, which comes free in Win32 land, handles CPU init, low
# level interrupt processing, etc.
LINUX_OBJECTS =  \
		root.o \
		BfcLinuxOsApplicationFactory.o \
        	linuxBfcApplication.o

# Even though the IP stack (and networking support in general) is optional,
# the eCos kernel has hooks that require our IP stack mods to be required for
# all builds.
#
# THIS IS CURRENTLY NOT REQUIRED BY THE LINUX VERSION OF BFC
#
# LINUX_OBJECTS += $(BFC_ROOT_DIR)/$(BFC_LINUX_PATCHES_DIR)/if_bcm.o

LINUX_DIRS = \
        $(BSP_COMMON_OS) \
        $(BSP_PLATFORM_OS) \
        $(BSP_COMMON_SRC) \
        $(BSP_COMMON_INC) \
        $(BSP_BOARDHAL)

LINUX_EXTRA_INCLUDES = \
        $(BFC_USB_NETWORKING_INCLUDE_DIRS)

# Add the eCos-specific stuff to the primary BFC macros.
BFC_OBJECTS      += $(LINUX_OBJECTS)
BFC_INCLUDE_DIRS += $(BFC_LINUX_DIR) $(LINUX_DIRS) $(LINUX_EXTRA_INCLUDES)
BFC_SOURCE_DIRS  += $(BFC_LINUX_DIR) $(LINUX_DIRS)

# PR12289 - add function profiling stuff if enabled.
BFC_COMPILER_DEFINES += BFC_INCLUDE_FUNCTION_PROFILE_SUPPORT=$(BFC_INCLUDE_FUNCTION_PROFILE_SUPPORT)
ifeq ($(BFC_INCLUDE_FUNCTION_PROFILE_SUPPORT),1)

    BFC_OBJECTS += GnuFunctionProfile.o

endif

# Move VersionBanner.o to the end of the list so that it is built last.  While
# we're at it, lets sort the list.
ALL_OBJS = $(sort $(filter-out VersionBanner.o, $(BFC_OBJECTS))) VersionBanner.o


# Each of the compiler defines need to be prepended with -D.
EXTRA_DEFINE += $(addprefix -D, $(BFC_COMPILER_DEFINES)) \
                -DBFC=1 \
                -DPROCESSOR_TYPE=BCMMIPSEL \
                -DTARGETOS_Linux=1 \
                -DBCM_REDUCED_IMAGE_SIZE=$(BCM_REDUCED_IMAGE_SIZE) \
                -DBFC_CONFIG_SUPERSLIM=$(BFC_CONFIG_SUPERSLIM) \
                -DBFC_CONFIG_MINIMAL_CONSOLE=$(BFC_CONFIG_MINIMAL_CONSOLE) \
                -DEXEC_FROM_FLASH=1 \
                -DXTALFREQ=$(XTALFREQ) \
                -Dxtal_freq=$(XTALFREQ) \
                -DBFC_CONFIG_SERIAL_PORT_DISABLED=$(BFC_CONFIG_SERIAL_PORT_DISABLED) \
                -DBFC_INCLUDE_SMP_SUPPORT=${BFC_INCLUDE_SMP_SUPPORT}


# UNFINISHED - may need to turn this on/off based on other configuration
# parameters; i.e. if HPNA is included, have to set this to 0.
EXTRA_DEFINE += -DCACHE_WRITEBACK_MODE=1

CURRENT_DEFINES := $(EXTRA_DEFINE)
EXTRA_DEFINE    := -include defines.h

NPTL_SUPPORT=0
ifeq ($(GCC_VERSION), 4)
  NPTL_SUPPORT = 1
endif

ifeq ($(NPTL_SUPPORT), 1)
  EXTRA_DEFINE += -DNPTL_SUPPORT=1
endif

# Generate header file with all compiler defines. This will be included by all
# source files using the '-include' compiler option. Note that a new defines.h
# is only generated if one of the compiler defines has changed from the previous
# compile. Otherwise, the old defines.h is retained. This prevents all source
# files from being re-compiled every iteration due to dependency files.
junk  := $(shell echo $(CURRENT_DEFINES) | sed -f $(BFC_ROOT_DIR)/$(BFC_LINUX_DIR)/defines.sed | sort >defines.h.new)
junk2 := $(shell cmp -s defines.h defines.h.new || cp -f defines.h.new defines.h)


# Now that we have the full list of include dirs, convert them so that they are
# relative to the current directory, then prepend the "-I " flag in front of
# them for use by the compiler.  BSP_PLATFORM_COMMON is added to the front of
# the list so files there can supersede other BSP files.
EXTRA_INCLUDE = $(addprefix -I$(BFC_ROOT_DIR)/, $(BSP_PLATFORM_COMMON) $(sort $(BFC_INCLUDE_DIRS))) \
                -I$(APP_LINUX_OUTPUT_DIR) \
                -I$(APP_LINUX_DIR)


# Use the source dirs as the VPATH so make knows where to find things.  We have
# to make the path relative to the current directory, and we also need to add
# the app-specific directories to it.  BSP_PLATFORM_COMMON is added to the
# front of the list so files there can supersede other BSP files.
#
# TODO:  TOOK OUT BSP_PLATFORM_COMMON FROM VPATH
VPATH = $(addprefix $(BFC_ROOT_DIR)/, $(sort $(BFC_SOURCE_DIRS))) \
        $(APP_LINUX_OUTPUT_DIR) \
        $(APP_LINUX_DIR)


# Pull in the specified libraries, prepending with the relative path to the BFC
# root directory.
LIB_EXTRA += $(addprefix $(BFC_ROOT_DIR)/, $(BFC_LIBRARIES))
LIB_EXTRA += $(BFC_LIBRARIES_ABS_PATH)


# Strip out the linker script from the linker flags; this may be overridden.
LDFLAGS_BASIC = $(filter-out -Ttarget.ld, $(LDFLAGS))



##############################################################################
#
# Include dependencies. Note that make will generate the dependency files
# if they're missing.
#
# The '-' at the beginning tells make not to complain about the missing
# dependency file. Since there are rules to make dependencies, make will
# automatically try to build the dependencies before building any targets.
#

ALL_DEPS = $(ALL_OBJS:.o=.d)

ifeq ($(BFC_MAKE_DEPENDENCY_FILES),1)
    ifeq ($(strip $(filter eCos, $(MAKECMDGOALS))),eCos)
        ifneq ($(wildcard $(filter-out VersionBanner.d BcmHeapManagerMutexApiEcos.d, $(ALL_DEPS))),)
            -include $(wildcard $(filter-out VersionBanner.d BcmHeapManagerMutexApiEcos.d, $(ALL_DEPS)))
        endif
    endif
endif


##############################################################################
#
# Rule to build the executable from the list of intermediate object files.
# The dependencies are set up so we always erase the version banner object
# module (so that the build date/time is correct).
#
# We have to write the linker options to external files because they can be
# very long, exceeding the max command line length.


# Force linkobjs.txt to be deleted so that we can correctly append the list of
# object modules to it.
GetRidOfLinkObjs := $(shell rm -f linkobjs.txt)

# Also force VersionBanner.o to be deleted; we always want to rebuild this .o
# file so that it contains the correct date and time.  We need to do this here
# because the "-j 2" parallel build option can cause the delete to be done
# after the dependency check is done, which will make the link phase fail.
GetRidOfVersionBanner := $(shell rm -f VersionBanner.o)

# UNFINISHED - this needs to be calculated...for now, hardcode to 6M
PIGGYBACK_ADDRS = 0x80600000

# Conditionally make the map file.  Linking without a map file cuts a minute
# or more from the link time.
ifeq ($(BFC_MAKE_MAP_FILE), 1)
    LDFLAGS_BASIC += -Wl,-Map -Wl,Linux.map
endif


LDFLAGS_BASIC += -L$(LIBROOT)

ifeq ($(BCM_ESTB_STANDALONE_APP_SUPPORT), 1)
   LIBS +=  -lpthread -lstdc++ -lm -lsupc++
else
ifeq ($(ARM_PLATFORM), yes)
  LIBS +=   $(LIBROOT)/libpthread.a \
            $(LIBROOT)/libstdc++.a \
            $(LIBROOT)/libm.a \
            $(LIBROOT)/libsupc++.a \
            $(LIBROOT)/../../lib/gcc/arm-linux-uclibcgnueabi/3.4.6/libgcc.a    

else
  LIBS +=   $(LIBROOT)/libpthread.a \
            $(LIBROOT)/libstdc++.a \
            $(LIBROOT)/libm.a \
            $(LIBROOT)/libsupc++.a \
            $(LIBROOT)/../../lib/gcc/mipsel-linux-uclibc/3.4.6/libgcc.a    

endif
  LDFLAGS_BASIC += -static
endif


# The Linux image, without the symbol table.  The side effects of this target
# are the sorted, demangled map file (ecram.map.d), the compressed image for
# downloading to the target (ecram_sto.bin), and a piggyback image for boards
# with old bootloaders (ecram_pb.bin).
Linux : $(APP_PORTABLE_DIR)/BfcSystemConfig.mak $(ALL_OBJS)
	$(foreach f, $(ALL_OBJS), $(shell echo "INPUT($(f))" >>linkobjs.txt))
	@ echo "==== Linking the application ($@)..."
ifeq ($(BCM_OPTIMIZE_CACHE), 1)
	@ echo "==== Using Cache-optimized linker script."
endif
	$(CXX) $(LDFLAGS_BASIC) -o $(BFC_IMAGE_NAME) -Xlinker linkobjs.txt \
            -Wl,--start-group $(LIBS) -Wl,--end-group
	@ echo ==== Post-processing...
	@ $(RM) -f $(BFC_IMAGE_NAME).map $(BFC_IMAGE_NAME).map.d $(BFC_IMAGE_NAME).shortmap $(BFC_IMAGE_NAME).dasm
	$(STRIP) -s $(BFC_IMAGE_NAME) -o $(BFC_IMAGE_NAME)_sto
ifeq ($(BFC_MAKE_MAP_FILE), 1)
	@ mv Linux.map $(BFC_IMAGE_NAME).map
  ifeq ($(BFC_DEMANGLE_MAP_FILE), 1)
	$(DEMANGLE) <$(BFC_IMAGE_NAME).map >$(BFC_IMAGE_NAME).map.d && rm $(BFC_IMAGE_NAME).map
  endif
else
  ifeq ($(BFC_DEMANGLE_MAP_FILE), 1)
	$(NM) --demangle --numeric-sort  $(BFC_IMAGE_NAME) >$(BFC_IMAGE_NAME).shortmap
  else
	$(NM)            --numeric-sort  $(BFC_IMAGE_NAME) >$(BFC_IMAGE_NAME).shortmap
  endif
endif
ifeq ($(BFC_DISASSEMBLE_OUTPUT), 1)
	$(OBJDUMP) -d  $(BFC_IMAGE_NAME) >$(BFC_IMAGE_NAME).dasm
endif

ifeq ($(BFC_PIGGYBACK), 1)
	@ echo ==== Making piggyback binary...
	-@ cat $(BFC_BOOTLOADER_DIR)/LZMAdecomp.bin $(BFC_IMAGE_NAME)_sto.bin >$(BFC_IMAGE_NAME)_sto.pb
	- ProgramStore -c 0 -f $(BFC_IMAGE_NAME)_sto.pb -o $(BFC_IMAGE_NAME)_pb.bin \
                   -v $(PROGRAM_STORE_VERSION) -a $(PIGGYBACK_ADDRS) -s 0x$(PROGRAM_STORE_PID)
	-@ $(RM) $(BFC_IMAGE_NAME)_sto.pb
endif
	@ echo "*"
	@ echo "* Success!"
	@ echo "*"
	@ pwd
	@ ls -al $(BFC_IMAGE_NAME)*


##############################################################################
#
# Generic rules/recipies for creating a .o (compiled object file) from a
# source file.

# GNU Make considers .o files to be uninteresting intermediate files that
# need to be deleted after the build completes.  However, this equates to doing
# a clean build every time.  Tell GNU Make that .o is "precious" so that these
# files will be preserved unless the user does a clean build.
.PRECIOUS : %.o

COMPILING_STRING  = "==== Compiling $(<F)  (in $(subst ../,,$(<D)))"
ASSEMBLING_STRING = "==== Assembling $(<F)  (in $(subst ../,,$(<D)))"

ifeq ($(BCM_ECHO_MAKE_COMMANDS),1)
    ECHO =
else
    ECHO = @
endif

%.o : %.s
	@ echo $(ASSEMBLING_STRING)
	$(ECHO) $(CC) $(CFLAGS_AS) -c -o $@ $<

%.d %.o : %.c
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CC) $(CFLAGS) -c $<

%.s : %.c
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CC) $(CFLAGS) -S $<

# For a slim build, we need to run a custom tool that comments out the Message
# Logging code directly in the source, and then compile this processed file.
# This has to be done because the GNU compiler will keep the constant strings
# associated with the Message Logging code, even though they aren't being used,
# which causes the slim build not to be a slim as it should be.
ifeq ($(BCM_REDUCED_IMAGE_SIZE),1)

%.d %.o : %.cpp
	@ echo $(COMPILING_STRING)
	MessageLogZapper $< $(*F).cpp.bak.cpp
	$(ECHO) $(CXX) $(C++FLAGS) -c $(*F).cpp.bak.cpp -o $(*F).o
ifeq ($(BFC_MAKE_DEPENDENCY_FILES),1)
    # The dependency file lists x.cpp.bak.cpp, so get rid of the ".bak.cpp".
	@ sed -e "s/\\.bak\\.cpp//g" $(*F).d >$(*F).d.new
	@ mv $(*F).d.new $(*F).d
endif
	@ $(RM) $(*F).cpp.bak.cpp


%.d %.o : %.cxx
	@ echo $(COMPILING_STRING)
	MessageLogZapper $< $(*F).cxx.bak.cxx
	$(ECHO) $(CXX) $(C++FLAGS) -c $(*F).cxx.bak.cxx -o $(*F).o
ifeq ($(BFC_MAKE_DEPENDENCY_FILES),1)
    # The dependency file lists x.cxx.bak.cxx, so get rid of the ".bak.cxx".
	@ sed -e "s/\\.bak\\.cxx//g" $(*F).d >$(*F).d.new
	@ mv $(*F).d.new $(*F).d
endif
	@ $(RM) $(*F).cxx.bak.cxx

%.s : %.cpp
	@ echo $(COMPILING_STRING)
	MessageLogZapper $< $(@:.o=.cpp).bak.cpp
	$(ECHO) $(CXX) $(C++FLAGS) -S $(@:.o=.cpp).bak.cpp -o $@
	@ $(RM) $(@:.o=.cpp).bak.cpp

%.s : %.cxx
	@ echo $(COMPILING_STRING)
	MessageLogZapper $< $(@:.o=.cpp).bak.cpp
	$(ECHO) $(CXX) $(C++FLAGS) -S $(@:.o=.cpp).bak.cpp -o $@
	@ $(RM) $(@:.o=.cpp).bak.cpp

else

%.d %.o : %.cpp
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CXX) $(C++FLAGS) -c $<

%.d %.o : %.cxx
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CXX) $(C++FLAGS) -c $<

%.s : %.cpp
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CXX) $(C++FLAGS) -S $<

%.s : %.cxx
	@ echo $(COMPILING_STRING)
	$(ECHO) $(CXX) $(C++FLAGS) -S $<

endif


# TODO:  CHECK IF THIS IS NEEDED FOR LINUX
# For BcmHeapManagerMutexApiEcos.o, we need to exclude the function-sections
# option so that we can override the malloc/free/realloc functions from the
# kernel.
#BcmHeapManagerMutexApiEcos.o : BcmHeapManagerMutexApiEcos.c BcmHeapManagerMutexApi.h BcmHeapManager.h
#	@ echo $(COMPILING_STRING)
#	$(ECHO) $(CC) $(filter-out -ffunction-sections, $(CFLAGS)) -c $<


