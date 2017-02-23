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

#ifndef BHSM_VERIFY_REG_H__
#define BHSM_VERIFY_REG_H__

#define FLASHMAP_V3

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bhsm_datatypes.h"
#include "bsp_s_mem_auth.h"

#include "bhsm.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_REGION_NUMBER (32)


typedef enum BHSM_VerificationRegionId
{
    BHSM_VerificationRegionId_eHost00        = 0x00,
    BHSM_VerificationRegionId_eHost01        = 0x01,
    BHSM_VerificationRegionId_eHost02        = 0x02,
    BHSM_VerificationRegionId_eHost03        = 0x03,
    BHSM_VerificationRegionId_eHost04        = 0x04,
    BHSM_VerificationRegionId_eHost05        = 0x05,
    BHSM_VerificationRegionId_eHost06        = 0x06,
    BHSM_VerificationRegionId_eHost07        = 0x07,
    BHSM_VerificationRegionId_eRave          = 0x08,
    BHSM_VerificationRegionId_eRaaga0        = 0x09,
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_VerificationRegionId_eSageDownloadRegion0 = 0x0A, /* Zeus 4.2 400 */
    BHSM_VerificationRegionId_eVdec0_Il2A    = 0x0B, /* Zeus 4.2*/
    BHSM_VerificationRegionId_eVdec0_Ila     = 0x0C, /* Zeus 4.2 */
    BHSM_VerificationRegionId_eVdec0_Ola     = 0x0D, /* Zeus 4.2 */
    #else
    BHSM_VerificationRegionId_eAvd0Inner     = 0x0A,
    BHSM_VerificationRegionId_eAvd0Outer     = 0x0B,
    BHSM_VerificationRegionId_eHvd0_Ila      = 0x0C,
    BHSM_VerificationRegionId_eHvd0_Ola      = 0x0D,
    #endif
    BHSM_VerificationRegionId_eSvd0Outer     = 0x0D, /*?*/
    BHSM_VerificationRegionId_eAvs           = 0x0E,
    BHSM_VerificationRegionId_eSvd0Bl        = 0x0E, /*?*/
    BHSM_VerificationRegionId_eSageDownloadRegion1 = 0x0E, /* Zeus 4.2 400 */
    BHSM_VerificationRegionId_eVice0Pic      = 0x0F,
    BHSM_VerificationRegionId_eVice0MacroBlk = 0x10,
    BHSM_VerificationRegionId_eSid0          = 0x11,
    BHSM_VerificationRegionId_eRaaga1        = 0x12,
    BHSM_VerificationRegionId_eVice1Pic      = 0x13,
    BHSM_VerificationRegionId_eVice1MacroBlk = 0x14,
    BHSM_VerificationRegionId_eRaaga0IntScm  = 0x15, /*?*/
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_VerificationRegionId_eVdec1_Ils     = 0x15, /* Zeus 4.2 */
    BHSM_VerificationRegionId_eVdec1_Ols     = 0x16, /* Zeus 4.2 */
    #else
    BHSM_VerificationRegionId_eHvd1_Ila      = 0x15,
    BHSM_VerificationRegionId_eHvd1_Ola      = 0x16,
    #endif
    BHSM_VerificationRegionId_eRaaga0IntAud  = 0x16, /*?*/
    BHSM_VerificationRegionId_eWebCpu        = 0x17,
    BHSM_VerificationRegionId_eSageDownloadRegion2 = 0x17, /* Zeus 4.2 400 */
    BHSM_VerificationRegionId_eScpuFsbl      = 0x18,
    BHSM_VerificationRegionId_eScpuOsApp     = 0x19,
    BHSM_VerificationRegionId_eScpuGeneric   = 0x1A,
    BHSM_VerificationRegionId_eScpuScm       = 0x1B,
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BHSM_VerificationRegionId_eVdec2_Ila     = 0x1C, /* Zeus 4.2 */
    BHSM_VerificationRegionId_eVdec2_Ola     = 0x1D, /* Zeus 4.2 */
    #else
    BHSM_VerificationRegionId_eHvd2_Ila      = 0x1C, /* Zeus 4.2 */
    BHSM_VerificationRegionId_eHvd2_Ola      = 0x1D, /* Zeus 4.2 */
    #endif

    BHSM_VerificationRegionId_eEnd /* Don't use this as a MAX value */
}BHSM_VerificationRegionId_e;




