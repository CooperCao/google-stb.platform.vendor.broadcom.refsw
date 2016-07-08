#############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += SECURITY

##################
# Module Options #
##################


###################
# Module Features #
###################

include ${NEXUS_TOP}/modules/security/security_defs.inc

NEXUS_SECURITY_EXTENSION := $(NEXUS_TOP)/extensions/security
BHSM_SRC                 := $(MAGNUM)/portinginterface/hsm/src
BSP_INCLUDE              := $(MAGNUM)/portinginterface/hsm/include/aegis/${ZEUSVER}

# Compile-time variables to cover specific features


###################
# Module Includes #
###################

NEXUS_SECURITY_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/security/include

##################
# Module Sources #
##################
ifneq ($(HSM_SOURCE_AVAILABLE),n)
  ifeq ($(filter $(CORETECH), core65nmASKM core65nm), $(CORETECH))
    NEXUS_SECURITY_SOURCES := $(NEXUS_TOP)/modules/security/src/nexus_security_65nm.c
  else
    NEXUS_SECURITY_SOURCES := $(NEXUS_TOP)/modules/security/src/nexus_security.c
  endif
  NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/src/nexus_security_misc.c
  ifeq ($(filter $(CORETECH), core40nm core28nm ),$(CORETECH))
    NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/src/nexus_security_regver.c
    NEXUS_SECURITY_FW_SIGN := y
  endif
endif

ifeq ($(NEXUS_SECURITY_FW_SIGN),y)
# use presence of key2 file to determine whether to generate fw signatures
ifneq ($(wildcard $(NEXUS_TOP)/../BSEAV/lib/security/fw_img_sign/key2/$(BCHP_CHIP)_nexus_security_regver_key.h),)
NEXUS_SECURITY_REGVER_SIGNATURES := $(NEXUS_BUILD_DIR)/nexus_security_regver_signatures.inc
NEXUS_SECURITY_REGVER_SIGNATURES_STUB := $(NEXUS_TOP)/modules/security/src/nexus_security_regver_signatures_stub.c
NEXUS_SECURITY_REGVER_KEY := $(NEXUS_BUILD_DIR)/nexus_security_regver_key.inc
NEXUS_SECURITY_REGVER_KEY_STUB := $(NEXUS_TOP)/modules/security/src/nexus_security_regver_key_stub.c
NEXUS_SECURITY_PRIVATE_INCLUDES += $(NEXUS_BUILD_DIR)
NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_FW_SIGN=1
endif
endif

# thunk layer
NEXUS_SECURITY_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/security/src $(NEXUS_SYNCTHUNK_DIR)

NEXUS_SECURITY_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_security_thunks.c


#######################
# Module Dependencies #
#######################
NEXUS_SECURITY_DEPENDENCIES := BASE CORE TRANSPORT

##################
# Magnum Modules #
##################
CUST ?= GENERIC3A
BHSM_OTPMSP ?=OFF
BHSM_IPTV ?=OFF
BHSM_SECURE_RSA ?=OFF
BHSM_SECURE_ACCESS ?=OFF
BHSM_IP_LICENSING ?=OFF
BHSM_AV_CMD ?=OFF
BHSM_BSECK ?= OFF
BHSM_GEN_ROOT_KEY ?=OFF
BHSM_NSKCMD ?=OFF
BHSM_SECURE_PKL ?=OFF
AVKEYLADDER_SUPPORT ?= n


NEXUS_SECURITY_MAGNUM_MODULES = $(MAGNUM)/portinginterface/hsm/bhsm.inc

NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ZEUS_VERSION_MAJOR=$(NEXUS_SECURITY_ZEUS_VERSION_MAJOR)
NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ZEUS_VERSION_MINOR=$(NEXUS_SECURITY_ZEUS_VERSION_MINOR)

ifeq ($(filter $(CORETECH), core65nmASKM core40nm core28nm), $(CORETECH))
    # Make these default for 65-nm ASKM, 40-nm, and 26-nm chips
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_EXT_KEY_IV
endif

