/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"

#ifdef BCHP_AUD_FMM_IOP_LOOPBACK_0_REG_START
#include "bchp_aud_fmm_iop_loopback_0.h"
#endif

#ifdef BCHP_AUD_FMM_IOP_DUMMYSINK_0_REG_START
#include "bchp_aud_fmm_iop_dummysink_0.h"
#endif

#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_0 || defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0
    #define BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) Mclk_gen##idx
#else
    #define BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) NCO_##idx
#endif
#endif

#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_0 || defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0
    #define BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) Mclk_gen##idx
#else
    #define BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(idx) NCO_##idx
#endif
#endif

BDBG_MODULE(bape_iop_priv);
BDBG_FILE_MODULE(bape_fci);

typedef struct BAPE_LoopbackGroup
{
    bool allocated;
    bool started;
    unsigned numChannelPairs;
    BAPE_Handle deviceHandle;
    uint32_t loopbackIds[BAPE_ChannelPair_eMax];    
    BAPE_LoopbackGroupSettings settings;
} BAPE_LoopbackGroup;

typedef struct BAPE_DummysinkGroup
{
    bool allocated;
    bool started;
    unsigned numChannelPairs;
    BAPE_Handle deviceHandle;
    uint32_t dummysinkIds[BAPE_ChannelPair_eMax];   
    BAPE_DummysinkGroupSettings settings;
} BAPE_DummysinkGroup;

#ifdef BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE
/* Typical STB */
    #define GET_LOOPBACK_STREAM_ID(idx) ((idx)+BAPE_CHIP_MAX_OUTPUT_PORTS)  /* Loopback output stream ID is always first after the O/P streams */
    #define GET_DUMMYSINK_STREAM_ID(idx) ((idx)+BAPE_CHIP_MAX_OUTPUT_PORTS+BAPE_CHIP_MAX_LOOPBACKS) /* Dummysink follows loopbacks */
    #define GET_LOOPBACK_CAPTURE_ID(idx) ((idx))        /* Loopback capture is first on all chips */
#elif defined BCHP_AUD_FMM_IOP_LOOPBACK_0_REG_START
    /* 7429-style chips don't use streams */
#else
    #error Unknown loopback register layout
#endif

static BERR_Code BAPE_LoopbackGroup_P_OpenHw( BAPE_LoopbackGroupHandle hLoopbackGroup );
static void BAPE_Loopback_P_SetGroup(BAPE_Handle deviceHandle, uint32_t loopback, uint32_t group, uint32_t numChannelPairs);

static BERR_Code BAPE_DummysinkGroup_P_OpenHw( BAPE_DummysinkGroupHandle hDummySinkGroup );
static void BAPE_Dummysink_P_SetGroup(BAPE_Handle deviceHandle, uint32_t dummysink, uint32_t group, uint32_t numChannelPairs);

#ifdef BCHP_AUD_FMM_MS_CTRL_REG_START
static BERR_Code BAPE_P_InitMsHw(BAPE_Handle handle)
{
    unsigned i;
    uint32_t regVal, regAddr, endAddr;
    BREG_Handle regHandle;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    /* Initialize the two address blocks to zero */
    regAddr = BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;
    endAddr = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + (BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END*(BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_ELEMENT_SIZE/8));
    while ( regAddr <= endAddr )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }
    regAddr = BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE;
    for ( i = 0; i <= BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }

    /* Download the firmware */
    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to  3 */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 1 */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 1);

    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 2 
             to enable internal access to instruction memory */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Instruction_memory);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Copy the microcode to the F/W instruction memory */
    regAddr = BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE;
    for ( i=BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_START; i <= BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, g_BAPE_MS_Firmware[i]);
        regAddr += 4;
    }

    /* Now Reset Pico */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 0 */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 0);

    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 0 
             to enable internal access to block external access */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, No_external_access);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

#define BAPE_CBIT_BUFFER_SIZE 12
    /* Initializing cBits */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE,  0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 4, 0x200);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BAPE_CBIT_BUFFER_SIZE * 4 + 4, 0x200);        

    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BAPE_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BAPE_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        

    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0,   0x50);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1,   0x68);

    /*Initializing ramp amount*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0,  0x0);           
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_1,  0x0);           

    /*Initializing Stream CTRL*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 0x112);                   
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 0x112);       

    /*Initializing burst*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0, 0x0);       
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_BURST_1, 0x0);       

    /* Initializing SPDIF cfg */
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0, 0xb);

    return BERR_SUCCESS;
}
#endif

static BERR_Code BAPE_P_InitPllSw(BAPE_Handle handle)
{
#if BAPE_CHIP_MAX_PLLS
    int i;
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_PllSettings pllSettings;

    BKNI_Memset(&pllSettings, 0, sizeof pllSettings);
    pllSettings.mode = BAPE_PllMode_eAuto;
    pllSettings.vcxo = 0;
    pllSettings.nDivInt = 0;
    pllSettings.mDivCh0 = 0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_PLLS; i++ )
    {
        handle->audioPlls[i].settings = pllSettings;
    }

    return errCode;
#else
    BSTD_UNUSED(handle);
    return BERR_SUCCESS;
#endif
}


