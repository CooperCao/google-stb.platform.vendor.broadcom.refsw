/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
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
 * Date:           Generated on               Thu Jun 18 10:52:57 2015
 *                 Full Compile MD5 Checksum  32b78c1804e11666b824f2b9450a6228
 *                     (minus title and desc)
 *                 MD5 Checksum               3452ff65b8043c1c458e059705af3b49
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     16265
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_BSCC_H__
#define BCHP_BSCC_H__

/***************************************************************************
 *BSCC - Broadcom Serial Control Master C
 ***************************************************************************/
#define BCHP_BSCC_CHIP_ADDRESS                   0x20417300 /* [RW] BSC Chip Address And Read/Write Control */
#define BCHP_BSCC_DATA_IN0                       0x20417304 /* [RW] BSC Write Data Register 0 */
#define BCHP_BSCC_DATA_IN1                       0x20417308 /* [RW] BSC Write Data Register 1 */
#define BCHP_BSCC_DATA_IN2                       0x2041730c /* [RW] BSC Write Data Register 2 */
#define BCHP_BSCC_DATA_IN3                       0x20417310 /* [RW] BSC Write Data Register 3 */
#define BCHP_BSCC_DATA_IN4                       0x20417314 /* [RW] BSC Write Data Register 4 */
#define BCHP_BSCC_DATA_IN5                       0x20417318 /* [RW] BSC Write Data Register 5 */
#define BCHP_BSCC_DATA_IN6                       0x2041731c /* [RW] BSC Write Data Register 6 */
#define BCHP_BSCC_DATA_IN7                       0x20417320 /* [RW] BSC Write Data Register 7 */
#define BCHP_BSCC_CNT_REG                        0x20417324 /* [RW] BSC Transfer Count Register */
#define BCHP_BSCC_CTL_REG                        0x20417328 /* [RW] BSC Control Register */
#define BCHP_BSCC_IIC_ENABLE                     0x2041732c /* [RW] BSC Read/Write Enable And Interrupt */
#define BCHP_BSCC_DATA_OUT0                      0x20417330 /* [RO] BSC Read Data Register 0 */
#define BCHP_BSCC_DATA_OUT1                      0x20417334 /* [RO] BSC Read Data Register 1 */
#define BCHP_BSCC_DATA_OUT2                      0x20417338 /* [RO] BSC Read Data Register 2 */
#define BCHP_BSCC_DATA_OUT3                      0x2041733c /* [RO] BSC Read Data Register 3 */
#define BCHP_BSCC_DATA_OUT4                      0x20417340 /* [RO] BSC Read Data Register 4 */
#define BCHP_BSCC_DATA_OUT5                      0x20417344 /* [RO] BSC Read Data Register 5 */
#define BCHP_BSCC_DATA_OUT6                      0x20417348 /* [RO] BSC Read Data Register 6 */
#define BCHP_BSCC_DATA_OUT7                      0x2041734c /* [RO] BSC Read Data Register 7 */
#define BCHP_BSCC_CTLHI_REG                      0x20417350 /* [RW] BSC Control Register */
#define BCHP_BSCC_SCL_PARAM                      0x20417354 /* [RW] BSC SCL Parameter Register */

#endif /* #ifndef BCHP_BSCC_H__ */

/* End of File */
