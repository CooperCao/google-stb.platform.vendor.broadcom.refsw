/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

#ifndef BSP_S_KEYCOMMON_H__
#define BSP_S_KEYCOMMON_H__


#define BSP_SWIZZLE0A_VERSION 0x0
#define BSP_ASKM_KDF_VERSION  0x0

typedef enum BCMD_CustomerSubMode_e
{
    BCMD_CustomerSubMode_eGeneric_CA_64_4 = 0x0,
    BCMD_CustomerSubMode_eGeneric_CP_64_4 = 0x1,
    BCMD_CustomerSubMode_eGeneric_CA_64_5 = 0x2,
    BCMD_CustomerSubMode_eGeneric_CP_64_5 = 0x3,
    BCMD_CustomerSubMode_eGeneric_CA_128_4 = 0x4,
    BCMD_CustomerSubMode_eGeneric_CP_128_4 = 0x5,
    BCMD_CustomerSubMode_eGeneric_CA_128_5 = 0x6,
    BCMD_CustomerSubMode_eGeneric_CP_128_5 = 0x7,
    BCMD_CustomerSubMode_eGeneric_CA_64_7 = 0x8,
    BCMD_CustomerSubMode_eGeneric_CA_128_7 = 0x9,


    BCMD_CustomerSubMode_eReserved10 = 0xA,
    BCMD_CustomerSubMode_eReserved11 = 0xB,


    BCMD_CustomerSubMode_eReserved12 = 0xC,

    BCMD_CustomerSubMode_eReserved13 = 0xD,

    BCMD_CustomerSubMode_eGeneralPurpose1 = 0xE,
    BCMD_CustomerSubMode_eGeneralPurpose2 = 0xF,

    BCMD_CustomerSubMode_eReserved16 = 0x10,

    BCMD_CustomerSubMode_eReserved17 = 0x11,
    BCMD_CustomerSubMode_eGeneric_CA_64_45 = 0x12,
    BCMD_CustomerSubMode_eGeneric_CP_64_45 = 0x13,
    BCMD_CustomerSubMode_eHWKL = 0x14,
    BCMD_CustomerSubMode_eReserved21  =  0x15,
    BCMD_CustomerSubMode_eRESERVED21 = 0x15,
    BCMD_CustomerSubMode_eETSI_5 = 0x17,
    BCMD_CustomerSubMode_eReserved24  =  0x18,
    BCMD_CustomerSubMode_eReserved25  =  0x19,
    BCMD_CustomerSubMode_eRESERVED24 = 0x18,
    BCMD_CustomerSubMode_eRESERVED25 = 0x19,
    BCMD_CustomerSubMode_eReserved26  =  0x1A,
    BCMD_CustomerSubMode_eReserved27  =  0x1B,
    BCMD_CustomerSubMode_eRESERVED26 = 0x1A,
    BCMD_CustomerSubMode_eRESERVED27 = 0x1B,
    BCMD_CustomerSubMode_eSCTE52_CA_5 = 0x1C,
    BCMD_CustomerSubMode_eSAGE_BL_DECRYPT = 0x1D,
    BCMD_CustomerSubMode_eReserved30  =  0x1E,
    BCMD_CustomerSubMode_eRESERVED30 = 0x1E,
    BCMD_CustomerSubMode_eRESERVED31 = 0x1F,


    BCMD_CustomerSubMode_eReserved32  =  0x20,
    BCMD_CustomerSubMode_eRESERVED32 = 0x20,
    BCMD_CustomerSubMode_eReserved33  =  0x21,

    BCMD_CustomerSubMode_eReserved34  =  0x22,
    BCMD_CustomerSubMode_eRESERVED34 = 0x22,
    BCMD_CustomerSubMode_eDupleSource = 0x23,
    BCMD_CustomerSubMode_eDupleDestination = 0x24,
    BCMD_CustomerSubMode_eRESERVED35 = 0x23,
    BCMD_CustomerSubMode_eRESERVED36 = 0x24,
    BCMD_CustomerSubMode_eOTPKeyFieldProgramDataDecrypt = 0x25,
    BCMD_CustomerSubMode_eRESERVED38 = 0x26,
    BCMD_CustomerSubMode_eRPMB = 0x27,
    BCMD_CustomerSubMode_eReserved40  =  0x28,
    BCMD_CustomerSubMode_eMax
} BCMD_CustomerSubMode_e;

typedef enum BCMD_STBOwnerID_e
{
    BCMD_STBOwnerID_eOTPVal = 0,
    BCMD_STBOwnerID_eOneVal = 1,
    BCMD_STBOwnerID_eZeroVal = 2,
    BCMD_STBOwnerID_eMax = 3
}BCMD_STBOwnerID_e;

