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

#include <stdio.h>
#include <string.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */

/* common utilities */
#include "brdc.h"
#include "bfmt.h"

/* porting interfaces */
#include "bvdc.h"
#include "bmma.h"
#include "bvdc_test.h"

#if SPLASH_SUPPORT_HDM
#include "bhdm_edid.h"
#endif

#include "splash_bmp.h"
#include "splash_file.h"
#include "bsplash_board.h"
#include "splash_vdc_rulgen.h"
#include "nexus_platform_features.h"

BDBG_MODULE(splash_setup_vdc);

#ifdef SPLASH_SUPPORT_HDM
/* prototypes */
static BERR_Code ActivateHdmi(BVDC_Handle hVDC, BHDM_Handle hHDM, BVDC_Display_Handle hDisplay);
static void   DeactivateHdmi(BVDC_Handle hVDC, BHDM_Handle hHDM, BVDC_Display_Handle hDisplay);
#ifndef SPLASH_HDMI_OUTPUTFORMAT_YCrCb
#define SPLASH_HDMI_OUTPUTFORMAT_YCrCb 1  /*YCrCB default*/
#endif

#else
#undef SPLASH_MASTERTG_DVI
#endif

#define IS_HD(fmt) (\
	(fmt == BFMT_VideoFmt_e480p)               || \
	(fmt == BFMT_VideoFmt_e576p_50Hz)          || \
	(fmt == BFMT_VideoFmt_e720p)               || \
	(fmt == BFMT_VideoFmt_e720p_24Hz)          || \
	(fmt == BFMT_VideoFmt_e720p_25Hz)          || \
	(fmt == BFMT_VideoFmt_e720p_30Hz)          || \
	(fmt == BFMT_VideoFmt_e720p_50Hz)          || \
	(fmt == BFMT_VideoFmt_e1080i)              || \
	(fmt == BFMT_VideoFmt_e1080i_50Hz)         || \
	(fmt == BFMT_VideoFmt_e1250i_50Hz)         || \
	(fmt == BFMT_VideoFmt_e1080p)              || \
	(fmt == BFMT_VideoFmt_e1080p_24Hz)         || \
	(fmt == BFMT_VideoFmt_e1080p_25Hz)         || \
	(fmt == BFMT_VideoFmt_e1080p_30Hz)         || \
	(fmt == BFMT_VideoFmt_e1080p_50Hz)         || \
	(fmt == BFMT_VideoFmt_e720p_60Hz_3DOU_AS)  || \
	(fmt == BFMT_VideoFmt_e720p_50Hz_3DOU_AS)  || \
	(fmt == BFMT_VideoFmt_e1080p_24Hz_3DOU_AS) || \
	(fmt == BFMT_VideoFmt_e1080p_30Hz_3DOU_AS) || \
	(fmt == BFMT_VideoFmt_eCUSTOM_1366x768p)   || \
	(fmt == BFMT_VideoFmt_eDVI_1600x1200p_60Hz)|| \
	(fmt == BFMT_VideoFmt_eCustom2)               \
)

/***************************************************************************
 * This function sets up the display with graphics and does ONE
 * ApplyChanges call. After this point, all register writes and RUL
 * updates should be intercepted and dumped.
 */
