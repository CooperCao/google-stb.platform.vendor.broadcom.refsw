/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Feb  8 12:53:12 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_FTM_SW_SPARE_H__
#define BCHP_FTM_SW_SPARE_H__

/***************************************************************************
 *FTM_SW_SPARE - FTM SW SPARE Register Set
 ***************************************************************************/
#define BCHP_FTM_SW_SPARE_0                      0x01210330 /* [RW] FTM CORE SW SPARE0 */
#define BCHP_FTM_SW_SPARE_1                      0x01210334 /* [RW] FTM CORE SW SPARE1 */

/***************************************************************************
 *0 - FTM CORE SW SPARE0
 ***************************************************************************/
/* FTM_SW_SPARE :: 0 :: SW_SPARE0 [31:00] */
#define BCHP_FTM_SW_SPARE_0_SW_SPARE0_MASK                         0xffffffff
#define BCHP_FTM_SW_SPARE_0_SW_SPARE0_SHIFT                        0
#define BCHP_FTM_SW_SPARE_0_SW_SPARE0_DEFAULT                      0x00000000

/***************************************************************************
 *1 - FTM CORE SW SPARE1
 ***************************************************************************/
/* FTM_SW_SPARE :: 1 :: SW_SPARE1 [31:00] */
#define BCHP_FTM_SW_SPARE_1_SW_SPARE1_MASK                         0xffffffff
#define BCHP_FTM_SW_SPARE_1_SW_SPARE1_SHIFT                        0
#define BCHP_FTM_SW_SPARE_1_SW_SPARE1_DEFAULT                      0x00000000

#endif /* #ifndef BCHP_FTM_SW_SPARE_H__ */

/* End of File */
