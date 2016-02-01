############################################################################
#     Copyright (c) 2011-2014, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
############################################################################
#
# DESCRIPTION:
#   Makefile for building drmrootfs library.
#
############################################################################

MAGNUM_TOP := $(shell cd ../../../magnum; pwd)
NEXUS_TOP := $(shell cd ../../../nexus; pwd)


ifeq ($(NEXUS_MODE),proxy)
PUBLIC_LIBS    += libdrmrootfs.a
else
PUBLIC_LIBS    += libdrmrootfs.a libdrmrootfs.so
endif

#include drmrootfs_libdir.inc to see which directory the lib will be installed in $(DRMROOTFS_LIBDIR)
include drmrootfs_libdir.inc


# Include nexus definitions
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(NEXUS_TOP)/build/os/linuxuser/os_tools.inc


#Note, Nexus's platform_app.inc will take care of loading magnum base module paths
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc


# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

include $(NEXUS_TOP)/base/base.inc
include $(NEXUS_TOP)/modules/dma/dma.inc
include $(NEXUS_TOP)/modules/core/core.inc

include $(MAGNUM_TOP)/basemodules/mem/bmem.inc
include $(MAGNUM_TOP)/portinginterface/hsm/bhsm.inc

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}
CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
CFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))


TOOLCHAIN_DIR=$(TOOLCHAIN)

ifeq ($(ANDROID_BUILD),y)
ARCH ?= $(B_REFSW_ARCH)
endif

SYSTEM ?= linux

ifeq ($(ARCH),mipsel-linux)
TOOLCHAIN_DIR=/opt/toolchains/mipsel
else
#
# Discover the uclibc toolchain directory assuming the compiler exists in bin subdir
# Use which and dirname bash shell commands.
#
TOOLCHAIN_DIR=$(shell dirname $(shell dirname $(shell which mipsel-linux-uclibc-gcc)))
endif

ifeq ($(B_REFSW_OS),linuxkernel)
USE_KERNEL_TIME ?= n  
ifeq ($(USE_KERNEL_TIME), y)
CFLAGS += -DKERNEL_TIME_DEFINE
endif
CFLAGS += -DMODULE -DLINUX -D__KERNEL__ #-DDRM_NO_NATIVE_64_TYPES 
CFLAGS += -nostdinc -msoft-float -fshort-wchar -O3 -fno-pic -G0 -mno-abicalls -mlong-calls
CFLAGS += -I$(MAGNUM)/basemodules/std/types/linuxkernel 
ifeq ($(SMP), y)
CFLAGS += -I$(LINUX)/include.mipsel-uclibc.$(PLATFORM)-smp
CFLAGS += -I$(LINUX)/include.mipsel-uclibc.$(PLATFORM)-smp/asm-mips/mach-brcmstb
CFLAGS += -I$(LINUX)/include.mipsel-uclibc.$(PLATFORM)-smp/asm/mach-generic/
else
CFLAGS += -I$(LINUX)/include
CFLAGS += -I$(LINUX)/arch/mips/include
CFLAGS += -I$(LINUX)/arch/mips/include/asm-generic
CFLAGS += -I$(LINUX)/arch/mips/include/asm/mach-rm
CFLAGS += -I$(LINUX)/arch/mips/include/asm/brcmceg
CFLAGS += -I$(LINUX)/arch/mips/include/asm/brcmceg/common
CFLAGS += -I$(LINUX)/arch/mips/include/asm/mach-generic
endif 
CFLAGS += -I$(TOOLCHAIN_DIR)/$(ARCH)/include
CFLAGS += -I$(TOOLCHAIN_DIR)/lib/gcc/$(ARCH)/4.5.3/include
endif

ifeq ($(ANDROID_BUILD),y)
CFLAGS += $(NEXUS_CFLAGS)
endif

ifeq ($(findstring arm-linux, ${B_REFSW_ARCH}), arm-linux)
CFLAGS +=-marm -march=armv7-a
endif


F_INSTALLED_LIBS	:= $(addprefix ${DRMROOTFS_LIBDIR}/, ${PUBLIC_LIBS})

############################################################################
#                              
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
SRCS	+= $(wildcard *.c )
OBJS 	+= $(patsubst %.c,%.o, ${SRCS})
CFLAGS  += -g -fshort-wchar  -fpic

PUBLIC_INCS    += drm_data.h

ifeq ($(ANDROID_BUILD),y)
LDFLAGS	+= $(NEXUS_LD_LIBRARIES)
LDFLAGS += --sysroot=$(ANDROID)/out/target/product/bcm_platform/system
LDFLAGS += $(NEXUS_LDFLAGS)
endif
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



# Must export MSDRM_PRDY_SUPPORT to build the lib with playready support. 
ifeq ($(MSDRM_PRDY_SUPPORT), y)
CFLAGS += -DB_HAS_MSDRM_PRDY=1
endif

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
install: all ${F_INSTALLED_LIBS} 

all: ${PUBLIC_LIBS}

.c.o: 
	@echo [Compile... $<]
	${Q_}$(CC) -c $< $(CFLAGS) 

clean: 
	rm -f *.o *.d 
	ls *.a | xargs rm -f
	ls *.so | xargs rm -f
	
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

${DRMROOTFS_LIBDIR}/%.a : %.a
	install -m 0755 $< $@ 

${DRMROOTFS_LIBDIR}/%.so : %.so
	install -m 0755 $< $@ 


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
libdrmrootfs.so: ${OBJS}
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-dy
endif

libdrmrootfs.a: ${OBJS} 
	${AR} rc  $@  $^ 


############################################################################
#                             DEP FILES, PHONY
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
-include *.d
.PHONY: all install clean distclean 
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##





