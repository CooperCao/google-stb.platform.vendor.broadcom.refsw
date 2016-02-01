/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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

