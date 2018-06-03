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

#ifndef BSP_S_MISC_H__
#define BSP_S_MISC_H__



typedef enum BCMD_SetMiscBitsSubCmd_e
{
    BCMD_SetMiscBitsSubCmd_eRaveBits = 0,



    BCMD_SetMiscBitsSubCmd_eReserved1 = 1,
    BCMD_SetMiscBitsSubCmd_eReserved2 = 2,
    BCMD_SetMiscBitsSubCmd_eM2MEndianSwapBits = 3,
    BCMD_SetMiscBitsSubCmd_eForceSCBit = 4,
    BCMD_SetMiscBitsSubCmd_eCPSRaveTimestampDis = 5,
    BCMD_SetMiscBitsSubCmd_eXPTM2MStatusDump = 6,
    BCMD_SetMiscBitsSubCmd_eXPTPipesBypass = 7,
    BCMD_SetMiscBitsSubCmd_eXPTReset = 8,
    BCMD_SetMiscBitsSubCmd_MACRead = 9,
    BCMD_SetMiscBitsSubCmd_LockMemcSize = 10,
    BCMD_SetMiscBitsSubCmd_eReserved11  =  11,
    BCMD_SetMiscBitsSubCmd_eReserved12  =  12,
    BCMD_SetMiscBitsSubCmd_eReserved13  =  13,
    BCMD_SetMiscBitsSubCmd_eReserved14  =  14,
    BCMD_SetMiscBitsSubCmd_eMax
}BCMD_SetMiscBitsSubCmd_e;





typedef enum BCMD_XPT_BandSel_e
{
    BCMD_XPT_BandSel_eInBand = 0,
    BCMD_XPT_BandSel_ePlayBack = 1,
    BCMD_XPT_BandSel_eMax
}BCMD_XPT_BandSel_e;

typedef enum BCMD_MISC_RaveCtrlBitsMask_e
{
    BCMD_MISC_RaveCtrlBitsMask_eIMEM_WR = 0x01,
    BCMD_MISC_RaveCtrlBitsMask_eIMEM_RD = 0x02,
    BCMD_MISC_RaveCtrlBitsMask_eDMEM_RD = 0x04,
    BCMD_MISC_RaveCtrlBitsMask_eRESERVED = 0x08,
    BCMD_MISC_RaveCtrlBitsMask_eDISABLE_CLR = 0x10,
    BCMD_MISC_RaveCtrlBitsMask_eMax
}BCMD_MISC_RaveCtrlBitsMask_e;


