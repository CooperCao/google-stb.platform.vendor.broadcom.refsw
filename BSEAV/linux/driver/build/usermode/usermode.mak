############################################################
#	  (c)2012-2015 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
# 
############################################################################

#
# Configure linux kernel source
# Output is LINUX_INC
#

ICM_CLI_SUPPORT ?= y
ifeq ($(ICM_CLI_SUPPORT),y)
CFLAGS += -DBICM_ICAM2
endif

OBJS = bcm_driver.o interrupt_table.o
CFLAGS += -I$(BSEAV)/linux/driver/usermode/

ifeq (${B_HAS_PLAYPUMP_IP},y)
CFLAGS += -DB_NETIF_DMA=1 -I$(BSEAV)/api/include/
OBJS += netif_dma_stub.o
endif

CFLAGS += ${B_REFSW_CFLAGS} ${B_REFSW_LINUXKERNEL_CFLAGS}
CFLAGS += -I../../../../../magnum/commonutils/lst/

ifeq ($(LINUX_VER_GE_2_6),y)
OBJS += umdrv_mod.o
CFLAGS += -DKBUILD_MODNAME=bcmdriver
MOD_EXT = ko
else
MOD_EXT = o
endif

UMDRV = $(BCM_OBJ_DIR)/bcmdriver.${MOD_EXT}
DRIVERS += $(UMDRV)

# rules
clean-umdrv:
	${Q_}$(RM) -r $(BCM_OBJ_DIR)

$(BCM_OBJ_DIR)/%.o: $(BSEAV)/linux/driver/usermode/%.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -MMD -c $(CFLAGS) $< -o $@

$(BCM_OBJ_DIR)/%.o: $(BSEAV)/linux/driver/usermode/$(NEXUS_PLATFORM)/%.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -MMD -c $(CFLAGS) $< -o $@

$(UMDRV): $(addprefix ${BCM_OBJ_DIR}/, ${OBJS})
	@echo [Linking... $(notdir $@)]
	${Q_}$(LD) -r $^ -o $@ $(LDFLAGS)

-include $(BCM_OBJ_DIR)/*.d

