/******************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
******************************************************************************/

#include "bmmd.h"
#include "blst_squeue.h"
#include "blst_slist.h"
#include "bchp_common.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

/* MEM_DMA HW */
#if BCHP_CHIP==3548 || BCHP_CHIP==3556 || BCHP_CHIP==3563 || BCHP_CHIP==7118 || \
    BCHP_CHIP==7401 || BCHP_CHIP==7403 || BCHP_CHIP==7440 || BCHP_CHIP==7601 || \
    BCHP_CHIP==7630 || BCHP_CHIP==7635
#define BMMD_P_NUM_DMA_ENGINES 1
#include "bchp_mem_dma.h"
#elif BCHP_CHIP==7400 || BCHP_CHIP==7435 /* 7400 A0 has one DMA engine but B0 has two. no support for 7400 A0 */
#define BMMD_P_NUM_DMA_ENGINES 2
#include "bchp_mem_dma_0.h"
#include "bchp_mem_dma_1.h"
#elif BCHP_CHIP==7550 /* 7550 is the only known instance of no MEM_DMA HW */
#define BMMD_P_NUM_DMA_ENGINES 0
#else /* everyone else */
#define BMMD_P_NUM_DMA_ENGINES 1
#include "bchp_mem_dma_0.h"
#endif

/* SHARF_DMA HW */
#if ((BCHP_CHIP==7550) && (BCHP_VER>=BCHP_VER_B0)) || BCHP_CHIP==7640 || \
      BCHP_CHIP==35125 || ((BCHP_CHIP==35230) && (BCHP_VER>=BCHP_VER_C0)) || \
      BCHP_CHIP==35233
#define BMMD_P_NUM_SHARF_DMA_ENGINES 2
#include "bchp_sharf_mem_dma0.h"
#include "bchp_sharf_mem_dma1.h"
/* 7440 C2 has 2 SHARF and 1 DMA. 7630 B0 has 2 SHARF and 1 DMA. No SHARF support for these until requested */
#else
#define BMMD_P_NUM_SHARF_DMA_ENGINES 0
#endif

#if (BCHP_CHIP==7550) && (BCHP_VER>=BCHP_VER_B0)
#define BMMD_P_HAS_LMC_CORE 1
#include "bchp_sharf_lmc_core.h"
#endif

/* MEM_DMA interrupt */
#if BCHP_CHIP==7125  || BCHP_CHIP==7231  || BCHP_CHIP==7340  || BCHP_CHIP==7342  || \
    BCHP_CHIP==7344  || BCHP_CHIP==7346  || BCHP_CHIP==7358  || BCHP_CHIP==7360  || \
    BCHP_CHIP==7420  || BCHP_CHIP==7422  || BCHP_CHIP==7425  || BCHP_CHIP==7429  || \
    BCHP_CHIP==7435  || BCHP_CHIP==7468  || BCHP_CHIP==7552  || BCHP_CHIP==7584  || \
    BCHP_CHIP==7640  || BCHP_CHIP==7563  || BCHP_CHIP==7543  || BCHP_CHIP==7362  || \
    BCHP_CHIP==7228  || BCHP_CHIP==75635 || BCHP_CHIP==73625 || BCHP_CHIP==75845 || \
    BCHP_CHIP==74295 || BCHP_CHIP==75525 || BCHP_CHIP==73465
#include "bchp_int_id_bsp_control_intr2.h"
#elif BCHP_CHIP==7400 || BCHP_CHIP==7405
#include "bchp_int_id_memc16_gfx_l2.h"
#elif BCHP_CHIP==7325 || BCHP_CHIP==7335 || BCHP_CHIP==7336
#include "bchp_int_id_graphics_l2.h"
#elif BCHP_CHIP==7408 || BCHP_CHIP==7440 || BCHP_CHIP==3563 || BCHP_CHIP==35230 || \
      BCHP_CHIP==35125 || BCHP_CHIP==35233
#include "bchp_int_id_xpt_bus_if.h"
#elif BCHP_CHIP==3548 || BCHP_CHIP==3556
#include "bchp_int_id_gfx_l2.h"
#else
#include "bchp_int_id_hif_intr2.h"
#endif

/* SHARF_DMA interrupt */
#if BMMD_P_NUM_SHARF_DMA_ENGINES
#if BCHP_CHIP==7640
#include "bchp_int_id_hif_intr2.h"
#else
#include "bchp_int_id_sharf_intr2.h"
#endif
#endif

/* non-40nm platform */
#if BCHP_CHIP==7038 || BCHP_CHIP==7118 || BCHP_CHIP==7125 || BCHP_CHIP==7325 || \
    BCHP_CHIP==7335 || BCHP_CHIP==7336 || BCHP_CHIP==7340 || BCHP_CHIP==7342 || \
    BCHP_CHIP==7400 || BCHP_CHIP==7401 || BCHP_CHIP==7403 || BCHP_CHIP==7405 || \
    BCHP_CHIP==7408 || BCHP_CHIP==7420 || BCHP_CHIP==7438 || BCHP_CHIP==7440 || \
    BCHP_CHIP==7468 || BCHP_CHIP==7550 || BCHP_CHIP==7601 || BCHP_CHIP==7630 || \
    BCHP_CHIP==7635 || BCHP_CHIP==7640 || \
    BCHP_CHIP==3548 || BCHP_CHIP==3556 || BCHP_CHIP==3560 || BCHP_CHIP==3563 || \
    BCHP_CHIP==35125 || BCHP_CHIP==35130 || BCHP_CHIP==35230 || BCHP_CHIP==35233
#define BMMD_P_DMA_REV_2A 0
#else
#define BMMD_P_DMA_REV_2A 1
#define BMMD_P_AVOID_WAKE 1
#endif

#ifndef BCHP_MEM_DMA_0_FIRST_DESC
    #ifdef BCHP_MEM_DMA_FIRST_DESC /* name difference */
    #define BCHP_MEM_DMA_0_FIRST_DESC BCHP_MEM_DMA_FIRST_DESC
    #define BCHP_MEM_DMA_0_CTRL BCHP_MEM_DMA_CTRL
    #define BCHP_MEM_DMA_0_WAKE_CTRL BCHP_MEM_DMA_WAKE_CTRL
    #define BCHP_MEM_DMA_0_STATUS BCHP_MEM_DMA_STATUS
    #define BCHP_MEM_DMA_0_CUR_DESC BCHP_MEM_DMA_CUR_DESC
    #define BCHP_MEM_DMA_0_CUR_BYTE BCHP_MEM_DMA_CUR_BYTE
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Idle BCHP_MEM_DMA_STATUS_DMA_STATUS_Idle
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy BCHP_MEM_DMA_STATUS_DMA_STATUS_Busy
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep BCHP_MEM_DMA_STATUS_DMA_STATUS_Sleep
    #elif (defined BCHP_SHARF_MEM_DMA0_FIRST_DESC) /* SHARF-only platforms */
    #define BCHP_MEM_DMA_0_FIRST_DESC BCHP_SHARF_MEM_DMA0_FIRST_DESC
    #define BCHP_MEM_DMA_0_CTRL BCHP_SHARF_MEM_DMA0_CTRL
    #define BCHP_MEM_DMA_0_WAKE_CTRL BCHP_SHARF_MEM_DMA0_WAKE_CTRL
    #define BCHP_MEM_DMA_0_STATUS BCHP_SHARF_MEM_DMA0_STATUS
    #define BCHP_MEM_DMA_0_CUR_DESC BCHP_SHARF_MEM_DMA0_CUR_DESC
    #define BCHP_MEM_DMA_0_CUR_BYTE BCHP_SHARF_MEM_DMA0_CUR_BYTE
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Idle BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Idle
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Busy
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_Sleep
    #endif
#endif

#ifndef BCHP_INT_ID_MEM_DMA_0_INTR
    #ifdef BCHP_INT_ID_MEM_DMA_INTR
    #define BCHP_INT_ID_MEM_DMA_0_INTR BCHP_INT_ID_MEM_DMA_INTR
    #elif defined BCHP_INT_ID_XPT_BUS_IF_MEM_DMA_INTR
    #define BCHP_INT_ID_MEM_DMA_0_INTR BCHP_INT_ID_XPT_BUS_IF_MEM_DMA_INTR
    #elif defined BCHP_INT_ID_BSP_CONTROL_INTR2_MEM_DMA_0_INTR
    #define BCHP_INT_ID_MEM_DMA_0_INTR BCHP_INT_ID_BSP_CONTROL_INTR2_MEM_DMA_0_INTR
    #elif defined BCHP_INT_ID_MEM_DMA_INT
    #define BCHP_INT_ID_MEM_DMA_0_INTR BCHP_INT_ID_MEM_DMA_INT
    #elif (BMMD_P_NUM_DMA_ENGINES==0)
    #define BCHP_INT_ID_MEM_DMA_0_INTR 0
    #endif