typedef enum BCMD_ModuleID_e
{
    BCMD_ModuleID_eReserved0  =  0,
    BCMD_ModuleID_eZero = 0,
    BCMD_ModuleID_eRESERVED01 = 1,
    BCMD_ModuleID_eRESERVED02 = 2,
    BCMD_ModuleID_eModuleID_3 = 3,
    BCMD_ModuleID_eModuleID_4 = 4,
    BCMD_ModuleID_eModuleID_5 = 5,
    BCMD_ModuleID_eModuleID_6 = 6,
    BCMD_ModuleID_eModuleID_7 = 7,
    BCMD_ModuleID_eModuleID_8 = 8,
    BCMD_ModuleID_eModuleID_9 = 9,
    BCMD_ModuleID_eModuleID_10 = 10,
    BCMD_ModuleID_eModuleID_11 = 11,
    BCMD_ModuleID_eModuleID_12 = 12,
    BCMD_ModuleID_eModuleID_13 = 13,
    BCMD_ModuleID_eModuleID_14 = 14,
    BCMD_ModuleID_eModuleID_15 = 15,
    BCMD_ModuleID_eModuleID_16 = 16,
    BCMD_ModuleID_eModuleID_17 = 17,
    BCMD_ModuleID_eModuleID_18 = 18,
    BCMD_ModuleID_eModuleID_19 = 19,
    BCMD_ModuleID_eModuleID_20 = 20,
    BCMD_ModuleID_eModuleID_21 = 21,
    BCMD_ModuleID_eModuleID_22 = 22,
    BCMD_ModuleID_eModuleID_23 = 23,
    BCMD_ModuleID_eModuleID_24 = 24,
    BCMD_ModuleID_eModuleID_25 = 25,
    BCMD_ModuleID_eModuleID_26 = 26,
    BCMD_ModuleID_eModuleID_27 = 27,
    BCMD_ModuleID_eModuleID_28 = 28,
    BCMD_ModuleID_eModuleID_29 = 29,
    BCMD_ModuleID_eModuleID_30 = 30,
    BCMD_ModuleID_eModuleID_31 = 31,
    BCMD_ModuleID_eWord0Boundary = 32,
    BCMD_ModuleID_eModuleID_32 = 32,
    BCMD_ModuleID_eModuleID_33 = 33,
    BCMD_ModuleID_eModuleID_34 = 34,
    BCMD_ModuleID_eModuleID_35 = 35,
    BCMD_ModuleID_eMax

}BCMD_ModuleID_e;

typedef enum BCMD_XptSecKeySlot_e
{
    BCMD_XptSecKeySlot_eType0 = 0,
    BCMD_XptSecKeySlot_eType1 = 1,
    BCMD_XptSecKeySlot_eType2 = 2,
    BCMD_XptSecKeySlot_eType3 = 3,
    BCMD_XptSecKeySlot_eType4 = 4,
    BCMD_XptSecKeySlot_eType5 = 5,
    BCMD_XptSecKeySlot_eTypeInlineKeySlotMax = 6,
    BCMD_XptSecKeySlot_eTypeMulti2SysKey = 6,
    BCMD_XptSecKeySlot_eTypeMax
}BCMD_XptSecKeySlot_e;

typedef enum BCMD_RootKeySrc_e
{
    BCMD_RootKeySrc_eCusKey = 0,
    BCMD_RootKeySrc_eOTPKeya = 1,
    BCMD_RootKeySrc_eOTPKeyb = 2,
    BCMD_RootKeySrc_eOTPKeyc = 3,
    BCMD_RootKeySrc_eOTPKeyd = 4,
    BCMD_RootKeySrc_eOTPKeye = 5,
    BCMD_RootKeySrc_eOTPKeyf = 6,
    BCMD_RootKeySrc_eOTPKeyg = 7,
    BCMD_RootKeySrc_eOTPKeyh = 8,
    BCMD_RootKeySrc_eOTPKeyMax = 8,
    BCMD_RootKeySrc_eReserved9  =  9,
    BCMD_RootKeySrc_eReserved10  =  10,
    BCMD_RootKeySrc_eReserved11  =  11,
    BCMD_RootKeySrc_eReserved0 = 9,
    BCMD_RootKeySrc_eReserved1 = 10,
    BCMD_RootKeySrc_eReserved2 = 11,
    BCMD_RootKeySrc_eASKMGlobalKey = 12,
    BCMD_RootKeySrc_eMax
}BCMD_RootKeySrc_e;

typedef enum BCMD_GenRootKeyDest_e
{




    BCMD_GenRootKeyDest_eReserved0 = 0,
    BCMD_GenRootKeyDest_eReserved1 = 1,
    BCMD_GenRootKeyDest_eReserved2 = 2,
    BCMD_GenRootKeyDest_eReserved3 = 3,
    BCMD_GenRootKeyDest_eMax
}BCMD_GenRootKeyDest_e;

typedef enum BCMD_VKLID_e
{
    BCMD_VKL0 = 0,
    BCMD_VKL1 = 1,
    BCMD_VKL2 = 2,
    BCMD_VKL3 = 3,
    BCMD_VKL4 = 4,
    BCMD_VKL5 = 5,
    BCMD_VKL6 = 6,
    BCMD_VKL7 = 7,
    BCMD_VKL_KeyRam_eMax = 8,
    BCMD_VKLID_eRPMBHmacKey = 0x14,
    BCMD_VKL_eMax
}BCMD_VKLID_e;

typedef enum BCMD_KeyRamBuf_e
{

    BCMD_KeyRamBuf_eKey3 = 3,
    BCMD_KeyRamBuf_eKey4 = 4,
    BCMD_KeyRamBuf_eKey5 = 5,
    BCMD_KeyRamBuf_eKey6 = 6,
    BCMD_KeyRamBuf_eKey7 = 7,

    BCMD_KeyRamBuf_eMax

}BCMD_KeyRamBuf_e;

typedef enum BCMD_SwizzleType_e
{
    BCMD_SwizzleType_eNoSwizzle = 0,
    BCMD_SwizzleType_eSwizzle1 = 1,
    BCMD_SwizzleType_eSwizzle0 = 2,
    BCMD_SwizzleType_eMax
}BCMD_SwizzleType_e;


