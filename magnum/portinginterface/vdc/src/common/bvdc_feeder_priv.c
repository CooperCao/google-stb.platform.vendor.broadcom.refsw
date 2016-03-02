/***************************************************************************
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brdc.h"
#include "bsur.h"
#include "bmma.h"
#include "bvdc.h"
#include "bvdc_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_vnet_priv.h"

#include "bchp_mfd_0.h"
#include "bchp_vfd_0.h"
#include "bchp_fmisc.h"
#include "bchp_timer.h"
#include "bchp_mmisc.h"
BDBG_MODULE(BVDC_VFD);
BDBG_OBJECT_ID(BVDC_FDR);

/* HW7422-776: MFD stalls when switched from AVC format to fixed color mode */
#if (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_10)
#define BVDC_P_MFD_FIXED_COLOR_WORKAROUND    (1)
#else
#define BVDC_P_MFD_FIXED_COLOR_WORKAROUND    (0)
#endif

#define BVDC_P_MFD_ENABLE_SKIP_FIRST_LINE    (0)

/****************************************************************************/
/* Making an entry for stripe width configurations                          */
/****************************************************************************/
#define BVDC_P_FEEDER_MAKE_STRIPE_WIDTH(eStripeWidth, ulStripeWidth, ulShift) \
{                                                                           \
	(BAVC_StripeWidth_##eStripeWidth),                                      \
	(ulStripeWidth),                                                        \
	(ulShift)                                                                \
}

/* Used for MPG 10bit */
#define BVDC_P_FEEDER_GET_ONE_THIRD(data)   (((data) * 21846) >> 16)

/* Stripe width config table. */
static const BVDC_P_Feeder_StripeWidthConfig s_aStripeWidthCfgTbl[] =
{
	/*                             eStripeWidth   ulStripeWidth, ulShift */
	BVDC_P_FEEDER_MAKE_STRIPE_WIDTH(e64Byte,      64,            6       ),
	BVDC_P_FEEDER_MAKE_STRIPE_WIDTH(e128Byte,     128,           7       ),
	BVDC_P_FEEDER_MAKE_STRIPE_WIDTH(e256Byte,     256,           8       )
};

#define BVDC_P_FEEDER_STRIPE_WIDTH_CONFIG_TABLE_CNT       \
	(sizeof(s_aStripeWidthCfgTbl) / sizeof(BVDC_P_Feeder_StripeWidthConfig))

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
#if (BVDC_P_SUPPORT_MFD_VER < BVDC_P_MFD_VER_16)
static const BVDC_P_Feeder_VideoFormatMode s_hwOrientation[] =
{
	BVDC_P_Feeder_VideoFormatMode_e2D,  /* BFMT_Orientation_e2D */
	BVDC_P_Feeder_VideoFormatMode_e3DLR, /* BFMT_Orientation_e3D_LeftRight */
	BVDC_P_Feeder_VideoFormatMode_e3DOU, /* BFMT_Orientation_e3D_OverUnder */
	BVDC_P_Feeder_VideoFormatMode_e3DDP, /* BFMT_Orientation_e3D_Left */
	BVDC_P_Feeder_VideoFormatMode_e3DDP, /* BFMT_Orientation_e3D_Right */
	BVDC_P_Feeder_VideoFormatMode_e3DMR  /* BFMT_Orientation_eLeftRight_Enhanced */
};
#endif
#endif

/***************************************************************************
 * Static functions
 ***************************************************************************/
static void BVDC_P_Feeder_SetFixedColor_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BAVC_Polarity                    ePolarity,
	  const uint32_t                   ulMuteColorYCrCb );

static BERR_Code BVDC_P_Feeder_SetVertWindow_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_Polarity              eSourcePolarity,
	  const uint32_t                   ulTop,
	  const uint32_t                   ulHeight );

#if (BVDC_P_MFD_SUPPORT_CSC)
static void BVDC_P_Feeder_SetCsc_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData );
#endif

#if BVDC_P_SUPPORT_MTG
static void BVDC_P_Feeder_BuildMtgRul_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  bool                             bInterlaced,
	  BAVC_FrameRateCode               eFrameRateCode );
#endif

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Feeder_Create
	( BVDC_P_Feeder_Handle            *phFeeder,
	  BRDC_Handle                      hRdc,
	  const BREG_Handle                hReg,
	  const BVDC_P_FeederId            eFeederId,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
	  BTMR_TimerHandle                 hTimer,
#endif
	  BVDC_Source_Handle               hSource,
	  BVDC_P_Resource_Handle           hResource,
	  bool                             b3dSrc )
{
	BERR_Code              err = BERR_SUCCESS;
	BVDC_P_FeederContext  *pFeeder;

	BDBG_ENTER(BVDC_P_Feeder_Create);

	/* check parameters */
	if(eFeederId >= BVDC_P_FeederId_eUnknown)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Alloc the window context. */
	pFeeder = (BVDC_P_FeederContext*)(BKNI_Malloc(sizeof(BVDC_P_FeederContext)));
	if(!pFeeder)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Clear out the context and set defaults. */
	BKNI_Memset((void*)pFeeder, 0x0, sizeof(BVDC_P_FeederContext));
	BDBG_OBJECT_SET(pFeeder, BVDC_FDR);

	/* Set up and initialization. */
	pFeeder->eId                   = eFeederId;
	pFeeder->eImageFormat          = BVDC_P_Feeder_ImageFormat_ePacked;
	pFeeder->ePxlFormat            = BPXL_INVALID;
	pFeeder->eInputOrientation     = BFMT_Orientation_e2D;
	pFeeder->eOutputOrientation    = BFMT_Orientation_e2D;
	pFeeder->hRdc                  = hRdc;
	pFeeder->hRegister             = hReg;
	pFeeder->hSource               = hSource;
	pFeeder->ulTimestamp           = 0;
	pFeeder->eCrcType              = BVDC_Source_CrcType_eCrc32;
#if (!BVDC_P_USE_RDC_TIMESTAMP)
	pFeeder->hTimer                = hTimer;
	BTMR_GetTimerRegisters(pFeeder->hTimer, &pFeeder->stTimerReg);
#endif

	/* Feeder reset address */
#if BVDC_P_SUPPORT_NEW_SW_INIT
	pFeeder->ulResetRegAddr = BCHP_FMISC_SW_INIT;
#else
	pFeeder->ulResetRegAddr = BCHP_FMISC_SOFT_RESET;
#endif

	switch( eFeederId )
	{
	case BVDC_P_FeederId_eMfd0:
		pFeeder->ulRegOffset = 0;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_0_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_0_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;

	case BVDC_P_FeederId_eVfd0:
		pFeeder->ulRegOffset = BCHP_VFD_0_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_0_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_0_MASK;
#endif
		break;

#if BCHP_VFD_1_REG_START
	case BVDC_P_FeederId_eVfd1:
		pFeeder->ulRegOffset = BCHP_VFD_1_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_1_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_1_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_1_MASK;
#endif
		break;
#endif

#if BCHP_VFD_2_REG_START
	case BVDC_P_FeederId_eVfd2:
		pFeeder->ulRegOffset = BCHP_VFD_2_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_2_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_2_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_2_MASK;
#endif
		break;
#endif

#if BCHP_MFD_1_REG_START
	case BVDC_P_FeederId_eMfd1:
		pFeeder->ulRegOffset = BCHP_MFD_1_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_1_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_1_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;
#endif

#if BCHP_MFD_2_REG_START
	case BVDC_P_FeederId_eMfd2:
		pFeeder->ulRegOffset = BCHP_MFD_2_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_2_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_2_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;
#endif

#if BCHP_MFD_3_REG_START
	case BVDC_P_FeederId_eMfd3:
		pFeeder->ulRegOffset = BCHP_MFD_3_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_3_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_3_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;
#endif

#if BCHP_MFD_4_REG_START
	case BVDC_P_FeederId_eMfd4:
		pFeeder->ulRegOffset = BCHP_MFD_4_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_4_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_4_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;
#endif

#if BCHP_MFD_5_REG_START
	case BVDC_P_FeederId_eMfd5:
		pFeeder->ulRegOffset = BCHP_MFD_5_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = 0;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_MFD_5_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_MFD_5_MASK;
#endif
		pFeeder->eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		break;
#endif

#if BCHP_VFD_3_REG_START
	case BVDC_P_FeederId_eVfd3:
		pFeeder->ulRegOffset = BCHP_VFD_3_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_3_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_3_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_3_MASK;
#endif
		break;
#endif

#if BCHP_VFD_4_REG_START
	case BVDC_P_FeederId_eVfd4:
		pFeeder->ulRegOffset = BCHP_VFD_4_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_4_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_4_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_4_MASK;
#endif
		break;
#endif

#if BCHP_VFD_5_REG_START
	case BVDC_P_FeederId_eVfd5:
		pFeeder->ulRegOffset = BCHP_VFD_5_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_5_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_5_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_5_MASK;
#endif
		break;
#endif

#if BCHP_VFD_6_REG_START
	case BVDC_P_FeederId_eVfd6:
		pFeeder->ulRegOffset = BCHP_VFD_6_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_6_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_6_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_6_MASK;
#endif
		break;
#endif

#if BCHP_VFD_7_REG_START
	case BVDC_P_FeederId_eVfd7:
		pFeeder->ulRegOffset = BCHP_VFD_7_REG_START - BCHP_MFD_0_REG_START;
		pFeeder->ulVfd0RegOffset = BCHP_VFD_7_REG_START - BCHP_VFD_0_REG_START;

#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulResetMask = BCHP_FMISC_SW_INIT_VFD_7_MASK;
#else
		pFeeder->ulResetMask = BCHP_FMISC_SOFT_RESET_VFD_7_MASK;
#endif
		break;
#endif

	default:
			BDBG_ASSERT(false);
	}

#if BVDC_P_SUPPORT_NEW_SW_INIT
	pFeeder->ulVnetResetAddr = BCHP_MMISC_VNET_F_CHANNEL_SW_INIT;
#elif BCHP_MMISC_VNET_F_CHANNEL_RESET
	pFeeder->ulVnetResetAddr = BCHP_MMISC_VNET_F_CHANNEL_RESET;
#endif
	if(BVDC_P_Feeder_IS_MPEG(pFeeder))
	{
#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulVnetResetMask = BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_0_MASK<<(eFeederId - BVDC_P_FeederId_eMfd0);
#elif BCHP_MMISC_VNET_F_CHANNEL_RESET
		pFeeder->ulVnetResetMask = BCHP_MMISC_VNET_F_CHANNEL_RESET_MFD_0_RESET_MASK<<(eFeederId - BVDC_P_FeederId_eMfd0);
#endif
	}
	else
	{
#if BVDC_P_SUPPORT_NEW_SW_INIT
		pFeeder->ulVnetResetMask = BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_0_MASK<<(eFeederId - BVDC_P_FeederId_eVfd0);
#elif BCHP_MMISC_VNET_F_CHANNEL_RESET
		pFeeder->ulVnetResetMask = BCHP_MMISC_VNET_F_CHANNEL_RESET_VFD_0_RESET_MASK<<(eFeederId - BVDC_P_FeederId_eVfd0);
#endif
	}
#if (BVDC_P_MFD_NEED_CRC_WORKAROUND)
	/* MPEG feeder only */
	if(BVDC_P_Feeder_IS_MPEG(pFeeder))
	{
		pFeeder->ulLumaCrcRegAddr = BRDC_AllocScratchReg(pFeeder->hRdc);
		if(!pFeeder->ulLumaCrcRegAddr)
		{
			BDBG_ERR(("Not enough scratch registers for Luma Crc!"));
			BDBG_OBJECT_DESTROY(pFeeder, BVDC_FDR);
			BKNI_Free((void*)pFeeder);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

		pFeeder->ulChromaCrcRegAddr = BRDC_AllocScratchReg(pFeeder->hRdc);
		if(!pFeeder->ulChromaCrcRegAddr)
		{
			BDBG_ERR(("Not enough scratch registers for Chroma Crc!"));
			BDBG_OBJECT_DESTROY(pFeeder, BVDC_FDR);
			BKNI_Free((void*)pFeeder);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
	}
#endif
	/* init the SubRul sub-module */
	BVDC_P_SubRul_Init(&(pFeeder->SubRul), 0 /* 0 means it has no src mux */,
		BVDC_P_Feeder_PostMuxValue(pFeeder), BVDC_P_DrainMode_eFront, 0, hResource);

	/* for VFD, hFeeder->hSource is NOT NULL iff hFeeder is created by
	 * BVDC_P_Source_Create */
	if(BVDC_P_Feeder_IS_MPEG(pFeeder) ||
	   (BVDC_P_Feeder_IS_VFD(pFeeder) && pFeeder->hSource))
	{
		uint32_t ulHwConfig;

		/* init stuff in surface manager */
		pFeeder->stGfxSurface.hRegister = hReg;
		pFeeder->stGfxSurface.eSrcId =  BVDC_P_Feeder_IS_MPEG(pFeeder)?
			(BAVC_SourceId_eMpeg0 + (eFeederId - BVDC_P_FeederId_eMfd0)):
			(BAVC_SourceId_eVfd0 + (eFeederId - BVDC_P_FeederId_eVfd0));
		pFeeder->stGfxSurface.ulHwMainSurAddrReg =
			BCHP_MFD_0_PICTURE0_LINE_ADDR_0 + pFeeder->ulRegOffset;

		/* allocate shadow registers for surface manager from scratch pool */
		BVDC_P_GfxSurface_AllocShadowRegs(
			&pFeeder->stGfxSurface, pFeeder->hRdc, b3dSrc);
		BDBG_MSG(("alloc regs for feeder %d", pFeeder->eId));

#if BVDC_P_SUPPORT_MTG
		/* Assume MTG is present on MFD < BVDC_P_SUPPORT_MTG only.  Otherwise
		 * if HW_CONFIGURATION is available we'll use it to read out. */
		pFeeder->hSource->bMtgIsPresent =
			((pFeeder->hSource->eId - BAVC_SourceId_eMpeg0) < BVDC_P_SUPPORT_MTG);
#endif

		/* read the HW_CONFIGURATION */
#ifdef BCHP_MFD_0_HW_CONFIGURATION
		ulHwConfig = BREG_Read32(pFeeder->hRegister, BCHP_MFD_0_HW_CONFIGURATION + pFeeder->ulRegOffset);
		pFeeder->hSource->bIs10BitCore  = BVDC_P_COMPARE_FIELD_DATA(ulHwConfig, MFD_0_HW_CONFIGURATION, CORE_BVB_WIDTH_10, 1);
#ifdef BCHP_MFD_0_HW_CONFIGURATION_SUPPORTS_MFD_TRIGGER_DEFAULT
		pFeeder->hSource->bMtgIsPresent = BVDC_P_COMPARE_FIELD_DATA(ulHwConfig, MFD_0_HW_CONFIGURATION, MFD_TRIGGER, 1);
#endif
#else
		BSTD_UNUSED(ulHwConfig);
#endif
	}

	/* All done. now return the new fresh context to user. */
	*phFeeder = (BVDC_P_Feeder_Handle)pFeeder;

	BDBG_LEAVE(BVDC_P_Feeder_Create);
	return err;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_Destroy
	( BVDC_P_Feeder_Handle             hFeeder )
{
	BDBG_ENTER(BVDC_P_Feeder_Destroy);
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

#if (BVDC_P_MFD_NEED_CRC_WORKAROUND)
	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		BRDC_FreeScratchReg(hFeeder->hRdc, hFeeder->ulLumaCrcRegAddr);
		BRDC_FreeScratchReg(hFeeder->hRdc, hFeeder->ulChromaCrcRegAddr);
	}
#endif

	/* for VFD, hFeeder->hSource is NOT NULL iff hFeeder is created by
	 * BVDC_P_Source_Create */
	if(BVDC_P_Feeder_IS_MPEG(hFeeder) ||
	   (BVDC_P_Feeder_IS_VFD(hFeeder) && hFeeder->hSource))
	{
		/* free surface address shadow registers back to scratch pool */
		BVDC_P_GfxSurface_FreeShadowRegs(
			&hFeeder->stGfxSurface, hFeeder->hRdc);
	}

	BDBG_OBJECT_DESTROY(hFeeder, BVDC_FDR);
	BKNI_Free((void*)hFeeder);

	BDBG_LEAVE(BVDC_P_Feeder_Destroy);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_Init
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_Window_Handle               hWindow,
	  BMMA_Heap_Handle                 hMem,
	  bool                             bGfxSrc,
	  bool                             bMtgSrc )
{
	BVDC_P_PictureNode *pPicture;

	BDBG_ENTER(BVDC_P_Feeder_Init);
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

	/* Clear out shadow registers. */
	BKNI_Memset((void*)hFeeder->aulRegs, 0x0, sizeof(hFeeder->aulRegs));

	/* Assign the new heap */
	hFeeder->hMem = hMem;
	hFeeder->hWindow = hWindow;
	hFeeder->stUpSampler.bUnbiasedRound = false;
	hFeeder->stUpSampler.eFilterType    = BVDC_422To444Filter_eTenTaps;
	hFeeder->stUpSampler.eRingRemoval   = BVDC_RingSuppressionMode_eDisable;
	hFeeder->ulThroughput = 0;
#if (BVDC_P_MFD_ENABLE_SKIP_FIRST_LINE)
	hFeeder->bSkipFirstLine = true;
#else
	hFeeder->bSkipFirstLine = false;
#endif
	hFeeder->ulPicOffset = 0;
	hFeeder->ulPicOffset_R = 0;

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		/* set init flag for MFD only */
		hFeeder->bInitial = true;
		hFeeder->bMtgSrc = bMtgSrc;

		/* enable CRC, in display mode and load seed on every vsync
		 * TODO: support N-field CRC checking; */
		BVDC_P_MFD_GET_REG_DATA(MFD_0_CRC_CTRL ) =
			BCHP_FIELD_ENUM(MFD_0_CRC_CTRL , ENABLE, ON)    |
#if (BVDC_P_MFD_SUPPORT_CRC_TYPE)
			BCHP_FIELD_DATA(MFD_0_CRC_CTRL , TYPE, hFeeder->hSource->stCurInfo.eCrcType) |
#endif
			BCHP_FIELD_ENUM(MFD_0_CRC_CTRL , MODE, DISPLAY) |
			BCHP_FIELD_ENUM(MFD_0_CRC_CTRL , LOAD_CRC_SEED, AT_SOF);

		/* Initial seed value should be all '1's */
		BVDC_P_MFD_GET_REG_DATA(MFD_0_CRC_SEED) = 0xFFFFFFFF;

#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_3) && (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_5)
		BVDC_P_MFD_GET_REG_DATA(MFD_0_US_422_TO_444) =
			BCHP_FIELD_ENUM(MFD_0_US_422_TO_444, UNBIASED_ROUND_ENABLE, DISABLE)	|
			BCHP_FIELD_ENUM(MFD_0_US_422_TO_444, FILT_CTRL, CHROMA_DUPLICATION);

		BVDC_P_MFD_GET_REG_DATA(MFD_0_CSC_CNTL) =
			BCHP_FIELD_ENUM(MFD_0_CSC_CNTL, CSC_ENABLE, OFF);

		BVDC_P_MFD_GET_REG_DATA(MFD_0_DS_CNTL) =
			BCHP_FIELD_ENUM(MFD_0_DS_CNTL, RING_SUPPRESSION, DISABLE)	|
			BCHP_FIELD_ENUM(MFD_0_DS_CNTL, FILTER_TYPE, FILTER_TYPE_DECIMATE);
#endif

	}
	else if(BVDC_P_Feeder_IS_VFD(hFeeder))
	{
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
		uint32_t  ulReg;
		ulReg = BREG_Read32(hFeeder->hRegister, BCHP_VFD_0_HW_CONFIGURATION + hFeeder->ulVfd0RegOffset);
		hFeeder->bSupportDcxm = BCHP_GET_FIELD_DATA(ulReg, VFD_0_HW_CONFIGURATION, DCD);
#else
		hFeeder->bSupportDcxm = false;
#endif
	}

#if (BVDC_P_MFD_VER_15 == BVDC_P_SUPPORT_MFD_VER)
	BVDC_P_MFD_GET_REG_DATA(MFD_0_TEST_MODE_CNTL) =
		BCHP_FIELD_DATA(MFD_0_TEST_MODE_CNTL, DISABLE_DOUBLE_BUFFER, 1);
#else
	BVDC_P_MFD_GET_REG_DATA(MFD_0_TEST_MODE_CNTL) = 0x0;
#endif

	hFeeder->bGfxSrc = bGfxSrc;
	if(bGfxSrc)
	{
		/* Init picture node used when VFD is used to input gfx surface and
		 * works as an intial source of a video window. MFD only uses
		 * pPicture->eSrcPolarity in BVDC_Feeder_BuildRulGfxSurAddr_isr */
		pPicture = &hFeeder->stPicture;

		BKNI_Memset((void*)pPicture, 0x0, sizeof(BVDC_P_PictureNode));

		pPicture->pHeapNode            = NULL;
		BKNI_Memset((void*)&pPicture->stFlags, 0x0, sizeof(BVDC_P_PicNodeFlags));
		pPicture->stFlags.bMute           = true;
		pPicture->stFlags.bCadMatching    = false;

		BVDC_P_CLEAN_ALL_DIRTY(&pPicture->stVnetMode);
		pPicture->stVnetMode.stBits.bInvalid  = BVDC_P_ON;
		pPicture->eFrameRateCode       = BAVC_FrameRateCode_e29_97;
		pPicture->eMatrixCoefficients  = BAVC_MatrixCoefficients_eSmpte_170M;
		pPicture->eDisplayPolarity     = BAVC_Polarity_eTopField;
		pPicture->eSrcPolarity         = BAVC_Polarity_eTopField;
		pPicture->PicComRulInfo.eSrcOrigPolarity = BAVC_Polarity_eTopField;
		pPicture->eDstPolarity         = BAVC_Polarity_eTopField;
		pPicture->pSurface             = NULL;
		pPicture->pSurface_R           = NULL;

		pPicture->stSrcOut.lLeft       = 0;
		pPicture->stSrcOut.lLeft_R     = 0;
		pPicture->stSrcOut.lTop        = 0;
		pPicture->stSrcOut.ulWidth     = BFMT_NTSC_WIDTH;
		pPicture->stSrcOut.ulHeight    = BFMT_NTSC_HEIGHT;

		pPicture->stSclOut             = pPicture->stSrcOut;
		pPicture->stWinOut             = pPicture->stSrcOut;
		pPicture->stSclCut             = pPicture->stSrcOut;
		pPicture->stVfdOut             = pPicture->stSrcOut;

		pPicture->pSrcOut              = &pPicture->stSrcOut;

		pPicture->pSclIn               = &pPicture->stSrcOut;
		pPicture->pSclOut              = &pPicture->stSclOut;
		pPicture->pWinOut              = &pPicture->stWinOut;

		pPicture->pVfdIn               = &pPicture->stSrcOut;
		pPicture->pVfdOut              = &pPicture->stSrcOut;

		/* init surface manager */
		BVDC_P_GfxSurface_Init(&hFeeder->stGfxSurface);
	}

	BDBG_LEAVE(BVDC_P_Feeder_Init);
	return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Feeder_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Feeder_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Feeder_BuildRul_DrainVnet_isr(
	BVDC_P_Feeder_Handle           hFeeder,
	BVDC_P_ListInfo               *pList,
	bool                           bNoCoreReset)
{

	if(hFeeder->eId < BVDC_P_FeederId_eVfd0)
	{
		/* Need to re-enable CRC for MFD */
		hFeeder->bInitial = true;
	}

#if (BVDC_P_SUPPORT_MFD_VER < BVDC_P_MFD_VER_14)
	BVDC_P_SubRul_Drain_isr(&(hFeeder->SubRul), pList,
		hFeeder->ulResetRegAddr, hFeeder->ulResetMask,
		hFeeder->ulVnetResetAddr, hFeeder->ulVnetResetMask);
	BSTD_UNUSED(bNoCoreReset);
#else
	/* reset sub and connect the module to a drain */
	BVDC_P_SubRul_Drain_isr(&(hFeeder->SubRul), pList,
		0, 0,
		bNoCoreReset? 0: hFeeder->ulVnetResetAddr,
		bNoCoreReset? 0: hFeeder->ulVnetResetMask);
#endif

#if BVDC_P_SUPPORT_MTG
	if(BVDC_P_Feeder_IS_MPEG(hFeeder) && hFeeder->bMtgSrc)
	{
		/* resetting has killed mtg, re-enable it now */
		BVDC_P_Feeder_BuildMtgRul_isr(hFeeder, pList, hFeeder->bMtgInterlaced, hFeeder->eMtgFrameRateCode);
	}
#endif
}

static void BVDC_Feeder_BuildRulGfxSurAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList );

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Feeder_BuildRulMfd_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_3) && (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_5)

	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_RANGE_EXP_REMAP_CNTL, MFD_0_DS_CNTL,
		pList->pulCurrent);

