############################################################
#     Copyright (c) 2015 Broadcom Corporation
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
#   RMFF media library module descriptor
#
# Revision History:
#
# $brcm_Log: $
#
############################################################


# COMMON CRYPTO module

# Do not extend COMMON_CRYPTO_INCLUDES with manual paths into Nexus.
# Instead, whoever compiles common_crypto must already have the required Nexus module paths set.
# This allows Nexus to prevent circular calling dependencies.
COMMON_CRYPTO_INCLUDES := ${BSEAV}/lib/security/common_crypto/include

########################################
# Determine core size and ZEUS version
########################################
include ${NEXUS_TOP}/modules/security/security_defs.inc
#$(info NEXUS_SECURITY_ZEUS_VERSION_MAJOR=$(NEXUS_SECURITY_ZEUS_VERSION_MAJOR) NEXUS_SECURITY_ZEUS_VERSION_MINOR=$(NEXUS_SECURITY_ZEUS_VERSION_MINOR))

########################################
# Core 65nm
########################################
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),0)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_65NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=0
    COMMON_CRYPTO_INCLUDES += ${NEXUS_TOP}/extensions/security/keyladder/include/65nm
else
    COMMON_CRYPTO_INCLUDES += ${NEXUS_TOP}/extensions/security/keyladder/include/40nm
endif

########################################
# Core 40nm
########################################
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),1)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=10
endif

ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),2)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1

    ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),0)
        NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=20
    else
        ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),2)
            NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=22
        endif
    endif
endif

ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),3)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=30
endif

########################################
# Core 28nm
########################################
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),4)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_28NM_CHIP=1

    ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),1)
        NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=41
    else
        ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),2)
            NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=42
        endif
    endif
endif

COMMON_CRYPTO_INCLUDES += ${NEXUS_TOP}/extensions/security/securersa/include
COMMON_CRYPTO_SOURCES :=  ${BSEAV}/lib/security/common_crypto/src/common_crypto.c

ifeq ($(SECURERSA_SUPPORT), y)
    COMMON_CRYPTO_SECURERSA := $(shell test -e ${BSEAV}/lib/security/common_crypto/src/common_crypto_securersa.c && echo y)
    ifeq ($(COMMON_CRYPTO_SECURERSA),y)
        $(warning Compiling Common Crypto Secure RSA module into Nexus)
        COMMON_CRYPTO_SOURCES +=  ${BSEAV}/lib/security/common_crypto/src/common_crypto_securersa.c
    endif
endif
