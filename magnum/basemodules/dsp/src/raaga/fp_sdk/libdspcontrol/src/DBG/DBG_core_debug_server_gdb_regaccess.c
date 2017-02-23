/****************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#include <stddef.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"
#include "fp_sdk_config.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER) || FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server variant of the DBG core"
#endif

#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER))
#include "libdebugagent/src/DBAv2/DBAv2DebugAgentInternals.h"

typedef DBAv2_full_Context DBG_full_Context;
#else
#include "libfp/context_save.h"
typedef struct
{
    CS_registerContext reg_context;
    uint32_t           u32_pred_regs;
}DBG_full_Context;
#endif
#include "DBG_core_debug_server_gdb_regaccess.h"

/*
 * Register offset translation table for octave
 * */
static DBG_core_reg_info_t DBG_core_octave_register_info[]=
{
    {true,   FP_REG_R0,  FP_REG_R61, 8,  offsetof(DBG_full_Context, reg_context) +
                                        offsetof(CS_registerContext, u64UserRegs[0]) },

    /* FIXME: Context save doesn't store OR and ZR -
     * Need to populate them correctly*/
    {false,   FP_REG_R62, FP_REG_R63, 8, 0 },

    /*Octave doesn't have banked registers*/
    {false,  FP_REG_SU_R60,  FP_REG_SI_R61, 8,   0 },

    /*Special Registers - PSR*/
    {true,   FP_REG_PSR, FP_REG_PSR,   8, offsetof(DBG_full_Context, reg_context) +
                                         offsetof(CS_registerContext, psr) },

    /*Special Registers - MSR*/
    {true,   FP_REG_MSR0, FP_REG_MSR3, 8, offsetof(DBG_full_Context, reg_context) +
                                         offsetof(CS_registerContext, mReg) +
                                         offsetof(CS_mRegContext, u64Msr[0]) },

    /*Special Registers - PC*/
    {true,   FP_REG_PC, FP_REG_PC, 8, offsetof(DBG_full_Context, reg_context) +
                                     offsetof(CS_registerContext, pc)},
    /*MAC registers*/
    {true,   FP_REG_M0, FP_REG_M7, 24, offsetof(DBG_full_Context, reg_context) +
                                      offsetof(CS_registerContext, mReg) +
                                      offsetof(CS_mRegContext, mRegInfo) },/* + */
                                      /*offsetof(CS_mRegInfo, mRegs[0]) +
                                      offsetof(CS_mReg, u64MReg[0]) }, */


    /*Predicate registers P0 - P3*/
    {true,   FP_REG_P0,  FP_REG_P3,  1,  offsetof(DBG_full_Context, u32_pred_regs)},

    /*Predicate registers P4 - P7 - not available in Octave*/
    {false, FP_REG_P4, FP_REG_P7, 1, 0},

#if FIREPATH_REGS_HAS_CREGS

    /*FIXME: Not all Custom registers have an equivalent storage in the saved context*/
#ifdef SIMD_BITFIELD_CONTEXT

    {true, FP_REGS_CREG_INDEX(CREG_BX_RES), FP_REGS_CREG_INDEX(CREG_BX_RES), 32, offsetof(DBG_full_Context, reg_context) +
                                         offsetof(CS_registerContext, bitFieldState) +
                                         offsetof(CS_bitFieldState, bxr)},

    /*FIXME: Check bx_count, bx_trellis, bc_count have one 64bit storage in bcxm context save*/
    {true, FP_REGS_CREG_INDEX(CREG_BX_COUNT), FP_REGS_CREG_INDEX(CREG_BX_COUNT), 8, offsetof(DBG_full_Context, reg_context) +
                                            offsetof(CS_registerContext, bitFieldState) },

    {true, FP_REGS_CREG_INDEX(CREG_BX_TRELLIS), FP_REGS_CREG_INDEX( CREG_BX_TRELLIS), 8, offsetof(DBG_full_Context, reg_context) +
                                                offsetof(CS_registerContext, bitFieldState) },

    {true, FP_REGS_CREG_INDEX(CREG_BC_RES), FP_REGS_CREG_INDEX( CREG_BC_RES), 16, offsetof(DBG_full_Context, reg_context) +
                                         offsetof(CS_registerContext, bitFieldState) +
                                         offsetof(CS_bitFieldState, bcr)},

    {true, FP_REGS_CREG_INDEX(CREG_BC_COUNT), FP_REGS_CREG_INDEX( CREG_BC_COUNT), 8, offsetof(DBG_full_Context, reg_context) +
                                            offsetof(CS_registerContext, bitFieldState) },
#endif

#ifdef TRELLIS_STATE_CONTEXT

    /* SIMD trellis decode */
    {true, FP_REGS_CREG_INDEX(CREG_TD_STATE), FP_REGS_CREG_INDEX( CREG_TD_STATE), 16, offsetof(DBG_full_Context, reg_context) +
                                             offsetof(CS_registerContext, trellisState)},

    /* SIMD trellis traceback */
    {true, FP_REGS_CREG_INDEX(CREG_TR_STATE), FP_REGS_CREG_INDEX( CREG_TR_STATE), 8, offsetof(DBG_full_Context, reg_context) +
                                            offsetof(CS_registerContext, trellisState) +
                                            offsetof(CS_trellisState, u64TrellisState)},
#endif

#ifdef FIR_STATE_CONTEXT

    /*FIR STATE*/
    {true, FP_REGS_CREG_INDEX(CREG_FIR_HOLD_0), FP_REGS_CREG_INDEX( CREG_FIR_HOLD_0), 8, offsetof(DBG_full_Context, reg_context) +
                                                offsetof(CS_registerContext, firState) + 0},

    {true, FP_REGS_CREG_INDEX(CREG_FIR_HOLD_1), FP_REGS_CREG_INDEX( CREG_FIR_HOLD_1), 8, offsetof(DBG_full_Context, reg_context) +
                                                offsetof(CS_registerContext, firState) + 8},

    {true, FP_REGS_CREG_INDEX(CREG_FIR_HOLD_2), FP_REGS_CREG_INDEX( CREG_FIR_HOLD_2), 8, offsetof(DBG_full_Context, reg_context) +
                                                offsetof(CS_registerContext, firState) + 16},

    {true, FP_REGS_CREG_INDEX(CREG_FIR_HOLD_3), FP_REGS_CREG_INDEX( CREG_FIR_HOLD_3), 8, offsetof(DBG_full_Context, reg_context) +
                                                offsetof(CS_registerContext, firState) + 24},
#endif

    /*FIXME: Transpose, PTMCRC, BRB, DSLBX registers not present in the context save structure */
    {false, FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_0), FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_0), 32, 0},
    {false, FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_1), FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_1), 32, 0},
    {false, FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_2), FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_2), 32, 0},
    {false, FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_3), FP_REGS_CREG_INDEX(CREG_TRANSPOSE_STATE_D_3), 32, 0},

    {false, FP_REGS_CREG_INDEX(CREG_PTM_CRC_STATE_1), FP_REGS_CREG_INDEX(CREG_PTM_CRC_STATE_1), 16, 0},
    {false, FP_REGS_CREG_INDEX(CREG_PTM_CRC_STATE_3), FP_REGS_CREG_INDEX(CREG_PTM_CRC_STATE_3), 16, 0},

    {false, FP_REGS_CREG_INDEX(CREG_OCT_BRB_DATA), FP_REGS_CREG_INDEX(CREG_OCT_BRB_DATA), 320, 0},
    {false, FP_REGS_CREG_INDEX(CREG_OCT_BRB_LEVEL), FP_REGS_CREG_INDEX(CREG_OCT_BRB_RD_PTR), 8, 0},

    {false, FP_REGS_CREG_INDEX(CREG_DSLBX_RES), FP_REGS_CREG_INDEX(CREG_DSLBX_RES), 32, 0},
    {false, FP_REGS_CREG_INDEX(CREG_DSLBX_STATE_A), FP_REGS_CREG_INDEX(CREG_DSLBX_PRBS), 8, 0},

