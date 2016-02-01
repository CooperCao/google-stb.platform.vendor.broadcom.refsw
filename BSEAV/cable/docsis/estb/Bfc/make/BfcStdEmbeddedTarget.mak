##############################################################################
#  
#  Copyright (c) 2003-2008 Broadcom Corporation
# 
#  This program is the proprietary software of Broadcom Corporation and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#  SOFTWARE.  
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
# 
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
##############################################################################
#  $Id$
#
#  Filename:       BfcStdEmbeddedTarget.mak
#  Author:         David Pullen
#  Creation Date:  October 28, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the Standard Embedded Target
#       files.  It adds files and settings that are used by
#       BfcStdEmbeddedTarget.cpp (flash driver, nonvol, etc)
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.
#
#	BFC_INCLUDE_DIRS - list of directories where header files can be found;
#			   this will be passed to the compiler.
#
#	BFC_SOURCE_DIRS - list of directories where source files can be found;
#			  this is used by the Make utility.
#
#	BFC_LIBRARIES - list of libraries that are to be used when linking the
#			application.
#
# External Constants (input only; do not modify these):
#
#	BFC_TARGET_OS - the OS that the project is being built for (i.e. Win32,
#			VxWorks, Ecos).  This can be used when making decisions
#			about which files/directories/libararies to include.
#
#	BFC_TARGET_PROCESSOR - the processor that the project is being built
#			       for (i.e. Intel, MIPS32, etc).  This can be used
#			       when making decisions about which
#			       files/directories/libararies to include.
#
#	BFC_ROOT_DIR - the "mount point" for the source code, relative to the
#		       output directory for the project.  This is defined in
#		       the top-level makefile for the OS/target.
#
#	BFC_MAKE_DIR - this is essentially $(BFC_ROOT_DIR)/Bfc/make, and is
#		       the directory where the core BFC component makefiles
#		       are located.
#
##############
#
# NOTE:  This file is geared toward the GNU Make utility, and may use any
#        syntax that is valid for recent-vintage GNU Make.
#
##############################################################################

# set this to 1 to get a test thread that can be used to 
# simulate applications that are CPU intensive. 
# use the console command waste_time to kick the thread off. 
BFC_INCLUDE_TIME_WASTER_THREAD = 0

##############################################################################
#
# Includes - include other makefiles here (if necessary).  You must specify
#	     the full path to the makefile, using either BFC_ROOT_DIR or
#	     BFC_MAKE_DIR as appropriate.

#include what is needed for L2TPV3
ifeq ($(BCM_L2TPV3_SUPPORT), 1)
    BFC_L2TPV3_LIB = Bfc/LibSupport/L2tpv3/L2tpV3.a
    -include $(BFC_MAKE_DIR)/BfcL2tpV3lib.mak
endif


##############################################################################
#
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.  You can specify multiple directories here
#	  if necessary, but you should have one variable per directory.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.  For
#	  example, if the Bfc directory is in c:\source\projects\bfcprojects\Bfc
#	  then c:\source\projects\bfcprojects is the mount point.  You will only
#	  specify the path beyond this point (i.e. Bfc).

BFC_STD_EMBEDDED_TARGET_DIR = Bfc/Target \
	cm_bsp_v2/bsp_common/BoardHal

# PR7327 - the new flash driver files are in this directory.
BFC_STD_EMBEDDED_TARGET_DIR += Bfc/Target/FlashDriver

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

BFC_STD_EMBEDDED_TARGET_OBJECTS = \
        BoardHalInfo.o          BoardHalInit.o      \
        BoardHalMode.o          InitBoard.o         \
        interrupt.o             PeriodicTimer.o     \
        timerUtils.o            Watchdog.o          \
        BfcStdEmbeddedTarget.o  \
        crc.o			

# PR12211
BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS = Ejtag.o EjtagCommandTable.o

ifeq ($(BCM93380_DS_BONDING), 1)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += BondedHal.o
endif

ifeq ($(buildProcessor), 5365)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += SiliconBackplaneLib.o PciLib.o
endif

ifeq ($(buildProcessor), 5352)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += SiliconBackplaneLib.o 
endif

ifeq ($(buildProcessor), 3348)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += hvgMonitor.o SpiDriver.o

    # PR12224
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PerformanceCounter.o
endif

ifeq ($(buildProcessor), 3368)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PciLib.o Bfc3368EmbeddedTarget.o SpiDriver.o

    # PR12224
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PerformanceCounter.o

    # PR12211
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += \
	Bmips4350Ejtag.o Bmips4350EcosDebugException.o
endif

ifeq ($(buildProcessor), 3361)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PciLib.o Bfc3368EmbeddedTarget.o SpiDriver.o

    # PR12224
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PerformanceCounter.o

    # PR12211
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += \
	Bmips4350Ejtag.o Bmips4350EcosDebugException.o
endif

ifeq ($(buildProcessor), 3378)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += Bfc3368EmbeddedTarget.o SpiDriver.o PerformanceCounter.o
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += Bmips4350Ejtag.o Bmips4350EcosDebugException.o
endif

