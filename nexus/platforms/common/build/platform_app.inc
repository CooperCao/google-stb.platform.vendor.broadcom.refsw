############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#   This file defines the variables needed to build an app using
#   nexus.  The output is two variables. NEXUS_APP_INCLUDE_PATHS
#   is a list of include paths needed to find the relevant files
#   needed to build a nexus applications.  NEXUS_APP_DEFINES
#   is a list of definitions needed to be defined in the app.
#   In order to use these variables, you need to convert to the
#   style your compiler needs -- e.g.
#   $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) and
#   $(addprefix -D,$(NEXUS_APP_DEFINES))
#
############################################################
ifndef NEXUS_TOP
$(error NEXUS_TOP is not defined)
endif

# Include nexus definitions
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc

# Include nexus modules
ifeq ($(PLATFORM_MODULES_INC),)
PLATFORM_MODULES_INC=$(NEXUS_TOP)/platforms/common/build/platform_modules.inc
endif
include $(PLATFORM_MODULES_INC)

# Remove BASE from public API. Leaving it in for now to ease the transition.
NEXUS_MODULES := $(patsubst BASE,,$(NEXUS_MODULES))

ifeq ($(NEXUS_EXCLUDE_PLATFORM),y)
NEXUS_MODULES := $(patsubst PLATFORM,,$(NEXUS_MODULES))
endif

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Sanity check that no modules have leaked into the build system
ifneq ($(MAGNUM_MODULES),)
$(error MAGNUM_MODULES should not be defined.  It's defined as $(MAGNUM_MODULES))
endif

# Add in magnum base module paths also
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/chp/bchp.inc
include $(MAGNUM)/basemodules/reg/breg.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/basemodules/dtu/bdtu.inc
include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/commonutils/img/bimg.inc
include $(MAGNUM)/commonutils/tee/btee.inc

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

ifeq ($(filter FILE, ${NEXUS_MODULES}), FILE)
include $(BSEAV)/lib/bfile/bfile.inc
NEXUS_APP_INCLUDE_PATHS += ${BFILE_INCLUDES}
endif

NEXUS_LD_LIBRARIES := -L${NEXUS_BIN_DIR}
ifeq ($(B_REFSW_ANDROID),)
NEXUS_LD_LIBRARIES += -lpthread -lrt
endif
ifeq ($(NEXUS_MODE),proxy)
NEXUS_CLIENT_LD_LIBRARIES := $(NEXUS_LD_LIBRARIES) -lnexus
else
ifeq ($(NEXUS_WEBCPU),core1_server)
NEXUS_CLIENT_LD_LIBRARIES := $(NEXUS_LD_LIBRARIES) -lnexus_webcpu
else
NEXUS_CLIENT_LD_LIBRARIES := $(NEXUS_LD_LIBRARIES) -lnexus_client
endif
endif
NEXUS_LD_LIBRARIES := $(NEXUS_LD_LIBRARIES) -lnexus

ifeq ($(B_REFSW_SHAREABLE),n)
# remove unused code to reduce size
NEXUS_LDFLAGS += -Wl,--gc-sections
endif

# filter out CFLAGS that aren't required of apps
ifneq ($(NEXUS_INTERNAL_CFLAGS),)
NEXUS_CFLAGS := $(filter-out $(NEXUS_INTERNAL_CFLAGS),$(NEXUS_CFLAGS))
endif

NEXUS_CXXFLAGS := $(NEXUS_CFLAGS)