#if defined (__FP4015__)
    /*Octave V2 extensions*/
    {false, FP_REGS_CREG_INDEX(CREG_WB_RES), FP_REGS_CREG_INDEX(CREG_WB_RES), 12, 0},
    {false, FP_REGS_CREG_INDEX(CREG_WB_POS), FP_REGS_CREG_INDEX(CREG_WB_POS), 8, 0},

    {false, FP_REGS_CREG_INDEX(CREG_OCT_BRB_MEM_PTR), FP_REGS_CREG_INDEX(CREG_OCT_BRB_MEM_PTR), 8, 0},
    {false, FP_REGS_CREG_INDEX(CREG_OCT_BRB_CACHE_DATA), FP_REGS_CREG_INDEX(CREG_OCT_BRB_CACHE_DATA), 8, 0}
#endif

#endif
};


/*
 * Translate the register number
 * Reply with the register offset in the CS_registerContext and the size of the register
 * */
void
DBG_core_get_dba_reg_info(uint32_t u32_reg_num, DBG_core_cs_reg_info_t *p_cs_reg_info)
{
    uint32_t index;

    p_cs_reg_info->b_valid = false;
    p_cs_reg_info->u32_cs_reg_size = 0;
    p_cs_reg_info->u32_cs_reg_offset = 0;

    if(u32_reg_num > FP_REGS_LIMIT)
    {
        DSPLOG_ERROR("get_dba_reg_info(): u32_reg_num %d exceeds limit %d", u32_reg_num, FP_REGS_LIMIT);
        return;
    }

    for(index = 0; index < NUM_ELEMS(DBG_core_octave_register_info); index++)
    {
        if((u32_reg_num >= DBG_core_octave_register_info[index].fp_reg_lo) && (u32_reg_num <= DBG_core_octave_register_info[index].fp_reg_hi))
        {
            DBG_core_reg_info_t *p_reg_info = &DBG_core_octave_register_info[index];
            if(DBG_core_octave_register_info[index].b_valid)
            {
                p_cs_reg_info->b_valid = true;
                p_cs_reg_info->u32_cs_reg_size = p_reg_info->u32_reg_size;
                p_cs_reg_info->u32_cs_reg_offset = p_reg_info->u32_dbp_cs_base_offset + (u32_reg_num - p_reg_info->fp_reg_lo) * p_reg_info->u32_reg_size;
            }
            else
                p_cs_reg_info->u32_cs_reg_size = p_reg_info->u32_reg_size;

            break;
        }
    }
}

#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
/*
 * Translate GDB register enumeration to threadxp internal register
 * enumeration. Since GDB register enumeration is a superset (for num)
 * the function will return TX_NUM_REGISTER_IDS if the input
 * gdb_regnum is not found in TX_REGISTER space
 * */

TX_REGISTER_ID
DBG_core_gdb_to_threadxp_regnum(firepath_regs_t gdb_regnum)
{
    /*General registers*/
    if(gdb_regnum >= FP_REG_R0 && gdb_regnum <= FP_REG_R61)
        return (TX_REG_ID_r0 + gdb_regnum);

    /*Predicates*/
    else if(gdb_regnum >= FP_REG_P0 && gdb_regnum <= FP_REG_P3)
        return (TX_REG_ID_p0 + (gdb_regnum - FP_REG_P0));

    /*PC*/
    else if (gdb_regnum == FP_REG_PC)
        return TX_REG_ID_pc;

    else
        return TX_NUM_REGISTER_IDS;
}
#endif
