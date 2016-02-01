############################################################
#     (c)2008-2009 Broadcom Corporation
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
# Created:
#
# $brcm_Log: $
# 
############################################################

#
# Conditional options for this makefile
#
# None at this time
#

#
# Identify this library
#
B_LIBS += B_DVR_LIB

B_DVR_LIB_SOURCES+= \
                   $(B_CABLE_TOP)/lib/dvr/manager/src/b_dvr_manager.c \
                   $(B_CABLE_TOP)/lib/dvr/manager/src/priv/b_dvr_manager_priv.c \
                   $(B_CABLE_TOP)/lib/dvr/manager/src/b_dvr_mediastorage.c \
                   $(B_CABLE_TOP)/lib/dvr/utils/file/src/b_dvr_file.c \
                   $(B_CABLE_TOP)/lib/dvr/utils/segmentedfile/src/b_dvr_segmentedfile.c \
                   $(B_CABLE_TOP)/lib/dvr/utils/media/src/b_dvr_mediafile.c  \
                   $(B_CABLE_TOP)/lib/dvr/utils/msutil/src/msutil.c  \
                   $(B_CABLE_TOP)/lib/dvr/utils/msutil/src/msdiag.c  \
                   $(B_CABLE_TOP)/lib/dvr/utils/prof/src/b_dvr_prof.c  \
                   $(B_CABLE_TOP)/lib/dvr/services/tsb/src/b_dvr_tsbservice.c \
                   $(B_CABLE_TOP)/lib/dvr/services/playback/src/b_dvr_playbackservice.c \
                   $(B_CABLE_TOP)/lib/dvr/services/record/src/b_dvr_recordservice.c \
                   $(B_CABLE_TOP)/lib/dvr/services/datainjection/src/b_dvr_datainjectionservice.c \
                   $(B_CABLE_TOP)/lib/dvr/services/drm/src/b_dvr_drmservice.c \
                   $(B_CABLE_TOP)/lib/dvr/utils/media_probe/src/b_dvr_mediaprobe.c 


ifeq ($(DVREXTLIB_TRANSCODE),y)
B_DVR_LIB_SOURCES+= $(B_CABLE_TOP)/lib/dvr/services/transcode/src/b_dvr_transcodeservice.c
endif
ifeq ($(DVREXTLIB_MEDIANODE_ONDEMAND_CACHING),y)
B_DVR_LIB_SOURCES+= $(B_CABLE_TOP)/lib/dvr/utils/list/src/b_dvr_list_v2.c
else
B_DVR_LIB_SOURCES+= $(B_CABLE_TOP)/lib/dvr/utils/list/src/b_dvr_list.c
endif

B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../nexus/modules/file/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../nexus/modules/file/include/priv
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../nexus/modules/security/$(BCHP_CHIP)/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../../magnum/commonutils/lst
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../base/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include 
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include/linuxuser
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../nexus/modules/file/src
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/../lib/bcmplayer/include 
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/../lib/bcmplayer/src
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/../lib/media
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/../lib/utils
ifeq ($(DVREXTLIB_DRM_SUPPORT),y)
B_DVR_LIB_DEFINES += DRM_SUPPORT

ifeq ($(DVREXTLIB_KEYLADDER_SUPPORT),y)
B_DVR_LIB_DEFINES += KEYLADDER_SUPPORT
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../extensions/security/keyladder/$(BCHP_CHIP)/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../extensions/security/otpmsp/$(BCHP_CHIP)/include
B_DVR_LIB_PUPLIC_INCLUDES += $(B_LIB_TOP)/../extensions/security/usercmd/$(BCHP_CHIP)/include
endif

endif

ifeq ($(DVREXTLIB_LABEL_SUPPORT),y)
B_DVR_LIB_DEFINES += LABEL_SUPPORT
endif

ifeq ($(DVREXTLIB_FALLOCATE_SUPPORT),y)
B_DVR_LIB_DEFINES += FALLOCATE_SUPPORT
endif

ifeq ($(DVREXTLIB_MEDIANODE_ONDEMAND_CACHING),y)
B_DVR_LIB_DEFINES += MEDIANODE_ONDEMAND_CACHING
endif

ifeq ($(DVREXTLIB_TRANSCODE),y)
B_DVR_LIB_DEFINES += TRANSCODE_SUPPORT
endif

B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/manager/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/file/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/segmentedfile/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/list/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/media/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/msutil/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/prof/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/tsb/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/playback/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/record/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/drm/include
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/datainjection/include
ifeq ($(DVREXTLIB_TRANSCODE),y)
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/services/transcode/include
endif
B_DVR_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dvr/utils/media_probe/include


B_DVR_LIB_PRIVATE_INCLUDES += $(B_CABLE_TOP)/lib/dvr/manager/include/priv

#
# Define other libs that this library is dependent upon.  
# This will check for cyclic dependencies and setup include paths
#
B_DVR_LIB_DEPENDENCIES := B_OS_LIB

#
# Define extra C definitions
#
#B_DVR_LIB_DEFINES += ## N/A for this library
#
# Define possible options - this is only for debug, it does _not_ actually define them.
#
#B_DVR_LIB_OPTIONS += ## N/A for this library

