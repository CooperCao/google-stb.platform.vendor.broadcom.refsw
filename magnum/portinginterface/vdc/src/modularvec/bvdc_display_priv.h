/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef BVDC_DISPLAY_PRIV_H__
#define BVDC_DISPLAY_PRIV_H__

#include "bvdc.h"
#include "bvdc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (BCHP_CHIP != 7420)
#include "bchp_prim_it.h"
#include "bchp_prim_vf.h"
#include "bchp_prim_sm.h"
#include "bchp_prim_csc.h"
#include "bchp_prim_rm.h"
#include "bchp_prim_src.h"

#if BVDC_P_SUPPORT_SEC_VEC
#include "bchp_sec_it.h"
#include "bchp_sec_vf.h"
#include "bchp_sec_sm.h"
#include "bchp_sec_csc.h"
#include "bchp_sec_rm.h"
#include "bchp_sec_src.h"
#endif

#if BVDC_P_SUPPORT_TER_VEC
#include "bchp_tert_it.h"
#include "bchp_tert_vf.h"
#include "bchp_tert_sm.h"
#include "bchp_tert_csc.h"
#include "bchp_tert_rm.h"
#include "bchp_tert_src.h"
#endif
#else
#include "bchp_it_0.h"
#include "bchp_vf_0.h"
#include "bchp_sm_0.h"
#include "bchp_csc_0.h"
#include "bchp_rm_0.h"
#include "bchp_sdsrc_0.h"
#include "bchp_secam_0.h"

#if BVDC_P_SUPPORT_SEC_VEC
#include "bchp_it_1.h"
#include "bchp_vf_1.h"
#include "bchp_sm_1.h"
#include "bchp_csc_1.h"
#include "bchp_rm_1.h"
#include "bchp_sdsrc_1.h"
#include "bchp_secam_1.h"
#endif

#if BVDC_P_SUPPORT_TER_VEC
#include "bchp_it_2.h"
#include "bchp_vf_2.h"
#include "bchp_sm_2.h"
#include "bchp_csc_2.h"
#include "bchp_rm_2.h"
#include "bchp_sdsrc_2.h"
#include "bchp_secam_2.h"
#endif
#endif /* 7420 */

#if BVDC_P_VEC_SUPPORT_DVI_COLOR_CNVT
#include "bchp_dvi_ccb.h"
#endif

/****************************************************************
 *  Defines
 ****************************************************************/
/* ---------------------------------------------
 * HDMI_RM revision
 * --------------------------------------------- */
/* 3548 Ax, B0, B1, 3556 Ax, B0, B1
 *  DVPO support
 */
#define BVDC_P_HDMI_RM_VER_0                 (0)

/* 3548 B2 and above, 3556 B2 and above
 *  DVPO support  with fixed spread spectrum
 */
#define BVDC_P_HDMI_RM_VER_1                 (1)

/* 7400, 7405, 7335, 7336
 *  DVI support: 65NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_2                 (2)

/* 7325
 *  DVI support: 65NM 27MHz
 */
#define BVDC_P_HDMI_RM_VER_3                 (3)

/* 7340, 7342, 7550, 7420, 7125, 7468, 7408
 *  DVI support: 65NM 27MHz
 */
#define BVDC_P_HDMI_RM_VER_4                 (4)

