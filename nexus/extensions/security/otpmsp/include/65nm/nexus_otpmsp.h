/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef NEXUS_OTPMSP_H__
#define NEXUS_OTPMSP_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
Summary:
This enum defines types as passed by the MIPS to the 8051 that say whether the programming
command is in bit mode or array mode

Description:
This enumeration defines all the supported programming modes.
*****************************************************************************/
typedef enum NEXUS_OtpMspCmdProgMode
{
    NEXUS_OtpMspCmdProgMode_eEnum    = 0x12,
    NEXUS_OtpMspCmdProgMode_eEnumHal = 0x34,
    NEXUS_OtpMspCmdProgMode_eBit     = 0x78
} NEXUS_OtpMspCmdProgMode;


/*****************************************************************************
Summary:
*****************************************************************************/
typedef enum NEXUS_OtpCmdMsp
{
    NEXUS_OtpCmdMsp_eBackgroundCheckEn       = 2,
    NEXUS_OtpCmdMsp_eFlashRegion0_2          = 3,  /* 3 bits */
    NEXUS_OtpCmdMsp_eBootMemChkEn            = 4,
    NEXUS_OtpCmdMsp_eExportCntrlEnable       = 5,
    NEXUS_OtpCmdMsp_ePubKeyIndx0_3           = 6,  /* 4 bits */

    NEXUS_OtpCmdMsp_eStopB                   = 7,
    NEXUS_OtpCmdMsp_eStartB                  = 8,
    NEXUS_OtpCmdMsp_eStopA                   = 9,
    NEXUS_OtpCmdMsp_eStartA                  = 10,
    
    NEXUS_OtpCmdMsp_eReserved11              = 11,
    NEXUS_OtpCmdMsp_eReserved12              = 12,
    
    NEXUS_OtpCmdMsp_eLockByte0_7             = 13,  /* 8 bits */


    NEXUS_OtpCmdMsp_eLockBitC                = 14,
    NEXUS_OtpCmdMsp_eCrOtpBit0               = 15,
    NEXUS_OtpCmdMsp_eCrOtpBit1               = 16,
    NEXUS_OtpCmdMsp_ePciHostProtectWithCR    = 17,
    NEXUS_OtpCmdMsp_ePciClientProtectWithCR  = 18,
    NEXUS_OtpCmdMsp_eTestPortProtectWithCR   = 19,
    NEXUS_OtpCmdMsp_eForceDramScrambler      = 20,


    NEXUS_OtpCmdMsp_eForceSha256             = 25,
    NEXUS_OtpCmdMsp_eOtpProductId0_3         = 26,  /* 4 bits*/

    NEXUS_OtpCmdMsp_eOtpCtrlBit0_30          = 27,  /* 31 bits*/

    NEXUS_OtpCmdMsp_eOtpCtrlBitSet2          = 36,

    NEXUS_OtpCmdMsp_eOtpCtrlBit44_eDisableCaKeyLadder = 41, /* 1 bit */

    NEXUS_OtpCmdMsp_eRaveVerifyEnable        = 44, /* 1 bit */

    NEXUS_OtpCmdMsp_eDisableMICH             = 46, /* 1 bit */

    NEXUS_OtpCmdMsp_eM2mWmdrmPd              = 51, /* 1 bit */

    NEXUS_OtpCmdMsp_eOtpCtrlBitSet3          = 53, /* 10 bits */
    NEXUS_OtpCmdMsp_eFlashPublicKeyAuthenticate = 54, /* 54 - 1 bit */

    NEXUS_OtpCmdMsp_eHashCodeDigest0         = 56, /* 56 - 32 bits */
    NEXUS_OtpCmdMsp_eHashCodeDigest1         = 57, /* 57 - 32 bits */
    NEXUS_OtpCmdMsp_eHashCodeDigest2         = 58, /* 58 - 32 bits */
    NEXUS_OtpCmdMsp_eHashCodeDigest3         = 59, /* 59 - 32 bits */
    NEXUS_OtpCmdMsp_eHashCodeDigest4         = 60, /* 60 - 32 bits */

    NEXUS_OtpCmdMsp_eOtpFullRomDisable       = 62, /* 62 - 1 bit */
    NEXUS_OtpCmdMsp_eOtpBootRomCrcEn         = 63, /* 63 - 1 bit */
    NEXUS_OtpCmdMsp_eOtpBspCodeBckgrdChkEnable = 64, /* 64 - 1 bit */
    NEXUS_OtpCmdMsp_eAesprime_KeySelect      = 65, /* 65 - 6 bits */
    NEXUS_OtpCmdMsp_eOtpSecurityMode         = 66, /* 66 - 5 bits */

    NEXUS_OtpCmdMsp_eMarketId                = 68, /* 68 - 32 bits */
    NEXUS_OtpCmdMsp_eCurrentEpoch            = 69, /* 69 - 8 bits */
    NEXUS_OtpCmdMsp_eKey0PrimeSigningRules   = 70, /* 70 - 4 bits */
    NEXUS_OtpCmdMsp_eKey0SigningRules        = 71, /* 71 - 4 bits */

    NEXUS_OtpCmdMsp_eExtendedBootProtect     = 74, /* 74 - 1 bit */
    NEXUS_OtpCmdMsp_eErrorMipsOutResetDisable = 75, /* 75 - 1 bit */
    NEXUS_OtpCmdMsp_eGenericPinBoundaryScanProtect = 76, /* 76 - 1 bit */

    NEXUS_OtpCmdMsp_eMichEnable              = 78, /* 78 - 1 bit */
    NEXUS_OtpCmdMsp_eVideoVerifyEnable       = 79, /* 79 - 1 bit */
    NEXUS_OtpCmdMsp_eAudioVerifyEnable       = 80, /* 80 - 1 bit */
 
    NEXUS_OtpCmdMsp_eSpdifI2SDisable         = 86, /* 86 - 1 bit */
    NEXUS_OtpCmdMsp_eOtpProgRights           = 87, /* 87 - 2 bits */
    NEXUS_OtpCmdMsp_eConformanceDisable      = 88, /* 88 - 1 bit */
    NEXUS_OtpCmdMsp_eDesParityCheckEn        = 89, /* 89 - 1 bit */
    NEXUS_OtpCmdMsp_eOtpKeyDeobfuscationEn   = 90, /* 90 - 1 bit */
    NEXUS_OtpCmdMsp_eOtpObfuscatedKeyHashReadDisable = 91, /* 91 - 1 bit */
  
    NEXUS_OtpCmdMsp_eForce_3DES_Key_Ladder   = 93, /* 93 - 1 bit */
    NEXUS_OtpCmdMsp_eStartHash               = 94,  /* 94 - 1 bit */
    NEXUS_OtpCmdMsp_eStopHash                = 95,  /* 95 - 1 bit */

    NEXUS_OtpCmdMsp_eEnableASKM              = 98,  /* 98 - 1 bit */
    NEXUS_OtpCmdMsp_eOTPSecurePSEQ           = 99,  /* 99 - 1 bit */
    NEXUS_OtpCmdMsp_eOTPChipPwrMgmtDisable   = 100, /* 100 - 1 bit */
    NEXUS_OtpCmdMsp_ePCIeHostProtectCR       = 101, /* 101 - 1 bit */
    NEXUS_OtpCmdMsp_ePCIeClientProtectCR     = 102, /* 102 - 1 bit */

    NEXUS_OtpCmdMsp_eMOCAScbDisable          = 105, /* 105 - 1 bit */
    NEXUS_OtpCmdMsp_eCpKeyLadderDisable      = 106, /* 106 - 1 bit */
    NEXUS_OtpCmdMsp_ePCIUnsecuredModeDisable = 107, /* 107 - 1 bit */
    NEXUS_OtpCmdMsp_eAskmStbOwnerId          = 108, /* 108 - 16 bit */
    NEXUS_OtpCmdMsp_eBseckFwEpoch            = 109, /* 109 - 8 bits */

    NEXUS_OtpCmdMsp_eDataSectionReadProtectBits = 111, /* 111 - 4 bits */
    NEXUS_OtpCmdMsp_eOtpCtrlBit31_eDisableCustKeyToCaKeyLadder = 112, /* 112 - 1 bit */    
    NEXUS_OtpCmdMsp_eFlashType               = 113, /* 3 bits */
    NEXUS_OtpCmdMsp_eMipsBootFromFlash       = 114, /* 1 bit */
    NEXUS_OtpCmdMsp_eUserReg                 = 115, /* 32 bits */
    NEXUS_OtpCmdMsp_eMarketIdWildCardDisable = 116, /* 1 bit */
    NEXUS_OtpCmdMsp_eCpWidevineAesCbcCtsEnable = 117, /* 1 bit */    

    NEXUS_OtpCmdMsp_eBcmBootSequence2        = 119, /* 1 bit */
    NEXUS_OtpCmdMsp_eOtpCtrlBit30_eDisableCustKeyToCpKeyLadder = 120, /* 1 bit */    
    NEXUS_OtpCmdMsp_eAesKeySelectForBootCodeDecrypt = 121, /* 4 bits */
    NEXUS_OtpCmdMsp_eDHRootKeyDisable        = 122, /* 1 bit */
    NEXUS_OtpCmdMsp_eDHGyMustSign            = 123, /* 1 bit */
    NEXUS_OtpCmdMsp_eCaRsaRootDisable        = 124, /* 1 bit */
    NEXUS_OtpCmdMsp_eCpRsaRootDisable        = 125, /* 1 bit */
    NEXUS_OtpCmdMsp_eEncKey3MustSign         = 126, /* 1 bit */
    NEXUS_OtpCmdMsp_eKey3ExportDisable       = 127, /* 1 bit */    
    NEXUS_OtpCmdMsp_eFastCyclingDisable      = 128, /* 1 bit */
    NEXUS_OtpCmdMsp_eNandConfigViaOtp        = 129, /* 1 bit */
    NEXUS_OtpCmdMsp_eNandFlashDeviceSize     = 130, /* 1 bit */
    NEXUS_OtpCmdMsp_eNandFlashBlockSize      = 131, /* 1 bit */
    NEXUS_OtpCmdMsp_eNandFlashPageSize       = 132, /* 1 bit */
	NEXUS_OtpCmdMsp_eDataSectionSecureRsa2RenewRsaKey = 140,
    NEXUS_OtpCmdMsp_eMax                     = 141
} NEXUS_OtpCmdMsp;

