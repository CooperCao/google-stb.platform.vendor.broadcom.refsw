/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/

#ifndef ZEUS_VERSION
#warning "ZEUS_VERSION is not defined under /cygdrive/f/zeus_src_65nm/zeus_src/aegis/DownloadCode/Common/share/bsp_s_otp.h"
#endif



#ifndef BSP_S_OTP_H__
#define BSP_S_OTP_H__



typedef enum BCMD_Otp_InCmdOfflineOtpRead_e
{
    BCMD_Otp_InCmdOfflineOtpRead_eEnum                = (5 << 2)+3,
    BCMD_Otp_InCmdOfflineOtpRead_eKeyType             = (5 << 2)+2,
    BCMD_Otp_InCmdOfflineOtpRead_eMax
}BCMD_Otp_InCmdOfflineOtpRead_e;

typedef enum BCMD_Otp_OutCmdOfflineOtpRead_e
{
    BCMD_Otp_OutCmdOfflineOtpRead_eRegValueLo         = (6 << 2)+0,
    BCMD_Otp_OutCmdOfflineOtpRead_eRegValueHi         = (7 << 2)+0,
    BCMD_Otp_OutCmdOfflineOtpRead_eRegValue2          = (8 << 2)+0,
    BCMD_Otp_OutCmdOfflineOtpRead_eRegValue3          = (9 << 2)+0,
    BCMD_Otp_OutCmdOfflineOtpRead_eMax
}BCMD_Otp_OutCmdOfflineOtpRead_e;


typedef enum BCMD_Otp_InCmdOfflineProgMsp_e
{
    BCMD_Otp_InCmdOfflineProgMsp_eReserved_5_3  =  (5<<2)+3,
    BCMD_Otp_InCmdOfflineProgMsp_eReserved_5_2  =  (5<<2)+2,
    BCMD_Otp_InCmdOfflineProgMsp_eMode32              = ( 5 << 2)+0,

    BCMD_Otp_InCmdOfflineProgMsp_eEnum                = ( 6 << 2)+2,
    BCMD_Otp_InCmdOfflineProgMsp_eNumOfBits           = ( 7 << 2)+3,
    BCMD_Otp_InCmdOfflineProgMsp_eMask                = ( 8 << 2)+0,
    BCMD_Otp_InCmdOfflineProgMsp_eData                = ( 9 << 2)+0,
    BCMD_Otp_InCmdOfflineProgMsp_eMax
}BCMD_Otp_InCmdOfflineProgMsp_e;

typedef enum BCMD_Otp_OutCmdOfflineProgMsp_e
{
    BCMD_Otp_OutCmdOfflineProgMsp_eReserved           = ( 6 << 2)+0,
    BCMD_Otp_OutCmdOfflineProgMsp_eMax
}BCMD_Otp_OutCmdOfflineProgMsp_e;

#define BCMD_OTP_PROGMSP_MODE   0x00010112UL


typedef enum BCMD_Otp_InCmdOfflineMspRead_e
{
    BCMD_Otp_InCmdOfflineMspRead_eEnum                = ( 5<< 2)+ 2,
    BCMD_Otp_InCmdOfflineMspRead_eMax

}BCMD_Otp_InCmdOfflineMspRead_e;

typedef enum BCMD_Otp_OutCmdOfflineMspRead_e
{
    BCMD_Otp_OutCmdOfflineMspRead_eMspValue           = (6 << 2)+0,
    BCMD_Otp_OutCmdOfflineMspRead_eMspLockValue       = (7 << 2)+0,
    BCMD_Otp_OutCmdOfflineMspRead_eMax
}BCMD_Otp_OutCmdOfflineMspRead_e;



typedef enum BCMD_Otp_InCmdOtpReadMSP32_e
{
    BCMD_Otp_InCmdOtpReadMSP32_eMspGroupType       = ( 5<< 2)+ 3,
    BCMD_Otp_InCmdOtpReadMSP32_eStartGroup         = ( 6<< 2)+ 3,
    BCMD_Otp_InCmdOtpReadMSP32_eRange              = ( 7<< 2)+ 3,
    BCMD_Otp_InCmdOtpReadMSP32_eMax

}BCMD_Otp_InCmdOtpReadMSP32_e;

typedef enum BCMD_Otp_OutCmdOtpReadMSP32_e
{
    BCMD_Otp_OutCmdOtpReadMSP32_eMspValueStartAddr = (6 << 2)+0,
    BCMD_Otp_OutCmdOtpReadMSP32_eMax
}BCMD_Otp_OutCmdOtpReadMSP32_e;



