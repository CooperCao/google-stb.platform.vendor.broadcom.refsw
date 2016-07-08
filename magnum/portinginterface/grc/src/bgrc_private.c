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
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"

#include "bchp_common.h"
#include "bchp_m2mc.h"
#ifdef BCHP_M2MC1_REG_START
#include "bchp_m2mc1.h"
#endif
#ifdef BCHP_M2MC_1_REG_START
#include "bchp_m2mc_1.h"
#endif
#include "bgrc.h"
#include "bgrc_private.h"

BDBG_MODULE(BGRC);

/***************************************************************************/
static uint32_t *BGRC_P_List_AllocPacket( BGRC_Handle hGrc, uint32_t ulPacketHeader, uint32_t *pulPacketOffset, uint32_t *pulPacketSize, BGRC_P_Block **ppBlock );
static BERR_Code BGRC_P_List_PreparePacket( BGRC_Handle hGrc, bool bEnableInterrupt );
static void BGRC_P_List_WritePacket( BGRC_Handle hGrc, uint32_t *pulPacket, uint32_t ulPacketHeader );
static void BGRC_P_List_RenderPacket( BGRC_Handle hGrc, uint32_t ulPacketOffset );
static BGRC_P_Block *BGRC_P_List_CreateBlock( BGRC_Handle hGrc );
static BGRC_P_Block *BGRC_P_List_DestroyBlock( BGRC_Handle hGrc, BGRC_P_Block *pBlock );
static BGRC_P_Operation *BGRC_P_Operation_Alloc( BGRC_Handle hGrc, BGRC_Callback pCallback, void *pData, BGRC_P_Block *pBlock, uint32_t ulPacketOffset, bool bSetEvent );
static void BGRC_P_Operation_AddToList( BGRC_Handle hGrc, BGRC_P_Operation *pOp );
static void BGRC_P_Operation_RemoveFromList( BGRC_Handle hGrc, BGRC_P_Operation *pOp );

