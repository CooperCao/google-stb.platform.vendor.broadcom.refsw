############################################################################
#     Copyright (c) 2011 Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################################

CWD := $(shell pwd)
B_REFSW_ROOT := ${CWD}/../../../..

ifneq ($(B_REFSW_VERBOSE),)
Q_:=
else
Q_:=@
MAKEFLAGS += --no-print-directory
endif

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
    $(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(LINUX),)
LINUX = /opt/brcm/linux
endif


ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif

# NOTE: $LINUX/Makefile defaults ARCH=mips. It cannot be set to mipsel-linux here.
ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux mipsel-uclibc mipsel-linux-uclibc mipsel-linux-android), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=mips
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc mips-linux-android), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=mips
B_REFSW_CROSS_COMPILE ?= mips-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm
B_REFSW_CROSS_COMPILE ?= arm-linux-
endif
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm64
B_REFSW_CROSS_COMPILE ?= aarch64-linux-
endif
endif
endif

ifeq ($(ANDROID_BUILD),y)
ifdef B_REFSW_KERNEL_CROSS_COMPILE
B_REFSW_CROSS_COMPILE := $(B_REFSW_KERNEL_CROSS_COMPILE)
endif
BCM_DRIVER_EXTRACFLAGS += -DB_REFSW_ANDROID
endif

ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/usermode
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)


# tools
CP = cp -f
CC = $(B_REFSW_CROSS_COMPILE)gcc
AS = $(B_REFSW_CROSS_COMPILE)as
LD = $(B_REFSW_CROSS_COMPILE)ld
OBJCOPY = $(B_REFSW_CROSS_COMPILE)objcopy

# parse the Linux Makefile
LINUX_VERSION := $(shell grep -m 1 '^VERSION = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_PATCHLEVEL := $(shell grep -m 1 '^PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_SUBLEVEL := $(shell grep -m 1 '^SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')

LINUX_VER_GE_3 :=  $(shell test $(LINUX_VERSION) -ge 3 && echo y)
ifeq ($(LINUX_VER_GE_3),y)
	LINUX_VER_GE_2_6 = y
	LINUX_VER_GE_2_6_31 = y
	LINUX_VER_GE_2_6_37 = y
else
LINUX_VER_GE_2_6 ?= $(shell test $(LINUX_PATCHLEVEL) -eq 6 && echo 'y')
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_GE_2_6_31 :=  $(shell test $(LINUX_SUBLEVEL) -ge 31 && echo y)
LINUX_VER_GE_2_6_37 :=  $(shell test $(LINUX_SUBLEVEL) -ge 37 && echo y)
LINUX_VER_EQ_2_6_12 := $(shell test $(LINUX_SUBLEVEL) -eq 12 && echo 'y')
endif
endif

vpath %.c ./
vpath %.c ./${NEXUS_PLATFORM}
vpath %.c ${BCM_OBJ_ROOT}/interrupt_table

BCM_DRIVER := $(BCM_OBJ_PATH)/bcmdriver.ko
BCM_DRIVER_CFLAGS += -DKBUILD_MODNAME=bcmdriver -DLINUX -D__KERNEL__ -DMODULE
BCM_DRIVER_CFLAGS += -I$(LINUX)/include -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include

ifneq ($(B_REFSW_LINUX_ARCH),arm64)
BCM_DRIVER_CFLAGS += -mlong-calls
endif

ifdef NEXUS_PLATFORM_VERSION_MAJOR
BCM_DRIVER_CFLAGS += -DNEXUS_VERSION_MAJOR=$(NEXUS_PLATFORM_VERSION_MAJOR) -DNEXUS_VERSION_MINOR=$(NEXUS_PLATFORM_VERSION_MINOR)
endif
ifeq ($(LINUX_VER_GE_3),y)
BCM_DRIVER_CFLAGS += -include linux/kconfig.h
else
ifeq ($(LINUX_VER_GE_2_6_37),y)
BCM_DRIVER_CFLAGS += -include generated/autoconf.h
else
BCM_DRIVER_CFLAGS += -include linux/autoconf.h
endif
endif

BCM_DRIVER_SRCS += \
	interrupt_table.c \
	bcm_driver.c

ifeq ($(filter ${B_REFSW_LINUX_ARCH}, arm arm64), ${B_REFSW_LINUX_ARCH})
BCM_DRIVER_CFLAGS += -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/uapi \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated/uapi \
                     -I$(LINUX)/include/uapi -I$(LINUX)/include/generated/uapi

ifeq ($(B_REFSW_LINUX_ARCH),arm)
BCM_DRIVER_SRCS_AS = bcmdriver_arm_v7.S
else
BCM_DRIVER_SRCS_AS = bcmdriver_arm_v8.S
endif

BRCMSTB_MEGA_BARRIER := $(shell grep -o 'brcmstb_mega_barrier' ${LINUX}/Module.symvers)
ifeq ($(BRCMSTB_MEGA_BARRIER),brcmstb_mega_barrier)
BCM_DRIVER_CFLAGS += -DUSE_BRCMSTB_MEGA_BARRIER
endif

endif

ICM_CLI_SUPPORT ?= y
ifeq ($(ICM_CLI_SUPPORT),y)
BCM_DRIVER_CFLAGS += -DBICM_ICAM2
endif

ifeq ($(B_HAS_PLAYPUMP_IP),y)
BCM_DRIVER_CFLAGS += -DB_NETIF_DMA=1 -I$(B_REFSW_ROOT)/BSEAV/api/include/
BCM_DRIVER_SRCS += netif_dma_stub.c
else
BCM_DRIVER_CFLAGS += -DB_NETIF_DMA=0
endif

BCM_DRIVER_CFLAGS += -I$(B_REFSW_ROOT)/magnum/commonutils/lst/
BCM_DRIVER_CFLAGS += -I.
BCM_DRIVER_CFLAGS += -DHAVE_RSRVD=1
BCM_DRIVER_CFLAGS += -DBCHP_VER=BCHP_VER_$(BCHP_VER)

# compile bcmdriver the same for webcpu servers. client gets just m2mc1.
ifeq ($(NEXUS_WEBCPU),core1_server)
BCM_DRIVER_CFLAGS += -DNEXUS_WEBCPU
endif

BCM_DRIVER_OBJS := $(patsubst %.c,%.o,$(BCM_DRIVER_SRCS))
BCM_DRIVER_OBJS := $(addprefix $(BCM_OBJ_PATH)/,$(BCM_DRIVER_OBJS))
BCM_DRIVER_OBJS_AS := $(patsubst %.S,%.o,$(BCM_DRIVER_SRCS_AS))
BCM_DRIVER_OBJS_AS := $(addprefix $(BCM_OBJ_PATH)/,$(BCM_DRIVER_OBJS_AS))

all: ${BCM_DRIVER}

ifndef BCM_KBUILD_CFLAGS
${BCM_DRIVER}: ${BCM_DRIVER_SRCS} $(BCM_OBJ_PATH)/Kbuild_flags
	$(eval $(shell grep -m 1 -e "BCM_KBUILD_CFLAGS:=" $(BCM_OBJ_PATH)/Kbuild_flags))
	$(eval $(shell grep -m 1 -e "BCM_KBUILD_LDFLAGS:=" $(BCM_OBJ_PATH)/Kbuild_flags))
	@[ -n "${BCM_KBUILD_CFLAGS}" ] || (echo "can't obtain BCM_KBUILD_CFLAGS";exit 1)
	${Q_}${MAKE} BCM_KBUILD_CFLAGS='${BCM_KBUILD_CFLAGS}' BCM_KBUILD_LDFLAGS='${BCM_KBUILD_LDFLAGS}'


else
$(BCM_DRIVER): $(BCM_DRIVER_OBJS) $(BCM_DRIVER_OBJS_AS)
	@echo [Link...... bcmdriver.ko]
ifeq ($(LINUX_VER_GE_2_6_31),y)
	$(Q_)cd ${LINUX} && $(LD) -Map $(BCM_DRIVER).map $(BCM_DRIVER_LDFLAGS) $(BCM_KBUILD_LDFLAGS) $(BCM_DRIVER_EXTRALDFLAGS) --strip-debug --build-id=none -r $^  -o $(BCM_OBJ_PATH)/bcmdriver.o_shipped
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) modules PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)
else
	@# do not use kbuild for 2.6.18
	$(Q_)$(LD) -Map $(BCM_DRIVER).map $(BCM_DRIVER_LDFLAGS) $(BCM_KBUILD_LDFLAGS) $(BCM_DRIVER_EXTRALDFLAGS) --strip-debug -r $^  -o $@
