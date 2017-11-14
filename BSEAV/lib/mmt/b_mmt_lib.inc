#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################
#
# Identify this library
#
B_LIBS += B_MMT_LIB

include $(B_BSEAV_TOP)/lib/utils/batom.inc
include $(B_BSEAV_TOP)/lib/bfile/bfile.inc

#
# Define source files in this library
#
B_MMT_LIB_SOURCES += \
                    $(B_BSEAV_TOP)/lib/mmt/src/bmmt_demux.c \
                    $(B_BSEAV_TOP)/lib/mmt/src/bmmt_parser.c \
                    $(B_BSEAV_TOP)/lib/mmt/src/bmmt_utils.c \
                    $(B_BSEAV_TOP)/lib/mmt/src/btlv_parser.c \
                    $(B_BSEAV_TOP)/lib/mmt/src/bmmt.c \
                    $(B_BSEAV_TOP)/lib/media/bmedia_util.c \
                    $(B_BSEAV_TOP)/lib/utils/bioatom.c \
                    $(B_BSEAV_TOP)/lib/utils/bpool.c \
                    $(B_BSEAV_TOP)/lib/utils/balloc.c \
                    $(B_BSEAV_TOP)/lib/utils/barena.c
#
# Define public include path(s) for this library
#
B_MMT_LIB_PUBLIC_INCLUDES += $(B_BSEAV_TOP)/lib/mmt/include

#
# Define private include path(s) for this library
#
B_MMT_LIB_PRIVATE_INCLUDES += $(B_BSEAV_TOP)/lib/mmt/include \
                              $(B_BSEAV_TOP)/lib/utils \
                              $(B_LIB_TOP)/os/include \
                              $(B_LIB_TOP)/os/include/$(B_REFSW_OS) \
                              $(B_BSEAV_TOP)/lib/media

#
# Define extra C definitions
#
#B_MMT_LIB_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#B_MMT_LIB_OPTIONS += ## N/A for this library