/* Region Status Bits -  anded with Region Status word */
#define REGION_STATUS_DEFINED               0x0001
#define REGION_STATUS_VERIFIED              0x0002
#define REGION_STATUS_ENABLED               0x0100
#define REGION_STATUS_FASTCHKSTART          0x0200
#define REGION_STATUS_FASTCHKFINISH         0x0400
#define REGION_STATUS_FASTCHKPADDINGRESULT  0x0800
#define REGION_STATUS_FASTCHKRESULT         0x1000
#define REGION_STATUS_BKCHKSTART            0x2000
#define REGION_STATUS_BHCHKFINISH           0x4000
#define REGION_STATUS_BKCHKRESULT           0x8000


typedef unsigned BHSM_RegionNumber;

/* The upper and lower addressed of a memory region */
typedef struct BHSM_MemoryLocation
{
    unsigned char startAddressMsb;  /* only required for 28nm parts*/
    uint32_t      startAddress;
    unsigned char endAddressMsb;    /* only required for 28nm parts*/
    uint32_t      endAddress;
} BHSM_MemoryLocation_t;



typedef enum {
    BHSM_CodeLocationRule_eRelocatable = 0,     /*  code is relocatable */
    BHSM_CodeLocationRule_eNotRelocatable = 1,  /*  code is not relocatable */
    BHSM_CodeLocationRule_eMax
} BHSM_CodeLocationRule_t;

#define BSP_STATUS_eRegionVerifyInProgress  0xB3 /*deprecated. Plaese use BHSM_RegionVerificationStatus_eInProgress */

/* The verifiaction state of a region. */
typedef enum
{
    BHSM_RegionVerificationStatus_eVerified          = 0x00,
    BHSM_RegionVerificationStatus_eAlreadyConfigured = 0xB0,
    BHSM_RegionVerificationStatus_eInProgress        = 0xB3,
    BHSM_RegionVerificationStatus_ePending           = 0xB6, /*Configured, but not enabled.*/
    BHSM_RegionVerificationStatus_eNotDefined        = 0xB7,
    BHSM_RegionVerificationStatus_eFailed            = 0xBB,
    BHSM_RegionVerificationStatus_eAlreadyDisabled   = 0xB1,
    BHSM_RegionVerificationStatus_eNotOtpEnabled     = 0xD4,

    BHSM_RegionVerificationStatus_eMax
} BHSM_RegionsVerificationState_e;

/* The verifiaction status of a region. */
typedef struct BHSM_RegionStatus
{
    bool verificationSupported;               /* Region verifcation is supported for this component. */
    bool verificationRequired;                /* OTP requires region to be verifed. */

    BHSM_RegionsVerificationState_e  state;                  /* verification status of a region          */
    BHSM_MemoryLocation_t            region;                 /* Address of region to be verified         */
    BHSM_MemoryLocation_t            signature;              /* Signature locations                      */
    BHSM_MemoryLocation_t            region01;               /* Address of region to be verified (01     */
    BHSM_MemoryLocation_t            signature01;            /* Signature                        (01)    */
} BHSM_RegionStatus_t;

/* represents the veriication status of all regions */
typedef struct BHSM_VerifcationStatus
{
    /* A status word for each of the regions */
    bool paused;
    uint32_t region[MAX_REGION_NUMBER];
} BHSM_VerifcationStatus_t;

/*Summary:
    Configuration parameters for a memory Region
 */
