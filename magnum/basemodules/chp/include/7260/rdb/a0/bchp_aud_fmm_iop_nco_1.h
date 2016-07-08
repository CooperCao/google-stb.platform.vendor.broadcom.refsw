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
 * Date:           Generated on               Fri Feb 26 13:24:11 2016
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

#ifndef BCHP_AUD_FMM_IOP_NCO_1_H__
#define BCHP_AUD_FMM_IOP_NCO_1_H__

/***************************************************************************
 *AUD_FMM_IOP_NCO_1
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_CONTROL 0x20cb1000 /* [RW] Rate Manager Controls */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_RATE_RATIO 0x20cb1004 /* [RW] Rate Manager Output Rate Setting I */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_SAMPLE_INC 0x20cb1008 /* [RW] Rate Manager Output Rate Setting II */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_PHASE_INC 0x20cb100c /* [RW] Rate Manager NCO Phase Increment */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_INTEGRATOR_HI 0x20cb1010 /* [WO] Loop Filter Integrator HI Value */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_INTEGRATOR_LO 0x20cb1014 /* [WO] Loop Filter Integrator LO Value */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_OFFSET 0x20cb1018 /* [RW] Rate Manager Output Formatting */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_FORMAT 0x20cb101c /* [RW] Rate Manager Output Formatting */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_SKIP_REPEAT_CONTROL 0x20cb1020 /* [RW] Time Base Skip or Repeat Control Register */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_SKIP_REPEAT_GAP 0x20cb1024 /* [RW] Time Base Skip or Repeat Gap Count Register */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_SKIP_REPEAT_NUMBER 0x20cb1028 /* [RW] Time Base Skip or Repeat Count Register */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_INTEGRATOR_HI_STATUS 0x20cb102c /* [RO] Loop Filter Integrator HI Status */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_INTEGRATOR_LO_STATUS 0x20cb1030 /* [RO] Loop Filter Integrator LO Status */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_STATUS 0x20cb1034 /* [RO] Rate Manager Status Register */
#define BCHP_AUD_FMM_IOP_NCO_1_MCLK_GEN_0_STATUS_CLEAR 0x20cb1038 /* [RW] Rate Manager Status Clear Register */

#endif /* #ifndef BCHP_AUD_FMM_IOP_NCO_1_H__ */

/* End of File */