/* 7422, 7425, 7358, 7552, 7231, 7346, 7344
 *  DVI support: 40NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_5                 (5)

/* 7445
 *  DVI support: 40NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_6                 (6)

#define BVDC_P_SUPPORT_DVPO                   \
	(BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_1)

#define BVDC_P_SUPPORT_DVI_65NM               \
	((BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_2) || \
	 (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_3) || \
	 (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_4))

#define BVDC_P_SUPPORT_DVI_40NM                   \
	((BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5) || \
	 (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6))

#define BVDC_P_MAX_VEC_RUL_ENTRIES          665
#define BVDC_P_RAM_TABLE_SIZE               256
#define BVDC_P_DTRAM_TABLE_SIZE            (128/2)
#define BVDC_P_RAM_TABLE_TIMESTAMP_IDX     (BVDC_P_RAM_TABLE_SIZE - 2)
#define BVDC_P_RAM_TABLE_CHECKSUM_IDX      (BVDC_P_RAM_TABLE_SIZE - 1)
#define BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX   (BVDC_P_DTRAM_TABLE_SIZE - 2)
#define BVDC_P_DTRAM_TABLE_CHECKSUM_IDX    (BVDC_P_DTRAM_TABLE_SIZE - 1)
#if BVDC_P_VEC_SUPPORT_DVI_COLOR_CNVT
#define BVDC_P_CCB_TABLE_SIZE              (BCHP_DVI_CCB_CCB_ELEMENT_i_ARRAY_END + 1)
#else
#define BVDC_P_CCB_TABLE_SIZE              (1)
#endif

#if (BCHP_CHIP != 7420)
#define BVDC_P_CSC_TABLE_SIZE              (uint32_t)((BCHP_PRIM_CSC_CSC_COEFF_C23_C22 - BCHP_PRIM_CSC_CSC_MODE)/4+1)
#define BVDC_P_DITHER_TABLE_SIZE           (uint32_t)((BCHP_PRIM_CSC_DITHER_LFSR_INIT - BCHP_PRIM_CSC_DITHER_CONTROL)/4+1)

#define BVDC_P_VF_TABLE_SIZE              (uint32_t)((BCHP_PRIM_VF_SYNC_TRANS_1 - BCHP_PRIM_VF_FORMAT_ADDER)/4+1)
#define BVDC_P_CHROMA_TABLE_SIZE          (uint32_t)((BCHP_PRIM_VF_CH0_TAP10 - BCHP_PRIM_VF_CH0_TAP1_3)/4+1)
#define BVDC_P_RM_TABLE_SIZE              (uint32_t)((BCHP_PRIM_RM_INTEGRATOR - BCHP_PRIM_RM_RATE_RATIO)/4+1)
#define BVDC_P_IT_TABLE_SIZE              (uint32_t)((BCHP_PRIM_IT_PCL_5 - BCHP_PRIM_IT_ADDR_0_3)/4+1)
#define BVDC_P_SM_TABLE_SIZE              (uint32_t)((BCHP_PRIM_SM_COMP_CNTRL - BCHP_PRIM_SM_ENVELOPE_GENERATOR)/4+1)
#else
#define BVDC_P_CSC_TABLE_SIZE              (uint32_t)((BCHP_CSC_0_CSC_COEFF_C23_C22 - BCHP_CSC_0_CSC_MODE)/4+1)
#define BVDC_P_DITHER_TABLE_SIZE           (uint32_t)((BCHP_CSC_0_DITHER_LFSR_INIT - BCHP_CSC_0_DITHER_CONTROL)/4+1)

#define BVDC_P_VF_TABLE_SIZE              (uint32_t)((BCHP_VF_0_SYNC_TRANS_1 - BCHP_VF_0_FORMAT_ADDER)/4+1)
#define BVDC_P_CHROMA_TABLE_SIZE          (uint32_t)((BCHP_VF_0_CH0_TAP10 - BCHP_VF_0_CH0_TAP1)/4+1)
#define BVDC_P_RM_TABLE_SIZE              (uint32_t)((BCHP_RM_0_INTEGRATOR - BCHP_RM_0_RATE_RATIO)/4+1)
#define BVDC_P_IT_TABLE_SIZE              (uint32_t)((BCHP_IT_0_PCL_5 - BCHP_IT_0_ADDR_0_3)/4+1)
#define BVDC_P_SM_TABLE_SIZE              (uint32_t)((BCHP_SM_0_COMP_CNTRL - BCHP_SM_0_PG_CNTRL)/4+1)
#endif /* 7420 */

/* Vec phase adjustment values */
#define BVDC_P_PHASE_OFFSET                0x1d8
#define BVDC_P_THRESHOLD1                  2
#define BVDC_P_THRESHOLD2                  0x15
#define BVDC_P_PHASE_180                   0x200
#define BVDC_P_PHASE_90                    0x100
#define BVDC_P_PHASE_45                    0x080
#define BVDC_P_PHASE_135                   0x180
#define BVDC_P_PHASE_225                   0x280
#define BVDC_P_PHASE_270                   0x300
#define BVDC_P_PHASE_315                   0x380

/* NOTE:
   DAC design spec linearly maps DAC code value between 16 ~ 800 onto
   output voltage level 0 ~ 1000 mV;
   so we have scaling factor for DAC code as:
         (800 - 16) / 1000 = 0.784/mV
   where, value 16 maps sync tip voltage level, and 800 maps to 1000 mV
   peak to peak (picture peak white to sync tip) voltage level;   */
#define BVDC_P_DAC_CODE_MAX_VALUE               (800)
#define BVDC_P_DAC_CODE_MIN_VALUE               (16)
#define BVDC_P_DAC_OUTPUT_RANGE                 (1000) /* 1000 mV */

/* To convert mv into DAC code value relative to sync tip level;
   Note: here 'mv' is relative to sync tip level! */
#define BVDC_P_DAC_CODE_VALUE(mv) \
	((mv) * (BVDC_P_DAC_CODE_MAX_VALUE - BVDC_P_DAC_CODE_MIN_VALUE) / \
	 BVDC_P_DAC_OUTPUT_RANGE + BVDC_P_DAC_CODE_MIN_VALUE)

/* Care needs to be taken for NTSC CVBS/Svideo outputs vs 480i YPrPb/RGB
   or outputs of other formats:
   NTSC CVBS/Svideo outputs have 714mV/286mV Picture/Sync ratio, while
   all other cases have 700mV/300mV Picture/Sync ratio, i.e. picture
   white is at 700mV and sync tip at -300mv, all relative to blank level. */

/* NTSC CVBS/Svideo:
	286mV blank level relative to sync tip; */
#define BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL      (286)
#define BVDC_P_DAC_OUTPUT_NTSC_PEAK_WHITE_LEVEL \
	(BVDC_P_DAC_OUTPUT_RANGE - BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL)

/* All other outputs, including 480i YPrPb/RGB and/or CVBS/Svideo/YPrPb/RGB
   for other formats:
   300mV blank level relative to sync tip; */
#define BVDC_P_DAC_OUTPUT_SYNC_LEVEL           (300)
#define BVDC_P_DAC_OUTPUT_PEAK_WHITE_LEVEL \
	(BVDC_P_DAC_OUTPUT_RANGE - BVDC_P_DAC_OUTPUT_SYNC_LEVEL)

/* To convert mv into DAC code value relative to blank level;
   Note: here 'mv' is relative to the blank level! */
