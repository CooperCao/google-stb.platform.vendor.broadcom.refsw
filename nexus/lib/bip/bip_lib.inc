#############################################################################
# (c) 2016 Broadcom Corporation
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
#
#############################################################################
#
# Protect against multiple/circular includes...
ifndef B_REFSW_BIP_LIB_INC_INCLUDED
B_REFSW_BIP_LIB_INC_INCLUDED := 1

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

BSEAV ?= ${NEXUS_TOP}/../BSEAV

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)

B_LIB_TOP   := $(NEXUS_TOP)/lib
BIP_LIB_TOP := $(NEXUS_TOP)/lib/bip

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BIP_LIB_OBJ_TOP_DIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/

# Library names
  BIP_BASE_LIB_OUTPUT_NAME := bip_base
BIP_CLIENT_LIB_OUTPUT_NAME := bip_client
BIP_SERVER_LIB_OUTPUT_NAME := bip_server

# Each library has a "prefix" e.g., "BIP_CLIENT_LIB"
# Also, each library has a lowercase nickname used in the various make targets.
# For example, "base", "client", "server".
BIP_LIB_LIST := base client server
BIP_LIB_LIST_PLUS_ALL := base client server all

# So here is a way to map from nickname to prefix:
BIP_LIB_PREFIX_FOR_NICKNAME_base   := BIP_BASE_LIB
BIP_LIB_PREFIX_FOR_NICKNAME_client := BIP_CLIENT_LIB
BIP_LIB_PREFIX_FOR_NICKNAME_server := BIP_SERVER_LIB
BIP_LIB_PREFIX_FOR_NICKNAME_all    := BIP_ALL_LIB

# Library directories
  BIP_BASE_LIB_OBJ_DIR := $(abspath ${B_REFSW_OBJ_ROOT}/nexus/lib/$(BIP_BASE_LIB_OUTPUT_NAME))
BIP_CLIENT_LIB_OBJ_DIR := $(abspath ${B_REFSW_OBJ_ROOT}/nexus/lib/$(BIP_CLIENT_LIB_OUTPUT_NAME))
BIP_SERVER_LIB_OBJ_DIR := $(abspath ${B_REFSW_OBJ_ROOT}/nexus/lib/$(BIP_SERVER_LIB_OUTPUT_NAME))

include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
include $(BSEAV)/lib/bfile/bfile.inc

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

#
# Define other libs that this library is dependent upon.
# This will check for cyclic dependencies and setup include paths
#
# The "<lib name>_DEPENDENCIES" are reserved variable names used by
# the "nexus/lib/build/" makefiles.
#
  BIP_BASE_LIB_DEPENDENCIES := B_OS_LIB
BIP_CLIENT_LIB_DEPENDENCIES := B_OS_LIB
BIP_SERVER_LIB_DEPENDENCIES := B_OS_LIB

#
# Define source files in each library.
#
# The "<lib name>_SOURCES" are reserved variable names used by
# the "nexus/lib/build/" makefiles.
#
####  Base Library - included in all BIP builds
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_class.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_status.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_string.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_xml.cpp
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_inet.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_mem.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_fd.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_media_info.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_url.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_iochecker.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_timer.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_listener.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_socket.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_console.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_to_str.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_str_to_value.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_arb.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_file.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_http_socket.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_http_socket_state.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_http_header_list.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_http_request.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_http_response.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_atom.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_probe.c
BIP_BASE_LIB_SOURCES +=       $(BIP_LIB_TOP)/src/bip_ssl.c
BIP_BASE_LIB_SOURCES +=       $(BSEAV)/lib/mlibs/mstring.cpp
BIP_BASE_LIB_SOURCES +=       $(BSEAV)/lib/mlibs/mxmlparser.cpp
BIP_BASE_LIB_SOURCES +=       $(BSEAV)/lib/mlibs/mxmlelement.cpp
BIP_BASE_LIB_SOURCES +=       $(BSEAV)/lib/mlibs/mvoidlist.cpp
BIP_BASE_LIB_SOURCES +=       $(BSEAV)/lib/mlibs/mregexp.cpp
BIP_BASE_LIB_SOURCES +=       $(NEXUS_TOP)/utils/namevalue.c


