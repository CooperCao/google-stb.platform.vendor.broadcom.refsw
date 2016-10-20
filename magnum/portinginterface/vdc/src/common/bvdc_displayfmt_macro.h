/******************************************************************************
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
 *****************************************************************************/

#include "bfmt_pick.h"

/* for all drop tables */
#define BVDC_P_MAKE_DROPTBL(t)
#define BVDC_P_HAS_MAKE_DROPTBL 0

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
#ifdef BVDC_FOR_BOOTUPDATER /** { **/

#define BFMT_PICK_BPP_e24bit   1
#define BFMT_PICK_BPP_e30bit   0
#define BFMT_PICK_BPP_e36bit   0
#define BFMT_PICK_BPP_e48bit   0
#define BFMT_PICK_BPP_ePacked  0

#define BFMT_PICK_PR_eNone     1
#define BFMT_PICK_PR_e1x       0
#define BFMT_PICK_PR_e3x       0
#define BFMT_PICK_PR_e4x       0
#define BFMT_PICK_PR_e5x       0
#define BFMT_PICK_PR_e6x       0
#define BFMT_PICK_PR_e7x       0
#define BFMT_PICK_PR_e8x       0
#define BFMT_PICK_PR_e9x       0
#define BFMT_PICK_PR_e10x      0

#else /** }  BVDC_FOR_BOOTUPDATER { **/

#define BFMT_PICK_BPP_e24bit   1
#define BFMT_PICK_BPP_e30bit   1
#define BFMT_PICK_BPP_e36bit   1
#define BFMT_PICK_BPP_e48bit   1
#define BFMT_PICK_BPP_ePacked  1

#define BFMT_PICK_PR_eNone     1
#define BFMT_PICK_PR_e1x       1
#define BFMT_PICK_PR_e3x       1
#define BFMT_PICK_PR_e4x       1
#define BFMT_PICK_PR_e5x       1
#define BFMT_PICK_PR_e6x       1
#define BFMT_PICK_PR_e7x       1
#define BFMT_PICK_PR_e8x       1
#define BFMT_PICK_PR_e9x       1
#define BFMT_PICK_PR_e10x      1

#endif  /** }  BVDC_FOR_BOOTUPDATER **/

#if BFMT_DO_PICK || BDBG_DEBUG_BUILD
    #define P_CONDITIONAL(a,b) BVDC_P_TmdsClock_##a, b
#else
    #define P_CONDITIONAL(a,b) b
#endif

#if (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)
#include "bvdc_hdmirm_lupick_40nm.h"
#define BVDC_P_MAKE_HDMIRM_0(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)
#define BVDC_P_MAKE_HDMIRM_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) \
    { P_CONDITIONAL(a, b), c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r },
#define BVDC_P_MAKE_HDMIRM__(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) \
    BVDC_P_MAKE_HDMIRM_##pick1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)
#define BVDC_P_MAKE_HDMIRM_(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) \
    BVDC_P_MAKE_HDMIRM__(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) \
    BVDC_P_MAKE_HDMIRM_(BVDC_PICK_TMDS_##a, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r)

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7)
#include "bvdc_hdmirm_lupick_28nm.h"
#define BVDC_P_MAKE_HDMIRM_0(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)
#define BVDC_P_MAKE_HDMIRM_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) \
    { P_CONDITIONAL(a, b), c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w },
#define BVDC_P_MAKE_HDMIRM__(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) \
    BVDC_P_MAKE_HDMIRM_##pick1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)
#define BVDC_P_MAKE_HDMIRM_(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) \
    BVDC_P_MAKE_HDMIRM__(pick1, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)
#define BVDC_P_MAKE_HDMIRM(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w) \
    BVDC_P_MAKE_HDMIRM_(BVDC_PICK_TMDS_##a, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w)

#else /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */
#error Unknown/undefined HDMI Rate Manager hardware version

#endif /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */

#define BVDC_P_MAKE_HDMILU_000(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_001(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_010(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_011(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_100(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_101(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_110(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_111(a, b, c, d, e) \
    { BFMT_PXL_##a, BFMT_ClockMod_##b, BAVC_HDMI_PixelRepetition_##c, BAVC_HDMI_BitsPerPixel_##d, BVDC_P_TmdsClock_##e },
#define BVDC_P_MAKE_HDMILU__(pick1, pick2, pick3,  a, b, c, d, e) \
    BVDC_P_MAKE_HDMILU_##pick1##pick2##pick3(a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU_(pick1, pick2, pick3,  a, b, c, d, e) \
    BVDC_P_MAKE_HDMILU__(pick1, pick2, pick3,  a, b, c, d, e)
#define BVDC_P_MAKE_HDMILU(a, b, c, d, e)  \
    BVDC_P_MAKE_HDMILU_(BFMT_PICK_PXL_##a, BFMT_PICK_PR_##c, BFMT_PICK_BPP_##d, a, b, c, d, e)

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