ifeq ($(MSDRM_PD_SUPPORT),y)
    NEXUS_SECURITY_DEFINES += MSDRM_PD_SUPPORT=1
endif

ifeq ($(BSP_DS2_KEY_ROUTING),ON)
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_DS2_ROUTING
endif

ifeq ($(BSP_M2M_EXT_KEY_IV_SUPPORT),ON)
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_EXT_KEY_IV
endif

ifeq ($(NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE),y)
    NEXUS_SECURITY_DEFINES += NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE=1
endif
ifeq ($(NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW),y)
    NEXUS_SECURITY_DEFINES += NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW=1
endif

#####################
# Module Extensions #
#####################
#
# We should NOT rely on B_REFSW_OS for our build, since Nexus build scripts
# switch this between "linuxkernel" and "linuxuser" when building kernel mode Nexus libraries.

ifneq ($(filter $(NEXUS_MODE),proxy driver),)
    SEC_BUILD_MODE := linuxkernel
    SEC_OBJ_EXT := o
else
    SEC_BUILD_MODE := linuxuser
    SEC_OBJ_EXT := lo
endif


ifeq ($(filter $(CORETECH), core65nmASKM core40nm core28nm ),$(CORETECH))
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_HAS_ASKM=1
    #$(warning NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_HAS_ASKM=1)
endif

ifeq ($(CORETECH),core28nm)
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=28
    #$(warning NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=28)
else
    ifeq ($(CORETECH),core40nm)
        NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=40
        #$(warning NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=40)
    else
        ifeq ($(CORETECH),core65nmASKM)
            NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=65
            #$(warning NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_CHIP_SIZE=65)
        endif
    endif
endif

ifneq ($(NEXUS_EXTRA_SECLIBS_FILES),)
    ifeq ($(ZEUSVER),Zeus42)
        ## temporary support for Linux 3.8 security libs.
        LINUX_VERSION := $(shell grep -m 1 '^VERSION = ' ${LINUX}/Makefile | awk '{print $$3}')
        LINUX_PATCHLEVEL := $(shell grep -m 1 '^PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
        ifeq ($(LINUX_VERSION),3)
            ifeq ($(LINUX_PATCHLEVEL),8)
                ZEUS_DIR_SUPPLEMENT ?= _for_linux_3_8
                #$(warning Selecting Zeus4.2 security libraries from ${ZEUSVER}${ZEUS_DIR_SUPPLEMENT} )
            endif
        endif
    endif

    ifneq ($(B_REFSW_DEBUG),y)
        SEC_LIB_MODE := retail
    else
        SEC_LIB_MODE := debug
    endif

    # Setup NEXUS_EXTRA_SECLIBS_DIR for Android and non-Android usage
    ifeq ($(ANDROID_BUILD),y)
        # Android case
        NEXUS_EXTRA_SECLIBS_DIR ?= ${NEXUS_TOP}/modules/security/lib/${ZEUSVER}/${SEC_BUILD_MODE}/${SEC_LIB_MODE}
        $(info NEXUS_EXTRA_SECLIBS_DIR is $(NEXUS_EXTRA_SECLIBS_DIR))
    else
        # Mainstream (non-Android) case
        NEXUS_EXTRA_SECLIBS_DIR ?= ${NEXUS_TOP}/modules/security/lib/${ZEUSVER}${ZEUS_DIR_SUPPLEMENT}/${SEC_BUILD_MODE}/${SEC_LIB_MODE}
    endif

    NEXUS_EXTRALIBS += ${NEXUS_EXTRA_SECLIBS_DIR}/${NEXUS_EXTRA_SECLIBS_FILES}

endif  ##($(NEXUS_EXTRA_SECLIBS_FILES),)



# Code detection
# has to also be able to take care of libnexus_security build (HSM_SOURCE_AVAILABLE = n)
# In this case only include the module if the PI component of the module is in libnexus_security archive.

ifeq ($(KEYLADDER_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_keyladder_enc.${SEC_OBJ_EXT} 2>/dev/null)
        KEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/keyladder/keyladder_ext.inc && test "${AR_OUT}" = "bhsm_keyladder_enc.${SEC_OBJ_EXT}" && echo y)
        #$(warning NEXUS_SECURITY_EXT is  $(NEXUS_SECURITY_EXTENSION))
        #$(warning NEXUS_EXTRALIBS is $(NEXUS_EXTRALIBS) AR_OUT is $(AR_OUT) KEYLADDER_SUPPORT is $(KEYLADDER_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            KEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/keyladder/keyladder_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/keyladder/bhsm_keyladder_enc.c && echo y)
        else
            KEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/keyladder/keyladder_ext.inc && test -e ${BHSM_SRC}/keyladder/bhsm_keyladder_enc.c && echo y)
        endif
    endif
