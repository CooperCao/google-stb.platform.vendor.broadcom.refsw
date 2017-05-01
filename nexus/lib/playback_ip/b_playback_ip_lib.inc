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
############################################################
#
# Protect against multiple/circular includes...
ifndef B_REFSW_B_PLAYBACK_IP_LIB_INC_INCLUDED
B_REFSW_B_PLAYBACK_IP_LIB_INC_INCLUDED := 1
#
# Conditional options for this makefile
#
# None at this time
#
#
# Sanity check that NEXUS_TOP is defined
#
ifndef NEXUS_TOP
$(error NEXUS_TOP is not defined.)
endif

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)

B_PLAYBACK_IP_LIB_TOP := $(NEXUS_TOP)/lib/playback_ip

B_LIB_TOP := $(B_PLAYBACK_IP_LIB_TOP)/..
#
# Identify this library
#
B_LIBS += B_PLAYBACK_IP_LIB

export PLAYBACK_IP_SUPPORT=y
B_PLAYBACK_IP_LIB_DEFINES := PLAYBACK_IP_SUPPORT
export HTTP_MP4_SUPPORT=y

ifeq ($(LOG_IP_LATENCY),y)
    B_PLAYBACK_IP_LIB_DEFINES += LOG_IP_LATENCY
endif

ifneq ($(LIVEMEDIA_SUPPORT),n)
    export LIVEMEDIA_SUPPORT=y

    include $(NEXUS_TOP)/../BSEAV/opensource/livemedia/livemedia.inc

    # LiveMedia start out from this directory and gets untarred to LIVEMEDIA_DIR
    LIVEMEDIA_ORG = $(B_LIVEMEDIA_BUILD_DIR)

    # LiveMedia include files and source files end up here
    LIVEMEDIA_DIR = $(B_LIVEMEDIA_DIR)

    BLIVE_EXT_DIR = $(NEXUS_TOP)/../BSEAV/opensource/livemedia/blive_ext
    include $(BLIVE_EXT_DIR)/include/blive_ext.mak
endif #LM

ifneq ($(HLS_PROTOCOL_SUPPORT),n)
    export HLS_PROTOCOL_SUPPORT=y
    ifeq ($(HTTP_AES_SUPPORT),n)
        $(error HTTP_AES_SUPPORT is disabled, must disable HLS_PROTOCOL_SUPPORT)
    endif #HTTP_AES_SUPPORT
    export HTTP_AES_SUPPORT=y
    ifeq ($(SSL_SUPPORT),n)
        $(error SSL_SUPPORT is disabled, must disable HLS_PROTOCOL_SUPPORT)
    endif
    export SSL_SUPPORT=y
endif

ifneq ($(MPEG_DASH_PROTOCOL_SUPPORT),n)
    export MPEG_DASH_PROTOCOL_SUPPORT=y
    export B_PLAYBACK_IP_XML_SUPPORT=y
endif

ifeq ($(HTTP_AES_SUPPORT),y)
    ifeq ($(SSL_SUPPORT),n)
        $(error SSL_SUPPORT is disabled, cannot enable HTTP_AES_SUPPORT)
    endif #SSL_SUPPORT
    export SSL_SUPPORT=y
endif #HTTP_AES_SUPPORT

#off by default
ifeq ($(NETACCEL_SUPPORT),y)
    export NETACCEL_SUPPORT
endif #NETACCEL_SUPPORT

ifeq ($(SMS_GATEWAY),y)
    # When compiling for SMS platforms, .inc files of Nexus Playback, Audio, Video modules are not included.
    # Thus, we explictly include their paths for header files.
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_SMS_GATEWAY

    ifneq ($(EROUTER_SUPPORT),n)
        B_PLAYBACK_IP_LIB_DEFINES += EROUTER_SUPPORT
    endif

    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/playback/include
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/video_decoder/${BCHP_CHIP}/include
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/audio/${BCHP_CHIP}/include
    B_PLAYBACK_IP_LIB_DEFINES += NEXUS_HAS_PLAYBACK
endif #SMS_GATEWAY

