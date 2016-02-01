/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
*
* Module Description:
*	This file contains the implementations of the APIs, which are exposed 
*	to the upper layer by the Raptor Audio PI. This file is part of Audio 
*   Manager Module. Audio Manager module is the top level module of the 
*   RAP PI, which interfaces with the caller and manages the internal 
*   channel objects to get RAP PI produce the desired result.
*
* Revision History:
*
* $brcm_Log: $
* 
* PR18604: Quick Fix f* $brcm_Revision: $
* $brcm_Date: $
or Encoder.
* 
* Hydra_Software_Devel/RAP_AUD_ENC/1   12/14/05 5:30p sushmit
* [PR18604]:Susmit
* Merging Encoder to latest.
* SUSMIT
* 14 DEC 2005
* 
* Hydra_Software_Devel/58   12/13/05 1:51p kagrawal
* PR 18578: Added support for dither and burst mode programming for
* SPDIFFM
* 
* Hydra_Software_Devel/57   12/9/05 4:39p rjain
* PR 18264: Capture PI Development
* 
* Hydra_Software_Devel/56   12/5/05 11:13a kagrawal
* PR 18312: Removed compile time warnings
* 
* Hydra_Software_Devel/55   12/5/05 10:34a nitinb
* PR 18061: Making TSM log support compile time selectable
* 
* Hydra_Software_Devel/54   12/4/05 8:12p kagrawal
* PR 18312: Added support to write PCM data in interrupt driven mode
* 
* Hydra_Software_Devel/53   12/2/05 8:55p kagrawal
* PR 18312: PCM Playback and Mixing support for 7411
* 
* Hydra_Software_Devel/52   12/2/05 3:21p nitinb
* PR 18061: Raptor memory allocation must be less than 0.5M for MPEG/AC-3
* only
* 
* Hydra_Software_Devel/51   12/1/05 8:19p kagrawal
* PR 18278: Watchdog support for PCM playback and mixing
* 
* Hydra_Software_Devel/50   11/30/05 4:10p kagrawal
* PR 18279: Rearchitected output sampling rate change for a PCM channel
* 
* Hydra_Software_Devel/49   11/22/05 1:54p bselva
* PR 17490: Fix for 44.1KHz I2S output
* 
* Hydra_Software_Devel/48   11/16/05 9:27p rjain
* PR 18129 : removing debug msgs.
* 
* Hydra_Software_Devel/47   11/16/05 9:11p rjain
* PR 18129: Making the changes reqd for PI-Fw interface to add an output
* port on the fly for a DEC channel.
* 
* Hydra_Software_Devel/46   11/16/05 1:30p rjain
* PR 18129: Added code for 1.Watchdog recovery. 2. Adding a cloned port
* before a channel start
* 
* Hydra_Software_Devel/45   11/15/05 7:54p bselva
* PR 17590: Added support for MAI linked to Flex
* 
* Hydra_Software_Devel/44   11/15/05 6:08p kagrawal
* PR 17590: Corrected watchdog breaks due to PCM playback changes
* 
* Hydra_Software_Devel/43   11/15/05 4:14p rjain
* PR 18129: Cloned port now get stopped/restarted on a normal channel
* stop/start
* 
* Hydra_Software_Devel/42   11/14/05 9:53a rjain
* PR 18129: Adding support for routing to multiple output ports
* 
* Hydra_Software_Devel/41   11/9/05 4:00p bselva
* PR 17590: Added support for MAI output
* 
* Hydra_Software_Devel/40   11/3/05 3:23p rjain
* PR 16982: Merging Vladimir's image interface related changes to main
* branch
* 
* Hydra_Software_Devel/39   11/2/05 3:22p bselva
* PR 17590: Added watchdog support for 7401
* 
* Hydra_Software_Devel/38   10/18/05 4:53p rjain
* PR 17651: Changing checks for BCHP_CHIP ==/!= 7401 to check for
* BCHP_7411_VER instead
* 
* Hydra_Software_Devel/37   10/17/05 2:16p rjain
* PR 17651: Making changes for 7411 D0
* 
* Hydra_Software_Devel/36   10/2/05 11:57p nitinb
* PR 17415: Fixing compilation warnings
* 
* Hydra_Software_Devel/35   9/30/05 5:01p nitinb
* PR 16982: Audio Raptor PI shall use BIMG interface to access firmware
* interface
* 
* Hydra_Software_Devel/34   9/27/05 8:07p nitinb
* PR 16706: Raptor should not fire StreamInfo interrupt when only chMod
* field changes
* 
* Hydra_Software_Devel/33   9/26/05 5:55p bselva
* PR 16148: Assigning variable sCdbItbCfg with proper endianness for CDB
* configurations
* 
* Hydra_Software_Devel/32   9/22/05 9:59p nitinb
* PR 16899: Implemented functions to install application callback
* functions for Raptor device level interrupts
* 
* Hydra_Software_Devel/31   9/21/05 10:16p nitinb
* PR 16899: Making changes in the code for watchdog feature
* 
* Hydra_Software_Devel/30   9/21/05 5:05p bselva
* PR 16148: Fixing the warnings caused while compiling for 7401
* 
* Hydra_Software_Devel/29   9/20/05 7:12p nitinb
* PR 16899: Fixing compilation errors in ProcessWatchdog function for
* 7401
* 
* Hydra_Software_Devel/28   9/20/05 4:01p nitinb
* PR 16899: Making changes in processwatchdog function for 7401
* 
* Hydra_Software_Devel/27   9/20/05 1:11p nitinb
* PR 16899: Added API BRAP_ProcessWatchdogInterrupt
* 
* Hydra_Software_Devel/26   9/13/05 7:41p bmishra
* PR 16148: Merged 7401 changes
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/4   8/19/05 12:02p bmishra
* PR 16148: Mergeded the latest changes in the main branch.
* 
* Hydra_Software_Devel/25   8/3/05 9:15p bselva
* PR 15766: Support for BRAP_GetRevision API added
*
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/3   8/1/05 5:07p bmishra
* PR 16148: Added API to query CDB/ITB sizes required
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/2   7/16/05 9:14p bmishra
* PR 16148: Enabled DAC for 7401
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/1   7/12/05 8:13p bmishra
* PR 16148: Modified the file for 7401 compatibility
* 
* Hydra_Software_Devel/23   7/11/05 3:35p nitinb
* PR 16148: Making changes for 7401 to access device memory. Undoing the
* previous change.
* 
* Hydra_Software_Devel/22   7/8/05 8:25p nitinb
* PR 16148: Making changes for 7401 to access device memory
* 
* Hydra_Software_Devel/21   6/24/05 3:27p bmishra
* PR 14143: Implemented code review comments
* 
* Hydra_Software_Devel/20   6/17/05 11:16a bselva
* PR 15837: BRAP_P_ClearInterrupts is called after BRAP_FMM_P_Open in
* BRAP_Open to avoid bus error when closing and opening RAP device
* 
* Hydra_Software_Devel/19   6/2/05 10:44a rjain
* PR 14143: Implemented FMM Code review comments.
* 
* Hydra_Software_Devel/18   5/30/05 6:48p rjain
* PR 15272: Updating interrupt infrastructure
* 
* Hydra_Software_Devel/17   5/25/05 3:58p rjain
* PR 15272: Adding some code for interrupt handling framework.
* 
* Hydra_Software_Devel/16   4/22/05 3:25p rjain
* PR 14507: Putting in checks to limit permissible output port types for
* 7411 to I2S0 and SPDIF.
* 
* Hydra_Software_Devel/15   3/3/05 4:36p nitinb
* PR 13082: Don't assert on memory handle in BRAP_Open for 7411C0
* 
* Hydra_Software_Devel/13   2/26/05 4:51p nitinb
* PR 14143: Splitted brap.c file. Moved decode related functions to
* brap_dec.c
* 
* Hydra_Software_Devel/12   2/17/05 7:34p bmishra
* PR 14143: Changes related to passing BARC Handle in place of BREG
* Handle to BRAP_open
* 
* Hydra_Software_Devel/11   2/16/05 8:30p bmishra
* PR 14143: Implemented some Phase 1 code review comments for the
* interfaces only
* 
* Hydra_Software_Devel/10   2/16/05 1:44p rjain
* PR 14143: Implementing some comments from Phase 1 Code review done till
* date
* 
* Hydra_Software_Devel/9   2/15/05 8:09p bmishra
* PR 14143: Implemented Phase 1 code review comments for the interfaces
* only
* 
* Hydra_Software_Devel/8   2/8/05 6:58p rjain
* PR13082: Some bug fixes and clean up
* 
* Hydra_Software_Devel/7   2/3/05 2:45p rjain
* PR13082: Fixing compilation warnings
* 
* Hydra_Software_Devel/6   2/2/05 7:13p nitinb
* PR 13082: Fixing compilation warnings
* 
* Hydra_Software_Devel/5   2/2/05 5:33p rjain
* PR13082:Converted BREG_Read32 and BRAP_Read32 and BREG_Write32 to
* BRAP_Write32
* 
* Hydra_Software_Devel/4   2/2/05 4:48p rjain
* PR13082: Modifying according to settings/bugs highlighted in emulation
* 
* Hydra_Software_Devel/3   1/25/05 9:52a rjain
* PR13082: Modifying according to settings/bugs highlighted in emulation
* so far.
* 
* Hydra_Software_Devel/2   1/12/05 5:01p rjain
* PR13082: Updating with changes after compilation of FMM, AM and RM
* together.
* 
* Hydra_Software_Devel/1   1/10/05 8:27p nitinb
* PR 13082: Moved to new directory structure
* 
***************************************************************************/
#include "brap.h"		/* API function and data structures declarations */

/* Internal module includes */
#include "brap_priv.h"	

BDBG_MODULE(rap);		/* Register software module with debug interface */

/*{{{ Local Defines */
#define BRAP_P_DEFAULT_RAMP_SIZE 3600 /* in 1/100 dB. We want the volume to go 
from 0x8000 to 0 in 64fs. This translates to 36 db and 0x200 gets written to 
the register */
#define BRAP_P_DEF_DEV_MEM_SIZE		(600 * 1024) /* Default device memory size = 600 KB */

#if (BRAP_DSP_P_7401_NEWMIT==1)
#define BRAP_P_DEF_PP_BRANCHES_PER_CXT	3	
#define BRAP_P_DEF_STG_PER_PP_BRANCH	7
#endif



/*}}}*/

/*{{{ Local Typedefs */

/*}}}*/


/*{{{ Static Variables & Function prototypes */

static const BRAP_Settings sDefSetting = 
{
#if (BRAP_SECURE_HEAP==1)
	NULL,						/*hSecureMemory. No secure memory by default */
#endif
#ifndef BCHP_7411_VER
	false,							/*bFwAuthEnable. No FW auth enabled by defaul*/
#endif
	true,							/* bExtDeviceMem. External device memory allocation */ 	
	0x0,							/* ui32MemBase. Device mem addr base */
	BRAP_P_DEF_DEV_MEM_SIZE,	/* ui32MemSize. 600 KB device memory */	
	BRAP_RBUF_P_DEFAULT_SIZE,	/* ui32MaxRbufSize. Default Worst Case Ring Buffer size */
	NULL, 						/* pImgInterface. The Image Interface function pointers */
	NULL,						/* pImgContext. The Image Interface Context */
	{							/* bSupportAlgos[BRAP_MAX_AUDIO_TYPES]. Default supported algorithms */
		true,						/* MPEG */
		false,					/* AAC */
		false,					/* AAC_SBR */
		true,						/* AC3 */
		false,					/* AC3_PLUS */
#if (BRAP_DTS_SUPPORTED == 1)		
		true,					/* DTS */
#else		
		false,					/* DTS */
#endif		
		false,					/* LPCM_BD */
		false,					/* LPCM_HDDVD */
		false,					/* DTS-HD */
		false,					/* LPCM-DVD */
		false	,					/* WMA_STD */
		false	,					/* AC3_LOSSLESS */
		false,						/* MLP */
		false,						/*WMA PRO*/
		false,                   /* PCM WAVE*/		
	   	true					/* DRA */   	
#ifdef RAP_SRSTRUVOL_CERTIFICATION
		,true					/* PCM */
#endif
	}
#if (BRAP_7401_FAMILY == 1) 
	,
	{
		false,						/* Dolby Digital Bass Management */
#ifdef RAP_SRSTRUVOL_CERTIFICATION
		false,		 				/* SRC */
#else
#ifndef WMA_CERTIFICATION   
        true,                       /* SRC */
#else
        false,
#endif 
#endif			       
        true,                       /* AAC downmix */
        true                        /* DSOLA */
#if (BRAP_AD_SUPPORTED == 1)
		,true,                       /* AD FadeCtrl */
		true                       /* AD PanCtrl */
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		,false
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		,true
#endif
	}
#elif (BCHP_CHIP == 7400)
	,
	{
		false,			  /* Dolby Digital Bass Management */
#ifdef RAP_SRSTRUVOL_CERTIFICATION
		false,				/* SRC */
#else
		true,                       /* SRC */
#endif
		true,                       /* AAC downmix */
		true                      /* DSOLA */
#if (BRAP_AD_SUPPORTED == 1)
		,false,                       /* AD FadeCtrl */
		false                       /* AD PanCtrl */
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		,true
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		,true
#endif	
	
	}
#endif	
#ifdef BCHP_7411_VER
	,
	{
		false						/* Dolby Digital Bass Management */
	},
	BRAP_AudioMemAllocation_eStereo	/* Memory allocation type */
#endif
#ifndef BCHP_7411_VER
	,
	{
		true,						/* MP3 ENC */
#if ( BCHP_CHIP == 7440 )
		false	,					/* DTS ENC */
#endif
	}
#endif
};

#ifndef BCHP_7411_VER
/* Static array defining CDB and ITB sizes for various 
   Buffer Configuration Modes */

#if (BSTD_CPU_ENDIAN==BSTD_ENDIAN_BIG)
/* For big endian code on 7401, 7400 */
static const BAVC_CdbItbConfig sCdbItbCfg[BRAP_BufferCfgMode_eMaxMode] =
{               
	{ { 128 * 1024, 4, false }, { 64 * 1024, 4, false },false },	/* BRAP_BufferCfgMode_eStdAudioMode */
	{ { 128  * 1024, 4, false }, { 64 * 1024, 4, false },false },	/* BRAP_BufferCfgMode_eAdvAudioMode */
	{ { 256 * 1024, 4, false }, { 256 * 1024, 4, false },false },/* BRAP_BufferCfgMode_eIpStbMode */
	{ { 32 * 1024, 4, false }, { 32 * 1024, 4, false },false },	/* BRAP_BufferCfgMode_eHdDvdMode */
	{ { 32 * 1024, 4, false }, { 32 * 1024, 4, false },false }	/* BRAP_BufferCfgMode_eBlueRayMode */
	/* TODO: Pupulate this array as more new modes are geting added */
};
#else
/* For little endian code on 7401, 7400 */
static const BAVC_CdbItbConfig sCdbItbCfg[BRAP_BufferCfgMode_eMaxMode] =
{


	{ { 128 * 1024, 4, true }, { 64 * 1024, 4, false }, false },	/* BRAP_BufferCfgMode_eStdAudioMode */
	{ { 128  * 1024, 4, true }, { 64 * 1024, 4, false }, false },	/* BRAP_BufferCfgMode_eAdvAudioMode */
	{ { 256 * 1024, 4, true }, { 256 * 1024, 4, false }, false },/* BRAP_BufferCfgMode_eIpStbMode */
	{ { 32 * 1024, 4, true }, { 32 * 1024, 4, false }, false },	/* BRAP_BufferCfgMode_eHdDvdMode */
	{ { 32 * 1024, 4, true }, { 32 * 1024, 4, false }, false }	/* BRAP_BufferCfgMode_eBlueRayMode */
	/* TODO: Pupulate this array as more new modes are geting added */
};
#endif

#endif /* 7401, 7400 etc */

/*}}}*/

