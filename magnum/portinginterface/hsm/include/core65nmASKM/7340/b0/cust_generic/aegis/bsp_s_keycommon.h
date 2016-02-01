/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * Broadcom Security Processor Code
 ***************************************************************************/
#ifndef BSP_S_KEYCOMMON_H__
#define BSP_S_KEYCOMMON_H__

/* This header file contains common enums for BSP and host*/

/* Supported customer sub-mode enums. The VKLs are allocated to these sub-modes */
typedef enum BCMD_CustomerSubMode_e
{
    BCMD_CustomerSubMode_eGeneric_CA_64_4   = 0x0,  /* Key Ladder for Generic CA with 64 bit keys using Key4 */
    BCMD_CustomerSubMode_eGeneric_CP_64_4   = 0x1,  /* Key Ladder for Generic CP with 64 bit keys using Key4 */
    BCMD_CustomerSubMode_eGeneric_CA_64_5   = 0x2,  /* Key Ladder for Generic CA with 64 bit keys using Key5 */
    BCMD_CustomerSubMode_eGeneric_CP_64_5   = 0x3,  /* Key Ladder for Generic CP with 64 bit keys using Key5 */
    BCMD_CustomerSubMode_eGeneric_CA_128_4  = 0x4,  /* Key Ladder for Generic CA with 128 bit keys using Key4 */
    BCMD_CustomerSubMode_eGeneric_CP_128_4  = 0x5,  /* Key Ladder for Generic CP with 128 bit keys using Key4 */
    BCMD_CustomerSubMode_eGeneric_CA_128_5  = 0x6,  /* Key Ladder for Generic CA with 128 bit keys using Key5 */
    BCMD_CustomerSubMode_eGeneric_CP_128_5  = 0x7,  /* Key Ladder for Generic CP with 128 bit keys using Key5 */
    BCMD_CustomerSubMode_eResserved8        = 0x8,  
    BCMD_CustomerSubMode_eResserved9        = 0x9,  
 
    BCMD_CustomerSubMode_eResserved10       = 0xA,  
    BCMD_CustomerSubMode_eResserved11       = 0xB,  

    BCMD_CustomerSubMode_eResserved12       = 0xC,  
    BCMD_CustomerSubMode_eResserved13       = 0xD,  

    BCMD_CustomerSubMode_eGeneralPurpose1   = 0xE, /* Key Ladder for HDMI, IV and Signed Commands */
    BCMD_CustomerSubMode_eGeneralPurpose2   = 0xF, /* Key Ladder for User Hmac */
    BCMD_CustomerSubMode_eReserved16          = 0x10,
    BCMD_CustomerSubMode_eReserved17       = 0x11,
    BCMD_CustomerSubMode_eReserved18       = 0x12,
    BCMD_CustomerSubMode_eReserved19       = 0x13,
    BCMD_CustomerSubMode_eResserved20       = 0x14,
    BCMD_CustomerSubMode_eResserved21       = 0x15,
    BCMD_CustomerSubMode_eMax               
} BCMD_CustomerSubMode_e;

/* STBOwnerID types */
typedef enum BCMD_STBOwnerID_e
{                    
    BCMD_STBOwnerID_eOTPVal                 = 0, /* Use OTP value as STB Owner ID */
    BCMD_STBOwnerID_eOneVal                 = 1, /* Use value 1 as STB Owner ID */
    BCMD_STBOwnerID_eZeroVal                = 2, /* Use value 0 as STB Owner ID */
    BCMD_STBOwnerID_eMax,                   /* The max enum is 0x3 */
    BCMD_STBOwnerID_eMask                   = 3  /* For ASKM, FW assume max enum is 0-3*/
}BCMD_STBOwnerID_e;

