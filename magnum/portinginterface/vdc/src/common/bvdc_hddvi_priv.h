/***************************************************************************
 * Copyright (C) 2019 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BVDC_HDDVI_PRIV_H__
#define BVDC_HDDVI_PRIV_H__

#include "bvdc.h"
#include "bvdc_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_csc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Software reset BCHP_HD_DVI_0_SW_RESET_CNTRL */
#define BVDC_P_HDDVI_SW_RESET            (1)

/***************************************************************************
 * Private defines
 ***************************************************************************/
/* Trigger offset from the picture height. */
#define BVDC_P_HDDVI_TRIGGER_OFFSET                 (6)


/* Wait this amount of vsync to get stable pixel decimate value. */
#define BVDC_P_HDDVI_PIXEL_DECIMATE_COUNT           (3)

/* Print out status in case source isn't detect every n second. */
#define BVDC_P_HDDVI_STATUS_MSG_THRESHOLD           (3 * 60)

/* Use to detect vertical refresh rate! */
#define BVDC_P_HDDVI_VERTFREQ_TOLER                 (50)
#define BVDC_P_HDDVI_DOUBLECLOCK_VERTFREQ_TOLER     (150)

/* Use to detect H scanline */
#define BVDC_P_HDDVI_SCANLINE_TOLER                 (16)

/* Use to detect format ! */
#define BVDC_P_HDDVI_FORMAT_TOLER_WIDTH             (2)
#define BVDC_P_HDDVI_FORMAT_TOLER_HEIGHT            (3)
#define BVDC_P_HDDVI_FORMAT_TOLER_MAX_WIDTH         (5)
#define BVDC_P_HDDVI_FORMAT_TOLER_MAX_HEIGHT        (80)

/* PR52568: special 1090i, 725p */
#define BVDC_P_HDDVI_1090i_MIN_TOLER_HEIGHT         (10)
#define BVDC_P_HDDVI_725p_MIN_TOLER_WIDTH           (60)
#define BVDC_P_HDDVI_725p_MIN_TOLER_HEIGHT          (5)

/* Number of vsync to detect trigger swap */
#define BVDC_P_HDDVI_DETECT_TRIGGER_SWAP            (15)

#define BVDC_P_HDDVI_VIDEO_DETECT_COUNT             (25)

#define BVDC_P_HDDVI_PICTURE_GENERATE_COUNT         (5)

/* Threshold of lost video before going into auto format detection. */
#define BVDC_P_HDDVI_NEW_VER_0                      (0) /* 7038x-ish */
#define BVDC_P_HDDVI_NEW_VER_1                      (1) /* 3560Bx, 7400-ish */
#define BVDC_P_HDDVI_NEW_VER_2                      (2) /* 3563-Ax,Bx */
#define BVDC_P_HDDVI_NEW_VER_3                      (3) /* 3563-Cx */
#define BVDC_P_HDDVI_NEW_VER_4                      (4) /* 3548-Ax,Bx,Cx,  */
#define BVDC_P_HDDVI_NEW_VER_5                      (5) /* 7420Ax, mainly HW fixes */
#define BVDC_P_HDDVI_NEW_VER_6                      (6) /* 7420Bx/Cx, added hw mute */
#define BVDC_P_HDDVI_NEW_VER_7                      (7) /* 7422Ax/7425Ax, added sw_init */
#define BVDC_P_HDDVI_NEW_VER_8                      (8) /* 7425Bx/7231Bx, added I3D detection */
#define BVDC_P_HDDVI_NEW_VER_9                      (9) /* 7435Ax, 7366 */
#define BVDC_P_HDDVI_NEW_VER_10                     (10) /* 7445Cx, 7439Ax */
#define BVDC_P_HDDVI_NEW_VER_11                     (11) /* 7445Dx, 7364, 7250, 7439Bx */
#define BVDC_P_HDDVI_NEW_VER_12                     (12) /* 7586 */

/* PR27209: Need HD_DVI trigger workaround*/
#if (BVDC_P_SUPPORT_HDDVI_VER == BVDC_P_HDDVI_NEW_VER_2)
#define BVFD_P_HDDVI_TRIGGER_WORKAROUND             (1)
#endif

/* PR33847: Marantz DVD player field inversion*/
#if ((BVDC_P_SUPPORT_HDDVI_VER >= BVDC_P_HDDVI_NEW_VER_3) && \
     (BVDC_P_SUPPORT_HDDVI_VER <= BVDC_P_HDDVI_NEW_VER_4))
#define BVFD_P_HDDVI_FIELD_INVERSION_WORKAROUND     (1)
#endif