/***************************************************************************
Summary:
	Instantiates the Raptor Audio Device.

Description:
	This function should be called to initialize the Raptor Audio Device. 
	This API must be called prior to any other Raptor PI call.
	As a result a handle to the Raptor Audio Device is returned by this API, 
	which represents the instance of this device. This Device handle needs 
	to be passed to APIs operating at device level.
Returns:
	BERR_SUCCESS - If open is successful.

See Also:
	BRAP_Close, BRAP_GetDefaultSettings.
***************************************************************************/
BERR_Code BRAP_Open(
	BRAP_Handle 			*phRap,			/* [out] The RAP handle returned*/
	BCHP_Handle 			hChip,			/* [in] Chip handle */
#if BRAP_P_USE_BARC == 1
	BARC_Handle				hArc,			/* [in] Arc handle */
#else
	BREG_Handle				hRegister,		/* [in] Register handle */	
#endif
	BMEM_Handle 			hMemory,		/* [in] Memory heap handle */
	BINT_Handle			hInt,			/* [in] Interrupt module handle */
#ifndef BCHP_7411_VER
	BTMR_Handle			hTmr,			/* [in]Timer modules handle */
#endif
	const BRAP_Settings	*pSettings		/* [in] Device settings */
	)
{
	BRAP_Handle			hRap;
	BERR_Code			ret = BERR_SUCCESS;
	BRAP_RM_Handle		hRm;
#if (BRAP_AD_SUPPORTED == 1)
	unsigned int        i=0;
#endif
	
	BDBG_ENTER(BRAP_Open);
	
	/* Assert the function arguments*/
	BDBG_ASSERT(phRap);
	BDBG_ASSERT(hChip);
	BDBG_ASSERT(hInt);
#ifndef BCHP_7411_VER
#if BRAP_P_WATERMARK_WORKAROUND== 1
	BDBG_ASSERT(hTmr);
#else
    BSTD_UNUSED(hTmr);
#endif
#endif
	BDBG_ASSERT(pSettings);	
#ifndef BCHP_7411_VER
	/* there's no memory handle for the 7411. Other chips have memory handle. */
	BDBG_ASSERT(hMemory);
#endif
#if BRAP_P_USE_BARC == 1
	BDBG_ASSERT(hArc);
#else
	BDBG_ASSERT(hRegister);
#endif
	BDBG_ASSERT(pSettings->pImgInterface);

	/* Allocate the Audio Device handle */
	hRap = (BRAP_Handle)BKNI_Malloc( sizeof(BRAP_P_Device));
	if(hRap == NULL)
	{
		ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		return ret;
	}



	/* Reset the entire structure */
	BKNI_Memset( hRap, 0, sizeof(BRAP_P_Device) );

	/* Store the device settings */
	hRap->sSettings = *pSettings ;
	
	/* Copy the base-module handles and the settings to the device handle */
	hRap->hChip = hChip;

#if BRAP_P_USE_BARC == 1
	hRap->hRegister = (BREG_Handle)hArc;
#else
	hRap->hRegister = hRegister;
#endif

	hRap->hHeap = hMemory;
	hRap->hInt = hInt;
	hRap->bResetHwDone=false;
#if BRAP_P_WATERMARK_WORKAROUND == 1
	hRap->hTmr = hTmr;
#endif
    hRap->uiRampStepSize = BRAP_P_DEFAULT_RAMP_SIZE;

#if (BRAP_AD_SUPPORTED == 1)
    /* Initalize the Audio Descriptor Objects */
    for (i=0; i<BRAP_P_MAX_AUDIO_DESC_OBJECTS; i++)
    {
		hRap->sAudioDescObjs[i].bAvailable = true;
		hRap->sAudioDescObjs[i].sExtAudioDescDetails.hPrimaryChannel = NULL;
		hRap->sAudioDescObjs[i].sExtAudioDescDetails.hDescriptorChannel = NULL;
		hRap->sAudioDescObjs[i].hRap = NULL;
		hRap->sAudioDescObjs[i].ui32PanFadeInterfaceAddr = BRAP_RM_P_INVALID_INDEX;
    }
#endif

	if (pSettings->bExtDeviceMem) 
      {
		hRap->sMemAllocInfo.currentStaticMemoryPointer =
			pSettings->ui32MemBase;          
			hRap->sMemAllocInfo.currentStaticMemoryBase =
			pSettings->ui32MemBase;
		hRap->sMemAllocInfo.staticMemorySize = 
			pSettings->ui32MemSize;
		BDBG_MSG(("\n*** Static memory size = 0x%x", pSettings->ui32MemSize));
	}

	ret = BERR_TRACE(BRAP_P_Open(hRap));
	if(ret != BERR_SUCCESS)
	{
            goto free;
	}

	/* TODO: Initialize common interrupt callbacks */
	ret = BERR_TRACE(BRAP_P_DeviceLevelInterruptInstall(hRap));
	if(ret != BERR_SUCCESS)
	{
            goto free;
	}

	/* Instantiate the resource manager */
	ret = BERR_TRACE(BRAP_RM_P_Open(hRap, &hRm));
	if(ret != BERR_SUCCESS)
	{
            goto free;
	}

	*phRap = hRap;
    goto end;

free:
	/* Free the memory allocated for Audio Device handle */
	BKNI_Free( hRap );
    
end:
	BDBG_LEAVE(BRAP_Open);
	
	return ret;
}

BERR_Code BRAP_P_Open(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
)
{
	BERR_Code			ret = BERR_SUCCESS;
	BRAP_DSP_Handle		hDsp;
	BRAP_DSP_P_Settings	sDspSettings;
	BRAP_FMM_P_Handle		hFmm;
	BRAP_FMM_P_Settings	sFmmSettings;
	unsigned int		uiDspIndex = 0;
	unsigned int		uiFmmIndex = 0;

  	/* Instantiate the DSP devices */
	for(uiDspIndex = 0; uiDspIndex < BRAP_RM_P_MAX_DSPS; uiDspIndex++)
	{
		/* If this is a watchdog recovery, we already have DSP handles */
		if (BRAP_P_GetWatchdogRecoveryFlag(hRap) == true)
        {
			hDsp = hRap->hDsp[uiDspIndex];
            BRAP_DSP_P_EnableDspWatchdogTimer (hDsp, true);
	    }
		/* Currently BRAP_DSP_P_Settings doesn't have any members, so no need to 
		   initialize sDspSettings structure. Once this structure has some member, 
		   initialize it as per the requirement */
		ret = BRAP_DSP_P_Open (&hDsp, hRap, uiDspIndex, &sDspSettings);
		if(ret != BERR_SUCCESS)
		{
		       BDBG_ERR(("BRAP_P_Open: call to BRAP_DSP_P_Open failed."));
			return BERR_TRACE(ret);
		}

		/* If not watchdog recovery, Store the DSP handle inside the Audio Device Handle  */
		if (BRAP_P_GetWatchdogRecoveryFlag(hRap)== false)
		hRap->hDsp[uiDspIndex] = hDsp;
	}

	/* Instantiate the FMM devices */
	for(uiFmmIndex = 0; uiFmmIndex < BRAP_RM_P_MAX_FMMS; uiFmmIndex++)
	{
		/* If this is a watchdog recovery, we already have FMM handles */
		if ((BRAP_P_GetWatchdogRecoveryFlag(hRap)))
			hFmm = hRap->hFmm[uiFmmIndex]; 
		/* Currently BRAP_FMM_P_Settings doesn't have any members, so no need to 
		   initialize sFmmSettings structure. Once this structure has some member, 
		   initialize it as per the requirement */
		ret = BRAP_FMM_P_Open (hRap, &hFmm, uiFmmIndex, &sFmmSettings);
		if(ret != BERR_SUCCESS)
		{
		       BDBG_ERR(("BRAP_P_Open: call to BRAP_FMM_P_Open failed."));		
			return BERR_TRACE(ret);
		}

		/* If not watchdog recovery, Store the FMM handle inside the Audio Device Handle */
		if (BRAP_P_GetWatchdogRecoveryFlag(hRap)== false)
		hRap->hFmm[uiFmmIndex] = hFmm;
	}


  	/* Clear all previous raptor interrupts and mask them all */
	/* This has to be called only after BRAP_FMM_P_Open as the 
	FMM module has to be released from reset state to access the registers */
	BRAP_P_ClearInterrupts (hRap->hRegister);

	/* TODO: Initialize common interrupt callbacks */
	return ret;
}
#ifndef BCHP_7411_VER
/***************************************************************************
Summary:
	This function takes out DSP processor from reset.

Description:
	When firmware authentication is enabled, BRAP_Open keeps DSP processor in 
	reset condition for security software module to authenticate the Raptor 
	firmware. After firmware authentication is done, application needs to call
	this function to take out DSP processor from reset. After BRAP_Open this 
	function should be called first before calling any other RAP PI functions.

Returns:
	BERR_SUCCESS - If unresetting DSP is successful.

See Also:
***************************************************************************/

BERR_Code BRAP_StartDsp(BRAP_Handle hRap)
{

	BRAP_DSP_Handle handle;
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BRAP_StartDSP);

	/* Assert the function arguments*/
	BDBG_ASSERT(hRap);
	
	handle=hRap->hDsp[0];
	/* Initializing the Hardware */

	rc = BRAP_DSP_P_InitHardware(handle);
	
	if (rc!=BERR_SUCCESS) {
		BKNI_Free(handle->psMitArrays);
		BDBG_MSG(("BRAP_DSP_P_InitHardware::Error returned %x!",rc));
		return BERR_TRACE(rc);
	
	}
	BDBG_LEAVE(BRAP_StartDSP);
	return rc;
}

/***************************************************************************
Summary:
	Returns physical memory offset and size of firmware executables.
	
Description:
	Returns the physical memory offset where firmware executables of all the 
	supported audio codecs are downloaded in contiguous memory locations. It 
	also returns the total size of the downloaded firmware executables. This 
	function is supported only when firmware authentication is enabled.

Returns:
	BERR_SUCCESS - If valid offset is set.
	BERR_NOT_SUPPORTED - If Firmware authentication is Disabled.

See Also:	
***************************************************************************/

BERR_Code BRAP_GetFwDwnldInfo( 
	BRAP_Handle 		hRap,			/* [in] The Raptor Audio device handle*/
	uint32_t		*ui32FwDwnldOffset,	/* [out] Physical memory offset of 
											downloaded firmware executables  */
	uint32_t		*ui32FwDwnldAddress,	/* [out] Virtual memory offset of
											downloaded firmware executables  */											
	uint32_t		*ui32FwDwnldSize	/* [out] Total size of downloaded 
											firmware executables */ 
	)
{
	
	BDBG_ENTER(BRAP_GetFwDwnldInfo);
	/* Assert the function arguments*/
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(ui32FwDwnldOffset);
	BDBG_ASSERT(ui32FwDwnldAddress);
	BDBG_ASSERT(ui32FwDwnldSize);

	
	if(hRap->sSettings.bFwAuthEnable==false) /*If Firmware Firmware authentication 
											is Disabled*/
	{
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	*ui32FwDwnldAddress = hRap->hDsp[0]->memInfo.fwDwnldOffset;	
	BRAP_P_ConvertAddressToOffset(hRap->hHeap,(void *)hRap->hDsp[0]->memInfo.fwDwnldOffset,ui32FwDwnldOffset);
	*ui32FwDwnldSize=hRap->hDsp[0]->memInfo.allocFwDwnldSize;
	
	BDBG_LEAVE(BRAP_GetFwDwnldInfo);
	return BERR_SUCCESS;
}
#endif

/***************************************************************************
Summary:
	Closes the Raptor Audio Device.

Description:
	This function should be called when the application is exiting or the 
	application doesn't intend to use Raptor Audio PI further. This call 
	destroys the instance of the Raptor Audio Device handle. 

Returns:
	BERR_SUCCESS - If close is successful.

See Also:
	BRAP_Open, BRAP_GetDefaultSettings.
***************************************************************************/
BERR_Code BRAP_Close(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
	)
{
	BERR_Code			ret = BERR_SUCCESS;
	unsigned int		uiDspIndex = 0;
	unsigned int		uiFmmIndex = 0;

	unsigned int	uiDecChIndex=0;
  	unsigned int	uiPCMPbIndex=0;
  	unsigned int	uiPCMCapIndex=0;

#if (!defined BCHP_7411_VER) && (BCHP_CHIP != 7118) /* For chips other than 7411 */
	unsigned int 	uiEncChIndex=0;
#endif
	
	BDBG_ENTER(BRAP_Close);
	
	/* Assert the function arguments*/
	BDBG_ASSERT(hRap);

	/* Stop and close all decode channels */
	for (uiDecChIndex = 0; uiDecChIndex < BRAP_RM_P_MAX_DEC_CHANNELS; uiDecChIndex++) 
	{
		if (hRap->hRapDecCh[uiDecChIndex] != NULL) 
		{
			if (hRap->hRapDecCh[uiDecChIndex]->bStarted == true)
			{
				ret = BRAP_DEC_Stop(hRap->hRapDecCh[uiDecChIndex]);
				if(ret == BERR_SUCCESS)
					BDBG_MSG(("\nRAP Decode Channel %d Stopped", uiDecChIndex));				
				else
					BDBG_MSG(("\nRAP Decode Channel %d Stop failed", uiDecChIndex));				
			}
								
			ret = BRAP_DEC_CloseChannel(hRap->hRapDecCh[uiDecChIndex]); 
			if(ret == BERR_SUCCESS)
				BDBG_MSG(("\nRAP Decode Channel %d closed", uiDecChIndex));					
			else
				BDBG_MSG(("\nRAP Decode Channel %d close failed", uiDecChIndex));					

		}		
	}

	/* Stop and Close all playback channels */
	for (uiPCMPbIndex = 0; uiPCMPbIndex < BRAP_RM_P_MAX_PCM_CHANNELS; uiPCMPbIndex++) 
	{
		if (hRap->hRapPbCh[uiPCMPbIndex] != NULL) 
		{
			if (hRap->hRapPbCh[uiPCMPbIndex]->bStarted == true)
			{
				ret = BRAP_PB_Stop(hRap->hRapPbCh[uiPCMPbIndex]);
				if(ret == BERR_SUCCESS)
					BDBG_MSG(("\nRAP PCM playback channel %d stopped", uiPCMPbIndex));
				else
					BDBG_MSG(("\nRAP PCM playback channel %d stop failed", uiPCMPbIndex));
			}

			ret= BRAP_PB_CloseChannel(hRap->hRapPbCh[uiPCMPbIndex]);
			if(ret == BERR_SUCCESS)
				BDBG_MSG(("\nRAP PCM playback channel %d closed", uiPCMPbIndex));
			else
				BDBG_MSG(("\nRAP PCM playback channel %d close failed", uiPCMPbIndex));

		}
	}

	/* Stop and close all capture channels */
	for (uiPCMCapIndex = 0; uiPCMCapIndex < BRAP_RM_P_MAX_CAP_CHANNELS; uiPCMCapIndex++) 
	{
      		if (hRap->hRapCapCh[uiPCMCapIndex]!=NULL) 
		{
			if (hRap->hRapCapCh[uiPCMCapIndex]->bStarted == true)
			{
				ret = BRAP_CAP_Stop(hRap->hRapCapCh[uiPCMCapIndex]);
				if(ret == BERR_SUCCESS)
					BDBG_MSG(("\nRAP capture channel %d stopped", uiPCMCapIndex));
				else
					BDBG_MSG(("\nRAP capture channel %d stop failed", uiPCMCapIndex));					
			}

			ret = BRAP_CAP_CloseChannel(hRap->hRapCapCh[uiPCMCapIndex]);
			if(ret == BERR_SUCCESS)
				BDBG_MSG(("\nRAP capture channel %d closed", uiPCMCapIndex));
			else
				BDBG_MSG(("\nRAP capture channel %d close failed", uiPCMCapIndex));
      		}
	}

#if (!defined BCHP_7411_VER) && (BCHP_CHIP != 7118) /* For chips other than 7411 */
	/* Stop & close all encode channels */
	for (uiEncChIndex = 0; uiEncChIndex < BRAP_RM_P_MAX_ENC_CHANNELS; uiEncChIndex++) 
	{
		if (hRap->hRapEncCh[uiEncChIndex]) 
		{
			if (hRap->hRapEncCh[uiEncChIndex]->bEncoderStopped== true)
			{
				ret = BRAP_ENC_Stop(hRap->hRapEncCh[uiEncChIndex], 0, uiEncChIndex);
				if(ret == BERR_SUCCESS)
					BDBG_MSG(("\nRAP Encode channel %d stopped", uiEncChIndex));
				else
					BDBG_MSG(("\nRAP Encode channel %d stop failed", uiEncChIndex));					
			}

			ret = BRAP_ENC_CloseChannel(hRap->hRapEncCh[uiEncChIndex], uiEncChIndex);
			if(ret == BERR_SUCCESS)
				BDBG_MSG(("\nRAP Encode channel %d closed", uiEncChIndex));
			else
				BDBG_MSG(("\nRAP Encode channel %d close failed", uiEncChIndex));	
		}
	}
#endif

	/* Close the DSP devices */
	for(uiDspIndex = 0; uiDspIndex < BRAP_RM_P_MAX_DSPS; uiDspIndex++)
	{
		ret = BRAP_DSP_P_Close (hRap->hDsp[uiDspIndex]);
		if(ret != BERR_SUCCESS)
		{
			return BERR_TRACE(ret);
		}
	}
	/* Close the FMM devices */
	for(uiFmmIndex = 0; uiFmmIndex < BRAP_RM_P_MAX_FMMS; uiFmmIndex++)
	{
		ret = BRAP_FMM_P_Close (hRap->hFmm[uiFmmIndex]);
		if(ret != BERR_SUCCESS)
		{
			return BERR_TRACE(ret);
		}
	}
	/* Close the resource manager */
	ret = BRAP_RM_P_Close(hRap, hRap->hRm);
	if(ret != BERR_SUCCESS)
	{
		return BERR_TRACE(ret);
	}

	/* Free up the Audio Device handle */
	BKNI_Free( hRap );

	BDBG_LEAVE(BRAP_Close);
	
	return ret;
}


/***************************************************************************
Summary:
	Process Raptor watchdog interrupt

Description:
	This function processes Raptor watchdog interrupt. This is not _isr safe
	function and should be called only from task context. Call to this function
	should be treated as critical section code.

Returns:
	BERR_SUCCESS 

See Also:
	None
****************************************************************************/
BERR_Code BRAP_ProcessWatchdogInterrupt(
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
#ifdef BCHP_7411_VER /* Only for the 7411 */
	 ,BRAP_WatchdogRecoveryMode	eWatchdogRecoveryMode /* [in] Watchdog Recovery Mode */
#endif
)
{
	unsigned int	uiDecChIndex;
  	unsigned int	uiPCMPbIndex;
  	unsigned int	uiPCMCapIndex;
#if (!defined BCHP_7411_VER) && (BCHP_CHIP != 7118) /* For chips other than 7411 */
	unsigned int 	uiEncChIndex;
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
	unsigned int	uiCount;
#endif
#if BRAP_P_USE_BRAP_TRANS ==1
	BRAP_TRANS_ChannelHandle	hRapTransCh;
	bool bTransChStarted = false;
#endif
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_ProcessWatchdogInterrupt);
	
    BKNI_EnterCriticalSection();
    /* BRAP_ProcessWatchdogInterrupt() should not be called from within 
    Critical sections. To ensure this, added these few lines of code. 
    Do nothing here. If BRAP_ProcessWatchdogInterrupt is called incorrectly from 
    a critical section, this piece of code will cause it to deadlock and exit */
    BKNI_LeaveCriticalSection();

    
	/* Assert the function arguments*/
	BDBG_ASSERT(hRap);

	/* Set the watchdog recovery flag */
	hRap->bWatchdogRecoveryOn = true;

	/* Reopen DSP and FMM devices */
	ret = BRAP_P_Open(hRap);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);

	 /* Set the Reset Hardware flag to false*/
	hRap->bResetHwDone=false;

	/* Open and start all the affected DSP and FMM modules */
	for (uiDecChIndex = 0; uiDecChIndex < BRAP_RM_P_MAX_DEC_CHANNELS; uiDecChIndex++) {
		/* Reopen active channels */	
		if (hRap->hRapDecCh[uiDecChIndex]) { 
#if BRAP_P_USE_BRAP_TRANS ==1			
                     hRapTransCh = hRap->hRapDecCh[uiDecChIndex]->hRapTransCh;
			bTransChStarted = BRAP_TRANS_P_GetChannelStatus(hRapTransCh);
			if ((eWatchdogRecoveryMode==BRAP_WatchdogRecoveryMode_eAudio) && bTransChStarted) {
				/* 7411 audio transport in good condition. Need to stop the audio transport 
				   channels. */
				ret = BRAP_TRANS_StopChannel(hRapTransCh);
				if (ret!=BERR_SUCCESS)
					return BERR_TRACE(ret);
			}
#endif

			/* Reopen DEC channels - Need not pass correct channel number and channel settings 
			   values. Stored values in  channel handle will get used. */
#if BRAP_P_USE_BRAP_TRANS ==1
                     ret = BRAP_DEC_OpenChannel(hRap, hRapTransCh, &hRap->hRapDecCh[uiDecChIndex], uiDecChIndex, NULL);
#else
			ret = BRAP_DEC_OpenChannel(hRap, &hRap->hRapDecCh[uiDecChIndex], uiDecChIndex, NULL);
#endif
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
			
#if BRAP_P_USE_BRAP_TRANS ==1
			/* Restart audio transport channel - Need not pass correct param values.
			    Stored values in  channel handle will get used. */
			if (bTransChStarted) {
				ret = BRAP_TRANS_P_StartChannel(hRapTransCh); 
				if (ret!=BERR_SUCCESS)
					return BERR_TRACE(ret);
			}
#endif

			/* Restart DEC channels - Need not pass correct param values.
			    Stored values in  channel handle will get used. */
			if (hRap->hRapDecCh[uiDecChIndex]->bStarted == true)
		    { /* re-start the channel ONLY if it was started earlier */
    			ret = BRAP_DEC_Start(hRap->hRapDecCh[uiDecChIndex], NULL);
    			if (ret!=BERR_SUCCESS)
    				return BERR_TRACE(ret);
		    }
		}
		/* TODO: Restore back PVR trick mode states */
	}

    /* Open and start all the affected PCM Playback channels */
	for (uiPCMPbIndex = 0; uiPCMPbIndex < BRAP_RM_P_MAX_PCM_CHANNELS; uiPCMPbIndex++) 
    {
       	/* Reopen active channels */	
		if (hRap->hRapPbCh[uiPCMPbIndex]) 
        { 
   			ret = BRAP_PB_OpenChannel(hRap, &hRap->hRapPbCh[uiPCMPbIndex], uiPCMPbIndex, NULL);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);

			if (hRap->hRapPbCh[uiPCMPbIndex]->bStarted == true)
		    { /* re-start the channel ONLY if it was started earlier */
               	ret = BRAP_PB_Start(hRap->hRapPbCh[uiPCMPbIndex], NULL);
        		if (ret!=BERR_SUCCESS)
        			return BERR_TRACE(ret);
		    }
        }
    }

    /* Open and start all the affected PCM Capture channels */
	for (uiPCMCapIndex = 0; uiPCMCapIndex < BRAP_RM_P_MAX_CAP_CHANNELS; uiPCMCapIndex++) 
    {
       	/* Reopen active channels */	
		if (hRap->hRapCapCh[uiPCMCapIndex]) 
        { 
            ret = BRAP_CAP_OpenChannel(hRap, &hRap->hRapCapCh[uiPCMCapIndex], uiPCMCapIndex, NULL);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);

            if (hRap->hRapCapCh[uiPCMCapIndex]->bStarted == true)
		    { /* re-start the channel ONLY if it was started earlier */
               	ret = BRAP_CAP_Start(hRap->hRapCapCh[uiPCMCapIndex], NULL);
    			if (ret!=BERR_SUCCESS)
    				return BERR_TRACE(ret);
            }
        }
    }