typedef enum BCMD_SetMiscBitsField_InCmdField_e
{
    BCMD_SetMiscBitsField_InCmdField_eSubCommandId = (5 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eAonWord = (6 << 2),
    BCMD_SetMiscBitsField_InCmdField_eCtrlBits = (6 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eBandSel = (6 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eMemcIdx = (6 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eBandNum = (6 << 2) + 2,
    BCMD_SetMiscBitsField_InCmd_Field_eGenInBits = (6 << 2) + 2,
    BCMD_SetMiscBitsField_InCmdField_eCPSRaveTimestampDis = (6 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eForceSCbits = (7 << 2) + 3,
    BCMD_SetMiscBitsField_InCmdField_eMax
} BCMD_SetMiscBitsField_InCmdField_e;

typedef enum BCMD_SetMiscBitsField_OutCmdField_e
{
    BCMD_SetMiscBitsField_OutCmdField_eRaveStatusBit = (6 << 2) + 3,
    BCMD_SetMiscBitsField_OutCmdField_eReserved_6_0  =  (6<<2),
    BCMD_SetMiscBitsField_OutCmdField_eM2MStatusBits = (6 << 2),
    BCMD_SetMiscBitsField_OutCmdField_eXPTStatusBits = (7 << 2),
    BCMD_SetMiscBitsField_OutCmdField_eXPTCheckerViolationAddress = (8 << 2),
    BCMD_SetMiscBitsField_OutCmdField_eXPTCheckerViolationIDs = (9 << 2),
    BCMD_SetMiscBitsField_OutCmdField_eMax
} BCMD_SetMiscBitsField_OutCmdField_e;




typedef enum BCMD_PCIE_Window_SizeField_InCmdField_e
{
    BCMD_PCIE_Window_SizeField_InCmdField_ePayloadStart = (5 << 2),
    BCMD_PCIE_Window_SizeField_InCmdField_ePCIESel = (5 << 2) + 3,
    BCMD_PCIE_Window_SizeField_InCmdField_eMaxWinSize0 = (6 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eMaxWinSize1 = (7 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eMaxWinSize2 = (8 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eMarketID = (9 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eMarketIDMask = (10 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eEpoch = (11 << 2) + 3,
    BCMD_PCIE_Window_SizeField_InCmdField_eEpochMask = (11 << 2) + 2,
    BCMD_PCIE_Window_SizeField_InCmdField_eEpochSel = (11 << 2) + 1,
    BCMD_PCIE_Window_SizeField_InCmdField_eSigVer = (12 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eSigType = (12 << 2) + 1,
    BCMD_PCIE_Window_SizeField_InCmdField_eSigStart = (13 << 2) + 0,
    BCMD_PCIE_Window_SizeField_InCmdField_eVKLId = (21 << 2) + 3,
    BCMD_PCIE_Window_SizeField_InCmdField_eKeyLayer = (21 << 2) + 2,
    BCMD_PCIE_Window_SizeField_InCmdField_eMax
} BCMD_PCIE_Window_SizeField_InCmdField_e;

typedef enum BCMD_PCIE_Window_SizeField_OutCmdField_e
{
    BCMD_PCIE_Window_SizeField_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_PCIE_Window_SizeField_OutCmdField_eMax
}BCMD_PCIE_Window_SizeField_OutCmdField_e;






typedef enum BCMD_MISC_SetArchField_InCmdField_e
{
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeLowMsb = (5 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeLow = (6 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeHighMsb = (7 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeHigh = (8 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eDramSel = (9 << 2) + 1,
    BCMD_MISC_SetArchField_InCmdField_ePCIWin = (9 << 2) + 2,
    BCMD_MISC_SetArchField_InCmdField_eArchSel = (9 << 2) + 3,
    BCMD_MISC_SetArchField_InCmdField_eReserved_10_2  =  (10<<2)+2,
    BCMD_MISC_SetArchField_InCmdField_eReserved_10_3  =  (10<<2)+3,
    BCMD_MISC_SetArchField_InCmdField_eReserved_11_0  =  (11<<2),
    BCMD_MISC_SetArchField_InCmdField_eReserved_11_1  =  (11<<2)+1,
    BCMD_MISC_SetArchField_InCmdField_eReserved_11_2  =  (11<<2)+2,
    BCMD_MISC_SetArchField_InCmdField_eReserved_11_3  =  (11<<2)+3,
    BCMD_MISC_SetArchField_InCmdField_eMax
} BCMD_MISC_SetArchField_InCmdField_e;

typedef enum BCMD_MISC_SetArchField_OutCmdField_e
{
    BCMD_MISC_SetArchField_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_MISC_SetArchField_OutCmdField_eMax
}BCMD_MISC_SetArchField_OutCmdField_e;

typedef enum BCMD_EncoderDecoder_e
{
    BCMD_EncoderDecoder_eVDEC0 = 0,
    BCMD_EncoderDecoder_eRESERVED = 1,
    BCMD_EncoderDecoder_eVICE = 2,
    BCMD_EncoderDecoder_eVICE1 = 3,
    BCMD_EncoderDecoder_eVDEC1 = 4,
    BCMD_EncoderDecoder_eVDEC2 = 5,
    BCMD_EncoderDecoder_eMax
}BCMD_EncoderDecoder_e;


typedef enum BCMD_SetVichRegParField_InCmdField_e
{
    BCMD_SetVichRegParField_InCmdField_eVDEC_Id = (5 << 2) + 3,
    BCMD_SetVichRegParField_InCmdField_eNumRanges = (6 << 2) + 3,

    BCMD_SetVichRegParField_InCmdField_eRegPar0Start = (7 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar0End = (8 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar1Start = (9 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar1End = (10 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar2Start = (11 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar2End = (12 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar3Start = (13 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar3End = (14 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar4Start = (15 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar4End = (16 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar5Start = (17 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar5End = (18 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar6Start = (19 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar6End = (20 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar7Start = (21 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eRegPar7End = (22 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eSig = (23 << 2) + 0,
    BCMD_SetVichRegParField_InCmdField_eKeyLayer = (31 << 2) + 2,
    BCMD_SetVichRegParField_InCmdField_eVKL = (31 << 2) + 3,
    BCMD_SetVichRegParField_InCmdField_eMax

} BCMD_SetVichRegParField_InCmdField_e;

typedef enum BCMD_SetVichRegParField_OutCmdField_e
{
    BCMD_SetVichRegParField_OutCmdField_eNOT_USED = (6 << 2) + 3,
    BCMD_SetVichRegParField_OutCmdField_eMax
} BCMD_SetVichRegParField_OutCmdField_e;

typedef enum BCMD_MISC_START_AVD_InCmdField_e
{
    BCMD_MISC_START_AVD_InCmdField_eAVDId = (5 << 2) + 3,
    BCMD_MISC_START_AVD_InCmdField_eNumofReg = (6 << 2) + 3,

    BCMD_MISC_START_AVD_InCmdField_eAddr1 = (7 << 2) + 0,
    BCMD_MISC_START_AVD_InCmdField_eValue1 = (8 << 2) + 0,
    BCMD_MISC_START_AVD_InCmdField_eSig = (9 << 2) + 0,
    BCMD_MISC_START_AVD_InCmdField_eVKL = (17 << 2) + 3,
    BCMD_MISC_START_AVD_InCmdField_eKeyLayer = (18 << 2) + 2,
    BCMD_MISC_START_AVD_InCmdField_eMax
} BCMD_MISC_START_AVD_InCmdField_e;

typedef enum BCMD_StartAVD_OutCmdField_e
{
    BCMD_StartAVD_OutCmdField_eNotUSED = (6 << 2) + 3,
    BCMD_StartAVD_OutCmdField_eNotUSED1 = (7 << 2) + 0,
    BCMD_StartAVD_OutCmdField_eMax
}BCMD_StartAVD_OutCmdField_e;


typedef enum BCMD_MISC_PowerMgmtField_InCmdField_e
{
    BCMD_MISC_PowerMgmtField_InCmdField_ePwrMgmtOp = (5 << 2) + 3,
    BCMD_MISC_PowerMgmtField_InCmdField_eBkGndPeriod = (6 << 2) + 3,
    BCMD_MISC_PowerMgmtField_InCmdField_eMax
} BCMD_MISC_PowerMgmtField_InCmdField_e;



typedef enum BCMD_ReadExceptionStatus_InCmdField_e
{
    BCMD_ReadExceptionStatus_InCmdField_eKeepStatus = (5 << 2) + 2,
    BCMD_ReadExceptionStatus_InCmdField_eSubCommand = (5 << 2) + 3,
    BCMD_ReadExceptionStatus_InCmdField_eDevice     = (6 << 2) + 1,
    BCMD_ReadExceptionStatus_InCmdField_eUnit       = (6 << 2) + 2,
    BCMD_ReadExceptionStatus_InCmdField_eSubUnit    = (6 << 2) + 3,
    BCMD_ReadExceptionStatus_InCmdField_eMax
} BCMD_ReadExceptionStatus_InCmdField_e;

typedef enum BCMD_ReadExceptionStatus_OutCmdField_e
{
    BCMD_ReadExceptionStatus_OutCmdField_eStatus = (5 << 2) + 3,

    BCMD_ReadExceptionStatus_OutCmdField_eReserved_6_0  =  (6<<2)+0,

    BCMD_ReadExceptionStatus_OutCmdField_eDevice = (6 << 2) + 1,
    BCMD_ReadExceptionStatus_OutCmdField_eUnit = (6 << 2) + 2,
    BCMD_ReadExceptionStatus_OutCmdField_eSubUnit = (6 << 2) + 3,
    BCMD_ReadExceptionStatus_OutCmdField_eNumRegs = (7 << 2) + 3,
    BCMD_ReadExceptionStatus_OutCmdField_eRegister0 = (8 << 2) + 0,

    BCMD_ReadExceptionStatus_OutCmdField_eReserved_6_1  =  (6<<2)+1,
    BCMD_ReadExceptionStatus_OutCmdField_eReserved_7_0  =  (7<<2)+0,
    BCMD_ReadExceptionStatus_OutCmdField_eReserved_8_0  =  (8<<2)+0,
    BCMD_ReadExceptionStatus_OutCmdField_eReserved_9_0  =  (9<<2)+0,

    BCMD_ReadExceptionStatus_OutCmdField_eMax
} BCMD_ReadExceptionStatus_OutCmdField_e;




typedef enum BCMD_ExceptionStatus_KeepStatus_e
{
    BCMD_ExceptionStatus_KeepStatus_eClearRegs = 0x00,
    BCMD_ExceptionStatus_KeepStatus_eDoNotClear = 0x05,
    BCMD_ExceptionStatus_KeepStatus_eMax
} BCMD_ExceptionStatus_KeepStatus_e;

typedef enum BCMD_ExceptionStatus_Operation_e
{
    BCMD_ExceptionStatus_Operation_eReserved0  =  0x00,

    BCMD_ExceptionStatus_Operation_eCaptureReg = 0x55,
    BCMD_ExceptionStatus_Operation_eMax
} BCMD_ExceptionStatus_Operation_e;

typedef enum BCMD_ExceptionStatus_Device_e
{
    BCMD_ExceptionStatus_Device_eReserved0  =  0x00,
    BCMD_ExceptionStatus_Device_eReserved1  =  0x01,

    BCMD_ExceptionStatus_Device_eMemcARCH     = 0x02,

    BCMD_ExceptionStatus_Device_eReserved3  =  0x03,
    BCMD_ExceptionStatus_Device_eReserved4  =  0x04,
    BCMD_ExceptionStatus_Device_eReserved5  =  0x05,
    BCMD_ExceptionStatus_Device_eReserved6  =  0x06,
    BCMD_ExceptionStatus_Device_eReserved7  =  0x07,
    BCMD_ExceptionStatus_Device_eReserved8  =  0x08,
    BCMD_ExceptionStatus_Device_eReserved9  =  0x09,

    BCMD_ExceptionStatus_Device_eMax
} BCMD_ExceptionStatus_Device_e;

#endif