/***************************************************************************
 * Private macros
 ***************************************************************************/
/* Get the offset of two groups of register. */
#define BVDC_P_HDDVI_GET_REG_OFFSET(eHdDviId, ulPriReg, ulSecReg) \
    ((BVDC_P_HdDviId_eHdDvi0 == (eHdDviId)) ? 0 : ((ulSecReg) - (ulPriReg)))

#define BVDC_P_HDDVI_INPUT_RGB(eInputColorSpace) \
    (BVDC_P_HdDvi_InputColorSpace_eRGB == eInputColorSpace)

#define BVDC_P_HDDVI_INPUT_420(eInputColorSpace) \
    (BVDC_P_HdDvi_InputColorSpace_eYCbCr420 == eInputColorSpace)

#define BVDC_P_HDDVI_INPUT_444(eInputColorSpace) \
    (BVDC_P_HdDvi_InputColorSpace_eYCbCr444 == eInputColorSpace)

#define BVDC_P_HDDVI_INPUT_422(eInputColorSpace) \
    (BVDC_P_HdDvi_InputColorSpace_eYCbCr422 == eInputColorSpace)

#define BVDC_P_HDDVI_INPUT_422_DOUBLECLOCK(eInputColorSpace) \
    (BVDC_P_HdDvi_InputColorSpace_eYCbCr422_DoubleClock == eInputColorSpace)

/***************************************************************************
 * Private enums
 ***************************************************************************/
typedef enum
{
    BVDC_P_HdDviId_eHdDvi0 = 0,
    BVDC_P_HdDviId_eHdDvi1

} BVDC_P_HdDviId;

/* Input color space */
typedef enum
{
    BVDC_P_HdDvi_InputColorSpace_eRGB = 0,
    BVDC_P_HdDvi_InputColorSpace_eYCbCr444,
    BVDC_P_HdDvi_InputColorSpace_eYCbCr422,
    BVDC_P_HdDvi_InputColorSpace_eYCbCr422_DoubleClock,
    BVDC_P_HdDvi_InputColorSpace_eYCbCr420

} BVDC_P_HdDvi_InputColorSpace;


typedef struct
{
    bool                           bInterlaced;         /* Raster type */
    bool                           bPllDetected;        /* No vsync count saturated */
    uint32_t                       ulPxlFreq;           /* Pixel frequency */
    uint32_t                       ulVertFreq;          /* Vertical refresh rate */
    uint32_t                       ulClkPerVsync;       /* Sysclk per vsync */
    uint32_t                       ulNrmlClkPerVsync;   /* Normalized to BVDC_P_BVB_BUS_CLOCK */

    uint32_t                       ulVBlank;            /* Total Vert blanking */
    uint32_t                       ulHBlank;            /* Total Horz blanking */
    uint32_t                       ulAvWidth;           /* Active video width */
    uint32_t                       ulAvHeight;          /* Active video height */
    uint32_t                       ulVPolarity;         /* Vert polarity */
    uint32_t                       ulHPolarity;         /* Horz polarity */

    uint32_t                       ulHFrontPorch;       /* Horz front porch */
    uint32_t                       ulHBackPorch;        /* Horz back porch */
    uint32_t                       ulVFrontPorch;       /* Vert front porch */
    uint32_t                       ulVBackPorch;        /* Vert back porch */

    uint32_t                       ulBottomVFrontPorch; /* Bottom field Vert front porch */
    uint32_t                       ulBottomVBackPorch;  /* Bottom field Vert back porch */
    uint32_t                       ulBottomVBlank;      /* Total bottom field Vert blanking */
} BVDC_P_HdDviInput;

/* Internal De settings. */
typedef struct
{
    BFMT_VideoFmt                  eVideoFmt;
    uint32_t                       ulHorzDelay;
    uint32_t                       ulVertDelay;

} BVDC_P_HdDvi_DeConfig;