####  Client Library - included in BIP Client builds
BIP_CLIENT_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_player.c
BIP_CLIENT_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_player_state.c
BIP_CLIENT_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_dtcp_ip_client.c


####  Server Library - included in BIP Server builds
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_streamer.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_http_server.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_dtcp_ip_server.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_http_server_state.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_http_server_socket.c
ifeq ($(VIDEO_ENCODER_SUPPORT),y)
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_transcode.c
else
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_transcode_stubs.c
endif
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_http_streamer.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_http_streamer_state.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_udp_streamer.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_udp_streamer_state.c
BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_igmp_listener.c
ifneq ($(LIVEMEDIA_SUPPORT),n)
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_listener.c
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_socket.c
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_request.c
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_response.c
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_lm_socket.cpp
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_lm_listener.cpp
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_session.c
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/bip_rtsp_lm_session.cpp
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/liveMedia/bip_rtsp_lm_sink.cpp
    BIP_SERVER_LIB_SOURCES +=     $(BIP_LIB_TOP)/src/liveMedia/bip_rtsp_lm_server.c
endif # LIVEMEDIA_SUPPORT

#
# Define private include path(s) for each library.
# These are used when compiling the BIP libraries.
#
# The "<lib name>_PRIVATE_INCLUDES" are reserved variable names used by
# the "nexus/lib/build/" makefiles.
#
# Here are separate include path lists for each BIP lib variant (BASE, CLIENT, SERVER, ALL).
  BIP_BASE_LIB_PRIVATE_INCLUDES +=
BIP_CLIENT_LIB_PRIVATE_INCLUDES +=
BIP_SERVER_LIB_PRIVATE_INCLUDES +=
   BIP_ALL_LIB_PRIVATE_INCLUDES +=

# And here is the COMMON list that will be added to each of the variants above.
BIP_COMMON_PRIVATE_INCLUDES += $(BIP_LIB_TOP)/src
BIP_COMMON_PRIVATE_INCLUDES += $(BIP_LIB_TOP)/include
BIP_COMMON_PRIVATE_INCLUDES += $(BATOM_INCLUDES)
BIP_COMMON_PRIVATE_INCLUDES += $(BMEDIA_INCLUDES)
BIP_COMMON_PRIVATE_INCLUDES += $(BFILE_INCLUDES)
BIP_COMMON_PRIVATE_INCLUDES += $(BSEAV)/lib/mlibs
BIP_COMMON_PRIVATE_INCLUDES += $(BSEAV)/lib/tspsi2
BIP_COMMON_PRIVATE_INCLUDES += $(NEXUS_TOP)/utils

#
# Define public include path(s) for this library.
# These are the include path(s) that should be used for compiling
# things (apps or other libs) that use this library.
#
# The "<lib name>_PUBLIC_INCLUDES" are reserved variable names used by
# the "nexus/lib/build/" makefiles.
#
# Here are separate include path lists for each BIP lib variant (BASE, CLIENT, SERVER, ALL).
  BIP_BASE_LIB_PUBLIC_INCLUDES +=
BIP_CLIENT_LIB_PUBLIC_INCLUDES +=
BIP_SERVER_LIB_PUBLIC_INCLUDES +=
   BIP_ALL_LIB_PUBLIC_INCLUDES +=

# And here is the COMMON list that will be added to each of the variants above.
BIP_COMMON_PUBLIC_INCLUDES += $(BIP_LIB_TOP)/include

#
# Define extra C definitions
#
# The "<lib name>_DEFINES" are reserved variable names used by
# the "nexus/lib/build/" makefiles.
#
# These BIP_XXX_LIB_DEFINES should be considered to be "private" defines...
# since they will only be defined when BIP is being compiled.
  BIP_BASE_LIB_DEFINES =
BIP_CLIENT_LIB_DEFINES =
BIP_SERVER_LIB_DEFINES =
   BIP_ALL_LIB_DEFINES =

# And here is the COMMON list that will be added to each of the variants above.
    BIP_COMMON_DEFINES =