endif

ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
    CHIP_GEOMETRY := 65nm
    ifeq (${KEYLADDER_SUPPORT}, y)
        NEXUS_COMMON_CRYPTO_SUPPORT ?=y
    endif
    SECUREACCESS_SUPPORT =?n
else
    NEXUS_COMMON_CRYPTO_SUPPORT ?=y
    ifeq ($(CORETECH),core40nm)
        CHIP_GEOMETRY := 40nm
    else
        ifeq ($(CORETECH),core28nm)
            CHIP_GEOMETRY := 40nm
            ## TODO ... 28nm is still mapped to 40nm
        endif
    endif
endif

ifeq ($(OTPMSP_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_otpmsp.${SEC_OBJ_EXT} 2>/dev/null)
        OTPMSP_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otpmsp/otpmsp_ext.inc && test "${AR_OUT}" = "bhsm_otpmsp.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(OTPMSP_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            OTPMSP_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otpmsp/otpmsp_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/otpmsp/bhsm_otpmsp.c && echo y)
        else
            OTPMSP_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otpmsp/otpmsp_ext.inc && test -e ${BHSM_SRC}/otpmsp/bhsm_otpmsp.c && echo y)
        endif
    endif
endif

ifeq ($(OTPID_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_otp_id.${SEC_OBJ_EXT} 2>/dev/null)
        OTPID_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otp_id/otp_id_ext.inc && test "${AR_OUT}" = "bhsm_otp_id.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) OTPID_SUPPORT [$(OTPID_SUPPORT)] )
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            OTPID_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otp_id/otp_id_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/common/bhsm_otp_id.c && echo y)
            #$(warning $(AR_OUT) OTPID_SUPPORT [$(OTPID_SUPPORT)] )
        else
            OTPID_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otp_id/otp_id_ext.inc && test -e ${BHSM_SRC}/common/bhsm_otp_id.c && echo y)
            #$(warning $(AR_OUT) OTPID_SUPPORT [$(OTPID_SUPPORT)] )
        endif
    endif
endif

ifeq ($(ZEUSVER),Zeus42)
    ifeq ($(OTPKEY_SUPPORT),)
        ifeq ($(HSM_SOURCE_AVAILABLE),n)
            AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_otp_key.${SEC_OBJ_EXT} 2>/dev/null)
            OTPKEY_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/otp_key/otp_key.inc && test "${AR_OUT}" = "bhsm_otp_key.${SEC_OBJ_EXT}" && echo y)
            $(warning $(AR_OUT) $(OTPKEY_SUPPORT))
        else
            OTPKEY_SUPPORT := $(shell test -e ${BHSM_SRC}/common/bhsm_otp_key.c && echo y)
        endif
    endif
else
    OTPKEY_SUPPORT := n
endif

ifeq ($(AEGIS_USERCMD_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} nexus_hmac_sha_cmd.${SEC_OBJ_EXT} 2>/dev/null)
        AEGIS_USERCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/usercmd/usercmd_ext.inc \
                                && test "${AR_OUT}" = "nexus_hmac_sha_cmd.${SEC_OBJ_EXT}" \
                                && echo y)
        #$(warning $(AR_OUT) AEGIS_USERCMD_SUPPORT[$(AEGIS_USERCMD_SUPPORT)] )
    else
        AEGIS_USERCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/usercmd/usercmd_ext.inc \
                                && test -e ${NEXUS_SECURITY_EXTENSION}/usercmd/src/${CHIP_GEOMETRY}/nexus_hmac_sha_cmd.c \
                                && test -e ${NEXUS_SECURITY_EXTENSION}/usercmd/src/${CHIP_GEOMETRY}/nexus_random_number.c \
                                && echo y)
        #$(warning $(AR_OUT) AEGIS_USERCMD_SUPPORT[$(AEGIS_USERCMD_SUPPORT)] )
    endif
