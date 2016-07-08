/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Feb 26 13:24:10 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     823
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

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