BERR_Code  splash_vdc_setup(
	BCHP_Handle         hChp,
	BREG_Handle         hReg,
	BINT_Handle         hInt,
	BTMR_Handle         hTmr,
	BBOX_Handle         hBox,
	ModeHandles        *pState
	)
{
	BERR_Code               eErr = BERR_SUCCESS;
	BVDC_Handle             hVdc;
#ifdef SPLASH_SUPPORT_HDM
	BHDM_Handle             hHdm;
	BHDM_Settings           HDMSettings;
#endif
#ifdef SPLASH_SUPPORT_RFM
	BRFM_Handle             hRfm;
	BRFM_Settings           rfmDevSettings;
#endif

	BVDC_Compositor_Handle  hCompositor;
	BVDC_Display_Handle     hDisplay;
	BFMT_VideoInfo          stVideoInfo;
	BPXL_Plane              surface;
	BVDC_Source_Handle      hGfxSource;
	BVDC_Window_Handle      hGfxWindow;
	BAVC_Gfx_Picture        pic;
	uint32_t         surfWidth;
	uint32_t         surfHeight;
	BPXL_Format      surfPxlFmt;
	void*            splashAddress;
	uint32_t         splashPitch;
	char  *bmpFileName = NULL;
	uint8_t  *bmpBuf = NULL;
	BMP_HEADER_INFO  myBmpInfo;
	int x,y;
	uint32_t  svideoDspIdx;
	uint32_t  composite0DspIdx;
	uint32_t  composite1DspIdx;
	uint32_t  componentDspIdx;
	uint32_t  hdmDspIdx;
	uint32_t  winHeight;
	BVDC_Settings  stDefSettings;
	int  ii;

	/* setup surfaces */
	for (ii=0; ii<SPLASH_NUM_SURFACE; ii++)
	{
		if ( pState->surf[ii].hMma != NULL)
		{
			pState->iNumSurf++;
			if ((bmpFileName == NULL) || strcmp(bmpFileName, &pState->surf[ii].bmpFile[0]))
			{
				if(bmpBuf)
					BKNI_Free(bmpBuf);

				bmpFileName = &pState->surf[ii].bmpFile[0];
				bmpBuf = splash_open_bmp(bmpFileName);
				if(bmpBuf)
					splash_bmp_getinfo(bmpBuf, &myBmpInfo);
				else
					BDBG_ERR(("Missing file %s. could use BSEAV/app/splash/splashgen/splash.bmp", bmpFileName));
			}

			/* scale up fullscreen AND gfd has scaler.  For vertical only scale
			 * if HW is available */
			if(pState->bScaleToFullScreen)
			{
				surfWidth  = myBmpInfo.info.width;
				if(pState->disp[ii].bGfdHasVertScale)
				{
					surfHeight = myBmpInfo.info.height;
				}
				else
				{
					surfHeight = pState->surf[ii].ulHeight;
				}
			}
			else
			{
				surfWidth  = pState->surf[ii].ulWidth;
				surfHeight = pState->surf[ii].ulHeight;
			}

			surfPxlFmt = pState->surf[ii].ePxlFmt;
			if (0==surfWidth)
				continue;

			BPXL_Plane_Init(&surface, surfWidth, surfHeight, surfPxlFmt);
			eErr = BPXL_Plane_AllocateBuffers(&surface, pState->surf[ii].hMma);
			if (eErr != BERR_SUCCESS)
			{
				BDBG_ERR(("Out of memory"));
				goto done;
			}
			pState->surf[ii].surface = surface;

			splashAddress = BMMA_Lock(surface.hPixels);
			splashPitch = surface.ulPitch;

			splash_set_surf_params(surfPxlFmt, splashPitch, surfWidth, surfHeight) ;

			/* splash_fillbuffer(splashAddress, 0xF8, 0xE0, 0) ; */
			splash_fillbuffer(splashAddress, 0x00, 0x00, 0x00);

			if(bmpBuf)
			{
				BDBG_MSG(("splash.bmp: Width = %d Height = %d", myBmpInfo.info.width, myBmpInfo.info.height));
				BDBG_MSG(("rendered into surface %d",ii));

				if(pState->bScaleToFullScreen)
				{
					x = 0;
					if(pState->disp[ii].bGfdHasVertScale)
					{
						y = 0;
					}
					else
					{
						y = ((int)surfHeight- (int)myBmpInfo.info.height)/2;
					}
				}
				else
				{
					x = ((int)surfWidth- (int)myBmpInfo.info.width)/2;
					y = ((int)surfHeight- (int)myBmpInfo.info.height)/2;
				}
				splash_render_bmp_into_surface(x, y, bmpBuf, splashAddress) ;
			}

			/* flush cached addr */
			BMMA_FlushCache(surface.hPixels, splashAddress, splashPitch * surfHeight);
			BMMA_Unlock(surface.hPixels, splashAddress);
		}
	}

	if(bmpBuf)
	{
		BKNI_Free(bmpBuf);
	}

	/* open VDC
	 * note: VDC default display format is 1080i, we must override it here
	 * if we are compiling with B_PI_FOR_BOOTUPDATER and 1080i is not supported in bootloader
	 */
	eErr = BVDC_GetDefaultSettings(hBox, &stDefSettings);
	if (eErr != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed BVDC_GetDefaultSettings"));
		goto done;
	}

	stDefSettings.eVideoFormat = BFMT_VideoFmt_e480p;
	eErr = BVDC_Open(&pState->hVdc, hChp, hReg, pState->hRulMem, hInt,
		pState->hRdc, hTmr, &stDefSettings);
	hVdc = pState->hVdc;

#ifdef SPLASH_SUPPORT_HDM
	eErr = BHDM_GetDefaultSettings(&HDMSettings);
	HDMSettings.hTMR = hTmr;
	eErr = BHDM_Open(&hHdm, hChp, hReg, hInt, pState->hRegI2c, &HDMSettings);
#endif

#ifdef SPLASH_SUPPORT_RFM
	eErr = BRFM_GetDefaultSettings( &rfmDevSettings, hChp );
	rfmDevSettings.audioEncoding = BRFM_AudioEncoding_eStereo;
	eErr = BRFM_Open( &hRfm, hChp, hReg, hInt, &rfmDevSettings );
	eErr = BRFM_SetModulationType( hRfm, BRFM_ModulationType_eNtscOpenCable,
	    BRFM_OutputChannel_eCh3 );
#endif

	/* which disp should drive which output interface? */
	svideoDspIdx = 0;
	composite0DspIdx = 0;
	composite1DspIdx = 0;
	componentDspIdx = 0;
	hdmDspIdx = 0;
	for(ii=0; ii<SPLASH_NUM_DISPLAY; ii++)
	{
		if (IS_HD(pState->disp[ii].eDispFmt))
		{
			componentDspIdx = ii;
			hdmDspIdx = ii;
		}
		else
		{
			svideoDspIdx = ii;
			composite0DspIdx = ii;
			composite1DspIdx = ii;
		}
	}

	/* setup display */
	for(ii=0; ii<SPLASH_NUM_DISPLAY; ii++)
	{
		int32_t iTop, iLeft;
		uint32_t ulWidth, ulHeight;

		if (pState->disp[ii].pSurf->hMma != NULL)
		{
			pState->iNumDisp++;
			BDBG_MSG(("***********display[%d]*************", ii));

			/* Create a compositor handle from our hVdc handle */
			TestError( BVDC_Compositor_Create(
				hVdc, &hCompositor, BVDC_CompositorId_eCompositor0 + ii, NULL),
				"ERROR: BVDC_Compositor_Create" );
			pState->disp[ii].hCompositor = hCompositor;

			/* Create display handle */
#ifdef SPLASH_MASTERTG_DVI
			if (hdmDspIdx == (uint32_t)ii)
			{
				BVDC_Display_Settings  cfg_display;

				BVDC_Display_GetDefaultSettings(BVDC_DisplayId_eDisplay0, &cfg_display);
				cfg_display.eMasterTg = BVDC_DisplayTg_eDviDtg;
				eErr = BVDC_Display_Create(hCompositor,&hDisplay,
					BVDC_DisplayId_eDisplay0, &cfg_display);
			}
			else
			{
				eErr = BVDC_Display_Create(hCompositor, &hDisplay, BVDC_DisplayId_eAuto, NULL);
			}
#else
			eErr = BVDC_Display_Create(hCompositor, &hDisplay, BVDC_DisplayId_eAuto, NULL);
#endif
			if (eErr != BERR_SUCCESS)
			{
				BDBG_ERR(("BVDC_Display_Create failed"));
				goto done;
			}
			pState->disp[ii].hDisplay = hDisplay;

			/* Set display format */
			TestError( BVDC_Display_SetVideoFormat(hDisplay, pState->disp[ii].eDispFmt),
				"ERROR: BVDC_Display_SetVideoFormat" );

			/* Set the background color to blue */
			TestError( BVDC_Compositor_SetBackgroundColor( hCompositor, 0x00, 0x00, 0x80 ),
				"ERROR: BVDC_Compositor_SetBackgroundColor" );

			/* set DAC configurations for specific display format
			 * Dac setup is specified in bsplash_board.h */
#if (SPLASH_NUM_COMPONENT_OUTPUTS != 0) && !defined(SPLASH_MASTERTG_DVI)
			if (componentDspIdx == (uint32_t)ii)
			{
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_PR, BVDC_DacOutput_ePr),
					"ERROR: BVDC_Display_SetDacConfiguration" );
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_Y, BVDC_DacOutput_eY),
					"ERROR: BVDC_Display_SetDacConfiguration" );
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_PB, BVDC_DacOutput_ePb),
					"ERROR: BVDC_Display_SetDacConfiguration" );
				BDBG_MSG(("Set dac for component with display %d", ii));
			}
