############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
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
#
# Revision History:
#
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

#-----------------------------------------------------------------------------------#
# HSM build-command-line-options:
#  CUST=G  or D  or E  or N or A  or GT  or NGNA (to turn on DCAS)  or S  or G or H to select customer mode
#  RMXPATCH=ON               to turn on RMX patch
#  SCVALUE=ON                to turn on scrambling ctrl bits setting with specified value
#  BHSM_REGIONVERIFY=ON      to turn on region verification with Generic mode  (invalid option for non-Generic mode build)
#  BHSM_BSECK=ON             to turn on/off BSECK feature
#  BHSM_IPTV=ON              to turn on/off IPTV feature
#  BHSM_KEYLADDER=ON         to select spitted, protected secure PIs
#  BHSM_MISC=ON              to select spitted, protected secure PIs
#  BHSM_OTPMSP=ON            to select spitted, protected secure PIs
#  BHSM_SECURE_RSA=ON        to turn on Secure RSA feature
#  BHSM_SECURE_ACCESS=ON     to turn on Secure Access (DRM Data Op) feature
#  BHSM_IP_LICENSING=ON      to turn on IP Licensing feature
#
#-----------------------------------------------------------------------------------#


#
# Conditional Options for this Makefile
#
# BHSM_CUST - HSM Specific Option.  This value will be provided for you if appropriate.
#

