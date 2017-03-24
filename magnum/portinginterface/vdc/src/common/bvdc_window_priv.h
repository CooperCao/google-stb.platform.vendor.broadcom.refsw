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
#ifndef BVDC_WINDOW_PRIV_H__
#define BVDC_WINDOW_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_buffer_priv.h"
#include "bmth_fix.h"
#include "bvdc_pep_priv.h"
#include "bvdc_tnt_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_csc_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_cfc_priv.h"

#if BVDC_P_SUPPORT_SEC_CMP
#include "bchp_cmp_1.h"
#endif

#if BVDC_P_SUPPORT_TER_CMP
#include "bchp_cmp_2.h"
#endif

#if BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT
#include "bchp_cmp_3.h"
#endif

#if BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT
#include "bchp_cmp_4.h"
#endif

#if BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT
#include "bchp_cmp_5.h"
#endif

#if BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT
#include "bchp_cmp_6.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_BFH);

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
/* Gets the offset of a register from the start of the window */
#define BVDC_P_WIN_GET_REG_WIN_OFFSET(reg) \
    (BCHP##_##reg - BCHP_CMP_0_V0_SURFACE_SIZE)

#define BVDC_P_WIN_GET_REG_IDX(reg) \
    ((BVDC_P_WIN_GET_REG_WIN_OFFSET(reg) + hWindow->ulRegOffset) / \
    sizeof(uint32_t))

/* All of the video & graphics windows control registers are part of a
 * compositor. */

/* Get/Set reg data */
#define BVDC_P_WIN_GET_REG_DATA(reg) \
    (hWindow->hCompositor->aulRegs[BVDC_P_WIN_GET_REG_IDX(reg)])
#define BVDC_P_WIN_SET_REG_DATA(reg, data) \
    (BVDC_P_WIN_GET_REG_DATA(reg) = (uint32_t)(data))

#define BVDC_P_WIN_GET_REG_DATA_I(idx, reg) \
    (hWindow->hCompositor->aulRegs[BVDC_P_WIN_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_WIN_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_WIN_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_WIN_COMPARE_FIELD_DATA(reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_WIN_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_WIN_COMPARE_FIELD_NAME(reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_WIN_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_WIN_WRITE_TO_RUL(reg, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BVDC_P_WIN_GET_REG_WIN_OFFSET(reg) + hWindow->ulRegOffset + \
                  BCHP_CMP_0_REVISION + hCompositor->ulRegOffset); \
    *addr_ptr++ = BVDC_P_WIN_GET_REG_DATA(reg); \
}

#define BVDC_P_WIN_WRITE_IMM_TO_RUL(reg, imm, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BVDC_P_WIN_GET_REG_WIN_OFFSET(reg) + hWindow->ulRegOffset + \
                  BCHP_CMP_0_REVISION + hCompositor->ulRegOffset); \
    *addr_ptr++ = (imm); \
}

/* short hand */
#define BVDC_P_WIN_IS_V0(window_id) \
    ((BVDC_P_WindowId_eComp0_V0==window_id) || \
     (BVDC_P_WindowId_eComp1_V0==window_id) || \
     (BVDC_P_WindowId_eComp2_V0==window_id) || \
     (BVDC_P_WindowId_eComp3_V0==window_id) || \
     (BVDC_P_WindowId_eComp4_V0==window_id) || \
     (BVDC_P_WindowId_eComp5_V0==window_id) || \
     (BVDC_P_WindowId_eComp6_V0==window_id))

#define BVDC_P_WIN_IS_V1(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==window_id) || \
     (BVDC_P_WindowId_eComp1_V1==window_id))

#define BVDC_P_WIN_IS_VIDEO_WINDOW(window_id)  \
    (BVDC_P_WIN_IS_V0(window_id) || \
     BVDC_P_WIN_IS_V1(window_id))

#define BVDC_P_WIN_IS_GFX_WINDOW(window_id) \
    ((BVDC_P_WindowId_eComp0_G0==window_id) || \
     (BVDC_P_WindowId_eComp0_G1==window_id) || \
     (BVDC_P_WindowId_eComp0_G2==window_id) || \
     (BVDC_P_WindowId_eComp1_G0==window_id) || \
     (BVDC_P_WindowId_eComp2_G0==window_id) || \
     (BVDC_P_WindowId_eComp3_G0==window_id) || \
     (BVDC_P_WindowId_eComp4_G0==window_id) || \
     (BVDC_P_WindowId_eComp5_G0==window_id) || \
     (BVDC_P_WindowId_eComp6_G0==window_id))

#define BVDC_P_WIN_GET_GFX_WINDOW_INDEX(window_id)   \
    ((BVDC_P_WindowId_eComp0_G0==(window_id)) ? 2    \
    :(BVDC_P_WindowId_eComp0_G1==(window_id)) ? 3    \
    :(BVDC_P_WindowId_eComp0_G2==(window_id)) ? 4    \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? 2    \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? 2    \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? 2    \
    :(BVDC_P_WindowId_eComp4_G0==(window_id)) ? 2    \
    :(BVDC_P_WindowId_eComp5_G0==(window_id)) ? 2    \
    : 2)

#define BVDC_P_WIN_MOSAIC_CSC_IDX_INVALID          BVDC_P_CMP_0_MOSAIC_CFCS
#define BVDC_P_WIN_MOSAIC_CSC_MAP_SIZE             BVDC_P_MatrixCoefficients_eMax

/* Bandwidth equation decision oscillations! */
#define BVDC_P_BW_RATE_FACTOR                    (1000)
#define BVDC_P_BW_BASE                           (1000)
#define BVDC_P_BW_DEFAULT_DELTA                  (10)      /* 1% */
#define BVDC_P_BW_FIXED_BIAS_DELTA               (1000000) /* for forced SCL-CAP bias */

/* Scaler output min. */
#define BVDC_P_WIN_SCL_OUTPUT_H_MIN           (8)
#define BVDC_P_WIN_SCL_OUTPUT_V_MIN           (1)

#define BVDC_P_WIN_VFD_OUTPUT_H_MIN          (16)
#define BVDC_P_WIN_VFD_OUTPUT_V_MIN           (2)
#define BVDC_P_WIN_CAP_INPUT_H_MIN           (16)
#define BVDC_P_WIN_CAP_INPUT_V_MIN            (1)

#if (BVDC_P_DCXM_MIN_HSIZE_WORKAROUND)
/* CRBVN-486: Mosiac compression mode does not support H size smaller than 28 */
#define BVDC_P_WIN_CAP_MOSAIC_INPUT_H_MIN    (28)
#define BVDC_P_WIN_VFD_MOSAIC_OUTPUT_H_MIN   (28)
#else
/* CRBVN-379: Mosiac compression mode doesn't support H size smaller than 16*/
#define BVDC_P_WIN_CAP_MOSAIC_INPUT_H_MIN    (16)
#define BVDC_P_WIN_VFD_MOSAIC_OUTPUT_H_MIN   (16)
#endif
#define BVDC_P_WIN_CAP_MOSAIC_INPUT_V_MIN    (5)
#define BVDC_P_WIN_VFD_MOSAIC_OUTPUT_V_MIN   (5)

#define BVDC_P_WIN_MAD_INPUT_H_MIN           (64)
#define BVDC_P_WIN_MAD_INPUT_V_MIN            (8)
#define BVDC_P_WIN_ANR_INPUT_H_MIN           (64)
#define BVDC_P_WIN_ANR_INPUT_V_MIN            (8)
#define BVDC_P_WIN_DNR_INPUT_H_MIN           (32)
#define BVDC_P_WIN_DNR_INPUT_V_MIN           (16)

/* SW7439-400
minimum picture size for VMXVM is 72 pixels
(i.e. 8x9, 4x18, 2x36 are okay)*/
#define BVDC_P_WIN_DST_OUTPUT_H_MIN          (16)
#if BVDC_P_DCXM_72PIXELS
#define BVDC_P_WIN_DST_OUTPUT_V_MIN          (5)
#else
#define BVDC_P_WIN_DST_OUTPUT_V_MIN          (1)
#endif

/* Mosaic mode min */
#define BVDC_P_WIN_MOSAIC_OUTPUT_H_MIN        (16)
#define BVDC_P_WIN_MOSAIC_OUTPUT_V_MIN        (6)

/* This is used in the assymmetric bandwidth equation */
#define BVDC_P_DIV_CEIL(x, y)   ((x + y - 1) / y)


#define BVDC_P_TestFeature1_FRACTIONAL_SHIFT  (1)
#define BVDC_P_TestFeature1_THRESHOLD         \
    ((BVDC_P_72_SYSCLK / 1000000) * BFMT_FREQ_FACTOR)

/* Non-linear scaling central region ratio */
#define BVDC_P_CENTRAL_REGION_WEIGHT_PERCENTAGE  (50)

#define BVDC_P_USE_BMTH_FIX                      (0)

/* Mad delay buffer count, >3  and 2 power*/
#define BVDC_MADDELAY_BUF_COUNT                  (4)

/* New pitch setting for mosaic */
#define BVDC_P_BUFFER_ADD_GUARD_MEMORY  (BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH)

typedef enum
{
    BVDC_P_BufHeapType_eCapture = 0,
    BVDC_P_BufHeapType_eMad_Pixel,
    BVDC_P_BufHeapType_eMad_QM,
    BVDC_P_BufHeapType_eAnr

} BVDC_P_BufHeapType;

typedef enum
{
    /* Only need to allocate buffer for left */
    BVDC_P_BufHeapAllocMode_eLeftOnly = 0,
    /* Allocate 1 buffer for both left and right */
    BVDC_P_BufHeapAllocMode_eLRCombined,
    /* Allocate 2 buffers, 1 for left, 1 for right */
    BVDC_P_BufHeapAllocMode_eLRSeparate

} BVDC_P_BufHeapAllocMode;


/* Use for dynamic RTS. */
typedef struct
{
    bool bRequireCapture;
    bool bRequirePlayback;
    bool bRequireScaler;
    bool bRequireMad32;
    bool bRequirePep;
    BVDC_P_CaptureId eCapture;
    BVDC_P_FeederId  ePlayback;
    BVDC_P_ScalerId  eScaler;
    BVDC_P_MadId     eMad32;
    BVDC_P_WindowId  eWinId;

} BVDC_P_ResourceRequire;

/* Use for acquiring shared resouces Scaler/Capture/Playback/.. for a window. */
typedef struct
{
    uint32_t  ulCap;
    uint32_t  ulVfd;
    uint32_t  ulScl;
    uint32_t  ulMad;
    uint32_t  ulAnr;
} BVDC_P_ResourceFeature;

/* Active_Format Description */
typedef struct
{
    uint32_t                           ulAfd;
    uint32_t                           aulHClip[3];
    uint32_t                           aulVClip[3];
    const char                        *pchDesc;

} BVDC_P_Afd;

#define BVDC_P_MAKE_AFD(ulAfd, ulHClip0, ulVClip0, ulHClip1, ulVClip1, ulHClip2, ulVClip2, pchDesc) \
{                                                                   \
    (ulAfd),                                                        \
    {(ulHClip0),(ulHClip1), (ulHClip2)},                            \
    {(ulVClip0),(ulVClip1), (ulVClip2)},                            \
    BDBG_STRING(pchDesc)                                            \
}

/* Bar Data Structure */
#define BVDC_P_WINDOW_ISBARDATA(ulAfd, eBarDataType) \
    (((ulAfd == 0) || (ulAfd == 4)) &&  \
    (eBarDataType != BAVC_BarDataType_eInvalid))


/****************************************************************************
 * Window dirty bits for building RUL.  New and other flags should be moving
 * as needed.
 *
 */
typedef union
{
    struct
    {
    /* Most important dirty bits. */
    uint32_t                 bRecAdjust               : 1;  /* 0 */
    uint32_t                 bReDetVnet               : 1;

    /* Color space conversion for Contrast, Hue, Brightness, and Saturation. */
    /* and/or user customize CSC */
    uint32_t                 bCscAdjust               : 1;

    /* Pep: Tnt, Cab, Lab */
    uint32_t                 bTntAdjust               : 1;
    uint32_t                 bLabAdjust               : 1;  /* 4 */
    uint32_t                 bCabAdjust               : 1;
    uint32_t                 bDitAdjust               : 1;

    /* Color Key */
    uint32_t                 bColorKeyAdjust          : 1;

    /* Dirty bits for destroy and cleanup windows, bShutdown must be taken
     * care first before handling bDestroy, bSrcPending, or bReConfigVnet. */
    uint32_t                 bShutdown                : 1;  /* 8 */
    uint32_t                 bDestroy                 : 1;
    uint32_t                 bSrcPending              : 1;
    uint32_t                 bReConfigVnet            : 1;

    /* Mad is dirty. */
    uint32_t                 bDeinterlace             : 1;  /* 12 */

    /* Anr is dirty. */
    uint32_t                 bAnrAdjust               : 1;

    /* shared SCL is dirty */
    uint32_t                 bSharedScl               : 1;

    /* User capture */
    uint32_t                 bUserCaptureBuffer       : 1;
    uint32_t                 bUserReleaseBuffer       : 1;  /* 16 */

    /* mosaic mode */
    uint32_t                 bMosaicMode              : 1;

    /* dynamic buffer allocation */
    uint32_t                 bReallocBuffers          : 1;

    /* box detect */
    uint32_t                 bBoxDetect               : 1;

    /* Histogram rect */
    uint32_t                 bHistoRect               : 1;  /* 20 */

    /* 3D Stereoscopic Video/Graphics */
    uint32_t                 b3D                      : 1;


    /* General user change that need flag to copy */
    uint32_t                 bMiscCtrl                : 1;

    /* Coefficient index */
    uint32_t                 bCtIndex                 : 1;

    uint32_t                 bVnetCrc                 : 1;  /* 24 */

    uint32_t                 bBufAllocMode            : 1;
    uint32_t                 bStg                     : 1; /* output to STG/ViCE triggerred */

    /* pending for out of capture buffer memory */
    uint32_t                 bBufferPending           : 1;

#if BVDC_P_SUPPORT_TNTD
    uint32_t                 bTntd                    : 1;  /* 28 */
#endif
    } stBits;

    uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_Window_DirtyBits;

/***************************************************************************
Summary:
    This structure describes MAD configurations.

Description:

    bReverse32Pulldown - turn on/off 3:2 reverse pulldown;
    bReverse22Pulldown - turn on/off 2:2 reverse pulldown;

    maybe more config ...

See Also:
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/

typedef struct
{
    BVDC_MadGameMode                   eGameMode;

    bool                               bReverse32Pulldown;
    BVDC_Deinterlace_Reverse32Settings stReverse32Settings;

    bool                               bReverse22Pulldown;
    BVDC_Deinterlace_Reverse22Settings stReverse22Settings;

    BVDC_Deinterlace_ChromaSettings    stChromaSettings;

    BVDC_Deinterlace_MotionSettings    stMotionSettings;

    /* down/up sampling. */
    BVDC_422To444UpSampler             stUpSampler;
    BVDC_444To422DnSampler             stDnSampler;

    /* low angles */
    BVDC_Deinterlace_LowAngleSettings  stLowAngles;

    BPXL_Format                        ePixelFmt;

    bool                               bShrinkWidth;
    BVDC_Mode                          ePqEnhancement;

    /* might add other modes */
} BVDC_P_Deinterlace_Settings;


/***************************************************************************
Summary:
    Window information structure.

Description:
    This is a window public structure that holds the all the input
    parameters.

***************************************************************************/
typedef struct
{
    /* Display window attributes */
    bool                          bVisible;             /* Window visibility */
    bool                          bDeinterlace;         /* Enable de-interlacing */
    bool                          bForceCapture;        /* Enable force capture */
    uint8_t                       ucZOrder;             /* Depth ordering */
    uint8_t                       ucAlpha;              /* Window alpha value of window */
    uint8_t                       ucConstantAlpha;      /* Constant alpha blending */
    BVDC_BlendFactor              eFrontBlendFactor;    /* front blending factor */
    BVDC_BlendFactor              eBackBlendFactor;     /* back blending factor. */
    BVDC_AspectRatioMode          eAspectRatioMode;     /* Aspect Ratio Correction Mode */
    uint32_t                      ulNonlinearSrcWidth;  /* for nonlinear scale mode */
    uint32_t                      ulNonlinearSclOutWidth;/* for nonlinear scale mode */
    bool                          bUseSrcFrameRate;     /* Use source framerate to drive display. */
    bool                          bCscRgbMatching;      /* color space conversion with RGB matching */
    unsigned int                  uiVsyncDelayOffset;   /* offset added to default */

    BPXL_Format                   ePixelFormat;
    BVDC_P_Deinterlace_Settings   stMadSettings;
    bool                          bRev32Custom;
    bool                          bRev22Custom;
    bool                          bChromaCustom;
    bool                          bMotionCustom;

    /* For user capture */
    /* number of capture buffers added to running total of needed buffers */
    unsigned int                  uiCaptureBufCnt;
    BVDC_P_PictureNode           *pBufferFromUser;

    /* Source cissors clipping rectangle (in pixel unit) */
    BVDC_P_ClipRect               stSrcClip;

    /* Scaler output rectangle.(in pixel unit)
     * lLeft, lTop: specify where the stDstRect on scaler output rectangle.
     * ulWidth, ulHeight: specify the dimension of scaler output. */
    BVDC_P_Rect                   stScalerOutput;

    /* Window display size on canvas.(in pixel unit)
     * lLeft, lTop: specify where on canvas.
     * ulWidth, ulHeight: specify the dimension of window. */
    BVDC_P_Rect                   stDstRect;

    /* X-offset for the right window in 3D mode */
    int32_t                       lRWinXOffsetDelta;

    /* User AFD settings */
    BVDC_AfdSettings              stAfdSettings;

    /* User pan scan settings */
    BVDC_PanScanType              ePanScanType;         /* Pan scan type */
    int32_t                       lUserHorzPanScan;     /* User horizontal pan scan */
    int32_t                       lUserVertPanScan;     /* User vertical pan scan */

    /* scale factor rounding tolerance */
    uint32_t                      ulHrzSclFctRndToler;
    uint32_t                      ulVrtSclFctRndToler;

    /* box detect */
    bool                          bBoxDetect;
    bool                          bAutoCutBlack;
    BVDC_Window_BoxDetectCallback_isr BoxDetectCallBack;
    void *                        pvBoxDetectParm1;
    int                           iBoxDetectParm2;

    /* What source is this window use. */
    BVDC_Source_Handle            hSource;

    /* The following is for sharpness */
    bool                          bSharpnessEnable;
    int16_t                       sSharpness;
    bool                          bUserSharpnessConfig;
    BVDC_SharpnessSettings        stSharpnessConfig;

#if (BVDC_P_SUPPORT_TNT_VER == 5)            /* TNT2 HW base */
    BVDC_P_SharpnessData          stSharpnessPrivData;
#else
    uint32_t                      ulLumaGain;
    uint32_t                      ulSharpnessPeakSetting;
    uint32_t                      ulSharpnessPeakScale;
#endif

    /* These attributes are used for contrast stretch */
    bool                          bContrastStretch;
    BVDC_ContrastStretch          stContrastStretch;
    /* These attributes are used for blue stretch */
    bool                          bBlueStretch;
    BVDC_BlueStretch              stBlueStretch;
    bool                          bUserLabLuma;
    bool                          bUserLabCbCr;
    uint32_t                     *pulLabCbTbl;
    uint32_t                     *pulLabCrTbl;
    uint32_t                     *pulLabLumaTbl;
    /* These are the CAB parameters */
    bool                          bUserCabEnable;
    uint32_t                      ulFleshtone;
    uint32_t                      ulGreenBoost;
    uint32_t                      ulBlueBoost;
    uint32_t                     *pulCabTable;
    BVDC_ColorBar                 stSatGain;
    BVDC_ColorBar                 stHueGain;

    /* This is used for demo mode */
    BVDC_Window_SplitScreenSettings stSplitScreenSetting;

    /* User CSC */
    bool                           bUserCsc;
    uint32_t                       ulUserShift;
    int32_t                        pl32_Matrix[BVDC_CSC_COEFF_COUNT];

    /* State for shutting down window to transfer resource. */
    BVDC_P_State                  eReaderState;
    BVDC_P_State                  eWriterState;

    /* Generic Callback */
    BVDC_CallbackFunc_isr         pfGenCallback;
    void                         *pvGenCallbackParm1;
    int                           iGenCallbackParm2;

    /* PEP: Dithering */
    BVDC_DitherSettings           stDither;

    /* Color adjustment attributes */
    int16_t                       sColorTemp;
    int16_t                       sContrast;
    int16_t                       sSaturation;
    int16_t                       sHue;
    int16_t                       sBrightness;
    int32_t                       lAttenuationR;
    int32_t                       lAttenuationG;
    int32_t                       lAttenuationB;
    int32_t                       lOffsetR;
    int32_t                       lOffsetG;
    int32_t                       lOffsetB;

    /* Color keying attributes */
    BVDC_ColorKey_Settings        stColorKey;

    /* Luma Rect setting */
    BVDC_LumaSettings             stLumaRect;

    /* MosaicMode: sub-rectangles placement and size; */
    /* bClearRect:
     *      Enables ClearRect. This flag alone doesn't enable mosaic mode.
     *      It's set by bEnable parameter in BVDC_Window_SetMosaicConfiguration
     * bMosaicMode:
     *      Enables video channels in Mosaics mode. It's set by bVideoInMosaics
     *      in BVDC_MosaicConfiguration
     *
     * Mosaic mode:
     *      bClearRect = true, bMosaicMode = true
     * ClearRect mode:
     *      bClearRect = true, bMosaicMode = false
     */
    bool                          bClearRect;
    bool                          bMosaicMode;
    bool                          bClearRectByMaskColor;
    uint32_t                      ulClearRectAlpha;
    uint32_t                      ulMaskColorYCrCb;
    uint32_t                      ulMosaicCount;
    uint32_t                      ulMaxMosaicCount; /* max mosaic count set by user on a same source */
    uint32_t                      ulMosaicTrackChannelId;
    bool                          abMosaicVisible[BAVC_MOSAIC_MAX];
    BVDC_P_Rect                   astMosaicRect[BAVC_MOSAIC_MAX];
    uint8_t                       aucMosaicZOrder[BAVC_MOSAIC_MAX];

    /* SCL: Fir coeff table Index, and related */
    BVDC_CoefficientIndex         stCtIndex;
    BVDC_Scaler_Settings          stSclSettings;

    /* Game mode delay setting */
    BVDC_Window_GameModeSettings  stGameDelaySetting;

    /* ANR settings */
    bool                          bAnr;/* Enable ANR */
    BVDC_Anr_Settings             stAnrSettings;

    /* Dirty bits of windows. */
    BVDC_P_Window_DirtyBits       stDirty;

    /* Parameters for bandwidth equation
     */
    uint32_t                      ulBandwidthDelta;
    BVDC_SclCapBias               eSclCapBias;

    /* Windows callback data/settings */
    BVDC_Window_CallbackSettings  stCbSettings;

} BVDC_P_Window_Info;

/***************************************************************************
Summary:
    This macro defines the polarity of the "eLastSrcPolarity" and
    "eLastLastSrcPolarity" fields before the arrival of real source
    pictures.

    Ideally such an enum should be added to BAVC_Polarity. But that
    may cause changes to other modules. So we keep it internal to VDC.
    This is only used by cadence handling.

***************************************************************************/
#define BAVC_Polarity_eInvalid    0xFF

/***************************************************************************
Summary:
    Window cadence handling structure.

***************************************************************************/
typedef struct
{
    bool                          bForceAltCap; /* Alternate capture cadence */
    BAVC_Polarity                 eLastCapPolarity;
    bool                          bReaderCadMatching; /* Cadence matching agaist VEC */
    bool                          bDecoderRateCov; /* 50Hz<->60Hz coversion by decoder */
    bool                          bTrickMode;
    bool                          bHandleFldInv;  /* smooth scl for expected field inversion */
} BVDC_P_Window_CadenceHandling;


/***************************************************************************
Summary:
    Rectangles and scaling setup delayed by MAD

***************************************************************************/
typedef struct
{
    BVDC_P_Rect                   stSrcOut;
    uint32_t                      ulOrigPTS;

    bool                          bIgnorePicture;
    bool                          bChannelChange;
    bool                          bLast;
    bool                          bMute;
    uint32_t                      ulDecodePictureId;
    uint32_t                      ulStgPxlAspRatio_x_y;
    BAVC_USERDATA_PictureCoding   ePictureType;
    BAVC_BarDataType              eBarDataType;
    uint32_t                      ulTopLeftBarValue;
    uint32_t                      ulBotRightBarValue;
    bool                          bPreChargePicture;
    bool                          bEndofChunk;
    uint32_t                      ulChunkId;
    BFMT_Orientation              eDispOrientation;
    uint32_t                      ulChannelId;
    BAVC_P_ColorSpace             stMosaicColorSpace;
} BVDC_P_Window_MadDelayed;

typedef struct
{
    BVDC_P_Scaler_Handle          hScaler;
    BVDC_P_Capture_Handle         hCapture;
    BVDC_P_Feeder_Handle          hPlayback;
    BVDC_P_Pep_Handle             hPep;
    BVDC_P_Dnr_Handle             hDnr;
    BVDC_P_Mcvp_Handle            hMcvp;
    BVDC_P_Xsrc_Handle            hXsrc;
    BVDC_P_Vfc_Handle             hVfc;
    BVDC_P_Tntd_Handle            hTntd;
    BVDC_P_Hscaler_Handle         hHscaler;
    BVDC_P_Anr_Handle             hAnr;
    BVDC_P_BoxDetect_Handle       hBoxDetect;
    BVDC_P_VnetCrc_Handle         hVnetCrc;
} BVDC_P_Window_Resource;

/***************************************************************************
Summary:
    Window aspect ratio structure.
***************************************************************************/
typedef struct
{
    bool             bDoAspRatCorrect;
    bool             bNonlinearScl;
    BFMT_AspectRatio eSrcAspectRatio;

    /* for calcu pixel aspect ratio */
    uint16_t         uiSampleAspectRatioX;
    uint16_t         uiSampleAspectRatioY;
    uint16_t         uiPrevSampleAspectRatioX;
    uint16_t         uiPrevSampleAspectRatioY;
    uint32_t         ulSrcPxlAspRatio_x_y;

    /* pixel asp ratio in U4.16 fix pt */
    uintAR_t         ulSrcPxlAspRatio;
    uintAR_t         ulDspPxlAspRatio;

} BVDC_P_Window_AspRatioSettings;

/***************************************************************************
Summary:
    Window AFD structure.
***************************************************************************/
typedef struct
{
    bool             bDoAfd;
    uint32_t         ulAfdVal;

} BVDC_P_Window_AfdSettings;


/***************************************************************************
Summary:
    Window Bar structure.
***************************************************************************/
typedef struct
{
    bool             bDoBar;
    uint32_t         ulTopLeftBarValue;
    uint32_t         ulBotRightBarValue;
    BAVC_BarDataType eBarDataType;

} BVDC_P_Window_BarSettings;

/***************************************************************************
Summary:
    Window Pan Scan structure.
***************************************************************************/
typedef struct
{
    bool             bDoPanScan;
    /* stream pan scan x, y */
    int32_t          lHorizontalPanScan;
    int32_t          lVerticalPanScan;

} BVDC_P_Window_PanScanSettings;

/***************************************************************************
 * Window Context
 ***************************************************************************/
typedef struct BVDC_P_WindowContext
{
    BDBG_OBJECT(BVDC_WIN)

    /* public fields that expose thru API. */
    BVDC_P_Window_Info            stNewInfo;
    BVDC_P_Window_Info            stCurInfo;

    /* Window default setting takin from creat */
    BVDC_Window_Settings          stSettings;

    /* Set to true when new & old validated by apply changes */
    bool                          bUserAppliedChanges;
    BVDC_P_VnetMode               stVnetMode;
    BVDC_P_MvpMode                stMvpMode;            /* Dictates what modules inside Mvp in use*/

    /* Shadow registers and context in parent compositor. */
    BVDC_P_WindowId               eId;
    BVDC_P_State                  eState;
    uint32_t                      ulRegOffset; /* V0_CMP_0, V1_CMP_0, and V0_CMP_1.
                                                * G0_CMP_0, G0_CMP_1.
                                                * Offset from the start of the compositor
                                                * the window is on */

    /* Use to determine Vnet mode. */
    bool                          bDstFullScreen;
    bool                          bCapture;
    bool                          bSyncLockSrc;
#if BVDC_P_SUPPORT_STG
    bool                          bSyncSlave; /* for sync-slaved STG window that the window RUL is owned by sync master source */
#endif
    uint32_t                      ulCapRate; /* In units of
                                                (MHz * BFMT_FREQ_FACTOR) */
    uint32_t                      ulFeedRate;
    uint32_t                      ulSourceRate;

    /* eBufferHeapIdRequest defines the buffer size. eBufferHeapIdPrefer is
     * the prefered buffer heap Id. The actual buffer heap used
     * maybe different if the prefered one is not available. */
    BVDC_P_BufferHeapId           eBufferHeapIdRequest;
    BVDC_P_BufferHeapId           eBufferHeapIdPrefer;
    uint32_t                      ulBufDelay; /* total capture buffer delay in Vsyncs */
    BVDC_P_Buffer_Handle          hBuffer;

    /* Internal VDC or App handed down. */
    BVDC_Heap_Handle              hCapHeap;
    BVDC_Heap_Handle              hDeinterlacerHeap;

    /* BVN video processing delay until display, use to config lipsync. */
    uint32_t                      ulBufCntNeeded;
    uint32_t                      ulPrevBufCntNeeded;
    bool                          bBufCntNeededChanged;
    /* Buffers allocated for multi-buffering and user capture */
    uint32_t                      ulBufCntAllocated;
    BVDC_P_BufHeapAllocMode       eBufAllocMode;
    BVDC_P_BufHeapAllocMode       ePrevBufAllocMode;
    bool                          bRightBufAllocated;

    /* Progressive mode buffer count support */
    bool                          bBufferCntDecremented;
    bool                          bBufferCntDecrementedForPullDown;

    /* needed for interlace with rate gap mode since this mode requires 1 extra buffer */
    bool                          bBufferCntIncremented;

    /* Using the other hardware block.  This could either be part of the
     * source handle or window.  For now we treat of it as part of the
     * window.  These will be created when we create window context. */
    BVDC_P_Window_Resource        stCurResource;
    BVDC_P_Window_Resource        stNewResource;
    bool                          bAllocResource;
    const BVDC_P_ResourceRequire *pResource;
    BVDC_P_ResourceFeature        stResourceFeature;

    /* box detect stuff */
    uint32_t                      ulBoxDetectCallBckCntr;

    /* Created from this Window */
    BVDC_Compositor_Handle        hCompositor;

    /* event handle to be set when a window state transitions from
     * Shutdown to Inactive */
    BKNI_EventHandle              hDestroyDoneEvent;
    bool                          bSetDestroyEventPending;

    /* Event to nofify that changes has been applied to hardware. */
    BKNI_EventHandle              hAppliedDoneEvent;
    bool                          bSetAppliedEventPending;
    bool                          bSlipBuiltWriter;

    /* user pan scan display size calculated at ValidateChanges */
    uint32_t                      ulAutoDispHorizontalSize; /* calculated src display width */
    uint32_t                      ulAutoDispVerticalSize;   /* calculated src display height */
    uint32_t                      ulFieldSrcWidth;   /* for calcu user disp size */
    uint32_t                      ulFieldSrcHeight;  /* for calcu user disp size */
    uint32_t                      eFieldSrcAspectR;  /* for calcu user disp size */

    /* stSrcCnt, stAdjSclOut, AdjDstRect are adjusted src clip, scaler-out
     * and dst rectangles, according to box auto cut, pan-scan, aspect ratio
     * correction, and scale factor rounding. They are private info, and are
     * the ones finally used to build picture nodes and RULs. They are updated
     * at every vsync for mpeg or hddvi src, or when box auto cut is enabled,
     * and once at the vsync after ApplyChanges for the other video cases (e.g.
     * analog src without box auto cut).
     *
     * stSrcCnt's lTop and lLeft values are in U27.4 fixed point format; but
     * ulWidth and ulHeight are in pixel unit */
    BVDC_P_Rect                   stSrcCnt;
    BVDC_P_Rect                   stAdjSclOut;
    BVDC_P_Rect                   stAdjDstRect;
    BVDC_P_Rect                   stPrevSrcCnt; /* intermim, for optimization */
    uint32_t                      ulNonlinearSrcWidth;/* for non-linear scaling */
    uint32_t                      ulNonlinearSclOutWidth;/* for non-linear scaling */
    uint32_t                      ulCentralRegionSclOutWidth;/* for non-linear scaling */
    uint32_t                      ulNrmHrzSrcStep;  /* normalized scl factor: U11.21 format */
    uint32_t                      ulNrmVrtSrcStep;  /* normalized scl factor: U11.21 format */
    bool                          bAdjRectsDirty; /* for optimization */

    /* pixel aspect ratio sent to ViCE2 */
    uint32_t                      ulStgPxlAspRatio_x_y; /* PxlAspR_x<<16 | PxlAspR_y */

    /* encode picture id sent to ragga */
    uint32_t                      ulEncPicId;
    uint32_t                      ulDecPicId;

    /* for MAD and ANR buffer reallocation */

    BVDC_P_BufferHeapId           eMadPixelHeapId[BAVC_MOSAIC_MAX];  /* Mad Pixel Field buffer ID */
    BVDC_P_BufferHeapId           eMadQmHeapId[BAVC_MOSAIC_MAX];     /* Mad Quantized Motion Field buffer ID */
    uint16_t                      usMadPixelBufferCnt[BAVC_MOSAIC_MAX];
    uint16_t                      usMadQmBufCnt[BAVC_MOSAIC_MAX];
    uint32_t                      ulMadPxlBufSize[BAVC_MOSAIC_MAX];
    bool                          bContinuous[BAVC_MOSAIC_MAX];
    uint32_t                      ulMosaicReconfig;


    /* for SclCut setting delay according to MAD pixel output vsync delay */
    bool                          bResetMadDelaySwPipe;
    uint32_t                      ulDeferIdxWr[BAVC_MOSAIC_MAX]; /* writer index to stMadDelayed */
    uint32_t                      ulDeferIdxRd[BAVC_MOSAIC_MAX]; /* reader index to stMadDelayed */
    BVDC_P_Window_MadDelayed      stMadDelayed[BAVC_MOSAIC_MAX][BVDC_MADDELAY_BUF_COUNT]; /* circle buf for mad-delayed rects and scl stuff */

    /* to overide pic bRepeatField to false when clipping before MAD changed */
    int32_t                       lPrevSrcOutLeft;  /* srcOut.lLeft last vsync */
    int32_t                       lPrevSrcOutTop;  /* srcOut.lTop last vsync */
    int32_t                       lPrevCapOutLeft;  /* CapOut.lLeft last vsync */
    int32_t                       lPrevCapOutTop;  /* CapOut.lTop last vsync */
    uint32_t                      ulMadFlushCntr; /* to flush change out during freeze */

    /* real blender used for this window */
    uint32_t                      ulBlenderId;
    uint32_t                      ulPrevBlenderId;

    /* to mute BVDC_P_WindowId_eCompB_V0 */
    bool                          bMuteBypass;

    /* for capture buffer use*/
    unsigned int                  uiAvailCaptureBuffers;
    bool                          bMute;

    /* Lip sync */
    bool                          bRepeatCurrReader;
    BVDC_P_PictureNode           *pCurWriterNode;
    BVDC_P_PictureNode           *pCurReaderNode;
    uint32_t                      ulTotalBvnDelay; /* total BVN delay in Vsyncs from input to output */

    /* Game mode buffer delay */
    uint32_t                      ulCurBufDelay; /* absolute delay from capture to playback in usec */
    int32_t                       lCurGameModeLag;/* ulCurBufDelay - game mode target */
    bool                          bAdjGameModeClock;
    bool                          bCoarseAdjClock;
    bool                          bFastAdjClock;

    /* Window information prepare to hand back to user when it's changed. */
    BVDC_Window_CallbackData      stCbData;

    /* MosaicMode: clear rect mask set */
    uint32_t                      ulMosaicRectSet;
    bool                          bClearRectSupport;

    /* sub-struct to manage vnet for the compositor's win src */
    BVDC_P_SubRulContext          stWinOutVnet;

    /* Capture pictures as frames. This is needed when source and display
     * rates and polarities differ.  Are we doing pulldown? */
    bool                          bDoPulldown;
    bool                          bFrameCapture;

    /* CMP CSC */
    bool                          bBypassCmpCsc;

    /* there are typically 16 mosaic rects and 5 or 6 mosaic csc slots and eotf slots.
     * aMosaicRectMatrixCoeffsList is used to store input matrixCoefficents for each rects,
     * aMosaicCscSlotForMatrixCoeffs is used to look up the csc slots for an input matrixCoeffs,
     * aMatrixCoeffsInMosaicCscSlot stores the input matrixCoeffs that this csc slot is for,
     * astMosaicCscSlotCfgList holds the csc configures for each csc slot;
     * aMosaicRectEotfList is used to store eotf for each rects,
     * aMosaicEotfConvSlotForEotf is used to look up the EotfConv slots for an input eotf,
     * aEotfInMosaicEotfConvSlot stores the input eotf that this EotfConv slot is for,
     * astMosaicEotfConvSlotCfgList holds the EotfConv configures for each EotfConv slot.
     */
    BVDC_P_CfcContext            *pMainCfc;
    BVDC_P_CfcContext            *pDemoCfc;    /* for demo mode */
    uint8_t                       ucNumMosaicRects;
    uint8_t                       aucMosaicCfcIdxForRect[BAVC_MOSAIC_MAX]; /* cfcIdx assigned to rect */
    BVDC_P_CfcContext             astMosaicCfc[BVDC_P_CMP_CFCS]; /* mosaic cfc array */
    bool                          bCfcAdjust; /* cleared only after AssignMosaicCfcToRect_isr is really called */

    /* This flag indicate if the bandwidth equation is symmetric or not */
    bool                          bSclCapSymmetric;

    /* Fields for cadence handling. */
    BVDC_P_Window_CadenceHandling stCadHndl;

#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* capture drain buffer for mosaic mode */
    BMMA_Block_Handle              hMosaicMmaBlock;
    BMMA_DeviceOffset              ullNullBufOffset;
    uint32_t                       aulMosaicZOrderIndex[BAVC_MOSAIC_MAX];
#endif

    uint32_t                       ulDropCntNonIgnoredPics;

#if (BVDC_P_MAX_MULTI_BUFFER_COUNT > 0)
    BVDC_P_HeapNodePtr             apHeapNode[BVDC_P_MAX_MULTI_BUFFER_COUNT];
    BVDC_P_HeapNodePtr             apHeapNode_R[BVDC_P_MAX_MULTI_BUFFER_COUNT];
#endif

    uint32_t                       ulBlenderMuteCnt;

    BVDC_P_Window_AfdSettings      stAfdSettings;
    BVDC_P_Window_BarSettings      stBarSettings;
    BVDC_P_Window_PanScanSettings  stPanScanSettings;
    BVDC_P_Window_AspRatioSettings stAspRatioSettings;

    bool                           bIs10BitCore;
    bool                           bIs2xClk;
    bool                           bSupportDcxm;

    /* Compression settings */
    BVDC_P_Compression_Settings    stCapCompression;
    BVDC_P_Compression_Settings    stMadCompression;

    /* for refine rev 3:2 pulldown cadence when MTG is used */
    bool                           bMadOutPhase3Over2;
    int                            iPrevPhaseCntr;
    int                            iPhaseCntr;
    int                            iLockCntr;

    BFMT_Orientation               eSrcOrientation;
    BFMT_Orientation               eDispOrientation;

    bool                           bNotEnoughCapBuf;

} BVDC_P_WindowContext;

/***************************************************************************
 * Captured Picture Info
 ***************************************************************************/
typedef struct
{
    /* the MMA block of the corresponding captured picture */
    BMMA_Block_Handle             hPicBlock;
    unsigned                      ulPicBlockOffset;
    BMMA_Block_Handle             hPicBlock_R;
    unsigned                      ulPicBlockOffset_R;
    BFMT_Orientation              eDispOrientation;
    uint32_t                      ulHeight;
    uint32_t                      ulWidth;
    uint32_t                      ulPitch;
    uint32_t                      ulEncPicId;
    uint32_t                      ulDecPicId;
    BPXL_Format                   ePxlFmt;
    uint32_t                      ulNumVbiLines;
    BAVC_Polarity                 ePolarity;
    uint32_t                      ulSourceRate;
    uint32_t                      ulOrigPTS;
    bool                          bStallStc;
    bool                          bIgnorePicture;
    uint32_t                      ulPxlAspRatio_x;
    uint32_t                      ulPxlAspRatio_y;
    BVDC_P_PictureNode           *pPicture;

} BVDC_P_Window_CapturedPicture;


/***************************************************************************
 * Window private functions
 ***************************************************************************/
BERR_Code BVDC_P_Window_Create
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Window_Handle              *phWindow,
      BAVC_SourceId                    eSrcId,
      BVDC_WindowId                    eWindowId );