#endif

#ifndef BCHP_INT_ID_MEM_DMA_1_INTR
#define BCHP_INT_ID_MEM_DMA_1_INTR 0
#endif

#ifndef BCHP_INT_ID_SHARF_MEM_DMA0_DONE
#define BCHP_INT_ID_SHARF_MEM_DMA0_DONE 0
#endif

#ifndef BCHP_INT_ID_SHARF_MEM_DMA1_DONE
#define BCHP_INT_ID_SHARF_MEM_DMA1_DONE 0
#endif

/* register masks and shifts */
#ifndef BCHP_MEM_DMA_0_FIRST_DESC_ADDR_SHIFT
    #ifdef BCHP_MEM_DMA_FIRST_DESC_ADDR_SHIFT /* name difference */
    #define BCHP_MEM_DMA_0_FIRST_DESC_ADDR_MASK      BCHP_MEM_DMA_FIRST_DESC_ADDR_MASK
    #define BCHP_MEM_DMA_0_FIRST_DESC_ADDR_SHIFT     BCHP_MEM_DMA_FIRST_DESC_ADDR_SHIFT
    #define BCHP_MEM_DMA_0_CTRL_RUN_MASK             BCHP_MEM_DMA_CTRL_RUN_MASK
    #define BCHP_MEM_DMA_0_CTRL_RUN_SHIFT            BCHP_MEM_DMA_CTRL_RUN_SHIFT
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MODE_MASK  BCHP_MEM_DMA_WAKE_CTRL_WAKE_MODE_MASK
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MODE_SHIFT BCHP_MEM_DMA_WAKE_CTRL_WAKE_MODE_SHIFT
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MASK       BCHP_MEM_DMA_WAKE_CTRL_WAKE_MASK
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_SHIFT      BCHP_MEM_DMA_WAKE_CTRL_WAKE_SHIFT
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_MASK    BCHP_MEM_DMA_STATUS_DMA_STATUS_MASK
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_SHIFT   BCHP_MEM_DMA_STATUS_DMA_STATUS_SHIFT
    #elif (defined BCHP_SHARF_MEM_DMA0_FIRST_DESC_ADDR_SHIFT) /* SHARF-only platforms */
    #define BCHP_MEM_DMA_0_FIRST_DESC_ADDR_MASK      BCHP_SHARF_MEM_DMA0_FIRST_DESC_ADDR_MASK
    #define BCHP_MEM_DMA_0_FIRST_DESC_ADDR_SHIFT     BCHP_SHARF_MEM_DMA0_FIRST_DESC_ADDR_SHIFT
    #define BCHP_MEM_DMA_0_CTRL_RUN_MASK             BCHP_SHARF_MEM_DMA0_CTRL_RUN_MASK
    #define BCHP_MEM_DMA_0_CTRL_RUN_SHIFT            BCHP_SHARF_MEM_DMA0_CTRL_RUN_SHIFT
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MODE_MASK  BCHP_SHARF_MEM_DMA0_WAKE_CTRL_WAKE_MODE_MASK
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MODE_SHIFT BCHP_SHARF_MEM_DMA0_WAKE_CTRL_WAKE_MODE_SHIFT
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_MASK       BCHP_SHARF_MEM_DMA0_WAKE_CTRL_WAKE_MASK
    #define BCHP_MEM_DMA_0_WAKE_CTRL_WAKE_SHIFT      BCHP_SHARF_MEM_DMA0_WAKE_CTRL_WAKE_SHIFT
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_MASK    BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_MASK
    #define BCHP_MEM_DMA_0_STATUS_DMA_STATUS_SHIFT   BCHP_SHARF_MEM_DMA0_STATUS_DMA_STATUS_SHIFT
    #endif
#endif

/* descriptor word definitions */
#ifndef BCHP_MEM_DMA_DESC_WORD0_READ_ADDR_SHIFT /* SHARF-only platforms */
#define BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_LITTLE_ENDIAN BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_LITTLE_ENDIAN
#define BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_BIG_ENDIAN BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_BIG_ENDIAN
#define BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_WORD_ALIGNED BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_WORD_ALIGNED
#define BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_HALF_WORD_ALIGNED BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_HALF_WORD_ALIGNED
#define BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_BYTE_ALIGNED BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_BYTE_ALIGNED

#define BCHP_MEM_DMA_DESC_WORD0_READ_ADDR_MASK BCHP_SHARF_MEM_DMA_DESC_WORD0_READ_ADDR_MASK
#define BCHP_MEM_DMA_DESC_WORD0_READ_ADDR_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD0_READ_ADDR_SHIFT
#define BCHP_MEM_DMA_DESC_WORD1_WRITE_ADDR_MASK BCHP_SHARF_MEM_DMA_DESC_WORD1_WRITE_ADDR_MASK
#define BCHP_MEM_DMA_DESC_WORD1_WRITE_ADDR_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD1_WRITE_ADDR_SHIFT
#define BCHP_MEM_DMA_DESC_WORD2_TRANSFER_SIZE_MASK BCHP_SHARF_MEM_DMA_DESC_WORD2_TRANSFER_SIZE_MASK
#define BCHP_MEM_DMA_DESC_WORD2_TRANSFER_SIZE_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD2_TRANSFER_SIZE_SHIFT
#define BCHP_MEM_DMA_DESC_WORD2_INTR_ENABLE_MASK BCHP_SHARF_MEM_DMA_DESC_WORD2_INTR_ENABLE_MASK
#define BCHP_MEM_DMA_DESC_WORD2_INTR_ENABLE_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD2_INTR_ENABLE_SHIFT
#define BCHP_MEM_DMA_DESC_WORD2_LAST_MASK BCHP_SHARF_MEM_DMA_DESC_WORD2_LAST_MASK
#define BCHP_MEM_DMA_DESC_WORD2_LAST_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD2_LAST_SHIFT
#define BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_MASK BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_MASK
#define BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_SHIFT
#define BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_MASK BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_MASK
#define BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_SHIFT
#define BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_MASK BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_MASK
#define BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT BCHP_SHARF_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT
/* WORD4 is not common between DMA and SHARF */

#define BCHP_MEM_DMA_DESC_WORD4_KEY_SELECT_MASK      0
#define BCHP_MEM_DMA_DESC_WORD4_KEY_SELECT_SHIFT     0
#define BCHP_MEM_DMA_DESC_WORD4_MODE_SEL_MASK        0
#define BCHP_MEM_DMA_DESC_WORD4_MODE_SEL_SHIFT       0
#define BCHP_MEM_DMA_DESC_WORD4_ENC_DEC_INIT_MASK    0
#define BCHP_MEM_DMA_DESC_WORD4_ENC_DEC_INIT_SHIFT   0
#define BCHP_MEM_DMA_DESC_WORD4_SG_ENABLE_MASK       0
#define BCHP_MEM_DMA_DESC_WORD4_SG_ENABLE_SHIFT      0
#define BCHP_MEM_DMA_DESC_WORD4_SG_SCRAM_START_SHIFT 0
#define BCHP_MEM_DMA_DESC_WORD4_SG_SCRAM_START_MASK  0
#define BCHP_MEM_DMA_DESC_WORD4_SG_SCRAM_END_SHIFT   0
#define BCHP_MEM_DMA_DESC_WORD4_SG_SCRAM_END_MASK    0
#endif

#ifndef BCHP_SHARF_MEM_DMA_DESC_WORD4_KEY_PRESENT_SHIFT /* no-SHARF platforms */
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_KEY_PRESENT_SHIFT    0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_KEY_PRESENT_MASK     0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_DIGEST_PRESENT_SHIFT 0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_DIGEST_PRESENT_MASK  0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_USE_BSP_KEY_SHIFT    0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_USE_BSP_KEY_MASK     0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_MODE_SEL_SHIFT       0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_MODE_SEL_MASK        0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_ENABLE_SHIFT      0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_ENABLE_MASK       0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_SCRAM_START_SHIFT 0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_SCRAM_START_MASK  0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_SCRAM_END_SHIFT   0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_SG_SCRAM_END_MASK    0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_CONTEXT_NUM_SHIFT    0
#define BCHP_SHARF_MEM_DMA_DESC_WORD4_CONTEXT_NUM_MASK     0
#endif

