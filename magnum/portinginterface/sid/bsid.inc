###############################################################################
# (c) 2004-2016 Broadcom Corporation
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
###############################################################################

#
# Conditional Options for this Makefile
#
# BSID_IMG_ONLY -     If defined, only the firmware image related code will be compiled.
# BSID_IMG_EXTERNAL - If defined, firmware related code (interface AND context) will NOT be compiled.
#                     The app is expected to specify their own firmware interface AND image context
#                     when calling BSID_Open().
#                     => this will define BSID_USE_CUSTOM_IMAGE
#
# NOTE: it is INVALID to define both of these at the same time! (nothing will be included!)

# Define this module
MAGNUM_MODULES += BSID

# Module Conditional Compile Flags
# BSID_DEFINES +=

# Module Includes
BSID_INCLUDES += $(MAGNUM)/portinginterface/sid/include/

# Module Sources
ifeq ($(BSID_IMG_ONLY),)
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_priv.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_msg.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_picture.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_dbg.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_platform.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_fw_load.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_power.c
endif # BSID_IMG_ONLY

# Module firmware image file(s)
ifeq ($(BSID_IMG_EXTERNAL),)
# Use the built-in image API & Context
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_img.c
BSID_SOURCES += $(MAGNUM)/portinginterface/sid/src/bsid_fw.c
else
# use a custom image & context for externally supplied interface
# (NOTE: in this case, the firmware and the context that supplies the
# location of the firmware will be supplied and linked in separately)
BSID_DEFINES += BSID_USE_CUSTOM_IMAGE
endif #!BSID_IMG_EXTERNAL

#
# EOF
#