#ifndef BCHP_7411_VER /* For chips other than 7411 */
#if (BCHP_CHIP != 7118)
    /* Open and start all the affected Audio Encode channels */
	for (uiEncChIndex = 0; uiEncChIndex < BRAP_RM_P_MAX_ENC_CHANNELS; uiEncChIndex++) 
    {
       	/* Reopen active channels */	
		if (hRap->hRapEncCh[uiEncChIndex]) 
        { 
   			ret = BRAP_ENC_OpenChannel(hRap, &hRap->hRapEncCh[uiEncChIndex], uiEncChIndex, NULL);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
/* Not starting ENC again here as it is a NRT encoding & files etc. need to be reset */
/*	           	ret = BRAP_ENC_Start(hRap->hRapEncCh[uiEncChIndex], NULL, uiEncChIndex);
				if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);*/
        }
    }
#endif
	/* Recover the ExtMClockSettings */
	for ( uiCount =0; uiCount<BRAP_FMM_P_MAX_EXT_MCLK; uiCount++ )
	{
		ret = BRAP_OP_ExtMClkConfig (hRap, (BRAP_OP_ExtMClk)uiCount, &hRap->hFmm[0]->sExtMClkSettings[uiCount]);
		if (ret!=BERR_SUCCESS)
			return BERR_TRACE(ret);
	}

	/* This is done to clear watchdog Interrupts properly */
	ret = BINT_DisableCallback (hRap->hCallback);
	ret = BINT_EnableCallback (hRap->hCallback);
#endif

	/* Reset the watchdog recovery flag */
	hRap->bWatchdogRecoveryOn = false;

	BDBG_LEAVE(BRAP_ProcessWatchdogInterrupt);

	return ret;
}

#if (BRAP_AD_SUPPORTED == 1)
/***************************************************************************
Summary:
    Creates audio descriptor fade table object.
Description:
    This function creates audio descriptor fade table object. For every audio 
    descriptor channel pair, there needs to be one audio descriptor fade table.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_DestroyAudioDescriptorFadeTable
    BRAP_ConfigureAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_CreateAudioDescriptorFadeTable(
    BRAP_Handle hRap, 
    BRAP_AudioDescriptorFadeHandle *phAdFade
    )
{
    BERR_Code ret  = BERR_SUCCESS;
    int i = 0;
    uint32_t    ui32PanFadeInterfaceAddr; /* Dram location for interface with fw */
    

    BDBG_ENTER(BRAP_CreateAudioDescriptorFadeTable);

    /* Validate input params */
    BDBG_ASSERT (hRap);
    BDBG_ASSERT (phAdFade);    

    *phAdFade = NULL;
    for (i=0; i<BRAP_P_MAX_AUDIO_DESC_OBJECTS; i++)
    {
        if (true == hRap->sAudioDescObjs[i].bAvailable)
        {
            break;
        }
    }

    if (BRAP_P_MAX_AUDIO_DESC_OBJECTS == i)
    {
        BDBG_ERR(("BRAP_CreateAudioDescriptorFadeTable: Max audio desciptor fade tables reached"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto exit;		
    }

    hRap->sAudioDescObjs[i].ui32PanFadeInterfaceAddr
	    =  (uint32_t)BRAP_P_AllocAligned(hRap, sizeof(uint32_t),2, 0
		    #if (BRAP_SECURE_HEAP==1)
						,false
		    #endif	
		    );	/* 32 bit aligned*/

    if((void *) BRAP_P_INVALID_DRAM_ADDRESS == (void *)hRap->sAudioDescObjs[i].ui32PanFadeInterfaceAddr)
    {
        BDBG_ERR(("BRAP_CreateAudioDescriptorFadeTable: Unable to Allocate memory for firmware interface address!"));
        ret = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto exit;
    }			

	/* Initialize the DRAM address with default */
    BRAP_MemWrite32 ((BARC_Handle) hRap->hRegister, 
        (hRap->sAudioDescObjs[i].ui32PanFadeInterfaceAddr), 
        0x7FFFFFFF);


    BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
            (void *)hRap->sAudioDescObjs[i].ui32PanFadeInterfaceAddr, 
            (void *)&(ui32PanFadeInterfaceAddr));

    BDBG_MSG(("ui32PanFadeInterfaceAddr = %x",ui32PanFadeInterfaceAddr));

	BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER5_CXT0, ui32PanFadeInterfaceAddr); 
	BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER5_CXT1, ui32PanFadeInterfaceAddr); 

    hRap->sAudioDescObjs[i].bAvailable = false;
    hRap->sAudioDescObjs[i].hRap = hRap;
    *phAdFade = &(hRap->sAudioDescObjs[i]);

exit:
    BDBG_LEAVE(BRAP_CreateAudioDescriptorFadeTable);
    return ret;
}

/***************************************************************************
Summary:
    Destroys audio descriptor fade table object.
Description:
    This function destroys audio descriptor fade table object. 
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_ConfigureAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_DestroyAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade
    )
{
    BERR_Code ret  = BERR_SUCCESS;

    BDBG_ENTER(BRAP_DestroyAudioDescriptorFadeTable);

    /* Validate input params */
    BDBG_ASSERT (hAdFade);

    hAdFade->sExtAudioDescDetails.hPrimaryChannel = NULL;
    hAdFade->sExtAudioDescDetails.hDescriptorChannel = NULL;

#if (BRAP_SECURE_HEAP==1)
    BRAP_P_Free (hAdFade->hRap, (void*)hAdFade->ui32PanFadeInterfaceAddr,false);
#else
    BRAP_P_Free (hAdFade->hRap, (void*)hAdFade->ui32PanFadeInterfaceAddr);
#endif    
    hAdFade->ui32PanFadeInterfaceAddr = BRAP_RM_P_INVALID_INDEX;
    hAdFade->hRap = NULL;
    hAdFade->bAvailable = true;
   
    BDBG_LEAVE(BRAP_DestroyAudioDescriptorFadeTable);
    return ret;
}

/***************************************************************************
Summary:
    Configures audio descriptor fade table object.
Description:
    This function configures audio descriptor fade table object. For a given audio
    descriptor feature, this function identifies which RAP channel is a primary channel
    and which RAP channel is audio descriptor channel.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_DestroyAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_ConfigureAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade, 
    BRAP_AudioDescriptorFadeTableParams sParams
    )
{
    BERR_Code ret  = BERR_SUCCESS;

    BDBG_ENTER(BRAP_ConfigureAudioDescriptorFadeTable);

    /* Validate input params */
    BDBG_ASSERT (hAdFade);

    hAdFade->sExtAudioDescDetails = sParams;
  
    BDBG_LEAVE(BRAP_ConfigureAudioDescriptorFadeTable);
    return ret;
}

#endif

/***************************************************************************
Summary:
	Gets the default settings of the Raptor Audio device.
Description:
	This API returns the default settings of the Raptor Audio device.
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_Open.
***************************************************************************/
BERR_Code BRAP_GetDefaultSettings(
	BRAP_Settings 			*pDefaultSettings /* [out] Default device 
													   settings */
	)
{
	BDBG_ENTER(BRAP_GetDefaultSettings);
	
	/* Assert the function argument(s) */
	BDBG_ASSERT(pDefaultSettings);
	
	*pDefaultSettings = sDefSetting;
	
	BDBG_LEAVE(BRAP_GetDefaultSettings);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	This API returns the default settings for a particular output port.
Returns:
	BERR_SUCCESS  on success,
	error value on failure
See Also:
	BRAP_SetOutputConfig, BRAP_GetOutputConfig
***************************************************************************/
BERR_Code BRAP_GetOutputDefaultConfig ( 
	BRAP_Handle 		hRap,			/* [in] The RAP device handle */
	BRAP_OutputPort		eOutput,		/* [in] Output type */
	BRAP_OutputSettings *pDefSettings	/* [out] Default output settings */
	)
{
	BERR_Code				ret = BERR_SUCCESS;
	BRAP_OP_P_MaiSettings	sMaiSettings;
	BRAP_OP_P_SpdifSettings	sSpdifSettings;
	BRAP_OP_P_I2sSettings	sI2sSettings;
	BRAP_OP_P_FlexSettings	sFlexSettings;
	BRAP_OP_P_DacSettings	sDacSettings;
    BRAP_SPDIFFM_P_Settings   sSpdifFmSettings;
#ifndef BCHP_7411_VER
    BRAP_SPDIFFM_P_Params sSpdifFmParams;
#endif
	BDBG_ENTER(BRAP_GetOutputDefaultConfig);

	/* Validate input parameters. */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pDefSettings);
	BSTD_UNUSED(hRap);

    BDBG_MSG(("BRAP_GetOutputDefaultConfig: output port = %d ", eOutput));

    pDefSettings->eOutput = eOutput;
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    pDefSettings->iDelay= 0;
    pDefSettings->bIndpDlyRqd = false;
#endif
    pDefSettings->eOutputSR = BAVC_AudioSamplingRate_e48k;     
    pDefSettings->eOutputTimebase = BAVC_Timebase_e0;     
    
    ret = BRAP_SPDIFFM_P_GetDefaultSettings(&sSpdifFmSettings);
    if(ret != BERR_SUCCESS) return BERR_TRACE(ret);
    pDefSettings->sSpdiffmSettings = sSpdifFmSettings.sExtSettings;

#ifndef BCHP_7411_VER /* For chips other than 7411 */  
    ret = BRAP_SPDIFFM_P_GetDefaultParams(&sSpdifFmParams);
    if(ret != BERR_SUCCESS) return BERR_TRACE(ret);
    pDefSettings->sSpdifChanStatusParams = sSpdifFmParams.sChanStatusParams;
#endif

    /* Get the output port default settings and return them */
	switch(eOutput)
	{
		case BRAP_OutputPort_eSpdif:
			ret = BRAP_OP_P_GetDefaultSettings(eOutput, &sSpdifSettings);
            if (ret != BERR_SUCCESS) return BERR_TRACE(ret);    
            pDefSettings->uOutputPortSettings.sSpdifSettings = sSpdifSettings.sExtSettings;
            /* SPDIF needs SPDIFFM. Use the default SPDIFFM settings */
			break;
            
		case BRAP_OutputPort_eI2s0:
		case BRAP_OutputPort_eI2s1:
		case BRAP_OutputPort_eI2s2:
			ret = BRAP_OP_P_GetDefaultSettings(eOutput, &sI2sSettings);
            if (ret != BERR_SUCCESS) return BERR_TRACE(ret);  
				pDefSettings->uOutputPortSettings.sI2sSettings 
							= sI2sSettings.sExtSettings;
            /* SPDIFFM is not required. So change the default SPDIFFM settings*/
            pDefSettings->sSpdiffmSettings.bEnableDither = false;
            pDefSettings->sSpdiffmSettings.eBurstType = BRAP_SPDIFFM_BurstType_eNone;
			break;
            
		case BRAP_OutputPort_eMai:
			ret = BRAP_OP_P_GetDefaultSettings(eOutput, &sMaiSettings);
            if (ret != BERR_SUCCESS) return BERR_TRACE(ret);  
				pDefSettings->uOutputPortSettings.sMaiSettings 
							= sMaiSettings.sExtSettings;
            /* Mai needs SPDIFFM. Use the default SPDIFFM settings */
			break;
            
		case BRAP_OutputPort_eFlex:
			ret = BRAP_OP_P_GetDefaultSettings(eOutput, &sFlexSettings);
            if (ret != BERR_SUCCESS) return BERR_TRACE(ret); 
				pDefSettings->uOutputPortSettings.sFlexSettings 
							= sFlexSettings.sExtSettings;
            /* Flex needs SPDIFFM. Use the default SPDIFFM settings */
			break;
            
		case BRAP_OutputPort_eDac0:
		case BRAP_OutputPort_eDac1:
			ret = BRAP_OP_P_GetDefaultSettings(eOutput, &sDacSettings);
            if (ret != BERR_SUCCESS) return BERR_TRACE(ret); 
				pDefSettings->uOutputPortSettings.sDacSettings 
							= sDacSettings.sExtSettings;
          
            /* SPDIFFM is not required. So change the default SPDIFFM settings*/
            pDefSettings->sSpdiffmSettings.bEnableDither = false;
            pDefSettings->sSpdiffmSettings.eBurstType = BRAP_SPDIFFM_BurstType_eNone;
            break;
            
        default:
            BDBG_ERR(("BRAP_GetOutputDefaultConfig: invalid output port type %d", eOutput));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            
    }

    BDBG_MSG(("BRAP_GetOutputDefaultConfig: bDither(%d) eBurstType(%d)",
        pDefSettings->sSpdiffmSettings.bEnableDither,
        pDefSettings->sSpdiffmSettings.eBurstType));    
	
	BDBG_LEAVE(BRAP_GetOutputDefaultConfig);

    return ret;
}