# Alternatively, the BIP_XXX_LIB_PUBLIC_DEFINES are "public" defines...
# They will be used to create the CFLAGS used by apps calling into BIP.
#
# It is recommended that apps and libraries conditionally using the BIP libraries
# use the BIP_SUPPORT=y env variable as a build option, then use
# "#if BIP_SUPPORT" (for full BIP support) in their code.  Partial BIP support
# can be tested for by using "#if BIP_<variant>_SUPPORT", where <variant>
# is either BASE, CLIENT, or SERVER.
#
  BIP_BASE_LIB_PUBLIC_DEFINES += BIP_BASE_SUPPORT
BIP_CLIENT_LIB_PUBLIC_DEFINES += BIP_BASE_SUPPORT  BIP_CLIENT_SUPPORT
BIP_SERVER_LIB_PUBLIC_DEFINES += BIP_BASE_SUPPORT  BIP_CLIENT_SUPPORT  BIP_SERVER_SUPPORT
   BIP_ALL_LIB_PUBLIC_DEFINES += BIP_BASE_SUPPORT  BIP_CLIENT_SUPPORT  BIP_SERVER_SUPPORT  BIP_SUPPORT

# And here is the COMMON list that will be added to each of the variants above.
    BIP_COMMON_PUBLIC_DEFINES +=

# When building our libraries, if we need command line flags that aren't
# exactly "_DEFINES" or "_INCLUDES" (include paths), use these "_PRIVATE_CFLAGS"
# variables:
  BIP_BASE_LIB_PRIVATE_CFLAGS +=
BIP_CLIENT_LIB_PRIVATE_CFLAGS +=
BIP_SERVER_LIB_PRIVATE_CFLAGS +=

# And here is the COMMON list that will be added to each of the variants above.
    BIP_COMMON_PRIVATE_CFLAGS +=

# Define the (public) LDFLAGS that apps can use to link to the BIP libraries.
# Start with each variant having only its own LDFLAGS
  BIP_BASE_LIB_LDFLAGS += -L$(BIP_BASE_LIB_OBJ_DIR) -l$(BIP_BASE_LIB_OUTPUT_NAME)

BIP_CLIENT_LIB_LDFLAGS += -L$(BIP_CLIENT_LIB_OBJ_DIR) -l$(BIP_CLIENT_LIB_OUTPUT_NAME)
BIP_CLIENT_LIB_LDFLAGS += -L$(BIP_BASE_LIB_OBJ_DIR)   -l$(BIP_BASE_LIB_OUTPUT_NAME)

BIP_SERVER_LIB_LDFLAGS += -L$(BIP_SERVER_LIB_OBJ_DIR) -l$(BIP_SERVER_LIB_OUTPUT_NAME)
BIP_SERVER_LIB_LDFLAGS += -L$(BIP_CLIENT_LIB_OBJ_DIR) -l$(BIP_CLIENT_LIB_OUTPUT_NAME)  # BIP Server needs Client for IP streamer input
BIP_SERVER_LIB_LDFLAGS += -L$(BIP_BASE_LIB_OBJ_DIR) -l$(BIP_BASE_LIB_OUTPUT_NAME)

   BIP_ALL_LIB_LDFLAGS += -L$(BIP_CLIENT_LIB_OBJ_DIR) -l$(BIP_CLIENT_LIB_OUTPUT_NAME)
   BIP_ALL_LIB_LDFLAGS += -L$(BIP_SERVER_LIB_OBJ_DIR) -l$(BIP_SERVER_LIB_OUTPUT_NAME)
   BIP_ALL_LIB_LDFLAGS += -L$(BIP_BASE_LIB_OBJ_DIR) -l$(BIP_BASE_LIB_OUTPUT_NAME)

# These COMMON_LDFLAGS will be added to the LDFLAGS for all BIP lib variants.
BIP_COMMON_LDFLAGS += $(addprefix -L,$(B_REFSW_EXTRA_LIB_DIRS))  $(addprefix -l,$(B_REFSW_EXTRA_LIBS))
# BIP_COMMON_LDFLAGS += -lstdc++

