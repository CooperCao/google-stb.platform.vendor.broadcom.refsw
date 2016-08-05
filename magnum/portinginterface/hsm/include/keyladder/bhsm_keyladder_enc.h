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


#ifndef BHSM_KEYLADDER_ENC_H__
#define BHSM_KEYLADDER_ENC_H__

#include "bhsm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_GEN_ROUTE_KEY_DATA_LEN            (((BCMD_GenKey_InCmd_eKeySize/4)*4) - ((BCMD_GenKey_InCmd_eProcIn/4)*4))
#define BHSM_KEYLADDER_CUSTSUBMODE_SHIFT    0x08
#define    BHSM_KL_ACTCODE_LEN                    16

#define BHSM_VKL_CUSTSUBMODE_MASK           0x000000FF
#define BHSM_VKL_CLIENT_MASK                0x0000FF00
#define BHSM_VKL_CLIENT_SHIFT               8

#define BHSM_VISTA_PROCIN_LEN               16



typedef enum BHSM_PKSMInitSizeType_e
{
    BHSM_PKSMInitSizeType_eNone,
    BHSM_PKSMInitSizeType_e64Bits,
    BHSM_PKSMInitSizeType_e128Bits,
    BHSM_PKSMInitSizeType_e192Bits,

    BHSM_PKSMInitSizeType_eMax

} BHSM_PKSMInitSizeType_e;


typedef enum BHSM_OwnerIDSelect_e
{
    BHSM_OwnerIDSelect_eMSP0        = 0,
    BHSM_OwnerIDSelect_eMSP1        = 1,
    BHSM_OwnerIDSelect_eReserved    = 2,  /* error will be returned */
    BHSM_OwnerIDSelect_eUse1        = 3,

    BHSM_OwnerIDSelect_eMax
}BHSM_OwnerIDSelect_e;


typedef enum BHSM_KeyTweak_e
{
    BHSM_KeyTweak_eNoTweak   = 0,
    BHSM_KeyTweak_ePKSM      = 1,
    BHSM_KeyTweak_eTransform = 2,
    BHSM_KeyTweak_eDSK       = 3,
    BHSM_KeyTweak_eDupleConnect = 4,
    BHSM_KeyTweak_eMax
} BHSM_KeyTweak_e;

typedef enum BHSM_KeyLadderOp_e
{
    BHSM_KeyLadderOp_eDecrypt,
    BHSM_KeyLadderOp_eEncrypt,

    BHSM_KeyLadderOp_eMax
} BHSM_KeyLadderOp_e;