#endif
#if (SPLASH_NUM_SVIDEO_OUTPUTS != 0)
			if ((svideoDspIdx == (uint32_t)ii) && !IS_HD(pState->disp[ii].eDispFmt))
			{
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_SVIDEO_CHROMA, BVDC_DacOutput_eSVideo_Chroma),
					"ERROR: BVDC_Display_SetDacConfiguration" );
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_SVIDEO_LUMA, BVDC_DacOutput_eSVideo_Luma),
						   "ERROR: BVDC_Display_SetDacConfiguration" );
				BDBG_MSG(("Set dac for svideo with display %d", ii));
			}
#endif
			if ((composite0DspIdx == (uint32_t)ii) && !IS_HD(pState->disp[ii].eDispFmt))
			{
#if (SPLASH_NUM_COMPOSITE_OUTPUTS != 0)
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_COMPOSITE_0, BVDC_DacOutput_eComposite),
					"ERROR: BVDC_Display_SetDacConfiguration" );
				BDBG_MSG(("Set dac for composite 0 with display %d", ii));
#endif
#ifdef SPLASH_SUPPORT_RFM
				pState->disp[ii].hRfm = hRfm;
				TestError( BVDC_Display_SetRfmConfiguration( hDisplay,
					BVDC_Rfm_0, BVDC_RfmOutput_eCVBS, 0),
					"ERROR: BVDC_Display_SetRfmConfiguration" );
				BDBG_MSG(("Set rfm output for composite 0 with display %d", ii));
