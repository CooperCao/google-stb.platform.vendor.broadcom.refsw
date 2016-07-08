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
 * [File Description:]
 *
 ***************************************************************************/
#include "bvdc_compositor_priv.h"
#include "bvdc_tnt.h"
#include "bvdc_tnt_priv.h"
#include "bvdc_source_priv.h"

BDBG_MODULE(BVDC_TNT);

#if (BVDC_P_SUPPORT_TNT_VER == 5)            /* TNT2 HW base */
#define BVDC_P_TNT_VAL_MIN                 INT16_MIN
#define BVDC_P_TNT_VAL_MAX                 INT16_MAX
#define BVDC_P_TNT_VAL_MID_1               INT16_MIN + (INT16_MAX - INT16_MIN)/3
#define BVDC_P_TNT_VAL_MID_2               INT16_MAX - (INT16_MAX - INT16_MIN)/3

#define BVDC_P_TNT_MAX_INDEX               2

/***************************************************************************
 * {private}
 * This section is for TNT algorithms
 */

static BVDC_SharpnessSettings astSharpness[BVDC_P_TNT_MAX_INDEX] =
{
    {/* SD sharpness */
        /* Luma Peaking */
        {
            {{0x0, 0x0},
             {0x20, 0x20},{0x20, 0x20}, {0x0, 0x0}, {0x20, 0x20}, {0x20, 0x20}, {0x20, 0x20}, {0x20, 0x20}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}},
            0x0,
            {0x5, 0x5, 0x5},
            0x0,
            {{0x70, 0x70, 0x78, 0x0}, {0xc8, 0x190}},
            {{0x0, 0x0, 0x0, 0x0}, {0xc8, 0x190}},
            0x0,
            {0x2, 0x384},
            {0x2, 0x32}
        },
        /* LTI */
        {
            0x0,
            {0x0, 0x0, 0x0, 0x0},
            {0xc8, 0x190},
            0x10, 0x1, 0x1, 0x2, 0x8, 0x8, 0x4, 0x0
        },
        /* CTI */
            {0x10, 0x10, 0x1},
            {0x10, 0x10, 0x1},
        /* Chroma-based filtering */
        0x1,
        {
            {
                0x1, 0x12a, 0xb5, 0x90, 0x2d, 0x3,
                {
                    {0x2, {{0x270, 0x40}, {0x2d5, 0x40}, {0x373, 0x1a}, {0x3dd, 0x0}}},
                    {0x3, {{0xbd, 0x0}, {0xce, 0x40}, {0x114, 0x1a}, {0x127, 0x0}}},
                    {0x4, {{0x39, 0x0}, {0x3f, 0x40}, {0x7a, 0x40}, {0x88, 0x0}}}
                },
                0x1, 0x1, 0x0, 0xfffffff0, 0x10
            },
            {
                0x1, 0x1fe, 0x15e, 0x1f6, 0xa, 0x3,
                {
                    {0x2, {{0x53, 0x0}, {0xd2, 0x40}, {0x373, 0x40}, {0x3d5, 0x0}}},
                    {0x3, {{0x15f, 0x0}, {0x160, 0x40}, {0x161, 0x40}, {0x1da, 0x0}}},
                    {0x4, {{0x16, 0x0}, {0xa6, 0x2d}, {0x176, 0x40}, {0x1e1, 0x0}}}
                },
                0x1, 0x1, 0x0, 0x10, 0xfffffff0
            },
            {
                0x1, 0x2ec, 0x253, 0x1fa, 0x12, 0x3,
                {
                    {0x2, {{0x6a, 0x40}, {0x1eb, 0x40}, {0x314, 0x40}, {0x353, 0x0}}},
                    {0x3, {{0x254, 0x0}, {0x255, 0x40}, {0x256, 0x20}, {0x257, 0x0}}},
                    {0x4, {{0x2d, 0x26}, {0x54, 0x40}, {0x175, 0x26}, {0x1ed, 0x0}}}
                },
                0x1, 0x1, 0x0, 0x0, 0x0
            },
            {
                0x1, 0x12a, 0xbc, 0x209, 0x93, 0x3,
                {
                    {0x2, {{0x5d, 0x40}, {0x155, 0x40}, {0x23a, 0x40}, {0x2ac, 0x0}}},
                    {0x3, {{0xbd, 0x0}, {0xca, 0x40}, {0xeb, 0x40}, {0x112, 0xffffffff}}},
                    {0x4, {{0xa4, 0x0}, {0xfa, 0x40}, {0x1bd, 0x29}, {0x1f3, 0x0}}}
                },
                0x1, 0x1, 0x0, 0xa, 0x0
            }
        },
        /* Luma Histogram */
        0x0
    },

    {/* HD sharpness */
        /* Luma Peaking */
        {
            {{0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x10, 0x10}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x18, 0x18}, {0x18, 0x18}, {0x18, 0x18}, {0x18, 0x18}, {0x18, 0x18}, {0x18, 0x18}},
            0x0,
            {0x8, 0x8, 0x8},
            0x0,
            {{0x0, 0x0, 0x0, 0x0}, {0xc8, 0x190}},
            {{0x70, 0x70, 0x78, 0x0}, {0xc8, 0x190}},
            0x0,
            {0x2, 0x384},
            {0x2, 0x32}
        },
        /* LTI */
        {
            0x0,
            {0x0, 0x0, 0x0, 0x0},
            {0xc8, 0x190},
            0x10, 0x1, 0x1, 0x2, 0x0, 0x0, 0x4, 0x0
        },
        /* CTI */
            {0x10, 0x10, 0x1},
            {0x10, 0x10, 0x1},
        /* Chroma-based filtering */
        0x1,
        {
            {
                0x1, 0x12a, 0xb5, 0x90, 0x2d, 0x3,
                {
                    {0x2, {{0x270, 0x40}, {0x2d5, 0x40}, {0x373, 0x1a}, {0x3dd, 0x0}}},
                    {0x3, {{0xbd, 0x0}, {0xce, 0x40}, {0x114, 0x1a}, {0x127, 0x0}}},
                    {0x4, {{0x39, 0xffffffff}, {0x3f, 0x40}, {0x7a, 0x40}, {0x88, 0x0}}}
                },
                0x1, 0x1, 0x0, 0xfffffff0, 0x10
            },
            {
                0x1, 0x1fe, 0x15e, 0x1f6, 0xa, 0x3,
                {
                    {0x2, {{0x53, 0x0}, {0xd2, 0x40}, {0x373, 0x40}, {0x3d5, 0x0}}},
                    {0x3, {{0x15f, 0x0}, {0x160, 0x40}, {0x161, 0x40}, {0x1da, 0x0}}},
                    {0x4, {{0x16, 0x0}, {0xa6, 0x2d}, {0x176, 0x40}, {0x1e1, 0x0}}}
                },
                0x1, 0x1, 0x0, 0x10, 0xfffffff0
            },
            {
                0x1, 0x2ec, 0x253, 0x1fa, 0x12, 0x3,
                {
                    {0x2, {{0x6a, 0x40}, {0x1eb, 0x40}, {0x314, 0x40}, {0x353, 0x0}}},
                    {0x3, {{0x254, 0x0}, {0x255, 0x40}, {0x256, 0x20}, {0x257, 0x0}}},
                    {0x4, {{0x2d, 0x26}, {0x54, 0x40}, {0x175, 0x26}, {0x1ed, 0x0}}}
                },
                0x1, 0x1, 0x0, 0x0, 0x0
            },
            {
                0x1, 0x12a, 0xbc, 0x209, 0x93, 0x3,
                {
                    {0x2, {{0x5d, 0x40}, {0x155, 0x40}, {0x23a, 0x40}, {0x2ac, 0x0}}},
                    {0x3, {{0xbd, 0x0}, {0xca, 0x40}, {0xeb, 0x40}, {0x112, 0xffffffff}}},
                    {0x4, {{0xa4, 0x0}, {0xfa, 0x40}, {0x1bd, 0x29}, {0x1f3, 0x0}}}
                },
                0x1, 0x1, 0x0, 0xa, 0x0
            }
        },
        /* Luma Histogram */
        0x0
    }

};