#
# Define other libs that this library is dependent upon (again),
# this time using the target nicknames used in the nexus/lib/bip/build/Makefile.
#
# These XXX_MAYBE variables will either contain the name of a sub-library, or be empty if that
# sublibrary is not being used.
BIP_OPENSSL_LIB_MAYBE := openssl
ifeq ($(SSL_SUPPORT),n)
    BIP_OPENSSL_LIB_MAYBE :=
endif
ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
    BIP_OPENSSL_LIB_MAYBE :=
endif

# Since "server" uses "client", it needs all of client's sublibs.
  BIP_BASE_LIB_SUBLIBS := b_os  livemedia             tshdrbuilder  psip  $(BIP_OPENSSL_LIB_MAYBE)  playback_ip dtcp_ip
BIP_CLIENT_LIB_SUBLIBS := b_os  livemedia  blive_ext  tshdrbuilder  psip  $(BIP_OPENSSL_LIB_MAYBE)  playback_ip dtcp_ip
BIP_SERVER_LIB_SUBLIBS := b_os  livemedia  blive_ext  tshdrbuilder  psip  $(BIP_OPENSSL_LIB_MAYBE)  playback_ip dtcp_ip
   BIP_ALL_LIB_SUBLIBS := b_os  livemedia  blive_ext  tshdrbuilder  psip  $(BIP_OPENSSL_LIB_MAYBE)  playback_ip dtcp_ip

# Define the nicknames of the libraries that aren't cleaned by the normal
# "clean" target... but require a "make distclean" to clean.
BIP_LIB_DISTCLEAN_SUBLIBS := $(BIP_OPENSSL_LIB_MAYBE) livemedia

##########################################################################
#  Define a make function to add a library's LDFLAGS to each of the BIP
#  component LDFLAGS variables, depending on whether that component
#  needs the sub-library.
#
#  For example, it will create/modify make variables named something like:
#      BIP_BASE_LIB_LDFLAGS  or BIP_SERVER_LIB_LDFLAGS as long as that
#      BIP library depends on the specified sub-library.
#
#  Calling sequence:
#    $(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB,<sub-library>,<libraryldflags>)
##########################################################################
    BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB =                                            \
        $(foreach lib,$(BIP_LIB_LIST_PLUS_ALL),                                          \
            $(eval $(BIP_LIB_PREFIX_FOR_NICKNAME_$(lib))_LDFLAGS +=                      \
                $(if                                                                     \
                    $(findstring $(1),$($(BIP_LIB_PREFIX_FOR_NICKNAME_$(lib))_SUBLIBS)   \
                    )                                                                    \
                  ,$(2),                                                                 \
                 )                                                                       \
             )                                                                           \
         )

##########################################################################
#  Define a make function to remove duplicate "-L" (library path) entries
#  from a list of LDFLAGS.  Anything else in the list gets kept.
#
#  Calling sequence:
#    $(call BIP_LIB_MAKEFUNC_RM_DUPES,<make-variable-name>)
##########################################################################
BIP_LIB_MAKEFUNC_RM_DUPES =$(strip                  \
    $(eval $(1)__RM_DUPES=)                         \
    $(foreach arg,$($(1)),                          \
        $(if $(filter -L%,$(arg)),                  \
            $(if $(filter $(arg),$($(1)__RM_DUPES)),\
            ,                                       \
                $(eval $(1)__RM_DUPES += $(arg))    \
            )                                       \
        ,                                           \
            $(eval $(1)__RM_DUPES += $(arg))        \
        )                                           \
    )                                               \
    $(eval $(1) = $($(1)__RM_DUPES))                \
    $(eval $(1)__RM_DUPES =)                        \
)

#  Include definitions for libraries used by BIP
#  The following will define the BIP_<COMPONENT>_LIB_LDFLAGS as
#  appropriate for each BIP component.

#  Start by including Nexus' platform_app.inc here because the following inc's depend on it.
ifeq ($(NEXUS_CFLAGS),)
    include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