/*****************************************************************************
Summary:
This enum defines OTP programming keys.
There are 3 keys in OTP. The first and second have start/stop, the third has lock/unlock.

Description:
This enumeration defines all the supported programming modes..
*****************************************************************************/
typedef enum NEXUS_OtpHalProgKey
{
    NEXUS_OtpHalProgKey_eA,
    NEXUS_OtpHalProgKey_eB,
    NEXUS_OtpHalProgKey_eC,
    NEXUS_OtpHalProgKey_eCAddEccCrc,
    NEXUS_OtpHalProgKey_eHash,
    NEXUS_OtpHalProgKey_eMax
} NEXUS_OtpHalProgKey;



#define NEXUS_OTP_MC0_DATA_LEN              4
#define NEXUS_OTP_MC0_RESERVED_DATA_LEN     4
#define NEXUS_OTP_KEY_ID_DATA_LEN           16
#define NEXUS_OTP_KEY_DATA_LEN              32
#define NEXUS_OTP_CRC_LEN               8



/*****************************************************************************
Summary:
Select bits to be read
*****************************************************************************/
typedef enum NEXUS_OtpCmdReadRegister
{
    NEXUS_OtpCmdReadRegister_eMc0OperationModeSelect = 0,


    NEXUS_OtpCmdReadRegister_eIdAWord                = 3,
    NEXUS_OtpCmdReadRegister_eIdBWord                = 4,
    NEXUS_OtpCmdReadRegister_eIdCWord                = 5,

    NEXUS_OtpCmdReadRegister_eBseckHashRead          = 17,
    NEXUS_OtpCmdReadRegister_eEnableASKM             = 18,
        
    NEXUS_OtpCmdReadRegister_eConcurrentCustomerMode = 19,

    NEXUS_OtpCmdReadRegister_eMax
} NEXUS_OtpCmdReadRegister;