BCHP_VER_LOWER := $(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')

#
# Check for core architecture
#

ifeq ($(filter $(BCHP_CHIP), 7125 7340 7342 7420 7468), $(BCHP_CHIP))
    CORETECH := core65nmASKM
endif

ifeq ($(filter $(BCHP_CHIP), 7405), $(BCHP_CHIP))
    CORETECH := core65nm
endif

ifeq ($(filter $(BCHP_CHIP), 7408 7550), $(BCHP_CHIP))
    CORETECH := coreNoBSP
endif

export ZEUS_VERSION=$(CORETECH)


# Define this module
MAGNUM_MODULES += BHSM


# Definitions
BHSM_DEFINES += ${ZEUS_VERSION}=1
BHSM_DEFINES += BHSM_AEGIS_SUPPORT

# Support uScValue definition for newer chip (7400C0 and later)
BHSM_DEFINES += BHSM_SC_VALUE



#-----------------------------------------------------------------------------------#
# old support "make CUST=E"
# space can NOT be put after 'E' in ifeq ($(CUST),  E),  yet can be put before 'E'

ifeq ($(CUST),E)
    BHSM_CUST := E
endif


#-----------------------------------------------------------------------------------#

ifeq ($(BHSM_CUST),E)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_e.inc
endif

#-----------------------------------------------------------------------------------#
# old support "make CUST=D"
ifeq ($(CUST),D)
    BHSM_CUST := D
endif

ifeq ($(BHSM_CUST),D)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_d.inc
endif


#-----------------------------------------------------------------------------------#
# old support "make CUST=S"
ifeq ($(CUST),S)
    BHSM_CUST := S
endif

ifeq ($(BHSM_CUST),S)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_s.inc
endif


#-----------------------------------------------------------------------------------#
# old support "make CUST=T"
ifeq ($(CUST),T)
    BHSM_CUST := T
endif

ifeq ($(BHSM_CUST),T)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_t.inc
endif


#-----------------------------------------------------------------------------------#
# old support "make CUST=H"
ifeq ($(CUST),H)
    BHSM_CUST := H
endif

ifeq ($(BHSM_CUST),H)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_h.inc
endif


#-----------------------------------------------------------------------------------#
# old support "make CUST=M"
ifeq ($(CUST),M)
    BHSM_CUST := M
endif

ifeq ($(BHSM_CUST),M)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_m.inc
endif


#-----------------------------------------------------------------------------------#
# old support "make CUST=N"
ifeq ($(CUST),N)
    BHSM_CUST := N
endif

ifeq ($(BHSM_CUST),N)
    BHSM_CUST_DIR := $(shell awk 'BEGIN{print tolower("$(BHSM_CUST)")}')
    include $(MAGNUM)/portinginterface/hsm/bhsm_n.inc
endif

#-----------------------------------------------------------------------------------#
# old support "make CUST=A"
ifeq ($(CUST),A)
    BHSM_CUST := A
endif

ifeq ($(BHSM_CUST),A)
    BHSM_CUST_DIR = all
    include $(MAGNUM)/portinginterface/hsm/bhsm_all_legacy.inc
endif


#  added 4/25/07 for DCAS specific commands. No chip/version limit yet. may be added later

ifeq ($(CUST),NGNA)
    BHSM_CUST :=NGNA
endif


ifeq ($(BHSM_CUST),NGNA)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/ngna

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/ngna/bhsm_dcas_cmds.c
    endif

    GREAT_GENERIC=true
    BHSM_CUST_GENERICNGNA=1
endif


#-----------------------------------------------------------------------------------#
ifeq ($(CUST),G)
    BHSM_CUST := G
endif
ifeq ($(BHSM_CUST),G)
    GREAT_GENERIC=true
endif

ifeq ($(findstring G, $(BHSM_CUST)), $(BHSM_CUST))
    BHSM_CUST := G
    GREAT_GENERIC=true
endif

ifeq ($(findstring G, $(CUST)), $(CUST))
    BHSM_CUST := G
    GREAT_GENERIC=true
endif


#-----------------------------------------------------------------------------------#
# Default BHSM_CUST to generic,
# new support : "make BHSM_CUST=G"
#                     "make without touch of BHSM_CUST"
#	          "make CUST=X,  make BHSM_CUST=X,   X is anything but defined"
#ifeq ($(findstring G, $(BHSM_CUST)), $(BHSM_CUST))

    ifeq ($(GREAT_GENERIC),true)
        # this line has no effect if make BHSM_CUST=G :            BHSM_CUST := generic

        # not used here, for testApp build
        BHSM_CUST_GENERIC=1

        BHSM_DEFINES += BSP_GENERIC

        BHSM_CUST_DIR = generic

        ###Region_Verification support in Generic Mode, there is an option to add the Region_Verification or not
        ifeq ($(BHSM_REGIONVERIFY),ON)
            BHSM_REGIONVERIFY_MK=1
        endif

        ifeq ($(BHSM_REGIONVERIFY),OFF)
            BHSM_REGIONVERIFY_MK=0
        endif

        ifeq ($(BHSM_REGIONVERIFY_MK),1)
            BHSM_REG_VERIFY_FEATURE=1
            #$(warning  ..........Region Verificationi Feature is built in with BHSM_REGIONVERIFY_MK= $(BHSM_REGIONVERIFY_MK) )
            BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify \
            $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify/aegis

            ifneq ($(HSM_SOURCE_AVAILABLE),n)
                BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify/bhsm_verify_reg.c
            endif
        endif
    endif

    #-----------------------------------------------------------------------------------#


    ### 7405 all 65nm, not needed

    ## you can turn it ON for any chip, with 'make RMXPATCH=ON', even for 65nm chips like 7405
    ifeq ($(RMXPATCH),ON)
        BHSM_RMX_PATCH=1
    endif

    ## you can turn it OFF for any chip, with 'make RMXPATCH=OFF', for any supported chip
    ifeq ($(RMXPATCH),OFF)
        BHSM_RMX_PATCH=0
    endif

    ifeq ($(BHSM_RMX_PATCH),1)
        #CFLAGS += -DBHSM_RMX_PATCH=1
        BHSM_DEFINES += BHSM_RMX_PATCH=1
        #$(warning  ..........bhsm_all.inc is entered with BHSM_RMX_PATCH= $(BHSM_RMX_PATCH) )
    endif

    ## you can turn it ON for any chip, with 'make BHSM_MSIPTV=ON', even for any chip
    ifeq ($(BHSM_MSIPTV),ON)
        BHSM_IPTV_MK=1
    endif

    ## you can turn it OFF for any chip, with 'make BHSM_MSIPTV=OFF', even any chip
    ifeq ($(BHSM_MSIPTV),OFF)
        BHSM_IPTV_MK=0
    endif

    ifeq ($(BHSM_IPTV_MK),1)
        #CFLAGS += -DBHSM_IPTV_CODE=1
        BHSM_DEFINES += BHSM_IPTV_CODE=1
        # $(warning  ..........IPTV Feature is built in with BHSM_IPTV_MK= $(BHSM_IPTV_MK) )
    endif

    #-----------------------------------------------------------------------------------#


    #-----------------------------------------------------------------------------------#
    ###  BSECK  support

    ifeq ($(findstring $(BCHP_CHIP), 7405 7420 7340 7342 7125 7468), $(BCHP_CHIP))
        BHSM_BSECK_MK=0
    endif

    ## you can turn it ON for any chip, with 'make BHSM_BSECK=ON', even for any chip
    ifeq ($(BHSM_BSECK),ON)
        BHSM_BSECK_MK=1
    endif

    ## you can turn it OFF for any chip, with 'make BHSM_BSECK=OFF', even any chip
    ifeq ($(BHSM_BSECK),OFF)
        BHSM_BSECK_MK=0
    endif

    ifeq ($(BHSM_BSECK_MK),1)
        #CFLAGS += -DBHSM_BSECK_CODE=1
        BHSM_DEFINES += BHSM_BSECK_CODE=1
        #$(warning  ..........BSECK Feature is built in with BHSM_BSECK_MK= $(BHSM_BSECK_MK) )
    endif

    #-----------------------------------------------------------------------------------#
    ###  PI split, to hide certain security related PIs in a separate files


    ifeq ($(BHSM_KEYLADDER),ON)
        BHSM_SECURE_KEYLADDER_MK=1
    endif

    ifeq ($(BHSM_KEYLADDER),OFF)
        BHSM_SECURE_KEYLADDER_MK=0
    endif

    ifeq ($(BSP_DS2_KEY_ROUTING),ON)
        BHSM_DEFINES += BHSM_DS2_KEY_ROUTING
        BHSM_SECURE_KEYLADDER_MK=1
    endif

    ifeq ($(BHSM_SECURE_KEYLADDER_MK),1)
        #CFLAGS += -DBHSM_SECURE_PI_SUPPORT=1  -DBHSM_SECURE_PI_SUPPORT_KEYLADDER=1
        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT=1
        else
            BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT=0
        endif

        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_KEYLADDER=1
        #$(warning  ..........Secure Keyladder Feature is built in with BHSM_SECURE_KEYLADDER_MK= $(BHSM_SECURE_KEYLADDER_MK) )
    else
        #CFLAGS += -DBHSM_SECURE_PI_SUPPORT=0  -DBHSM_SECURE_PI_SUPPORT_KEYLADDER=0
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT=0
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_KEYLADDER=0
    endif

    #-----------------------------------------------------------------------------------#

    ##--MS DRM Support--##
    ifeq ($(MSDRM_PD_SUPPORT),y)
        BHSM_DEFINES += MS_DRM
    endif

    ifeq ($(BHSM_MISC),ON)
        BHSM_SECURE_MISC_MK=1
    endif

    ifeq ($(BHSM_MISC),OFF)
        BHSM_SECURE_MISC_MK=0
    endif

    ifeq ($(BHSM_SECURE_MISC_MK),1)
        #CFLAGS +=  -DBHSM_SECURE_PI_SUPPORT_MISC=1
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_MISC=1
        #$(warning  ..........Secure Misc Feature is built in with BHSM_SECURE_MISC_MK= $(BHSM_SECURE_MISC_MK) )
    else
        #CFLAGS +=  -DBHSM_SECURE_PI_SUPPORT_MISC=0
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_MISC=0
    endif


    #-----------------------------------------------------------------------------------#
    ifeq ($(BHSM_OTPMSP),ON)
        BHSM_SECURE_OTPMSP_MK=1
    endif

    ifeq ($(BHSM_OTPMSP),OFF)
        BHSM_SECURE_OTPMSP_MK=0
    endif

    ifeq ($(BHSM_SECURE_OTPMSP_MK),1)
        #CFLAGS +=  -DBHSM_SECURE_PI_SUPPORT_OTPMSP=1
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_OTPMSP=1
        #$(warning  ..........Secure OTP_MSP Feature is built in with BHSM_SECURE_OTPMSP_MK= $(BHSM_SECURE_OTPMSP_MK) )
    else
        #CFLAGS +=  -DBHSM_SECURE_PI_SUPPORT_OTPMSP=0
        BHSM_DEFINES += BHSM_SECURE_PI_SUPPORT_OTPMSP=0
    endif

    # Secure RSA Functions
    ifeq ($(BHSM_SECURE_RSA),ON)
        BHSM_SECURE_RSA_MK=1
    endif

    ifeq ($(BHSM_SECURE_RSA),OFF)
        BHSM_SECURE_RSA_MK=0
    endif

    ifeq ($(BHSM_SECURE_RSA_MK),1)
        BHSM_SECURE_RSA_FEATURE=1
        #$(warning  ..........BHSM SECURE_RSA Feature is built in with BHSM_SECURE_RSA_MK= $(BHSM_SECURE_RSA_MK) )
        BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_rsa \
                    $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_rsa/aegis

        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_rsa/bhsm_secure_rsa.c
        endif

        BHSM_DEFINES += BSP_SECURE_RSA=1
        BHSM_DEFINES += BSP_SECURE_RSA_IN_VKL=1
    endif

    # Secure Access Functions
    ifeq ($(BHSM_SECURE_ACCESS),ON)
        BHSM_SECURE_ACCESS_MK=1
    endif

    ifeq ($(BHSM_SECURE_ACCESS),OFF)
        BHSM_SECURE_ACCESS_MK=0
    endif

    ifeq ($(BHSM_SECURE_ACCESS_MK),1)
        BHSM_SECURE_ACCESS_FEATURE=1
        #$(warning  ..........BHSM SECURE_ACCESS Feature is built in with BHSM_SECURE_ACCESS_MK= $(BHSM_SECURE_ACCESS_MK) )
        BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_access \
                    $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_access/aegis
        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/secure_access/bhsm_secure_access.c
        endif
    endif

    # IP Licensing Functions
    ifeq ($(BHSM_IP_LICENSING),ON)
        BHSM_IP_LICENSING_MK=1
    endif

    ifeq ($(BHSM_IP_LICENSING),OFF)
        BHSM_IP_LICENSING_MK=0
    endif

    ifeq ($(BHSM_IP_LICENSING_MK),1)
        BHSM_IP_LICENSING_FEATURE=1
        #$(warning  ..........BHSM IP_LICENSING Feature is built in with BHSM_IP_LICENSING_MK= $(BHSM_IP_LICENSING_MK) )
        BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/ip_licensing \
                    $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/ip_licensing/aegis
        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/ip_licensing/bhsm_ip_licensing.c
        endif
    endif


    # Debug_RdWr feature
    ifeq ($(BHSM_DEBUG_RD_WR),ON)
        BHSM_DEBUG_RD_WR_MK=1
    endif

    ifeq ($(BHSM_DEBUG_RD_WR),OFF)
        BHSM_DEBUG_RD_WR_MK=0
    endif

    ifeq ($(BHSM_DEBUG_RD_WR_MK),1)
        BHSM_DEBUG_RD_WR_FEATURE=1
        #$(warning  ..........BHSM DEBUG_RD_WR Feature is built in with BHSM_DEBUG_RD_WR_MK= $(BHSM_DEBUG_RD_WR_MK) )

        BHSM_INCLUDES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/debug_rdwr

        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += 	$(MAGNUM)/portinginterface/hsm/src/${CORETECH}/debug_rdwr/bhsm_debug_rdwr.c
        endif
    endif

    #######################################################################
    BHSM_KEY_LADDER_FEATURE=1

    BHSM_OTPMSP_FEATURE=1
    #######################################################################

    # Module Includes
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common
    ifeq ($(findstring $(BCHP_CHIP), 7550,7408), $(BCHP_CHIP))
        #$(warning  ..........No Aegis header files needed for 7550  )
    else
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/${CORETECH}/common/aegis
    endif

    ifdef BHSM_KEY_LADDER_FEATURE
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/keyladder
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/keyladder/bhsm_keyladder.c
    endif

    ifeq ($(findstring $(BCHP_CHIP), 7550,7408), $(BCHP_CHIP))
        #$(warning  ..........No Aegis header files needed for 7550  )
    else
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/keyladder/aegis
    endif
endif


# automatically pick up the customer specific shared header files, cust_x or cust_generic
ifeq ($(findstring $(BCHP_CHIP), 7550,7408), $(BCHP_CHIP))
    #$(warning  ..........No customer specific header files needed for 7550  )
else
    ifeq ($(filter $(CORETECH), core65nmASKM), $(CORETECH))
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/${CORETECH}/common/cust_$(BHSM_CUST_DIR)
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/${CORETECH}/common/cust_$(BHSM_CUST_DIR)/aegis
    else
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/${CORETECH}/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/cust_$(BHSM_CUST_DIR)
        BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/include/${CORETECH}/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/cust_$(BHSM_CUST_DIR)/aegis
    endif
endif

# Module Sources
ifeq ($(filter $(CORETECH), coreNoBSP), $(CORETECH))
    ifeq ($(findstring $(BCHP_CHIP), 7550, 7408), $(BCHP_CHIP))
        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm.c
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_priv.c
        endif
    else
        ifneq ($(HSM_SOURCE_AVAILABLE),n)
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/common/bhsm.c
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/common/bhsm_priv.c
        endif
    endif
else
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_priv.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_misc.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_user_cmds.c
    endif
endif

ifeq ($(BHSM_SECURE_OTPMSP_MK),1)
    ifdef BHSM_OTPMSP_FEATURE
        ifeq ($(filter $(CORETECH), coreNoBSP), $(CORETECH))
            ifeq ($(findstring $(BCHP_CHIP), 7550, 7408), $(BCHP_CHIP))
                BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp
                BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp/aegis

                ifneq ($(HSM_SOURCE_AVAILABLE),n)
                    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp/bhsm_otpmsp.c
                endif
            else
                BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/otpmsp
                BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/otpmsp/aegis

                ifneq ($(HSM_SOURCE_AVAILABLE),n)
                    BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/otpmsp/bhsm_otpmsp.c
                endif
            endif
        else
            BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp
            BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp/aegis
            ifneq ($(HSM_SOURCE_AVAILABLE),n)
                BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/otpmsp/bhsm_otpmsp.c
            endif
        endif

        ifeq ($(BHSM_PROG_PATTERN),ON)
            BHSM_DEFINES += BSP_OTP_PROG_PATTERN_SEQUENCE=1
        endif
    endif
endif

ifeq ($(BHSM_OTPID),ON)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common
    # $(warning  ..........BHSM_OTPID is enabled  lagacy )

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_otp_id.c
    endif

    BHSM_DEFINES += BHSM_OTPID=1
endif

# Extended options

ifeq ($(BHSM_IPTV_MK),1)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/iptv

    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/iptv/bhsm_iptv.c
    endif

    BHSM_DEFINES += BHSM_IPTV=1
    BHSM_DEFINES += BSP_MSIPTV
endif

ifeq ($(BHSM_IPTV_MK),0)
    #CFLAGS += -DBHSM_IPTV=0
    BHSM_DEFINES += BHSM_IPTV=0
endif

ifeq ($(BHSM_BSECK_MK),1)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify/aegis
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/region_verify/bhsm_bseck.c
    endif
    #CFLAGS += -DBSP_DOWNLOAD_CODE=1
    BHSM_DEFINES += BSP_DOWNLOAD_CODE=1
endif

ifeq ($(BHSM_SECURE_KEYLADDER_MK),1)
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        ifeq ($(filter $(CORETECH), coreNoBSP), $(CORETECH))
            ifeq ($(findstring $(BCHP_CHIP), 7550, 7408), $(BCHP_CHIP))
                BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/keyladder/bhsm_keyladder_enc.c
            else
                BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/keyladder/bhsm_keyladder_enc.c
            endif
        else
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/keyladder/bhsm_keyladder_enc.c
        endif
    endif
endif


ifeq ($(BHSM_SECURE_MISC_MK),1)
    ifneq ($(HSM_SOURCE_AVAILABLE),n)
        ifeq ($(filter $(CORETECH), coreNoBSP), $(CORETECH))
            ifeq ($(findstring $(BCHP_CHIP), 7550, 7408), $(BCHP_CHIP))
                BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_misc_enc.c
            else
                BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/$(BCHP_CHIP)/common/bhsm_misc_enc.c
            endif
        else
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/src/${CORETECH}/common/bhsm_misc_enc.c
        endif
    endif
endif