typedef enum BCMD_OwnerIDSelectType_e
{
    BCMD_OwnerIDSelect_eMSP0 = 0,
    BCMD_OwnerIDSelect_eMSP1 = 1,
    BCMD_OwnerIDSelect_eReserved = 2,
    BCMD_OwnerIDSelect_eUse1 = 3,
    BCMD_OwnerIDSelect_eMax
}BCMD_OwnerIDSelectType_e;

typedef enum BCMD_KeyDestBlockType_e
{
    BCMD_KeyDestBlockType_eCPDescrambler = 0,
    BCMD_KeyDestBlockType_eCA = 1,
    BCMD_KeyDestBlockType_eCPScrambler = 2,

    BCMD_KeyDestBlockType_eReserved3  =  3,

    BCMD_KeyDestBlockType_eHdmi = 4,

    BCMD_KeyDestBlockType_eReserved5 = 5,

    BCMD_KeyDestBlockType_eReserved6 = 6,

    BCMD_KeyDestBlockType_eMax

} BCMD_KeyDestBlockType_e;

typedef enum BCMD_KeyDestEntryType_e
{
    BCMD_KeyDestEntryType_eOddKey = 0,
    BCMD_KeyDestEntryType_eEvenKey = 1,
    BCMD_KeyDestEntryType_eClearKey = 2,
    BCMD_KeyDestEntryType_eMax
} BCMD_KeyDestEntryType_e;

typedef enum BCMD_KeyDestIVType_e
{
    BCMD_KeyDestIVType_eNoIV = 0,
    BCMD_KeyDestIVType_eIV = 1,
    BCMD_KeyDestIVType_eAesShortIV = 2,
    BCMD_KeyDestIVType_eMax
} BCMD_KeyDestIVType_e;

typedef enum BCMD_XptM2MSecCryptoAlg_e
{
    BCMD_XptM2MSecCryptoAlg_eDVBCSA2 = 0,
    BCMD_XptM2MSecCryptoAlg_eMulti2 = 1,
    BCMD_XptM2MSecCryptoAlg_eDes = 2,
    BCMD_XptM2MSecCryptoAlg_e3DesAba = 3,
    BCMD_XptM2MSecCryptoAlg_e3DesAbc = 4,
    BCMD_XptM2MSecCryptoAlg_eDVBCSA3 = 5,
    BCMD_XptM2MSecCryptoAlg_eAes128 = 6,
    BCMD_XptM2MSecCryptoAlg_eAes192 = 7,
    BCMD_XptM2MSecCryptoAlg_eAesCounter0 = 8,
    BCMD_XptM2MSecCryptoAlg_eC2 = 9,
    BCMD_XptM2MSecCryptoAlg_eNotSuported = 10,
    BCMD_XptM2MSecCryptoAlg_eM6KE = 11,
    BCMD_XptM2MSecCryptoAlg_eM6 = 12,
    BCMD_XptM2MSecCryptoAlg_eRc4 = 13,
    BCMD_XptM2MSecCryptoAlg_eMSMULTISWAPMAC = 14,
    BCMD_XptM2MSecCryptoAlg_eWMDrmPd = 15,
    BCMD_XptM2MSecCryptoAlg_eAes128G = 16,
    BCMD_XptM2MSecCryptoAlg_eHdDVD = 17,
    BCMD_XptM2MSecCryptoAlg_eBrDVD = 18,
    BCMD_XptM2MSecCryptoAlg_eReserved19  =  19,
    BCMD_XptM2MSecCryptoAlg_eRESERVED19 = 19,
    BCMD_XptM2MSecCryptoAlg_eMax
}BCMD_XptM2MSecCryptoAlg_e;

typedef enum BCMD_KeySize_e
{
    BCMD_KeySize_e64 = 0,
    BCMD_KeySize_e128 = 1,
    BCMD_KeySize_e192 = 2,
    BCMD_KeySize_e256 = 3,
    BCMD_KeySize_eMax

}BCMD_KeySize_e;

typedef enum BCMD_KeyLadderType_e
{
    BCMD_KeyLadderType_e1DES = 0,
    BCMD_KeyLadderType_e3DESABA = 1,
    BCMD_KeyLadderType_eAES128 = 2,

    BCMD_KeyLadderType_eReserved3 = 3,
    BCMD_KeyLadderType_eMax = 4
}BCMD_KeyLadderType_e;
typedef enum BCMD_KeyLadderOp_e
{
    BCMD_KeyLadderOp_eDecrypt = 0,
    BCMD_KeyLadderOp_eEncrypt = 1,
    BCMD_KeyLadderOp_eMax
}BCMD_KeyLadderOp_e;

typedef enum BCMD_Key3Op_e
{
    BCMD_Key3Op_eKey3NoProcess = 0,
    BCMD_Key3Op_eKey3Export = 1,
    BCMD_Key3Op_eMax
}BCMD_Key3Op_e;

typedef enum BCMD_KeyMode_e
{
    BCMD_KeyMode_eRegular = 0,
    BCMD_KeyMode_eDes56 = 1,
    BCMD_KeyMode_eReserved2  =  2,
    BCMD_KeyMode_eReserved3  =  3,
    BCMD_KeyMode_eReserved0 = 2,
    BCMD_KeyMode_eReserved1 = 3,
    BCMD_KeyMode_eDvbConformance = 4,
    BCMD_KeyMode_eReserved5  =  5,
    BCMD_KeyMode_eRESERVED5 = 5,
    BCMD_KeyMode_eMax
}BCMD_KeyMode_e;

