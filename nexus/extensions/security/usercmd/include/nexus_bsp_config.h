/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef NEXUS_BSP_CONFIG_H__
#define NEXUS_BSP_CONFIG_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_keyladder.h"


#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_AVDSVD_RANGE                    (8)
#define NEXUS_HMACSHA256_SIGNATURE_SIZE          (32)
#define NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE  (256)   /* The maximum size of a PCIE Max Window size signed command. */



/**
Summary:
Description:
See Also:

**/
typedef struct NEXUS_SecurityRaveCtrlSettings
{
    bool                         bEnableWriteIMem;
    bool                         bEnableReadIMem;
    bool                         bEnableReadDMem;
    bool                         bRAVEEncryptionBypass;
    bool                         bDisableClear;

} NEXUS_SecurityRaveCtrlSettings;

/**
Summary:

Description:

**/
NEXUS_Error NEXUS_Security_RaveControl(
    const NEXUS_SecurityRaveCtrlSettings *pSettings
    );

/**
Summary:
Description:
See Also:

**/
typedef struct NEXUS_SecurityDMABypassSettings
{
    bool                           bDMAXPTPacketBypassEnable;
    bool                           bNMAPacketBypassEnable;
    bool                           bDMAM2MPacketBypassEnable;

} NEXUS_SecurityDMABypassSettings;

/**
Summary:

Description:

**/
NEXUS_Error NEXUS_Security_DMABypassControl(
    const NEXUS_SecurityDMABypassSettings *pSettings
    );


/**
Summary:
Description:
See Also:

**/
typedef struct NEXUS_SecurityM2MEndianSwapSettings
{
    bool                           bCBCMACDataInSwapEnable;
    bool                           bCBCMACDataOutSwapEnable;
    bool                           bCBCMACKeySwapEnable;

} NEXUS_SecurityM2MEndianSwapSettings;

/**
Summary:

Description:

**/
NEXUS_Error NEXUS_Security_M2MEndianSwapControl(
    const NEXUS_SecurityM2MEndianSwapSettings *pSettings
    );


/**
Summary:
Description:
See Also:

**/
typedef struct NEXUS_SecurityForceSCSettings
{
    bool                           bBandSelect_PB;
    unsigned char                  bandNumber;
    bool                           bSCForceEnAll;
    bool                           bSCForceEnNZ;
    unsigned char                  SCForceEnValue;

} NEXUS_SecurityForceSCSettings;

/**
Summary:

Description:

**/
NEXUS_Error NEXUS_Security_ForceSCControl(
    const NEXUS_SecurityForceSCSettings *pSettings
    );



/**
Summary:
Description:
See Also:

**/
typedef struct NEXUS_SecurityXPTM2MStatusOutput
{
    uint32_t                     unM2MSecurityErrStatus;
    uint32_t                     unXPTSecurityErrStatus;

} NEXUS_SecurityXPTM2MStatusOutput;

/**
Summary:

Description:

**/
NEXUS_Error NEXUS_Security_XPTM2MStatusQuery(
    NEXUS_SecurityXPTM2MStatusOutput *pStatusQuery            /* [out] */
    );





/**
Summary:

Description:
See Also:

**/
typedef enum NEXUS_SecurityPCIArchType
{
    NEXUS_SecurityPCIArchType_eNonPCIWin,
    NEXUS_SecurityPCIArchType_ePCIWin,
    NEXUS_SecurityPCIArchType_ePCIeWin,

    NEXUS_SecurityPCIArchType_eMax
}    NEXUS_SecurityPCIArchType;


/**
Summary:

Description:
See Also:

**/


typedef enum NEXUS_SecurityArchSelect
{
    NEXUS_SecurityArchSelect_eSel00,
    NEXUS_SecurityArchSelect_eSel01,
    NEXUS_SecurityArchSelect_eSel02,
    NEXUS_SecurityArchSelect_eSel03,

    NEXUS_SecurityArchSelect_eMax

}    NEXUS_SecurityArchSelect;


/**
Summary:

Description:
See Also:

**/


typedef enum NEXUS_SecurityDRAMSelect
{
    NEXUS_SecurityDRAMSelect_eSel00,
    NEXUS_SecurityDRAMSelect_eSel01,

    NEXUS_SecurityDRAMSelect_eMax

}    NEXUS_SecurityDRAMSelect;


/**
Summary:

Description:
See Also:

**/