endif


ifeq ($(SECUREACCESS_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_secure_access.${SEC_OBJ_EXT} 2>/dev/null)
        SECUREACCESS_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/secureaccess/secureaccess_ext.inc && test "${AR_OUT}" = "bhsm_secure_access.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(SECUREACCESS_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            SECUREACCESS_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/secureaccess/secureaccess_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/secure_access/bhsm_secure_access.c && echo y)
        else
            SECUREACCESS_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/secureaccess/secureaccess_ext.inc && test -e ${BHSM_SRC}/secure_access/bhsm_secure_access.c && echo y)
        endif
    endif
endif

ifeq ($(SECURERSA_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_secure_rsa.${SEC_OBJ_EXT} 2>/dev/null)
        SECURERSA_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securersa/securersa_ext.inc && test "${AR_OUT}" = "bhsm_secure_rsa.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(SECURERSA_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            SECURERSA_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securersa/securersa_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/secure_rsa/bhsm_secure_rsa.c && echo y)
        else
            SECURERSA_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securersa/securersa_ext.inc && test -e ${BHSM_SRC}/secure_rsa/bhsm_secure_rsa.c && echo y)
        endif
    endif
endif

ifeq ($(SECURERAWCMD_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} nexus_security_rawcommand.${SEC_OBJ_EXT} 2>/dev/null)
        SECURERAWCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/rawcommand/rawcmd_ext.inc && test "${AR_OUT}" = "nexus_security_rawcommand.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(SECURERAWCMD_SUPPORT))
    else
        SECURERAWCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/rawcommand/rawcmd_ext.inc \
                                  && test -e ${NEXUS_SECURITY_EXTENSION}/rawcommand/src/nexus_security_rawcommand.c \
                                  && echo y)
    endif
endif