#elif (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_6)
	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_BYTE_ORDER,
		MFD_0_CHROMA_REPOSITION_DERING_ENABLE, pList->pulCurrent);

	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_DATA_MODE, MFD_0_RANGE_EXP_REMAP_CNTL,
		pList->pulCurrent);

#elif (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_9)
	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_PICTURE0_LINE_ADDR_1,
		MFD_0_CHROMA_NMBY, pList->pulCurrent);

	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_CHROMA_REPOSITION_DERING_ENABLE, pList->pulCurrent);
	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_RANGE_EXP_REMAP_CNTL, pList->pulCurrent);

#elif (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_12)
	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_PICTURE0_LINE_ADDR_1,
		MFD_0_CHROMA_NMBY, pList->pulCurrent);

	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_CHROMA_REPOSITION_DERING_ENABLE, pList->pulCurrent);
	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_RANGE_EXP_REMAP_CNTL, pList->pulCurrent);
	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_PICTURE0_LINE_ADDR_0_R,
		MFD_0_PICTURE0_LINE_ADDR_1_R, pList->pulCurrent);

#elif (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_13) && (BVDC_P_MFD_VER_15 >= BVDC_P_SUPPORT_MFD_VER)
	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_PICTURE0_LINE_ADDR_1,
		MFD_0_CHROMA_NMBY, pList->pulCurrent);

	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_CHROMA_REPOSITION_DERING_ENABLE, pList->pulCurrent);
	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_RANGE_EXP_REMAP_CNTL, pList->pulCurrent);

	BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_PICTURE0_LINE_ADDR_0_R,
		MFD_0_PICTURE0_LINE_ADDR_1_R, pList->pulCurrent);
#endif

#if (BVDC_P_MFD_NEED_CRC_WORKAROUND)
	/* Read and store CRC values. Need to read before feeder is enabled */
	*pList->pulCurrent++ = BRDC_OP_REG_TO_REG( 1 );
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_LUMA_CRC + hFeeder->ulRegOffset);
	*pList->pulCurrent++ = BRDC_REGISTER(hFeeder->ulLumaCrcRegAddr);

	*pList->pulCurrent++ = BRDC_OP_REG_TO_REG( 1 );
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_CHROMA_CRC + hFeeder->ulRegOffset);
	*pList->pulCurrent++ = BRDC_REGISTER(hFeeder->ulChromaCrcRegAddr);

#endif

	BVDC_P_MFD_WRITE_TO_RUL(MFD_0_TEST_MODE_CNTL, pList->pulCurrent);

	if (hFeeder->bGfxSrc && hFeeder->stMfdPicture.ePxlFmt != BPXL_INVALID)
	{
#if (BVDC_P_SUPPORT_MFD_VER<=BVDC_P_MFD_VER_6)
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_FEEDER_CNTL, MFD_0_CHROMA_NMBY,
			pList->pulCurrent);
#else
#if (BVDC_P_MFD_VER_16 <= BVDC_P_SUPPORT_MFD_VER)
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_FEEDER_CNTL, MFD_0_COMP_ORDER,
					pList->pulCurrent);
#else
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_FEEDER_CNTL, MFD_0_BYTE_ORDER,
			pList->pulCurrent);
#endif
#endif

		BVDC_Feeder_BuildRulGfxSurAddr_isr(hFeeder, pList);

		BVDC_P_MFD_WRITE_TO_RUL(MFD_0_PIC_FEED_CMD, pList->pulCurrent);
	}
	else
	{
		BVDC_Feeder_BuildRulGfxSurAddr_isr(hFeeder, pList);
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(MFD_0_FEEDER_CNTL, MFD_0_PIC_FEED_CMD,
			pList->pulCurrent);
	}

	return;
}

#if BVDC_P_SUPPORT_MTG

/* INDEX: By source id, do not put ifdefs and nested ifdefs around these that
* become impossible to decipher.  The eSourceId != BVDC_P_NULL_SOURCE will
* indicate that this source's trigger (lost) is to be monitor or poll by
* display interrupt. */
static const BVDC_P_SourceParams s_aMtgParams[] =
{
	/* Mpeg feeder 0/1/2/3/4/5 */
	BVDC_P_MAKE_SRC_PARAMS(eMpeg0, eMfd0Mtg0, eMfd0Mtg1),
	BVDC_P_MAKE_SRC_PARAMS(eMpeg1, eMfd1Mtg0, eMfd1Mtg1),
	BVDC_P_MAKE_SRC_PARAMS(eMpeg2, eMfd2Mtg0, eMfd2Mtg1),
	BVDC_P_MAKE_SRC_PARAMS(eMpeg3, eMfd3Mtg0, eMfd3Mtg1),
	BVDC_P_MAKE_SRC_PARAMS(eMpeg4, eMfd4Mtg0, eMfd4Mtg1),
	BVDC_P_MAKE_SRC_PARAMS(eMpeg5, eMfd5Mtg0, eMfd5Mtg1),
};

#define BVDC_P_27MHZ   27000000
/***************************************************************************
 * called during init and when bInterlaced / picFrq / timeBase changed
 */
static void BVDC_P_Feeder_BuildMtgRul_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  bool                             bInterlaced,
	  BAVC_FrameRateCode               eFrameRateCode )
{
	/* XXX TODO: decide timebase */
	uint32_t  ulMtgRefreshRate, ulFrameSize;
	uint32_t  ulTimeBase = BCHP_FIELD_ENUM(MFD_0_MTG_CONTROL, TIMEBASE_SEL, TIMEBASE_0);
	uint32_t  ulScanMode = (bInterlaced) ?
		BCHP_FIELD_ENUM(MFD_0_MTG_CONTROL, SCAN_MODE, IS_INTERLACED) :
		BCHP_FIELD_ENUM(MFD_0_MTG_CONTROL, SCAN_MODE, IS_PROGRESSIVE);

	ulMtgRefreshRate = BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe(eFrameRateCode);
	ulFrameSize = ((uint32_t)BVDC_P_27MHZ * (uint32_t)BFMT_FREQ_FACTOR) / ulMtgRefreshRate;

	/* reset 1st to prepare for new config */
	BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MFD_0_MTG_CONTROL, hFeeder->ulRegOffset,
		BCHP_FIELD_ENUM(MFD_0_MTG_CONTROL, ENABLE, OFF));

	/* new config */
	BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MFD_0_MTG_FRAME_SIZE, hFeeder->ulRegOffset,
		BCHP_FIELD_DATA(MFD_0_MTG_FRAME_SIZE, FRAME_SIZE, ulFrameSize));

	BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MFD_0_MTG_CONTROL, hFeeder->ulRegOffset,
		ulTimeBase | ulScanMode |
		BCHP_FIELD_ENUM(MFD_0_MTG_CONTROL, ENABLE, ON));

	hFeeder->bMtgInterlaced = bInterlaced;
	hFeeder->eMtgFrameRateCode = eFrameRateCode;
}

/***************************************************************************
 *
 */
void BVDC_P_Feeder_Mtg_Bringup_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	BRDC_List_Handle hList;
	BRDC_Slot_Handle hSlot;
	BVDC_P_ListInfo stList;

	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder) && hFeeder->bMtgSrc);

	/* This will arm frame slot with top trigger */
	BRDC_Slot_ExecuteOnTrigger_isr(hFeeder->hSource->ahSlot[BAVC_Polarity_eFrame],
		s_aMtgParams[hFeeder->eId].aeTrigger[0], true);

	/* init with default refresh rate */
	BVDC_P_SRC_NEXT_RUL(hFeeder->hSource, BAVC_Polarity_eFrame);
	hList = BVDC_P_SRC_GET_LIST(hFeeder->hSource, BAVC_Polarity_eFrame);
	BVDC_P_ReadListInfo_isr(&stList, hList);
	BVDC_P_Feeder_BuildMtgRul_isr(hFeeder, &stList, false, hFeeder->hSource->eDefMfdVertRateCode);
	BVDC_P_WriteListInfo_isr(&stList, hList);
	hSlot = BVDC_P_SRC_GET_SLOT(hFeeder->hSource, BAVC_Polarity_eFrame);
	BRDC_Slot_SetList_isr(hSlot, hList);
	/* BRDC_Slot_Execute_isr(hSlot); */

	/* manually force mtg triger0 once */
	BREG_Write32(hFeeder->hSource->hVdc->hRegister, BCHP_MFD_0_MTG_FORCE + hFeeder->ulRegOffset,
		BCHP_FIELD_DATA(MFD_0_MTG_FORCE, REPEAT_POLARITY, 1) |
		BCHP_FIELD_DATA(MFD_0_MTG_FORCE, TRIGGER0, 1));
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_Mtg_MpegDataReady_isr
	( BVDC_P_Feeder_Handle             hFeeder,
      BRDC_List_Handle                 hList,
	  BAVC_MVD_Field                  *pNewPic)
{
	bool bSrcInterlaced;
	BVDC_P_ListInfo stList;

	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder) && hFeeder->bMtgSrc);
	BDBG_MSG(("MpegDataReady fd %d, IntrPl %d, srcPl %d, frmRate %d", hFeeder->eId,
		pNewPic->eInterruptPolarity, pNewPic->eSourcePolarity, pNewPic->eFrameRateCode));

	/* unlikely MpegDataReady_isr before MtgCallback_isr, but play safe:
	 * avoid to reset MTG every MTG vsync */
	if (!hFeeder->bMtgInited)
	{
		hFeeder->bMtgInited = true;
		BVDC_P_Source_CleanupSlots_isr(hFeeder->hSource);
	}

	if (!pNewPic->bMute)
	{
		BAVC_FrameRateCode eMtgFrameRateCode = hFeeder->hSource->eMfdVertRateCode;
		/* bSrcInterlaced = pNewPic->bStreamProgressive || (pNewPic->eSourcePolarity != BAVC_Polarity_eFrame);*/
		bSrcInterlaced = false;
		if ((bSrcInterlaced != hFeeder->bMtgInterlaced) ||
			(eMtgFrameRateCode != hFeeder->eMtgFrameRateCode))
		{
			BVDC_P_ReadListInfo_isr(&stList, hList);
			BVDC_P_Feeder_BuildMtgRul_isr(hFeeder, &stList, bSrcInterlaced, eMtgFrameRateCode);
			BVDC_P_WriteListInfo_isr(&stList, hList);
		}
	}
}

/***************************************************************************
 * This function get register when create source.
 *
 * User need not to access this functions.
 */
void BVDC_P_Source_MtgCallback_isr
	( void                            *pvSourceHandle,
	  int                              iParam2 )
{
	BVDC_Source_Handle hSource = (BVDC_Source_Handle)pvSourceHandle;

	BDBG_ENTER(BVDC_P_Source_MtgCallback_isr);

	/* Get Source context */
	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_ASSERT(BVDC_P_SRC_IS_MPEG(hSource->eId));
	BDBG_ASSERT(hSource->hMpegFeeder->bMtgSrc);
	BDBG_MSG(("Mtg %d callback, polarity %d", hSource->eId, iParam2));
	BSTD_UNUSED(iParam2);

	/* Make sure the BKNI enter/leave critical section works. */
	BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

	/* MTG source is always sync-slip with a compositor/vec. */
	BDBG_ASSERT(!hSource->hSyncLockCompositor);

	/* avoid to reset MTG every MTG vsync */
	if (!hSource->hMpegFeeder->bMtgInited)
	{
		hSource->hMpegFeeder->bMtgInited = true;
		BVDC_P_Source_CleanupSlots_isr(hSource);
	}

	BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
	BDBG_LEAVE(BVDC_P_Source_MtgCallback_isr);

	return;
}

#endif /* #if BVDC_P_SUPPORT_MTG */

/*------------------------------------------------------------------------
 * {private}
 *
 */
static void BVDC_Feeder_BuildRulGfxSurAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList )
{
	BVDC_P_GfxSurfaceContext  *pGfxSurface;
	BVDC_P_SurfaceInfo   *pCurSur;
	uint32_t  ulSurAddr, ulBottomOffset;
	uint32_t  ulAddrShadowReg0, ulAddrShadowReg1;
	uint32_t  ulRAddrShadowReg0, ulRAddrShadowReg1;

	pGfxSurface = &hFeeder->stGfxSurface;
	pCurSur = &(pGfxSurface->stCurSurInfo);

	/* ---------------------------------------------------------------------
	 * (1) send new surface to address shadow registers.
	 */
	ulSurAddr = pCurSur->ulAddress + pGfxSurface->ulMainByteOffset;
	if (ulSurAddr != pGfxSurface->stSurNode[pGfxSurface->ucNodeIdx].ulAddr)
	{
		BVDC_P_GfxSurface_SetShadowRegs_isr(&hFeeder->stGfxSurface, pCurSur, hFeeder->hSource);
	}

	/* ---------------------------------------------------------------------
	 * (2) setup RUL to pick one set of surface address shadow registers.
	 *
	 * After this RUL is built, gfx surface might change with SetSurface, we
	 * want the change showing up as soon as possible. SetSurface will put the
	 * surface address to a shadow register, and here we pre-build the RUL to
	 * copy surface address value from shadow register to GFD.
	 *
	 * In order to activate left and right surface atomically, we use an index
	 * register, and ping-pong buffered left addr and right addr register
	 * pairs.  The index indicates the left/right surface addr in which pair
	 * should be used. The following is the algorithm:
	 *
	 * v_0  <-  index
	 * v_1  <-  ~index
	 *
	 * v_2 = v_0 & left_start_1
	 * v_3 = v_1 & left_start_0
	 * v_2 = v_2 | v_3
	 * v_3 <- ulBottomOffset
	 * BCHP_GFD_0_SRC_START_ADDR = v_2 + v3
	 *
	 * v_2 = v_0 & right_start_0
	 * v_3 = v_1 & right_start_1
	 * v_2 = v_2 | v_3
	 * v_3 <-  ulBottomOffset
	 * BCHP_GFD_0_SRC_START_R_ADDR = v_2 + v3
	 */
	ulBottomOffset = (BAVC_Polarity_eBotField == hFeeder->stPicture.eSrcPolarity)?
		pCurSur->ulPitch : 0;
	if (pGfxSurface->b3dSrc && pCurSur->stAvcPic.pSurface)
	{
		if (BVDC_P_ORIENTATION_IS_3D(pCurSur->stAvcPic.eInOrientation) &&
			BFMT_Orientation_e3D_Right == hFeeder->eOutputOrientation)
		{
			/* sawp right Surface and left surface due to output orientation */
			ulAddrShadowReg0  = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[0]);
			ulAddrShadowReg1  = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[1]);
			ulRAddrShadowReg0 = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[0]);
			ulRAddrShadowReg1 = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[1]);
		}
		else
		{
			ulAddrShadowReg0  = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[0]);
			ulAddrShadowReg1  = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[1]);
			ulRAddrShadowReg0 = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[0]);
			ulRAddrShadowReg1 = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[1]);
		}

		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
		*pList->pulCurrent++ = BRDC_REGISTER(pGfxSurface->ulRegIdxReg);
		*pList->pulCurrent++ = BRDC_OP_VAR_XOR_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);
		*pList->pulCurrent++ = ~0;

		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
		*pList->pulCurrent++ = ulAddrShadowReg1;
		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_3);
		*pList->pulCurrent++ = ulAddrShadowReg0;

		*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_2, BRDC_Variable_2);
		*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_1, BRDC_Variable_3, BRDC_Variable_3);
		*pList->pulCurrent++ = BRDC_OP_VAR_OR_VAR_TO_VAR (BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_3);
		*pList->pulCurrent++ = ulBottomOffset;
		*pList->pulCurrent++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

		*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_PICTURE0_LINE_ADDR_0) + hFeeder->ulRegOffset;