ifeq ($(buildProcessor), 3379)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += Bfc3368EmbeddedTarget.o SpiDriver.o PerformanceCounter.o PciLib.o
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += Bmips4350Ejtag.o Bmips4350EcosDebugException.o 
endif

ifeq ($(buildProcessor), 3373)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += Bfc3368EmbeddedTarget.o SpiDriver.o PerformanceCounter.o PciLib.o
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += Bmips4350Ejtag.o Bmips4350EcosDebugException.o 
endif

ifeq ($(buildProcessor), 3255)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PciLib.o Bfc3368EmbeddedTarget.o SpiDriver.o Mem2MemDmaDriver.o

    # PR12224
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PerformanceCounter.o

    # PR12211
    BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS += \
	Bmips4350Ejtag.o Bmips4350EcosDebugException.o
endif

ifeq ($(buildProcessor), 6348)
    BFC_STD_EMBEDDED_TARGET_OBJECTS += PciLib.o
endif

BFC_STD_EMBEDDED_TARGET_NONVOL_OBJECTS = NonVolDeviceDriver.o \
                                         NonVolDeviceDriverBridge.o

BFC_STD_EMBEDDED_TARGET_PROGRAM_STORE_OBJECTS = ProgramStoreDeviceDriver.o \
                                                ProgramStoreDeviceDriverBridge.o

BFC_STD_EMBEDDED_TARGET_BOOTLOADER_STORE_OBJECTS = BootloaderStoreDeviceDriver.o \
                                                   BootloaderStoreDeviceDriverBridge.o

BFC_STD_EMBEDDED_TARGET_CONSOLE_OBJECTS = EmbeddedTargetCommandTable.o

# Files associated with the flash driver.  The nonvol, ProgramStore and
# BootloaderStore drivers all make use of this driver, so if any of them is
# included, then we also need to include these files.
BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS = FlashDeviceDriver.o
BFC_STD_EMBEDDED_TARGET_FLASH_CMD_OBJECTS = FlashDriverCommandTable.o

# PR7327 - files associated with the new flash driver.  Some of these files
# are optional.
BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += FlashBlock.o FlashDevice.o FlashRegion.o