ifeq ($(IPLICENSING_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_ip_licensing.${SEC_OBJ_EXT} 2>/dev/null)
        IPLICENSING_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/iplicensing/iplicensing_ext.inc && test "${AR_OUT}" = "bhsm_ip_licensing.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(IPLICENSING_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            IPLICENSING_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/iplicensing/iplicensing_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/ip_licensing/bhsm_ip_licensing.c && echo y)
        else
            IPLICENSING_SUPPORT := $(shell test -e ${BHSM_SRC}/ip_licensing/bhsm_ip_licensing.c && echo y)
        endif
    endif
endif

ifeq ($(AVKEYLADDER_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_ip_licensing.${SEC_OBJ_EXT} 2>/dev/null)
        AVKEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/avkeyladder/avkeyladder_ext.inc && test "${AR_OUT}" = "bhsm_avcmd.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(AVKEYLADDER_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            AVKEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/avkeyladder/avkeyladder_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/av_cmd/bhsm_avcmd.c && echo y)
        else
            AVKEYLADDER_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/avkeyladder/avkeyladder_ext.inc && test -e ${BHSM_SRC}/av_cmd/bhsm_avcmd.c && echo y)
        endif
    endif
endif

ifeq ($(BSECKCMD_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_bseck.${SEC_OBJ_EXT} 2>/dev/null)
        BSECKCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/bseckcmd/bseckcmd_ext.inc && test "${AR_OUT}" = "bhsm_bseck.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(BSECKCMD_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            BSECKCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/bseckcmd/bseckcmd_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/bseck_cmd/bhsm_bseck.c && echo y)
        else
            BSECKCMD_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/bseckcmd/bseckcmd_ext.inc && test -e ${BHSM_SRC}/bseck_cmd/bhsm_bseck.c && echo y)
        endif
    endif
endif

ifeq ($(SECURITY_MSIPTV_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_iptv.${SEC_OBJ_EXT} 2>/dev/null)
        SECURITY_MSIPTV_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/msiptv/msiptv_ext.inc && test "${AR_OUT}" = "bhsm_iptv.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(SECURITY_MSIPTV_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            SECURITY_MSIPTV_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/msiptv/msiptv_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/iptv/bhsm_iptv.c && echo y)
        else
            SECURITY_MSIPTV_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/msiptv/msiptv_ext.inc && test -e ${BHSM_SRC}/msiptv/bhsm_iptv.c && test -e ${BSP_INCLUDE}/bsp_s_msiptv.h && echo y)
        endif
    endif
endif

ifeq ($(GENROOTKEY_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_gen_root_key.${SEC_OBJ_EXT} 2>/dev/null)
        GENROOTKEY_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/genrootkey/genrootkey_ext.inc && test "${AR_OUT}" = "bhsm_gen_root_key.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(GENROOTKEY_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            GENROOTKEY_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/genrootkey/genrootkey_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/gen_root_key/bhsm_gen_root_key.c && echo y)
        else
            GENROOTKEY_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/genrootkey/genrootkey_ext.inc && test -e ${BHSM_SRC}/gen_root_key/bhsm_gen_root_key.c && echo y)
        endif
    endif
endif

ifeq ($(SECUREPKL_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} bhsm_secure_pkl.${SEC_OBJ_EXT} 2>/dev/null)
        SECUREPKL_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securepkl/securepkl_ext.inc && test "${AR_OUT}" = "bhsm_secure_pkl.${SEC_OBJ_EXT}" && echo y)
        #$(warning $(AR_OUT) $(SECUREPKL_SUPPORT))
    else
        ifeq ($(filter $(CORETECH), core65nm core65nmASKM), $(CORETECH))
            SECUREPKL_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securepkl/securepkl_ext.inc && test -e ${BHSM_SRC}/${CORETECH}/secure_pkl/bhsm_secure_pkl.c && echo y)
        else
            SECUREPKL_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/securepkl/securepkl_ext.inc && test -e ${BHSM_SRC}/secure_pkl/bhsm_secure_pkl.c && echo y)
        endif
    endif
endif