#if BVDC_P_MFD_SUPPORT_3D_VIDEO
		if(BVDC_P_ORIENTATION_IS_3D(pCurSur->stAvcPic.eInOrientation) &&
		   BVDC_P_ORIENTATION_IS_3D(hFeeder->eOutputOrientation))
		{
			*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
			*pList->pulCurrent++ = ulRAddrShadowReg1;
			*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_3);
			*pList->pulCurrent++ = ulRAddrShadowReg0;

			*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_2, BRDC_Variable_2);
			*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_1, BRDC_Variable_3, BRDC_Variable_3);
			*pList->pulCurrent++ = BRDC_OP_VAR_OR_VAR_TO_VAR (BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

			*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_3);
			*pList->pulCurrent++ = ulBottomOffset;
			*pList->pulCurrent++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

			*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_PICTURE0_LINE_ADDR_0_R) + hFeeder->ulRegOffset;
		}
		else
		{
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_PICTURE0_LINE_ADDR_0_R) + hFeeder->ulRegOffset;
			*pList->pulCurrent++ = 0;
		}
#endif
	}
	else if(hFeeder->stMfdPicture.ePxlFmt != BPXL_INVALID)/* !(pGfxSurface->b3dSrc) && not striped */
	{
		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
		*pList->pulCurrent++ = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[0]);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
		*pList->pulCurrent++ = ulBottomOffset;
		*pList->pulCurrent++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_0);

		*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_0);
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_MFD_0_PICTURE0_LINE_ADDR_0) + hFeeder->ulRegOffset;
	}

	/* set RUL to increase VsyncCntr */
	*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
	*pList->pulCurrent++ = BRDC_REGISTER(pGfxSurface->ulVsyncCntrReg);
	*pList->pulCurrent++ = BRDC_OP_VAR_SUM_IMM_TO_VAR(BRDC_Variable_2, BRDC_Variable_2);
	*pList->pulCurrent++ = 1;
	*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
	*pList->pulCurrent++ = BRDC_REGISTER(pGfxSurface->ulVsyncCntrReg);
}


#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_P_Feeder_BuildRulVfd_DcxmMosaicRect_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  const BVDC_P_PictureNodePtr      pPicture,
	  bool                             bEnable )
{
	uint32_t        i;
	bool            bInterlaced;
	BAVC_Polarity   eCapturePolarity;

	eCapturePolarity =
		(BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)
		? pPicture->eDstPolarity : pPicture->eSrcPolarity);
	bInterlaced = (eCapturePolarity != BAVC_Polarity_eFrame);

	if(bEnable)
	{
		BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_VFD_0_DCDM_RECT_CTRL, hFeeder->ulVfd0RegOffset,
			((BCHP_VFD_0_DCDM_RECT_ENABLE_MASK - BCHP_VFD_0_DCDM_RECT_CTRL) / 4) + 1);

		/* VFD_0_DCDM_RECT_CTRL */
		*pList->pulCurrent++ = (
			BCHP_FIELD_ENUM(VFD_0_DCDM_RECT_CTRL, RECT_ENABLE, ENABLE));

		/* VFD_0_DCDM_RECT_ENABLE_MASK */
		*pList->pulCurrent++ = (
			BCHP_FIELD_DATA(VFD_0_DCDM_RECT_ENABLE_MASK, RECT_ENABLE_MASK,
				(1<<pPicture->ulMosaicCount) - 1));

		/* BCHP_VFD_0_DCDM_RECT_SIZEi_ARRAY_BASE */
		BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_VFD_0_DCDM_RECT_SIZEi_ARRAY_BASE,
			hFeeder->ulVfd0RegOffset, pPicture->ulMosaicCount);

		for(i = 0; i < pPicture->ulMosaicCount; i++)
		{
			*pList->pulCurrent++ = (
				BCHP_FIELD_DATA(VFD_0_DCDM_RECT_SIZEi, HSIZE,
					pPicture->astMosaicRect[i].ulWidth) |
				BCHP_FIELD_DATA(VFD_0_DCDM_RECT_SIZEi, VSIZE,
					pPicture->astMosaicRect[i].ulHeight>> bInterlaced));
		}

		/* BCHP_VFD_0_DCDM_RECT_OFFSETi_ARRAY_BASE */
		BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_VFD_0_DCDM_RECT_OFFSETi_ARRAY_BASE,
			hFeeder->ulVfd0RegOffset, pPicture->ulMosaicCount);

		for(i = 0; i < pPicture->ulMosaicCount; i++)
		{
			*pList->pulCurrent++ = (
				BCHP_FIELD_DATA(VFD_0_DCDM_RECT_OFFSETi, X_OFFSET,
					pPicture->astMosaicRect[i].lLeft) |
				BCHP_FIELD_DATA(VFD_0_DCDM_RECT_OFFSETi, Y_OFFSET,
					pPicture->astMosaicRect[i].lTop >> bInterlaced));
		}
	}
	else
	{
		BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_VFD_0_DCDM_RECT_CTRL, hFeeder->ulVfd0RegOffset,
				((BCHP_VFD_0_DCDM_RECT_ENABLE_MASK - BCHP_VFD_0_DCDM_RECT_CTRL) / 4) + 1);

		/* VFD_0_DCDM_RECT_CTRL */
		*pList->pulCurrent++ = (
			BCHP_FIELD_ENUM(VFD_0_DCDM_RECT_CTRL, RECT_ENABLE, DISABLE));

		/* VFD_0_DCDM_RECT_ENABLE_MASK */
		*pList->pulCurrent++ = (
			BCHP_FIELD_DATA(VFD_0_DCDM_RECT_ENABLE_MASK, RECT_ENABLE_MASK, 0));
	}

	return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Feeder_BuildRulVfd_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  const BVDC_P_PictureNodePtr      pPicture )
{
#if (!BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
	BSTD_UNUSED(pPicture);
#endif

	BDBG_ASSERT(!BVDC_P_Feeder_IS_MPEG(hFeeder));

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
	if(hFeeder->bSupportDcxm)
	{
		uint32_t   ulData;
#ifdef BCHP_VFD_0_DCDM_CFG_HALF_VBR_BFR_MODE_SHIFT
		bool       bHalfSizeBufMode;

		bHalfSizeBufMode =
			(pPicture->eSrcOrientation == BFMT_Orientation_e3D_LeftRight) ||
			(pPicture->eDispOrientation == BFMT_Orientation_e3D_LeftRight);
#endif

		/* DCDM is not in MFD !!!.
		 * Can not use shadow registers settings in MFD */
		/* VFD_0_DCDM_CFG */
#if (BVDC_P_MFD_SUPPORT_LPDDR4_MEMORY_PITCH)
		if(!hFeeder->bFixedColor)
#else
		if(pPicture->bEnableDcxm && !hFeeder->bFixedColor)
#endif
		{
			BVDC_P_Feeder_BuildRulVfd_DcxmMosaicRect_isr(hFeeder,
				pList, pPicture, pPicture->bMosaicMode);
		}

		ulData =
			BCHP_FIELD_DATA(VFD_0_DCDM_CFG, ENABLE, pPicture->bEnableDcxm && !hFeeder->bFixedColor) |
#ifdef BCHP_VFD_0_DCDM_CFG_HALF_VBR_BFR_MODE_SHIFT
			BCHP_FIELD_DATA(VFD_0_DCDM_CFG, HALF_VBR_BFR_MODE, bHalfSizeBufMode) |
#endif
			BCHP_FIELD_ENUM(VFD_0_DCDM_CFG, APPLY_QERR,  Apply_Qerr ) |
			BCHP_FIELD_ENUM(VFD_0_DCDM_CFG, FIXED_RATE,  Fixed      ) |
			BCHP_FIELD_ENUM(VFD_0_DCDM_CFG, COMPRESSION, BPP_10     );

		BVDC_P_SUBRUL_ONE_REG(pList, BCHP_VFD_0_DCDM_CFG,
			hFeeder->ulVfd0RegOffset, ulData);
	}
#endif

	/* Address Offsets are different between MFD and VFD.
	 * Be careful when using BVDC_P_MFD_BLOCK_WRITE_TO_RUL or
	 * BVDC_P_MFD_WRITE_TO_RUL */
	/* VFD_0_TEST_MODE_CNTL */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(
		BCHP_VFD_0_TEST_MODE_CNTL + hFeeder->ulVfd0RegOffset);
	*pList->pulCurrent++ = BVDC_P_MFD_GET_REG_DATA(MFD_0_TEST_MODE_CNTL);

	if (!hFeeder->bGfxSrc)
	{
#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_FEEDER_CNTL,  MFD_0_PICTURE0_LINE_ADDR_0_R, pList->pulCurrent);
		BVDC_P_MFD_WRITE_TO_RUL(
			MFD_0_PIC_FEED_CMD, pList->pulCurrent);
#else

#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_10)
		BVDC_P_MFD_WRITE_TO_RUL(
			MFD_0_PICTURE0_LINE_ADDR_0_R, pList->pulCurrent);
#endif

		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_FEEDER_CNTL, MFD_0_PIC_FEED_CMD, pList->pulCurrent);
#endif

#if (!BVDC_P_USE_RDC_TIMESTAMP)
		/* Read and store timestamp */
		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
		*pList->pulCurrent++ = BRDC_REGISTER(hFeeder->stTimerReg.status);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
		*pList->pulCurrent++ = BCHP_TIMER_TIMER0_STAT_COUNTER_VAL_MASK;

		*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(
			BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_2);

		*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
		*pList->pulCurrent++ = BRDC_REGISTER(hFeeder->ulTimestampRegAddr);
#endif

	}

	else /* feed gfx surface */
	{
#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_10)
		/* Split the above BLOCK WRITE to set PICTURE0_LINE_ADDR_0
		 * from shadow registers */
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_FEEDER_CNTL, MFD_0_DISP_VSIZE, pList->pulCurrent);

		BVDC_Feeder_BuildRulGfxSurAddr_isr(hFeeder, pList);

		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_DATA_MODE, MFD_0_PIC_FEED_CMD, pList->pulCurrent);

#else
		/* Split the above BLOCK WRITE to set PICTURE0_LINE_ADDR_0
		 * from shadow registers */
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_FEEDER_CNTL, MFD_0_PICTURE0_DISP_VERT_WINDOW, pList->pulCurrent);

		BVDC_Feeder_BuildRulGfxSurAddr_isr(hFeeder, pList);

#if (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_5)
		BVDC_P_MFD_WRITE_TO_RUL(
			MFD_0_PIC_FEED_CMD, pList->pulCurrent);
#elif (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_7)
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_US_422_TO_444, MFD_0_PIC_FEED_CMD, pList->pulCurrent);
#elif (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_9)
		BVDC_P_MFD_BLOCK_WRITE_TO_RUL(
			MFD_0_BYTE_ORDER, MFD_0_PIC_FEED_CMD, pList->pulCurrent);
#endif
#endif
	}

	return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_BuildRul_isr
	( const BVDC_P_Feeder_Handle       hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  const BVDC_P_PictureNodePtr      pPicture,
	  BVDC_P_State                     eVnetState,
	  BVDC_P_PicComRulInfo            *pPicComRulInfo )
{
	uint32_t  ulRulOpsFlags;

	BDBG_ENTER(BVDC_P_Feeder_BuildRul_isr);
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

	/* currently this is only for vnet building / tearing-off*/

	ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
		&(hFeeder->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

	if ((0 == ulRulOpsFlags) ||
		(ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
		return;
	else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
	{
		BVDC_P_Feeder_SetEnable_isr(hFeeder, false);
		BVDC_P_MFD_WRITE_TO_RUL(MFD_0_PIC_FEED_CMD, pList->pulCurrent);
	}

	/* Include a reset at the beginning of feeder bringup. */
	if(hFeeder->bInitial)
	{
#if (BVDC_P_SUPPORT_MFD_VER < BVDC_P_MFD_VER_14)
		BVDC_P_BUILD_RESET(pList->pulCurrent,
			hFeeder->ulResetRegAddr, hFeeder->ulResetMask);
#endif

		if(BVDC_P_Feeder_IS_MPEG(hFeeder))
		{
#if BVDC_P_SUPPORT_MTG
			if(hFeeder->bMtgSrc)
			{
				/* resetting has killed mtg, re-enable it now */
				BVDC_P_Feeder_BuildMtgRul_isr(hFeeder, pList, hFeeder->bMtgInterlaced, hFeeder->eMtgFrameRateCode);
			}
#endif

			BVDC_P_MFD_WRITE_TO_RUL(MFD_0_CRC_CTRL, pList->pulCurrent);
			BVDC_P_MFD_WRITE_TO_RUL(MFD_0_CRC_SEED, pList->pulCurrent);
		}

		hFeeder->bInitial = false;
	}

	/* Add feeder registers to RUL */
	if((BVDC_P_MFD_COMPARE_FIELD_DATA(MFD_0_PIC_FEED_CMD, START_FEED, 1)) &&
	   (ulRulOpsFlags & BVDC_P_RulOp_eEnable))
	{
		if (BVDC_P_Feeder_IS_MPEG(hFeeder))
		{
			BVDC_P_Feeder_BuildRulMfd_isr(hFeeder, pList);
		}
		else
		{
			BVDC_P_Feeder_BuildRulVfd_isr(hFeeder, pList, pPicture);
		}
	}

	if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
	{
		BVDC_P_Feeder_BuildRul_DrainVnet_isr(hFeeder, pList, pPicComRulInfo->bNoCoreReset);
	}

	BDBG_LEAVE(BVDC_P_Feeder_BuildRul_isr);
	return;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_GetFormatCtrlSettings_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_Rect               *pScanoutRect,
	  const BVDC_P_PictureNodePtr      pPicture )
{
	BPXL_Format                 ePxlFormat = BPXL_INVALID;
	BVDC_P_Feeder_ImageFormat   eImageFormat;
	BFMT_Orientation            eInOrientation, eOutOrientation;
	BERR_Code                   err = BERR_SUCCESS;

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		/* MFD data comes from pFieldData */
		BDBG_ASSERT(pFieldData);

		/* Get orientatation */
		if((pFieldData->eOrientation == BFMT_Orientation_e3D_LeftRight)||
		   (pFieldData->eOrientation == BFMT_Orientation_e3D_OverUnder) ||
		   (pFieldData->eOrientation == BFMT_Orientation_e3D_Left) ||
		   (pFieldData->eOrientation == BFMT_Orientation_e3D_Right))
		{
			/* always use dual pointer if 3D non-MRE mode */
			eInOrientation = BFMT_Orientation_e3D_Left;
		}
		else
		{
			eInOrientation = pFieldData->eOrientation;
		}

		BVDC_P_Source_GetOutputOrientation_isr(hFeeder->hSource, pFieldData,
			NULL, &eOutOrientation);

		if(eOutOrientation != hFeeder->eOutputOrientation)
			hFeeder->hSource->stCurInfo.stDirty.stBits.bOrientation = BVDC_P_DIRTY;

		/* Get ulThroughput */
		hFeeder->ulThroughput = pScanoutRect->ulWidth
			* (pScanoutRect->ulHeight  >> (pFieldData->eSourcePolarity != BAVC_Polarity_eFrame))
			* (hFeeder->hSource->ulVertFreq / BFMT_FREQ_FACTOR);
		if(hFeeder->eInputOrientation != BFMT_Orientation_e2D)
			hFeeder->ulThroughput = 2 * hFeeder->ulThroughput;

		ePxlFormat = pFieldData->ePxlFmt;

		if(ePxlFormat == BPXL_INVALID)
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_eAVC_MPEG;
		}
		else if(BPXL_eX2_Cr10_Y10_Cb10 == ePxlFormat)
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_eYUV444;
		}
		else
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_ePacked;
		}
	}
	else
	{
#if (BVDC_P_MFD_SUPPORT_IMAGE_FORMAT_PACKED_NEW)
		bool bCompressionEnable;
#endif

		/* VFD data comes from pPicture */
		BDBG_ASSERT(pPicture);

		/* always use dual pointer if 3D mode */
		eInOrientation = (pPicture->eSrcOrientation == BFMT_Orientation_e2D)
			? BFMT_Orientation_e2D : BFMT_Orientation_e3D_Left;
		eOutOrientation = pPicture->eDispOrientation;
#if (BVDC_P_DCX_3D_WORKAROUND)
		if(pPicture->bEnableDcxm)
			eOutOrientation = BFMT_Orientation_e2D;
#endif

		ePxlFormat = pPicture->ePixelFormat;
#if (BVDC_P_MFD_SUPPORT_IMAGE_FORMAT_PACKED_NEW)
		bCompressionEnable = (hFeeder->bGfxSrc)
			? false : pPicture->stCapCompression.bEnable;
#endif

		/* default for VFD */
#if (!BVDC_P_VFD_SUPPORT_IMAGE_FORMAT)
		/* No VFD_0_FEEDER_CNTL_IMAGE_FORMAT field. Reserved field
		 * must be written with 0 */
		eImageFormat = 0;
#else
		eImageFormat = BVDC_P_Feeder_ImageFormat_ePacked;
#endif

#if (BVDC_P_MFD_SUPPORT_10BIT_444)
		/* Image format */
		if(BVDC_P_CAP_PIXEL_FORMAT_10BIT444 == (BPXL_Format)ePxlFormat)
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_eYUV444;
		}
		else
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_ePacked;
		}
#endif

#if (BVDC_P_MFD_SUPPORT_IMAGE_FORMAT_PACKED_NEW)
		/* Use new format to match the byte order with capture */
		if((eImageFormat == BVDC_P_Feeder_ImageFormat_ePacked) &&
		   (BPXL_IS_YCbCr422_FORMAT(ePxlFormat)) &&
		   (!bCompressionEnable))
		{
			eImageFormat = BVDC_P_Feeder_ImageFormat_ePacked_new;
		}
#endif
	}

	hFeeder->ePxlFormat         = ePxlFormat;
	hFeeder->eImageFormat       = eImageFormat;
	hFeeder->eInputOrientation  = eInOrientation;
	hFeeder->eOutputOrientation = eOutOrientation;

	return BERR_TRACE(err);
}

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
/***************************************************************************
 * Set MFD_0_FEEDER_CNTL.MEM_VIDEO and MFD_0_FEEDER_CNTL.BVB_VIDEO
 */
static BERR_Code BVDC_P_Feeder_SetOrientation_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	/* MFD_0_FEEDER_CNTL */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, MEM_VIDEO) |
		BCHP_MASK(MFD_0_FEEDER_CNTL, BVB_VIDEO));

	/* Only MODE_2D and MODE_3D_DUAL_POINTER are supported
	 * starting from BVDC_P_MFD_VER_16 */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
#if (BVDC_P_SUPPORT_MFD_VER < BVDC_P_MFD_VER_16)
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, MEM_VIDEO,
			s_hwOrientation[hFeeder->eInputOrientation]) |
