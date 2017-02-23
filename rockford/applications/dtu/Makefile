############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
############################################################
ROCKFORD=$(shell cd ../..;pwd)
MAGNUM=$(shell cd $(ROCKFORD)/../magnum;pwd)
NEXUS_TOP=$(MAGNUM)/../nexus

NEXUS_PLATFORM ?= $(PLATFORM)
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

ODIR=obj

include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/chp/bchp.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/basemodules/reg/breg.inc

# bring in XPT for capability #defines
include $(MAGNUM)/portinginterface/xpt/bxpt.inc
include $(MAGNUM)/basemodules/int/bint.inc
include $(MAGNUM)/basemodules/mem/bmem.inc
include $(MAGNUM)/basemodules/mma/bmma.inc
include $(MAGNUM)/basemodules/dtu/bdtu.inc
include $(MAGNUM)/commonutils/avc/bavc.inc
include $(MAGNUM)/commonutils/sur/bsur.inc
include $(MAGNUM)/commonutils/fmt/bfmt.inc
include $(MAGNUM)/commonutils/pxl/bpxl.inc

# use nexus for BCHP_CHIP
CFLAGS += $(addprefix -D,$(NEXUS_APP_DEFINES)) $(NEXUS_CFLAGS)
CFLAGS += $(addprefix -D,$(foreach module,$(MAGNUM_MODULES),$($(module)_DEFINES)))
CFLAGS += $(addprefix -I,$(foreach module,$(MAGNUM_MODULES),$($(module)_INCLUDES)))
MAGNUM_OBJS += $(patsubst %.c,%.o,$(BSTD_SOURCES) $(BKNI_SOURCES) $(BDBG_SOURCES) $(BCHP_SOURCES) $(BERR_SOURCES) $(BLST_SOURCES) $(BREG_SOURCES) $(BDTU_SOURCES))
CFLAGS += -I.
LFLAGS += -lpthread

APPS=dtutool
all: $(APPS)

$(ODIR):
	mkdir $(ODIR)

Q_ ?= @

dtutool: $(ODIR) $(ODIR)/dtutool.o $(MAGNUM_OBJS)
	$(Q_)echo [link $@]
	$(Q_)$(CC) -o $@ $(LFLAGS) $(filter %.o,$^)

$(ODIR)/%.o: %.c
	$(Q_)echo [compile $@]
	$(Q_)$(Q_)$(CC) -c -o $@ $(CFLAGS) $^

%.o: %.c
	$(Q_)echo [compile $@]
	$(Q_)$(Q_)$(CC) -c -o $@ $(CFLAGS) $^

clean:
	$(Q_)$(RM) -r $(ODIR) $(APPS) $(MAGNUM_OBJS)

ifeq ($(DESTDIR),)
install:
	@echo Must define DESTDIR
else
install:
	cp $(APPS) $(DESTDIR)
endif
