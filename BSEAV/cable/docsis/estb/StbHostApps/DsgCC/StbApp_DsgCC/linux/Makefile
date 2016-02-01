#############################################################################
# (c) 2003-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

# TGT_DIR=$(WIND_BASE)/target
# include $(TGT_DIR)/h/make/defs.bsp
# include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
# include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

# All of the above make files are condensed into defs.bsp.
include $(BUILD_DIR)/../../StbApp_DsgCC/linux/makeinc/defs.bsp

# These definitions are used by the generic make include files to define
# the names of directories and files.  It would be better if all directories
# and file names were consistent.
TARGETOS     = Linux
LC_TARGETOS  = linux
OS_PREFIX    = Lnx
LC_OS_PREFIX = lnx

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

# Enable support for multiple interfaces on same subnet
ifeq ($(BCM_VENDOR_SUPPORT), 1)
  ADDED_CFLAGS += -DMULTI_INF_SAME_SUBNET=1
endif

ifeq ($(GCC_VERSION), 4)
  NPTL_SUPPORT = 1
endif

ifeq ($(NPTL_SUPPORT), 1)
  ADDED_CFLAGS += -DNPTL_SUPPORT=1
endif

ADDED_CFLAGS += -DTARGETOS_Linux

############################################################################
## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.
############################################################################

LINUX_STL_DIR     = $(BUILD_DIR)/../../../../Common/LinuxStl

# This is where any changes to the Linux header files reside.
# Include this path first in the full include path.
EXTRA_INCLUDE  = -I$(LINUX_STL_DIR) -I.

MAKE_INC_DIR   = $(BUILD_DIR)/../makeinc

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
ifeq ($(BCM_VENDOR_SUPPORT), 1)
  #include $(MAKE_INC_DIR)/docsisVendorV2components.inc
endif

###########################################################################
##  deps - this includes the definitions to build makefile.deps
############################################################################

include $(MAKE_INC_DIR)/deps.inc

###########################################################################
##  swdload - this includes the swdload components
############################################################################

include $(MAKE_INC_DIR)/estbswdload.inc

###########################################################################
##  dsg - this includes the dsg components
############################################################################

include $(MAKE_INC_DIR)/dsgcc.inc

###########################################################################
##  commondload - this includes the commondload components
############################################################################

include $(MAKE_INC_DIR)/commondload.inc

###########################################################################
##  canned snmp - this includes the canned snmp components
############################################################################

include $(MAKE_INC_DIR)/estbcannedsnmp.inc

###########################################################################
##  davic api - this includes the davic api components
############################################################################

include $(BUILD_DIR)/../../../Davic/davicapi.inc

###########################################################################
##  seb api - this includes the seb api components
############################################################################
include $(BUILD_DIR)/../../../Seb/sebapi.inc
###########################################################################
# MACH_EXTRA
###########################################################################
ifeq ($(DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM), 1)

EXCLUDED_FILES = BcmBasicString.o BcmSocket.o BfcAppCompositeNonVolCommandTable.o \
BfcAppCompositeNonVolSettings.o BfcApplication.o BitmaskCommandParameter.o \
BoolCommandParameter.o BuiltInCommandTable.o CallTraceCommandTable.o \
CmAppCompositeNonVolCommandTable.o CmAppCompositeNonVolSettings.o CmAppMain.o \
CmAppNonVolCommandTable.o CmAppNonVolSettings.o CmAppVersionInfo.o CmApplication.o \
CmTargetMain.o Command.o CommandParameter.o CommandParameterList.o  \
CompanyLogo.o CompletionHandlerACT.o CompositeNonVolSettings.o ConsoleThread.o Counter64.o \
CountingSemaphore.o \
EthernetPacket.o Event.o EventPublisher.o EventSet.o FlagCommandParameter.o \
FloatCommandParameter.o IpAddress.o IpAddressCommandParameter.o IpAddressList.o \
 IpV4Address.o IpV6Address.o LinuxConsoleCommands.o \
LnxCountingSemaphore.o LnxEvent.o LnxEventSet.o LnxInterface.o LnxMessageQueue.o \
LnxMutexSemaphore.o LnxOperatingSystem.o LnxSocket.o LnxTerminalConsoleThread.o \
LnxTimeout.o LnxTimer.o MacAddress.o MacAddressCommandParameter.o MainCommandTable.o \
MessageLog.o MessageLogSettings.o MessageQueue.o MsgLogSettingsCommandTable.o \
MutexSemaphore.o NonVolDevice.o NonVolSettings.o NonVolSettingsAccessLock.o NonVolSettingsCommandTable.o \
OctetBuffer.o OperatingSystem.o OperatingSystemFactory.o SignedIntCommandParameter.o \
SocketFactory.o SocketSet.o StringCommandParameter.o StringSetCommandParameter.o \
SystemTime.o TLV.o TerminalConsoleThread.o  Timer.o TimerMux.o TimerMuxACT.o \
UnrecognizedNonVolSettings.o UnsignedIntCommandParameter.o Utilities.o V2Main.o \
root.o stringfind.o

# CommandTable.o Thread.o

MACH_EXTRA = $(sort $(filter-out $(EXCLUDED_FILES), $(DSGCC_OBJS)))

else
MACH_EXTRA=$(TARGETDIR)/$(DSGCC_OBJS)
#MACH_EXTRA=$(DSGCC_OBJS)
endif


###########################################################################
##  DSGCC_BUILT_AS_LIB
############################################################################
ifeq ($(DSGCC_BUILT_AS_LIB), 1)

# A rule to create the DSG-CC library from object files.  It will
# cause the objects to be built if they don't exist.
$(TARGETDIR)/dsgcclib.a: $(MACH_EXTRA)
		$(AR) cru $(TARGETDIR)/dsgcclib.a $(MACH_EXTRA)
endif


###########################################################################
##  DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM
############################################################################
ifeq ($(DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM), 1)

# A rule to create the DSG-CC library as a module to add to Estb OpenCable
# System application. This library will exclude the files that are included
# in Estb OpenCable System build. It will
# cause the objects to be built if they don't exist.
dsgcc_estbsystem_lib.a: $(MACH_EXTRA)
		$(AR) cru dsgcc_estbsystem_lib.a $(MACH_EXTRA)
endif


###########################################################################
##  DSGCC_BUILT_AS_LIB
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
ADDED_C++FLAGS   += $(ADDED_CFLAGS) -Wno-deprecated

#########################################################################
## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.
#########################################################################

include $(BUILD_DIR)/../../StbApp_DsgCC/linux/makeinc/rules.bsp

