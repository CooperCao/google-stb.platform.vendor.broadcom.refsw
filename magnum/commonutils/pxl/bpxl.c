/******************************************************************************
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
 ******************************************************************************/


#include "bstd.h"
#include "bstd_defs.h"
#include "berr.h"
#include "bdbg.h"
#include "bpxl.h"
#include "bchp_common.h"
#ifdef BCHP_M2MC_REG_START
#include "bchp_m2mc.h"     /* for gfx compression mode. */
#endif
#include "bpxl_priv.h"


typedef struct
{
    BPXL_Format ePxlFmt;
    const char *pcFmtName;
} BPXL_P_Format_Info;


/****************************************************************************/
/* Making an entry for pixel format                                         */
/****************************************************************************/
#define BPXL_P_MAKE_PXL(PxlFmt)                                             \
{                                                                           \
    (PxlFmt),                                                               \
    BDBG_STRING(#PxlFmt)                                                    \
}

static const BPXL_P_Format_Info s_aPxlFmtInfo[] =
{
    /* YCbCr 444 */
    BPXL_P_MAKE_PXL(BPXL_eA8_Y8_Cb8_Cr8),
    BPXL_P_MAKE_PXL(BPXL_eCr8_Cb8_Y8_A8),
    BPXL_P_MAKE_PXL(BPXL_eY8_Cb8_Cr8_A8),
    BPXL_P_MAKE_PXL(BPXL_eA8_Cr8_Cb8_Y8),

    /* YCbCr 422 */
    BPXL_P_MAKE_PXL(BPXL_eCr8_Y18_Cb8_Y08),
    BPXL_P_MAKE_PXL(BPXL_eY18_Cr8_Y08_Cb8),
    BPXL_P_MAKE_PXL(BPXL_eY08_Cb8_Y18_Cr8),
    BPXL_P_MAKE_PXL(BPXL_eCb8_Y08_Cr8_Y18),
    BPXL_P_MAKE_PXL(BPXL_eCb8_Y18_Cr8_Y08),
    BPXL_P_MAKE_PXL(BPXL_eY18_Cb8_Y08_Cr8),
    BPXL_P_MAKE_PXL(BPXL_eY08_Cr8_Y18_Cb8),
    BPXL_P_MAKE_PXL(BPXL_eCr8_Y08_Cb8_Y18),

    /* YCbCr 420 */
    BPXL_P_MAKE_PXL(BPXL_eY8),
    BPXL_P_MAKE_PXL(BPXL_eCb8_Cr8),
    BPXL_P_MAKE_PXL(BPXL_eCr8_Cb8),

    /* YCbCr 444 10-bit */
    BPXL_P_MAKE_PXL(BPXL_eX2_Cr10_Y10_Cb10),

    /* YCbCr 422 10-bit */
    BPXL_P_MAKE_PXL(BPXL_eX2_Y010_Cb10_Y110_X2_Cr10_Y010_Cb10_X2_Y110_Cr10_Y010_X2_Cb10_Y110_Cr10),

    /* YCbCr 422 10-bit (packed 40-bit pixel)*/
    BPXL_P_MAKE_PXL(BPXL_eCr10_Y110_Cb10_Y010),
    BPXL_P_MAKE_PXL(BPXL_eY110_Cr10_Y010_Cb10),
    BPXL_P_MAKE_PXL(BPXL_eY010_Cb10_Y110_Cr10),
    BPXL_P_MAKE_PXL(BPXL_eCb10_Y010_Cr10_Y110),
    BPXL_P_MAKE_PXL(BPXL_eCb10_Y110_Cr10_Y010),
    BPXL_P_MAKE_PXL(BPXL_eY110_Cb10_Y010_Cr10),
    BPXL_P_MAKE_PXL(BPXL_eY010_Cr10_Y110_Cb10),
    BPXL_P_MAKE_PXL(BPXL_eCr10_Y010_Cb10_Y110),

    /* RGB */
    BPXL_P_MAKE_PXL(BPXL_eA8_R8_G8_B8),
    BPXL_P_MAKE_PXL(BPXL_eA8_B8_G8_R8),
    BPXL_P_MAKE_PXL(BPXL_eR8_G8_B8_A8),
    BPXL_P_MAKE_PXL(BPXL_eB8_G8_R8_A8),

    BPXL_P_MAKE_PXL(BPXL_eX8_R8_G8_B8),
    BPXL_P_MAKE_PXL(BPXL_eX8_B8_G8_R8),
    BPXL_P_MAKE_PXL(BPXL_eR8_G8_B8_X8),
    BPXL_P_MAKE_PXL(BPXL_eB8_G8_R8_X8),

    BPXL_P_MAKE_PXL(BPXL_eR8_G8_B8),
    BPXL_P_MAKE_PXL(BPXL_eB8_G8_R8),

    BPXL_P_MAKE_PXL(BPXL_eR5_G6_B5),
    BPXL_P_MAKE_PXL(BPXL_eB5_G6_R5),

    BPXL_P_MAKE_PXL(BPXL_eA1_R5_G5_B5),
    BPXL_P_MAKE_PXL(BPXL_eA1_B5_G5_R5),
    BPXL_P_MAKE_PXL(BPXL_eR5_G5_B5_A1),
    BPXL_P_MAKE_PXL(BPXL_eB5_G5_R5_A1),

    BPXL_P_MAKE_PXL(BPXL_eX1_R5_G5_B5),
    BPXL_P_MAKE_PXL(BPXL_eX1_B5_G5_R5),
    BPXL_P_MAKE_PXL(BPXL_eR5_G5_B5_X1),
    BPXL_P_MAKE_PXL(BPXL_eB5_G5_R5_X1),

    BPXL_P_MAKE_PXL(BPXL_eW1_R5_G5_B5),
    BPXL_P_MAKE_PXL(BPXL_eW1_B5_G5_R5),
    BPXL_P_MAKE_PXL(BPXL_eR5_G5_B5_W1),
    BPXL_P_MAKE_PXL(BPXL_eB5_G5_R5_W1),

    BPXL_P_MAKE_PXL(BPXL_eA4_R4_G4_B4),
    BPXL_P_MAKE_PXL(BPXL_eA4_B4_G4_R4),
    BPXL_P_MAKE_PXL(BPXL_eR4_G4_B4_A4),
    BPXL_P_MAKE_PXL(BPXL_eB4_G4_R4_A4),

    BPXL_P_MAKE_PXL(BPXL_eX4_R4_G4_B4),
    BPXL_P_MAKE_PXL(BPXL_eX4_B4_G4_R4),
    BPXL_P_MAKE_PXL(BPXL_eR4_G4_B4_X4),
    BPXL_P_MAKE_PXL(BPXL_eB4_G4_R4_X4),

    /* Palette */
    BPXL_P_MAKE_PXL(BPXL_eP1),
    BPXL_P_MAKE_PXL(BPXL_eP2),
    BPXL_P_MAKE_PXL(BPXL_eP4),
    BPXL_P_MAKE_PXL(BPXL_eP8),
    BPXL_P_MAKE_PXL(BPXL_eA8_P8),
    BPXL_P_MAKE_PXL(BPXL_eY8_P8),

    /* Alpha */
    BPXL_P_MAKE_PXL(BPXL_eA1),
    BPXL_P_MAKE_PXL(BPXL_eA2),
    BPXL_P_MAKE_PXL(BPXL_eA4),
    BPXL_P_MAKE_PXL(BPXL_eA8),
    BPXL_P_MAKE_PXL(BPXL_eW1),

    /* 3D (Special) */
    BPXL_P_MAKE_PXL(BPXL_eL8),
    BPXL_P_MAKE_PXL(BPXL_eL8_A8),
    BPXL_P_MAKE_PXL(BPXL_eZ16),

    BPXL_P_MAKE_PXL(BPXL_eL4_A4),
    BPXL_P_MAKE_PXL(BPXL_eL15_L05_A6),

    /* compressed format */
    BPXL_P_MAKE_PXL(BPXL_eCompressed_A8_R8_G8_B8),
    BPXL_P_MAKE_PXL(BPXL_eUIF_R8_G8_B8_A8),

    BPXL_P_MAKE_PXL(BPXL_INVALID)
};

/***************************************************************************/
static unsigned int BPXL_ConvertComponent_static_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int uiCompNum
    )
{
    unsigned int uiDstComp = 0;
    unsigned int uiSrcSize;
    unsigned int uiDstSize;

    /* Convert pixel components */
    uiSrcSize = BPXL_COMPONENT_SIZE(eSrcFormat, uiCompNum);
    uiDstSize = BPXL_COMPONENT_SIZE(eDstFormat, uiCompNum);

    /* Check if both src and dst have specified component */
    if( uiSrcSize && uiDstSize )
    {
        unsigned int uiSrcPos = BPXL_COMPONENT_POS(eSrcFormat, uiCompNum);
        unsigned int uiSrcMask = BPXL_COMPONENT_MASK(eSrcFormat, uiCompNum);
        unsigned int uiDstMask = BPXL_COMPONENT_MASK(eDstFormat, uiCompNum);

        if( uiSrcSize >= uiDstSize )
        {
            /* Convert component down in size */
            uiDstComp = (uiSrcPixel >> (uiSrcPos + uiSrcSize - uiDstSize)) & uiDstMask;
        }
        else
        {
            /* Convert component up using high to low order replication */
            signed int siBitCount = uiDstSize;

            while( siBitCount > 0 )
            {
                siBitCount -= (signed int) uiSrcSize;
                uiDstComp |= (siBitCount > 0) ?
                    ((uiSrcPixel >> uiSrcPos) & uiSrcMask) << siBitCount :
                    ((uiSrcPixel >> uiSrcPos) & uiSrcMask) >> siBitCount;
            }
        }
    }

    return uiDstComp;
}