typedef enum BCMD_InvalidateKey_Flag_e
{
    BCMD_InvalidateKey_Flag_eSrcKeyOnly = 0,
    BCMD_InvalidateKey_Flag_eDestKeyOnly = 1,
    BCMD_InvalidateKey_Flag_eBoth = 2,
    BCMD_InvalidateKey_Flag_eMax
} BCMD_InvalidateKey_Flag_e;

typedef enum BCMD_TerminationMode_e
{
    BCMD_TerminationMode_eCLEAR = 0,

    BCMD_TerminationMode_eCOUNTERMODE0 = 0,
    BCMD_TerminationMode_eCOUNTERMODE1 = 1,
    BCMD_TerminationMode_eCOUNTERMODE2 = 2,
    BCMD_TerminationMode_eCOUNTERMODE3 = 3,
    BCMD_TerminationMode_eCOUNTERMODE4 = 4,

    BCMD_TerminationMode_eSCTE52_TERM = 1,
    BCMD_TerminationMode_eCTS_ECB_TERM = 2,
    BCMD_TerminationMode_eCTS_DVB_CPCM = 3,
    BCMD_TerminationMode_eFRONT_RESIDUE = 4,
    BCMD_TerminationMode_eMSC = 5,

    BCMD_TerminationMode_eReserved6 = 6,
    BCMD_TerminationMode_eTS_AND_PACKET = 7,

    BCMD_TerminationMode_ePACKET = 8,
    BCMD_TerminationMode_eCBCMAC = 9,
    BCMD_TerminationMode_eCMAC = 10,
    BCMD_TerminationMode_eMax
} BCMD_TerminationMode_e;

typedef enum BCMD_CipherModeSelect_e
{
    BCMD_CipherModeSelect_eECB = 0,
    BCMD_CipherModeSelect_eXptMode = 0,
    BCMD_CipherModeSelect_eCBC = 1,
    BCMD_CipherModeSelect_ePesMode = 1,
    BCMD_CipherModeSelect_eCTR = 2,
    BCMD_CipherModeSelect_eRCBC = 3,
    BCMD_CipherModeSelect_eMax
} BCMD_CipherModeSelect_e;

typedef enum BCMD_IVSelect_e
{
    BCMD_CounterSizeSelect_e32Bit = 0,
    BCMD_CounterSizeSelect_e64Bit = 1,
    BCMD_CounterSizeSelect_e96Bit = 2,
    BCMD_CounterSizeSelect_e128Bit = 3,

    BCMD_IVSelect_eRegular = 0,


    BCMD_IVSelect_eReserved1 = 1,
    BCMD_IVSelect_eReserved2 = 2,
    BCMD_IVSelect_eNO_PREPROC_WRITEBACK = 3,
    BCMD_IVSelect_eMax
} BCMD_IVSelect_e;

typedef enum BCMD_SolitarySelect_e
{
    BCMD_SolitarySelect_eCLEAR = 0,

    BCMD_SolitarySelect_eReserved1 = 1,
    BCMD_SolitarySelect_eIV1 = 2,
    BCMD_SolitarySelect_eIV2 = 3,
    BCMD_SolitarySelect_eDVB_CPCM = 4,
    BCMD_SolitarySelect_eMax
} BCMD_SolitarySelect_e;

typedef enum BCMD_ASKM_MaskKeySel_e
{
    BCMD_ASKM_MaskKeySel_eRealMaskKey = 0,

    BCMD_ASKM_MaskKeySel_eReserved1 = 1,
    BCMD_ASKM_MaskKeySel_eFixedMaskKey = 2,
    BCMD_ASKM_MaskKeySel_eMax
} BCMD_ASKM_MaskKeySel_e;

typedef enum BCMD_XptKeyTableCustomerMode_e
{
    BCMD_XptKeyTableCustomerMode_eGeneric = 0,

    BCMD_XptKeyTableCustomerMode_eReserved1 = 1,

    BCMD_XptKeyTableCustomerMode_eReserved2 = 2,
    BCMD_XptKeyTableCustomerMode_eReserved3  =  3,
    BCMD_XptKeyTableCustomerMode_eRESERVED3 = 3,
    BCMD_XptKeyTableCustomerMode_eMax
} BCMD_XptKeyTableCustomerMode_e;

typedef enum BSP_MscValues_e
{
    BSP_MscValues_eReserved2  =  2,
    BSP_MscValues_eReserved18  =  18,
    BSP_MscValues_eReserved4  =  4,

    BSP_MscValues_eReserved28  =  28,
    BSP_MscValues_eReserved12  =  12,
    BSP_MscValues_eMpegAesEcbForGeneric_0 = 4,
    BSP_MscValues_eMpegAesEcbForGeneric_1 = 4,
    BSP_MscValues_eMax
} BSP_MscValues_e;

#define  BCMD_CRYPTOALG_USEFWALG    0x1F

#define BCMD_DATA_OUTBUF        (6 << 2)