typedef enum NEXUS_OtpHalOperationMode
{
    NEXUS_OtpHalOperationMode_eGeneric               = 0x00,
    NEXUS_OtpHalOperationMode_eMs                    = 0x0c,

    NEXUS_OtpHalOperationMode_eMax
    
} NEXUS_OtpHalOperationMode;



#define NEXUS_OTP_KEY_ID_LEN            8
#define NEXUS_MSP_DATA_LEN              4
#define NEXUS_MSP_SIGNATURE_DATA_LEN    20          /* 16 for legacy, 20 for ASKM */
#define NEXUS_MSP_OUTPUT_DATA_LEN       4
#define NEXUS_OTP_DATASECTION_LEN       (32)        /* in byte now, may change to word32*/



/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ProgramOtpIO {
    NEXUS_OtpMspCmdProgMode    progMode;
    NEXUS_OtpHalProgKey        progKeyEnum;
    unsigned char              mc0Data[NEXUS_OTP_MC0_DATA_LEN]; /* include 10-bit ECC */
    unsigned char              keyId[NEXUS_OTP_KEY_ID_DATA_LEN]; /* include 10-bit ECC */
    unsigned char              keyData[NEXUS_OTP_KEY_DATA_LEN]; /* include 10-bit ECC */
    unsigned char              crc[NEXUS_OTP_CRC_LEN]; /* include 10-bit ECC */
} NEXUS_ProgramOtpIO;



