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



#ifndef BSP_S_USER_AES_DES_H__
#define BSP_S_USER_AES_DES_H__

#define USER_AES_MAX_SIZE_WDS (UINT16)84

typedef enum BCMD_User_AES_DES_InCmdField_e
{
    BCMD_User_AES_DES_InCmdField_eAlgoSelect = (5 << 2) + 1,
    BCMD_User_AES_DES_InCmdField_eIsEncrypt = (5 << 2) + 2,
    BCMD_User_AES_DES_InCmdField_eIsUserKey = (5 << 2) + 3,
    BCMD_User_AES_DES_InCmdField_eReserved_6_2  =  (6<<2)+2,
    BCMD_User_AES_DES_InCmdField_eReserved_6_3  =  (6<<2)+3,
    BCMD_User_AES_DES_InCmdField_eReserved_7_2  =  (7<<2)+2,
    BCMD_User_AES_DES_InCmdField_eReserved_8_0  =  (8<<2),

    BCMD_User_AES_DES_InCmdField_eUserKeyFirstWord = (8 << 2),
    BCMD_User_AES_DES_InCmdField_eDataFirstWord = (12 << 2),
    BCMD_User_AES_DES_InCmdField_eMax

}BCMD_User_AES_DES_InCmdField_e;


typedef enum BCMD_User_AES_DES_OutCmdField_e
{
    BCMD_User_AES_DES_OutCmdField_eStatus = (5 << 2) + 3,
    BCMD_User_AES_DES_OutCmdField_eDataFirstWord = (6 << 2),
    BCMD_User_AES_DES_OutCmdField_eMax
}BCMD_User_AES_DES_OutCmdField_e;



typedef enum BCMD_User_AES_DES_AlgoSelect_e
{
    BCMD_User_AES_DES_AlgoSelect_e1DES = 0,
    BCMD_User_AES_DES_AlgoSelect_e3DESABA = 1,
    BCMD_User_AES_DES_AlgoSelect_eAES1281Blk = 2,
    BCMD_User_AES_DES_AlgoSelect_eReserved3  =  3,
    BCMD_User_AES_DES_AlgoSelect_eReserved4  =  4,
    BCMD_User_AES_DES_AlgoSelect_eMax
}BCMD_User_AES_DES_AlgoSelect_e;

#endif