#else
		((((hFeeder->eInputOrientation == BFMT_Orientation_e2D) ||
		  hFeeder->eOutputOrientation == BFMT_Orientation_e2D))
		? BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, MEM_VIDEO, MODE_2D)
		: BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, MEM_VIDEO, MODE_3D_DUAL_POINTER)) |
#endif
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, BVB_VIDEO, hFeeder->eOutputOrientation));

	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_PACKING_TYPE)
/***************************************************************************
 * Set MFD_0_FEEDER_CNTL.PACKING_TYPE
 *
 * MFD_0_FEEDER_CNTL.PACKING_TYPE is only used when MFD_0_FEEDER_CNTL.IMAGE_FORMAT
 * is PACKED on older chipset. On Chipset PACKED_NEW is support, MFD_0_BYTE_ORDER
 * is used instead.
 */
static BERR_Code BVDC_P_Feeder_SetPackingType_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_PictureNodePtr      pPicture )
{
	uint32_t     ulPackingType = 0;
	BERR_Code    err = BERR_SUCCESS;
	BPXL_Format  ePxlFormat = BPXL_eCr8_Y18_Cb8_Y08;

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		/* MFD data comes from pFieldData */
		BDBG_ASSERT(pFieldData);
		ePxlFormat = pFieldData->ePxlFmt;
	}
	else
	{
		/* VFD data comes from pPicture */
		BDBG_ASSERT(pPicture);
		ePxlFormat = pPicture->ePixelFormat;
	}

	/* Note:
	 * The following 4 pixel formats:
	 *      BPXL_eY08_Cr8_Y18_Cb8
	 *      BPXL_eCr8_Y08_Cb8_Y18
	 *      BPXL_eCb8_Y08_Cr8_Y18
	 *      BPXL_eY08_Cb8_Y18_Cr8
	 * are not supported by VFD in older chipset, warning sent out in
	 * BVDC_Window_SetPixelFormat, and use opposite endian packing type here.
	 * Video display will have wrong color. However CAP_0_BYTE_ORDER is
	 * set correctly, so the data in capture is correct.
	 * For the newer version of VFD (when PACKED_NEW format is supported), use
	 * VFD_0_BYTE_ORDER to set pixel format
	 */
	ePxlFormat =
		(ePxlFormat == BPXL_eY08_Cr8_Y18_Cb8) ? BPXL_eCb8_Y18_Cr8_Y08 :
		(ePxlFormat == BPXL_eCr8_Y08_Cb8_Y18) ? BPXL_eY18_Cb8_Y08_Cr8 :
		(ePxlFormat == BPXL_eY08_Cb8_Y18_Cr8) ? BPXL_eCr8_Y18_Cb8_Y08 :
		(ePxlFormat == BPXL_eCb8_Y08_Cr8_Y18) ? BPXL_eY18_Cr8_Y08_Cb8 :
		ePxlFormat;

	/* feeder is big endian */
	switch(ePxlFormat)
	{
		case BPXL_eCb8_Y18_Cr8_Y08:
			ulPackingType = BCHP_MFD_0_FEEDER_CNTL_PACKING_TYPE_Y0_V0_Y1_U0;
			break;

		case BPXL_eY18_Cb8_Y08_Cr8:
			ulPackingType = BCHP_MFD_0_FEEDER_CNTL_PACKING_TYPE_V0_Y0_U0_Y1;
			break;

		case BPXL_eY18_Cr8_Y08_Cb8:
			ulPackingType = BCHP_MFD_0_FEEDER_CNTL_PACKING_TYPE_U0_Y0_V0_Y1;
			break;

		case BPXL_eCr8_Y18_Cb8_Y08:
			ulPackingType = BCHP_MFD_0_FEEDER_CNTL_PACKING_TYPE_Y0_U0_Y1_V0;
			break;

		default:
			ulPackingType = BCHP_MFD_0_FEEDER_CNTL_PACKING_TYPE_Y0_U0_Y1_V0;
			break;
	}

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, PACKING_TYPE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, PACKING_TYPE, ulPackingType));

	return BERR_TRACE(err);
}
#endif

/***************************************************************************
 * Set MFD_0_FEEDER_CNTL.IMAGE_FORMAT
 */
static BERR_Code BVDC_P_Feeder_SetImageFormat_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, IMAGE_FORMAT) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, IMAGE_FORMAT, hFeeder->eImageFormat) );

	return BERR_SUCCESS;
}

#if (BVDC_P_MFD_SUPPORT_PIXEL_SATURATION_ENABLE)
/***************************************************************************
 * Set MFD_0_FEEDER_CNTL.PIXEL_SATURATION_ENABLE
 */
static BERR_Code BVDC_P_Feeder_SetPixelSaturation_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, PIXEL_SATURATION_ENABLE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, PIXEL_SATURATION_ENABLE, OFF));

	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_SCB_CLIENT_SEL)
/***************************************************************************
 *  MFD only
 * Set MFD_0_FEEDER_CNTL.SCB_CLIENT_SEL
 */
static BERR_Code BVDC_P_Feeder_SetScbClient_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, SCB_CLIENT_SEL));

	/* CLIENT_A is designed to handle data rates up to 1080i60
	 * (aka 540x1920@60 = 62208000). Should match RTS settings. */
	/* Always use CLIENT_B for mosaic mode:
	 *      Should compare total throughput combined for all mosaic channels
	 *      with 1080i60. But we don't have that information when programming
	 *      first mosaic channel. So always use CLIENT_B for mosaic mode with
	 *      the assumption that deinterlacer is disabled in mosaic mode.
	 */
	if((hFeeder->ulThroughput <= 62208000) && !hFeeder->hSource->ulMosaicCount)
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
			BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, SCB_CLIENT_SEL, CLIENT_A));
	}
	else
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
			BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, SCB_CLIENT_SEL, CLIENT_B));
	}

	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_SMOOTHING_BUFFER)
/***************************************************************************
 *  MFD only
 * Set MFD_0_FEEDER_CNTL.SMOOTHING_BUFFER
 */
static BERR_Code BVDC_P_Feeder_SetSmoothBuffer_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, SMOOTHING_BUFFER));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, SMOOTHING_BUFFER, ENABLE));

	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_INIT_PHASE)
/***************************************************************************
 *  MFD only
 * Set BOT_INIT_PHASE and TOP_INIT_PHASE
 */
static BERR_Code BVDC_P_Feeder_SetInitPhase_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	int8_t    cTopInitPhase, cBotInitPhase;
	BAVC_InterpolationMode  eChromaInterpolationMode;

	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	eChromaInterpolationMode = pFieldData->eChrominanceInterpolationMode;

	/*
	 * TM5:
	 * If BAVC_InterpolationMode_eFrame:
	 *      MFD_FEEDER_CNTL.TOP_INIT_PHASE = 0;
	 *      MFD_FEEDER_CNTL.BOT_INIT_PHASE = 32;
	 * If BAVC_InterpolationMode_eField:
	 *      MFD_FEEDER_CNTL.TOP_INIT_PHASE = 0;
	 *      MFD_FEEDER_CNTL.BOT_INIT_PHASE = -16;

	 * AVC or MPEG1:
	 * If BAVC_InterpolationMode_eFrame:
	 *      MFD_FEEDER_CNTL.TOP_INIT_PHASE = -16;
	 *      MFD_FEEDER_CNTL.BOT_INIT_PHASE = 16;
	 * If BAVC_InterpolationMode_eField:
	 *      MFD_FEEDER_CNTL.TOP_INIT_PHASE = -8;
	 *      MFD_FEEDER_CNTL.BOT_INIT_PHASE = -24;
	 */
	if((pFieldData->eMpegType == BAVC_ChromaLocation_eType2) ||
	   (pFieldData->eMpegType == BAVC_ChromaLocation_eType4))
	{
		if(eChromaInterpolationMode == BAVC_InterpolationMode_eFrame)
		{
			cTopInitPhase = 0;
			cBotInitPhase = 32;
		}
		else
		{
			cTopInitPhase = 0;
			cBotInitPhase = -16;
		}
	}
	else
	{
		if(eChromaInterpolationMode == BAVC_InterpolationMode_eFrame)
		{
			cTopInitPhase = -16;
			cBotInitPhase = 16;
		}
		else
		{
			cTopInitPhase = -8;
			cBotInitPhase = -24;
		}
	}

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, BOT_INIT_PHASE) |
		BCHP_MASK(MFD_0_FEEDER_CNTL, TOP_INIT_PHASE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, BOT_INIT_PHASE, cBotInitPhase) |
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, TOP_INIT_PHASE, cTopInitPhase));
	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_INIT_PHASE)
/***************************************************************************
 *  MFD only
 * Set BOT_SKIP_FIRST_LINE and TOP_SKIP_FIRST_LINE
 */
static BERR_Code BVDC_P_Feeder_SetSkipFirstLine_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	/* TODO: Settings ? */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, BOT_SKIP_FIRST_LINE) |
		BCHP_MASK(MFD_0_FEEDER_CNTL, TOP_SKIP_FIRST_LINE));
	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, BOT_SKIP_FIRST_LINE, hFeeder->bSkipFirstLine) |
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, TOP_SKIP_FIRST_LINE, hFeeder->bSkipFirstLine));

	return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * Set MFD_0_FEEDER_CNTL
 */
static BERR_Code BVDC_P_Feeder_SetFeederCntl_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_PictureNodePtr      pPicture )
{
	BERR_Code     err = BERR_SUCCESS;

#if (!BVDC_P_MFD_SUPPORT_PACKING_TYPE)
	BSTD_UNUSED(pPicture);
#endif

#if ((!BVDC_P_MFD_SUPPORT_PACKING_TYPE) && (!BVDC_P_MFD_SUPPORT_INIT_PHASE))
	BSTD_UNUSED(pFieldData);
#endif

	/* ------------------------------------------
	 * Common settings for both MFD and VFD
	 */
	/* Set IMAGE_FORMAT */
	err = BVDC_P_Feeder_SetImageFormat_isr(hFeeder);

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	/* Set MEM_VIDEO and BVB_VIDEO */
	err = BVDC_P_Feeder_SetOrientation_isr(hFeeder);
#endif

#if (BVDC_P_MFD_SUPPORT_PIXEL_SATURATION_ENABLE)
	/* Set PIXEL_SATURATION_ENABLE */
	err = BVDC_P_Feeder_SetPixelSaturation_isr(hFeeder);
#endif

#if (BVDC_P_MFD_SUPPORT_PACKING_TYPE)
	err = BVDC_P_Feeder_SetPackingType_isr(hFeeder, pFieldData, pPicture);
#endif

	/* ------------------------------------------
	 * MFD settings only starts here
	 */
	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
#if (BVDC_P_MFD_SUPPORT_SCB_CLIENT_SEL)
		/* Set SCB_CLIENT_SEL */
		err = BVDC_P_Feeder_SetScbClient_isr(hFeeder);
#endif

#if (BVDC_P_MFD_SUPPORT_SMOOTHING_BUFFER)
		/* Set SMOOTHING_BUFFER */
		err = BVDC_P_Feeder_SetSmoothBuffer_isr(hFeeder);
#endif

#if (BVDC_P_MFD_SUPPORT_INIT_PHASE)
		/* Set BOT_INIT_PHASE and TOP_INIT_PHASE */
		err = BVDC_P_Feeder_SetInitPhase_isr(hFeeder, pFieldData);
#endif

#if (BVDC_P_MFD_SUPPORT_SKIP_FIRST_LINE)
		/* Set BOT_SKIP_FIRST_LINE and TOP_SKIP_FIRST_LINE */
		err = BVDC_P_Feeder_SetSkipFirstLine_isr(hFeeder);
#endif

	/* Set LINE_REPEAT? */

	}

	/* MFD_0_FEEDER_CNTL */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
		BCHP_MASK(MFD_0_FEEDER_CNTL, FIXED_COLOUR_ENABLE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, FIXED_COLOUR_ENABLE, OFF));

	return (err);
}

#if (BVDC_P_MFD_SUPPORT_BYTE_ORDER)
/***************************************************************************
 * Set MFD_0_BYTE_ORDER
 */
static BERR_Code BVDC_P_Feeder_SetByteOrder_isr
	( BVDC_P_Feeder_Handle             hFeeder )
{
	uint32_t      ulByteOrder =0;


#if (BVDC_P_MFD_VER_16 <= BVDC_P_SUPPORT_MFD_VER)
	switch((BPXL_Format)hFeeder->ePxlFormat)
	{
		default:
		case BPXL_eY18_Cb8_Y08_Cr8:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, Y1));
			break;

		case BPXL_eY08_Cb8_Y18_Cr8:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, Y0));
			break;

		case BPXL_eY18_Cr8_Y08_Cb8:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, Y1));
			break;

		case BPXL_eY08_Cr8_Y18_Cb8:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, Y0));
				break;

		case BPXL_eCr8_Y18_Cb8_Y08:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, CR));
			break;

		case BPXL_eCr8_Y08_Cb8_Y18:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, CB) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, CR));
				break;

		case BPXL_eCb8_Y18_Cr8_Y08:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, CB));
			break;

		case BPXL_eCb8_Y08_Cr8_Y18:
			ulByteOrder |= (
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_3_SEL, Y1) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_2_SEL, CR) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_1_SEL, Y0) |
				BCHP_FIELD_ENUM(MFD_0_COMP_ORDER, COMP_0_SEL, CB));
			break;
	}
	BVDC_P_MFD_GET_REG_DATA(MFD_0_COMP_ORDER) = ulByteOrder;
#else
	switch((BPXL_Format)hFeeder->ePxlFormat)
	{
	default:
	case BPXL_eY18_Cb8_Y08_Cr8:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, Y1));
		break;

	case BPXL_eY08_Cb8_Y18_Cr8:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, Y0));
		break;

	case BPXL_eY18_Cr8_Y08_Cb8:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, Y1));
		break;

	case BPXL_eY08_Cr8_Y18_Cb8:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, Y0));
			break;

	case BPXL_eCr8_Y18_Cb8_Y08:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, CR));
		break;

	case BPXL_eCr8_Y08_Cb8_Y18:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, CB) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, CR));
			break;

	case BPXL_eCb8_Y18_Cr8_Y08:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, CB));
		break;

	case BPXL_eCb8_Y08_Cr8_Y18:
		ulByteOrder |= (
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_3_SEL, Y1) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_2_SEL, CR) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
			BCHP_FIELD_ENUM(MFD_0_BYTE_ORDER, BYTE_0_SEL, CB));
		break;

	}

	BVDC_P_MFD_GET_REG_DATA(MFD_0_BYTE_ORDER) = ulByteOrder;
#endif
	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_DATA_MODE)
/***************************************************************************
 * Set MFD_0_DATA_MODE
 */
static BERR_Code BVDC_P_Feeder_SetDataMode_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BVDC_P_PictureNodePtr      pPicture,
	  const BAVC_MVD_Field            *pFieldData )
{
	bool          b8BitMode = true;

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		BDBG_ASSERT(pFieldData);
		b8BitMode = (BAVC_VideoBitDepth_e8Bit == pFieldData->eBitDepth);
	}
	else if(BVDC_P_FEEDER_PIXEL_FORMAT_IS_10BIT((BPXL_Format)hFeeder->ePxlFormat) ||
	    pPicture->bEnable10Bit)
	{
		b8BitMode = false;
	}

	BVDC_P_MFD_GET_REG_DATA(MFD_0_DATA_MODE) &= ~(
#if (BCHP_MFD_0_DATA_MODE_PRECISION_MASK)
		BCHP_MASK(MFD_0_DATA_MODE, PRECISION) |
#endif
		BCHP_MASK(MFD_0_DATA_MODE, PIXEL_WIDTH));

	/* 10-bit capable MFD0 will process in 10-bit; 8-bit only MFDs will ignore PRECISION bit */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_DATA_MODE) |= (
#if (BCHP_MFD_0_DATA_MODE_PRECISION_MASK)
		BCHP_FIELD_ENUM(MFD_0_DATA_MODE, PRECISION, MODE_10_BIT) |
#endif
		BCHP_FIELD_DATA(MFD_0_DATA_MODE, PIXEL_WIDTH, !b8BitMode));

	return BERR_SUCCESS;
}
#endif

#if (BVDC_P_MFD_SUPPORT_10BIT_DITHER)
/***************************************************************************
 * Set MFD_0_DITHER_CTRL etc
 */
static BERR_Code BVDC_P_Feeder_SetDither_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	/* only enable 10->8-bit dithering for 8-bit MFDs */
	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		BDBG_ASSERT(pFieldData);

		BVDC_P_MFD_GET_REG_DATA(MFD_0_DITHER_CTRL) &= ~(
			BCHP_MASK(MFD_0_DITHER_CTRL,       MODE) |
			BCHP_MASK(MFD_0_DITHER_CTRL, OFFSET_CH2) |
			BCHP_MASK(MFD_0_DITHER_CTRL, OFFSET_CH1) |
			BCHP_MASK(MFD_0_DITHER_CTRL, OFFSET_CH0) |
			BCHP_MASK(MFD_0_DITHER_CTRL,  SCALE_CH2) |
			BCHP_MASK(MFD_0_DITHER_CTRL,  SCALE_CH1) |
			BCHP_MASK(MFD_0_DITHER_CTRL,  SCALE_CH0));

		if(BAVC_VideoBitDepth_e10Bit == pFieldData->eBitDepth && !hFeeder->hSource->bIs10BitCore)
		{
			BVDC_P_MFD_GET_REG_DATA(MFD_0_DITHER_CTRL) |= (
				BCHP_FIELD_ENUM(MFD_0_DITHER_CTRL, MODE, DITHER) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH2, 1) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH1, 1) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH0, 1) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL,  SCALE_CH2, 4) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL,  SCALE_CH1, 4) |
				BCHP_FIELD_DATA(MFD_0_DITHER_CTRL,  SCALE_CH0, 4));

			BVDC_P_MFD_GET_REG_DATA(MFD_0_DITHER_LFSR_INIT) = (
				BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_INIT, SEQ, ONCE_PER_SOP) |
				BCHP_FIELD_DATA(MFD_0_DITHER_LFSR_INIT,          VALUE, 0));

			BVDC_P_MFD_GET_REG_DATA(MFD_0_DITHER_LFSR_CTRL) = (
				BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T0,  B3) |
				BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T1,  B8) |
				BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T2, B12));
		}
	}

	return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_SetLacCntlOutput_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BAVC_Polarity                    ePolarity)
{
	BVDC_P_MFD_GET_REG_DATA(MFD_0_LAC_CNTL) &= ~(
		BCHP_MASK(MFD_0_LAC_CNTL, OUTPUT_TYPE          ) |
		BCHP_MASK(MFD_0_LAC_CNTL, OUTPUT_FIELD_POLARITY) );

	if(ePolarity != BAVC_Polarity_eFrame)
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_LAC_CNTL) |=
			BCHP_FIELD_ENUM(MFD_0_LAC_CNTL, OUTPUT_TYPE, INTERLACED         ) |
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, OUTPUT_FIELD_POLARITY, ePolarity);
	}
	else
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_LAC_CNTL) |=
			BCHP_FIELD_ENUM(MFD_0_LAC_CNTL, OUTPUT_TYPE, PROGRESSIVE   ) |
			BCHP_FIELD_ENUM(MFD_0_LAC_CNTL, OUTPUT_FIELD_POLARITY, TOP );
	}

	return BERR_SUCCESS;
}

/***************************************************************************
  Set MFD_0_LAC_CNTL:
 */
