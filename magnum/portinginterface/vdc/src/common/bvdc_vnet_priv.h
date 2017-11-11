/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BVDC_VNET_PRIV_H__
#define BVDC_VNET_PRIV_H__

#include "bkni.h"
#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_resource_priv.h"
#include "bchp_vnet_f.h"
#include "bchp_vnet_b.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_RES);

/* VNET_F/VNET_B versioning! */
#define BVDC_P_VNET_VER_0                          (0) /* pre-3548 tapeout */
#define BVDC_P_VNET_VER_1                          (1) /* 3548x/3556x */
#define BVDC_P_VNET_VER_2                          (2) /* 7422x, 7425x */
#define BVDC_P_VNET_VER_3                          (3) /* 7358, 7552, 7231, 7346, 7344 */
#define BVDC_P_VNET_VER_4                          (4) /* 7364 */
#define BVDC_P_VNET_VER_5                          (5) /* 7445D, 7366B */

#if (BVDC_P_SUPPORT_DRAIN_VER == BVDC_P_VNET_VER_5)
/* HW7445-1520: VNET DRAIN EXP_PIC_XSIZE is half the real picture_width
 * when used in dual-pixel BVB Bus */
#define BVDC_P_DRAIN_PIC_XSIZE_DUAL_PIXEL_WORKAROUND   (1)
#endif

/* Check if this mode uses the capture. */
#define BVDC_P_VNET_USED_CAPTURE(stVnetMode) \
    ((stVnetMode).stBits.bUseCap)

/* Check if this mode uses the playback. */
#define BVDC_P_VNET_USED_PLAYBACK(stVnetMode) \
    ((stVnetMode).stBits.bUseCap)

/* Check if this mode uses the scaler. */
#define BVDC_P_VNET_USED_SCALER(stVnetMode) \
    ((stVnetMode).stBits.bUseScl)

/* Check if this mode uses the scaler. */
#define BVDC_P_VNET_USED_SCALER_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseScl) && \
     !(((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bSclBeforeCap)))

/* Check if this mode uses the scaler. */
#define BVDC_P_VNET_USED_SCALER_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseScl) && \
     ((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bSclBeforeCap))

/* Check if this mode uses the mcvp. */
#define BVDC_P_VNET_USED_MVP(stVnetMode) \
    ((stVnetMode).stBits.bUseMvp)

/* Check if this mode uses the mcvp at reader. */
#define BVDC_P_VNET_USED_MVP_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseMvp) && \
     !(((stVnetMode).stBits.bUseCap) && \
       (((stVnetMode).stBits.bSclBeforeCap) || ((stVnetMode).stBits.bSrcSideDeinterlace))))

/* Check if this mode uses the mcvp at writer. */
#define BVDC_P_VNET_USED_MVP_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseMvp) && \
     ((stVnetMode).stBits.bUseCap) && \
     ((stVnetMode).stBits.bSclBeforeCap || (stVnetMode).stBits.bSrcSideDeinterlace))


/* Check if this mode bypass the mcvp. */
#define BVDC_P_MVP_BYPASS_MVP(stMvpMode) \
    ((stMvpMode).stBits.bUseMvpBypass)

/* Check if this mode uses the HSCL (tied to MAD usage). */
#define BVDC_P_MVP_USED_HSCL(stMvpMode) \
    ((stMvpMode).stBits.bUseHscl)

/* Check if this mode uses the ANR. */
#define BVDC_P_MVP_USED_ANR(stMvpMode) \
    ((stMvpMode).stBits.bUseAnr)

/* Check if this mode uses the mcdi/madr. */
#define BVDC_P_MVP_USED_MAD(stMvpMode) \
    ((stMvpMode).stBits.bUseMad)

/* Check if this mode uses the mad32 at reader. */
#define BVDC_P_MVP_USED_MAD_AT_READER(stVnetMode, stMvpMode) \
    (((stMvpMode).stBits.bUseMad) && \
     !(((stVnetMode).stBits.bUseCap) && \
       (((stVnetMode).stBits.bSclBeforeCap) || ((stVnetMode).stBits.bSrcSideDeinterlace))))

/* Check if this mode uses the mad32 at writer. */
#define BVDC_P_MVP_USED_MAD_AT_WRITER(stVnetMode, stMvpMode) \
    (((stMvpMode).stBits.bUseMad) && \
     ((stVnetMode).stBits.bUseCap) && \
     ((stVnetMode).stBits.bSclBeforeCap || (stVnetMode).stBits.bSrcSideDeinterlace))