void BPXL_ConvertComponent_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int uiCompNum,
    unsigned int *puiDstComp
    )
{
    /* Check parameter */
    BDBG_ASSERT(puiDstComp);
    *puiDstComp = BPXL_ConvertComponent_static_isrsafe(eDstFormat, eSrcFormat, uiSrcPixel, uiCompNum);
    return;
}

/***************************************************************************/
void BPXL_ConvertPixel_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int *puiDstPixel )
{
    unsigned int uiDstPixel = 0;
    unsigned int uiDstComp;
    unsigned int ii;

    /* Check parameter */
    BDBG_ASSERT(puiDstPixel);

    /* Convert pixel components */
    if( BPXL_IS_YCbCr422_FORMAT(eSrcFormat) && (!BPXL_IS_YCbCr422_FORMAT(eDstFormat)) )
    {
        for( ii = 0; ii < 3; ++ ii )
        {
            uiDstComp = BPXL_ConvertComponent_static_isrsafe(
                eDstFormat, eSrcFormat, uiSrcPixel, ii);
            uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, ii);
        }
    }
    else if( (!BPXL_IS_YCbCr422_FORMAT(eSrcFormat)) && BPXL_IS_YCbCr422_FORMAT(eDstFormat) )
    {
        uiDstComp = BPXL_ConvertComponent_static_isrsafe(
            eDstFormat, eSrcFormat, uiSrcPixel, 0);
        uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, 0);
        uiDstComp = BPXL_ConvertComponent_static_isrsafe(
            eDstFormat, eSrcFormat, uiSrcPixel, 1);
        uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, 1);
        uiDstComp = BPXL_ConvertComponent_static_isrsafe(
            eDstFormat, eSrcFormat, uiSrcPixel, 2);
        uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, 2);
        uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, 3);
    }
    else
    {
        for( ii = 0; ii < 4; ++ ii )
        {
            uiDstComp = BPXL_ConvertComponent_static_isrsafe(
                eDstFormat, eSrcFormat, uiSrcPixel, ii);
            uiDstPixel |= uiDstComp << BPXL_COMPONENT_POS(eDstFormat, ii);
        }
    }

    *puiDstPixel = uiDstPixel;

    return;
}