void BVDC_P_Window_Destroy
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_Rts_Init
    (
#if BVDC_P_SUPPORT_XCODE_WIN_CAP
      bool                             bCmpXcode,
      bool                             bDispNrtStg,
#endif
      bool                            *pbForceCapture,
      bool                            *pbSclCapSymmetric,
      BVDC_SclCapBias                 *peSclCapBias,
      uint32_t                        *pulBandwidthDelta );

void BVDC_P_Window_Compression_Init_isr
    ( bool                             bIs10BitCore,
      bool                             bSupportDcxm,
      BVDC_P_Compression_Settings     *pCapCompSetting,
      BVDC_P_Compression_Settings     *pMadCompSetting,
      BVDC_P_MvpDcxCore                eDcxCore);
#define BVDC_P_Window_Compression_Init BVDC_P_Window_Compression_Init_isr

void BVDC_P_Window_Init
    ( BVDC_Window_Handle               hWindow,
      BVDC_Source_Handle               hSource );

#if (BVDC_P_SUPPORT_TNT_VER < 5)         /* TNT HW base */
void BVDC_P_Window_Sharpness_Init
    ( BVDC_Window_Handle               hWindow,
      BVDC_SharpnessSettings          *pSharpnessConfig );
#endif

void BVDC_P_Window_BuildRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eNextFieldId,
      bool                             bBuildWriter,
      bool                             bBuildReader,
      bool                             bBuildCanvasCtrl );