/* Check if this mode uses the XSRC (tied to 10-bit 4K usage). */
#define BVDC_P_VNET_USED_XSRC(stVnetMode) \
    ((stVnetMode).stBits.bUseXsrc)

/* Check if this mode uses the XSRC at reader. */
#define BVDC_P_VNET_USED_XSRC_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseXsrc) && \
     !((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the XSRC at writer. */
#define BVDC_P_VNET_USED_XSRC_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseXsrc) && \
     ((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the XSRC (tied to 10-bit 4K usage). */
#define BVDC_P_VNET_USED_VFC(stVnetMode) \
    ((stVnetMode).stBits.bUseVfc)

/* Check if this mode uses the XSRC at reader. */
#define BVDC_P_VNET_USED_VFC_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseVfc) && \
     !((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the XSRC at writer. */
#define BVDC_P_VNET_USED_VFC_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseVfc) && \
     ((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the TNTD. */
#define BVDC_P_VNET_USED_TNTD(stVnetMode) \
    ((stVnetMode).stBits.bUseTntd)

/* Check if this mode uses the TNTD at reader. */
#define BVDC_P_VNET_USED_TNTD_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseTntd) &&((stVnetMode).stBits.bUseScl) && \
     !(((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bSclBeforeCap)))

/* Check if this mode uses the TNTD at writer. */
#define BVDC_P_VNET_USED_TNTD_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseTntd) && ((stVnetMode).stBits.bUseScl) && \
     ((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bSclBeforeCap))

/* Check if this mode uses the DNR. */
#define BVDC_P_VNET_USED_DNR(stVnetMode) \
    ((stVnetMode).stBits.bUseDnr)

/* Check if this mode uses the DNR at reader. */
#define BVDC_P_VNET_USED_DNR_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseDnr) && \
     !((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the DNR at writer. */
#define BVDC_P_VNET_USED_DNR_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseDnr) && \
     ((stVnetMode).stBits.bUseCap))

/* Check if this mode uses the vnet crc. */
#define BVDC_P_VNET_USED_VNETCRC_AT_READER(stVnetMode) \
    (((stVnetMode).stBits.bUseVnetCrc) && \
     !(((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bVnetCrcBeforeCap)))

/* Check if this mode uses the vnet crc. */
#define BVDC_P_VNET_USED_VNETCRC_AT_WRITER(stVnetMode) \
    (((stVnetMode).stBits.bUseVnetCrc) && \
     ((stVnetMode).stBits.bUseCap) && ((stVnetMode).stBits.bVnetCrcBeforeCap))

/* Check if this mode uses the vnet crc. */
#define BVDC_P_VNET_USED_VNETCRC(stVnetMode) \
    ((stVnetMode).stBits.bUseVnetCrc)

/* Check if this mode is invalid */
#define BVDC_P_VNET_IS_INVALID(stVnetMode) \
    ((stVnetMode).stBits.bInvalid)

/* Make things look cleaner */
#define BVDC_P_VNET_F(vnet_f_enum) \
    BCHP_VNET_F_SCL_0_SRC_SOURCE_##vnet_f_enum
#define BVDC_P_VNET_B(vnet_b_enum) \
    BCHP_VNET_B_CAP_0_SRC_SOURCE_##vnet_b_enum

#define BVDC_P_DRN_INVALID_OFFSET                   UINT32_C(-1)
#define BVDC_P_DRN_HAS_DEBUG(pDrain) \
    (BVDC_P_DRN_INVALID_OFFSET != pDrain->ulDbgOffset)

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_2
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_2 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_3
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_3 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_4
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_4 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_5
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_5 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_6
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_6 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_7
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_7 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_HD_DVI_0
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_HD_DVI_0 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_HD_DVI_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_HD_DVI_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif

/* If there isn't _1 make it equal to _0 */
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_0
#endif

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_2
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_2 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_3
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_3 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_4
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_4 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_5
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_5 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_6
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_6 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_7
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_7 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_8
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_8 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_9
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_9 0
#endif

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_1 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_2
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_2 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_3
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_3 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_4
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_4 0
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_5
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_5 0
#endif

/* Make the 0 equal to _0 */
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_0
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_0 BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656
#endif

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_0
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_0 BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC
#endif
#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_1
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC_1 BCHP_VNET_F_SCL_0_SRC_SOURCE_VDEC
#endif

#ifndef BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0
#define BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback_0 BCHP_VNET_F_SCL_0_SRC_SOURCE_Loopback
#endif
#ifndef BCHP_VNET_B_LOOPBACK_SRC
#define BCHP_VNET_B_LOOPBACK_SRC BCHP_VNET_B_LOOPBACK_0_SRC
#endif

#ifndef BCHP_VNET_F_MAD_0_SRC
#define BCHP_VNET_F_MAD_0_SRC BCHP_VNET_F_MAD32_SRC
#endif

#ifndef BCHP_VNET_F_MAD_0_SRC_SOURCE_Output_Disabled
#define BCHP_VNET_F_MAD_0_SRC_SOURCE_MASK  BCHP_VNET_F_MAD32_SRC_SOURCE_MASK
#define BCHP_VNET_F_MAD_0_SRC_SOURCE_SHIFT BCHP_VNET_F_MAD32_SRC_SOURCE_SHIFT
#define BCHP_VNET_F_MAD_0_SRC_SOURCE_Output_Disabled BCHP_VNET_F_MAD32_SRC_SOURCE_Output_Disabled
#endif

#ifndef BCHP_VNET_F_DNR_0_SRC
#define BCHP_VNET_F_DNR_0_SRC BCHP_VNET_F_DNR_SRC
#endif

#ifndef BCHP_VNET_F_DNR_0_SRC_SOURCE_Output_Disabled
#define BCHP_VNET_F_DNR_0_SRC_SOURCE_MASK  BCHP_VNET_F_DNR_SRC_SOURCE_MASK
#define BCHP_VNET_F_DNR_0_SRC_SOURCE_SHIFT BCHP_VNET_F_DNR_SRC_SOURCE_SHIFT
#define BCHP_VNET_F_DNR_0_SRC_SOURCE_Output_Disabled BCHP_VNET_F_DNR_SRC_SOURCE_Output_Disabled
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD32
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD32 0
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_0 BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD32
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_1
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_1 0
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_0 BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_1
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_1 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_2
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_2 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_3
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_3 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_4
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_DNR_4 0
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_ANR_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_ANR_0 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_ANR_1
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_ANR_1 0
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_1
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_1 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_2
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_2 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_3
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_3 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_4
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_4 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_5
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_5 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_6
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_6 0
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_7
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Scaler_7 0
#endif

#ifndef BCHP_VNET_B_DRAIN_0_SRC_SOURCE_MAD_0
#define BCHP_VNET_B_DRAIN_0_SRC_SOURCE_MAD_0 BCHP_VNET_B_DRAIN_0_SRC_SOURCE_MAD32
#endif

#ifndef BCHP_VNET_B_DRAIN_0_SRC_SOURCE_DNR_0
#define BCHP_VNET_B_DRAIN_0_SRC_SOURCE_DNR_0 BCHP_VNET_B_DRAIN_0_SRC_SOURCE_DNR
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_FGT_0
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_FGT_0 0
#endif

#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_2
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_2 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_3
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_3 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_4
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_4 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_5
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_5 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_6
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_6 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_7
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_7 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_8
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_8 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_9
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_9 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_10
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_10 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_11
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_11 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_12
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_12 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif
#ifndef BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_13
#define BCHP_VNET_B_CAP_0_SRC_SOURCE_Free_Ch_13 BCHP_VNET_B_CAP_0_SRC_SOURCE_Output_Disabled
#endif

/* Enumeration that defines the front and back muxes. */
typedef enum
{
    BVDC_P_VnetF_eMpeg_0    = BVDC_P_VNET_F(MPEG_Feeder_0),
    BVDC_P_VnetF_eMpeg_1    = BVDC_P_VNET_F(MPEG_Feeder_1),
    BVDC_P_VnetF_eMpeg_2    = BVDC_P_VNET_F(MPEG_Feeder_2),
    BVDC_P_VnetF_eMpeg_3    = BVDC_P_VNET_F(MPEG_Feeder_3),
    BVDC_P_VnetF_eMpeg_4    = BVDC_P_VNET_F(MPEG_Feeder_4),
    BVDC_P_VnetF_eMpeg_5    = BVDC_P_VNET_F(MPEG_Feeder_5),
    BVDC_P_VnetF_eVideo_0   = BVDC_P_VNET_F(Video_Feeder_0),
    BVDC_P_VnetF_eVideo_1   = BVDC_P_VNET_F(Video_Feeder_1),
    BVDC_P_VnetF_eVideo_2   = BVDC_P_VNET_F(Video_Feeder_2),
    BVDC_P_VnetF_eVideo_3   = BVDC_P_VNET_F(Video_Feeder_3),
    BVDC_P_VnetF_eVideo_4   = BVDC_P_VNET_F(Video_Feeder_4),
    BVDC_P_VnetF_eVideo_5   = BVDC_P_VNET_F(Video_Feeder_5),
    BVDC_P_VnetF_eVideo_6   = BVDC_P_VNET_F(Video_Feeder_6),
    BVDC_P_VnetF_eVideo_7   = BVDC_P_VNET_F(Video_Feeder_7),
    BVDC_P_VnetF_eHdDvi_0   = BVDC_P_VNET_F(HD_DVI_0),
    BVDC_P_VnetF_eHdDvi_1   = BVDC_P_VNET_F(HD_DVI_1),
    BVDC_P_VnetF_eCcir656_0 = BVDC_P_VNET_F(CCIR_656_0),
    BVDC_P_VnetF_eCcir656_1 = BVDC_P_VNET_F(CCIR_656_1),
    BVDC_P_VnetF_eAnalog_0  = BVDC_P_VNET_F(VDEC_0),
    BVDC_P_VnetF_eAnalog_1  = BVDC_P_VNET_F(VDEC_1),
    BVDC_P_VnetF_eLoopback_0  = BVDC_P_VNET_F(Loopback_0),
    BVDC_P_VnetF_eLoopback_1  = BVDC_P_VNET_F(Loopback_1),
    BVDC_P_VnetF_eLoopback_2  = BVDC_P_VNET_F(Loopback_2),
    BVDC_P_VnetF_eLoopback_3  = BVDC_P_VNET_F(Loopback_3),
    BVDC_P_VnetF_eLoopback_4  = BVDC_P_VNET_F(Loopback_4),
    BVDC_P_VnetF_eLoopback_5  = BVDC_P_VNET_F(Loopback_5),
    BVDC_P_VnetF_eLoopback_6  = BVDC_P_VNET_F(Loopback_6),
    BVDC_P_VnetF_eLoopback_7  = BVDC_P_VNET_F(Loopback_7),
    BVDC_P_VnetF_eLoopback_8  = BVDC_P_VNET_F(Loopback_8),
    BVDC_P_VnetF_eLoopback_9  = BVDC_P_VNET_F(Loopback_9),
    BVDC_P_VnetF_eDisabled  = BVDC_P_VNET_F(Output_Disabled),
    BVDC_P_VnetF_eInvalid   = -1
} BVDC_P_VnetF;

typedef enum
{
    BVDC_P_VnetB_eScaler_0  = BVDC_P_VNET_B(Scaler_0),
    BVDC_P_VnetB_eScaler_1  = BVDC_P_VNET_B(Scaler_1),
    BVDC_P_VnetB_eScaler_2  = BVDC_P_VNET_B(Scaler_2),
    BVDC_P_VnetB_eScaler_3  = BVDC_P_VNET_B(Scaler_3),
    BVDC_P_VnetB_eScaler_4  = BVDC_P_VNET_B(Scaler_4),
    BVDC_P_VnetB_eScaler_5  = BVDC_P_VNET_B(Scaler_5),
    BVDC_P_VnetB_eScaler_6  = BVDC_P_VNET_B(Scaler_6),
    BVDC_P_VnetB_eScaler_7  = BVDC_P_VNET_B(Scaler_7),
    BVDC_P_VnetB_eMad32_0   = BVDC_P_VNET_B(MAD_0),
    BVDC_P_VnetB_eMad32_1   = BVDC_P_VNET_B(MAD_1),
    BVDC_P_VnetB_eChannel_0 = BVDC_P_VNET_B(Free_Ch_0),
    BVDC_P_VnetB_eChannel_1 = BVDC_P_VNET_B(Free_Ch_1),
    BVDC_P_VnetB_eChannel_2 = BVDC_P_VNET_B(Free_Ch_2),
    BVDC_P_VnetB_eChannel_3 = BVDC_P_VNET_B(Free_Ch_3),
    BVDC_P_VnetB_eChannel_4 = BVDC_P_VNET_B(Free_Ch_4),
    BVDC_P_VnetB_eChannel_5 = BVDC_P_VNET_B(Free_Ch_5),
    BVDC_P_VnetB_eChannel_6 = BVDC_P_VNET_B(Free_Ch_6),
    BVDC_P_VnetB_eChannel_7 = BVDC_P_VNET_B(Free_Ch_7),
    BVDC_P_VnetB_eChannel_8 = BVDC_P_VNET_B(Free_Ch_8),
    BVDC_P_VnetB_eChannel_9 = BVDC_P_VNET_B(Free_Ch_9),
    BVDC_P_VnetB_eChannel_10 = BVDC_P_VNET_B(Free_Ch_10),
    BVDC_P_VnetB_eChannel_11 = BVDC_P_VNET_B(Free_Ch_11),
    BVDC_P_VnetB_eChannel_12 = BVDC_P_VNET_B(Free_Ch_12),
    BVDC_P_VnetB_eChannel_13 = BVDC_P_VNET_B(Free_Ch_13),
    BVDC_P_VnetB_eDnr_0     = BVDC_P_VNET_B(DNR_0),
    BVDC_P_VnetB_eDnr_1     = BVDC_P_VNET_B(DNR_1),
    BVDC_P_VnetB_eDnr_2     = BVDC_P_VNET_B(DNR_2),
    BVDC_P_VnetB_eDnr_3     = BVDC_P_VNET_B(DNR_3),
    BVDC_P_VnetB_eDnr_4     = BVDC_P_VNET_B(DNR_4),
    BVDC_P_VnetB_eAnr_0     = BVDC_P_VNET_B(ANR_0),
    BVDC_P_VnetB_eAnr_1     = BVDC_P_VNET_B(ANR_1),
    BVDC_P_VnetB_eFgt_0     = BVDC_P_VNET_B(FGT_0),
    BVDC_P_VnetB_eDisabled  = BVDC_P_VNET_B(Output_Disabled),
    BVDC_P_VnetB_eInvalid   = -1
} BVDC_P_VnetB;

/* Vnet drain configuration */
typedef struct
{
    /* VNET_F */
    BVDC_P_ResourceType       eVnetFDrainType;    /* vnet_f drain type */
    uint32_t                  ulVnetFDrainId;     /* vnet_f drain id */
    BVDC_P_VnetF              eVnetFDrainSrc;     /* vnet_f src */

    /* VNET_B */
    BVDC_P_ResourceType       eVnetBDrainType;    /* vnet_b drain type */
    uint32_t                  ulVnetBDrainId;     /* vnet_b drain id */
    BVDC_P_VnetB              eVnetBDrainSrc;     /* vnet_b src */

    /* Drain Debug */
    uint32_t                  ulDbgOffset;        /* offset from BCHP_VNET_F_DRAIN_0_PIC_SIZE_CNT */

} BVDC_P_DrainContext;


/***************************************************************************
 * VNET Front and back context.
 ***************************************************************************/
typedef struct BVDC_P_VnetContext
{
    BDBG_OBJECT(BVDC_VNT)

    BREG_Handle                        hRegister;

} BVDC_P_VnetContext;


/***************************************************************************
 * Build RUL for VNET front & back
 ***************************************************************************/
BERR_Code BVDC_P_Drain_Acquire
    ( BVDC_P_DrainContext             *pDrain,
      BVDC_P_ResourceContext          *pResource,
      BAVC_SourceId                    eSourceId );

void BVDC_P_Drain_Release
    ( const BVDC_P_DrainContext       *pDrain,
      BVDC_P_ResourceContext          *pResource );

void BVDC_P_Drain_BuildRul_isr
    ( const BVDC_P_DrainContext       *pDrain,
      BVDC_P_ListInfo                 *pList );

void BVDC_P_Drain_BuildFormatRul_isr
    ( const BVDC_P_DrainContext       *pDrain,
      const BVDC_P_Rect               *pScanOut,
      const BFMT_VideoInfo            *pFmtInfo,
      BVDC_P_ListInfo                 *pList );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_VNET_PRIV_H__ */

/* End of file. */