typedef struct BVDC_P_HdDviContext
{
    BDBG_OBJECT(BVDC_DVI)

    BREG_Handle                    hReg;
    BVDC_Source_Handle             hSource;

    BVDC_P_HdDviId                 eId;
    uint32_t                       ulOffset;

    /* Used by HD_DVI source to sync with XVD */
    BAVC_VDC_HdDvi_Picture         stXvdField;
    bool                           bVideoDetected;
    bool                           bResetFormatDetect;
    uint32_t                       ulClkPerVsyncDelta;
    /* Normalized to BVDC_P_BVB_BUS_CLOCK */
    uint32_t                       ulNrmlClkPerVsyncDelta;

#if (BVDC_P_HDDVI_SW_RESET)
    bool                           bSwReset;
#endif
    int32_t                        lPicGenCnt;

    /* Status detection */
    bool                           bBvbErr;
    bool                           bFifoErr;
    bool                           bPctrErr;
    bool                           bBridgeErr;
    bool                           bFormatUpdate;
    uint32_t                       ulStartFeedCnt;

    uint32_t                       ulBridgeErrRegAddr; /* Scratch reg to hold bridge errors */
    uint32_t                       ulPctrErrRegAddr;   /* Scratch reg to hold PCTR errors */
    uint32_t                       ulFormatUpdateRegAddr;   /* Scratch reg to hold format update status */

    uint32_t                       ulOutputMonitor0RegAddr;
    uint32_t                       ulOutputMonitor1RegAddr;
    uint32_t                       ulBVBOutputErr;
    uint32_t                       ulBVBOutputSize;

    uint32_t                       ulVsyncCnt;         /* Vsync counter */
    BVDC_P_HdDviInput              stStatus;           /* status read from HW that potentially changes per vsync */

    /* Misc.  Configuration. */
    BVDC_P_HdDvi_InputColorSpace   eInputColorSpace;

    bool                           b24BitsMode;
    uint32_t                       ulPixelDecimate;
    BVDC_P_CscCoeffs               stCsc;

    uint32_t                       ulPixelDecimateCnt;       /* Pixel Decimate counter */

    /* DE enable signal */
    uint32_t                       ulVertDelay;
    uint32_t                       ulHorzDelay;

    /* Swapped trigger. */
    bool                           bReverseTrigger;
    uint32_t                       ulDetectTrig;

    /* Tolerance on detection of vertical rate */
    uint32_t                       ulVertFreqTolerance;
    uint32_t                       ulScanWidthTolerance;

    /* 444 to 422 down sampler setting */
    BVDC_444To422DnSampler         stDnSampler;
    /* 422 to 444 up sampler setting */
    BVDC_422To444UpSampler         stUpSampler;

    /* Picture clean cfg regs. */
    uint32_t                       ulPicCleanUpCfg1;
    uint32_t                       ulPicCleanUpCfg5;

    uint32_t                       ulExtInputType;

    uint32_t                       ulSystemClock;
    bool                           bUpConversion;

} BVDC_P_HdDviContext;

/***************************************************************************
 * Private function prototypes
 ***************************************************************************/
BERR_Code BVDC_P_HdDvi_ValidateChanges
    ( BVDC_P_HdDvi_Handle              hHdDvi );

#if (BVDC_P_SUPPORT_HDDVI)
BERR_Code BVDC_P_HdDvi_Create
    ( BVDC_P_HdDvi_Handle             *phHdDvi,
      BVDC_P_HdDviId                   eHdDviId,
      BREG_Handle                      hReg,
      BVDC_Source_Handle               hSource );

void BVDC_P_HdDvi_Destroy
    ( BVDC_P_HdDvi_Handle              hHdDvi );

void BVDC_P_HdDvi_Init
    ( BVDC_P_HdDvi_Handle              hHdDvi );

void BVDC_P_HdDvi_GetStatus_isr
    ( const BVDC_P_HdDvi_Handle        hHdDvi,
      bool                            *pbVideoDetected );
void BVDC_P_HdDvi_GetInputStatus
    ( BVDC_P_HdDviContext             *pHdDvi,
      BVDC_Source_InputStatus         *pInputStatus );

void BVDC_P_HdDvi_Bringup_isr
    ( BVDC_P_HdDvi_Handle              hHdDvi );

void BVDC_P_HdDvi_DisableTriggers_isr
    ( BVDC_P_HdDvi_Handle              hHdDvi );

#else
#define BVDC_P_HdDvi_Create(phHdDvi, eHdDviId, hReg, hSource)    BDBG_ASSERT(0)
#define BVDC_P_HdDvi_Destroy(hHdDvi)                             BDBG_ASSERT(0)
#define BVDC_P_HdDvi_Init(hHdDvi)                                BDBG_ASSERT(0)
#define BVDC_P_HdDvi_GetStatus_isr(hHdDvi, pbVideoDetected)      BDBG_ASSERT(0)
#define BVDC_P_HdDvi_GetInputStatus(pHdDvi, pInputStatus)        BDBG_ASSERT(0)
#define BVDC_P_HdDvi_Bringup_isr(hHdDvi)                         BDBG_ASSERT(0)
#define BVDC_P_HdDvi_DisableTriggers_isr(hHdDvi)                 BDBG_ASSERT(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_HDDVI_PRIV_H__ */

/* End of file. */
