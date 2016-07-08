/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bstd_defs.h"
#include "berr.h"
#include "bkni.h"
#include "bint.h"
#include "bchp.h"

/* bchp_common.h affects the folloing #include files */
#include "bchp_common.h"

#ifdef BCHP_M2MC1_REG_START
#include "bchp_m2mc_l2.h"
#include "bchp_m2mc1_l2.h"
#include "bchp_m2mc_gr.h"
#include "bchp_m2mc1_gr.h"
#include "bchp_int_id_m2mc1_l2.h"
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

#include "bgrc.h"
#include "bgrc_private.h"

BDBG_MODULE(BGRC);
BDBG_OBJECT_ID(BGRC);

/***************************************************************************/
#define BGRC_P_SURFACE_RECT_SIZE_MAX    8191

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
BERR_Code BGRC_Open(
    BGRC_Handle *phGrc,
    BCHP_Handle hChip,
    BREG_Handle hRegister,
    BMEM_Handle hMemory,
    BINT_Handle hInterrupt,
    const BGRC_Settings *pDefSettings )
{
    BGRC_Handle hGrc = 0;
    BGRC_P_State *pState;
    BERR_Code err = BERR_SUCCESS;
    const int32_t ai32_Matrix[20] = { 0 };
    const uint8_t aucPattern[8] = { 0 };

#if defined(BCHP_INT_ID_GFX_L2_M2MC_INTR)
    BINT_Id IntID = BCHP_INT_ID_GFX_L2_M2MC_INTR;
#elif defined(BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_0_INTR_SHIFT)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_MEMC16_GFX_L2_CPU_STATUS, BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_0_INTR_SHIFT);
#elif defined(BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_MEMC16_GFX_L2_CPU_STATUS, BCHP_MEMC16_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT);
#elif defined(BCHP_INT_ID_M2MC_0_INTR)
    BINT_Id IntID = BCHP_INT_ID_M2MC_0_INTR;
#elif defined(BCHP_GRAPHICS_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_GRAPHICS_L2_CPU_STATUS, BCHP_GRAPHICS_L2_CPU_STATUS_M2MC_0_INTR_SHIFT);
#elif defined(BCHP_GFX_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_GFX_L2_CPU_STATUS, BCHP_GFX_L2_CPU_STATUS_M2MC_INTR_SHIFT);
#elif defined(BCHP_M2MC_WRAP_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_M2MC_WRAP_L2_CPU_STATUS, BCHP_M2MC_WRAP_L2_CPU_STATUS_M2MC_INTR_SHIFT);
#elif defined(BCHP_M2MC_TOP_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_M2MC_TOP_L2_CPU_STATUS, BCHP_M2MC_TOP_L2_CPU_STATUS_M2MC_INTR_SHIFT);
#elif defined(BCHP_WRAP_M2MC_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_WRAP_M2MC_L2_CPU_STATUS, BCHP_WRAP_M2MC_L2_CPU_STATUS_M2MC_0_INTR_SHIFT);
#elif defined(BCHP_M2MC_L2_CPU_STATUS)
    BINT_Id IntID = BCHP_INT_ID_CREATE(BCHP_M2MC_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_M2MC_INTR_SHIFT);
#elif defined(BCHP_INT_ID_M2MC_INTR)
    BINT_Id IntID = BCHP_INT_ID_M2MC_INTR;
#else
    #error need port this chip for intID
#endif

    BDBG_ENTER(BGRC_Open);
    BDBG_ASSERT( phGrc );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hMemory );
    BDBG_ASSERT( hInterrupt );

    /* allocate memory for private data */
    hGrc = (BGRC_Handle) BKNI_Malloc( sizeof (BGRC_P_Handle) );
    if( hGrc == 0 )
    {
        err = BERR_OUT_OF_SYSTEM_MEMORY;
        goto fail;
    }

    /* clear main data structure */
    BKNI_Memset( (void *) hGrc, 0, sizeof (BGRC_P_Handle) );
    BDBG_OBJECT_SET(hGrc, BGRC);

    /* create interrupt events */
    err = BKNI_CreateEvent( &hGrc->hInterruptEvent );
    if( err != BERR_SUCCESS )
        goto fail;

    BKNI_ResetEvent( hGrc->hInterruptEvent );

    err = BKNI_CreateEvent( &hGrc->hPeriodicEvent );
    if( err != BERR_SUCCESS )
        goto fail;

    BKNI_ResetEvent( hGrc->hPeriodicEvent );
    BKNI_SetEvent( hGrc->hPeriodicEvent );

    /* set private data */
    hGrc->hChip = hChip;
    hGrc->hRegister = hRegister;
    hGrc->hMemory = hMemory;
    hGrc->hInterrupt = hInterrupt;
    hGrc->ulPacketMemoryMax = pDefSettings ? pDefSettings->ulPacketMemoryMax : BGRC_P_DEFAULT_SETTINGS.ulPacketMemoryMax;
    hGrc->ulOperationMax = pDefSettings ? pDefSettings->ulOperationMax : BGRC_P_DEFAULT_SETTINGS.ulOperationMax;
    hGrc->ulDeviceNum = pDefSettings ? pDefSettings->ulDeviceNum : BGRC_P_DEFAULT_SETTINGS.ulDeviceNum;
    hGrc->ulWaitTimeout = pDefSettings ? pDefSettings->ulWaitTimeout : BGRC_P_DEFAULT_SETTINGS.ulWaitTimeout;
    hGrc->bPreAllocMemory = pDefSettings ? pDefSettings->bPreAllocMemory : BGRC_P_DEFAULT_SETTINGS.bPreAllocMemory;
    hGrc->bUninitialized = true;
    pState = &hGrc->CurrentState;

    hGrc->ulPacketMemoryMax = BGRC_P_MAX(hGrc->ulPacketMemoryMax, BGRC_P_LIST_BLOCK_MIN_SIZE);
    hGrc->ulOperationMax = BGRC_P_MAX(hGrc->ulOperationMax, BGRC_P_OPERATION_MIN);

    /* validate choosen m2mc device */
#if (!defined(BCHP_M2MC_1_REG_START)) && (!defined(BCHP_M2MC1_REG_START))
    if( hGrc->ulDeviceNum != 0 )
    {
        err = BGRC_ERR_M2MC_DEVICE_NUM_INVALID;
        goto fail;
    }
#endif
    if( hGrc->ulDeviceNum > 1 )
    {
        err = BGRC_ERR_M2MC_DEVICE_NUM_INVALID;
        goto fail;
    }

    /* reset m2mc */
#if defined(BCHP_MEMC16_GFX_GRB_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_MEMC16_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(MEMC16_GFX_GRB_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_MEMC16_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(MEMC16_GFX_GRB_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_MEMC_GFX_GRB_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_MEMC_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(MEMC_GFX_GRB_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_MEMC_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(MEMC_GFX_GRB_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_GRAPHICS_GRB_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_GRAPHICS_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GRAPHICS_GRB_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_GRAPHICS_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GRAPHICS_GRB_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_GFX_GR_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GR_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GR_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_M2MC_WRAP_GR_BRIDGE_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_WRAP_GR_BRIDGE_SW_RESET_0, BCHP_FIELD_ENUM(M2MC_WRAP_GR_BRIDGE_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_WRAP_GR_BRIDGE_SW_RESET_0, BCHP_FIELD_ENUM(M2MC_WRAP_GR_BRIDGE_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0)
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
#elif defined(BCHP_M2MC_TOP_GR_BRIDGE_SW_RESET_0)
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_RESET_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_TOP_GR_BRIDGE_SW_RESET_0, BCHP_FIELD_ENUM(M2MC_TOP_GR_BRIDGE_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
#elif defined(BCHP_GFX_GR_SW_INIT_0)
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_MASK );
    while( BREG_Read32(hRegister, BCHP_M2MC_BLIT_STATUS) == 0 );
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
    BREG_Write32( hGrc->hRegister, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
#elif defined(BCHP_M2MC1_REG_START) /* 40 nm chips, 28 nm chips, ... */
    if( hGrc->ulDeviceNum )
    {
        /* we are using m2mc 1 */
        BREG_Write32( hGrc->hRegister, BCHP_M2MC1_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC1_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC1_CLK_GATE_AND_SW_INIT_CONTROL, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_MASK );
        while( BREG_Read32(hRegister, BCHP_M2MC1_BLIT_STATUS) == 0 );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC1_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC1_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
    }
    else
    {
        /* we are using m2mc 0 */
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL, BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_MASK );
        while( BREG_Read32(hRegister, BCHP_M2MC_BLIT_STATUS) == 0 );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_GR_SW_INIT_0, BCHP_FIELD_ENUM(M2MC_GR_SW_INIT_0, M2MC_CLK_108_SW_INIT, DEASSERT) );
    }
#elif defined(BCHP_GFX_GRB_SW_RESET_0)
    if( hGrc->ulDeviceNum )
    {
        /* we are using m2mc 1 */
        #if defined(BCHP_M2MC_1_GRB_SW_RESET_0)
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_1_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_M2MC_1_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
        #else
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_1_SW_RESET, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_1_SW_RESET, DEASSERT) );
        #endif
    }
    else
    {
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_SW_RESET, ASSERT) );
        BREG_Write32( hGrc->hRegister, BCHP_GFX_GRB_SW_RESET_0, BCHP_FIELD_ENUM(GFX_GRB_SW_RESET_0, M2MC_SW_RESET, DEASSERT) );
    }
#else
    #error need port for m2mc init
#endif

    /* reset m2mc list */
    BGRC_P_WRITE_REG( LIST_CTRL, BCHP_FIELD_ENUM(M2MC_LIST_CTRL, RUN, Stop) );

    /* create interrupt callback */
#if defined(BCHP_INT_ID_M2MC_1_INTR)
    IntID = hGrc->ulDeviceNum ? BCHP_INT_ID_M2MC_1_INTR : BCHP_INT_ID_M2MC_INTR;
#elif defined(BCHP_INT_ID_M2MC_1_L2_M2MC_INTR)
    IntID = hGrc->ulDeviceNum ? BCHP_INT_ID_M2MC_1_L2_M2MC_INTR : BCHP_INT_ID_GFX_L2_M2MC_INTR;
#elif defined(BCHP_M2MC1_REG_START)
    IntID = hGrc->ulDeviceNum ? BCHP_INT_ID_M2MC1_L2_M2MC_INTR : IntID;
#endif

    err = BINT_CreateCallback( &hGrc->hInterruptCallback, hInterrupt, IntID, BGRC_P_List_PacketIsr, hGrc, 0 );
    if( err != BERR_SUCCESS )
        goto fail;

    /* clear interrupt callback */
    err = BINT_ClearCallback( hGrc->hInterruptCallback );
    if( err != BERR_SUCCESS )
        goto fail;

    /* enable interrupt callback */
    err = BINT_EnableCallback( hGrc->hInterruptCallback );
    if( err != BERR_SUCCESS )
        goto fail;

    /* initialize list packet memory */
    if( !BGRC_P_List_InitPacketMemory( hGrc, hGrc->bPreAllocMemory ? hGrc->ulPacketMemoryMax : BGRC_P_LIST_BLOCK_SIZE ) )
    {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto fail;
    }

    /* create surface for waiting */
    err = BSUR_Surface_Create( hGrc->hMemory, 1, 1, 0, NULL, BPXL_eA8_R8_G8_B8, 0, 0, NULL, &hGrc->hWaitSurface );
    if( err != BERR_SUCCESS )
        return BERR_TRACE(err);

    /* preallocate memory for operation structures */
    if( hGrc->bPreAllocMemory )
    {
        if( !BGRC_P_Operation_Prealloc( hGrc, hGrc->ulOperationMax ) )
        {
            err = BERR_OUT_OF_SYSTEM_MEMORY;
            goto fail;
        }
    }

#if defined(BCHP_M2MC_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_MASK)
    BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, BLOCK_AUTO_SPLIT_FIFO, ENABLE );