#define BVDC_P_POS_SYNC_AMPLITUDE_VALUE(mv) \
	((mv) * (BVDC_P_DAC_CODE_MAX_VALUE - BVDC_P_DAC_CODE_MIN_VALUE) \
	/ BVDC_P_DAC_OUTPUT_RANGE)

/* To convert mv into DAC code value relative to sync tip level */
#define BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(mv) BVDC_P_DAC_CODE_VALUE(mv)

#define BVDC_P_NEG_SYNC_TIP_VALUE           BVDC_P_DAC_CODE_MIN_VALUE

/* Check for macro vision type */
#define BVDC_P_MACROVISION_ON_RGB(type) \
	((type) > BVDC_MacrovisionType_eCustomized)

#define BVDC_P_MACROVISION_WITH_2LINES_CS(type) \
	(((type) == BVDC_MacrovisionType_eAgc2Lines) || \
	 ((type) == BVDC_MacrovisionType_eAgc2Lines_Rgb))

#define BVDC_P_MACROVISION_WITH_4LINES_CS(type) \
	(((type) == BVDC_MacrovisionType_eAgc4Lines) || \
	 ((type) == BVDC_MacrovisionType_eAgc4Lines_Rgb))

/* Where to trigger after vsync (line 1) */
#define BVDC_P_TRIGGER_LINE               (3)

/* Bx software workaround, number of vsyncs to wait before enabling
 * DVI input. */
#define BVDC_P_DVIINPUT_WAIT_VSYNCS       (1)

/* DVI DTRam LOCATION! */
#define BVDC_P_DVI_DTRAM_START_ADDR       (0x40)

#define BVDC_P_DISP_GET_TOP_TRIGGER(pDisplay) \
	((pDisplay)->eTopTrigger)

#define BVDC_P_DISP_GET_BOT_TRIGGER(pDisplay) \
	((pDisplay)->eBotTrigger)

#define BVDC_P_DISP_IS_COMPONENT(eColorSpace) ((eColorSpace == BVDC_P_Output_eSDYPrPb) || \
											   (eColorSpace == BVDC_P_Output_eHDYPrPb) || \
											   (eColorSpace == BVDC_P_Output_eSDRGB)   || \
											   (eColorSpace == BVDC_P_Output_eHDRGB)   || \
											   (eColorSpace == BVDC_P_Output_eHsync))

#define BVDC_P_DISP_IS_VALID_DISPOUTPUT_AND_DAC(eDispOutput, eDacOutput)                                                         \
	(((eDisplayOutput == BVDC_DisplayOutput_eComponent) &&                                                                       \
	  ((eDacOutput == BVDC_DacOutput_eY) || (eDacOutput == BVDC_DacOutput_ePr) || (eDacOutput == BVDC_DacOutput_ePb) ||          \
	   (eDacOutput == BVDC_DacOutput_eRed) || (eDacOutput == BVDC_DacOutput_eGreen) || (eDacOutput == BVDC_DacOutput_eBlue) ||   \
	   (eDacOutput == BVDC_DacOutput_eGreen_NoSync))) ? true :                                                                   \
	    (((eDisplayOutput == BVDC_DisplayOutput_eComposite) && (eDacOutput == BVDC_DacOutput_eComposite)) ? true :               \
	     ((eDisplayOutput == BVDC_DisplayOutput_eSVideo) &&                                                                      \
		  ((eDacOutput == BVDC_DacOutput_eSVideo_Luma) || (eDacOutput == BVDC_DacOutput_eSVideo_Chroma)) ? true : false)))

#define BVDC_P_DISP_INVALID_VF_CH (-1)
#define BVDC_P_DISP_GET_VF_CH_FROM_DAC(eDacOutput) \
	(((eDacOutput == BVDC_DacOutput_eComposite) || (eDacOutput == BVDC_DacOutput_eSVideo_Luma) || (eDacOutput == BVDC_DacOutput_eGreen) ||  \
	  (eDacOutput == BVDC_DacOutput_eGreen_NoSync) || (eDacOutput == BVDC_DacOutput_eY)) ? 0 :                                              \
	  (((eDacOutput == BVDC_DacOutput_eSVideo_Chroma) || (eDacOutput == BVDC_DacOutput_eRed) || (eDacOutput == BVDC_DacOutput_ePr)) ? 1 :   \
	    ((eDacOutput == BVDC_DacOutput_eBlue) || (eDacOutput == BVDC_DacOutput_ePb)) ? 2 : BVDC_P_DISP_INVALID_VF_CH))

/* BAVC_FrameRateCode is full rate (non 1/1001 drop).  */
#define BVDC_P_IS_FULL_FRAMRATE(eFrameRate)    \
	((BAVC_FrameRateCode_e24 == eFrameRate) || \
	 (BAVC_FrameRateCode_e25 == eFrameRate) || \
	 (BAVC_FrameRateCode_e30 == eFrameRate) || \
	 (BAVC_FrameRateCode_e50 == eFrameRate) || \
	 (BAVC_FrameRateCode_e60 == eFrameRate) || \
	 (BAVC_FrameRateCode_e15 == eFrameRate) || \
	 (BAVC_FrameRateCode_e10 == eFrameRate) || \
	 (BAVC_FrameRateCode_e12_5 == eFrameRate) || \
	 (BAVC_FrameRateCode_e20 == eFrameRate))

