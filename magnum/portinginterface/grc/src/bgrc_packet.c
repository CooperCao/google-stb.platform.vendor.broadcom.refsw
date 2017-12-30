/***************************************************************************
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
 *
 * Module Description: GRC Packet API
 *
 ***************************************************************************/

#include "bstd.h"
#include "bstd_defs.h"
#include "berr.h"
#include "bkni.h"
#include "bint.h"
#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"
#include "bgrc_errors.h"
#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#endif

/* bchp_common.h affects the folloing #include files */
#include "bchp_common.h"

#ifdef BCHP_M2MC1_REG_START
#include "bchp_m2mc_l2.h"
#include "bchp_m2mc1_l2.h"
#include "bchp_m2mc_gr.h"
#include "bchp_m2mc1_gr.h"
#include "bchp_int_id_m2mc1_l2.h"
#endif
#ifdef BCHP_MM_M2MC0_REG_START
#include "bchp_mm_m2mc0.h"
#include "bchp_mm_m2mc0_l2.h"
#include "bchp_int_id_mm_m2mc0_l2.h"
#endif

/* Needed to determine UIF memory configuration where supported */
#if (BGRC_P_VER >= BGRC_P_VER_2)
#include "bchp_memc_ddr_0.h"
#endif

#ifdef BCHP_MEMC16_GFX_L2_REG_START
#include "bchp_memc16_gfx_l2.h"
#endif
#ifdef BCHP_MEMC16_GFX_GRB_REG_START
#include "bchp_memc16_gfx_grb.h"
#endif
#ifdef BCHP_MEMC_GFX_GRB_REG_START
#include "bchp_memc_gfx_grb.h"
#endif
#ifdef BCHP_GRAPHICS_L2_REG_START
#include "bchp_graphics_l2.h"
#include "bchp_graphics_grb.h"
#endif
#ifdef BCHP_GFX_L2_REG_START
#include "bchp_gfx_l2.h"
#include "bchp_gfx_gr.h"
#endif
#ifdef BCHP_M2MC_TOP_L2_REG_START
#include "bchp_m2mc_top_l2.h"
#include "bchp_m2mc_top_gr_bridge.h"
#endif
#ifdef BCHP_M2MC_WRAP_L2_REG_START
#include "bchp_m2mc_wrap_l2.h"
#include "bchp_m2mc_wrap_gr_bridge.h"
#endif
#ifdef BCHP_WRAP_M2MC_L2_REG_START
#include "bchp_wrap_m2mc_l2.h"
#include "bchp_wrap_m2mc_grb.h"
#include "bchp_gfx_grb.h"
#endif
#ifdef BCHP_MEMC_1_1_REG_START
#include "bchp_memc_1_1.h"
#endif
#ifdef BCHP_M2MC_GR_REG_START
#include "bchp_m2mc_gr.h"
#endif
#ifdef BCHP_M2MC_L2_REG_START
#include "bchp_m2mc_l2.h"
#endif
#ifdef BCHP_SUN_TOP_CTRL_REG_START
#include "bchp_sun_top_ctrl.h"
#endif

#include "bgrc.h"
#include "bgrc_packet.h"
#include "bgrc_private.h"
#include "bgrc_packet_priv.h"

#if (BGRC_P_VER >= BGRC_P_VER_2)
#include "bgrc_mmpacket_priv.h"
#endif


#if BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT
#include "bchp_m2mc_cxt_sch.h"
#include "bchp_mm_m2mc_cxt_sch.h"
#endif
BDBG_MODULE(BGRC);
BDBG_FILE_MODULE(BGRC_LISTSTATUS);
BDBG_FILE_MODULE(BGRC_MIPMAP);
BDBG_FILE_MODULE(BGRC_SWPKT_FIFO);
BDBG_OBJECT_ID(BGRC);
BDBG_OBJECT_ID(BGRC_PacketContext);

/***************************************************************************/
#if defined(BCHP_INT_ID_GFX_L2_M2MC_INTR)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_GFX_L2_M2MC_INTR
#elif defined(BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_0_INTR_SHIFT)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_MEMC16_GFX_L2_CPU_STATUS, BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_0_INTR_SHIFT)
#elif defined(BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_MEMC16_GFX_L2_CPU_STATUS, BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#elif defined(BCHP_INT_ID_M2MC_0_INTR)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_M2MC_0_INTR
#elif defined(BCHP_GRAPHICS_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_GRAPHICS_L2_CPU_STATUS, BCHP_GRAPHICS_L2_CPU_STATUS_M2MC_0_INTR_SHIFT)
#elif defined(BCHP_GFX_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_GFX_L2_CPU_STATUS, BCHP_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#elif defined(BCHP_M2MC_WRAP_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_M2MC_WRAP_L2_CPU_STATUS, BCHP_M2MC_WRAP_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#elif defined(BCHP_M2MC_TOP_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_M2MC_TOP_L2_CPU_STATUS, BCHP_M2MC_TOP_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#elif defined(BCHP_WRAP_M2MC_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_WRAP_M2MC_L2_CPU_STATUS, BCHP_WRAP_M2MC_L2_CPU_STATUS_M2MC_0_INTR_SHIFT)
#elif defined(BCHP_M2MC_L2_CPU_STATUS)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_CREATE(BCHP_M2MC_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#elif defined(BCHP_INT_ID_M2MC_INTR)
#define BGRC_PACKET_P_M2MC0_INT_ID   BCHP_INT_ID_M2MC_INTR
#else
#error need port this chip for intID
#endif

#if defined(BCHP_M2MC1_REG_START)
#define BGRC_PACKET_P_M2MC1_INT_ID   BCHP_INT_ID_M2MC1_L2_M2MC_INTR
#else
#define BGRC_PACKET_P_M2MC1_INT_ID
#endif

#if defined(BCHP_MM_M2MC0_REG_START)
#define BGRC_PACKET_P_MM_M2MC0_INT_ID BCHP_INT_ID_MM_M2MC0_L2_M2MC_INTR
#else
#define BGRC_PACKET_P_MM_M2MC0_INT_ID
#endif

#if BCHP_PWR_SUPPORT
#if defined(BCHP_PWR_RESOURCE_M2MC1)
#define BGRC_PWR_RESOURCE_M2MC1      BCHP_PWR_RESOURCE_M2MC1
#else
#define BGRC_PWR_RESOURCE_M2MC1      BCHP_PWR_RESOURCE_M2MC
#endif
#endif

/***************************************************************************/
static const BGRC_Settings BGRC_P_DEFAULT_SETTINGS =
{
    BGRC_PACKET_MEMORY_MAX,   /* ulPacketMemoryMax */
    BGRC_OPERATION_MAX,       /* ulOperationMax */
    0,                        /* ulDeviceNum */
    BGRC_WAIT_TIMEOUT,        /* ulWaitTimeout */
    true                      /* bPreAllocMemory */
};

/***************************************************************************/
BERR_Code BGRC_GetDefaultSettings(
    BGRC_Settings *pDefSettings )
{
    BDBG_ENTER(BGRC_GetDefaultSettings);

    if( pDefSettings )
        *pDefSettings = BGRC_P_DEFAULT_SETTINGS;

    BDBG_LEAVE(BGRC_GetDefaultSettings);
    return BERR_SUCCESS;
}
/***************************************************************************/
void BGRC_P_SetMemoryInfo(
    BGRC_Handle hGrc )
{
#if (BGRC_P_VER >= BGRC_P_VER_2)
    uint32_t  ulPage, ulBankValue /* bank value to set register field*/, ulBank /* true bank info*/;

    /* PAGE_SIZE
        00 = 2k SDRAM Page
        01 = 4k SDRAM Page
        10 = 8k SDRAM Page
    */

    switch(hGrc->stMemInfo.memc[0].ulPageSize/1024)
    {
        case 2:
        default:
            ulPage = 0;
            break;
        case 4:
            ulPage = 1;
            break;
        case 8:
            ulPage = 2;
            break;
    }
    /* NUM_BANKS
        Number of SDRAM Banks
        00 = 4
        01 = 8
        10 = 16
        11 = 32
          */
    switch(BCHP_MEMC_DDR_0_CNTRLR_CONFIG_2_BANK_BITS_DEFAULT)
    {
        case 3:
            ulBankValue = 1;
            ulBank = 8;
            break;
        case 4:
            ulBankValue = 2;
            ulBank = 16;
            break;
        case 5:
            ulBankValue = 3;
            ulBank = 32;
            break;
        case 2:
        default:
            ulBankValue = 0;
            ulBank = 4;
            break;
    }

    hGrc->stPxlMemoryInfo.ulPageSize = hGrc->stMemInfo.memc[0].ulPageSize;
    hGrc->stPxlMemoryInfo.ulPageinUBRows = hGrc->stMemInfo.memc[0].ulPageSize/(BPXL_UIF_BLOCKSIZE * BPXL_UIF_COL_WIDTH_IN_BLOCK);
    hGrc->stPxlMemoryInfo.ul15PageinUBRows = hGrc->stPxlMemoryInfo.ulPageinUBRows*3/2;    /*1.5  UIFBlockRows */
    /* How many UIF-block rows the "page cache" covers */
    hGrc->stPxlMemoryInfo.ulPcInUBRows = ulBank*hGrc->stPxlMemoryInfo.ulPageinUBRows;
    hGrc->stPxlMemoryInfo.ulPc15InUBRows = hGrc->stPxlMemoryInfo.ulPcInUBRows - hGrc->stPxlMemoryInfo.ul15PageinUBRows;

    BDBG_MODULE_MSG(BGRC_MIPMAP, ("M2MC[%d] Memory configuration: ", hGrc->eDeviceNum));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("page size        %8d ", hGrc->stPxlMemoryInfo.ulPageSize));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("bank             %8d ", ulBank));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("ulPageinUBRows   %8d ", hGrc->stPxlMemoryInfo.ulPageinUBRows));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("ul15PageinUBRows %8d ", hGrc->stPxlMemoryInfo.ul15PageinUBRows));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("ulPcInUBRows     %8d ", hGrc->stPxlMemoryInfo.ulPcInUBRows));
    BDBG_MODULE_MSG(BGRC_MIPMAP, ("ulPc15InUBRows   %8d ", hGrc->stPxlMemoryInfo.ulPc15InUBRows));