#endif

    /* set palette bypass field */
    BGRC_P_SET_FIELD_ENUM( DEST_SURFACE_FORMAT_DEF_3, PALETTE_BYPASS, DONT_LOOKUP );
    pState->bDstPaletteBypass = true;

    /* set default source state */
    BGRC_Source_SetSurface( hGrc, NULL );
    BGRC_Source_SetAlphaSurface( hGrc, NULL );
    BGRC_Source_TogglePaletteBypass( hGrc, false );
    BGRC_Source_SetChromaExpansion( hGrc, BGRC_ChromaExpansion_eReplicate );
    BGRC_Source_SetZeroPad( hGrc, false );
    BGRC_Source_SetColorKey( hGrc, 0, 0, 0xFFFFFFFF, 0, 0xFFFFFFFF, false );
    BGRC_Source_ToggleColorKey( hGrc, false );
    BGRC_Source_SetColorMatrix5x4( hGrc, ai32_Matrix, 0 );
    BGRC_Source_ToggleColorMatrix( hGrc, false );
    BGRC_Source_SetColorMatrixRounding( hGrc, BGRC_Rounding_eTruncate );
    BGRC_Source_SetColor( hGrc, 0xFF000000 );
    BGRC_Source_SetRectangle( hGrc, 0, 0, 0, 0 );
    BGRC_Source_SetFilterCoeffs( hGrc, BGRC_FilterCoeffs_eSharp, BGRC_FilterCoeffs_eSharp );
    BGRC_Source_ToggleFilter( hGrc, false, false );
    BGRC_Source_SetFilterPhaseAdjustment( hGrc, 0, 0, 0 );
    BGRC_Source_SetScaleRounding( hGrc, BGRC_Rounding_eNearest );
    BGRC_Source_SetScaleAlphaAdjust( hGrc, false );

    BGRC_Source_SetKeyMatrixOrder( hGrc, BGRC_KeyMatrixOrder_eKeyThenMatrix );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_MASK)
    BGRC_Source_SetScaleAlphaPreMultiply( hGrc, false );
#endif

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_MASK)
    BGRC_Source_SetAlphaPreMultiply( hGrc, false );
#endif

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_MASK)
    BGRC_Source_SetAlphaPreMultiplyOffset( hGrc, false );
#endif

#if defined(BCHP_M2MC_SCALER_CTRL_EDGE_CONDITION_MASK)
    BGRC_Source_SetScaleEdgeCondition( hGrc, BGRC_EdgeCondition_eReplicateLast );
#endif

#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)
    BGRC_Source_SetMacroBlock( hGrc, BGRC_MacroBlockRange_None, BGRC_MacroBlockRange_None, 0, 0 );
    BGRC_Source_SetMacroBlock_StripWidth( hGrc, BGRC_P_YCbCr420_STRIP_WIDTH );
#endif

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    BGRC_Source_SetMacroBlock_LinearFormat( hGrc, false );
#endif

    /* set default destination state */
    BGRC_Destination_SetSurface( hGrc, NULL );
    BGRC_Destination_SetAlphaSurface( hGrc, NULL );
    BGRC_Destination_SetChromaExpansion( hGrc, BGRC_ChromaExpansion_eReplicate );
    BGRC_Destination_SetZeroPad( hGrc, false );
    BGRC_Destination_SetColorKey( hGrc, 0, 0, 0xFFFFFFFF, 0, 0xFFFFFFFF, false );
    BGRC_Destination_ToggleColorKey( hGrc, false );
    BGRC_Destination_TogglePaletteBypass( hGrc, false );
    BGRC_Destination_SetColor( hGrc, 0xFF000000 );
    BGRC_Destination_SetRectangle( hGrc, 0, 0, 0, 0 );

    /* set default pattern state */
    BGRC_Pattern_Set( hGrc, 0, aucPattern, 0, 0 );

    /* set default blend state */
    BGRC_Blend_SetColor( hGrc, 0xFF000000 );

    err = BGRC_Blend_SetColorBlend( hGrc,
        BGRC_Blend_Source_eSourceColor, BGRC_Blend_Source_eOne, false,
        BGRC_Blend_Source_eZero, BGRC_Blend_Source_eZero, false,
        BGRC_Blend_Source_eZero );
    if( err != BERR_SUCCESS )
        goto fail;

    err = BGRC_Blend_SetAlphaBlend( hGrc,
        BGRC_Blend_Source_eSourceAlpha, BGRC_Blend_Source_eOne, false,
        BGRC_Blend_Source_eZero, BGRC_Blend_Source_eZero, false,
        BGRC_Blend_Source_eZero );
    if( err != BERR_SUCCESS )
        goto fail;

    /* set default output state */
    BGRC_Output_SetSurface( hGrc, NULL );
    BGRC_Output_SetAlphaSurface( hGrc, NULL );
    BGRC_Output_SetRectangle( hGrc, 0, 0, 0, 0 );
    BGRC_Output_SetColorKeySelection( hGrc,
        BGRC_Output_ColorKeySelection_eTakeSource,
        BGRC_Output_ColorKeySelection_eTakeSource,
        BGRC_Output_ColorKeySelection_eTakeDestination,
        BGRC_Output_ColorKeySelection_eTakeDestination );
    BGRC_Output_SetDither( hGrc, false );
    BGRC_Source_SetFixedScaleFactor( hGrc, 0, 0, 0, 0 );

    hGrc->bUninitialized = false;

    /* copy current state to default state */
    BGRC_P_Source_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );
    BGRC_P_Destination_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );
    BGRC_P_Pattern_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );
    BGRC_P_Blend_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );
    BGRC_P_Output_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    /* enable loading of all registers groups */
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_COLOR_KEY_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_COLOR_MATRIX_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_COLOR_KEY_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
    BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, ROP_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    /* set return handle */
    *phGrc = hGrc;

#if (BCHP_CHIP==7405)
{
    uint32_t reg;
    reg = BREG_Read32(hRegister, BCHP_MEMC_1_1_CLIENT_INFO_2);
    reg |= BCHP_MEMC_1_1_CLIENT_INFO_2_RR_EN_MASK;
    BREG_Write32(hRegister, BCHP_MEMC_1_1_CLIENT_INFO_2, reg);
    reg = BREG_Read32(hRegister, BCHP_MEMC_1_1_CLIENT_INFO_2);
}
#endif

    BDBG_LEAVE(BGRC_Open);
    return BERR_SUCCESS;

fail:
    if( hGrc )
    {
        if( hGrc->hWaitSurface )
            BSUR_Surface_Destroy( hGrc->hWaitSurface );

        if( hGrc->hPeriodicEvent )
            BKNI_DestroyEvent( hGrc->hPeriodicEvent );

        if( hGrc->hInterruptEvent )
            BKNI_DestroyEvent( hGrc->hInterruptEvent );

        if( hGrc->hInterruptCallback )
            BINT_DestroyCallback( hGrc->hInterruptCallback );

        BKNI_Free( (void *) hGrc );
    }

    BDBG_LEAVE(BGRC_Open);
    return BERR_TRACE(err);
}

/***************************************************************************/
void BGRC_Close(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Close);
    BDBG_ASSERT( hGrc );

    /* wait for device to finish */
    (void)BGRC_WaitForOperationsComplete( hGrc, NULL, NULL );

    /* free operation memory */
    BGRC_P_Operation_CleanupList( hGrc );
    BGRC_P_Operation_FreeAll( hGrc );

    /* free list packet memory */
    BGRC_P_List_FreePacketMemory( hGrc );

    /* free wait surface */
    BSUR_Surface_Destroy( hGrc->hWaitSurface );

    /* destroy interrupt events */
    BKNI_DestroyEvent( hGrc->hInterruptEvent );
    BKNI_DestroyEvent( hGrc->hPeriodicEvent );

    /* disable and destroy interrupt callback */
    BINT_DisableCallback( hGrc->hInterruptCallback );
    BINT_DestroyCallback( hGrc->hInterruptCallback );

    /* free memory for private data */
    BDBG_OBJECT_DESTROY(hGrc, BGRC);
    BKNI_Free( (void *) hGrc );

    BDBG_LEAVE(BGRC_Close);
}