static BERR_Code BAPE_P_InitPllHw(BAPE_Handle handle)
{
#if BAPE_CHIP_MAX_PLLS
    int i;
    BERR_Code errCode = BERR_SUCCESS;


    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_PLLS; i++ )
    {
        BAPE_PllSettings pllSettings;

        pllSettings =  handle->audioPlls[i].settings;

        errCode = BAPE_Pll_SetSettings(handle, i, &pllSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return errCode;
#else
    BSTD_UNUSED(handle);
    return BERR_SUCCESS;
#endif
}


BERR_Code BAPE_P_InitIopSw(BAPE_Handle    handle)
{
    BERR_Code errCode;
    unsigned i;

    /* Initialize PLLs */
    BDBG_MSG(("Initializing PLLs"));
    errCode = BAPE_P_InitPllSw(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Allocate Loopback Groups */
    /* Setup Group Handles */
    BDBG_MSG(("Allocating %u Loopback Groups", BAPE_CHIP_MAX_LOOPBACK_GROUPS));
    handle->loopbackGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_LOOPBACK_GROUPS*sizeof(BAPE_LoopbackGroup));
    if ( NULL == handle->loopbackGroups[0] )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->loopbackGroups[0], 0, BAPE_CHIP_MAX_LOOPBACK_GROUPS*sizeof(BAPE_LoopbackGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_LOOPBACK_GROUPS; i++ )
    {
        handle->loopbackGroups[i] = handle->loopbackGroups[0] + i;
    }

    BDBG_MSG(("Allocating %u Dummysink Groups", BAPE_CHIP_MAX_DUMMYSINK_GROUPS));
    handle->dummysinkGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_DUMMYSINK_GROUPS*sizeof(BAPE_DummysinkGroup));
    if ( NULL == handle->dummysinkGroups[0] )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_dummysink;
    }
    BKNI_Memset(handle->dummysinkGroups[0], 0, BAPE_CHIP_MAX_DUMMYSINK_GROUPS*sizeof(BAPE_DummysinkGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_DUMMYSINK_GROUPS; i++ )
    {
        handle->dummysinkGroups[i] = handle->dummysinkGroups[0] + i;
    }

    return BERR_SUCCESS;

err_alloc_dummysink:
    if ( handle->loopbackGroups[0] )
    {
        BKNI_Free(handle->loopbackGroups[0]);
        BKNI_Memset(handle->loopbackGroups, 0, sizeof(handle->loopbackGroups));
    }
    return errCode;
}