/* this came from BDMA_P_MEM_KEY_SIZE in bdma_mem_priv.h.
   it's not used in SHARF, and appears to be a DTV HW quirk */
#if BCHP_CHIP==35125 || BCHP_CHIP==35230 || BCHP_CHIP==35233 || BCHP_CHIP==7408
#define BMMD_P_KEY_SIZE 8
#else
#define BMMD_P_KEY_SIZE 6
#endif

/* magic number written to DESC_WORD5 to indicate QMD completion. 'BMMD' in ASCII */
#define BMMD_P_COMPLETION_CODE 0x424d4d44

BDBG_MODULE(bmmd);
BDBG_OBJECT_ID(BMMD_Handle_Tag);
BDBG_OBJECT_ID(BMMD_Context);

#define BMMD_P_DESC_PROTECT 1 /* run-time check for transfers that corrupt descriptor memory location */

#define BDBG_MSG_TRACE(x) /*BDBG_MSG(x)*/
#define BMMD_DEBUG_MODE 0 /* extra asserts for sanity checks */

void BMMD_Context_P_DescDump(BMMD_ContextHandle ctx, unsigned numBlocks);

typedef struct BMMD_Context
{
    BDBG_OBJECT(BMMD_Context)
    BMMD_Handle parent;
    uint32_t dmaDesc_3; /* prototype of dmaDesc[3] */
    uint32_t dmaDesc_4; /* prototype of dmaDesc[4] */
    void *firstDescCached;    /* cached pointer to first descriptor */
    uint32_t firstDescOffset; /* physical address of first descriptor */
    BMMD_ContextSettings settings;

    BLST_S_ENTRY(BMMD_Context) ctxNode;
    BLST_SQ_ENTRY(BMMD_Context) activeNode;
    enum {
        BMMD_Context_P_State_eIdle,
        BMMD_Context_P_State_eQueued,   /* ctx is queued in activeNode and hardware */
        BMMD_Context_P_State_eDestroyed /* Context_Destroy() was called */
    } state;

    uint32_t *lastDescCached; /* cached pointer to last descriptor that is used to mark the end of the chain */
    uint32_t lastDescOffset;     /* physical address of last descriptor */
    unsigned dmaLength; /* total length (in bytes) of DMA transaction */
    bool sg; /* true if context has sgScram descriptor(s) */
} BMMD_Context;

typedef struct BMMD_Handle_Tag
{
    BDBG_OBJECT(BMMD_Handle_Tag)
    BCHP_Handle chp;
    BREG_Handle reg;
    BMEM_Handle mem;
    BINT_Handle bint;
    BINT_CallbackHandle irq;
    uint32_t baseRegOffset; /* 0 if using HW index 0 */
    BMMD_Settings settings;
    void *qmdConstCached; /* cached mem location that holds constant value to indicate QMD completion */
    uint32_t qmdConstOffset;
    BMMD_ContextHandle lastQueuedCtx;
    unsigned numSgCtx;
    BLST_S_HEAD(CtxList, BMMD_Context) ctxList;
    BLST_SQ_HEAD(ActiveCtxList, BMMD_Context) activeCtxList;
    bool standby;
} BMMD_Handle_Tag;

#define BMMD_P_DESC_SIZE (4*8)
#define BMMD_P_MAX_TRANSFER_SIZE (0x1000000)

static void BMMD_P_CompleteCallback_isr(void *pParam1, int parm2);

void BMMD_GetDefaultSettings(
    BMMD_Settings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BMMD_Open(
    BCHP_Handle hChp,
    BREG_Handle hReg,
    BMEM_Handle hMem,
    BINT_Handle hInt,
    const BMMD_Settings *pSettings,
    BMMD_Handle *phMmd
    )
{
    BMMD_Handle pMmd = NULL;
    BERR_Code rc;
    uint32_t intr;
    void *ptr;

    BDBG_ASSERT(hChp);
    BDBG_ASSERT(hReg);
    BDBG_ASSERT(hMem);
    BDBG_ASSERT(hInt);
    BDBG_ASSERT(pSettings);

    BDBG_ASSERT(pSettings->engineType<BMMD_EngineType_eMax);
#if (BMMD_P_NUM_SHARF_DMA_ENGINES && BMMD_P_NUM_DMA_ENGINES)
    /* for DMA-only or SHARF-only platforms, this is not needed.
       but for platforms with both, SHARF register access relies on this */
    BDBG_CASSERT(BCHP_SHARF_MEM_DMA0_CTRL     -BCHP_SHARF_MEM_DMA0_FIRST_DESC == BCHP_MEM_DMA_0_CTRL     -BCHP_MEM_DMA_0_FIRST_DESC);
    BDBG_CASSERT(BCHP_SHARF_MEM_DMA0_WAKE_CTRL-BCHP_SHARF_MEM_DMA0_FIRST_DESC == BCHP_MEM_DMA_0_WAKE_CTRL-BCHP_MEM_DMA_0_FIRST_DESC);
    BDBG_CASSERT(BCHP_SHARF_MEM_DMA0_STATUS   -BCHP_SHARF_MEM_DMA0_FIRST_DESC == BCHP_MEM_DMA_0_STATUS   -BCHP_MEM_DMA_0_FIRST_DESC);
    BDBG_CASSERT(BCHP_SHARF_MEM_DMA0_CUR_DESC -BCHP_SHARF_MEM_DMA0_FIRST_DESC == BCHP_MEM_DMA_0_CUR_DESC -BCHP_MEM_DMA_0_FIRST_DESC);
    BDBG_CASSERT(BCHP_SHARF_MEM_DMA0_CUR_BYTE -BCHP_SHARF_MEM_DMA0_FIRST_DESC == BCHP_MEM_DMA_0_CUR_BYTE -BCHP_MEM_DMA_0_FIRST_DESC);
#endif

    if (pSettings->engineType==BMMD_EngineType_eDma) {
        if (pSettings->engineIndex>=BMMD_P_NUM_DMA_ENGINES) {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
        }
    }
    else {
        if (pSettings->engineIndex>=BMMD_P_NUM_SHARF_DMA_ENGINES) {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
        }
    }

    *phMmd = NULL;
    pMmd = BKNI_Malloc(sizeof(*pMmd));
    if (pMmd == NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pMmd, 0, sizeof(*pMmd));
    BDBG_OBJECT_SET(pMmd, BMMD_Handle_Tag);

    pMmd->chp = hChp;
    pMmd->reg = hReg;
    pMmd->mem = hMem;
    pMmd->bint = hInt;
    pMmd->settings = *pSettings;

    pMmd->baseRegOffset = 0;
    /* baseRegOffset abstracts the HW address (DMA_0/DMA_1/SHARF_0/SHARF_1) differences.
       DMA only platforms:      BCHP_MEM_DMA_0_* points to itself
       SHARF only platforms:    BCHP_MEM_DMA_0_* is defined to point to BCHP_SHARF_MEM_DMA0_*
       DMA and SHARF platforms: BCHP_MEM_DMA_0_* points to itself
                                SHARF requires a special offset */
#if (BMMD_P_NUM_DMA_ENGINES==2)
    if (pSettings->engineType==BMMD_EngineType_eDma && pSettings->engineIndex==1) {
        pMmd->baseRegOffset = BCHP_MEM_DMA_1_FIRST_DESC-BCHP_MEM_DMA_0_FIRST_DESC;
    }
#endif
#if (BMMD_P_NUM_DMA_ENGINES && BMMD_P_NUM_SHARF_DMA_ENGINES)
    if (pSettings->engineType==BMMD_EngineType_eSharf) {
        pMmd->baseRegOffset = BCHP_SHARF_MEM_DMA0_FIRST_DESC-BCHP_MEM_DMA_0_FIRST_DESC;
    }
#endif
#if (BMMD_P_NUM_SHARF_DMA_ENGINES==2)
    if (pSettings->engineType==BMMD_EngineType_eSharf && pSettings->engineIndex==1) {
        pMmd->baseRegOffset += (BCHP_SHARF_MEM_DMA1_FIRST_DESC-BCHP_SHARF_MEM_DMA0_FIRST_DESC);
    }
#endif
    BDBG_MSG_TRACE(("%p: baseRegOffset %#x", (void*)pMmd, pMmd->baseRegOffset));

    BLST_S_INIT(&pMmd->ctxList);
    BLST_SQ_INIT(&pMmd->activeCtxList);

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_AcquireResource(pMmd->chp, BCHP_PWR_RESOURCE_DMA);
#endif
    BREG_Write32(pMmd->reg, pMmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
        BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, false)); /* stop DMA on open */
    BREG_Write32(pMmd->reg, pMmd->baseRegOffset+BCHP_MEM_DMA_0_FIRST_DESC,
        BCHP_FIELD_DATA(MEM_DMA_0_FIRST_DESC, ADDR, 0));
    BREG_Write32(pMmd->reg, pMmd->baseRegOffset+BCHP_MEM_DMA_0_WAKE_CTRL,
        BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE_MODE, 1)); /* always use wake_from_last */

