############################################################
#     (c)2007-2012 Broadcom Corporation
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
############################################################


#
# Conditional Options for this Makefile
#
# NEXUS_OS - OS to be used
#


######################
# Define this module #
######################
NEXUS_MODULES += FILE

##################
# Module Options #
##################

# alias certain OS values
ifeq ($(B_REFSW_OS),linuxemu)
NEXUS_FILE_OS=linuxuser
else
NEXUS_FILE_OS=$(B_REFSW_OS)
endif

###################
# Module Includes #
###################
NEXUS_FILE_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/file/include $(NEXUS_TOP)/modules/file/include/$(NEXUS_FILE_OS)
NEXUS_FILE_PRIVATE_INCLUDES = \
    $(NEXUS_TOP)/modules/file/src \
    $(NEXUS_TOP)/modules/file/src/$(NEXUS_FILE_OS) \
    ${BSEAV}/lib/bcmplayer/include 

##################
# Module Sources #
##################
NEXUS_FILE_SOURCES = \
    $(NEXUS_TOP)/modules/file/src/nexus_file.c \
    $(NEXUS_TOP)/modules/file/src/nexus_file_module.c \
    $(NEXUS_TOP)/modules/file/src/nexus_file_scheduler.c \
    $(NEXUS_TOP)/modules/file/src/nexus_file_chunk.c \
    $(NEXUS_TOP)/modules/file/src/nexus_file_fifo.c \
    $(NEXUS_TOP)/modules/file/src/nexus_file_fifo_chunk.c \
    $(NEXUS_TOP)/modules/file/src/$(NEXUS_FILE_OS)/nexus_file_posix.c

# thunk layer
NEXUS_FILE_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_FILE_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_file_thunks.c


##################
# Module Defines #
##################

#######################
# Module Dependencies #
#######################
NEXUS_FILE_DEPENDENCIES := CORE

NEXUS_FILE_MAGNUM_MODULES :=  \
    ${BSEAV}/lib/utils/batom.inc \
    ${BSEAV}/lib/bfile/bfile.inc

B_NAV_PLAYER_SOURCES = \
    ${BSEAV}/lib/bcmplayer/src/bcmplayer.c \
    ${BSEAV}/lib/bcmplayer/src/bcmindexer_vc1.c \
    ${BSEAV}/lib/bcmplayer/src/bcmindexer.c

B_NAV_PLAYER_INCLUDES = \
    ${BSEAV}/lib/bcmplayer/include

# BSEAV makefiles don't include their module names into MAGNUM_MODULES
NEXUS_EXTRA_MODULES += BATOM BFILE BFILE_MEDIA

# if nexus is compiled without playback module, file needs to include bcmplayer code
NEXUS_EXTRA_MODULES += B_NAV_PLAYER