BERR_Code BAPE_P_InitIopHw(BAPE_Handle    handle)
{
    BERR_Code errCode;
    unsigned i;

    /* Initialize PLLs */
    BDBG_MSG(("Initializing PLLs"));
    errCode = BAPE_P_InitPllHw(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

#ifdef BCHP_AUD_FMM_MS_CTRL_REG_START
    /* Initialize Micro Sequencer Registers */
    BDBG_MSG(("Initializing MS registers"));
    errCode = BAPE_P_InitMsHw(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
#endif

#ifdef BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE
    BDBG_MSG(("Setting IOP Capture group IDs to default"));
    {
        uint32_t regAddr, regVal;
        regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE;
        for ( i = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_START; i <= BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_END; i++ )
        {
            regVal = BREG_Read32(handle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP, i);
            BREG_Write32(handle->regHandle, regAddr, regVal);
            regAddr += BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_ELEMENT_SIZE/8;
        }
    }
#endif

#if BAPE_CHIP_MAX_IOP_STREAMS > 0
    /* Initialize each stream to defaults */
    for ( i = 0; i < BAPE_CHIP_MAX_IOP_STREAMS; i++ )
    {
        static const BAPE_IopStreamSettings defaultStreamSettings = {24, BAPE_FCI_ID_INVALID};
        errCode = BAPE_Iop_P_SetStreamSettings(handle, i, &defaultStreamSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
#endif

    /* Initialize each dummysink to default (ungrouped) operation */
    for ( i = 0; i < BAPE_CHIP_MAX_LOOPBACKS; i++ )
    {
        BAPE_Loopback_P_SetGroup(handle, i, i, 1);
    }

    /* Initialize each dummysink to default (ungrouped) operation */
    for ( i = 0; i < BAPE_CHIP_MAX_DUMMYSINKS; i++ )
    {
        BAPE_Dummysink_P_SetGroup(handle, i, i, 1);
    }

    return BERR_SUCCESS;
}

void BAPE_P_UninitIopSw(BAPE_Handle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( handle->loopbackGroups[0] )
    {
        BKNI_Free(handle->loopbackGroups[0]);
        BKNI_Memset(handle->loopbackGroups, 0, sizeof(handle->loopbackGroups));
    }
    if ( handle->dummysinkGroups[0] )
    {
        BKNI_Free(handle->dummysinkGroups[0]);
        BKNI_Memset(handle->dummysinkGroups, 0, sizeof(handle->dummysinkGroups));
    }
}


void BAPE_Iop_P_GetStreamSettings(
    BAPE_Handle handle,
    unsigned streamId,
    BAPE_IopStreamSettings *pSettings   /* [out] */
    )
{
#if defined BCHP_AUD_FMM_IOP_CTRL_REG_START
#ifdef BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE
    uint32_t regAddr, regVal, value;
    /* STB chips */
    BDBG_ASSERT(streamId <= BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_END);  /* END is inclusive */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + 
              ((BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_ELEMENT_SIZE/8) * streamId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    value = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION);
    switch ( value )
    {
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_16_Bit:
        pSettings->resolution = 16;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_17_Bit:
        pSettings->resolution = 17;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_18_Bit:
        pSettings->resolution = 18;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_19_Bit:
        pSettings->resolution = 19;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_20_Bit:
        pSettings->resolution = 20;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_21_Bit:
        pSettings->resolution = 21;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_22_Bit:
        pSettings->resolution = 22;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_23_Bit:
        pSettings->resolution = 23;
        break;
    default:
        BDBG_WRN(("Unsupported stream bit resolution field data %u", value));
        /* Fall through */
    case BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_STREAM_BIT_RESOLUTION_Res_24_Bit:
        pSettings->resolution = 24;
        break;
    }
    value = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_IOP_CTRL_FCI_CFGi, ID);
    pSettings->input = value;
#elif defined BCHP_AUD_FMM_IOP_CTRL_STRM_RES_16TO19
    /* DTV chips */
    uint32_t regAddr, regVal, value, mask, shift;
    /* Handle Resolution */
    if ( streamId <= 7 )
    {
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7;
    }
    else if ( streamId <= 15 )
    {
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_8TO15;
    }
    else
    {
        BDBG_ASSERT(streamId <= 19);
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_16TO19;
    }
    regVal = BREG_Read32(handle->regHandle, regAddr);
    shift = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM1_BIT_RESOLUTION_SHIFT * (streamId % 8);
    mask = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_MASK << shift;
    value = (regVal & mask) >> shift;
    switch ( value )
    {
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_16:
        pSettings->resolution = 16;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_17:
        pSettings->resolution = 17;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_18:
        pSettings->resolution = 18;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_19:
        pSettings->resolution = 19;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_20:
        pSettings->resolution = 20;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_21:
        pSettings->resolution = 21;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_22:
        pSettings->resolution = 22;
        break;
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_23:
        pSettings->resolution = 23;
        break;
    default:
        BDBG_WRN(("Unsupported stream bit resolution field data %u [get] addr %#x id %u shift %u mask %#x", value, regAddr, streamId, shift, mask));
        /* Fall through */
    case BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_24:
        pSettings->resolution = 24;
        break;
    }
    /* Handle ID */
    BDBG_CASSERT((BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP_13_12 - BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10) == 36);    /* If these aren't contiguous bad things will happen */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10 + (4*(streamId/2));
    regVal = BREG_Read32(handle->regHandle, regAddr);
    if ( (streamId % 2) )
    {
        /* Odd */
        value = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM1);
    }
    else
    {
        /* Even */
        value = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM0);
    }
    pSettings->input = value;
#else
#error Unknown IOP Stream Layout
#endif
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(streamId);
    BSTD_UNUSED(pSettings);
    BDBG_ERR(("IOP Streams are not supported on this chipset"));
    BDBG_ASSERT(false);
#endif
}

#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
static void BAPE_Iop_P_SetCaptureGroup(
    BAPE_Handle handle,
    unsigned captureId,
    uint32_t value
    )
{
#ifdef BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE
    uint32_t regAddr, regVal;
    /* These chips support input grouping (DTV does not) */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE + (4*captureId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP, value);
    BREG_Write32(handle->regHandle, regAddr, regVal);
#else
    /* No grouping, just carry on */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(captureId);
    BSTD_UNUSED(value);
#endif
}

static void BAPE_Iop_P_SetCaptureEnable(
    BAPE_Handle handle,
    unsigned captureId,
    uint32_t value
    )
{
    uint32_t regAddr, regVal;
#ifdef BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE + (4*captureId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, ENA));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, ENA, value);
    BREG_Write32(handle->regHandle, regAddr, regVal);   
#elif defined BCHP_AUD_FMM_IOP_CTRL_CAP_CFG
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFG;
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFG, ENA0)<<captureId);
    regVal |= (BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFG, ENA0, value)<<captureId);
    BREG_Write32(handle->regHandle, regAddr, regVal);   
#else
#error Unknown IOP Capture Layout
#endif
}
#endif

BERR_Code BAPE_Iop_P_EnableCapture(
    BAPE_Handle handle,
    unsigned baseCaptureId,
    unsigned numChannelPairs
    )
{
#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    /* Set Grouping First */
    for ( i = 0 ; i < numChannelPairs; i++ )
    {
        BAPE_Iop_P_SetCaptureGroup(handle, baseCaptureId+i, baseCaptureId);
    }
    /* Start */
    for ( i = 0 ; i < numChannelPairs; i++ )
    {
        BAPE_Iop_P_SetCaptureEnable(handle, baseCaptureId+i, 1);
    }
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(baseCaptureId);
    BSTD_UNUSED(numChannelPairs);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void BAPE_Iop_P_DisableCapture(
    BAPE_Handle handle,
    unsigned baseCaptureId,
    unsigned numChannelPairs
                              )
{
#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    /* Stop */
    for ( i = 0 ; i < numChannelPairs; i++ )
    {
        BAPE_Iop_P_SetCaptureEnable(handle, baseCaptureId+i, 0);
    }
    /* Reset Grouping */
    for ( i = 0 ; i < numChannelPairs; i++ )
    {
        BAPE_Iop_P_SetCaptureGroup(handle, baseCaptureId+i, baseCaptureId+i);
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(baseCaptureId);
    BSTD_UNUSED(numChannelPairs);
#endif
}

BERR_Code BAPE_Iop_P_SetStreamSettings(
    BAPE_Handle handle,
    unsigned streamId,
    const BAPE_IopStreamSettings *pSettings
    )
{
#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
#ifdef BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE
    uint32_t regAddr, regVal, value;
    /* STB chips */
    BDBG_ASSERT(streamId <= BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_END);  /* END is inclusive */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + 
              ((BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_ELEMENT_SIZE/8) * streamId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION)|
                BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID));
    switch ( pSettings->resolution )
    {
    case 16:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_16_Bit);
        break;
    case 17:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_17_Bit);
        break;
    case 18:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_18_Bit);
        break;
    case 19:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_19_Bit);
        break;
    case 20:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_20_Bit);
        break;
    case 21:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_21_Bit);
        break;
    case 22:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_22_Bit);
        break;
    case 23:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_23_Bit);
        break;
    case 24:
        value = BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_24_Bit);
        break;
    default:
        BDBG_ERR(("Unsupported stream bit resolution field data %u", pSettings->resolution));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    regVal |= value;
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID, pSettings->input);
    BREG_Write32(handle->regHandle, regAddr, regVal);