/* Work around a hardware bug in a specific WSE core.
 * This value should be identical to BVBI_P_WSE_VER3 in
 * the other porting interface module. */
#if (BCHP_CHIP == 7325) && (BCHP_VER == BCHP_VER_B0)
	#define BVDC_P_WSE_VER3 1
#endif
#if (BCHP_CHIP == 7335)  || (BCHP_CHIP == 7336)
	#define BVDC_P_WSE_VER3 1
#endif
#if (BCHP_CHIP == 7601)
	#define BVDC_P_WSE_VER3 1
#endif

/* Number of channels */
#define BVDC_P_VEC_CH_NUM       3


/****************************************************************
 *  Macros
 ****************************************************************/
#define BVDC_P_TIMER_FREQ                                    (27000000) /* 27 MHz */

/* one vsync in usecs */
#define BVDC_P_USEC_ONE_VSYNC_INTERVAL(vrate)    ( \
	(1000000 * BFMT_FREQ_FACTOR / (vrate)))

/* alignment threshold in usecs */
#define BVDC_P_USEC_ALIGNMENT_THRESHOLD     (200)

/* one vsync in 27 MHz clock cycless */
#define BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(vrate)    ( \
	(BVDC_P_TIMER_FREQ / (vrate)) * BFMT_FREQ_FACTOR )
#define BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD    ( \
	BVDC_P_TIMER_FREQ / 1000000 * BVDC_P_USEC_ALIGNMENT_THRESHOLD) /* 200 usecs */
#define BVDC_P_DISPLAY_ALIGNMENT_OFFSET    (0*BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD)

/***************************************************************************
 *  Private Enums
 ***************************************************************************/
/* Contains VEC path selected by VDC */
typedef enum
{
	BVDC_P_eVecPrimary = 0,
	BVDC_P_eVecSecondary,
	BVDC_P_eVecTertiary,
	BVDC_P_eVecBypass0
} BVDC_P_VecPath;

/* DAC Output */
typedef enum
{
	BVDC_P_Output_eYQI=0,        /* SVideo and/or CVBS for NTSC */
	BVDC_P_Output_eYQI_M,        /* SVideo and/or CVBS for NTSC_J */
	BVDC_P_Output_eYUV,          /* SVideo and/or CVBS for Pal */
	BVDC_P_Output_eYUV_M,        /* SVideo and/or CVBS for Pal_M */
	BVDC_P_Output_eYUV_N,        /* SVideo and/or CVBS for Pal_N */
	BVDC_P_Output_eYUV_NC,       /* SVideo and/or CVBS for PAL_NC */
#if BVDC_P_SUPPORT_VEC_SECAM
	BVDC_P_Output_eYDbDr_LDK,    /* SVideo and/or CVBS for SECAM_L/D/K */
	BVDC_P_Output_eYDbDr_BG,     /* SVideo and/or CVBS for SECAM_B/G */
	BVDC_P_Output_eYDbDr_H,      /* SVideo and/or CVBS for SECAM_H */
#endif
	/* Folks, would you please put all of the component and RGB formats below
	 * this line. Thanks.
	 */
	BVDC_P_Output_eSDYPrPb,      /* SYPrPb, CYPrPb, YPrPb */
	BVDC_P_Output_eSDRGB,        /* SRGB, CRGB, SCRGB, RGB */
	BVDC_P_Output_eHDYPrPb,      /* HDYPrPb */
	BVDC_P_Output_eHDRGB,        /* HDYRGB */
	BVDC_P_Output_eHsync,        /* HSYNC */
	BVDC_P_Output_eUnknown,
	BVDC_P_Output_eNone,
	BVDC_P_Output_eMax
} BVDC_P_Output;

/* Output Filter Types */
typedef enum
{
	BVDC_P_OutputFilter_eHDYPrPb=0,
	BVDC_P_OutputFilter_eHDRGB,
	BVDC_P_OutputFilter_eED,
	BVDC_P_OutputFilter_eSDYPrPb,
	BVDC_P_OutputFilter_eSDRGB,
	BVDC_P_OutputFilter_eYQI,
	BVDC_P_OutputFilter_eYUV,
	BVDC_P_OutputFilter_eSECAM,
	BVDC_P_OutputFilter_eHsync,
	BVDC_P_OutputFilter_eUnknown,
	BVDC_P_OutputFilter_eNone,
	BVDC_P_OutputFilter_eMax
} BVDC_P_OutputFilter;

typedef enum
{
	BVDC_P_ItState_eActive = 0, /* Active, no change necessary */
	BVDC_P_ItState_eNotActive,  /* Initial state. Vecs are not running */
	BVDC_P_ItState_eSwitchMode, /* Mode switch request */
	BVDC_P_ItState_eNewMode     /* Vec switched to new mode. */
} BVDC_P_ItState;

/***************************************************************************
 * Display Context
 ***************************************************************************/
typedef struct
{
	const uint32_t   *pulCCBTbl;
	const char       *pchTblName;
} BVDC_P_FormatCCBTbl;

typedef struct
{
	uint32_t                    bPsAgc            : 1; /* Pseudo-Sync/AGC      */
	uint32_t                    bBp               : 1; /* Back Porch           */
	uint32_t                    bCs               : 1; /* Color-Stripes        */
	uint32_t                    bCycl             : 1; /* AGC Cyclic variation */
	uint32_t                    bHamp             : 1; /* H-sync amaplitude reduction outside VBI */
	uint32_t                    bRgb              : 1; /* RGB on/off */
} BVDC_P_MacrovisionCtrlBits;

