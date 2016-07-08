#/******************************************************************************
#* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#*
#* This program is the proprietary software of Broadcom and/or its
#* licensors, and may only be used, duplicated, modified or distributed pursuant
#* to the terms and conditions of a separate, written license agreement executed
#* between you and Broadcom (an "Authorized License").  Except as set forth in
#* an Authorized License, Broadcom grants no license (express or implied), right
#* to use, or waiver of any kind with respect to the Software, and Broadcom
#* expressly reserves all rights in and to the Software and all intellectual
#* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#* Except as expressly set forth in the Authorized License,
#*
#* 1. This program, including its structure, sequence and organization,
#*    constitutes the valuable trade secrets of Broadcom, and you shall use all
#*    reasonable efforts to protect the confidentiality thereof, and to use
#*    this information only in connection with your use of Broadcom integrated
#*    circuit products.
#*
#* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#*
#* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
#*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#******************************************************************************/

include $(MAGNUM)/portinginterface/hsm/bhsm_defs.inc

BHSM_DEFINES += BHSM_ZEUS_VER_MAJOR=$(BHSM_ZEUS_VER_MAJOR)
BHSM_DEFINES += BHSM_ZEUS_VER_MINOR=$(BHSM_ZEUS_VER_MINOR)

#$(warning  ..........bhsm_zeus.inc entered with CORETECH = $(CORETECH) and ZEUSVER = $(ZEUSVER))


export ZEUS_VERSION=$(ZEUSVER)

# Define this module
MAGNUM_MODULES += BHSM


# Definitions
ifeq ($(ZEUS_VERSION), )
    $(warning WARNING No Zeus Version for $(BCHP_CHIP) $(BCHP_VER))
else
    BHSM_DEFINES += ${ZEUS_VERSION}=1
endif

BHSM_DEFINES += BHSM_AEGIS_SUPPORT


ifeq ($(BHSM_TEST_INCLUDE_ALL),ON)
    BHSM_SUPPORT_CARLSBAD=ON
    BHSM_SUPPORT_IRDETO=ON
    BHSM_SUPPORT_RESTRICTED_REG_VER=ON
    BHSM_SUPPORT_KEYLADDER_VISTA=ON
    BHSM_SUPPORT_USER_AESDES=ON
    BHSM_SUPPORT_KKC=ON
    BHSM_SUPPORT_HDCP22=ON
    BHSM_MISC_BITS_SAGE=ON
    BHSM_SECURE_PKL=ON
    BHSM_NSKCMD=ON
    BHSM_SUPPORT_ALPINE=ON
    BHSM_IP_LICENSING=ON
    BHSM_SECURE_ACCESS=ON
    BHSM_SECURE_RSA=ON
    BHSM_SAGE_ANTI_ROLLBACK_COUNTER=ON
    BHSM_WEB_CPU=ON
    BHSM_PWR_MGMT=ON
    BHSM_BSECK=ON
    BHSM_GEN_ROOT_KEY=ON
    BHSM_CHALLENGE_RESPONSE=ON
    # BHSM_AV_CMD=ON
endif


ifeq (${BHSM_DEBUG_DUMP_RAW_BSP_COMMAND},y)
    BHSM_DEFINES += BHSM_DEBUG_DUMP_RAW_BSP_COMMAND=1
endif

ifeq (${BHSM_DEBUG_INDIVIDUAL_COMMAND},y)
    BHSM_DEFINES += BHSM_DEBUG_INDIVIDUAL_COMMAND=1
endif

# Support uScValue definition for newer chip (7400C0 and later)
BHSM_DEFINES += BHSM_SC_VALUE

BHSM_DEFINES += BSP_REFORMATTED_FOR_URSR_15_1    # Scheduled to be removed for URSR 16.2

BHSM_OTPMSP       := ON

###Region_Verification support                                                                          #
#-------------------------------------------------------------------------------------------------------#

BHSM_INCLUDES +=    $(MAGNUM)/portinginterface/hsm/include/region_verify
BHSM_SOURCES  +=    $(MAGNUM)/portinginterface/hsm/src/region_verify/bhsm_verify_reg.c