#elif defined BCHP_AUD_FMM_IOP_CTRL_STRM_RES_16TO19
    /* DTV chips */
    uint32_t regAddr, regVal, value, mask, shift;
    /* Handle Resolution */
    if ( streamId <= 7 )
    {
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7;
    }
    else if ( streamId <= 15 )
    {
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_8TO15;
    }
    else
    {
        BDBG_ASSERT(streamId <= 19);
        regAddr = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_16TO19;
    }
    regVal = BREG_Read32(handle->regHandle, regAddr);
    shift = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM1_BIT_RESOLUTION_SHIFT * (streamId % 8);
    mask = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_MASK << shift;
    regVal &= ~mask;
    switch ( pSettings->resolution )
    {
    case 16:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_16;
        break;
    case 17:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_17;
        break;
    case 18:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_18;
        break;
    case 19:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_19;
        break;
    case 20:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_20;
        break;
    case 21:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_21;
        break;
    case 22:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_22;
        break;
    case 23:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_23;
        break;
    case 24:
        value = BCHP_AUD_FMM_IOP_CTRL_STRM_RES_0TO7_STREAM0_BIT_RESOLUTION_RES_24;
        break;
    default:
        BDBG_ERR(("Unsupported stream bit resolution field data %u [set]", pSettings->resolution));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    regVal |= value << shift;
    BREG_Write32(handle->regHandle, regAddr, regVal);
    /* Handle ID */
    BDBG_CASSERT((BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP_13_12 - BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10) == 36);    /* If these aren't contiguous bad things will happen */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10 + (4*(streamId/2));
    regVal = BREG_Read32(handle->regHandle, regAddr);
    if ( (streamId % 2) )
    {
        /* Odd */
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM1, pSettings->input);
    }
    else
    {
        /* Even */
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM0);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_MIX_IOP_IDMAP10, STREAM0, pSettings->input);
    }
    BREG_Write32(handle->regHandle, regAddr, regVal);
#else
#error Unknown IOP Stream Layout
#endif
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(streamId);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void BAPE_LoopbackGroup_P_GetDefaultCreateSettings(BAPE_LoopbackGroupCreateSettings *pSettings  /* [out] */ )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->numChannelPairs = 1;
}

BERR_Code BAPE_LoopbackGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_LoopbackGroupCreateSettings *pSettings,
    BAPE_LoopbackGroupHandle *pHandle       /* [out] */
    )
{
    unsigned i, loopback;
    BERR_Code errCode;
    BAPE_LoopbackGroupHandle handle=NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);

    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_LOOPBACK_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->loopbackGroups[i]);
        if ( !deviceHandle->loopbackGroups[i]->allocated )
        {
            handle = deviceHandle->loopbackGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Now search for the correct number of resources */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eLoopback, pSettings->numChannelPairs, &loopback);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_loopback;
    }

    /* Successfully allocated resources.  Initialize Group */
    BKNI_Memset(handle, 0, sizeof(BAPE_LoopbackGroup));
    handle->allocated = true;
    handle->started = false;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        handle->loopbackIds[i] = loopback + i;
        /* Loopbacks don't support grouping on any chips today */
    }
    BAPE_FciIdGroup_Init(&handle->settings.input);
#if BAPE_CHIP_MAX_FS > 0
    handle->settings.fs = 0;
#else
    handle->settings.mclkSource = BAPE_MclkSource_ePll0;
    handle->settings.pllChannel = 0;
    handle->settings.mclkFreqToFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;
#endif
    handle->settings.resolution = 24;

    /* Initialize the hardware for this group */
    errCode = BAPE_LoopbackGroup_P_OpenHw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_after_alloc_loopback;
    }

    /* Make sure FS is set */
    BKNI_EnterCriticalSection();
    (void)BAPE_LoopbackGroup_P_SetSettings_isr(handle, &handle->settings);
    BKNI_LeaveCriticalSection();
    *pHandle = handle;
    return BERR_SUCCESS;

err_after_alloc_loopback:
    BAPE_LoopbackGroup_P_Destroy(handle);
    /* fall through */
err_alloc_loopback:
    return errCode; 
}

void BAPE_LoopbackGroup_P_Destroy(BAPE_LoopbackGroupHandle handle)
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Revert to ungrouped */
        BAPE_Loopback_P_SetGroup(handle->deviceHandle, handle->loopbackIds[i], handle->loopbackIds[i], handle->numChannelPairs);
    }
    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eLoopback, handle->numChannelPairs, handle->loopbackIds[0]);
    BKNI_Memset(handle->loopbackIds, 0xff, sizeof(handle->loopbackIds));
    handle->allocated = false;
}