typedef enum BCMD_Otp_InCmdDataSectionRead_e
{
    BCMD_Otp_InCmdDataSectionRead_eEnum               = (5 << 2)+3,
    BCMD_Otp_InCmdDataSectionRead_eMax

}BCMD_Otp_InCmdDataSectionRead_e;

typedef enum BCMD_Otp_IntermedDataSectionRead_e
{
    BCMD_Otp_IntermedDataSectionRead_eData             = (8 << 2),
    BCMD_Otp_IntermedDataSectionRead_eMax

}BCMD_Otp_IntermedDataSectionRead_e;

typedef enum BCMD_Otp_OutCmdDataSectionRead_e
{
    BCMD_Otp_OutCmdDataSectionRead_eArrayData         = (6 << 2)+0,
    BCMD_Otp_OutCmdDataSectionRead_eMax

}BCMD_Otp_OutCmdDataSectionRead_e;


typedef enum BCMD_Otp_InCmdDataSectionProg_e
{
    BCMD_Otp_InCmdDataSectionProg_eEnum             = ( 5 << 2)+3,
    BCMD_Otp_InCmdDataSectionProg_eArrayData        = ( 6 << 2)+0,

    BCMD_Otp_InCmdDataSectionProg_eReserved_14_3  =  (14<<2)+3,
    BCMD_Otp_InCmdDataSectionProg_eReserved_14_2  =  (14<<2)+2,
    BCMD_Otp_InCmdDataSectionProg_eReserved_14_1  =  (14<<2)+1,
    BCMD_Otp_InCmdDataSectionProg_eReserved0        = (14 << 2)+3,
    BCMD_Otp_InCmdDataSectionProg_eReserved1        = (14 << 2)+2,
    BCMD_Otp_InCmdDataSectionProg_eReserved2        = (14 << 2)+1,
    BCMD_Otp_InCmdDataSectionProg_eMode32           = (14 << 2)+0,

    BCMD_Otp_InCmdDataSectionProg_eCrc              = (15 << 2)+0,
    BCMD_Otp_InCmdDataSectionProg_eMax

}BCMD_Otp_InCmdDataSectionProg_e;

typedef enum BCMD_Otp_OutCmdDataSectionProg_e
{
    BCMD_Otp_OutCmdDataSectionProg_eReserved          = (6 << 2)+0,
    BCMD_Otp_OutCmdDataSectionProg_eMax
}BCMD_Otp_OutCmdDataSectionProg_e;

#define BCMD_OTP_DATASECTIONPROG_MODE   0x00010112UL




typedef enum BCMD_Otp_InStatusCmd_e
{
    BCMD_Otp_InCmdOffline_eNotUsedIndex             = (5 << 0),
    BCMD_Otp_InCmdOffline_eMax
}BCMD_Otp_InStatusCmd_e;