typedef union
{
	struct
	{
		uint32_t                    bWidthTrim        : 1; /* 704-sample vs. 720-sample */
		uint32_t                    bCscAdjust        : 1; /* Colorspace change by user / src */
		uint32_t                    bTiming           : 1; /* New output timing format */
		uint32_t                    bRateChange       : 1; /* New rate change 59.94Hz vs 60.00Hz */
		uint32_t                    bAlignChange      : 1; /* new alignment settings */
		uint32_t                    bDacSetting       : 1; /* new dac settings */
		uint32_t                    bCcbAdjust        : 1;
		uint32_t                    bVfFilter         : 1; /* user VF filters */
		uint32_t                    bOutputMute       : 1; /* output Mute */
		uint32_t                    bAspRatio         : 1; /* aspect ratio might changed */
	} stBits;

	uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_Display_DirtyBits;

typedef struct
{
	uint32_t                    ulSavRemove;
	uint32_t                    ulSavReplicate;
	uint32_t                    ulEavPredict;
} BVDC_P_Display_ShaperSettings;

typedef struct
{
	uint32_t                    ulMin;
	uint32_t                    ulMax;
	BVDC_P_CscCoeffs            stCscCoeffs;
} BVDC_P_DisplayCscMatrix;

/* Digital trigger */
#define BVDC_P_DISPLAY_USED_DIGTRIG(eMasterTg)    \
		((eMasterTg == BVDC_DisplayTg_eStg0)   || \
		 (eMasterTg == BVDC_DisplayTg_eStg1)   || \
		 (eMasterTg == BVDC_DisplayTg_eDviDtg) || \
		 (eMasterTg == BVDC_DisplayTg_e656Dtg))

/* DVI trigger */
#define BVDC_P_DISPLAY_USED_DVI(eMasterTg)    \
	(eMasterTg == BVDC_DisplayTg_eDviDtg)

/* STG trigger transcoding path*/
#define BVDC_P_DISPLAY_USED_STG(eMasterTg)    \
		((eMasterTg == BVDC_DisplayTg_eStg0)   || \
		 (eMasterTg == BVDC_DisplayTg_eStg1))
/* Customer Fmt */
#define BVDC_P_IS_CUSTOMFMT(eVideoFmt)        \
	((eVideoFmt == BFMT_VideoFmt_eCustom0) || \
	(eVideoFmt == BFMT_VideoFmt_eCustom1)  || \
	(eVideoFmt == BFMT_VideoFmt_eCustom2))
/* Custom fmt comparison */
#define BVDC_P_IS_CUSTOMFMT_DIFF(pStgFmtInfo, pCustomFmtInfo)          \
	(((pStgFmtInfo) != NULL) && \
	 (BVDC_P_IS_CUSTOMFMT((pStgFmtInfo)->eVideoFmt)) &&                       \
	 (BVDC_P_IS_CUSTOMFMT((pCustomFmtInfo)->eVideoFmt)) &&                    \
	 (((pStgFmtInfo)->ulDigitalWidth  != (pCustomFmtInfo)->ulDigitalWidth)  ||\
	 ((pStgFmtInfo)->ulDigitalHeight != (pCustomFmtInfo)->ulDigitalHeight)  ||\
	 ((pStgFmtInfo)->ulVertFreq      != (pCustomFmtInfo)->ulVertFreq)))

#define BVDC_P_DISPLAY_NODELAY(pStgFmt, pFmt)        \
	    (((pStgFmt == NULL) || (pStgFmt == pFmt)) || \
	    (( pStgFmt != NULL) && \
	     BVDC_P_IS_CUSTOMFMT(pStgFmt->eVideoFmt) &&                       \
	     BVDC_P_IS_CUSTOMFMT(pFmt->eVideoFmt)))
#if BVDC_P_SUPPORT_DVPO
typedef struct BVDC_P_RateInfo
{
	/* Use for searching a matching one! */
	uint64_t                    ulPixelClkRate;
	uint32_t                    ulMDiv;
	uint32_t                    ulNDiv; /* Offset: 9.22 format */
	uint32_t                    ulRDiv;
	uint32_t                    ulSampleInc;
	uint32_t                    ulNumerator;
	uint32_t                    ulDenominator;
	uint32_t                    ulVcoRange;
	uint32_t                    ulLinkDivCtrl;
	uint32_t                    ulP2; /* pll feedback pre-divider */
	const char                 *pchRate;
} BVDC_P_RateInfo;

#elif BVDC_P_SUPPORT_DVI_65NM
typedef struct BVDC_P_RateInfo
{
	/* Use for searching a matching one! */
	uint64_t                    ulPixelClkRate;
	uint32_t                    ulDenominator;
	uint32_t                    ulNumerator;
	uint32_t                    ulSampleInc;
	uint32_t                    ulOffset;
	uint32_t                    ulShift;
	uint32_t                    ulRmDiv;
	uint32_t                    ulVcoRange;
	uint32_t                    ulPxDiv;
	uint32_t                    ulFeedbackPreDiv;
	uint32_t                    ulInputPreDiv;
	const char                 *pchRate;
} BVDC_P_RateInfo;
#else
typedef struct BVDC_P_RateInfo
{
	/* Use for searching a matching one! */
	uint64_t                    ulPixelClkRate;
	uint32_t                    ulMDiv;
	uint32_t                    ulPDiv;
	uint32_t                    ulNDiv;
	uint32_t                    ulRDiv;
	uint32_t                    ulSampleInc;
	uint32_t                    ulNumerator;
	uint32_t                    ulDenominator;
	uint32_t                    ulOpOffset;
	const char                 *pchRate;
} BVDC_P_RateInfo;
#endif

typedef struct
{
	BVDC_P_CmpColorSpace        eCmpColorSpace;
	BVDC_P_Output               eOutputColorSpace;
	BVDC_P_Output               eCoOutputColorSpace;      /* Component-only color space */
	BVDC_DacOutput              aDacOutput[BVDC_P_MAX_DACS];
	BAVC_MatrixCoefficients     eHdmiOutput;
	BVDC_MacrovisionType        eMacrovisionType;
	uint32_t                    ulDCSKeyLow;
	uint32_t                    ulDCSKeyHigh;
	BVDC_RfmOutput              eRfmOutput;
	const BFMT_VideoInfo       *pFmtInfo;
	BFMT_VideoInfo              stCustomFmt;
	uint32_t                    ulCheckSum;
	uint32_t                    ulVertFreq;

	/* User Dvo's CSC */
	bool                        bUserCsc;
	uint32_t                    ulUserShift;
	int32_t                     pl32_Matrix[BVDC_CSC_COEFF_COUNT];

	/* Misc shares bits for 656/Hdmi/Rfm */
	BVDC_P_VecPath              e656Vecpath;        /* 656 path output */
	BVDC_P_VecPath              eHdmiVecpath;       /* Prim/Sec, Bypass=off both */
	BVDC_P_VecPath              eRfmVecpath;        /* Prim/Sec, Bypass=off both */

	bool                        bFullRate;
	BAVC_Timebase               eTimeBase;          /* timebase for this display */
	BAVC_HDMI_PixelRepetition   eHdmiPixelRepetition; /* 2x or 4x */

	uint32_t                    uiNumDacsOn;        /* #Dacs enabled for this display */
	uint32_t                    ulRfmConst;         /* constant value to RF port */

	bool                        abEnableDac[BVDC_P_MAX_DACS];
	bool                        bSyncOnly;          /* hdmi output sync only no data */
	bool                        bEnableHdmi;
	bool                        bXvYcc;             /* hdmi XvYcc output */
	bool                        bEnable656;

	uint32_t                    aulHdmiDropLines[BFMT_VideoFmt_eMaxCount];

	/* MPAA decimation bit field */
	uint32_t                    aulEnableMpaaDeci[BVDC_MpaaDeciIf_eUnused];

	/* Generic callback */
	BVDC_CallbackFunc_isr       pfGenericCallback;
	void                       *pvGenericParm1;
	int                         iGenericParm2;
	BAVC_VdcDisplay_Info        stRateInfo;

	/* MV N0 control bits */
	BVDC_P_MacrovisionCtrlBits  stN0Bits;

	/* 704 vs. 720 samples */
	bool                        bWidthTrimmed;

	/* Force vec to run at 59.94Hz, 29.97Hz, etc. */
	BVDC_Mode                   eDropFrame;

	/* Color Correction parameters */
	bool                        bCCEnable;
	bool                        bUserCCTable;
	uint32_t                    ulGammaTableId;
	uint32_t                    ulColorTempId;
	uint32_t                    aulUserCCTable[BVDC_P_CCB_TABLE_SIZE];

	/* PR28836: DVO H/V/De sync polarity. */
	BVDC_Display_DvoSettings    stDvoCfg;

	/* Display alignment settings */
	BVDC_Display_Handle              hTargetDisplay;
	BVDC_Display_AlignmentSettings  stAlignCfg;

	/* color adjustment attributes */
	int32_t                     lDvoAttenuationR;
	int32_t                     lDvoAttenuationG;
	int32_t                     lDvoAttenuationB;
	int32_t                     lDvoOffsetR;
	int32_t                     lDvoOffsetG;
	int32_t                     lDvoOffsetB;

	bool                        abOutputMute[BVDC_DisplayOutput_e656 + 1];

	bool                        bMultiRateAllow;

	BVDC_3dSourceBufferSelect   e3dSrcBufSel;

	/* Rate manager attributes */
	const uint32_t             *pulAnalogRateTable;
	BAVC_VdcDisplay_Info        aAnalogRateInfo;
	const BVDC_P_RateInfo      *pHdmiRateInfo;

	/* display aspect ratio */
	BFMT_AspectRatio            eAspectRatio;
	uint32_t                    uiSampleAspectRatioX;
	uint16_t                    uiSampleAspectRatioY;
	BVDC_P_ClipRect             stAspRatRectClip;

	bool                        bBypassVideoProcess;
	BFMT_Orientation            eOrientation;

	/* user VF filters */
	bool                        abUserVfFilterCo[BVDC_P_VEC_CH_NUM];
	bool                        abUserVfFilterCvbs[BVDC_P_VEC_CH_NUM];
	uint32_t                    aaulUserVfFilterCo[BVDC_P_VEC_CH_NUM][BVDC_P_CHROMA_TABLE_SIZE];
	uint32_t                    aaulUserVfFilterCvbs[BVDC_P_VEC_CH_NUM][BVDC_P_CHROMA_TABLE_SIZE];

	/* dirty bits */
	BVDC_P_Display_DirtyBits    stDirty;
} BVDC_P_DisplayInfo;

typedef struct BVDC_P_DisplayContext
{
	BDBG_OBJECT(BVDC_DSP)

	BVDC_P_DisplayInfo          stNewInfo;           /* new(to-be apply) display info */
	BVDC_P_DisplayInfo          stCurInfo;           /* current(hw-applied) display info */

	BVDC_DisplayId              eId;                 /* might be different from cmp id */
	BVDC_P_State                eState;              /* Context state. */
	BVDC_Handle                 hVdc;                /* From which main VDC handle */
	BVDC_P_ItState              eItState;            /* Current Vec state */
	BVDC_Compositor_Handle      hCompositor;         /* Conntected to compositor */
	bool                        bUserAppliedChanges; /* New changes are ready */
	uint32_t                    ulRdcVarAddr;        /* Temp RDC var addr use for format change. */

	/* DVO, and Main CSC */
	BVDC_P_DisplayCscMatrix     stDvoCscMatrix;

	/* VF filters */
	const uint32_t             *apVfFilterCvbs[BVDC_P_VEC_CH_NUM];
	const uint32_t             *apVfFilterCo[BVDC_P_VEC_CH_NUM];

	/* Display path */
	BVDC_P_VecPath              eVecPath;
	BAVC_VbiPath                eVbiPath;
	BRDC_Trigger                eTopTrigger;
	BRDC_Trigger                eBotTrigger;
	BVDC_DisplayTg              eMasterTg;

    /* IT VEC status */
    uint32_t                    ulItLctrReg;

	/* into the other vec with similar cores */
	int32_t                     lItOffset;
	int32_t                     lVfOffset;
	int32_t                     lSmOffset;
	int32_t                     lCscOffset;
	int32_t                     lSrcOffset;
	int32_t                     lRmOffset;

	/* Event to nofify that changes has been applied to hardware. */
	BKNI_EventHandle            hAppliedDoneEvent;
	bool                        bSetEventPending;
	bool                        bRateManagerUpdatedPending;
	bool                        bRateManagerUpdated;

	const BFMT_VideoInfo        *pStgFmtInfo;          /* STG fmt setting, possibly buffer delay */
	/* vec phase adjustment */
	bool                        bVecPhaseInAdjust;

	/* MPAA decimation supported interface port mask */
	uint32_t                    aulMpaaDeciIfPortMask[BVDC_MpaaDeciIf_eUnused];

	/* used for delaying dvi input for a number of vsyncs as part of Bx software
	 * dvi reset workaround */
	uint32_t                    ulDviElapsedVsyncs;

	/* secret scratch register that holds the VBI encoder control setting */
	uint32_t                    ulVbiEncOffset;
	uint32_t                    ulScratchVbiEncControl;
	uint32_t                    ulPrevVbiEncControlValue;

	/* Same, but for ancillary 656 path */
	uint32_t                    ulVbiEncOffset_656;
	uint32_t                    ulScratchVbiEncControl_656;
	uint32_t                    ulPrevVbiEncControlValue_656;

	/* MV type change flag */
	bool                        bMvChange;
	BVDC_MacrovisionType        ePrevMvType;

	/* DCS state change indicator */
	/* 0 for no change */
	/* 1 for minor change (just enable/disable some VEC IT microcontrollers) */
	/* 2 for major change (new VEC microcode and register settings */
	int                         iDcsChange;
	/* Needed to maintain the above */
	uint32_t                    ulCurShadowDCSKeyLow;
	uint32_t                    ulNewShadowDCSKeyLow;
	/* TODO: I am about 90% convinced that I can collapse the above two
	 * variables into one. Need to examine program logic carefully first.
	 */

	/* Is this a bypass display? which means no VEC analog output. */
	bool                        bIsBypass;

	/* Internal VDC or App handed down. */
	BVDC_Heap_Handle            hHeap;

	/* Game mode tracking window */
	BVDC_Window_Handle          hWinGameMode;
	const uint32_t             *pRmTable;  /* VEC RM */
	const BVDC_P_RateInfo      *pDvoRmInfo;/* DVO RM */
	bool                        bRmAdjusted;

	/* alignment Timestamps */
	BTMR_TimerHandle            hTimer;
	BTMR_TimerRegisters         stTimerReg;
	uint32_t                    ulScratchTsAddr;
	uint32_t                    ulCurrentTs;
	BAVC_Polarity               eNextPolarity;
	uint32_t                    ulAlignSlaves;
	bool                        bAlignAdjusting;
	uint32_t                    ulTsSampleCount;
	uint32_t                    ulTsSamplePeriod;

	/* Dither */
	BVDC_P_DitherSetting        stCscDither;
	BVDC_P_DitherSetting        stDviDither;
	BVDC_P_DitherSetting        st656Dither;

	BVDC_Display_CallbackSettings   stCallbackSettings;
	BVDC_Display_CallbackData       stCallbackData;

	/* Count down for kick start (in vsync unit) */
	uint32_t                    ulKickStartDelay;

	/* display pixel aspect ratio */
	uintAR_t                    ulPxlAspRatio;     /* PxlAspR_int.PxlAspR_frac */
	uint32_t                    ulPxlAspRatio_x_y; /* PxlAspR_x<<16 | PxlAspR_y */

	/* Specific flavor of 480P output */
	bool                        bArib480p;

	/* Option to modify sync on 720P, 1080I, 1080P YPrPb video */
	bool                        bModifiedSync;

    /* For debug logs */
    uint32_t                    ulVsyncCnt;        /* Vysnc heartbeat */
} BVDC_P_DisplayContext;

/***************************************************************************
 * Private tables
 ***************************************************************************/
/* Always available */
extern const uint32_t   BVDC_P_aaulPrimaryDacTable[][BVDC_P_MAX_DACS];

#if BVDC_P_SUPPORT_SEC_VEC
extern const uint32_t   BVDC_P_aaulSecondaryDacTable[][BVDC_P_MAX_DACS];
#endif

#if BVDC_P_SUPPORT_TER_VEC
extern const uint32_t   BVDC_P_aaulTertiaryDacTable[][BVDC_P_MAX_DACS];
#endif


/***************************************************************************
 * Display private functions
 ***************************************************************************/
BERR_Code BVDC_P_Display_Create
	( BVDC_P_Context                  *pVdc,
	  BVDC_Display_Handle             *phDisplay,
	  BVDC_DisplayId                   eId);

void BVDC_P_Display_Destroy
	( BVDC_Display_Handle              hDisplay );

void BVDC_P_Display_Init
	( BVDC_Display_Handle              hDisplay );

void BVDC_P_Vec_BuildRul_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldPolarity );

