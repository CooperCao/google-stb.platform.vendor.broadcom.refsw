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
 * Date:           Generated on               Mon Mar 21 13:44:45 2016
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

#ifndef BCHP_MDI_FCN_1_H__
#define BCHP_MDI_FCN_1_H__

/***************************************************************************
 *MDI_FCN_1 - MADR Field Control Block Registers
 ***************************************************************************/
#define BCHP_MDI_FCN_1_FIELD_STATE_FIFO_STATUS_0 0x00092c00 /* [RO] Field State FIFO Status 0 */
#define BCHP_MDI_FCN_1_FIELD_STATE_FIFO_STATUS_1 0x00092c04 /* [RO] Field State FIFO Status 1 */
#define BCHP_MDI_FCN_1_OBTS_STATUS               0x00092c08 /* [RO] OBTS Status */
#define BCHP_MDI_FCN_1_IT_STATISTICS_STORE_STATUS 0x00092c0c /* [RO] Inverse Telecine Statistics Store Status */
#define BCHP_MDI_FCN_1_DEBUG_IT_PHASE_0_CORRELATION_COUNTER 0x00092c10 /* [RO] Inverse Telecine Phase 0 Correlation Counter */
#define BCHP_MDI_FCN_1_DEBUG_IT_PHASE_1_CORRELATION_COUNTER 0x00092c14 /* [RO] Inverse Telecine Phase 1 Correlation Counter */
#define BCHP_MDI_FCN_1_DEBUG_IT_PHASE_2_CORRELATION_COUNTER 0x00092c18 /* [RO] Inverse Telecine Phase 2 Correlation Counter */
#define BCHP_MDI_FCN_1_DEBUG_IT_PHASE_3_CORRELATION_COUNTER 0x00092c1c /* [RO] Inverse Telecine Phase 3 Correlation Counter */
#define BCHP_MDI_FCN_1_DEBUG_IT_PHASE_4_CORRELATION_COUNTER 0x00092c20 /* [RO] Inverse Telecine Phase 4 Correlation Counter */
#define BCHP_MDI_FCN_1_IT_HISTOGRAM_BIN_0        0x00092c24 /* [RO] Inverse Telecine Histogram Bin 0 */
#define BCHP_MDI_FCN_1_IT_HISTOGRAM_BIN_1        0x00092c28 /* [RO] Inverse Telecine Histogram Bin 1 */
#define BCHP_MDI_FCN_1_IT_HISTOGRAM_BIN_2        0x00092c2c /* [RO] Inverse Telecine Histogram Bin 2 */
#define BCHP_MDI_FCN_1_IT_HISTOGRAM_BIN_3        0x00092c30 /* [RO] Inverse Telecine Histogram Bin 3 */
#define BCHP_MDI_FCN_1_IT_HISTOGRAM_BIN_4        0x00092c34 /* [RO] Inverse Telecine Histogram Bin 4 */
#define BCHP_MDI_FCN_1_IT_FRAME_UNEXPECTED_MOTION_0 0x00092c38 /* [RO] Inverse Telecine Frame Unexpected Motion 0 */
#define BCHP_MDI_FCN_1_IT_FRAME_UNEXPECTED_MOTION_1 0x00092c3c /* [RO] Inverse Telecine Frame Unexpected Motion 1 */
#define BCHP_MDI_FCN_1_IT_FRAME_UNEXPECTED_MOTION_2 0x00092c40 /* [RO] Inverse Telecine Frame Unexpected Motion 2 */
#define BCHP_MDI_FCN_1_IT_FRAME_UNEXPECTED_MOTION_3 0x00092c44 /* [RO] Inverse Telecine Frame Unexpected Motion 3 */
#define BCHP_MDI_FCN_1_IT_FRAME_UNEXPECTED_MOTION_4 0x00092c48 /* [RO] Inverse Telecine Frame Unexpected Motion 4 */
#define BCHP_MDI_FCN_1_DEBUG_IT_REV22_PHASE_0_COUNTER 0x00092c4c /* [RO] Inverse Telecine Reverse 2:2 Phase 0 Correlation Counter */
#define BCHP_MDI_FCN_1_DEBUG_IT_REV22_PHASE_1_COUNTER 0x00092c50 /* [RO] Inverse Telecine Reverse 2:2 Phase 1 Correlation Counter */
#define BCHP_MDI_FCN_1_IT_PCC_LUMA_BWD           0x00092c54 /* [RO] Inverse Telecine Polarity-Change Luma BWD Count */
#define BCHP_MDI_FCN_1_DEBUG_CURRENT_FIELD_CONTROL_0 0x00092c60 /* [RW] Current Field Control set 0 */
#define BCHP_MDI_FCN_1_DEBUG_CURRENT_FIELD_CONTROL_1 0x00092c64 /* [RW] Current Field Control set 1 */
#define BCHP_MDI_FCN_1_DEBUG_CURRENT_FIELD_CONTROL_2 0x00092c68 /* [RW] Current Field Control set 2 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_0 0x00092c70 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 0 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_1 0x00092c74 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 1 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_2 0x00092c78 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 2 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_3 0x00092c7c /* [CFG] Inverse Telecine Field Phase Calculation Control Set 3 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_4 0x00092c80 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 4 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_5 0x00092c84 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 5 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_8 0x00092c88 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 8 */
#define BCHP_MDI_FCN_1_IT_OUTPUT_CONTROL         0x00092c8c /* [RW] Inverse Telecine Output Control */
#define BCHP_MDI_FCN_1_OBTS_DECAY                0x00092c90 /* [CFG] OBTS Decay */
#define BCHP_MDI_FCN_1_OBTS_HOLDOFF              0x00092c94 /* [CFG] OBTS Hold Off */
#define BCHP_MDI_FCN_1_OBTS_MAX_HOLDOFF          0x00092c98 /* [CFG] OBTS Maximum Hold Off */
#define BCHP_MDI_FCN_1_OBTS_CONTROL              0x00092c9c /* [CFG] OBTS Control */
#define BCHP_MDI_FCN_1_MODE_CONTROL_0            0x00092ca0 /* [CFG] Operation Mode Set 0 */
#define BCHP_MDI_FCN_1_MODE_CONTROL_1            0x00092ca4 /* [CFG] Operation Mode Set 1 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_9 0x00092cb0 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 9 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_10 0x00092cb4 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 10 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_11 0x00092cbc /* [CFG] Inverse Telecine Field Phase Calculation Control Set 11 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_12 0x00092cc0 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 12 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_13 0x00092cc4 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 13 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_14 0x00092cc8 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 14 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_15 0x00092ccc /* [CFG] Inverse Telecine Field Phase Calculation Control Set 15 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_16 0x00092cd0 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 16 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_17 0x00092cd4 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 17 */
#define BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_18 0x00092cd8 /* [CFG] Inverse Telecine Field Phase Calculation Control Set 18 */
#define BCHP_MDI_FCN_1_IT_BAD_WEAVE_CONTROL_0    0x00092cdc /* [CFG] Bad Weave Control Set 0 */
#define BCHP_MDI_FCN_1_IT_BAD_WEAVE_CONTROL_1    0x00092ce0 /* [CFG] Bad Weave Control Set 1 */
#define BCHP_MDI_FCN_1_IT_LG_PCC_COUNT           0x00092cf0 /* [RO] Inverse Large PCC Counter */
#define BCHP_MDI_FCN_1_IT_FEATHER_COUNT          0x00092cf4 /* [RO] Inverse Feather Detection Counter */
#define BCHP_MDI_FCN_1_IT_TICKER_COUNT           0x00092cf8 /* [RO] Inverse Ticker Detection Counter */
#define BCHP_MDI_FCN_1_IT_STAIR_COUNT            0x00092cfc /* [RO] Inverse Stair Detection Counter */
#define BCHP_MDI_FCN_1_SCRATCH_0                 0x00092dfc /* [CFG] Scratch register 0 */

#endif /* #ifndef BCHP_MDI_FCN_1_H__ */

/* End of File */