/***************************************************************************/
BERR_Code BGRC_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_ResetState);
    BDBG_ASSERT( hGrc );

    /* reset all states */
    BGRC_Source_ResetState( hGrc );
    BGRC_Destination_ResetState( hGrc );
    BGRC_Blend_ResetState( hGrc );
    BGRC_Pattern_ResetState( hGrc );
    BGRC_Output_ResetState( hGrc );

    BDBG_LEAVE(BGRC_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_P_IssueState(
    BGRC_Handle   hGrc,
    BGRC_Callback pCallback_isr,
    void         *pData )
{
    BGRC_P_State *pState;
    BERR_Code err;

    pState = &hGrc->CurrentState;

    /* check if there is an output surface */
    if( pState->OutSurface.hSurface == 0 )
        return BERR_TRACE(BGRC_ERR_NO_OUTPUT_SURFACE);

    /* validate surface dimensions */
    if( !BGRC_P_VALIDATE_SURFACE_RECTANGLE( &pState->SrcRect ) )
        return BERR_TRACE(BGRC_ERR_SOURCE_DIMENSIONS_INVALID);

    if( !BGRC_P_VALIDATE_SURFACE_RECTANGLE( &pState->DstRect ) )
        return BERR_TRACE(BGRC_ERR_DESTINATION_DIMENSIONS_INVALID);

    if( !BGRC_P_VALIDATE_SURFACE_RECTANGLE( &pState->OutRect ) )
        return BERR_TRACE(BGRC_ERR_OUTPUT_DIMENSIONS_INVALID);

    /* set surface dimensions */
    pState->SrcSurface.ulX = pState->SrcRect.ulX;
    pState->SrcSurface.ulY = pState->SrcRect.ulY;
    pState->SrcSurface.ulWidth = pState->SrcRect.ulWidth ? pState->SrcRect.ulWidth : pState->SrcSurface.ulSurfaceWidth;
    pState->SrcSurface.ulHeight = pState->SrcRect.ulHeight ? pState->SrcRect.ulHeight : pState->SrcSurface.ulSurfaceHeight;
    pState->DstSurface.ulX = pState->DstRect.ulX;
    pState->DstSurface.ulY = pState->DstRect.ulY;
    pState->DstSurface.ulWidth = pState->DstRect.ulWidth ? pState->DstRect.ulWidth : pState->DstSurface.ulSurfaceWidth;
    pState->DstSurface.ulHeight = pState->DstRect.ulHeight ? pState->DstRect.ulHeight : pState->DstSurface.ulSurfaceHeight;
    pState->OutSurface.ulX = pState->OutRect.ulX;
    pState->OutSurface.ulY = pState->OutRect.ulY;
    pState->OutSurface.ulWidth = pState->OutRect.ulWidth ? pState->OutRect.ulWidth : pState->OutSurface.ulSurfaceWidth;
    pState->OutSurface.ulHeight = pState->OutRect.ulHeight ? pState->OutRect.ulHeight : pState->OutSurface.ulSurfaceHeight;

    /* validate surface rectangles */
    if( !BGRC_P_VALIDATE_SURFACE_BOUNDS( pState, Src ) )
        return BERR_TRACE(BGRC_ERR_SOURCE_RECT_OUT_OF_BOUNDS);

    if( !BGRC_P_VALIDATE_SURFACE_BOUNDS( pState, Dst ) )
        return BERR_TRACE(BGRC_ERR_DESTINATION_RECT_OUT_OF_BOUNDS);

    if( !BGRC_P_VALIDATE_SURFACE_BOUNDS( pState, Out ) )
        return BERR_TRACE(BGRC_ERR_OUTPUT_RECT_OUT_OF_BOUNDS);

    /* check if destination and output rectangle sizes are the same */
    if( pState->DstSurface.hSurface && pState->OutSurface.hSurface && (
        (pState->DstSurface.ulWidth != pState->OutSurface.ulWidth) ||
        (pState->DstSurface.ulHeight != pState->OutSurface.ulHeight)) )
        return BERR_TRACE(BGRC_ERR_DESTINATION_DIMENSIONS_INVALID);

    /* check if rectangle has odd egdes when the surface is YCbCr422 */
    if( pState->SrcSurface.hSurface && BPXL_IS_YCbCr422_FORMAT(pState->SrcSurface.eFormat) &&
        ((pState->SrcSurface.ulX & 1) || (pState->SrcSurface.ulWidth & 1)) )
        return BERR_TRACE(BGRC_ERR_YCBCR422_SURFACE_HAS_ODD_EDGE);

    if( pState->DstSurface.hSurface && BPXL_IS_YCbCr422_FORMAT(pState->DstSurface.eFormat) &&
        ((pState->DstSurface.ulX & 1) || (pState->DstSurface.ulWidth & 1)) )
        return BERR_TRACE(BGRC_ERR_YCBCR422_SURFACE_HAS_ODD_EDGE);

    if( pState->OutSurface.hSurface && BPXL_IS_YCbCr422_FORMAT(pState->OutSurface.eFormat) &&
        ((pState->OutSurface.ulX & 1) || (pState->OutSurface.ulWidth & 1)) )
        return BERR_TRACE(BGRC_ERR_YCBCR422_SURFACE_HAS_ODD_EDGE);

    /* Fire interrupt periodically so that operations can be marked as completed and memory can be reallocated. */
    if( hGrc->bPreAllocMemory )
    {
        if( hGrc->ulPacketMemorySinceInterrupt > BGRC_P_LIST_BLOCK_SIZE * 4 )
        {
            hGrc->bPeriodicInterrupt = true;
            hGrc->ulPeriodicInterrupts++;
            hGrc->ulPacketMemorySinceInterrupt = 0;
        }
    }

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    /* check for YCbCr420 source */
    hGrc->bYCbCr420Source = false;
    if( ((pState->SrcSurface.eFormat == BPXL_eY8) || (pState->SrcSurface.eFormat == BPXL_eA8_Y8)) &&
        ((pState->SrcAlphaSurface.eFormat == BPXL_eCb8_Cr8) || (pState->SrcAlphaSurface.eFormat == BPXL_eCr8_Cb8)) )
    {
        hGrc->bYCbCr420Source = true;
    }
#endif

    /* check for no scale filtering */
    hGrc->bNoScaleFilter = false;
    if( pState->SrcSurface.hSurface &&
        (pState->SrcSurface.ulWidth == pState->OutSurface.ulWidth) &&
        (pState->SrcSurface.ulHeight == pState->OutSurface.ulHeight)  )
    {
        if( pState->bHorzFilter || pState->bVertFilter )
            hGrc->bNoScaleFilter = true;

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
        if( hGrc->bYCbCr420Source )
            hGrc->bNoScaleFilter = true;
#endif
    }

    /* check if scaling or filtering */
    if( hGrc->bNoScaleFilter || (pState->SrcSurface.hSurface && (
        (pState->SrcSurface.ulWidth != pState->OutSurface.ulWidth) ||
        (pState->SrcSurface.ulHeight != pState->OutSurface.ulHeight))) )
    {
        /* start filter blit operation */
        err = BGRC_P_FilterBlit( hGrc, pCallback_isr, pData, hGrc->bSetEvent );
        if( err != BERR_SUCCESS )
            return BERR_TRACE(err);
    }
    else
    {
        pState->ulHorzScalerStep = (1 << BGRC_P_SCALER_STEP_FRAC_BITS);
        pState->ulVertScalerStep = (1 << BGRC_P_SCALER_STEP_FRAC_BITS);
        pState->ulHorzAveragerCount = 1;
        pState->ulVertAveragerCount = 1;

        /* start blit operation */
        err = BGRC_P_Blit( hGrc, pCallback_isr, pData, hGrc->bSetEvent );
        if( err != BERR_SUCCESS )
            return err;
    }

    /* dump debug info */
    if( pState->SrcSurface.hSurface && pState->DstSurface.hSurface )
    {
        BDBG_MSG(("GRC Blit: Src(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) + Dst(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) -> Out(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) %s",
            pState->SrcSurface.ulOffset, pState->SrcSurface.eFormat, pState->SrcSurface.ulX, pState->SrcSurface.ulY, pState->SrcSurface.ulWidth, pState->SrcSurface.ulHeight, pState->SrcSurface.ulSurfaceWidth, pState->SrcSurface.ulSurfaceHeight,
            pState->DstSurface.ulOffset, pState->DstSurface.eFormat, pState->DstSurface.ulX, pState->DstSurface.ulY, pState->DstSurface.ulWidth, pState->DstSurface.ulHeight, pState->DstSurface.ulSurfaceWidth, pState->DstSurface.ulSurfaceHeight,
            pState->OutSurface.ulOffset, pState->OutSurface.eFormat, pState->OutSurface.ulX, pState->OutSurface.ulY, pState->OutSurface.ulWidth, pState->OutSurface.ulHeight, pState->OutSurface.ulSurfaceWidth, pState->OutSurface.ulSurfaceHeight,
            hGrc->bSetEvent ? "wait" : "async"));
    }
    else if( pState->SrcSurface.hSurface )
    {
        BDBG_MSG(("GRC Blit: Src(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) -> Out(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) %s",
            pState->SrcSurface.ulOffset, pState->SrcSurface.eFormat, pState->SrcSurface.ulX, pState->SrcSurface.ulY, pState->SrcSurface.ulWidth, pState->SrcSurface.ulHeight, pState->SrcSurface.ulSurfaceWidth, pState->SrcSurface.ulSurfaceHeight,
            pState->OutSurface.ulOffset, pState->OutSurface.eFormat, pState->OutSurface.ulX, pState->OutSurface.ulY, pState->OutSurface.ulWidth, pState->OutSurface.ulHeight, pState->OutSurface.ulSurfaceWidth, pState->OutSurface.ulSurfaceHeight,
            hGrc->bSetEvent ? "wait" : "async"));
    }
    else if( pState->DstSurface.hSurface )
    {
        BDBG_MSG(("GRC Blit: Dst(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) -> Out(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) %s",
            pState->DstSurface.ulOffset, pState->DstSurface.eFormat, pState->DstSurface.ulX, pState->DstSurface.ulY, pState->DstSurface.ulWidth, pState->DstSurface.ulHeight, pState->DstSurface.ulSurfaceWidth, pState->DstSurface.ulSurfaceHeight,
            pState->OutSurface.ulOffset, pState->OutSurface.eFormat, pState->OutSurface.ulX, pState->OutSurface.ulY, pState->OutSurface.ulWidth, pState->OutSurface.ulHeight, pState->OutSurface.ulSurfaceWidth, pState->OutSurface.ulSurfaceHeight,
            hGrc->bSetEvent ? "wait" : "async"));
    }
    else
    {
        BDBG_MSG(("GRC Blit: Out(o=%08x,f=%08x,r=%dx%d-%dx%d,s=%dx%d) %s",
            pState->OutSurface.ulOffset, pState->OutSurface.eFormat, pState->OutSurface.ulX, pState->OutSurface.ulY, pState->OutSurface.ulWidth, pState->OutSurface.ulHeight, pState->OutSurface.ulSurfaceWidth, pState->OutSurface.ulSurfaceHeight,
            hGrc->bSetEvent ? "wait" : "async"));
    }

    /* disable set event flag */
    hGrc->bSetEvent = false;
    hGrc->bPeriodicInterrupt = false;

    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_P_IssueStateAndWait(
    BGRC_Handle hGrc )
{
    uint32_t ulCurrentAddress = 0;
    uint32_t ulPreviousAddress = 0;
    BERR_Code err;

    /* enable set event flag */
    hGrc->bSetEvent = true;

    /* initiate device operation */
    err = BGRC_P_IssueState( hGrc, NULL, NULL );
    if( err != BERR_SUCCESS )
        return BERR_TRACE(err);

    /* wait for the device operation to finish */
    while( BKNI_WaitForEvent( hGrc->hInterruptEvent, hGrc->ulWaitTimeout * 1000 ) == BERR_TIMEOUT )
    {
        ulCurrentAddress = BGRC_P_READ_REG( BLIT_OUTPUT_ADDRESS );
        if( ulCurrentAddress == ulPreviousAddress )
        {
            BGRC_P_PrintRegisters( hGrc );
            return BERR_TRACE(BGRC_ERR_M2MC_DEVICE_IS_HUNG);
        }
        ulPreviousAddress = ulCurrentAddress;
    }

    return BERR_SUCCESS;
}

#ifdef BCHP_M2MC_DCE_PRED_CFG
/***************************************************************************/
void BGRC_P_EnableTestFeature1(
    BGRC_Handle hGrc,
    BSUR_TestFeature1_Settings *pSettings )
{
    uint32_t ulReg = BREG_Read32( hGrc->hRegister, BCHP_M2MC_DCE_PRED_CFG ) & (~(
        BCHP_MASK(M2MC_DCE_PRED_CFG, ENABLE) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, CONVERT_RGB) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, PREDICTION_MODE) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, EDGE_PRED_ENA) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, LEFT_PRED_ENA) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, ABCD_PRED_ENA) |
        BCHP_MASK(M2MC_DCE_PRED_CFG, LS_PRED_ENA)));

    BREG_Write32( hGrc->hRegister, BCHP_M2MC_DCE_PRED_CFG, ulReg |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, ENABLE, Enable) |
        BCHP_FIELD_DATA(M2MC_DCE_PRED_CFG, PREDICTION_MODE, pSettings->ulPredictionMode) |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, CONVERT_RGB, Disable) |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, EDGE_PRED_ENA, Disable) |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, LEFT_PRED_ENA, Disable) |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, ABCD_PRED_ENA, Disable) |
        BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, LS_PRED_ENA, Enable) );

    ulReg = BREG_Read32( hGrc->hRegister, BCHP_M2MC_DCE_COMPR_CFG1 ) & (~(
        BCHP_MASK(M2MC_DCE_COMPR_CFG1, PIXELS_PER_GROUP) |
        BCHP_MASK(M2MC_DCE_COMPR_CFG1, TGT_BPG)));
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_DCE_COMPR_CFG1, ulReg |
        BCHP_FIELD_ENUM(M2MC_DCE_COMPR_CFG1, PIXELS_PER_GROUP, Four) |
        BCHP_FIELD_DATA(M2MC_DCE_COMPR_CFG1, TGT_BPG, pSettings->ulBitsPerPixel * 2) );

    ulReg = BREG_Read32( hGrc->hRegister, BCHP_M2MC_DCE_VIDEO_CFG ) & (~(
        BCHP_MASK(M2MC_DCE_VIDEO_CFG, VIDEO_FORMAT) |
        BCHP_MASK(M2MC_DCE_VIDEO_CFG, COMP10BIT)));
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_DCE_VIDEO_CFG, ulReg |
        BCHP_FIELD_ENUM(M2MC_DCE_VIDEO_CFG, VIDEO_FORMAT, Fmt_4444) |
        BCHP_FIELD_ENUM(M2MC_DCE_VIDEO_CFG, COMP10BIT, Eight_bit) );

    BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_FORMAT_DEF_1, FORMAT_TYPE, 9 );
}

/***************************************************************************/
void BGRC_P_DisableTestFeature1(
    BGRC_Handle hGrc )
{
    uint32_t ulReg = BREG_Read32( hGrc->hRegister, BCHP_M2MC_DCE_PRED_CFG ) & (~BCHP_MASK(M2MC_DCE_PRED_CFG, ENABLE));
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_DCE_PRED_CFG, ulReg | BCHP_FIELD_ENUM(M2MC_DCE_PRED_CFG, ENABLE, Disable) );
}

/***************************************************************************/
BERR_Code BGRC_P_IssueStateTestFeature1(
    BGRC_Handle   hGrc,
    BGRC_Callback pCallback_isr,
    void         *pData )
{
    BERR_Code err = BERR_SUCCESS;

    BSUR_TestFeature1_Settings stTestFeature1Settings;
    BSUR_Surface_GetTestFeature1( hGrc->CurrentState.OutSurface.hSurface, &stTestFeature1Settings );

    BGRC_WaitForOperationsComplete( hGrc, NULL, NULL );
    BGRC_P_EnableTestFeature1( hGrc, &stTestFeature1Settings );

    err = BGRC_P_IssueStateAndWait( hGrc );

    BGRC_P_DisableTestFeature1( hGrc );
    BGRC_WaitForOperationsComplete( hGrc, pCallback_isr, pData );

    return BERR_TRACE(err);
}
#endif