/*****************************************************************************
Summary:
This function is reserved for Broadcom internal usage only.
 The programming tool is strictly controlled.

Description:
See Also:
NEXUS_Security_ProgramMSP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ProgramOTP(
    const NEXUS_ProgramOtpIO    *pProgOtpIO       /* Pointer to structure containing OTP programming information */
    );

/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadOtpIO {
    unsigned char       otpKeyIdBuf[NEXUS_OTP_KEY_ID_LEN];      /* Buffer to hold OTP Key ID for the current OTP Key ID read request */
    unsigned int        otpKeyIdSize;                           /* Actual size of Otp read buffer */
} NEXUS_ReadOtpIO;

/*****************************************************************************
Summary:
This function returns one OTP key identifiers or one MC0 OTP value.

Description:
This function shall be used to read either OTP key identifier or other OTP field value.
Depends on the access control matrix, only certain OTP fields can be read in a specific
customer mode.  Note that this function can only read back one OTP value at a time.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadMSP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ReadOTP(
    NEXUS_OtpCmdReadRegister    readOtpEnum,
    NEXUS_ReadOtpIO            *pReadOtpIO         /* [out] structure holding read OTP buffer and size */
    );


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to program, with what data,  using what mask and proper mode,  and holds the returned status
of a programming request

See Also:
NEXUS_OtpMsp_ProgramMSP
**************************************************************************************************/
typedef struct NEXUS_ProgramMspIO {
    NEXUS_OtpMspCmdProgMode    progMode;    /* This field should contain the value of NEXUS_OtpMspCmdProgMode
           for the bits to be programmed. This is a sanity check on the command.
           The value NEXUS_OtpMspCmdProgMode_Enum  specifies that command enum mode
           programming is used.
        */

    NEXUS_OtpCmdMsp            progMspEnum; /* specifies which MSP bits to program. The values this field can take are specified by the
          typedef enum NEXUS_OtpCmdMsp in the share C header file.  Each chip will have different enums
          and customers will only have access to the files for the chips that they use.
        */

    unsigned char               dataBitLen; /* number of bits of the MSP enum to program, from 1 to 32 */

    unsigned char               dataBitMask [NEXUS_MSP_DATA_LEN];   /* 0x0000_0001 to 0xFFFF_FFFF
          A value 1 in a bit position specifies that the data value at that bit position is to be programmed.
          BSP will not program bits that have a zero bit in the mask.
          For example 0x0F means to program the 4 LSBits of an enum.
          For example 0x8F means to program the bit 7 and 4 LSBits of an enum.
        */

    /* The following 2 fields are for signed cmd in ASKM mode */
    NEXUS_SecurityVirtualKeyladderID    vkl;
    NEXUS_SecurityKeySource             keyLayer;


    unsigned char               mspData[NEXUS_MSP_DATA_LEN];    /* the value that needs to be programmed */

    /* HMAC-SHA1 for signed cmd, in legacy or ASKM mode */
    unsigned char               signature[NEXUS_MSP_SIGNATURE_DATA_LEN]; /* only when checkSignature is true,  must be set correctly, big endian */

} NEXUS_ProgramMspIO;