#if BMMD_P_HAS_LMC_CORE
    BREG_Write32(pMmd->reg, BCHP_SHARF_LMC_CORE_SWITCH_MEMC,
        BCHP_FIELD_DATA(SHARF_LMC_CORE_SWITCH_MEMC, MUX_SELECT, 1)); /* always point to MEMC, not LMC */
#endif
#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_ReleaseResource(pMmd->chp, BCHP_PWR_RESOURCE_DMA);
#endif

    intr = 0;
    if (pSettings->engineType==BMMD_EngineType_eDma) {
        intr = (pSettings->engineIndex==0)?BCHP_INT_ID_MEM_DMA_0_INTR:BCHP_INT_ID_MEM_DMA_1_INTR;
    }
    else {
        intr = (pSettings->engineIndex==0)?BCHP_INT_ID_SHARF_MEM_DMA0_DONE:BCHP_INT_ID_SHARF_MEM_DMA1_DONE;
    }
    BDBG_ASSERT(intr);

    rc = BINT_CreateCallback(&pMmd->irq, hInt, intr, BMMD_P_CompleteCallback_isr, pMmd, 0);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BINT_EnableCallback(pMmd->irq);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

    ptr = BMEM_AllocAligned(pMmd->mem, 4, 0, 0);
    if (ptr == NULL) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }
    rc = BMEM_ConvertAddressToCached(pMmd->mem, ptr, &pMmd->qmdConstCached);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    rc = BMEM_ConvertAddressToOffset(pMmd->mem, ptr, &pMmd->qmdConstOffset);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    *((uint32_t*)pMmd->qmdConstCached) = BMMD_P_COMPLETION_CODE;
    BMEM_FlushCache(pMmd->mem, pMmd->qmdConstCached, 4);
    BDBG_MSG_TRACE(("%p: QMD source cached %#x, offset %#x", (void*)pMmd, pMmd->qmdConstCached, pMmd->qmdConstOffset));

    *phMmd = pMmd;
    return BERR_SUCCESS;

error:
    if (pMmd != NULL) {
        if (pMmd->irq) {
            BINT_DestroyCallback(pMmd->irq);
        }
        BMEM_FreeCached(pMmd->mem, pMmd->qmdConstCached);
        BKNI_Free(pMmd);
        *phMmd = NULL;
    }
    return rc;
}

void BMMD_Context_GetDefaultSettings(
    BMMD_ContextSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->maxNumBlocks = 1;
}

void BMMD_Context_GetDefaultBlockSettings(
    BMMD_ContextBlockSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings != NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code BMMD_Context_P_SetSettings(
    BMMD_ContextHandle ctx,
    const BMMD_ContextSettings *pSettings)
{
    unsigned readEndian, swapMode;
    BMMD_EngineType engineType = ctx->parent->settings.engineType;
    BDBG_OBJECT_ASSERT(ctx, BMMD_Context);

    /* this only affects non-word aligned data */
    switch (pSettings->endianMode) {
        case BMMD_EndianMode_eLittle:
            readEndian = BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_LITTLE_ENDIAN;
            break;
        case BMMD_EndianMode_eBig:
            readEndian = BCHP_MEM_DMA_DESC_WORD3_READ_ENDIAN_MODE_BIG_ENDIAN;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    switch (pSettings->swapMode) {
        case BMMD_SwapMode_eNone:
            swapMode = BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_WORD_ALIGNED;
            break;
        case BMMD_SwapMode_eWord:
            swapMode = BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_HALF_WORD_ALIGNED;
            break;
        case BMMD_SwapMode_eByte:
            swapMode = BCHP_MEM_DMA_DESC_WORD3_WRITE_ENDIAN_XLATE_MODE_BYTE_ALIGNED;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    ctx->dmaDesc_3 =
        BCHP_FIELD_DATA(MEM_DMA_DESC_WORD3, READ_ENDIAN_MODE,        readEndian) |
        BCHP_FIELD_DATA(MEM_DMA_DESC_WORD3, WRITE_ENDIAN_XLATE_MODE, swapMode);

    if (engineType==BMMD_EngineType_eDma) {
        if (pSettings->scramMode >= BMMD_ScramMode_eMax) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        ctx->dmaDesc_4 =
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, MODE_SEL,   (unsigned)pSettings->scramMode) |
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, KEY_SELECT, pSettings->keyslot*BMMD_P_KEY_SIZE);
    }
    else {
        unsigned shaContext = 0;
        if (pSettings->sharf.mode >= BMMD_SharfMode_eMax) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        if (pSettings->sharf.shaContext >= 2) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        switch (pSettings->sharf.mode) {
            case BMMD_SharfMode_ePassThrough:
            case BMMD_SharfMode_eSha1:
                shaContext = pSettings->sharf.shaContext;
                break;
            default:
                shaContext = 0; /* must be 0 for AES or CMAC */
                break;
        }

        ctx->dmaDesc_4 =
            BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, USE_BSP_KEY, (unsigned)pSettings->sharf.useBspKey) |
            BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, MODE_SEL,    (unsigned)pSettings->sharf.mode) |
            BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, CONTEXT_NUM, shaContext);
    }

    ctx->settings = *pSettings;
    return BERR_SUCCESS;
}

BMMD_ContextHandle BMMD_Context_Create(
    BMMD_Handle hMmd,
    const BMMD_ContextSettings *pSettings
    )
{
    BMMD_Context *pCtx;
    BMMD_ContextSettings ctxSettings;
    BERR_Code rc;
    void *ptr;

    BDBG_OBJECT_ASSERT(hMmd, BMMD_Handle_Tag);

#if (BCHP_CHIP==7408) /* SW7408-385 */
    if (BLST_S_EMPTY(&hMmd->ctxList)) {
        BKNI_EnterCriticalSection();
        BINT_DisableCallback_isr(hMmd->irq);
        BINT_EnableCallback_isr(hMmd->irq);
        BKNI_LeaveCriticalSection();
    }
#endif

    if (pSettings == NULL) {
        BMMD_Context_GetDefaultSettings(&ctxSettings);
        pSettings = &ctxSettings;
    }
    if (pSettings->maxNumBlocks == 0) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_settings;
    }
    BDBG_ASSERT(pSettings->endianMode < BMMD_EndianMode_eMax);
    BDBG_ASSERT(pSettings->swapMode < BMMD_SwapMode_eMax);
    BDBG_ASSERT(pSettings->scramMode < BMMD_ScramMode_eMax);

    pCtx = BKNI_Malloc(sizeof(BMMD_Context));
    if ( NULL == pCtx ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    BKNI_Memset(pCtx, 0, sizeof(BMMD_Context));
    BDBG_OBJECT_SET(pCtx, BMMD_Context);

    pCtx->parent = hMmd;
    rc = BMMD_Context_P_SetSettings(pCtx, pSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_set_settings; }

    ptr = BMEM_AllocAligned(hMmd->mem, (1+pCtx->settings.maxNumBlocks)*BMMD_P_DESC_SIZE, 5, 0); /* +1 for QMD. 32 byte aligned */
    if (ptr == NULL) { rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto err_mem_alloc; }
    rc = BMEM_ConvertAddressToCached(hMmd->mem, ptr, &pCtx->firstDescCached);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_mem_alloc; }
    rc = BMEM_ConvertAddressToOffset(hMmd->mem, ptr, &pCtx->firstDescOffset);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_mem_alloc; }
    BDBG_MSG_TRACE(("%p create: firstDesc %#x, offset %#x, blocks %#x", (void*)pCtx, pCtx->firstDescCached, pCtx->firstDescOffset, pSettings->maxNumBlocks));

    /* initialize all (maxNumBlocks+1) descriptors */
    BKNI_Memset(pCtx->firstDescCached, 0, (1+pCtx->settings.maxNumBlocks)*BMMD_P_DESC_SIZE);

