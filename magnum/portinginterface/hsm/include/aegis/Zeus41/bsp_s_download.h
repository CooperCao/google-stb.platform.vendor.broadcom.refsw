/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/



#ifndef BSP_S_DOWNLOAD_H__
#define BSP_S_DOWNLOAD_H__


typedef enum BCMD_Download_InCmdSecondTierKeyVerify_e
{
    BCMD_Download_InCmdSecondTierKeyVerify_eKeyIdentifier   = ( 5 << 2)+3,
    BCMD_Download_InCmdSecondTierKeyVerify_eRootKeySource   = ( 6 << 2)+3,
    BCMD_Download_InCmdSecondTierKeyVerify_eMultiTierKey    = ( 6 << 2)+2,
    BCMD_Download_InCmdSecondTierKeyVerify_eChipResetOnFail = ( 6 << 2)+1,
    BCMD_Download_InCmdSecondTierKeyVerify_eAddress         = ( 7 << 2)+0,
    BCMD_Download_InCmdSecondTierKeyVerify_eMax
}BCMD_Download_InCmdSecondTierKeyVerify_e;

typedef enum BCMD_Download_OutCmdSecondTierKeyVerify_e
{
    BCMD_Download_OutCmdSecondTierKeyVerify_eReserved       = (6 << 2)+0,
    BCMD_Download_OutCmdSecondTierKeyVerify_eMax
}BCMD_Download_OutCmdSecondTierKeyVerify_e;


typedef enum BCMD_Download_InCmdSecondStageCodeLoad_e
{
    BCMD_Download_InCmdSecondStageCodeLoad_eSecondStageCodePtrs = (5 << 2)+0,
    BCMD_Download_InCmdSecondStageCodeLoad_eSecondStageCodeSig = (6 << 2)+0,
    BCMD_Download_InCmdSecondStageCodeLoad_eHashLocked = (7 << 2)+1,
    BCMD_Download_InCmdSecondStageCodeLoad_eLoadFromDram = (7 << 2)+2,
    BCMD_Download_InCmdSecondStageCodeLoad_eKeySelect = (7 << 2)+3,
    BCMD_Download_InCmdSecondStageCodeLoad_eMax
}BCMD_Download_InCmdSecondStageCodeLoad_e;

typedef enum BCMD_Download_OutCmdSecondStageCodeLoad_e
{
    BCMD_Download_OutCmdSecondStageCodeLoad_eReserved = (6 << 2)+0,
    BCMD_Download_OutCmdSecondStageCodeLoad_eMax
}BCMD_Download_OutCmdSecondStageCodeLoad_e;


typedef enum BCMD_SecondTierKeyId_e
{
    BCMD_SecondTierKeyId_eKey1  = 1,
    BCMD_SecondTierKeyId_eKey2  = 2,
    BCMD_SecondTierKeyId_eKey3  = 3,
    BCMD_SecondTierKeyId_eKey4  = 4,
    BCMD_SecondTierKeyId_eMax
} BCMD_SecondTierKeyId_e;

typedef enum BCMD_FirstTierKeyId_e
{
    BCMD_FirstTierKeyId_eKey0Prime  = 0,
    BCMD_FirstTierKeyId_eKey0       = 1,
    BCMD_FirstTierKeyId_eMax
} BCMD_FirstTierKeyId_e;

#define RSAKEYID_IS_VALID(k)   (((k) >= BCMD_SecondTierKeyId_eKey1) && ((k) <= BCMD_SecondTierKeyId_eKey4))

#endif