/*****************************************************************************
Summary:
This function allows the programming of each of the field programmable
OTP (MSP) bits.

Description:
This function allows the programming of each of the field programmable
OTP (MSP) bits.  Based on the Access Control Matrix (ACL), programming of the
bit is allowed or disallowed.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ProgramOTP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ProgramMSP(
    const NEXUS_ProgramMspIO    *pProgMspIO
    );


#define NEXUS_MSP_KEY3_DATA_LEN        16
#define NEXUS_MSP_KEY4_DATA_LEN        16


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to read and its required key/data if command authentication is needed by BSP,
and holds the returned value of the MSP field

See Also:
NEXUS_OtpMsp_ReadMSP
**************************************************************************************************/
typedef struct NEXUS_ReadMspParms
{
    NEXUS_OtpCmdMsp             readMspEnum;     /* which MSP to read */

    bool                        checkSignature; /*  ask BSP do the command authentication for this MSP read, true/yes, false/no
           only if readOtpEnum == NEXUS_OtpCmdMsp_eOtpReserved32, we require to check signature */
           
    /* The following 6 members are for legacy BSP chipsets except 7405B0/C0 */
    unsigned char               custKeyHigh;   /* only when checkSignature is true,  must be set correctly*/
    unsigned char               custKeyLow;   /* only when checkSignature is true,  must be set correctly*/
    unsigned char               keyVarHigh;   /* only when checkSignature is true,  must be set correctly*/
    unsigned char               keyVarLow;    /* only when checkSignature is true,  must be set correctly*/
    unsigned char               procInForKey3[NEXUS_MSP_KEY3_DATA_LEN]; /* only when checkSignature is true,  must be set correctly, big endian */
    unsigned char               procInForKey4[NEXUS_MSP_KEY4_DATA_LEN];  /* only when checkSignature is true,  must be set correctly, big endian */


    unsigned char               signature[NEXUS_MSP_SIGNATURE_DATA_LEN]; /* only when checkSignature is true,  must be set correctly, big endian */
} NEXUS_ReadMspParms;


/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadMspIO {
    unsigned char       mspDataBuf[NEXUS_MSP_OUTPUT_DATA_LEN];      /* Buffer to hold MSP data for the current MSP read request   */
    unsigned char       lockMspDataBuf[NEXUS_MSP_OUTPUT_DATA_LEN];  /* Buffer to hold lock MSP data for the current read request       */
                                                                    /* This is used to tell if value '0' is programmed or unprogrammed */
    unsigned int        mspDataSize;                                /* Actual size of MSP output buffer */
} NEXUS_ReadMspIO;