#
# Define source files in this library
#
B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_udp.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_rtp.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_file_streaming.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_live_streaming.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_utils.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_tts_throttle.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_psi.c \
                    $(NEXUS_TOP)/../BSEAV/lib/rtp/brtp_spf.c \
                    $(NEXUS_TOP)/../BSEAV/lib/rtp/btimestamp.c
ifeq ($(HTTP_MP4_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_HTTP_MP4_SUPPORT
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/ts_priv.c \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/ts_psi.c \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/ts_pmt.c \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/ts_pat.c \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/ts_packet.c \
                    $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse/tspsi_validate.c
endif

# bcmplayer & index creation related files
B_PLAYBACK_IP_LIB_DEFINES += CMDLINE_TEST
B_PLAYBACK_IP_LIB_DEFINES += B_IS_SOFTINDEXER
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/bcmplayer/include
B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_nav_indexer.c\
                    $(NEXUS_TOP)/../BSEAV/lib/bcmplayer/src/tsindexer.c\
                    $(NEXUS_TOP)/../BSEAV/lib/bcmplayer/src/bcmindexer.c

# we dont compile these files for SMS as they have depdendencies on the AV decoder headers which are not included in SMS

ifneq ($(SMS_GATEWAY),y)
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_http.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_trickmode.c
endif

ifeq ($(NEXUS_PLATFORM),93383sms)
    #Special case sms does have decode
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_http.c \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_trickmode.c
endif

ifeq ($(HLS_PROTOCOL_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_HLS_PROTOCOL_SUPPORT
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_hls.c
endif

ifeq ($(MPEG_DASH_PROTOCOL_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_MPEG_DASH_PROTOCOL_SUPPORT
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_mpeg_dash.c
endif

ifeq ($(B_PLAYBACK_IP_XML_SUPPORT),y)
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_xml_helper.cpp \
                    $(BSEAV)/lib/mlibs/mstring.cpp \
                    $(BSEAV)/lib/mlibs/mxmlparser.cpp \
                    $(BSEAV)/lib/mlibs/mxmlelement.cpp \
                    $(BSEAV)/lib/mlibs/mvoidlist.cpp \
                    $(BSEAV)/lib/mlibs/mregexp.cpp
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(BSEAV)/lib/mlibs
endif

ifeq ($(HTTP_AES_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_HTTP_AES_SUPPORT
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_aes.c
endif

ifneq ($(LIVEMEDIA_SUPPORT),n)
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_lm_helper.cpp \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_lm_rtp_src.cpp
endif # LIVEMEDIA_SUPPORT

ifeq ($(SPF_SUPPORT),y)
#Changes for RTSP-ES support
    export SPF_SUPPORT=y
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/opensource/livemedia
    B_PLAYBACK_IP_LIB_DEFINES += SPF_SUPPORT
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_LIB_TOP)/playback_ip/src/b_playback_ip_rtsp_es.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_delay.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_h264.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_h263.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_mpeg4part2.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_mpeg4.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_aacloas.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_parser_amr.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_util.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/lm.cpp \
                    $(B_LIB_TOP)/../../BSEAV/lib/utils/biovec.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/utils/biobits.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/utils/bbase64.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/brtp_spf.c \
                    $(B_LIB_TOP)/../../BSEAV/lib/rtp/btimestamp.c
else
    B_PLAYBACK_IP_LIB_SOURCES += \
                    $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_rtsp.c
endif # SPF_SUPPORT

ifeq ($(LIVE_STREAMING_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_LIVE_STREAMING
endif

ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
    B_PLAYBACK_IP_LIB_SOURCES += $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_dtcp_ip.c
endif

ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
    B_PLAYBACK_IP_LIB_SOURCES += $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_dtcp_ip.c
endif

ifeq ($(SSL_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_SSL
    B_PLAYBACK_IP_LIB_SOURCES += $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_ssl.c
endif

ifeq ($(RAD_EA_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_RAD_EA
    B_PLAYBACK_IP_LIB_SOURCES += $(B_PLAYBACK_IP_LIB_TOP)/src/b_playback_ip_rad_ea.c \
                             $(B_PLAYBACK_IP_LIB_TOP)/src/cryptdes.cpp \
                             $(B_PLAYBACK_IP_LIB_TOP)/src/radeadecoder.cpp
endif

ifeq ($(HTTP_MP4_SUPPORT),y)
    #
    # Add media related sources
    #
    MEDIA_MODULES := BFLV_PROBE BMEDIA_PROBE BASF_PROBE BAVI_PROBE BFILE_STDIO
    ifeq ($(SMS_GATEWAY),y)
        MEDIA_MODULES += BMEDIA_PLAYER BFILE BFILE_CACHE BFILE_MEDIA B_MEDIA_NAV_PLAYER BASF_PLAYER BAVI_PLAYER
    endif
    # bmedia sources are already built into libnexus.so
    #B_PLAYBACK_IP_LIB_SOURCES += $(sort $(foreach module, $(MEDIA_MODULES), $($(module)_SOURCES)))
endif

ifeq ($(IP_STREAMER_DEMO_MODE),y)
    B_PLAYBACK_IP_LIB_DEFINES += IP_STREAMER_DEMO_MODE
endif

ifeq ($(PLAYBACK_IP_GCB_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += PLAYBACK_IP_GCB_SUPPORT
endif

#
# Define public include path(s) for this library
#
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_PLAYBACK_IP_LIB_TOP)/../os/include $(B_PLAYBACK_IP_LIB_TOP)/../os/include/linuxuser
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/netaccel/include
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/api/include
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/utils
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/rtp
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/tspsi
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/mpeg2_ts_parse
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/bfile
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/media
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/asf
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../BSEAV/lib/media/flv
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(BAVI_INCLUDES)
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/file/include/priv
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(BLIVE_EXT_DIR)/include
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_LIVEMEDIA_INCLUDES)
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../magnum/commonutils/lst
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/../magnum/commonutils/vlc
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(NEXUS_TOP)/base/include
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_PLAYBACK_IP_LIB_TOP)/include

ifeq ($(HTTP_MP4_SUPPORT),y)
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(sort $(foreach module, $(MEDIA_MODULES), $($(module)_INCLUDES)))
endif

#
# Define private include path(s) for this library
#
B_PLAYBACK_IP_LIB_PRIVATE_INCLUDES += $(B_PLAYBACK_IP_LIB_TOP)/src \
                             $(B_PLAYBACK_IP_LIB_TOP)/src/$(SAMPLE)

#
# Define other libs that this library is dependent upon.
# This will check for cyclic dependencies and setup include paths
#
B_PLAYBACK_IP_LIB_DEPENDENCIES := B_OS_LIB

#  Temporary: Eventually, B_REFSW_EXTRA_INCLUDE_DIRS needs to be handled system-wide.
B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_REFSW_EXTRA_INCLUDE_DIRS)

#
# Define extra C definitions
#
#B_PLAYBACK_IP_LIB_DEFINES += ## N/A for this library
ifeq ($(NETACCEL_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_NETACCEL
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_NETACCEL_SUPPORT
endif

ifneq ($(LIVEMEDIA_SUPPORT),n)
    B_PLAYBACK_IP_LIB_DEFINES += LIVEMEDIA_SUPPORT
endif

ifeq ($(B_PLAYBACK_IP_XML_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_PLAYBACK_IP_XML_SUPPORT
endif

ifeq ($(B_HAS_PLAYPUMP_IP),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_PLAYPUMP_IP
endif

ifeq ($(MEDIA_ASF_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_ASF
endif

ifeq ($(MEDIA_FLV_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_FLV
endif

ifeq ($(MEDIA_AVI_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_HAS_AVI
    #B_PLAYBACK_IP_LIB_SOURCES += $(sort $(BAVI_PROBE_SOURCES))
endif

ifeq ($(USE_HTTP_CHUNK_ENCODING),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_USE_HTTP_CHUNK_ENCODING
endif

ifeq ($(USE_HTTP_KEEPALIVE),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_USE_HTTP_KEEPALIVE
endif

ifeq ($(USE_CUSTOM_DLNA_PROTOCOL_1),y)
    B_PLAYBACK_IP_LIB_DEFINES += B_USE_CUSTOM_DLNA_PROTOCOL_1
endif

ifeq ($(RVU_SUPPORT),y)
    B_PLAYBACK_IP_LIB_DEFINES += RVU_SUPPORT
endif

ifeq ($(HLS_HARDWARE_DECRYPTION),y)
    B_PLAYBACK_IP_LIB_DEFINES += HW_DEC
endif

B_PLAYBACK_IP_LIB_DEFINES += $(sort $(foreach module, $(MEDIA_MODULES), $($(module)_DEFINES)))
#
# Define possible options - this is only for debug, it does _not_ actually define them.
#
#B_PLAYBACK_IP_LIB_OPTIONS += ## N/A for this library

#
# PLAYBACK_IP_CFLAGS and PLAYBACK_IP_LDFLAGS
#

PLAYBACK_IP_LDFLAGS = -L$(NEXUS_BIN_DIR) -lb_playback_ip -lnexus -lb_os

ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
    include $(NEXUS_TOP)/lib/dtcp_ip/dtcp_ip_lib.inc

    PLAYBACK_IP_LDFLAGS += $(B_DTCP_IP_LIB_LDFLAGS)
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_DTCP_IP_LIB_PUBLIC_INCLUDES)
    B_PLAYBACK_IP_LIB_DEFINES += $(B_DTCP_IP_LIB_DEFINES)
endif #DTCP_IP_SUPPORT

ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
    ifneq ($(SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT is ON, please enable SAGE_SUPPORT)
    endif
    include $(NEXUS_TOP)/lib/dtcp_ip_sage/dtcp_ip_lib.inc

    PLAYBACK_IP_LDFLAGS += $(B_DTCP_IP_LIB_LDFLAGS)
    B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_DTCP_IP_LIB_PUBLIC_INCLUDES)
    B_PLAYBACK_IP_LIB_DEFINES += $(B_DTCP_IP_LIB_DEFINES)
endif #DTCP_IP_SAGE_SUPPORT

ifeq ($(SSL_SUPPORT),y)
    PLAYBACK_IP_LDFLAGS +=  -ldl
    ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
        include $(NEXUS_TOP)/../BSEAV/opensource/openssl/openssl.inc
        B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES += $(B_OPENSSL_INC_INSTALL_DIR)
        PLAYBACK_IP_LDFLAGS +=  $(B_OPENSSL_LDFLAGS)
    endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
endif # SSL_SUPPORT

PLAYBACK_IP_CFLAGS = $(addprefix -I,  \
        $(foreach thispath,$(B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES),$(abspath ${thispath})))  \
        $(addprefix -D,$(B_PLAYBACK_IP_LIB_DEFINES))

#  Temporary: Eventually, B_REFSW_EXTRA_LIB_DIRS and B_REFSW_EXTRA_LIBS need to be handled system-wide.
PLAYBACK_IP_LDFLAGS += $(addprefix -L,$(B_REFSW_EXTRA_LIB_DIRS))
PLAYBACK_IP_LDFLAGS += $(addprefix -l,$(B_REFSW_EXTRA_LIBS))

PLAYBACK_IP_LDFLAGS += -lm

ifneq ($(NEXUS_ANDROID_SUPPORT),y)
    PLAYBACK_IP_LDFLAGS += -lpthread
endif

ifneq ($(LIVEMEDIA_SUPPORT),n)
    PLAYBACK_IP_LDFLAGS += $(B_LIVEMEDIA_LDFLAGS)
    PLAYBACK_IP_LDFLAGS += $(BLIVE_EXT_LDFLAGS)
    PLAYBACK_IP_NEEDS_STDCPPLIB=y
    B_LINKER := $(CXX)
else #LIVEMEDIA_SUPPORT
    B_LINKER := $(CC)
endif #LIVEMEDIA_SUPPORT

ifeq ($(B_PLAYBACK_IP_XML_SUPPORT),y)      # XML parser is C++
    PLAYBACK_IP_NEEDS_STDCPPLIB=y
    B_LINKER := $(CXX)
endif #B_PLAYBACK_IP_XML_SUPPORT

ifeq ($(PLAYBACK_IP_NEEDS_STDCPPLIB),y)
    PLAYBACK_IP_LDFLAGS += -lstdc++
endif #PLAYBACK_IP_NEEDS_STDCPPLIB

endif # ifdef B_REFSW_B_PLAYBACK_IP_LIB_INC_INCLUDED