/**************************************************************************************************
Summary:

Description:
this structure defines all the required setting and key seed to generate an intermediate or final key,
and route it into a pre-allocated key slot if required.

See Also:
BHSM_GenerateRouteKey
**************************************************************************************************/
typedef struct BHSM_GenerateRouteKeyIO {

    /* In: The request is from host or from SAGE  -- Zeus 3.0 and later with SCPU support */
    BHSM_ClientType_e               client;

    /* In: Key Ladder selection - FW, HW, or Poway */
    /*      Of type BCMD_KeyLadderSelection_e for Zeus 2.0 and above */
    unsigned int                    keyLadderSelect;

    /* In: ASKM 3Des Key ladder root key swap enable */
    bool                            bASKM3DesKLRootKeySwapEnable;

    /* In: Sub Command ID - to retrieve VKL association table */
    /*      BCMD_KeyGenSubCmdID_e for Zeus 1.0; BCMD_VKLAssociationQueryFlag_e for others */
    unsigned int                    subCmdID;

    /* In: 0/DES, 1/3DESABA*/
    BCMD_KeyLadderType_e            keyLadderType;

     /* In: needed and meaningful only for key3 generation, to select a key source,
         values in BCMD_RootKeySrc_e         */
    BCMD_RootKeySrc_e                rootKeySrc;

    /* In: Customer Sub-Mode Select - VKLs are allocated based on these */
    BCMD_CustomerSubMode_e            customerSubMode;

    /* In: ASKM Mode Enabled - 1 : Disabled - 0 */
    bool                            bASKMModeEnabled;

    /* In: Key Generation Mode - Generate or do not generate key */
    /*      of type BCMD_KeyGenFlag_e for Zeus 2.0 and above */
    unsigned int                    keyGenMode;

    /* In: needed and meaningful only for key3 generation, valid only when Swizzle1 is used */
    unsigned char                    ucSwizzle1Index;

    /* In: needed and meaningful only for key3 generation, to select a swizzle type as
    defined in BCMD_SwizzleType_e*/
    BCMD_SwizzleType_e                swizzleType;

    /* In: needed and meaningful only for key3 generation,0 - encryption, 1 decryption */
    bool                            bUseCustKeyLowDecrypt;

    /* In: needed and meaningful only for key3 generation, the low bypte of custom key selection pair */
    unsigned char                    ucCustKeyLow;

    /* In: needed and meaningful only for key3 generation,0 - encryption, 1 decryption */
    bool                            bUseCustKeyHighDecrypt;

    /* In: needed and meaningful only for key3 generation,the high bypte of custom key selection pair */
    unsigned char                    ucCustKeyHigh;

    /* In: needed and meaningful only for key3 generation,the low bypte of key variable pair */
    unsigned char                    ucKeyVarLow;

    /* In: needed and meaningful only for key3 generation,the high bypte of key variable pair */
    unsigned char                    ucKeyVarHigh;

    /* In: Swizzle0a CusKey option  -- For SAGE app */
    bool                            cusKeySwizzle0aEnable;

    /* In: CusKey Swizzle0a variant  -- For cusKey with swizzle0a                  */
    /*                               --   Zeus 3.0 type : BCMD_Swizzle0aType_e     */
    /*                               --   Zeus 4.1+ type: BCMD_OwnerIDSelectType_e */
    uint32_t                        cusKeySwizzle0aVariant;

    /* In: CusKey Swizzle0a version  -- For cusKey with swizzle0a */
    uint32_t                        cusKeySwizzle0aVersion;

    /* In: Global key owner ID Select -- For ASKM global key (available on Zeus 4.1 and up) */
    /*                                --   type BCMD_OwnerIDSelectType_e                    */
    BHSM_OwnerIDSelect_e            globalKeyOwnerId;

    /* In: Global Key Index - used by ASKM harware for Key2 generation - 0x0 - 0x3F  */
    unsigned char                   AskmGlobalKeyIndex;

    /* In: Key2 Generation Version: BSP_ASKM_KDF_VERSION for ASKM Global key         */
    int                             globalKeyVersion;

    struct{
        bool                        configurationEnable;    /* In: If true,  the following askm paramters are active.
                                                                   if false, the GRK function will look to the associated keyslot for these parameters. */
        BCMD_ASKM_MaskKeySel_e      maskKeySelect;          /* In: ASKM mask selector                                       */
        BCMD_ModuleID_e             moduleId;               /* In: Module ID                                                */
        BCMD_STBOwnerID_e           stbOwnerId;             /* In: STB Owner ID                                             */
        uint32_t                    caVendorId;             /* In: CA Vendor ID (16 bit)                                    */
       #if BHSM_BUILD_HSM_FOR_SAGE /* if  we're on SAGE */
        uint8_t                     caVendorIdExtension;    /* In: CA Vendor ID Extension. Only interpreted from a SAGE context. */
       #endif
    }askm;

    bool                            sageAskmConfigurationEnable;  /* DEPRECATED, use askm.configurationEnable */
    BCMD_ASKM_MaskKeySel_e          sageMaskKeySelect;            /* DEPRECATED, use askm.maskKeySelect       */
    BCMD_ModuleID_e                 sageModuleID;                 /* DEPRECATED, use askm.moduleId            */
    BCMD_STBOwnerID_e               sageSTBOwnerID;               /* DEPRECATED, use askm.stbOwnerId          */
    uint32_t                        sageCAVendorID;               /* DEPRECATED, use askm.caVendorId          */

    /* In: Virtual Key Ladder ID */
    BCMD_VKLID_e                    virtualKeyLadderID;

    /* In: Key Layer used for key ladder key generation key3 - key5 */
    BCMD_KeyRamBuf_e                keyLayer;

    /* In: K3 Operation information */
    BCMD_Key3Op_e                    key3Op;

    /* In: keyTweak   -- Zeus 4.1+  - BCMD_KeyTweak_e */
    /* Also used on some special Zeus */
    BHSM_KeyTweak_e                 keyTweak;

    /* In: if true, apply key contribution vector */
    bool                            applyKeyContribution;

    /* In: 0 - decryption, 1- encryption */
    bool                            bIsKeyLadder3DESEncrypt;

     /* In:  how many bytes from the begging of "aucKeyData" is actually used as key seed*/
    unsigned char                    ucKeyDataLen;

     /* In: big endian byte array, to input from the 1st byte, padding at bottom*/
    unsigned char                    aucKeyData[BHSM_GEN_ROUTE_KEY_DATA_LEN];

     /* In:  the size of generated key is 64/128/192bits as defined in BCMD_KeySize_e */
    BCMD_KeySize_e                    keySize;

     /* In: route this generated into the key slot, true for final key, false for intermediate key*/
    bool                            bIsRouteKeyRequired;

    bool                            bSwapAesKey;

     /* In: which kind of key slot destination, CA/RMX/M2M/HDMI/etc, see BCMD_KeyDestBlockType_e*/
    BCMD_KeyDestBlockType_e            keyDestBlckType;

    /* In: which entry type of key destination, odd/even key entry or IV entry or else, see
    BCMD_KeyDestEntryType_e
     */
    BCMD_KeyDestEntryType_e            keyDestEntryType;

    /* In: IV type for this particular key element */
    BCMD_KeyDestIVType_e            keyDestIVType;

     /* In:  value of the allocated key slot type  */
    BCMD_XptSecKeySlot_e            caKeySlotType;

     /* In:  value of the allocated key slot number or, for HDMI, HDMI Key Serializer address*/
    unsigned int                    unKeySlotNum;

     /* In:  which mode to use the key, BCMD_KeyMode_eRegular normally, see BCMD_KeyMode_e */
    BCMD_KeyMode_e                    keyMode;

    /* In: The equivalent value of SC bits 0x01 for R-pipe (either odd (0x11) or clear (0x00))   */
    unsigned char                   RpipeSC01Val;

    /* In: The equivalent value of SC bits 0x01 for G-pipe (either odd (0x11) or clear (0x00))   */
    unsigned char                   GpipeSC01Val;

    /* In:    SC01 mode word mapping - of type BCMD_SC01ModeWordMapping_e for Zeus 3.0 */
    BHSM_SC01ModeWordMapping_e      SC01ModeMapping;

    /* In: Hardware (HW) key ladder destination algorithm */
    BCMD_XptM2MSecCryptoAlg_e        hwklDestAlg;

    /* In: HW Key Ladder length */
    /*      of Type BCMD_HwKeyLadderLength_e */
    unsigned int                    hwklLength;

    /* In: Enable HWKL Vista Key generation */
    bool                            bHWKLVistaKeyGenEnable;

    /* In: 128-bit Activation code - BE array */
    unsigned char                    activationCode[BHSM_KL_ACTCODE_LEN];

    /* In: external IV pointer */
    uint32_t                        externalIVPtr;

    /* In: whether to reset PKSM */
    bool                            bResetPKSM;

    /* In: PKSM Initial size type */
    BHSM_PKSMInitSizeType_e         PKSMInitSize;

    /* In: Cycle   - from 0 to 1023  */
    uint32_t                        PKSMcycle;

    /* In: Identify the source Keyladder of a Duple configuration */
    BCMD_VKLID_e                    sourceDupleKeyLadderId;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;

} BHSM_GenerateRouteKeyIO_t;