/*****************************************************************************
Summary:
This function returns one MSP value.

Description:

This function shall be used to read MSP field value. Depends on the access control matrix, only
certain MSP fields can be read in a specific customer mode.  Note that this function can only
read back one MSP value at a time.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadOTP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ReadMSP(
    const NEXUS_ReadMspParms    *pReadMspParms,     /* structure holding input parameters */
    NEXUS_ReadMspIO             *pReadMspIO         /* [out] structure holding read MSP buufer and size */
    );

/*****************************************************************************
Summary:
*****************************************************************************/
typedef enum NEXUS_OtpDataSection
{
    NEXUS_OtpDataSection_e0     = 0x0,
    NEXUS_OtpDataSection_e1     = 0x1,
    NEXUS_OtpDataSection_e2     = 0x2,
    NEXUS_OtpDataSection_e3     = 0x3,
    NEXUS_OtpDataSection_e4     = 0x4,
    NEXUS_OtpDataSection_e5     = 0x5,
    NEXUS_OtpDataSection_e6     = 0x6,
    NEXUS_OtpDataSection_e7     = 0x7,
    NEXUS_OtpDataSection_eMax   = 0x8

} NEXUS_OtpDataSection;

/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadDataSectIO {
    unsigned char       dataSectBuf[NEXUS_OTP_DATASECTION_LEN]; /* Buffer to hold DataSection data for the current DataSect read request */
    unsigned int        dataSectSize;                           /* Actual size of data section read */
} NEXUS_ReadDataSectIO;


/*****************************************************************************
Summary:
This function returns one 32-byte data section value.

Description:
There are total of 8 32-byte data sections. This function shall be used to read each 32-byte data section.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ProgramDataSect
*****************************************************************************/
NEXUS_Error NEXUS_Security_ReadDataSect(
    NEXUS_OtpDataSection        readDsEnum,       /* NEXUS_OtpDataSection enum of which data section to be read */
    NEXUS_ReadDataSectIO       *pReadDataSectIO    /* [out] structure holding read datasect buffer and size */
    );



#define NEXUS_OTP_DATASECTION_CRC_LEN               4
#define NEXUS_OTP_DATASECTIONPROG_MODE              0x00010112


/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP data section to program with what data using a proper mode, and holds
the returned status

See Also:
NEXUS_Security_ProgramDataSect
**************************************************************************************************/
typedef struct NEXUS_ProgramDataSectIO {
    NEXUS_OtpDataSection       progDsEnum; /* select which OTP data section to program, between BPI_Otp_DataSection_e0 ~
            BPI_Otp_DataSection_e7 */

    unsigned char               dataSectData[NEXUS_OTP_DATASECTION_LEN]; /* provide the actual 32-byte data to be programmed into the specified OTP data section*/

    unsigned char               crc[NEXUS_OTP_DATASECTION_CRC_LEN]; /* provide the crc of data section */

    uint32_t                    mode; /* a kind of program magic number, must be NEXUS_OTP_DATASECTIONPROG_MODE (0x00010112).
            if it is not this value the command will be rejected by  a sanity check at BSP */

    uint32_t                    padding; /* padding field  */
} NEXUS_ProgramDataSectIO;


/*****************************************************************************
Summary:
This function is used to program one 32-byte data section value.

Description:
There are total of 8 32-byte data sections. This function shall be used to write one 32-byte data
section.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadDataSect
*****************************************************************************/
NEXUS_Error NEXUS_Security_ProgramDataSect(
    const NEXUS_ProgramDataSectIO   *pProgDataSectIO
    );

#ifdef __cplusplus
}
#endif

#endif