###AV Command support                                                                                   #
#-------------------------------------------------------------------------------------------------------#

ifeq ($(BHSM_AV_CMD),ON)
    #$(warning  ..........AV Comamnd Feature is built in with BHSM_AV_CMD = $(BHSM_AV_CMD) )

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/av_cmd

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/av_cmd/bhsm_avcmd.c
    endif
endif


### Challenge Response support                                                                          #
#-------------------------------------------------------------------------------------------------------#

ifeq ($(BHSM_CHALLENGE_RESPONSE),ON)
    #$(warning  ..........Challenge Response Feature is built in with BHSM_CHALLENGE_RESPONSE = $(BHSM_CHALLENGE_RESPONSE) )

    BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/include/challenge_response

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/challenge_response/bhsm_challenge_response.c
    endif
endif

### Challenge Response support                                                                          #
#-------------------------------------------------------------------------------------------------------#

ifeq ($(BHSM_GEN_ROOT_KEY),ON)
    #$(warning  ..........Generate RootKey Feature is built in with BHSM_GEN_ROOT_KEY = $(BHSM_GEN_ROOT_KEY) )

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/gen_root_key

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/gen_root_key/bhsm_gen_root_key.c
    endif
endif


### BHSM_MSIPTV build option   #
#-------------------------#

ifeq ($(BHSM_MSIPTV),ON)
    BHSM_DEFINES += BHSM_IPTV_CODE=1
    BHSM_DEFINES += BHSM_IPTV=1
    BHSM_DEFINES += BSP_MSIPTV=1
    # $(warning  ..........IPTV Feature is built in with BHSM_IPTV = $(BHSM_IPTV) )
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/msiptv
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/msiptv/bhsm_iptv.c
    endif
else
    BHSM_DEFINES += BHSM_IPTV_CODE=0
    BHSM_DEFINES += BHSM_IPTV=0
    BHSM_DEFINES += BHSM_MSIPTV=0
endif



###  BSECK  support
#-----------------------------------------------------------------------------------#

ifeq ($(BHSM_BSECK),ON)
    BHSM_DEFINES += BHSM_BSECK_CODE=1
    BHSM_DEFINES += BSP_DOWNLOAD_CODE=1
    #$(warning  ..........BSECK Feature is built in with BHSM_BSECK = $(BHSM_BSECK) )
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/bseck_cmd

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/bseck_cmd/bhsm_bseck.c
    endif
else
    BHSM_DEFINES += BHSM_BSECK_CODE=0
    BHSM_DEFINES += BSP_DOWNLOAD_CODE=0
endif



###  Key Ladder support
#-----------------------------------------------------------------------------------#


BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/keyladder
BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/keyladder/bhsm_keyladder_enc.c


##--MS DRM Support--##
#-----------------------------------------------------------------------------------#


ifeq ($(MSDRM_PD_SUPPORT),y)
    BHSM_DEFINES += MS_DRM
endif


## Power Management Support
#----------------------------------------------------------------------------#

ifeq ($(BHSM_PWR_MGMT),ON)
    BHSM_DEFINES += BHSM_PWRMGMT_SUPPORT=1
    #$(warning  ..........Power Management Feature is built in with BHSM_PWR_MGMT= $(BHSM_PWR_MGMT) )
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_pwr_mgmt.c
    endif
else
    BHSM_DEFINES += BHSM_PWRMGMT_SUPPORT=0
endif


## Sandbox Support
#----------------------------------------------------------------------------#

ifeq ($(BHSM_WEB_CPU),ON)

    ifeq ($(findstring $(ZEUSVER), Zeus40 Zeus41 Zeus42), $(ZEUSVER))

        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/sandbox

        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/sandbox/bhsm_sandbox.c
        endif

    endif

    BHSM_DEFINES += BHSM_WEBCPU_SUPPORT=1
    #$(warning  ..........Web CPU Feature is built in with BHSM_WEB_CPU= $(BHSM_WEB_CPU) )
else
    BHSM_DEFINES += BHSM_WEBCPU_SUPPORT=0
endif


ifeq ($(BHSM_SAGE_SETARCH),ON)
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_restricted_arch_sage.c
    endif