typedef struct BHSM_GenerateGlobalKey_t
{
    BCMD_KeyLadderType_e     keyLadderType;                          /* Aes, des, etc.                            */
    BHSM_OwnerIDSelect_e     globalKeyOwnerId;
    unsigned char            globalKeyIndex;                         /* Global Key Index  [0x0...0x3F ]   */
    BCMD_VKLID_e             virtualKeyLadderId;                     /* Virtual Key Ladder ID                 */
    unsigned char            keyData[BHSM_GEN_ROUTE_KEY_DATA_LEN];   /* Proc in                                       */
    BCMD_KeySize_e           keySize;
    BCMD_KeyLadderOp_e       operation;                              /* Encrypt or decrypt.                     */
    bool                     routeKeyRequired;

    BCMD_STBOwnerID_e        stbOwnerId;
    uint32_t                 caVendorId;
    BCMD_ASKM_MaskKeySel_e   maskKeySelect;
    BCMD_ModuleID_e          moduleId;
} BHSM_GenerateGlobalKey_t;




/*****************************************************************************
Summary:

This function can be used to decrypt an intermediate encrypted key and store the intermediate key
as key3, key4 or key5.  It can also route this intermediate key to CA, M2M, HDMI, IV
and REMUX modules.

Description:

Secure key management is definitely one of the most important BSP security features.
BSP can decrypt keys and route the decrypted keys to various destination securely.
These keys can be generated through BSP key ladder, where the top root key
can only be accessed by hardware. Using this top root key, BSP can generate different
intermediate keys, such as key3, key4 or key5.

This function can be used to decrypt the intermediate key. If bIsRouteKeyRequired is true,
this function will also route the decrypted key to CA, M2M, HDMI, IV or REMUX modules.
Since MIPS cannot access the intermediate keys,  this method is therefore much more secure
than routing control word into those modules.

At any given time, BSP can support up to 3 set of key ladders. Each key ladder can store
a set of key3, key4 and key5.  However, depend on the access control matrix and
OTP progamming, intermediate key in certain key ladder may be blocked from routing to
certain destination.  Refer to ACL Document for the detail security policies.

Key slot of certain destination, for example CA, may contain a few keys, such as odd, even, and IV
key.  This function can be used to decrypt only one key at a time. If the key is decrypted
and routed to the same slot entry more than once, the last call may overwrite any previous result.

The key will be kept valid in the key slot until either the key slot is invalidated by
BHSM_InvalidateKey function or the chip is reset. The loaded algorithm and control bits will be
kept in the key slot until either the key slot is reset by BHSM_ResetKeySlotCtrlBits or the
chip is reset.

The key order shall be in big-endianness. The length of decrypted key could be 64/128/192 bits.
If the actual key bytes to load is shorter than 192 bits, the encrypted will be loaded at the most
significant byte of the aucKeyData variable. For example 64-bit DES key is occupying first 8
bytes of aucKeyData variable.

Refer to Load key documentation on how to load 3DES-ABA and 3DES-ABC keys.

Note that some systems may require to load the control word for CA and CP. BHSM_LoadRouteUserKey
can be used for this purpose.


Calling Context:

This function shall be called after BHSM_ConfigAlgorithm configure the algorithm.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.



Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_generateRouteKeyIO  - BHSM_GenerateRouteKeyIO_t.

Output:
inoutp_generateRouteKeyIO  - BHSM_GenerateRouteKeyIO_t.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_LoadRouteUserKey
BHSM_FreeCAKeySlot
BHSM_AllocateCAKeySlot
BHSM_ConfigAlgorithm

*****************************************************************************/
BERR_Code   BHSM_GenerateRouteKey (
        BHSM_Handle                hHsm,
        BHSM_GenerateRouteKeyIO_t *pGrk
);


