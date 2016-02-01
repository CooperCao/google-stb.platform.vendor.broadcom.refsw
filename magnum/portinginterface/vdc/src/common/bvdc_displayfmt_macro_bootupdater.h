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
#include "bfmt_pick.h"

/* for all drop tables */
#define BVDC_P_MAKE_DROPTBL(t)

/* for BVDC_P_aFormatInfoTable */
#define BVDC_P_MAKE_FMTINFO_1(fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd) \
    {fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, {SavRmv, SavRp, EavPrd}},
#define BVDC_P_MAKE_FMTINFO_0(fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd)
#define BVDC_P_MAKE_FMTINFO__(pick, fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd) \
    BVDC_P_MAKE_FMTINFO_##pick(fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd)
#define BVDC_P_MAKE_FMTINFO_(pick, fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd) \
    BVDC_P_MAKE_FMTINFO__(pick, fmt, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd)
#define BVDC_P_MAKE_FMTINFO(FMT, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd) \
    BVDC_P_MAKE_FMTINFO_(BFMT_PICK_##FMT, BFMT_VideoFmt_##FMT, bHD, bED, bSD, bVESA, bProg, bMacVin, bDCS, bHDMI, bUseDropTbl, SavRmv, SavRp, EavPrd)

/* for s_aFormatDataTable */
#define BVDC_P_MAKE_FMTDATA_1(fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg) \
    {fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg},
#define BVDC_P_MAKE_FMTDATA_0(fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg)
#define BVDC_P_MAKE_FMTDATA__(pick, fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg) \
    BVDC_P_MAKE_FMTDATA_##pick(fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg)
#define BVDC_P_MAKE_FMTDATA_(pick, fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg) \
    BVDC_P_MAKE_FMTDATA__(pick, fmt, analogUcode, digiUcode, digiUcodeDropTbl, itTbl, itCfg)
#define BVDC_P_MAKE_FMTDATA(FMT, AnalogUcode, DigiUcode, DigiUcodeDropTbl, ItTbl, ItCfg) \
    BVDC_P_MAKE_FMTDATA_(BFMT_PICK_##FMT, BFMT_VideoFmt_##FMT, AnalogUcode, DigiUcode, DigiUcodeDropTbl, ItTbl, ItCfg)

/* for s_HdmiRm
 * note: The following #define with BFMT_PICK_* will make all entries in s_HdmiRm with
 * (ColorDepth, PixelRep) != (e24bit, eNone) be compiled out.
 * if you want to support other color depth and pixel repeat mode in bootupdater, you need to change
 * the following #define with BFMT_PICK_*
 */
#define BFMT_PICK_BPP_e24bit   1
#define BFMT_PICK_BPP_e30bit   0
#define BFMT_PICK_BPP_e36bit   0
#define BFMT_PICK_BPP_e48bit   0
#define BFMT_PICK_BPP_ePacked  0

#define BFMT_PICK_PR_eNone     1
#define BFMT_PICK_PR_e2x       0
#define BFMT_PICK_PR_e3x       0
#define BFMT_PICK_PR_e4x       0
#define BFMT_PICK_PR_e5x       0
#define BFMT_PICK_PR_e6x       0
#define BFMT_PICK_PR_e7x       0
#define BFMT_PICK_PR_e8x       0
#define BFMT_PICK_PR_e9x       0
#define BFMT_PICK_PR_e10x      0

#if (BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_1)
#define BVDC_P_MAKE_HDMIRM_1(a, b, c, d, e, f, g, h, i, j, k)  \
    {BFMT_PXL_##a, b, c, d, e, f, g, h, i, j, NULL},
#define BVDC_P_MAKE_HDMIRM_0(a, b, c, d, e, f, g, h, i, j, k)
#define BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j, k) \
    BVDC_P_MAKE_HDMIRM_##pick(a, b, c, d, e, f, g, h, i, j, k)
#define BVDC_P_MAKE_HDMIRM_(pick, a, b, c, d, e, f, g, h, i, j, k) \
    BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j, k)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k)  \
    BVDC_P_MAKE_HDMIRM_(BFMT_PICK_PXL_##a, a, b, c, d, e, f, g, h, i, j, k)

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_2) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_3)
#define BVDC_P_MAKE_HDMIRM_1(a, b, c, d, e, f, g, h, i, j, k, l) \
    {BFMT_PXL_##a, b, c, d, e, f, g, h, i, j, k, NULL},
#define BVDC_P_MAKE_HDMIRM_0(a, b, c, d, e, f, g, h, i, j, k, l)
#define BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j, k, l) \
    BVDC_P_MAKE_HDMIRM_##pick(a, b, c, d, e, f, g, h, i, j, k, l)
#define BVDC_P_MAKE_HDMIRM_(pick, a, b, c, d, e, f, g, h, i, j, k, l) \
    BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j, k, l)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l)  \
    BVDC_P_MAKE_HDMIRM_(BFMT_PICK_PXL_##a, a, b, c, d, e, f, g, h, i, j, k, l)

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_4) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)
#define BVDC_P_MAKE_HDMIRM_111(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    {BFMT_PXL_##a, BAVC_HDMI_BitsPerPixel_##b, BAVC_HDMI_PixelRepetition_##c, d, e, f, g, h, i, j, k, l, m, n, o, NULL},
#define BVDC_P_MAKE_HDMIRM_000(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_001(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_010(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_011(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_100(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_101(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_110(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM__(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    BVDC_P_MAKE_HDMIRM_##pick1##pick2##pick3(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM_(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    BVDC_P_MAKE_HDMIRM__(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)  \
    BVDC_P_MAKE_HDMIRM_(BFMT_PICK_PXL_##a, BFMT_PICK_BPP_##b, BFMT_PICK_PR_##c, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7)
#define BVDC_P_MAKE_HDMIRM_111(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) \
    {BFMT_PXL_##a, BAVC_HDMI_BitsPerPixel_##b, BAVC_HDMI_PixelRepetition_##c, BAVC_Colorspace_##d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, NULL, v},
#define BVDC_P_MAKE_HDMIRM_000(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_001(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_010(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_011(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_100(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_101(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_110(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM__(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) \
    BVDC_P_MAKE_HDMIRM_##pick1##pick2##pick3(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM_(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v) \
    BVDC_P_MAKE_HDMIRM__(pick1, pick2, pick3,  a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)  \
    BVDC_P_MAKE_HDMIRM_(BFMT_PICK_PXL_##a, BFMT_PICK_BPP_##b, BFMT_PICK_PR_##c, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v)

#else /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */
#define BVDC_P_MAKE_HDMIRM_1(a, b, c, d, e, f, g, h, i, j)  {BFMT_PXL_##a, b, c, d, e, f, g, h, i, NULL},
#define BVDC_P_MAKE_HDMIRM_0(a, b, c, d, e, f, g, h, i, j)
#define BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j) \
    BVDC_P_MAKE_HDMIRM_##pick(a, b, c, d, e, f, g, h, i, j)
#define BVDC_P_MAKE_HDMIRM_(pick, a, b, c, d, e, f, g, h, i, j) \
    BVDC_P_MAKE_HDMIRM__(pick, a, b, c, d, e, f, g, h, i, j)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j)  \
    BVDC_P_MAKE_HDMIRM_(BFMT_PICK_PXL_##a, a, b, c, d, e, f, g, h, i, j)

#endif /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */

/* for misc tables */
#define BVDC_P_MAKE_MISC_1(fmt, t) {fmt, t},
#define BVDC_P_MAKE_MISC_0(fmt, t)
#define BVDC_P_MAKE_MISC__(pick, fmt, t) BVDC_P_MAKE_MISC_##pick(fmt, t)
#define BVDC_P_MAKE_MISC_(pick, fmt, t)  BVDC_P_MAKE_MISC__(pick, fmt, t)
#define BVDC_P_MAKE_MISC(fmt, t) \
   BVDC_P_MAKE_MISC_(BFMT_PICK_##fmt, BFMT_VideoFmt_##fmt, t)

/* for rate manager tables */
#define BVDC_P_MAKE_RMTBL_1(frq, t, n) {frq, t, NULL},
#define BVDC_P_MAKE_RMTBL_0(frq, t, n)
#define BVDC_P_MAKE_RMTBL__(pick, frq, t, n) BVDC_P_MAKE_RMTBL_##pick(frq, t, n)
#define BVDC_P_MAKE_RMTBL_(pick, frq, t, n)  BVDC_P_MAKE_RMTBL__(pick, frq, t, n)
#define BVDC_P_MAKE_RMTBL(frq, t, n) \
   BVDC_P_MAKE_RMTBL_(BFMT_PICK_PXL_##frq, BFMT_PXL_##frq, t, n)

/* End of File */