#if 0 /* this code is redundant */
    unsigned i;
    uint32_t *descAddr;
    for (i=0,descAddr=pCtx->firstDescCached; i<=pSettings->maxNumBlocks; i++,descAddr+=BMMD_P_DESC_SIZE/sizeof(uint32_t)) {
        descAddr[2] =
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, TRANSFER_SIZE, 0) |
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, INTR_ENABLE, false) |
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, LAST, true);
        descAddr[3] =
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, (pCtx->firstDescOffset+BMMD_P_DESC_SIZE)>>BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT); /* this always points to the address of the second descriptor */
    }
#endif
    /* no need to flush pCtx->firstDescCached until we actually start */
    BLST_S_INSERT_HEAD(&hMmd->ctxList, pCtx, ctxNode);

#if 0 /* for debug */
    BMMD_Context_P_DescDump(pCtx);
#endif

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_AcquireResource(pCtx->parent->chp, BCHP_PWR_RESOURCE_DMA);
#endif
    return pCtx;

err_mem_alloc:
err_set_settings:
    BKNI_Free(pCtx);
err_alloc:
err_settings:
    return NULL;
}

BERR_Code BMMD_Context_GetSettings(
    BMMD_ContextHandle hCtx,
    BMMD_ContextSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BMMD_Context);

    if (hCtx->state == BMMD_Context_P_State_eDestroyed) {
        return BERR_NOT_SUPPORTED;
    }

    *pSettings = hCtx->settings;
    return 0;
}

BERR_Code BMMD_Context_SetSettings(
    BMMD_ContextHandle hCtx,
    const BMMD_ContextSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BMMD_Context);

    if (hCtx->state != BMMD_Context_P_State_eIdle) {
        return BERR_NOT_SUPPORTED;
    }

    if (pSettings->maxNumBlocks != hCtx->settings.maxNumBlocks) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (pSettings->memoryBounds.offset != hCtx->settings.memoryBounds.offset ||
        pSettings->memoryBounds.size != hCtx->settings.memoryBounds.size) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BMMD_Context_P_SetSettings(hCtx, pSettings);
}


static void
BMMD_Context_P_NotifyFirst_isr(BMMD_Handle mmd, BMMD_ContextHandle ctx)
{
#if BMMD_DEBUG_MODE
    BDBG_ASSERT(ctx==BLST_SQ_FIRST(&mmd->activeCtxList));
#endif

#if (!BMMD_P_DMA_REV_2A)
    if (ctx->sg) {
        BDBG_ASSERT(mmd->numSgCtx>0);
        mmd->numSgCtx--;
    }
#endif

    if (ctx->state == BMMD_Context_P_State_eDestroyed) {
        BLST_SQ_REMOVE_HEAD(&mmd->activeCtxList, activeNode);
        return;
    }

    if (ctx->state != BMMD_Context_P_State_eIdle) {
        ctx->state = BMMD_Context_P_State_eIdle;
        BLST_SQ_REMOVE_HEAD(&mmd->activeCtxList, activeNode);

        if (ctx->settings.callback_isr) {
            ctx->settings.callback_isr(ctx->settings.pParm1, ctx->settings.pParm2);
        }
    }
    return;
}

/* lastDesc is the QMD. go back one descriptor and then go to word 5 */
#if BMMD_P_DMA_REV_2A
bool BMMD_Context_P_IsComplete_isr(BMMD_ContextHandle ctx)
{
    BMEM_FlushCache_isr(ctx->parent->mem, ctx->lastDescCached-8, BMMD_P_DESC_SIZE);
    return (*(ctx->lastDescCached-8+5) == BMMD_P_COMPLETION_CODE);
}
#else
bool BMMD_Context_P_IsComplete_isr(BMMD_ContextHandle ctx)
{
    uint32_t data = BREG_Read32(ctx->parent->reg, ctx->parent->baseRegOffset+BCHP_MEM_DMA_0_STATUS);
    data = BCHP_GET_FIELD_DATA(data, MEM_DMA_0_STATUS, DMA_STATUS);
    if (ctx->sg) {
        if (data==BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy) {
            return false;
        }
        else {
            data = BREG_Read32(ctx->parent->reg, ctx->parent->baseRegOffset+BCHP_MEM_DMA_0_CUR_DESC);
            return (data==ctx->lastDescOffset);
        }
    }
    else {
        BMEM_FlushCache_isr(ctx->parent->mem, ctx->lastDescCached-8, BMMD_P_DESC_SIZE);
        return (*(ctx->lastDescCached-8+5) == BMMD_P_COMPLETION_CODE);
    }
    return false;
}
#endif

void BMMD_P_DumpActiveCtxStatus_isrsafe(BMMD_Handle mmd)
{
    BMMD_ContextHandle ctx;
    bool complete;
    for (ctx=BLST_SQ_FIRST(&mmd->activeCtxList); ctx; ctx=BLST_SQ_NEXT(ctx, activeNode)) {
        complete = BMMD_Context_P_IsComplete_isr(ctx);
        BKNI_Printf("%#x %d\n", ctx->firstDescOffset, complete);
    }
}

/* this function is only called while HW is in SLEEP */
static void
BMMD_P_CheckAll_isr(BMMD_Handle mmd)
{
    BMMD_ContextHandle ctx;

    while (NULL!=(ctx=BLST_SQ_FIRST(&mmd->activeCtxList))) {
        if (BMMD_Context_P_IsComplete_isr(ctx)) {
            BDBG_MSG_TRACE(("%#x notify from idle", ctx->firstDescOffset));
            BMMD_Context_P_NotifyFirst_isr(mmd, ctx);
        }
        else {
#if BMMD_DEBUG_MODE
#if BMMD_P_DMA_REV_2A
            BDBG_ASSERT(0);
#endif
#endif

            BDBG_MSG_TRACE(("%#x restart", ctx->firstDescOffset));

            BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
                BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, false));
            BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_FIRST_DESC,
                BCHP_FIELD_DATA(MEM_DMA_0_FIRST_DESC, ADDR, ctx->firstDescOffset));
            BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
                BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, true));
            break;
        }
    }
    BDBG_MSG_TRACE(("check done"));
    return;
}

/* this function takes into account the fact that we may not receive one interrupt for each context that finished */
static void
BMMD_P_UpdateStatus_isr(BMMD_Handle mmd, bool isr)
{
    uint32_t data;
    BMMD_ContextHandle ctx;
#if (BMMD_P_DMA_REV_2A)
    BSTD_UNUSED(isr);
#endif

    data = BREG_Read32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_STATUS);
    switch (BCHP_GET_FIELD_DATA(data, MEM_DMA_0_STATUS, DMA_STATUS)) {
        default:
        case BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Idle:
        case BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep:
            BDBG_MSG_TRACE(("update: idle %u", data));
            BMMD_P_CheckAll_isr(mmd);
            break;
        case BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy:
            BDBG_MSG_TRACE(("update: busy"));
            while (NULL!=(ctx = BLST_SQ_FIRST(&mmd->activeCtxList))) {
                if (BMMD_Context_P_IsComplete_isr(ctx)) {
                    BDBG_MSG_TRACE(("%#x notify from busy", ctx->firstDescOffset));
                    BMMD_Context_P_NotifyFirst_isr(mmd, ctx);
                }
                else {
#if (!BMMD_P_DMA_REV_2A)
                    /* there appears to be a quirky race condition on older platforms where,
                       when two sgScram jobs are queued in quick succession, an ISR fires while the HW is busy.
                       there could be two possibilities:
                       a) the first job's ISR fired late, i.e. after the second job was restarted. but the second job's ISR does not fire later for whatever reason.
                       b) the second job's ISR fired while the HW is still busy.
                       below is the workaround */
                    if (isr && ctx->sg) {
                        BKNI_Delay(1);
                        data = BREG_Read32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_STATUS);
                        if (BCHP_GET_FIELD_DATA(data, MEM_DMA_0_STATUS, DMA_STATUS)==BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep) {
                            BDBG_MSG(("%#x UpdateStatus while busy->sleep", ctx->firstDescOffset));
                            BMMD_P_CheckAll_isr(mmd);
                        }
                    }
#endif
                    break;
                }
            }
            break;
    }
    return;
}