/*************************************************************************
 *  {secret}
 *  BVDC_P_Tnt_Calculate_LumaPeakingGain
 *  Calculate Luma peaking gain values from sSharpness value
 *
 **************************************************************************/
static void BVDC_P_Tnt_Calculate_LumaPeakingGain
    ( const int16_t                      sSharpness,
      uint32_t                           ulTblIndex,
      BVDC_Sharpness_LumaPeakingGain     aulPeakingGain[BVDC_MAX_LUMA_PEAKING_FREQ_BANDS] )
{
    uint32_t i;

    for (i=0; i<BVDC_MAX_LUMA_PEAKING_FREQ_BANDS; i++)
    {
        aulPeakingGain[i].ulOvershootGain =
                (sSharpness + (BVDC_P_TNT_VAL_MAX + 1)) *
                astSharpness[ulTblIndex].stLumaPeaking.astGain[i].ulOvershootGain/(UINT16_MAX + 1);
        aulPeakingGain[i].ulUndershootGain =
                (sSharpness + (BVDC_P_TNT_VAL_MAX + 1)) *
                astSharpness[ulTblIndex].stLumaPeaking.astGain[i].ulUndershootGain/(UINT16_MAX + 1);
        BDBG_MSG(("Inteprolated peaking gains[%d] = %d,%d",
            i, aulPeakingGain[i].ulUndershootGain, aulPeakingGain[i].ulOvershootGain));
    }

}

BERR_Code BVDC_P_Tnt_InterpolateSharpness
    ( BVDC_Window_Handle         hWindow,
      const int16_t              sSharpness)
{
    BERR_Code                    err = BERR_SUCCESS;
    BVDC_P_Window_Info          *pNewInfo, *pCurInfo;
    BVDC_SharpnessSettings       stSharpnessSettings;
    uint32_t                     ulIndex = 0;

    BDBG_ENTER(BVDC_P_Tnt_InterpolateSharpness);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pNewInfo = &hWindow->stNewInfo;
    pCurInfo = &hWindow->stCurInfo;

    BDBG_OBJECT_ASSERT(pCurInfo->hSource, BVDC_SRC);

    /* Determine if source is SD/ED or HD */
    ulIndex = pCurInfo->hSource->stCurInfo.pVdcFmt->bHd ? 1 : 0;

    stSharpnessSettings = astSharpness[ulIndex];

    /* Luma Peaking Gain */
    BVDC_P_Tnt_Calculate_LumaPeakingGain(sSharpness, ulIndex, stSharpnessSettings.stLumaPeaking.astGain);

    err = BVDC_P_Tnt_ValidateSharpnessSettings(&stSharpnessSettings);
    if (err == BERR_SUCCESS)
    {
        /* set new value */
        pNewInfo->stSharpnessConfig = stSharpnessSettings;

        BVDC_P_Tnt_StoreSharpnessSettings(hWindow, &stSharpnessSettings);
    }

    BDBG_LEAVE(BVDC_P_Tnt_InterpolateSharpness);
    return err;
}

#endif /* BVDC_P_SUPPORT_TNT_VER >= 5 */
