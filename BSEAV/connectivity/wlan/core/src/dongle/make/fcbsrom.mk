#
# FCBS ROM and metadata make file
#
# Generates the following files:
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $
#
# ROML build:
#         - FCBS ROM
#         - FCBS Metadata file
#
# ROM offload build:
#         - FCBS RAM data file (FCBS ROM patches)
#         - FCBS Metadata file
#
# RAM build:
#         - FCBS RAM data file
#         - FCBS Metadata file
#

FCBS_DIR = $(SRCBASE)/tools/fcbs/

ifdef ROMTBL_VARIANT
	# ROML build
	FCBS_CFLAGS = -DFCBS_ROMLIB
else ifeq ($(ROMOFFLOAD),1)
	# ROM offload build
	FCBS_CFLAGS = -DFCBS_ROMOFFLOAD
else
	# RAM build
	FCBS_CFLAGS = -DFCBS_RAM_BUILD
endif

FCBS_CFLAGS += -DFCBS_ROM_BUILD


vpath fcbs_%.c $(SRCBASE)/tools/fcbs/
vpath fcbs_%.c $(SRCBASE)/../components/chips/fcbs/$(CHIP)/$(REV)
vpath %.c $(SRCBASE)/../components/phy/ac/dsi/
vpath %.c $(SRCBASE)/../components/phy/ac/papdcal/
vpath %.c $(SRCBASE)/wl/sys/

FCBS_INCLUDE = -I $(SRCBASE)/include/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/ac/include/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/ac/dsi/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/ac/papdcal/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/old
FCBS_INCLUDE += -I $(SRCBASE)/wl/sys/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/cmn/include/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/cmn/hal/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/cmn/core/
FCBS_INCLUDE += -I $(SRCBASE)/../components/phy/cmn/temp/
FCBS_INCLUDE += -I $(SRCBASE)/wl/ppr/include/
FCBS_INCLUDE += -I $(SRCBASE)/wl/dump/include/
FCBS_INCLUDE += -I $(SRCBASE)/wl/iocv/include/
FCBS_INCLUDE += -I $(SRCBASE)/wl/chctx/include/
FCBS_INCLUDE += -I $(SRCBASE)/../components/shared/
FCBS_INCLUDE += -I $(SRCBASE)/shared/bcmwifi/include/
FCBS_INCLUDE += -I $(SRCBASE)/shared/
FCBS_INCLUDE += -I .

FCBS_CFILES = fcbsutils.c fcbs_rom.c
FCBS_CFILES += fcbs_input_sdio.c fcbs_input_initvals.c phy_ac_dsi_data.c fcbs_input_ctrl.c d11ucode_ulp.c phy_ac_papdcal_data.c

FCBS_OBJS = $(FCBS_CFILES:.c=.fcbs.o)

FCBS_CHIP_DIR = $(SRCBASE)/../components/chips/images/fcbs/$(CHIP)/$(REV)/

# This argument order should not be changed becuase, fcbsrom utility
# expects the output files in the same order.
FCBS_UTIL_ARGS = $(FCBS_CHIP_DIR)/fcbs_input.bin
FCBS_UTIL_ARGS += $(FCBS_CHIP_DIR)/fcbs_input_metadata.bin
FCBS_UTIL_ARGS += $(FCBS_CHIP_DIR)/fcbs_rom_metadata.bin
FCBS_UTIL_ARGS += $(FCBS_CHIP_DIR)/fcbs_rom.bin
FCBS_UTIL_ARGS += $(FCBS_CHIP_DIR)/fcbs_input_seq_cnt.bin

FCBS_ROM_OUTPUT_FILES = fcbs_rom.bin fcbs_metadata.bin fcbs_metadata.c fcbs_input.bin fcbs_input_metadata.bin fcbs_input_seq_cnt.bin

%.fcbs.o: %.c wlconf.h d11shm.h
	gcc -c $(FCBS_CFLAGS) $(FCBS_INCLUDE) $< -o $@

fcbsrom: $(FCBS_OBJS)
	gcc $(FCBS_OBJS) -o $@

fcbs_metadata.bin: fcbsrom
	./$< --ram-data=fcbs_ram_data.bin -o $@ -- $(FCBS_UTIL_ARGS)

fcbs_ram_data.c: fcbs_metadata.c
	bin2c $(@:.c=.bin) $@ $(@:.c=)

fcbs_metadata.c: fcbs_metadata.bin
	bin2c $< $@ $(@:.c=)
ifdef ROMTBL_VARIANT
	cp $(FCBS_ROM_OUTPUT_FILES) $(FCBS_CHIP_DIR)
	cp $(FCBS_CHIP_DIR)/fcbs_metadata.bin $(FCBS_CHIP_DIR)/fcbs_rom_metadata.bin
	cp $(FCBS_CHIP_DIR)/fcbs_metadata.c $(FCBS_CHIP_DIR)/fcbs_rom_metadata.c
endif