endif

## HSM PI On SAGE Support
#  To prepare HSM PI code for SAGE compilation (running on SAGE)
#----------------------------------------------------------------------------#


ifeq ($(BHSM_SAGE_BSP_PI),ON)  #BHSM_SAGE_BSP_PI is DEPRECATED. Use BHSM_BUILD_HSM_FOR_SAGE
    BHSM_BUILD_HSM_FOR_SAGE=ON
endif

ifeq ($(BHSM_BUILD_HSM_FOR_SAGE),ON)
    BHSM_DEFINES += BHSM_BUILD_HSM_FOR_SAGE=1
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/sage_bsp_cmd
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/sage_bsp_cmd/bhsm_sage_cmds.c
    endif
endif

## SAGE_ANTI_ROLLBACK_COUNTER_RAW support
#-----------------------------------------------------------------------------------#


ifeq ($(BHSM_SAGE_ANTI_ROLLBACK_COUNTER_RAW),ON) # BHSM_SAGE_ANTI_ROLLBACK_COUNTER_RAW is DEPRECATED. use BHSM_SAGE_ANTI_ROLLBACK_COUNTER
    BHSM_SAGE_ANTI_ROLLBACK_COUNTER=ON
endif

ifeq ($(BHSM_SAGE_ANTI_ROLLBACK_COUNTER),ON)

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/sage_anti_rollback

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/sage_anti_rollback/bhsm_restricted_anti_rollback_sage.c
    endif
endif

## OTP/MSP I/O Support
#-----------------------------------------------------------------------------------#

ifeq ($(BHSM_OTPMSP),ON)
    BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_OTPMSP=1
    #$(warning  ..........Secure OTP_MSP Feature is built in with BHSM_OTPMSP= $(BHSM_OTPMSP) )
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/otpmsp
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/otpmsp/bhsm_otpmsp.c
    endif
else
    BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_OTPMSP=0
endif


## Secure RSA Functions
#----------------------------------------------------------------------------#

ifeq ($(BHSM_SECURE_RSA),ON)
    #$(warning  ..........BHSM SECURE_RSA Feature is built in with BHSM_SECURE_RSA = $(BHSM_SECURE_RSA) )

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/secure_rsa

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/secure_rsa/bhsm_secure_rsa.c
    endif

    BHSM_DEFINES += BSP_SECURE_RSA=1
    BHSM_DEFINES += BSP_SECURE_RSA_IN_VKL=1
endif


## Secure Access Functions
#----------------------------------------------------------------------------#

ifeq ($(BHSM_SECURE_ACCESS),ON)
    #$(warning  ..........BHSM SECURE_ACCESS Feature is built in with BHSM_SECURE_ACCESS = $(BHSM_SECURE_ACCESS) )

	BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/secure_access
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/secure_access/bhsm_secure_access.c
    endif
endif


## IP Licensing Functions
#----------------------------------------------------------------------------#

ifeq ($(BHSM_IP_LICENSING),ON)
    #$(warning  ..........BHSM IP_LICENSING Feature is built in with BHSM_IP_LICENSING = $(BHSM_IP_LICENSING) )

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/ip_licensing

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/ip_licensing/bhsm_ip_licensing.c
    endif
endif

## Support the ALPINE feature.
ifeq ($(BHSM_SUPPORT_ALPINE),ON)
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_restricted_alpine.c
    endif
endif


#----------------------------------------------------------------------------#

ifeq ($(BHSM_NSKCMD),ON)

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/nsk_cmd

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/nsk_cmd/bhsm_nskcmd.c
    endif

endif

## SECURE PKL Functions
#----------------------------------------------------------------------------#

ifeq ($(BHSM_SECURE_PKL),ON)
    # $(warning  ..........BHSM SECURE PKL Feature is built in with BHSM_SECURE_PKL = $(BHSM_SECURE_PKL) )

    BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/include/secure_pkl

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/secure_pkl/bhsm_secure_pkl.c
    endif

    BHSM_DEFINES += PKL
    BHSM_DEFINES += BSP_PKL
endif