static BERR_Code BVDC_P_Feeder_SetLacCntl_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  BAVC_Polarity                    ePolarity)
{
#if (BVDC_P_MFD_SUPPORT_CHROMA_VERT_POSITION)
	BVDC_P_Feeder_ChromaVertPos eChromaVertPos =
		BVDC_P_Feeder_ChromaVertPos_eHalfPixelGridBetweenLuma;
#endif

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		BDBG_ASSERT(pFieldData);
		BDBG_ASSERT(pFieldData->eMpegType <= BAVC_ChromaLocation_eType3);

#if (BVDC_P_MFD_SUPPORT_CHROMA_VERT_POSITION)
		if((pFieldData->eMpegType == BAVC_ChromaLocation_eType0) ||
		   (pFieldData->eMpegType == BAVC_ChromaLocation_eType1))
		{
			eChromaVertPos = BVDC_P_Feeder_ChromaVertPos_eHalfPixelGridBetweenLuma;
		}
		else if((pFieldData->eMpegType == BAVC_ChromaLocation_eType2) ||
		        (pFieldData->eMpegType == BAVC_ChromaLocation_eType3))
		{
			eChromaVertPos = BVDC_P_Feeder_ChromaVertPos_eColorcatedWithLuma;
		}
#endif

		BVDC_P_MFD_GET_REG_DATA(MFD_0_LAC_CNTL) &= ~(
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
			BCHP_MASK(MFD_0_LAC_CNTL, SEPARATE_FIELD_BUFFER     ) |
#endif
#if (BVDC_P_MFD_SUPPORT_STRIPE_WIDTH_SEL)
			BCHP_MASK(MFD_0_LAC_CNTL, STRIPE_WIDTH_SEL          ) |
#endif
#if (BVDC_P_MFD_SUPPORT_CHROMA_VERT_POSITION)
			BCHP_MASK(MFD_0_LAC_CNTL, CHROMA_VERT_POSITION      ) |
#endif
			BCHP_MASK(MFD_0_LAC_CNTL, CHROMA_TYPE               ) |
			BCHP_MASK(MFD_0_LAC_CNTL, CHROMA_INTERPOLATION      ));

		BVDC_P_MFD_GET_REG_DATA(MFD_0_LAC_CNTL) = (
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, SEPARATE_FIELD_BUFFER, pFieldData->eBufferFormat) |
#endif
#if (BVDC_P_MFD_SUPPORT_STRIPE_WIDTH_SEL)
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, STRIPE_WIDTH_SEL, pFieldData->eStripeWidth) |
#endif
#if (BVDC_P_MFD_SUPPORT_CHROMA_VERT_POSITION)
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, CHROMA_VERT_POSITION, eChromaVertPos) |
#endif
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, CHROMA_TYPE,
				pFieldData->eYCbCrType - BAVC_YCbCrType_e4_2_0) |
			BCHP_FIELD_DATA(MFD_0_LAC_CNTL, CHROMA_INTERPOLATION,
				pFieldData->eChrominanceInterpolationMode     ) );

		BVDC_P_Feeder_SetLacCntlOutput_isr(hFeeder, ePolarity);
	}
	else
	{
		BVDC_P_Feeder_SetLacCntlOutput_isr(hFeeder, ePolarity);
	}

	return BERR_SUCCESS;
}


/***************************************************************************
 * MFD only
 * Set MFD_0_CHROMA_SAMPLING_CNTL, MFD_0_CHROMA_REPOSITION_DERING_ENABLE
 */
static BERR_Code BVDC_P_Feeder_SetChromaRepEnable_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	bool     bChromaRepEnable = false;

	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));
	BDBG_ASSERT(pFieldData->eMpegType <= BAVC_ChromaLocation_eType3);

	if((pFieldData->eMpegType == BAVC_ChromaLocation_eType0) ||
	   (pFieldData->eMpegType == BAVC_ChromaLocation_eType2))
	{
		bChromaRepEnable = false;
	}
	else if((pFieldData->eMpegType == BAVC_ChromaLocation_eType1) ||
	        (pFieldData->eMpegType == BAVC_ChromaLocation_eType3))
	{
		bChromaRepEnable = true;
	}

	/* CHROMA_REPOSITION_ENABLE controls horizontal filter enabled, and is used to
	 * reposition the chroma pixel to the luma pixel position in horizontal position.
	 * It's required for MPEG1. In the case of MEPG2, chroma is co-located with luma
	 * after vertical filtering. */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_SAMPLING_CNTL) &= ~(
		BCHP_MASK(MFD_0_CHROMA_SAMPLING_CNTL, CHROMA_REPOSITION_ENABLE) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_SAMPLING_CNTL) |= (
		BCHP_FIELD_DATA(MFD_0_CHROMA_SAMPLING_CNTL, CHROMA_REPOSITION_ENABLE, bChromaRepEnable));

#if (BVDC_P_MFD_SUPPORT_DERINGING)
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_REPOSITION_DERING_ENABLE) &= ~(
		BCHP_MASK(MFD_0_CHROMA_REPOSITION_DERING_ENABLE, DERING_EN));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_REPOSITION_DERING_ENABLE) |=  (
		BCHP_FIELD_DATA(MFD_0_CHROMA_REPOSITION_DERING_ENABLE, DERING_EN, bChromaRepEnable));
#endif

	return BERR_SUCCESS;
}

/***************************************************************************
 * MFD only
 * Set MFD_0_RANGE_EXP_REMAP_CNTL
 */
static BERR_Code BVDC_P_Feeder_SetRangeExpRemap_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_RANGE_EXP_REMAP_CNTL) = (
		BCHP_FIELD_DATA(MFD_0_RANGE_EXP_REMAP_CNTL, SCALE_Y, pFieldData->ulLumaRangeRemapping) |
		BCHP_FIELD_DATA(MFD_0_RANGE_EXP_REMAP_CNTL, SCALE_C, pFieldData->ulChromaRangeRemapping) );

	return BERR_SUCCESS;
}

/***************************************************************************
 * MFD only
 * Set MFD_0_CRC_CTRL
 */
static BERR_Code BVDC_P_Feeder_SetCrcType_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_Source_CrcType              eCrcType )
{
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

#if (BVDC_P_MFD_SUPPORT_CRC_TYPE)
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CRC_CTRL) &= ~(
		BCHP_MASK(MFD_0_CRC_CTRL, TYPE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_CRC_CTRL) |=  (
		BCHP_FIELD_DATA(MFD_0_CRC_CTRL, TYPE, eCrcType));

	/* Initial seed value should be all '1's */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CRC_SEED) = 0xFFFFFFFF;

#else
	BSTD_UNUSED(hFeeder);
	BSTD_UNUSED(eCrcType);
#endif

	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_SetFormatCtrl_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_PictureNodePtr      pPicture,
	  BAVC_Polarity                    ePolarity)
{
	/* Set MFD_0_FEEDER_CNTL */
	BVDC_P_Feeder_SetFeederCntl_isr(hFeeder, pFieldData, pPicture);

#if (BVDC_P_MFD_SUPPORT_BYTE_ORDER)
	/* Set MFD_0_BYTE_ORDER */
	BVDC_P_Feeder_SetByteOrder_isr(hFeeder);
#endif

#if (BVDC_P_MFD_SUPPORT_DATA_MODE)
	/* Set MFD_0_DATA_MODE */
	BVDC_P_Feeder_SetDataMode_isr(hFeeder, pPicture, pFieldData);
#endif

#if (BVDC_P_MFD_SUPPORT_10BIT_DITHER)
	/* Set MFD_0_DITHER_CTRL etc */
	BVDC_P_Feeder_SetDither_isr(hFeeder, pFieldData);
#endif

	/* Set MFD_0_LAC_CNTL */
	BVDC_P_Feeder_SetLacCntl_isr(hFeeder, pFieldData, ePolarity);

	if(BVDC_P_Feeder_IS_MPEG(hFeeder))
	{
		BDBG_ASSERT(pFieldData);

		if(hFeeder->eCrcType != hFeeder->hSource->stCurInfo.eCrcType)
		{
			hFeeder->bInitial = true;
			hFeeder->eCrcType = hFeeder->hSource->stCurInfo.eCrcType;
			BVDC_P_Feeder_SetCrcType_isr(hFeeder, hFeeder->eCrcType);
		}

		/* Set MFD_0_RANGE_EXP_REMAP_CNTL */
		BVDC_P_Feeder_SetRangeExpRemap_isr(hFeeder, pFieldData);
		/* Set MFD_0_CHROMA_SAMPLING_CNTL, MFD_0_CHROMA_REPOSITION_DERING_ENABLE */
		BVDC_P_Feeder_SetChromaRepEnable_isr(hFeeder, pFieldData);
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_SetMpegStride_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	uint32_t                           ulLumaStride, ulChromaStride;
	BVDC_P_Feeder_StripeWidthConfig    eStripeWidthCfg;

	/* Get stripe width settings */
	eStripeWidthCfg = s_aStripeWidthCfgTbl[pFieldData->eStripeWidth];

	/* Get stride */
	ulLumaStride = ((pFieldData->eSourcePolarity == BAVC_Polarity_eFrame) ||
		(pFieldData->eBufferFormat == BAVC_DecodedPictureBuffer_eFieldsPair))
		? eStripeWidthCfg.ulStripeWidth : eStripeWidthCfg.ulStripeWidth * 2;

	if(pFieldData->eYCbCrType == BAVC_YCbCrType_e4_2_0)
	{
		ulChromaStride = ((pFieldData->eChrominanceInterpolationMode == BAVC_InterpolationMode_eFrame) ||
			(pFieldData->eBufferFormat == BAVC_DecodedPictureBuffer_eFieldsPair))
			? eStripeWidthCfg.ulStripeWidth : eStripeWidthCfg.ulStripeWidth * 2;
	}
	else
	{
		ulChromaStride = ulLumaStride;
	}

	hFeeder->ulChromaStride = ulChromaStride;
	hFeeder->ulLumaStride = ulLumaStride;
	BDBG_MSG(("Chroma stride = 0x%x", ulChromaStride));
	BDBG_MSG(("Luma   stride = 0x%x", ulLumaStride));

	/* Set stride */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) &= ~(
		BCHP_MASK(MFD_0_STRIDE, AVC_MPEG_CHROMA_LINE_STRIDE) |
		BCHP_MASK(MFD_0_STRIDE, AVC_MPEG_LUMA_LINE_STRIDE  ) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) |= (
		BCHP_FIELD_DATA(MFD_0_STRIDE, AVC_MPEG_CHROMA_LINE_STRIDE, ulChromaStride) |
		BCHP_FIELD_DATA(MFD_0_STRIDE, AVC_MPEG_LUMA_LINE_STRIDE,   ulLumaStride  ) );

	/* Set NMBY */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_LUMA_NMBY) &= ~BCHP_MASK(MFD_0_LUMA_NMBY, VALUE);
	BVDC_P_MFD_GET_REG_DATA(MFD_0_LUMA_NMBY) |=
		BCHP_FIELD_DATA(MFD_0_LUMA_NMBY, VALUE, pFieldData->ulLuminanceNMBY);

	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_NMBY) &= ~BCHP_MASK(MFD_0_CHROMA_NMBY, VALUE);
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CHROMA_NMBY) |=
		BCHP_FIELD_DATA(MFD_0_CHROMA_NMBY, VALUE, pFieldData->ulChrominanceNMBY);

	return BERR_SUCCESS;
}


static void BVDC_P_Feeder_GetDpbDeviceOffset_isr
	( BMMA_Block_Handle           hBlock,
	  uint32_t                    ulBlockOffset,
	  uint32_t                   *pulDevOffset )
{
	BMMA_DeviceOffset devOffset;

	devOffset = (hBlock != NULL) ? BMMA_GetOffset_isr(hBlock) : 0;

	*pulDevOffset = (uint32_t)devOffset + ulBlockOffset;
}

/***************************************************************************
	 * Start address calculation
	 * Note:
	 *   AVD frame buffer is organized by stripes (64 pixels wide, or 4
	 * MacroBlock colomns); different AVD frame buffers might be non-
	 * contiguous; H.264 picture might have stream clip;
	 */
static uint32_t BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr
	( BVDC_P_Feeder_StripeWidthConfig  eStripeWidthCfg,
	  uint32_t                         ulDeviceOffset,
	  uint32_t                         ulNMBY,
	  uint32_t                         ulLeftOffset,
	  uint32_t                         ulVertOffset,
	  bool                             bIsLuma,
	  const BAVC_MVD_Field            *pFieldData,
	  uint32_t                        *pulPicOffset )
{
	uint32_t        ulLeftOffsetInByte, ulPicOffset;
	uint32_t        ulStripeWidth, ulStripeShift, ulStride;

	if((pFieldData->ePxlFmt != BPXL_INVALID) && !bIsLuma)
		return ulDeviceOffset;

	ulStripeWidth   = eStripeWidthCfg.ulStripeWidth;
	ulStripeShift   = eStripeWidthCfg.ulShift;

	/* ulLeftOffset is already even pixels aligned. 10Bit needs to be 6 pixels aligned. */
	if(pFieldData->eBitDepth == BAVC_VideoBitDepth_e10Bit)
	{
		ulLeftOffsetInByte = BVDC_P_FEEDER_GET_ONE_THIRD(4 * ulLeftOffset) & ~7;
		ulPicOffset = ulLeftOffset % 6;
	}
	else
	{
		ulLeftOffsetInByte = ulLeftOffset;
		ulPicOffset = 0;
	}
	BDBG_MSG(("GetBaseAddr: ulLeftOffset: %d, ulLeftOffsetInByte: %d, ulPicOffset: %d",
		ulLeftOffset, ulLeftOffsetInByte, ulPicOffset));

	/* to calculate the chroma starting address including the pan/scan/cropping; */
	ulDeviceOffset +=
		/* stripe offset */
		(ulLeftOffsetInByte >> ulStripeShift) * ulNMBY * 16 * ulStripeWidth +

		/* vertical offset within a stripe (take care of 422 vs 420 chroma type) */
		ulVertOffset +

		/* horizontal offset within a stripe;
		 *  NOTE: we don't support 4:4:4 chroma; keep it even number; */
		(ulLeftOffsetInByte & (ulStripeWidth-1));

	if(pFieldData->ePxlFmt == BPXL_INVALID)
		ulStride = ulStripeWidth;
	else
		ulStride = pFieldData->ulRowStride;

#if (BVDC_P_MFD_FEEDER_IS_MVFD)
	/* adjustment for bottom field offset within a stripe on MVFD:
	 * Luma: add stripe width for bottom field
	 * Chroma: add stripe width for bottom field when CHROMA_INTERPOLATION is field
	 */
	if( (pFieldData->eSourcePolarity == BAVC_Polarity_eBotField) &&
	    (BAVC_DecodedPictureBuffer_eFieldsPair != pFieldData->eBufferFormat))
	{
		if((pFieldData->eChrominanceInterpolationMode == BAVC_InterpolationMode_eField) ||
		    bIsLuma )
		{
			ulDeviceOffset   += ulStride;
		}
	}

#else
	/* adjustment for bottom field offset within a stripe */
	if( (pFieldData->eSourcePolarity == BAVC_Polarity_eBotField) &&
	    (BAVC_DecodedPictureBuffer_eFieldsPair != pFieldData->eBufferFormat))
	{
		ulDeviceOffset   += ulStride ;
	}
#endif
	if(pulPicOffset)
		*pulPicOffset = ulPicOffset;

	return ulDeviceOffset;
}

/***************************************************************************
	 * Device address calculation
	 */
static void BVDC_P_Feeder_GetMpegDeviceAddr_isr
	( BVDC_P_Feeder_Handle                 hFeeder,
	  const BAVC_MVD_Field                *pFieldData,
	  const BVDC_P_Rect                   *pScanoutRect,
	  BVDC_P_Feeder_MpegDeviceAddrConfig  *pstDeviceAddr )
{
	uint32_t           lumaDevOffset, chromaDevOffset;
	uint32_t           ulLumaVertOffset, ulChromaVertOffset;

	uint32_t           ulTopOffset, ulLeftOffset, ulLeftOffset_R;
	uint32_t           ulStripeWidth;
	BAVC_YCbCrType     eYCbCrType = pFieldData->eYCbCrType;
	BVDC_P_Feeder_StripeWidthConfig    eStripeWidthCfg;

	BDBG_ASSERT(pstDeviceAddr);

	eStripeWidthCfg = s_aStripeWidthCfgTbl[pFieldData->eStripeWidth];
	ulStripeWidth   = eStripeWidthCfg.ulStripeWidth;

	/* a stripe is 64 pixels wide
	   NOTE: don't support 4:4:4; */
	ulLeftOffset = (pScanoutRect->lLeft + pFieldData->ulSourceClipLeft) & ~1;
	/* Get the offset of the right buffer */
	ulLeftOffset_R = (pScanoutRect->lLeft_R + pFieldData->ulSourceClipLeft +
		pFieldData->ulSourceHorizontalSize) & ~1;

	if(pFieldData->eYCbCrType == BAVC_YCbCrType_e4_2_0)
	{
		ulTopOffset  = (pScanoutRect->lTop  + pFieldData->ulSourceClipTop ) & ~1;
	}
	else
	{
		ulTopOffset  = pScanoutRect->lTop  + pFieldData->ulSourceClipTop;
	}

	ulLumaVertOffset   = ulTopOffset * ulStripeWidth;
	ulChromaVertOffset = (ulTopOffset * ulStripeWidth / 2) << (eYCbCrType - BAVC_YCbCrType_e4_2_0);

	/* get the 1st eye's top or frame buffer base address */
	BVDC_P_Feeder_GetDpbDeviceOffset_isr(pFieldData->hLuminanceFrameBufferBlock,
		pFieldData->ulLuminanceFrameBufferBlockOffset, &lumaDevOffset);
	BVDC_P_Feeder_GetDpbDeviceOffset_isr(pFieldData->hChrominanceFrameBufferBlock,
		pFieldData->ulChrominanceFrameBufferBlockOffset, &chromaDevOffset);
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
	if(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat) {
		/* get the 1st eye's bottom buffer base address */
		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pFieldData->hLuminanceBotFieldBufferBlock,
			pFieldData->ulLuminanceBotFieldBufferBlockOffset, &pstDeviceAddr->ulLumaBotDeviceAddr);
		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pFieldData->hChrominanceBotFieldBufferBlock,
			pFieldData->ulChrominanceBotFieldBufferBlockOffset, &pstDeviceAddr->ulChromaBotDeviceAddr);

		/* compute the bottom field starting address */
		pstDeviceAddr->ulLumaBotDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, pstDeviceAddr->ulLumaBotDeviceAddr,
			pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
			true, pFieldData, &hFeeder->ulPicOffset);

		pstDeviceAddr->ulChromaBotDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, pstDeviceAddr->ulChromaBotDeviceAddr,
			pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
			false, pFieldData, &hFeeder->ulPicOffset);
	}
#endif

	/* compute frame or top field starting address */
	pstDeviceAddr->ulLumaDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
		eStripeWidthCfg, (uint32_t)lumaDevOffset,
		pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
		true, pFieldData, &hFeeder->ulPicOffset);

	pstDeviceAddr->ulChromaDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
		eStripeWidthCfg, (uint32_t)chromaDevOffset,
		pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
		false, pFieldData, &hFeeder->ulPicOffset);

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	/* e3D_LeftRight has single picture buffer */
	if(pFieldData->eOrientation == BFMT_Orientation_e3D_LeftRight)
	{
		/* compute the top or frame buffer Right eye start address */
		pstDeviceAddr->ulLumaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, lumaDevOffset,
			pFieldData->ulLuminanceNMBY, ulLeftOffset_R, ulLumaVertOffset,
			true, pFieldData, &hFeeder->ulPicOffset_R);

		pstDeviceAddr->ulChromaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, chromaDevOffset,
			pFieldData->ulChrominanceNMBY, ulLeftOffset_R, ulChromaVertOffset,
			false, pFieldData, &hFeeder->ulPicOffset_R);
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		if(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat) {
			/* compute the bottom field Right-eye starting address */
			pstDeviceAddr->ulLumaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulLumaBotDeviceAddr,
				pFieldData->ulLuminanceNMBY, ulLeftOffset_R, ulLumaVertOffset,
				true, pFieldData, &hFeeder->ulPicOffset_R);

			pstDeviceAddr->ulChromaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulChromaBotDeviceAddr,
				pFieldData->ulChrominanceNMBY, ulLeftOffset_R, ulChromaVertOffset,
				false, pFieldData, &hFeeder->ulPicOffset_R);
		}