/***************************************************************************/
BERR_Code BGRC_IssueState(
    BGRC_Handle   hGrc,
    BGRC_Callback pCallback_isr,
    void         *pData )
{
#ifdef BCHP_M2MC_DCE_PRED_CFG
    BSUR_TestFeature1_Settings stTestFeature1Settings;
#endif

    BDBG_ENTER(BGRC_IssueState);
    BDBG_ASSERT( hGrc );

    if( hGrc->CurrentState.OutSurface.hSurface == 0 )
        return BERR_TRACE(BGRC_ERR_NO_OUTPUT_SURFACE);

#ifdef BCHP_M2MC_DCE_PRED_CFG
    BSUR_Surface_GetTestFeature1( hGrc->CurrentState.OutSurface.hSurface, &stTestFeature1Settings );
    if( stTestFeature1Settings.bEnable )
    {
        BGRC_P_IssueStateTestFeature1( hGrc, pCallback_isr, pData );
    }
    else
#endif
    {
        BERR_Code err = BGRC_P_IssueState( hGrc, pCallback_isr, pData );
        if( err != BERR_SUCCESS )
        {
            BDBG_LEAVE(BGRC_IssueState);
            return BERR_TRACE(err);
        }
    }

    BDBG_LEAVE(BGRC_IssueState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_IssueStateAndWait(
    BGRC_Handle hGrc )
{
#ifdef BCHP_M2MC_DCE_PRED_CFG
    BSUR_TestFeature1_Settings stTestFeature1Settings;
#endif

    BDBG_ENTER(BGRC_IssueStateAndWait);
    BDBG_ASSERT( hGrc );

    if( hGrc->CurrentState.OutSurface.hSurface == 0 )
        return BERR_TRACE(BGRC_ERR_NO_OUTPUT_SURFACE);

#ifdef BCHP_M2MC_DCE_PRED_CFG
    BSUR_Surface_GetTestFeature1( hGrc->CurrentState.OutSurface.hSurface, &stTestFeature1Settings );
    if( stTestFeature1Settings.bEnable )
    {
        BGRC_P_IssueStateTestFeature1( hGrc, NULL, NULL );
    }
    else
#endif
    {
        BERR_Code err = BGRC_P_IssueStateAndWait( hGrc );
        if( err != BERR_SUCCESS )
        {
            BDBG_LEAVE(BGRC_IssueStateAndWait);
            return BERR_TRACE(err);
        }
    }

    BDBG_LEAVE(BGRC_IssueStateAndWait);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_OperationsComplete(
    BGRC_Handle hGrc,
    bool *pbComplete )
{
    uint32_t ulBlitStatus;
    uint32_t ulListStatus;

    BDBG_ENTER(BGRC_OperationsComplete);
    BDBG_ASSERT( hGrc );
    BDBG_ASSERT( pbComplete );

    /* check if device is idle */
    ulBlitStatus = BGRC_P_READ_REG( BLIT_STATUS );
    ulListStatus = BGRC_P_READ_REG( LIST_STATUS );

    if( (BCHP_GET_FIELD_DATA(ulListStatus, M2MC_LIST_STATUS, BUSY) == BCHP_M2MC_LIST_STATUS_BUSY_Busy) ||
        (BCHP_GET_FIELD_DATA(ulListStatus, M2MC_LIST_STATUS, FINISHED) == BCHP_M2MC_LIST_STATUS_FINISHED_NotFinished) ||
        (BCHP_GET_FIELD_DATA(ulBlitStatus, M2MC_BLIT_STATUS, STATUS) == BCHP_M2MC_BLIT_STATUS_STATUS_RUNNING) )
        *pbComplete = false;
    else
        *pbComplete = true;

    /* wait for dummy blit if device is idle to flush operation processing */
    if( *pbComplete )
    {
        BERR_Code err = BGRC_WaitForOperationsComplete( hGrc, 0, 0 );
        if( err != BERR_SUCCESS )
        {
            BDBG_ERR(( "BGRC_WaitForOperationsComplete FAILED IN BGRC_OperationsComplete" ));
            return BERR_TRACE(err);
        }
    }

    BDBG_LEAVE(BGRC_OperationsComplete);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_WaitForOperationReady(
    BGRC_Handle hGrc,
    BGRC_Callback pCallback_isr,
    void         *pData )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BGRC_WaitForOperationReady);
    BDBG_ASSERT( hGrc );

    if( pCallback_isr )
    {
        hGrc->pPeriodicData = pData;
        hGrc->pPeriodicCallback = pCallback_isr;

        if( hGrc->ulPeriodicInterrupts == 0 )
        {
            err = BGRC_WaitForOperationsComplete( hGrc, pCallback_isr, pData );
            if( err != BERR_SUCCESS )
            {
                BDBG_ERR(( "BGRC_WaitForOperationsComplete FAILED IN BGRC_WaitForOperationReady" ));
                return BERR_TRACE(err);
            }
            hGrc->pPeriodicData = NULL;
            hGrc->pPeriodicCallback = NULL;
        }
    }
    else
    {
        while( hGrc->pCurrListBlock->pNextBlock->ulRefCount )
        {
            BERR_Code err = BKNI_WaitForEvent( hGrc->hPeriodicEvent, hGrc->ulWaitTimeout * 1000 );
            if( err != BERR_SUCCESS )
            {
                BDBG_ERR(( "BKNI_WaitForEvent TIMED OUT IN BGRC_WaitForOperationReady." ));
                return BERR_TRACE(err);
            }

            BGRC_P_Operation_CleanupList( hGrc );
        }
    }

    BDBG_LEAVE(BGRC_WaitForOperationReady);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_WaitForOperationsComplete(
    BGRC_Handle hGrc,
    BGRC_Callback pCallback_isr,
    void *pData )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BGRC_WaitForOperationsComplete);
    BDBG_ASSERT( hGrc );

    /* Save current state */
    BGRC_P_Source_CopyState( &hGrc->StoredState, &hGrc->CurrentState, hGrc->aulStoredRegs, hGrc->aulCurrentRegs );
    BGRC_P_Destination_CopyState( &hGrc->StoredState, &hGrc->CurrentState, hGrc->aulStoredRegs, hGrc->aulCurrentRegs );
    BGRC_P_Output_CopyState( &hGrc->StoredState, &hGrc->CurrentState, hGrc->aulStoredRegs, hGrc->aulCurrentRegs );
    BGRC_P_Pattern_CopyState( &hGrc->StoredState, &hGrc->CurrentState, hGrc->aulStoredRegs, hGrc->aulCurrentRegs );
    BGRC_P_Blend_CopyState( &hGrc->StoredState, &hGrc->CurrentState, hGrc->aulStoredRegs, hGrc->aulCurrentRegs );

    /* Do fill and wait */
    err = BGRC_ResetState( hGrc );
    if( err != BERR_SUCCESS )
    {
        BDBG_LEAVE(BGRC_WaitForOperationsComplete);
        return BERR_TRACE(err);
    }

    BGRC_Output_SetSurface( hGrc, hGrc->hWaitSurface );

    if( pCallback_isr )
        err = BGRC_P_IssueState( hGrc, pCallback_isr, pData );
    else
        err = BGRC_P_IssueStateAndWait( hGrc );

    /* Restore current state */
    BGRC_P_Source_CopyState( &hGrc->CurrentState, &hGrc->StoredState, hGrc->aulCurrentRegs, hGrc->aulStoredRegs );
    BGRC_P_Destination_CopyState( &hGrc->CurrentState, &hGrc->StoredState, hGrc->aulCurrentRegs, hGrc->aulStoredRegs );
    BGRC_P_Output_CopyState( &hGrc->CurrentState, &hGrc->StoredState, hGrc->aulCurrentRegs, hGrc->aulStoredRegs );
    BGRC_P_Pattern_CopyState( &hGrc->CurrentState, &hGrc->StoredState, hGrc->aulCurrentRegs, hGrc->aulStoredRegs );
    BGRC_P_Blend_CopyState( &hGrc->CurrentState, &hGrc->StoredState, hGrc->aulCurrentRegs, hGrc->aulStoredRegs );

    BDBG_LEAVE(BGRC_WaitForOperationsComplete);
    if( err != BERR_SUCCESS )
        return BERR_TRACE(err);
    else
        return BERR_SUCCESS;
}

#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_MASK          BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_SHIFT         BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_DEFAULT       BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_MASK          BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_SHIFT         BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_DEFAULT       BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_MASK         BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_SHIFT        BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_DEFAULT      BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_MASK         BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_SHIFT        BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_DEFAULT      BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_MASK       BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_SHIFT      BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_DEFAULT    BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_MASK       BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_SHIFT      BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_DEFAULT    BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB_ADDR_MSB_DEFAULT
#endif
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_MASK     BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_SHIFT    BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_DEFAULT  BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB_ADDR_MSB_DEFAULT
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_MASK     BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_MSB_MASK
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_SHIFT    BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_MSB_SHIFT
#define BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_DEFAULT  BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD_MSB_ADDR_MSB_DEFAULT
#endif
/***************************************************************************/
BERR_Code BGRC_Source_SetSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Source_SetSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->SrcSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        /* validate surface format */
        if( hSurface && BPXL_IS_DEPTH_FORMAT(Surface.eFormat) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        if( hSurface && (BPXL_IS_YCbCr444_10BIT_FORMAT(Surface.eFormat) || BPXL_IS_YCbCr422_10BIT_FORMAT(Surface.eFormat)) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_COMP( SRC_FEEDER_ENABLE, ENABLE, ENABLE, DISABLE, hSurface );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_0_MSB, ADDR, 0 );
#endif
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_0, ADDR, Surface.ulOffset );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_0_BOT_FLD_MSB, ADDR, 0 );
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_0_BOT_FLD, ADDR, 0 );
#endif
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_STRIDE_0, STRIDE, Surface.ulPitch );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_1_MSB, ADDR, 0);
#endif
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_1, ADDR, 0);
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_1_BOT_FLD_MSB, ADDR, 0);
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_1_BOT_FLD, ADDR, 0);
#endif
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_STRIDE_1, STRIDE, 0);
        BGRC_P_SET_FIELD_FORMAT( SRC, FORMAT_TYPE, Surface.eFormat, pState->SrcAlphaSurface.hSurface, pState->bSrcPaletteBypass );
        BGRC_P_SET_FIELD_CHANNELS( SRC, Surface.eFormat, hSurface );

        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_FORMAT_DEF_3, CH0_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 0) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_FORMAT_DEF_3, CH1_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 1) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_FORMAT_DEF_3, CH2_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 2) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_FORMAT_DEF_3, CH3_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 3) );

#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0)
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0, STRIPE_HEIGHT, 0 );
#endif
#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_STRIPED_IMAGE_FORMAT_MASK)
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_1, STRIPED_IMAGE_FORMAT, 0 );
#endif
#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_Y, BGRC_P_MACROBLOCK_RANGE_NONE );
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_C, BGRC_P_MACROBLOCK_RANGE_NONE );
#endif

        /* set registers for YCbCr 420 format */
        if( (Surface.eFormat == BPXL_eY8) || (Surface.eFormat == BPXL_eA8_Y8) )
        {
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0)
            BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0, STRIPE_HEIGHT, Surface.ulSurfaceHeight );
#endif
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_MASK)
            BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0, STRIPE_WIDTH,
                (pState->ulMacroBlockStripWidth == BGRC_P_YCbCr420_STRIP_WIDTH) ? 0 : 1 );
#endif

#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_STRIPED_IMAGE_FORMAT_MASK)
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_1, STRIPED_IMAGE_FORMAT, pState->bMacroBlockLinear ? 0 : 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_1, FORMAT_TYPE, pState->bMacroBlockBigEndian ? 0 : 7 );
#else
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_FORMAT_DEF_1, FORMAT_TYPE, pState->bMacroBlockBigEndian ? 0 : (pState->bMacroBlockLinear ? 7 : 15) );
#endif
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_FORMAT_DEF_3, CH0_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_FORMAT_DEF_3, CH1_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_FORMAT_DEF_3, CH2_DISABLE, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_FORMAT_DEF_3, CH3_DISABLE, (Surface.eFormat == BPXL_eA8_Y8) ? 0 : 1 );

#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_Y, pState->ulMacroBlockRangeY );
#endif
        }

#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0)
        if( BGRC_P_REGISTER_CHANGED( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
#endif

        /* store surface data */
        BKNI_Memcpy( &pState->SrcSurface, &Surface, sizeof (BGRC_P_Surface) );
    }

    BDBG_LEAVE(BGRC_Source_SetSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetAlphaSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Source_SetAlphaSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->SrcAlphaSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        /* validate surface format */
        if( hSurface && BPXL_IS_DEPTH_FORMAT(Surface.eFormat) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_ADDR_1, ADDR, Surface.ulOffset );
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_STRIDE_1, STRIDE, Surface.ulPitch );

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH0_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 0) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH1_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 1) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH2_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 2) );
        BGRC_P_SET_FIELD_COMP_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH3_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 3) );
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, FORMAT_TYPE, 0 );

#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1)
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1, STRIPE_HEIGHT, 0 );
#endif
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1_STRIPED_IMAGE_FORMAT_MASK)
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, STRIPED_IMAGE_FORMAT, 0 );
#endif
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_Y, BGRC_P_MACROBLOCK_RANGE_NONE );
        BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_C, BGRC_P_MACROBLOCK_RANGE_NONE );