/***************************************************************************/
void BPXL_ConvertPixel_RGBtoYCbCr_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int *puiDstPixel )
{
    unsigned int ulA, ulR, ulG, ulB;
    unsigned int ulY, ulCr, ulCb;

    /* Check parameter */
    BDBG_ASSERT(puiDstPixel);

    /* Get ARGB components */
    ulB = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 0);
    ulG = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 1);
    ulR = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 2);
    ulA = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 3);

    /* Convert RGB components to YCbCr */
    /* Y  = R *  0.257 + G *  0.504 + B *  0.098 + 16  */
    /* Cb = R * -0.148 + G * -0.291 + B *  0.439 + 128 */
    /* Cr = R *  0.439 + G * -0.368 + B * -0.071 + 128 */
    ulY  = ulR *  0x41CA + ulG *  0x8106 + ulB *  0x1916 + 0x100000;
    ulCb = ulR * -0x25E3 + ulG * -0x4A7F + ulB *  0x7062 + 0x800000;
    ulCr = ulR *  0x7062 + ulG * -0x5E35 + ulB * -0x122D + 0x800000;

    /* Make destination pixel */
    if( eDstFormat == BPXL_eX2_Cr10_Y10_Cb10 )
    {
        ulY  = ulY >> 14;
        ulCb = ulCb >> 14;
        ulCr = ulCr >> 14;
        *puiDstPixel = BPXL_MAKE_PIXEL_10BIT(eDstFormat, ulY, ulCb, ulCr);
    }
    else
    {
        ulY  = ulY >> 16;
        ulCb = ulCb >> 16;
        ulCr = ulCr >> 16;
        *puiDstPixel = BPXL_MAKE_PIXEL(eDstFormat, BPXL_HAS_ALPHA(eDstFormat) ? ulA : ulY, ulY, ulCb, ulCr);
    }

    return ;
}