void BAPE_LoopbackGroup_P_GetSettings_isr(
    BAPE_LoopbackGroupHandle handle,
    BAPE_LoopbackGroupSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_LoopbackGroup_P_SetSettings_isr(
    BAPE_LoopbackGroupHandle handle,
    const BAPE_LoopbackGroupSettings *pSettings
    )
{
    uint32_t regAddr, regVal;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    handle->settings = *pSettings;

#ifdef BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_BASE
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Typical STB */
        regAddr = BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_BASE + (4*handle->loopbackIds[i]);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, TMG_SRC_SEL);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, TMG_SRC_SEL, pSettings->fs);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    }
#elif defined BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG
    /* Typical DTV */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t mask, value;
        switch ( handle->loopbackIds[i] )
        {
        default: 
            BDBG_ASSERT(0); /* Should never be possible */
            /* Fall through */
        case 0: 
            mask = BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL0); 
            value = BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL0, pSettings->fs); 
            break;
        case 1: 
            mask = BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL1); 
            value = BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL1, pSettings->fs); 
            break;
        case 2: 
            mask = BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL2); 
            value = BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL2, pSettings->fs); 
            break;
        case 3: 
            mask = BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL3); 
            value = BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFG, TMG_SRC_SEL3, pSettings->fs); 
            break;
        }
        regVal = (regVal & (~mask))|value;
    }
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
#elif defined BCHP_AUD_FMM_IOP_LOOPBACK_0_REG_START
    /* 7429-style */
    BSTD_UNUSED(regVal);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        switch ( handle->settings.mclkSource ) 
        {
#ifdef BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch1
        case BAPE_MclkSource_ePll0:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch3); break;                
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif
#ifdef BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch1
        case BAPE_MclkSource_ePll1:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch3); break;                
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif
#ifdef BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL2_ch1
        case BAPE_MclkSource_ePll2:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch3); break;                
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif

#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_0
        case BAPE_MclkSource_eNco0:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(0));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen1 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_1
        case BAPE_MclkSource_eNco1:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(1));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen2 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_2
        case BAPE_MclkSource_eNco2:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(2));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen3 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_3
        case BAPE_MclkSource_eNco3:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(3));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen4 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_4
        case BAPE_MclkSource_eNco4:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(4));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen5 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_5
        case BAPE_MclkSource_eNco5:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(5));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen6 || BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_NCO_6
        case BAPE_MclkSource_eNco6:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_LB_MCLKCFG_NCO_CONSTRUCT_PARAM(6));
            break;
#endif
        default:
            BDBG_ERR(("Unsupported clock source %u for Loopback %u", handle->settings.mclkSource, handle->loopbackIds[i]));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        switch ( handle->settings.mclkFreqToFsRatio )
        {
        case 128: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
        case 256: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
        case 384: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
        case 512: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i, MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
        default:
            BDBG_ERR(("Unsupported MCLK Rate of %uFs", handle->settings.mclkFreqToFsRatio));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);
    }
#else
#error Unknown loopback register layout
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_LoopbackGroup_P_Start(BAPE_LoopbackGroupHandle handle)
{
    BERR_Code errCode;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);

#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
    /* Set input FCI */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_IopStreamSettings streamSettings;
        unsigned streamId;
        streamId = GET_LOOPBACK_STREAM_ID(handle->loopbackIds[i]);
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.resolution = handle->settings.resolution;
        streamSettings.input = handle->settings.input.ids[i];
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Enable Capture */
    BDBG_MSG(("Enabling loopback capture"));
    errCode = BAPE_Iop_P_EnableCapture(handle->deviceHandle, GET_LOOPBACK_CAPTURE_ID(handle->loopbackIds[0]), handle->numChannelPairs);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
#else
    BSTD_UNUSED(errCode);
    /* Newer 7429-style chips */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr;
        BAPE_Reg_P_FieldList regFieldList;

        /* Configure Loopback "output" interface */
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        switch ( handle->settings.resolution )
        {
        case 16: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_16_Bit); break;
        case 17: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_17_Bit); break;
        case 18: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_18_Bit); break;
        case 19: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_19_Bit); break;
        case 20: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_20_Bit); break;
        case 21: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_21_Bit); break;
        case 22: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_22_Bit); break;
        case 23: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_23_Bit); break;
        case 24: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, STREAM_BIT_RESOLUTION, Res_24_Bit); break;
        default: 
            BDBG_ERR(("Invalid loopback stream resolution (%u)", handle->settings.resolution)); 
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, FCI_ID, handle->settings.input.ids[i]);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

        /* Configure loopback "capture" interface */
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, handle->loopbackIds[0]);
    }

    /* Now that both input and output are configured, enable the interface.  Enable capture first and then the output (consumer..producer) */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, CAP_ENA, 1);
    }

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, ENA, 1);
    }
#endif

    handle->started = true;

    return BERR_SUCCESS;
}

void BAPE_LoopbackGroup_P_Stop(BAPE_LoopbackGroupHandle handle)
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( !handle->started )
    {
        return;
    }

#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
    /* Disable Capture */
    BDBG_MSG(("Disabling loopback capture"));
    BAPE_Iop_P_DisableCapture(handle->deviceHandle, GET_LOOPBACK_CAPTURE_ID(handle->loopbackIds[0]), handle->numChannelPairs);

    /* Set input FCI to invalid */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_IopStreamSettings streamSettings;
        unsigned streamId;
        BERR_Code errCode;
        streamId = GET_LOOPBACK_STREAM_ID(handle->loopbackIds[i]);
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.input = BAPE_FCI_ID_INVALID;
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }
    }
