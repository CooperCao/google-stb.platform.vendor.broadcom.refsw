/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/

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
    BCMD_SetMiscBitsSubCmd_eXPTPipesBypass   = 7,
    BCMD_SetMiscBitsSubCmd_eXPTReset   = 8,
	BCMD_SetMiscBitsSubCmd_eMax
}BCMD_SetMiscBitsSubCmd_e;




typedef enum BCMD_XPT_BandSel_e
{
   BCMD_XPT_BandSel_eInBand = 0,
   BCMD_XPT_BandSel_ePlayBack = 1,
   BCMD_XPT_BandSel_eMax
}BCMD_XPT_BandSel_e;

typedef enum BCMD_MISC_RaveCtrlBitsMask_e {
	BCMD_MISC_RaveCtrlBitsMask_eIMEM_WR = 0x01,
	BCMD_MISC_RaveCtrlBitsMask_eIMEM_RD = 0x02,
	BCMD_MISC_RaveCtrlBitsMask_eDMEM_RD = 0x04,
	BCMD_MISC_RaveCtrlBitsMask_eRESERVED  = 0x08,
	BCMD_MISC_RaveCtrlBitsMask_eDISABLE_CLR = 0x10,
	BCMD_MISC_RaveCtrlBitsMask_eMax
}BCMD_MISC_RaveCtrlBitsMask_e;


typedef enum BCMD_SetMiscBitsField_InCmdField_e
{
	BCMD_SetMiscBitsField_InCmdField_eSubCommandId = (5<<2) + 3,
	BCMD_SetMiscBitsField_InCmdField_eCtrlBits = (6<<2) + 3,
	BCMD_SetMiscBitsField_InCmdField_eBandSel = (6<<2) + 3,
	BCMD_SetMiscBitsField_InCmdField_eBandNum = (6<<2) + 2,
	BCMD_SetMiscBitsField_InCmdField_eForceSCbits = (7<<2) + 3,
	BCMD_SetMiscBitsField_InCmdField_eCPSRaveTimestampDis = (6<<2) + 3,
	BCMD_SetMiscBitsField_InCmdField_eMax
} BCMD_SetMiscBitsField_InCmdField_e;

typedef enum BCMD_SetMiscBitsField_OutCmdField_e
{
	BCMD_SetMiscBitsField_OutCmdField_eRaveStatusBit = (6 <<2) + 3,
	BCMD_SetMiscBitsField_OutCmdField_eM2MStatusBits = (6 <<2),
	BCMD_SetMiscBitsField_OutCmdField_eXPTStatusBits = (7 <<2),
	BCMD_SetMiscBitsField_OutCmdField_eXPTCheckerViolationAddress = (8 <<2),
	BCMD_SetMiscBitsField_OutCmdField_eXPTCheckerViolationIDs = (9 <<2),
	BCMD_SetMiscBitsField_OutCmdField_eMax
} BCMD_SetMiscBitsField_OutCmdField_e;


typedef enum BCMD_MISC_SetArchField_InCmdField_e
{
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeLow = (5 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eAddrRangeHigh = (6 << 2) + 0,
    BCMD_MISC_SetArchField_InCmdField_eDramSel = (7 << 2) + 1,
    BCMD_MISC_SetArchField_InCmdField_ePCIWin = (7 << 2) + 2,
    BCMD_MISC_SetArchField_InCmdField_eArchSel = (7 << 2) + 3,
    BCMD_MISC_SetArchField_InCmdField_eMemcArchBgckEnable = (8 << 2) + 3,
    BCMD_MISC_SetArchField_InCmdField_eMax
} BCMD_MISC_SetArchField_InCmdField_e;


typedef enum BCMD_SetVichRegParField_InCmdField_e
{
    BCMD_SetVichRegParField_InCmdField_eVDEC_Id    = (5 << 2) + 3,
    BCMD_SetVichRegParField_InCmdField_eNumRanges    = (6 << 2) + 3,

	BCMD_SetVichRegParField_InCmdField_eRegPar0Start = (7<<2) + 0,
	BCMD_SetVichRegParField_InCmdField_eRegPar0End   = (8<<2) + 0,

	BCMD_SetVichRegParField_InCmdField_eRegPar7Start = (22<<2) + 0,
	BCMD_SetVichRegParField_InCmdField_eRegPar7End   = (23<<2) + 0,
	BCMD_SetVichRegParField_InCmdField_eMax

} BCMD_SetVichRegParField_InCmdField_e;

typedef enum BCMD_SetVichRegParField_OutCmdField_e
{
	BCMD_SetVichRegParField_OutCmdField_eNOT_USED    = (6 <<2) + 3,
	BCMD_SetVichRegParField_OutCmdField_eMax
} BCMD_SetVichRegParField_OutCmdField_e;


typedef enum BCMD_MISC_START_AVD_InCmdField_e
{
	BCMD_MISC_START_AVD_InCmdField_eAVDId       = (5<<2) + 3,
	BCMD_MISC_START_AVD_InCmdField_eNumofReg    = (6<<2) + 3,

	BCMD_MISC_START_AVD_InCmdField_eAddr1       = (7<<2) + 0,
	BCMD_MISC_START_AVD_InCmdField_eValue1      = (8<<2) + 0,
    BCMD_MISC_START_AVD_InCmdField_eMax
} BCMD_MISC_START_AVD_InCmdField_e;

typedef enum BCMD_StartAVD_OutCmdField_e
{
	BCMD_StartAVD_OutCmdField_eNotUSED = (6 <<2) + 3,
    BCMD_StartAVD_OutCmdField_eNotUSED1   = (7<<2) + 0,
    BCMD_StartAVD_OutCmdField_eMax
}BCMD_StartAVD_OutCmdField_e;


typedef enum BCMD_MISC_PowerMgmtField_InCmdField_e
{
    BCMD_MISC_PowerMgmtField_InCmdField_ePwrMgmtOp = (5<<2) + 3,
    BCMD_MISC_PowerMgmtField_InCmdField_eBkGndPeriod = (6<<2) + 3,
    BCMD_MISC_PowerMgmtField_InCmdField_eMax
} BCMD_MISC_PowerMgmtField_InCmdField_e;





#endif
