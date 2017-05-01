#***************************************************************************
#*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#*
#*  This program is the proprietary software of Broadcom and/or its licensors,
#*  and may only be used, duplicated, modified or distributed pursuant to the terms and
#*  conditions of a separate, written license agreement executed between you and Broadcom
#*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#*  no license (express or implied), right to use, or waiver of any kind with respect to the
#*  Software, and Broadcom expressly reserves all rights in and to the Software and all
#*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#*  Except as expressly set forth in the Authorized License,
#*
#*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#*  and to use this information only in connection with your use of Broadcom integrated circuit products.
#*
#*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#*  USE OR PERFORMANCE OF THE SOFTWARE.
#*
#*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#*  ANY LIMITED REMEDY.
#**************************************************************************
LIBVPX_SRCDIR := ${NEXUS_TOP}/../BSEAV/opensource/libvpx
LIBVPX_OBJDIR := ${B_REFSW_OBJ_ROOT}/BSEAV/opensource/libvpx
LIBVPX?=${LIBVPX_OBJDIR}/libvpx-1.5.0/vpx/libvpx.a
ifeq ($(filter $(BSOFT_VIDEO_DECODER_LIBVPX), y), y)
NEXUS_VIDEO_DECODER_P_SOFTDECODE := ${LIBVPX_SRCDIR}
NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIBDIR := ${LIBVPX_OBJDIR}
NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB := ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIBDIR}/libsoftdecode.a
NEXUS_VIDEO_DECODER_DEFINES += NEXUS_VIDEO_DECODER_LIBVPX=1
else
NEXUS_VIDEO_DECODER_P_SOFTDECODE := ${NEXUS_TOP}/../BSEAV/opensource/ffmpeg
NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIBDIR := ${B_REFSW_OBJ_ROOT}/BSEAV/opensource/ffmpeg
NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB := ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIBDIR}/libffmpeg.a
NEXUS_VIDEO_DECODER_P_SOFTDECODE_INC := ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIBDIR}
endif

B_SOFTDECODE_ROOT := ${NEXUS_VIDEO_DECODER_P_SOFTDECODE}

###################
# Module Includes #
###################
NEXUS_VIDEO_DECODER_PRIVATE_INCLUDES += \
    ${NEXUS_VIDEO_DECODER_P_SOFTDECODE} \
    $(NEXUS_TOP)/extensions/video_decoder/soft_video_decoder/src

ifeq ($(filter $(BSOFT_VIDEO_DECODER_LIBVPX), y), y)
NEXUS_VIDEO_DECODER_PRIVATE_INCLUDES += ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_INC}
endif

##################
# Module Sources #
##################
NEXUS_VIDEO_DECODER_SOURCES += \
    $(NEXUS_TOP)/extensions/video_decoder/soft_video_decoder/src/nexus_video_decoder_soft.c

##################
# Module Defines #
##################

ifeq ($(filter $(BSOFT_VIDEO_DECODER_LIBVPX), y), y)
NEXUS_EXTRALIBS += ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB} ${LIBVPX}
else
NEXUS_EXTRALIBS += ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB} ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}/ffmpeg-2.4/libavcodec/libavcodec.a ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}/ffmpeg-2.4/libavutil/libavutil.a
NEXUS_EXTRA_LDFLAGS += -lm
endif

NEXUS_VIDEO_DECODER_DEPENDENCIES += GRAPHICS2D

ifdef NEXUS_INCLUDE_MODULE_RULES
ifeq ($(filter $(BSOFT_VIDEO_DECODER_LIBVPX), y), y)
${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}:
	${Q_}${MAKE} -C ${NEXUS_VIDEO_DECODER_P_SOFTDECODE} B_REFSW_OBJ_ROOT=$(abspath ${B_REFSW_OBJ_ROOT}) B_REFSW_CROSS_COMPILE=${B_REFSW_CROSS_COMPILE}

${LIBVPX}: ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}

${LIBVPX_OBJDIR}/libvpx-1.5.0/vpx/libvpx.a: ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}

${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}.clean:
	${Q_}${MAKE} -C ${NEXUS_VIDEO_DECODER_P_SOFTDECODE} ffmpeg-2.4.clean
else

${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}:
	${Q_}${MAKE} -C ${NEXUS_VIDEO_DECODER_P_SOFTDECODE} B_REFSW_OBJ_ROOT=$(abspath ${B_REFSW_OBJ_ROOT}) B_REFSW_CROSS_COMPILE=${B_REFSW_CROSS_COMPILE} LIB=ffmpeg ffmpeg-2.4

${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}/ffmpeg-2.4/libavcodec/libavcodec.a ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}/ffmpeg-2.4/libavutil/libavutil.a: ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}

${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}.clean:
	${Q_}${MAKE} -C ${NEXUS_VIDEO_DECODER_P_SOFTDECODE} ffmpeg-2.4.clean
endif

OTHER_CLEANS += ${NEXUS_VIDEO_DECODER_P_SOFTDECODE_LIB}.clean

endif