/***************************************************************************/
/* FIR FILTER COEFFICENT TABLES                                            */
/***************************************************************************/
extern const uint32_t BGRC_P_FIRCOEFF_POINTSAMPLE[];
extern const uint32_t BGRC_P_FIRCOEFF_BILINEAR[];
extern const uint32_t BGRC_P_FIRCOEFF_BLURRY_3to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_1to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_2to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_3to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_4to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_5to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_6to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_7to1[];
extern const uint32_t BGRC_P_FIRCOEFF_ANISOTROPIC_8to1[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to9[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to8[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to7[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to6[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to5[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to4[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to3[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to2[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_8to1[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARP_1toN[];
extern const uint32_t BGRC_P_FIRCOEFF_SHARPER_1toN[];
extern const uint32_t BGRC_P_FIRCOEFF_ANTIFLUTTER[];
extern const uint32_t BGRC_P_FIRCOEFF_ANTIFLUTTERSCALE[];
extern const uint32_t BGRC_P_FIRCOEFF_ANTIFLUTTERBLURRY[];
extern const uint32_t BGRC_P_FIRCOEFF_ANTIFLUTTERSHARP[];

/***************************************************************************/
/* SOURCE PAD ARRAYS FOR SPLIT FILTERED BLITS                              */
/***************************************************************************/
static const uint32_t BGRC_P_FILTER_SRCPADLEFT[] =
{
    0,                            /* BGRC_FilterCoeffs_ePointSample */
    0,                            /* BGRC_FilterCoeffs_eBilinear */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eAnisotropic */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eSharp */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eSharper */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eBlurry */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eAntiFlutter */
    BGRC_P_FIR_TAP_COUNT / 2 - 1, /* BGRC_FilterCoeffs_eAntiFlutterBlurry */
    BGRC_P_FIR_TAP_COUNT / 2 - 1  /* BGRC_FilterCoeffs_eAntiFlutterSharp */
};

static const uint32_t BGRC_P_FILTER_SRCPADRIGHT[] =
{
    0,                            /* BGRC_FilterCoeffs_ePointSample */
    2,                            /* BGRC_FilterCoeffs_eBilinear */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eAnisotropic */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eSharp */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eSharper */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eBlurry */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eAntiFlutter */
    BGRC_P_FIR_TAP_COUNT / 2,     /* BGRC_FilterCoeffs_eAntiFlutterBlurry */
    BGRC_P_FIR_TAP_COUNT / 2      /* BGRC_FilterCoeffs_eAntiFlutterSharp */
};

/***************************************************************************/
/* TABLE OF FIR FILTER TABLE RETRIEVAL FUNCTIONS                           */
/***************************************************************************/
typedef const uint32_t *(*GetFilterCoefficients)( uint32_t ulScalerStep );

static const uint32_t *BGRC_P_GetFilterCoefficients_PointSample( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_Bilinear( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_Anisotropic( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_Sharp( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_Sharper( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_Blurry( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutter( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutterBlurry( uint32_t ulScalerStep );
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutterSharp( uint32_t ulScalerStep );

/***************************************************************************/
static const GetFilterCoefficients BGRC_P_GET_FILTER_COEFFICIENTS[] =
{
    BGRC_P_GetFilterCoefficients_PointSample,
    BGRC_P_GetFilterCoefficients_Bilinear,
    BGRC_P_GetFilterCoefficients_Anisotropic,
    BGRC_P_GetFilterCoefficients_Sharp,
    BGRC_P_GetFilterCoefficients_Sharper,
    BGRC_P_GetFilterCoefficients_Blurry,
    BGRC_P_GetFilterCoefficients_AntiFlutter,
    BGRC_P_GetFilterCoefficients_AntiFlutterBlurry,
    BGRC_P_GetFilterCoefficients_AntiFlutterSharp
};

/***************************************************************************/
/* FUNCTIONS FOR RETRIEVING FIR FILTER COEFFICIENT TABLES                  */
/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_PointSample(
    uint32_t ulScalerStep )
{
    BSTD_UNUSED( ulScalerStep );

    return (uint32_t *) BGRC_P_FIRCOEFF_POINTSAMPLE;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_Bilinear(
    uint32_t ulScalerStep )
{
    BSTD_UNUSED( ulScalerStep );

    return (uint32_t *) BGRC_P_FIRCOEFF_BILINEAR;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_Blurry(
    uint32_t ulScalerStep )
{
    BSTD_UNUSED( ulScalerStep );

    return (uint32_t *) BGRC_P_FIRCOEFF_BLURRY_3to1;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_Anisotropic(
    uint32_t ulScalerStep )
{
    if( ulScalerStep < (2 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_1to1;
    else if( ulScalerStep < (3 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_2to1;
    else if( ulScalerStep < (4 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_3to1;
    else if( ulScalerStep < (5 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_4to1;
    else if( ulScalerStep < (6 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_5to1;
    else if( ulScalerStep < (7 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_6to1;
    else if( ulScalerStep < (8 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_7to1;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_ANISOTROPIC_8to1;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_Sharp(
    uint32_t ulScalerStep )
{
    if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 9) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_1toN;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 8) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to9;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 7) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to8;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 6) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to7;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 5) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to6;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 4) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to5;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 3) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to4;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 2) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to3;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 1) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to2;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to1;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_Sharper(
    uint32_t ulScalerStep )
{
    if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 9) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARPER_1toN;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 8) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to9;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 7) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to8;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 6) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to7;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 5) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to6;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 4) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to5;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 3) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to4;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 2) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to3;
    else if( ulScalerStep < ((8 << BGRC_P_SCALER_STEP_FRAC_BITS) / 1) )
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to2;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_SHARP_8to1;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutter(
    uint32_t ulScalerStep )
{
    if( ulScalerStep == (1 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTER;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTERSCALE;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutterBlurry(
    uint32_t ulScalerStep )
{
    if( ulScalerStep == (1 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTERBLURRY;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTERSCALE;
}

/***************************************************************************/
static const uint32_t *BGRC_P_GetFilterCoefficients_AntiFlutterSharp(
    uint32_t ulScalerStep )
{
    if( ulScalerStep == (1 << BGRC_P_SCALER_STEP_FRAC_BITS) )
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTERSHARP;
    else
        return (uint32_t *) BGRC_P_FIRCOEFF_ANTIFLUTTERSHARP;
}

/***************************************************************************/
static const uint8_t BGRC_P_BLEND_OP[] =
{
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_ZERO),           /* eZero */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_HALF),           /* eHalf */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_ZERO),           /* eOne */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_SOURCE_COLOR),   /* eSourceColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_SOURCE_COLOR),   /* eInverseSourceColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_SOURCE_ALPHA),   /* eSourceAlpha */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_SOURCE_ALPHA),   /* eInverseSourceAlpha */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_DEST_COLOR),     /* eDestinationColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_DEST_COLOR),     /* eInverseDestinationColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_DEST_ALPHA),     /* eDestinationAlpha */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_DEST_ALPHA),     /* eInverseDestinationAlpha */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_CONSTANT_COLOR), /* eConstantColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_CONSTANT_COLOR), /* eInverseConstantColor */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_CONSTANT_ALPHA), /* eConstantAlpha */
    BGRC_M2MC(BLEND_COLOR_OP_OP_A_CONSTANT_ALPHA)  /* eInverseConstantAlpha */
};

/***************************************************************************/
static const uint8_t BGRC_P_BLEND_OP_INV[] =
{
    0,                                            /* eZero */
    0,                                            /* eHalf */
    1,                                            /* eOne */
    0,                                            /* eSourceColor */
    1,                                            /* eInverseSourceColor */
    0,                                            /* eSourceAlpha */
    1,                                            /* eInverseSourceAlpha */
    0,                                            /* eDestinationColor */
    1,                                            /* eInverseDestinationColor */
    0,                                            /* eDestinationAlpha */
    1,                                            /* eInverseDestinationAlpha */
    0,                                            /* eConstantColor */
    1,                                            /* eInverseConstantColor */
    0,                                            /* eConstantAlpha */
    1                                             /* eInverseConstantAlpha */
};

/***************************************************************************/
uint32_t BGRC_P_GetBlendOp(
    BGRC_Blend_Source eSource )
{
    return BGRC_P_BLEND_OP[eSource];
}

/***************************************************************************/
uint32_t BGRC_P_GetBlendOpInv(
    BGRC_Blend_Source eSource )
{
    return BGRC_P_BLEND_OP_INV[eSource];
}

/***************************************************************************/
/* BLIT FUNCTIONS                                                          */
/***************************************************************************/
BERR_Code BGRC_P_Blit(
    BGRC_Handle hGrc,
    BGRC_Callback pUserCallback,
    void *pUserData,
    bool bSetEvent )
{
    BGRC_P_Operation *pOp;
    BGRC_P_Block *pBlock = NULL;
    uint32_t ulPacketHeader;
    uint32_t ulPacketSize = 0;
    uint32_t ulPacketOffset = 0;
    uint32_t *pulPacket;
    BERR_Code err;

    BKNI_ResetEvent( hGrc->hPeriodicEvent );

    /* prepare packet */
    err = BGRC_P_List_PreparePacket( hGrc, (pUserCallback || bSetEvent || hGrc->bPeriodicInterrupt) ? true : false );
    if( err != BERR_SUCCESS )
        return err;

    ulPacketHeader = hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(LIST_PACKET_HEADER_1)];

    /* free unused memory that is over the allocation limit */
    BGRC_P_Operation_CleanupList( hGrc );
    BGRC_P_List_CleanupPacketMemory( hGrc );

    /* allocate packet */
    pulPacket = BGRC_P_List_AllocPacket( hGrc, ulPacketHeader, &ulPacketOffset, &ulPacketSize, &pBlock );
    if( pulPacket == NULL )
        return BERR_OUT_OF_DEVICE_MEMORY;

    /* allocate operation */
    pOp = BGRC_P_Operation_Alloc( hGrc, pUserCallback, pUserData, pBlock, ulPacketOffset, bSetEvent );
    if( pOp == NULL )
    {
        hGrc->pCurrListBlock->ulRefCount--;
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    /* set periodic event */
    if( hGrc->bPeriodicInterrupt )
    {
        pOp->bSetPeriodicEvent = true;
        hGrc->bPeriodicInterrupt = false;
    }

    /* add operation to list */
BKNI_EnterCriticalSection();
    BGRC_P_Operation_AddToList( hGrc, pOp );

    /* write packet */
    BGRC_P_List_WritePacket( hGrc, pulPacket, ulPacketHeader );

#if 1
    /* flush memory writes for register data */
    BMEM_FlushCache( hGrc->hMemory, pulPacket, ulPacketSize );
#endif

    /* initiate packet */
    BGRC_P_List_RenderPacket( hGrc, ulPacketOffset );
BKNI_LeaveCriticalSection();

    /* disable loading of all register groups */
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SRC_COLOR_KEY_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, DST_COLOR_KEY_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SRC_COLOR_MATRIX_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, DST_COLOR_MATRIX_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, OUTPUT_COLOR_MATRIX_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SCALE_COEF_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, ROP_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, SRC_CLUT_GRP_CNTRL, GRP_DISABLE );
    BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, DST_CLUT_GRP_CNTRL, GRP_DISABLE );

    /* store some previous packet information */
    hGrc->pulPrevPacket = pulPacket;
    hGrc->pPrevUserCallback = pUserCallback;

    return BERR_SUCCESS;
}

/***************************************************************************/
#define BGRC_P_FIXED_SHIFT_RIGHT( data, shift ) \
    (((data) >> (shift)) | (((data) & 0x80000000) ? ~(0xFFFFFFFF >> (shift)) : 0))

/***************************************************************************/
BERR_Code BGRC_P_FilterBlit(
    BGRC_Handle hGrc,
    BGRC_Callback pCallback,
    void *pData,
    bool bSetEvent )
{
    BGRC_P_State *pState = &hGrc->CurrentState;
    BERR_Code err;

    uint32_t ulSrcWidth = pState->SrcSurface.ulWidth;
    uint32_t ulOutWidth = pState->OutSurface.ulWidth;
    uint32_t ulSrcHeight = pState->SrcSurface.ulHeight;
    uint32_t ulOutHeight = pState->OutSurface.ulHeight;
    uint32_t ulHorzScalerStep;
    uint32_t ulVertScalerStep;

    /* set averager count and coefficient values */
    pState->ulHorzAveragerCount = 1;
    pState->ulHorzAveragerCoeff = 1 << BGRC_P_AVERAGER_COEFF_FRAC_BITS;
    pState->ulVertAveragerCount = 1;
    pState->ulVertAveragerCoeff = 1 << BGRC_P_AVERAGER_COEFF_FRAC_BITS;

    if( (ulSrcWidth > ulOutWidth * BGRC_P_SCALE_DOWN_MAX) ||
        (ulSrcHeight > ulOutHeight * BGRC_P_SCALE_DOWN_MAX) )
        return BERR_TRACE(BGRC_ERR_MAX_SCALE_DOWN_LIMIT_EXCEEDED);

    /* set scaler step and initial phase values */
    if( ((ulSrcWidth << BGRC_P_SCALER_STEP_FRAC_BITS) >> BGRC_P_SCALER_STEP_FRAC_BITS) != ulSrcWidth )
        ulHorzScalerStep = ((ulSrcWidth / 2) << BGRC_P_SCALER_STEP_FRAC_BITS) / (ulOutWidth / 2);
    else
        ulHorzScalerStep = (ulSrcWidth << BGRC_P_SCALER_STEP_FRAC_BITS) / ulOutWidth;

    if( ((ulSrcHeight << BGRC_P_SCALER_STEP_FRAC_BITS) >> BGRC_P_SCALER_STEP_FRAC_BITS) != ulSrcHeight )
        ulVertScalerStep = ((ulSrcHeight / 2) << BGRC_P_SCALER_STEP_FRAC_BITS) / (ulOutHeight / 2);
    else
        ulVertScalerStep = (ulSrcHeight << BGRC_P_SCALER_STEP_FRAC_BITS) / ulOutHeight;

    if ( (pState->ulHorzScalerDen == 0) || (pState->ulHorzScalerNum == 0) )
        pState->ulHorzScalerStep = ulHorzScalerStep;
    else
        pState->ulHorzScalerStep = (pState->ulHorzScalerNum << BGRC_P_SCALER_STEP_FRAC_BITS) / pState->ulHorzScalerDen;

    if ( (pState->ulVertScalerDen == 0) || (pState->ulVertScalerNum == 0) )
        pState->ulVertScalerStep = ulVertScalerStep;
    else
        pState->ulVertScalerStep = (pState->ulVertScalerNum << BGRC_P_SCALER_STEP_FRAC_BITS) / pState->ulVertScalerDen;

    pState->ulOverlapStrip = 0;

    /* set initial phases */
    pState->ulHorzInitPhase = BGRC_P_FIXED_SHIFT_RIGHT(pState->ulHorzScalerStep - BGRC_P_SCALER_STEP_FRAC_MASK - 1, 1) +
        (pState->iHorzPhaseAdj << (BGRC_P_SCALER_STEP_FRAC_BITS - pState->ulPhaseShift));
    pState->ulVertInitPhase = BGRC_P_FIXED_SHIFT_RIGHT(pState->ulVertScalerStep - BGRC_P_SCALER_STEP_FRAC_MASK - 1, 1) +
        (pState->iVertPhaseAdj << (BGRC_P_SCALER_STEP_FRAC_BITS - pState->ulPhaseShift));

    /* check if both src and out widths are greater than max striping width */
    if( (ulSrcWidth > BGRC_P_STRIP_WIDTH_MAX) && (ulOutWidth > BGRC_P_STRIP_WIDTH_MAX) )
    {
        bool bSrcYCbCr422 = BPXL_IS_YCbCr422_FORMAT(pState->SrcSurface.eFormat) |
            BPXL_IS_YCbCr420_FORMAT(pState->SrcSurface.eFormat);
        bool bOutYCbCr422 = BPXL_IS_YCbCr422_FORMAT(pState->OutSurface.eFormat);

#ifdef BGRC_P_SW_STRIPE_SCALE_YCBCR42X
        if( (pState->DstSurface.hSurface == NULL) && (!hGrc->bYCbCr420Source) && (!bSrcYCbCr422) &&
#else
        if( (pState->DstSurface.hSurface == NULL) && (!bSrcYCbCr422) &&
#endif
            (ulOutHeight * BGRC_P_SCALE_DOWN_MAX_Y >= ulSrcHeight) )
        {
            /* setup hardware striping */
            uint32_t ulOverlap = ((pState->ulHorzScalerStep + BGRC_P_SCALER_STEP_FRAC_MASK) >> BGRC_P_SCALER_STEP_FRAC_BITS);
            uint32_t ulOverlapClamp = BGRC_P_MIN(ulOverlap + BGRC_P_FIR_OVERLAP_MIN, BGRC_P_FIR_OVERLAP_MIN * 2);
            uint32_t ulStripWidth = BGRC_P_STRIP_WIDTH_MAX - ulOverlapClamp * 2;
            if( ulSrcWidth < ulOutWidth )
            {
                pState->ulHorzScalerStep--;
                if( pState->ulHorzScalerNum && pState->ulHorzScalerDen )
                    pState->ulOutStripWidth = (ulStripWidth * pState->ulHorzScalerDen) / pState->ulHorzScalerNum;
                else
                    pState->ulOutStripWidth = (ulStripWidth * ulOutWidth) / ulSrcWidth;
            }
            else
            {
                pState->ulOutStripWidth = ulStripWidth;
            }

            if( (ulOutWidth % pState->ulOutStripWidth) <= ulOverlapClamp )
                pState->ulOutStripWidth -= ulOverlapClamp;

            if( bOutYCbCr422 && (pState->ulOutStripWidth & 1) )
            {
                pState->ulOutStripWidth--;
                if( (ulOutWidth % pState->ulOutStripWidth) == 0 )
                    pState->ulOutStripWidth -= 2;
            }

#if defined(BCHP_M2MC_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_MASK)
{
            /* make sure last stripe is less pixels than the rest */
            uint32_t scale_factor = ((BGRC_P_SCALER_STEP_FRAC_MASK+1) << 2) / ulHorzScalerStep;
            if( pState->ulOutStripWidth && ((ulOutWidth % pState->ulOutStripWidth) >= pState->ulOutStripWidth - scale_factor) )
                pState->ulOutStripWidth += 2;
}
#endif

            pState->ulSrcStripWidth = (pState->ulOutStripWidth * ulHorzScalerStep) >> BGRC_P_SCALER_STEP_TO_STRIPE_WIDTH_SHIFT;
            pState->ulOverlapStrip = ulOverlapClamp;
        }
        else
        {
            uint32_t ulSrcX = pState->SrcSurface.ulX;
            uint32_t ulStripWidth;
            uint32_t ulStripCount;
            uint32_t ulSrcPadLeft;
            uint32_t ulSrcPadRight;
            uint32_t ii;

            /* calculate fixed shift for large source widths */
            uint32_t ulFixedLeft;
            uint32_t ulFixedRight;
            uint32_t ulFixedShift = 0;
            if ( (pState->ulHorzScalerDen == 0) || (pState->ulHorzScalerNum == 0) )
            {
                uint32_t ulSrcSize = ulSrcWidth;
                while( ((ulSrcSize << (BGRC_P_SCALER_STEP_FRAC_BITS + 1)) >> (BGRC_P_SCALER_STEP_FRAC_BITS + 1)) != ulSrcSize )
                {
                    ulSrcSize /= 2;
                    ulFixedShift++;;
                }
            }

            /* break scale into multiple vertical strip blits */
/*          ulFixedLeft = (pState->ulHorzInitPhase & BCHP_M2MC_HORIZ_SCALER_INITIAL_PHASE_PHASE_MASK) >> ulFixedShift;*/
            ulFixedLeft = pState->ulHorzInitPhase >> ulFixedShift;
            ulStripWidth = (ulSrcWidth > ulOutWidth) ? (BGRC_P_STRIP_WIDTH_MAX - 4) : (((BGRC_P_STRIP_WIDTH_MAX - 7) * ulOutWidth) / ulSrcWidth);
            ulStripCount = (ulOutWidth + ulStripWidth - 1) / ulStripWidth;

            ulSrcPadLeft = 0;
            ulSrcPadRight = BGRC_P_FILTER_SRCPADRIGHT[pState->eHorzCoeffs];
            if( bSrcYCbCr422 && (pState->SrcSurface.ulX & 1) )
            {
                pState->SrcSurface.ulX++;
                ulSrcPadLeft--;
            }

            /* set strip's output width */
            pState->OutSurface.ulWidth = (ulOutWidth + ulStripCount - 1) / ulStripCount;
            pState->DstSurface.ulWidth = pState->OutSurface.ulWidth;
            if( bOutYCbCr422 && (pState->OutSurface.ulWidth & 1) )
            {
                pState->OutSurface.ulWidth--;
                pState->DstSurface.ulWidth--;
            }

            /* blit all but last strip */
            for( ii = 0; ii < ulOutWidth - pState->OutSurface.ulWidth; ii += pState->OutSurface.ulWidth )
            {
                ulFixedRight = ulFixedLeft + ((pState->OutSurface.ulWidth * pState->ulHorzScalerStep) >> ulFixedShift);

                /* set strip's source width */
                pState->SrcSurface.ulWidth = (BGRC_P_FIXED_SHIFT_RIGHT(ulFixedRight, BGRC_P_SCALER_STEP_FRAC_BITS - ulFixedShift) -
                    BGRC_P_FIXED_SHIFT_RIGHT(ulFixedLeft, BGRC_P_SCALER_STEP_FRAC_BITS - ulFixedShift)) + ulSrcPadLeft + ulSrcPadRight;

                if( BGRC_P_Blit( hGrc, NULL, NULL, false ) != BERR_SUCCESS )
                {
                    BGRC_WaitForOperationReady( hGrc, NULL, NULL );
                    err = BGRC_P_Blit( hGrc, NULL, NULL, false );
                    if( err != BERR_SUCCESS )
                        return err;
                }

                ulFixedLeft = ulFixedRight;
                ulSrcPadLeft = BGRC_P_FILTER_SRCPADLEFT[pState->eHorzCoeffs];

                /* set next strip's positions and initial phase */
                pState->SrcSurface.ulX = ulSrcX + BGRC_P_FIXED_SHIFT_RIGHT(ulFixedLeft, BGRC_P_SCALER_STEP_FRAC_BITS - ulFixedShift) - ulSrcPadLeft;
                if( bSrcYCbCr422 && (pState->SrcSurface.ulX & 1) )
                {
                    pState->SrcSurface.ulX++;
                    ulSrcPadLeft--;
                }
                pState->OutSurface.ulX += pState->OutSurface.ulWidth;
                pState->DstSurface.ulX += pState->OutSurface.ulWidth;
                pState->ulHorzInitPhase = ((ulFixedLeft << ulFixedShift) & BGRC_P_SCALER_STEP_FRAC_MASK) + (ulSrcPadLeft << BGRC_P_SCALER_STEP_FRAC_BITS);
            }

            /* set last strip's widths */
            pState->SrcSurface.ulWidth = ulSrcWidth + ulSrcX - pState->SrcSurface.ulX;
            pState->OutSurface.ulWidth = ulOutWidth - ii;
            pState->DstSurface.ulWidth = pState->OutSurface.ulWidth;
        }
    }

    /* blit last or only strip */
    if( BGRC_P_Blit( hGrc, pCallback, pData, bSetEvent ) != BERR_SUCCESS )
    {
        BGRC_WaitForOperationReady( hGrc, NULL, NULL );
        err = BGRC_P_Blit( hGrc, pCallback, pData, bSetEvent );
        if( err != BERR_SUCCESS )
            return err;
    }

    return BERR_SUCCESS;
}

/***************************************************************************/
/* STATE COPY FUNCTIONS                                                    */
/***************************************************************************/
void BGRC_P_Source_CopyState(
    BGRC_P_State *pDstState,
    BGRC_P_State *pSrcState,
    uint32_t aulDstRegs[],
    uint32_t aulSrcRegs[] )
{
    /* store current source registers */
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_FEEDER_ENABLE );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_0_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_0 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_0_BOT_FLD_MSB );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_0_BOT_FLD );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_1_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_1 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_1_BOT_FLD_MSB );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_ADDR_1_BOT_FLD );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_STRIDE_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_FORMAT_DEF_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_FORMAT_DEF_2 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_FORMAT_DEF_3 );
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_1_FORMAT_DEF_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_1_FORMAT_DEF_2 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_SURFACE_1_FORMAT_DEF_3 );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_FEEDER_GRP_CNTRL, SRC_CONSTANT_COLOR );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_KEY_GRP_CNTRL, SRC_COLOR_KEY_LOW );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_KEY_GRP_CNTRL, SRC_COLOR_KEY_HIGH );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_KEY_GRP_CNTRL, SRC_COLOR_KEY_MASK );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_KEY_GRP_CNTRL, SRC_COLOR_KEY_REPLACEMENT );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_KEY_GRP_CNTRL, SRC_COLOR_KEY_REPLACEMENT_MASK );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C00_C01 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C02_C03 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C04 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C10_C11 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C12_C13 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C14 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C20_C21 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C22_C23 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C24 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C30_C31 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C32_C33 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, SRC_COLOR_MATRIX_GRP_CNTRL, SRC_CM_C34 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_TOP_LEFT );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_SIZE );
#if defined(BCHP_M2MC_BLIT_SRC_TOP_LEFT_1)
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_TOP_LEFT_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_SIZE_1 );
#endif
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0)
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 );
#endif
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1)
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 );
#endif

    /* store current source register fields */
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, SRC_COLOR_KEY_ENABLE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, SRC_COLOR_KEY_COMPARE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, SRC_COLOR_MATRIX_ENABLE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, SRC_COLOR_MATRIX_ROUNDING );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, CBAR_SRC_COLOR );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, SCALER_ORDER );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, ROUNDING_MODE );