# Be sure to use NXCLIENT_CFLAGS so BIP code knows when it has NXCLIENT_SUPPORT.
ifeq ($(NXCLIENT_SUPPORT),y)
    ifeq ($(NXCLIENT_CFLAGS),)
        # Nexus NxClient Mode: NXCLIENT_SUPPORT=y, NEXUS_CLIENT_SUPPORT=<don't care>
        include $(NEXUS_TOP)/nxclient/include/nxclient.inc
        BIP_COMMON_PRIVATE_CFLAGS += ${NXCLIENT_CFLAGS}
    endif
endif

# Library: playback_ip
#  We need to filter-out the "-lnexus" LDFLAG from playback_ip because we want the
#  App's makefile to decide which nexus libraries to link to.
include ${NEXUS_TOP}/lib/playback_ip/b_playback_ip_lib.inc
BIP_COMMON_PRIVATE_INCLUDES += $(B_PLAYBACK_IP_LIB_PUBLIC_INCLUDES)
BIP_COMMON_DEFINES += $(B_PLAYBACK_IP_LIB_DEFINES)
$(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,playback_ip, $(filter-out -lnexus,$(PLAYBACK_IP_LDFLAGS)))

# Library: livemedia
ifneq ($(LIVEMEDIA_SUPPORT),n)
    # Library: blive_ext
    include ${NEXUS_TOP}/../BSEAV/lib/livemedia/blive_ext/include/blive_ext.mak
    BIP_COMMON_PRIVATE_CFLAGS += ${BLIVE_EXT_CFLAGS}
    $(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,blive_ext, ${BLIVE_EXT_LDFLAGS})

    include ${NEXUS_TOP}/../BSEAV/lib/livemedia/livemedia.inc
    BIP_COMMON_PRIVATE_CFLAGS += ${B_LIVEMEDIA_CFLAGS}
    BIP_COMMON_DEFINES += $(B_LIVEMEDIA_DEFINES)
    $(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,livemedia, ${B_LIVEMEDIA_LDFLAGS})

endif # LIVEMEDIA_SUPPORT

# Library: dtcp_ip
# Start by sorting out differences for DTCP_IP_SUPPORT and DTCP_IP_SAGE_SUPPORT.
BIP_DTCP_IP_NEEDS_OPENSSL :=
BIP_DTCP_IP_LIB_DIR_NAME := dtcp_ip

ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)        # DTCP_IP_SAGE_SUPPORT enabled and DTCP_IP_SUPPORT enabled
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    else                               # DTCP_IP_SAGE_SUPPORT enabled and DTCP_IP_SUPPORT disabled
        BIP_DTCP_IP_LIB_DIR_NAME := dtcp_ip_sage
    endif
else
    ifeq ($(DTCP_IP_SUPPORT),y)        # DTCP_IP_SAGE_SUPPORT disabled and DTCP_IP_SUPPORT enabled
        BIP_DTCP_IP_NEEDS_OPENSSL := y
    else                               # DTCP_IP_SAGE_SUPPORT disabled and DTCP_IP_SUPPORT disabled
                                       #     Use dtcp_ip (stub version) which doesn't need openssl
    endif
endif

# Library: nexus
#  We won't deal with nexus or nxclient libraries.  The App's makefile
#  is responsible for building those and using the proper CFLAGS and LDFLAGS.

# Library: openssl
ifneq ($(SSL_SUPPORT),n)
    include ${NEXUS_TOP}/../BSEAV/lib/openssl/openssl.inc
    BIP_COMMON_PRIVATE_CFLAGS += $(B_OPENSSL_CFLAGS)
    $(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,openssl, $(B_OPENSSL_LDFLAGS))
endif # SSL_SUPPORT

include $(NEXUS_TOP)/lib/$(BIP_DTCP_IP_LIB_DIR_NAME)/dtcp_ip_lib.inc
BIP_COMMON_PUBLIC_INCLUDES += $(B_DTCP_IP_LIB_PUBLIC_INCLUDES)
BIP_COMMON_PUBLIC_INCLUDES += $(BSEAV)/lib/security/bcrypt/include
BIP_COMMON_DEFINES += $(B_DTCP_IP_LIB_DEFINES)
$(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,dtcp_ip, $(B_DTCP_IP_LIB_LDFLAGS))