static void BMMD_P_CompleteCallback_isr(void *pParam1, int parm2)
{
    BMMD_Handle mmd = (BMMD_Handle)pParam1;
    BSTD_UNUSED(parm2);

    BDBG_OBJECT_ASSERT(mmd, BMMD_Handle_Tag);
    BDBG_MSG_TRACE(("ISR"));

    /* ISR could fire after all contexts have been destroyed. short-circuit to avoid register access */
    if (BLST_S_EMPTY(&mmd->ctxList)) { return; }

#if 0
    BDBG_MSG_TRACE(("CUR_DESC %#x", BREG_Read32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_CUR_DESC)));
#endif
    BMMD_P_UpdateStatus_isr(mmd, true);
    return;
}

static BERR_Code
BMMD_Context_P_Start_isr(BMMD_Handle mmd, BMMD_ContextHandle ctx)
{
    uint32_t data;
    unsigned status;
    BMMD_ContextHandle lastCtx = mmd->lastQueuedCtx;
    uint32_t qmdOffset = ctx->lastDescOffset; /* offset of QMD. the magic number is written to word[5] of previous desc */
    bool empty = (BLST_SQ_FIRST(&mmd->activeCtxList)==NULL);

    BSTD_UNUSED(qmdOffset); /* silence unused variable warning */
    mmd->lastQueuedCtx = ctx;
    data = BREG_Read32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_STATUS);
    status = BCHP_GET_FIELD_DATA(data, MEM_DMA_0_STATUS, DMA_STATUS);

    if (empty || !lastCtx) {
#if BMMD_DEBUG_MODE
        BDBG_ASSERT(status != BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy); /* empty has to imply idle/sleep */
        BDBG_ASSERT(mmd->numSgCtx==0);
#else
        if (status == BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy) {
            BDBG_WRN(("%#x start while BUSY", ctx->firstDescOffset));
            /* activeCtxList may need to be cleared, if we got here via (!lastCtx) */
        }
        if (mmd->numSgCtx) {
            BDBG_WRN(("%#x start with sg %u", ctx->firstDescOffset, mmd->numSgCtx));
            mmd->numSgCtx = 0;
        }
#endif

        BDBG_MSG_TRACE(("%#x start  %u, QMD %#x", ctx->firstDescOffset, status, qmdOffset));
        BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
            BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, false));
        BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_FIRST_DESC,
            BCHP_FIELD_DATA(MEM_DMA_0_FIRST_DESC, ADDR, ctx->firstDescOffset));
        BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
            BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, true));

        return BERR_SUCCESS;
    }
    else {
#if BMMD_P_AVOID_WAKE /* avoid using WAKE for all platforms */
        BDBG_MSG_TRACE(("%#x postpone, QMD %#x", ctx->firstDescOffset, qmdOffset));
#else
        uint32_t data = lastCtx->dmaDesc_3;

        if (mmd->numSgCtx==0) { /* always true for 40nm platforms */
            BDBG_MSG_TRACE(("%#x link   %u, QMD %#x", ctx->firstDescOffset, status, qmdOffset));
            /* update NEXT_DESC_ADDR, clear LAST in previous descriptor and hit WAKE, in that order */
            lastCtx->lastDescCached[3] = data | BCHP_FIELD_DATA(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, ctx->firstDescOffset>>BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);
            lastCtx->lastDescCached[2] &= ~(BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, LAST, 1));
            BMEM_FlushCache_isr(ctx->parent->mem, lastCtx->lastDescCached, BMMD_P_DESC_SIZE);
            BREG_Write32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_WAKE_CTRL,
                BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE, 1));
        }
        else {
            BDBG_MSG_TRACE(("%#x postpone, QMD %#x", ctx->firstDescOffset, qmdOffset));
        }
#endif
        /* mmd->numSgCtx++ takes place in Enqueue() */
        return BMMD_QUEUED;
    }
}

static BERR_Code BMMD_Context_P_PrepareBlocks(
    BMMD_ContextHandle ctx,
    const BMMD_ContextBlockSettings *pSettings,
    unsigned numBlocks
    )
{
    unsigned i;
    uint32_t *descAddr;
    uint32_t nextDescOffset, data;
    unsigned dmaLength;
    bool sg;
    BMMD_EngineType engineType = ctx->parent->settings.engineType;
    BMMD_ContextHandle context;
    bool sgStart = false;

    ctx->sg = false;

    for (dmaLength=0, sg=false, i=0,
         nextDescOffset=ctx->firstDescOffset>>BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT,
         descAddr=ctx->firstDescCached; ; pSettings++)
    {

        if (pSettings->size>BMMD_P_MAX_TRANSFER_SIZE) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

#if BMMD_P_DESC_PROTECT
        /* check if queued descriptor reads from / writes to memory locations where BMMD contexts' descriptors are located */
        for (context=BLST_S_FIRST(&ctx->parent->ctxList); context; context=BLST_S_NEXT(context, ctxNode)) {
            uint32_t beg = context->firstDescOffset; /* inclusive */
            uint32_t end = context->firstDescOffset + BMMD_P_DESC_SIZE*(context->settings.maxNumBlocks+1); /* not inclusive. don't use lastDescOffset because a) it's not populated yet, b) it's dependent on how many descs you queue */

            if (pSettings->src >= beg && pSettings->src < end) {
                BDBG_WRN(("%#x: src %#x reads from BMMD private descriptor memory location", ctx->firstDescOffset, pSettings->src));
                /* it's a warning, but continue */
            }

            if (pSettings->dst >= beg && pSettings->dst < end) {
                BDBG_ERR(("%#x: dst %#x writes to BMMD private descriptor memory location", ctx->firstDescOffset, pSettings->dst));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
        /* do a similar check for the QMD const location */
        if (pSettings->dst >= ctx->parent->qmdConstOffset && pSettings->dst < ctx->parent->qmdConstOffset+4) {
            BDBG_ERR(("%#x: dst %#x writes to BMMD private const memory location", ctx->firstDescOffset, pSettings->dst));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
#endif

        if (ctx->settings.memoryBounds.offset && ctx->settings.memoryBounds.size) {
            uint32_t b_offset = ctx->settings.memoryBounds.offset;
            unsigned b_size = ctx->settings.memoryBounds.size;

            if (pSettings->src < b_offset || (pSettings->src+pSettings->size > b_offset+b_size)) {
                BDBG_ERR(("%#x: src violates memory bounds %#x:%#x %#x:%#x", ctx->firstDescOffset,
                    pSettings->src, pSettings->size, b_offset, b_size));
                return BERR_INVALID_PARAMETER;
            }

            if (pSettings->dst < b_offset || (pSettings->dst+pSettings->size > b_offset+b_size)) {
                BDBG_ERR(("%#x: dst violates memory bounds %#x:%#x %#x:%#x", ctx->firstDescOffset,
                    pSettings->dst, pSettings->size, b_offset, b_size));
                return BERR_INVALID_PARAMETER;
            }
        }
        if (pSettings->sgScramStart) {
            if (sgStart) {
                BDBG_ERR(("%#x: unpaired sgScramStart flag", ctx->firstDescOffset));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            sgStart = true;
        }
        if (pSettings->sgScramEnd) {
            if (!sgStart) {
                BDBG_ERR(("%#x: unpaired sgScramEnd flag", ctx->firstDescOffset));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            sgStart = false;
        }

        if (i==numBlocks-1 && sgStart) {
            BDBG_ERR(("%#x: unpaired sgScramStart flag", ctx->firstDescOffset));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        dmaLength += pSettings->size;
        nextDescOffset+=(BMMD_P_DESC_SIZE>>BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);

        descAddr[0] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD0, READ_ADDR, pSettings->src);
        descAddr[1] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD1, WRITE_ADDR, pSettings->dst);
        descAddr[2] =
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, TRANSFER_SIZE, pSettings->size) |
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, INTR_ENABLE, false) |
            BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, LAST, false);
        descAddr[3] = ctx->dmaDesc_3 | BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, nextDescOffset);
        descAddr[4] = ctx->dmaDesc_4;

        if (engineType==BMMD_EngineType_eDma && ctx->settings.scramMode>BMMD_ScramMode_eNone) {
            data = ctx->dmaDesc_4;
            if (pSettings->resetCrypto) {
                data |= BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, ENC_DEC_INIT, 1);
            }
            if (pSettings->sgScramStart) {
                data |= BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, SG_SCRAM_START, 1);
                sg = true;
                ctx->sg = true;
            }
            if (sg) {
                /* SG_ENABLE must only be 1 between START and END.
                   not only is this the cause of HW hangs (65nm platforms), there is also a performance penalty.
                   further, with SG, the CUR_DESC can no longer reliably used as a status register. however, in our
                   current usage of the HW, this doesn't matter; we will only start running into problems if
                   INTR_ENABLE is true in the middle of a SG crypto chain */
                data |= BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, SG_ENABLE, 1);
            }
            if (pSettings->sgScramEnd) {
                data |= BCHP_FIELD_DATA(MEM_DMA_DESC_WORD4, SG_SCRAM_END, 1);
                sg = false;
#if (!BMMD_P_DMA_REV_2A)
                if (i!=numBlocks-1) {
                    BDBG_ERR(("SG crypto to clear transition within a single chain is not supported on this platform"));
                    return BERR_INVALID_PARAMETER;
                }
#endif
            }
            descAddr[4] = data;
        }
        else if (engineType==BMMD_EngineType_eSharf && ctx->settings.sharf.mode>BMMD_SharfMode_ePassThrough) {
            data = ctx->dmaDesc_4;
            data |= BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, KEY_PRESENT,    (unsigned)pSettings->sharf.keyPresent);
            data |= BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, DIGEST_PRESENT, (unsigned)pSettings->sharf.digestPresent);
            if (pSettings->sgScramStart) {
                data |= BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_START, 1);
                sg = true;
                ctx->sg = true;
            }
            if (sg) {
                data |= BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, SG_ENABLE, 1);
            }
            if (pSettings->sgScramEnd) {
                data |= BCHP_FIELD_DATA(SHARF_MEM_DMA_DESC_WORD4, SG_SCRAM_END, 1);
                sg = false;
#if (!BMMD_P_DMA_REV_2A)
                if (i!=numBlocks-1) {
                    BDBG_ERR(("SG crypto to clear transition within a single chain is not supported on this platform"));
                    return BERR_INVALID_PARAMETER;
                }
#endif
            }
            descAddr[4] = data;
        }

        descAddr[5] = 0; /* QMD destination must be cleared */

        descAddr+=BMMD_P_DESC_SIZE/sizeof(uint32_t);
        i++;
        if (i==numBlocks) {
            /* save length of DMA transaction */
            ctx->dmaLength = dmaLength;
            /* save address of nextDescOffset for further use in range check */
            ctx->lastDescOffset = (nextDescOffset<<BCHP_MEM_DMA_DESC_WORD3_NEXT_DESC_ADDR_SHIFT);
            /* save address of lastDesc for further use in linking */
            ctx->lastDescCached = descAddr;

            /* if non-40nm platform AND sgScram descriptor, then use empty descriptor at end. otherwise, use QMD */
#if (!BMMD_P_DMA_REV_2A)
            if (ctx->sg) {
                data = ctx->dmaDesc_3;
                /* create an empty descriptor at the end of transaction */
                    descAddr[0] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD0, READ_ADDR, nextDescOffset);
                    descAddr[1] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD1, WRITE_ADDR, nextDescOffset);
                descAddr[2] =
                    BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, TRANSFER_SIZE, 0) |
                    BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, INTR_ENABLE, true) |
                    BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, LAST, true);
                descAddr[3] = data | BCHP_FIELD_DATA( MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, nextDescOffset);
                descAddr[4] = 0;
                break;
            }
