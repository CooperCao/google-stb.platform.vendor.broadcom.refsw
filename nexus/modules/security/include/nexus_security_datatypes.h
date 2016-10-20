/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2008-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#ifndef NEXUS_SECURITY_DATATYPES_H__
#define NEXUS_SECURITY_DATATYPES_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Security_P_Handle * NEXUS_SecurityHandle;

#define NEXUS_SECURITY_MAX_KEYSLOT_TYPES    8

/* defines to interprete Zeus version. */
#define NEXUS_ZEUS_VERSION_CALC(major,minor)            ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8)                      )
#define NEXUS_ZEUS_VERSION_CALC_3(major,minor,subMinor) ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8) | ((subMinor) & 0xFF))

#define NEXUS_ZEUS_VERSION_MAJOR(zeusVersion)    (((zeusVersion)>>16) & 0xFF)
#define NEXUS_ZEUS_VERSION_MINOR(zeusVersion)    (((zeusVersion)>> 8) & 0xFF)
#define NEXUS_ZEUS_VERSION_SUBMINOR(zeusVersion) ( (zeusVersion)      & 0xFF)

#define NEXUS_ZEUS_VERSION  NEXUS_ZEUS_VERSION_CALC_3(NEXUS_SECURITY_ZEUS_VERSION_MAJOR, NEXUS_SECURITY_ZEUS_VERSION_MINOR,NEXUS_SECURITY_ZEUS_VERSION_SUBMINOR)

/* defines to interprete the BFW firmware version */
#define NEXUS_BFW_VERSION_CALC(major,minor,subMinor) ( (((major) & 0xFF)<<16) |(((minor) & 0xFF)<<8) | ((subMinor) & 0xFF) )
#define NEXUS_BFW_VERSION_MAJOR(bfwVersion)          ( ((bfwVersion)>>16) & 0xFF )
#define NEXUS_BFW_VERSION_MINOR(bfwVersion)          ( ((bfwVersion)>>8 ) & 0xFF )
#define NEXUS_BFW_VERSION_SUBMINOR(bfwVersion)       (  (bfwVersion)      & 0xFF )


/**
Summary:
Structure to hold the numbers of key slots allocated for each key slot type.
Description:
See Also:
 NEXUS_SecurityCapabilities
**/
typedef struct NEXUS_KeySlotTableSettings
{
    unsigned     numKeySlotsForType[NEXUS_SECURITY_MAX_KEYSLOT_TYPES];
    bool         enableMulti2Key;   /* Deprecated. True if any Multi2 KeySlots are supported. */
    unsigned     numMulti2KeySlots; /* Number of Multi2 KeySlots.  */
} NEXUS_KeySlotTableSettings;


/**
Summary:
This struct defines the capabilities that are returned from function NEXUS_GetSecurityCapabilities.

Description:

See Also:
NEXUS_GetSecurityCapabilities
**/
typedef struct NEXUS_SecurityCapabilities
{
   struct {
       unsigned zeus;        /* use NEXUS_ZEUS_VERSION_* defines to interpret  */
       unsigned firmware;    /* use NEXUS_BFW_VERSION_* defines to interpret  */
   } version;

   struct{
       bool valid;           /* true if the firmware EPOCH is available */
       uint8_t value;        /* the EPOCH value ranging from 0 to 255*/
   }firmwareEpoch;           /* the EPOCH of the BSECK Firmware (BFW) */

   NEXUS_KeySlotTableSettings keySlotTableSettings;
} NEXUS_SecurityCapabilities;