# Library: tshdrbuilder
include ${NEXUS_TOP}/../BSEAV/lib/tshdrbuilder/tshdrbuilder.inc
BIP_COMMON_PRIVATE_INCLUDES += $(TSHDRBUILDER_INCLUDES)
$(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,tshdrbuilder, -ltshdrbuilder)

# Library: psip
B_PSIP_TOP := ${NEXUS_TOP}/../rockford/lib/psip
include ${NEXUS_TOP}/../rockford/lib/psip/b_psip_lib.inc
BIP_COMMON_PRIVATE_INCLUDES += $(B_PSIP_LIB_PUBLIC_INCLUDES)
$(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,psip, -lb_psip)

# Library: b_os
include ${NEXUS_TOP}/lib/os/b_os_lib.inc
BIP_COMMON_PRIVATE_INCLUDES += $(B_OS_LIB_PUBLIC_INCLUDES)
$(call BIP_LIB_MAKEFUNC_SET_LDFLAGS_FOR_SUBLIB ,b_os, -L$(NEXUS_BIN_DIR) -lb_os)

#
# Apply the COMMON sets to the variant-specific sets.
#
# Apply the COMMON_PRIVATE_INCLUDES to each BIP lib variant.
  BIP_BASE_LIB_PRIVATE_INCLUDES += $(BIP_COMMON_PRIVATE_INCLUDES)
BIP_CLIENT_LIB_PRIVATE_INCLUDES += $(BIP_COMMON_PRIVATE_INCLUDES)
BIP_SERVER_LIB_PRIVATE_INCLUDES += $(BIP_COMMON_PRIVATE_INCLUDES)
   BIP_ALL_LIB_PRIVATE_INCLUDES += $(BIP_COMMON_PRIVATE_INCLUDES)

# Apply the COMMON_PUBLIC_INCLUDES to each BIP lib variant.
  BIP_BASE_LIB_PUBLIC_INCLUDES += $(BIP_COMMON_PUBLIC_INCLUDES)
BIP_CLIENT_LIB_PUBLIC_INCLUDES += $(BIP_COMMON_PUBLIC_INCLUDES)
BIP_SERVER_LIB_PUBLIC_INCLUDES += $(BIP_COMMON_PUBLIC_INCLUDES)
   BIP_ALL_LIB_PUBLIC_INCLUDES += $(BIP_COMMON_PUBLIC_INCLUDES)

# Apply the COMMON_DEFINES (private defines) to each BIP lib variant.
  BIP_BASE_LIB_DEFINES += $(BIP_COMMON_DEFINES)
BIP_CLIENT_LIB_DEFINES += $(BIP_COMMON_DEFINES)
BIP_SERVER_LIB_DEFINES += $(BIP_COMMON_DEFINES)

# Apply the COMMON_PUBLIC_DEFINES to each BIP lib variant.
  BIP_BASE_LIB_PUBLIC_DEFINES += $(BIP_COMMON_PUBLIC_DEFINES)
BIP_CLIENT_LIB_PUBLIC_DEFINES += $(BIP_COMMON_PUBLIC_DEFINES)
BIP_SERVER_LIB_PUBLIC_DEFINES += $(BIP_COMMON_PUBLIC_DEFINES)
   BIP_ALL_LIB_PUBLIC_DEFINES += $(BIP_COMMON_PUBLIC_DEFINES)

# Apply the COMMON_PRIVATE_CFLAGS to each BIP lib variant.
  BIP_BASE_LIB_PRIVATE_CFLAGS += $(BIP_COMMON_PRIVATE_CFLAGS)
BIP_CLIENT_LIB_PRIVATE_CFLAGS += $(BIP_COMMON_PRIVATE_CFLAGS)
BIP_SERVER_LIB_PRIVATE_CFLAGS += $(BIP_COMMON_PRIVATE_CFLAGS)

# Apply the COMMON_PUBLIC_LDFLAGS to each BIP lib variant.
  BIP_BASE_LIB_LDFLAGS += $(BIP_COMMON_LDFLAGS)