#endif
	}
	/* e3D_OverUnder has single picture buffer */
	else if(pFieldData->eOrientation == BFMT_Orientation_e3D_OverUnder)
	{
		uint32_t   ulTopOffset_R, ulLumaVertOffset_R, ulChromaVertOffset_R;

		/* Get the offset of the right buffer */
		if(pFieldData->eYCbCrType == BAVC_YCbCrType_e4_2_0)
		{
			ulTopOffset_R  = (pScanoutRect->lTop  + pFieldData->ulSourceClipTop
				+ pFieldData->ulSourceVerticalSize) & ~1;
		}
		else
		{
			ulTopOffset_R  = pScanoutRect->lTop  + pFieldData->ulSourceClipTop
				+ pFieldData->ulSourceVerticalSize;
		}

		ulLumaVertOffset_R = ulTopOffset_R * ulStripeWidth;
		ulChromaVertOffset_R = (ulTopOffset_R * ulStripeWidth / 2) << (eYCbCrType - BAVC_YCbCrType_e4_2_0);

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		if(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat) {
			/* compute the bottom field Right-eye starting address */
			pstDeviceAddr->ulLumaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulLumaBotDeviceAddr,
				pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset_R,
				true, pFieldData, &hFeeder->ulPicOffset_R);

			pstDeviceAddr->ulChromaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulChromaBotDeviceAddr,
				pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset_R,
				false, pFieldData, &hFeeder->ulPicOffset_R);
		}
#endif

		/* compute the top or frame buffer Right eye start address */
		pstDeviceAddr->ulLumaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, lumaDevOffset,
			pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset_R,
			true, pFieldData, &hFeeder->ulPicOffset_R);

		pstDeviceAddr->ulChromaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, chromaDevOffset,
			pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset_R,
			false, pFieldData, &hFeeder->ulPicOffset_R);

	}
	/* e3D_Left or e3D_Right has separate L/R picture buffers */
	else if((pFieldData->eOrientation == BFMT_Orientation_e3D_Left) ||
		(pFieldData->eOrientation == BFMT_Orientation_e3D_Right))
	{
		BAVC_MVD_Field  *pEnhancedFieldData;
		uint32_t  enhancedLumaDevOffset, enhancedChromaDevOffset;
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		uint32_t  enhancedLumaBotDevOffset, enhancedChromaBotDevOffset;
#endif

		pEnhancedFieldData = (BAVC_MVD_Field*)(pFieldData->pEnhanced);
		BDBG_ASSERT(pEnhancedFieldData);

		/* get the 2nd eye's top or frame buffer base address */
		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hLuminanceFrameBufferBlock,
			pEnhancedFieldData->ulLuminanceFrameBufferBlockOffset, &enhancedLumaDevOffset);
		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hChrominanceFrameBufferBlock,
			pEnhancedFieldData->ulChrominanceFrameBufferBlockOffset, &enhancedChromaDevOffset);

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		if(BAVC_DecodedPictureBuffer_eFieldsPair == pEnhancedFieldData->eBufferFormat) {
			BDBG_ASSERT(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat);
			/* get the 2nd eye's bottom buffer base address */
			BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hLuminanceBotFieldBufferBlock,
				pEnhancedFieldData->ulLuminanceBotFieldBufferBlockOffset, &enhancedLumaBotDevOffset);
			BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hChrominanceBotFieldBufferBlock,
				pEnhancedFieldData->ulChrominanceBotFieldBufferBlockOffset, &enhancedChromaBotDevOffset);
		}
		else
		{
			enhancedLumaBotDevOffset   = 0;
			enhancedChromaBotDevOffset = 0;
		}
#endif


		if(pFieldData->eOrientation == BFMT_Orientation_e3D_Left)
		{
			BDBG_ASSERT(pEnhancedFieldData->eOrientation == BFMT_Orientation_e3D_Right);

			/* pEnhancedFieldData is right buffer */
			pstDeviceAddr->ulLumaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, enhancedLumaDevOffset,
				pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
				true, pFieldData, &hFeeder->ulPicOffset_R);

			pstDeviceAddr->ulChromaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, enhancedChromaDevOffset,
				pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
				false, pFieldData, &hFeeder->ulPicOffset_R);

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
			if(BAVC_DecodedPictureBuffer_eFieldsPair == pEnhancedFieldData->eBufferFormat) {
				/* compute the 2nd eye's bottom field starting address */
				pstDeviceAddr->ulLumaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
					eStripeWidthCfg, enhancedLumaBotDevOffset,
					pEnhancedFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
					true, pEnhancedFieldData, &hFeeder->ulPicOffset_R);

				pstDeviceAddr->ulChromaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
					eStripeWidthCfg, enhancedChromaBotDevOffset,
					pEnhancedFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
					false, pEnhancedFieldData, &hFeeder->ulPicOffset_R);
			}
#endif
		}
		else
		{
			BDBG_ASSERT(pEnhancedFieldData->eOrientation == BFMT_Orientation_e3D_Left);

			/* 1st eye is Right, 2nd eye is Left: swap */
			pstDeviceAddr->ulLumaDeviceAddr_R   = pstDeviceAddr->ulLumaDeviceAddr;
			pstDeviceAddr->ulChromaDeviceAddr_R = pstDeviceAddr->ulChromaDeviceAddr;

			/* pEnhancedFieldData is left buffer */
			pstDeviceAddr->ulLumaDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, enhancedLumaDevOffset,
				pFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
				true, pFieldData, &hFeeder->ulPicOffset);

			pstDeviceAddr->ulChromaDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, enhancedChromaDevOffset,
				pFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
				false, pFieldData, &hFeeder->ulPicOffset);

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
			if(BAVC_DecodedPictureBuffer_eFieldsPair == pEnhancedFieldData->eBufferFormat) {
				pstDeviceAddr->ulLumaBotDeviceAddr_R   = pstDeviceAddr->ulLumaBotDeviceAddr;
				pstDeviceAddr->ulChromaBotDeviceAddr_R = pstDeviceAddr->ulChromaBotDeviceAddr;

				/* compute the 2nd eye's bottom field starting address */
				pstDeviceAddr->ulLumaBotDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
					eStripeWidthCfg, enhancedLumaBotDevOffset,
					pEnhancedFieldData->ulLuminanceNMBY, ulLeftOffset, ulLumaVertOffset,
					true, pEnhancedFieldData, &hFeeder->ulPicOffset);

				pstDeviceAddr->ulChromaBotDeviceAddr = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
					eStripeWidthCfg, enhancedChromaBotDevOffset,
					pEnhancedFieldData->ulChrominanceNMBY, ulLeftOffset, ulChromaVertOffset,
					false, pEnhancedFieldData, &hFeeder->ulPicOffset);
			}
#endif
		}
	}
	else if(pFieldData->eOrientation == BFMT_Orientation_eLeftRight_Enhanced)
	{
		BAVC_MVD_Field  *pEnhancedFieldData;
		uint32_t  enhancedLumaDevOffset, enhancedChromaDevOffset;

		pEnhancedFieldData = (BAVC_MVD_Field*)(pFieldData->pEnhanced);
		BDBG_ASSERT(pEnhancedFieldData);

		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hLuminanceFrameBufferBlock,
			pEnhancedFieldData->ulLuminanceFrameBufferBlockOffset, &enhancedLumaDevOffset);
		BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hChrominanceFrameBufferBlock,
			pEnhancedFieldData->ulChrominanceFrameBufferBlockOffset, &enhancedChromaDevOffset);

		/* Get the offset of the right buffer */
		ulLeftOffset_R = (pScanoutRect->lLeft_R + pFieldData->ulSourceClipLeft +
			pFieldData->ulSourceHorizontalSize/2) & ~1;

		pstDeviceAddr->ulLumaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, enhancedLumaDevOffset,
			pFieldData->ulLuminanceNMBY, ulLeftOffset_R, ulLumaVertOffset,
			true, pFieldData, &hFeeder->ulPicOffset_R);

		pstDeviceAddr->ulChromaDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
			eStripeWidthCfg, enhancedChromaDevOffset,
			pFieldData->ulChrominanceNMBY, ulLeftOffset_R, ulChromaVertOffset,
			false, pFieldData, &hFeeder->ulPicOffset_R);

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		if(BAVC_DecodedPictureBuffer_eFieldsPair == pEnhancedFieldData->eBufferFormat) {
			uint32_t  enhancedLumaBotDevOffset, enhancedChromaBotDevOffset;
			BDBG_ASSERT(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat);

			/* get the 2nd eye's bottom buffer base address */
			BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hLuminanceBotFieldBufferBlock,
				pEnhancedFieldData->ulLuminanceBotFieldBufferBlockOffset, &enhancedLumaBotDevOffset);
			BVDC_P_Feeder_GetDpbDeviceOffset_isr(pEnhancedFieldData->hChrominanceBotFieldBufferBlock,
				pEnhancedFieldData->ulChrominanceBotFieldBufferBlockOffset, &enhancedChromaBotDevOffset);

			/* compute the bottom field Right-eye starting address */
			pstDeviceAddr->ulLumaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulLumaBotDeviceAddr,
				pEnhancedFieldData->ulLuminanceNMBY, ulLeftOffset_R, ulLumaVertOffset,
				true, pEnhancedFieldData, &hFeeder->ulPicOffset_R);

			pstDeviceAddr->ulChromaBotDeviceAddr_R = BVDC_P_Feeder_GetMpegDeviceBaseAddr_isr(
				eStripeWidthCfg, pstDeviceAddr->ulChromaBotDeviceAddr,
				pEnhancedFieldData->ulChrominanceNMBY, ulLeftOffset_R, ulChromaVertOffset,
				false, pEnhancedFieldData, &hFeeder->ulPicOffset_R);
		}
#endif
	}
	else
	{
		pstDeviceAddr->ulLumaDeviceAddr_R   = pstDeviceAddr->ulLumaDeviceAddr;
		pstDeviceAddr->ulChromaDeviceAddr_R = pstDeviceAddr->ulChromaDeviceAddr;
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
		if(BAVC_DecodedPictureBuffer_eFieldsPair == pFieldData->eBufferFormat) {
			pstDeviceAddr->ulLumaBotDeviceAddr_R   = pstDeviceAddr->ulLumaBotDeviceAddr;
			pstDeviceAddr->ulChromaBotDeviceAddr_R = pstDeviceAddr->ulChromaBotDeviceAddr;
		}
#endif
	}
#endif

	/* TODO: clarify the usage of this feature */
	if(hFeeder->bSkipFirstLine)
	{
		pstDeviceAddr->ulLumaDeviceAddr += hFeeder->ulLumaStride;
		pstDeviceAddr->ulChromaDeviceAddr += hFeeder->ulChromaStride;

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
/* TODO: anything to do here?? */
#endif

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
		pstDeviceAddr->ulChromaDeviceAddr_R += hFeeder->ulChromaStride;
		pstDeviceAddr->ulLumaDeviceAddr_R += hFeeder->ulLumaStride;
#endif
	}

	return;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_Feeder_SetMpegAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_Rect               *pScanoutRect)
{
	BVDC_P_Feeder_MpegDeviceAddrConfig   stMpegDeviceAddr;

	BKNI_Memset((void*)&stMpegDeviceAddr, 0x0, sizeof(BVDC_P_Feeder_MpegDeviceAddrConfig));

	hFeeder->stGfxSurface.ulMainByteOffset = 0;/* clear the gfx offset for striped surface */
	if (!hFeeder->bGfxSrc || pFieldData->ePxlFmt == BPXL_INVALID)
	{
		BVDC_P_Feeder_GetMpegDeviceAddr_isr(hFeeder, pFieldData, pScanoutRect,
			&stMpegDeviceAddr);
	}
#if (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PIC_OFFSET) &= ~(
		BCHP_MASK(MFD_0_PIC_OFFSET,  H_OFFSET_R) |
		BCHP_MASK(MFD_0_PIC_OFFSET,  H_OFFSET));
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PIC_OFFSET) |= (
		BCHP_FIELD_DATA(MFD_0_PIC_OFFSET, H_OFFSET_R, hFeeder->ulPicOffset_R) |
		BCHP_FIELD_DATA(MFD_0_PIC_OFFSET, H_OFFSET, hFeeder->ulPicOffset));
#endif

	/* Set address */
	/* MFD_0_PICTURE0_LINE_ADDR_0 */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0, AVC_MPEG_LUMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0, AVC_MPEG_LUMA_ADDR,
			stMpegDeviceAddr.ulLumaDeviceAddr) );

	/* MFD_0_PICTURE0_LINE_ADDR_1 */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_1) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_1, AVC_MPEG_CHROMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_1) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_1, AVC_MPEG_CHROMA_ADDR,
			stMpegDeviceAddr.ulChromaDeviceAddr) );

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
	/* MFD_0_PICTURE1_LINE_ADDR_0 */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_0) &= ~(
		BCHP_MASK(MFD_0_PICTURE1_LINE_ADDR_0, AVC_MPEG_LUMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_0) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE1_LINE_ADDR_0, AVC_MPEG_LUMA_ADDR,
			stMpegDeviceAddr.ulLumaBotDeviceAddr) );

	/* MFD_0_PICTURE1_LINE_ADDR_1 */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_1) &= ~(
		BCHP_MASK(MFD_0_PICTURE1_LINE_ADDR_1, AVC_MPEG_CHROMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_1) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE1_LINE_ADDR_1, AVC_MPEG_CHROMA_ADDR,
			stMpegDeviceAddr.ulChromaBotDeviceAddr) );
#endif

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	/* MFD_0_PICTURE0_LINE_ADDR_0_R */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0_R, AVC_MPEG_LUMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R, AVC_MPEG_LUMA_ADDR,
			stMpegDeviceAddr.ulLumaDeviceAddr_R) );

	/* MFD_0_PICTURE0_LINE_ADDR_1_R */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_1_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_1_R, AVC_MPEG_CHROMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_1_R) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_1_R, AVC_MPEG_CHROMA_ADDR,
			stMpegDeviceAddr.ulChromaDeviceAddr_R) );

#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
	/* MFD_0_PICTURE1_LINE_ADDR_0_R */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_0_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE1_LINE_ADDR_0_R, AVC_MPEG_LUMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_0_R) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE1_LINE_ADDR_0_R, AVC_MPEG_LUMA_ADDR,
			stMpegDeviceAddr.ulLumaBotDeviceAddr_R) );

	/* MFD_0_PICTURE1_LINE_ADDR_1_R */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_1_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE1_LINE_ADDR_1_R, AVC_MPEG_CHROMA_ADDR) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE1_LINE_ADDR_1_R) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE1_LINE_ADDR_1_R, AVC_MPEG_CHROMA_ADDR,
			stMpegDeviceAddr.ulChromaBotDeviceAddr_R) );
#endif
#endif

	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_SetSurfaceStrideAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_Rect               *pScanoutRect)
{
	uint32_t  ulStride, ulOffsetByteInLine;
	BERR_Code     err = BERR_SUCCESS;

	/* BVDC_P_Source_ValidateMpegData_isr overwrite
	 * pFieldData->ulRowStride to use surface stride */
	/* Gfx surface with field polarity does leave space for the other poalrity */
	ulStride = (pFieldData->eSourcePolarity == BAVC_Polarity_eFrame)
		? pFieldData->ulRowStride : 2 * pFieldData->ulRowStride;

	BPXL_GetBytesPerNPixels_isr(pFieldData->ePxlFmt,
		(pScanoutRect->lLeft & ~1), &ulOffsetByteInLine);
	hFeeder->stGfxSurface.ulMainByteOffset =
		pScanoutRect->lTop * ulStride + ulOffsetByteInLine;

	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) &= ~(
		BCHP_MASK(MFD_0_STRIDE, PACKED_LINE_STRIDE) );
	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) |=
		BCHP_FIELD_DATA(MFD_0_STRIDE, PACKED_LINE_STRIDE, ulStride);

	/* Set address: real addr is set to scratch registers */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0, PACKED_LUMA_CHROMA_ADDR) );
#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0_R, PACKED_LUMA_CHROMA_ADDR) );
#endif

	return err;
}