#endif

        /* set registers for YCbCr 420 format */
        if( (Surface.eFormat == BPXL_eCb8_Cr8) || (Surface.eFormat == BPXL_eCr8_Cb8) )
        {
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1)
            BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1, STRIPE_HEIGHT, Surface.ulSurfaceHeight );
#endif
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_WIDTH_MASK)
            BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1, STRIPE_WIDTH,
                (pState->ulMacroBlockStripWidth == BGRC_P_YCbCr420_STRIP_WIDTH) ? 0 : 1 );
#endif

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1_STRIPED_IMAGE_FORMAT_MASK)
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, STRIPED_IMAGE_FORMAT, pState->bMacroBlockLinear ? 0 : 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, FORMAT_TYPE, pState->bMacroBlockBigEndian ? 0 : 7 );
#else
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, FORMAT_TYPE, pState->bMacroBlockBigEndian ? 0 : (pState->bMacroBlockLinear ? 7 : 15) );
#endif
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH0_NUM_BITS, BPXL_COMPONENT_SIZE(Surface.eFormat, 0) );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH1_NUM_BITS, BPXL_COMPONENT_SIZE(Surface.eFormat, 1) );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH2_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH3_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH0_LSB_POS, BPXL_COMPONENT_POS(Surface.eFormat, 0) );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH1_LSB_POS, BPXL_COMPONENT_POS(Surface.eFormat, 1) );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH2_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH3_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH0_DISABLE, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH1_DISABLE, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH2_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH3_DISABLE, 1 );

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_MASK)
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_C, pState->ulMacroBlockRangeC );
#endif
        }
        else
        {
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH0_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH1_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH2_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_1, CH3_NUM_BITS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH0_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH1_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH2_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_2, CH3_LSB_POS, 0 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH0_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH1_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH2_DISABLE, 1 );
            BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, CH3_DISABLE, 1 );
        }

        if( BGRC_P_REGISTER_CHANGED( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
#endif

        /* store surface data */
        BKNI_Memcpy( &pState->SrcAlphaSurface, &Surface, sizeof (BGRC_P_Surface) );
    }

    BDBG_LEAVE(BGRC_Source_SetAlphaSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_TogglePaletteBypass(
    BGRC_Handle hGrc,
    bool bEnableBypass )
{
    BGRC_P_State *pState;

    BDBG_ENTER(BGRC_Source_TogglePaletteBypass);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* set palette bypass field */
    BGRC_P_SET_FIELD_COMP( SRC_SURFACE_FORMAT_DEF_3, PALETTE_BYPASS, DONT_LOOKUP, LOOKUP, bEnableBypass );

    /* set format type due to bypass */
    if( pState->SrcSurface.hSurface )
        BGRC_P_SET_FIELD_FORMAT( SRC, FORMAT_TYPE, pState->SrcSurface.eFormat, pState->SrcAlphaSurface.hSurface, bEnableBypass );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SRC_SURFACE_FORMAT_DEF_1 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

    pState->bSrcPaletteBypass = bEnableBypass;

    BDBG_LEAVE(BGRC_Source_TogglePaletteBypass);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetChromaExpansion(
    BGRC_Handle hGrc,
    BGRC_ChromaExpansion eChromaExpansion )
{
    BDBG_ENTER(BGRC_Source_SetChromaExpansion);
    BDBG_ASSERT( hGrc );

    /* set chroma expansion field */
    BGRC_P_SET_FIELD_COMP( SRC_SURFACE_FORMAT_DEF_3, CHROMA_FILTER,
        REPLICATE, FILTER, eChromaExpansion == BGRC_ChromaExpansion_eReplicate );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SRC_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetChromaExpansion);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetZeroPad(
    BGRC_Handle hGrc,
    bool bEnableZeroPad )
{
    BDBG_ENTER(BGRC_Source_SetZeroPad);
    BDBG_ASSERT( hGrc );

    /* set zero pad field */
    BGRC_P_SET_FIELD_COMP( SRC_SURFACE_FORMAT_DEF_3, ZERO_PAD,
        ZERO_PAD, REPLICATE, bEnableZeroPad );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SRC_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetZeroPad);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetColorKey(
    BGRC_Handle hGrc,
    uint32_t ulMin,
    uint32_t ulMax,
    uint32_t ulMask,
    uint32_t ulReplacement,
    uint32_t ulReplacementMask,
    bool bExclusive )
{
    BDBG_ENTER(BGRC_Source_SetColorKey);
    BDBG_ASSERT( hGrc );

    /* set color key fields */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, SRC_COLOR_KEY_COMPARE, EXCUSIVE, INCLUSIVE, bExclusive );
    BGRC_P_SET_FIELD_FULL( SRC_COLOR_KEY_LOW, ulMin );
    BGRC_P_SET_FIELD_FULL( SRC_COLOR_KEY_HIGH, ulMax );
    BGRC_P_SET_FIELD_FULL( SRC_COLOR_KEY_MASK, ulMask );
    BGRC_P_SET_FIELD_FULL( SRC_COLOR_KEY_REPLACEMENT, ulReplacement );
    BGRC_P_SET_FIELD_FULL( SRC_COLOR_KEY_REPLACEMENT_MASK, ulReplacementMask );

    /* set register load fields */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    if( BGRC_P_REGISTER_CHANGED( SRC_COLOR_KEY_LOW ) ||
        BGRC_P_REGISTER_CHANGED( SRC_COLOR_KEY_HIGH ) ||
        BGRC_P_REGISTER_CHANGED( SRC_COLOR_KEY_MASK ) ||
        BGRC_P_REGISTER_CHANGED( SRC_COLOR_KEY_REPLACEMENT ) ||
        BGRC_P_REGISTER_CHANGED( SRC_COLOR_KEY_REPLACEMENT_MASK ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_COLOR_KEY_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetColorKey);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_ToggleColorKey(
    BGRC_Handle hGrc,
    bool bEnable )
{
    BDBG_ENTER(BGRC_Source_ToggleColorKey);
    BDBG_ASSERT( hGrc );

    /* set color key enable field */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, SRC_COLOR_KEY_ENABLE,
        ENABLE, DISABLE, bEnable );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_ToggleColorKey);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetColorMatrix5x4(
    BGRC_Handle hGrc,
    const int32_t ai32_Matrix[],
    uint32_t ulShift )
{
    BDBG_ENTER(BGRC_Source_SetColorMatrix5x4);
    BDBG_ASSERT( hGrc );

    /* set color matrix fields */
    BGRC_P_SET_FIELD_MATRIX_ROW( SRC, 0, ai32_Matrix,  0, ulShift );
    BGRC_P_SET_FIELD_MATRIX_ROW( SRC, 1, ai32_Matrix,  5, ulShift );
    BGRC_P_SET_FIELD_MATRIX_ROW( SRC, 2, ai32_Matrix, 10, ulShift );
    BGRC_P_SET_FIELD_MATRIX_ROW( SRC, 3, ai32_Matrix, 15, ulShift );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SRC_CM_C00_C01 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C02_C03 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C04 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C10_C11 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C12_C13 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C14 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C20_C21 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C22_C23 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C24 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C30_C31 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C32_C33 ) ||
        BGRC_P_REGISTER_CHANGED( SRC_CM_C34 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_COLOR_MATRIX_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetColorMatrix5x4);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_ToggleColorMatrix(
    BGRC_Handle hGrc,
    bool bEnable )
{
    BDBG_ENTER(BGRC_Source_ToggleColorMatrix);
    BDBG_ASSERT( hGrc );

    /* set color matrix enable field */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, SRC_COLOR_MATRIX_ENABLE,
        ENABLE, DISABLE, bEnable );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_ToggleColorMatrix);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetColorMatrixRounding(
    BGRC_Handle hGrc,
    BGRC_Rounding eMatrixRounding )
{
    BDBG_ENTER(BGRC_Source_SetColorMatrixRounding);
    BDBG_ASSERT( hGrc );

    /* set matrix rounding field */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, SRC_COLOR_MATRIX_ROUNDING,
        TRUNCATE, NEAREST, eMatrixRounding == BGRC_Rounding_eTruncate );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetColorMatrixRounding);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetKeyMatrixOrder(
    BGRC_Handle hGrc,
    BGRC_KeyMatrixOrder eKeyMatrixOrder )
{
    BDBG_ENTER(BGRC_Source_SetKeyMatrixOrder);
    BDBG_ASSERT( hGrc );

    /* set colorkey/matrix order field */
#if defined(BCHP_M2MC_BLIT_HEADER_CBAR_SRC_COLOR_KEY_THEN_MATRIX)
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, CBAR_SRC_COLOR, KEY_THEN_MATRIX,
        MATRIX_THEN_KEY, eKeyMatrixOrder == BGRC_KeyMatrixOrder_eKeyThenMatrix );
#else
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, CBAR_SRC_COLOR, SCALE_THEN_KEY_THEN_MATRIX,
        SCALE_THEN_MATRIX_THEN_KEY, eKeyMatrixOrder == BGRC_KeyMatrixOrder_eKeyThenMatrix );
#endif

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetKeyMatrixOrder);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetKeyMatrixScaleOrder(
    BGRC_Handle hGrc,
    BGRC_KeyMatrixScaleOrder eKeyMatrixScaleOrder )
{
    BDBG_ENTER(BGRC_Source_SetKeyMatrixScaleOrder);
    BDBG_ASSERT( hGrc );

    /* set colorkey/matrix order field */
#if defined(BCHP_M2MC_BLIT_HEADER_CBAR_SRC_COLOR_KEY_THEN_MATRIX)
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, CBAR_SRC_COLOR, KEY_THEN_MATRIX, MATRIX_THEN_KEY,
        (eKeyMatrixScaleOrder == BGRC_KeyMatrixScaleOrder_eScaleThenKeyThenMatrix) ||
        (eKeyMatrixScaleOrder == BGRC_KeyMatrixScaleOrder_eKeyThenMatrixThenScale) ||
        (eKeyMatrixScaleOrder == BGRC_KeyMatrixScaleOrder_eKeyThenScaleThenMatrix) );
#else
    BGRC_P_SET_FIELD_DATA( BLIT_HEADER, CBAR_SRC_COLOR, eKeyMatrixScaleOrder );