#else
    /* Disable the interface */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, ENA, 0);
    }

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, CAP_ENA, 0);
    }

    /* Clear any grouping */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t regAddr;
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, GROUP_ID, handle->loopbackIds[i]);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, CHANNEL_GROUPING, 1);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, handle->loopbackIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, handle->loopbackIds[i]);
    }

#endif

    handle->started = false;
}

void BAPE_LoopbackGroup_P_GetCaptureFciIds(
    BAPE_LoopbackGroupHandle handle,
    BAPE_FciIdGroup *pGroup             /* [out] */
    )
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pGroup);

    BAPE_FciIdGroup_Init(pGroup);

#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
    /* Thankfully on all chips so far, loopback is the first set of captures */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        pGroup->ids[i] = BAPE_FCI_BASE_INPUT | GET_LOOPBACK_CAPTURE_ID(handle->loopbackIds[i]);
    }
#else
    {
        uint32_t startId;

        startId = BCHP_GET_FIELD_DATA(
            BAPE_Reg_P_Read(handle->deviceHandle, BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE),
            AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE, 
            START_FCI_ID);

        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            pGroup->ids[i] = handle->loopbackIds[i] + startId;
        }
    }
#endif
}

BERR_Code BAPE_LoopbackGroup_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    i;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened LoopbackGroup, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( i=0 ; i<BAPE_CHIP_MAX_LOOPBACK_GROUPS ; i++ )
    {
        if ( bapeHandle->loopbackGroups[i]->allocated )       /* If this one is open... */
        {
            BAPE_LoopbackGroupHandle hLoopbackGroup = bapeHandle->loopbackGroups[i];

            /* Put the HW into the generic open state. */
            errCode = BAPE_LoopbackGroup_P_OpenHw(hLoopbackGroup );
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
            BKNI_EnterCriticalSection();
            errCode = BAPE_LoopbackGroup_P_SetSettings_isr(hLoopbackGroup, &hLoopbackGroup->settings);
            BKNI_LeaveCriticalSection();
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
                /* Nothing to do for this. */
        }
    }
    return errCode;
}

static BERR_Code BAPE_LoopbackGroup_P_OpenHw( BAPE_LoopbackGroupHandle hLoopbackGroup )
{
    unsigned i;

    BDBG_ASSERT(NULL != hLoopbackGroup);
    BDBG_ASSERT(hLoopbackGroup->allocated);

    for ( i = 0; i < hLoopbackGroup->numChannelPairs; i++ )
    {
        BAPE_Loopback_P_SetGroup(hLoopbackGroup->deviceHandle, hLoopbackGroup->loopbackIds[i], hLoopbackGroup->loopbackIds[0], hLoopbackGroup->numChannelPairs);
    }

    return BERR_SUCCESS;
}

void BAPE_DummysinkGroup_P_GetDefaultCreateSettings(BAPE_DummysinkGroupCreateSettings *pSettings  /* [out] */ )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->numChannelPairs = 1;
}

BERR_Code BAPE_DummysinkGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DummysinkGroupCreateSettings *pSettings,
    BAPE_DummysinkGroupHandle *pHandle      /* [out] */
    )
{
    unsigned i, dummysink;
    BERR_Code errCode;
    BAPE_DummysinkGroupHandle handle=NULL;
    BAPE_DummysinkGroupSettings groupSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);

    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_DUMMYSINK_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->dummysinkGroups[i]);
        if ( !deviceHandle->dummysinkGroups[i]->allocated )
        {
            handle = deviceHandle->dummysinkGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Now search for the correct number of resources */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eDummysink, pSettings->numChannelPairs, &dummysink);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_dummysink;
    }

    /* Successfully allocated resources.  Initialize Group */
    BKNI_Memset(handle, 0, sizeof(BAPE_DummysinkGroup));
    BKNI_Memset(&groupSettings, 0, sizeof(BAPE_DummysinkGroupSettings));
    handle->allocated = true;
    handle->started = false;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    BAPE_FciIdGroup_Init(&groupSettings.input);
    groupSettings.resolution = 24;
#if BAPE_CHIP_MAX_FS > 0
    groupSettings.fs = 0;
#else
    groupSettings.mclkSource = BAPE_MclkSource_ePll0;
    groupSettings.pllChannel = 0;
    groupSettings.mclkFreqToFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;
#endif

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        handle->dummysinkIds[i] = dummysink + i;
    }

    /* Initialize the hardware for this group */
    errCode = BAPE_DummysinkGroup_P_OpenHw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_after_alloc_dummysink;
    }

    /* Make sure FS is set */
    BKNI_EnterCriticalSection();
    (void)BAPE_DummysinkGroup_P_SetSettings_isr(handle, &groupSettings);
    BKNI_LeaveCriticalSection();
    *pHandle = handle;
    return BERR_SUCCESS;

err_after_alloc_dummysink:
    handle->allocated = false;
    /* fall through */
err_alloc_dummysink:
    return errCode; 
}

void BAPE_DummysinkGroup_P_Destroy(BAPE_DummysinkGroupHandle handle)
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);
    /* Release Resources */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Revert to ungrouped */
        BAPE_Dummysink_P_SetGroup(handle->deviceHandle, handle->dummysinkIds[i], handle->dummysinkIds[i], handle->numChannelPairs);
    }
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eDummysink, handle->numChannelPairs, handle->dummysinkIds[0]);
    BKNI_Memset(handle->dummysinkIds, 0xff, sizeof(handle->dummysinkIds));
    handle->allocated = false;
}