/**
Summary:
This enum defines the supported crypto alogrithms.
**/
typedef enum NEXUS_SecurityAlgorithm
{
    NEXUS_SecurityAlgorithm_eDvb,    /* DVB will only be used by CA descrambler */
    NEXUS_SecurityAlgorithm_eDvbCsa2 = NEXUS_SecurityAlgorithm_eDvb,
    NEXUS_SecurityAlgorithm_eMulti2,
    NEXUS_SecurityAlgorithm_eDes,
    NEXUS_SecurityAlgorithm_e3DesAba,
    NEXUS_SecurityAlgorithm_e3DesAbc,
    NEXUS_SecurityAlgorithm_eAes,
    NEXUS_SecurityAlgorithm_eAes128 = NEXUS_SecurityAlgorithm_eAes,
    NEXUS_SecurityAlgorithm_eAes192,
    NEXUS_SecurityAlgorithm_eReserved9,

    /* The 4 algorithms below are only supported for M2M */
    NEXUS_SecurityAlgorithm_eC2,
    NEXUS_SecurityAlgorithm_eCss,
    NEXUS_SecurityAlgorithm_eM6Ke,
    NEXUS_SecurityAlgorithm_eM6,

    /* added WMDRM_PD */
    NEXUS_SecurityAlgorithm_eRc4,
    NEXUS_SecurityAlgorithm_eCbcMac,
    NEXUS_SecurityAlgorithm_eWMDrmPd,

    NEXUS_SecurityAlgorithm_eAes128G,
    NEXUS_SecurityAlgorithm_eHdDVD,
    NEXUS_SecurityAlgorithm_eBrDVD,

    NEXUS_SecurityAlgorithm_eDvbCsa3,
    NEXUS_SecurityAlgorithm_eAsf,
    NEXUS_SecurityAlgorithm_eAesCounter = NEXUS_SecurityAlgorithm_eAsf,
    NEXUS_SecurityAlgorithm_eMSMultiSwapMac,
    NEXUS_SecurityAlgorithm_eAsa,

    /* Reserved values */
    NEXUS_SecurityAlgorithm_eReserved0,
    NEXUS_SecurityAlgorithm_eReserved1,
    NEXUS_SecurityAlgorithm_eReserved2,
    NEXUS_SecurityAlgorithm_eReserved3,
    NEXUS_SecurityAlgorithm_eReserved4,
    NEXUS_SecurityAlgorithm_eReserved5,
    NEXUS_SecurityAlgorithm_eReserved6,
    NEXUS_SecurityAlgorithm_eReserved7,
    NEXUS_SecurityAlgorithm_eReserved8,

    NEXUS_SecurityAlgorithm_eMax
} NEXUS_SecurityAlgorithm;

/**
Summary:
This enum defines the supported crypto alogrithm variants.
**/
typedef enum NEXUS_SecurityAlgorithmVariant
{
    NEXUS_SecurityAlgorithmVariant_eEcb,
    NEXUS_SecurityAlgorithmVariant_eXpt = NEXUS_SecurityAlgorithmVariant_eEcb,  /* for NEXUS_SecurityAlgorithm_eDvb, scramble level */
    NEXUS_SecurityAlgorithmVariant_eCbc,
    NEXUS_SecurityAlgorithmVariant_ePes = NEXUS_SecurityAlgorithmVariant_eCbc,  /* for NEXUS_SecurityAlgorithm_eDvb, scramble level */
    NEXUS_SecurityAlgorithmVariant_eCounter,
    NEXUS_SecurityAlgorithmVariant_eRCbc,
    NEXUS_SecurityAlgorithmVariant_eMax
} NEXUS_SecurityAlgorithmVariant;

/**
Summary:
This enum defines the customer modes.
**/
typedef enum NEXUS_Security_CustomerMode
{
    NEXUS_Security_CustomerMode_eGeneric,
    NEXUS_Security_CustomerMode_eReserved1,
    NEXUS_Security_CustomerMode_eReserved2,
    NEXUS_Security_CustomerMode_eReserved3,
    NEXUS_Security_CustomerMode_eMax
} NEXUS_Security_CustomerMode;