#if (BGRC_P_VER >= BGRC_P_VER_3)
    /* current 7278 setting
    -DGFX_DEFAULT_UIF_PAGE_SIZE=4096 \
    -DGFX_DEFAULT_UIF_NUM_BANKS=8 \
    -DGFX_DEFAULT_UIF_XOR_ADDR=16 \
    */
    if(hGrc->eDeviceNum == BGRC_eMM_M2mc0){
        BREG_Write32( hGrc->hRegister, BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG,
            BCHP_FIELD_DATA(MM_M2MC0_MM_M2MC_SYSTEM_CONFIG,NUM_BANKS, ulBankValue)|
            BCHP_FIELD_DATA(MM_M2MC0_MM_M2MC_SYSTEM_CONFIG,PAGE_SIZE, ulPage)|
            /* @@@ to do: get the right formula from HW and v3d*/
            BCHP_FIELD_DATA(MM_M2MC0_MM_M2MC_SYSTEM_CONFIG,XOR_ADDR , 16));
        BDBG_MODULE_MSG(BGRC_MIPMAP, ("ulBank %d ulPage %d", ulBank, ulPage));
    }
#else
    BSTD_UNUSED(ulBankValue);
    BSTD_UNUSED(ulPage);
#endif /* BGRC_P_VER >= BGRC_P_VER_3 */

#else
    BSTD_UNUSED(hGrc);
#endif
}

/***************************************************************************/
void BGRC_P_ResetDevice(
    BGRC_Handle hGrc )
{

    uint32_t ulRegOffset = hGrc->ulRegOffset;

#if defined(BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0)
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
    BSTD_UNUSED(ulRegOffset);
#else
    {
        /* we are using m2mc 1 */
#if defined(BCHP_GFX_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_ASSERT)
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
        hGrc->ulRegCoreReset = 0;
#elif BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_ASSERT
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0 + ulRegOffset, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0 + ulRegOffset, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT));
        hGrc->ulRegCoreReset = 0;
#endif
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL + ulRegOffset, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_MASK );
        while( BREG_Read32(hGrc->hRegister, BCHP_M2MC_BLIT_STATUS+ulRegOffset) == 0 );
        if(hGrc->ulRegCoreReset)
        {
            BREG_Write32( hGrc->hRegister, hGrc->ulRegCoreReset,   hGrc->ulCoreResetMask);
            BREG_Write32( hGrc->hRegister, hGrc->ulRegClearCore,   hGrc->ulCoreResetMask);
        }
#if defined(BCHP_GFX_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_ASSERT)
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
#elif BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_ASSERT
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0 + ulRegOffset, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0 + ulRegOffset, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT));
#endif

        BREG_Write32( hGrc->hRegister, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL + ulRegOffset, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL_INTER_BLT_CLK_GATE_ENABLE_MASK );

#if BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT
        BREG_Write32(hGrc->hRegister, hGrc->ulWeightReg, BCHP_FIELD_DATA(M2MC_CXT_SCH_WEIGHT_CONTEXT0, WEIGHT, hGrc->ulWeight));
#endif

#ifdef BCHP_M2MC_DITHER_CONTROL_0
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_DITHER_CONTROL_0 + ulRegOffset,
            BCHP_FIELD_ENUM(M2MC_DITHER_CONTROL_0, MODE, DITHER) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, OFFSET_CH2, 1) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, OFFSET_CH1, 1) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, OFFSET_CH0, 1) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, SCALE_CH2, 4) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, SCALE_CH1, 4) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_0, SCALE_CH0, 4));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_DITHER_CONTROL_1 + ulRegOffset,
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_1, OFFSET_CH3, 1) |
            BCHP_FIELD_DATA(M2MC_DITHER_CONTROL_1, SCALE_CH3, 4));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_DITHER_LFSR_INIT + ulRegOffset,
            BCHP_FIELD_ENUM(M2MC_DITHER_LFSR_INIT, SEQ, ONCE_PER_SOP) |
            BCHP_FIELD_DATA(M2MC_DITHER_LFSR_INIT, VALUE, 0));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_DITHER_LFSR + ulRegOffset,
            BCHP_FIELD_ENUM(M2MC_DITHER_LFSR, T0, B3) |
            BCHP_FIELD_ENUM(M2MC_DITHER_LFSR, T1, B8) |
            BCHP_FIELD_ENUM(M2MC_DITHER_LFSR, T2, B12));
#endif
    }
#endif

#if defined(BCHP_M2MC_DRAM_MAP_MAP_MASK)
    {
        uint32_t ulScbType = 0;
        /* Assuming memcs have same type of dram */
        switch(hGrc->stMemInfo.memc[0].type)
        {
        case BCHP_DramType_eDDR2:
            ulScbType = BCHP_M2MC_DRAM_MAP_MAP_MAP_2P0;
            break;

        case BCHP_DramType_eDDR3:
        case BCHP_DramType_eDDR4:
        case BCHP_DramType_eGDDR5:
        case BCHP_DramType_eGDDR5M:
            ulScbType = BCHP_M2MC_DRAM_MAP_MAP_MAP_5P0;
            break;

        case BCHP_DramType_eLPDDR4:
            ulScbType = BCHP_M2MC_DRAM_MAP_MAP_MAP_8P0;
            break;

        default:
            ulScbType = BCHP_M2MC_DRAM_MAP_MAP_DEFAULT;
            break;
        }
        BDBG_MSG(("MEMC_TYPE = %d - selected MAP_TYPE %d", hGrc->stMemInfo.memc[0].type, ulScbType));
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_DRAM_MAP+ulRegOffset,
                           BCHP_FIELD_DATA( M2MC_DRAM_MAP, MAP, ulScbType ) );
    }
#endif
    return;
}

#ifndef BCHP_PWR_RESOURCE_M2MC_SRAM
#define BCHP_PWR_RESOURCE_M2MC_SRAM 0
#endif
/***************************************************************************/
void BGRC_P_SetupPower(
    BGRC_Handle  hGrc,
    bool         bOn)
{

#if BCHP_PWR_SUPPORT
    if (bOn)
    {
        /* turn on power for M2MC engine */
        BCHP_PWR_AcquireResource(hGrc->hChip, hGrc->ulM2mcPwrId);



        if (0 != hGrc->ulSramPwrId)
        {
            BCHP_PWR_AcquireResource(hGrc->hChip, hGrc->ulSramPwrId);
        }
    }
    else
    {
        if (0 != hGrc->ulSramPwrId)
        {
            /* turn off power for M2MC SRAM */
            BCHP_PWR_ReleaseResource(hGrc->hChip, hGrc->ulSramPwrId);
        }

        /* turn off power for M2MC engine */
        BCHP_PWR_ReleaseResource(hGrc->hChip, hGrc->ulM2mcPwrId);
    }
#else
    BSTD_UNUSED(hGrc);
    BSTD_UNUSED(bOn);
#endif
}

/***************************************************************************/
#define BGRC_PACKET_P_ALIGN_HW_PKT_SIZE( a ) \
    (((a) + BGRC_PACKET_P_MEMORY_ALIGN_MASK) & (~BGRC_PACKET_P_MEMORY_ALIGN_MASK))

