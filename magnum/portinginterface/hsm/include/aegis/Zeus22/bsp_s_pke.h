/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


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