/* Module ID types */
typedef enum BCMD_ModuleID_e
{                    
    BCMD_ModuleID_eZero                     = 0, /* Zero module ID */
    BCMD_ModuleID_eNSK                      = 1, /* NSK_KEY2 module ID */
    BCMD_ModuleID_eAVK                      = 2, /* AVK_KEY2 module ID */
    BCMD_ModuleID_eModuleID_3               = 3, /* Module ID 3 */
    BCMD_ModuleID_eModuleID_4               = 4, /* Module ID 4 */
    BCMD_ModuleID_eModuleID_5               = 5, /* Module ID 5 */
    BCMD_ModuleID_eModuleID_6               = 6, /* Module ID 6 */
    BCMD_ModuleID_eModuleID_7               = 7, /* Module ID 7 */
    BCMD_ModuleID_eModuleID_8               = 8, /* Module ID 8 */
    BCMD_ModuleID_eModuleID_9               = 9, /* Module ID 9 */
    BCMD_ModuleID_eModuleID_10              = 10, /* Module ID 10 */
    BCMD_ModuleID_eModuleID_11              = 11, /* Module ID 11 */
    BCMD_ModuleID_eModuleID_12              = 12, /* Module ID 12 */
    BCMD_ModuleID_eModuleID_13              = 13, /* Module ID 13 */
    BCMD_ModuleID_eModuleID_14              = 14, /* Module ID 14 */
    BCMD_ModuleID_eModuleID_15              = 15, /* Module ID 15 */
    BCMD_ModuleID_eModuleID_16              = 16, /* Module ID 16 */
    BCMD_ModuleID_eModuleID_17              = 17, /* Module ID 17 */
    BCMD_ModuleID_eModuleID_18              = 18, /* Module ID 18 */
    BCMD_ModuleID_eModuleID_19              = 19, /* Module ID 19 */
    BCMD_ModuleID_eModuleID_20              = 20, /* Module ID 20 */
    BCMD_ModuleID_eModuleID_21              = 21, /* Module ID 21 */
    BCMD_ModuleID_eModuleID_22              = 22, /* Module ID 22 */
    BCMD_ModuleID_eModuleID_23              = 23, /* Module ID 23 */
    BCMD_ModuleID_eModuleID_24              = 24, /* Module ID 24 */
    BCMD_ModuleID_eModuleID_25              = 25, /* Module ID 25 */
    BCMD_ModuleID_eMax
}BCMD_ModuleID_e;

/* key slot types supported */
typedef enum BCMD_XptSecKeySlot_e
{                                        
    BCMD_XptSecKeySlot_eType0               = 0, /* 10 entries*/
    BCMD_XptSecKeySlot_eType1               = 1, /* 12 entries*/
    BCMD_XptSecKeySlot_eType2               = 2, /* 14 entries */
    BCMD_XptSecKeySlot_eType3               = 3, /* 16 entries*/
    BCMD_XptSecKeySlot_eType4               = 4, /* 16 entries */
    BCMD_XptSecKeySlot_eType5               = 5, /* 20 entries */
    BCMD_XptSecKeySlot_eType6               = 6, /* 26 entries SVP mode usage only */
    BCMD_XptSecKeySlot_eType7               = 7, /* 26 entries */
    BCMD_XptSecKeySlot_eTypeMax
}BCMD_XptSecKeySlot_e;

/* 
 * key ladder root key src enum used for key ladder
 * root key generation 
 */
typedef enum BCMD_RootKeySrc_e
{
    BCMD_RootKeySrc_eCusKey                 = 0,
    BCMD_RootKeySrc_eOTPKeya                = 1,
    BCMD_RootKeySrc_eOTPKeyb                = 2,
    BCMD_RootKeySrc_eOTPKeyc                = 3,
    BCMD_RootKeySrc_eReserved0              = 4,
    BCMD_RootKeySrc_eReserved1              = 5,
    BCMD_RootKeySrc_eReserved2              = 6,
    BCMD_RootKeySrc_eASKMSel                = 7,
    BCMD_RootKeySrc_eMax
}BCMD_RootKeySrc_e;

/* 
 * key ladder root key destination enum used for key ladder
 * root key generation 
 */
typedef enum BCMD_GenRootKeyDest_e
{
    BCMD_GenRootKeyDest_eReserved0          = 0,
    BCMD_GenRootKeyDest_eReserved1          = 1,
    BCMD_GenRootKeyDest_eReserved2          = 2,
    BCMD_GenRootKeyDest_eReserved3          = 3,	
    BCMD_GenRootKeyDest_eMax
}BCMD_GenRootKeyDest_e;

/* Virtual Key ladder ID */
typedef enum BCMD_VKLID_e
{
    BCMD_VKL0                               = 0, /* Virtual Key Ladder 0 */
    BCMD_VKL1                               = 1, /* Virtual Key Ladder 1 */
    BCMD_VKL2                               = 2, /* Virtual Key Ladder 2 */
    BCMD_VKL3                               = 3, /* Virtual Key Ladder 3 */
    BCMD_VKL4                               = 4, /* Virtual Key Ladder 4 */
    BCMD_VKL5                               = 5, /* Virtual Key Ladder 5 */
    BCMD_VKL6                               = 6, /* Virtual Key Ladder 6 */
    BCMD_VKL7                               = 7, /* Virtual Key Ladder 7 */

    BCMD_VKLDummy                           = 8,  
    BCMD_VKL_KeyRam_eMax                    = 8, /* Max of VKL ram FW supported */
    /* Reserve 7-0x1F for future VKL expand */
    BCMD_SWKey                              = 0x10, /* SW key source */
    BCMD_VKLReserved17                      = 0x11,

    BCMD_VKLReserved18                      = 0x12,
    BCMD_VKLReserved19                      = 0x13,
    BCMD_VKLReserved20                      = 0x14,
    BCMD_VKL_eMax
}BCMD_VKLID_e;

