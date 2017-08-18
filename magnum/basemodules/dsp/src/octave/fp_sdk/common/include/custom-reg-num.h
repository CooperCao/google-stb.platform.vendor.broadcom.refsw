/************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ************************************************************************/


// Define names of custom registers. Custom registers hold internal
// state for specific instructions.

// WARNING: this file is imported into the apps tree and munged by SWIG
// to make some Python.  The interface file for this lives at
// apps/src/pythonServices/customregs.i.  Please make sure that any
// changes you make here are reflected there; they may mean adding more
// ``%ignore'' declarations.

#pragma once

#ifdef __cplusplus
namespace Sim {
#endif // __cplusplus

enum CustomRegNum
{
    // NOTE: to preserve corefile backward compatibility, always add new
    //       registers to the end of this enumeration.

    // NOTE: where a register is split into numbered parts (e.g. VMX0,
    //       VMX1), any update side-effects (tracing, callbacks) will occur
    //       for all parts even if some do not change value. For the BX and
    //       BC registers, the count is always updated at the same time as
    //       the reservoir.

    // NOTE: Some of these registers are given specific numbers (making the
    //       enumeration sparse) for compatibility with an earlier numbering
    //       scheme, so that corefiles and the debug agent, which rely on
    //       the enumeration for buffer layout, can continue to work. Please
    //       don't interfere with these numbers.

    // =================================================================
    // Toliman (ARCH_V4) registers.
    // =================================================================

    // Viterbi registers.
    // Note that these must be the first four entries in the enumeration,
    // for backward compatibility with Toliman corefiles.
    CREG_VDX,  // vdx<63:0>
    CREG_VDY,  // vdy<63:0>
    CREG_VMX,  // vmx<31:0>
    CREG_VMY,  // vmy<31:0>

    // =================================================================
    // Mikeno (ARCH_V6) registers.
    // =================================================================

    // SIMD bit-field extractor.
    CREG_BX_RES = 4,      // BX_reservoir<254:0>
    CREG_BX_RES0 = CREG_BX_RES,
    CREG_BX_COUNT = 8,    // BX_count<7:0>
    CREG_BX_TRELLIS,      // BX_trellis<0>

    // SIMD bit-field combiner.
    CREG_BC_RES = 10,     // BC_reservoir<126:0>
    CREG_BC_COUNT = 12,   // BC_count<6:0>

    // Reed-Solomon coding support.
    CREG_RS_ACC_X = 13,   // RS_acc_x<127:0>
    CREG_RS_ACC_Y = 15,   // RS_acc_y<127:0>

    // SIMD trellis decode.
    CREG_TD_STATE = 17,   // TrellisDecodePipe.states[15:0]

    // SIMD trellis traceback.
    CREG_TR_STATE = 19,   // <35:32> = TrellisDecodePipe.tracepointer<3:0>
                          // <7:0>   = TrellisDecodePipe.symbol_noise<7:0>

    // 3G rake finger algorithm registers.  Each accumulator register
    // contains a pair of 18-bit values in word lanes.
    CREG_SCRAMA_X = 20,   // scramA_x[2]<31:0>
    CREG_SCRAMB_X,        // scramB_x[2]<31:0>
    CREG_DATACC_X,        // dataCC_x<3:0>
    CREG_SCRAMA_Y,        // scramA_y[2]<31:0>
    CREG_SCRAMB_Y,        // scramB_y[2]<31:0>
    CREG_DATACC_Y,        // dataCC_y<3:0>
    CREG_RKACC_C_X = 26,  // rkaccC_x[0..1]
    CREG_RKACC_D_X = 28,  // rkaccD_x[0..1]
    CREG_RKACC_C_Y = 30,  // rkaccC_y[0..1]
    CREG_RKACC_D_Y = 32,  // rkaccD_y[0..1]

    // 3G viterbi registers.  TX and TY are arranged as 4 half-words,
    // each of which only uses bits 8..0.
    CREG_VDX_V6 = 34,     // DX<63:0>
    CREG_VDY_V6,          // DY<63:0>
    CREG_VMX_V6 = 36,     // MX<127:0>
    CREG_VMY_V6 = 38,     // MY<127:0>
    CREG_VTX = 40,        // TX[4]<8:0>
    CREG_VTY,             // TY[4]<8:0>

    // FIR acceleration.
    CREG_FIR_HOLD_X = 42, // FIR_hold_x<63:0>
    CREG_FIR_HOLD_Y,      // FIR_hold_y<63:0>