BIP_CLIENT_LIB_LDFLAGS += $(BIP_COMMON_LDFLAGS)
BIP_SERVER_LIB_LDFLAGS += $(BIP_COMMON_LDFLAGS)
   BIP_ALL_LIB_LDFLAGS += $(BIP_COMMON_LDFLAGS)

# Now remove any duplicates "-L" (library path) entries from the LDFLAGS lists.
$(call BIP_LIB_MAKEFUNC_RM_DUPES ,BIP_BASE_LIB_LDFLAGS)
$(call BIP_LIB_MAKEFUNC_RM_DUPES ,BIP_CLIENT_LIB_LDFLAGS)
$(call BIP_LIB_MAKEFUNC_RM_DUPES ,BIP_SERVER_LIB_LDFLAGS)
$(call BIP_LIB_MAKEFUNC_RM_DUPES ,BIP_ALL_LIB_LDFLAGS)

ifeq ($(B_REFSW_ANDROID),y)
LDFLAGS += $(ANDROID_LIB)/crtbegin_so.o $(ANDROID_LIB)/crtend_so.o
endif

# Construct (public) CFLAGS for the app to use.
  BIP_BASE_LIB_CFLAGS += $(addprefix -I,$(BIP_BASE_LIB_PUBLIC_INCLUDES))   $(addprefix -D,$(BIP_BASE_LIB_PUBLIC_DEFINES))
BIP_CLIENT_LIB_CFLAGS += $(addprefix -I,$(BIP_CLIENT_LIB_PUBLIC_INCLUDES)) $(addprefix -D,$(BIP_CLIENT_LIB_PUBLIC_DEFINES))
BIP_SERVER_LIB_CFLAGS += $(addprefix -I,$(BIP_SERVER_LIB_PUBLIC_INCLUDES)) $(addprefix -D,$(BIP_SERVER_LIB_PUBLIC_DEFINES))
   BIP_ALL_LIB_CFLAGS += $(addprefix -I,$(BIP_ALL_LIB_PUBLIC_INCLUDES))    $(addprefix -D,$(BIP_ALL_LIB_PUBLIC_DEFINES))

# Uncomment the next line to print out these variables for debugging...
# BIP_LIB_DEBUG=1
ifneq ($(BIP_LIB_DEBUG),)
    $(foreach i,                        \
    NEXUS_LDFLAGS                       \
    NXCLIENT_LDFLAGS                    \
    NEXUS_LD_LIBRARIES                  \
    NEXUS_CLIENT_LD_LIBRARIES           \
    B_PLAYBACK_IP_LIB_DEFINES           \
    PLAYBACK_IP_LDFLAGS                 \
    BIP_BASE_LIB_CFLAGS                 \
    BIP_CLIENT_LIB_CFLAGS               \
    BIP_SERVER_LIB_CFLAGS               \
    BIP_ALL_LIB_CFLAGS                  \
    BIP_BASE_LIB_LDFLAGS                \
    BIP_CLIENT_LIB_LDFLAGS              \
    BIP_SERVER_LIB_LDFLAGS              \
    BIP_ALL_LIB_LDFLAGS                 \
    ,                                   \
        $(info )                        \
        $(info $i: **************)      \
        $(foreach j,$($(i)),            \
            $(info $(j))                \
         )                              \
    )
endif

# How application Makefiles should use these CFLAGS and LDFLAGS:
# BIP comes as three component libraries (BASE, CLIENT, SERVER) which
# can be used in four combinations:
#    BASE (base library): Basic network support (e.g., BIP_Socket), BIP utility classes (e.g., BIP_Strings).
#    CLIENT (base+client libs): Client side (player) functionality.
#    SERVER (base+server libs): Server side (streamer) functionality (e.g., BIP_Http_Server).
#    ALL (base+client+server libs): Full (client and server) functionality.
#
# So choose what level your application needs, then use only the CFLAGS and LDFLAGS that are named
# for that level.
#
# For example, for full BIP functionality, the application's Makefile should include:
#    CFLAGS  += BIP_ALL_LIB_CFLAGS
#    LDFLAGS += BIP_ALL_LIB_LDFLAGS
#
endif # ifdef B_REFSW_BIP_LIB_INC_INCLUDED