#endif
			}
#if (SPLASH_NUM_COMPOSITE_OUTPUTS > 1)
			if ((composite1DspIdx == (uint32_t)ii) && !IS_HD(pState->disp[ii].eDispFmt))
			{
				TestError( BVDC_Display_SetDacConfiguration( hDisplay,
					BRCM_DAC_COMPOSITE_1, BVDC_DacOutput_eComposite),
						   "ERROR: BVDC_Display_SetDacConfiguration" );
				BDBG_MSG(("Set dac for composite 1 with display %d", ii));
			}
#endif

			/* to determine size of display */
			TestError( BFMT_GetVideoFormatInfo(pState->disp[ii].eDispFmt, &stVideoInfo),
				"ERROR:BFMT_GetVideoFormatInfo" );

#ifdef SPLASH_SUPPORT_HDM
			if (hdmDspIdx == (uint32_t)ii)
			{
				pState->disp[hdmDspIdx].hHdm = hHdm;
				eErr = ActivateHdmi(hVdc, hHdm, pState->disp[hdmDspIdx].hDisplay);
				if (eErr != BERR_SUCCESS)
				BDBG_ERR(("Error ActivateHDMI, HDMI is not connected, or TV is off?"));
				TestError( BVDC_ApplyChanges(hVdc), "ERROR:BVDC_ApplyChanges" );
			}
#endif

			/* create a graphics source handle */
			TestError( BVDC_Source_Create( hVdc, &hGfxSource, BAVC_SourceId_eGfx0 + ii, NULL),
				"ERROR: BVDC_Source_Create" );
			pState->disp[ii].hGfxSource = hGfxSource;

			/* obtain RDC scratch registers */
			BVDC_Test_Source_GetGfdScratchRegisters(pState->disp[ii].hGfxSource,
				&pState->disp[ii].ulGfdScratchReg0, &pState->disp[ii].ulGfdScratchReg1);

			/* specify the source surface */
			BKNI_Memset(&pic, 0, sizeof(pic));
			pic.pSurface = &(pState->disp[ii].pSurf->surface);
			splashAddress = BMMA_Lock(pic.pSurface->hPixels);

			TestError( BVDC_Source_SetSurface( hGfxSource, &pic ),
				"ERROR: BVDC_Source_SetSurface" );

			BDBG_MSG(("uses surface %d", pState->disp[ii].iSurfIdx));
			BMMA_Unlock(pic.pSurface->hPixels, splashAddress);

#ifdef SPLASH_SUPPORT_HDM
			if (hdmDspIdx == (uint32_t)ii)
			{
				pState->disp[hdmDspIdx].hHdm = hHdm;
				eErr = ActivateHdmi(hVdc, hHdm, pState->disp[hdmDspIdx].hDisplay);
				if (eErr != BERR_SUCCESS)
				BDBG_ERR(("Error ActivateHDMI, HDMI is not connected, or TV is off?"));
				TestError( BVDC_ApplyChanges(hVdc), "ERROR:BVDC_ApplyChanges" );
			}
