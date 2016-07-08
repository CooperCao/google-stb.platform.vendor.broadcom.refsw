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
#ifndef BVDC_TNT_H__
#define BVDC_TNT_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This structure describes the sharpness settings.

Description:
    BVDC_SharpnessSettings is a structure containing
    custom luma and chroma settings used with TNT.

    Warning: This may change in future chip revisions.

    ulLumaCtrlCore         - Larger values prevent sharpening of low-level
    textures and noise. Values greater than 32 (or less than -32) are not
    commonly used. This is a signed number S7.0. Legal range: -128 ~ 127.

    ulLumaCtrlGain         - Larger values increase luma sharpness (or
    increase softness if "LUMA_CTRL_SOFTEN" is set). Zero disables luma
    sharpness (or softness). Values greater than 32 are usually too harsh
    for normal viewing. This is an unsigned number U6.0. Legal range: 0 ~ 63.

    ulLumaCtrlBlur         - Large values reduce sharpness for very high
    frequence texture and prevent sharpening of noise. Legal range: 0 ~ 2.

    bLumaCtrlSoften        - If enable, TNT will soften luma data and we
    recommend to set "LUMA_CTRL_CORE" to be 0. If disable, TNT will sharpen
    luma data.

    bLumaCtrlHOnly         - When enabled, TNT will only sharpen luma data
    horizontally, when disabled, TNT will sharpen luma data in both
    (horizontal and vertical) direction.

    ulLumaPeakingHAvoid    - Luma vertical sharp edge avoidance control.
    Larger values avoid luma sharpening of strong vertical edges. This is an
    unsigned number U6.0. Legal range: 0 ~ 63.

    ulLumaPeakingVAvoid    - Luma horizontal sharp edge avoidance. Larger
    values avoid luma sharpening of strong horizontal edges. This is an
    unsigned number U6.0. Legal range: 0 ~ 63.

    ulLumaPeakingPeakLimit - This value sets a hard limit on luma overshoot.
    This is an unsigned number U7.0. Legal range: 0 ~ 127.

    ulLumaPeakingPeakValue - Larger values reduce luma overshoot. This is an
    unsigned number U5.0. Legal range: 0 ~ 31.

    ulChromaCtrlCore       - Larger values prevent sharpening of low-level
    textures and noise. This is an unsigned number U8.0. Legal range: 0 ~ 255.

    bChromaCtrlWideChroma  - When enabled, TNT will use wider range for
    chroma peaking. When disabled, TNT will use normal range for chroma
    peaking.

    ulChromaCtrlFalseColor - This is to avoid false color when sharpening
    sharp edges of chroma data. Larger values will reduce false colors, at
    the cost of some chroma sharpness. This is an unsigned number U3.0.
    Legal range: 0 ~ 7.

    ulChromaCtrlGain       - Larger values increase chroma sharpness. Zero
    disables chroma. sharpness. Values greater than 32 are usually too harsh
    for normal viewing. This is an unsigned number U6.0. Legal range: 0 ~ 63.

    bChromaCtrlHOnly       - When enabled, TNT will only sharpen chroma data
    horizontally, when disabled, TNT will sharpen chroma data in both
    (horizontal and vertical) direction.

    ulWideLumaCtrlCore     - Larger values prevent sharpening of low-level
    textures and noise. Values greater than 32 (or less than -32) are not
    commonly used. This is a signed number S7.0. Legal range: -128 ~ 127.

    ulWideLumaCtrlMode     - Wide luma mode control.
    2'b00: luma peaking is kept narrow for tight, crisp peaking.
    2'b01: luma peaking use a wider range for more dramatic, intense peaking.
    2'b10: luma peaking use the widest range for dramatic, intense peaking.

See Also:
    BVDC_Window_SetSharpnessConfig, BVDC_Window_GetSharpnessConfig
***************************************************************************/
typedef struct
{
    uint32_t                      ulLumaCtrlCore;
    uint32_t                      ulLumaCtrlGain;
    uint32_t                      ulLumaCtrlBlur;
    bool                          bLumaCtrlSoften;
    bool                          bLumaCtrlHOnly;
    uint32_t                      ulLumaPeakingHAvoid;
    uint32_t                      ulLumaPeakingVAvoid;
    uint32_t                      ulLumaPeakingPeakLimit;
    uint32_t                      ulLumaPeakingPeakValue;
    uint32_t                      ulChromaCtrlCore;
    bool                          bChromaCtrlWideChroma;
    uint32_t                      ulChromaCtrlFalseColor;
    uint32_t                      ulChromaCtrlGain;
    bool                          bChromaCtrlHOnly;
    uint32_t                      ulWideLumaCtrlCore;
    uint32_t                      ulWideLumaCtrlMode;
    uint32_t                      ulSimpleLumaCtrlCore;
    bool                          bSimpleLumaCtrlMode;
} BVDC_SharpnessSettings;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_TNT_H__ */

/* End of File */