    // Bitstream reading
    CREG_BS_BRB_DATA = 44,  // brb<2047:0>
    CREG_BS_BRB_LEVEL = 76, // brbLevel<11:0>
    CREG_BS_BRB_THRESHOLD,  // brbThreshold<10:0>
    CREG_BS_BRB_WR_PTR,     // buffer_wr_ptr<3:0>
    CREG_BS_BRB_RD_PTR,     // buffer_rd_ptr<3:0>

    // PTM CRC.
    CREG_PTM_CRC_STATE = 80,     // PTMCRCSTATE<127:0>

    // State for TRANSPOSEBD instruction.
    CREG_TRANSPOSE_STATE_D = 82, // TRANSPOSE_STATE_D<255:0>

    // Octave per-slot FIR hold state.
    CREG_FIR_HOLD_0 = 86,  // FIR_hold_0<63:0>
    CREG_FIR_HOLD_1,       // FIR_hold_1<63:0>
    CREG_FIR_HOLD_2,       // FIR_hold_2<63:0>
    CREG_FIR_HOLD_3,       // FIR_hold_3<63:0>

    // Octave per-slot TRANSPOSE state. Note that the state
    // values in even/odd slot pairs are always identical.
    CREG_TRANSPOSE_STATE_D_0 =  90, // TRANSPOSE_STATE_D_0<255:0>
    CREG_TRANSPOSE_STATE_D_1 =  94, // TRANSPOSE_STATE_D_1<255:0>
    CREG_TRANSPOSE_STATE_D_2 =  98, // TRANSPOSE_STATE_D_2<255:0>
    CREG_TRANSPOSE_STATE_D_3 = 102, // TRANSPOSE_STATE_D_3<255:0>

    // Octave per-slot PTMCRC state.
    CREG_PTM_CRC_STATE_1 = 106,     // PTMCRCSTATE_1<127:0>
    CREG_PTM_CRC_STATE_3 = 108,     // PTMCRCSTATE_3<127:0>

    // Octave bitstream read buffer state.
    CREG_OCT_BRB_DATA = 110,  // brb<2559:0>
    CREG_OCT_BRB_LEVEL = 150, // brbLevel<11:0>
    CREG_OCT_BRB_THRESHOLD,   // brbThreshold<11:0>
    CREG_OCT_BRB_WR_PTR,      // buffer_wr_ptr<4:0>
    CREG_OCT_BRB_RD_PTR,      // buffer_rd_ptr<4:0>

    // Octave slot 1 DSLBX state.
    CREG_DSLBX_RES     = 154, // DSLBX_reservoir<254:0>
    CREG_DSLBX_STATE_A = 158, // DSLBX_state<6:4>
    CREG_DSLBX_STATE_B,       // DSLBX_state<10:7,3:0>
    CREG_DSLBX_COUNT,         // DSLBX_count<7:0>
    CREG_DSLBX_PRBS,          // DSLBX_prbs<31:0>

    // Octave V2 WRITEBITS state.
    CREG_WB_RES = 162,        // WB_RES<95:0>
    CREG_WB_POS = 164,        // WB_POS<6:0>

    // Octave V2 BRB cache state.
    CREG_OCT_BRB_MEM_PTR     = 165, // buffer_mem_ptr<4:0>
    CREG_OCT_BRB_CACHE_DATA  = 166, // brbCacheData<62:0>

    // Total number of registers (all architectures) in this list.
    NUM_CREGS,

    // Specify the number of registers needed to include all those for a given
    // architecture. This enables a fixed corefile layout to be defined for
    // each architecture.
    NUM_CREGS_V4  = 1 + CREG_VMY,
    NUM_CREGS_V6  = 1 + CREG_FIR_HOLD_Y,
    NUM_CREGS_V8  = 1 + CREG_FIR_HOLD_Y,
    NUM_CREGS_V11 = 1 + CREG_BS_BRB_RD_PTR,
    NUM_CREGS_V12 = 4 + CREG_TRANSPOSE_STATE_D,
    NUM_CREGS_V14 = 1 + CREG_DSLBX_PRBS,
    NUM_CREGS_V16 = NUM_CREGS,

    // Aliases used to compute slot-specific instances
    CREG_FIR_HOLD = CREG_FIR_HOLD_0,

    // Error return value.
    INVALID_CREG = NUM_CREGS
};

#ifdef __cplusplus
}
#endif // __cplusplus