typedef struct NEXUS_SecuritySRegionSettings
{
    uint32_t                    sRegStartAddress;
    uint32_t                    sRegEndAddress;
    NEXUS_SecurityArchSelect    archType;
    NEXUS_SecurityPCIArchType   pciArchType;
    NEXUS_SecurityDRAMSelect    DRAMType;

} NEXUS_SecuritySRegionSettings;

/**
Summary:

Description:
**/

NEXUS_Error NEXUS_Security_DefineSecureRegion(
    const NEXUS_SecuritySRegionSettings *pSettings
    );


typedef struct NEXUS_SecurityPciEMaxWindowSizeSettings
{
    NEXUS_SecurityVirtualKeyladderID vkl;                            /* DEPRECATED */
    uint8_t  signedCommand[NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE];  /* Header + Command + Signature combined. Provided by CA vendor.  */
    unsigned signedCommandLength;                                    /* must be <= NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE.
                                                                        For backward compat, if 0 it is treated as NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE. */
} NEXUS_SecurityPciEMaxWindowSizeSettings;

typedef NEXUS_SecurityPciEMaxWindowSizeSettings NEXUS_SecurityPciEMaxWindowSizeSettings_t;   /* for backward compatibility */

void NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings(
    NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig  /* [out] */
    );


/**
Description: Configure the maximum amount of memory that can be allocated from each to the Mem Controllers for PCI access.
This is a signed command. It is most likely that the CA-vendor is the signing authority.
Use NEXUS_Security_SetPciERestrictedRange to specify that actual size and location of the memory that can be access from PCI.
**/
NEXUS_Error NEXUS_Security_SetPciEMaxWindowSize (
    const NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig
    );

/**
Description: Allows the client to specify the location and size of memory that is accessible via a PCIe interface.
**/
NEXUS_Error NEXUS_Security_SetPciERestrictedRange(
                           NEXUS_Addr baseOffset,     /* Start address of memory accessible from PCI */
                           size_t size,               /* Size of memory accessible from PCI */
                           unsigned index             /* index of PCIe to be restricted [ 0 .. Max # PciE Controllers - 1 ]*/
                           );



/**
Summary:

Description:
See Also:

**/
typedef enum NEXUS_SecurityAVDType
{
    NEXUS_SecurityAVDType_eVDEC0 = 0, /* Zeus4.1 + */
    NEXUS_SecurityAVDType_eSVD = NEXUS_SecurityAVDType_eVDEC0, /* pre Zeus4  */
    NEXUS_SecurityAVDType_eAVD, /* pre Zeus4  */
    NEXUS_SecurityAVDType_eVICE = 2,
    NEXUS_SecurityAVDType_eVICE1 = 3,
    NEXUS_SecurityAVDType_eVDEC1 = 4, /* Zeus4.1 + */
    NEXUS_SecurityAVDType_eVDEC2 = 5, /* Zeus4.1 + */
    NEXUS_SecurityAVDType_eMax
} NEXUS_SecurityAVDType;


/**
Summary:

Description:
See Also:

**/

typedef struct NEXUS_SecurityAVDSRegRangeSettings
{
    NEXUS_SecurityVirtualKeyladderID    vkl;
    NEXUS_SecurityKeySource             keyLayer;
    NEXUS_SecurityAVDType               VDECId;
    uint32_t                            nRange;
    uint32_t                            loRange[NEXUS_MAX_AVDSVD_RANGE];
    uint32_t                            hiRange[NEXUS_MAX_AVDSVD_RANGE];
    unsigned char                       signature[NEXUS_HMACSHA256_SIGNATURE_SIZE];

} NEXUS_SecurityAVDSRegRangeSettings;




/**
Summary:

Description:

**/

NEXUS_Error NEXUS_Security_AVDSRegistersSetUp(
    const NEXUS_SecurityAVDSRegRangeSettings   *pSettings
    );



/**
Summary:

Description:
See Also:

**/

typedef struct NEXUS_SecurityAVDSRegModifySettings
{
    NEXUS_SecurityVirtualKeyladderID    vkl;
    NEXUS_SecurityKeySource             keyLayer;
    NEXUS_SecurityAVDType               avdID;
    uint32_t                            nAVDReg;
    uint32_t                            regAddr[NEXUS_MAX_AVDSVD_RANGE];
    uint32_t                            regVal[NEXUS_MAX_AVDSVD_RANGE];
    unsigned char                       signature[NEXUS_HMACSHA256_SIGNATURE_SIZE];

} NEXUS_SecurityAVDSRegModifySettings;




/**
Summary:

Description:

**/

NEXUS_Error NEXUS_Security_AVDSRegistersModify(
    const NEXUS_SecurityAVDSRegModifySettings   *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