/***************************************************************************/
BERR_Code BGRC_Open(
    BGRC_Handle *phGrc,
    BCHP_Handle hChip,
    BREG_Handle hRegister,
    BMMA_Heap_Handle hMemory,
    BINT_Handle hInterrupt,
    const BGRC_Settings *pDefSettings )
{
    BERR_Code err = BERR_SUCCESS;
    BGRC_PacketContext_CreateSettings  stCtxSettings;

    BGRC_Handle hGrc = (BGRC_Handle) BKNI_Malloc( sizeof (BGRC_P_Handle) );
    if( hGrc == 0 )
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

    BKNI_Memset( (void *) hGrc, 0, sizeof (BGRC_P_Handle) );
    BDBG_OBJECT_SET(hGrc, BGRC);

    hGrc->hChip = hChip;
    hGrc->hRegister = hRegister;
    hGrc->hMemory = hMemory;
    hGrc->hInterrupt = hInterrupt;
    hGrc->ulPacketMemoryMax = pDefSettings ? pDefSettings->ulPacketMemoryMax : BGRC_P_DEFAULT_SETTINGS.ulPacketMemoryMax;
    hGrc->ulOperationMax = pDefSettings ? pDefSettings->ulOperationMax : BGRC_P_DEFAULT_SETTINGS.ulOperationMax;
    hGrc->eDeviceNum = (BGRC_eM2mcId)(pDefSettings ? pDefSettings->ulDeviceNum : BGRC_P_DEFAULT_SETTINGS.ulDeviceNum);
    hGrc->ulWaitTimeout = pDefSettings ? pDefSettings->ulWaitTimeout : BGRC_P_DEFAULT_SETTINGS.ulWaitTimeout;
#if (BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT)
    /* different number can schedule traffic to m2mc0 and m2mc1 traffice */
    hGrc->ulWeight = BCHP_M2MC_CXT_SCH_WEIGHT_CONTEXT0_WEIGHT_DEFAULT;
#endif

    switch(hGrc->eDeviceNum)
    {
        case BGRC_eM2mc0:
            hGrc->ulRegOffset = 0;
#if BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT
            hGrc->ulWeightReg = BCHP_M2MC_CXT_SCH_WEIGHT_CONTEXT0;
#endif
#if BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_gfx_sw_init_MASK
            hGrc->ulRegCoreReset = BCHP_SUN_TOP_CTRL_SW_INIT_0_SET;
            hGrc->ulRegClearCore = BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR;
            hGrc->ulCoreResetMask = BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_gfx_sw_init_MASK;
#endif
#ifdef BCHP_PWR_RESOURCE_M2MC
            hGrc->ulM2mcPwrId =
#if BCHP_PWR_RESOURCE_M2MC0
                BCHP_PWR_RESOURCE_M2MC0;
#else
                BCHP_PWR_RESOURCE_M2MC;
#endif
#endif
            hGrc->ulSramPwrId =
#if BCHP_PWR_RESOURCE_M2MC0_SRAM
                BCHP_PWR_RESOURCE_M2MC0_SRAM;
#else
                BCHP_PWR_RESOURCE_M2MC_SRAM;
#endif
            hGrc->ulIntid = BGRC_PACKET_P_M2MC0_INT_ID;

            break;
        case BGRC_eM2mc1:
#if BCHP_M2MC1_REG_START
            hGrc->ulRegOffset = BCHP_M2MC1_REG_START - BCHP_M2MC_REG_START;
            hGrc->ulRegCoreReset = BCHP_SUN_TOP_CTRL_SW_INIT_1_SET;
            hGrc->ulRegClearCore = BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR;
            hGrc->ulCoreResetMask = BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_m2mc1_sw_init_MASK;
            hGrc->ulIntid = BGRC_PACKET_P_M2MC1_INT_ID;
#endif
#if BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT
            hGrc->ulWeightReg = BCHP_M2MC_CXT_SCH_WEIGHT_CONTEXT1;
#endif

#if BCHP_PWR_SUPPORT
#if BGRC_PWR_RESOURCE_M2MC1
            hGrc->ulM2mcPwrId = BGRC_PWR_RESOURCE_M2MC1;
#endif
            hGrc->ulSramPwrId =
#if BCHP_PWR_RESOURCE_M2MC1_SRAM
                BCHP_PWR_RESOURCE_M2MC1_SRAM;
#else
                BCHP_PWR_RESOURCE_M2MC_SRAM;
#endif
#endif

            break;
        case BGRC_eMM_M2mc0:
#if BCHP_MM_M2MC0_REG_START
            hGrc->ulRegOffset = BCHP_MM_M2MC0_REG_START - BCHP_M2MC_REG_START;
            hGrc->ulWeightReg = BCHP_MM_M2MC_CXT_SCH_WEIGHT_CONTEXT0;
            hGrc->ulRegCoreReset = BCHP_SUN_TOP_CTRL_SW_INIT_1_SET;
            hGrc->ulRegClearCore = BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR;
            hGrc->ulCoreResetMask = BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_mm_m2mc_sw_init_MASK;
#if BCHP_PWR_SUPPORT
            hGrc->ulM2mcPwrId = BCHP_PWR_RESOURCE_MM_M2MC0;
            hGrc->ulSramPwrId = BCHP_PWR_RESOURCE_MMM2MC0_SRAM;
#endif
            hGrc->ulIntid     = BGRC_PACKET_P_MM_M2MC0_INT_ID;
#endif
            break;
        case BGRC_eM2mcMax:
        default:   /* BGRC_eM2mc0 or Max*/
            BDBG_OBJECT_DESTROY(hGrc, BGRC);
            BKNI_Free(hGrc);
            return BERR_TRACE(BERR_INVALID_PARAMETER);


    }

    /* turn on power for this m2mc engine */
    BGRC_P_SetupPower(hGrc, true);

    /* read memory controller type to use for scb map setting*/
    BCHP_GetMemoryInfo(hChip, &hGrc->stMemInfo);
    BGRC_P_SetMemoryInfo(hGrc);

    /* allocate device hw pakets buf, with some extra size for flushing blit */
    hGrc->pHwPktFifoBaseAlloc = BMMA_Alloc( hGrc->hMemory, BGRC_PACKET_P_ALIGN_HW_PKT_SIZE(hGrc->ulPacketMemoryMax) + BGRC_PACKET_P_BLIT_GROUP_SIZE_MAX, 1<<BGRC_PACKET_P_MEMORY_ALIGN_BITS, NULL );
    if( hGrc->pHwPktFifoBaseAlloc == NULL )
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    hGrc->ulHwPktFifoBaseOffset = BMMA_LockOffset(hGrc->pHwPktFifoBaseAlloc);
    if(!hGrc->ulHwPktFifoBaseOffset)
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    hGrc->pHwPktFifoBase = BMMA_Lock(hGrc->pHwPktFifoBaseAlloc);
    if(!hGrc->pHwPktFifoBase)
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    hGrc->ulHwPktFifoSize = hGrc->ulPacketMemoryMax;
    hGrc->pHwPktWritePtr = hGrc->pHwPktFifoBase;

    /* allocate dummy plane */
    hGrc->pDummySurAlloc = BMMA_Alloc( hGrc->hMemory, 1024, 1<<BGRC_PACKET_P_MEMORY_ALIGN_BITS, NULL);
    if( hGrc->pDummySurAlloc == NULL )
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    hGrc->ulDummySurOffset = BMMA_LockOffset(hGrc->pDummySurAlloc);
    if(!hGrc->ulDummySurOffset)
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    hGrc->pDummySurBase = BMMA_Lock(hGrc->pDummySurAlloc);
    if(!hGrc->pDummySurBase)
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    /* in case *hGrc->pDummySurBase is checked before BGRC_Packet_SyncPackets */
    *(uint32_t *)hGrc->pDummySurBase = ++hGrc->ulSyncCntr;
    BMMA_FlushCache(hGrc->pDummySurAlloc, hGrc->pDummySurBase, 1024 );

    /* init context list */
    BLST_D_INIT(&hGrc->context_list);

    /* create dummy context for extra flush blit */
    BGRC_Packet_GetDefaultCreateContextSettings( hGrc, &stCtxSettings );
    stCtxSettings.packet_buffer_size = BGRC_PACKET_P_FLUSH_PACKET_SIZE;
    err = BGRC_Packet_CreateContext( hGrc, &hGrc->hDummyCtx, &stCtxSettings );
    if( err != BERR_SUCCESS )
    {
        BGRC_Close( hGrc );
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    /* dummy context should not be in the context link list */
    BLST_D_REMOVE(&hGrc->context_list, hGrc->hDummyCtx, context_link);

    /* reset m2mc */
#if (BCHP_CHIP != 11360)
/*
 * For all platforms which do not have a supported 2D engine
 * BCHP_CHIP must be added either in an RDB header included by this source,
 * or in the specific for the platform bchp .inc file which affects all modules.
 */
    BGRC_P_ResetDevice( hGrc );
#endif

    BGRC_P_CheckTableMismach();

    *phGrc = hGrc;

    return BERR_SUCCESS;
}

/**************************************************************************/
void BGRC_GetCapabilities
    ( BGRC_Handle                      hGrc,
      BGRC_Capabilities               *pCapabilities )
{
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_ASSERT(pCapabilities);

    pCapabilities->ulMaxHrzDownSclRatio = BGRC_P_SCALE_DOWN_MAX_X;
    pCapabilities->ulMaxVerDownSclRatio = BGRC_P_SCALE_DOWN_MAX_Y;
}

/**************************************************************************/
BGRC_Mode BGRC_GetMode_isrsafe
    ( uint32_t                       ulDeviceIdx)
{
    BGRC_Mode eMode=BGRC_eBlitter;

#if BCHP_MM_M2MC0_REG_START
    eMode = ((BGRC_eM2mcId)ulDeviceIdx == BGRC_eMM_M2mc0)? BGRC_eMipmap :BGRC_eBlitter;
#else
	BSTD_UNUSED(ulDeviceIdx);
#endif
    return eMode;
}
/***************************************************************************/
void BGRC_Close(
    BGRC_Handle hGrc )
{
    BGRC_PacketContext_Handle hContext;

    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_ASSERT(BLST_D_EMPTY(&hGrc->context_list));

    /* in case power off/on might have problem? */
    BGRC_P_WriteReg32( hGrc, LIST_CTRL,
        BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE, Ack ) |
        BCHP_FIELD_ENUM( M2MC_LIST_CTRL, RUN, Stop ) |
        BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE_MODE, ResumeFromLast));

    /* destroy callbacks */
    if( hGrc->hCallback )
    {
        BINT_DisableCallback( hGrc->hCallback );
        BINT_DestroyCallback( hGrc->hCallback );
    }

    /* free hw pakets buf */
    if( hGrc->pHwPktFifoBaseAlloc ) {
        BMMA_Free(hGrc->pHwPktFifoBaseAlloc);
    }

    /* free dummy surface buf */
    if( hGrc->pDummySurAlloc ) {
        BMMA_Free(hGrc->pDummySurAlloc );
    }

    /* destroy dummy context */
    if (hGrc->hDummyCtx)
    {
        BLST_D_INSERT_HEAD(&hGrc->context_list, hGrc->hDummyCtx, context_link);
        BGRC_Packet_DestroyContext( hGrc, hGrc->hDummyCtx );
    }

    /* destroy all contexts */
    hGrc->hLastSyncCtx = NULL; /* avoid setting new hGrc->hLastSyncCtx */
    hContext = BLST_D_FIRST(&hGrc->context_list);
    while( hContext )
    {
        BGRC_PacketContext_Handle hNext = BLST_D_NEXT(hContext, context_link);
        BGRC_Packet_DestroyContext( hGrc, hContext );
        hContext = hNext;
    }

    /* turn off power for this m2mc engine */
    BGRC_P_SetupPower(hGrc, false);

    BDBG_OBJECT_DESTROY(hGrc, BGRC);
    BKNI_Free( (void *) hGrc );
}