BERR_Code  BHSM_GenerateGlobalKey( BHSM_Handle hHsm, BHSM_GenerateGlobalKey_t *pGlobalKey );


/**************************************************************************************************
Summary:
This function retrieves the VKL-CustSubMode association table.

Description:
This function retrieves the VKL-CustSubMode association table.


Calling Context:
This function is called by application or higher layer SW like Nexus.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.

Output:


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code BHSM_LoadVklOwnershipFromBsp ( BHSM_Handle hHsm );


/**************************************************************************************************
Summary:

Description:
this structure defines all the required settings for requesting an available VKL to be associated with the
specified custSubMode for a given requestor (host or SAGE).

See Also:
BHSM_LoadVklOwnershipFromBsp

**************************************************************************************************/
typedef struct BHSM_AllocateVKLIO
{


    /* In: Customer Sub-Mode Select - VKLs are allocated based on these */
    BCMD_CustomerSubMode_e            customerSubMode;  /* Pre Zeus 3. */


    /* In: Who the requestor is */
    BHSM_ClientType_e               client;             /* Zeus 3 and above  */

    /* In: new VKL needed, even if there's existing vkl-specified customerSubMode association  -- For Zeus 2.x and prior platforms only */
    bool                            bNewVKLCustSubModeAssoc;

    /* Out: Virtual Key Ladder ID */
    BCMD_VKLID_e                    allocVKL;

    /* Out: DEPRECATED: */
    uint32_t                        unStatus;

} BHSM_AllocateVKLIO_t;