/* 
 * key ladder key source/destination enum used for key ladder
 * non-root key generation from key3 - key5
 */
typedef enum BCMD_KeyRamBuf_e
{
    BCMD_KeyRamBuf_eReserved2               = 0,       
    BCMD_KeyRamBuf_eReserved3               = 1,
    BCMD_KeyRamBuf_eReserved4               = 2,

    BCMD_KeyRamBuf_eKey3                    = 3, /* key3 */
    BCMD_KeyRamBuf_eKey4                    = 4, /* key4 */
    BCMD_KeyRamBuf_eKey5                    = 5, /* key5 */
    
    BCMD_KeyRamBuf_eReserved0               = 6,
    BCMD_KeyRamBuf_eReserved1               = 7,

    BCMD_KeyRamBuf_eFirstRam                = 8, /* First 192 bit user key ram*/
    BCMD_KeyRamBuf_eSecondRam               = 9, /* Second 192 bit user key ram*/
    BCMD_KeyRamBuf_eThirdRam                = 10, /* third 192 bit user key ram*/

    BCMD_KeyRamBuf_eFirstRam2048            = 11,   /* First 2048bit user ram*/
    BCMD_KeyRamBuf_eSecondRam2048           = 12,   /* Second 2048bit user ram*/
    BCMD_KeyRamBuf_eThirdRam2048            = 13,   /* Third 2048bit user ram*/

    BCMD_KeyRamBuf_eFirstRNRam1024          = 14,  /* First 1024 bit user ram*/
    BCMD_KeyRamBuf_eSecondRNRam1024         = 15,  /* Second 1024 bit user ram*/
    BCMD_KeyRamBuf_eMax

}BCMD_KeyRamBuf_e;

/* swizzle type supported*/
typedef enum BCMD_SwizzleType_e
{
    BCMD_SwizzleType_eNoSwizzle             = 0,
    BCMD_SwizzleType_eSwizzle1              = 1,
    BCMD_SwizzleType_eSwizzle0              = 2,
    BCMD_SwizzleType_eMax
}BCMD_SwizzleType_e;


/* key destination block types from host side */
typedef enum BCMD_KeyDestBlockType_e
{
    BCMD_KeyDestBlockType_eCA               = 0, /* for CA */  
    
    BCMD_KeyDestBlockType_eCPScrambler      = 1, /* for use of CP scrambler */
	BCMD_KeyDestBlockType_eReserved1		= 1, 

    BCMD_KeyDestBlockType_eCPDescrambler    = 2, /* for use of CP descrambler */ 
	BCMD_KeyDestBlockType_eReserved0		= 2, 
    
    BCMD_KeyDestBlockType_eRmx              = 3, /* Rmx or other use of CP scrambler */ 
    BCMD_XptKeyTblType_eMax                 = 3, /* This is used by FW to construct structure array */
    BCMD_KeyDestBlockType_eMem2Mem          = 4, /* M2M0 for CP */
    BCMD_KeyDestBlockType_eHdmi             = 5, /* HDMI */
    
#if defined (SECOND_MEM2MEM)
    BCMD_KeyDestBlockType_eReserved2        = 6, /* reserved in other chip */
    BCMD_KeyDestBlockType_eMem2Mem1         = 7, /* M2M1 for CP */
    /* BCMD_KeyDestBlockType_eSharf   = 8 */
#else
    BCMD_KeyDestBlockType_eReserved3        = 6, /* reserved in other chip */
    BCMD_KeyDestBlockType_eReserved4        = 7, /* M2M1 for CP */
#endif
    
    /* The 5 enums below are only used for ACL. They are not used by the host */
    BCMD_KeyDestBlockType_eIV               = 9,
    BCMD_KeyDestBlockType_eProcOut          = 10,
    BCMD_KeyDestBlockType_eSignedCmds       = 11,
    BCMD_KeyDestBlockType_eUserHmac         = 12,
    BCMD_KeyDestBlockType_eReserved13       = 13,
    /* The 5 enums above are only used for ACL. They are not used by the host */
    BCMD_KeyDestBlockType_eMax
    
} BCMD_KeyDestBlockType_e;

/* key destination entry types*/
typedef enum BCMD_KeyDestEntryType_e
{
    BCMD_KeyDestEntryType_eOddKey           = 0,
    BCMD_KeyDestEntryType_eEvenKey          = 1,
    BCMD_KeyDestEntryType_eReserved0        = 2,
    BCMD_KeyDestEntryType_eIV               = 3,
    BCMD_KeyDestEntryType_eMax
} BCMD_KeyDestEntryType_e;

