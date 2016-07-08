/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef NEXUS_SECURITY_REGVER_PRIV_H__
#define NEXUS_SECURITY_REGVER_PRIV_H__

#include "nexus_types.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NEXUS_REGVER_MAX_REGIONS (32)


#define NEXUS_SECURITY_ERROR_OFFSET_REGION_VERIFICATION (0x0400)
#define NEXUS_SECURITY_REGION_NOT_OTP_ENABLED      NEXUS_MAKE_ERR_CODE(0x109, (NEXUS_SECURITY_ERROR_OFFSET_REGION_VERIFICATION | 1))
#define NEXUS_SECURITY_VERIFICATION_ENABLE_FAILED  NEXUS_MAKE_ERR_CODE(0x109, (NEXUS_SECURITY_ERROR_OFFSET_REGION_VERIFICATION | 2))
#define NEXUS_SECURITY_FAILED_TO_LOAD_REGVER_KEY   NEXUS_MAKE_ERR_CODE(0x109, (NEXUS_SECURITY_ERROR_OFFSET_REGION_VERIFICATION | 3))


/*
    The Region Verification Module authenticates (i.e., verifies) a region of memory against a signature. It is frequently used to
    authenticate firmware code. In this case, the signing authority (Boardcom or CA vendor) will generate and provide a signature
    (nexus_security_regver_signatures.h) that can be used against a particular firmware image.
    The requirement to authentication a firmware category (rave, audio, video, vice, sid)  will be stipulated in OTP. Once authentication,
    most regions will be background checked (this is region dependant), thus any modification to the reqion after this will generate a system reset.
*/


/**
Summary:
This enum defines the supported regions for verification.
 **/
typedef enum NEXUS_SecurityRegverRegionID
{
    NEXUS_SecurityRegverRegionID_eHost00        = 0x00,
    NEXUS_SecurityRegverRegionID_eHost01        = 0x01,
    NEXUS_SecurityRegverRegionID_eHost02        = 0x02,
    NEXUS_SecurityRegverRegionID_eHost03        = 0x03,
    NEXUS_SecurityRegverRegionID_eHost04        = 0x04,
    NEXUS_SecurityRegverRegionID_eHost05        = 0x05,
    NEXUS_SecurityRegverRegionID_eHost06        = 0x06,
    NEXUS_SecurityRegverRegionID_eHost07        = 0x07,
    NEXUS_SecurityRegverRegionID_eRave          = 0x08,
    NEXUS_SecurityRegverRegionID_eRaaga0        = 0x09,
    NEXUS_SecurityRegverRegionID_eAvd0Inner     = 0x0A, /* Zeus 4.1 and pre */
    NEXUS_SecurityRegverRegionID_eAvd0Outer     = 0x0B, /* Zeus 4.1 and pre */
    NEXUS_SecurityRegverRegionID_eVDEC0_IL2A    = 0x0B, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eHvdIla        = 0x0C, /* Zeus 4.1 */
    NEXUS_SecurityRegverRegionID_eSvd0Inner     = 0x0C,
    NEXUS_SecurityRegverRegionID_eVDEC0_ILA     = 0x0C, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eHvdOla        = 0x0D,
    NEXUS_SecurityRegverRegionID_eSvd0Outer     = 0x0D,
    NEXUS_SecurityRegverRegionID_eVDEC0_OLA     = 0x0D, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eAvs           = 0x0E,
    NEXUS_SecurityRegverRegionID_eSvd0Bl        = 0x0E,
    NEXUS_SecurityRegverRegionID_eVice0Pic      = 0x0F,
    NEXUS_SecurityRegverRegionID_eVice0MacroBlk = 0x10,
    NEXUS_SecurityRegverRegionID_eSid0          = 0x11,
    NEXUS_SecurityRegverRegionID_eRaaga1        = 0x12,
    NEXUS_SecurityRegverRegionID_eVice1Pic      = 0x13,
    NEXUS_SecurityRegverRegionID_eVice1MacroBlk = 0x14,
    NEXUS_SecurityRegverRegionID_eRaaga0IntScm  = 0x15,
    NEXUS_SecurityRegverRegionID_eHVD1_ILA      = 0x15,
    NEXUS_SecurityRegverRegionID_eVDEC1_ILA     = 0x15, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eRaaga0IntAud  = 0x16,
    NEXUS_SecurityRegverRegionID_eHVD1_OLA      = 0x16,
    NEXUS_SecurityRegverRegionID_eVDEC1_OLA     = 0x16, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eWebCpu        = 0x17,
    NEXUS_SecurityRegverRegionID_eScpuFsbl      = 0x18,
    NEXUS_SecurityRegverRegionID_eScpuOsApp     = 0x19,
    NEXUS_SecurityRegverRegionID_eScpuGeneric   = 0x1A,
    NEXUS_SecurityRegverRegionID_eScpuScm       = 0x1B,
    NEXUS_SecurityRegverRegionID_eHVD2_ILA      = 0x1C,
    NEXUS_SecurityRegverRegionID_eVDEC2_ILA     = 0x1C, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eHVD2_OLA      = 0x1D,
    NEXUS_SecurityRegverRegionID_eVDEC2_OLA     = 0x1D, /* Zeus 4.2 */
    NEXUS_SecurityRegverRegionID_eMax

} NEXUS_SecurityRegverRegionID;