/**************************************************************************************************
Summary:

Description:


Calling Context:
This function is called by application or higher layer SW like Nexus.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle                   - BHSM_Handle, Host Secure module handle.
inoutp_AllocateVKLIO  - BHSM_AllocateVKLIO_t.

Output:
inoutp_AllocateVKLIO  - BHSM_AllocateVKLIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
BERR_Code BHSM_AllocateVKL(
    BHSM_Handle            in_handle,
    BHSM_AllocateVKLIO_t   *inoutp_AllocateVKLIO
);


/**************************************************************************************************
Summary:

Description:


Calling Context:
This function is called by application or higher layer SW like Nexus.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
in_handle                  - BHSM_Handle, Host Secure module handle.
inoutp_vklRequestIO  - BHSM_vklRequestIO_t.

Output:
inoutp_vklRequestIO  - BHSM_vklRequestIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
N/A
**************************************************************************************************/
void BHSM_FreeVKL(
    BHSM_Handle             in_handle,
    BCMD_VKLID_e            vkl
);

/**************************************************************************************************
Summary:

Description:
This structure defines all the data members to invalidate a VKL.

The configurations of the expected behaviors are listed in the following table.
'ignored' means the field in BHSM_InvalidateVkl_t parameter passed to function BHSM_InvalidateVKL() is ignored, and it does not need to be specified by the client.
'needed' means the field in BHSM_InvalidateVkl_t  need to be specified with a valid value by the client.

bInvalidateVkl |   bInvalidateAllKeyLayers | keyLayer |  Expected behavior
===============|===========================|==========|===================
true           |  ignored                  |ignored   |  BFW will invalidate all the key layers and VKL ownership of the VKL identified by virtualKeyLadderID.
false          |   true                    |ignored   |  BFW will invalidate all the key layers of the VKL identified by virtualKeyLadderID.
false          |   false                   |needed    |  This is the case to invalidate a single key layer for the VKL identified by virtualKeyLadderID.

See Also:
BHSM_InvalidateKey and BHSM_InvalidateVKL
**************************************************************************************************/
typedef struct BHSM_InvalidateVkl {

    /* In: key from which virtual key ladder to invalidate */
    BCMD_VKLID_e                   virtualKeyLadderID;

    /* In: If bInvalidateVkl is set to true,  bInvalidateAllKeyLayers and keyLayer are ignored, BFW will invalidate all the key layers and VKL ownership, the Vkl is completely freed. */
    bool                           bInvalidateVkl;

    /* In: If set to true, keyLayer is ignored, BFW will invalidate all the key layers, while the ownership is kept. */
    bool                           bInvalidateAllKeyLayers;

    /* In: key from which key layer of the above virtual key ladder to invalidate */
    BCMD_KeyRamBuf_e               keyLayer;

} BHSM_InvalidateVkl_t;

/*****************************************************************************
Summary:

This function is used to invalidate the intermediate key in BSP Key RAM.
The functionalities have been extended from BHSM_InvalidateKey's invalidating a VKL's
one keyLayer to freeing up the VKL completely, and/or invalidating its all key layers.

Description:

This function can be used to invalidate the intermedaite key in BSP Key RAM.
The invalidated intermedaite key in BSP Key RAM cannot be used to generate the
next intermediate key.


Calling Context:

After key loading/generation successfully, if caller determines the key is no longer valid,
this function can be used to invalidate the key.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
BHSM_InvalidateVkl_t  - BHSM_InvalidateVkl_t


Output:


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_InvalidateKey

*****************************************************************************/
BERR_Code BHSM_InvalidateVKL( BHSM_Handle hHsm, BHSM_InvalidateVkl_t * pConfig );