BERR_Code BVDC_P_Display_ValidateChanges
	( BVDC_Display_Handle              ahDisplay[] );

void BVDC_P_Display_ApplyChanges_isr
	( BVDC_Display_Handle              hDisplay);

void BVDC_P_Display_AbortChanges
	( BVDC_Display_Handle              hDisplay);

bool BVDC_P_Display_FindDac_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DacOutput                   eDacOutput);

/* Helper functions. */
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG)
#define BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(idx) (1 << (BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE - 1 - (idx % BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE)))
#else
#define BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(idx) (1 << idx % BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE)
#endif

#define BVDC_P_DISPLAY_IS_BIT_DIRTY(pDirty, iIdx)    ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] & BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx))
#define BVDC_P_DISPLAY_CLEAR_DIRTY_BIT(pDirty, iIdx) ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] &= ~(BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx)))
#define BVDC_P_DISPLAY_SET_DIRTY_BIT(pDirty, iIdx)   ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] |= BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx))

uint32_t BVDC_P_GetPosSyncValue_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  uint32_t                       **ppulRul,
	  BVDC_P_VecPath                   eVecPath );
uint32_t BVDC_P_GetCoPosSyncValue_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  uint32_t                       **ppulRul,
	  BVDC_P_VecPath                   eVecPath );