/*
 * the region configuration
 */
typedef struct NEXUS_SecurityRegionConfiguration
{
    struct{
        unsigned size;             /* Signature size. When greater than 0, the intenally maintained signature for
                                    * the region can be overridden. Not all regions have an internally maintained
                                    * signature, in which case, a signature must be provided */
        void* p;                   /* Pointer to region signature.  */
    }signature;
    unsigned rsaKeyIndex;          /* The RSA key index */

    bool enableInstructionChecker; /* Required for SAGE/BHSM_SUPPORT_HDDTA */
    bool enableBackgroundChecker;  /* Required for SAGE */
    unsigned scmVersion;           /* Required for BHSM_SUPPORT_HDDTA  */
    NEXUS_SecurityVirtualKeyladderID  keyLadderId;     /* Requried for SCPU FSBL region */
    NEXUS_SecurityKeySource           keyLadderLayer;  /* Requried for SCPU FSBL region*/

} NEXUS_SecurityRegionConfiguration;


/**
DEBUG ... captures detailed information on the state of the Regions.
**/
typedef struct NEXUS_SecurityRegionInfoQuery
{
    unsigned int     regionStatus[NEXUS_REGVER_MAX_REGIONS];
} NEXUS_SecurityRegionInfoQuery;



/**
    Intialise region verification module. Call on platform initialisation.
**/
NEXUS_Error NEXUS_Security_RegionVerification_Init_priv( void );


/**
    Unintialise region verification module. Call on platform uninitialisation.
**/
void NEXUS_Security_RegionVerification_UnInit_priv( void );



void NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID regionId,
                                            NEXUS_SecurityRegionConfiguration *pConfiguration  /* [out] */
                                            );

/**
    Setup security to verify the speified region. Most Region Verification clients can use the default values
    returned by NEXUS_Security_RegionGetDefaultConfig
**/
NEXUS_Error NEXUS_Security_RegionConfig_priv ( NEXUS_SecurityRegverRegionID            regionId,
                                          const NEXUS_SecurityRegionConfiguration *pConfiguration
                                          );

/**
    This function verifies the specified region. Depending on region and system configuration, it will initiate
    background checking that will result in a system reset if memory withing the region is modified.
**/
NEXUS_Error NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID regionId,
                                                    void* pRegionAddress,
                                                    unsigned regionSize
                                                    );

/**
    Disable verification of a region. This will disable background checking on a region, also, firmware
    from the region can no longer be executed.
**/
void NEXUS_Security_RegionVerifyDisable_priv( NEXUS_SecurityRegverRegionID regionId );


/**
    DEBUG ... returns detailed information on the state of the Regions.
**/
NEXUS_Error NEXUS_Security_RegionQueryInformation_priv( NEXUS_SecurityRegionInfoQuery *pRegionQuery /* [out] */
                                                );

/**
    Checks if the specified region needs to be verified/authenticated.
**/
bool  NEXUS_Security_RegionVerification_IsRequired_priv( NEXUS_SecurityRegverRegionID regionId );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