void BAPE_DummysinkGroup_P_GetSettings_isrsafe(
    BAPE_DummysinkGroupHandle handle,
    BAPE_DummysinkGroupSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_DummysinkGroup_P_SetSettings_isr(
    BAPE_DummysinkGroupHandle handle,
    const BAPE_DummysinkGroupSettings *pSettings
    )
{
    uint32_t regAddr, regVal;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    handle->settings = *pSettings;

#ifdef BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE
    /* Set FS timing source */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE + (4*handle->dummysinkIds[i]);
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, TMG_SRC_SEL);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, TMG_SRC_SEL, pSettings->fs);
        BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
    }
#else
    /* 7429-style */
    BSTD_UNUSED(regVal);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, handle->dummysinkIds[i]);
        BAPE_Reg_P_InitFieldList_isr(handle->deviceHandle, &regFieldList);
        switch ( handle->settings.mclkSource ) 
        {
#ifdef BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch1
        case BAPE_MclkSource_ePll0:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL0_ch3); break;
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif
#ifdef BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch1
        case BAPE_MclkSource_ePll1:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL1_ch3); break;
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif
#ifdef BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL2_ch1
        case BAPE_MclkSource_ePll2:
            switch ( handle->settings.pllChannel )
            {
            case 0: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch1); break;
            case 1: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch2); break;
            case 2: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, PLL2_ch3); break;
            default: return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_0
        case BAPE_MclkSource_eNco0:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(0));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen1 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_1
        case BAPE_MclkSource_eNco1:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(1));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen2 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_2
        case BAPE_MclkSource_eNco2:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(2));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen3 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_3
        case BAPE_MclkSource_eNco3:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(3));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen4 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_4
        case BAPE_MclkSource_eNco4:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(4));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen5 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_5
        case BAPE_MclkSource_eNco5:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(5));
            break;
#endif
#if defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen6 || BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_NCO_6
        case BAPE_MclkSource_eNco6:
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, PLLCLKSEL, BAPE_DS_MCLKCFG_NCO_CONSTRUCT_PARAM(6));
            break;
#endif
        default:
            BDBG_ERR(("Unsupported clock source %u for Loopback %u", handle->settings.mclkSource, handle->dummysinkIds[i]));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        switch ( handle->settings.mclkFreqToFsRatio )
        {
        case 128: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
        case 256: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
        case 384: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
        case 512: BAPE_Reg_P_AddEnumToFieldList_isr(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i, MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
        default:
            BDBG_ERR(("Unsupported MCLK Rate of %uFs", handle->settings.mclkFreqToFsRatio));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BAPE_Reg_P_ApplyFieldList_isr(&regFieldList, regAddr);
    }    
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_DummysinkGroup_P_Start(BAPE_DummysinkGroupHandle handle)
{
    BERR_Code errCode;
    uint32_t regAddr, regVal;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);

#ifdef BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE
    /* Set input FCI */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_IopStreamSettings streamSettings;
        unsigned streamId;
        streamId = GET_DUMMYSINK_STREAM_ID(handle->dummysinkIds[i]);
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.resolution = handle->settings.resolution;
        streamSettings.input = handle->settings.input.ids[i];
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Enable Dummysink */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BDBG_MSG(("Enabling dummysink %u", handle->dummysinkIds[i]));
        regAddr = BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE + (4*handle->dummysinkIds[i]);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, ENA);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, ENA, 1);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    }
#else
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(regVal);
    /* Newer 7429-style */
    BDBG_MODULE_MSG(bape_fci, ("Linking DUMMY OUTPUT FCI Source"));
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Reg_P_FieldList regFieldList;

        /* Configure dummysink output interface */
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, handle->dummysinkIds[i]);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, GROUP_ID, handle->dummysinkIds[0]);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, CHANNEL_GROUPING, 0xf>>(4-handle->numChannelPairs));
        switch ( handle->settings.resolution )
        {
        case 16: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_16_Bit); break;
        case 17: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_17_Bit); break;
        case 18: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_18_Bit); break;
        case 19: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_19_Bit); break;
        case 20: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_20_Bit); break;
        case 21: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_21_Bit); break;
        case 22: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_22_Bit); break;
        case 23: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_23_Bit); break;
        case 24: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, STREAM_BIT_RESOLUTION, Res_24_Bit); break;
        default: 
            BDBG_ERR(("Invalid loopback stream resolution (%u)", handle->settings.resolution)); 
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        BDBG_MODULE_MSG(bape_fci, ("  fci %x -> DUMMY %d pair[%d]", handle->settings.input.ids[i], handle->dummysinkIds[i], i));
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, FCI_ID, handle->settings.input.ids[i]);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
    }

    /* Enable the interface. */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, handle->dummysinkIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, ENA, 1);
    }
#endif

    handle->started = true;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get enable params for a Dummy Output
***************************************************************************/
void BAPE_DummysinkGroup_P_GetOutputEnableParams(
    BAPE_DummysinkGroupHandle handle,
    bool enable,
    BAPE_OutputPort_P_EnableParams *pEnableParams        /* [out] */
    )
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pEnableParams);

    BKNI_Memset(pEnableParams, 0, sizeof(*pEnableParams));

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
#if defined BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE
        pEnableParams->chPairs[i].address = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, handle->dummysinkIds[i]);
        pEnableParams->chPairs[i].mask = BCHP_MASK(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, ENA);
        pEnableParams->chPairs[i].value = BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, ENA, (int)enable);
#elif defined BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_BASE
        pEnableParams->chPairs[i].address = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, handle->dummysinkIds[i]);
        pEnableParams->chPairs[i].mask = BCHP_MASK(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, ENA);
        pEnableParams->chPairs[i].value = BCHP_FIELD_DATA(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, ENA, (int)enable);