BERR_Code BVDC_P_Window_ValidateChanges
    ( const BVDC_Window_Handle         hWindow,
      const BFMT_VideoInfo            *pDstFmtInfo );

BERR_Code BVDC_P_Window_ApplyChanges_isr
    ( BVDC_Window_Handle               hWindow );

BERR_Code BVDC_P_Window_AbortChanges
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_ValidateRects_isr
    ( const BVDC_Window_Handle         hWindow,
      const BAVC_MVD_Field            *pFieldData );

void BVDC_P_Window_InitMuteRec_isr
    ( const BVDC_Window_Handle         hWindow,
      const BFMT_VideoInfo            *pDstFmtInfo,
      const BAVC_MVD_Field            *pMvdFieldData,
      uint32_t                         ulRectIdx );

void BVDC_P_Window_AdjustRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      uint32_t                         ulPicCnt );

void BVDC_P_Window_UpdateState_isr
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_UpdateUserState_isr
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_AlignPreMadRect_isr
    ( int32_t                          lMin,
      int32_t                          lMin_R,
      uint32_t                         ulSize,
      uint32_t                         ulFullSize,
      uint32_t                         ulMinFracBits,
      uint32_t                         ulAlignUnit,
      int32_t                         *plNewMin,
      int32_t                         *plNewMin_R,
      uint32_t                        *pulNewSize );

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
void BVDC_P_Window_PreMadAdjustWidth_isr
    ( uint32_t                  ulPicWidth,
      uint32_t                  ulPicHeight,
      uint32_t                  ulBitsPerPixel,
      uint32_t                  ulPixelPerGroup,
      uint32_t                 *pulNewPicWidth );
