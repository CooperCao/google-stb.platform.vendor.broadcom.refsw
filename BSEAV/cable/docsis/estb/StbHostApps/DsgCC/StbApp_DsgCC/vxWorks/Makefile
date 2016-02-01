########################################################################
#
# Makefile - 
#
# modification history
# --------------------
# 7/1/2000 - JRM - original
#
# DESCRIPTION
# This file contains rules for building VxWorks 
#
#########################################################################

# Include Wind River make definition files.
TGT_DIR=$(WIND_BASE)/target
include $(TGT_DIR)/h/make/defs.bsp
ifeq ($(filter 22 221,$(TORNADO_VERSION)),)
include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)
endif

# These definitions are used by the generic make include files to define
# the names of directories and files.  It would be better if all directories
# and file names were consistent.
TARGETOS     = vxWorks
LC_TARGETOS  = vxworks
OS_PREFIX    = Vx
LC_OS_PREFIX = vx

ifeq ($(TORNADO_VERSION), 20)
  CC_COMPILER     = -B -ansi -pedantic -nostdinc
  CC_OPTIM_NORMAL = -O2 -fno-for-scope -fno-exceptions -funroll-loops 
  CC_OPTIM_TARGET = -O2 -fno-for-scope -fno-exceptions -funroll-loops 
else
  # T21 & T22: override the default -ansi in make.MIPS32sfgnu such that it will support // style comment in .c file
  CC_COMPILER   = -fno-builtin
endif

# Support for Tornado WDB Agent 
ifeq ($(TORNADO_DEBUGGING),1)
  ADDED_CFLAGS += -DTORNADO_DEBUGGING=1
endif

# Indicator that vxWorks symbol table included
ifeq ($(VXWORKS_SYMS_INCLUDED),1)
  ADDED_CFLAGS += -DVXWORKS_SYMS_INCLUDED=1
endif

# Support for making V2 image smaller 
# ( Removes much of the debug logging capability )
ifeq ($(BCM_REDUCED_IMAGE_SIZE),1)
  ADDED_CFLAGS += -DBCM_REDUCED_IMAGE_SIZE=1
endif

# Enable cache writeback mode
ifeq ($(CACHE_WRITEBACK_MODE),1)
  ADDED_CFLAGS += -DCACHE_WRITEBACK_MODE=1
endif

############################################################################
## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.
############################################################################
# This is where any changes to the vxWorks header files reside.
# Include this path first in the full include path.
TORNADO_PATCH_DIR = ../../../Common/vxWorks/patches/Tornado$(TORNADO_VERSION)
EXTRA_INCLUDE  = -I$(TORNADO_PATCH_DIR) -I.
ADDED_CFLAGS  += -DTORNADO_VERSION=$(TORNADO_VERSION)

ifeq ($(TORNADO_VERSION), 21)
  ifeq ($(VXWORKS_SYMS_INCLUDED), 1)
    #supress huge implicit function declaration warnings when compiling symTbl.c in vxWorks.st build
    ADDED_CFLAGS  += -Wno-implicit
  endif
endif
  
ifeq ($(TORNADO_VERSION), 22)
  # required for T22: default CPU_VARIANT for MIPS32
  # TODO - TPS - This is different from main stream.  
  # Some type of equality check should be done.
  CPU_VARIANT	  = _rc32xxx
endif

ifeq ($(TORNADO_VERSION), 221)
  ifeq ($(CPU), MIPS64)
    CPU_VARIANT = _mti5kx
  endif
endif

MAKE_INC_DIR   = ../../makeinc

###########################################################################
##  rootV2Components - includes the root file 
############################################################################

include $(MAKE_INC_DIR)/rootV2components.inc

###########################################################################
##  coreV2Components - includes only the necessary components needed to make 
##  a V2 based application
############################################################################

include $(MAKE_INC_DIR)/coreV2components.inc

###########################################################################
##  bspV2components - includes architecture-specific components needed to 
##  aid in building the V2 application
############################################################################

include $(MAKE_INC_DIR)/bspV2components.inc

###########################################################################
##  osV2Components - includes the operating system specifics needed to build
##  the V2 application
############################################################################

include $(MAKE_INC_DIR)/osV2support.inc

###########################################################################
##  addedV2Components - this includes the portions of V2 that you would like
##  to add to the coreV2 application
############################################################################

include $(MAKE_INC_DIR)/addedv2components.inc


###########################################################################
##  docsisV2components - this includes the DOCSIS-specific components needed
## in building a DOCSIS V2 application
############################################################################

# Check for Vendor-specific components 
#ifeq ($(BCM_VENDOR_SUPPORT), 1)
#  #include $(MAKE_INC_DIR)/docsisVendorV2components.inc
#endif

###########################################################################
##  deps - this includes the definitions to build makefile.deps
############################################################################

include $(MAKE_INC_DIR)/deps.inc

###########################################################################
##  dsg - this includes the dsg components
############################################################################

include $(MAKE_INC_DIR)/dsgcc.inc

include $(MAKE_INC_DIR)/dsgccbsp.inc
############################################################################
ROM_LDFLAGS       = -Map vxoutput.map
BOOT_EXTRA        = itab.o
ADDED_C++FLAGS   += $(ADDED_CFLAGS)
#LDFLAGS       	+= -Map vxoutput.map

############################################################################
##  Tornado support - Build DOCSIS into a library to be linked into 
##  a Tornado project.
##  Currently supports US version only.
############################################################################
#vxWorks_docsis_us.a:  \
#        $(MACH_EXTRA)
#		$(AR) cru $@ $(MACH_EXTRA)

#########################################################################
## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.
#########################################################################

ifeq ($(TORNADO_VERSION), 20)
  ifeq ($(CPU), VR5400)
    include $(BSP_OS_SRC)/makeinc/rules.bsp.$(CPU)
  else
    include $(BSP_OS_SRC)/makeinc/rules.bsp
  endif
else
  # T21 & T22:
  include $(BSP_OS_SRC)/makeinc/rules.bsp.t$(TORNADO_VERSION)
endif

# Add the vxWorks system include path
EXTRA_INCLUDE += -I$(TGT_DIR)/h

ifeq ($(filter 22 221,$(TORNADO_VERSION)),)
  include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)
endif
#include $(TGT_DIR)/h/make/rules.bsp
#------------------------------------------------------------------------
# The following definitions override settings in the vxWorks make files,
# so they must come after all "include" statements.
#------------------------------------------------------------------------

SAVE_CFLAGS := $(ADDED_CFLAGS)
ADDED_CFLAGS = -include defines.h

defines.h :
	@ echo ==== Making defines.h
	@ echo $(SAVE_CFLAGS)
	@ echo $(SAVE_CFLAGS) > defines.tmp
	@ sed -f ../defines.sed defines.tmp | sort >defines.h
	@ $(RM) defines.tmp