/***************************************************************************
Summary:
	API used to configure an output port. This API needs to be called 
	seperately for each output port that is in use by the system.    

Description:
    This API needs to be called atleast once before any channel using this 
    output port can be started.

    Note that an output port may be in use by mutliple channels.
    => an output port is 'stopped' when ALL the channels using it are stopped.
    => an output port is 'running' when one or more channels using it have been 
    started.


    Configuration parameters fall into 2 types:

    1. 'start-time only parameters: can be reprogrammed can be reprogrammed every 
    time an output port is stopped and restarted. These can be modified only when 
    the output port is stopped. and will take effect the next time the output port 
    is started. Note that an output port may be in use by mutliple channels.
    => an output port is 'stopped' when ALL the channels using it are stopped.


    2. 'on-the-fly parameters: can be reprogrammed any time ie even while the 
    output port is running, without stopping and restarting the channel.

    The configuration paramters will take effect for ALL channels that are 
    currently using this output port.

    Refer to the description of the output configuration paramters for details 
    on which of them are start-time paramters and which can be changed 
    on-the-fly.  


Returns:
	BERR_SUCCESS on success
	BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED if the output port is not supported
	BERR_NOT_SUPPORTED if there is an attempt to change the start-time 
	paramters while ie output port is running - ie if the output port is in use 
	by ANY running channel

See Also:
	BRAP_GetOutputDefaultConfig, BRAP_GetOutputConfig
****************************************************************************/
BERR_Code BRAP_SetOutputConfig( 
	BRAP_Handle 				hRap,				/* [in] The Raptor Audio device handle*/
	const BRAP_OutputSettings	*pOutputSettings	/* [in] Output port settings*/ 
	)
{

    /* Algo:
    If ((the Output port hasnt been opened) 
         ||(output port is not running at present))
    {
        Only save the settings in hRap->sOutputSettings.
        On a channel start, the settings will be read from 
        hRap->sOutputSettings, programmed to the appropriate registers, and 
        saved in hOp->sSettings.
    }
    else if (if Output port has already been started)
    {
        if (application is trying to change any of the start-time parameters)
            return error;
        else
            1. If not already muted, Mute the output port
            2. Call the Output port's BRAP_OP_P_XXXHWConfig() function to program the
            on-the-fly changeable registers and save the new settings in 
            hOp->sSettings.
            3. If we had muted the output port in 1. above, unmute it.
    }
    */

	BERR_Code				ret = BERR_SUCCESS;	
    BRAP_OP_P_SpdifSettings sSpdifSettings;  
	BRAP_OP_P_I2sSettings sI2sSettings;	
#ifndef BCHP_7411_VER /* For chips other than 7411 */        
    BRAP_OP_P_MaiSettings sMaiSettings;      
    BRAP_OP_P_DacSettings sDacSettings;      
#endif    
    bool bMute = false;
	bool bUsed = false;
	int i=0;	

	BDBG_ENTER(BRAP_SetOutputConfig);

	/* Assert the function argument(s) */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pOutputSettings);

    /* Check requested o/p port with the currently supported output ports */
        ret = BRAP_RM_P_IsOutputPortSupported(pOutputSettings->eOutput);
    	if( ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_SetOutputConfig: eOutputPort(%d) is not supported",
    			pOutputSettings->eOutput));
    		return BERR_TRACE(ret);
    	}


    /* Return an error if  the new port is already
    associated with a decode channel.*/
	for(i=0; i<BRAP_RM_P_MAX_DEC_CHANNELS; i++)
	{
        if (hRap->hRapDecCh[i] != NULL)
        {
            /* Return an error if the new Output port is 
            already in use by any other DEC channel*/
            ret = BRAP_P_IsOpPortUsedbyChannel (hRap->hRapDecCh[i], 
                pOutputSettings->eOutput, &bUsed, NULL /* bClone - ignoring */);
            if (( bUsed == true) && 
                (hRap->sOutputSettings[pOutputSettings->eOutput].bIndpDlyRqd != pOutputSettings->bIndpDlyRqd))
            {
                BDBG_ERR (("BRAP_SetOutputConfig: Cannot Change the bIndpDlyRqd. Requested output"
                    "port %d is already added", pOutputSettings->eOutput));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }	
        }
    }

     /* Return an error if  the new port is already
    associated with a playback channel.*/
	for(i=0; i<BRAP_RM_P_MAX_PCM_CHANNELS; i++)
	{
        if (hRap->hRapPbCh[i] != NULL)
        {
            /* Return an error if the new Output port is 
            already in use by any other Playback channel*/
            ret = BRAP_P_IsOpPortUsedbyChannel (hRap->hRapPbCh[i], 
                pOutputSettings->eOutput, &bUsed, NULL /* bClone - ignoring */);
            if (( bUsed == true) && 
                (hRap->sOutputSettings[pOutputSettings->eOutput].bIndpDlyRqd != pOutputSettings->bIndpDlyRqd))
            {
                BDBG_ERR (("BRAP_SetOutputConfig: Cannot Change the bIndpDlyRqd. Requested output"
                    "port %d is already added", pOutputSettings->eOutput));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }	
        }
    }

    /* Return an error if  the new port is already
    associated with a capture channel.*/
	for(i=0; i<BRAP_RM_P_MAX_CAP_CHANNELS; i++)
	{
        if (hRap->hRapCapCh[i] != NULL)
        {
            /* Return an error if the new Output port is 
            already in use by any other capture channel*/
            ret = BRAP_P_IsOpPortUsedbyChannel (hRap->hRapCapCh[i], 
                pOutputSettings->eOutput, &bUsed, NULL /* bClone - ignoring */);
            if (( bUsed == true) && 
                (hRap->sOutputSettings[pOutputSettings->eOutput].bIndpDlyRqd != pOutputSettings->bIndpDlyRqd))
            {
                BDBG_ERR (("BRAP_SetOutputConfig: Cannot Change the bIndpDlyRqd. Requested output"
                    "port %d is already added", pOutputSettings->eOutput));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }	
        }
    }
		
    
    if ((hRap->hFmm[0]->hOp[pOutputSettings->eOutput] != NULL)
        && (hRap->hFmm[0]->hOp[pOutputSettings->eOutput]->uiStartCnt > 0))
    {
        /* Return an error if the application is attempting to change any 
        start-time parameters. 
        Note: In order to keep the Mute time minimum, do not club this check
        with the switch case for programming on-the-fly registers in HW  */

        if (pOutputSettings->eOutputTimebase 
                != hRap->sOutputSettings[pOutputSettings->eOutput].eOutputTimebase)
        {
            BDBG_ERR(("BRAP_SetOutputConfig: cannot change Timebase "
            "since the output port is currently running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);    
        }
        
        switch (pOutputSettings->eOutput)
        {
            case BRAP_OutputPort_eSpdif:
                if ( (pOutputSettings->uOutputPortSettings.sSpdifSettings.eMClkRate 
                      != hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sSpdifSettings.eMClkRate)
                    ||(pOutputSettings->uOutputPortSettings.sSpdifSettings.ePll 
                      != hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sSpdifSettings.ePll) )
                {
                    BDBG_ERR(("BRAP_SetOutputConfig: cannot change Spdif "
                    "MClkRate/Pll since the output port is currently running."));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);    
                }
                break;              
#ifndef BCHP_7411_VER /* For chips other than 7411 */       
            case BRAP_OutputPort_eMai:
                if ( (pOutputSettings->uOutputPortSettings.sMaiSettings.eMaiMuxSelector 
                      != hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sMaiSettings.eMaiMuxSelector)
                    ||(pOutputSettings->uOutputPortSettings.sMaiSettings.eMaiAudioFormat 
                      != hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sMaiSettings.eMaiAudioFormat)                      
                    ||(pOutputSettings->uOutputPortSettings.sMaiSettings.bSpdifoutput 
                      != hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sMaiSettings.bSpdifoutput) )
                {
                    BDBG_ERR(("BRAP_SetOutputConfig: cannot change Mai "
                    "eMaiMuxSelector/eMaiAudioFormat/bSpdifoutput since the output port is currently running."));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);    
                }
                break;  
            case BRAP_OutputPort_eDac0:
#if (BCHP_CHIP == 7400)                
            case BRAP_OutputPort_eDac1:
#endif                  
                /* no start-time only params for Dac. So do nothing here*/              
                break;
		case BRAP_OutputPort_eI2s0:
		case BRAP_OutputPort_eI2s1:			
		case BRAP_OutputPort_eI2s2:
			if((pOutputSettings->uOutputPortSettings.sI2sSettings.eMClkRate
				!=hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sI2sSettings.eMClkRate)
				||(pOutputSettings->uOutputPortSettings.sI2sSettings.ePll
				!=hRap->sOutputSettings[pOutputSettings->eOutput].uOutputPortSettings.sI2sSettings.ePll) )
			{
                    BDBG_ERR(("BRAP_SetOutputConfig: cannot change I2S"
                    "MClkRate/Pll since the output port is currentlyrunning."));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);   
                }
                break;			
#endif  /* For chips other than 7411 */               

            default:
                break; /* do nothing */
        }      
        

        /* Mute the output port if it wasnt already muted earlier*/
        /* Note: Cant use SRCCH_Mute 'coz that will affect the other output
        ports too. Cant use MIXER_Mute 'coz that wont work for compressed data. 
        */
        ret =BRAP_OP_GetMute (hRap, pOutputSettings->eOutput, &bMute);
	if( ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_OP_GetMute: eOutputPort(%d) is not supported",
			pOutputSettings->eOutput));
		return BERR_TRACE(ret);
	}
		
        if (bMute == false)
        {
            BRAP_OP_SetMute (hRap, pOutputSettings->eOutput, true);
        }

        /* Program the relevant registers for on-the-fly parameters */
        switch (pOutputSettings->eOutput)
        {
            case BRAP_OutputPort_eSpdif:
                sSpdifSettings.sExtSettings = 
                    pOutputSettings->uOutputPortSettings.sSpdifSettings;
                BRAP_OP_P_SpdifHWConfig (hRap->hFmm[0]->hOp[pOutputSettings->eOutput], 
                    &sSpdifSettings);
#ifndef BCHP_7411_VER /* For chips other than 7411 */                  
                BRAP_P_SetCbit(hRap, pOutputSettings); 
#endif
                break;        
                
#ifndef BCHP_7411_VER /* For chips other than 7411 */   
            case BRAP_OutputPort_eMai:
                sMaiSettings.sExtSettings = 
                    pOutputSettings->uOutputPortSettings.sMaiSettings;
                BRAP_OP_P_MaiHWConfig (hRap->hFmm[0]->hOp[pOutputSettings->eOutput], 
                    &sMaiSettings);
                BRAP_P_SetCbit(hRap, pOutputSettings);                
                break;
                
            case BRAP_OutputPort_eDac0:
#if (BCHP_CHIP == 7400)                                
            case BRAP_OutputPort_eDac1:
#endif                
                sDacSettings.sExtSettings = 
                    pOutputSettings->uOutputPortSettings.sDacSettings;
                BRAP_OP_P_DacHWConfig (hRap->hFmm[0]->hOp[pOutputSettings->eOutput], 
                    &sDacSettings);
                break;
		case BRAP_OutputPort_eI2s0:
		case BRAP_OutputPort_eI2s1:			
		case BRAP_OutputPort_eI2s2:
			sI2sSettings.sExtSettings =pOutputSettings->uOutputPortSettings.sI2sSettings;			
			BRAP_OP_P_I2sHWConfig(hRap->hFmm[0]->hOp[pOutputSettings->eOutput],&sI2sSettings);
			break;			
#endif  /* For chips other than 7411 */          

            default:
                break; /* do nothing */            
        }

        /* Unmute the OP port only if we'd muted it earlier in this function */
        if (bMute == false)
        {
            BRAP_OP_SetMute (hRap, pOutputSettings->eOutput, false);
        }
    }
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    BDBG_MSG(("pOutputSettings.eOutput = %d", pOutputSettings->eOutput));
    BDBG_MSG(("pOutputSettings.bIndpDlyRqd = %d", pOutputSettings->bIndpDlyRqd));
    BDBG_MSG(("pOutputSettings.iDelay = %d", pOutputSettings->iDelay));
#endif
    
	/* Copy the output port settings to the Audio Device Handle */
	hRap->sOutputSettings[pOutputSettings->eOutput] = *pOutputSettings;

	/* Mark the corresponding output valid flag to TRUE */
	hRap->bOpSettingsValid[pOutputSettings->eOutput] = true;

	BDBG_LEAVE(BRAP_SetOutputConfig);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	API used to get the output port configuration.

Description:
	This API is used to get the output configuration of an output port. 
	If the caller has not configured the output port before, this API 
	will return error. 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_SetOutputConfig
****************************************************************************/
BERR_Code BRAP_GetOutputConfig( 
	BRAP_Handle 		hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_OutputPort		eOutput,			/* [in] Output port type */
	BRAP_OutputSettings	*pOutputSettings/* [out] Output port settings*/ 
	)
{
       BERR_Code ret = BERR_SUCCESS;
	BDBG_ENTER(BRAP_GetOutputConfig);
	
	/* Assert the function argument(s) */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pOutputSettings);

	if(eOutput >= BRAP_OutputPort_eMax)
	{
		BDBG_ERR(("BRAP_GetOutputConfig: OutputPort(%d) is not supported",
			eOutput));		
	}

       ret = BRAP_RM_P_IsOutputPortSupported(eOutput);
    	if( ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_GetOutputConfig: OutputPort(%d) is not supported",
    			eOutput));
    		return BERR_TRACE(ret);
    	}
	
	/* Return the output configuration, if it exists, otherwise return error */
	if(hRap->bOpSettingsValid[eOutput] == true)
	{
		*pOutputSettings = hRap->sOutputSettings[eOutput];
	}
	else
	{
		return BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
	}

	BDBG_LEAVE(BRAP_GetOutputConfig);

	return ret;
}

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt.

Description:
    This PI should be used by the application to unmask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_Interrupt. For all other interrupts, application has to use the 
    standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskInterrupt (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_P_UnmaskInterrupt);

    BKNI_EnterCriticalSection();
    ret = BRAP_P_UnmaskInterrupt_isr(hRapCh,	eInterrupt);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_P_UnmaskInterrupt);
    return ret;
}


/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_UnmaskInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskInterrupt_isr (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
)
{
    BERR_Code ret = BERR_SUCCESS;
	uint32_t ui32RegVal = 0, ui32Offset,ui32RegValFMMCrc=0;
    BDBG_ENTER (BRAP_P_UnmaskInterrupt_isr);

    /* Check input parameters */
    BDBG_ASSERT (hRapCh);     

    if((eInterrupt == BRAP_Interrupt_eFmmRbufFreeByte) && 
       (hRapCh->eChannelType != BRAP_P_ChannelType_ePcmPlayback))
    {
        BDBG_ERR(("BRAP_P_UnmaskInterrupt_isr: eFmmRbufFreeByte should be unmasked "       
                  "\n\tonly for a PCM playback channel"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if((eInterrupt == BRAP_Interrupt_eFmmDstRbufFullMark) &&
       (hRapCh->eChannelType != BRAP_P_ChannelType_eCapture))
    {
        BDBG_ERR(("BRAP_P_UnmaskInterrupt_isr: eFmmDstRbufFullMark should be unmasked "       
                  "\n\tonly for a PCM capture channel"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
	ui32RegValFMMCrc=BRAP_Read32_isr(hRapCh->hRegister,
								BCHP_AUD_FMM_OP_ESR_MASK_CLEAR); 
#else
	BSTD_UNUSED(ui32RegValFMMCrc);
#endif

    switch (eInterrupt)
    {
        case BRAP_Interrupt_eBitRateChange:
            ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, BR_CHG, 1);
            break;
        case BRAP_Interrupt_eModeChange:
            ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, MOD_CHG, 1);
            break;
        case BRAP_Interrupt_eCrcError:
            ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CRC_ERR, 1);
            break;
#if BRAP_P_WATERMARK_WORKAROUND == 0			 			
        case BRAP_Interrupt_eFmmDstRbufFullMark:
            switch(hRapCh->sModuleHandles.hDstCh->uiIndex)
            {
                case 0:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, DEST_RINGBUF_0_EXCEED_FULLMARK, 1);
                    break;
                default:
                     BDBG_ERR(("BRAP_P_UnmaskInterrupt_isr: Invalid dstch index %d",
                            hRapCh->sModuleHandles.hDstCh->uiIndex));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
            }/* switch */
            break;
        case BRAP_Interrupt_eFmmRbufFreeByte:
            switch(hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId)
            {
                case 0:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_0_EXCEED_FREEMARK, 1);
                    break;
                case 1:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_1_EXCEED_FREEMARK, 1);
                    break;
                case 2:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_2_EXCEED_FREEMARK, 1);
                    break;
                case 3:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_3_EXCEED_FREEMARK, 1);
                    break;
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
                case 4:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_4_EXCEED_FREEMARK, 1);
                    break;
                case 5:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_5_EXCEED_FREEMARK, 1);
                    break;
#endif
#if ( BCHP_CHIP == 7400 )
                case 6:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_6_EXCEED_FREEMARK, 1);
                    break;
                case 7:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_CLEAR, SOURCE_RINGBUF_7_EXCEED_FREEMARK, 1);
                    break;
#endif /* defined ( BCHP_CHIP == 7400 ) */
                default: 
                    BDBG_ERR(("BRAP_P_UnmaskInterrupt_isr: Invalid srcch index %d",
                            hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                     break;
            }/* switch srcch->index */
            break;
#endif			
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
		case BRAP_Interrupt_eFmmCrc:
		{
			switch(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2)
			{
				case 0:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC0_INT, 1);
				break;
				
				case 1:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC1_INT, 1);					
				break;
				
				case 2:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC2_INT, 1);					
				break;
				
				case 3:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC3_INT, 1);					
				break;
#if (BCHP_CHIP != 7403)
				case 4:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC4_INT, 1);					
				break;
				
				case 5:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC5_INT, 1);					
				break;
				
				case 6:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC6_INT, 1);					
				break;
				
				case 7:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC7_INT, 1);					
				break;
#endif
                default:
			BDBG_ERR(("BRAP_P_UnmaskInterrupt_isr: Invalid iParam2 %d",
						hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                break;
			}
			break;
		}		
#endif
        default: 
             /* Do nothing */
             break;
    }

    if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
    {
#if 0    
        switch (hRapCh->sRsrcGrnt.uiDspContextId)
        {
            case 0:
                ui32Offset = 0;
                break;
            case 1:
                ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                break;
            case 2:
                ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                break;
        }
#else
		ui32Offset = ( hRapCh->sRsrcGrnt.uiDspContextId) * ( BCHP_AUD_DSP_ESR_SO10_INT_STATUS -
						BCHP_AUD_DSP_ESR_SO00_INT_STATUS );
#endif		
       	BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + ui32Offset, ui32RegVal );
    }

    if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
       hRapCh->eChannelType == BRAP_P_ChannelType_eCapture )
    {
        /* Set the bit in ui32FmmBFIntMask */
        hRapCh->ui32FmmBFIntMask |= ui32RegVal;

        /* Unmask the interrupt */
        BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_MASK_CLEAR, ui32RegVal);
    }
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
	BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_FMM_OP_ESR_MASK_CLEAR, ui32RegValFMMCrc);
#endif

    BDBG_LEAVE (BRAP_P_UnmaskInterrupt_isr);
    return ret;
}

/*****************************************************************************
Summary:
    Used to mask a particular interrupt.

Description:
    This PI should be used by the application to mask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_Interrupt. For all other interrupts, application has to use the 
    standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskInterrupt (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_P_MaskInterrupt);

    BKNI_EnterCriticalSection();
    ret = BRAP_P_MaskInterrupt_isr(hRapCh, eInterrupt);
    BKNI_LeaveCriticalSection();
    
    BDBG_LEAVE(BRAP_P_MaskInterrupt);
    return ret;
}


/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_P_UnmaskInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskInterrupt_isr (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
)
{
    BERR_Code ret = BERR_SUCCESS;
	uint32_t ui32RegVal = 0, ui32Offset,ui32RegValFMMCrc=0;
    BDBG_ENTER (BRAP_P_MaskInterrupt_isr);

    /* Check input parameters */
    BDBG_ASSERT (hRapCh);     
    if((eInterrupt == BRAP_Interrupt_eFmmRbufFreeByte) && 
       (hRapCh->eChannelType != BRAP_P_ChannelType_ePcmPlayback))
    {
        BDBG_ERR(("BRAP_P_MaskInterrupt_isr: eFmmRbufFreeByte should be masked "       
                  "\n\tonly for a PCM playback channel"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if((eInterrupt == BRAP_Interrupt_eFmmDstRbufFullMark) &&
       (hRapCh->eChannelType != BRAP_P_ChannelType_eCapture))
    {
        BDBG_ERR(("BRAP_P_MaskInterrupt_isr: eFmmDstRbufFullMark should be masked "       
                  "\n\tonly for a PCM capture channel"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
		ui32RegValFMMCrc=BRAP_Read32_isr(hRapCh->hRegister,
									BCHP_AUD_FMM_OP_ESR_MASK_SET); 
#else
	BSTD_UNUSED(ui32RegValFMMCrc);
#endif

    switch (eInterrupt)
    {
        case BRAP_Interrupt_eBitRateChange:
             ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_SET, BR_CHG, 1);
             break;
        case BRAP_Interrupt_eModeChange:
             ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_SET, MOD_CHG, 1);
             break;
        case BRAP_Interrupt_eCrcError:
             ui32RegVal |= BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_SET, CRC_ERR, 1);
             break;
#if BRAP_P_WATERMARK_WORKAROUND == 0			 
        case BRAP_Interrupt_eFmmDstRbufFullMark:
            switch(hRapCh->sModuleHandles.hDstCh->uiIndex)
            {
                case 0:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, DEST_RINGBUF_0_EXCEED_FULLMARK, 1);
                    break;
                default:
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
            }/* switch */
            break;
        case BRAP_Interrupt_eFmmRbufFreeByte:
            switch(hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId)
            {
                case 0:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_0_EXCEED_FREEMARK, 1);
                    break;
                case 1:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_1_EXCEED_FREEMARK, 1);
                    break;
                case 2:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_2_EXCEED_FREEMARK, 1);
                    break;
                case 3:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_3_EXCEED_FREEMARK, 1);
                    break;
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
                case 4:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_4_EXCEED_FREEMARK, 1);
                    break;
                case 5:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_5_EXCEED_FREEMARK, 1);
                    break;