#endif

void BVDC_P_Window_GetSourceContentRect_isr
    ( const BVDC_Window_Handle         hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      int32_t                         *plWinXMin,
      int32_t                         *plWinYMin,
      int32_t                         *plWinXMin_R,
      int32_t                         *plWinXMax,
      int32_t                         *plWinYMax );

void BVDC_P_Window_SetDisplaySize_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Rect               *pDstRect,
      BAVC_Polarity                    eScanType,
      uint32_t                         ulRWinXOffsetulRWinXOffset);

BERR_Code BVDC_P_Window_EnableBoxDetect
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_BoxDetectCallback_isr  BoxDetectCallBack,
      void                            *pvParm1,
      int                              iParm2,
      bool                             bAutoCutBlack );

BERR_Code BVDC_P_Window_DisableBoxDetect
    ( BVDC_Window_Handle               hWindow );

BERR_Code BVDC_P_Window_GetPrivHandle
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_WindowId                    eWindowId,
      BAVC_SourceId                    eSrcId,
      BVDC_Window_Handle              *phWindow,
      BVDC_P_WindowId                 *peWindowId );

/* For gfx applychanges. */
void BVDC_P_Window_GetNewRectangles
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect );

void BVDC_P_Window_GetNewRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect );

void BVDC_P_Window_GetCurrentRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect );

