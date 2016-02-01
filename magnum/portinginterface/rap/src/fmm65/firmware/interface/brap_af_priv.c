/***************************************************************************
*     Copyright (c) 2003-2011, Broadcom Corporation
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

/* HEADER FILE INCLUSION */

#ifdef TESTBENCH_BUILD
	/* Test bench File Inclusion */
	#include "data_types.h"
	#include "brap_buff_defs_7043.h"
#else 

	/* PI header File Inclusion */
	#include "brap_types.h"
	#include "brap_dspchn.h"
#endif

/* CIT-Gen  header File Inclusion */
#include "brap_af_priv.h"
#include "brap_cit_priv.h"

/*-------------------------------------------------------------------------*/
/* Defines Used only in the brap_af_priv.c File*/
/*-------------------------------------------------------------------------*/

#define BRAP_AF_P_EXTRA_SAMPLES	(8)	/* Extra Buffer Spacing given for	
									   IoBuffers 
									*/

/*-------------------------------------------------------------------------*/
/*						Array instantiations							   */
/*-------------------------------------------------------------------------*/

/*	The structure will be instantiated based on the max algo type id */
/*	This will be a two dimensional structure. The second dimesion
	indicates what algos need to be executed for say passthrough of the same
	algorithm.
	There will be three such structures. One for Decoder, Encoder and 
	one for post process */
/*	The structure will be initialized by the firmware and provided to the
	PI for each of the algorithm type */
/*	WARNING!!! Removed static declaration due to compilation error */

