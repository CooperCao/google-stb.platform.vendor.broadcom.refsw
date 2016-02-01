############################################################
#     Copyright (c) 2014 Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#   Livemedia library makefile include fragment
#
# Revision History:
#
# $brcm_Log: $
#
############################################################

#  Check for required definitions.
$(if ${NEXUS_PLATFORM},,$(error NEXUS_PLATFORM is not defined))
$(if ${NEXUS_TOP},,$(error NEXUS_TOP is not defined))
$(if ${B_REFSW_ARCH},,$(error B_REFSW_ARCH is not defined))

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

#  Define Livemedia's Makefile directory... probably the same dir that this file is in.
B_LIVEMEDIA_BUILD_DIR := $(abspath $(NEXUS_TOP)/../BSEAV/lib/livemedia)

ifneq ($(filter $(B_REFSW_DEBUG),n no_error_messages),)
    DEBUG_SUFFIX=release
else
    DEBUG_SUFFIX=debug
endif

#  Define the Livemedia out-of-source directory tree.
B_LIVEMEDIA_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/livemedia/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

#  List any compiler defines here.
B_LIVEMEDIA_DEFINES :=

B_LIVEMEDIA_STAGED_INSTALL_DIR := ${B_LIVEMEDIA_DIR}/staged_install

#  List directories that contain include files.
B_LIVEMEDIA_INCLUDES := $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/liveMedia             \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/groupsock             \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/UsageEnvironment      \
                        $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/include/BasicUsageEnvironment

#  Assemble compiler defines and include directories into CFLAGS.
B_LIVEMEDIA_CFLAGS := $(addprefix -I,$(B_LIVEMEDIA_INCLUDES))

#  List names of libraries.
B_LIVEMEDIA_LIBS := liveMedia               \
                    groupsock               \
                    UsageEnvironment        \
                    BasicUsageEnvironment

#  List directories that contain the libraries.
B_LIVEMEDIA_LIB_DIRS := $(B_LIVEMEDIA_STAGED_INSTALL_DIR)/lib

#  Assemble libraries and their directories into LDFLAGS.
B_LIVEMEDIA_LDFLAGS := $(addprefix -L,$(B_LIVEMEDIA_LIB_DIRS))   $(addprefix -l,$(B_LIVEMEDIA_LIBS))