void BVDC_P_Window_GetNewWindowAlpha
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucWindowAlpha );

void BVDC_P_Window_GetNewDispOrientation
    ( const BVDC_Window_Handle         hWindow,
      BFMT_Orientation                 *pOrientation);

void BVDC_P_Window_GetNewBlendFactor
    ( const BVDC_Window_Handle         hWindow,
      BVDC_BlendFactor                *peFrontBlendFactor,
      BVDC_BlendFactor                *peBackBlendFactor,
      uint8_t                         *pucConstantAlpha );

void BVDC_P_Window_GetNewScanType
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbInterlaced );

/* For RUL building. */
void BVDC_P_Window_Writer_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      const BAVC_Polarity              eFieldId,
      BVDC_P_ListInfo                 *pList,
      uint32_t                         ulPictureIndex );

void BVDC_P_Window_Reader_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId,
      BVDC_P_ListInfo                 *pList );

bool BVDC_P_Window_GetReconfiguring_isr
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_SetReconfiguring_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bSrcPending,
      bool                             bReConfigVnet,
      bool                             bBufferPending );

BERR_Code BVDC_P_Window_CapturePicture_isr
    ( BVDC_Window_Handle               hWindow,
     BVDC_P_Window_CapturedPicture    *pCapturedPic );