ifeq ($(BHSM_OTPID),ON)
    #$(warning  ..........BHSM_OTPID is enabled )

    BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/include/common

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/common/bhsm_otp_id.c
        #$(warning building ............................. bhsm_otp_id.c )
    endif

    BHSM_DEFINES += BHSM_OTPID=1
endif


ifeq ($(BHSM_MISC_BITS_SAGE),ON)
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_restricted_misc_bits_sage.c
endif


## Debug_RdWr feature
#----------------------------------------------------------------------------#

ifeq ($(BHSM_DEBUG_RD_WR),ON)
    #$(warning  ..........BHSM DEBUG_RD_WR Feature is built in with BHSM_DEBUG_RD_WR = $(BHSM_DEBUG_RD_WR) )

    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/brcm_only

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/brcm_only/bhsm_debug_rdwr.c
    endif
endif

ifneq ($(BHSM_VERBOSE_BSP_TRACE),n)
    BHSM_DEFINES += BHSM_VERBOSE_BSP_TRACE
endif

## Enforce the client to use allocated VKLs. Default is ON
#----------------------------------------------------------------------------#

ifeq ($(BHSM_ENFORCE_VKL_ALLOCATION),OFF)
    BHSM_DEFINES += BHSM_ENFORCE_VKL_ALLOCATION=0
else
    BHSM_DEFINES += BHSM_ENFORCE_VKL_ALLOCATION=1
endif

#----------------------------------------------------------------------------#

#######################################################################
## MAIN HSM PI MODULE
#######################################################################

# Module Includes
BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/common
BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/common/${CORETECH}

# Private Module Includes
BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/common

# Common Aegis Includes
BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/aegis/${ZEUSVER}

# Public Module Sources
ifneq ($(HSM_SOURCE_AVAILABLE),n)
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_bsp_msg.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_misc.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_usercmd_common.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_user_rsa.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_keyslots.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_random_number.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_hmac_sha.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_otp_key.c
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_exception_status.c
endif

ifeq ($(BHSM_KEY_TWEAK),ON)
    BHSM_DEFINES += BHSM_SECURE_KEY_TWEAK_MK=1
endif

ifeq ($(BHSM_SUPPORT_HDCP22),ON)
    BHSM_DEFINES += BHSM_SUPPORT_HDCP22=1
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_restricted_hdcp22.c
endif

ifeq ($(BHSM_SUPPORT_KKC),ON)
    BHSM_DEFINES += BHSM_SUPPORT_KKC=1
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/keyladder/bhsm_restricted_keyladder_key_contribution.c
endif

ifeq ($(BHSM_SUPPORT_USER_AESDES),ON)
    BHSM_DEFINES += BHSM_SUPPORT_USER_AESDES=1
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/common/bhsm_restricted_aes_des.c
endif

ifeq ($(BHSM_SUPPORT_KEYLADDER_VISTA),ON)
    BHSM_DEFINES += BHSM_SUPPORT_KEYLADDER_VISTA=1
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/keyladder/bhsm_restricted_keyladder_vista.c
endif

ifeq ($(BHSM_SUPPORT_RESTRICTED_REG_VER),ON)
    BHSM_DEFINES += BHSM_SUPPORT_RESTRICTED_REG_VER=1
    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/region_verify/bhsm_restricted_verify_reg.c
endif

ifeq ($(BHSM_SUPPORT_IRDETO),ON)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/irdeto
    BHSM_SOURCES  += $(MAGNUM)/portinginterface/hsm/src/irdeto/bhsm_restricted_chipconfig_irdeto.c
    BHSM_SOURCES  += $(MAGNUM)/portinginterface/hsm/src/irdeto/bhsm_restricted_transform_irdeto.c
endif

ifeq ($(BHSM_SUPPORT_CARLSBAD),ON)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/carlsbad
    BHSM_SOURCES  += $(MAGNUM)/portinginterface/hsm/src/carlsbad/bhsm_restricted_carlsbad.c
endif

ifeq ($(BHSM_SUPPORT_HDDTA),ON)
    BHSM_DEFINES += BHSM_SUPPORT_HDDTA=1
endif