ifeq ($(KEYLADDER_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/keyladder/keyladder_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_KEYLADDER=1
    NEXUS_SECURITY_DEFINES += NEXUS_HAS_KEYLADDER_SUPPORT=1
    BHSM_KEYLADDER=ON
else
    AVKEYLADDER_SUPPORT := n
endif

ifeq ($(NEXUS_SECURITY_IRDETO_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} nexus_irdeto.${SEC_OBJ_EXT} 2>/dev/null)
        NEXUS_SECURITY_IRDETO_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/irdeto/irdeto.inc && test "${AR_OUT}" = "nexus_irdeto.${SEC_OBJ_EXT}" && echo y)
    else
        NEXUS_SECURITY_IRDETO_SUPPORT := $(shell test -e $(MAGNUM)/portinginterface/hsm/include/aegis/$(ZEUSVER)/bsp_restricted_commands_irdeto.h && echo y)
    endif
endif

ifeq ($(NEXUS_SECURITY_IRDETO_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/irdeto/irdeto.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_IRDETO=1
    BHSM_SUPPORT_IRDETO=ON
endif

ifeq ($(NEXUS_SECURITY_ALPINE_SUPPORT),)
    NEXUS_SECURITY_ALPINE_SUPPORT := $(shell test -e $(MAGNUM)/portinginterface/hsm/include/aegis/$(ZEUSVER)/bsp_restricted_commands_alpine.h && echo y)
endif

ifeq ($(NEXUS_SECURITY_ALPINE_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/alpine/alpine.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ALPINE=1
    BHSM_SUPPORT_ALPINE=ON
endif

ifeq ($(NEXUS_SECURITY_VISTA_CWC_SUPPORT),)
    ifeq ($(HSM_SOURCE_AVAILABLE),n)
        AR_OUT := $(shell ar t ${NEXUS_EXTRALIBS} nexus_keyladder_vista.${SEC_OBJ_EXT} 2>/dev/null)
        NEXUS_SECURITY_VISTA_CWC_SUPPORT := $(shell test -e ${NEXUS_SECURITY_EXTENSION}/keyladder/secure_ext.inc && test "${AR_OUT}" = "nexus_keyladder_vista.${SEC_OBJ_EXT}" && echo y)
    else
        NEXUS_SECURITY_VISTA_CWC_SUPPORT := $(shell test -e $(MAGNUM)/portinginterface/hsm/src/keyladder/bhsm_restricted_keyladder_vista.c && echo y)
    endif
endif

ifeq ($(NEXUS_SECURITY_VISTA_CWC_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/keyladder_vista/keyladder_vista.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_VISTA_CWC=1
    BHSM_SUPPORT_KEYLADDER_VISTA=ON
endif

ifeq ($(OTPMSP_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/otpmsp/otpmsp_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_OTPMSP=1
    BHSM_OTPMSP=ON
else
    SECUREPKL_SUPPORT := n
    GENROOTKEY_SUPPORT := n
endif

ifeq ($(OTPID_SUPPORT),y)
    #$(warning OTPID_SUPPORT is yes)
    include ${NEXUS_TOP}/extensions/security/otp_id/otp_id_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_OTPID=1
    BHSM_OTPID=ON
endif

ifeq ($(OTPKEY_SUPPORT),y)
    #$(warning OTPKEY_SUPPORT is yes)
    include ${NEXUS_TOP}/extensions/security/otp_key/otp_key_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_OTPKEY=1
endif

ifeq ($(AEGIS_USERCMD_SUPPORT),y)
    #$(warning  AEGIS_USERCMD_SUPPORT is yes)
    include ${NEXUS_TOP}/extensions/security/usercmd/usercmd_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_USERCMD=1
endif

ifeq ($(SECUREACCESS_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/secureaccess/secureaccess_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_SECUREACCESS=1
    BHSM_SECURE_ACCESS=ON
endif


ifeq ($(SECURERSA_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/securersa/securersa_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_SECURERSA=1
    BHSM_SECURE_RSA=ON
endif

ifeq ($(SECURERAWCMD_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/rawcommand/rawcmd_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_RAWCMD=1
    NEXUS_CFLAGS +=  -DBHSM_AUTO_TEST
endif

ifeq ($(IPLICENSING_SUPPORT),y)
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_IPLICENSING=1
    BHSM_IP_LICENSING=ON
endif

ifeq ($(AVKEYLADDER_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/avkeyladder/avkeyladder_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_AVKEYLADDER=1
    BHSM_AV_CMD=ON
endif

ifeq ($(BSECKCMD_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/bseckcmd/bseckcmd_ext.inc
    NEXUS_CFLAGS +=  -DBSP_BSECK_COMMAND
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_BSECK_COMMAND=1
    BHSM_BSECK=ON
endif

ifeq ($(SECURITY_MSIPTV_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/msiptv/msiptv_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_MSIPTV=1
    BHSM_MSIPTV=ON
endif

ifeq ($(GENROOTKEY_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/genrootkey/genrootkey_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_GENROOTKEY=1
    BHSM_GEN_ROOT_KEY=ON
endif


ifeq ($(SECUREPKL_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/securepkl/securepkl_ext.inc
    NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_PKL=1
    BHSM_SECURE_PKL=ON
endif


ifeq ($(HSM_SOURCE_AVAILABLE),n)
    NEXUS_SECURITY_SOURCES := $(filter %_thunks.c, $(NEXUS_SECURITY_SOURCES))
endif

ifeq ($(NEXUS_COMMON_CRYPTO_SUPPORT), y)
include $(BSEAV)/lib/security/common_crypto/common_crypto.inc
NEXUS_SECURITY_PRIVATE_INCLUDES += $(COMMON_CRYPTO_INCLUDES)
NEXUS_SECURITY_SOURCES += $(COMMON_CRYPTO_SOURCES)
endif