typedef struct BHSM_EncryptedHdcpKeyStruct
{
    uint8_t  Alg;
    uint8_t  cusKeyVarL;
    uint8_t  cusKeyVarH;
    uint8_t  cusKeySel;
    uint32_t CaDataLo;
    uint32_t CaDataHi;
    uint32_t TCaDataLo;
    uint32_t TCaDataHi;
    uint32_t HdcpKeyLo;
    uint32_t HdcpKeyHi;
} BHSM_EncryptedHdcpKeyStruct;

BERR_Code BHSM_FastLoadEncryptedHdcpKey (
    BHSM_Handle                        in_handle,
    uint32_t                        index,
    BHSM_EncryptedHdcpKeyStruct     *keyStruct
);


#define BHSM_MAX_PROC_IN_DATA_LEN        16
#define BHSM_MAX_PROC_OUT_DATA_LEN    32

/**************************************************************************************************
Summary:

Description:
this structure defines all data members to get an unwrapped key or decrypted data

See Also:
BHSM_ProcOutCmd
**************************************************************************************************/
typedef struct BHSM_ProcOutCmdIO {

    /* In: key from which virtual key ladder  is used to decrypt the input data or key */
    BCMD_VKLID_e                virtualKeyLadderID;

    /* In: how many bytes of the provided data in "aucProcIn" are used to do the decryption   */
    uint32_t                    unProcInLen;

    /* In: the input encrypted data or key to be decrypted, big endian, input from the top, padding
     at the bottom */
    unsigned char                aucProcIn[BHSM_MAX_PROC_IN_DATA_LEN];

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;

    /* Out: the length in bytes of the returned decrypted key or data in "aucProcOut"  */
    uint32_t                    unProcOutLen;

    /* Out: the actual returned decrypted key or data, big endian, filled up from the 1st byte */
    unsigned char                aucProcOut[BHSM_MAX_PROC_OUT_DATA_LEN];

} BHSM_ProcOutCmdIO_t;


/*****************************************************************************
Summary:

This function returns a decrypted output which can be used to indirectly verify the result of
each intermediate key in the specific key ladder.


Description:

BSP does not allow key3, key4 and key5 to be returned to the host since they can
be used in CA or CP destinations. Via this function, BSP allows the host to use key5 to
decrypt an input data with 128-bit AES-ECB algorithm and return the result back
to the host.

However, depending on the access control matrix and OTP progamming, this operation
may be blocked for security reasons.  Refer to ACL Document for the detail security policies.


Calling Context:
After BHSM_GenerateRouteKey has been called three times to generate Key5, this function will
then be called to generate output for verification.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_procOutCmdIO  - BHSM_ProcOutCmdIO_t.

Output:
inoutp_procOutCmdIO  - BHSM_ProcOutCmdIO_t.



Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_GenerateRouteKey

*****************************************************************************/
BERR_Code   BHSM_ProcOutCmd (
        BHSM_Handle            in_handle,
        BHSM_ProcOutCmdIO_t    *inoutp_procOutCmdIO
);


/* End of Module Specific Functions */

#define BHSM_MAX_OTPKEYID_LEN       8    /* Length of OTP ID in bytes */
#define BHSM_MAX_NONCE_DATA_LEN     16   /* Length of password in bytes */

/***************************************************************************
Summary:
Structure that defines BHSM_KladChallengeIO members

Description:
Structure that defines BHSM_KladChallengeIO members

See Also:
BHSM_KladChallenge
****************************************************************************/
typedef struct BHSM_KladChallengeIO {
    /* Input */
    /* In: needed and meaningful only for key3 generation, to select a key source,
           values in BCMD_RootKeySrc_e */
    BCMD_RootKeySrc_e       keyId;  /* Which OTP Key to read: OTP Keya....OTP Keyf */

    /* Output */
    uint32_t                unStatus; /* status of command */
    unsigned char           aucKeyId[BHSM_MAX_OTPKEYID_LEN]; /* ID of device if successful */
    unsigned char           ucBlackBoxID;
    unsigned int            STBOwnerIDMsp;
} BHSM_KladChallengeIO_t;


typedef enum BCMD_KladMode_e {
    BCMD_KladMode_e3DES = 0,
    BCMD_KladMode_eAES = 1,
    BCMD_KladMode_eMax
} BCMD_KladMode_e;