#endif
#if ( BCHP_CHIP == 7400 )
                case 6:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_6_EXCEED_FREEMARK, 1);
                    break;
                case 7:
                    ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_BF_ESR0_MASK_SET, SOURCE_RINGBUF_7_EXCEED_FREEMARK, 1);
                    break;
#endif /* defined ( BCHP_CHIP == 7400 ) */
                default: 
                    BDBG_ERR(("BRAP_P_MaskInterrupt_isr: Invalid srcch index %d",
                            hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                     break;
            }/* switch srcch->index */
            break;
#endif			
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
		case BRAP_Interrupt_eFmmCrc:
		{
			switch(hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2)
			{
				case 0:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC0_INT, 1);
				break;
				
				case 1:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC1_INT, 1);					
				break;
				
				case 2:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC2_INT, 1);					
				break;
				
				case 3:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC3_INT, 1);					
				break;

#if (BCHP_CHIP != 7403)				
				case 4:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC4_INT, 1);					
				break;
				
				case 5:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC5_INT, 1);					
				break;
				
				case 6:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC6_INT, 1);					
				break;
				
				case 7:
					ui32RegValFMMCrc |= BCHP_FIELD_DATA (AUD_FMM_OP_ESR_MASK_SET, CRC7_INT, 1);					
				break;
#endif
                default:
      			BDBG_ERR(("BRAP_P_MaskInterrupt_isr: Invalid iParam2 %d",
						hRapCh->sAppIntCbInfo[BRAP_Interrupt_eFmmCrc].iParm2));
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                break;

			}
		}		
		break;
#endif
        default: 
             /* Do nothing */
             break;
    }

    if(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
    {
#if 0    
        switch (hRapCh->sRsrcGrnt.uiDspContextId)
        {
            case 0:
                ui32Offset = 0;
                break;
            case 1:
                ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                break;
            case 2:
                ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
                break;
        }
#else
		ui32Offset = ( hRapCh->sRsrcGrnt.uiDspContextId) * ( BCHP_AUD_DSP_ESR_SO10_INT_STATUS -
						BCHP_AUD_DSP_ESR_SO00_INT_STATUS );
#endif		
       	BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_SET + ui32Offset, ui32RegVal );
    }

    if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
       hRapCh->eChannelType == BRAP_P_ChannelType_eCapture)
    {
        /* Clear the bit in ui32FmmBFIntMask */
        hRapCh->ui32FmmBFIntMask &= ~ui32RegVal;

        /* Mask the interrupt */
        BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_FMM_BF_ESR1_MASK_SET, ui32RegVal);
    }
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
	BRAP_Write32_isr(hRapCh->hRegister, BCHP_AUD_FMM_OP_ESR_MASK_SET, ui32RegValFMMCrc);
#endif

    BDBG_LEAVE (BRAP_P_MaskInterrupt_isr);
    return ret;
}

/*****************************************************************************
Summary:
    Used to install an application callback.

Description:
    This PI should be used by the application to install an application specified 
    callback in addition to the RAptor PI interrupt handler for that interrupt.
    The speficied Application callback gets called from the interrupt handler that the Raptor PI 
    already has installed. 
    This can be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_Interrupt. For all other interrupts, application has to use the standard INT interface.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_InstallAppInterruptCb (
	BRAP_ChannelHandle hRapCh,	   /* [in] The RAP channel handle */
    BRAP_Interrupt     eInterrupt, /* [in] The interrupt that needs to 
                                      be activated */
    BRAP_CallbackFunc  pfAppInterruptCb, /* [in] The RAP callback function 
                                      to be registered for eInterrupt*/ 
    void               *pParm1,    /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    int                iParm2      /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t ui32IntMask = 0;
	BDBG_ENTER (BRAP_InstallAppInterruptCb);

	/* Check input parameters */
	BDBG_ASSERT (hRapCh);     

#ifndef BRAP_SUPPORT_TSM_LOG
	if (eInterrupt==BRAP_Interrupt_eTsmLog)
	{
		BDBG_ERR(("To support TSM Log, enable it at compile time by defining flag BRAP_SUPPORT_TSM_LOG"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
#endif

	hRapCh->sAppIntCbInfo[eInterrupt].pfAppCb = pfAppInterruptCb;
	hRapCh->sAppIntCbInfo[eInterrupt].pParm1  = pParm1;
	hRapCh->sAppIntCbInfo[eInterrupt].iParm2  = iParm2;

	BDBG_MSG(("BRAP_InstallAppInterruptCb: pfAppCb=%d", hRapCh->sAppIntCbInfo[eInterrupt].pfAppCb));
	BDBG_MSG(("BRAP_InstallAppInterruptCb: pParm1=%d", hRapCh->sAppIntCbInfo[eInterrupt].pParm1));
	BDBG_MSG(("BRAP_InstallAppInterruptCb: iParm2=%d", hRapCh->sAppIntCbInfo[eInterrupt].iParm2));

	if (hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
		hRapCh->eChannelType == BRAP_P_ChannelType_eCapture ||
		(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode &&
		hRapCh->bStarted == true))
	{
		ret = BRAP_P_UnmaskInterrupt(hRapCh, eInterrupt);
		BDBG_MSG(("BRAP_InstallAppInterruptCb: Unmasked Interrupt [%d]",eInterrupt));
	}

	if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
		hRapCh->eChannelType == BRAP_P_ChannelType_eCapture)
	{
		ui32IntMask = (BCHP_FIELD_DATA (AIO_INTH_R5F_MASK_SET, FMM_BF, 1));
		hRapCh->ui32FmmIntMask |= ui32IntMask;
	}        

	BDBG_LEAVE (BRAP_InstallAppInterruptCb);
	return ret;
}

/*****************************************************************************
Summary:
    Used to remove the application callback.

Description:
    This PI should be used by the application to remove an application specified 
    callback that has been install in addition to the RAptor PI interrupt handler 
    for that interrupt. 

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_RemoveAppInterruptCb (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t ui32IntMask = 0;
	BDBG_ENTER (BRAP_RemoveAppInterruptCb);

	/* Check input parameters */
	BDBG_ASSERT (hRapCh);     

	hRapCh->sAppIntCbInfo[eInterrupt].pfAppCb = NULL;
	hRapCh->sAppIntCbInfo[eInterrupt].pParm1  = NULL;
	hRapCh->sAppIntCbInfo[eInterrupt].iParm2  = 0;

	BDBG_MSG(("BRAP_RemoveAppInterruptCb: pfAppCb=%d", hRapCh->sAppIntCbInfo[eInterrupt].pfAppCb));
	BDBG_MSG(("BRAP_RemoveAppInterruptCb: pParm1=%d", hRapCh->sAppIntCbInfo[eInterrupt].pParm1));
	BDBG_MSG(("BRAP_RemoveAppInterruptCb: iParm2=%d", hRapCh->sAppIntCbInfo[eInterrupt].iParm2));

	if (hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
		hRapCh->eChannelType == BRAP_P_ChannelType_eCapture ||
		(hRapCh->eChannelType == BRAP_P_ChannelType_eDecode &&
		hRapCh->bStarted == true))
	{
		ret = BRAP_P_MaskInterrupt(hRapCh, eInterrupt);
		BDBG_MSG(("BRAP_RemoveAppInterruptCb: Unmasked Interrupt [%d]",eInterrupt));		
	}

	if(hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback ||
		hRapCh->eChannelType == BRAP_P_ChannelType_eCapture)
	{
		BDBG_MSG(("Remove App Interrupt Cb: ui32IntMask = 0x%x, FmmIntMask = 0x%x", ui32IntMask, hRapCh->ui32FmmIntMask));
		ui32IntMask = ~(BCHP_FIELD_DATA (AIO_INTH_R5F_MASK_CLEAR, FMM_BF, 1));
		hRapCh->ui32FmmIntMask &= ui32IntMask;
	}
    
	BDBG_LEAVE (BRAP_RemoveAppInterruptCb);
	return ret;
}

/*****************************************************************************
Summary:
	Install application callback function for device level interrupts
	
Description:
	This API is used to install application callback function for device
	level Raptor interrupts. Application callback function is called when
	these interrupts are triggered.

Returns:
    BERR_SUCCESS on success
     Error code on failure
******************************************************************************/
BERR_Code BRAP_InstallDeviceLevelAppInterruptCb (
	BRAP_Handle hRap,	   /* [in] The RAP handle */
    BRAP_DeviceLevelInterrupt     eInterrupt, /* [in] The device level interrupt
										that needs to be activated */
    BRAP_CallbackFunc  pfAppInterruptCb, /* [in] The RAP callback function 
                                      to be registered for eInterrupt*/ 
    void               *pParm1,    /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    int                iParm2      /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
)
{
	BERR_Code ret;
	uint32_t regVal;

#ifndef BCHP_7411_VER
	BSTD_UNUSED(regVal);
#endif

	BDBG_ENTER(BRAP_InstallDeviceLevelAppInterruptCb);

	/* Assert on bad input parameters */
	BDBG_ASSERT(hRap);

	switch (eInterrupt) {
		case BRAP_DeviceLevelInterrupt_eWatchdog:
			ret = BRAP_DSP_P_EnableDspWatchdogTimer(hRap->hDsp[0],true);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
#ifdef BCHP_7411_VER
			regVal = 0;
			regVal = regVal | BCHP_MASK(AUD_DSP_INTH0_PCI_MASK_SET, WDOG_TIMER_ATTN);
			BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_INTH0_PCI_MASK_SET, regVal);
#else
			ret = BINT_EnableCallback (hRap->hCallback);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
#endif
			break;
		default:
			BDBG_ERR(("Interrupt type %d is not a device level interrupt",eInterrupt));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hRap->sAppIntCbInfo[eInterrupt].pfAppCb = pfAppInterruptCb;
	hRap->sAppIntCbInfo[eInterrupt].pParm1  = pParm1;
	hRap->sAppIntCbInfo[eInterrupt].iParm2  = iParm2;
    
	BDBG_MSG(("BRAP_InstallDeviceLevelAppInterruptCb: pfAppCb=%d", hRap->sAppIntCbInfo[eInterrupt].pfAppCb));
    BDBG_MSG(("BRAP_InstallDeviceLevelAppInterruptCb: pParm1=%d", hRap->sAppIntCbInfo[eInterrupt].pParm1));
    BDBG_MSG(("BRAP_InstallDeviceLevelAppInterruptCb: iParm2=%d", hRap->sAppIntCbInfo[eInterrupt].iParm2));

	BDBG_LEAVE(BRAP_InstallDeviceLevelAppInterruptCb);
	return ret;
}

/*****************************************************************************
Summary:
	Remove application callback function for device level interrupts

Description:
	This API is used to remove application callback function for device
	level Raptor interrupts. 

Returns:
    BERR_SUCCESS on success
     Error code on failure
******************************************************************************/
BERR_Code BRAP_RemoveDeviceLevelAppInterruptCb (
	BRAP_Handle hRap,	/* [in] The RAP channel handle */
    BRAP_DeviceLevelInterrupt  eInterrupt  /* [in] The device level interrupt 
								that needs to be removed */
)
{
	BERR_Code ret;
	uint32_t regVal;

#ifndef BCHP_7411_VER
	BSTD_UNUSED(regVal);
#endif

	BDBG_ENTER(BRAP_RemoveDeviceLevelAppInterruptCb);

	/* Assert on bad input parameters */
	BDBG_ASSERT(hRap);

		switch (eInterrupt) {
		case BRAP_DeviceLevelInterrupt_eWatchdog:
			ret = BRAP_DSP_P_EnableDspWatchdogTimer(hRap->hDsp[0],false);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
#ifdef BCHP_7411_VER
			regVal = 0;
			regVal = regVal | BCHP_MASK(AUD_DSP_INTH0_PCI_MASK_CLEAR, WDOG_TIMER_ATTN);
			BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_INTH0_PCI_MASK_CLEAR, regVal);
#else
			ret = BINT_DisableCallback (hRap->hCallback);
			if (ret!=BERR_SUCCESS)
				return BERR_TRACE(ret);
#endif
			break;
		default:
			BDBG_ERR(("Interrupt type %d not supported",eInterrupt));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hRap->sAppIntCbInfo[eInterrupt].pfAppCb = NULL;
	hRap->sAppIntCbInfo[eInterrupt].pParm1  = NULL;
	hRap->sAppIntCbInfo[eInterrupt].iParm2  = 0;

    BDBG_MSG(("BRAP_RemoveAppInterruptCb: pfAppCb=%d", hRap->sAppIntCbInfo[eInterrupt].pfAppCb));
    BDBG_MSG(("BRAP_RemoveAppInterruptCb: pParm1=%d", hRap->sAppIntCbInfo[eInterrupt].pParm1));
    BDBG_MSG(("BRAP_RemoveAppInterruptCb: iParm2=%d", hRap->sAppIntCbInfo[eInterrupt].iParm2));    

	BDBG_LEAVE(BRAP_RemoveDeviceLevelAppInterruptCb);
	return ret;
}

/***************************************************************************
Summary:
	API returns revision information.

Description:
	Gets the revision information of all the firmwares executing on Raptor 
	DSP and FMM hardware as well as the revision information of the Raptor 
	Audio PI.
Returns:
	BERR_SUCCESS 

See Also:
	None
	
****************************************************************************/
BERR_Code BRAP_GetRevision ( 	
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
	BRAP_RevisionInfo		*pRevInfo	/* [out]Version infomation of Raptor 
												Audio PI, the microcode 
												running on Raptor DSP and 
												Raptor FMM */ 
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiCount = 0;
	
	BDBG_ENTER (BRAP_GetRevision);	
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pRevInfo);

	/* Initialize the structure */
	pRevInfo->ui32ChipId = 0;
	for ( uiCount = 0; uiCount < BRAP_MAX_DSP_DECODE_ALGO; uiCount++ )
	{
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32FwVersionField1 = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32FwVersionField2 = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32CustomerID = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32Date = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32MajorVersion = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32MinorVersion = 0;
		pRevInfo->sDSPVersion.sDecAlgoVersion[uiCount].ui32Year= 0;
	}
	for ( uiCount = 0; uiCount < BRAP_MAX_DSP_FS_ALGO; uiCount++ )
	{
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32FwVersionField1 = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32FwVersionField2 = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32CustomerID = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32Date = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32MajorVersion = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32MinorVersion = 0;
		pRevInfo->sDSPVersion.sDecFsVersion[uiCount].ui32Year= 0;
	}
	for ( uiCount = 0; uiCount < BRAP_MAX_DSP_PP_ALGO; uiCount++ )
	{
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32FwVersionField1 = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32FwVersionField2 = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32CustomerID = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32Date = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32MajorVersion = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32MinorVersion = 0;
		pRevInfo->sDSPVersion.sPPAlgoVersion[uiCount].ui32Year= 0;
	}

	BRAP_DSP_P_GetFwVersionInfo (hRap->hDsp[0], &pRevInfo->sDSPVersion);

	/* Assign PI version info */
	pRevInfo->sPIVersion.ui32Date = BRAP_PRIV_PI_VERSION_DATE; /* Includes month and date */
	pRevInfo->sPIVersion.ui32MajorVersion = BRAP_PRIV_PI_VERSION_MAJOR;
	pRevInfo->sPIVersion.ui32MinorVersion = BRAP_PRIV_PI_VERSION_MINOR;
	pRevInfo->sPIVersion.ui32Year = BRAP_PRIV_PI_VERSION_YEAR;

	pRevInfo->ui32ChipId = BRAP_PRIV_PI_CHIP_ID;
	pRevInfo->sPIVersion.pCustomerID = BRAP_PRIV_PI_VERSION_CUSTOMER_ID;

	BDBG_MSG(("Chip ID=%x",pRevInfo->ui32ChipId));
	BDBG_MSG(("Customer ID=%s",pRevInfo->sPIVersion.pCustomerID));
	BDBG_MSG(("PI MajorVersion=%x",pRevInfo->sPIVersion.ui32MajorVersion));
	BDBG_MSG(("PI MinorVersion=%x",pRevInfo->sPIVersion.ui32MinorVersion));
	BDBG_MSG(("PI Month and Date=%x",pRevInfo->sPIVersion.ui32Date));
	BDBG_MSG(("PI Year=%x",pRevInfo->sPIVersion.ui32Year));
	
	BDBG_LEAVE (BRAP_GetRevision);
    
	return ret;

}

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/***************************************************************************
Summary:
	API returns the buffer requirements for CDB and ITB.

Description:
	This API returns the size required for the CDB and ITB buffers
	based upon the algorithm type to be used.
	
Returns:
	BERR_SUCCESS 

See Also:
	None
	
****************************************************************************/
BERR_Code BRAP_GetBufferConfig ( 	
	BRAP_Handle 		hRap,				/* [in] The RAP device handle */
	BRAP_BufferCfgMode	eCfgMode,			/* [in] CDB/ITB Configuration Mode */
	BAVC_CdbItbConfig	*pCdbItbConfigInfo	/* [out] Buffer information of CDB and ITB*/ 
	)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER (BRAP_GetBufferConfig);

    	/* Check input parameters */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pCdbItbConfigInfo);
	BDBG_ASSERT(eCfgMode < BRAP_BufferCfgMode_eMaxMode);
	BSTD_UNUSED(hRap);

	/* Get the CDB & ITB info corresponding to the buffer config mode */
	if(eCfgMode < BRAP_BufferCfgMode_eMaxMode)
	{
	*pCdbItbConfigInfo = sCdbItbCfg[eCfgMode];
	}
	else
	{
		BDBG_ERR(("BRAP_GetBufferConfig - INVALID CONFIG MODE = %d\n",eCfgMode));
		ret= BERR_INVALID_PARAMETER;
	}

   	return ret;

}
#endif