/* All the algorithms supported in XPT Security blocks*/
typedef enum BCMD_XptM2MSecCryptoAlg_e	 
{  
    BCMD_XptM2MSecCryptoAlg_eDvb               = 0,  
    BCMD_XptM2MSecCryptoAlg_eReserved0         = 1,
    BCMD_XptM2MSecCryptoAlg_eMulti2            = 2,
    BCMD_XptM2MSecCryptoAlg_eDes               = 3,
    BCMD_XptM2MSecCryptoAlg_e3DesAba           = 4,
    BCMD_XptM2MSecCryptoAlg_e3DesAbc           = 5,
    BCMD_XptM2MSecCryptoAlg_eAes128            = 6,
    BCMD_XptM2MSecCryptoAlg_eReserved1         = 7,

    BCMD_XptM2MSecCryptoAlg_eAes192            = 8,
    BCMD_XptM2MSecCryptoAlg_eTivoCipher        = 9,
    BCMD_XptM2MSecCryptoAlg_eC2                = 10,
    BCMD_XptM2MSecCryptoAlg_eCss               = 11,
    BCMD_XptM2MSecCryptoAlg_eM6KE              = 12,
    BCMD_XptM2MSecCryptoAlg_eM6                = 13,
    BCMD_XptM2MSecCryptoAlg_eRc4               = 14,
    BCMD_XptM2MSecCryptoAlg_eCbcMac            = 15,
    BCMD_XptM2MSecCryptoAlg_eWMDrmPd           = 16,
#if defined (AACS)
    BCMD_XptM2MSecCryptoAlg_eAes128G           = 17,
    BCMD_XptM2MSecCryptoAlg_eHdDVD             = 18,
    BCMD_XptM2MSecCryptoAlg_eBrDVD             = 19,
#endif
    BCMD_XptM2MSecCryptoAlg_eMax
}BCMD_XptM2MSecCryptoAlg_e;

/* key size enum */
typedef enum BCMD_KeySize_e
{
    BCMD_KeySize_e64                        = 0,
    BCMD_KeySize_e128                       = 1,
    BCMD_KeySize_e192                       = 2,
    BCMD_KeySize_eMax
		
}BCMD_KeySize_e;
/* key size union used for user key and key ladder keys*/
typedef union BCMD_KeySize_u
{
    BCMD_KeySize_e eKeySize;
    unsigned short usKeySize; /* number of bits used only for 2048 bit user key*/
}BCMD_KeySize_u;

/* key ladder type enum*/
typedef enum BCMD_KeyLadderType_e
{
    BCMD_KeyLadderType_e1DES                = 0, 
    BCMD_KeyLadderType_e3DESABA             = 1,
    BCMD_KeyLadderType_eAES128              = 2,
    BCMD_KeyLadderType_eMax
}BCMD_KeyLadderType_e;
/* key ladder operation enum*/
typedef enum BCMD_KeyLadderOp_e
{  
    BCMD_KeyLadderOp_eDecrypt               = 0,
    BCMD_KeyLadderOp_eEncrypt               = 1,
    BCMD_KeyLadderOp_eMax
}BCMD_KeyLadderOp_e;

typedef enum BCMD_Key3Op_e
{  
    BCMD_Key3Op_eKey3NoProcess = 0,
    BCMD_Key3Op_eKey3Export    = 1,
	BCMD_Key3Op_eReserved0	   = 2

}BCMD_Key3Op_e;

/* All the key mode that Aegis supports - 
this is algorithm dependent*/
typedef enum BCMD_KeyMode_e
{
    BCMD_KeyMode_eRegular                   = 0,
    BCMD_KeyMode_eDes56                     = 1,
    BCMD_KeyMode_eReserved0                 = 2,
    BCMD_KeyMode_eReserved1                 = 3,
    BCMD_KeyMode_eDvbConformance            = 4,
    BCMD_KeyMode_eMax
}BCMD_KeyMode_e;

/* enum for key invalidation command*/
typedef enum BCMD_InvalidateKey_Flag_e
{ 
    BCMD_InvalidateKey_Flag_eSrcKeyOnly     =0,
    BCMD_InvalidateKey_Flag_eDestKeyOnly    =1,
    BCMD_InvalidateKey_Flag_eBoth           =2,
    BCMD_InvalidateKey_Flag_eMax
} BCMD_InvalidateKey_Flag_e;

/* Algorithm Termination Mode enums, has to match with RDB definition */
typedef enum BCMD_TerminationMode_e
{
    BCMD_TerminationMode_eClear             = 0, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_TERM_MODE_SEL_CLEAR_TERM */
    
    BCMD_TerminationMode_eCbcResidual       = 1, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_TERM_MODE_SEL_RES_TERM */
    BCMD_TerminationMode_eReserved2         = 2,

    BCMD_TerminationMode_eCts_Cpcm          = 3, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_TERM_MODE_SEL_CTS2 */
    BCMD_TerminationMode_eReserved4         = 4,
    BCMD_TerminationMode_eReserved5         = 5,

    BCMD_TerminationMode_eMax
} BCMD_TerminationMode_e;