#endif

            /* QMD */
            descAddr[0] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD0, READ_ADDR, ctx->parent->qmdConstOffset);
            descAddr[1] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD1, WRITE_ADDR, ctx->lastDescOffset-BMMD_P_DESC_SIZE+(4*5)); /* DESC_WORD5 of last descriptor */
            descAddr[2] =
                BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, TRANSFER_SIZE, 4) |
                BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, INTR_ENABLE, true) |
                BCHP_FIELD_DATA(MEM_DMA_DESC_WORD2, LAST, true);
            descAddr[3] = BCHP_FIELD_DATA(MEM_DMA_DESC_WORD3, NEXT_DESC_ADDR, nextDescOffset); /* WRITE_ENDIAN_XLATE mode has to be fixed. READ_ENDIAN_MODE does not matter */
            descAddr[4] = 0; /* not sgScram */
            descAddr[5] = 0;
            break;
        }
    }

    BMEM_FlushCache(ctx->parent->mem, ctx->firstDescCached, (numBlocks+1)*BMMD_P_DESC_SIZE); /* +1 for QMD */
    return BERR_SUCCESS;
}


/* this is the only function that triggers BLST_SQ_INSERT_TAIL */
BERR_Code BMMD_Context_Enqueue(
    BMMD_ContextHandle hCtx,
    const BMMD_ContextBlockSettings *pSettings,
    unsigned numBlocks
    )
{
    BERR_Code rc;
    BMMD_Handle mmd;
    uint32_t data;

    if (hCtx->parent->standby) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (numBlocks==0 || numBlocks > hCtx->settings.maxNumBlocks ) {
        BDBG_ERR(("numBlocks %u out of range - max=%u", numBlocks, hCtx->settings.maxNumBlocks));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* a single context can only be enqueued once */
    if (hCtx->state != BMMD_Context_P_State_eIdle) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if (pSettings==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    rc = BMMD_Context_P_PrepareBlocks(hCtx, pSettings, numBlocks);
    if (rc!=BERR_SUCCESS) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#if 0 /* for debug */
    BMMD_Context_P_DescDump(hCtx, numBlocks);
#endif

    mmd = hCtx->parent;
    BKNI_EnterCriticalSection();
    (void)BMMD_Context_P_Start_isr(mmd, hCtx);

    hCtx->state = BMMD_Context_P_State_eQueued;
    data = BREG_Read32(mmd->reg, mmd->baseRegOffset+BCHP_MEM_DMA_0_STATUS);
    switch (BCHP_GET_FIELD_DATA(data, MEM_DMA_0_STATUS, DMA_STATUS)) {
        case BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Busy:
#if (!BMMD_P_DMA_REV_2A)
            if (hCtx->sg) {
                mmd->numSgCtx++;
            }
#endif
            BLST_SQ_INSERT_TAIL(&mmd->activeCtxList, hCtx, activeNode);
            break;
        case BCHP_MEM_DMA_0_STATUS_DMA_STATUS_Sleep:
            if (BMMD_Context_P_IsComplete_isr(hCtx)) {
                BDBG_MSG_TRACE(("%#x slow/fast complete", hCtx->firstDescOffset));
                hCtx->state = BMMD_Context_P_State_eIdle; /* don't even need to add into the list */
                BKNI_LeaveCriticalSection();
                goto completed;
            }
            /* keep going */
        default:
            /* add into the list and run normal completion sequence */
            BDBG_MSG_TRACE(("%#x fastcheck", hCtx->firstDescOffset));
#if BMMD_DEBUG_MODE
#if BMMD_P_DMA_REV_2A
            BDBG_ASSERT(0);
#endif
#endif
#if (!BMMD_P_DMA_REV_2A)
            if (hCtx->sg) {
                mmd->numSgCtx++;
            }
#endif
            BLST_SQ_INSERT_TAIL(&mmd->activeCtxList, hCtx, activeNode);
            BMMD_P_CheckAll_isr(mmd);
            break;
    }
    BKNI_LeaveCriticalSection();
    return BMMD_QUEUED;

completed:
    return BERR_SUCCESS;
}

BERR_Code BMMD_Context_GetStatus(
    BMMD_ContextHandle hCtx,
    BMMD_ContextStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hCtx, BMMD_Context);
    BDBG_ASSERT(NULL != pStatus);

    if (hCtx->state == BMMD_Context_P_State_eDestroyed) {
        return BERR_NOT_SUPPORTED;
    }

    if (hCtx->state != BMMD_Context_P_State_eIdle) {
        BKNI_EnterCriticalSection();
        BDBG_MSG_TRACE(("UpdateStatus"));
        BMMD_P_UpdateStatus_isr(hCtx->parent, false);
        BKNI_LeaveCriticalSection();
    }
    if (hCtx->state == BMMD_Context_P_State_eIdle) {
        pStatus->state = BMMD_ContextState_eIdle;
    }
    else {
        pStatus->state = BMMD_ContextState_eInProgress;
    }
    return BERR_SUCCESS;
}

void BMMD_Context_P_Destroy(
    BMMD_ContextHandle hCtx
    )
{
    if (hCtx->parent->lastQueuedCtx == hCtx) { /* BMMD_Context_P_Start_isr() may try to use lastQueuedCtx, so clear it */
        hCtx->parent->lastQueuedCtx = NULL;
    }
    BDBG_MSG_TRACE(("%p destroy", (void*)hCtx));
    BLST_S_REMOVE(&(hCtx->parent->ctxList), hCtx, BMMD_Context, ctxNode);
    BMEM_FreeCached(hCtx->parent->mem, hCtx->firstDescCached);
#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_ReleaseResource(hCtx->parent->chp, BCHP_PWR_RESOURCE_DMA);
#endif
    BDBG_OBJECT_DESTROY(hCtx, BMMD_Context);
    BKNI_Free(hCtx);
}

void BMMD_Context_Destroy(
    BMMD_ContextHandle hCtx
    )
{
    BMMD_ContextStatus status;
    BMMD_Context_GetStatus(hCtx, &status);

    /* if not idle, then mark it as destroyed, but defer the actual destroy to BMMD_Close() */
    if (hCtx->state!=BMMD_Context_P_State_eIdle) {
#if BMMD_DEBUG_MODE
        BDBG_ASSERT(0); /* don't allow */
#endif
        BDBG_ERR(("BMMD_Context_Destroy: %p is busy, leaking context", (void*)hCtx));
        /* when marked as destroyed, as long as the context's descriptor memory is alive,
           BMMD can figure out the completion status and remove it from activeCtxList, if needed */
        hCtx->state = BMMD_Context_P_State_eDestroyed;
    }
    else {
        /* otherwise, really destroy */
        BMMD_Context_P_Destroy(hCtx);
    }
}

void BMMD_Close(
    BMMD_Handle hMmd
    )
{
    BMMD_ContextHandle ctx;
    BDBG_OBJECT_ASSERT(hMmd, BMMD_Handle_Tag);

    /* wait for pending operations to finish */
    if (BLST_SQ_FIRST(&hMmd->activeCtxList)) { /* Context_Destroy() never actually destroys any contexts that are part of activeCtxList */
        BDBG_WRN(("MMD still busy. Waiting before forced stop"));
        BKNI_Sleep(100);

        /* stop HW. powering down the DMA clock while the HW is running can lead to an unrecoverable HW state  */
        BREG_Write32(hMmd->reg, hMmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL, BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, false));
    }

    BINT_DestroyCallback(hMmd->irq);

    /* all pending operations are done. shutdown */
    while ((ctx = BLST_S_FIRST(&hMmd->ctxList))) {
        BDBG_WRN(("BMMD_Close: stale context %p (%s)", (void*)ctx, ctx->state==BMMD_Context_P_State_eIdle?"idle":"active"));
        BMMD_Context_P_Destroy(ctx);
    }

    BMEM_FreeCached(hMmd->mem, hMmd->qmdConstCached);
    BDBG_OBJECT_UNSET(hMmd, BMMD_Handle_Tag);
    BKNI_Free(hMmd);
}

