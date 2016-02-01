/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


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