#ifndef BCHP_7411_VER
BERR_Code BRAP_AddOutputPort (
    BRAP_ChannelHandle hRapCh,         /* [in] The Raptor channel handle */

    bool                          bSimulPt,       /* [in] TRUE: Reserve this 
    port for the PassThru context of Simul Mode => This port will be used 
    ONLY if the channel is started in Simul Mode. It will be used for the 
    compressed data ie Pass Thru context of the Simul mode. This port will NOT 
    be used at all if the channel is started in Decode/Passthrough mode.

    FALSE: This port will carry PCM data if the channel is started in Decode
    /Simul mode or compressed data if the channel is started in PassThru mode 

    This field is applicable only to decode channels. Set to False for Playback
    and Capture channels */    
    
    BRAP_OutputChannelPair eChanPair,    /* [in]  The channel pair -LR/LsRs/C -
    data to send to the new outport. This field is ignored if bSimulPt is TRUE
    This field is applicable only to decode channels. Set to 
    BRAP_OutputChannelPair_eLR for Playback and Capture channels*/    
                                                                              
    BRAP_OutputPort                eNewOpPort   /* [in] New output port */
)
{

    /* Algorithm:
            1. Check whether we can add this port or not 
            2. Figure out which resources need to be requested and get them.
                This will depend on whether the port is a master, master simul 
                or cloned port and whether or not it requires indep delay 
            3. Save the allocated resources correctly in the channel handle
            4. Open the new resources that were just allocated with the correct settings
            5. Save the new resource handles correctly in the channel handle             */


    BERR_Code ret = BERR_SUCCESS;
    BRAP_Handle                   hRap;
    BRAP_RM_P_ResrcReq      sResrcReq;
    BRAP_RM_P_ResrcGrant    sResrcGrant,sTempResrcGrant;
    BRAP_P_ObjectHandles      sTempHandles;
    BRAP_P_ObjectHandles *   pTemp=NULL;
    
    BRAP_RBUF_P_Settings      sRbufSettings[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
    BRAP_SRCCH_P_Settings   sSrcChSettings;
    BRAP_MIXER_P_Settings    sMixerSettings;
    BRAP_SPDIFFM_P_Settings sSpdifFmSettings;
    BRAP_OP_P_SpdifSettings  sSpdifSettings;    
    BRAP_OP_P_MaiSettings     sMaiSettings;
    BRAP_OP_P_I2sSettings     sI2sSettings;
    BRAP_OP_P_FlexSettings    sFlexSettings;
    BRAP_OP_P_DacSettings    sDacSettings;
    void * pNewOpSettings = NULL;    

    BRAP_OutputPort eMasterPort = BRAP_OutputPort_eMax; /* If the new port is a clone, eMaster will keep track of its Master */
    BRAP_OutputPort eOutput = BRAP_OutputPort_eMax; /* If the new port is Mai, this will hold the Mai Mux Select, else it will hold eNewOpPort */

    int i,j;
    bool bUsed = false;
    bool bNewSimul = false; /* TRUE: this is the first time we're requesting
    resources for the PT context for simul mode */
    bool bNewMaster = false; /* TRUE: this is the first time we're requesting 
    resources for this channel pair */ 
    bool bClone = true;    /* TRUE: this is a cloned port (compressed or PCM)*/

    unsigned int	uiNewDpId;
    unsigned int	uiOrigDpId;

    BDBG_ENTER (BRAP_AddOutputPort);
    
    BDBG_ASSERT (hRapCh);

    /***** Start checks: Need to check a plethora of things before allowing addition of a port */

    /* Make sure the channel has been stopped. 
    We cant add/remove an output port while the channel is running */
    if (hRapCh->bStarted == true)
    {
        BDBG_ERR(("BRAP_AddOutputPort: We cant add/remove an output port while"
            "the channel is running. Please stop it first"));
        return BERR_TRACE(ret);        
    }


    BDBG_MSG(("BRAP_AddOutputPort: hRapCh=0x%x, hRapCh->uiChannelNo=%d", hRapCh,hRapCh->uiChannelNo));

    ret = BRAP_RM_P_IsOutputPortSupported(eNewOpPort);
    if( ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_AddOutputPort: invalid output port %d", eNewOpPort));
        return BERR_TRACE(ret);
    }

    /* Make sure the temp handles etc are clean */
    BKNI_Memset (&sResrcReq, 0, sizeof(sResrcReq));
    BKNI_Memset (&sResrcGrant, 0, sizeof(sResrcGrant));
    BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));

    sResrcGrant.uiNumOpPorts = 1;
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {    
        sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
        sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
    }
    sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
    for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
		sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 
       
    hRap = hRapCh->hRap;        

    BDBG_MSG (("BRAP_AddOutputPort: New port  = %d", eNewOpPort));

    if (hRapCh->eChannelType != BRAP_P_ChannelType_eDecode)
    {
            BDBG_MSG(("BRAP_AddOutputPort: This is not a decode channel."
                "bSimulPt will be ignored. eChanPair is internally forced to"
                "BRAP_OutputChannelPair_eLR"));
    }        

#if ((BCHP_CHIP != 7400) && (BRAP_AD_SUPPORTED == 0))
    /* At present, we support only 1 channel pair BRAP_OutputChannelPair_eLR,
         so return an error if the user selects anything else. Remove this restriction
         later when multuple pairs are supported*/
    if (eChanPair > BRAP_OutputChannelPair_eLR)
    {
        BDBG_ERR(("BRAP_AddOutputPort: Channel pair %d not supported", 
            eChanPair));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif

    /* If this playback/capture channel, it has only one channel pair. so force
    eChanPair = BRAP_OutputChannelPair_eLR.
    Also if bSimulPt == true => this is for the PT context of Simul mode, and 
    so would also have only one channel pair. therefore force 
    eChanPair = BRAP_OutputChannelPair_eLR */
    if (((hRapCh->eChannelType != BRAP_P_ChannelType_eDecode) 
            || (bSimulPt == true))
        && (eChanPair != BRAP_OutputChannelPair_eLR))
    {
        BDBG_ERR(("BRAP_AddOutputPort: Channel pair %d is not supported in" 
            "PB/CAP channels and in Simul mode. Please select channel pair as"
            "BRAP_OutputChannelPair_eLR", 
            eChanPair));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#ifndef RAP_I2S_COMPRESS_SUPPORT
    if (bSimulPt == true)
    {
        if ((eNewOpPort != BRAP_OutputPort_eSpdif) 
            && (eNewOpPort != BRAP_OutputPort_eMai) 
            && (eNewOpPort != BRAP_OutputPort_eFlex))
        {
                BDBG_ERR(("BRAP_AddOutputPort: Output port %d cannot carry"
                    "compressed data.", eNewOpPort));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }   
#endif
    
    /* Make sure new port is configured */ 
    if(hRapCh->hRap->bOpSettingsValid[eNewOpPort] == false) 
    {
        BDBG_ERR(("BRAP_AddOutputPort: New Output port %d is not configured."
            "Please configure before calling this PI.", eNewOpPort));
        return BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
    }   


    /* Return an error if the new Output port is already in use by this channel */
    ret = BRAP_P_IsOpPortUsedbyChannel (hRapCh, eNewOpPort, &bUsed, NULL /*bClone - ignored*/);
    if ( bUsed == true)
    {
        BDBG_ERR (("BRAP_AddOutputPort: Requested output port %d is already"
            "reserved for this channel. First free it.", eNewOpPort));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
     }


    /* Return an error if this is a decode channel AND the new port is already
    associated with a decode channel. We shouldnt be mixing 2 decode channels*/
    if (hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
    {
    	for(i=0; i<BRAP_RM_P_MAX_DEC_CHANNELS; i++)
    	{
            if (hRap->hRapDecCh[i] != NULL)
            {
                /* Return an error if the new Output port is 
                already in use by any other DEC channel */
                ret = BRAP_P_IsOpPortUsedbyChannel (hRap->hRapDecCh[i], 
                    eNewOpPort, &bUsed, NULL /* bClone - ignoring */);
                if ( bUsed == true)
                {
#if (BRAP_AD_SUPPORTED == 1)                
                	if((hRap->hRapDecCh[i] != hRap->sAudioDescObjs[0].sExtAudioDescDetails.hPrimaryChannel)
						|| (hRapCh != hRap->sAudioDescObjs[0].sExtAudioDescDetails.hDescriptorChannel))
					{						
	                    BDBG_ERR (("BRAP_AddOutputPort: Channels mismatch cannot mix these channels"));
	                    return BERR_TRACE(BERR_INVALID_PARAMETER);
					}
					else if((hRap->hRapDecCh[i]->bStarted == true)||(hRapCh->bStarted == true))
					{
	                    BDBG_ERR (("BRAP_AddOutputPort: Channel must be in stopped state before mixing"));
	                    return BERR_TRACE(BERR_INVALID_PARAMETER);
					}
#else
                    BDBG_ERR (("BRAP_AddOutputPort: Requested output"
                        "port %d is already in cloned by this channel for "
                        "channel pair %d. Cant clone.", eNewOpPort, i));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
#endif						
                }
            }
        }

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
        if ((hRapCh->bLargeRbuf == false) && (hRap->sOutputSettings[eNewOpPort].bIndpDlyRqd == true))
        {
            BDBG_ERR(("hRapCh->bLargeRbuf == false. Channel has to be opened with bLargeRbuf=true ",
                "for this port to have an indepedent delay. Please close and reopen the channel with bLargeRbuf=true"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);            
        }
#endif
    } 
    /***** End checks: Need to check a plethora of things before allowing addition of a port */    


    if (eNewOpPort != BRAP_OutputPort_eMai)
    {
           eOutput = eNewOpPort;
    }
    else
    {
           eOutput = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }           

    /*Ask resource manager to allocate resources related to the new output port
    For capture channel, we will be duplicating only the OP path, 
    not the capture path. So we never request new DstCh etc    */

       
    sResrcReq.bAllocateDstCh = false; 
    sResrcReq.bSimulModePt = false; 
    sResrcReq.bAllocateDsp = false;
    sResrcReq.bAllocateDspCtx = false;
    sResrcReq.bLastChMono = false; 
    sResrcReq.bAllocateSrcCh = true;
    sResrcReq.bAllocatePpm = false;
    sResrcReq.bAllocateRBuf = true;

    /* Find out whether this is a master or a clone */    
    if (hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
    {
        /* If this new port has to carry the compressed data in a Simul mode, 
        look in sSimulPtModuleHandles, else check sModulesHandles */    
        if (bSimulPt == true)
        {
            if (hRapCh->sSimulPtModuleHandles.hOp[eChanPair] == NULL)
            {
                /* This is the first time resources are being allocated for the 
                      PT context of simul mode=> all resources for SimulModePT
                      context have to be allocated. including DSP context. */            
                sResrcReq.bSimulModePt = true; 
                sResrcReq.bAllocateDsp = false;  
 		  sResrcReq.bAllocateDspCtx = false;
                bNewSimul = true;
                bClone = false;
                bNewMaster = false;
                pTemp = &(hRapCh->sSimulPtModuleHandles);    
                BDBG_MSG(("BRAP_AddOutputPort(): New port and other resources" 
                    "requested for PT context of Simul mode."));
            }        
        }
        else
        {
            if (hRapCh->sModuleHandles.hOp[eChanPair] == NULL)
            {
                /* => this is a Master port */
                bNewMaster = true;
                bNewSimul = false;
                bClone = false;                
                pTemp = &(hRapCh->sModuleHandles);                
                BDBG_MSG(("BRAP_AddOutputPort(): New port ie master requested"
                    " for channel pair %d.", eChanPair));                                      
            }
        }
    }
    else
    {    /* For a  PB or CAP channel, check if this is a Master or clone */
            if (hRapCh->sModuleHandles.hOp[eChanPair] == NULL)
            {
                /* => this is a Master port */
                bNewMaster = true;
                bNewSimul = false;
                bClone = false;                
                pTemp = &(hRapCh->sModuleHandles);                
                BDBG_MSG(("BRAP_AddOutputPort(): New port ie master requested"
                    " for channel pair %d.", eChanPair));                                           
            }
    }
    
    BDBG_MSG(("bNewMaster=%d, bNewSimul=%d, bClone=%d, eChanPair=%d",
        bNewMaster, bNewSimul, bClone, eChanPair));

    /* If this is a cloned port, find its Master port */
    if (bClone==true)
    {   
        if (bSimulPt==false)
        {
            eMasterPort = hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort;
        }
        else
        {
            eMasterPort = hRapCh->sSimulPtModuleHandles.hOp[eChanPair]->eOutputPort;
        }            
    }

    /* Check if we need new RBUF/SrcCh to be allocated. 

        Allocate master rbuf if (memory is allocated along with master RBUF)
        1. this is a new master 
        2. this is a new SimulPt port

        Allocate shared RBUF/SrcCh  for cloned ports if 
        3.we need to support indep output delay for this port
        4.the master for this clone has an indep output delay.
        5.there is presently no other SrcCh in this DP, or in the other DP 
            which carries the data without delay
        (Note: shared RBUF implies  that no new memory is allocated. 
        the memory associated with the master port is reused) 

        Else
        6.no new Rbuf/SrcCh           
        
        Case 1 - 4 are pretty simple and easy to detect. But since we've initialised 
                sResrcReq.bAllocateSrcCh = true;
                sResrcReq.bAllocateRBuf = true;
        we dont need to bother about them .   

    if ((bNewMaster == true) //case 1 
        || (bNewSimul == true) // case 2 
        || ((bClone==true) && (hRap->sOutputSettings[eNewOpPort].bIndpDlyRqd == true)) // case 3 
        || ((bClone==true) && (hRap->sOutputSettings[eMasterPort].bIndpDlyRqd == true)) // case 4 
        {
        } 

    But to differentitate between case 5 and 6 we need the following code. 
    */

     if ((bClone == true) 
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)        
        && (hRap->sOutputSettings[eNewOpPort].bIndpDlyRqd== false)
#endif        
        )
    {
        /* This is a  clone which doesnt require indep delay. 
        See if there's any other SrcCh/RBUF which can used, else request new ones.
        First check if the SrcCh/Rbuf of the master port can be reused. If not, 
        check the SrcCh/RBUF of the other clones */
        
        /* Get the DP id for the new port */
        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                	eOutput, &uiNewDpId);
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_AddOutputPort(): Cant get new DP ID"));
            return BERR_TRACE (ret);
        }   

        /* get  DP Id  of master port */


        /* if the master port is Mai, store the Mai Mux select instead */
        if (eMasterPort == BRAP_OutputPort_eMai)
        {          
        	 ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                    	hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                    	&uiOrigDpId);
               
        }
        else
        {
        	 ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                    	eMasterPort, &uiOrigDpId);
        }
        
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_AddOutputPort(): Cant get master port's DP ID"));
            return BERR_TRACE (ret);
        }  
      
        /* If newDp=Master DP, we can resuse the SrcCh/RBUF -
            For a decode channel make sure the master didnt have a delay. if it did, we'll need new SrcCh/RBuf */
        if ((uiNewDpId == uiOrigDpId) 
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)            
            && !((hRapCh->eChannelType == BRAP_P_ChannelType_eDecode) && (hRap->sOutputSettings[eMasterPort].bIndpDlyRqd== true))
#endif            
            )
        {
            sResrcReq.bAllocateSrcCh = false; /* dont need a new SrcCh */
            sResrcReq.bAllocatePpm = false;
            sResrcReq.bAllocateRBuf = false; /* dont need new RBufs*/
            BDBG_MSG(("Dont need new RBUF/SRCCH"));
            
        }
        else 
        {
            /* See if there's any other clone in the new DP whose SrcCh we can use */
            for(i = 0; i < BRAP_RM_P_MAX_OUTPUTS; i++)
            {
                if (hRapCh->sCloneOpPathHandles[eChanPair][i].hOp != NULL)
                {
                    if ( i != BRAP_OutputPort_eMai )
                    {
                    		ret=BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                            	(BRAP_OutputPort)i, 
                            	&uiOrigDpId);
                   	}
		        else
		        {
                		ret=BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                        	 hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                        	&uiOrigDpId);
		        }
                if (ret != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_AddOutputPort(): Cant get orig port's DP ID"));
                    return BERR_TRACE (ret);
                }                
                        /* If there is a clone in the new DP, whose SrcCh/RBUF we can link to, use them.              
                        For a decode channel make sure the clone didnt have a delay. if it did, we'll need new SrcCh/RBuf */
                       if ((uiNewDpId == uiOrigDpId) 
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)                        
                                && !((hRapCh->eChannelType == BRAP_P_ChannelType_eDecode) && (hRap->sOutputSettings[i].bIndpDlyRqd== true))
#endif
                        )
                       {
                            sResrcReq.bAllocateSrcCh = false;   /* dont need a new SrcCh */
                            sResrcReq.bAllocatePpm = false;
                            sResrcReq.bAllocateRBuf = false;     /* Dont need new RBUFs */                          
                            break;
                       }
                }               
            }
        }
     }
    else if(hRapCh->eChannelType != BRAP_P_ChannelType_eDecode)
    {
		sResrcReq.bAllocateSrcCh = false; /* No need to allocate Source channel, as already allocated in channel open */
		sResrcReq.bAllocateRBuf = false; /* No need to allocate Ringbuffer, as already allocated in channel open */
    }    

      
    /* Check how many ring buffers are needed (if any) for cloned port */
    if ((hRapCh->eChannelType != BRAP_P_ChannelType_eDecode) 
        && (hRapCh->eMaxBufDataMode != BRAP_BufDataMode_eStereoNoninterleaved))
    {
        sResrcReq.bLastChMono = true;
    }
    else
    {
        sResrcReq.bLastChMono = false;     
    }

    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
    }

    sResrcReq.eChannelType 	= hRapCh->eChannelType;
    sResrcReq.uiNumOpPorts = 1; /* Only one port required */
    sResrcReq.sOpPortReq[eChanPair].eOpPortType = eNewOpPort;

    /* If output port is MAI then initialize the MAI mux selector */            
    if (eNewOpPort == BRAP_OutputPort_eMai )
    {
        sResrcReq.sOpPortReq[eChanPair].eMuxSelect = eOutput;
        BDBG_MSG(("Mai Mux select is %d", sResrcReq.sOpPortReq[eChanPair].eMuxSelect));            
    }
  
    /* Call resource manager to allocate required resources. */
    ret = BRAP_RM_P_AllocateResources (hRap->hRm, 
                                        &sResrcReq, 
                                        &sResrcGrant);
    if(ret != BERR_SUCCESS) return BERR_TRACE(ret);

    /* save this info in the channel handle */
    if (bClone == true)
    {
        hRapCh->sCloneOpPathResc[eChanPair][eNewOpPort] 
                = sResrcGrant.sOpResrcId[eChanPair];
    }
    else if (bNewMaster == true)
    {
    	 /* Take a copy of the previous grant structure as we has allocated the DSP in DEC_OpenChannel */
    	 sTempResrcGrant = hRapCh->sRsrcGrnt;

	 /* Copy only the resources which is granted only for the specified channel pair */
	 hRapCh->sRsrcGrnt.sOpResrcId[eChanPair] = sResrcGrant.sOpResrcId[eChanPair];
	 hRapCh->sRsrcGrnt.uiFmmId = sResrcGrant.uiFmmId;
	 hRapCh->sRsrcGrnt.uiNumOpPorts = sResrcGrant.uiNumOpPorts;
		
	 /* Assign the DSP context ID and DSP ID from temp structure saved above*/
	 hRapCh->sRsrcGrnt.uiDspContextId = sTempResrcGrant.uiDspContextId;
 	 hRapCh->sRsrcGrnt.uiDspId = sTempResrcGrant.uiDspId;
    }
    else if (bNewSimul== true)
    {
        hRapCh->sSimulPtRsrcGrnt = sResrcGrant;      
    }
    
    BDBG_MSG(("BRAP_AddOutputPort: Resources allocated. "));

    /* Call BRAP_P_OpenOpPathFmmModules() to open all newly allocated 
           resources  and save their handles in the channel handle */

    /* Formulate open time settings for all FMM modules */
    for(j=0; j < BRAP_RM_P_MAX_RBUFS_PER_SRCCH; j++)
    {	
        BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[j]);
        if ((bClone == true) && (sResrcReq.bAllocateRBuf == true))
        {
            /* This is a shared RBUF. Note that no new memory will be allocated for it. 
            We'll be reusing the memory associated with the Master RBUF and also master RBUF settings */
            if(hRapCh->sModuleHandles.hRBuf[(2*eChanPair)+j] != NULL)
            {
                sRbufSettings[j] = hRapCh->sModuleHandles.hRBuf[(2*eChanPair)+j]->sSettings;
                sRbufSettings[j].bRbufOfClonedPort = true;
            }
        }
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)        
        if (hRapCh->bLargeRbuf == true)
        {   /* We need to allocate larger RBUFs */
            sRbufSettings[j].sExtSettings.uiSize = BRAP_RBUF_P_DEFAULT_DELAY_RBUF_SIZE;
        }        
