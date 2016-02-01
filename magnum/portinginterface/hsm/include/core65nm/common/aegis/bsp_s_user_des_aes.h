/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Command input and output field offsets for USER_DES_AES command.
 *
 ***************************************************************************/

#ifndef BSP_S_USER_DES_AES_H__
#define BSP_S_USER_DES_AES_H__

/*--------------------------------------------------------------------------*/
/* Includes                                                                 */
/*--------------------------------------------------------------------------*/
#include "bsp_s_commands.h"


/*--------------------------------------------------------------------------*/
/* #defines and enums                                                       */
/*--------------------------------------------------------------------------*/
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_ONE_DES_ECB           0x00
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABA_ECB     0x01
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_AES_128_ECB           0x02
#define BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABC_ECB     0x03

#define KEY_INPUT_PARAMETER_SIZE                    (192/8) /* 192-bit field */

/* fields of USER_DES_AES command input
|-------------------|-------------------|-------------------|---------------------| 
|  31..24           | 23..16            | 15..8             | 7..0                | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Version Number      | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Owner ID            | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Continuous Mode     |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Tag ID              |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            |          Parameter Length               |
|-------------------|-------------------|-------------------|---------------------|
|                      BCMD_UserDESAES_CmdInputField_eKey [191:160] (unused)      |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eKey [159:128] (unused)      |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eKey [127:96]                |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eKey [95:64]                 |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eKey [63:32]                 |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eKey [31:0]                  |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           |  unused           |  unused           |..._eAlgorithmSel    |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           |  unused           |  unused           |..._eOperationMode   |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           |  unused           | ..._eDataLength   |..._eDataLengthPlus1 |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eData [127:96]               |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eData [95:64]                |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eData [63:32]                |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdInputField_eData [31:0]                 |
|-------------------|-------------------|-------------------|---------------------| 
*/
enum BCMD_UserDESAES_CmdInputField_e
{
    BCMD_UserDESAES_CmdInputField_eKey                 = (BCMD_CommonBufferFields_eParamLen + 2),                             /* byte 20 starting from 0 */ 
    BCMD_UserDESAES_CmdInputField_eAlgorithmSel        = (BCMD_UserDESAES_CmdInputField_eKey + KEY_INPUT_PARAMETER_SIZE + 3), /* byte 47 starting from 0 */
    BCMD_UserDESAES_CmdInputField_eOperationMode       = (BCMD_UserDESAES_CmdInputField_eAlgorithmSel + 4),                   /* byte 51 starting from 0 */
    BCMD_UserDESAES_CmdInputField_eDataLength          = (BCMD_UserDESAES_CmdInputField_eOperationMode + 3),                  /* byte 54 starting from 0 */
    BCMD_UserDESAES_CmdInputField_eDataLengthPlus1     = (BCMD_UserDESAES_CmdInputField_eDataLength + 1),                     /* byte 55 starting from 0 */
    BCMD_UserDESAES_CmdInputField_eData                = (BCMD_UserDESAES_CmdInputField_eDataLength + 2)                      /* byte 56 starting from 0 */
};

#define BCMD_USER_DES_AES_INPUT_DATA_MAX_LENGTH     (BCMD_BUFFER_BYTE_SIZE - BCMD_UserDESAES_CmdInputField_eData)

/* fields of USER_DES_AES command output
|-------------------|-------------------|-------------------|---------------------| 
|  31..24           | 23..16            | 15..8             | 7..0                | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Version Number      | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Owner ID            | 
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Continuous Mode     |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Tag ID              |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            |           Parameter Length              |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | unused            | Status              |
|-------------------|-------------------|-------------------|---------------------| 
|  unused           | unused            | ..._eDataLength   |...__eDataLengthPlus1|
|-------------------|-------------------|-------------------|---------------------|
|                      BCMD_UserDESAES_CmdOutputField_eData [127:96]              |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdOutputField_eData [95:64]               |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdOutputField_eData [63:32]               |
|-------------------|-------------------|-------------------|---------------------| 
|                      BCMD_UserDESAES_CmdOutputField_eData [31:0]                |
|-------------------|-------------------|-------------------|---------------------| 
*/
enum BCMD_UserDESAES_CmdOutputField_e
{
    BCMD_UserDESAES_CmdOutputField_eStatus             = (BCMD_CommonBufferFields_eParamLen + 5),          /* byte 23 starting from 0 */
    BCMD_UserDESAES_CmdOutputField_eDataLength         = (BCMD_UserDESAES_CmdOutputField_eStatus + 3),     /* byte 26 starting from 0 */
    BCMD_UserDESAES_CmdOutputField_eDataLengthPlus1    = (BCMD_UserDESAES_CmdOutputField_eDataLength + 1), /* byte 27 starting from 0 */
    BCMD_UserDESAES_CmdOutputField_eData               = (BCMD_UserDESAES_CmdOutputField_eDataLength + 2)  /* byte 28 starting from 0 */
};

#endif /* BSP_S_USER_DES_AES_H__ */