/*	Init structure for decoder algorithms */
const BRAP_AF_P_sALGO_EXEC_INFO BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_VideoType_eMax][BRAP_DSPCHN_DecodeMode_eMax] =
{
	/* BRAP_DSPCHN_AudioType_eMpeg */
	{
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eMpeg][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		/* Number of nodes in Mpeg Decode */
			{
				BRAP_AF_P_AlgoId_eMpegFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,
				
				BRAP_AF_P_AlgoId_eMpegDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eMpeg][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in Mpeg pass through */
			{
				BRAP_AF_P_AlgoId_eMpegFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,
				
				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/* BRAP_DSPCHN_AudioType_eAacAdts */
	{
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		/* MS10 Dolby ADTS Pulse support */
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacAdts][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			6,		
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacAdts][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

#else
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacAdts][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			7,		/* Number of nodes in AAC ADTS Decode */
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacAdts][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AAC ADTS  pass thru */
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},

	/* BRAP_DSPCHN_AudioType_eAacLoas */
	{

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

		/* MS10 Dolby LOAS Pulse support */
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoas][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			6,		
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoas][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

#else
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoas][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			7,		/* Number of nodes in AAC LOAS Decode */
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoas][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AAC LOAS pass thru */
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif

	},	
	
	/* BRAP_DSPCHN_AudioType_eAacSbrAdts */
	{

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		
		/* MS10 Dolby ADTS Pulse support */

		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacSbrAdts][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			6,		
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacSbrAdts][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

#else
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacSbrAdts][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			7,		/* Number of nodes in AAC-HE ADTS Decode */
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacSbrAdts][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AAC-HE ADTS pass thru */
			{
				BRAP_AF_P_AlgoId_eAdtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},
		
	/* BRAP_DSPCHN_AudioType_eAacSbrLoas */
	{
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

		/* MS10 Dolby LOAS Pulse support */

		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoasSbr][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			6,		
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2,
				BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoasSbr][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

#else
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacSbrLoas][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			7,		/* Number of nodes in AAC-HE LOAS Decode */
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3,
				BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAacLoasSbr][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AAC-HE  LOAS pass thru */
			{
				BRAP_AF_P_AlgoId_eLoasFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},
	/* BRAP_DSPCHN_AudioType_eAc3 */
	{
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		
		/*MS10 DD Decoder */
					
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		
			{
				BRAP_AF_P_AlgoId_eMs10DdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eMs10DdpDecodeStage1,
				BRAP_AF_P_AlgoId_eMs10DdpDecodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eMs10DdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
#else					
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		/* Number of nodes in AC3 Decode */
			{
				BRAP_AF_P_AlgoId_eAc3FrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAc3DecodeStage1,
				BRAP_AF_P_AlgoId_eAc3DecodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AC3 Pass thru */
			{
				BRAP_AF_P_AlgoId_eAc3FrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},

	/* BRAP_DSPCHN_AudioType_eAc3Plus */
	{

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

		/*MS10 DDP  Decoder */				
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3Plus][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		
			{
				BRAP_AF_P_AlgoId_eMs10DdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eMs10DdpDecodeStage1,
				BRAP_AF_P_AlgoId_eMs10DdpDecodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3Plus][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eMs10DdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
#else	
		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3Plus][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		/* Number of nodes in AC3 Plus Decode */
			{
				BRAP_AF_P_AlgoId_eDdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDdpDecodeStage1,
				BRAP_AF_P_AlgoId_eDdpDecodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAc3Plus][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in AC3 Plus Pass thru */
			{
				BRAP_AF_P_AlgoId_eDdpFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},
	/* BRAP_DSPCHN_AudioType_eDts */
	{
		{/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDts][BRAP_DSPCHN_DecodeMode_eDecode] = */
			3,		
			{
				BRAP_AF_P_AlgoId_eDtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDtsDecodeStage1,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDts][BRAP_DSPCHN_DecodeMode_ePassThru] = */
			3,		
			{
				BRAP_AF_P_AlgoId_eDtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/* BRAP_DSPCHN_AudioType_eLpcmBd */
	{
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	/* BRAP_DSPCHN_AudioType_eLpcmHdDvd */
	{
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/*BRAP_DSPCHN_AudioType_eDtshd*/
	{
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDtshd][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		
			{
				BRAP_AF_P_AlgoId_eDtsHdFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDtsHdDecodeStage0,
				BRAP_AF_P_AlgoId_eDtsHdDecodeStage1,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDtshd][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eDtsHdFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	/* BRAP_DSPCHN_AudioType_eLpcmDvd */
	{
		/*BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eLpcmDvd][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eDvdLpcmFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDvdLpcmDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/*BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eLpcmDvd][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/* BRAP_DSPCHN_AudioType_eWmaStd */
	{

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eWmaStd][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		/* Number of nodes in WMA-STD Decode */
			{
				BRAP_AF_P_AlgoId_eWmaStdFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eWmaStdDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eWmaStd][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in WMA-STD Pass thru */
			{
				BRAP_AF_P_AlgoId_eWmaStdFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},		
	},
	
	/* BRAP_DSPCHN_AudioType_eAc3Lossless */
	{		
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/*BRAP_DSPCHN_AudioType_eMlp*/
	{		
		{	
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/* BRAP_DSPCHN_AudioType_ePcm*/
	{		
		{
			/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_ePcm][BRAP_DSPCHN_DecodeMode_eDecode] = */
			3,		
			{
				BRAP_AF_P_AlgoId_ePesFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,
				BRAP_AF_P_AlgoId_ePassThru,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_ePcm][BRAP_DSPCHN_DecodeMode_ePassThru] = */
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	{
		/*BRAP_DSPCHN_AudioType_eDtsLbr*/
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	{
		/* BRAP_DSPCHN_AudioType_eDdp7_1*/
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	{
		/*BRAP_DSPCHN_AudioType_eMpegMc*/
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},


	/* BRAP_DSPCHN_AudioType_eWmaPro */
	{

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eWmaPro][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		/* Number of nodes in WMA-pro Decode */
			{
				BRAP_AF_P_AlgoId_eWmaProFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eWmaProStdDecodeStage1,
				BRAP_AF_P_AlgoId_eWmaProStdDecodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eWmaPro][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		/* Number of nodes in WMA-pro Pass thru */
			{
				BRAP_AF_P_AlgoId_eWmaProFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},		
	},

	{
		/*BRAP_DSPCHN_AudioType_eDtshdSub*/
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	{
		/*BRAP_DSPCHN_AudioType_eLpcmDvdA*/
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eLpcmDvdA][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eLpcmDvdA][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	{
		/*BRAP_DSPCHN_AudioType_eDtsBroadcast*/
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDtsBroadcast][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			4,		
			{
				BRAP_AF_P_AlgoId_eDtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDtsHdDecodeStage0,
				BRAP_AF_P_AlgoId_eDtsHdDecodeStage1,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDtsBroadcast][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eDtsFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	{
		/*BRAP_DSPCHN_AudioType_ePcmWav*/
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_ePcmWav][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_ePcmWavFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePcmWavDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_ePcmWav][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	/*BRAP_DSPCHN_AudioType_eAmr*/
	{		
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAmr][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eAmrFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eAmrDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eAmr][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eAmrFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/*BRAP_DSPCHN_AudioType_eDra*/
	{		
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDra][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eDraFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eDraDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eDra][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eDraFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	/*BRAP_DSPCHN_AudioType_eRealAudioLbr*/
	{		
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eRealAudioLbr][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eRealAudioLbrFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_eRealAudioLbrDecode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eRealAudioLbr][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			3,		
			{
				BRAP_AF_P_AlgoId_eRealAudioLbrFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,

				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},

	/*BRAP_DSPCHN_AudioType_eRealVideo9*/
	{		
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eRealVideo9][BRAP_DSPCHN_DecodeMode_eDecode] = */
		{
			5,		
			{
				BRAP_AF_P_AlgoId_eRealVideo9PLF,
				BRAP_AF_P_AlgoId_eRealVideo9Stage1,
				BRAP_AF_P_AlgoId_eRealVideo9Stage2,
				BRAP_AF_P_AlgoId_eRealVideo9Stage3,
				BRAP_AF_P_AlgoId_eRealVideo9PPD,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/* BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_AudioType_eRealVideo9][BRAP_DSPCHN_DecodeMode_ePassThru] = */
		{
			0,		
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,

				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		}
	},
	
};

/*	Structure for Encoder algorithms */

const BRAP_AF_P_sALGO_EXEC_INFO BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMax][BRAP_DSPCHN_EncodeMode_eSimulMode] =
{
	/*BRAP_CIT_P_EncAudioType_eMpeg1Layer3*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMpeg1Layer3][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in MPEG1 L3 Encode */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eMpegL3EncodeStage1,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMpeg1Layer3][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in MPEG1 Layer 3 Encode NRT */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eMpegL3EncodeStage1,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_EncAudioType_eMpeg1Layer2*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMpeg1Layer2][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			4,	/* Number of nodes in MPEG1 Layer 2 Encode */
			{
				BRAP_AF_P_AlgoId_eMpegL2EncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eMpegL2EncodeStage1,
				BRAP_AF_P_AlgoId_eMpegL2EncodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMpeg1Layer2][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			4,	/* Number of nodes in MPEG1 Layer 2 Encode NRT */
			{
				BRAP_AF_P_AlgoId_eMpegL2EncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eMpegL2EncodeStage1,
				BRAP_AF_P_AlgoId_eMpegL2EncodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_EncAudioType_eDTS*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eDTS][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in DTS Encode */
			{
				BRAP_AF_P_AlgoId_eDtsEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eDtsEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eDTS][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in DTS Encode */
			{
				BRAP_AF_P_AlgoId_eDtsEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eDtsEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_EncAudioType_eAacLc*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAacLc][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			4,	/* Number of nodes in AAC-LC Encode */
			{
				BRAP_AF_P_AlgoId_eAacLcEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eAacLcEncodeStage1,
				BRAP_AF_P_AlgoId_eAacLcEncodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAacLc][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			4,	/* Number of nodes in AAC-LC Encode */
			{
				BRAP_AF_P_AlgoId_eAacLcEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eAacLcEncodeStage1,
				BRAP_AF_P_AlgoId_eAacLcEncodeStage2,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_EncAudioType_eAacHe*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAacHe][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			5,	/* Number of nodes in AAC-HE Encode */
			{
				BRAP_AF_P_AlgoId_eAacHeEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eAacHeEncodeStage1,
				BRAP_AF_P_AlgoId_eAacHeEncodeStage2,
				BRAP_AF_P_AlgoId_eAacHeEncodeStage3,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAacHe][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			5,	/* Number of nodes in AAC-HE Encode NRT*/
			{
				BRAP_AF_P_AlgoId_eAacHeEncFrameSync,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eAacHeEncodeStage1,
				BRAP_AF_P_AlgoId_eAacHeEncodeStage2,
				BRAP_AF_P_AlgoId_eAacHeEncodeStage3,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_EncAudioType_eAc3*/
	{

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

		/*MS10 DD Encoder */
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAc3][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in DD Transcode */
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eMs10DDTranscode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAc3][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in DDTranscode NRT*/
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eMs10DDTranscode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#else

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAc3][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in AC3 Encode */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAc3Encode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eAc3][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in AC3 Encode NRT*/
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAc3Encode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
#endif
	},

	/*BRAP_CIT_P_EncAudioType_eDTSBroadcast*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eDTSBroadcast][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in DTSBroadcast Encode */
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eDtsBroadcastEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid 
			},
		},

		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eDTSBroadcast][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in DTSBroadcast Encode NRT*/
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eDtsBroadcastEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_EncAudioType_eSbc*/
	{
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eSbc][BRAP_DSPCHN_EncodeMode_eRealtime] = */
		{
			3,	/* Number of nodes in SBC Encode */
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eSbcEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
		/*	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eSbc][BRAP_DSPCHN_EncodeMode_eNonRealtime] = */
		{
			3,	/* Number of nodes in SBC Encode NRT*/
			{
                BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eEncodeTsm,
				
				BRAP_AF_P_AlgoId_eSbcEncode,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},	
};


/*	Structure for Encoder algorithms */
const BRAP_AF_P_sALGO_EXEC_INFO BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eMax][BRAP_DSPCHN_ProcessingMode_eMax] =
{
	/*BRAP_CIT_P_ProcessingType_eDdbm*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDdbm][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DDBM Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDdbmPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eDtsNeo*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDtsNeo][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DTS Neo Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDtsNeoPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eAVL*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eAVL][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in AVL Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAvlPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

	/*BRAP_CIT_P_ProcessingType_eDDConvert,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDDConvert][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes MS DDConvert Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eMs10DDConvert,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
#else
	
	/*BRAP_CIT_P_ProcessingType_eDDConvert*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDDConvert][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DDConvert Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDDConvert,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
#endif

	/*BRAP_CIT_P_ProcessingType_ePLll,*/    /* Dolby Prologic-II. */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_ePLll][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Dolby Prologic-II*/
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_ePl2PostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_eSrsXt,	*/    /* TruSurroundXT. */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrsXt][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Srs Xt */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrsTruSurroundPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*	BRAP_CIT_P_ProcessingType_eXen,		*/	/* XEN 3D Sound Processing. */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eXen][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			0,	/* Number of nodes in XEN 3D Sound Processing Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*	BRAP_CIT_P_ProcessingType_eBbe,	  */       /* BBE */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eBbe][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			0,	/* Number of nodes in BBE Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*	BRAP_CIT_P_ProcessingType_eSrc,	 */	    /* Sample Rate Change */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrc][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in SRC Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrcPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*	BRAP_CIT_P_ProcessingType_eCustomSurround, */	/* CUSTOM Surround Algorithm */
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eCustomSurround][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in CUSTOM Surround Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eCustomSurroundPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*	BRAP_CIT_P_ProcessingType_eCustomBass,   */  /* CUSTOM Bass Algorithm */	
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eCustomBass][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in CUSTOM Bass Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eCustomBassPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eCustomVoice, */  /* CUSTOM Voice Algorithm*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eCustomVoice][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in CUSTOM Voice Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eCustomVoicePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_ePeq, */          /* PEQ Algorithm */	
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_ePeq][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			0,	/* Number of nodes in PEQ Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eDownmix0,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDownmix0][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			0,	/* Number of nodes in Downmix Post Proc */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_eAudioDescriptorFade,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eAudioDescriptorFade][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in AudioDescriptor Fade */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAudioDescriptorFadePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_eAudioDescriptorPan,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eAudioDescriptorPan][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in AudioDescriptor Pan */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAudioDescriptorPanPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_ePCMRouter,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_ePCMRouter][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in PCMRouter */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_ePCMRouterPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_eWMAPassThrough,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eWMAPassThrough][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in WMAPassThrough */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eWMAPassThrough,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},	

	/*BRAP_CIT_P_ProcessingType_eDsola,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDsola][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DSOLA  */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDsolaPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eSrsHd,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrsHd][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in SrsHd  */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrsTruSurroundHDPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eGenericPassThru,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eGenericPassThru][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Generic Pass through  */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_ePassThru,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eSrsTruVolume,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrsTruVolume][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Srs VolumeIq  */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrsTruVolumePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eDolbyVolume,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDolbyVolume][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Dolby Volume */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDolbyVolumePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eAudysseyVolume,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eAudysseyVolume][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in AudysseyV olume */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAudysseyVolumePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eBrcm3DSurround,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eBrcm3DSurround][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in AudysseyV olume */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eBrcm3DSurroundPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eFWMixerPostProc,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eFWMixerPostProc][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in FWMixer */
			{
				BRAP_AF_P_AlgoId_eMixerTaskFrameSync,
				BRAP_AF_P_AlgoId_eDecodeTsm,
				
				BRAP_AF_P_AlgoId_eFWMixerPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},

	/*BRAP_CIT_P_ProcessingType_eMonoDownmix,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eMonoDownmix][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Mono Downmix */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eMonoDownMixPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eAudysseyABX,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eAudysseyABX][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in Mono Downmix */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eAudysseyABXPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eDdre,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDdre][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DDRE */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDdrePostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eDv258,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eDv258][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in DV258 */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eDv258PostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eSrsCsdTd,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrsCsdTd][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in CSD and TD */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrsCircleSurrPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eSrsGeq,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eSrsGeq][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in SRS GEQ */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eSrsEqualizerPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
	/*BRAP_CIT_P_ProcessingType_eBtsc,*/
	{
		/*	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eBtsc][BRAP_DSPCHN_ProcessingMode_ePP] = */
		{
			3,	/* Number of nodes in BTSC encoder PP */
			{
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				
				BRAP_AF_P_AlgoId_eBtscEncPostProc,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid,
				BRAP_AF_P_AlgoId_eInvalid
			},
		},
	},
};


/*	Structure for Gfx Decode */

const BRAP_AF_P_sALGO_EXEC_INFO BRAP_sGfxDecodeExecInfo[BRAP_DSPCHN_GfxType_eMax] =
{	
	/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_GfxType_eSoftGfx] = */
	{
		1,		/* Number of nodes in Mpeg Decode */
		{
			BRAP_AF_P_AlgoId_eGfxDecode,
			BRAP_AF_P_AlgoId_eInvalid,
			
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid
		},
	}
};

/*	Structure for Secure Code Module */

const BRAP_AF_P_sALGO_EXEC_INFO BRAP_sScmDecodeExecInfo[BRAP_DSPCHN_ScmType_eMax] =
{	
	/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_ScmType_eScm_1] = */
	{
		1,		/* Number of nodes in SCM */
		{
			BRAP_AF_P_AlgoId_eScm_1,
			BRAP_AF_P_AlgoId_eInvalid,
			
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid
		},
	},

	/*	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_ScmType_eScm_2] = */
	{
		1,		/* Number of nodes in SCM */
		{
			BRAP_AF_P_AlgoId_eScm_2,
			BRAP_AF_P_AlgoId_eInvalid,
			
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid,
			BRAP_AF_P_AlgoId_eInvalid
		},
	}
	
	
};



/* ------------------------------------------------------------------------------ */
/*                       NODE BUFFER DETAILS									  */
/* ------------------------------------------------------------------------------ */

const BRAP_AF_P_sNODE_INFO BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMax] =
{
	/*	this entry will contain all the sizes (in bytes) of the Mpeg Decode algorihtm */
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegDecode] =  */
	{
		28436,											/*	ui32CodeSize */
		14336,											/*	ui32RomTableSize */
		10272,											/*	ui32InterFrameBuffSize */
		(1152+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		64,												/*	ui32UserCfgBuffSize */
		(1152+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eStatusBuffType */
		64												/*  StatusBuffSize */
		
		/*BRAP_AF_P_NodeStatusBuffType_ePresent */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAc3DecodeStage1] =  */
	{
		30000,											/*	ui32CodeSize */
		16000,											/*	ui32RomTableSize */
		6000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		600,											/*	ui32UserCfgBuffSize	 */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		200												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAc3DecodeStage2] =  */
	{
		25000,											/*	ui32CodeSize */
		8000,											/*	ui32RomTableSize */
		65000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacDecodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacDecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0] =  */
	{ 
		33792,											/*	ui32CodeSize */
		35720,											/*	ui32RomTableSize */	
		15000,											/*	ui32InterFrameBuffSize */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		41984,											/*	ui32InterStageGenericBuffSize */
		81920,											/*	ui32ScratchBuffSize */
		1024,											/*	ui32UserCfgBuffSize		 */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		400,											/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1] =  */
	{ 
		40960,											/*	ui32CodeSize */
		15288,											/*	ui32RomTableSize */	
		75700,											/*	ui32InterFrameBuffSize */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		41984,											/*	ui32InterStageGenericBuffSize */
		81920,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0,												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2] =  */
	{ 
		33792,											/*	ui32CodeSize */
		12500,											/*	ui32RomTableSize */	
		76700,											/*	ui32InterFrameBuffSize */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		41984,											/*	ui32InterStageGenericBuffSize */
		81920,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0,												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3] =  */
	{ 
		15240,											/*	ui32CodeSize */
		12500,											/*	ui32RomTableSize */	
		19400,											/*	ui32InterFrameBuffSize */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		41984,											/*	ui32InterStageGenericBuffSize */
		81920,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0,												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4] =  */
	{ 
		7000,											/*	ui32CodeSize */
		2048,											/*	ui32RomTableSize */	
		10240,											/*	ui32InterFrameBuffSize */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		41984,											/*	ui32InterStageGenericBuffSize */
		81920,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(12300+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0,												/*  FwStatusBuffSize */
	},


	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdpDecodeStage1] =  */
	{
		30000,											/*	ui32CodeSize */
		16000,											/*	ui32RomTableSize */
		6000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		600,											/*	ui32UserCfgBuffSize	 */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		200												/*  FwStatusBuffSize */
	
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdpDecodeStage2] =  */
	{
		25000,											/*	ui32CodeSize */
		8000,											/*	ui32RomTableSize */
		65000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize		 */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */		
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdLosslessDecodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdLosslessDecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eLpcmCustomDecode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eBdLpcmDecode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDvdLpcmDecode] =  */
	{	
		23000,											/*	ui32CodeSize */
		4000,											/*	ui32RomTableSize */	
		2000,											/*	ui32InterFrameBuffSize */
		(5000+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		4300,											/*	ui32UserCfgBuffSize		 */
		(5000+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		200,											/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eHdDvdLpcmDecode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegMcDecode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWmaStdDecode] =  */
	{	
		36000,											/*	ui32CodeSize */
		34000,											/*	ui32RomTableSize */	
		34000,											/*	ui32InterFrameBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		7000,											/*	ui32ScratchBuffSize */
		512,											/*	ui32UserCfgBuffSize		 */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		400,											/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWmaProStdDecodeStage1] =  */
	 {
        27000,                                          /*  ui32CodeSize */
        12500,                                          /*  ui32RomTableSize */
        200,                                            /*  ui32InterFrameBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        60000,                                          /*  ui32InterStageGenericBuffSize */
        76000,                                          /*  ui32ScratchBuffSize */
        496,                                            /*  ui32UserCfgBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BRAP_AF_P_InterFrameBuffType_ePresent,			/*  eInterFrameBuffType */
        BRAP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        400                                             /*  FwStatusBuffSize */
     },


	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWmaProStdDecodeStage2] =  */
	{
        28000,                                          /*  ui32CodeSize */
        20000,                                          /*  ui32RomTableSize */
        160000,                                         /*  ui32InterFrameBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        13000,											/*  ui32InterStageGenericBuffSize */
        0,												/*  ui32ScratchBuffSize */
        0,												/*  ui32UserCfgBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BRAP_AF_P_InterFrameBuffType_ePresent,			/*  eInterFrameBuffType */
        BRAP_AF_P_FwStatus_eShared,                     /*  eFwStatusBuffType */
        0												/*  FwStatusBuffSize */
     },	

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMlpDecode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdp71DecodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdp71DecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsDecodeStage1] =  */
	{   7000,	                                        /*  ui32CodeSize */
        100,                                            /*  ui32RomTableSize */
        100,											/*  ui32InterFrameBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        2048,											/*  ui32InterStageGenericBuffSize */
        100,											/*  ui32ScratchBuffSize */
        100,											/*  ui32UserCfgBuffSize */
        (4500+BRAP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BRAP_AF_P_InterFrameBuffType_ePresent,			/*  eInterFrameBuffType */
        BRAP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        200	 
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsDecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsDecodeStage3] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsLbrDecodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsLbrDecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsLbrDecodeStage3] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdDecodeStage0] =  */
	{
		37888,											/*	ui32CodeSize */
		81920,											/*	ui32RomTableSize */
		7168,											/*	ui32InterFrameBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		54000,											/*	ui32InterStageGenericBuffSize */
		51200,											/*	ui32ScratchBuffSize */
		1100,											/*	ui32UserCfgBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		200												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdDecodeStage1] =  */
	{
		37888,											/*	ui32CodeSize */
		35840,											/*	ui32RomTableSize */
		60000,											/*	ui32InterFrameBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		54000,											/*	ui32InterStageGenericBuffSize */
		51200,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdDecodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdDecodeStage3] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdDecodeStage4] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePcmWavDecode] =  */
	{
		7600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		100,											/*	ui32InterFrameBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		1024,											/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAmrDecode] =  */
	{
		30720,											/*	ui32CodeSize */
		30720,											/*	ui32RomTableSize */
		2048,											/*	ui32InterFrameBuffSize */
		(160+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		256,											/*	ui32UserCfgBuffSize */
		(160+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDraDecode] =  */
	{
		30720,											/*	ui32CodeSize */
		38912,											/*	ui32RomTableSize */
		39936,											/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		38912,											/*	ui32ScratchBuffSize */
		128,											/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealAudioLbrDecode] =  */
	{
		30720,											/*	ui32CodeSize */
		30720,											/*	ui32RomTableSize */
		18432,											/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		4096,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		200,											/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		64												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0] =  */
	{
		46080,											/*	ui32CodeSize */
		16000,											/*	ui32RomTableSize */
		14336,											/*	ui32InterFrameBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		12400,											/*	ui32InterStageGenericBuffSize */
		25500,											/*	ui32ScratchBuffSize */
		512,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1] =  */
	{
		46080,											/*	ui32CodeSize */
		7680,											/*	ui32RomTableSize */
		48600,											/*	ui32InterFrameBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		12400,											/*	ui32InterStageGenericBuffSize */
		200,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2] =  */
	{
		46080,											/*	ui32CodeSize */
		10240,											/*	ui32RomTableSize */
		20992,											/*	ui32InterFrameBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		12400,											/*	ui32InterStageGenericBuffSize */
		200,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3] =  */
	{
		46080,											/*	ui32CodeSize */
		6000,											/*	ui32RomTableSize */
		76800,											/*	ui32InterFrameBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		12400,											/*	ui32InterStageGenericBuffSize */
		200,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared	,					/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMs10DdpDecodeStage1] =  */
	{
		30000,											/*	ui32CodeSize */
		16000,											/*	ui32RomTableSize */
		6000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		600,											/*	ui32UserCfgBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		150												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMs10DdpDecodeStage2] =  */
	{
		25000,											/*	ui32CodeSize */
		8000,											/*	ui32RomTableSize */
		65000,											/*	ui32InterFrameBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4*8,				/*	ui32InterStageIoBuffSize */
		40000,											/*	ui32InterStageGenericBuffSize */
		15360,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(5250+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		8,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	Algo inits for the frame syncs of the decoder algorithms */
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegFrameSync] =  */
	{
		29696,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegMcFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAdtsFrameSync] =  */
	{
		25600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eLoasFrameSync] =  */
	{
		25600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWmaStdFrameSync] =  */
	{
		26624,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWmaProFrameSync] =  */
	{
		26624,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAc3FrameSync] =  */
	{
		28672,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdpFrameSync] =  */
	{ 	
	    28672,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdp71FrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsFrameSync] =  */	
	{ 	
	    25600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsLbrFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdFrameSync] =  */
	{ 	
	    25600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdFrameSync_1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsHdHdDvdFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdLosslessFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMlpFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMlpHdDvdFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePesFrameSync] =  */
	{ 	
	    25000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eBdLpcmFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eHdDvdLpcmFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDvdLpcmFrameSync] =  */
	{ 	
	    26624,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDvdLpcmFrameSync_1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },


	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePcmWavFrameSync] =  */
	{ 
		24576,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAmrFrameSync] =  */
	{ 
		25000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDraFrameSync] =  */
	{ 
		25000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealAudioLbrFrameSync] =  */
	{ 
		25000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500 ,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300 ,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMs10DdpFrameSync] =  */
	{ 
		28500,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		6500,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},	

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMixerTaskFrameSync] =  */
	{ 
		24500,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		480,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		7300,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},	

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDecodeTsm] =  */
	{
		6600,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		100,											/*	ui32InterFrameBuffSize */
		1152*4*2,										/*	ui32InterStageIoBuffSize */
		400,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		72,												/*	ui32UserCfgBuffSize */
		1152*4,											/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		400												/*  FwStatusBuffSize */
		
	},
	
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eEncodeTsm] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePassthroughTsm] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSystemDecodeTask] =  */
	{ 	10000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSystemGraphicTask] =  */
	{ 	10000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSystemVideoTask] =  */
	{ 	10000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSCMTask] =  */
	{	10000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent, 					/*	eFwStatusBuffType */
		0												/*	FwStatusBuffSize */
	},
	
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAc3Encode] =  */
	{ 	44000,											/*	ui32CodeSize */
		12000,											/*	ui32RomTableSize */
		56000,											/*	ui32InterFrameBuffSize */
		(12000 +BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		85000,											/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		12000*4,										/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL2EncodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL2EncodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL3EncodeStage1] =  */
	{ 	
		37000,											/*	ui32CodeSize */
		28000,											/*	ui32RomTableSize */
		25500,											/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		1024*4,											/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL3EncodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacLcEncodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacLcEncodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeEncodeStage1] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeEncodeStage2] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeEncodeStage3] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsEncode] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsBroadcastEncode] =  */
	{	
		21504,											/* ui32CodeSize */
		41984,											/* ui32RomTableSize */
		26624,											/* ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/* ui32InterStageIoBuffSize */
		2048,											/* ui32InterStageGenericBuffSize */
		0,												/* ui32ScratchBuffSize */
		200,											/* ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/* ui32MaxSizePerChan */
		1,												/* ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/* eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/* eFwStatusBuffType */
		400												/* FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSbcEncode] =  */
	{	
		10240,											/* ui32CodeSize */
		1024,											/* ui32RomTableSize */
		2048,											/* ui32InterFrameBuffSize */
		(37400+BRAP_AF_P_EXTRA_SAMPLES)*1,				/* ui32InterStageIoBuffSize */
		2048,											/* ui32InterStageGenericBuffSize */
		256,											/* ui32ScratchBuffSize */
		40,												/* ui32UserCfgBuffSize */
		(37400+BRAP_AF_P_EXTRA_SAMPLES),				/* ui32MaxSizePerChan */
		1,												/* ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/* eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/* eFwStatusBuffType */
		0												/* FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMs10DDTranscode] =  */
	{
		32000,											/*	ui32CodeSize */
		10000,											/*	ui32RomTableSize */
		54000,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		40000,											/*	ui32ScratchBuffSize */
		32,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAc3EncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL3EncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMpegL2EncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacLcEncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAacHeEncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsEncFrameSync] =  */
		{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePassThru] =  */
	{
		6000,											/*	ui32CodeSize */
		220,											/*	ui32RomTableSize */
		100,											/*	ui32InterFrameBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		32768,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		4,												/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		400												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePtAuxDataGenStg] =  */
	{
		10000,											/*	ui32CodeSize */
		1000,											/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		2048,											/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		2048*4,											/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrsTruSurroundPostProc] =  */
	{
		13312,											/*	ui32CodeSize */
		256,											/*	ui32RomTableSize */
		2000,											/*	ui32InterFrameBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrcPostProc] =  */
	{
		10000,											/*	ui32CodeSize */
		2000,											/*	ui32RomTableSize */
		5000,											/*	ui32InterFrameBuffSize */
		(8192+BRAP_AF_P_EXTRA_SAMPLES)*4*6,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		20,												/*	ui32UserCfgBuffSize */
		(8192+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdbmPostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDownmixPostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eCustomSurroundPostProc] =  */
	{
		7168,											/*	ui32CodeSize */
		8000,											/*	ui32RomTableSize */
		1500,											/*	ui32InterFrameBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		60,												/*	ui32UserCfgBuffSize	 */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0	 
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eCustomBassPostProc] =  */
	{
		7168,											/*	ui32CodeSize */
		5120 ,											/*	ui32RomTableSize */
		800,											/*	ui32InterFrameBuffSize */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		64,												/*	ui32UserCfgBuffSize	 */
		(2048+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0	 
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eKaraokeCapablePostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eCustomVoicePostProc] =  */ 
	{
		25600,											/*	ui32CodeSize */
		33000,											/*	ui32RomTableSize */
		14000,											/*	ui32InterFrameBuffSize */
		(256*4+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		1700,											/*	ui32UserCfgBuffSize	 */
		(256*4+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePeqPostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAvlPostProc] =  */
	{
		10240 ,											/*	ui32CodeSize */
		300 ,											/*	ui32RomTableSize */
		512,											/*	ui32InterFrameBuffSize */
		(1152*4+BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		60,												/*	ui32UserCfgBuffSize*/
		(1152*4+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePl2PostProc] =  */
	{
		18000,											/*	ui32CodeSize */
		1536,											/*	ui32RomTableSize */
		3072,											/*	ui32InterFrameBuffSize */
		(256*4+BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		64,												/*	ui32UserCfgBuffSize */
		(256*4+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eXenPostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eBbePostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDsolaPostProc] =  */
	{
		10000,											/*	ui32CodeSize */
		1536,											/*	ui32RomTableSize */
		14000,											/*	ui32InterFrameBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		60000,											/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize	*/
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDtsNeoPostProc] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDDConvert] =  */
	{
		14336,											/*	ui32CodeSize */
		4096,											/*	ui32RomTableSize */
		2048,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		1024,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize	*/
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAudioDescriptorFadePostProc] =  */
	{
		7168,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		48,												/*	ui32UserCfgBuffSize	*/
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAudioDescriptorPanPostProc] =  */
	{
		7168,											/*	ui32CodeSize */
		4096,											/*	ui32RomTableSize */
		256,											/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		48,												/*	ui32UserCfgBuffSize	*/
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePCMRouterPostProc] =  */
	{
		12000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize	*/
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eWMAPassThrough] =  */
	{
		6000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		0,												/*	ui32InterStageIoBuffSize */
		13000,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize	*/
		0,												/*	ui32MaxSizePerChan */
		0,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eAbsent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrsTruSurroundHDPostProc] =  */
	{
		13312,											/*	ui32CodeSize */
		3072,											/*	ui32RomTableSize */
		800,											/*	ui32InterFrameBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		4096,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		300,											/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrsTruVolumePostProc] =  */
	{
		20000,											/*	ui32CodeSize */
		8192,											/*	ui32RomTableSize */
		15360,											/*	ui32InterFrameBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4*2,				/*	ui32InterStageIoBuffSize */
		4096,											/*	ui32InterStageGenericBuffSize */
		10,												/*	ui32ScratchBuffSize */
		360,											/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDolbyVolumePostProc] =  */
	{
		40000,											/*	ui32CodeSize */
		200000,											/*	ui32RomTableSize */
		20000,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		114700,											/*	ui32ScratchBuffSize */
		200,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAudysseyVolumePostProc] =  */
	{
		40000,											/*	ui32CodeSize */
		5000,											/*	ui32RomTableSize */
		107000,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		120,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		200												/*  FwStatusBuffSize */
	},

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eBrcm3DSurroundPostProc] =  */
	{
		6000,											/*	ui32CodeSize */
		12624,											/*	ui32RomTableSize */
		2400,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		0,												/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		16,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eFWMixerPostProc] =  */
	{
		40000,											/*	ui32CodeSize */
		4152,											/*	ui32RomTableSize */
		12400,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		1024,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,												/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMonoDownMixPostProc] =  */
	{
		6000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		128,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		20,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMs10DDConvert] =  */
	{
		15000,											/*	ui32CodeSize */
		2600,											/*	ui32RomTableSize */
		1000,											/*	ui32InterFrameBuffSize */
		(1536 + BRAP_AF_P_EXTRA_SAMPLES)*4*1,			/*	ui32InterStageIoBuffSize */
		128,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		20,												/*	ui32UserCfgBuffSize */
		(1536+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eAudysseyABXPostProc] = */
	{
		22500,											/*	ui32CodeSize */
		2560,											/*	ui32RomTableSize */
		6144,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		1024,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrsCircleSurrPostProc] =  */
	{
		22500,											/*	ui32CodeSize */
		1024,											/*	ui32RomTableSize */
		6144,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*7,			/*	ui32InterStageIoBuffSize */
		4096,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		200,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		7,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eSrsEqualizerPostProc] =  */
	{
		22500,											/*	ui32CodeSize */
		10240,											/*	ui32RomTableSize */
		13312,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*2,			/*	ui32InterStageIoBuffSize */
		4096,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		1300,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		2,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDdrePostProc] =  */
	{
		22000,											/*	ui32CodeSize */
		6500,											/*	ui32RomTableSize */
		45000,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		200,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eDv258PostProc] =  */
	{
		30000,											/*	ui32CodeSize */
		2500,											/*	ui32RomTableSize */
		20000,											/*	ui32InterFrameBuffSize */
		(4096 + BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eBtscEncPostProc] =  */
	{
		10000,											/*	ui32CodeSize */
		3000,											/*	ui32RomTableSize */
		1800,											/*	ui32InterFrameBuffSize */
		(4096 + BRAP_AF_P_EXTRA_SAMPLES)*4*1,			/*	ui32InterStageIoBuffSize */
		1024,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(4096+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eAbsent,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},



	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_ePostProcFrameSync] =  */
	{ 0,0,0,0,0,0,0,0,0,0,0,0 }, 

	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eGfxDecode] =  */
	{
		25000,											/*	ui32CodeSize */
		6200,											/*	ui32RomTableSize */
		48000,											/*	ui32InterFrameBuffSize */
		(6144 + BRAP_AF_P_EXTRA_SAMPLES)*4*6,			/*	ui32InterStageIoBuffSize */
		1024,											/*	ui32InterStageGenericBuffSize */
		100,											/*	ui32ScratchBuffSize */
		164,											/*	ui32UserCfgBuffSize */
		(6144+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		6,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		12												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealVideo9PLF] =  */
	{
		20000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		200000,											/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		1536,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		100,											/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent, 					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */

	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealVideo9Stage1] =  */
	{
		20000,											/*	ui32CodeSize */
		0,												/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eShared,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealVideo9Stage2] =  */
	{
		42000,											/*	ui32CodeSize */
		200000,											/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eShared,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealVideo9Stage3] =  */
	{
		30000,											/*	ui32CodeSize */
		20000,											/*	ui32RomTableSize */
		0,												/*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,											    /*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eShared,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eRealVideo9PPD] =  */
	{
		20000,											/*	ui32CodeSize */
		20000,											/*	ui32RomTableSize */
		0,											    /*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		0,												/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_eShared,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_eShared,						/*	eFwStatusBuffType */
		0												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eScm_1] =  */
	{
		20000,											/*	ui32CodeSize */
		20000,											/*	ui32RomTableSize */
		100,										    /*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		2000,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	},
	/*	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eScm_2] =  */
	{
		20000,											/*	ui32CodeSize */
		20000,											/*	ui32RomTableSize */
		100,										    /*	ui32InterFrameBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4*1,				/*	ui32InterStageIoBuffSize */
		2048,											/*	ui32InterStageGenericBuffSize */
		2000,											/*	ui32ScratchBuffSize */
		0,												/*	ui32UserCfgBuffSize */
		(1024+BRAP_AF_P_EXTRA_SAMPLES)*4,				/*	ui32MaxSizePerChan */
		1,												/*	ui32MaxNumChansSupported */
		BRAP_AF_P_InterFrameBuffType_ePresent,			/*	eInterFrameBuffType */
		BRAP_AF_P_FwStatus_ePresent,					/*	eFwStatusBuffType */
		100												/*  FwStatusBuffSize */
	}
};
/*
	This structure is used in the PI to determine the input-output channel mapping for all the algorithms 
	for configuring the FORK matrix configuration.

	Bypass  =  Output number of channel is the same as that of input number channel.
	eFixed  =  Output number of channel is fixed to a particular value.

*/
const BRAP_AF_P_PpOutMode BRAP_sPpOutMode[/*BRAP_CIT_P_ProcessingType_eMax*/] =
{
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Ddbm*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Dts Neo*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*AVL*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eCompressed},	/*DD Convert*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_e5_1_PCM},		/*Pl2*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eStereo_PCM},	/*SrsXt*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Xen*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Bbe*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*SRC*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Custom Surround*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Custom Bass*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eStereo_PCM},  /*Custom Voice*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Peq*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Downmix0*/    
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eStereo_PCM},  /*Ad Fade*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eStereo_PCM},	/*Ad Pan*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*PCM router*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eCompressed},	/*WMA Passthrough*/
    {BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*DSOLA*/
    {BRAP_AF_P_OutModeType_eFixed,	BRAP_AF_P_DistinctOpType_eStereo_PCM},	/*SRSHD*/
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Generic pass through*/
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*SRS Volume IQ */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Dolby Volume*/
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Audyssey Volume*/
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*3D Surround Volume*/
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Fw mixer */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Mono Downmix */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*Audyssey ABX */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*DDRE */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*DV258 */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*SRS CSD */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*SRS GEQ */
	{BRAP_AF_P_OutModeType_eBypass,	BRAP_AF_P_DistinctOpType_eInvalid},		/*BTSC */
};