/**
Summary:
This enum defines the supported termination modes.
For pairs of Enums, first one for ASKM; second for regular
**/
typedef enum NEXUS_SecurityTerminationMode
{
    NEXUS_SecurityTerminationMode_eClear,
    NEXUS_SecurityTerminationMode_eCbcResidual,
    NEXUS_SecurityTerminationMode_eBlock = NEXUS_SecurityTerminationMode_eCbcResidual,
    NEXUS_SecurityTerminationMode_eScte52Term = NEXUS_SecurityTerminationMode_eCbcResidual,
    NEXUS_SecurityTerminationMode_eReserved2,
    NEXUS_SecurityTerminationMode_eCipherStealing = NEXUS_SecurityTerminationMode_eReserved2,
    NEXUS_SecurityTerminationMode_eCtsCpcm,
    NEXUS_SecurityTerminationMode_eCipherStealingComcast = NEXUS_SecurityTerminationMode_eCtsCpcm,
    NEXUS_SecurityTerminationMode_eReserved4,
    NEXUS_SecurityTerminationMode_eReserved5,
    NEXUS_SecurityTerminationMode_eReserved6,
    NEXUS_SecurityTerminationMode_eTsAndPacket,
    NEXUS_SecurityTerminationMode_ePacket,
    NEXUS_SecurityTerminationMode_eCbcMac,
    NEXUS_SecurityTerminationMode_eCmac,
    NEXUS_SecurityTerminationMode_eMax
} NEXUS_SecurityTerminationMode;


typedef enum NEXUS_SecurityCounterMode
{
    NEXUS_SecurityCounterMode_eGenericFullBlocks,
    NEXUS_SecurityCounterMode_eGenericAllBlocks,
    NEXUS_SecurityCounterMode_eIvPlayBackFullBlocks,
    NEXUS_SecurityCounterMode_ePartialBlockInNextPacket,
    NEXUS_SecurityCounterMode_eSkipPesHeaderAllBlocks,
    NEXUS_SecurityCounterMode_eMax
}NEXUS_SecurityCounterMode;

typedef enum NEXUS_SecurityIVMode
{
    NEXUS_SecurityIVMode_eRegular,
    NEXUS_SecurityIVMode_eReserved1,
    NEXUS_SecurityIVMode_eReserved2,
    NEXUS_SecurityIVMode_eNoPreProcWriteBack,
    NEXUS_SecurityIVMode_eMax
} NEXUS_SecurityIVMode;

typedef enum NEXUS_SecuritySolitarySelect
{
    NEXUS_SecuritySolitarySelect_eClear,
    NEXUS_SecuritySolitarySelect_eSa,
    NEXUS_SecuritySolitarySelect_eCbcXorIv,
    NEXUS_SecuritySolitarySelect_eIv1 = NEXUS_SecuritySolitarySelect_eCbcXorIv,
    NEXUS_SecuritySolitarySelect_eReserved3,
    NEXUS_SecuritySolitarySelect_eXorIv = NEXUS_SecuritySolitarySelect_eReserved3,
    NEXUS_SecuritySolitarySelect_eReserved4,
    NEXUS_SecuritySolitarySelect_eMax

} NEXUS_SecuritySolitarySelect;

/**
Summary:
Deprecated: Use unsigned integer instead.
                   This enum did define the supported Module IDs used for platforms with ASKM support.
**/
#define NEXUS_SecurityAskmModuleID_eZero        (0)
#define NEXUS_SecurityAskmModuleID_eModuleID_3  (3)
#define NEXUS_SecurityAskmModuleID_eModuleID_4  (4)
#define NEXUS_SecurityAskmModuleID_eModuleID_5  (5)
#define NEXUS_SecurityAskmModuleID_eModuleID_6  (6)
#define NEXUS_SecurityAskmModuleID_eModuleID_7  (7)
#define NEXUS_SecurityAskmModuleID_eModuleID_8  (8)
#define NEXUS_SecurityAskmModuleID_eModuleID_9  (9)
#define NEXUS_SecurityAskmModuleID_eModuleID_10 (10)
#define NEXUS_SecurityAskmModuleID_eModuleID_11 (11)
#define NEXUS_SecurityAskmModuleID_eModuleID_12 (12)
#define NEXUS_SecurityAskmModuleID_eModuleID_13 (13)
#define NEXUS_SecurityAskmModuleID_eModuleID_14 (14)
#define NEXUS_SecurityAskmModuleID_eModuleID_15 (15)
#define NEXUS_SecurityAskmModuleID_eModuleID_16 (16)
#define NEXUS_SecurityAskmModuleID_eModuleID_17 (17)
#define NEXUS_SecurityAskmModuleID_eModuleID_18 (18)
#define NEXUS_SecurityAskmModuleID_eModuleID_19 (19)
#define NEXUS_SecurityAskmModuleID_eModuleID_20 (20)
#define NEXUS_SecurityAskmModuleID_eModuleID_21 (21)
#define NEXUS_SecurityAskmModuleID_eModuleID_22 (22)
#define NEXUS_SecurityAskmModuleID_eModuleID_23 (25)
#define NEXUS_SecurityAskmModuleID_eModuleID_24 (24)
#define NEXUS_SecurityAskmModuleID_eModuleID_25 (25)
#define NEXUS_SecurityAskmModuleID_eMax         (26)

