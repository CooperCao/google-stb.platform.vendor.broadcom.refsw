/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Thu Apr 13 10:09:31 2017
 *                 Full Compile MD5 Checksum  7f180d7646477bba2bae1a701efd9ef5
 *                     (minus title and desc)
 *                 MD5 Checksum               a2a4a53aa20c0c2f46073b879159b85d
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1395
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   LOCAL tools/dvtsw/current/Linux/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_BSCC_H__
#define BCHP_BSCC_H__

/***************************************************************************
 *BSCC - Broadcom Serial Control Master C
 ***************************************************************************/
#define BCHP_BSCC_CHIP_ADDRESS                   0x0041a300 /* [RW][32] BSC Chip Address And Read/Write Control */
#define BCHP_BSCC_DATA_IN0                       0x0041a304 /* [RW][32] BSC Write Data Register 0 */
#define BCHP_BSCC_DATA_IN1                       0x0041a308 /* [RW][32] BSC Write Data Register 1 */
#define BCHP_BSCC_DATA_IN2                       0x0041a30c /* [RW][32] BSC Write Data Register 2 */
#define BCHP_BSCC_DATA_IN3                       0x0041a310 /* [RW][32] BSC Write Data Register 3 */
#define BCHP_BSCC_DATA_IN4                       0x0041a314 /* [RW][32] BSC Write Data Register 4 */
#define BCHP_BSCC_DATA_IN5                       0x0041a318 /* [RW][32] BSC Write Data Register 5 */
#define BCHP_BSCC_DATA_IN6                       0x0041a31c /* [RW][32] BSC Write Data Register 6 */
#define BCHP_BSCC_DATA_IN7                       0x0041a320 /* [RW][32] BSC Write Data Register 7 */
#define BCHP_BSCC_CNT_REG                        0x0041a324 /* [RW][32] BSC Transfer Count Register */
#define BCHP_BSCC_CTL_REG                        0x0041a328 /* [RW][32] BSC Control Register */
#define BCHP_BSCC_IIC_ENABLE                     0x0041a32c /* [RW][32] BSC Read/Write Enable And Interrupt */
#define BCHP_BSCC_DATA_OUT0                      0x0041a330 /* [RO][32] BSC Read Data Register 0 */
#define BCHP_BSCC_DATA_OUT1                      0x0041a334 /* [RO][32] BSC Read Data Register 1 */
#define BCHP_BSCC_DATA_OUT2                      0x0041a338 /* [RO][32] BSC Read Data Register 2 */
#define BCHP_BSCC_DATA_OUT3                      0x0041a33c /* [RO][32] BSC Read Data Register 3 */
#define BCHP_BSCC_DATA_OUT4                      0x0041a340 /* [RO][32] BSC Read Data Register 4 */
#define BCHP_BSCC_DATA_OUT5                      0x0041a344 /* [RO][32] BSC Read Data Register 5 */
#define BCHP_BSCC_DATA_OUT6                      0x0041a348 /* [RO][32] BSC Read Data Register 6 */
#define BCHP_BSCC_DATA_OUT7                      0x0041a34c /* [RO][32] BSC Read Data Register 7 */
#define BCHP_BSCC_CTLHI_REG                      0x0041a350 /* [RW][32] BSC Control Register */
#define BCHP_BSCC_SCL_PARAM                      0x0041a354 /* [RW][32] BSC SCL Parameter Register */

#endif /* #ifndef BCHP_BSCC_H__ */

/* End of File */