/* Algorithm Cipher Mode Select enums */
typedef enum BCMD_CipherModeSelect_e
{
    BCMD_CipherModeSelect_eECB              = 0, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_NOT_DVB_CIPHER_MODE_SEL_ECB */
    BCMD_CipherModeSelect_eXptMode          = 0, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_DVB_DVB_MODE_SEL_TRANSPORT_MODE */
    BCMD_CipherModeSelect_eCBC              = 1, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_NOT_DVB_CIPHER_MODE_SEL_CBC */
    BCMD_CipherModeSelect_ePesMode          = 1, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_DVB_DVB_MODE_SEL_PES_MODE */
    BCMD_CipherModeSelect_eCTR              = 2, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_NOT_DVB_CIPHER_MODE_SEL_COUNTER */
    BCMD_CipherModeSelect_eRCBC             = 3, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_NOT_DVB_CIPHER_MODE_SEL_RCBC */
    BCMD_CipherModeSelect_eMax
} BCMD_CipherModeSelect_e;

/* Algorithm IV handling enums */
typedef enum BCMD_IVSelect_e
{
    BCMD_IVSelect_eRegular                  = 0, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_IV_MODE_SEL_NO_PREPROC */
    BCMD_IVSelect_eReserved1                = 1, 
    BCMD_IVSelect_eReserved2                = 2, 
    BCMD_IVSelect_eMax
} BCMD_IVSelect_e;

/* Algorithm SOLITARY enums */
typedef enum BCMD_SolitarySelect_e
{
    BCMD_SolitarySelect_eClear              = 0, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_SOLITARY_TERM_SEL_CLEAR */
    BCMD_SolitarySelect_eSa                 = 1, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_SOLITARY_TERM_SEL_XOR_ENC_0 */
    BCMD_SolitarySelect_eCbcXorIV           = 2, /* XPT_SECURITY_KEY_TABLE_MODULE_KEY_MODE_SOLITARY_TERM_SEL_XOR_IV */
    BCMD_SolitarySelect_eReserved3          = 3,

    BCMD_SolitarySelect_eReserved4          = 4,

    BCMD_SolitarySelect_eMax
} BCMD_SolitarySelect_e;

/* command interface enums */
/* Output buffer table for all the following commands*/
/* common output header bytes(first 5 words - see bsp_s_commands.h)
|--------------|------------|------------|------------------------------------| 
|  31..24      | 23..16     | 15..8      | 7..0                               | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_CommonBufferFields_eVerNum    | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_CommonBufferFields_eOwnerId   | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_CommonBufferFields_eContMode  | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | BCMD_CommonBufferFields_eParamLen               | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_CommonBufferFields_eStatus    | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_DATA_OUTBUF                   | 
|--------------|------------|------------|------------------------------------| 
*/
#define BCMD_DATA_OUTBUF        (6 << 2)  /* output data start from */
/* Command buffer information for BCMD_KeyLoad_InitKeySlot*/ 
/*
|--------------|-------------------|-------------------|-----------------------| 
|  31..24      | 23..16            | 15..8             | 7..0                  | 
|--------------|-------------------|-------------------|-----------------------| 
|  unused      | unused            | unused            | BCMD_KeySlot_InCmdInit| 
|--------------|-------------------|-------------------|-----------------------| 
*/
#define BCMD_KeySlot_InCmdInit  (5 << 2) + 3         /* 8 bit index*/

/* Command buffer information for BCMD_KeyLoad_ConfigKeyPointerTable for 7401A0 */
/*
|--------------|------------|------------|------------------------------------| 
|  31..24      | 23..16     | 15..8      | 7..0                               | 
|--------------|------------|------------|------------------------------------| 
|  SetSlotB    | unused     | unused     | BCMD_KeyPointer_InCmdCfg_ePidChan  | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_KeyPointer_InCmdCfg_eSlotType | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_KeyPointer_InCmdCfg_eSlotNumber| 
|--------------|------------|------------|------------------------------------| 
*/
/* Additional command buffers for 7400A0 and beyond */
/*
|--------------|------------|------------|------------------------------------| 
|  31..24      | 23..16     | 15..8      | 7..0                               | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_KeyPointer_InCmdCfg_eSlotTypeB | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_KeyPointer_InCmdCfg_eSlotNumberB| 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_KeyPointer_InCmdCfg_eKeyPointerSel| 
|--------------|------------|------------|------------------------------------| 
*/