#define NEXUS_SECURITY_MAX_HOST_FIRMWARE_KEYLADDER_MODULE_ID           127
#define NEXUS_SECURITY_MAX_HOST_HARDWARE_KEYLADDER_MODULE_ID           224
#define NEXUS_SECURITY_MAX_SAGE_KEYLADDER_MODULE_ID                    255

/**
Summary:
This enum defines the choices for OTP ID Select used for platforms with ASKM support.
**/
typedef enum NEXUS_SecurityOtpId
{
    NEXUS_SecurityOtpId_eOtpVal,  /* Use value programmed in OTP */
    NEXUS_SecurityOtpId_eOneVal,  /* Use value 1   */
    NEXUS_SecurityOtpId_eZeroVal, /* Use value 0   */
    NEXUS_SecurityOtpId_eMax
} NEXUS_SecurityOtpId;


/**
Summary:
This enum defines the choices for Key2 selection used for platforms with ASKM support.
**/
typedef enum NEXUS_SecurityKey2Select
{
    NEXUS_SecurityKey2Select_eRealKey,
    NEXUS_SecurityKey2Select_eReserved1,
    NEXUS_SecurityKey2Select_eFixedKey,
    NEXUS_SecurityKey2Select_eMax
} NEXUS_SecurityKey2Select;


/**
Summary:
This enum defines the supported DVB scrambling levels.
**/
typedef enum NEXUS_SecurityDvbScrambleLevel
{
    NEXUS_SecurityDvbScrambleLevel_eTs,
    NEXUS_SecurityDvbScrambleLevel_ePes,
    NEXUS_SecurityDvbScrambleLevel_eMax
} NEXUS_SecurityDvbScrambleLevel;

/**
Summary:
This enum defines the supported AES counter modes.
**/
typedef enum NEXUS_SecurityAesCounterSize
{
    NEXUS_SecurityAesCounterSize_e32Bits,
    NEXUS_SecurityAesCounterSize_e64Bits,
    NEXUS_SecurityAesCounterSize_e96Bits,
    NEXUS_SecurityAesCounterSize_e128Bits,
    NEXUS_SecurityAesCounterSize_e128BitsS = NEXUS_SecurityAesCounterSize_e128Bits,
    NEXUS_SecurityAesCounterSize_eMax
} NEXUS_SecurityAesCounterSize;

/**
Summary:
This enum defines crypto operations.
**/
typedef enum NEXUS_SecurityOperation
{
    NEXUS_SecurityOperation_eEncrypt,
    NEXUS_SecurityOperation_eDecrypt,
    NEXUS_SecurityOperation_ePassThrough, /* M2M only */
    NEXUS_SecurityOperation_eMax
}  NEXUS_SecurityOperation;

/**
Summary:
This enum defines the supported key entry types.
**/
typedef enum NEXUS_SecurityKeyType
{
    NEXUS_SecurityKeyType_eOdd,      /* ODD key */
    NEXUS_SecurityKeyType_eEven,     /* EVEN key */
    NEXUS_SecurityKeyType_eClear,    /* CLEAR key */
    NEXUS_SecurityKeyType_eIv,       /* Initial Vector (for chips which do not support per-key IVs) */
    NEXUS_SecurityKeyType_eOddAndEven,/* For configuring a single algorithm for both ODD and EVEN types at the same time */
    NEXUS_SecurityKeyType_eMax
} NEXUS_SecurityKeyType;