BERR_Code BVDC_P_Window_ReleasePicture_isr
    ( BVDC_Window_Handle               hWindow,
      BPXL_Plane                      *pCaptureBuffer );

bool BVDC_P_Window_CheckForUnReturnedUserCapturedBuffer_isr
    ( BVDC_Window_Handle               hWindow );

BERR_Code BVDC_P_Window_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB );

BERR_Code BVDC_P_Window_SetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      const int32_t                    pl32_UserMatrix[BVDC_CSC_COEFF_COUNT],
      int32_t                          pl32_WinMatrix[BVDC_CSC_COEFF_COUNT] );

BERR_Code BVDC_P_Window_GetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                             bOverride,
      int32_t                          pl32_MatrixWin[BVDC_CSC_COEFF_COUNT],
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT] );

void BVDC_P_Window_CalculateCsc_isr
    ( BVDC_Window_Handle               hWindow );

void BVDC_P_Window_SetSecCscDemo_isr
    ( BVDC_Window_Handle               hWindow );

BERR_Code BVDC_P_Window_SetMcvp_DeinterlaceConfiguration
    (BVDC_Window_Handle               hWindow,
    bool                             bDeinterlace,
    const BVDC_Deinterlace_Settings *pMadSettings);


const BVDC_P_ResourceFeature* BVDC_P_Window_GetResourceFeature_isrsafe
    ( BVDC_P_WindowId                  eWindowId );