/***************************************************************************
 * {private}
 *
 * Input:
 *   pFieldData:
 *      Only contains the data from stream.
 *   pScanoutRect:
 *       Points to the actual display rectangle feeder needs to feeds out.
 *       See BVDC_P_Source_GetScanOutRect for detailed information.
 *   pScanOutRect->lLeft:
 *      This is the horizontal pan scan for feeder, a positive number in the format of U32,
 *      where bits 0-3 are pixel pan scan in pixel grid, bits 4-31 are macroblock pan
 *      scan in macroblock grid. Even number of pixels are handled in feeder, others handled
 *      by scaler.
 *  pScanOutRect->lTop:
  *     This is the vertical pan scan for feeder, a positive number in the format of U32,
 *      where bits 0-3 are pixel pan scan in pixel grid, bits 4-31 are macroblock pan scan
 *      in macroblock grid. Down to pixels are handled in feeder, others handled by scaler.
 *  pScanOutRect->ulWidth:
 *      This is the horizontal scan out size, a positive number in units of pixel.
 *  pScanOutRect->ulHeight:
 *      This is the vertical scan out size, a positive number in units of pixel.
 *
   ==============
  Interlaced HEVC will have eBufferFormat = eFieldsPair, which has special treatment.
  Here are the programming combinations for eBufferFormat = BAVC_DecodedPictureBuffer_eFieldsPair:
  1. Interlaced scanout (eSourcePolarity = top or bottom)
     The both top/bottom field buffers pair need to be provided; VDC will program MFD address registers accordingly:
     1) On 7445C0, MFD hw only support luma/chroma frame buffer, so VDC can program MFD picture address registers
         with the buffer matching with the eSourcePolarity; MFD scanout type set to progressive; MFD chroma interpolation
         register set to frame interpolation and ignore the one from DM.
     2) On 7445D0 and beyond MFD hw that supports field pair buffers, VDC programming both top/bottom address
         registers with provided field pair buffers; MFD scanout type set to interlaced; MFD chroma interpolation register set
         according to the one in BAVC structure; MFD_x_LAC_CNTL.SEPARATE_FIELD_BUFFER = 1.
  2. Progressive scanout (eSourcePolarity = frame) (this is rare but could be true if user forces XVD to scanout
     progressive from interlaced pulldown field sequence)
     The both top/bottom field buffers pair need to be provided; VDC will program MFD address registers accordingly:
     1) On 7445C0, MFD hw only support luma/chroma frame buffer, so VDC may report error and scanout mute color.
     2) On 7445D0 and beyond MFD hw that supports field pair buffers, VDC programming both top/bottom address
         registers with provided field pair buffers; MFD scanout type set to progressive; MFD chroma interpolation register
         set according to the one in BAVC structure; MFD_x_LAC_CNTL.SEPARATE_FIELD_BUFFER = 1.

  If eBufferFormat = BAVC_DecodedPictureBuffer_eFrame, only the first set of luma/chroma buffers are provided (VDC may
         report error if the 2nd set of buffers are not NULL). MFD is programmed the same as legacy platforms.
*/
BERR_Code BVDC_P_Feeder_SetMpegInfo_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const uint32_t                   ulMuteColorYCrCb,
	  const BAVC_MVD_Field            *pPicture,
	  const BVDC_P_Rect               *pScanout)
{
	BAVC_MVD_Field *pFieldData = &hFeeder->stMfdPicture;
	BVDC_P_Rect stRect = *pScanout;
	BVDC_P_Rect *pScanoutRect = &stRect;
	BDBG_ENTER(BVDC_P_Feeder_SetMpegInfo_isr);
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

	/* Check parameter */
	BDBG_ASSERT(BVDC_P_Feeder_IS_MPEG(hFeeder));

	BKNI_Memcpy_isr(&hFeeder->stMfdPicture, pPicture, sizeof(BAVC_MVD_Field));

	if(pFieldData->eBufferFormat == BAVC_DecodedPictureBuffer_eFieldsPair) {
		/* 1) fields buffers vertical croppings are halved of normalized frame vsize; */
		pFieldData->ulSourceClipTop /= 2;
		if(pFieldData->eBarDataType == BAVC_BarDataType_eTopBottom) {
			pFieldData->ulTopLeftBarValue  /= 2;
			pFieldData->ulBotRightBarValue /= 2;
		}
		/* Note: window rectangle calculation is also based on normalized frame size! */
		pScanoutRect->lTop     /= 2;

#if !(BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for legacy hw that doesn't support fields pair */
		/* assume interlaced fields pair buffers only for interlaced format */
		BDBG_ASSERT(pFieldData->eSourcePolarity != BAVC_Polarity_eFrame);

		/* 2) take correct field buffer address as faked frame buffer address */
		if(BAVC_Polarity_eBotField == pFieldData->eSourcePolarity) {
			pFieldData->hLuminanceFrameBufferBlock = pFieldData->hLuminanceBotFieldBufferBlock;
			pFieldData->ulLuminanceFrameBufferBlockOffset = pFieldData->ulLuminanceBotFieldBufferBlockOffset;
			pFieldData->hChrominanceFrameBufferBlock = pFieldData->hChrominanceBotFieldBufferBlock;
			pFieldData->ulChrominanceFrameBufferBlockOffset = pFieldData->ulChrominanceBotFieldBufferBlockOffset;
		}

		/* 3) override with progressive scanout in legacy frame buffer format */
		pFieldData->ulSourceVerticalSize  /= 2;
		pScanoutRect->ulHeight /= 2;/* halved height in progressive scanout */
		pFieldData->eChrominanceInterpolationMode = BAVC_InterpolationMode_eFrame;
		pFieldData->eSourcePolarity = BAVC_Polarity_eFrame;
		pFieldData->eBufferFormat = BAVC_DecodedPictureBuffer_eFrame;
		/*BDBG_MSG(("Legacy MFD hw scans out interlaced field buffer progressively."));*/
#endif
	}

	/* Chroma interpolation mode can be frame or field when scan
	 * type is interlaced. Chroma interpolation mode is always
	 * frame when scan type is progressive */
	if((!pFieldData->bMute) && (pFieldData->ePxlFmt == BPXL_INVALID) &&
	   (pFieldData->eSourcePolarity == BAVC_Polarity_eFrame) &&
	   (pFieldData->eChrominanceInterpolationMode == BAVC_InterpolationMode_eField))
	{
		BDBG_WRN(("Bad scan type %d and interpolation mode combination %d",
			pFieldData->eSourcePolarity, pFieldData->eChrominanceInterpolationMode));
	}

	if(pFieldData->eStripeWidth >= BVDC_P_FEEDER_STRIPE_WIDTH_CONFIG_TABLE_CNT)
	{
		BDBG_WRN(("Bad eStripeWidth %d", pFieldData->eStripeWidth));
		BDBG_ASSERT(0);
		return BERR_INVALID_PARAMETER;
	}

	/* Common MPEG settings */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_HSIZE) &= ~(BCHP_MASK(MFD_0_DISP_HSIZE, VALUE));
	BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_HSIZE) |= (
		BCHP_FIELD_DATA(MFD_0_DISP_HSIZE, VALUE, pScanoutRect->ulWidth) );
	BDBG_MSG(("Scanout HSIZE = 0x%x", pScanoutRect->ulWidth));

	BVDC_P_Feeder_SetVertWindow_isr(hFeeder, pFieldData->eSourcePolarity,
		0, pScanoutRect->ulHeight);

	/* Gather information */
	BVDC_P_Feeder_GetFormatCtrlSettings_isr(hFeeder, pFieldData,
		pScanoutRect, NULL);

	/* Check if need to do fixed color feed */
	hFeeder->bFixedColor = pFieldData->bMute;
	if( pFieldData->bMute )
	{
		BVDC_P_Feeder_SetFixedColor_isr(hFeeder, pFieldData->eSourcePolarity,
			ulMuteColorYCrCb);
	}
	else
	{
		/*
		* Start format change settings
		*/
		BVDC_P_Feeder_SetFormatCtrl_isr(hFeeder, pFieldData, NULL,
			pFieldData->eSourcePolarity);

		/* Set strides and buf start addr */
		if(pFieldData->ePxlFmt == BPXL_INVALID)
		{
			BVDC_P_Feeder_SetMpegStride_isr(hFeeder, pFieldData);
		}
		else
		{
			BVDC_P_Feeder_SetSurfaceStrideAddr_isr(hFeeder, pFieldData,
				pScanoutRect);
		}

		if (!hFeeder->bGfxSrc || pFieldData->ePxlFmt == BPXL_INVALID)
		{
			BVDC_P_Feeder_SetMpegAddr_isr(hFeeder, pFieldData, pScanoutRect);
		}
	}

#if (BVDC_P_MFD_SUPPORT_CSC)
	BVDC_P_Feeder_SetCsc_isr(hFeeder, pFieldData);
#endif

	/* Enable feeder */
	if( pScanoutRect->ulWidth )
	{
		BVDC_P_Feeder_SetEnable_isr(hFeeder, true);
	}
	else
	{
		BVDC_P_Feeder_SetEnable_isr(hFeeder, false);
	}

	BDBG_LEAVE(BVDC_P_Feeder_SetMpegInfo_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_Feeder_SetPlaybackStrideAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BVDC_P_PictureNodePtr      pPicture,
	  bool                             bProgressiveCap,
	  bool                             bEnableAlignWkr,
	  BAVC_Polarity                    ePolarity)
{
	unsigned int   uiStride, uiByteOffset, ulWidth;
	uint32_t  ulDeviceAddr, ulStride;
#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	uint32_t  ulDeviceAddr_R = 0;
#endif

	if (hFeeder->bGfxSrc)
	{
		BVDC_P_Rect *pSrcOut = &pPicture->stSrcOut;
		uint32_t  ulBitsPerPixel, ulOffsetByteInLine;

		ulBitsPerPixel = BPXL_BITS_PER_PIXEL(pPicture->ePixelFormat);
		ulOffsetByteInLine = ulBitsPerPixel * pSrcOut->lLeft / 8;
		ulStride = (ePolarity == BAVC_Polarity_eFrame)?
			hFeeder->stGfxSurface.stCurSurInfo.ulPitch :
			hFeeder->stGfxSurface.stCurSurInfo.ulPitch * 2;
		ulDeviceAddr = 0; /* really set from scratch register */
		hFeeder->stGfxSurface.ulMainByteOffset =
			pSrcOut->lTop * ulStride + ulOffsetByteInLine;
	}
	else
	{
		/* Stride is defined by capture width and alignment has to match
		   with CAP pitch as well! */
		ulWidth = (pPicture->pVfdIn->ulWidth + 1) & ~0x1;

#if (BVDC_P_MFD_SUPPORT_LPDDR4_MEMORY_PITCH)
		if(pPicture->bMosaicMode)
		{
			if(pPicture->bEnableDcxm)
				ulStride = (ulWidth * BVDC_P_DCXM_BITS_PER_PIXEL) / 8;
			else
				BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &ulStride);
			ulStride += (pPicture->eCapOrientation == BFMT_Orientation_e2D)
				? BVDC_P_CAP_LPDDR4_GUARD_MEMORY_2D : BVDC_P_CAP_LPDDR4_GUARD_MEMORY_3D;
		}
		else
		{
			BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &uiStride);
			ulStride = BVDC_P_ALIGN_UP(uiStride, BVDC_P_PITCH_ALIGN);
		}
#else
		BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat, ulWidth, &uiStride);
		ulStride = BVDC_P_ALIGN_UP(uiStride, BVDC_P_PITCH_ALIGN);
#endif

		/* CaptureScaler mode analog video needs to handle source clipping here */
		/* we allow different vbi pass-through lines from the same vdec source */
		BPXL_GetBytesPerNPixels_isr(pPicture->ePixelFormat,
			(pPicture->pVfdOut->lLeft & ~1), &uiByteOffset);

		/* Convert address to device offset address */
		ulDeviceAddr = BVDC_P_Buffer_GetDeviceOffset(pPicture);
		ulDeviceAddr += uiByteOffset +
			 ulStride * (bProgressiveCap ? pPicture->pVfdOut->lTop : pPicture->pVfdOut->lTop / 2);

		if(bEnableAlignWkr)
			ulDeviceAddr += 4;

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
		/* TODO: optimize ulDeviceAddr and ulDeviceAddr_R */
		if(hFeeder->hWindow && hFeeder->hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)
		{
			ulDeviceAddr_R = BVDC_P_Buffer_GetDeviceOffset_R(pPicture);
			ulDeviceAddr_R += uiByteOffset +
				 ulStride * (bProgressiveCap ? pPicture->pVfdOut->lTop : pPicture->pVfdOut->lTop / 2);
			if(bEnableAlignWkr)
				ulDeviceAddr_R += 4;
		}
		else if(hFeeder->hWindow && hFeeder->hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRCombined)
		{
			uint32_t  ulBufHeapSize;

			BVDC_P_BufferHeap_GetHeapSizeById_isr(
				hFeeder->hWindow->hCompositor->hVdc->hBufferHeap,
				hFeeder->hWindow->eBufferHeapIdRequest, &ulBufHeapSize);

			ulDeviceAddr_R = ulDeviceAddr + ulBufHeapSize / 2;
		}
		else
		{
			/* If source is 2D, set right buffer same as left in case display
			 * is 3D mode */
			ulDeviceAddr_R = ulDeviceAddr;
		}
#endif

		if(ePolarity != BAVC_Polarity_eFrame)
		{
			/* bottom field starts from one frame line later; */
			if(BAVC_Polarity_eBotField == ePolarity)
			{
				bool   bAddStride = true;

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
				if(hFeeder->bSupportDcxm && pPicture->bEnableDcxm)
					bAddStride = false;
#endif

				if(bAddStride)
				{
					/* one frame line stride */
					ulDeviceAddr += ulStride;
#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
					ulDeviceAddr_R += ulStride;
#endif
				}
			}
			/* Double the stride because we captured frames and scanout as fields. */
			ulStride *= 2;
		}
	}


	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) &= ~(
		BCHP_MASK(MFD_0_STRIDE, PACKED_LINE_STRIDE) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_STRIDE) |= (
		BCHP_FIELD_DATA(MFD_0_STRIDE, PACKED_LINE_STRIDE, ulStride) );

	/* Set address */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0, PACKED_LUMA_CHROMA_ADDR) );

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	if((pPicture->eDispOrientation != BFMT_Orientation_e2D) &&
	   (pPicture->e3dSrcBufSel == BVDC_3dSourceBufferSelect_eRightBuffer))
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) |= (
			BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0, PACKED_LUMA_CHROMA_ADDR, ulDeviceAddr_R) );
	}
	else
#endif
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0) |= (
			BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0, PACKED_LUMA_CHROMA_ADDR, ulDeviceAddr) );
	}

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	/* Set address */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_LINE_ADDR_0_R, PACKED_LUMA_CHROMA_ADDR) );

	if((pPicture->eDispOrientation != BFMT_Orientation_e2D) &&
	   (pPicture->e3dSrcBufSel == BVDC_3dSourceBufferSelect_eLeftBuffer))
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) |= (
			BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R, PACKED_LUMA_CHROMA_ADDR, ulDeviceAddr) );
	}
	else
	{
		BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R) |= (
			BCHP_FIELD_DATA(MFD_0_PICTURE0_LINE_ADDR_0_R, PACKED_LUMA_CHROMA_ADDR, ulDeviceAddr_R) );
	}
#endif

	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_SetPlaybackInfo_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BVDC_P_PictureNodePtr      pPicture,
	  bool                             bFixedColor,
	  const uint32_t                   ulMuteColorYCrCb)
{
	uint32_t                   ulHeight, ulWidth;
	bool                       bProgressiveCap = false;/* capture type */
	BAVC_Polarity              ePolarity;/* VFD scanout type */
#if BFMT_LEGACY_3DTV_SUPPORT
	BFMT_VideoFmt              eDispVideoFmt;
#endif
	bool                       bEnableAlignWkr = false;

	BDBG_ENTER(BVDC_P_Feeder_SetPlaybackInfo_isr);
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

	/* Common video feeder settings */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_HSIZE) &= ~(BCHP_MASK(MFD_0_DISP_HSIZE, VALUE));

	hFeeder->bFixedColor = bFixedColor;
	if (hFeeder->bGfxSrc)
	{
		BVDC_P_SurfaceInfo  *pCurSur = &hFeeder->stGfxSurface.stCurSurInfo;

		ulHeight = pCurSur->ulHeight;
		ulWidth = pCurSur->ulWidth;

		ePolarity = hFeeder->stPicture.eSrcPolarity;
		bProgressiveCap = (BAVC_Polarity_eFrame == ePolarity);

		BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_HSIZE) |= (
			BCHP_FIELD_DATA(MFD_0_DISP_HSIZE, VALUE, ulWidth & ~1));
	}
	else
	{
		ulWidth = pPicture->pVfdOut->ulWidth;

		/* need to determine the scanout size based on the picture polarity of src or display */
		bProgressiveCap =
			((BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)
			 ? pPicture->eDstPolarity : pPicture->eSrcPolarity) == BAVC_Polarity_eFrame);

#if BFMT_LEGACY_3DTV_SUPPORT
		eDispVideoFmt = pPicture->hBuffer->hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt;
#endif
		/* frame capture -> interlaced playback */
		if ((pPicture->eDstPolarity == BAVC_Polarity_eFrame) &&
				(pPicture->eDisplayPolarity != BAVC_Polarity_eFrame)
#if BFMT_LEGACY_3DTV_SUPPORT
			&& (!VIDEO_FORMAT_IS_CUSTOM_1080P3D(eDispVideoFmt))
#endif
			)
		{
			ePolarity = pPicture->eDisplayPolarity;
			/* progressive capture and interlaced playback might have odd frame height value! */
			ulHeight = pPicture->pVfdOut->ulHeight & ~1;
		}
		else
		{
			ePolarity = BAVC_Polarity_eFrame;
#if BFMT_LEGACY_3DTV_SUPPORT
			ulHeight = (bProgressiveCap &&
				(!VIDEO_FORMAT_IS_CUSTOM_1080P3D(eDispVideoFmt) ||
				 !BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)))
				? pPicture->pVfdOut->ulHeight : pPicture->pVfdOut->ulHeight / 2;
#else
			ulHeight = bProgressiveCap ? pPicture->pVfdOut->ulHeight : pPicture->pVfdOut->ulHeight / 2;
#endif
		}

	/* mpeg source playback vfd always scan out the whole captured progressively */
		{
			ulWidth = (ulWidth + 1) & ~1;
#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
			if(ulWidth % BVDC_P_SCB_BURST_SIZE == 2)
			{
				BDBG_MSG(("Feeder[%d] Enable alignment workaround for MVFD for Width %d",
					hFeeder->eId, ulWidth));
				bEnableAlignWkr = true;
			}
#endif

			BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_HSIZE) |= (
				BCHP_FIELD_DATA(MFD_0_DISP_HSIZE, VALUE, ulWidth));
		}
	}

	BVDC_P_Feeder_SetVertWindow_isr(hFeeder, ePolarity, 0, ulHeight);

	/* Check if need to do fixed color feed */
	if( bFixedColor )
	{
		BVDC_P_Feeder_SetFixedColor_isr(hFeeder, BAVC_Polarity_eFrame,
			ulMuteColorYCrCb);
	}
	else
	{
		/* Gather information */
		BVDC_P_Feeder_GetFormatCtrlSettings_isr(hFeeder, NULL, NULL, pPicture);

		/*
		* Start format change settings
		*/
		BVDC_P_Feeder_SetFormatCtrl_isr(hFeeder, NULL, pPicture, ePolarity);

		/* Set strides and buf start addr */
		BVDC_P_Feeder_SetPlaybackStrideAddr_isr(hFeeder,
			pPicture, bProgressiveCap, bEnableAlignWkr, ePolarity);
	}

	/* Enable feeder */
	if(ulWidth)
	{
		BVDC_P_Feeder_SetEnable_isr(hFeeder, true);
	}
	else
	{
		BVDC_P_Feeder_SetEnable_isr(hFeeder, false);
	}

	BDBG_LEAVE(BVDC_P_Feeder_SetPlaybackInfo_isr);
	return;
}

/***************************************************************************
 *
 */
static void BVDC_P_Feeder_SetFixedColor_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BAVC_Polarity                    ePolarity,
	  const uint32_t                   ulMuteColorYCrCb )
{
	uint8_t               ucLuma, ucCb, ucCr;

	/* Fixed color feed. Program every format change.*/
	ucLuma = (uint8_t)BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, ulMuteColorYCrCb, 2);
	ucCb   = (uint8_t)BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, ulMuteColorYCrCb, 1);
	ucCr   = (uint8_t)BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, ulMuteColorYCrCb, 0);

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) &= ~(
#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
		BCHP_MASK(MFD_0_FEEDER_CNTL, MEM_VIDEO          ) |
		BCHP_MASK(MFD_0_FEEDER_CNTL, BVB_VIDEO          ) |
#endif
		BCHP_MASK(MFD_0_FEEDER_CNTL, IMAGE_FORMAT       ) |
		BCHP_MASK(MFD_0_FEEDER_CNTL, FIXED_COLOUR_ENABLE) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FEEDER_CNTL) |= (
#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, MEM_VIDEO, MODE_2D     ) |
		BCHP_FIELD_DATA(MFD_0_FEEDER_CNTL, BVB_VIDEO, hFeeder->eOutputOrientation)|
#endif
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, IMAGE_FORMAT, PACKED   ) |
		BCHP_FIELD_ENUM(MFD_0_FEEDER_CNTL, FIXED_COLOUR_ENABLE, ON) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FIXED_COLOUR) &= ~(
		BCHP_MASK(MFD_0_FIXED_COLOUR, LUMA) |
		BCHP_MASK(MFD_0_FIXED_COLOUR, CB  ) |
		BCHP_MASK(MFD_0_FIXED_COLOUR, CR  ) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_FIXED_COLOUR) |= (
		BCHP_FIELD_DATA(MFD_0_FIXED_COLOUR, LUMA, ucLuma) |
		BCHP_FIELD_DATA(MFD_0_FIXED_COLOUR, CB,   ucCb  ) |
		BCHP_FIELD_DATA(MFD_0_FIXED_COLOUR, CR,   ucCr  ) );

	BVDC_P_Feeder_SetLacCntlOutput_isr(hFeeder, ePolarity);

#if BVDC_P_MFD_FIXED_COLOR_WORKAROUND
	/* reset feeder in case transition from memory feed to fixed color stalls. */
	hFeeder->bInitial = true;