/**
Summary:
This enum defines the IV types when associated with specific keys.
**/
typedef enum NEXUS_SecurityKeyIVType
{
    NEXUS_SecurityKeyIVType_eNoIV,
    NEXUS_SecurityKeyIVType_eIV,         /* Normal IV */
    NEXUS_SecurityKeyIVType_eAesShortIV,
    NEXUS_SecurityKeyIVType_eMax
} NEXUS_SecurityKeyIVType;


/**
Summary:
This enum defines the supported key modes.
**/
typedef enum NEXUS_SecurityKeyMode
{
    NEXUS_SecurityKeyMode_eRegular,
    NEXUS_SecurityKeyMode_eDes56,
    NEXUS_SecurityKeyMode_eReserved2,
    NEXUS_SecurityKeyMode_eReserved3,
    NEXUS_SecurityKeyMode_eDvbConformance,
    NEXUS_SecurityKeyMode_eCwc,
    NEXUS_SecurityKeyMode_eMax
}   NEXUS_SecurityKeyMode;



/**
Summary:
This enum defines the algorithm configuration options for M2M/CACP keyslot.
**/
typedef enum NEXUS_SecurityAlgorithmConfigDestination
{
    NEXUS_SecurityAlgorithmConfigDestination_eCa,
    NEXUS_SecurityAlgorithmConfigDestination_eCps,
    NEXUS_SecurityAlgorithmConfigDestination_eCpd,
    NEXUS_SecurityAlgorithmConfigDestination_eRmx,
    NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,
    NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem2,
    NEXUS_SecurityAlgorithmConfigDestination_eHdmi,
    NEXUS_SecurityAlgorithmConfigDestination_eMax
} NEXUS_SecurityAlgorithmConfigDestination;

/**
Summary:
This enum defines the algorithm configuration options on SC bits for CACP keyslot.
**/
typedef enum NEXUS_SecurityAlgorithmScPolarity
{
    NEXUS_SecurityAlgorithmScPolarity_eClear,
    NEXUS_SecurityAlgorithmScPolarity_eMpegOdd,
    NEXUS_SecurityAlgorithmScPolarity_eAtscClear = NEXUS_SecurityAlgorithmScPolarity_eMpegOdd,
    NEXUS_SecurityAlgorithmScPolarity_eEven,
    NEXUS_SecurityAlgorithmScPolarity_eOdd,
    NEXUS_SecurityAlgorithmScPolarity_eMax
} NEXUS_SecurityAlgorithmScPolarity;


/**
Summary:
This enum defines the supported Virtual Key Ladder ID for 7420B0.
**/
typedef enum NEXUS_SecurityVirtualKeyladderID
{
    NEXUS_SecurityVirtualKeyladderID_eVKL0,
    NEXUS_SecurityVirtualKeyladderID_eVKL1,
    NEXUS_SecurityVirtualKeyladderID_eVKL2,
    NEXUS_SecurityVirtualKeyladderID_eVKL3,
    NEXUS_SecurityVirtualKeyladderID_eVKL4,
    NEXUS_SecurityVirtualKeyladderID_eVKL5,
    NEXUS_SecurityVirtualKeyladderID_eVKL6,
    NEXUS_SecurityVirtualKeyladderID_eVKL7,
    NEXUS_SecurityVirtualKeyladderID_eVKLDummy,
    NEXUS_SecurityVirtualKeyladderID_eSWKey,
    NEXUS_SecurityVirtualKeyladderID_eReserved11,
    NEXUS_SecurityVirtualKeyladderID_eReserved12,
    NEXUS_SecurityVirtualKeyladderID_eReserved13,
    NEXUS_SecurityVirtualKeyladderID_eReserved14,
    NEXUS_SecurityVirtualKeyladderID_eReserved15,
    NEXUS_SecurityVirtualKeyladderID_eMax
} NEXUS_SecurityVirtualKeyladderID;


/**
Summary:
This enum defines packet types for SC bit rewriting.
**/
typedef enum NEXUS_SecurityPacketType {
    NEXUS_SecurityPacketType_eGlobal,
    NEXUS_SecurityPacketType_eRestricted,
    NEXUS_SecurityPacketType_eMax
} NEXUS_SecurityPacketType;