#endif

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetKeyMatrixScaleOrder);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetColor(
    BGRC_Handle hGrc,
    uint32_t ulColor )
{
    BDBG_ENTER(BGRC_Source_SetColor);
    BDBG_ASSERT( hGrc );

    /* set single color field */
    BGRC_P_SET_FIELD_FULL( SRC_CONSTANT_COLOR, ulColor );
    BGRC_P_SET_FIELD_DATA( SRC_W_ALPHA, W0_ALPHA, BPXL_GET_COMPONENT( BPXL_eA8_R8_G8_B8, ulColor, 3 ) );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SRC_CONSTANT_COLOR ) ||
        BGRC_P_REGISTER_CHANGED( SRC_W_ALPHA ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetColor);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetRectangle(
    BGRC_Handle hGrc,
    uint32_t ulLeft,
    uint32_t ulTop,
    uint32_t ulWidth,
    uint32_t ulHeight )
{
    BDBG_ENTER(BGRC_Source_SetRectangle);
    BDBG_ASSERT( hGrc );

    /* validate dimensions */
    if( !BGRC_P_VALIDATE_SURFACE_DIMENSIONS( ulLeft, ulTop, ulWidth, ulHeight ) )
        return BERR_TRACE(BGRC_ERR_SOURCE_DIMENSIONS_INVALID);

    if( (ulLeft > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulTop > BGRC_P_SURFACE_RECT_SIZE_MAX) ||
        (ulWidth > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulHeight > BGRC_P_SURFACE_RECT_SIZE_MAX) )
        return BERR_TRACE(BGRC_ERR_SOURCE_DIMENSIONS_INVALID);

    /* store width and height */
    hGrc->CurrentState.SrcRect.ulX = ulLeft;
    hGrc->CurrentState.SrcRect.ulY = ulTop;
    hGrc->CurrentState.SrcRect.ulWidth = ulWidth;
    hGrc->CurrentState.SrcRect.ulHeight = ulHeight;

    BDBG_LEAVE(BGRC_Source_SetRectangle);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetFilterCoeffs(
    BGRC_Handle hGrc,
    BGRC_FilterCoeffs eHorizontalCoeffs,
    BGRC_FilterCoeffs eVerticalCoeffs )
{
    BDBG_ENTER(BGRC_Source_SetFilterCoeffs);
    BDBG_ASSERT( hGrc );

    /* set filter coefficients */
    hGrc->CurrentState.eHorzCoeffs = eHorizontalCoeffs;
    hGrc->CurrentState.eVertCoeffs = eVerticalCoeffs;

    BDBG_LEAVE(BGRC_Source_SetFilterCoeffs);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_ToggleFilter(
    BGRC_Handle hGrc,
    bool bEnableHorizontalFilter,
    bool bEnableVerticalFilter )
{
    BDBG_ENTER(BGRC_Source_ToggleFilter);
    BDBG_ASSERT( hGrc );

    /* set filter enables */
    hGrc->CurrentState.bHorzFilter = bEnableHorizontalFilter;
    hGrc->CurrentState.bVertFilter = bEnableVerticalFilter;

    BDBG_LEAVE(BGRC_Source_ToggleFilter);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetFilterPhaseAdjustment(
    BGRC_Handle hGrc,
    int32_t iHorizontalPhase,
    int32_t iVerticalPhase,
    uint32_t ulShift )
{
    BDBG_ENTER(BGRC_Source_SetFilterPhaseAdjustment);
    BDBG_ASSERT( hGrc );

    /* set filter phase */
    hGrc->CurrentState.iHorzPhaseAdj = iHorizontalPhase;
    hGrc->CurrentState.iVertPhaseAdj = iVerticalPhase;
    hGrc->CurrentState.ulPhaseShift = ulShift;

    BDBG_LEAVE(BGRC_Source_SetFilterPhaseAdjustment);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetFixedScaleFactor(
    BGRC_Handle hGrc,
    uint32_t ulHorizontalNumerator,
    uint32_t ulHorizontalDenominator,
    uint32_t ulVerticalNumerator,
    uint32_t ulVerticalDenominator )
{
    BDBG_ENTER(BGRC_Source_SetFixedScaleFactor);
    BDBG_ASSERT( hGrc );

    /* set scale factor */
    hGrc->CurrentState.ulHorzScalerNum = ulHorizontalNumerator;
    hGrc->CurrentState.ulHorzScalerDen = ulHorizontalDenominator;
    hGrc->CurrentState.ulVertScalerNum = ulVerticalNumerator;
    hGrc->CurrentState.ulVertScalerDen = ulVerticalDenominator;

    BDBG_LEAVE(BGRC_Source_SetFixedScaleFactor);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetScaleAlphaPreMultiply(
    BGRC_Handle hGrc,
    bool bAlphaPreMultiply )
{
    BDBG_ENTER(BGRC_Source_SetScaleAlphaPreMultiply);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_MASK)
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, ALPHA_PRE_MULTIPLY, ENABLE, DISABLE, bAlphaPreMultiply );

    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetScaleAlphaPreMultiply);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( bAlphaPreMultiply );
    BDBG_LEAVE(BGRC_Source_SetScaleAlphaPreMultiply);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************/
BERR_Code BGRC_Source_SetScaleAlphaAdjust(
    BGRC_Handle hGrc,
    bool bAdjustAlpha )
{
    BDBG_ENTER(BGRC_Source_SetScaleAlphaAdjust);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_ADJUST_ENABLE)
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, ALPHA_ADJUST, ENABLE, DISABLE, bAdjustAlpha );
#else
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, OFFSET_ADJUST, ENABLE, DISABLE, bAdjustAlpha );
#endif

    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetScaleAlphaAdjust);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetScaleRounding(
    BGRC_Handle hGrc,
    BGRC_Rounding eScaleRounding )
{
    BDBG_ENTER(BGRC_Source_SetScaleRounding);
    BDBG_ASSERT( hGrc );

    /* set scale rounding field */
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, ROUNDING_MODE,
        TRUNCATE, NEAREST, eScaleRounding == BGRC_Rounding_eTruncate );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetScaleRounding);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetScaleEdgeCondition(
    BGRC_Handle hGrc,
    BGRC_EdgeCondition eEdgeCondition )
{
    BDBG_ENTER(BGRC_Source_SetScaleEdgeCondition);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SCALER_CTRL_EDGE_CONDITION_MASK)
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, EDGE_CONDITION, REPLICATE, MIRROR, eEdgeCondition == BGRC_EdgeCondition_eReplicateLast );

    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetScaleEdgeCondition);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( eEdgeCondition );
    BDBG_LEAVE(BGRC_Source_SetScaleEdgeCondition);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************/
BERR_Code BGRC_Source_SetAlphaPreMultiply(
    BGRC_Handle hGrc,
    bool bAlphaPreMultiply )
{
    BDBG_ENTER(BGRC_Source_SetAlphaPreMultiply);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_MASK)
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, ALPHA_PRE_MULTIPLY_ENABLE, ENABLE, DISABLE, bAlphaPreMultiply );

    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetAlphaPreMultiply);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( bAlphaPreMultiply );

    BDBG_LEAVE(BGRC_Source_SetAlphaPreMultiply);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************/
BERR_Code BGRC_Source_SetAlphaPreMultiplyOffset(
    BGRC_Handle hGrc,
    bool bOffset )
{
    BDBG_ENTER(BGRC_Source_SetAlphaPreMultiplyOffset);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_MASK)
    BGRC_P_SET_FIELD_COMP( SCALER_CTRL, ALPHA_PRE_MULTIPLY_OFFSET_EN, ENABLE, DISABLE, bOffset );

    if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetAlphaPreMultiplyOffset);
    return BERR_SUCCESS;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( bOffset );
    BDBG_LEAVE(BGRC_Source_SetAlphaPreMultiplyOffset);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock(
    BGRC_Handle hGrc,
    BGRC_MacroBlockRange eRangeY,
    BGRC_MacroBlockRange eRangeC,
    uint8_t ucScaleFactorY,
    uint8_t ucScaleFactorC )
{
    BDBG_ENTER(BGRC_Source_SetMacroBlock);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)
    if( eRangeY == BGRC_MacroBlockRange_None )
        hGrc->CurrentState.ulMacroBlockRangeY = BGRC_P_MACROBLOCK_RANGE_NONE;
    else if( eRangeY == BGRC_MacroBlockRange_Expansion )
        hGrc->CurrentState.ulMacroBlockRangeY = BGRC_P_MACROBLOCK_RANGE_EXPANSION;
    else if( eRangeY == BGRC_MacroBlockRange_Remapping )
        hGrc->CurrentState.ulMacroBlockRangeY = BGRC_P_MACROBLOCK_RANGE_REMAPPING + ucScaleFactorY;

    BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_Y, hGrc->CurrentState.ulMacroBlockRangeY );
    BGRC_P_SET_FIELD_DATA( SRC_SURFACE_0_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_C, BGRC_P_MACROBLOCK_RANGE_NONE );

    if( eRangeC == BGRC_MacroBlockRange_None )
        hGrc->CurrentState.ulMacroBlockRangeC = BGRC_P_MACROBLOCK_RANGE_NONE;
    else if( eRangeC == BGRC_MacroBlockRange_Expansion )
        hGrc->CurrentState.ulMacroBlockRangeC = BGRC_P_MACROBLOCK_RANGE_EXPANSION;
    else if( eRangeC == BGRC_MacroBlockRange_Remapping )
        hGrc->CurrentState.ulMacroBlockRangeC = BGRC_P_MACROBLOCK_RANGE_REMAPPING + ucScaleFactorC;

    BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_Y, BGRC_P_MACROBLOCK_RANGE_NONE );
    BGRC_P_SET_FIELD_DATA( SRC_SURFACE_1_FORMAT_DEF_3, RANGE_EXP_MAP_SCALE_FACTOR_C, hGrc->CurrentState.ulMacroBlockRangeC );
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( eRangeY );
    BSTD_UNUSED( eRangeC );
    BSTD_UNUSED( ucScaleFactorY );
    BSTD_UNUSED( ucScaleFactorC );
    BDBG_LEAVE(BGRC_Source_SetMacroBlock);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

    BDBG_LEAVE(BGRC_Source_SetMacroBlock);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_StripWidth(
    BGRC_Handle hGrc,
    uint32_t ulStripWidth )
{
    BDBG_ENTER(BGRC_Source_SetMacroBlock_StripWidth);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK)

#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_MASK)
    if( (ulStripWidth != BGRC_P_YCbCr420_STRIP_WIDTH) && (ulStripWidth != BGRC_P_YCbCr420_STRIP_WIDTH * 2) )
        return BERR_TRACE(BERR_INVALID_PARAMETER);
#else
    if( ulStripWidth != BGRC_P_YCbCr420_STRIP_WIDTH )
        return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif

    hGrc->CurrentState.ulMacroBlockStripWidth = ulStripWidth;