typedef enum BCMD_Otp_OutCmdStatus_e
{
    BCMD_Otp_OutCmdStatus_eSignalRead               = ( 6 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStat                  = ( 7 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatMc0S              = ( 8 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyA              = ( 9 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyB              = (10 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyC              = (11 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyD              = (12 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyE              = (13 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyF              = (14 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyG              = (15 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatKeyH              = (16 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatHash              = (17 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatHash2             = (18 << 2)+0,
    BCMD_Otp_OutCmdStatus_eFltStatDS                = (19 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_20_0  =  (20<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_21_0  =  (21<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_22_0  =  (22<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_23_0  =  (23<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_24_0  =  (24<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_25_0  =  (25<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_26_0  =  (26<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_27_0  =  (27<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_28_0  =  (28<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_29_0  =  (29<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_30_0  =  (30<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_31_0  =  (31<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_32_0  =  (32<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved20               = (20 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved21               = (21 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved22               = (22 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved23               = (23 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved24               = (24 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved25               = (25 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved26               = (26 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved27               = (27 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved28               = (28 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved29               = (29 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved30               = (30 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved31               = (31 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved32               = (32 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_33_0  =  (33<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_34_0  =  (34<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_35_0  =  (35<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_36_0  =  (36<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_37_0  =  (37<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_38_0  =  (38<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_39_0  =  (39<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_40_0  =  (40<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_41_0  =  (41<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved33               = (33 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved34               = (34 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved35               = (35 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved36               = (36 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved37               = (37 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved38               = (38 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved39               = (39 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved40               = (40 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved41               = (41 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_42_0  =  (42<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_43_0  =  (43<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_44_0  =  (44<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_45_0  =  (45<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_46_0  =  (46<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_47_0  =  (47<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_48_0  =  (48<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_49_0  =  (49<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved_50_0  =  (50<<2)+0,
    BCMD_Otp_OutCmdStatus_eReserved42               = (42 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved43               = (43 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved44               = (44 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved45               = (45 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved46               = (46 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved47               = (47 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved48               = (48 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved49               = (49 << 2)+0,
    BCMD_Otp_OutCmdStatus_eReserved50               = (50 << 2)+0,
	BCMD_Otp_OutCmdStatus_eMax                      = (51 << 2)+0
}BCMD_Otp_OutCmdStatus_e;






typedef enum BCMD_Otp_InCmdOtpProgPatternSequence_e
{
    BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray  = (5 << 2) + 0,
    BCMD_Otp_InCmdOtpProgPatternSequence_eMax
} BCMD_Otp_InCmdOtpProgPatternSequence_e;



typedef enum BCMD_Otp_InCmdCounterOp_e
{
    BCMD_Otp_InCmdCounterOp_ePartition                 = (5 << 2)+3,
    BCMD_Otp_InCmdCounterOp_eOperation                 = (6 << 2)+3,
    BCMD_Otp_InCmdCounterOp_eFlag                      = (7 << 2)+3,
    BCMD_Otp_InCmdCounterOp_eMax
}BCMD_Otp_InCmdCounterOp_e;

typedef enum BCMD_Otp_OutCmdCounterOp_e
{
    BCMD_Otp_OutCmdCounterOp_eCounterValue             = (6 << 2)+0,
    BCMD_Otp_OutCmdCounterOp_eMax
}BCMD_Otp_OutCmdCounterOp_e;




typedef enum BPI_Otp_CmdReadRegister_e
{
    BPI_Otp_CmdReadRegister_eReserved0  =  0,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved0                         = 0,

    BPI_Otp_CmdReadRegister_eReserved1  =  1,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved1                         = 1,

#if (ZEUS_VERSION < ZEUS_4_2)
    BPI_Otp_CmdReadRegister_eReserved2  =  2,
#endif
    BPI_Otp_CmdReadRegister_eMc0S_Reserved2                         = 2,

    BPI_Otp_CmdReadRegister_eMc0S_Reserved3                         = 3,

    BPI_Otp_CmdReadRegister_eReserved4  =  4,
    BPI_Otp_CmdReadRegister_eReserved5  =  5,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved4                         = 4,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved5                         = 5,

    BPI_Otp_CmdReadRegister_eKeyMc0_CustomerMode                    = 6,

    BPI_Otp_CmdReadRegister_eReserved7  =  7,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved7                         = 7,

    BPI_Otp_CmdReadRegister_eKeyMc0_DeobfuscationEnable             = 8,
    BPI_Otp_CmdReadRegister_eKeyMc0_BlackBoxId                      = 9,
    BPI_Otp_CmdReadRegister_eKeyMc0_CaKeyLadderDisallow             = 10,
    BPI_Otp_CmdReadRegister_eKeyMc0_CpKeyLadderDisallow             = 11,
    BPI_Otp_CmdReadRegister_eKeyMc0_Gp1KeyLadderDisallow            = 12,
    BPI_Otp_CmdReadRegister_eKeyMc0_Gp2KeyLadderDisallow            = 13,

    BPI_Otp_CmdReadRegister_eReserved14  =  14,
    BPI_Otp_CmdReadRegister_eKeyMc0_Reserved14                      = 14,

    BPI_Otp_CmdReadRegister_eKeyID                                  = 15,
    BPI_Otp_CmdReadRegister_eKeyHash                                = 16,
    BPI_Otp_CmdReadRegister_eBseckHashCrc                           = 17,

    BPI_Otp_CmdReadRegister_eReserved18  =  18,
    BPI_Otp_CmdReadRegister_eMc0S_Reserved18                        = 18,

    BPI_Otp_CmdReadRegister_eMc0S_Reserved19                        = 19,

    BPI_Otp_CmdReadRegister_eKeyMc0_RootKeySwapDisallow             = 20,

    BPI_Otp_CmdReadRegister_eReserved21  =  21,
    BPI_Otp_CmdReadRegister_eKeyMc0_Reserved21                      = 21,

    BPI_Otp_CmdReadRegister_eStage2BseckHashCrc                     = 22,

    BPI_Otp_CmdReadRegister_eReserved23  =  23,
    BPI_Otp_CmdReadRegister_eKeyMc0_Reserved23                      = 23,

    BPI_Otp_CmdReadRegister_eReserved24  =  24,
    BPI_Otp_CmdReadRegister_eKeyMc0_Reserved24                      = 24,

    BPI_Otp_CmdReadRegister_eKeyMc0_SageKeyLadderDisallow           = 25,

    BPI_Otp_CmdReadRegister_eSize
} BPI_Otp_CmdReadRegister_e ;

typedef enum BPI_Otp_KeyType_e
{
    BPI_Otp_KeyType_eA = 0,
    BPI_Otp_KeyType_eB = 1,
    BPI_Otp_KeyType_eC = 2,
    BPI_Otp_KeyType_eD = 3,
    BPI_Otp_KeyType_eE = 4,
    BPI_Otp_KeyType_eF = 5,
    BPI_Otp_KeyType_eG = 6,
    BPI_Otp_KeyType_eH = 7,
    BPI_Otp_KeyType_eSize
} BPI_Otp_KeyType_e ;

typedef enum BPI_Otp_RolloverCounterPartition_e
{
    BPI_Otp_RolloverCounterPartition_e0 = 0,
    BPI_Otp_RolloverCounterPartition_e1 = 1,
    BPI_Otp_RolloverCounterPartition_e2 = 2,
    BPI_Otp_RolloverCounterPartition_eMax
} BPI_Otp_RolloverCounterPartition_e;

typedef enum BPI_Otp_RolloverCounterOp_e
{
    BPI_Otp_RolloverCounterOp_eRead = 0,
    BPI_Otp_RolloverCounterOp_eIncrement = 1,
    BPI_Otp_RolloverCounterOp_eMax
} BPI_Otp_RolloverCounterOp_e;

typedef enum BPI_Otp_RolloverCounterFlag_e
{
    BPI_Otp_RolloverCounterFlag_eHost	= 0,

    BPI_Otp_RolloverCounterFlag_eReserved1	= 1,
    BPI_Otp_RolloverCounterFlag_eMax
} BPI_Otp_RolloverCounterFlag_e;


typedef enum BPI_Otp_MspGroupType_e
{
    BPI_Otp_BPI_Otp_MspGroupType_eData = 0,
    BPI_Otp_BPI_Otp_MspGroupType_eLock = 1,
    BPI_Otp_BPI_Otp_MspGroupType_eMax
} BPI_Otp_MspGroupType_e;

#define BPI_OTP_PROGRAM_MSP_NUMBER_BITS_TO_PROG_MAX     32

typedef enum BPI_Otp_HalProgKey_e
{
    BPI_Otp_HalProgKey_eMc0_S       = 0,
    BPI_Otp_HalProgKey_eA           = 1,
    BPI_Otp_HalProgKey_eB           = 2,
    BPI_Otp_HalProgKey_eC           = 3,
    BPI_Otp_HalProgKey_eD           = 4,
    BPI_Otp_HalProgKey_eE           = 5,
    BPI_Otp_HalProgKey_eF           = 6,
    BPI_Otp_HalProgKey_eG           = 7,
    BPI_Otp_HalProgKey_eH           = 8,
    BPI_Otp_HalProgKey_eHash        = 9,
    BPI_Otp_HalProgKey_eStage2Hash  = 10,









    BPI_Otp_HalProgKey_eReserved11  = 11,
    BPI_Otp_HalProgKey_eReserved12  = 12,
    BPI_Otp_HalProgKey_eReserved13  = 13,
    BPI_Otp_HalProgKey_eReserved14  = 14,
    BPI_Otp_HalProgKey_eReserved15  = 15,
    BPI_Otp_HalProgKey_eReserved16  = 16,
    BPI_Otp_HalProgKey_eReserved17  = 17,
    BPI_Otp_HalProgKey_eReserved18  = 18,
    BPI_Otp_HalProgKey_eReserved19  = 19,
    BPI_Otp_HalProgKey_eSize
} BPI_Otp_HalProgKey_e ;


typedef enum BPI_Otp_DataSection_e
{
    BPI_Otp_DataSection_e0 = 0x0,
    BPI_Otp_DataSection_e1 = 0x1,
    BPI_Otp_DataSection_e2 = 0x2,
    BPI_Otp_DataSection_e3 = 0x3,
    BPI_Otp_DataSection_e4 = 0x4,
    BPI_Otp_DataSection_e5 = 0x5,
    BPI_Otp_DataSection_e6 = 0x6,
    BPI_Otp_DataSection_e7 = 0x7,
    BPI_Otp_DataSection_e_size = 0x8
} BPI_Otp_DataSection_e ;

#endif