/**
Summary:
This struct defines settings for algorithm configuration.
**/
typedef struct NEXUS_SecurityAlgorithmSettings
{
    NEXUS_SecurityAlgorithm algorithm;
    NEXUS_SecurityAlgorithmVariant algorithmVar; /* Cipher chain mode for selected cipher */
    NEXUS_SecurityTerminationMode terminationMode; /* Termination Type for residual block to be ciphered */
    NEXUS_SecurityCounterMode     aesCounterMode;  /* for AesCounter algorithm    */
    NEXUS_SecurityAesCounterSize  aesCounterSize; /* This member is required for AES counter mode */
    NEXUS_SecurityIVMode          ivMode; /* Initialization Vector (IV) type - to be used by the selected cipher */
    NEXUS_SecuritySolitarySelect  solitarySelect; /* Process selection for short blocks in data packets to be ciphered */
    uint32_t                      caVendorID; /* Conditional Access Vendor ID - assigned by Broadcom for cipher key selection/computation */
    unsigned                      askmModuleID; /* Used in ASKM mode for key2 generation - one-to-one mapped into Customer Submode */
    NEXUS_SecurityOtpId           otpId; /* Source for OTP ID - used in ASKM mode for key2 generation */
    NEXUS_SecurityKey2Select      key2Select;     /* For 40-nm platforms */
    NEXUS_SecurityDvbScrambleLevel dvbScramLevel; /* This member is only required for DVB */
    NEXUS_SecurityKeyType          keyDestEntryType;

    /* This member is NOT required for CA */
    NEXUS_SecurityOperation operation; /* Appropriate settings:
                                            CA:   N/A
                                            M2M:  NEXUS_SecurityOperation_eEncrypt or NEXUS_SecurityOperation_eDecrypt or NEXUS_SecurityOperation_ePassThrough
                                            CP:   NEXUS_SecurityOperation_eEncrypt or NEXUS_SecurityOperation_eDecrypt
                                            RMX:  NEXUS_SecurityOperation_eEncrypt or NEXUS_SecurityOperation_eDecrypt
                                        */
    NEXUS_SecurityAlgorithmConfigDestination dest; /* This member is required only for CACP keyslot configuration */
    NEXUS_SecurityKeySource keySource; /* This member should only be changed from the default for keyladder operations */
    bool          bMulti2Config;
    uint16_t      multi2KeySelect; /* This member should only be changed from the default for Multi2 operations */
    uint8_t       multi2RoundCount;

    bool enableExtKey;  /* For Zeus3.0 M2M, if either enableExtKey or enableExtIv it true, the client must insert the key and IV before the data. */
    bool enableExtIv;   /* For Zeus4.x and later, if either enableExtKey or enableExtIv it true, an "external keyslot" will be */
                        /* allocated for this ketslot. Use function NEXUS_KeySlotExternalKeyData                               */
						/* to return parameters required to compile the associated BTP packet.                                 */

    bool bEncryptBeforeRave;        /* this setting is for ASKM platforms only */
    bool bRestrictEnable;
    bool bGlobalEnable;

    /* The following control rewriting the SC bits on newer platforms. */
    bool modifyScValue[NEXUS_SecurityPacketType_eMax];
    NEXUS_SecurityAlgorithmScPolarity scValue[NEXUS_SecurityPacketType_eMax];

    /* The following are only required for CA/CP on 40nm, 28nm chips */
    uint8_t  keyOffset;
    uint8_t  ivOffset;
    uint8_t  mscLengthSelect;
    NEXUS_Security_CustomerMode customerType;
    uint8_t  macRegSelect;
	bool     macNonSecureRegRead;
    bool     IrModEnable;
    uint8_t  dvbCsa2keyCtrl;
    uint8_t  dvbCsa2ivCtrl;
    bool     dvbCsa2modEnabled;
    uint8_t  dvbCsa3dvbcsaVar;
    uint8_t  dvbCsa3permutation;
    bool     dvbCsa3modXRC;
    bool     EsModEnable;


    /* The following group of settings is for Zeus 4.0 platforms only */
    bool     RpipeFromRregion;
    bool     RpipeFromGregion;
    bool     RpipeToRregion;
    bool     RpipeToGregion;
    bool     GpipeFromRregion;
    bool     GpipeFromGregion;
    bool     GpipeToRregion;
    bool     GpipeToGregion;

    /* The following is for ASKM 40nm Zeus 2 only. This array corresponds to bGpipePackets2Rregion and bRpipePackets2Rregion. */
    bool bRoutePipeToRestrictedRegion[NEXUS_SecurityPacketType_eMax];


    /* The following are only required for CA and CP for pre-Zeus 4.x platforms */
    bool bScAtscMode;
    bool bAtscModEnable;
    bool bGlobalDropPktEnable;
    bool bRestrictDropPktEnable;
    bool bGlobalRegionOverwrite;
    bool bRestrictSourceDropPktEnable; /* ASKM 40nm Zeus 2 only */

    /* The following are for CA/CP on legacy (pre-ASKM) systems only */
    bool bScPolarityEnable;
    bool bSynEnable;
    bool bCPDDisable;
    bool bCPSDisable;


    /* The following are only required for M2M - pre-Zeus 4.x platforms */
    bool mscBitSelect;
    bool bDisallowGG;
    bool bDisallowGR;
    bool bDisallowRG;
    bool bDisallowRR;
    bool enableTimestamps; /* This member should be set for m2m dma and pvr operations on transport streams with timestamps enabled */


    /* The following are legacy settings and should generally not be used */
    bool testKey2Select; /* Test Key2 to be used for cipher key generation when set. (For debugging purposes only) */
} NEXUS_SecurityAlgorithmSettings;