#endif

			/* create a window handle */
			TestError( BVDC_Window_Create( hCompositor,
				&hGfxWindow, BVDC_WindowId_eAuto, hGfxSource, NULL ),
				"ERROR:BVDC_Window_Create" );
			pState->disp[ii].hGfxWindow = hGfxWindow;

			/* set destination height not bigger than src height */
			winHeight = (stVideoInfo.ulHeight <= pState->disp[ii].pSurf->ulHeight)?
				stVideoInfo.ulHeight : pState->disp[ii].pSurf->ulHeight;

			if(pState->bScaleToFullScreen)
			{
				iLeft = 0;
				ulWidth  = stVideoInfo.ulWidth;

				/* set destination size to match display */
				if(pState->disp[ii].bGfdHasVertScale)
				{
					iTop     = 0;
					ulHeight = stVideoInfo.ulHeight;
				}
				else
				{
					iTop  = (stVideoInfo.ulHeight - winHeight)/2;
					ulHeight = winHeight;
				}
			}
			else
			{
				iLeft    = 0;
				iTop     = (stVideoInfo.ulHeight - winHeight)/2;
				ulWidth  = stVideoInfo.ulWidth;
				ulHeight = winHeight;
			}

			BDBG_MSG(("output rect(%4d, %4d, %4d, %4d)", iTop, iTop, ulWidth, ulHeight));

			TestError( BVDC_Window_SetDstRect(hGfxWindow, iTop, iTop, ulWidth, ulHeight),
				"ERROR:BVDC_Window_SetDstRect");
			TestError( BVDC_Window_SetScalerOutput(hGfxWindow, iTop, iTop, ulWidth, ulHeight),
				"ERROR:BVDC_Window_SetScalerOutput");

			/* set order to front */
			TestError( BVDC_Window_SetZOrder( hGfxWindow, 1),
				"ERROR:BVDC_Window_SetZOrder" );

			/* enable visibility */
			TestError( BVDC_Window_SetVisibility( hGfxWindow, true),
				"ERROR:BVDC_Window_SetVisibility" );
		}
	}

	/***************************
	 * Apply Changes
	 */
	TestError( BVDC_ApplyChanges(hVdc), "ERROR:BVDC_ApplyChanges" );

done:
	/* return status */
	BDBG_ASSERT(!eErr) ;
	return eErr;
}