typedef enum BCMD_InitKeySlot_InCmdCfg_e
{
    BCMD_InitKeySlot_InCmdCfg_eSlotNumber = (5 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eConfigMulti2Slot = (5 << 2) + 0,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType0 = (5 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType1 = (6 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType2 = (7 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType3 = (8 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType4 = (9 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eSlotNumberSlotType5 = (10 << 2) + 3,
    BCMD_InitKeySlot_InCmdCfg_eMax
}BCMD_InitKeySlot_InCmdCfg_e;

typedef enum BCMD_InitKeySlot_OutCmdField_e
{
    BCMD_InitKeySlot_OutCmd_eMulti2SystemKeyConfig = (6 << 2) + 3,
    BCMD_InitKeySlot_OutCmd_eSlotNumber = (7 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType0 = (7 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType1 = (8 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType2 = (9 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType3 = (10 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType4 = (11 << 2) + 3,
    BCMD_InitKeySlot_OutCmdCfg_eSlotNumberSlotType5 = (12 << 2) + 3,
    BCMD_InitKeySlot_OutCmd_eMax
}BCMD_InitKeySlot_OutCmdField_e;




typedef enum BCMD_KeyPointer_InCmdCfg_e
{
    BCMD_KeyPointer_InCmdCfg_ePidChan = (5 << 2),
    BCMD_KeyPointer_InCmdCfg_eSlotType = (6 << 2) + 3,
    BCMD_KeyPointer_InCmdCfg_eSlotNumber = (7 << 2) + 3,
    BCMD_KeyPointer_InCmdCfg_eSlotTypeB = (8 << 2) + 3,
    BCMD_KeyPointer_InCmdCfg_eSlotNumberB = (9 << 2) + 3,
    BCMD_KeyPointer_InCmdCfg_eKeyPointerSel = (10 << 2) + 3,
    BCMD_KeyPointer_InCmdCfg_eSetMultiplePidChan = (11 << 2) + 1,

    BCMD_KeyPointer_InCmdCfg_ePidChanEnd  = (11 << 2) + 2,

    BCMD_KeyPointer_InCmdCfg_eMax
}BCMD_KeyPointer_InCmdCfg_e;

typedef enum BCMD_KeyPointer_OutCmdField_e
{
    BCMD_KeyPointer_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_KeyPointer_OutCmdField_eMax
}BCMD_KeyPointer_OutCmdField_e;

typedef enum BCMD_InCmdCfgMulti2_e
{
    BCMD_Multi2_InCmdCfg_eRoundCount = (5 << 2),
    BCMD_Multi2_InCmdCfg_eSystemKeys = (6 << 2),
    BCMD_Multi2_InCmdCfg_eWhichSysKey = (14 << 2) + 3,
    BCMD_Multi2_InCmdCfg_eMax
}BCMD_InCmdCfgMulti2_e;

typedef enum BCMD_Multi2_OutCmdField_e
{
    BCMD_Multi2_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_Multi2_OutCmdField_eMax
}BCMD_Multi2_OutCmdField_e;


typedef enum BCMD_GenKey_InCmd_e
{
    BCMD_GenKey_InCmd_eKeyLadderSelection = (5 << 2) + 0,
    BCMD_GenKey_InCmd_eASKM3DesKLRootKeySwapEnable = (5 << 2) + 1,
    BCMD_GenKey_InCmd_eVKLAssociationQuery = (5 << 2) + 2,
    BCMD_GenKey_InCmd_eKeyLadderType = (5 << 2) + 3,
    BCMD_GenKey_InCmd_eRootKeySrc = (6 << 2) + 3,
    BCMD_GenKey_InCmd_eCustomerSel = (6 << 2) + 2,
    BCMD_GenKey_InCmd_eASKMSel = (6 << 2) + 1,
    BCMD_GenKey_InCmd_eNoKeyGenFlag = (6 << 2) + 0,

    BCMD_GenKey_InCmd_eSwizzle1IndexSel = (7 << 2) + 3,
    BCMD_GenKey_InCmd_eSwizzleType = (8 << 2) + 3,
    BCMD_GenKey_InCmd_eCusKeySelL = (9 << 2) + 3,
    BCMD_GenKey_InCmd_eOwnerIDSelect = (9 << 2) + 2,
    BCMD_GenKey_InCmd_eASKMGlobalKeyIndex = (9 << 2) + 1,
    BCMD_GenKey_InCmd_eKey2GenVersion = (9 << 2) + 0,
    BCMD_GenKey_InCmd_eKeyVarL = (10 << 2) + 3,
    BCMD_GenKey_InCmd_eCusKeySelH = (11 << 2) + 3,
    BCMD_GenKey_InCmd_eKeyVarH = (12 << 2) + 3,
    BCMD_GenKey_InCmd_eVKLID = (13 << 2) + 3,
    BCMD_GenKey_InCmd_eSourceDuple = (13 << 2) + 2,
    BCMD_GenKey_InCmd_eKeyLayer = (14 << 2) + 3,
    BCMD_GenKey_InCmd_eKeyTweak = (14 << 2) + 2,
    BCMD_GenKey_InCmd_eApplyKKCV  =  (14<<2)+1,
    BCMD_GenKey_InCmd_eCwProtectionKeyIvSource = (14 << 2) + 0,
    BCMD_GenKey_InCmd_eKeyLadderOpera = (15 << 2) + 3,
    BCMD_GenKey_InCmd_eProcIn = (16 << 2),
    BCMD_GenKey_InCmd_eKeySize = (24 << 2) + 3,
    BCMD_GenKey_InCmd_eSwapAESKey = (25 << 2) + 2,
    BCMD_GenKey_InCmd_eRouteKeyFlag = (25 << 2) + 3,
    BCMD_GenKey_InCmd_eBlkType = (26 << 2) + 3,
    BCMD_GenKey_InCmd_eEntryType = (27 << 2) + 3,
    BCMD_GenKey_InCmd_eIVType = (27 << 2) + 2,
    BCMD_GenKey_InCmd_eKeySlotType = (28 << 2) + 3,
    BCMD_GenKey_InCmd_eKeySlotNumber = (29 << 2) + 3,
    BCMD_GenKey_InCmd_eHDMIKeyAddress = (29 << 2) + 2,
    BCMD_GenKey_InCmd_eSC01ModeWordMapping = (30 << 2) + 0,
    BCMD_GenKey_InCmd_eGPipeSC01Value = (30 << 2) + 1,
    BCMD_GenKey_InCmd_eRPipeSC01Value = (30 << 2) + 2,
    BCMD_GenKey_InCmd_eKeyMode = (30 << 2) + 3,
    BCMD_GenKey_InCmd_eCtrlWord0 = (31 << 2),
    BCMD_GenKey_InCmd_eCtrlWord1 = (32 << 2),
    BCMD_GenKey_InCmd_eCtrlWord2 = (33 << 2),
    BCMD_GenKey_InCmd_eCtrlWord3 = (34 << 2),
    BCMD_GenKey_InCmd_eCtrlWord4 = (35 << 2),
    BCMD_GenKey_InCmd_eCtrlWord5 = (36 << 2),
    BCMD_GenKey_InCmd_eSTBOwnerIDSel = (37 << 2) + 3,
	BCMD_GenKey_InCmd_eCAVendorIDExtension = (38 << 2) + 1,
    BCMD_GenKey_InCmd_eCAVendorID = (38 << 2) + 2,
    BCMD_GenKey_InCmd_eTestKeySel = (39 << 2) + 2,
    BCMD_GenKey_InCmd_eModuleID = (39 << 2) + 3,
    BCMD_GenKey_InCmd_eHwKlLength = (40 << 2) + 3,
    BCMD_GenKey_InCmd_eHwKlDestinationAlg = (40 << 2) + 2,
    BCMD_GenKey_InCmd_eReserved_40_1  =  (40<<2)+1,
    BCMD_GenKey_InCmd_eReserved_41_0  =  (41<<2),
    BCMD_GenKey_InCmd_eExtIVPtr = (45 << 2) + 2,
    BCMD_GenKey_InCmd_eReserved_46_3  =  (46<<2)+3,
    BCMD_GenKey_InCmd_eReserved_47_3  =  (47<<2)+3,
    BCMD_GenKey_InCmd_eReserved_48_2  =  (48<<2)+2,
    BCMD_GenKey_InCmd_eReservedDataForDSK = (49 << 2),
    BCMD_GenKey_InCmd_eMax
}BCMD_GenKey_InCmd_e;

typedef enum BCMD_GenKey_OutCmdField_e
{
    BCMD_GenKey_OutCmd_eReserved = (6 << 2),
    BCMD_GenKey_OutCmd_eVKLAllocation = (6 << 2) + 2,
    BCMD_GenKey_OutCmd_eVKLAssociation = (6 << 2) + 3,
    BCMD_GenKey_OutCmd_eMax
}BCMD_GenKey_OutCmdField_e;




typedef enum BCMD_SC01ModeWordMapping_e
{
    BCMD_SC01ModeWordMapping_eClear = 0,
    BCMD_SC01ModeWordMapping_eOdd = 1,
    BCMD_SC01ModeWordMapping_eEven = 2,
    BCMD_SC01ModeWordMapping_eMax
} BCMD_SC01ModeWordMapping_e;


typedef enum BCMD_KeyTweak_e
{
    BCMD_KeyTweak_eNoTweak = 0,
    BCMD_KeyTweak_eReserved1  =  1,
    BCMD_KeyTweak_eRESERVED1 = 1,
    BCMD_KeyTweak_eReserved2  =  2,
    BCMD_KeyTweak_eRESERVED2 = 2,
    BCMD_KeyTweak_eDSK = 3,
    BCMD_KeyTweak_eRESERVED3 = 3,
    BCMD_KeyTweak_eDupleConnect = 4,
    BCMD_KeyTweak_eRESERVED4 = 4,
    BCMD_KeyTweak_eMax
} BCMD_KeyTweak_e;

typedef enum BCMD_KeyGenFlag_e
{
    BCMD_KeyGenFlag_eGen = 0,
    BCMD_KeyGenFlag_eNoGen = 1,
    BCMD_KeyGenFlag_eMax
} BCMD_KeyGenFlag_e;

typedef enum BCMD_VKLAssociationQueryFlag_e
{
    BCMD_VKLAssociationQueryFlag_eNoQuery = 0,
    BCMD_VKLAssociationQueryFlag_eQuery = 1,
    BCMD_VKLAssociationQueryFlag_eMax
} BCMD_VKLAssociationQueryFlag_e;


typedef enum BCMD_ByPassKTS_e
{
    BCMD_ByPassKTS_eNoByPass = 0,
    BCMD_ByPassKTS_eFromRGToR = 1,
    BCMD_ByPassKTS_eFromGToRG = 2,
    BCMD_ByPassKTS_eMax
} BCMD_ByPassKTS_e;

typedef enum BCMD_KeyLadderSelection_e
{
    BCMD_eFWKL = 0,
    BCMD_eHWKL = 1,
    BCMD_KeyLadderSelection_eReserved2  =  2,
    BCMD_eRESERVED02 = 2,
    BCMD_KeyLadderSelection_eReserved3  =  3,
    BCMD_eRESERVED03 = 3,
    BCMD_KeyLadderSelection_eMax
}BCMD_KeyLadderSelection_e;


typedef enum BCMD_HwKeyLadderLength_e
{
    BCMD_HWKL_LEN0 = 0,
    BCMD_HWKL_LEN1 = 1,
    BCMD_HWKL_LEN2 = 2,
    BCMD_HWKL_LEN3 = 3,
    BCMD_HWKL_eMax
} BCMD_HwKeyLadderLength_e;


typedef enum BCMD_LoadUseKey_InCmd_e
{
    BCMD_LoadUseKey_InCmd_eKeyLength = (5 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eKeyData = (6 << 2),
    BCMD_LoadUseKey_InCmd_eRouteKeyFlag = (14 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eIsMiraCast = (15 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eBlkType = (15 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eEntryType = (16 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eIVType = (16 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eKeySlotType = (17 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eKeySlotNumber = (18 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eHDMIKeyAddress = (18 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eSC01ModeWordMapping = (19 << 2) + 0,
    BCMD_LoadUseKey_InCmd_eGPipeSC01Value = (19 << 2) + 1,
    BCMD_LoadUseKey_InCmd_eRPipeSC01Value = (19 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eKeyMode = (19 << 2) + 3,
    BCMD_LoadUseKey_InCmd_eCtrlWord0 = (20 << 2),
    BCMD_LoadUseKey_InCmd_eCtrlWord1 = (21 << 2),
    BCMD_LoadUseKey_InCmd_eCtrlWord2 = (22 << 2),
    BCMD_LoadUseKey_InCmd_eCtrlWord3 = (23 << 2),
    BCMD_LoadUseKey_InCmd_eCtrlWord4 = (24 << 2),
    BCMD_LoadUseKey_InCmd_eCtrlWord5 = (25 << 2),
    BCMD_LoadUseKey_InCmd_eExtKeyPtr = (26 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eExtIVPtr = (27 << 2) + 2,
    BCMD_LoadUseKey_InCmd_eMax
}BCMD_LoadUseKey_InCmd_e;

typedef enum BCMD_LoadUseKey_OutCmdField_e
{
    BCMD_LoadUseKey_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_LoadUseKey_OutCmdField_eMax
}BCMD_LoadUseKey_OutCmdField_e;

typedef enum BCMD_InvalidateKey_InCmd_e
{
    BCMD_InvalidateKey_InCmd_eKeyFlag = (5 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eIsAllKTS = (5 << 2) + 2,
    BCMD_InvalidateKey_InCmd_eSetByPassKTS = (5 << 2) + 1,
    BCMD_InvalidateKey_InCmd_eKeyLayer = (6 << 2) + 2,
    BCMD_InvalidateKey_InCmd_eVKLID = (6 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eAllKeyLayer = (6 << 2) + 1,
    BCMD_InvalidateKey_InCmd_eFreeVKLOwnerShip = (6 << 2) + 0,
    BCMD_InvalidateKey_InCmd_eBlkType = (7 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eSC01ModeWordMapping = (8 << 2) + 2,
    BCMD_InvalidateKey_InCmd_eEntryType = (8 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eKeySlotType = (9 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eKeySlotNumber = (10 << 2) + 3,
    BCMD_InvalidateKey_InCmd_eMax
}BCMD_InvalidateKey_InCmd_e;

typedef enum BCMD_InvalidateKey_OutCmdField_e
{
    BCMD_InvalidateKey_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_InvalidateKey_OutCmdField_eMax
}BCMD_InvalidateKey_OutCmdField_e;



typedef enum BCMD_AllocateKeySlot_InCmd_e
{
    BCMD_AllocateKeySlot_InCmd_eKeySlotType = (5 << 2) + 3,
    BCMD_AllocateKeySlot_InCmd_eKeySlotNumber = (5 << 2) + 2,
    BCMD_AllocateKeySlot_InCmd_eSetKeySlotOwnership = (5 << 2) + 1,
    BCMD_AllocateKeySlot_InCmd_eOwnershipAction = (6 << 2) + 3,
    BCMD_AllocateKeySlot_InCmd_eMax
}BCMD_AllocateKeySlot_InCmd_e;

typedef enum BCMD_AllocateKeySlot_SubCmd_e
{
    BCMD_HostSage_KeySlotAction_SubCmd_eSetOwnership = 0,
    BCMD_HostSage_KeySlotAction_SubCmd_eOwnership_Query = 1,
    BCMD_HostSage_KeySlotAction_SubCmd_eMax
}BCMD_AllocateKeySlot_SubCmd_e;

typedef enum BCMD_HostSage_KeySlotAllocation_e
{
    BCMD_HostSage_KeySlotAllocation_eFREE = 0,
    BCMD_HostSage_KeySlotAllocation_eSAGE = 1,
    BCMD_HostSage_KeySlotAllocation_eReserved1 = 1,
    BCMD_HostSage_KeySlotAllocation_eSHARED = 2,
    BCMD_HostSage_KeySlotAllocation_eMax = 2
} BCMD_HostSage_KeySlotAllocation_e;


typedef enum BCMD_AllocateKeySlot_OutCmd_e
{
    BCMD_AllocateKeySlot_OutCmd_eStatus = (5 << 2) + 3,
    BCMD_AllocateKeySlot_OutCmd_eKeySlotOwnership = (6 << 2) + 3,
    BCMD_AllocateKeySlot_OutCmd_eMax
}BCMD_AllocateKeySlot_OutCmd_e;

typedef enum BCMD_KeySlotStatusQuery_InCmd_e
{
    BCMD_KeySlotStatusQuery_InCmd_eMax
}BCMD_KeySlotStatusQuery_InCmd_e;


typedef enum BCMD_KeySlotStatusQuery_OutCmd_e
{
    BCMD_KeySlotStatusQuery_OutCmd_eStatus = (5 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType0 = (6 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eMulti2SystemKeyConfig = (6 << 2) + 2,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType1 = (7 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType2 = (8 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType3 = (9 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType4 = (10 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eSlotNumberSlotType5 = (11 << 2) + 3,
    BCMD_KeySlotStatusQuery_OutCmd_eKeySlotOwnership = (12 << 2),
    BCMD_KeySlotStatusQuery_OutCmd_eMax
}BCMD_KeySlotStatusQuery_OutCmd_e;

typedef enum BCMD_ASKM_KEY_GEN_OutCmdField_e
{
    BCMD_ASKM_KEY_GEN_OutCmd_eZeroDetectDump = (6 << 2),
    BCMD_ASKM_KEY_GEN_OutCmd_eMax
}BCMD_ASKM_KEY_GEN_OutCmdField_e;

typedef enum BCMD_KLAD_Challenge_InputCommandField_e
{
    BCMD_KLAD_Challenge_InputCommandField_eId = (5 << 2) + 3,
    BCMD_KLAD_Challenge_InputCommandField_eMax
}BCMD_KLAD_Challenge_InputCommandField_e;

typedef enum BCMD_KLAD_Challenge_OutCmdField_e
{
    BCMD_KLAD_Challenge_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_KLAD_Challenge_OutCmdField_eIdHi = (6 << 2) + 0,
    BCMD_KLAD_Challenge_OutCmdField_eIdLo = (7 << 2) + 0,
    BCMD_KLAD_Challenge_OutCmdField_eBlackBoxID = (8 << 2) + 3,
    BCMD_KLAD_Challenge_OutCmdField_eSTBOwnerID = (9 << 2) + 0,
    BCMD_KLAD_Challenge_OutCmdField_eMax
}BCMD_KLAD_Challenge_OutCmdField_e;

typedef enum BCMD_KLAD_Response_InCmdField_e
{
    BCMD_KLAD_Response_InCmdField_eKLADMode = (5 << 2) + 3,
    BCMD_KLAD_Response_InCmd_eVKLID = (6 << 2) + 3,
    BCMD_KLAD_Response_InCmd_eKeyLayer = (7 << 2) + 3,
    BCMD_KLAD_Response_InCmdField_eNonce = (8 << 2) + 0,
    BCMD_KLAD_Response_InCmdField_eMax
}BCMD_KLAD_Response_InCmdField_e;

typedef enum BCMD_KLAD_Response_OutCmdField_e
{
    BCMD_KLAD_Response_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_KLAD_Response_OutCmdField_eResponse = (6 << 2) + 0,
    BCMD_KLAD_Response_OutCmdField_eMax
}BCMD_KLAD_Response_OutCmdField_e;







typedef enum BCMD_RPMB_KEYGEN_InCmd_e
{
    BCMD_RPMB_KEYGEN_eKeyLadderType = (5 << 2) + 2,
    BCMD_RPMB_KEYGEN_eVKLID     = (5 << 2) + 3,
    BCMD_RPMB_KEYGEN_eMax
}BCMD_RPMB_KEYGEN_InCmd_e;

typedef enum BCMD_RPMB_KEYGEN_OutCmdField_e
{
    BCMD_RPMB_KEYGEN_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_RPMB_KEYGEN_OutCmdField_eKey    = (6 << 2) + 0,
    BCMD_RPMB_KEYGEN_OutCmdField_eMax
}BCMD_RPMB_KEYGEN_OutCmdField_e;



typedef enum BCMD_DISABLE_RPMB_HMAC_InCmd_e
{
    BCMD_DISABLE_RPMB_HMAC_eMax
}BCMD_DISABLE_RPMB_HMAC_InCmd_e;

typedef enum BCMD_DISABLE_RPMB_HMAC_OutCmdField_e
{
    BCMD_DISABLE_RPMB_HMAC_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_DISABLE_RPMB_HMAC_OutCmdField_eMax
}BCMD_DISABLE_RPMB_HMAC_OutCmdField_e;




typedef enum BCMD_RPMB_ProgUseKey_InCmd_e
{
    BCMD_RPMB_ProgUseKey_eRandomConstant            = (5 << 2) + 0,
    BCMD_RPMB_ProgUseKey_eDesiredRPMBUseKeyModBits  = (9 << 2) + 3,
    BCMD_RPMB_ProgUseKey_eCurrentRPMBUseKeyModBits  = (10 << 2) + 3,
    BCMD_RPMB_ProgUseKey_InCmdField_eRPMBUseKeyModBitsMask = (11 << 2) + 3,
    BCMD_RPMB_ProgUseKey_eSignature = (12 << 2) + 0,
    BCMD_RPMB_ProgUseKey_InCmdField_eVKLId = (20 << 2) + 3,
    BCMD_RPMB_ProgUseKey_InCmdField_eKeyLayer = (21 << 2) + 3,
    BCMD_RPMB_ProgUseKey_InCmd_eMax
}BCMD_RPMB_ProgUseKey_InCmd_e;


typedef enum BCMD_RPMB_ProgUseKey_OutCmdField_e
{
    BCMD_RPMB_ProgUseKey_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_RPMB_ProgUseKey_OutCmdField_eMax
}BCMD_RPMB_ProgUseKey_OutCmdField_e;

#endif
