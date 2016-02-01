##############################################################################
#
#  Copyright (c) 2003  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16215 Alton Parkway
#          Irvine, California 92618
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
##############################################################################
#  $Id: BfcWin32.mak 1.21 2006/09/22 13:43:23Z dpullen Release $
#
#  Filename:       BfcWin32.mak
#  Author:         David Pullen
#  Creation Date:  Sept 5, 2003
#
##############################################################################
#  Description:
#
#	Makefile for the GNU make utility, which has specific rules for building
#	BFC systems for Win32 (command-line, not GUI or MFC).
#
##############################################################################


##############################################################################
#
# Set up relative paths as macros; this makes it easier to relocate things as
# needed, and also makes it easier to read the rest of the stuff.

BFC_MAKE_DIR = $(BFC_ROOT_DIR)/Bfc/make

BFC_SGISTL_DIR = Bfc/SgiStl
BFC_WIN32_DIR = Bfc/Win32

# UNFINISHED - I'm not sure if I want this here or not...
BFC_TARGET_DIR = Bfc/Target


##############################################################################
#
# Set up the variables that are used as external constants by the component
# makefiles.

BFC_TARGET_OS = Win32
BFC_TARGET_PROCESSOR = Intel


##############################################################################
#
# Tell GNU Make about the file extensions that it needs to pay attention to.
# I have to clear the default list first.

.SUFFIXES :
.SUFFIXES : .cpp .cxx .obj .h .c .lib .exe


##############################################################################
#
# Macros for the various tools we use, and their options.

CPP = cl.exe
CPP_OPTS = /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" \
           /D "_MBCS" /D PROCESSOR_TYPE=INTEL /Fo".\\" /Fd".\\" /FD /GZ /c \
	   /FItypedefs.h

LINK32 = link.exe
LINK32_FLAGS = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
	odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
        wsock32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
	odbccp32.lib winmm.lib /nologo /subsystem:console /incremental:yes \
	/debug /machine:I386 /pdbtype:sept 


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
	-@ erase /Q *

CLEAN : clean


##############################################################################
#
# Include the relevant component makefiles, adding them to the list of files,
# directories, options, etc., as appropriate for the system configuration.


# Get the BFC system configuration file.  This is specific to each
# application/system, and comes from the app's portable directory.
include $(APP_PORTABLE_DIR)/BfcSystemConfig.mak


# BFC global makefile.  This has all the BFC component makefiles together.
include $(BFC_MAKE_DIR)/Bfc.mak


# Finally, include the application/system specific makefile.
include $(APP_PORTABLE_DIR)/BfcSystem.mak


# Because this is the Win32 host target, we need to add a few odds and ends to
# the macros.
BFC_OBJECTS += Win32BfcMain.o BfcWin32OsApplicationFactory.o
BFC_INCLUDE_DIRS += $(BFC_WIN32_DIR) $(BFC_SGISTL_DIR) $(BFC_TARGET_DIR)
BFC_SOURCE_DIRS += $(BFC_WIN32_DIR) $(BFC_SGISTL_DIR) $(BFC_TARGET_DIR)


# Because this is the Win32 host target, the object file extension needs to be
# translated from .o to .obj.  Also, move VersionBanner.o to the end of the
# list so that it is built last.
ALL_OBJS = $(patsubst %.o,%.obj, $(sort $(filter-out VersionBanner.o, $(BFC_OBJECTS))) VersionBanner.o)


# Each of the compiler defines need to be prepended with /D.
CPP_DEFINES = $(addprefix /D , $(BFC_COMPILER_DEFINES)) /D BFC=1


# Now that we have the full list of include dirs, convert them so that they are
# relative to the current directory, then prepend the "/I " flag in front of
# them for use by the compiler.
CPP_INCLUDES = $(addprefix /I $(BFC_ROOT_DIR)/, $(sort $(BFC_INCLUDE_DIRS))) \
               /I $(APP_WIN32_OUTPUT_DIR) /I $(APP_WIN32_DIR)


# Use the source dirs as the VPATH so make knows where to find things.  We have
# to make the path relative to the current directory, and we also need to add
# the app-specific directories to it.
VPATH = $(addprefix $(BFC_ROOT_DIR)/, $(sort $(BFC_SOURCE_DIRS))) \
        $(APP_WIN32_OUTPUT_DIR) $(APP_WIN32_DIR)


##############################################################################
#
# Rule to build the executable from the list of intermediate object files.
# The dependencies are set up so we create username.h if needed, and we always
# erase the version banner object module (so that the build date/time is
# correct).
#
# We have to write the linker options to external files because they can be
# very long, exceeding the max command line length.  We even have to take care
# when writing the .obj files out, because they can exceed our ability to use
# echo and pipe them to a file.  For them, we simply iterate over the list and
# echo them one at a time to linkobjs.txt.  Note that we have to delete the
# file first so that the append (>>) will work without duplicating data in it.

GetRidOfLinkObjs := $(shell if exist linkobjs.txt erase /Q linkobjs.txt)

%.exe : $(APP_WIN32_OUTPUT_DIR)/username.h CleanVersionBanner $(ALL_OBJS) \
        $(APP_PORTABLE_DIR)/BfcSystemConfig.mak
	@ echo ==== Linking the application ($@)...
	@ echo $(LINK32_FLAGS) >linkflags.txt
	$(foreach f,$(ALL_OBJS),$(shell echo $(f) >>linkobjs.txt))
	@ $(LINK32) /nologo @linkflags.txt /pdb:"$*.pdb" /out:"$@" @linkobjs.txt
	@ echo *
	@ echo * Success!
	@ echo *


##############################################################################
#
# Generic rules/recipies for creating a .obj (compiled object file) from a
# source file.


# GNU Make considers .obj files to be uninteresting intermediate files that
# need to be deleted after the build completes.  However, this equates to doing
# a clean build every time.  Tell GNU Make that .obj is "precious" so that these
# files will be preserved unless the user does a clean build.
.PRECIOUS : %.obj

COMPILING_STRING = ==== Compiling $(<F)  (in $(subst ../,,$(<D)))

%.obj : %.c
	@ echo $(COMPILING_STRING)
	@ $(CPP) $(CPP_OPTS) $(CPP_DEFINES) $(CPP_INCLUDES) $<

%.obj : %.cpp
	@ echo $(COMPILING_STRING)
	@ $(CPP) $(CPP_OPTS) $(CPP_DEFINES) $(CPP_INCLUDES) $<

%.obj : %.cxx
	@ echo $(COMPILING_STRING)
	@ $(CPP) $(CPP_OPTS) $(CPP_DEFINES) $(CPP_INCLUDES) $<


##############################################################################
#
# Custom build step for creating username.h; this is used as part of the
# version banner to show who built the image, giving some amount of
# traceability for the origin of a particular image.

$(APP_WIN32_OUTPUT_DIR)/username.h :
	@ echo ==== Creating username.h...
	@ echo #define USERNAME "$(USERNAME)" >$(APP_WIN32_OUTPUT_DIR)/username.h

##############################################################################
#
# Custom build step for VersionBanner.obj; we always want this file to be
# built, since it contains the date and time of the build.  This custom
# dependency chain causes VersionBanner.obj to be deleted so that a rebuild
# will be forced.

CleanVersionBanner :
	@ echo ==== Removing VersionBanner.obj so that it will be rebuilt...
	-@ if exist VersionBanner.obj erase /Q VersionBanner.obj


