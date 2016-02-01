##############################################################################
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
##############################################################################

########################################################################
#
# Makefile - 
#
# modification history
# --------------------
# 7/1/2000 - JRM - original
# 5/9/2002 - Mike Sieweke - modified for Linux
#
# DESCRIPTION
# This file contains rules for building Linux
#
#########################################################################

# These definitions are used by the generic make include files to define
# the names of directories and files.  It would be better if all directories
# and file names were consistent.
TARGETOS     = Linux
LC_TARGETOS  = linux
OS_PREFIX    = Lnx
LC_OS_PREFIX = lnx

# All of the above make files are condensed into defs.bsp.
include ../../../EstbApp_SnmpAgent/$(LC_TARGETOS)/makeinc/defs.bsp

# If DSG is supported, vendor is required, so make sure it's on.
ifeq ($(BCM_DSG_SUPPORT), 1)
  BCM_VENDOR_SUPPORT=1
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

# Duplicate, already existed in osV2support.inc
# Enable support for multiple interfaces on same subnet
#ifeq ($(BCM_VENDOR_SUPPORT), 1)
#  ADDED_CFLAGS += -DMULTI_INF_SAME_SUBNET=1
#endif

ifeq ($(GCC_VERSION), 4)
  NPTL_SUPPORT = 1
endif

ifeq ($(NPTL_SUPPORT), 1)
  ADDED_CFLAGS += -DNPTL_SUPPORT=1
endif


ADDED_CFLAGS += -DTARGETOS_Linux
ADDED_C++FLAGS += -Wno-deprecated

############################################################################
## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.
############################################################################

LINUX_STL_DIR     = ../../../Common/LinuxStl

# This is where any changes to the Linux header files reside.
# Include this path first in the full include path.
EXTRA_INCLUDE  = -I$(LINUX_STL_DIR) -I.

MAKE_INC_DIR   = ../../makeinc

###########################################################################
##  rootV2Components - includes the root file 
############################################################################

ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)
include $(MAKE_INC_DIR)/rootV2components.inc
endif
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
##  addedV2Components - this includes the portions of V2 that you would like
##  to add to the coreV2 application
############################################################################
ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)
include $(MAKE_INC_DIR)/addedv2components.inc
endif
###########################################################################
##  snmpV2Components - this includes the portions of V2 that you would like
##  to add to the coreV2 application
############################################################################

include $(MAKE_INC_DIR)/snmpV2components.inc

###########################################################################
##  estbV2 - this includes the eSTB settop components
############################################################################

  include $(MAKE_INC_DIR)/estbV2.inc

###########################################################################
##  docsisV2components - this includes the DOCSIS-specific components needed
## in building a DOCSIS V2 application
############################################################################
ifeq ($(CM_HAL_INCLUDED), 1)

  include $(MAKE_INC_DIR)/docsisV2components.inc

  ##  Propane - this includes the Propane components, which is associated with
  ##  DOCSIS.
  ifeq ($(PROPANE_INCLUDED), 1)
    include $(MAKE_INC_DIR)/propane.inc
  endif

else
  # include $(MAKE_INC_DIR)/docsisstubV2components.inc
endif

# Check for Vendor-specific components 
ifeq ($(BCM_VENDOR_SUPPORT), 1)
  include $(MAKE_INC_DIR)/docsisVendorV2components.inc
endif

###########################################################################
##  ipSecV2 - this includes the IP security component specifics
############################################################################

ifeq ($(BCM_IPSEC_SUPPORT), 1)
  include $(MAKE_INC_DIR)/ipsecV2.inc
endif
ifeq ($(BCM_IPSEC_SUPPORT), 2)
  include $(MAKE_INC_DIR)/ipsecV2.inc
endif
 
###########################################################################
##  deps - this includes the definitions to build makefile.deps
############################################################################

include $(MAKE_INC_DIR)/deps.inc

###########################################################################
##  dsg - this includes the dsg components
############################################################################

ifeq ($(BCM_DSG_SUPPORT), 1)
  include $(MAKE_INC_DIR)/dsg.inc
endif

###########################################################################
##  ESTB OPENCABLE HOST APP - EVENT DISPATCHER SUPPORT
############################################################################
ifeq ($(BCM_EVD_INTERFACE_INCLUDED), 1)
  include ../../../EventDispatcherIf/EventDispatcherIf.mak
endif

###########################################################################
##  ESTBSNMP_BUILT_AS_LIB
############################################################################
ifeq ($(ESTBSNMP_BUILT_AS_LIB), 1)

# A rule to create the eSTB SNMP Agent library from object files.  It will
# cause the objects to be built if they don't exist.
estbsnmplib.a: $(MACH_EXTRA)
	$(AR) cru estbsnmplib.a $(MACH_EXTRA)
endif

###########################################################################
##  CORE_ESTBSNMP_BUILT_AS_LIB
############################################################################
ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 1)

# A rule to create the eSTB SNMP Agent library from object files.  It will
# cause the objects to be built if they don't exist.
core_estbsnmplib.a: $(MACH_EXTRA)
	$(AR) cru core_estbsnmplib.a $(MACH_EXTRA)
endif

###########################################################################
##  ESTBSNMP_BUILT_AS_LIB
############################################################################


## klmod added:
ifeq ($(GDB_DEBUGGING), 0)
  ## Omit all symbol info from the output file. Add map file
  LDFLAGS      += -s
else
  LDFLAGS      += -Wl,-Map -Wl,lxoutput.map
  CC_OPTIM_NORMAL += -g -ggdb
  CC_OPTIM_TARGET += -g -ggdb
endif

############################################################################
ROM_LDFLAGS       = -Map lxoutput.map
BOOT_EXTRA        = itab.o
ADDED_C++FLAGS   += $(ADDED_CFLAGS)

#########################################################################
## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.
#########################################################################

include ../../../EstbApp_SnmpAgent/$(LC_TARGETOS)/makeinc/rules.bsp