#if DCS_SUPPORT
uint32_t BVDC_P_GetPosSyncValueDcs_isr
(
	BFMT_VideoFmt eVideoFmt,
	BVDC_P_Output eCoOutputColorSpace,
	uint32_t ulDCSKeyLow
);
uint32_t BVDC_P_GetCoPosSyncValueDcs_isr
(
	BFMT_VideoFmt eVideoFmt,
	BVDC_P_Output eCoOutputColorSpace,
	uint32_t ulDCSKeyLow
);
#endif

BERR_Code BVDC_P_ChangeMvType_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  uint32_t                       **ppulRul );

typedef enum {
	BVDC_P_PROT_Alt_None = 0,
	BVDC_P_PROT_ALT_MV,
	BVDC_P_PROT_ALT_DCS
} BVDC_P_Prot_Alt;
BVDC_P_Prot_Alt BVDC_P_Get_Prot_Alt_isr (void);

uint32_t BVDC_P_Display_Modify_SYNC_TRANS_0_isr (
	BVDC_P_DisplayInfo               *pNewInfo,
	uint32_t                         ulVfSTRegVal );

uint32_t BVDC_P_Macrovision_GetNegSyncValue_isr
	( BVDC_P_DisplayInfo              *pDispInfo,
	  BVDC_P_Output                    eOutputColorSpace,
	  bool                             bDacOutput_Green_NoSync );