const BVDC_P_ResourceRequire* BVDC_P_Window_GetResourceRequire_isrsafe
    ( BVDC_P_WindowId                  eWindowId );

void BVDC_P_Window_CalculateMosaicCsc_isr
    ( BVDC_Window_Handle               hWindow );

BERR_Code BVDC_P_Window_ConfigureMadGameMode_isr
    ( const BVDC_Window_Handle         hWindow);

void BVDC_P_Window_SetInvalidVnetMode_isr
    ( BVDC_P_VnetMode                  *pVnetMode );

void BVDC_P_Window_DecideCapture_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_Source_Handle               hSource,
      const BVDC_Compositor_Handle     hCompositor );

#if (BDBG_DEBUG_BUILD)
void BVDC_P_Window_DumpRects_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_PictureNode        *pPicture );
#else
#define BVDC_P_Window_DumpRects_isr(hWindow, pPicture)
#endif

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_MAD_ANR)
BERR_Code BVDC_P_Window_UpdateMadAnrCompression_isr
    ( BVDC_Window_Handle                       hWindow,
      BPXL_Format                              ePxlFormat,
      const BVDC_P_Rect                       *pRect,
      const BVDC_P_PictureNodePtr              pPicture,
      BVDC_P_Compression_Settings             *pWinCompression,
      bool                                     bWriter);