typedef enum BCMD_KeyPointer_InCmdCfg_e
{
    /* 
     * Bit31 is for slot B, 
     * Bit30 is for reseting back to default key entries
     * LSbyte is for pidchan
     */
    BCMD_KeyPointer_InCmdCfg_ePidChan       = (5 << 2),   
    BCMD_KeyPointer_InCmdCfg_eSlotType      = (6 << 2) + 3,  /* 8 bit index*/
    BCMD_KeyPointer_InCmdCfg_eSlotNumber    = (7 << 2) + 3,  /* 8 bit index*/
    BCMD_KeyPointer_InCmdCfg_eSlotTypeB     = (8 << 2) + 3,  /* 8 bit index*/
    BCMD_KeyPointer_InCmdCfg_eSlotNumberB   = (9 << 2) + 3,  /* 8 bit index*/
    BCMD_KeyPointer_InCmdCfg_eKeyPointerSel = (10 << 2) + 3  /* 8 bit index*/
} BCMD_KeyPointer_InCmdCfg_e;

/* Command buffer information for BCMD_KeyLoad_ConfigMulti2 */
/*
|--------------|------------|------------|------------------------------------| 
|  31..24      | 23..16     | 15..8      | 7..0                               | 
|--------------|------------|------------|------------------------------------| 
|                      BCMD_Multi2_InCmdCfg_eRoundCount                       | 
|--------------|------------|------------|------------------------------------| 
|                      BCMD_Multi2_InCmdCfg_eSystemKeys (8-32 bits word)      | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_Multi2_InCmdCfg_eWhichSysKey  | 
|--------------|------------|------------|------------------------------------| 
*/
typedef enum BCMD_InCmdCfgMulti2_e
{
    BCMD_Multi2_InCmdCfg_eRoundCount        = (5 << 2),       /* 32 bit index*/
    BCMD_Multi2_InCmdCfg_eSystemKeys        = (6 << 2) ,      /* 32 bit index - 256 bit */
    BCMD_Multi2_InCmdCfg_eWhichSysKey       = (14 << 2) + 3   /* 8 bit index*/
} BCMD_InCmdCfgMulti2_e;