typedef struct BHSM_RegionConfiguration
{

    BHSM_MemoryLocation_t           region;                   /* Address of region to be verified */
    BHSM_MemoryLocation_t           signature;                /* Signature */

    uint32_t                        ucIntervalCheckBw;        /* Number of interval checks to perform every 16 cycles (valid values 1-16) */
    BCMD_ScbBurstSize_e             SCBBurstSize;             /* Effective MIPS Cache line fill size */
    unsigned int                    verifyFailAction;         /* Applies to Region 2 SSBL; BSP FW does chip reset upon verification failure */
    BCMD_VKLID_e                    vkl;                      /* SCPU Virtual key ladder for key used in SCPU FSBL decryption */
    BCMD_KeyRamBuf_e                keyLayer;                 /* SCPU key layer for key used in SCPU FSBL decryption */
    uint32_t                        unRSAKeyID;               /* RSA Key ID  */
    BHSM_CodeLocationRule_t         codeRelocatable;          /* whether this region should have non-relocatable code  */
    uint32_t                        unCPUAccessRights;        /* Access Rights of the CPU running this code */
    BCMD_MemAuth_CpuType_e          cpuType;                  /* CPU type */
    uint32_t                        unEpoch;                  /* Epoch for the code  --- to be checked with the OTP Epoch */
    uint8_t                         epochSelect;               /* Selected Epoch */
    uint32_t                        unEpochMask;              /* Epoch Mask  if >= ZEUS 2.0 */
    uint8_t                         ucEpochSel;               /* Selected Epoch */
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    uint8_t                         svpFwReleaseVersion;      /* The SVP version of the FW. Applicable for BFW400+ */
    uint8_t                         ucSigVersion;             /* Signature Version */
    BCMD_SignatureType_e            ucSigType;                /* Signature type */
    bool                            enforceAuthentication;    /* Force region authentication irrespective of OTP. */
    bool                            disallowDisable;          /* Disallow the region to be disabled once enalbed. */
   #endif
    uint32_t                        unMarketID;               /* Market ID  >= ZEUS 2.0 */
    uint32_t                        unMarketIDMask;           /* Market ID Mask  if >= ZEUS 2.0 */
    bool                            bgCheck;                  /* SCPU. True to enable background checking for the region  */
    bool                            instrChecker;             /* SCPU  True to enable instruction checker for the region */
    uint32_t                        RaagaDMEMStartAddress;    /*  (!SCPU) Raaga DMEM Starting address */
    uint32_t                        RaagaDMEMEndAddress;      /* (!SCPU) Raaga DMEM Ending Address */
    uint32_t                        SCMVersion;               /* SCM version */
    uint32_t                        AVSDMEMStartAddress;      /* AVS DMEM Start address */
    uint32_t                        AVSDMEMEndAddress;        /* AVS DMEM End address */

} BHSM_RegionConfiguration_t;


/* Memory Verification API */

/* Initialise Region verifcaiton on startup. */
BERR_Code BHSM_RegionVerification_Init( BHSM_Handle hHsm );
/* Uninitialise Region verifcaiton on shutdown. */
BERR_Code BHSM_RegionVerification_UnInit( BHSM_Handle hHsm );



/* Configure a region. */
BERR_Code BHSM_RegionVerification_Configure( BHSM_Handle hHsm, BHSM_RegionNumber region, BHSM_RegionConfiguration_t *pConf );

/* Enable verification of all configured regions. */
BERR_Code BHSM_RegionVerification_Enable( BHSM_Handle hHsm );

/* Disable region verification of the specified region. */
BERR_Code BHSM_RegionVerification_Disable( BHSM_Handle hHsm, BHSM_RegionNumber region );

/* Returns the verifcation status of a specifed region. */
BERR_Code BHSM_RegionVerification_Status( BHSM_Handle hHsm, BHSM_RegionNumber region, BHSM_RegionStatus_t *pStatus );


/* Returns a status indications of all of the regions */
BERR_Code BHSM_RegionVerification_QueryStatus( BHSM_Handle hHsm, BHSM_VerifcationStatus_t *pStatus );


#ifdef __cplusplus
}
#endif

#endif /* BHSM_VERIFY_REG_H__ */
