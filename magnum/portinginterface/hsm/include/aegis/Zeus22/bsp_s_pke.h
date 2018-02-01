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


#ifndef BSP_S_PKE_H__
#define BSP_S_PKE_H__


#define BCMD_PKE_DH_MAX_DATA_LENGTH     344
#define BCMD_PKE_DSA_MAX_DATA_LENGTH    336
#define BCMD_PKE_RSA_MAX_DATA_LENGTH    344


typedef enum BCMD_PKE_CommonOutFields_e
{
    BCMD_PKE_CommonOutFields_eStatus = (5<<2) + 3,
    BCMD_PKE_CommonOutFields_eMax
}BCMD_PKE_CommonOutFields_e;

typedef enum BCMD_PKE_DHInputCmdField_e
{
    BCMD_PKE_DHInputCmdField_eOp            = (5<<2) + 3,
    BCMD_PKE_DHInputCmdField_eModSrcIdx          = (6<<2) + 3,
    BCMD_PKE_DHInputCmdField_eBaseSrcIdx         = (7<<2) + 3,
    BCMD_PKE_DHInputCmdField_ePrivateKeySrcIdx  = (8<<2) + 3,
    BCMD_PKE_DHInputCmdField_eDataLen          = (9<<2) + 2,
    BCMD_PKE_DHInputCmdField_eData            = (10<<2) + 0,
    BCMD_PKE_DHInputCmdField_eMax
}BCMD_PKE_DHInputCmdField_e;

typedef enum BCMD_PKE_DHOutCmdField_e
{
    BCMD_PKE_DHOutCmdField_eKeyLen = (6<<2) + 2,
    BCMD_PKE_DHOutCmdField_eKeyBase = (7<<2) + 0,
    BCMD_PKE_DHOutCmdField_eMax
}BCMD_PKE_DHOutCmdField_e;

typedef enum BCMD_PKE_DSAInCmdField_e
{
    BCMD_PKE_DSAInCmdField_eOp       = (5<<2) + 3,
    BCMD_PKE_DSAInCmdField_ePKeySrc       = (6<<2) + 3,
    BCMD_PKE_DSAInCmdField_eGKeySrc       = (7<<2) + 3,
    BCMD_PKE_DSAInCmdField_eQKeySrc       = (8<<2) + 3,
    BCMD_PKE_DSAInCmdField_ePrivKeyX   = (9<<2) + 3,
    BCMD_PKE_DSAInCmdField_ePrivKeyK   = (10<<2) + 3,
    BCMD_PKE_DSAInCmdField_eDataLen        = (11<<2) + 2,
    BCMD_PKE_DSAInCmdField_eData       = (12<<2) + 0,
    BCMD_PKE_DSAInCmdField_eMax
}BCMD_PKE_DSAInCmdField_e;

typedef enum BCMD_PKE_DSAOutCmdField_e
{
    BCMD_PKE_DSAOutCmdField_eData   = (6<<2) + 0,
    BCMD_PKE_DSAOutCmdField_eMax
}BCMD_PKE_DSAOutCmdField_e;

typedef enum BCMD_PKE_RSAInCmdField_e
{
    BCMD_PKE_RSAInCmdField_eOp   = (5<<2) + 3,
    BCMD_PKE_RSAInCmdField_eKeySrcIdx1= (6<<2) + 3,
    BCMD_PKE_RSAInCmdField_eExpSrcIdx = (7<<2) + 3,
    BCMD_PKE_RSAInCmdField_eKeySrcIdx2= (8<<2) + 3,
    BCMD_PKE_RSAInCmdField_eDataLen = (9<<2) + 2,
    BCMD_PKE_RSAInCmdField_eData   = (10<<2) + 0,
    BCMD_PKE_RSAInCmdField_eMax
}BCMD_PKE_RSAInCmdField_e;

typedef enum BCMD_PKE_RSAOutCmdField_e
{
    BCMD_PKE_RSAOutCmdField_eDataLen   = (6<<2) + 2,
    BCMD_PKE_RSAOutCmdField_eData   = (7<<2) + 0,
    BCMD_PKE_RSAOutCmdField_eMax
}BCMD_PKE_RSAOutCmdField_e;

typedef enum BPI_PKE_Opcode_e
{
    BPI_PKE_Opcode_eDHPublicKeyGen = 1,
    BPI_PKE_Opcode_eDHPrivateKeyGen = 2,
    BPI_PKE_Opcode_eRSANormal = 3,
    BPI_PKE_Opcode_eRSACrt= 4,
    BPI_PKE_Opcode_eDSASign = 5,
    BPI_PKE_Opcode_eDSAVerify = 6,
    BPI_PKE_Opcode_eMax
}BPI_PKE_Opcode_e;

typedef enum BPI_PKE_DHOperation_e
{
    BPI_PKE_DHOperation_ePublicKeyGen = 0,
    BPI_PKE_DHOperation_eSecretKeyGen = 1,
    BPI_PKE_DHOperation_eMax
}BPI_PKE_DHOperation_e;

typedef enum BPI_PKE_RSAOperation_e
{
    BPI_PKE_RSAOperation_eNormal = 0,
    BPI_PKE_RSAOperation_eCrt = 1,
    BPI_PKE_RSAOperation_eMax
}BPI_PKE_RSAOperation_e;

typedef enum BPI_PKE_DSAOperation_e
{
    BPI_PKE_DSAOperation_ePrivateKeySign = 0,
    BPI_PKE_DSAOperation_ePublicKeyVer = 1,
    BPI_PKE_DSAOperation_eMax
}BPI_PKE_DSAOperation_e;

#define BPI_PKE_PASS_THROUGH	0xFF



typedef enum BCMD_PKE_CmdPollOutCmdField_e
{
    BCMD_PKE_CmdPollOutCmdField_estatus = (5<<2) + 3,
    BCMD_PKE_CmdPollOutCmdField_eData   = (6<<2) + 0,
    BCMD_PKE_CmdPollOutCmdField_eMax
}BCMD_PKE_CmdPollOutCmdField_e;


#endif