/* BCMD_KeyLoad_GenerateRouteKey input buffer enum */
/*
|--------------|------------|-------------------------------|------------------------------------| 
|  31..24      | 23..16     | 15..8                         | 7..0                               | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyLadderType   | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | BCMD_GenKey_InCmd_eCustomerSel| BCMD_GenKey_InCmd_eRootKeySrc| 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eSwizzle1IndexSel| 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eSwizzleType     | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eCusKeySelL      | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyVarL         | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eCusKeySelH      | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyVarH         | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eVKLID           | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyLayer        | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyLadderOpera  | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eProcIn (6 - 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeySize         | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | BCMD_GenKey_InCmd_eSwapAESKey | BCMD_GenKey_InCmd_eRouteKeyFlag    | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eBlkType         | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eEntryType       | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeySlotType     | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeySlotNumber   | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eKeyMode         | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eCtrlWord0 ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eCtrlWord1 ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eCtrlWord2 ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eCtrlWord3 ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenKey_InCmd_eSTBOwnerIDSel   | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | BCMD_GenKey_InCmd_eCAVendorID                                      | 
|--------------|------------|-------------------------------|------------------------------------| 
|  unused      | unused     | BCMD_GenKey_InCmd_eTestKeySel | BCMD_GenKey_InCmd_eModuleID        | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eFastCyclingProcIn ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eFastCyclingProcIn ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eFastCyclingProcIn ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
|                               BCMD_GenKey_InCmd_eFastCyclingProcIn ( 32 bits words )                   | 
|--------------|------------|-------------------------------|------------------------------------| 
*/
typedef enum BCMD_GenKey_InCmd_e
{
    BCMD_GenKey_InCmd_eKeyLadderType            = (5 << 2) + 3,	/* 8 bit index*/ 
    BCMD_GenKey_InCmd_eRootKeySrc               = (6 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eCustomerSel              = (6 << 2) + 2, /* 8 bit index*/
    BCMD_GenKey_InCmd_eSwizzle1IndexSel         = (7 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eSwizzleType              = (8 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eCusKeySelL               = (9 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeyVarL                  = (10 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eCusKeySelH               = (11 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeyVarH                  = (12 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eVKLID                    = (13 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeyLayer                 = (14 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKey3Operation 		    = (14 << 2) + 2, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeyLadderOpera           = (15 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eProcIn                   = (16 << 2),     /* 32 bit index*/
    BCMD_GenKey_InCmd_eKeySize                  = (24 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eSwapAESKey               = (25 << 2) + 2, /* 8 bit index*/
    BCMD_GenKey_InCmd_eRouteKeyFlag             = (25 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eBlkType                  = (26 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eEntryType                = (27 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeySlotType              = (28 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeySlotNumber            = (29 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eKeyMode                  = (30 << 2) + 3, /* 8 bit index*/
    BCMD_GenKey_InCmd_eCtrlWord0                = (31 << 2) ,    /* 32 bits index */	
    BCMD_GenKey_InCmd_eCtrlWord1                = (32 << 2) ,    /* 32 bits index */	
    BCMD_GenKey_InCmd_eCtrlWord2                = (33 << 2) ,    /* 32 bits index */	
    BCMD_GenKey_InCmd_eCtrlWord3                = (34 << 2),     /* 32 bits index */	
    BCMD_GenKey_InCmd_eSTBOwnerIDSel            = (35 << 2) + 3, /* 8 bit */
    BCMD_GenKey_InCmd_eCAVendorID               = (36 << 2) + 2, /* 16 bit */ 
    BCMD_GenKey_InCmd_eTestKeySel               = (37 << 2) + 2, /* 8 bit */
    BCMD_GenKey_InCmd_eModuleID                 = (37 << 2) + 3, /* 8 bit */
    BCMD_GenKey_InCmd_eFastCyclingProcIn 	    = (38 << 2)      /* 32 bit index*/
} BCMD_GenKey_InCmd_e;

typedef enum BCMD_GenKey_OutCmdField_e
{
    BCMD_GenKey_OutCmd_eReserved        = (6 << 2),        /* 32 bit */
    BCMD_GenKey_OutCmd_eKey3ExportSize  = (7 << 2) + 3,    /* 8 bit */
    BCMD_GenKey_OutCmd_eKey3Export      = (8 << 2)         /* 8 bit */
} BCMD_GenKey_OutCmdField_e;

/* BCMD_KeyLoad_GenRootKey input buffer enum */
/*
|--------------|------------|-------------------------------|---------------------------------------| 
|  31..24      | 23..16     | 15..8                         | 7..0                                  | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eKeyLadderType  | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eRootKeySrc     | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eSwizzle1IndexSel| 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eSwizzleType    | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eCusKeySelL     | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eKeyVarL        | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eCusKeySelH     | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eKeyVarH        | 
|--------------|------------|-------------------------------|---------------------------------------| 
|  unused      | unused     | unused                        | BCMD_GenRootKey_InCmd_eDest(4 byte)   | 
|--------------|------------|------------|----------------------------------------------------------| 
|        BCMD_GenRootKey_InCmd_eWrappedKey (4 - 32 bit words)                                       | 
|--------------|------------|------------|----------------------------------------------------------| 
|        BCMD_GenRootKey_InCmd_eDigest(3 - 32 bit words)                                     | 
|--------------|------------|------------|----------------------------------------------------------| 
|  unused      | unused     | unused     | BCMD_GenRootKey_InCmd_eTDesOpera                         | 
|--------------|------------|------------|----------------------------------------------------------| 
|  unused      | unused     | unused     | BCMD_GenRootKey_InCmd_eMaxOTPErr                         | 
|--------------|------------|------------|----------------------------------------------------------| 
*/
typedef enum BCMD_GenRootKey_InCmd_e
{
    BCMD_GenRootKey_InCmd_eKeyLadderType        = (5 << 2) + 3,	/* 8 bit index*/ 
    BCMD_GenRootKey_InCmd_eRootKeySrc           = (6 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eSwizzle1IndexSel     = (7 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eSwizzleType          = (8 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eCusKeySelL           = (9 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eKeyVarL              = (10 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eCusKeySelH           = (11 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eKeyVarH              = (12 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eDest			= (13 << 2) ,    /* 32 bit dest*/	  
    BCMD_GenRootKey_InCmd_eWrappedKey           = (14 << 2),     /* 128 bits here*/
    BCMD_GenRootKey_InCmd_eDigest        	= (18 << 2) ,    /* 96 bits digest*/	  
    BCMD_GenRootKey_InCmd_eTDesOpera            = (21 << 2) + 3, /* 8 bit index*/
    BCMD_GenRootKey_InCmd_eMaxOTPErr            = (22 << 2) + 3  /* 8 bit - only allow 0 or 1 */
} BCMD_GenRootKey_InCmd_e;

/* Command buffer information for BCMD_KeyLoad_RouteUserKey */
/*
|--------------|------------|------------|------------------------------------| 
|  31..24      | 23..16     | 15..8      | 7..0                               | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eKeyBuffer   | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eKeyLength   | 
|--------------|------------|------------|------------------------------------| 
|           BCMD_LoadUseKey_InCmd_eKeyData (64 - 32 bit words)                | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eRouteKeyFlag| 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eBlkType     | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eEntryType   | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eKeySlotType | 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eKeySlotNumber| 
|--------------|------------|------------|------------------------------------| 
|  unused      | unused     | unused     | BCMD_LoadUseKey_InCmd_eKeyMode     | 
|--------------|------------|------------|------------------------------------| 
|          BCMD_LoadUseKey_InCmd_eCtrlWord0( 32 bits words )                  | 
|--------------|------------|------------|------------------------------------| 
|          BCMD_LoadUseKey_InCmd_eCtrlWord1( 32 bits words )                  | 
|--------------|------------|------------|------------------------------------| 
|          BCMD_LoadUseKey_InCmd_eCtrlWord2( 32 bits words )                  | 
|--------------|------------|------------|------------------------------------| 
|          BCMD_LoadUseKey_InCmd_eCtrlWord3( 32 bits words )                  | 
|--------------|------------|------------|------------------------------------| 
*/
typedef enum BCMD_LoadUseKey_InCmd_e
{
    BCMD_LoadUseKey_InCmd_eKeyBuffer            = (5 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eKeyLength            = (6 << 2) + 2, /* 16 bit index*/
    BCMD_LoadUseKey_InCmd_eKeyData              = (7 << 2) ,    /* 32 bits max 2048 bits - 64 words*/	
    BCMD_LoadUseKey_InCmd_eRouteKeyFlag         = (71 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eBlkType              = (72 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eEntryType            = (73 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eKeySlotType          = (74 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eKeySlotNumber        = (75 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eKeyMode              = (76 << 2) + 3, /* 8 bit index*/
    BCMD_LoadUseKey_InCmd_eCtrlWord0            = (77 << 2) ,    /* 32 bits index */	
    BCMD_LoadUseKey_InCmd_eCtrlWord1            = (78 << 2) ,    /* 32 bits index */	
    BCMD_LoadUseKey_InCmd_eCtrlWord2            = (79 << 2) ,    /* 32 bits index */	
    BCMD_LoadUseKey_InCmd_eCtrlWord3            = (80 << 2)      /* 32 bits index */	
} BCMD_LoadUseKey_InCmd_e;
/* command buffer enums for BCMD_KeyLadder_InvalidateKey*/
/*
|--------------|------------|-----------------------------------|----------------------------------------| 
|  31..24      | 23..16     | 15..8                             | 7..0                                   | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | unused                            | BCMD_InvalidateKey_InCmd_eKeyFlag      | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | BCMD_InvalidateKey_InCmd_eKeyLayer| BCMD_InvalidateKey_InCmd_eVKLID        | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | unused                            | BCMD_InvalidateKey_InCmd_eBlkType      | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | unused                            | BCMD_InvalidateKey_InCmd_eEntryType    | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | unused                            | BCMD_InvalidateKey_InCmd_eKeySlotType  | 
|--------------|------------|-----------------------------------|----------------------------------------| 
|  unused      | unused     | unused                            | BCMD_InvalidateKey_InCmd_eKeySlotNumber| 
|--------------|------------|-----------------------------------|----------------------------------------| 
*/
typedef enum BCMD_InvalidateKey_InCmd_e
{
    BCMD_InvalidateKey_InCmd_eKeyFlag           = (5 << 2) + 3, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eKeyLayer          = (6 << 2) + 2, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eVKLID             = (6 << 2) + 3, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eBlkType           = (7 << 2) + 3, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eEntryType         = (8 << 2) + 3, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eKeySlotType       = (9 << 2) + 3, /* 8 bit index*/
    BCMD_InvalidateKey_InCmd_eKeySlotNumber     = (10 << 2) + 3 /* 8 bit index*/
} BCMD_InvalidateKey_InCmd_e;
/* command buffer defines for BCMD_GetProcOut*/
/*
|--------------|------------|---------|----------------------------------------| 
|  31..24      | 23..16     | 15..8   | 7..0                                   | 
|--------------|------------|---------|----------------------------------------| 
|  unused      | unused     | unused  |  BCMD_ProcOut_InCmd_eVKLID             | 
|--------------|------------|---------|----------------------------------------|
|              BCMD_ProcOut_InCmd_ProcIn 4 32 bits words                       | 
|--------------|------------|---------|----------------------------------------|  
*/
typedef enum BCMD_ProcOut_InCmd_e
{
    BCMD_ProcOut_InCmd_eVKLID                   = (5 << 2)+ 3, /* 8 bit index */
    BCMD_ProcOut_InCmd_eProcIn                  = 6 << 2       /* 4 - 32 bit words*/ 
} BCMD_ProcOut_InCmd_e;

typedef enum BCMD_ASKM_KEY_GEN_OutCmdField_e
{
    BCMD_ASKM_KEY_GEN_OutCmd_eZeroDetectDump    = (6 << 2) /* 32 bit */
} BCMD_ASKM_KEY_GEN_OutCmdField_e;
#endif