/**
Summary:
This struct defines the clear key structure.

Description:
This structure contains the clear parameters which will be filled
by the caller of NEXUS_Security_LoadClearKey.

See Also:
NEXUS_Security_LoadClearKey
**/
typedef struct NEXUS_SecurityClearKey
{
    unsigned int keySize;               /* keySize - size of the key */
    NEXUS_SecurityKeyType keyEntryType; /* keyEntryType - key entry type */
    NEXUS_SecurityKeyIVType keyIVType;  /* IV type for this key */
	NEXUS_SecurityKeyMode   keyMode;
    NEXUS_SecurityAlgorithmConfigDestination dest;/* This member is required only for CACP keyslot configuration */
    NEXUS_SecurityAlgorithmScPolarity sc01Polarity[NEXUS_SecurityPacketType_eMax]; /* Packets with SC bits 0x01 will be treated as this polarity */
    NEXUS_SecurityAlgorithmScPolarity sc01GlobalMapping;
    uint8_t keyData[64];    /* keyData - key data array */
} NEXUS_SecurityClearKey;


/**
Summary:
This struct is used to retrieve External Key Data for inclusion in a BTP pactket.

See Also:
NEXUS_KeySlot_GetExternalKeyData
**/
typedef struct NEXUS_KeySlotExternalKeyData
{
    unsigned slotIndex; /* Index into external key-slot table.  */

    struct {
        bool valid;      /* Indicates that the associated index is valid.  */
        unsigned offset;  /* offset into external key slot (in 64bit steps). */
    } key, iv;
}NEXUS_KeySlotExternalKeyData;


/**
Summary:
This struct defines the multi2 configuration structure.

Description:
This structure contains the parameters which will be filled
by the caller of NEXUS_Security_ConfigMulti2.

See Also:
NEXUS_Security_ConfigMulti2
**/
typedef struct NEXUS_SecurityMulti2Settings
{
    uint16_t multi2KeySelect;
    uint8_t multi2Rounds;
    uint8_t multi2SysKey[32];
} NEXUS_SecurityMulti2Settings;

/**
Summary:
This enum defines the key to be invalidated, source, destination, or both.
**/
typedef enum NEXUS_SecurityInvalidateKeyFlag
{
    NEXUS_SecurityInvalidateKeyFlag_eSrcKeyOnly,
    NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly,
    NEXUS_SecurityInvalidateKeyFlag_eAllKeys,
    NEXUS_SecurityInvalidateKeyFlag_eMax
} NEXUS_SecurityInvalidateKeyFlag;


