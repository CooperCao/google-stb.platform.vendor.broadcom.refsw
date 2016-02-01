/***************************************************************************
 *
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 *   Contains tables for Display settings.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/* for all drop tables */
#define BVDC_P_MAKE_DROPTBL(t)  (t),

/* for BVDC_P_aFormatInfoTable */
#define BVDC_P_MAKE_FMTINFO(FMT, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd) \
    {BFMT_VideoFmt_##FMT, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, {SavRmv, SavRp, EavPrd}},

/* for s_aFormatDataTable */
#define BVDC_P_MAKE_FMTDATA(FMT, AnalogUcode, DigiUcode, DigiUcodeDropTbl, ItTbl, ItCfg) \
    {BFMT_VideoFmt_##FMT, AnalogUcode, DigiUcode, DigiUcodeDropTbl, ItTbl, ItCfg},

/* for s_HdmiRm
 */
#if (BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_1)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k) \
    {BFMT_PXL_##a, b, c, d, e, f, g, h, i, j, k},

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_2) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_3)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l) \
    {BFMT_PXL_##a, b, c, d, e, f, g, h, i, j, k, l},

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_4) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)  \
    {BFMT_PXL_##a, BAVC_HDMI_BitsPerPixel_##b, BAVC_HDMI_PixelRepetition_##c, BAVC_Colorspace_##d, e, f, g, h, i, j, k, l, m, n, o, p},

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) \
    {BFMT_PXL_##a, BAVC_HDMI_BitsPerPixel_##b, BAVC_HDMI_PixelRepetition_##c, BAVC_Colorspace_##d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v},

#else /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j) \
    {BFMT_PXL_##a, b, c, d, e, f, g, h, i, j},

#endif /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */

/* for misc tables */
#define BVDC_P_MAKE_MISC(fmt, t) {BFMT_VideoFmt_##fmt, t},

/* for rate manager tables */
#define BVDC_P_MAKE_RMTBL(frq, t, n) {BFMT_PXL_##frq, t, n},

/* End of File */