ifeq ($(BFC_INCLUDE_INTEL_FLASH_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += IntelFlashDevice.o
endif
ifeq ($(BFC_INCLUDE_AMD_FLASH_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += AmdFlashDevice.o
endif
ifeq ($(BFC_INCLUDE_CFI_FLASH_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += CfiFlashDevice.o
endif
ifeq ($(BFC_INCLUDE_JEDEC_FLASH_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += JedecFlashDevice.o
endif
ifeq ($(BFC_INCLUDE_SPI_FLASH_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS += \
        SpiFlashDevice.o \
        GenericSpiFlashDevice.o
endif

# These are deliberately primed to be empty; if any of the other drivers are
# included, they will set these to contain the files listed above for the flash
# driver.  If none of the other drivers are included, then the flash driver
# files will be left out.
BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_OBJECTS = 
BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_CMD_OBJECTS = 

ifeq ($(BFC_INCLUDE_TIME_WASTER_THREAD), 1)
BFC_STD_EMBEDDED_TARGET_OBJECTS += TimeWasterThread.o
endif


##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_STD_EMBEDDED_TARGET_DEFINES = BCM_INCLUDE_PERIODIC_TIMERS=1

ifdef BFC_INCLUDE_PROGRAMSTORE_SUPPORT
    BFC_STD_EMBEDDED_TARGET_DEFINES += BFC_INCLUDE_PROGRAMSTORE_SUPPORT=$(BFC_INCLUDE_PROGRAMSTORE_SUPPORT)
endif

ifdef BFC_INCLUDE_BOOTLOADERSTORE_SUPPORT
    BFC_STD_EMBEDDED_TARGET_DEFINES += BFC_INCLUDE_BOOTLOADERSTORE_SUPPORT=$(BFC_INCLUDE_BOOTLOADERSTORE_SUPPORT)
endif

ifeq ($(buildProcessor), 3348)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3361)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3368)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3378)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3379)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3373)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3255)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3255)
    BFC_STD_EMBEDDED_TARGET_DEFINES += INCLUDE_MEM2MEM_DMA_SUPPORT=1
endif

ifeq ($(BFC_INCLUDE_TIME_WASTER_THREAD), 1)
    BFC_STD_EMBEDDED_TARGET_DEFINES += BFC_INCLUDE_TIME_WASTER_THREAD=1
endif

# If we include the ProgramStore driver, we need to include this compiler
# define.
#
# DPullen, Aug 5 2004 - it turns out that we need this define even if
# ProgramStore is not included, so add it to the main defines variable.
BFC_STD_EMBEDDED_TARGET_DEFINES += PROGRAM_STORE_SIGNATURE=0x$(buildProcessor)

# PR6398 - pass the PID in as well.  This is often used when building the model
# number for the sysDescr string.
#
# DPullen, Aug 5 2004 - it turns out that we need this define even if
# ProgramStore is not included, so add it to the main defines variable.
BFC_STD_EMBEDDED_TARGET_DEFINES += PROGRAM_STORE_PID=0x$(PROGRAM_STORE_PID)

# If the flash driver files are included, we also need to use these compiler
# defines.  This is used by the flash driver (among other files) to determine
# the address of the flash device.
BFC_STD_EMBEDDED_TARGET_FLASH_DEFINES = EXEC_FROM_FLASH=1

# PR7327 - defines used by the new flash driver.
BFC_STD_EMBEDDED_TARGET_FLASH_DEFINES += \
        BFC_INCLUDE_DUAL_FLASH_SUPPORT=$(BFC_INCLUDE_DUAL_FLASH_SUPPORT) \
        BFC_INCLUDE_INTEL_FLASH_SUPPORT=$(BFC_INCLUDE_INTEL_FLASH_SUPPORT) \
        BFC_INCLUDE_AMD_FLASH_SUPPORT=$(BFC_INCLUDE_AMD_FLASH_SUPPORT) \
        BFC_INCLUDE_CFI_FLASH_SUPPORT=$(BFC_INCLUDE_CFI_FLASH_SUPPORT) \
        BFC_INCLUDE_JEDEC_FLASH_SUPPORT=$(BFC_INCLUDE_JEDEC_FLASH_SUPPORT) \
        BFC_INCLUDE_SPI_FLASH_SUPPORT=$(BFC_INCLUDE_SPI_FLASH_SUPPORT) \
        BFC_SPLIT_BOOT_BLOCK_DEFAULT=$(BFC_SPLIT_BOOT_BLOCK_DEFAULT)

# Again, this is deliberately primed to be empty, and will be set non-empty if
# appropriate.
BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_DEFINES = 

# PR12211
# PR13462 - EJTAG support needs to be forced off if there is no console.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
BFC_STD_EMBEDDED_TARGET_DEFINES += BFC_INCLUDE_EJTAG_SUPPORT=$(BFC_INCLUDE_EJTAG_SUPPORT)
else
BFC_STD_EMBEDDED_TARGET_DEFINES += BFC_INCLUDE_EJTAG_SUPPORT=0
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_STD_EMBEDDED_TARGET_INCLUDE_DIRS = $(BFC_STD_EMBEDDED_TARGET_DIR)


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_STD_EMBEDDED_TARGET_SOURCE_DIRS = $(BFC_STD_EMBEDDED_TARGET_DIR)


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_STD_EMBEDDED_TARGET_LIBRARIES = 

# Check if L2TPV3 Library is being built
ifeq ($(BCM_L2TPV3_SUPPORT), 1)
    BFC_STD_EMBEDDED_TARGET_LIBRARIES += $(BFC_L2TPV3_LIB)
endif


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Always add this stuff.
BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_OBJECTS)

BFC_COMPILER_DEFINES += $(BFC_STD_EMBEDDED_TARGET_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_STD_EMBEDDED_TARGET_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_STD_EMBEDDED_TARGET_SOURCE_DIRS)

BFC_LIBRARIES += $(BFC_STD_EMBEDDED_TARGET_LIBRARIES)

# Add Nonvol driver support, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_NONVOL_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_NONVOL_OBJECTS)

    # The nonvol driver uses the flash driver, so make sure this will be added
    # to the build.
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_CMD_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_CMD_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_DEFINES = $(BFC_STD_EMBEDDED_TARGET_FLASH_DEFINES)

endif


# Add ProgramStore driver support, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_PROGRAMSTORE_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_PROGRAM_STORE_OBJECTS)

    # The ProgramStore driver uses the flash driver, so make sure this will be
    # added to the build.
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_CMD_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_CMD_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_DEFINES = $(BFC_STD_EMBEDDED_TARGET_FLASH_DEFINES)

endif


# Add ProgramStore driver support, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_BOOTLOADERSTORE_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_BOOTLOADER_STORE_OBJECTS)

    # The Bootloader Store driver uses the flash driver, so make sure this will
    # be added to the build.
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_CMD_OBJECTS = $(BFC_STD_EMBEDDED_TARGET_FLASH_CMD_OBJECTS)
    BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_DEFINES = $(BFC_STD_EMBEDDED_TARGET_FLASH_DEFINES)

endif


# Add the flash driver if any of nonvol, program store, or bootloader store are
# enabled.
BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_OBJECTS)
BFC_COMPILER_DEFINES += $(BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_DEFINES)

# Add the command tables (flash driver command table only if the flash driver 
# is included) if console is enabled.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_INCLUDED_FLASH_CMD_OBJECTS) \
                   $(BFC_STD_EMBEDDED_TARGET_CONSOLE_OBJECTS)

    # PR12211 - include EJTAG debug support files if console support is
    # included, and if EJTAG support is included.
    ifeq ($(BFC_INCLUDE_EJTAG_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_STD_EMBEDDED_TARGET_EJTAG_OBJECTS)
	
    endif
endif