#endif
	return;
}

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_P_Feeder_SetVertWindow_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_Polarity              eSourcePolarity,
	  const uint32_t                   ulTop,
	  const uint32_t                   ulHeight)
{
#if (BVDC_P_SUPPORT_MFD_VER < BVDC_P_MFD_VER_10)
	uint32_t   ulVertStart, ulVertEnd;

	switch( eSourcePolarity )
	{
	case BAVC_Polarity_eTopField:
		/* Top field interlaced scan out from 0 to (height - 2) */
		ulVertStart  = ulTop;
		ulVertEnd    = ulTop + ulHeight - 2;
		BDBG_ASSERT((ulVertStart & 1) == (ulVertEnd & 1));
		break;

	case BAVC_Polarity_eBotField:
		/* Bottom field interlaced scan out from 1 to (height - 1) */
		ulVertStart  = ulTop + 1;
		ulVertEnd    = ulTop + ulHeight - 1;
		BDBG_ASSERT((ulVertStart & 1) == (ulVertEnd & 1));
		break;

	case BAVC_Polarity_eFrame:
		/* Progressive scan out from 0 to (height - 1) */
		ulVertStart  = ulTop;
		ulVertEnd    = ulTop + ulHeight - 1;
		break;

	default:
		/* Top field interlaced scan out from 0 to (height - 2) */
		ulVertStart  = ulTop;
		ulVertEnd    = ulTop + ulHeight - 2;
		break;
	}

	BDBG_MSG(("Feeder %d ePolarity = %d", hFeeder->eId, eSourcePolarity));
	BDBG_MSG(("Scanout VERT_WIN_START = %d", ulVertStart));
	BDBG_MSG(("Scanout VERT_WIN_END   = %d", ulVertEnd));
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_DISP_VERT_WINDOW) &= ~(
		BCHP_MASK(MFD_0_PICTURE0_DISP_VERT_WINDOW, START) |
		BCHP_MASK(MFD_0_PICTURE0_DISP_VERT_WINDOW, END  ) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PICTURE0_DISP_VERT_WINDOW) |= (
		BCHP_FIELD_DATA(MFD_0_PICTURE0_DISP_VERT_WINDOW, START, ulVertStart) |
		BCHP_FIELD_DATA(MFD_0_PICTURE0_DISP_VERT_WINDOW, END,   ulVertEnd  ) );
#else
	BDBG_MSG(("Feeder %d ePolarity = %d", hFeeder->eId, eSourcePolarity));
	BDBG_MSG(("Scanout MFD_0_DISP_VSIZE = %d",
		(BAVC_Polarity_eFrame==eSourcePolarity)
		? (ulHeight - ulTop) : ((ulHeight - ulTop)/2)));
	BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_VSIZE) &= ~(
		BCHP_MASK(MFD_0_DISP_VSIZE, VALUE));

	BVDC_P_MFD_GET_REG_DATA(MFD_0_DISP_VSIZE) |= (
		BCHP_FIELD_DATA(MFD_0_DISP_VSIZE, VALUE, (BAVC_Polarity_eFrame==eSourcePolarity)
			? (ulHeight - ulTop) : ((ulHeight - ulTop)/2)) );
#endif

	return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Feeder_SetEnable_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  bool                             bEnable )
{
	/* Turn on/off the feeder. */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_PIC_FEED_CMD) &= ~(
		BCHP_MASK(MFD_0_PIC_FEED_CMD, START_FEED) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_PIC_FEED_CMD) |= (
		BCHP_FIELD_DATA(MFD_0_PIC_FEED_CMD, START_FEED, bEnable) );

	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Feeder_GetCrc_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BREG_Handle                      hReg,
	  BVDC_Source_CallbackData        *pData )
{
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);
	BDBG_ASSERT(hFeeder->eId < BVDC_P_FeederId_eVfd0);

	/* Fetch the new CRC values */
#if (BVDC_P_MFD_NEED_CRC_WORKAROUND)
	pData->ulLumaCrc   = BREG_Read32_isr(hReg, hFeeder->ulLumaCrcRegAddr);
	pData->ulChromaCrc = BREG_Read32_isr(hReg, hFeeder->ulChromaCrcRegAddr);

	pData->ulChroma1Crc = BVDC_P_MFD_INVALID_CRC;
	pData->ulLumaCrcR   = BVDC_P_MFD_INVALID_CRC;
	pData->ulChromaCrcR = BVDC_P_MFD_INVALID_CRC;
	pData->ulChroma1CrcR = BVDC_P_MFD_INVALID_CRC;
#else

	pData->ulLumaCrc   = BREG_Read32_isr(hReg, BCHP_MFD_0_LUMA_CRC + hFeeder->ulRegOffset);
	pData->ulChromaCrc = BREG_Read32_isr(hReg, BCHP_MFD_0_CHROMA_CRC + hFeeder->ulRegOffset);

#if (BVDC_P_MFD_SUPPORT_CRC_R)
	pData->ulLumaCrcR   = BREG_Read32_isr(hReg, BCHP_MFD_0_LUMA_CRC_R + hFeeder->ulRegOffset);
	pData->ulChromaCrcR = BREG_Read32_isr(hReg, BCHP_MFD_0_CHROMA_CRC_R + hFeeder->ulRegOffset);
#else
	pData->ulLumaCrcR   = BVDC_P_MFD_INVALID_CRC;
	pData->ulChromaCrcR = BVDC_P_MFD_INVALID_CRC;
#endif

#if (BVDC_P_MFD_SUPPORT_CRC_TYPE)
	pData->ulChroma1Crc = BREG_Read32_isr(hReg, BCHP_MFD_0_CHROMA_1_CRC + hFeeder->ulRegOffset);
	pData->ulChroma1CrcR = BREG_Read32_isr(hReg, BCHP_MFD_0_CHROMA_1_CRC_R + hFeeder->ulRegOffset);

#else
	pData->ulChroma1Crc = BVDC_P_MFD_INVALID_CRC;
	pData->ulChroma1CrcR = BVDC_P_MFD_INVALID_CRC;
#endif

#endif
	pData->stMask.bCrcValue = BVDC_P_DIRTY;

	return;
}

#if (BVDC_P_MFD_SUPPORT_CSC)
/***************************************************************************
 *
 */
static void BVDC_P_Feeder_SetCsc_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData )
{
	BVDC_P_CscCoeffs stCscCoeffs;

	BDBG_OBJECT_ASSERT(hFeeder->hSource, BVDC_SRC);

#if (BVDC_P_SUPPORT_CMP_MOSAIC_CSC == 0)
	if(hFeeder->hSource->stCurInfo.bMosaicMode)
	{
		BVDC_P_CscCfg stCscCfg;
		/* Correct way is to convert individual input color space from
		 * pFieldData->eMatrixCoefficients to tracked channel color space
		 * defined by hFeeder->hSource->eMatrixCoefficients.
		 * BVDC_P_Compositor_GetCscTable_isr(&stCscCoeffs, false,
		 * pFieldData->eMatrixCoefficients, hFeeder->hSource->eMatrixCoefficients);
		 *
		 * Since we don't have all the matrices yet, just convert all to HD.
		 * hFeeder->hSource->eMatrixCoefficients is hard coded to
		 * BAVC_MatrixCoefficients_eItu_R_BT_709 in
		 * BVDC_P_Window_UpdateSrcAndUserInfo_isr. */
		BVDC_P_Compositor_GetCscTable_isrsafe(&stCscCfg, true,
			pFieldData->eMatrixCoefficients, BVDC_P_CmpColorSpace_eHdYCrCb, false);

		stCscCoeffs = stCscCfg.stCscMC;
	}
	else
#endif
	{
		/* Use s_Identity */
		BVDC_P_Csc_GetHdDviTable_isr(&stCscCoeffs, BAVC_CscMode_e709YCbCr);
	}

	/* Turn on CSC. */
	BVDC_P_MFD_GET_REG_DATA(MFD_0_CSC_CNTL) &= ~(
		BCHP_MASK(MFD_0_CSC_CNTL, CSC_ENABLE) );

	BVDC_P_MFD_GET_REG_DATA(MFD_0_CSC_CNTL) |= (
		BCHP_FIELD_ENUM(MFD_0_CSC_CNTL, CSC_ENABLE, ON) );

#if (BVDC_P_MFD_SUPPORT_NEW_CSC)
	/* [ c00, c01 c02 c03 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C00, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C00, COEFF_MUL, stCscCoeffs.usY0));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C01, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C01, COEFF_MUL, stCscCoeffs.usY1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C02, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C02, COEFF_MUL, stCscCoeffs.usY2));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C03, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C03, COEFF_ADD, stCscCoeffs.usYOffset));

	/* [ c10, c11 c12 c13 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C10, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C10, COEFF_MUL, stCscCoeffs.usCb0));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C11, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C11, COEFF_MUL, stCscCoeffs.usCb1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C12, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C12, COEFF_MUL, stCscCoeffs.usCb2));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C13, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C13, COEFF_ADD, stCscCoeffs.usCbOffset));

	/* [ c20, c21 c22 c23 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C20, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C20, COEFF_MUL, stCscCoeffs.usCr0));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C21, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C21, COEFF_MUL, stCscCoeffs.usCr1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C22, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C22, COEFF_MUL, stCscCoeffs.usCr2));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C23, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C23, COEFF_ADD, stCscCoeffs.usCrOffset));

#else
	/* [ c00, c01 c02 c03 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C01_C00, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C01_C00, COEFF_C0, stCscCoeffs.usY0) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C01_C00, COEFF_C1, stCscCoeffs.usY1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C03_C02, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C03_C02, COEFF_C2, stCscCoeffs.usY2) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C03_C02, COEFF_C3, stCscCoeffs.usYOffset));

	/* [ c10, c11 c12 c13 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C11_C10, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C11_C10, COEFF_C0, stCscCoeffs.usCb0) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C11_C10, COEFF_C1, stCscCoeffs.usCb1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C13_C12, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C13_C12, COEFF_C2, stCscCoeffs.usCb2) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C13_C12, COEFF_C3, stCscCoeffs.usCbOffset));

	/* [ c20, c21 c22 c23 ] */
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C21_C20, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C21_C20, COEFF_C0, stCscCoeffs.usCr0) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C21_C20, COEFF_C1, stCscCoeffs.usCr1));
	BVDC_P_MFD_SET_REG_DATA(MFD_0_CSC_COEFF_C23_C22, \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C23_C22, COEFF_C2, stCscCoeffs.usCr2) | \
		BCHP_FIELD_DATA(MFD_0_CSC_COEFF_C23_C22, COEFF_C3, stCscCoeffs.usCrOffset));
#endif

	return;
}

#endif


/*****************************************************************************
 * Functions specially for VFD suuport on Gfx surface
 */

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_Feeder_ValidateGfxSurAndRects_isr
 *
 * It validates the combination of clip, scaler-out and dest rectangles
 * and surface size.
 *
 * It could be called by BVDC_P_Feeder_ValidateChanges in user mode,
 * or during RUL build for isr surface.
 */
BERR_Code BVDC_P_Feeder_ValidateGfxSurAndRects_isr
	( BVDC_P_SurfaceInfo                *pSur,
	  const BVDC_P_ClipRect             *pClipRect,
	  const BVDC_P_Rect                 *pSclOutRect )
{
	uint32_t  ulCntWidth, ulOutWidth, ulCntHeight, ulOutHeight;

	/* Note that the combination of scale-out with dst rect and canvas is checked
	 * by BVDC_P_Window_ValidateChanges.
	 */

	/* check the combination of the surface size with the clip rectange */
	if ( ((pClipRect->ulLeft + pClipRect->ulRight)  >= pSur->ulWidth) ||
		 ((pClipRect->ulTop  + pClipRect->ulBottom) >= pSur->ulHeight) )
	{
		return BERR_TRACE(BVDC_ERR_ILLEGAL_CLIPPING_VALUES);
	}

	/* check the combination of clip rectange and scale-out rectangle */
	ulCntWidth  = (pSur->ulWidth  - (pClipRect->ulLeft + pClipRect->ulRight));
	ulCntHeight = (pSur->ulHeight - (pClipRect->ulTop  + pClipRect->ulBottom));
	ulOutWidth  = pSclOutRect->ulWidth;
	ulOutHeight = pSclOutRect->ulHeight;

	if ( (0 == ulCntWidth  && 0 != ulOutWidth) || /* inifinite scaling! */
		 (0 == ulCntHeight && 0 != ulOutHeight) )
	{
		return BERR_TRACE(BVDC_ERR_ILLEGAL_CLIPPING_VALUES);
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 * Validate the settings in pNewInfo set by user!
 *
 */
BERR_Code BVDC_P_Feeder_ValidateChanges
	( BVDC_P_Feeder_Handle               hFeeder,
	  BVDC_Source_PictureCallback_isr    pfPicCallbackFunc )
{
	BERR_Code eResult = BERR_SUCCESS;
	BVDC_P_SurfaceInfo  *pNewSur;
	const BVDC_P_ClipRect  *pNewClip;
	const BVDC_P_Rect  *pNewDst, *pNewSclOut;

	BDBG_ENTER(BVDC_P_Feeder_ValidateChanges);

	/* whether user set new surface, isr set surface, or current surface
	 * will be used to be validated against configure ? */
	if ( true == hFeeder->stGfxSurface.stNewSurInfo.bChangeSurface )
	{
		pNewSur = &(hFeeder->stGfxSurface.stNewSurInfo);
	}
	else if ( true == hFeeder->stGfxSurface.stIsrSurInfo.bChangeSurface )
	{
		pNewSur = &(hFeeder->stGfxSurface.stIsrSurInfo);
	}
	else /* no new user or isr set to surface, contunue to use cur surface */
	{
		pNewSur = &(hFeeder->stGfxSurface.stCurSurInfo);
	}
	hFeeder->pNewSur = pNewSur;

	/* don't validate if no surface is set yet */
	if (hFeeder->hWindow && pNewSur->ulAddress)
	{
		/* validates the combination of clip, scaler-out and dest rectangles
		 * and surface size. */
		BVDC_P_Window_GetNewRectangles( hFeeder->hWindow, &pNewClip, &pNewSclOut, &pNewDst );
		eResult = BVDC_P_Feeder_ValidateGfxSurAndRects(pNewSur, pNewClip, pNewSclOut);
		if (NULL != pfPicCallbackFunc)
		{
			/* allow pfPicCallbackFunc to provide gfx surface later */
			eResult = BERR_SUCCESS;
		}
	}

	BDBG_LEAVE(BVDC_P_Feeder_ValidateChanges);
	return BERR_TRACE(eResult);
}

/*--------------------------------------------------------------------------
 * {private}
 * BVDC_P_Feeder_ApplyChanges_isr
 *
 * To be called by BVDC_ApplyChanges, to copy "new user state" to "current
 * state", after validation of all VDC modules passed.
 */
BERR_Code BVDC_P_Feeder_ApplyChanges_isr
	( BVDC_P_Feeder_Handle               hFeeder )
{
	BVDC_P_SurfaceInfo  *pNewSur, *pCurSur;

	BDBG_ENTER(BVDC_P_Feeder_ApplyChanges_isr);

	/* validate paramters */
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);
	BDBG_OBJECT_ASSERT(hFeeder->hSource, BVDC_SRC);

	if (hFeeder->hWindow)
	{
		pCurSur = &(hFeeder->stGfxSurface.stCurSurInfo);
		pNewSur = hFeeder->pNewSur; /* decided by ValidateChanges */

		/* Copy NewSur to curSur if bChangeSurface.
		 * Note: pNewSur could be _isr set Surface,
		 * Note: pNewSur could be NULL before a window is connected to the src */
		if ( NULL != pNewSur && pNewSur->bChangeSurface && pNewSur != pCurSur)
		{
			/* pCurSur->bChangeSurface would stay true so that the surface change
			 * is built into RUL later */
			*pCurSur = *pNewSur;

			/* stGfxSurface.stCurSurInfo.bChangeSurface will be set to true iff
			 * there is format / size / pitch change */
			hFeeder->hSource->bPictureChanged = true;

			/* to avoid future validation if surface no longer changes */
			pNewSur->bChangeSurface  = false;

			/* any previous set IsrSur should no longer be used */
			hFeeder->stGfxSurface.stIsrSurInfo.bChangeSurface = false;
		}

		hFeeder->hSource->bStartFeed = true;
		hFeeder->hSource->bPsfScanout = false;
		hFeeder->hSource->stCurInfo.pFmtInfo =
			hFeeder->hWindow->hCompositor->hDisplay->stCurInfo.pFmtInfo;
	}

	BDBG_LEAVE(BVDC_P_Feeder_ApplyChanges_isr);
	return BERR_SUCCESS;
}

#if 0
/*************************************************************************
 * {private}
 * BVDC_P_Feeder_AbortChanges
 *
 * Cancel the user settings that either fail to validate or simply
 * because user wish to abort the changes in mid-way.
 *************************************************************************/
void BVDC_P_Feeder_AbortChanges
	( BVDC_P_Feeder_Handle               hFeeder )
{
	BDBG_ENTER(BVDC_P_Feeder_AbortChanges);

	/* validate paramters */
	BDBG_OBJECT_ASSERT(hFeeder, BVDC_FDR);

	/* any user set surface should not be used again */
	hFeeder->stGfxSurface.stNewSurInfo.bChangeSurface = false;

	BDBG_LEAVE(BVDC_P_Feeder_AbortChanges);
	return;
}
#endif

/*------------------------------------------------------------------------
 *  {secret}
 *	BVDC_P_Feeder_HandleIsrGfxSurface_isr
 *
 *  No paramter error check is performed. Designed to be called by
 *  BVDC_P_Feeder_BuildRul_isr only
 *
 *  It pulls a new isr surface if callback func is installed; then activates
 *  any new set surace (by Source_SetSurface_isr or this callback func). It
 *  at first validates the combination of this surface and current confugure,
 *  and set up to build RUL for this combination.
 */
void BVDC_P_Feeder_HandleIsrGfxSurface_isr
	( BVDC_P_Feeder_Handle         hFeeder,
	  BVDC_P_Source_Info *         pCurSrcInfo,
	  BAVC_Polarity                eFieldId )
{
	void  *pvGfxPic;
	const BVDC_P_ClipRect  *pCurClip;
	const BVDC_P_Rect  *pCurDst, *pCurSclOut;
	BVDC_P_SurfaceInfo  *pIsrSur = &(hFeeder->stGfxSurface.stIsrSurInfo);
	BERR_Code  eResult;

	if( NULL != pCurSrcInfo->pfPicCallbackFunc )
	{
		pCurSrcInfo->pfPicCallbackFunc(
			pCurSrcInfo->pvParm1, pCurSrcInfo->iParm2,
			eFieldId, BAVC_SourceState_eActive, &pvGfxPic);
		if ( NULL != pvGfxPic )
		{
			if (BVDC_P_GfxSurface_SetSurface_isr(&hFeeder->stGfxSurface,
				pIsrSur, (BAVC_Gfx_Picture *)pvGfxPic, hFeeder->hSource) != BERR_SUCCESS)
			{
				BDBG_ERR(("Previous surface is displayed."));
				return;
			}

			/* stGfxSurface.stCurSurInfo.bChangeSurface will be set to true iff
			 * there is format / size / pitch change */
			hFeeder->hSource->bPictureChanged =
				hFeeder->stGfxSurface.stCurSurInfo.bChangeSurface;
		}
	}

	/* after previous vsync, pIsrSur->bChangeSurface might be set by
	 * Source_SetSurface_isr, by the above callback function, or not re-set
	 * at all. If it was re-set, and there was no change in format, size, or
	 * pitch, then it has already been sent to HW in GfxSurface_SetSurface_isr,
	 * and pIsrSur->bChangeSurface has been marked as false */
	if (pIsrSur->bChangeSurface && hFeeder->hWindow)
	{
		BVDC_P_Window_GetCurrentRectangles_isr(
			hFeeder->hWindow, &pCurClip, &pCurSclOut, &pCurDst);

		/* validate surface size and rectangles, adjust rectangles and do some
		 * scale related computation. The intermediate values are stored in
		 * CurCfg, and will affect RUL build immediately. */
		eResult = BVDC_P_Feeder_ValidateGfxSurAndRects_isr(
			pIsrSur, pCurClip, pCurSclOut);
		if (BERR_SUCCESS == eResult)
		{
			/* Copy IsrSur to curSur
			 * note: pCurSur->bChangeSurface would stay true so that the surface
			 * change is built into RUL later */
			hFeeder->stGfxSurface.stCurSurInfo = *pIsrSur;

			/* to avoid future validation if isr surface no longer changes */
			pIsrSur->bChangeSurface  = false;
		}
		/* otherwise we stay with original current surface and ignore this
		 * IsrSur until either Cfg or surface changes
		 * note: ValidateSurAndRects will NOT change CurCfg until all error
		 * check are completed */
	}
}

/* End of file. */