#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_MASK)
    BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0, STRIPE_WIDTH,
        (hGrc->CurrentState.ulMacroBlockStripWidth == BGRC_P_YCbCr420_STRIP_WIDTH) ? 0 : 1 );
    BGRC_P_SET_FIELD_DATA( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1, STRIPE_WIDTH,
        (hGrc->CurrentState.ulMacroBlockStripWidth == BGRC_P_YCbCr420_STRIP_WIDTH) ? 0 : 1 );

    if( BGRC_P_REGISTER_CHANGED( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 ) ||
        BGRC_P_REGISTER_CHANGED( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );
#endif

#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( ulStripWidth );
    BDBG_LEAVE(BGRC_Source_SetMacroBlock_StripWidth);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

    BDBG_LEAVE(BGRC_Source_SetMacroBlock_StripWidth);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_LinearFormat(
    BGRC_Handle hGrc,
    bool bLinearFormat )
{
    BDBG_ENTER(BGRC_Source_SetMacroBlock_LinearFormat);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    hGrc->CurrentState.bMacroBlockLinear = bLinearFormat;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( bLinearFormat );
    BDBG_LEAVE(BGRC_Source_SetMacroBlock_LinearFormat);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

    BDBG_LEAVE(BGRC_Source_SetMacroBlock_LinearFormat);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_Endian(
    BGRC_Handle hGrc,
    bool bBigEndian )
{
    BDBG_ENTER(BGRC_Source_SetMacroBlock_Endian);
    BDBG_ASSERT( hGrc );

#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    hGrc->CurrentState.bMacroBlockBigEndian = bBigEndian;
#else
    BSTD_UNUSED( hGrc );
    BSTD_UNUSED( bBigEndian );
    BDBG_LEAVE(BGRC_Source_SetMacroBlock_Endian);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

    BDBG_LEAVE(BGRC_Source_SetMacroBlock_Endian);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetDirection(
    BGRC_Handle hGrc,
    bool bRightToLeft,
    bool bBottomToTop )
{
    BDBG_ENTER(BGRC_Source_SetDirection);
    BDBG_ASSERT( hGrc );

    hGrc->CurrentState.bSrcRightToLeft = bRightToLeft;
    hGrc->CurrentState.bSrcBottomToTop = bBottomToTop;

    BDBG_LEAVE(BGRC_Source_SetDirection);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Source_ResetState);
    BDBG_ASSERT( hGrc );

    /* copy default state to current state */
    BGRC_P_Source_CopyState( &hGrc->CurrentState, &hGrc->DefaultState,
        hGrc->aulCurrentRegs, hGrc->aulDefaultRegs );

    BDBG_LEAVE(BGRC_Source_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Source_SetDefault(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Source_SetDefault);
    BDBG_ASSERT( hGrc );

    /* copy current state to default state */
    BGRC_P_Source_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Source_SetDefault);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Destination_SetSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->DstSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        /* validate surface format */
        if( hSurface && BPXL_IS_DEPTH_FORMAT(Surface.eFormat) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        if( hSurface && (BPXL_IS_YCbCr444_10BIT_FORMAT(Surface.eFormat) || BPXL_IS_YCbCr422_10BIT_FORMAT(Surface.eFormat)) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_COMP( DEST_FEEDER_ENABLE, ENABLE, ENABLE, DISABLE, hSurface );
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB
        BGRC_P_SET_FIELD_DATA( DEST_SURFACE_ADDR_0_MSB, ADDR, 0 );
#endif
        BGRC_P_SET_FIELD_DATA( DEST_SURFACE_ADDR_0, ADDR, Surface.ulOffset );
        BGRC_P_SET_FIELD_DATA( DEST_SURFACE_STRIDE_0, STRIDE, Surface.ulPitch );
        BGRC_P_SET_FIELD_FORMAT( DEST, FORMAT_TYPE, Surface.eFormat, pState->DstAlphaSurface.hSurface, pState->bDstPaletteBypass );
        BGRC_P_SET_FIELD_CHANNELS( DEST, Surface.eFormat, hSurface );

        BGRC_P_SET_FIELD_COMP_DATA( DEST_SURFACE_FORMAT_DEF_3, CH0_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 0) );
        BGRC_P_SET_FIELD_COMP_DATA( DEST_SURFACE_FORMAT_DEF_3, CH1_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 1) );
        BGRC_P_SET_FIELD_COMP_DATA( DEST_SURFACE_FORMAT_DEF_3, CH2_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 2) );
        BGRC_P_SET_FIELD_COMP_DATA( DEST_SURFACE_FORMAT_DEF_3, CH3_DISABLE, 0, 1, hSurface && BPXL_COMPONENT_SIZE(Surface.eFormat, 3) );

        /* store surface data */
        BKNI_Memcpy( &pState->DstSurface, &Surface, sizeof (BGRC_P_Surface) );
    }

    BDBG_LEAVE(BGRC_Destination_SetSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetAlphaSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Destination_SetAlphaSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->DstAlphaSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        /* validate surface format */
        if( hSurface && (Surface.eFormat != BPXL_eA1) && (Surface.eFormat != BPXL_eW1) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_DATA( DEST_SURFACE_ADDR_1, ADDR, Surface.ulOffset );
        BGRC_P_SET_FIELD_DATA( DEST_SURFACE_STRIDE_1, STRIDE, Surface.ulPitch );

        /* set surface format field */
        if( pState->DstSurface.hSurface )
            BGRC_P_SET_FIELD_FORMAT( DEST, FORMAT_TYPE, pState->DstSurface.eFormat, hSurface, pState->bDstPaletteBypass );

        /* store surface data */
        BKNI_Memcpy( &pState->DstAlphaSurface, &Surface, sizeof (BGRC_P_Surface) );
    }

    BDBG_LEAVE(BGRC_Destination_SetAlphaSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_TogglePaletteBypass(
    BGRC_Handle hGrc,
    bool bEnableBypass )
{
    BGRC_P_State *pState;

    BDBG_ENTER(BGRC_Destination_TogglePaletteBypass);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* set palette bypass field */
    BGRC_P_SET_FIELD_COMP( DEST_SURFACE_FORMAT_DEF_3, PALETTE_BYPASS, DONT_LOOKUP, LOOKUP, bEnableBypass );

    /* set format type due to bypass */
    if( pState->DstSurface.hSurface )
        BGRC_P_SET_FIELD_FORMAT( DEST, FORMAT_TYPE, pState->DstSurface.eFormat, pState->DstAlphaSurface.hSurface, bEnableBypass );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( DEST_SURFACE_FORMAT_DEF_1 ) ||
        BGRC_P_REGISTER_CHANGED( DEST_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

    pState->bDstPaletteBypass = bEnableBypass;

    BDBG_LEAVE(BGRC_Destination_TogglePaletteBypass);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetChromaExpansion(
    BGRC_Handle hGrc,
    BGRC_ChromaExpansion eChromaExpansion )
{
    BDBG_ENTER(BGRC_Destination_SetChromaExpansion);
    BDBG_ASSERT( hGrc );

    /* set chroma expansion field */
    BGRC_P_SET_FIELD_COMP( DEST_SURFACE_FORMAT_DEF_3, CHROMA_FILTER,
        REPLICATE, FILTER, eChromaExpansion == BGRC_ChromaExpansion_eReplicate );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( DEST_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_SetChromaExpansion);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetZeroPad(
    BGRC_Handle hGrc,
    bool bEnableZeroPad )
{
    BDBG_ENTER(BGRC_Destination_SetZeroPad);
    BDBG_ASSERT( hGrc );

    /* set zero pad field */
    BGRC_P_SET_FIELD_COMP( DEST_SURFACE_FORMAT_DEF_3, ZERO_PAD,
        ZERO_PAD, REPLICATE, bEnableZeroPad );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( DEST_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_SetZeroPad);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetColorKey(
    BGRC_Handle hGrc,
    uint32_t ulMin,
    uint32_t ulMax,
    uint32_t ulMask,
    uint32_t ulReplacement,
    uint32_t ulReplacementMask,
    bool bExclusive )
{
    BDBG_ENTER(BGRC_Destination_SetColorKey);
    BDBG_ASSERT( hGrc );

    /* set color key fields */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, DEST_COLOR_KEY_COMPARE, EXCUSIVE, INCLUSIVE, bExclusive );
    BGRC_P_SET_FIELD_FULL( DEST_COLOR_KEY_LOW, ulMin );
    BGRC_P_SET_FIELD_FULL( DEST_COLOR_KEY_HIGH, ulMax );
    BGRC_P_SET_FIELD_FULL( DEST_COLOR_KEY_MASK, ulMask);
    BGRC_P_SET_FIELD_FULL( DEST_COLOR_KEY_REPLACEMENT, ulReplacement );
    BGRC_P_SET_FIELD_FULL( DEST_COLOR_KEY_REPLACEMENT_MASK, ulReplacementMask );

    /* set register load fields */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    if( BGRC_P_REGISTER_CHANGED( DEST_COLOR_KEY_LOW ) ||
        BGRC_P_REGISTER_CHANGED( DEST_COLOR_KEY_HIGH ) ||
        BGRC_P_REGISTER_CHANGED( DEST_COLOR_KEY_MASK ) ||
        BGRC_P_REGISTER_CHANGED( DEST_COLOR_KEY_REPLACEMENT ) ||
        BGRC_P_REGISTER_CHANGED( DEST_COLOR_KEY_REPLACEMENT_MASK ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_COLOR_KEY_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_SetColorKey);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_ToggleColorKey(
    BGRC_Handle hGrc,
    bool bEnable )
{
    BDBG_ENTER(BGRC_Destination_ToggleColorKey);
    BDBG_ASSERT( hGrc );

    /* set color key enable field */
    BGRC_P_SET_FIELD_COMP( BLIT_HEADER, DEST_COLOR_KEY_ENABLE,
        ENABLE, DISABLE, bEnable );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_ToggleColorKey);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetColorMatrix5x4(
    BGRC_Handle hGrc,
    const int32_t ai32_Matrix[],
    uint32_t ulShift )
{
    BDBG_ENTER(BGRC_Destination_SetColorMatrix5x4);
    BDBG_ASSERT( hGrc );
    BDBG_ASSERT( ai32_Matrix );
    BSTD_UNUSED( ulShift );
    BDBG_LEAVE(BGRC_Destination_SetColorMatrix5x4);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Destination_ToggleColorMatrix(
    BGRC_Handle hGrc,
    bool bEnable )
{
    BDBG_ENTER(BGRC_Destination_ToggleColorMatrix);
    BDBG_ASSERT( hGrc );
    BSTD_UNUSED( bEnable );
    BDBG_LEAVE(BGRC_Destination_ToggleColorMatrix);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetColorMatrixRounding(
    BGRC_Handle hGrc,
    BGRC_Rounding eMatrixRounding )
{
    BDBG_ENTER(BGRC_Destination_SetColorMatrixRounding);
    BDBG_ASSERT( hGrc );
    BSTD_UNUSED( eMatrixRounding );
    BDBG_LEAVE(BGRC_Destination_SetColorMatrixRounding);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetKeyMatrixOrder(
    BGRC_Handle hGrc,
    BGRC_KeyMatrixOrder eKeyMatrixOrder )
{
    BDBG_ENTER(BGRC_Destination_SetKeyMatrixOrder);
    BDBG_ASSERT( hGrc );
    BSTD_UNUSED( eKeyMatrixOrder );
    BDBG_LEAVE(BGRC_Destination_SetKeyMatrixOrder);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetColor(
    BGRC_Handle hGrc,
    uint32_t ulColor )
{
    BDBG_ENTER(BGRC_Destination_SetColor);
    BDBG_ASSERT( hGrc );

    /* set single color field */
    BGRC_P_SET_FIELD_FULL( DEST_CONSTANT_COLOR, ulColor );
    BGRC_P_SET_FIELD_DATA( DEST_W_ALPHA, W0_ALPHA, BPXL_GET_COMPONENT( BPXL_eA8_R8_G8_B8, ulColor, 3 ) );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( DEST_CONSTANT_COLOR ) ||
        BGRC_P_REGISTER_CHANGED( DEST_W_ALPHA ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_SetColor);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetRectangle(
    BGRC_Handle hGrc,
    uint32_t ulLeft,
    uint32_t ulTop,
    uint32_t ulWidth,
    uint32_t ulHeight )
{
    BDBG_ENTER(BGRC_Destination_SetRectangle);
    BDBG_ASSERT( hGrc );

    /* validate dimensions */
    if( !BGRC_P_VALIDATE_SURFACE_DIMENSIONS( ulLeft, ulTop, ulWidth, ulHeight ) )
        return BERR_TRACE(BGRC_ERR_DESTINATION_DIMENSIONS_INVALID);

    if( (ulLeft > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulTop > BGRC_P_SURFACE_RECT_SIZE_MAX) ||
        (ulWidth > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulHeight > BGRC_P_SURFACE_RECT_SIZE_MAX) )
        return BERR_TRACE(BGRC_ERR_DESTINATION_DIMENSIONS_INVALID);

    /* store width and height */
    hGrc->CurrentState.DstRect.ulX = ulLeft;
    hGrc->CurrentState.DstRect.ulY = ulTop;
    hGrc->CurrentState.DstRect.ulWidth = ulWidth;
    hGrc->CurrentState.DstRect.ulHeight = ulHeight;

    BDBG_LEAVE(BGRC_Destination_SetRectangle);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetDirection(
    BGRC_Handle hGrc,
    bool bRightToLeft,
    bool bBottomToTop )
{
    BDBG_ENTER(BGRC_Destination_SetDirection);
    BDBG_ASSERT( hGrc );

    hGrc->CurrentState.bDstRightToLeft = bRightToLeft;
    hGrc->CurrentState.bDstBottomToTop = bBottomToTop;

    BDBG_LEAVE(BGRC_Destination_SetDirection);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Destination_ResetState);
    BDBG_ASSERT( hGrc );

    /* copy default state to current state */
    BGRC_P_Destination_CopyState( &hGrc->CurrentState, &hGrc->DefaultState,
        hGrc->aulCurrentRegs, hGrc->aulDefaultRegs );

    BDBG_LEAVE(BGRC_Destination_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Destination_SetDefault(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Destination_SetDefault);
    BDBG_ASSERT( hGrc );

    /* copy current state to default state */
    BGRC_P_Destination_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Destination_SetDefault);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Pattern_Set(
    BGRC_Handle hGrc,
    uint8_t ucVector,
    const uint8_t aucPattern[8],
    uint32_t ulColor0,
    uint32_t ulColor1 )
{
    uint32_t aulPattern[2] = { 0, 0 };

    BDBG_ENTER(BGRC_Pattern_Set);
    BDBG_ASSERT( hGrc );

    if( aucPattern )
    {
        /* setup pattern */
        hGrc->CurrentState.aucPattern[0] = aucPattern[4];
        hGrc->CurrentState.aucPattern[1] = aucPattern[5];
        hGrc->CurrentState.aucPattern[2] = aucPattern[6];
        hGrc->CurrentState.aucPattern[3] = aucPattern[7];
        hGrc->CurrentState.aucPattern[4] = aucPattern[0];
        hGrc->CurrentState.aucPattern[5] = aucPattern[1];
        hGrc->CurrentState.aucPattern[6] = aucPattern[2];
        hGrc->CurrentState.aucPattern[7] = aucPattern[3];
        BGRC_P_ALIGN_PATTERN( aulPattern, hGrc->CurrentState.aucPattern, 0, 0 );
    }

    /* set pattern/rop fields */
    BGRC_P_SET_FIELD_DATA( ROP_OPERATION, VECTOR, ucVector );
    BGRC_P_SET_FIELD_DATA( ROP_PATTERN_TOP, PATTERN, aulPattern[1] );
    BGRC_P_SET_FIELD_DATA( ROP_PATTERN_BOTTOM, PATTERN, aulPattern[0] );
    BGRC_P_SET_FIELD_FULL( ROP_PATTERN_COLOR_0, ulColor0 );
    BGRC_P_SET_FIELD_FULL( ROP_PATTERN_COLOR_1, ulColor1 );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( ROP_OPERATION ) ||
        BGRC_P_REGISTER_CHANGED( ROP_PATTERN_TOP ) ||
        BGRC_P_REGISTER_CHANGED( ROP_PATTERN_BOTTOM ) ||
        BGRC_P_REGISTER_CHANGED( ROP_PATTERN_COLOR_0 ) ||
        BGRC_P_REGISTER_CHANGED( ROP_PATTERN_COLOR_1 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, ROP_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Pattern_Set);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Pattern_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Pattern_ResetState);
    BDBG_ASSERT( hGrc );

    /* copy default state to current state */
    BGRC_P_Pattern_CopyState( &hGrc->CurrentState, &hGrc->DefaultState,
        hGrc->aulCurrentRegs, hGrc->aulDefaultRegs );

    BDBG_LEAVE(BGRC_Pattern_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Pattern_SetDefault(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Pattern_SetDefault);
    BDBG_ASSERT( hGrc );

    /* copy current state to default state */
    BGRC_P_Pattern_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Pattern_SetDefault);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Blend_SetColor(
    BGRC_Handle hGrc,
    uint32_t ulColor )
{
    BDBG_ENTER(BGRC_Blend_SetColor);
    BDBG_ASSERT( hGrc );

    /* set blend color field */
    BGRC_P_SET_FIELD_FULL( BLEND_CONSTANT_COLOR, ulColor );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLEND_CONSTANT_COLOR ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Blend_SetColor);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Blend_SetColorBlend(
    BGRC_Handle hGrc,
    BGRC_Blend_Source eSourceA,
    BGRC_Blend_Source eSourceB,
    bool bSubtractCD,
    BGRC_Blend_Source eSourceC,
    BGRC_Blend_Source eSourceD,
    bool bSubtractE,
    BGRC_Blend_Source eSourceE )
{
    BDBG_ENTER(BGRC_Blend_SetColorBlend);
    BDBG_ASSERT( hGrc );

    /* set color blend fields */
    BGRC_P_SET_FIELD_BLEND( COLOR, eSourceA, eSourceB, eSourceC, eSourceD, eSourceE, bSubtractCD, bSubtractE );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLEND_COLOR_OP ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Blend_SetColorBlend);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Blend_SetAlphaBlend(
    BGRC_Handle hGrc,
    BGRC_Blend_Source eSourceA,
    BGRC_Blend_Source eSourceB,
    bool bSubtractCD,
    BGRC_Blend_Source eSourceC,
    BGRC_Blend_Source eSourceD,
    bool bSubtractE,
    BGRC_Blend_Source eSourceE )
{
    BDBG_ENTER(BGRC_Blend_SetAlphaBlend);
    BDBG_ASSERT( hGrc );

    /* set alpha blend fields */
    BGRC_P_SET_FIELD_BLEND( ALPHA, eSourceA, eSourceB, eSourceC, eSourceD, eSourceE, bSubtractCD, bSubtractE );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLEND_ALPHA_OP ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Blend_SetAlphaBlend);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Blend_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Blend_ResetState);
    BDBG_ASSERT( hGrc );

    /* copy default state to current state */
    BGRC_P_Blend_CopyState( &hGrc->CurrentState, &hGrc->DefaultState,
        hGrc->aulCurrentRegs, hGrc->aulDefaultRegs );

    BDBG_LEAVE(BGRC_Blend_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Blend_SetDefault(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Blend_SetDefault);
    BDBG_ASSERT( hGrc );

    /* copy current state to default state */
    BGRC_P_Blend_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Blend_SetDefault);
    return BERR_SUCCESS;
}

#include "bchp_sun_top_ctrl.h"

/***************************************************************************/
BERR_Code BGRC_Output_SetSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Output_SetSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->OutSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        bool bBypass = true;

        /* validate surface format */
        if( hSurface && BPXL_IS_DEPTH_FORMAT(Surface.eFormat) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        if( hSurface && (BPXL_IS_YCbCr444_10BIT_FORMAT(Surface.eFormat) || BPXL_IS_YCbCr422_10BIT_FORMAT(Surface.eFormat)) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_COMP( OUTPUT_FEEDER_ENABLE, ENABLE, ENABLE, DISABLE, hSurface );
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB
        BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_ADDR_0_MSB, ADDR, 0 );
#endif
        BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_ADDR_0, ADDR, Surface.ulOffset );
        BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_STRIDE_0, STRIDE, Surface.ulPitch );
        /* TODO SURFACE_ADDR_1 */
        BGRC_P_SET_FIELD_FORMAT( OUTPUT, FORMAT_TYPE, Surface.eFormat, pState->OutAlphaSurface.hSurface, bBypass );
        BGRC_P_SET_FIELD_CHANNELS( OUTPUT, Surface.eFormat, hSurface );

        /* store surface data */
        BKNI_Memcpy( &pState->OutSurface, &Surface, sizeof (BGRC_P_Surface) );
    }

    BDBG_LEAVE(BGRC_Output_SetSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetAlphaSurface(
    BGRC_Handle hGrc,
    BSUR_Surface_Handle hSurface )
{
    BGRC_P_State *pState;
    BGRC_P_Surface Surface;
    bool bSurfaceChanged = false;

    BDBG_ENTER(BGRC_Output_SetAlphaSurface);
    BDBG_ASSERT( hGrc );

    pState = &hGrc->CurrentState;

    /* get surface data */
    BGRC_P_GET_SURFACE_DATA( hSurface, Surface, pState->OutAlphaSurface );

    /* check if surface is changing */
    if( hGrc->bUninitialized || bSurfaceChanged )
    {
        bool bBypass = true;

        /* validate surface format */
        if( hSurface && (Surface.eFormat != BPXL_eA1) && (Surface.eFormat != BPXL_eW1) )
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        /* store surface data */
        BKNI_Memcpy( &pState->OutAlphaSurface, &Surface, sizeof (BGRC_P_Surface) );

        /* set register load field */
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        /* set surface fields */
        BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_ADDR_1, ADDR, pState->OutAlphaSurface.ulOffset );
        BGRC_P_SET_FIELD_DATA( OUTPUT_SURFACE_STRIDE_1, STRIDE, pState->OutAlphaSurface.ulPitch );

        /* set surface format field */
        if( pState->OutSurface.hSurface )
            BGRC_P_SET_FIELD_FORMAT( OUTPUT, FORMAT_TYPE, pState->OutSurface.eFormat, hSurface, bBypass );
    }

    BDBG_LEAVE(BGRC_Output_SetAlphaSurface);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetColorMatrix5x4(
    BGRC_Handle hGrc,
    const int32_t ai32_Matrix[],
    uint32_t ulShift )
{
    BDBG_ENTER(BGRC_Output_SetColorMatrix5x4);
    BDBG_ASSERT( hGrc );
    BDBG_ASSERT( ai32_Matrix );
    BSTD_UNUSED( ulShift );
    BDBG_LEAVE(BGRC_Output_SetColorMatrix5x4);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Output_ToggleColorMatrix(
    BGRC_Handle hGrc,
    bool bEnable )
{
    BDBG_ENTER(BGRC_Output_ToggleColorMatrix);
    BDBG_ASSERT( hGrc );
    BSTD_UNUSED( bEnable );
    BDBG_LEAVE(BGRC_Output_ToggleColorMatrix);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Output_SetColorMatrixRounding(
    BGRC_Handle hGrc,
    BGRC_Rounding eMatrixRounding )
{
    BDBG_ENTER(BGRC_Output_SetColorMatrixRounding);
    BDBG_ASSERT( hGrc );
    BSTD_UNUSED( eMatrixRounding );
    BDBG_LEAVE(BGRC_Output_SetColorMatrixRounding);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************/
BERR_Code BGRC_Output_SetRectangle(
    BGRC_Handle hGrc,
    uint32_t ulLeft,
    uint32_t ulTop,
    uint32_t ulWidth,
    uint32_t ulHeight )
{
    uint32_t aulPattern[2] = { 0, 0 };

    BDBG_ENTER(BGRC_Output_SetRectangle);
    BDBG_ASSERT( hGrc );

    /* validate dimensions */
    if( !BGRC_P_VALIDATE_SURFACE_DIMENSIONS( ulLeft, ulTop, ulWidth, ulHeight ) )
        return BERR_TRACE(BGRC_ERR_OUTPUT_DIMENSIONS_INVALID);

    if( (ulLeft > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulTop > BGRC_P_SURFACE_RECT_SIZE_MAX) ||
        (ulWidth > BGRC_P_SURFACE_RECT_SIZE_MAX) || (ulHeight > BGRC_P_SURFACE_RECT_SIZE_MAX) )
        return BERR_TRACE(BGRC_ERR_OUTPUT_DIMENSIONS_INVALID);

    /* store width and height */
    hGrc->CurrentState.OutRect.ulX = ulLeft;
    hGrc->CurrentState.OutRect.ulY = ulTop;
    hGrc->CurrentState.OutRect.ulWidth = ulWidth;
    hGrc->CurrentState.OutRect.ulHeight = ulHeight;

    /* set pattern fields */
    BGRC_P_ALIGN_PATTERN( aulPattern, hGrc->CurrentState.aucPattern, 0, 0 );
    BGRC_P_SET_FIELD_DATA( ROP_PATTERN_TOP, PATTERN, aulPattern[1] );
    BGRC_P_SET_FIELD_DATA( ROP_PATTERN_BOTTOM, PATTERN, aulPattern[0] );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( ROP_PATTERN_TOP ) ||
        BGRC_P_REGISTER_CHANGED( ROP_PATTERN_BOTTOM ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, ROP_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Output_SetRectangle);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetColorKeySelection(
    BGRC_Handle hGrc,
    BGRC_Output_ColorKeySelection eSrcNotKeyedDstNotKeyed,
    BGRC_Output_ColorKeySelection eSrcNotKeyedDstKeyed,
    BGRC_Output_ColorKeySelection eSrcKeyedDstNotKeyed,
    BGRC_Output_ColorKeySelection eSrcKeyedDstKeyed )
{
    BDBG_ENTER(BGRC_Output_SetColorKeySelection);
    BDBG_ASSERT( hGrc );

    /* set output selction fields */
    BGRC_P_SET_FIELD_COLORKEY( eSrcNotKeyedDstNotKeyed, eSrcNotKeyedDstKeyed, eSrcKeyedDstNotKeyed, eSrcKeyedDstKeyed );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLEND_COLOR_KEY_ACTION ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Output_SetColorKeySelection);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetDither(
    BGRC_Handle hGrc,
    bool bEnableDither )
{
    BDBG_ENTER(BGRC_Output_SetDither);
    BDBG_ASSERT( hGrc );

    /* set dither field */
    BGRC_P_SET_FIELD_COMP( OUTPUT_SURFACE_FORMAT_DEF_3, DITHER_ENABLE,
        ENABLE, DISABLE, bEnableDither );

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( OUTPUT_SURFACE_FORMAT_DEF_3 ) )
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_ENABLE, &hGrc->CurrentState, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Output_SetDither);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetDirection(
    BGRC_Handle hGrc,
    bool bRightToLeft,
    bool bBottomToTop )
{
    BDBG_ENTER(BGRC_Output_SetDirection);
    BDBG_ASSERT( hGrc );

    hGrc->CurrentState.bOutRightToLeft = bRightToLeft;
    hGrc->CurrentState.bOutBottomToTop = bBottomToTop;

    BDBG_LEAVE(BGRC_Output_SetDirection);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_ResetState(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Output_ResetState);
    BDBG_ASSERT( hGrc );

    /* copy default state to current state */
    BGRC_P_Output_CopyState( &hGrc->CurrentState, &hGrc->DefaultState,
        hGrc->aulCurrentRegs, hGrc->aulDefaultRegs );

    BDBG_LEAVE(BGRC_Output_ResetState);
    return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BGRC_Output_SetDefault(
    BGRC_Handle hGrc )
{
    BDBG_ENTER(BGRC_Output_SetDefault);
    BDBG_ASSERT( hGrc );

    /* copy current state to default state */
    BGRC_P_Output_CopyState( &hGrc->DefaultState, &hGrc->CurrentState,
        hGrc->aulDefaultRegs, hGrc->aulCurrentRegs );

    BDBG_LEAVE(BGRC_Output_SetDefault);
    return BERR_SUCCESS;
}

/* End of File */
