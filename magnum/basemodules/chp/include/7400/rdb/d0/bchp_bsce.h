/***************************************************************************
 *     Copyright (c) 1999-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Mon Jul 23 12:01:10 2007
 *                 MD5 Checksum         61f9c4d8dcdcd06017506dddbf23f434
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008004
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_BSCE_H__
#define BCHP_BSCE_H__

/***************************************************************************
 *BSCE - Broadcom Serial Control Master E
 ***************************************************************************/
#define BCHP_BSCE_CHIP_ADDRESS                   0x00400800 /* BSC Chip Address And Read/Write Control */
#define BCHP_BSCE_DATA_IN0                       0x00400804 /* BSC Write Data Byte 0 */
#define BCHP_BSCE_DATA_IN1                       0x00400808 /* BSC Write Data Byte 1 */
#define BCHP_BSCE_DATA_IN2                       0x0040080c /* BSC Write Data Byte 2 */
#define BCHP_BSCE_DATA_IN3                       0x00400810 /* BSC Write Data Byte 3 */
#define BCHP_BSCE_DATA_IN4                       0x00400814 /* BSC Write Data Byte 4 */
#define BCHP_BSCE_DATA_IN5                       0x00400818 /* BSC Write Data Byte 5 */
#define BCHP_BSCE_DATA_IN6                       0x0040081c /* BSC Write Data Byte 6 */
#define BCHP_BSCE_DATA_IN7                       0x00400820 /* BSC Write Data Byte 7 */
#define BCHP_BSCE_CNT_REG                        0x00400824 /* BSC Transfer Count Register */
#define BCHP_BSCE_CTL_REG                        0x00400828 /* BSC Control Register */
#define BCHP_BSCE_IIC_ENABLE                     0x0040082c /* BSC Read/Write Enable And Interrupt */
#define BCHP_BSCE_DATA_OUT0                      0x00400830 /* BSC Read Data Byte 0 */
#define BCHP_BSCE_DATA_OUT1                      0x00400834 /* BSC Read Data Byte 1 */
#define BCHP_BSCE_DATA_OUT2                      0x00400838 /* BSC Read Data Byte 2 */
#define BCHP_BSCE_DATA_OUT3                      0x0040083c /* BSC Read Data Byte 3 */
#define BCHP_BSCE_DATA_OUT4                      0x00400840 /* BSC Read Data Byte 4 */
#define BCHP_BSCE_DATA_OUT5                      0x00400844 /* BSC Read Data Byte 5 */
#define BCHP_BSCE_DATA_OUT6                      0x00400848 /* BSC Read Data Byte 6 */
#define BCHP_BSCE_DATA_OUT7                      0x0040084c /* BSC Read Data Byte 7 */
#define BCHP_BSCE_CTLHI_REG                      0x00400850 /* BSC Control Register */
#define BCHP_BSCE_SCL_PARAM                      0x00400854 /* BSC SCL Parameter Register */

#endif /* #ifndef BCHP_BSCE_H__ */

/* End of File */