#endif		
#if (BRAP_SECURE_HEAP==1)	
	/* If BRAP_SECURE_HEAP is defined, RBUFS should be allocated from Secure 
	Memory Region for Decode channels, and normal region for PB and 
	CAP channels. */
	if (hRapCh->eChannelType == BRAP_P_ChannelType_eDecode)
	{
		sRbufSettings[j].bSecureMemory = true;
	}
#endif		
    }	        
       
    /* Source channel settings: Currently it is a blank struct  */     
 
    /* Mixer settings */
    BRAP_MIXER_P_GetDefaultSettings(&sMixerSettings);
    sMixerSettings.uiMixerInput = sResrcGrant.sOpResrcId[eChanPair].uiMixerInputId;

    BRAP_SPDIFFM_P_GetDefaultSettings (&sSpdifFmSettings);
    sSpdifFmSettings.sExtSettings = hRap->sOutputSettings[eNewOpPort].sSpdiffmSettings;
    
    /* Get New Output port settings: these should have been set by the app 
    earlier by calling BRAP_SetOutputConfig(). So we just need to get them from hRap */
    switch(eNewOpPort)
    {
        case BRAP_OutputPort_eSpdif:
            sSpdifSettings.sExtSettings = 
                hRap->sOutputSettings[eNewOpPort].uOutputPortSettings.sSpdifSettings;
            pNewOpSettings = &sSpdifSettings;
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:              
            sI2sSettings.sExtSettings = 
                hRap->sOutputSettings[eNewOpPort].uOutputPortSettings.sI2sSettings;
            pNewOpSettings = &sI2sSettings;
            break;
        case BRAP_OutputPort_eMai:
            sMaiSettings.sExtSettings = 
                hRap->sOutputSettings[eNewOpPort].uOutputPortSettings.sMaiSettings;
            pNewOpSettings = &sMaiSettings;
            break;
        case BRAP_OutputPort_eFlex:
            sFlexSettings.sExtSettings = 
                hRap->sOutputSettings[eNewOpPort].uOutputPortSettings.sFlexSettings;
            pNewOpSettings = &sFlexSettings;
            break;
        case BRAP_OutputPort_eDac0:
        case BRAP_OutputPort_eDac1:
            sDacSettings.sExtSettings = 
                hRap->sOutputSettings[eNewOpPort].uOutputPortSettings.sDacSettings;
            pNewOpSettings = &sDacSettings;
            break;
                default:
                    BDBG_ERR(("BRAP_AddOutputPort: Output port type %d not supported",  eNewOpPort ));
                    ret = BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED;
                    ret = BERR_TRACE(ret);   
                    goto free_resrc;
            
    }

    sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
    sTempHandles.hDsp = hRapCh->sModuleHandles.hDsp;
    ret = BRAP_P_OpenOpPathFmmModules(
                                    hRapCh,
                                    &sRbufSettings[0],
                                    &sSrcChSettings,
                                    &sMixerSettings,
                                    &sSpdifFmSettings,
                                    pNewOpSettings,
                                    &(sResrcGrant.sOpResrcId[eChanPair]),
                                    eChanPair,
                                    &(sTempHandles));
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        goto free_resrc;
    }

    /* Save the ids and handles of the new resources in the channel handle */
    if (bClone == true)
   {
        /*Save RBUF Handles: note handles will be NULL if no new rbufs were opened.*/
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hRbuf[0]=
                                            sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair];
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hRbuf[1]=
                                            sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1];

        /*Save SrcCh Handles: note handles will be NULL if no new SrcCh were opened.*/
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hSrcCh = sTempHandles.hSrcCh[eChanPair];        
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hMixer = sTempHandles.hMixer[eChanPair];  
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].uiMixerInputIndex = sTempHandles.uiMixerInputIndex[eChanPair];  
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hSpdifFm = sTempHandles.hSpdifFm[eChanPair];  
        hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hOp = sTempHandles.hOp[eChanPair];  


        if (bSimulPt == true)
        {
            BDBG_MSG(("This is the clone of a simul Pt port. "));
            hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].bSimul = true; 
        }
        else
        {
            BDBG_MSG(("This is not a clone of Simul port"));
            hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].bSimul = false; 
        }		

        BDBG_MSG(("sCloneOpPathHandles[%d][%d].hRbuf[%d]=0x%x",
            eChanPair, eNewOpPort, BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair, 
            hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hRbuf[0]));        
        BDBG_MSG(("sCloneOpPathHandles[%d][%d].hRbuf[%d]=0x%x",
            eChanPair, eNewOpPort, BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair+1, 
            hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hRbuf[1]));           
        BDBG_MSG (("sCloneOpPathHandles[%d][%d].hSrcCh = 0x%x", 
                    eChanPair, eNewOpPort, hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hSrcCh));                          
        BDBG_MSG (("sCloneOpPathHandles[%d][%d].hMixer = 0x%x", 
                    eChanPair, eNewOpPort, hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hMixer));                
        BDBG_MSG (("sCloneOpPathHandles[%d][%d].uiMixerInputIndex = %d", 
                    eChanPair, eNewOpPort, hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].uiMixerInputIndex));                
        BDBG_MSG (("sCloneOpPathHandles[%d][%d].hSpdifFm = 0x%x", 
                    eChanPair, eNewOpPort, hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hSpdifFm));                
        BDBG_MSG (("sCloneOpPathHandles[%d][%d].hOp = 0x%x", 
                    eChanPair, eNewOpPort, hRapCh->sCloneOpPathHandles[eChanPair][eNewOpPort].hOp));  

    }
    else 
    {
        /* its a new master or simul  port */
        pTemp->hDsp =  sTempHandles.hDsp;
        pTemp->hFmm =  sTempHandles.hFmm;
        pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair];
        pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1];
        pTemp->hSrcCh[eChanPair] = sTempHandles.hSrcCh[eChanPair];
        pTemp->hMixer[eChanPair] = sTempHandles.hMixer[eChanPair];  
        pTemp->uiMixerInputIndex[eChanPair] = sTempHandles.uiMixerInputIndex[eChanPair];  
        pTemp->hSpdifFm[eChanPair] = sTempHandles.hSpdifFm[eChanPair];  
        pTemp->hOp[eChanPair] = sTempHandles.hOp[eChanPair];    

        BDBG_MSG(("hRBuf[%d]=0x%x", BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair, pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] )); 
        BDBG_MSG(("hRBuf[%d]=0x%x", BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1, pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] )); 
        BDBG_MSG(("hSrcCh[%d]=0x%x", eChanPair, pTemp->hSrcCh[eChanPair] ));         
        BDBG_MSG(("hMixer[%d]=0x%x", eChanPair, pTemp->hMixer[eChanPair] ));     
        BDBG_MSG(("uiMixerInputIndex[%d]=0x%x", eChanPair, pTemp->uiMixerInputIndex[eChanPair] ));     
        BDBG_MSG(("hSpdifFm[%d]=0x%x", eChanPair, pTemp->hSpdifFm[eChanPair] ));     
        BDBG_MSG(("hOp[%d]=0x%x", eChanPair, pTemp->hOp[eChanPair] ));             
    }
        
    BDBG_MSG(("BRAP_AddOutputPort: Resources have been opened. "));

    if (bSimulPt == true)
    {
        /* Remember that we've now reserved a port for SimulPt context */
        hRapCh->bSimulModeConfig = true;
    }
    
    goto end;    

free_resrc:
    ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
    if(ret != BERR_SUCCESS)
    {
            BDBG_ERR(("BRAP_AddOutputPort: BRAP_RM_P_FreeResources() failed. Ignoring failure"));
    }    

end:    
    BDBG_LEAVE (BRAP_AddOutputPort);
    return ret;
}


BERR_Code BRAP_RemoveOutputPort (
    BRAP_ChannelHandle hRapCh,  /* [in] The RAP channel handle */
    BRAP_OutputPort eOpPort            /* [in] Output port to be removed */
)
{

    /* Depending on the type of port and whether or not there's an associated 
    clone, the algorithm followed to remove ports is as follows

    1. eOpPort is a master port carrying PCM data. It has not been cloned:
        - Close associated RBUF, SRCCH, MIXER input, DP, SPDIFFM, Op port.
        - Free all the resources closed above.
        ?? Should we also close and free the DSP context??

    2. eOpPort is a SimulPt port carrying compressed data. It has not been cloned:
        - Close DSP context, RBUF, SRCCH, MIXER input, DP, SPDIFFM, OP 
           associated with this SimulPt port
        - Free all the resources closed above.


    3. eOpPort is a cloned port carrying compressed/PCM data.
        - Close MIXER input, DP, SPDIFFM, OP associated
           with this SimulPt port
        - Free all the resources closed above.

    4. eOpPort is a master port carrying PCM data. But the same channel pair is
    also being sent to one or more clones.
        - Close MIXER input, DP, SPDIFFM, hOp associated with the master port.
        - Free all the resources closed above.
        - find the first cloned port.
        - save this cloned port resources handles into hRapCh->sModuleHandles 
           and and info about the resources into 
           hRapCh->sRsrcGrnt.sOpResrcId[eChanPair]
        - clear out the hRapCh->sCloneOpPathHandles and 
           hRapCh->sCloneOpPathResc structures.
        
    5. eOpPort is a SimulPt port carrying PCM data. But the same channel pair 
    is also being sent to one or more clones.
        - Close MIXER input, DP, SPDIFFM, hOp associated with the SimulPt port.
        - Free all the resources closed above.
        - find the first cloned port.
        - save this cloned port resources handles into 
           hRapCh->sSimulPtModuleHandles and and info about the resources into 
           hRapCh->sSimulPtRsrcGrnt.sOpResrcId[eChanPair]
        - clear out the hRapCh->sCloneOpPathHandles and 
           hRapCh->sCloneOpPathResc structures.

    */
    
    BERR_Code ret = BERR_SUCCESS;
    BRAP_Handle                    hRap;    
    BRAP_RM_P_ResrcGrant    sResrcGrant;
    
    BRAP_P_ObjectHandles      sTempHandles;
    BRAP_OutputChannelPair   eChanPair=0xff;

    BRAP_P_ObjectHandles *   pTemp=NULL;    
    BRAP_RM_P_OpPathResrc *  pTempGrant=NULL;
    
    BRAP_OutputPort eClonePort=BRAP_OutputPort_eMax; /* the cloned port which is usingthe same channel pair */

    bool bSimul = false; /* TRUE: this is a simul PT port. can be cloned or 
    master. */
    bool bMaster = false; /* TRUE: this is the master for this channel pair - PCM data*/ 
    bool bClone = false;    /* TRUE: this is a cloned port (compressed or PCM) */    
    bool bUsedInSameDp = false;
    bool bUsedInOtherDp = false;

    int i,j;

    unsigned int uiOrigDpId =0, uiNewDpId =0;

    BDBG_ENTER (BRAP_RemoveOutputPort);
    
    BDBG_ASSERT (hRapCh);

    /* Make sure the channel has been stopped. 
    We cant add/remove an output port while the channel is running */
    if (hRapCh->bStarted == true)
    {
        BDBG_ERR(("BRAP_RemoveOutputPort: We cant add/remove an output port while"
            "the channel is running. Please stop it first"));
        return BERR_TRACE(ret);        
    }

    BDBG_MSG(("BRAP_RemoveOutputPort: hRapCh=0x%x, hRapCh->uiChannelNo=%d",
           hRapCh,hRapCh->uiChannelNo));   

    ret = BRAP_RM_P_IsOutputPortSupported(eOpPort);
    if( ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_RemoveOutputPort: invalid output port %d", eOpPort));
        return BERR_TRACE(ret);
    }
        

    /* Make sure the temp handles etc are clean */
    BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));

    sResrcGrant.uiNumOpPorts = 1;
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {    
        sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
        sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
        sResrcGrant.sOpResrcId[i].uiPpmId = BRAP_RM_P_INVALID_INDEX;          
    }
    sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
    for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
		sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 
    
    hRap = hRapCh->hRap;        

    BDBG_MSG(("BRAP_RemoveOutputPort: hRapCh = 0x%x, Output port = %d", hRapCh, eOpPort));
  
    /* Find out whether this channle pair is cloned, or master PCM or compressed for Simul mode etc */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        if (hRapCh->sModuleHandles.hOp[i] != NULL)    
        {
            if (hRapCh->sModuleHandles.hOp[i]->eOutputPort == eOpPort)
            {
                /* This is a master port */
                eChanPair = (BRAP_OutputChannelPair)i;
                bMaster = true;
                bClone = false;
                break;
            }
        }
        if (hRapCh->sSimulPtModuleHandles.hOp[i] != NULL)    
        {
            if (hRapCh->sSimulPtModuleHandles.hOp[i]->eOutputPort == eOpPort)
            {
                /* This port was the main PT context for Simul mode */
                eChanPair = (BRAP_OutputChannelPair)i;
                bSimul = true;
                bClone = false;
                /* BRAP_OutputChannelPair_eLR is the only permissible chan pair  for compressed data in simul mode */
                BDBG_ASSERT((eChanPair == BRAP_OutputChannelPair_eLR)); 
                break;
            }
        }        
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
           if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
            {   
                if (hRapCh->sCloneOpPathHandles[i][j].hOp->eOutputPort == eOpPort)
                {
                    /* This port was cloned */
                    eChanPair = (BRAP_OutputChannelPair) i;
                    bClone = true;
                    bSimul = hRapCh->sCloneOpPathHandles[i][j].bSimul;
                    break;
                }
            }
        }
        if (bClone == true) break;
    }   

    BDBG_MSG(("BRAP_RemoveOutputPort: bMaster=%d, bSimul=%d, bClone=%d, eChanPair=%d ", bMaster, bSimul, bClone, eChanPair));
    if ((bMaster == false) && (bSimul==false) && (bClone==false))
    {
        /* No reference to this output port was found in the channel handle */
        BDBG_ERR(("BRAP_RemoveOutputPort: output port does not correspond to this channel!!!"));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    if (bMaster == true)
    {
          pTemp = &(hRapCh->sModuleHandles);   
          pTempGrant = &(hRapCh->sRsrcGrnt.sOpResrcId[eChanPair]);
    }
    else if (bSimul == true)    
    {
        pTemp = &(hRapCh->sSimulPtModuleHandles);
        pTempGrant = &(hRapCh->sSimulPtRsrcGrnt.sOpResrcId[eChanPair]);
    }
  /* Fill the  BRAP_P_ObjectHandles structure for the modules to be closed 
       and the sResrcGrant structure for the resources to be freed */

    /*Get the DP for the port  to be removed.*/
    if ( eOpPort != BRAP_OutputPort_eMai )
    {
        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, eOpPort, &uiOrigDpId);
    }
    else
    {
        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                        hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                        &uiOrigDpId);
    }
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_RemoveOutputPort(): Cant get orig port's DP ID"));
        return BERR_TRACE (ret);
    }
    if (bClone == true)
    {
        sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = 
                            hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hRbuf[0];
        sTempHandles.hRBuf[(BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair) + 1] = 
                            hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hRbuf[1];
        sTempHandles.hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hSrcCh;

        sTempHandles.hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hMixer;  
        sTempHandles.uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].uiMixerInputIndex;  
        sTempHandles.hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hSpdifFm;  
        sTempHandles.hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hOp;  
        
        /* use the cloned resrc grant structure */
        sResrcGrant.sOpResrcId[eChanPair] = hRapCh->sCloneOpPathResc[eChanPair][eOpPort];
        
        /* If this clone has an indep delay, we free all the associated 
        resources unconditionally. But if it does not have a delay, then it 
        might be sharing resources with another clone in the same DP... if yes, 
        then dont free the RRBUF/Srcch. */

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)        
        if (hRap->sOutputSettings[eOpPort].bIndpDlyRqd== false)
        {
#endif                
            /* If there are any other clones in same DP attched to this channel pair, 
        then we need not free ring buffers and srcch. 
            Note: if this cloned port is a simul port, look only for other simul ports*/
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
           if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)  
                  && ((BRAP_OutputPort)j != eOpPort)
                  && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == bSimul))
            {   
                if ( j != BRAP_OutputPort_eMai )
                {
                        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, (BRAP_OutputPort)j, &uiNewDpId);
                }
                else
                {
                    ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                            hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                                                   &uiNewDpId);
                }
                if (ret != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_RemoveOutputPort(): Cant get cloned port's DP ID"));
                    return BERR_TRACE (ret);
                }

                if (uiOrigDpId == uiNewDpId)
                {
                    /* We need to preserve the RBUFs and SrcCh.*/
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = NULL;
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = NULL;
                    sTempHandles.hSrcCh[eChanPair] = NULL;

                    sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;
                    break;
                }
                } /* if hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL */
            }/* for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++) */
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)        
        } /* if (hRap->sOutputSettings[eOpPort].bIndpDlyRqd== false) */