#else
    #warning "UNSUPPORTED CHIP - update this code"
#endif
        BDBG_MSG(("Get dummy grp idx[%d], dummy id %d, enable=%d:", i, handle->dummysinkIds[i], enable));
        BDBG_MSG(("\taddr %08x, mask %08x, value %08x", 
            pEnableParams->chPairs[i].address,
            pEnableParams->chPairs[i].mask,
            pEnableParams->chPairs[i].value));
    }
}

void BAPE_DummysinkGroup_P_Stop(BAPE_DummysinkGroupHandle handle)
{
    BERR_Code errCode;
    uint32_t regAddr, regVal;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        return;
    }

#ifdef BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE
    /* Disable Dummysink */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BDBG_MSG(("Enabling dummysink %u", handle->dummysinkIds[i]));
        regAddr = BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE + (4*handle->dummysinkIds[i]);
        regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, ENA);
        BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    }

    /* Clear input FCI */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_IopStreamSettings streamSettings;
        unsigned streamId;
        streamId = GET_DUMMYSINK_STREAM_ID(handle->dummysinkIds[i]);
        BAPE_Iop_P_GetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        streamSettings.input = BAPE_FCI_ID_INVALID;
        errCode = BAPE_Iop_P_SetStreamSettings(handle->deviceHandle, streamId, &streamSettings);
        BDBG_ASSERT(BERR_SUCCESS == errCode);
    }
#else
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(regVal);
    /* Disable the interface */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, handle->dummysinkIds[i]);
        BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, ENA, 0);
    }

    /* Clear any grouping */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, handle->dummysinkIds[i]);
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, GROUP_ID, handle->dummysinkIds[i]);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, CHANNEL_GROUPING, 1);
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
    }
#endif

    handle->started = false;
}


BERR_Code BAPE_DummysinkGroup_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    i;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened DummysinkGroup, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( i=0 ; i<BAPE_CHIP_MAX_DUMMYSINK_GROUPS ; i++ )
    {
        if ( bapeHandle->dummysinkGroups[i]->allocated )       /* If this one is open... */
        {
            BAPE_DummysinkGroupHandle hDummysinkGroup = bapeHandle->dummysinkGroups[i];

            /* Put the HW into the generic open state. */
            errCode = BAPE_DummysinkGroup_P_OpenHw(hDummysinkGroup );
            if ( errCode ) return BERR_TRACE(errCode);
            
            /* Now apply changes for the settings struct. */
            BKNI_EnterCriticalSection();
            errCode = BAPE_DummysinkGroup_P_SetSettings_isr(hDummysinkGroup, &hDummysinkGroup->settings);
            BKNI_LeaveCriticalSection();
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
                /* Nothing to do for this. */
        }
    }
    return errCode;
}


static BERR_Code BAPE_DummysinkGroup_P_OpenHw( BAPE_DummysinkGroupHandle hDummysinkGroup )
{
    unsigned i;

    BDBG_ASSERT(NULL != hDummysinkGroup);
    BDBG_ASSERT(hDummysinkGroup->allocated);

    for ( i = 0; i < hDummysinkGroup->numChannelPairs; i++ )
    {
        BAPE_Dummysink_P_SetGroup(hDummysinkGroup->deviceHandle, hDummysinkGroup->dummysinkIds[i], hDummysinkGroup->dummysinkIds[0], hDummysinkGroup->numChannelPairs);
    }

    return BERR_SUCCESS;
}

static void BAPE_Loopback_P_SetGroup(BAPE_Handle deviceHandle, uint32_t loopback, uint32_t group, uint32_t numChannelPairs)
{
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(loopback < BAPE_CHIP_MAX_LOOPBACKS);
    BDBG_ASSERT(group <= loopback);
    BDBG_ASSERT((loopback-group) <= BAPE_ChannelPair_eMax);
    BDBG_ASSERT(group < BAPE_CHIP_MAX_LOOPBACKS);

#ifdef BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_BASE
    {
        BAPE_Reg_P_FieldList regFieldList;
        uint32_t regAddr;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, loopback);
        BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, GROUP_ID, group);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i, CHANNEL_GROUPING, 0xf>>(4-numChannelPairs));
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, loopback);
        BAPE_Reg_P_UpdateField(deviceHandle, regAddr, AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, group);
    }
#else
    BSTD_UNUSED(numChannelPairs);
#endif
}

static void BAPE_Dummysink_P_SetGroup(BAPE_Handle deviceHandle, uint32_t dummysink, uint32_t group, uint32_t numChannelPairs)
{
    uint32_t regAddr;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(dummysink < BAPE_CHIP_MAX_DUMMYSINKS);
    BDBG_ASSERT(group >= dummysink);
    BDBG_ASSERT(group < BAPE_CHIP_MAX_DUMMYSINKS);

#ifdef BCHP_AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi_ARRAY_BASE
    BSTD_UNUSED(numChannelPairs);
    regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, dummysink);
    BAPE_Reg_P_UpdateField(deviceHandle, regAddr, AUD_FMM_IOP_CTRL_DUMMYSINK_CFGi, GROUP, group);
#else
    {
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, dummysink);
        BAPE_Reg_P_InitFieldList(deviceHandle, &regFieldList);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, GROUP_ID, group);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i, CHANNEL_GROUPING, 0xf>>(4-numChannelPairs));
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
    }
#endif
}