/***************************************************************************/
void BPXL_ConvertPixel_RGBtoHdYCbCr_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int *puiDstPixel )
{
    unsigned int ulA, ulR, ulG, ulB;
    unsigned int ulY, ulCr, ulCb;

    /* Check parameter */
    BDBG_ASSERT(puiDstPixel);

    /* Get ARGB components */
    ulB = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 0);
    ulG = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 1);
    ulR = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 2);
    ulA = BPXL_ConvertComponent_static_isrsafe(
        BPXL_eA8_R8_G8_B8, eSrcFormat, uiSrcPixel, 3);

    /* Convert RGB components to YCbCr */
    /* Y  = R *  0.183 + G *  0.614 + B *  0.062 + 16  */
    /* Cb = R * -0.101 + G *-0.338 + B *  0.439 + 128 */
    /* Cr = R *  0.439 + G * -0.399 + B * -0.04 + 128 */
    ulY  = ulR *  0x2ED9 + ulG *  0x9D2F + ulB *  0xFDF + 0x100000;
    ulCb = ulR * -0x19DB + ulG * -0x5687 + ulB *  0x7062 + 0x800000;
    ulCr = ulR *  0x7062 + ulG * -0x6624 + ulB * -0xA3D + 0x800000;

    /* Make destination pixel */
    if( eDstFormat == BPXL_eX2_Cr10_Y10_Cb10 )
    {
        ulY  = ulY >> 14;
        ulCb = ulCb >> 14;
        ulCr = ulCr >> 14;
        *puiDstPixel = BPXL_MAKE_PIXEL_10BIT(eDstFormat, ulY, ulCb, ulCr);
    }
    else
    {
        ulY  = ulY >> 16;
        ulCb = ulCb >> 16;
        ulCr = ulCr >> 16;
        *puiDstPixel = BPXL_MAKE_PIXEL(eDstFormat, BPXL_HAS_ALPHA(eDstFormat) ? ulA : ulY, ulY, ulCb, ulCr);
    }

    return;
}

/***************************************************************************/
void BPXL_ConvertPixel_YCbCrtoRGB_isrsafe(
    BPXL_Format eDstFormat,
    BPXL_Format eSrcFormat,
    unsigned int uiSrcPixel,
    unsigned int uiSrcAlign,
    unsigned int uiSrcAlpha,
    unsigned int *puiDstPixel )
{
    signed int lA, lR, lG, lB;
    signed int lY, lY0, lY1, lCr, lCb;

    /* Check parameter */
    BDBG_ASSERT(puiDstPixel);

    /* Get YCbCr components */
    lCr = BPXL_GET_COMPONENT(eSrcFormat, uiSrcPixel, 0) - 128;
    lCb = BPXL_GET_COMPONENT(eSrcFormat, uiSrcPixel, 1) - 128;
    lY1 = BPXL_GET_COMPONENT(eSrcFormat, uiSrcPixel, 2) - 16;
    lY0 = BPXL_GET_COMPONENT(eSrcFormat, uiSrcPixel, 3) - 16;
    lY  = (BPXL_HAS_ALPHA(eSrcFormat) || (uiSrcAlign & 1)) ? lY1 : lY0;

    /* Convert to ARGB components */
    /* R = (Y - 16) * 1.164 + (Cr - 128) * 1.596 */
    /* G = (Y - 16) * 1.164 - (Cr - 128) * 0.813 - (Cb - 128) * 0.391 */
    /* B = (Y - 16) * 1.164 + (Cb - 128) * 2.018 */
    lA = (signed int) (BPXL_HAS_ALPHA(eSrcFormat) ? BPXL_GET_COMPONENT(eSrcFormat, uiSrcPixel, 3) : uiSrcAlpha);
    lR = (signed int) ((lY * 0x129FB + lCr * 0x19893) >> 16);
    lG = (signed int) ((lY * 0x129FB - lCr * 0x0D020 - lCb * 0x06418) >> 16);
    lB = (signed int) ((lY * 0x129FB + lCb * 0x2049B) >> 16);

    /* Clamp RGB components */
    lR = (lR > 0xFF) ? 0xFF : ((lR < 0) ? 0 : lR);
    lG = (lG > 0xFF) ? 0xFF : ((lG < 0) ? 0 : lG);
    lB = (lB > 0xFF) ? 0xFF : ((lB < 0) ? 0 : lB);

    /* Make destination pixel */
    *puiDstPixel = BPXL_MAKE_PIXEL(eDstFormat, lA, lR, lG, lB);

    return;
}