endif
	@# disable MODVERSIONS if kernel symvers are missing
	@if [ -s $(LINUX)/Modules.symvers ]; then \
		$(OBJCOPY) --remove-section __versions $(BCM_OBJ_PATH)/bcmdriver.ko ; \
	fi
endif # BCM_KBUILD_CFLAGS

$(BCM_OBJ_PATH)/%.o: %.c
	@echo [Compile... $(notdir $<)];
	$(Q_)$(CC) $(BCM_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(BCM_DRIVER_EXTRACFLAGS) -o $@ -c $<

$(BCM_OBJ_PATH)/%.o: %.S
	@echo [Compile... $<];
	$(Q_)$(CC) $(BCM_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(BCM_DRIVER_EXTRACFLAGS) -D__ASSEMBLY__ -o $@ -c $<


clean: $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C interrupt_table clean
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) clean
	${Q_}$(RM) $(BCM_DRIVER) $(BCM_DRIVER).map $(BCM_OBJ_PATH)/Kbuild_flags $(BCM_OBJ_PATH)/bcmdriver.o_shipped $(BCM_DRIVER_OBJS) $(BCM_DRIVER_OBJS_AS)

ifeq ($(INSTALL_DIR),)
install:
	$(error INSTALL_DIR is undefined)
else
install: all
	@echo "[Install... bcmdriver.ko]"
	${Q_}$(CP) $(BCM_OBJ_PATH)/bcmdriver.ko $(INSTALL_DIR)
endif

$(BCM_OBJ_PATH)/Kbuild: Kbuild $(BCM_OBJ_PATH)/exists
	@echo [Copying... Kbuild];
	$(Q_)$(CP) $< $@

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

.PHONY: $(BCM_OBJ_PATH)/Kbuild_flags

$(BCM_OBJ_PATH)/Kbuild_flags : $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) modules ARCH=$(B_REFSW_LINUX_ARCH) PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE) BCM_PRINT_KBUILD_ENV_ONLY=1 2>$@ >/dev/null


$(BCM_OBJ_ROOT)/interrupt_table/exists:
	${Q_}mkdir -p $@

interrupt_table.c:$(BCM_OBJ_ROOT)/interrupt_table/exists

${BCM_OBJ_ROOT}/interrupt_table/interrupt_table.c:
	${Q_}${MAKE} -C interrupt_table BREG_CAPTURE=

