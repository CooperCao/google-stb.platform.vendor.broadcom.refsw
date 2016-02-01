#**@@@+++@@@@*********************************************************
#*
#* Microsoft PlayReady
#* Copyright (c) Microsoft Corporation. All rights reserved.
#*
#**@@@---@@@@********************************************************/

#*********************************************************************
#
#
# Description:
#    Defines the necessary environment variables
#
# Outputs:
#   - DRM_TARGET_TYPE: FREE, CHECKED, PERF
#   - DRM_BUILD_ARCH: X86, X64, ARM, ...
#   - DRM_BUILD_PLATFORM: ANSI, ...
#   - DRM_ROOT_PATH: <some path, with trailing slash>
#   - _DRM_BUILD_TOOL: NMAKE, NTBUILD, ...
#   - _DRM_BUILD_PLATARCH: ANSI_X86, ...
#   - _DRM_BUILD_PATH: <off of DRM_ROOT_PATH, with trailing slash>
#   - Forward declared targets: DRM_BUILD_TARGET, DRM_CLEAN_TARGET
#   - For _DRM_BUILD_TOOL = NMAKE
#       - all: target rule (dependent on DRM_BUILD_TARGET)
#       - clean: target rule (dependent on DRM_CLEAN_TARGET)
#
#********************************************************************/

##############################################################################
#                                                                            #
#  Pick up build definitions from the shell environment                      #
#  ----------------------------------------------------                      #
#                                                                            #
##############################################################################

#------------------------------------
# Determine the build tool being used
#------------------------------------

!if defined(_DRM_BUILD_TOOL)
#
# The user has provided an override for the tool. This may occur when the user is calling nmake from within a build.exe-invoked script
#
!elseif defined(MAKEDIR_RELATIVE_TO_BASEDIR) || defined(_PROJECT_MK_PATH) || "$(_BUILDARCH)"=="ppc"
_DRM_BUILD_TOOL=NTBUILD
!else
_DRM_BUILD_TOOL=NMAKE
!endif

#==============================================================================
# Derive settings appropriately when NTBUILD (Build.exe) is in use
#==============================================================================

!if "$(_DRM_BUILD_TOOL)" == "NTBUILD"

#-------------------------------------
# Define build platform
#-------------------------------------
DRM_BUILD_PLATFORM=WIN32

#-------------------------------------
# Define build architecture
#-------------------------------------
!if "$(_BUILDARCH)" == "x86"
DRM_BUILD_ARCH=X86
#-------------------------------
!elseif "$(_BUILDARCH)" == "amd64"
DRM_BUILD_ARCH=X64
#-------------------------------
!elseif "$(_BUILDARCH)" == "arm"
DRM_BUILD_ARCH=ARM
#-------------------------------
!elseif "$(_BUILDARCH)" == "ppc"
DRM_BUILD_ARCH=XBOX
DRM_BUILD_PLATFORM=XBOX
DRM_BUILD_PROFILE=XBOX
!undef MAKEDIR_RELATIVE_TO_BASEDIR
#-------------------------------
# We are temporarily hijacking the ia64 processor type for PPC.
!elseif "$(_BUILDARCH)" == "ia64"
# By default CoreXT's sources.all adds a bunch of things to TARGETLIBS that don't exists on XBox, disable this.
# It would make more sense to define this in drmbuild.cmn but it needs to happen before makefile.def gets included
# and drmbuild.cmn is included after makefile.def
COREXT_NO_DEFAULT_LIBS=1

DRM_BUILD_ARCH=XBOX
DRM_BUILD_PLATFORM=XBOX
DRM_BUILD_PROFILE=XBOX
!undef MAKEDIR_RELATIVE_TO_BASEDIR
!undef _WIN64

!else

!error "Unknown _BUILDARCH: $(_BUILDARCH)"
!endif
#-------------------------------

#------------------------------------------------------------------------------
# Define build type( FREE/CHECKED/PERF ) if not already defined
#------------------------------------------------------------------------------

!if !defined(BUILD_TYPE)
!if ("$(NTDEBUG)" == "") || ("$(NTDEBUG)" == "ntsdnodbg")
BUILD_TYPE=fre
!else
BUILD_TYPE=chk
!endif
!endif

#-------------------------------
!if "$(BUILD_TYPE)" == "fre"
DRM_BUILD_TYPE=FREE
#-------------------------------
!elseif "$(BUILD_TYPE)" == "chk"
DRM_BUILD_TYPE=CHECKED
!endif
#-------------------------------

# if the build type is free, check if perf support is enabled

!if "$(DRM_SUPPORT_PROFILING)" == "1" && "$(DRM_BUILD_TYPE)" == "FREE"
DRM_BUILD_TYPE = PERF
!endif

#-------------------------------------
# Derive source paths from BASEDIR when using build.
# These need to be defined when not using build environment.
#-------------------------------------
DRM_ROOT_PATH=$(BASEDIR)\source\pk\WMDRM-MD^\