/***************************************************************************
Summary:
Structure that defines BHSM_KladResponseIO members

Description:
Structure that defines BHSM_KladResponseIO members

See Also:
BHSM_KladResponse
****************************************************************************/
typedef struct BHSM_KladResponseIO {
    /* input */
    /* In: 3DES or AES */
    BCMD_KladMode_e     kladMode;
    /* In: key from which virtual key ladder  is used to decrypt the input data or key */
    BCMD_VKLID_e        virtualKeyLadderID;
    /* In: Key Layer used for key ladder key generation key3 - key5 */
    BCMD_KeyRamBuf_e    keyLayer;
    unsigned char       aucNonce[BHSM_MAX_NONCE_DATA_LEN];          /* 128 bit index */
    /* Output */
    uint32_t            unStatus;        /* status of command: '0' = successful */
    unsigned char       aucReponse[BHSM_MAX_NONCE_DATA_LEN];
} BHSM_KladResponseIO_t;

/*****************************************************************************
Summary:
This function is used to read the chip ID.

Description:
This function is used to read back the chip ID.  This ID is used as an index to
a look-up table to send the proper password to the BSP through the command
BHSM_KladResponse().  If the correct password is submitted, the test ports are
unlocked.  The command can select 3 different IDs of the chip.  If the requested
ID is not allowed by the ACL table, an error status is returned and no more
challenge/response attempts can be done until the next power-on/reset.

The challenge command must be sent after 3.3 seconds have elapsed after power-on/reset.
This prevents rogue code from attempting a brute force attack to accidentally hit upon the
correct password.  The challenge command has to be sent before a password is submitted.
If a challenge is attempted before 3.3 seconds, no more challenge/response attempts
will be accepted until the next power-on/reset.

Performance and Timing:
This is a synchronous/blocking function that won't return until it is done or failed.

Input:
hHsm - BHSM_Handle, Host Secure module handle.
inoutp_kladChallengeIO - BHSM_KladChallengeIO_t, ref/pointer to challenge
                        structure containing the OTP ID to read back.

Output:
inoutp_kladChallengeIO - BHSM_KladChallengeIO_t, ref/pointer to challenge
                        structure containing the 64-bit ID

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_KladResponse
*****************************************************************************/
BERR_Code BHSM_KladChallenge (
    BHSM_Handle              hHsm,
    BHSM_KladChallengeIO_t   *pChallenge
);

/*****************************************************************************
Summary:
This function is used to send the password to the BSP

Description:
This function is used to send the password to the BSP.  In order to send the
correct password, a BHSM_KladChallenge must be called to get the ID of the chip.
When the password is verified, the test ports are unlocked.

Before a password can be submitted, a BHSM_KladChallenge must be called first.  If
a challenge is not done, no more challenge/response commands will be accepted
until the next power-on/reset.

A password can't be submitted until 3.3 seconds have elapsed since the last
power-on reset.  If done before 3.3 seconds, no more challenge/response attempts
can be made until the next power-on/reset.

If the submitted password is invalid, an error will be returned and no more
challenge/response attempts can be made until the next power-on/reset.

Performance and Timing:
This is a synchronous/blocking function that won't return until it is done or failed.

Input:
hHsm - BHSM_Handle, Host Secure module handle.
inoutp_kladResponseIO - BHSM_KladResponseIO_t, ref/pointer to response
                        structure containing the password.

Output:
inoutp_kladResponseIO - BHSM_KladResponseIO_t, ref/pointer to response
                        structure containing status of password verification.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_KladChallenge
*****************************************************************************/
BERR_Code BHSM_KladResponse (
    BHSM_Handle             hHsm,
    BHSM_KladResponseIO_t   *pResponse
);



/**************************************************************************************************
Summary:

Description:


Calling Context:
This function is called by nexus and hsm.

Performance and Timing:
This is a synchronous/blocking function that will not return until it completes.

Input:
vkl id - enum.



Returns:
vkl id - enum.

See Also:
N/A
**************************************************************************************************/
BCMD_VKLID_e BHSM_RemapVklId( BCMD_VKLID_e vkl );






#ifdef __cplusplus
}
#endif

#endif /* BHSM_KEYLADDER_ENC_H__ */