uint32_t BVDC_P_GetFmtAdderValue_isr
	( BVDC_P_DisplayInfo              *pDispInfo );

BERR_Code BVDC_P_ValidateMacrovision
	( BVDC_P_DisplayContext           *pDisplay );

void BVDC_P_Vec_Build_SyncPCL_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  BVDC_P_ListInfo                 *pList );

void BVDC_P_Vec_Update_OutMuxes_isr
	( BVDC_P_Context                  *pVdc );

void BVDC_P_Display_EnableTriggers_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  bool                             bEnable );

const BVDC_P_FormatCCBTbl* BVDC_P_Display_GetCCBTable
	( const uint32_t                   ulGammaId,
	  const uint32_t                   ulColorTempId );

void BVDC_P_Display_GetMaxTable
	( uint32_t                        *pulGammaTable,
	  uint32_t                        *pulColorTempTable );

void BVDC_P_Vec_BuildCCBRul_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList);

void BVDC_P_ResetVec
	( BVDC_P_Context                  *pVdc );

BERR_Code BVDC_P_Display_GetRasterLineNumber
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                        *pulRasterLineNumber );

BERR_Code BVDC_P_Display_GetFieldPolarity_isr
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                       **ppulRulCur,
	  BAVC_Polarity                    eFieldPolarity );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_DISPLAY_PRIV_H__ */
/* End of file. */