#endif  
    } /* if (bClone == true) */
    else 
    {
        /* this holds for all master ports */
        sTempHandles.hMixer[eChanPair] = pTemp->hMixer[eChanPair];  
        sTempHandles.uiMixerInputIndex[eChanPair] = pTemp->uiMixerInputIndex[eChanPair];  
        sTempHandles.hSpdifFm[eChanPair] = pTemp->hSpdifFm[eChanPair];  
        sTempHandles.hOp[eChanPair] = pTemp->hOp[eChanPair];    

        sResrcGrant.sOpResrcId[eChanPair].eOutputPortType = pTempGrant->eOutputPortType;
        sResrcGrant.sOpResrcId[eChanPair].uiDataPathId = pTempGrant->uiDataPathId;
        sResrcGrant.sOpResrcId[eChanPair].uiMixerId = pTempGrant->uiMixerId;
        sResrcGrant.sOpResrcId[eChanPair].uiMixerInputId = pTempGrant->uiMixerInputId;
        sResrcGrant.sOpResrcId[eChanPair].uiSpdiffmId = pTempGrant->uiSpdiffmId;
        sResrcGrant.sOpResrcId[eChanPair].uiSpdiffmStreamId = pTempGrant->uiSpdiffmStreamId;              
     
        /* Close the RBUF & SrcCh only if no other port is using this channel pair */ 
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
           if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == bSimul)
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)    
                && (hRap->sOutputSettings[j].bIndpDlyRqd == false) /* 
                first try to find a port which doesnt have indep delay */
#endif        
               )
            {   
                /* Check if any other cloned port uses this master in same DP. If any other port in 
                   same Dp is cloned than no need to free SrcCh and RBUFs. And if any port in other 
                   DP is using this port then, SrcCh can freed but not the RBUF's*/
                   
                   /* TODO: handle indep output delay */
                if ( (BRAP_OutputPort)j != BRAP_OutputPort_eMai )
                {
                    ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, (BRAP_OutputPort) j, 
                              &uiNewDpId);
                }
                else
                {
                    ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                                hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                                                    &uiNewDpId);
                }
                if (ret != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_RemoveOutputPort(): Cant get cloned port's DP ID"));
                    return BERR_TRACE (ret);
                }

                if (uiOrigDpId == uiNewDpId)
                {
                    bUsedInSameDp = true;
                    eClonePort = (BRAP_OutputPort)j;
                    BDBG_MSG(("NOTE: Cloned port %d also uses the same data in same DP",j));
                    break;
                }
                else
                {
                    bUsedInOtherDp = true;
                    eClonePort = (BRAP_OutputPort)j;
                    BDBG_MSG(("NOTE: Cloned port %d also uses the same data in other DP",j));
                    break;
                }
            } /* if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL) */
        } /*  for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++) */

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)    
        /* if we didnt find any cloned port without indep delay, try and find 
        if there is any with an indep delay */

        if (eClonePort==BRAP_OutputPort_eMax)
        {
            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {
               if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                    && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == bSimul)
                    && (hRap->sOutputSettings[j].bIndpDlyRqd == true)  
                   )
                {   
                    /* Check if any other cloned port uses this master in same DP. If any other port in 
                       same Dp is cloned than no need to free SrcCh and RBUFs. And if any port in other 
                       DP is using this port then, SrcCh can freed but not the RBUF's*/                  
                    if ( (BRAP_OutputPort)j != BRAP_OutputPort_eMai )
                    {
                        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, (BRAP_OutputPort) j, 
                                  &uiNewDpId);
                    }
                    else
                    {
                        ret = BRAP_RM_P_GetDpForOpPort (hRap->hRm, 
                                    hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                                    &uiNewDpId);
                    }                
                    if (ret != BERR_SUCCESS)
                    {
                        BDBG_ERR(("BRAP_RemoveOutputPort(): Cant get cloned port's DP ID"));
                        return BERR_TRACE (ret);
                    }

                    if (uiOrigDpId == uiNewDpId)
                    {
                        bUsedInSameDp = true;
                        eClonePort = (BRAP_OutputPort)j;
                        BDBG_MSG(("NOTE: Cloned port %d also uses the same data in same DP",j));
                        break;
                    }
                    else
                    {
                        bUsedInOtherDp = true;
                        eClonePort = (BRAP_OutputPort)j;
                        BDBG_MSG(("NOTE: Cloned port %d also uses the same data in other DP",j));
                        break;
                    }
                } /* if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL) */
            } /*  for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++) */           
        } /* if (eClonePort==BRAP_OutputPort_eMax) */
#endif 


        if (bUsedInSameDp == true)
        {
            if (eClonePort != BRAP_OutputPort_eMax)
            {
                if (( hRap->sOutputSettings[eClonePort].bIndpDlyRqd != true )&&
                       ( hRap->sOutputSettings[eOpPort].bIndpDlyRqd != true ))
                {
                    /* => Output port j in same Dp is using the same channel pair. So make that the master now. 
                    Dont close & remove the SrcCh and rbuf here */
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = NULL;
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = NULL; 
                    sTempHandles.hSrcCh[eChanPair] = NULL;

                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
                    sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
                }

                else
                {
                    /* no other port is using the same rbuf/srcch. so we can close them */
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair];
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1];
                    sTempHandles.hSrcCh[eChanPair] = pTemp->hSrcCh[eChanPair];     

                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = pTempGrant->uiRbufId[0];
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = pTempGrant->uiRbufId[1];    
                    sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = pTempGrant->uiSrcChId;
                }
           }
        }
        else if (bUsedInOtherDp == true)
        {
            if (eClonePort != BRAP_OutputPort_eMax)
            {        
                if ( hRap->sOutputSettings[eClonePort].bIndpDlyRqd == true )
                {
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = NULL;
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = NULL;
                    sTempHandles.hSrcCh[eChanPair] = pTemp->hSrcCh[eChanPair];

                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
                    sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = pTempGrant->uiSrcChId;
                }
                else
                {
                    /* no other port is using the same rbuf/srcch. so we can close them */
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair];
                    sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1];
                    sTempHandles.hSrcCh[eChanPair] = pTemp->hSrcCh[eChanPair];     

                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = pTempGrant->uiRbufId[0];
                    sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = pTempGrant->uiRbufId[1];    
                    sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = pTempGrant->uiSrcChId;
                }
            }
        }
        else
        {      
            /* no other port is using the same rbuf/srcch. so we can close them */
            sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair];
            sTempHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1] = pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair + 1];
            sTempHandles.hSrcCh[eChanPair] = pTemp->hSrcCh[eChanPair];     

            sResrcGrant.sOpResrcId[eChanPair].uiRbufId[0] = pTempGrant->uiRbufId[0];
            sResrcGrant.sOpResrcId[eChanPair].uiRbufId[1] = pTempGrant->uiRbufId[1];    
            sResrcGrant.sOpResrcId[eChanPair].uiSrcChId = pTempGrant->uiSrcChId;
        }
    }
      
    /* Close cloned modules associated with this sTempHandles */
    ret = BRAP_P_CloseOpPathFmmModules (eChanPair, &sTempHandles);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_RemoveOutputPort: call to BRAP_P_CloseOpPathFmmModules() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }
    BDBG_MSG(("Closed FMM resource associated with this output port"));

#if 0 /* Pass through context can be freed in Dec stop and not here */	
    /* If this was the "master" SimulPt port, then we also need to free the DSP context */
    if ((bSimul==true) && (bClone==false) && (bUsedInOtherDp == false) && (bUsedInSameDp == false))
    {
        sResrcGrant.uiDspId = hRapCh->sSimulPtRsrcGrnt.uiDspId;
        sResrcGrant.uiDspContextId = hRapCh->sSimulPtRsrcGrnt.uiDspContextId;
        BDBG_MSG(("freed dsp context %d ", sResrcGrant.uiDspContextId));
    }
#endif	

    /* Free the resources in the RM */
    ret = BRAP_RM_P_FreeResources(hRap->hRm, &(sResrcGrant));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_RemoveOutputPort: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }    
    BDBG_MSG(("Freed the resources associated with this port"));   
    
    /* Remove the closed modules' handles from the channel handle */
    BDBG_MSG(("BRAP_RemoveOutputPort: Printing out mnodule handles after removing output port:"));    
    if (bClone == true)
    {
        /* Remove the cloned modules' handle frmo the channel handle */ 
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hSrcCh = NULL;
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hMixer = NULL;  
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;  
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hSpdifFm = NULL;  
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].hOp = NULL;  
        hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].bSimul = false;

        /* Clear the Resc Grant struct for this output port!!! */
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiMixerId = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
        hRapCh->sCloneOpPathResc[eChanPair][eOpPort].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  

        BDBG_MSG(("Cloned port path closed and freed"));
    }
    else 
    {      
        /* for a master port or for SimulPT port */
        /* remove the RBUF & SrcCh only if no other port is using this channel pair */

        if (bUsedInSameDp == true)
        {
            /* => Output port j is using the same channel pair. So make that the master now. 
                Dont remove the SrcCh and rbuf here */

            /* Move the cloned port's handles to the Master structure now. Preserve earlier RBUF and SrcCh handles*/
            pTemp->hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hMixer; 
            pTemp->uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].uiMixerInputIndex;  
            pTemp->hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hSpdifFm;  
            pTemp->hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hOp;    

            /* Copy the new SrcCh and Reset the bSharedRbuf for the new master*/
	     /* Copy only if the cloned port has a srch channel allocated for it */
	     if ( hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hSrcCh != NULL )
	     {
                pTemp->hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hSrcCh;
                pTempGrant->uiSrcChId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSrcChId;
	     }
	     if ( hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[0] != NULL )
	     {
	        pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = 
				hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[0];
		 pTempGrant->uiRbufId[0] = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiRbufId[0];
	     }
     	     if ( hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[1] != NULL )
	     {
	        pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair+1] = 
				hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[1];
		 pTempGrant->uiRbufId[1] = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiRbufId[1];
	     }
		 
            /* Clear the cloned handles, since its' now been made a master port */
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[0]= NULL;
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hRbuf[1]= NULL;
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hSrcCh = NULL;
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hMixer = NULL;  
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;  
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hSpdifFm = NULL;  
            hRapCh->sCloneOpPathHandles[eChanPair][eClonePort].hOp = NULL;  
            hRapCh->sCloneOpPathHandles[eChanPair][eOpPort].bSimul = false;

            /* Save the Ids of the Cloned resource in the main RescrGrant structure now.
            Dont change the rbuf and src ch*/
            pTempGrant->eOutputPortType = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].eOutputPortType ;
            pTempGrant->uiDataPathId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiDataPathId;
            pTempGrant->uiMixerId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiMixerId;
            pTempGrant->uiMixerInputId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiMixerInputId;
            pTempGrant->uiSpdiffmId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSpdiffmId;
            pTempGrant->uiSpdiffmStreamId = hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSpdiffmStreamId;


            /* Clear the Resc Grant struct for this output port!!! */
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiMixerId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][eClonePort].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  

            if (bSimul == true)
                BDBG_MSG(("Cloned port %d swapped with Simul port %d", eClonePort, eOpPort));
            else if (bMaster == true)
                BDBG_MSG(("Cloned port %d swapped with master port %d", eClonePort, eOpPort));

        }
        else
        {
      
            /* the OP path isnt used by anyone else. Close and free all resources */
            pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair] = NULL;
            pTemp->hRBuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH*eChanPair +1] = NULL;            
            pTemp->hSrcCh[eChanPair] = NULL;            
            pTemp->hMixer[eChanPair] = NULL;
            pTemp->uiMixerInputIndex[eChanPair] = BRAP_RM_P_INVALID_INDEX;
            pTemp->hSpdifFm[eChanPair] = NULL;            
            pTemp->hOp[eChanPair] = NULL;

            pTempGrant->eOutputPortType = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
            pTempGrant->uiSrcChId = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiDataPathId = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiMixerId = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
            pTempGrant->uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;    

            /* If this was the "master" SimulPt port, then we also need to free the DSP context and clear the handle*/
            if ((bSimul==true) && (bUsedInSameDp==false) && (bUsedInOtherDp == false))
            {
                hRapCh->sSimulPtRsrcGrnt.uiDspContextId = BRAP_RM_P_INVALID_INDEX;  
                hRapCh->bSimulModeConfig = false;
            }            
         }
    }

    BDBG_LEAVE (BRAP_RemoveOutputPort);
    return ret;
}

#endif /* not for 7411 */

/**********************************************************************
Summary:
	This function will configure the CRC registers

Description:
	This function will configure the CRC registers and give the crc configuration
	to install the inerrupt for respective output port. 
	This api must be called before installing BRAP_Interrupt_eFmmCrc interrupt.

Returns:
    BERR_SUCCESS - If function is successful.
    BERR_NOT_SUPPORTED - On bad input parameters.
***********************************************************************/

BERR_Code BRAP_ConfigCrc
(
	BRAP_ChannelHandle	hRapCh,	/* [in] The RAP Channel handle */
	uint32_t				uiSamplePeriod, /* [in] The sample period of Crc */
	uint32_t				*pCrcCfg  /* [out] the crc configuration*/
)
{
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)|| (BCHP_CHIP == 7403))
	uint32_t				reg=0;
	BERR_Code			rc=BERR_SUCCESS;
	uint32_t				uicrc_config;
	uint32_t				uiBitsPerSample=24;
	BRAP_OutputPort		eOp = BRAP_OutputPort_eMax;

	BDBG_ENTER (BRAP_ConfigCrc);
	BDBG_ASSERT(hRapCh);

	if(hRapCh->sModuleHandles.hOp[0]->eOutputPort==BRAP_OutputPort_eMax)
	{
		BDBG_ERR(("BRAP_ConfigCrc: Output destination not added"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	eOp=hRapCh->sModuleHandles.hOp[0]->eOutputPort;
	switch(eOp)	
	{
		case BRAP_OutputPort_eSpdif: 	uicrc_config=0; 	break;
		case BRAP_OutputPort_eMai:	uicrc_config=1; 	
									uiBitsPerSample=hRapCh->sModuleHandles.hOp[0]->uOpParams.sMai.uiSampleWidth;
									break;
		case BRAP_OutputPort_eDac0:	uicrc_config=2;	break;
		case BRAP_OutputPort_eI2s0:	uicrc_config=3;	
									uiBitsPerSample=hRapCh->sModuleHandles.hOp[0]->uOpParams.sI2s.uiBitsPerSample;
									break;
		case BRAP_OutputPort_eI2s1:	uicrc_config=4; 	
									uiBitsPerSample=hRapCh->sModuleHandles.hOp[0]->uOpParams.sI2s.uiBitsPerSample;		
									break;
		case BRAP_OutputPort_eFlex:	uicrc_config=5;	break;
		case BRAP_OutputPort_eDac1:	uicrc_config=6;	break;
		case BRAP_OutputPort_eI2s2:	uicrc_config=7;	break; /*chaeck with bala*/
		default :
			BDBG_ERR(("BRAP_ConfigCrc: CRC is not supported on eOp = %d",eOp));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
		break;
	}
	*pCrcCfg=uicrc_config;
	switch(uiBitsPerSample)
	{
		case 8:
			BDBG_ERR(("BRAP_ConfigCrc: 8 bits/sample CRC not supported"));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
			break;
		case 24:
			reg= reg | (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_CRC_CFGi,CRC_DAT_WIDTH,sample_24_bit));			
			break;
		case 20:
			reg= reg | (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_CRC_CFGi,CRC_DAT_WIDTH,sample_20_bit));			
			break;			
		case 16:
			reg= reg | (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_CRC_CFGi,CRC_DAT_WIDTH,sample_16_bit));			
			break;
		default:
			reg= reg | (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_CRC_CFGi,CRC_DAT_WIDTH,spdif_status));
			break;		
	}

    /* ePolarity is assigned to BRAP_CrcPolarity_0, That is all_zeroes*/  
	reg= reg | (BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_CRC_CFGi,CRC_INIT_POL,all_zeroes));

	reg |=0x1; /* To enable the crc HardWare */
	BRAP_Write32 (hRapCh->hRegister, 
	                   ( BCHP_AUD_FMM_OP_CTRL_CRC_CFGi_ARRAY_BASE 
	                   + uicrc_config*4 ), reg); 
	/* Write the sample period of crc */
	BRAP_Write32 (hRapCh->hRegister, 
	                   ( BCHP_AUD_FMM_OP_CTRL_CRC_PERIODi_ARRAY_BASE 
	                   + uicrc_config*4 ), uiSamplePeriod); 
	
	BDBG_LEAVE(BRAP_ConfigCrc);
	return rc;	
#else
	BSTD_UNUSED(hRapCh);
	BSTD_UNUSED(uiSamplePeriod);
	BSTD_UNUSED(pCrcCfg);
	return BERR_NOT_SUPPORTED;
#endif
}


/* End of File */
