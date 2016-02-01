/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/

#ifndef BSP_S_USER_RSA_H__
#define BSP_S_USER_RSA_H__



#define USER_RSA_COUNTERMEASURES_MASK     0x0000000FU

#define USER_RSA_DPA_MODEXP_PROTECT_MASK  0x00000008U
#define USER_RSA_DPA_MODEXP_PROTECT_SHIFT 3

#define USER_RSA_DPA_RANDOM_STALL_MASK    0x00000006U
#define USER_RSA_DPA_RANDOM_STALL_SHIFT   1

#define USER_RSA_FW_RSA_MODE1_MASK       0x00000001U
#define USER_RSA_FW_RSA_MODE1_SHIFT      0

typedef enum BCMD_User_RSA_InCmdField_e
{
    BCMD_User_RSA_InCmdField_eCountermeasure    = ( 5 << 2)+1,
    BCMD_User_RSA_InCmdField_eRSASize           = ( 5 << 2)+2,
    BCMD_User_RSA_InCmdField_esubCmdId          = ( 5 << 2)+3,
    BCMD_User_RSA_InCmdField_eRSAKeyData        = ( 8 << 2)+0,
    BCMD_User_RSA_InCmdField_eMax

}BCMD_User_RSA_InCmdField_e;

typedef enum BCMD_User_RSA_OutCmdField_e
{
    BCMD_User_RSA_OutCmdField_eOutputDataSize= (6 << 2)+2,
    BCMD_User_RSA_OutCmdField_eOutputData= (7 << 2),
    BCMD_User_RSA_OutCmdField_eMax

}BCMD_User_RSA_OutCmdField_e;

typedef enum BCMD_User_RSA_SubCommand_e
{
    BCMD_User_RSA_SubCommand_eLoadModulus      = 0,
    BCMD_User_RSA_SubCommand_eLoadExponent     = 1,
    BCMD_User_RSA_SubCommand_eLoadBaseStartOp  = 2,
    BCMD_User_RSA_SubCommand_eMax
}BCMD_User_RSA_SubCommand_e;


typedef enum BCMD_User_RSA_Size_e
{
    BCMD_User_RSA_Size_e1024                     = 0,
    BCMD_User_RSA_Size_e2048                     = 1,
    BCMD_User_RSA_Size_eMax
}BCMD_User_RSA_Size_e;

#endif