#ifndef BVDC_FOR_BOOTUPDATER
BERR_Code  close_mode(
	ModeHandles            *pState
	)
{
	BERR_Code  eErr = BERR_SUCCESS;
	int  ii;

	for(ii=0; ii<SPLASH_NUM_DISPLAY; ii++)
	{
		if (pState->disp[ii].pSurf->hMma != NULL)
		{

#ifdef SPLASH_SUPPORT_HDM
			if (pState->disp[ii].hHdm)
			{
				DeactivateHdmi(pState->hVdc, pState->disp[ii].hHdm, pState->disp[ii].hDisplay);
			}
#endif
			TestError( BVDC_Window_Destroy(pState->disp[ii].hGfxWindow),
				"BVDC_Window_Destroy");
			TestError( BVDC_ApplyChanges(pState->hVdc),
				"BVDC_ApplyChanges");
			TestError( BVDC_Source_Destroy(pState->disp[ii].hGfxSource),
				"BVDC_Source_Destroy");
			TestError( BVDC_Display_Destroy(pState->disp[ii].hDisplay),
				"BVDC_Display_Destroy");
			TestError( BVDC_Compositor_Destroy(pState->disp[ii].hCompositor),
				"BVDC_Compositor_Destroy");

#ifdef SPLASH_SUPPORT_HDM
			if (pState->disp[ii].hHdm)
			{
				BHDM_Close(pState->disp[ii].hHdm);
			}
#endif
#ifdef SPLASH_SUPPORT_RFM
			if (pState->disp[ii].hRfm)
			{
				BRFM_Close(pState->disp[ii].hRfm);
				pState->disp[ii].hRfm = 0;
			}
#endif

			TestError( BVDC_ApplyChanges(pState->hVdc),
				"BVDC_ApplyChanges");
		}
	}

	for (ii=0; ii<SPLASH_NUM_SURFACE; ii++)
	{
		if (pState->surf[ii].hMma && pState->surf[ii].surface.hPixels)
		{
			BMMA_Free(pState->surf[ii].surface.hPixels);
			pState->surf[ii].surface.hPixels = NULL;
		}
	}

	TestError( BVDC_Close(pState->hVdc),
		"BVDC_Close");

done:
	return eErr;
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

#ifdef SPLASH_SUPPORT_HDM
static BERR_Code ActivateHdmi(BVDC_Handle hVDC, BHDM_Handle hHDM, BVDC_Display_Handle hDisplay)
{
	BERR_Code  eErr = BERR_SUCCESS;
	BFMT_VideoFmt           vidfmt;
	BHDM_Settings           hdmiSettings;
	BHDM_EDID_ColorimetryParams edidParameters ;
	const BFMT_VideoInfo*   vidinfo;
	bool                    hasHdmiSupport;
	BVDC_Display_HdmiSettings stVdcHdmiSettings;

	BHDM_EDID_RxVendorSpecificDB    vsdb;

	/* Get Current Display format */
	TestError(BVDC_Display_GetVideoFormat(hDisplay, &vidfmt),
		"BVDC_Display_GetVideoFormat") ;

	/* Get video info */
	vidinfo = BFMT_GetVideoFormatInfoPtr(vidfmt);

	/* Get Current Settings */
	TestError(BHDM_GetHdmiSettings(hHDM, &hdmiSettings),
		"BHDM_GetHdmiSettings");

	/* Set the video format */
	hdmiSettings.eInputVideoFmt = vidfmt;

	if (hdmiSettings.eOutputPort == BHDM_OutputPort_eHDMI)
	{
		BHDM_EDID_IsRxDeviceHdmi(hHDM, &vsdb, &hasHdmiSupport);
		if (SPLASH_HDMI_OUTPUTFORMAT_YCrCb /*hasHdmiSupport*/)
		{
			hdmiSettings.eOutputFormat = BHDM_OutputFormat_eHDMIMode;
			hdmiSettings.eAspectRatio = vidinfo->eAspectRatio;

			/* Audio settings (for later):
			set hdmi audio()
			BAUD_GetClockSamplingRate(GetBAUD(), ??, &sampleRate);
			hdmiSettings.eAudioSamplingRate = sampleRate;
			hdmiSettings.AudioBits = BAVC_AudioBits_e16; */
		}
		else
		{
			/* Configure for DVI mode */
			hdmiSettings.eOutputFormat = BHDM_OutputFormat_eDVIMode;
		}
	}

	edidParameters.eVideoFmt = vidfmt ;
	edidParameters.xvYccEnabled = false ;
	TestError(BHDM_EDID_GetPreferredColorimetry(hHDM, &edidParameters, &hdmiSettings.eColorimetry),
		"BHDM_EDID_GetPreferredColorimetry");

    hdmiSettings.eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709;
    hdmiSettings.stVideoSettings.eColorSpace = BAVC_Colorspace_eYCbCr422;

	TestError(BVDC_Display_GetHdmiSettings(hDisplay, &stVdcHdmiSettings),
		"BVDC_Display_GetHdmiSettings");
	stVdcHdmiSettings.ulPortId      = BVDC_Hdmi_0;
	stVdcHdmiSettings.eMatrixCoeffs = hdmiSettings.eColorimetry;
    stVdcHdmiSettings.eColorComponent = BAVC_Colorspace_eYCbCr422;
    stVdcHdmiSettings.eColorRange = BAVC_ColorRange_eLimited;
    stVdcHdmiSettings.eEotf = BAVC_HDMI_DRM_EOTF_eSDR;
	TestError(BVDC_Display_SetHdmiSettings(hDisplay, &stVdcHdmiSettings),
		"BVDC_Display_SetHdmiSettings");

	TestError(BVDC_ApplyChanges(hVDC),
		"BVDC_ApplyChanges");

#ifdef SPLASH_MASTERTG_DVI
	BHDM_SetHdmiDataTransferMode(hHDM, true);
#endif

	hdmiSettings.bForceEnableDisplay = true;
	TestError(BHDM_EnableDisplay(hHDM, &hdmiSettings),
		"BHDM_EnableDisplay");

	#ifndef BVDC_FOR_BOOTUPDATER
	TestError(BVDC_Display_InstallCallback(hDisplay,
		(BVDC_CallbackFunc_isr)BHDM_AudioVideoRateChangeCB_isr, hHDM,
		BHDM_Callback_Type_eVideoChange),
		"BVDC_Display_InstallCallback");
	#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

done:
	return eErr;
}

/* Activate HDMI to match the current video format */
#ifndef BVDC_FOR_BOOTUPDATER
static void DeactivateHdmi(BVDC_Handle hVDC, BHDM_Handle hHDM, BVDC_Display_Handle hDisplay)
{
	/* uninstall audio callback (later) */
	BVDC_Display_InstallCallback(hDisplay, NULL, NULL, 0) ;
	(void) BVDC_ApplyChanges(hVDC);
	(void) BHDM_DisableDisplay(hHDM);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
#endif

/* End of File */
