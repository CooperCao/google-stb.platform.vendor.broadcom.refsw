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

#Identify this library
#
B_LIBS += B_DTCP_IP_LIB

#
#Define source files in this library
#

B_DTCP_IP_VENDOR_LIB_SOURCES = drm_dtcp_ip_vendor.c

include $(B_LIB_TOP)/../../BSEAV/opensource/openssl/openssl.inc
#
#Define public include path for this library
#
B_DTCP_IP_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include \
						$(B_LIB_TOP)/os/include/linuxuser \
						$(B_OPENSSL_INC_INSTALL_DIR) \
						$(B_LIB_TOP)/../../BSEAV/lib/security/bcrypt/include \

#
#Define private include path(s) for this library
#

#
#Defines extra C definations, These are from original DTCP package.
#NOTE: the DTCP_IP_HARDWARE_DECRYPTION need to be defined, if you wan to use
#Broadcom's M2M hardware to decrypt DTCP content stream.
#
B_DTCP_IP_LIB_DEFINES += LINUX_BUILD _POSIX \
								__cdecl="" \
								__stdcall=""


# Using common DRM for content encryption/decryption
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT


B_DTCP_IP_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../../BSEAV/lib/security/common_drm/include \
                                 $(B_LIB_TOP)/../../BSEAV/lib/security/third_party/dtcp_ip \
                                 $(B_LIB_TOP)/../../BSEAV/lib/security/common_crypto/include \
                                 $(B_LIB_TOP)/../../BSEAV/lib/drmrootfs

#build with legacy production key provisioning if requested.
ifeq ($(DTCP_IP_LEGACY_PRODUCTION_KEY), y)
B_DTCP_IP_LIB_DEFINES += B_DTCP_IP_LEGACY_PRODUCTION_KEY
B_DTCP_IP_LIB_SOURCES += $(DTCP_TOP_DIR)/src/decrypto.c

B_DTCP_IP_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/../../nexus/modules/security/$(BCHP_CHIP)/include \
								$(B_LIB_TOP)/../../nexus/extensions/security/keyladder/$(BCHP_CHIP)/include
endif