/***************************************************************************/
void BPXL_GetBytesPerNPixels_isrsafe(
    BPXL_Format eFormat,
    unsigned int uiNPixels,
    unsigned int *puiBytes )
{
    unsigned int uiBytes = 0;
    unsigned int ulAlignment = BPXL_IS_YCbCr422_FORMAT(eFormat)?
        BPXL_P_PITCH_YCBCR422_ALIGNMENT:BPXL_P_PITCH_ALIGNMENT;

    /* Check parameter */
    BDBG_ASSERT(puiBytes);

    /* check if format is YCbCr 422 10-bit non-packed format */
    if( BPXL_IS_YCbCr422_10BIT_FORMAT(eFormat) )
    {
        /* align size to 6 pixels (per 4 dwords) */
        uiBytes = ((uiNPixels + 5) / 6) * 16;
    }
    /* check if format is YCbCr 422 10-bit packed format (eg. BPXL_eCr10_Y110_Cb10_Y010)*/
    else if( BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(eFormat) )
    {
        /* align size to 8 pixels (per 5 dwords) */
        uiBytes = ((uiNPixels + 7) / 8) * 20;
    }
    /* check if format is YCbCr 420 Luma 10-bit (non-packed) format (BPXL_eY10) */
    else if (BPXL_IS_YCbCr420_LUMA_10BIT_FORMAT(eFormat))
    {
        /* align size to 6 pixels (per 2 dwords) */
        uiBytes = ((uiNPixels + 5) / 6) * 8;
    }
    /* check if format is YCbCr 420 Chroma 10-bit (non-packed) format (BPXL_eCb10_Cr10)*/
    else if (BPXL_IS_YCbCr420_CHROMA_10BIT_FORMAT(eFormat))
    {
        /* align size to 3 pixels (per 2 dwords), width passed in is already half of luma plane */
        uiBytes = ((uiNPixels + 2) / 3) * 8;
    }
    /* check if format is BPXL_eCompressed_A8_R8_G8_B8 */
    else if( BPXL_eCompressed_A8_R8_G8_B8 == eFormat )
    {
#if defined(BCHP_M2MC_DCEG_CFG)
        uiBytes = ((((uiNPixels + 3) / 4) * 66) + 7) / 8;
#elif defined(BCHP_M2MC_BSTC_COMPRESS_CONTROL)
        uiBytes = ((uiNPixels + 3) / 4) * 8;
#else
        uiBytes = uiNPixels * 4; /* non-compressed ARGB8888 */
#endif
    }
    else if( BPXL_eUIF_R8_G8_B8_A8 == eFormat )
    {
        /* TODO: varies by height, assume height > 8
        4 bytes per pixel, rounded up to 128 bytes */
        uiBytes = ((uiNPixels*4)+127) & ~127; /* round up to 128 */
    }
    else if( eFormat != BPXL_INVALID )
    {
        unsigned int uiBitsPerPixel = BPXL_BITS_PER_PIXEL(eFormat);

        /* calculate bytes for sub-byte formats */
        if( uiBitsPerPixel < 8 )
        {
            unsigned int uiPixPerByte = (8 / uiBitsPerPixel);
            uiBytes = (uiNPixels + (uiPixPerByte - 1)) / uiPixPerByte;
        }
        /* calculate bytes for formats that are a byte or larger */
        else
        {
            /* align size of YCbCr 422 and YCbCr 420 luma formats to 2 pixels */
            if( BPXL_IS_YCbCr422_FORMAT(eFormat) || BPXL_IS_YCbCr420_LUMA_FORMAT(eFormat) )
                uiNPixels = (uiNPixels + 1) & (~1);

            uiBytes = (uiNPixels * uiBitsPerPixel) / 8;
        }
    }

    *puiBytes = BPXL_P_ALIGN_UP(uiBytes, ulAlignment);
    return;
}

/***************************************************************************/
const char* BPXL_ConvertFmtToStr_isrsafe(
        BPXL_Format eFormat )
{
    uint32_t i = 0;

    while (s_aPxlFmtInfo[i].ePxlFmt != BPXL_INVALID)
    {
        if (s_aPxlFmtInfo[i].ePxlFmt == eFormat)
        {
            return s_aPxlFmtInfo[i].pcFmtName;
        }
        i++;
    }

    return BDBG_STRING("BPXL_INVALID");
}
/* End of File */