/**
Summary:
This structure provides information for CA key invalidation

Description:
See Also:
NEXUS_Security_InvalidateKey
**/
typedef struct NEXUS_SecurityInvalidateKeySettings
{
    NEXUS_SecurityInvalidateKeyFlag             invalidateKeyType;
	bool                                        invalidateAllEntries;
    NEXUS_SecurityVirtualKeyladderID            virtualKeyLadderID;
    NEXUS_SecurityKeySource                     keySrc;
    NEXUS_SecurityAlgorithmConfigDestination    keyDestBlckType;
    NEXUS_SecurityKeyType                       keyDestEntryType;
} NEXUS_SecurityInvalidateKeySettings;


/**
Summary:
The following are region IDs to be used in the region bitmaps for G-pipe output, R-pipe output,
and input to G-pipe and R-pipe.
**/
#define NEXUS_SECURITY_G_REGION           0x01
#define NEXUS_SECURITY_R_REGION_0         0x02
#define NEXUS_SECURITY_R_REGION_1         0x04
#define NEXUS_SECURITY_R_REGION_2         0x08
#define NEXUS_SECURITY_R_REGION_4         0x10
#define NEXUS_SECURITY_R_REGION_5         0x20
#define NEXUS_SECURITY_R_REGION_6         0x40
#define NEXUS_SECURITY_R_REGION_7         0x80
/**
Summary:
The following are specified region ID remapping from Generic.
**/
#define NEXUS_SECURITY_REGION_GLR  NEXUS_SECURITY_G_REGION     /* GLR - Global Region */
#define NEXUS_SECURITY_REGION_CRR  NEXUS_SECURITY_R_REGION_0   /* CRR - Compresses Restricted Region, compressed referring to compressed media */


/**
Summary:
This structure provides information for setting up the Global Control Word for a given key slot.
The settings specified in this structure are applied for all destination blocks (CA/CPS/CPD) targeted by the given key slots,
and for all key slot entries (Odd, Even, Clear, IV) of the given key slot.
**/
typedef struct NEXUS_KeySlotGlobalControlWordSettings
{
	uint8_t            inputRegion;        /* Region(s) selected as input to G-pipe and R-pipe */
	uint8_t            rPipeOutput;        /* Region(s) connected to R-pipe output */
	uint8_t            gPipeOutput;        /* Region(s) connected to G-pipe output */
	bool               encryptBeforeRave;  /* if packets need to be encrypted before RAVE */
} NEXUS_KeySlotGlobalControlWordSettings;

/**
Summary:
This error code is used to pass HSM error codes to the application.
The value from HSM is embedded in the lowest byte via OR.
**/
#define NEXUS_SECURITY_HSM_ERROR NEXUS_MAKE_ERR_CODE(0x109, 0)

#define NEXUS_SECURITY_IP_LICENCE_SIZE     (64)

/** NEXUS_SecurityCustomerMode  DEPRECATED **/
typedef enum NEXUS_SecurityCustomerMode {
    NEXUS_SecurityCustomerMode_eGeneric,
    NEXUS_SecurityCustomerMode_eDvs042,
    NEXUS_SecurityCustomerMode_eDesCts,
    NEXUS_SecurityCustomerMode_eDvbCsa
} NEXUS_SecurityCustomerMode;

/**
Summary:
Settings used to configure the Security module.

Description:

See Also:
NEXUS_SecurityModule_GetDefaultInternalSettings
NEXUS_SecurityModule_Init
**/
typedef struct NEXUS_SecurityModuleSettings
{
    NEXUS_CommonModuleSettings common;
    NEXUS_SecurityCustomerMode customerMode; /* customerMode is DEPRECATED */
    unsigned int numKeySlotsForType[NEXUS_SECURITY_MAX_KEYSLOT_TYPES];
    bool enableMulti2Key;           /* DEPRECATED, replaced by numMulti2KeySlots. If set true and numMulti2KeySlots is 0, numMulti2KeySlots will be treated as 8. */
    unsigned numMulti2KeySlots;     /* Number of Multi2 KeySlots */
    struct {
        bool valid;
        uint8_t data[NEXUS_SECURITY_IP_LICENCE_SIZE];
    }ipLicense;
} NEXUS_SecurityModuleSettings;



#ifdef __cplusplus
}
#endif

#endif