#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_ADJUST_ENABLE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, ALPHA_ADJUST );
#else
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, OFFSET_ADJUST );
#endif
#if defined(BCHP_M2MC_SCALER_CTRL_EDGE_CONDITION_REPLICATE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, EDGE_CONDITION );
#endif
#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, ALPHA_PRE_MULTIPLY );
#endif
#if defined(BCHP_M2MC_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_ENABLE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, ALPHA_PRE_MULTIPLY_ENABLE );
#endif
#if defined(BCHP_M2MC_SCALER_CTRL_CLUT_SCALE_MODE_ENABLE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, SCALER_CTRL, CLUT_SCALE_MODE );
#endif

    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, HORIZ_SCALER_INITIAL_PHASE, PHASE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, HORIZ_SCALER_STEP, STEP );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, VERT_SCALER_INITIAL_PHASE, PHASE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, VERT_SCALER_STEP, STEP );

#if defined(BCHP_M2MC_HORIZ_SCALER_1_INITIAL_PHASE)
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, VERT_SCALER_1_INITIAL_PHASE, PHASE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, VERT_SCALER_1_STEP, STEP );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, HORIZ_SCALER_1_INITIAL_PHASE, PHASE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, SCALE_PARAM_GRP_CNTRL, HORIZ_SCALER_1_STEP, STEP );
#endif

    /* store current source state information */
    pDstState->SrcSurface             = pSrcState->SrcSurface;
    pDstState->SrcAlphaSurface        = pSrcState->SrcAlphaSurface;
    pDstState->SrcRect                = pSrcState->SrcRect;
    pDstState->eHorzCoeffs            = pSrcState->eHorzCoeffs;
    pDstState->eVertCoeffs            = pSrcState->eVertCoeffs;
    pDstState->pulHorzFirCoeffs       = pSrcState->pulHorzFirCoeffs;
    pDstState->pulVertFirCoeffs       = pSrcState->pulVertFirCoeffs;
    pDstState->bHorzFilter            = pSrcState->bHorzFilter;
    pDstState->bVertFilter            = pSrcState->bVertFilter;
    pDstState->bSrcPaletteBypass      = pSrcState->bSrcPaletteBypass;
    pDstState->iHorzPhaseAdj          = pSrcState->iHorzPhaseAdj;
    pDstState->iVertPhaseAdj          = pSrcState->iVertPhaseAdj;
    pDstState->ulPhaseShift           = pSrcState->ulPhaseShift;
    pDstState->ulHorzScalerNum        = pSrcState->ulHorzScalerNum;
    pDstState->ulHorzScalerDen        = pSrcState->ulHorzScalerDen;
    pDstState->ulVertScalerNum        = pSrcState->ulVertScalerNum;
    pDstState->ulVertScalerDen        = pSrcState->ulVertScalerDen;
    pDstState->ulMacroBlockRangeY     = pSrcState->ulMacroBlockRangeY;
    pDstState->ulMacroBlockRangeC     = pSrcState->ulMacroBlockRangeC;
    pDstState->ulMacroBlockStripWidth = pSrcState->ulMacroBlockStripWidth;
    pDstState->bMacroBlockLinear      = pSrcState->bMacroBlockLinear;
}

/***************************************************************************/
void BGRC_P_Destination_CopyState(
    BGRC_P_State *pDstState,
    BGRC_P_State *pSrcState,
    uint32_t aulDstRegs[],
    uint32_t aulSrcRegs[] )
{
    /* store current destination registers */
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_FEEDER_ENABLE );
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_ADDR_0_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_ADDR_0 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_ADDR_1_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_ADDR_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_STRIDE_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_FORMAT_DEF_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_FORMAT_DEF_2 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_SURFACE_FORMAT_DEF_3 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_FEEDER_GRP_CNTRL, DEST_CONSTANT_COLOR );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_COLOR_KEY_GRP_CNTRL, DEST_COLOR_KEY_LOW );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_COLOR_KEY_GRP_CNTRL, DEST_COLOR_KEY_HIGH );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_COLOR_KEY_GRP_CNTRL, DEST_COLOR_KEY_MASK );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_COLOR_KEY_GRP_CNTRL, DEST_COLOR_KEY_REPLACEMENT );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, DST_COLOR_KEY_GRP_CNTRL, DEST_COLOR_KEY_REPLACEMENT_MASK );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_DEST_TOP_LEFT );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_DEST_SIZE );

    /* store current destination register fields */
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, DEST_COLOR_KEY_ENABLE );
    BGRC_P_COPY_FIELD( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_HEADER, DEST_COLOR_KEY_COMPARE );

    /* store current destination state information */
    pDstState->DstSurface        = pSrcState->DstSurface;
    pDstState->DstAlphaSurface   = pSrcState->DstAlphaSurface;
    pDstState->DstRect           = pSrcState->DstRect;
    pDstState->bDstPaletteBypass = pSrcState->bDstPaletteBypass;
}

/***************************************************************************/
void BGRC_P_Pattern_CopyState(
    BGRC_P_State *pDstState,
    BGRC_P_State *pSrcState,
    uint32_t aulDstRegs[],
    uint32_t aulSrcRegs[] )
{
    /* store current rop registers */
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, ROP_GRP_CNTRL, ROP_OPERATION );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, ROP_GRP_CNTRL, ROP_PATTERN_TOP );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, ROP_GRP_CNTRL, ROP_PATTERN_BOTTOM );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, ROP_GRP_CNTRL, ROP_PATTERN_COLOR_0 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, ROP_GRP_CNTRL, ROP_PATTERN_COLOR_1 );

    /* store current pattern state information */
    BKNI_Memcpy( pDstState->aucPattern, pSrcState->aucPattern, 8 );
}

/***************************************************************************/
void BGRC_P_Blend_CopyState(
    BGRC_P_State *pDstState,
    BGRC_P_State *pSrcState,
    uint32_t aulDstRegs[],
    uint32_t aulSrcRegs[] )
{
    BSTD_UNUSED( pDstState );
    BSTD_UNUSED( pSrcState );

    /* store current blend registers */
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLEND_PARAM_GRP_CNTRL, BLEND_CONSTANT_COLOR );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLEND_PARAM_GRP_CNTRL, BLEND_COLOR_OP );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLEND_PARAM_GRP_CNTRL, BLEND_ALPHA_OP );
}

/***************************************************************************/
void BGRC_P_Output_CopyState(
    BGRC_P_State *pDstState,
    BGRC_P_State *pSrcState,
    uint32_t aulDstRegs[],
    uint32_t aulSrcRegs[] )
{
    /* store current output registers */
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_FEEDER_ENABLE );
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_ADDR_0_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_ADDR_0 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_ADDR_1_MSB );
#endif
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_ADDR_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_STRIDE_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_FORMAT_DEF_1 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_FORMAT_DEF_2 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, OUTPUT_FEEDER_GRP_CNTRL, OUTPUT_SURFACE_FORMAT_DEF_3 );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_OUTPUT_TOP_LEFT );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLIT_GRP_CNTRL, BLIT_OUTPUT_SIZE );
    BGRC_P_COPY_REGISTER( pDstState, aulDstRegs, aulSrcRegs, BLEND_PARAM_GRP_CNTRL, BLEND_COLOR_KEY_ACTION );

    /* store current output state information */
    pDstState->OutSurface      = pSrcState->OutSurface;
    pDstState->OutAlphaSurface = pSrcState->OutAlphaSurface;
    pDstState->OutRect         = pSrcState->OutRect;
}