#endif

void BVDC_P_Window_GetBufSize_isr
    ( BVDC_P_WindowId                         eWinId,
      const BVDC_P_Rect                      *pSrcRect,
      const bool                              bInterlaced,
      const bool                              bMosaicMode,
      const bool                              b3DMode,
      const bool                              bMinSrc,
      const BPXL_Format                       eBufPxlFmt,
      const BVDC_P_Compression_Settings      *pCompression,
      BVDC_P_BufHeapType                      eBufHeapType,
      uint32_t                               *pulBufSize,
      BAVC_VideoBitDepth                      eBitDepth);

/* init mosaic colorSpace array in picture node as all invalid
 */
void BVDC_P_Window_InitVideoInputColorSpace_isr(
    BVDC_P_PictureNode         *pPicture);

/* init mosaic cfcs in cmp
 */
void BVDC_P_Window_InitCfcs(
    BVDC_Window_Handle          hWindow );

/* copy input color space info from BAVC_MVD_Field to mosaic colorSpace
 * array in picture node
 */
void BVDC_P_Window_UpdateVideoInputColorSpace_isr(
    BVDC_Window_Handle               hWindow,
    const BAVC_MVD_Field            *pMvdFieldData,
    const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
    BAVC_MatrixCoefficients          eMatrixCoefficients, /* for analogue */
    BAVC_P_ColorSpace               *pColorSpace );

/* Assign CFC for each mosaic rectangle
 * return true if some mosaic rect's colorSpace changed
 */
bool BVDC_P_Window_AssignMosaicCfcToRect_isr(
    BVDC_Window_Handle          hWindow,
    BVDC_P_PictureNode         *pPicture,
    bool                        bOutColorSpaceDirty);

/* Build RUL for a mosaic-rect / cfc inside a compositor.
 */
void BVDC_P_Window_BuildCfcRul_isr
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectIdx, /* Mosaic rect index */
      BVDC_P_ListInfo                 *pList);

void BVDC_P_Window_SetSurfaceSize_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Rect               *pSurRect,
      BAVC_Polarity                    eScanType );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_WINDOW_PRIV_H__ */
/* End of file. */