#-------------------------------------
# Build.exe has a phased build system to enable multiproc builds
# with a minimum of explicit dependencies
# - Pass0:  Walk build tree creating headers
# - Pass1: Walk build tree creating objs & libs, *sometimes* broken into
#     Pass1a: create objs
#     Pass1b: create libs
# - Pass2:  Walk build tree creating exe's and dll's
#-------------------------------------
!if "$(BUILD_PASS)" == "PASS0" || "$(BUILD_PASS)" == "PASSALL"
_DRM_BUILD_OBJECT_TARGETS=0
_DRM_BUILD_LIBRARY_TARGETS=0
_DRM_BUILD_BINARY_TARGETS=0
_DRM_BUILD_POST_PROCESS_TARGET=0
!elseif "$(BUILD_PASS)" == "PASS1" && "$(PASS1_NOLIB)" == "1" # Pass1a
_DRM_BUILD_OBJECT_TARGETS=1
_DRM_BUILD_LIBRARY_TARGETS=0
_DRM_BUILD_BINARY_TARGETS=0
_DRM_BUILD_POST_PROCESS_TARGET=0
!elseif "$(BUILD_PASS)" == "PASS1" # Pass1b or merged Pass1a & Pass1b
_DRM_BUILD_OBJECT_TARGETS=1
_DRM_BUILD_LIBRARY_TARGETS=1
_DRM_BUILD_BINARY_TARGETS=0
_DRM_BUILD_POST_PROCESS_TARGET=1
!elseif "$(BUILD_PASS)" == "PASS2"
_DRM_BUILD_OBJECT_TARGETS=0
_DRM_BUILD_LIBRARY_TARGETS=0
_DRM_BUILD_BINARY_TARGETS=1
_DRM_BUILD_POST_PROCESS_TARGET=1
!else
!    error "Unknown build pass: $(BUILD_PASS)"
!endif



#-------------------------------------
# If BASEDIR is set, this is an MS internal compilation.
# In that case, the build profile should default to PC.
#-------------------------------------
!if "$(DRM_BUILD_PROFILE)" == "" && "$(BASEDIR)" != ""

!IF "$(PLAYREADY_PRODUCT_OVERRIDE)" == "WP8"
DRM_BUILD_PROFILE=WP8
!else
DRM_BUILD_PROFILE=PC
!endif

!endif



#==============================================================================
# Derive settings appropriately when Microsoft NMAKE is in use
#==============================================================================

!elseif  "$(_DRM_BUILD_TOOL)" == "NMAKE"

#-------------------------------------
# Following environment variables ought to be predefined
#       DRM_BUILD_ARCH
#       DRM_BUILD_TYPE
#       DRM_ROOT_PATH
#       DRM_BUILD_PLATFORM
#-------------------------------------
DRM_OBJ_ROOT_PATH=$(DRM_ROOT_PATH)


#-------------------------------------
# Since nmake is single-threaded (and declares all dependencies)
# build everything in the same pass.
#-------------------------------------
_DRM_BUILD_LIBRARY_TARGETS=1
_DRM_BUILD_BINARY_TARGETS=1
_DRM_BUILD_POST_PROCESS_TARGET=1


##############################################################################
#                                                                            #
#  Declare targets for build NMAKE and NTBUILD                               #
#  -------------------------------------------                               #
#                                                                            #
##############################################################################

#---------------------------------------------------------------
# Since all: is declared first, it becomes the default for nmake
#---------------------------------------------------------------
all: DRM_BUILD_TARGET
clean: DRM_CLEAN_TARGET

##############################################################################
#                                                                            #
#  Validate resulting build parameters                                       #
#  -----------------------------------                                       #
#                                                                            #
##############################################################################

#-------------------------------------
# Verify if DRM_BUILD_ARCH is defined or not
#-------------------------------------
!if !defined(DRM_BUILD_ARCH)
!    message +--------------------------------------------------------
!    message | The following variables need to be defined:
!    message |     DRM_BUILD_ARCH - Either X86, X64, or ARM
!    message |     DRM_BUILD_PLATFORM - ANSI
!    message |     DRM_BUILD_TYPE - Either FREE, CHECKED, or PERF
!    message |     DRM_ROOT_PATH - This must end in a trailing slash.
!    message |                     (e.g. C:\PlayReady\Device_PK_2.0.0\)
!    message |     DRM_BUILD_PROFILE - OEM
!    message +--------------------------------------------------------
!    error DRM_BUILD_ARCH not defined.
!elseif ("$(DRM_BUILD_ARCH)" != "X86" && \
         "$(DRM_BUILD_ARCH)" != "X64" && \
         "$(DRM_BUILD_ARCH)" != "ARM")
!    error DRM_BUILD_ARCH=$(DRM_BUILD_ARCH), must be either X86, X64, or ARM
!endif