/***************************************************************************/
void BGRC_GetDefaultStandbySettings
    ( BGRC_StandbySettings            *pStandbySettings )
{
    BDBG_ASSERT(pStandbySettings);

    pStandbySettings->bS3Standby = false;
    return;
}

/***************************************************************************/
BERR_Code BGRC_Standby(
    BGRC_Handle hGrc,
    const BGRC_StandbySettings      *pStandbySettings )
{
    BGRC_Packet_Status status;
    BERR_Code err = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hGrc, BGRC);

    /* ensure engine is idle */
    BGRC_Packet_GetStatus( hGrc, &status );
    while( status.m2mc_busy )
    {
        BGRC_Packet_AdvancePackets( hGrc, NULL );
        BGRC_Packet_GetStatus( hGrc, &status );
    }

    hGrc->hContext = NULL; /* make resume re-load all register groups */
    hGrc->hLastSyncCtx = NULL; /* don't add extra flush blit right after resume */
    if( hGrc->hCallback )
    {
        BINT_DisableCallback( hGrc->hCallback );
    }

    /* turn off power for this m2mc engine */
    BGRC_P_SetupPower(hGrc, false);

    hGrc->stStandbySettings = *pStandbySettings;

    return err;
}

/***************************************************************************/
BERR_Code BGRC_Resume(
    BGRC_Handle hGrc )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hGrc, BGRC);

    /* turn on power for this m2mc engine */
    BGRC_P_SetupPower(hGrc, true);

    /* make use FromFirst mode after reset */
    hGrc->pHwPktSubmitLinkPtr = NULL;
    BGRC_P_ResetDevice( hGrc );

    if( hGrc->hCallback )
    {
        BINT_EnableCallback( hGrc->hCallback );
    }

    return err;
}