BERR_Code BMMD_Standby(
    BMMD_Handle hMmd,
    const BMMD_StandbySettings *pSettings
    )
{
    BERR_Code rc;
    BSTD_UNUSED(pSettings);
    BDBG_MSG(("BMMD_Standby: %s %u", hMmd->settings.engineType==BMMD_EngineType_eDma?"DMA":"SHARF", hMmd->settings.engineIndex));

    if (hMmd->standby) {
        /* handle it silently */
        BDBG_MSG(("Already in standby"));
        return BERR_INVALID_PARAMETER;
    }
    /* cannot enter standby while mmd is busy */
    if (BLST_SQ_FIRST(&hMmd->activeCtxList)) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BINT_DisableCallback(hMmd->irq);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); /* keep going */ }

#ifdef BCHP_PWR_RESOURCE_DMA
{
    BMMD_ContextHandle context;
    for (context=BLST_S_FIRST(&hMmd->ctxList); context; context=BLST_S_NEXT(context, ctxNode)) {
        /* DMA must be shutdown */
        BCHP_PWR_ReleaseResource(hMmd->chp, BCHP_PWR_RESOURCE_DMA);
    }
}
#endif
    hMmd->standby = true;
    /* no state to remember / restore */
    return BERR_SUCCESS;
}

void BMMD_Resume(
    BMMD_Handle hMmd
    )
{
    BERR_Code rc;
    BDBG_MSG(("BMMD_Resume: %s %u", hMmd->settings.engineType==BMMD_EngineType_eDma?"DMA":"SHARF", hMmd->settings.engineIndex));
    if (!hMmd->standby) {
        /* handle it silently */
        BDBG_MSG(("Not in standby"));
        return;
    }

#ifdef BCHP_PWR_RESOURCE_DMA
{
    BMMD_ContextHandle context;
    for (context=BLST_S_FIRST(&hMmd->ctxList); context; context=BLST_S_NEXT(context, ctxNode)) {
        BCHP_PWR_AcquireResource(hMmd->chp, BCHP_PWR_RESOURCE_DMA);
    }
}
#endif

    /* replicate the register writes done in BMMD_Open */
#ifdef BCHP_PWR_RESOURCE_DMA
    /* must acquire again in case there were no contexts open */
    BCHP_PWR_AcquireResource(hMmd->chp, BCHP_PWR_RESOURCE_DMA);
#endif
    BREG_Write32(hMmd->reg, hMmd->baseRegOffset+BCHP_MEM_DMA_0_CTRL,
        BCHP_FIELD_DATA(MEM_DMA_0_CTRL, RUN, false)); /* stop DMA on open */
    BREG_Write32(hMmd->reg, hMmd->baseRegOffset+BCHP_MEM_DMA_0_FIRST_DESC,
        BCHP_FIELD_DATA(MEM_DMA_0_FIRST_DESC, ADDR, 0));
    BREG_Write32(hMmd->reg, hMmd->baseRegOffset+BCHP_MEM_DMA_0_WAKE_CTRL,
        BCHP_FIELD_DATA(MEM_DMA_0_WAKE_CTRL, WAKE_MODE, 1)); /* always use wake_from_last */
#if BMMD_P_HAS_LMC_CORE
    BREG_Write32(hMmd->reg, BCHP_SHARF_LMC_CORE_SWITCH_MEMC,
        BCHP_FIELD_DATA(SHARF_LMC_CORE_SWITCH_MEMC, MUX_SELECT, 1)); /* always point to MEMC, not LMC */
#endif

    rc = BINT_EnableCallback(hMmd->irq);
    if (rc != BERR_SUCCESS) { rc = BERR_TRACE(rc); /* keep going */ }

#ifdef BCHP_PWR_RESOURCE_DMA
    BCHP_PWR_ReleaseResource(hMmd->chp, BCHP_PWR_RESOURCE_DMA);
#endif

    hMmd->standby = false;
    return;
}

void BMMD_Context_P_DescDump(
    BMMD_ContextHandle ctx,
    unsigned numBlocks
    )
{
    unsigned i;
    uint32_t *descAddr, descOffset;

    BMEM_FlushCache(ctx->parent->mem, ctx->firstDescCached, (numBlocks+1)*BMMD_P_DESC_SIZE); /* +1 for QMD */

    for (i=0,descAddr=ctx->firstDescCached; i<=numBlocks; i++,descAddr+=BMMD_P_DESC_SIZE/sizeof(uint32_t)) {
        descOffset = ctx->firstDescOffset+BMMD_P_DESC_SIZE*i;
        BKNI_Printf("desc[%d:%08x] = %08x, %08x, %08x, %08x, %08x\n",
            i, descOffset,
            descAddr[0], descAddr[1], descAddr[2], descAddr[3], descAddr[4]);
    }
    BKNI_Printf("\n");
}