/***************************************************************************/
/* DEBUG DUMPING FUNCTIONS                                                 */
/***************************************************************************/
void BGRC_P_PrintRegisters(
    BGRC_Handle hGrc )
{
    void *pNext = 0;

    BSTD_UNUSED( hGrc );

    BMEM_ConvertOffsetToAddress( hGrc->hMemory, BGRC_P_READ_REG( LIST_CURR_PKT_ADDR ), &pNext );

    if( pNext )
        BDBG_ERR(( "NEXT_ADDR                      %08x", *((uint32_t *) pNext) ));

    BDBG_ERR(( "LIST_CTRL                      %08x", (unsigned int) BGRC_P_READ_REG( LIST_CTRL ) ));
    BDBG_ERR(( "LIST_STATUS                    %08x", (unsigned int) BGRC_P_READ_REG( LIST_STATUS ) ));
    BDBG_ERR(( "LIST_FIRST_PKT_ADDR            %08x", (unsigned int) BGRC_P_READ_REG( LIST_FIRST_PKT_ADDR ) ));
    BDBG_ERR(( "LIST_CURR_PKT_ADDR             %08x", (unsigned int) BGRC_P_READ_REG( LIST_CURR_PKT_ADDR ) ));
    BDBG_ERR(( "BLIT_STATUS                    %08x", (unsigned int) BGRC_P_READ_REG( BLIT_STATUS ) ));
    BDBG_ERR(( "BLIT_SRC_ADDRESS               %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_ADDRESS ) ));
    BDBG_ERR(( "BLIT_DEST_ADDRESS              %08x", (unsigned int) BGRC_P_READ_REG( BLIT_DEST_ADDRESS ) ));
    BDBG_ERR(( "BLIT_OUTPUT_ADDRESS            %08x", (unsigned int) BGRC_P_READ_REG( BLIT_OUTPUT_ADDRESS ) ));
    BDBG_ERR(( "BLIT_MEM_HI                    %08x", (unsigned int) BGRC_P_READ_REG( BLIT_MEM_HI ) ));
    BDBG_ERR(( "BLIT_MEM_LO                    %08x", (unsigned int) BGRC_P_READ_REG( BLIT_MEM_LO ) ));
    BDBG_ERR(( "SRC_FEEDER_ENABLE              %08x", (unsigned int) BGRC_P_READ_REG( SRC_FEEDER_ENABLE ) ));
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB
    BDBG_ERR(( "SRC_SURFACE_ADDR_0_MSB         %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_0_MSB ) ));
#endif
    BDBG_ERR(( "SRC_SURFACE_ADDR_0             %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_0 ) ));
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD
    BDBG_ERR(( "SRC_SURFACE_ADDR_0_BOT_FLD_MSB %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_0_BOT_FLD_MSB ) ));
    BDBG_ERR(( "SRC_SURFACE_ADDR_0_BOT_FLD     %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_0_BOT_FLD ) ));
#endif
    BDBG_ERR(( "SRC_SURFACE_STRIDE_0           %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_STRIDE_0 ) ));
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB
    BDBG_ERR(( "SRC_SURFACE_ADDR_1_MSB         %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_1_MSB ) ));
#endif
    BDBG_ERR(( "SRC_SURFACE_ADDR_1             %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_1 ) ));
    BDBG_ERR(( "SRC_SURFACE_STRIDE_1           %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_STRIDE_1 ) ));
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD
    BDBG_ERR(( "SRC_SURFACE_ADDR_1_BOT_FLD_MSB %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_1_BOT_FLD_MSB ) ));
    BDBG_ERR(( "SRC_SURFACE_ADDR_1_BOT_FLD     %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_ADDR_1_BOT_FLD ) ));
#endif
    BDBG_ERR(( "SRC_SURFACE_FORMAT_DEF_1       %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_FORMAT_DEF_1 ) ));
    BDBG_ERR(( "SRC_SURFACE_FORMAT_DEF_2       %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_FORMAT_DEF_2 ) ));
    BDBG_ERR(( "SRC_SURFACE_FORMAT_DEF_3       %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_FORMAT_DEF_3 ) ));
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
    BDBG_ERR(( "SRC_SURFACE_1_FORMAT_DEF_1     %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_1_FORMAT_DEF_1 ) ));
    BDBG_ERR(( "SRC_SURFACE_1_FORMAT_DEF_2     %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_1_FORMAT_DEF_2 ) ));
    BDBG_ERR(( "SRC_SURFACE_1_FORMAT_DEF_3     %08x", (unsigned int) BGRC_P_READ_REG( SRC_SURFACE_1_FORMAT_DEF_3 ) ));
#endif
    BDBG_ERR(( "SRC_W_ALPHA                    %08x", (unsigned int) BGRC_P_READ_REG( SRC_W_ALPHA ) ));
    BDBG_ERR(( "SRC_CONSTANT_COLOR             %08x", (unsigned int) BGRC_P_READ_REG( SRC_CONSTANT_COLOR ) ));
    BDBG_ERR(( "DEST_FEEDER_ENABLE             %08x", (unsigned int) BGRC_P_READ_REG( DEST_FEEDER_ENABLE ) ));
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB
    BDBG_ERR(( "DEST_SURFACE_ADDR_0_MSB        %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_ADDR_0_MSB ) ));
#endif
    BDBG_ERR(( "DEST_SURFACE_ADDR_0            %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_ADDR_0 ) ));
    BDBG_ERR(( "DEST_SURFACE_STRIDE_0          %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_STRIDE_0 ) ));
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB
    BDBG_ERR(( "DEST_SURFACE_ADDR_1_MSB        %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_ADDR_1_MSB ) ));
#endif
    BDBG_ERR(( "DEST_SURFACE_ADDR_1            %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_ADDR_1 ) ));
    BDBG_ERR(( "DEST_SURFACE_STRIDE_1          %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_STRIDE_1 ) ));
    BDBG_ERR(( "DEST_SURFACE_FORMAT_DEF_1      %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_FORMAT_DEF_1 ) ));
    BDBG_ERR(( "DEST_SURFACE_FORMAT_DEF_2      %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_FORMAT_DEF_2 ) ));
    BDBG_ERR(( "DEST_SURFACE_FORMAT_DEF_3      %08x", (unsigned int) BGRC_P_READ_REG( DEST_SURFACE_FORMAT_DEF_3 ) ));
    BDBG_ERR(( "DEST_W_ALPHA                   %08x", (unsigned int) BGRC_P_READ_REG( DEST_W_ALPHA ) ));
    BDBG_ERR(( "DEST_CONSTANT_COLOR            %08x", (unsigned int) BGRC_P_READ_REG( DEST_CONSTANT_COLOR ) ));
    BDBG_ERR(( "OUTPUT_FEEDER_ENABLE           %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_FEEDER_ENABLE ) ));
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB
    BDBG_ERR(( "OUTPUT_SURFACE_ADDR_0_MSB      %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_ADDR_0_MSB ) ));
#endif
    BDBG_ERR(( "OUTPUT_SURFACE_ADDR_0          %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_ADDR_0 ) ));
    BDBG_ERR(( "OUTPUT_SURFACE_STRIDE_0        %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_STRIDE_0 ) ));
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB
    BDBG_ERR(( "OUTPUT_SURFACE_ADDR_1_MSB      %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_ADDR_1_MSB ) ));
#endif
    BDBG_ERR(( "OUTPUT_SURFACE_ADDR_1          %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_ADDR_1 ) ));
    BDBG_ERR(( "OUTPUT_SURFACE_STRIDE_1        %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_STRIDE_1 ) ));
    BDBG_ERR(( "OUTPUT_SURFACE_FORMAT_DEF_1    %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_FORMAT_DEF_1 ) ));
    BDBG_ERR(( "OUTPUT_SURFACE_FORMAT_DEF_2    %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_FORMAT_DEF_2 ) ));
    BDBG_ERR(( "OUTPUT_SURFACE_FORMAT_DEF_3    %08x", (unsigned int) BGRC_P_READ_REG( OUTPUT_SURFACE_FORMAT_DEF_3 ) ));
    BDBG_ERR(( "BLIT_HEADER                    %08x", (unsigned int) BGRC_P_READ_REG( BLIT_HEADER ) ));
    BDBG_ERR(( "BLIT_SRC_TOP_LEFT              %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_TOP_LEFT ) ));
    BDBG_ERR(( "BLIT_SRC_SIZE                  %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_SIZE ) ));
#if defined(BCHP_M2MC_BLIT_SRC_TOP_LEFT_1)
    BDBG_ERR(( "BLIT_SRC_TOP_LEFT_1            %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_TOP_LEFT_1 ) ));
    BDBG_ERR(( "BLIT_SRC_SIZE_1                %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_SIZE_1 ) ));
#endif
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0)
#endif
    BDBG_ERR(( "BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 ) ));
#if defined(BCHP_M2MC_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1)
    BDBG_ERR(( "BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 %08x", (unsigned int) BGRC_P_READ_REG( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 ) ));
#endif
    BDBG_ERR(( "BLIT_DEST_TOP_LEFT             %08x", (unsigned int) BGRC_P_READ_REG( BLIT_DEST_TOP_LEFT ) ));
    BDBG_ERR(( "BLIT_DEST_SIZE                 %08x", (unsigned int) BGRC_P_READ_REG( BLIT_DEST_SIZE ) ));
    BDBG_ERR(( "BLIT_OUTPUT_TOP_LEFT           %08x", (unsigned int) BGRC_P_READ_REG( BLIT_OUTPUT_TOP_LEFT ) ));
    BDBG_ERR(( "BLIT_OUTPUT_SIZE               %08x", (unsigned int) BGRC_P_READ_REG( BLIT_OUTPUT_SIZE ) ));
    BDBG_ERR(( "BLIT_INPUT_STRIPE_WIDTH        %08x", (unsigned int) BGRC_P_READ_REG( BLIT_INPUT_STRIPE_WIDTH ) ));
#if defined(BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_1)
    BDBG_ERR(( "BLIT_INPUT_STRIPE_WIDTH_1      %08x", (unsigned int) BGRC_P_READ_REG( BLIT_INPUT_STRIPE_WIDTH_1 ) ));
#endif
    BDBG_ERR(( "BLIT_OUTPUT_STRIPE_WIDTH       %08x", (unsigned int) BGRC_P_READ_REG( BLIT_OUTPUT_STRIPE_WIDTH ) ));
    BDBG_ERR(( "BLIT_STRIPE_OVERLAP            %08x", (unsigned int) BGRC_P_READ_REG( BLIT_STRIPE_OVERLAP ) ));
#if defined(BCHP_M2MC_BLIT_STRIPE_OVERLAP_1)
    BDBG_ERR(( "BLIT_STRIPE_OVERLAP_1          %08x", (unsigned int) BGRC_P_READ_REG( BLIT_STRIPE_OVERLAP_1 ) ));
#endif
    BDBG_ERR(( "BLIT_CTRL                      %08x", (unsigned int) BGRC_P_READ_REG( BLIT_CTRL ) ));
#if defined(BCHP_M2MC_DCEG_CFG)
    BDBG_ERR(( "DCEG_CFG                       %08x", (unsigned int) BGRC_P_READ_REG( DCEG_CFG ) ));
#endif
    BDBG_ERR(( "SCALER_CTRL                    %08x", (unsigned int) BGRC_P_READ_REG( SCALER_CTRL ) ));
    BDBG_ERR(( "HORIZ_AVERAGER_COUNT           %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_AVERAGER_COUNT ) ));
    BDBG_ERR(( "HORIZ_AVERAGER_COEFF           %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_AVERAGER_COEFF ) ));
    BDBG_ERR(( "VERT_AVERAGER_COUNT            %08x", (unsigned int) BGRC_P_READ_REG( VERT_AVERAGER_COUNT ) ));
    BDBG_ERR(( "VERT_AVERAGER_COEFF            %08x", (unsigned int) BGRC_P_READ_REG( VERT_AVERAGER_COEFF ) ));
    BDBG_ERR(( "HORIZ_SCALER_INITIAL_PHASE     %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_SCALER_INITIAL_PHASE ) ));
    BDBG_ERR(( "HORIZ_SCALER_STEP              %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_SCALER_STEP ) ));
#if defined(BCHP_M2MC_HORIZ_SCALER_1_INITIAL_PHASE)
    BDBG_ERR(( "HORIZ_SCALER_1_INITIAL_PHASE   %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_SCALER_1_INITIAL_PHASE ) ));
    BDBG_ERR(( "HORIZ_SCALER_1_STEP            %08x", (unsigned int) BGRC_P_READ_REG( HORIZ_SCALER_1_STEP ) ));
#endif
    BDBG_ERR(( "VERT_SCALER_INITIAL_PHASE      %08x", (unsigned int) BGRC_P_READ_REG( VERT_SCALER_INITIAL_PHASE ) ));
    BDBG_ERR(( "VERT_SCALER_STEP               %08x", (unsigned int) BGRC_P_READ_REG( VERT_SCALER_STEP ) ));
#if defined(BCHP_M2MC_VERT_SCALER_1_INITIAL_PHASE)
    BDBG_ERR(( "VERT_SCALER_1_INITIAL_PHASE    %08x", (unsigned int) BGRC_P_READ_REG( VERT_SCALER_1_INITIAL_PHASE ) ));
    BDBG_ERR(( "VERT_SCALER_1_STEP             %08x", (unsigned int) BGRC_P_READ_REG( VERT_SCALER_1_STEP ) ));
#endif
    BDBG_ERR(( "BLEND_COLOR_OP                 %08x", (unsigned int) BGRC_P_READ_REG( BLEND_COLOR_OP ) ));
    BDBG_ERR(( "BLEND_ALPHA_OP                 %08x", (unsigned int) BGRC_P_READ_REG( BLEND_ALPHA_OP ) ));
    BDBG_ERR(( "BLEND_CONSTANT_COLOR           %08x", (unsigned int) BGRC_P_READ_REG( BLEND_CONSTANT_COLOR ) ));
    BDBG_ERR(( "BLEND_COLOR_KEY_ACTION         %08x", (unsigned int) BGRC_P_READ_REG( BLEND_COLOR_KEY_ACTION ) ));
    BDBG_ERR(( "SRC_CM_C00_C01                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C00_C01 ) ));
    BDBG_ERR(( "SRC_CM_C02_C03                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C02_C03 ) ));
    BDBG_ERR(( "SRC_CM_C04                     %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C04 ) ));
    BDBG_ERR(( "SRC_CM_C10_C11                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C10_C11 ) ));
    BDBG_ERR(( "SRC_CM_C12_C13                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C12_C13 ) ));
    BDBG_ERR(( "SRC_CM_C14                     %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C14 ) ));
    BDBG_ERR(( "SRC_CM_C20_C21                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C20_C21 ) ));
    BDBG_ERR(( "SRC_CM_C22_C23                 %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C22_C23 ) ));
    BDBG_ERR(( "SRC_CM_C24                     %08x", (unsigned int) BGRC_P_READ_REG( SRC_CM_C24 ) ));
    BDBG_ERR(( "" ));
}

/***************************************************************************/
/* LIST PROCESSING FUNCTIONS                                               */
/***************************************************************************/
static const uint8_t BGRC_P_LIST_GROUP_COUNT[] =
{
    BGRC_P_LIST_DST_CLUT_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SRC_CLUT_GRP_CNTRL_COUNT,
    BGRC_P_LIST_OUTPUT_COLOR_MATRIX_GRP_CNTRL_COUNT,
    BGRC_P_LIST_DST_COLOR_MATRIX_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SRC_COLOR_MATRIX_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SCALE_COEF_GRP_CNTRL_COUNT,
    BGRC_P_LIST_DST_COLOR_KEY_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SRC_COLOR_KEY_GRP_CNTRL_COUNT,
    BGRC_P_LIST_ROP_GRP_CNTRL_COUNT,
    BGRC_P_LIST_BLEND_PARAM_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SCALE_PARAM_GRP_CNTRL_COUNT,
    BGRC_P_LIST_BLIT_GRP_CNTRL_COUNT,
    BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT,
    BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT,
    BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT
};

/***************************************************************************/
static bool BGRC_P_IsBlockDone(
    BGRC_Handle hGrc,
    BGRC_P_Block *pBlock )
{
    if( pBlock->ulRefCount == 0 )
    {
        uint32_t ulDeviceOffset = BGRC_P_READ_REG( LIST_CURR_PKT_ADDR ) & BGRC_M2MC(LIST_CURR_PKT_ADDR_CURR_PKT_ADDR_MASK);

        if( (ulDeviceOffset < pBlock->ulOffset) || (ulDeviceOffset >= pBlock->ulOffset + BGRC_P_LIST_BLOCK_SIZE) )
            return true;
    }

    return false;
}

/***************************************************************************/
#define BGRC_P_GET_PACKET_SIZE( ulHeader, ulSize ) \
{ \
    uint32_t ii; \
    ulSize = 0; \
    for( ii = 0; ii < BGRC_P_GROUP_COUNT; ++ii ) \
        ulSize += (((ulHeader) >> ii) & 1) ? BGRC_P_LIST_GROUP_COUNT[ii] * sizeof (uint32_t) : 0; \
    ulSize += (BGRC_P_HEADER_COUNT + BGRC_P_USERDATA_COUNT) * sizeof (uint32_t); \
}

/***************************************************************************/
#define BGRC_P_ALIGN_PACKET_SIZE( ulSize ) \
{ \
    ulSize += (1 << BGRC_P_LIST_BLOCK_ALIGN) - 1; \
    ulSize &= ~((1 << BGRC_P_LIST_BLOCK_ALIGN) - 1); \
}

/***************************************************************************/
bool BGRC_P_List_InitPacketMemory(
    BGRC_Handle hGrc,
    uint32_t ulMemorySize )
{
#if 1
    BGRC_P_Block *pCurrBlock = NULL;
    BGRC_P_Block *pPrevBlock = NULL;
    uint32_t ulBlockCount = (ulMemorySize + BGRC_P_LIST_BLOCK_SIZE - 1) / BGRC_P_LIST_BLOCK_SIZE;
    uint32_t ii;

    for( ii = 0; ii < ulBlockCount; ++ii )
    {
        /* create memory block */
        pCurrBlock = BGRC_P_List_CreateBlock( hGrc );
        if( pCurrBlock == NULL )
            return false;

        /* attach to previous block */
        if( pPrevBlock )
            pPrevBlock->pNextBlock = pCurrBlock;
        else
            hGrc->pCurrListBlock = pCurrBlock;

        pPrevBlock = pCurrBlock;
    }

    /* set last block to point to first block */
    if( pCurrBlock )
        pCurrBlock->pNextBlock = hGrc->pCurrListBlock;

    hGrc->pPrevListBlock = pCurrBlock;
#else
    /* create memory block */
    hGrc->pCurrListBlock = BGRC_P_List_CreateBlock( hGrc );
    if( hGrc->pCurrListBlock == NULL )
        return false;

    /* attach block to itself */
    hGrc->pCurrListBlock->pNextBlock = hGrc->pCurrListBlock;
#endif
    hGrc->ulListBlockPos = 0;

    return true;
}

/***************************************************************************/
void BGRC_P_List_FreePacketMemory(
    BGRC_Handle hGrc )
{
    /* free all list blocks */
    BGRC_P_Block *pBlock = hGrc->pCurrListBlock;
    while( pBlock )
    {
        pBlock = BGRC_P_List_DestroyBlock( hGrc, pBlock );
    }
}

/***************************************************************************/
void BGRC_P_List_CleanupPacketMemory(
    BGRC_Handle hGrc )
{
    BGRC_P_Block *pBlock = hGrc->pCurrListBlock;

    /* free packet memory blocks while total allocation size is greater than max */
    while( pBlock && hGrc->ulPacketMemoryMax &&
        (hGrc->ulPacketMemorySize > hGrc->ulPacketMemoryMax) && BGRC_P_IsBlockDone( hGrc, pBlock ) )
    {
        pBlock = BGRC_P_List_DestroyBlock( hGrc, pBlock );
    }
}

/***************************************************************************/
static uint32_t *BGRC_P_List_AllocPacket(
    BGRC_Handle hGrc,
    uint32_t ulPacketHeader,
    uint32_t *pulPacketOffset,
    uint32_t *pulPacketSize,
    BGRC_P_Block **ppBlock )
{
    uint32_t *pulPacket;
    uint32_t ulPacketSize;

    /* get packet size */
    BGRC_P_GET_PACKET_SIZE( ulPacketHeader, ulPacketSize );
    BGRC_P_ALIGN_PACKET_SIZE( ulPacketSize );

    /* check if current memory block is full */
    if( hGrc->ulListBlockPos + ulPacketSize > BGRC_P_LIST_BLOCK_SIZE )
    {
        /* if next block is busy, check if it can be set to not busy */
        BGRC_P_Block *pCurrBlock = hGrc->pCurrListBlock->pNextBlock;
        if( pCurrBlock->bBusy && BGRC_P_IsBlockDone( hGrc, pCurrBlock ) )
            pCurrBlock->bBusy = false;

        /* check if next block is still busy */
        if( pCurrBlock->bBusy )
        {
            /* return error if memory is pre-allocated */
            if( hGrc->bPreAllocMemory )
                return NULL;

            /* create new memory block */
            pCurrBlock = BGRC_P_List_CreateBlock( hGrc );
            if( pCurrBlock == NULL )
                return NULL;

            /* attach block to list */
            pCurrBlock->pNextBlock = hGrc->pCurrListBlock->pNextBlock;
            hGrc->pCurrListBlock->pNextBlock = pCurrBlock;
        }

        /* clear new block */
        hGrc->pPrevListBlock = hGrc->pCurrListBlock;
        hGrc->pCurrListBlock = pCurrBlock;
        hGrc->ulListBlockPos = 0;
        pCurrBlock->bBusy = true;
    }

    /* get packet address */
#if 1
    pulPacket = (uint32_t *) ((uint8_t *) hGrc->pCurrListBlock->pvCached + hGrc->ulListBlockPos);
#else
    pulPacket = (uint32_t *) ((uint8_t *) hGrc->pCurrListBlock->pvMemory + hGrc->ulListBlockPos);
#endif
    *pulPacketOffset = hGrc->pCurrListBlock->ulOffset + hGrc->ulListBlockPos;
    *pulPacketSize = ulPacketSize;

    hGrc->ulListBlockPos += ulPacketSize;
    hGrc->pCurrListBlock->ulRefCount++;

    hGrc->ulPacketMemorySinceInterrupt += ulPacketSize;

    *ppBlock = hGrc->pCurrListBlock;
    return pulPacket;
}

/***************************************************************************/
static BERR_Code BGRC_P_List_PreparePacket(
    BGRC_Handle hGrc,
    bool bEnableInterrupt )
{
    BGRC_P_State *pState = &hGrc->CurrentState;

    /* set W alpha for alpha channel expansion for ARGB1555 formats */
    if( pState->SrcSurface.hSurface && (!BPXL_IS_WINDOW_FORMAT(pState->SrcSurface.eFormat)) &&
        BGRC_P_COMPARE_VALUE( SRC_SURFACE_FORMAT_DEF_1, FORMAT_TYPE, 1 ) )
    {
        BGRC_P_SET_FIELD_DATA( SRC_W_ALPHA, W0_ALPHA, 0 );
        BGRC_P_SET_FIELD_DATA( SRC_W_ALPHA, W1_ALPHA,
            BGRC_P_COMPARE_FIELD( SRC_SURFACE_FORMAT_DEF_3, ZERO_PAD, REPLICATE ) ? 0xFF : 0x80 );
        if( BGRC_P_REGISTER_CHANGED( SRC_W_ALPHA ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    if( pState->DstSurface.hSurface && (!BPXL_IS_WINDOW_FORMAT(pState->DstSurface.eFormat)) &&
        BGRC_P_COMPARE_VALUE( DEST_SURFACE_FORMAT_DEF_1, FORMAT_TYPE, 1 ) )
    {
        BGRC_P_SET_FIELD_DATA( DEST_W_ALPHA, W0_ALPHA, 0 );
        BGRC_P_SET_FIELD_DATA( DEST_W_ALPHA, W1_ALPHA,
            BGRC_P_COMPARE_FIELD( DEST_SURFACE_FORMAT_DEF_3, ZERO_PAD, REPLICATE ) ? 0xFF : 0x80 );
        if( BGRC_P_REGISTER_CHANGED( DEST_W_ALPHA ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    /* set rectangle fields */
    BGRC_P_SET_SURFACE_DIMENSIONS( SRC, pState->SrcSurface.hSurface,
        pState->SrcSurface.ulX, pState->SrcSurface.ulY, pState->SrcSurface.ulWidth, pState->SrcSurface.ulHeight );
    BGRC_P_SET_SURFACE_DIMENSIONS( DEST, pState->DstSurface.hSurface,
        pState->DstSurface.ulX, pState->DstSurface.ulY, pState->DstSurface.ulWidth, pState->DstSurface.ulHeight );
    BGRC_P_SET_SURFACE_DIMENSIONS( OUTPUT, pState->OutSurface.hSurface,
        pState->OutSurface.ulX, pState->OutSurface.ulY, pState->OutSurface.ulWidth, pState->OutSurface.ulHeight );

#if defined(BCHP_M2MC_BLIT_SRC_TOP_LEFT_1)
    /* set rectangle registers for YCbCr 420 format */
    if( hGrc->bYCbCr420Source )
    {
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_TOP_LEFT_1, LEFT, pState->SrcSurface.ulX / 2 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_TOP_LEFT_1, TOP, pState->SrcSurface.ulY / 2 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE_1, SURFACE_WIDTH, pState->SrcSurface.ulWidth / 2 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE_1, SURFACE_HEIGHT, pState->SrcSurface.ulHeight / 2 );
    }
    else
    {
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_TOP_LEFT_1, LEFT, 0 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_TOP_LEFT_1, TOP, 0 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE_1, SURFACE_WIDTH, 0 );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE_1, SURFACE_HEIGHT, 0 );
    }
#endif

    /* check if there is no source surface */
    if( pState->SrcSurface.hSurface == 0 )
    {
        /* setup for source fill */
        BGRC_P_SET_FIELD_ENUM( SRC_FEEDER_ENABLE, ENABLE, ENABLE );
        BGRC_P_SET_FIELD_CHANNELS( SRC, BPXL_eA8_R8_G8_B8, pState->OutSurface.hSurface );

        /* set source width and height registers to same as output */
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE, SURFACE_WIDTH, BGRC_P_GET_FIELD_DATA(BLIT_OUTPUT_SIZE, SURFACE_WIDTH) );
        BGRC_P_SET_FIELD_DATA( BLIT_SRC_SIZE, SURFACE_HEIGHT, BGRC_P_GET_FIELD_DATA(BLIT_OUTPUT_SIZE, SURFACE_HEIGHT) );

        /* set register load fields */
        if( BGRC_P_REGISTER_CHANGED( SRC_FEEDER_ENABLE ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

        if( BGRC_P_REGISTER_CHANGED( BLIT_SRC_SIZE ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    /* check if there is no destination surface */
    {
        if( pState->DstSurface.hSurface == 0 )
        {
            /* setup for destination fill */
            BGRC_P_SET_FIELD_ENUM( DEST_FEEDER_ENABLE, ENABLE, ENABLE );
            BGRC_P_SET_FIELD_CHANNELS( DEST, BPXL_eA8_R8_G8_B8, pState->OutSurface.hSurface );

            /* set destination width and height registers to same as output */
            BGRC_P_SET_FIELD_DATA( BLIT_DEST_SIZE, SURFACE_WIDTH, BGRC_P_GET_FIELD_DATA(BLIT_OUTPUT_SIZE, SURFACE_WIDTH) );
            BGRC_P_SET_FIELD_DATA( BLIT_DEST_SIZE, SURFACE_HEIGHT, BGRC_P_GET_FIELD_DATA(BLIT_OUTPUT_SIZE, SURFACE_HEIGHT) );

            /* set register load fields */
            if( BGRC_P_REGISTER_CHANGED( DEST_FEEDER_ENABLE ) )
                BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

            if( BGRC_P_REGISTER_CHANGED( BLIT_DEST_SIZE ) )
                BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
        }
    }

    /* set blit direction fields */
    if( pState->bSrcRightToLeft || pState->bSrcBottomToTop ||
        pState->bDstRightToLeft || pState->bDstBottomToTop ||
        pState->bOutRightToLeft || pState->bOutBottomToTop )
    {
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, SRC_H_DIRECTION, RIGHT_TO_LEFT, LEFT_TO_RIGHT, pState->bSrcRightToLeft );
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, SRC_V_DIRECTION, BOTTOM_TO_TOP, TOP_TO_BOTTOM, pState->bSrcBottomToTop );
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, DEST_H_DIRECTION, RIGHT_TO_LEFT, LEFT_TO_RIGHT, pState->bDstRightToLeft );
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, DEST_V_DIRECTION, BOTTOM_TO_TOP, TOP_TO_BOTTOM, pState->bDstBottomToTop );
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, OUTPUT_H_DIRECTION, RIGHT_TO_LEFT, LEFT_TO_RIGHT, pState->bOutRightToLeft );
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, OUTPUT_V_DIRECTION, BOTTOM_TO_TOP, TOP_TO_BOTTOM, pState->bOutBottomToTop );
    }
    else if( pState->SrcSurface.hSurface && BGRC_P_SURFACE_INTERSECT( SRC, OUTPUT ) )
    {
        BGRC_P_SET_FIELD_DIRECTION( SRC, OUTPUT, DEST, H, LEFT, LEFT_TO_RIGHT, RIGHT_TO_LEFT );
        BGRC_P_SET_FIELD_DIRECTION( SRC, OUTPUT, DEST, V, TOP, TOP_TO_BOTTOM, BOTTOM_TO_TOP );
    }
    else if( pState->DstSurface.hSurface && BGRC_P_SURFACE_INTERSECT( DEST, OUTPUT ) )
    {
        BGRC_P_SET_FIELD_DIRECTION( DEST, OUTPUT, SRC, H, LEFT, LEFT_TO_RIGHT, RIGHT_TO_LEFT );
        BGRC_P_SET_FIELD_DIRECTION( DEST, OUTPUT, SRC, V, TOP, TOP_TO_BOTTOM, BOTTOM_TO_TOP );
    }
    else
    {
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, SRC_H_DIRECTION, LEFT_TO_RIGHT );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, SRC_V_DIRECTION, TOP_TO_BOTTOM );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, DEST_H_DIRECTION, LEFT_TO_RIGHT );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, DEST_V_DIRECTION, TOP_TO_BOTTOM );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, OUTPUT_H_DIRECTION, LEFT_TO_RIGHT );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, OUTPUT_V_DIRECTION, TOP_TO_BOTTOM );
    }

    /* set scaler fields */
    if( pState->SrcSurface.hSurface && (hGrc->bNoScaleFilter ||
        (pState->ulHorzScalerStep != (1 << BGRC_P_SCALER_STEP_FRAC_BITS)) ||
        (pState->ulVertScalerStep != (1 << BGRC_P_SCALER_STEP_FRAC_BITS))) )
    {
        /* get fir filter coefficient tables */
        const uint32_t *pulHorzFirCoeffs = BGRC_P_GET_FILTER_COEFFICIENTS[pState->eHorzCoeffs]( pState->ulHorzScalerStep );
        const uint32_t *pulVertFirCoeffs = BGRC_P_GET_FILTER_COEFFICIENTS[pState->eVertCoeffs]( pState->ulVertScalerStep );

        /* check if scaler step has changed */
        if( (pState->pulHorzFirCoeffs != pulHorzFirCoeffs) ||
            (pState->pulVertFirCoeffs != pulVertFirCoeffs) )
        {
#if defined(BCHP_M2MC_SCALER_CTRL_CLUT_SCALE_MODE_ENABLE)
            BGRC_P_SET_FIELD_COMP( SCALER_CTRL, CLUT_SCALE_MODE, ENABLE, DISABLE,
                (pState->eHorzCoeffs == BGRC_FilterCoeffs_ePointSample) &&
                (pState->eVertCoeffs == BGRC_FilterCoeffs_ePointSample) );
#endif

            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_COEF_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
            pState->pulHorzFirCoeffs = pulHorzFirCoeffs;
            pState->pulVertFirCoeffs = pulVertFirCoeffs;
        }

        /* set scaler enable and scale order */
        BGRC_P_SET_FIELD_ENUM( BLIT_HEADER, SRC_SCALER_ENABLE, ENABLE );
        BGRC_P_SET_FIELD_COMP( SCALER_CTRL, SCALER_ORDER, VERT_THEN_HORIZ, HORIZ_THEN_VERT,
            pState->ulHorzScalerStep < (1 << BGRC_P_SCALER_STEP_FRAC_BITS) );

        /* set horizontal scaler */
        BGRC_P_SET_FIELD_ENUM( SCALER_CTRL, HORIZ_SCALER_ENABLE, ENABLE );
        BGRC_P_SET_FIELD_DATA( HORIZ_SCALER_INITIAL_PHASE, PHASE, pState->ulHorzInitPhase / pState->ulHorzAveragerCount )
        BGRC_P_SET_FIELD_DATA( HORIZ_SCALER_STEP, STEP, pState->ulHorzScalerStep / pState->ulHorzAveragerCount );

#if defined(BCHP_M2MC_HORIZ_SCALER_1_INITIAL_PHASE)
        /* set scaler registers for YCbCr 420 format */
        if( hGrc->bYCbCr420Source )
        {
            BGRC_P_SET_FIELD_DATA( HORIZ_SCALER_1_INITIAL_PHASE, PHASE, pState->ulHorzInitPhase / 2 );/*-
                pState->ulHorzScalerStep / 2 );*/
            BGRC_P_SET_FIELD_DATA( HORIZ_SCALER_1_STEP, STEP, pState->ulHorzScalerStep / 2 );
        }
#endif

        /* check if striping */
        if( (pState->SrcSurface.ulWidth > BGRC_P_STRIP_WIDTH_MAX) && (pState->OutSurface.ulWidth > BGRC_P_STRIP_WIDTH_MAX) )
        {
            /* set striping */
            BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, STRIPE_ENABLE, ENABLE );
            BGRC_P_SET_FIELD_DATA( BLIT_INPUT_STRIPE_WIDTH, STRIPE_WIDTH, pState->ulSrcStripWidth );
            BGRC_P_SET_FIELD_DATA( BLIT_OUTPUT_STRIPE_WIDTH, STRIPE_WIDTH, pState->ulOutStripWidth );
            BGRC_P_SET_FIELD_DATA( BLIT_STRIPE_OVERLAP, STRIPE_WIDTH, pState->ulOverlapStrip );

#if defined(BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_1)
            if( hGrc->bYCbCr420Source )
            {
                BGRC_P_SET_FIELD_DATA( BLIT_INPUT_STRIPE_WIDTH_1, STRIPE_WIDTH, pState->ulSrcStripWidth / 2 );
                BGRC_P_SET_FIELD_DATA( BLIT_STRIPE_OVERLAP_1, STRIPE_WIDTH, pState->ulOverlapStrip );
            }
#endif

            /* disable dst feeder when striping enabled */
            BGRC_P_SET_FIELD_ENUM( DEST_FEEDER_ENABLE, ENABLE, DISABLE );

            if( BGRC_P_REGISTER_CHANGED( DEST_FEEDER_ENABLE ) )
                BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );

            /* set register load field */
            if( BGRC_P_REGISTER_CHANGED( BLIT_INPUT_STRIPE_WIDTH ) ||
                BGRC_P_REGISTER_CHANGED( BLIT_OUTPUT_STRIPE_WIDTH ) ||
                BGRC_P_REGISTER_CHANGED( BLIT_STRIPE_OVERLAP ) )
                BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
        }
        else
        {
            /* disable striping */
            BGRC_P_SET_FIELD_ENUM ( BLIT_CTRL, STRIPE_ENABLE, DISABLE );
        }

        /* check if vertical scaling */
        if( pState->bVertFilter || (pState->ulVertScalerStep != (1 << BGRC_P_SCALER_STEP_FRAC_BITS)) || hGrc->bYCbCr420Source )
        {
            /* set vertical scaler */
            BGRC_P_SET_FIELD_ENUM( SCALER_CTRL, VERT_SCALER_ENABLE, ENABLE );
            BGRC_P_SET_FIELD_DATA( VERT_SCALER_INITIAL_PHASE, PHASE, pState->ulVertInitPhase / pState->ulVertAveragerCount );
            BGRC_P_SET_FIELD_DATA( VERT_SCALER_STEP, STEP, pState->ulVertScalerStep / pState->ulVertAveragerCount );

#if defined(BCHP_M2MC_VERT_SCALER_1_INITIAL_PHASE)
        /* set scaler registers for YCbCr 420 format */
        if( hGrc->bYCbCr420Source )
        {
            BGRC_P_SET_FIELD_DATA( VERT_SCALER_1_INITIAL_PHASE, PHASE, pState->ulVertInitPhase / 2 );/*-
                pState->ulVertScalerStep / 2 );*/
            BGRC_P_SET_FIELD_DATA( VERT_SCALER_1_STEP, STEP, pState->ulVertScalerStep / 2 );
        }
#endif
        }
        else
        {
            /* disable vertical scaling and averagering */
            BGRC_P_SET_FIELD_ENUM( SCALER_CTRL, VERT_SCALER_ENABLE, DISABLE );
        }

        /* set register load field */
        if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) ||
            BGRC_P_REGISTER_CHANGED( HORIZ_AVERAGER_COUNT ) ||
            BGRC_P_REGISTER_CHANGED( HORIZ_AVERAGER_COEFF ) ||
            BGRC_P_REGISTER_CHANGED( HORIZ_SCALER_INITIAL_PHASE ) ||
            BGRC_P_REGISTER_CHANGED( HORIZ_SCALER_STEP ) ||
            BGRC_P_REGISTER_CHANGED( VERT_AVERAGER_COUNT ) ||
            BGRC_P_REGISTER_CHANGED( VERT_AVERAGER_COEFF ) ||
            BGRC_P_REGISTER_CHANGED( VERT_SCALER_INITIAL_PHASE ) ||
            BGRC_P_REGISTER_CHANGED( VERT_SCALER_STEP ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }
    else
    {
        /* disable scaling */
        BGRC_P_SET_FIELD_ENUM( BLIT_HEADER, SRC_SCALER_ENABLE, DISABLE );
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, STRIPE_ENABLE, DISABLE );
        BGRC_P_SET_FIELD_ENUM( SCALER_CTRL, HORIZ_SCALER_ENABLE, DISABLE );
        BGRC_P_SET_FIELD_ENUM( SCALER_CTRL, VERT_SCALER_ENABLE, DISABLE );

        /* set register load field */
        if( BGRC_P_REGISTER_CHANGED( SCALER_CTRL ) )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    /* set interrupt enable field */
    BGRC_P_SET_FIELD_DATA( BLIT_HEADER, INTERRUPT_ENABLE, bEnableInterrupt ? 1 : 0 );

    /* check if loading a source palette */
    if( BGRC_P_COMPARE_VALUE( SRC_SURFACE_FORMAT_DEF_1, FORMAT_TYPE, 3 ) &&
        BGRC_P_COMPARE_FIELD( SRC_SURFACE_FORMAT_DEF_3, PALETTE_BYPASS, LOOKUP ) )
    {
        if( pState->SrcSurface.ulPaletteOffset )
            BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, SRC_CLUT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    /* set register load field */
    if( BGRC_P_REGISTER_CHANGED( BLIT_HEADER ) ||
        BGRC_P_REGISTER_CHANGED( BLIT_CTRL ) ||
        (hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(LIST_PACKET_HEADER_1)] == 0) )
    {
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE, pState, hGrc->aulCurrentRegs );
    }

    return BERR_SUCCESS;
}

#define BGRC_P_OPTIMIZE_SET_REGISTER

/***************************************************************************/
static void BGRC_P_List_WritePacket(
    BGRC_Handle hGrc,
    uint32_t *pulPacket,
    uint32_t ulPacketHeader )
{
    BGRC_P_State *pState = &hGrc->CurrentState;

    /* write list header */
    *pulPacket++ = BCHP_FIELD_ENUM(M2MC_LIST_PACKET_HEADER_0, LAST_PKT_IND, NextPktInvalid);
    *pulPacket++ = ulPacketHeader;

    /* write source group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SRC_FEEDER_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( SRC_FEEDER, SRC_FEEDER_ENABLE );
#else
        BGRC_P_SET_REGISTER( SRC_FEEDER_ENABLE );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_0_MSB );
#endif
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_0 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_0_BOT_FLD_MSB );
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_0_BOT_FLD );
#endif
        BGRC_P_SET_REGISTER( SRC_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_MSB
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_1_MSB );
#endif
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_1 );
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_1_BOT_FLD
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_1_BOT_FLD_MSB );
        BGRC_P_SET_REGISTER( SRC_SURFACE_ADDR_1_BOT_FLD );
#endif
        BGRC_P_SET_REGISTER( SRC_SURFACE_STRIDE_1 );
        BGRC_P_SET_REGISTER( SRC_SURFACE_FORMAT_DEF_1 );
        BGRC_P_SET_REGISTER( SRC_SURFACE_FORMAT_DEF_2 );
        BGRC_P_SET_REGISTER( SRC_SURFACE_FORMAT_DEF_3 );
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
        BGRC_P_SET_REGISTER( SRC_SURFACE_1_FORMAT_DEF_1 );
        BGRC_P_SET_REGISTER( SRC_SURFACE_1_FORMAT_DEF_2 );
        BGRC_P_SET_REGISTER( SRC_SURFACE_1_FORMAT_DEF_3 );
#endif
        BGRC_P_SET_REGISTER( SRC_W_ALPHA );
        BGRC_P_SET_REGISTER( SRC_CONSTANT_COLOR );
#endif
    }

    /* write destination group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, DST_FEEDER_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( DST_FEEDER, DEST_FEEDER_ENABLE );
#else
        BGRC_P_SET_REGISTER( DEST_FEEDER_ENABLE );
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_0_MSB
        BGRC_P_SET_REGISTER( DEST_SURFACE_ADDR_0_MSB );
#endif
        BGRC_P_SET_REGISTER( DEST_SURFACE_ADDR_0 );
        BGRC_P_SET_REGISTER( DEST_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_DEST_SURFACE_ADDR_1_MSB
        BGRC_P_SET_REGISTER( DEST_SURFACE_ADDR_1_MSB );
#endif
        BGRC_P_SET_REGISTER( DEST_SURFACE_ADDR_1 );
        BGRC_P_SET_REGISTER( DEST_SURFACE_STRIDE_1 );
        BGRC_P_SET_REGISTER( DEST_SURFACE_FORMAT_DEF_1 );
        BGRC_P_SET_REGISTER( DEST_SURFACE_FORMAT_DEF_2 );
        BGRC_P_SET_REGISTER( DEST_SURFACE_FORMAT_DEF_3 );
        BGRC_P_SET_REGISTER( DEST_W_ALPHA );
        BGRC_P_SET_REGISTER( DEST_CONSTANT_COLOR );
#endif
    }

    /* write output group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, OUTPUT_FEEDER_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( OUTPUT_FEEDER, OUTPUT_FEEDER_ENABLE );
#else
        BGRC_P_SET_REGISTER( OUTPUT_FEEDER_ENABLE );
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_0_MSB
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_ADDR_0_MSB );
#endif
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_ADDR_0 );
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_STRIDE_0 );
#ifdef BCHP_M2MC_OUTPUT_SURFACE_ADDR_1_MSB
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_ADDR_1_MSB );
#endif
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_ADDR_1 );
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_STRIDE_1 );
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_FORMAT_DEF_1 );
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_FORMAT_DEF_2 );
        BGRC_P_SET_REGISTER( OUTPUT_SURFACE_FORMAT_DEF_3 );
#endif
    }

    /* write blit group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( BLIT, BLIT_HEADER );
#else
        BGRC_P_SET_REGISTER( BLIT_HEADER );
        BGRC_P_SET_REGISTER( BLIT_SRC_TOP_LEFT );
        BGRC_P_SET_REGISTER( BLIT_SRC_SIZE );
#if defined(BCHP_M2MC_BLIT_SRC_TOP_LEFT_1)
        BGRC_P_SET_REGISTER( BLIT_SRC_TOP_LEFT_1 );
        BGRC_P_SET_REGISTER( BLIT_SRC_SIZE_1 );
        BGRC_P_SET_REGISTER( BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 );
        BGRC_P_SET_REGISTER( BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 );
#endif
        BGRC_P_SET_REGISTER( BLIT_DEST_TOP_LEFT );
        BGRC_P_SET_REGISTER( BLIT_DEST_SIZE );
        BGRC_P_SET_REGISTER( BLIT_OUTPUT_TOP_LEFT );
        BGRC_P_SET_REGISTER( BLIT_OUTPUT_SIZE );
        BGRC_P_SET_REGISTER( BLIT_INPUT_STRIPE_WIDTH );
#if defined(BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_1)
        BGRC_P_SET_REGISTER( BLIT_INPUT_STRIPE_WIDTH_1 );
#endif
        BGRC_P_SET_REGISTER( BLIT_OUTPUT_STRIPE_WIDTH );
        BGRC_P_SET_REGISTER( BLIT_STRIPE_OVERLAP );
#if defined(BCHP_M2MC_BLIT_STRIPE_OVERLAP_1)
        BGRC_P_SET_REGISTER( BLIT_STRIPE_OVERLAP_1 );
#endif
        BGRC_P_SET_REGISTER( BLIT_CTRL );
#if defined(BCHP_M2MC_DCEG_CFG)
        BGRC_P_SET_REGISTER( DCEG_CFG );
#endif
#endif
    }

    /* write scaler group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SCALE_PARAM_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( SCALE_PARAM, SCALER_CTRL );
#else
        BGRC_P_SET_REGISTER( SCALER_CTRL );
        BGRC_P_SET_REGISTER( HORIZ_AVERAGER_COUNT );
        BGRC_P_SET_REGISTER( HORIZ_AVERAGER_COEFF );
        BGRC_P_SET_REGISTER( VERT_AVERAGER_COUNT );
        BGRC_P_SET_REGISTER( VERT_AVERAGER_COEFF );
        BGRC_P_SET_REGISTER( HORIZ_SCALER_INITIAL_PHASE );
        BGRC_P_SET_REGISTER( HORIZ_SCALER_STEP );
#if defined(BCHP_M2MC_HORIZ_SCALER_1_INITIAL_PHASE)
        BGRC_P_SET_REGISTER( HORIZ_SCALER_1_INITIAL_PHASE );
        BGRC_P_SET_REGISTER( HORIZ_SCALER_1_STEP );
#endif
        BGRC_P_SET_REGISTER( VERT_SCALER_INITIAL_PHASE );
        BGRC_P_SET_REGISTER( VERT_SCALER_STEP );
#if defined(BCHP_M2MC_VERT_SCALER_1_INITIAL_PHASE)
        BGRC_P_SET_REGISTER( VERT_SCALER_1_INITIAL_PHASE );
        BGRC_P_SET_REGISTER( VERT_SCALER_1_STEP );
#endif
#endif
    }

    /* write blend group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, BLEND_PARAM_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( BLEND_PARAM, BLEND_COLOR_OP );
#else
        BGRC_P_SET_REGISTER( BLEND_COLOR_OP );
        BGRC_P_SET_REGISTER( BLEND_ALPHA_OP );
        BGRC_P_SET_REGISTER( BLEND_CONSTANT_COLOR );
        BGRC_P_SET_REGISTER( BLEND_COLOR_KEY_ACTION );
#endif
    }

    /* write rop group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, ROP_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( ROP, ROP_OPERATION );
#else
        BGRC_P_SET_REGISTER( ROP_OPERATION );
        BGRC_P_SET_REGISTER( ROP_PATTERN_TOP );
        BGRC_P_SET_REGISTER( ROP_PATTERN_BOTTOM );
        BGRC_P_SET_REGISTER( ROP_PATTERN_COLOR_0 );
        BGRC_P_SET_REGISTER( ROP_PATTERN_COLOR_1 );
#endif
    }

    /* write source color key group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SRC_COLOR_KEY_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( SRC_COLOR_KEY, SRC_COLOR_KEY_HIGH );
#else
        BGRC_P_SET_REGISTER( SRC_COLOR_KEY_HIGH );
        BGRC_P_SET_REGISTER( SRC_COLOR_KEY_LOW );
        BGRC_P_SET_REGISTER( SRC_COLOR_KEY_MASK );
        BGRC_P_SET_REGISTER( SRC_COLOR_KEY_REPLACEMENT );
        BGRC_P_SET_REGISTER( SRC_COLOR_KEY_REPLACEMENT_MASK );
#endif
    }

    /* write destination color key group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, DST_COLOR_KEY_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( DST_COLOR_KEY, DEST_COLOR_KEY_HIGH );
#else
        BGRC_P_SET_REGISTER( DEST_COLOR_KEY_HIGH );
        BGRC_P_SET_REGISTER( DEST_COLOR_KEY_LOW );
        BGRC_P_SET_REGISTER( DEST_COLOR_KEY_MASK );
        BGRC_P_SET_REGISTER( DEST_COLOR_KEY_REPLACEMENT );
        BGRC_P_SET_REGISTER( DEST_COLOR_KEY_REPLACEMENT_MASK );
#endif
    }

    /* write scaler coefficient group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SCALE_COEF_GRP_CNTRL, GRP_ENABLE ) )
    {
        uint32_t ii, jj;

#if defined(BCHP_M2MC_HORIZ_FIR_1_COEFF_PHASE0_01)
        uint32_t kk;

        /* write horizontal coefficient group registers */
        for( kk = 0; kk < 2; ++kk )
#endif
            for( jj = 0; jj < BGRC_P_FIR_PHASE_COUNT; ++jj )
                for( ii = 0; ii < BGRC_P_FIR_TAP_COUNT / 2; ++ii )
                    *pulPacket++ = pState->pulHorzFirCoeffs[ii + jj * BGRC_P_FIR_TAP_COUNT / 2];

        /* write vertical coefficient group registers */
#if defined(BCHP_M2MC_HORIZ_FIR_1_COEFF_PHASE0_01)
        for( kk = 0; kk < 2; ++kk )
#endif
            for( jj = 0; jj < BGRC_P_FIR_PHASE_COUNT; ++jj )
                for( ii = 0; ii < BGRC_P_FIR_TAP_COUNT / 2; ++ii )
                    *pulPacket++ = pState->pulVertFirCoeffs[ii + jj * BGRC_P_FIR_TAP_COUNT / 2];
    }

    /* write source color matrix group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SRC_COLOR_MATRIX_GRP_CNTRL, GRP_ENABLE ) )
    {
#ifdef BGRC_P_OPTIMIZE_SET_REGISTER
        BGRC_P_SET_REGISTERS( SRC_COLOR_MATRIX, SRC_CM_C00_C01 );
#else
        BGRC_P_SET_REGISTER( SRC_CM_C00_C01 );
        BGRC_P_SET_REGISTER( SRC_CM_C02_C03 );
        BGRC_P_SET_REGISTER( SRC_CM_C04 );
        BGRC_P_SET_REGISTER( SRC_CM_C10_C11 );
        BGRC_P_SET_REGISTER( SRC_CM_C12_C13 );
        BGRC_P_SET_REGISTER( SRC_CM_C14 );
        BGRC_P_SET_REGISTER( SRC_CM_C20_C21 );
        BGRC_P_SET_REGISTER( SRC_CM_C22_C23 );
        BGRC_P_SET_REGISTER( SRC_CM_C24 );
        BGRC_P_SET_REGISTER( SRC_CM_C30_C31 );
        BGRC_P_SET_REGISTER( SRC_CM_C32_C33 );
        BGRC_P_SET_REGISTER( SRC_CM_C34 );
#endif
    }

    /* write source palette group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, SRC_CLUT_GRP_CNTRL, GRP_ENABLE ) )
        *pulPacket++ = pState->SrcSurface.ulPaletteOffset;

    /* write destination palette group registers */
    if( BGRC_P_COMPARE_FIELD( LIST_PACKET_HEADER_1, DST_CLUT_GRP_CNTRL, GRP_ENABLE ) )
        *pulPacket++ = pState->DstSurface.ulPaletteOffset;
}

/***************************************************************************/
static void BGRC_P_List_RenderPacket(
    BGRC_Handle hGrc,
    uint32_t ulPacketOffset )
{
    /* write packet offset to previous packet header */
    if( hGrc->pulPrevPacket )
    {
        *hGrc->pulPrevPacket = ulPacketOffset | BCHP_FIELD_DATA(M2MC_LIST_PACKET_HEADER_0, LAST_PKT_IND,
            BCHP_M2MC_LIST_PACKET_HEADER_0_LAST_PKT_IND_NextPktValid);
#if 1
        BMEM_FlushCache( hGrc->hMemory, hGrc->pulPrevPacket, 4 );
#endif
    }

    if( hGrc->pulPrevPacket == NULL )
    {
        /* write first packet offset and initiate */
        BGRC_P_WRITE_REG( LIST_FIRST_PKT_ADDR, ulPacketOffset );
        BGRC_P_WRITE_REG( LIST_CTRL,
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, WAKE_MODE, ResumeFromFirst) |
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, RUN, Run) |
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, WAKE, Ack) );
    }
    else
    {
        /* initiate packet */
        BGRC_P_WRITE_REG( LIST_CTRL,
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, WAKE_MODE, ResumeFromLast) |
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, RUN, Run) |
            BCHP_FIELD_ENUM(M2MC_LIST_CTRL, WAKE, WakeUp) );
    }
}

/***************************************************************************/
void BGRC_P_List_PacketIsr(
    void *pvParam1,
    int iParam2 )
{
    BGRC_Handle hGrc = (BGRC_Handle) pvParam1;
    BGRC_P_Operation *pOp;
    uint32_t ulDeviceOffset;
    bool bDeviceBusy;

    BSTD_UNUSED( iParam2 );

    if( hGrc == 0 )
        return;

    ulDeviceOffset = BGRC_P_READ_REG( LIST_CURR_PKT_ADDR ) & BGRC_M2MC(LIST_CURR_PKT_ADDR_CURR_PKT_ADDR_MASK);

    bDeviceBusy = (((BGRC_P_READ_REG( BLIT_STATUS ) & BGRC_M2MC(BLIT_STATUS_STATUS_MASK)) == BGRC_M2MC(BLIT_STATUS_STATUS_RUNNING))/* ||
        ((BGRC_P_READ_REG( LIST_STATUS ) & BGRC_M2MC(LIST_STATUS_BUSY_MASK)) == BGRC_M2MC(LIST_STATUS_BUSY_Busy)) ||
        ((BGRC_P_READ_REG( LIST_STATUS ) & BGRC_M2MC(LIST_STATUS_FINISHED_MASK)) == BGRC_M2MC(LIST_STATUS_FINISHED_NotFinished)) */);

    /* find first active operation */
    pOp = hGrc->pLastOp;
    while( pOp && (!pOp->bActive) )
        pOp = pOp->pPrevOp;

    if( pOp == NULL )
        return;

    if( hGrc->pLastOp == NULL )
        return;

    while( ((!bDeviceBusy) && (pOp->pPrevOp == NULL)) || (ulDeviceOffset != pOp->ulPacketOffset) )
    {
        BGRC_P_Operation *pPrevOp = pOp->pPrevOp;
        BGRC_P_Operation *pCurrOp = pOp;

        /* call user callback function */
        if( pOp->pUserCallback )
            (*pOp->pUserCallback)( hGrc, pOp->pUserData );

        /* set events */
        if( pOp->bSetEvent )
            BKNI_SetEvent( hGrc->hInterruptEvent );

        /* set period event and call callback */
        if( pOp->bSetPeriodicEvent )
        {
            if( hGrc->pPeriodicCallback )
            {
                (*hGrc->pPeriodicCallback)( hGrc, hGrc->pPeriodicData );
                hGrc->pPeriodicCallback = NULL;
            }

            if( hGrc->ulPeriodicInterrupts )
                hGrc->ulPeriodicInterrupts--;

            BKNI_SetEvent( hGrc->hPeriodicEvent );
        }

        if( pOp->pUserCallback || pOp->bSetEvent )
            hGrc->ulIntReceived++;

        /* get next operation and inactivate current operation */
        pOp = pPrevOp;
        pCurrOp->bActive = false;

        /* exit loop if no more operations */
        if( pOp == NULL )
            break;
    }
}

/***************************************************************************/
static BGRC_P_Block *BGRC_P_List_CreateBlock(
    BGRC_Handle hGrc )
{
    /* allocate memory for block structure */
    BGRC_P_Block *pBlock = (BGRC_P_Block *) BKNI_Malloc( sizeof (BGRC_P_Block) );
    if( pBlock == NULL )
        return NULL;

    /* allocate device memory for list block */
    pBlock->pvMemory = BMEM_AllocAligned( hGrc->hMemory, BGRC_P_LIST_BLOCK_SIZE, BGRC_P_LIST_BLOCK_ALIGN, 0 );
    if( pBlock->pvMemory == NULL )
    {
        BKNI_Free( (void *) pBlock );
        return NULL;
    }

    /* convert block address to offset */
    if( BMEM_ConvertAddressToOffset( hGrc->hMemory, pBlock->pvMemory, &pBlock->ulOffset ) != BERR_SUCCESS )
    {
        BMEM_Free( hGrc->hMemory, pBlock->pvMemory );
        BKNI_Free( (void *) pBlock );
        return NULL;
    }

    /* get cached address */
    if( BMEM_ConvertAddressToCached( hGrc->hMemory, pBlock->pvMemory, &pBlock->pvCached ) != BERR_SUCCESS )
    {
        BMEM_Free( hGrc->hMemory, pBlock->pvMemory );
        BKNI_Free( (void *) pBlock );
        return NULL;
    }

    hGrc->ulPacketMemorySize += BGRC_P_LIST_BLOCK_SIZE;
    pBlock->ulRefCount = 0;
/*  pBlock->bBusy = true;*/

    return pBlock;
}

/***************************************************************************/
static BGRC_P_Block *BGRC_P_List_DestroyBlock(
    BGRC_Handle hGrc,
    BGRC_P_Block *pBlock )
{
    BGRC_P_Block *pNextBlock = pBlock->pNextBlock;

    /* free block memory */
    BMEM_Free( hGrc->hMemory, pBlock->pvMemory );
    BKNI_Free( (void *) pBlock );
    hGrc->ulPacketMemorySize -= BGRC_P_LIST_BLOCK_SIZE;

    /* dettach block from list */
    if( hGrc->pCurrListBlock == pNextBlock )
    {
        hGrc->pPrevListBlock = NULL;
        pNextBlock = NULL;
    }

    if( hGrc->pPrevListBlock )
        hGrc->pPrevListBlock->pNextBlock = pNextBlock;

    hGrc->pCurrListBlock = pNextBlock;
    return pNextBlock;
}

/***************************************************************************/
/* OPERATION PROCESSING FUNCTIONS                                          */
/***************************************************************************/
static BGRC_P_Operation *BGRC_P_Operation_Alloc(
    BGRC_Handle hGrc,
    BGRC_Callback pCallback,
    void *pData,
    BGRC_P_Block *pBlock,
    uint32_t ulPacketOffset,
    bool bSetEvent )
{
    BGRC_P_Operation *pOp;

    /* check if there are any free operation structures */
    if( hGrc->pFreeOp )
    {
        /* get a free operation structure */
        pOp = hGrc->pFreeOp;
        hGrc->pFreeOp = pOp->pNextOp;
    }
    else
    {
        /* return error if operation max reached */
        if( hGrc->bPreAllocMemory && (hGrc->ulOperationCount == hGrc->ulOperationMax) )
            return NULL;

        /* allocate operation structure */
        pOp = (BGRC_P_Operation *) BKNI_Malloc( sizeof (BGRC_P_Operation) );
        if( pOp == NULL )
            return NULL;
        hGrc->ulOperationCount++;
    }

    pOp->pPrevOp = NULL;
    pOp->pNextOp = NULL;
    pOp->pUserCallback = pCallback;
    pOp->pUserData = pData;
    pOp->pBlock = pBlock;
    pOp->ulPacketOffset = ulPacketOffset;
    pOp->bSetEvent = bSetEvent;
    pOp->bActive = true;
    pOp->bSetPeriodicEvent = false;

    if( pCallback || bSetEvent )
        hGrc->ulIntExpected++;

    return pOp;
}

/***************************************************************************/
void BGRC_P_Operation_FreeAll(
    BGRC_Handle hGrc )
{
    BGRC_P_Operation *pOp = hGrc->pFreeOp;
    while( pOp )
    {
        BGRC_P_Operation *pNextOp = pOp->pNextOp;
        BKNI_Free( pOp );
        pOp = pNextOp;
    }

    hGrc->pFreeOp = NULL;
}

/***************************************************************************/
static void BGRC_P_Operation_AddToList(
    BGRC_Handle hGrc,
    BGRC_P_Operation *pOp )
{
    if( hGrc->pCurrOp )
        hGrc->pCurrOp->pPrevOp = pOp;

    if( hGrc->pLastOp == NULL )
        hGrc->pLastOp = pOp;

    pOp->pNextOp = hGrc->pCurrOp;
    hGrc->pCurrOp = pOp;
}

/***************************************************************************/
static void BGRC_P_Operation_RemoveFromList(
    BGRC_Handle hGrc,
    BGRC_P_Operation *pOp )
{
    BGRC_P_Operation *pPrevOp = pOp->pPrevOp;

    /* dettach operation from list */
    if( hGrc->pCurrOp == hGrc->pLastOp )
        hGrc->pCurrOp = NULL;

    hGrc->pLastOp = pPrevOp;
    if( pPrevOp )
    {
        pPrevOp->pNextOp = NULL;
        pOp->pPrevOp = NULL;
    }

    /* decrement reference count of packet memory block */
    pOp->pBlock->ulRefCount--;

    /* check if more operations allocated than allowed */
    if( hGrc->ulOperationMax && (!hGrc->bPreAllocMemory) && (hGrc->ulOperationCount > hGrc->ulOperationMax) )
    {
        /* free operation memory */
        BKNI_Free( pOp );
        hGrc->ulOperationCount--;
    }
    else
    {
        /* attach operation to free list */
        pOp->pNextOp = hGrc->pFreeOp;
        hGrc->pFreeOp = pOp;
    }
}

/***************************************************************************/
void BGRC_P_Operation_CleanupList(
    BGRC_Handle hGrc )
{
    BGRC_P_Operation *pOp;

    /* remove inactive operations from list */
    pOp = hGrc->pLastOp;
    while( pOp && (!pOp->bActive) )
    {
        BGRC_P_Operation *pPrevOp = pOp->pPrevOp;
        BGRC_P_Operation_RemoveFromList( hGrc, pOp );
        pOp = pPrevOp;
    }
}

/***************************************************************************/
bool BGRC_P_Operation_Prealloc(
    BGRC_Handle hGrc,
    uint32_t ulCount )
{
    BGRC_P_Operation *pPrevOp = NULL;
    uint32_t ii;

    /* setup free operations list */
    for( ii = 0; ii < ulCount; ++ii )
    {
        BGRC_P_Operation *pOp = (BGRC_P_Operation *) BKNI_Malloc( sizeof (BGRC_P_Operation) );
        if( pOp == NULL )
            return false;

        if( pPrevOp )
            pPrevOp->pNextOp = pOp;
        else
            hGrc->pFreeOp = pOp;

        pPrevOp = pOp;
    }

    if( pPrevOp )
        pPrevOp->pNextOp = NULL;

    hGrc->ulOperationCount = ulCount;
    return true;
}

/* End of File */