#-------------------------------------
# Verify if DRM_BUILD_PLATFORM is defined or not
#-------------------------------------
!if !defined(DRM_BUILD_PLATFORM)
!    message +--------------------------------------------------------
!    message | The following variables need to be defined:
!    message |     DRM_BUILD_ARCH - Either X86, X64, or ARM
!    message |     DRM_BUILD_PLATFORM - ANSI
!    message |     DRM_BUILD_TYPE - Either FREE, CHECKED, or PERF
!    message |     DRM_ROOT_PATH - This must end in a trailing slash.
!    message |                     (e.g. C:\PlayReady\Device_PK_2.0.0\)
!    message |     DRM_BUILD_PROFILE - OEM
!    message +--------------------------------------------------------
!    error DRM_BUILD_PLATFORM not defined.
!elseif ("$(DRM_BUILD_PLATFORM)" != "ANSI" && \
         "$(DRM_BUILD_PLATFORM)" != "WIN32" && \
         "$(DRM_BUILD_PLATFORM)" != "WMSP60")
!    error DRM_BUILD_PLATFORM=$(DRM_BUILD_PLATFORM), must be ANSI
!endif

#-------------------------------------
# Verify if DRM_BUILD_TYPE is defined or not
#-------------------------------------

!if !defined(DRM_BUILD_TYPE)
!    message +--------------------------------------------------------
!    message | The following variables need to be defined:
!    message |     DRM_BUILD_ARCH - Either X86, X64, or ARM
!    message |     DRM_BUILD_PLATFORM - ANSI
!    message |     DRM_BUILD_TYPE - Either FREE, CHECKED, or PERF
!    message |     DRM_ROOT_PATH - This must end in a trailing slash.
!    message |                     (e.g. C:\PlayReady\Device_PK_2.0.0\)
!    message |     DRM_BUILD_PROFILE - OEM
!    message +--------------------------------------------------------
!    error DRM_BUILD_TYPE not defined.
!elseif ("$(DRM_BUILD_TYPE)" != "FREE" && \
         "$(DRM_BUILD_TYPE)" != "CHECKED" && \
         "$(DRM_BUILD_TYPE)" != "PERF")
!    error DRM_BUILD_TYPE=$(DRM_BUILD_TYPE), must be either FREE, CHECKED, or PERF
!endif


#-------------------------------------
# Verify if DRM_ROOT_PATH is defined or not
#-------------------------------------
!if !defined(DRM_ROOT_PATH)
!    message +-----------------------------------------------------------------------
!    message | Set DRM_ROOT_PATH to the directory where the porting kit is installed.
!    message | Example: C:\PlayReady\Device_PK_2.0.0\\

# The above blank line is needed to properly display the slash above.
!    message |
!    message | Note the trailing slash as well!
!    message +-----------------------------------------------------------------------
!    error DRM_ROOT_PATH not defined.
!endif

#-------------------------------------
# Verify if DRM_BUILD_PROFILE is defined or not
#-------------------------------------
!if !defined(DRM_BUILD_PROFILE)
!    message +--------------------------------------------------------
!    message | The following variables need to be defined:
!    message |     DRM_BUILD_ARCH - Either X86, X64, or ARM
!    message |     DRM_BUILD_PLATFORM - ANSI
!    message |     DRM_BUILD_TYPE - Either FREE, CHECKED, or PERF
!    message |     DRM_ROOT_PATH - This must end in a trailing slash.
!    message |                     (e.g. C:\PlayReady\Device_PK_2.0.0\)
!    message |     DRM_BUILD_PROFILE - OEM
!    message +--------------------------------------------------------
!    error DRM_BUILD_PROFILE not defined.
!elseif ("$(DRM_BUILD_PROFILE)" != "PK" && \
         "$(DRM_BUILD_PROFILE)" != "OEM" && \
         "$(DRM_BUILD_PROFILE)" != "OEMPLAYONLY" && \
         "$(DRM_BUILD_PROFILE)" != "WM7" && \
         "$(DRM_BUILD_PROFILE)" != "PK_NO_WMDRM" && \
         "$(DRM_BUILD_PROFILE)" != "PK_NO_WMDRM_NO_BBXLEGACY" && \
         "$(DRM_BUILD_PROFILE)" != "PK_YES_SECUREHAL_NO_PLAYONLY" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_1" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_2" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_3" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_4" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_5" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_6" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_7" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_8" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_9" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_10" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_11" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_12" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_13" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_14" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_15" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_16" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_17" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_18" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_19" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_20" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_21" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_22" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_23" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_24" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_25" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_26" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_27" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_28" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_29" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_30" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_31" && \
         "$(DRM_BUILD_PROFILE)" != "PK_TESTFLAGSET_32" )
!    error DRM_BUILD_PROFILE=$(DRM_BUILD_PROFILE), must be OEM
!endif

!else

error !!BUILD TOOL NOT DEFINED!!

!endif



##############################################################################
#                                                                            #
#  Derive variables based on above parameters                                #
#  ------------------------------------------                                #
#                                                                            #
##############################################################################

#-------------------------------------
# Derive DRM_BUILD_PATH from DRM_ROOT_PATH
#-------------------------------------
DRM_BUILD_PATH=$(DRM_ROOT_PATH)build^\


#-------------------------------------
# Derive DRM_BUILD_PLATARCH
#-------------------------------------

DRM_BUILD_PLATARCH=$(DRM_BUILD_PLATFORM)_$(DRM_BUILD_ARCH)