/***************************************************************************/
static BERR_Code BGRC_PACKET_P_SetCallback_BINT(
    BGRC_Handle hGrc,
    BINT_CallbackHandle *phCallback,
    bool create )
{
    if( create )
    {

        if( *phCallback == 0 )
        {
            BERR_Code err = BINT_CreateCallback( phCallback, hGrc->hInterrupt, hGrc->ulIntid, BGRC_PACKET_P_Isr, hGrc, 0);
            if( err != BERR_SUCCESS )
                return BERR_TRACE(err);

            BINT_ClearCallback( *phCallback );
            BINT_EnableCallback( *phCallback );
        }
    }
    else if( *phCallback )
    {
        BINT_DisableCallback( *phCallback );
        BINT_DestroyCallback( *phCallback );
        *phCallback = 0;
    }

    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_SetCallback(
    BGRC_Handle hGrc,
    BGRC_Callback callback_isr,
    void *callback_data )
{
    BDBG_OBJECT_ASSERT(hGrc, BGRC);

    BKNI_EnterCriticalSection();
    hGrc->callback_isr = callback_isr;
    hGrc->callback_data = callback_data;
    BKNI_LeaveCriticalSection();

    BGRC_PACKET_P_SetCallback_BINT( hGrc, &hGrc->hCallback, NULL != callback_isr);

    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_GetDefaultCreateContextSettings(
    BGRC_Handle hGrc,
    BGRC_PacketContext_CreateSettings *pSettings )
{
    BSTD_UNUSED(hGrc);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->packet_buffer_size = 1024*64;
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_CreateContext(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle *phContext,
    BGRC_PacketContext_CreateSettings *pSettings )
{
    BMMA_Heap_Handle packet_heap = pSettings->packet_buffer_heap ? pSettings->packet_buffer_heap : hGrc->hMemory;
    BERR_Code err = BERR_SUCCESS;

    BGRC_PacketContext_Handle hContext = (BGRC_PacketContext_Handle) BKNI_Malloc( sizeof (BGRC_P_PacketContext) );
    if( hContext == 0 )
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

    BKNI_Memset( (void *) hContext, 0, sizeof (BGRC_P_PacketContext) );
    BDBG_OBJECT_SET(hContext, BGRC_PacketContext);
    hContext->hGrc = hGrc;

    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_ASSERT(phContext);

    /* add context to list */
    BLST_D_INSERT_HEAD(&hGrc->context_list, hContext, context_link);
    hContext->ulId = (hGrc->eDeviceNum << 30) | (++hGrc->ulNumCreates & 0x3fffffff);

    /* allocate software packet buffer */
    hContext->ulSwPktFifoSize = pSettings->packet_buffer_size;
    hContext->pSwPktFifoBaseAlloc = BMMA_Alloc( packet_heap, hContext->ulSwPktFifoSize + BGRC_PACKET_P_EXTRA_PACKET_SIZE, 1<<2, NULL);
    if(!hContext->pSwPktFifoBaseAlloc)
    {
        err = BERR_OUT_OF_SYSTEM_MEMORY;
        BGRC_Packet_DestroyContext( hGrc, hContext );
        return BERR_TRACE(err);
    }

    hContext->pSwPktFifoBase = BMMA_Lock(hContext->pSwPktFifoBaseAlloc);
    if(!hContext->pSwPktFifoBase)
    {
        err = BERR_OUT_OF_SYSTEM_MEMORY;
        BGRC_Packet_DestroyContext( hGrc, hContext );
        return BERR_TRACE(err);
    }

    hContext->pSwPktWritePtr = hContext->pSwPktFifoBase;
    hContext->pSwPktReadPtr = hContext->pSwPktFifoBase;
    hContext->pSwPktWrapPtr = hContext->pSwPktFifoBase + hContext->ulSwPktFifoSize;

    /* reset state */
    BGRC_PACKET_P_ResetState( hContext );

    /* store settings */
    hContext->create_settings = *pSettings;

    *phContext = hContext;
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_DestroyContext(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext )
{
    BDBG_ASSERT(hGrc);
    BDBG_ASSERT(hContext);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_OBJECT_ASSERT(hContext, BGRC_PacketContext);

#if (BCHP_CHIP != 11360)
    BDBG_ASSERT(hGrc->callback_isr);
#endif
    /* remove context from list */
    BLST_D_REMOVE(&hGrc->context_list, hContext, context_link);

    /* destroy callbacks */
    if( hContext->hCallback )
    {
        BINT_DisableCallback( hContext->hCallback );
        BINT_DestroyCallback( hContext->hCallback );
    }

    if( hContext->pSwPktFifoBaseAlloc ) {
        BMMA_Free(hContext->pSwPktFifoBaseAlloc);
    }

    if ( hContext == hGrc->hContext )
    {
        hGrc->hContext = NULL;
    }

    /* if all remaining contexts are not eQueuedInHw nor eSynced, hGrc->hLastSyncCtx will be NULL */
    if ( hContext == hGrc->hLastSyncCtx )
    {
        /* new hGrc->hLastSyncCtx is the eQueuedInHw context with the "biggest" ulSyncCntr */
        uint32_t ulSyncCntr = 0;
        BGRC_PacketContext_Handle hTmpCtx = bgrc_p_first_context(hGrc);
        hGrc->hLastSyncCtx = NULL;
        while( hTmpCtx )
        {
            if (BGRC_PACKET_P_SyncState_eQueuedInHw == hContext->eSyncState)
            {
                if ((0 == ulSyncCntr) || BGRC_P_SYNC_CNTR_OVER(hContext->ulSyncCntr, ulSyncCntr))
                {
                    hGrc->hLastSyncCtx = hTmpCtx;
                    ulSyncCntr = hContext->ulSyncCntr;
                }
            }
            hTmpCtx = bgrc_p_next_context(hGrc, hTmpCtx);
        }
    }

    BDBG_OBJECT_DESTROY(hContext, BGRC_PacketContext);
    BKNI_Free( hContext );

    return BERR_SUCCESS;
}

/***************************************************************************
 * note: might be called when hContext->ulLockedSwPktBufSize is not 0 */
static int BGRC_PACKET_P_SwPktBufAvailable(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext,
    size_t size_in )
{
    int buffer_available = 0;

    BDBG_ASSERT(hGrc);
    BDBG_ASSERT(hContext);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_OBJECT_ASSERT(hContext, BGRC_PacketContext);

    /* don't change hContext->pSwPktWritePtr if user has buf locked; and
     * don't change hContext->pSwPktReadPtr if there is sw pkt in fifo to be processed */
    if( hContext->pSwPktReadPtr > hContext->pSwPktWritePtr )
    {
        /* writer ptr already wrapped around */
        buffer_available = hContext->pSwPktReadPtr - hContext->pSwPktWritePtr - 4;
        BDBG_MSG(("sw pkt alloc: already wrapped w %p -> r %p", hContext->pSwPktWritePtr, hContext->pSwPktReadPtr));
    }
    else /* hContext->pSwPktReadPtr <= hContext->pSwPktWritePtr */
    {
        /* regular case */
        int buffer_available_wrapped = hContext->pSwPktReadPtr - hContext->pSwPktFifoBase - 4;
        buffer_available = hContext->ulSwPktFifoSize - (hContext->pSwPktWritePtr - hContext->pSwPktFifoBase);
        if (0 == hContext->ulLockedSwPktBufSize)
        {
            if (hContext->pSwPktWritePtr == hContext->pSwPktReadPtr)
            {
                if (0 != hContext->ulSwPktSizeToProc)
                {
                    /* BGRC_Packet_SubmitPackets is just called with whole sw pkt buf */
                    BDBG_ASSERT(hContext->ulSwPktFifoSize == hContext->ulSwPktSizeToProc &&
                                hContext->pSwPktWritePtr == hContext->pSwPktFifoBase);
                    buffer_available = 0;
                }
                else
                {
                    /* reset for better alloc behavour in future */
                    hContext->pSwPktWritePtr = hContext->pSwPktFifoBase;
                    hContext->pSwPktReadPtr = hContext->pSwPktFifoBase;
                    hContext->pSwPktWrapPtr = hContext->pSwPktFifoBase + hContext->ulSwPktFifoSize;
                    buffer_available = hContext->ulSwPktFifoSize;
                    BDBG_MODULE_MSG(BGRC_SWPKT_FIFO, ("ctx[%x] sw pkt alloc: reset [%p, %p]", hContext->ulId, hContext->pSwPktWritePtr, hContext->pSwPktWrapPtr));
                }
            }
            else if (((buffer_available < (int)(size_in)) && (buffer_available_wrapped >= (int)size_in)) ||
                     ((0 == (int)(size_in)) && (buffer_available < buffer_available_wrapped)))
            {
                /* wrap around */
                hContext->pSwPktWrapPtr = hContext->pSwPktWritePtr;
                hContext->pSwPktWritePtr = hContext->pSwPktFifoBase;
                buffer_available = buffer_available_wrapped;
                BDBG_MODULE_MSG(BGRC_SWPKT_FIFO, ("ctx[%x] sw pkt alloc: wraping %p > rptr %p for %d", hContext->ulId, hContext->pSwPktWrapPtr, hContext->pSwPktReadPtr, (uint32_t)size_in));
            }
        }
    }

    return buffer_available;
}

/***************************************************************************/
BERR_Code BGRC_Packet_GetPacketMemory(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext,
    void **buffer,
    size_t *size_out,
    size_t size_in )
{
    int buffer_available = 0;

    BDBG_ASSERT(hGrc);
    BDBG_ASSERT(hContext);
    BDBG_ASSERT(buffer);
    BDBG_ASSERT(size_in >= sizeof (BM2MC_PACKET_Header));
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_OBJECT_ASSERT(hContext, BGRC_PacketContext);
    BGRC_P_ENTER(hGrc);

    buffer_available = BGRC_PACKET_P_SwPktBufAvailable( hGrc, hContext, size_in );
    if( (buffer_available < (int) (size_in)) && (hContext->ulSwPktSizeToProc) )
    {
        if (hGrc->secure == hContext->create_settings.secure) {
            /* try to process some sw pkts to release enough sw pkt buf */
            BGRC_PACKET_P_ProcessSwPktFifo( hGrc, hContext );
            buffer_available = BGRC_PACKET_P_SwPktBufAvailable( hGrc, hContext, size_in );
        }
    }

    if( buffer_available < (int) (size_in) )
    {
        *buffer = NULL;
        buffer_available = 0;
    }
    else
    {
        *buffer = hContext->pSwPktWritePtr;
    }

    /* if size_in is 0, nexus meant to query only, not lock */
    if (size_in)
        hContext->ulLockedSwPktBufSize = buffer_available;
    if( size_out )
        *size_out = buffer_available;

    BGRC_P_LEAVE(hGrc);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_SubmitPackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext,
    size_t size )
{
    BDBG_ASSERT(hGrc);
    BDBG_ASSERT(hContext);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_OBJECT_ASSERT(hContext, BGRC_PacketContext);
    BGRC_P_ENTER(hGrc);

    if( size > (size_t) hContext->ulLockedSwPktBufSize )
    {
        BDBG_ERR(("BGRC_Packet_SubmitPackets %d bytes exceeds last BGRC_Packet_GetPacketMemory %d bytes, packet buffer is now corrupted!!!",
                  (uint32_t) size, hContext->ulLockedSwPktBufSize));
        BGRC_P_LEAVE(hGrc);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if( size >= sizeof (BM2MC_PACKET_Header) )
    {
        hContext->ulLockedSwPktBufSize = 0;
        hContext->ulSwPktSizeToProc += size;
        hContext->pSwPktWritePtr += size;
        hContext->bPktsInPipe = true;
        if (hContext->pSwPktWritePtr == hContext->pSwPktWrapPtr)
        {
            hContext->pSwPktWritePtr = hContext->pSwPktFifoBase;
            BDBG_MODULE_MSG(BGRC_SWPKT_FIFO, ("ctx[%x] sw pkt submit: wrap [%p, %p]", hContext->ulId, hContext->pSwPktWritePtr, hContext->pSwPktWrapPtr));
        }

    /*SWSTB-3670  workaround for 7278A0 blit hw change, hw will be fixed in 7278B0*/
#if BGRC_PACKET_P_BLIT_WORKAROUND
        if( hContext->ulSwPktSizeToProc > BGRC_PACKET_P_BLIT_GROUP_SIZE_MAX)
#else
        if( hContext->ulSwPktSizeToProc > hContext->create_settings.packet_buffer_store )
#endif
            BGRC_PACKET_P_ProcessSwPktFifo( hGrc, hContext );
    }

    /* return message indicating sw packet processing is incomplete */
    BGRC_P_LEAVE(hGrc);
    if( hContext->ulSwPktSizeToProc )
        return BGRC_PACKET_MSG_PACKETS_INCOMPLETE;
    else
        return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_AdvancePackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext )
{
    BERR_Code err = BERR_SUCCESS;
    BGRC_PacketContext_Handle hLastContext = 0;
    BGRC_PacketContext_Handle hCurrContext;

    BSTD_UNUSED(hContext);
    BDBG_ASSERT(hGrc);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BGRC_P_ENTER(hGrc);

    hCurrContext = bgrc_p_first_context(hGrc);

    /* loop through contexts */
    while( hCurrContext )
    {
        /* check if packets left to process */
        if (hCurrContext->eSyncState == BGRC_PACKET_P_SyncState_eRequested)
        {
            /* checkpoint is expensive, app is waiting, so we must force it in */
            while ( hCurrContext->ulSwPktSizeToProc ) {
                BGRC_PACKET_P_ProcessSwPktFifo( hGrc, hCurrContext );
            }
            BDBG_MSG(( ".. BGRC_PACKET_P_SyncState_eRequested -> " ));
            BGRC_PACKET_P_InsertFlushBlit( hGrc, hCurrContext );
        }
        else
        {
            if( hCurrContext->ulSwPktSizeToProc ) {
                BGRC_PACKET_P_ProcessSwPktFifo( hGrc, hCurrContext );
            }
            if( hCurrContext->ulSwPktSizeToProc ) {
                err = BGRC_PACKET_MSG_PACKETS_INCOMPLETE;
            }
        }

        /* get next context */
        hLastContext = hCurrContext;
        hCurrContext = bgrc_p_next_context(hGrc, hCurrContext);
    }

    /* put last context at the head of the list to give it first access to free memory */
    if (hLastContext)
    {
        BLST_D_REMOVE(&hGrc->context_list, hLastContext, context_link);
        BLST_D_INSERT_HEAD(&hGrc->context_list, hLastContext, context_link);
    }

#if BGRC_PACKET_P_BLIT_WORKAROUND
    if(hGrc->ulNumbersToRun)
    {
        uint32_t ulListStatus = BGRC_P_ReadReg32(hGrc, LIST_STATUS);
        bool bListFinished = BCHP_GET_FIELD_DATA(ulListStatus, M2MC_LIST_STATUS, FINISHED);
        if(bListFinished)
        {
            BGRC_P_WriteReg32( hGrc, LIST_CTRL,
                BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE, WakeUp ) |
                    BCHP_FIELD_ENUM( M2MC_LIST_CTRL, RUN, Run ) |
                    BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE_MODE, ResumeFromLast ));
            BKNI_EnterCriticalSection();
            hGrc->ulNumbersToRun --;
            BKNI_LeaveCriticalSection();
            BDBG_MODULE_MSG(BGRC_LISTSTATUS, ("ulNumbersToRun %d", hGrc->ulNumbersToRun));
        }
    }
#endif

    BGRC_P_LEAVE(hGrc);
    return err;
}

/***************************************************************************/
BERR_Code BGRC_Packet_SyncPackets(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext )
{
    BDBG_ASSERT(hGrc);
    BDBG_ASSERT(hContext);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_OBJECT_ASSERT(hContext, BGRC_PacketContext);
    BGRC_P_ENTER(hGrc);

    /* can not call in again before previous request has been cleared */
    if (BGRC_PACKET_P_SyncState_eCleared != hContext->eSyncState)
    {
        BDBG_ERR(("SyncPackets called for ctx[%x] when syncState is %d", hContext->ulId, hContext->eSyncState));
        BGRC_P_LEAVE(hGrc);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* return message indicating context's blits are complete */
    if( !hContext->bPktsInPipe )
    {
        BGRC_P_LEAVE(hGrc);
        return BGRC_PACKET_MSG_BLITS_COMPLETE;
    }

/*#define BGRC_P_DEBUG_WITHOUT_SYNC 1*/
#if BGRC_P_DEBUG_WITHOUT_SYNC
    while ( hContext->ulSwPktSizeToProc ) {
        BGRC_PACKET_P_ProcessSwPktFifo( hGrc, hContext );
    }
    return BGRC_PACKET_MSG_BLITS_COMPLETE;
#else
    hContext->eSyncState = BGRC_PACKET_P_SyncState_eRequested;
    return BGRC_Packet_AdvancePackets(hGrc, NULL);
#endif
}

void BGRC_Packet_PrintStatus(BGRC_PacketContext_Handle hContext)
{
    int iSwPktBufAvailable = BGRC_PACKET_P_SwPktBufAvailable( hContext->hGrc, hContext, 0 );
    BDBG_WRN(("Ctx[%x] sync %d[off " BDBG_UINT64_FMT "], SwPkt: Size 0x%x[%d], range[%p, %p], w %p, r %p, wrap %p, bufAvail %d, %s",
              hContext->ulId, hContext->eSyncState, BDBG_UINT64_ARG(hContext->ulSyncHwPktOffset), hContext->ulSwPktSizeToProc, (int)hContext->bPktsInPipe,
              hContext->pSwPktFifoBase, hContext->pSwPktFifoBase + hContext->ulSwPktFifoSize,
              hContext->pSwPktWritePtr, hContext->pSwPktReadPtr, hContext->pSwPktWrapPtr, iSwPktBufAvailable,
              hContext->create_settings.secure?"secure":"unsecure"));
    BSTD_UNUSED (iSwPktBufAvailable);
}

#if (BDBG_DEBUG_BUILD)
static BSTD_DeviceOffset bgrc_p_hw_fifo_ptr_to_offset(BGRC_Handle hGrc, void *ptr)
{
    return ptr ? (BSTD_DeviceOffset)((uint8_t*)ptr - hGrc->pHwPktFifoBase) + hGrc->ulHwPktFifoBaseOffset: 0;
}
#endif

/***************************************************************************
 * could also be called from app to dump status in the case something goes wrong.
 *   To use it, define "BGRC_Handle g_hGrc = NULL;" and add  "g_hGrc = hGrc;" into BGRC_Open
 */
static void BGRC_PrintStatus(
    BGRC_Handle hGrc )
{
#if (BDBG_DEBUG_BUILD)
    BGRC_PacketContext_Handle hContext;
    uint32_t blit_status;
    uint32_t list_status;
    BSTD_DeviceOffset cur_desc;

    blit_status = BGRC_P_ReadReg32( hGrc, BLIT_STATUS);
    list_status = BGRC_P_ReadReg32( hGrc, LIST_STATUS);
    cur_desc = BGRC_P_ReadAddr( hGrc, LIST_CURR_PKT_ADDR);
    BDBG_WRN(("Total HwPkt range[" BDBG_UINT64_FMT "," BDBG_UINT64_FMT "], w " BDBG_UINT64_FMT ", r " BDBG_UINT64_FMT ", last " BDBG_UINT64_FMT ", blitStatus 0x%x, listStatus 0x%x, curDesc 0x"BDBG_UINT64_FMT ", %s",
              BDBG_UINT64_ARG(hGrc->ulHwPktFifoBaseOffset), BDBG_UINT64_ARG(hGrc->ulHwPktFifoBaseOffset + hGrc->ulHwPktFifoSize),
              BDBG_UINT64_ARG(bgrc_p_hw_fifo_ptr_to_offset(hGrc, hGrc->pHwPktWritePtr)),
              BDBG_UINT64_ARG(hGrc->ulHwPktOffsetExecuted),
              BDBG_UINT64_ARG(bgrc_p_hw_fifo_ptr_to_offset(hGrc, hGrc->pLastHwPktPtr)),
              blit_status, list_status, BDBG_UINT64_ARG(cur_desc),
              hGrc->secure?"secure":"unsecure"));

    /* get first context */
    hContext = BLST_D_FIRST(&hGrc->context_list);
    while( hContext )
    {
        if (hContext->eSyncState != BGRC_PACKET_P_SyncState_eCleared || hContext->ulSyncHwPktOffset) {
            BGRC_Packet_PrintStatus(hContext);
        }
        hContext = BLST_D_NEXT(hContext, context_link);
    }
#else
    BSTD_UNUSED(hGrc);
#endif
}

#if BGRC_USE_WATCHDOG_RESET
/* unproven to actually work. defaulting off. */
static void BGRC_P_WatchdogReset(
    BGRC_Handle hGrc,
    BGRC_PacketContext_Handle hContext )
{
    BSTD_DeviceOffset ulCurDesc;

    BSTD_UNUSED(hContext);

    BGRC_PrintStatus(hGrc);

    ulCurDesc = BGRC_P_ReadAddr( hGrc, LIST_CURR_PKT_ADDR);
    BGRC_P_ResetDevice(hGrc);
    BGRC_P_WriteAddr( hGrc, LIST_FIRST_PKT_ADDR, ulCurDesc);
    BGRC_P_WriteReg32( hGrc, LIST_CTRL,
                       BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE, Ack ) |
                       BCHP_FIELD_ENUM( M2MC_LIST_CTRL, RUN, Run ) |
                       BCHP_FIELD_ENUM( M2MC_LIST_CTRL, WAKE_MODE, ResumeFromFirst ));
    BDBG_WRN(("Reset m2mc%d for ctx[%x] and restart at 0x%x", hGrc->ulDeviceNum, hContext->ulId, ulCurDesc));
}
#endif

/***************************************************************************/
BERR_Code BGRC_Packet_CheckpointWatchdog(BGRC_Handle hGrc, BGRC_PacketContext_Handle hContext, bool force)
{
    if (BGRC_PACKET_P_SyncState_eCleared != hContext->eSyncState)
    {
        BGRC_P_CheckHwStatus( hGrc );
        BDBG_WRN(("Ctx[%x] watchdog[0]: syncState %d, secure %u", hContext->ulId, hContext->eSyncState, hContext->create_settings.secure));
        if (!force) return BERR_SUCCESS;

        if (BGRC_PACKET_P_SyncState_eSynced != hContext->eSyncState) {
            BGRC_PrintStatus(hGrc);
            BDBG_WRN(("Ctx[%x] watchdog[1]", hContext->ulId));
            BGRC_Packet_PrintStatus(hContext);
            if (hContext->create_settings.secure == hGrc->secure) {
                BGRC_Packet_AdvancePackets( hGrc, NULL );
                BGRC_P_CheckHwStatus( hGrc );
                if (BGRC_PACKET_P_SyncState_eSynced != hContext->eSyncState) {
                    BDBG_WRN(("Ctx[%x] watchdog[2]", hContext->ulId));
#if BGRC_USE_WATCHDOG_RESET
                    /* we cannot reset for secure without system failure. */
                    if (!hContext->create_settings.secure) {
                        BGRC_P_WatchdogReset( hGrc, hContext );
                    }
#endif
                }
            }
            hContext->eSyncState = BGRC_PACKET_P_SyncState_eSynced;
        }

        if (!hContext->create_settings.secure) {
            /* ensure BGRC_Packet_GetContextStatus to report 'synced' */
            BMMA_FlushCache(hGrc->pDummySurAlloc, hGrc->pDummySurBase, 1024 );
            hContext->ulSyncCntr = *(uint32_t *)hGrc->pDummySurBase;
        }

        /* inform nexus to check sync status again */
        BKNI_EnterCriticalSection();
        (*hGrc->callback_isr)( hGrc, hGrc->callback_data );
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_GetContextStatus(
    BGRC_Handle hGrc,
    BGRC_Packet_ContextStatus *status_array,
    size_t *size_out,
    size_t size_in,
    bool *pSecureModeSwitchNeeded)
{
    BGRC_PacketContext_Handle hContext;
    size_t context_index = 0;

    BDBG_ASSERT(hGrc);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BDBG_ASSERT(status_array);
    BDBG_ASSERT(size_out);
    BDBG_ASSERT(size_in);
    BGRC_P_ENTER(hGrc);

    hGrc->waitForSync = false;
    *pSecureModeSwitchNeeded = false;
    BGRC_P_CheckHwStatus( hGrc );

    /* get first context */
    hContext = BLST_D_FIRST(&hGrc->context_list);
    while( hContext )
    {
        bool bCtxSynced = false;

        if (hContext->create_settings.secure != hGrc->secure) {
            if (hContext->eSyncState != BGRC_PACKET_P_SyncState_eCleared)
                *pSecureModeSwitchNeeded = true;
            goto next;
        }

        if (BGRC_PACKET_P_SyncState_eSynced == hContext->eSyncState)
        {
            bCtxSynced = true;
            hContext->bPktsInPipe = false;
        }

        /* check if context sync'ed or packets still need processing */
        if( bCtxSynced || hContext->bPktsInPipe )
        {
            int iSwPktBufAvailable = BGRC_PACKET_P_SwPktBufAvailable( hGrc, hContext, 0 );

            /* don't write to array if no more space */
            if( context_index == size_in ) {
                /* this is not an error, but could be a problem if hit repeatedly */
                BDBG_WRN(("BGRC_Packet_GetContextStatus hit %d limit", (uint32_t)size_in));
            }
            else
            {
                /* fill status entry */
                status_array[context_index].hContext = hContext;
                status_array[context_index].private_data = hContext->create_settings.private_data;
                status_array[context_index].sync = bCtxSynced;
                status_array[context_index].packet_buffer_available = iSwPktBufAvailable;
                context_index++;
                if (bCtxSynced)
                {
                    hContext->eSyncState = BGRC_PACKET_P_SyncState_eCleared;
                }
            }
        }

        if (hContext->eSyncState != BGRC_PACKET_P_SyncState_eSynced &&
            hContext->eSyncState != BGRC_PACKET_P_SyncState_eCleared)
        {
            hGrc->waitForSync = true;
        }

next:
        /* get next context */
        hContext = BLST_D_NEXT(hContext, context_link);
    }
    *size_out = context_index;

    /* flush sw fifo, and if the last sync blit is not synced yet, add one more flush blit
       to further flush and to generate one more _isr that keeps the system rolling */
    if (!hGrc->secure) {
        BGRC_PACKET_P_CheckFlush(hGrc);
    }

    BGRC_P_LEAVE(hGrc);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_GetStatus(
    BGRC_Handle hGrc,
    BGRC_Packet_Status *status )
{
    BDBG_ASSERT(hGrc);
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    BGRC_P_ENTER(hGrc);

    BGRC_P_CheckHwStatus(hGrc);
    status->m2mc_busy = hGrc->pLastHwPktPtr != NULL;
    if (!status->m2mc_busy) {
        BGRC_PacketContext_Handle hContext;
        for (hContext = BLST_D_FIRST(&hGrc->context_list);hContext;hContext = BLST_D_NEXT(hContext, context_link)) {
            if (hContext->ulSwPktSizeToProc) {
                status->m2mc_busy = true;
                break;
            }
        }
    }
    BGRC_P_LEAVE(hGrc);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_ConvertPixelFormat(
    BM2MC_PACKET_PixelFormat *pformat,
    BPXL_Format pxl_format )
{
    BDBG_ASSERT(pformat);

    *pformat = BGRC_PACKET_P_ConvertPixelFormat( pxl_format );
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Packet_ConvertFilter(
    BM2MC_PACKET_FilterCoeffs *coeffs,
    BGRC_FilterCoeffs filter,
    size_t src_size,
    size_t out_size )
{
    BERR_Code rc=BERR_SUCCESS;
    BDBG_ASSERT(coeffs);

    rc = BGRC_PACKET_P_ConvertFilterCoeffs( coeffs, filter, src_size, out_size );
    return (rc);
}

/***************************************************************************/
BERR_Code BGRC_Packet_ConvertColorMatrix(
    BM2MC_PACKET_ColorMatrix *matrix_out,
    const int32_t *matrix_in,
    size_t shift )
{
    BDBG_ASSERT(matrix_out);
    BDBG_ASSERT(matrix_in);

    BGRC_PACKET_P_ConvertColorMatrix( matrix_out, matrix_in, shift );
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************/
BERR_Code BGRC_Packet_InitPacketPlane(
    BPXL_Plane *pSurface,
    BM2MC_PACKET_Plane *pPckPlane )
{
    pPckPlane->address = BMMA_LockOffset(pSurface->hPixels) + pSurface->ulPixelsOffset;
    pPckPlane->pitch = pSurface->ulPitch;
    BGRC_Packet_ConvertPixelFormat((BM2MC_PACKET_PixelFormat *)&(pPckPlane->format), pSurface->eFormat);
    pPckPlane->width = pSurface->ulWidth;
    pPckPlane->height = pSurface->ulHeight;
    return BERR_SUCCESS;
}
#endif

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSourcePlanePacket( BGRC_Handle hGrc, void **ppPacket,
    const BM2MC_PACKET_Plane *pSrcPlane0, const BM2MC_PACKET_Plane *pSrcPlane1, uint32_t color )
{
    if( pSrcPlane0 && pSrcPlane1 )
    {
        BM2MC_PACKET_PacketSourceFeeders *pPacket = (BM2MC_PACKET_PacketSourceFeeders *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, SourceFeeders, false );
        pPacket->plane0 = *pSrcPlane0;
        pPacket->plane1 = *pSrcPlane1;
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else if( pSrcPlane0 )
    {
        BM2MC_PACKET_PacketSourceFeeder *pPacket = (BM2MC_PACKET_PacketSourceFeeder *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, SourceFeeder, false );
        pPacket->plane = *pSrcPlane0;
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else
    {
        BM2MC_PACKET_PacketSourceColor *pPacket = (BM2MC_PACKET_PacketSourceColor *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, SourceColor, false );
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetStripedSourcePlanePacket( BGRC_Handle hGrc, void **ppPacket,
    const BM2MC_PACKET_StripedPlane *pSrcPlane, uint32_t color )
{
    BSTD_UNUSED(hGrc);
    if( pSrcPlane )
    {
        BM2MC_PACKET_PacketStripedSourceFeeders *pPacket = (BM2MC_PACKET_PacketStripedSourceFeeders *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, StripedSourceFeeders, false );
        pPacket->plane = *pSrcPlane;
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
        return BERR_SUCCESS;
    }
    else
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSourceControl( BGRC_Handle hGrc, void **ppPacket,
    bool zero_pad, bool chroma_filter, bool linear_destripe )
{
    BM2MC_PACKET_PacketSourceControl *pPacket = (BM2MC_PACKET_PacketSourceControl *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, SourceControl, false );
    pPacket->zero_pad = zero_pad;
    pPacket->chroma_filter = chroma_filter;
    pPacket->linear_destripe = linear_destripe;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestinationPlanePacket( BGRC_Handle hGrc, void **ppPacket,
    const BM2MC_PACKET_Plane *pPlane, uint32_t color )
{
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    if( pPlane )
    {
        BM2MC_PACKET_PacketDestinationFeeder *pPacket = (BM2MC_PACKET_PacketDestinationFeeder *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, DestinationFeeder, false );
        pPacket->plane = *pPlane;
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else
    {
        BM2MC_PACKET_PacketDestinationColor *pPacket = (BM2MC_PACKET_PacketDestinationColor *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, DestinationColor, false );
        pPacket->color = color;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }

    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestinationControl( BGRC_Handle hGrc, void **ppPacket,
    bool zero_pad, bool chroma_filter )
{
    BM2MC_PACKET_PacketDestinationControl *pPacket = (BM2MC_PACKET_PacketDestinationControl *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, DestinationControl, false );
    pPacket->zero_pad = zero_pad;
    pPacket->chroma_filter = chroma_filter;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetOutputPlanePacket( BGRC_Handle hGrc, void **ppPacket,
    const BM2MC_PACKET_Plane *pPlane)
{
    BM2MC_PACKET_PacketOutputFeeder *pPacket = (BM2MC_PACKET_PacketOutputFeeder *) (*ppPacket);

    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
    {
        if (BM2MC_PACKET_PixelFormat_eUIF_R8_G8_B8_A8 != pPlane->format)
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* The start address of the level0 image must be 64byte aligned */
        if (((pPlane->address ) & (uint64_t)BGRC_P_MIPMAP_SURFACE_ALIGN_MASK) != 0)
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BM2MC_PACKET_INIT( pPacket, OutputFeeder, false );
    pPacket->plane = *pPlane;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetOutputControl( BGRC_Handle hGrc, void **ppPacket,
    bool dither, bool chroma_filter)
{
    BM2MC_PACKET_PacketOutputControl *pPacket = (BM2MC_PACKET_PacketOutputControl *) (*ppPacket);

    BM2MC_PACKET_INIT( pPacket, OutputControl, false );
    pPacket->dither = dither;
    pPacket->chroma_filter = chroma_filter;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetMipmapControl( BGRC_Handle hGrc, void **ppPacket,
    uint8_t miplevel)
{
#if (BGRC_P_VER >= BGRC_P_VER_3)
    BM2MC_PACKET_PacketMipmapControl *pPacket = (BM2MC_PACKET_PacketMipmapControl *) (*ppPacket);

    BM2MC_PACKET_INIT( pPacket, MipmapControl, false );
    if (BGRC_eMM_M2mc0 != hGrc->eDeviceNum)
    {
        return BERR_NOT_SUPPORTED;
    }

    if(miplevel > BPXL_UIF_MAX_MIP_LEVELS)
    {
        BDBG_WRN(("miplevel should be less than %d", BPXL_UIF_MAX_MIP_LEVELS));
        miplevel = BPXL_UIF_MAX_MIP_LEVELS;
        return BERR_NOT_SUPPORTED;
    }

    pPacket->mipLevel = miplevel;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(hGrc);
    BSTD_UNUSED(ppPacket);
    BSTD_UNUSED(miplevel);
    return BERR_NOT_SUPPORTED;
#endif
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetBlendPacket( BGRC_Handle hGrc, void **ppPacket,
    const BM2MC_PACKET_Blend *pColor, const BM2MC_PACKET_Blend *pAlpha, uint32_t color )
{
    BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, Blend, false );
    pPacket->color_blend = *pColor;
    pPacket->alpha_blend = *pAlpha;
    pPacket->color = color;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetRopPacket( BGRC_Handle hGrc, void **ppPacket,
    uint8_t rop, uint32_t* pattern, uint32_t color0, uint32_t color1 )
{
    BM2MC_PACKET_PacketRop *pPacket = (BM2MC_PACKET_PacketRop *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, Rop, false );
    pPacket->rop = rop;
    pPacket->pattern0 = pattern ? pattern[0] : 0;
    pPacket->pattern1 = pattern ? pattern[1] : 0;
    pPacket->color0 = color0;
    pPacket->color1 = color1;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSourceColorkeyPacket( BGRC_Handle hGrc, void **ppPacket,
    bool enable, uint32_t high, uint32_t low, uint32_t mask, uint32_t replacement, uint32_t replacement_mask )
{
    BM2MC_PACKET_PacketSourceColorkeyEnable *pPacket = (BM2MC_PACKET_PacketSourceColorkeyEnable *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, SourceColorkeyEnable, false );
    pPacket->enable = enable ? 1 : 0;
    BM2MC_PACKET_TERM( pPacket );
    *ppPacket = (void *) pPacket;

    if( enable )
    {
        BM2MC_PACKET_PacketSourceColorkey *pPacket = (BM2MC_PACKET_PacketSourceColorkey *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, SourceColorkey, false );
        pPacket->high = high;
        pPacket->low = low;
        pPacket->mask = mask;
        pPacket->replacement = replacement;
        pPacket->replacement_mask = replacement_mask;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetDestinationColorkeyPacket( BGRC_Handle hGrc, void **ppPacket,
    bool enable, uint32_t high, uint32_t low, uint32_t mask, uint32_t replacement, uint32_t replacement_mask )
{
    BM2MC_PACKET_PacketDestinationColorkeyEnable *pPacket = (BM2MC_PACKET_PacketDestinationColorkeyEnable *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, DestinationColorkeyEnable, false );
    pPacket->enable = enable ? 1 : 0;
    BM2MC_PACKET_TERM( pPacket );
    *ppPacket = (void *) pPacket;

    if( enable )
    {
        BM2MC_PACKET_PacketDestinationColorkey *pPacket = (BM2MC_PACKET_PacketDestinationColorkey *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, DestinationColorkey, false );
        pPacket->high = high;
        pPacket->low = low;
        pPacket->mask = mask;
        pPacket->replacement = replacement;
        pPacket->replacement_mask = replacement_mask;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetFilterPacket( BGRC_Handle hGrc, void **ppPacket,
    BGRC_FilterCoeffs hor, BGRC_FilterCoeffs ver, BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect )
{
    bool enable = (hor != BGRC_FilterCoeffs_ePointSample) || (ver != BGRC_FilterCoeffs_ePointSample);

    BM2MC_PACKET_PacketFilterEnable *pPacket = (BM2MC_PACKET_PacketFilterEnable *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, FilterEnable, false );
    pPacket->enable = enable ? 1 : 0;
    BM2MC_PACKET_TERM( pPacket );
    *ppPacket = (void *) pPacket;

    {
        BM2MC_PACKET_PacketFilter *pPacket = (BM2MC_PACKET_PacketFilter *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, Filter, false );
        BGRC_Packet_ConvertFilter( &pPacket->hor, hor, pSrcRect->width, pOutRect->width );
        BGRC_Packet_ConvertFilter( &pPacket->ver, ver, pSrcRect->height, pOutRect->height );
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetColorMatrixPacket( BGRC_Handle hGrc, void **ppPacket,
    const int32_t matrix[], uint32_t shift )
{
    BM2MC_PACKET_PacketSourceColorMatrixEnable *pPacket = (BM2MC_PACKET_PacketSourceColorMatrixEnable *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, SourceColorMatrixEnable, false );
    pPacket->enable = matrix ? 1 : 0;
    BM2MC_PACKET_TERM( pPacket );
    *ppPacket = (void *) pPacket;

    if( matrix )
    {
        BM2MC_PACKET_PacketSourceColorMatrix *pPacket = (BM2MC_PACKET_PacketSourceColorMatrix *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, SourceColorMatrix, false );
        BGRC_Packet_ConvertColorMatrix( &pPacket->matrix, matrix, shift );
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetSourcePalette( BGRC_Handle hGrc, void **ppPacket,
    uint64_t paletteAddress )
{
    BM2MC_PACKET_PacketSourcePalette *pPacket = (BM2MC_PACKET_PacketSourcePalette *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, SourcePalette, false );
    pPacket->address = paletteAddress;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetAlphaPremultiply( BGRC_Handle hGrc, void **ppPacket, bool enable )
{
    BM2MC_PACKET_PacketAlphaPremultiply *pPacket = (BM2MC_PACKET_PacketAlphaPremultiply *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    BM2MC_PACKET_INIT( pPacket, AlphaPremultiply, false );
    pPacket->enable = enable ? 1 : 0;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetFilterControlPacket( BGRC_Handle hGrc, void **ppPacket,
    bool round, bool adjust_color, bool sub_alpha, BM2MC_PACKET_eFilterOrder sourceFilterOrder )
{
    BM2MC_PACKET_PacketFilterControl *pPacket = (BM2MC_PACKET_PacketFilterControl *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, FilterControl, false );
    pPacket->round = round ? 1 : 0;
    pPacket->adjust_color = adjust_color? 1 : 0;
    pPacket->sub_alpha = sub_alpha? 1 : 0;
    pPacket->order = sourceFilterOrder;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetMirrorPacket( BGRC_Handle hGrc, void **ppPacket,
    bool srcHor, bool srcVer, bool dstHor, bool dstVer, bool outHor, bool outVer )
{
    BM2MC_PACKET_PacketMirror *pPacket = (BM2MC_PACKET_PacketMirror *) (*ppPacket);
    BM2MC_PACKET_INIT( pPacket, Mirror, false );
    pPacket->src_hor = srcHor;
    pPacket->src_ver = srcVer;
    pPacket->dst_hor = dstHor;
    pPacket->dst_ver = dstVer;
    pPacket->out_hor = outHor;
    pPacket->out_ver = outVer;
    BM2MC_PACKET_TERM( pPacket );

    *ppPacket = (void *) pPacket;
    BSTD_UNUSED(hGrc);
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetBlitPacket( BGRC_Handle hGrc, void **ppPacket,
    BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Point *pOutPoint, BM2MC_PACKET_Point *pDstPoint )
{
    if( pDstPoint )
    {
        BM2MC_PACKET_PacketBlendBlit *pPacket = (BM2MC_PACKET_PacketBlendBlit *) (*ppPacket);
    if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
        return BERR_TRACE(BERR_NOT_SUPPORTED);
        BM2MC_PACKET_INIT( pPacket, BlendBlit, true );
        pPacket->src_rect = *pSrcRect;
        pPacket->out_point = *pOutPoint;
        pPacket->dst_point = *pDstPoint;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else if( pOutPoint )
    {
        BM2MC_PACKET_PacketCopyBlit *pPacket = (BM2MC_PACKET_PacketCopyBlit *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, CopyBlit, true );
        pPacket->src_rect = *pSrcRect;
        pPacket->out_point = *pOutPoint;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else
    {
        BM2MC_PACKET_PacketFillBlit *pPacket = (BM2MC_PACKET_PacketFillBlit *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, FillBlit, true );
        pPacket->rect = *pSrcRect;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    return BERR_SUCCESS;
}

/*****************************************************************************/
BERR_Code BGRC_Packet_SetScaleBlitPacket( BGRC_Handle hGrc, void **ppPacket,
    BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect, BM2MC_PACKET_Point *pDstPoint )
{
    if( pDstPoint )
    {
        BM2MC_PACKET_PacketScaleBlendBlit *pPacket = (BM2MC_PACKET_PacketScaleBlendBlit *) (*ppPacket);
        if (BGRC_eMM_M2mc0 == hGrc->eDeviceNum)
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        BM2MC_PACKET_INIT( pPacket, ScaleBlendBlit, true );
        pPacket->src_rect = *pSrcRect;
        pPacket->out_rect = *pOutRect;
        pPacket->dst_point = *pDstPoint;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    else
    {
        BM2MC_PACKET_PacketScaleBlit *pPacket = (BM2MC_PACKET_PacketScaleBlit *) (*ppPacket);
        BM2MC_PACKET_INIT( pPacket, ScaleBlit, true );
        pPacket->src_rect = *pSrcRect;
        pPacket->out_rect = *pOutRect;
        BM2MC_PACKET_TERM( pPacket );
        *ppPacket = (void *) pPacket;
    }
    return BERR_SUCCESS;
}

BERR_Code BGRC_SetSecureMode( BGRC_Handle hGrc, bool secure )
{
    BDBG_OBJECT_ASSERT(hGrc, BGRC);
    if (hGrc->waitForSync) return BERR_NOT_AVAILABLE;
    hGrc->secure = secure;
    hGrc->hLastSyncCtx = NULL;
    return BERR_SUCCESS;
}

/* End of File */
