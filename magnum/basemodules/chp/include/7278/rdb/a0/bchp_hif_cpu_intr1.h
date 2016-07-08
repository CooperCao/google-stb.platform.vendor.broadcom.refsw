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
 * Date:           Generated on               Mon Mar 21 13:44:43 2016
 *                 Full Compile MD5 Checksum  48e7e549bb13082ab30187cb156f35ed
 *                     (minus title and desc)
 *                 MD5 Checksum               949df837b98c31b52074d06d129f7b79
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     880
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_HIF_CPU_INTR1_H__
#define BCHP_HIF_CPU_INTR1_H__

/***************************************************************************
 *HIF_CPU_INTR1 - HIF CPU0 Thread Processor 0 (TP0) Level 1 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_HIF_CPU_INTR1_INTR_W0_STATUS        0x00499500 /* [RO] Interrupt Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W1_STATUS        0x00499504 /* [RO] Interrupt Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W2_STATUS        0x00499508 /* [RO] Interrupt Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W3_STATUS        0x0049950c /* [RO] Interrupt Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS   0x00499510 /* [RO] Interrupt Mask Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS   0x00499514 /* [RO] Interrupt Mask Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W2_MASK_STATUS   0x00499518 /* [RO] Interrupt Mask Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W3_MASK_STATUS   0x0049951c /* [RO] Interrupt Mask Status Register */
#define BCHP_HIF_CPU_INTR1_INTR_W0_MASK_SET      0x00499520 /* [WO] Interrupt Mask Set Register */
#define BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET      0x00499524 /* [WO] Interrupt Mask Set Register */
#define BCHP_HIF_CPU_INTR1_INTR_W2_MASK_SET      0x00499528 /* [WO] Interrupt Mask Set Register */
#define BCHP_HIF_CPU_INTR1_INTR_W3_MASK_SET      0x0049952c /* [WO] Interrupt Mask Set Register */
#define BCHP_HIF_CPU_INTR1_INTR_W0_MASK_CLEAR    0x00499530 /* [WO] Interrupt Mask Clear Register */
#define BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR    0x00499534 /* [WO] Interrupt Mask Clear Register */
#define BCHP_HIF_CPU_INTR1_INTR_W2_MASK_CLEAR    0x00499538 /* [WO] Interrupt Mask Clear Register */
#define BCHP_HIF_CPU_INTR1_INTR_W3_MASK_CLEAR    0x0049953c /* [WO] Interrupt Mask Clear Register */

#endif /* #ifndef BCHP_HIF_CPU_INTR1_H__ */

/* End of File */
