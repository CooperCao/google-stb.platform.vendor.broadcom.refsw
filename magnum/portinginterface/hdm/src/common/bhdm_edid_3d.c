/******************************************************************************
* (c) 2016 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bhdm_priv.h"
#include "bhdm_edid_3d.h"


BDBG_MODULE(BHDM_EDID_3D) ;


typedef struct _BHDM_EDID_HDMI_3D_FORMAT_
{
	BFMT_VideoFmt eVideoFmt;
} BHDM_EDID_HDMI_3D_FORMAT;

static const BHDM_EDID_HDMI_3D_FORMAT BHDM_EDID_3DFormats[]=
{
	{	BFMT_VideoFmt_e720p_60Hz_3DOU_AS	},		   /* HD 3D 720p */
	{	BFMT_VideoFmt_e720p_50Hz_3DOU_AS	},		   /* HD 3D 720p50 */
	{	BFMT_VideoFmt_e720p_30Hz_3DOU_AS	},		   /* HD 3D 720p30 */
	{	BFMT_VideoFmt_e720p_24Hz_3DOU_AS	},		   /* HD 3D 720p24 */
	{	BFMT_VideoFmt_e1080p_24Hz_3DOU_AS	},		   /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
	{	BFMT_VideoFmt_e1080p_30Hz_3DOU_AS	}		   /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */
};


/******************************************************************************
Summary:
Set all associated 3D formats
*******************************************************************************/
static void BHDM_EDID_P_SetSupportedHdmi3DFormats(const BHDM_Handle hHDMI)
{
	BFMT_VideoFmt eVideoFmt = 0 ;
#if BHDM_CONFIG_DEBUG_EDID_3DSTRUCT
	BFMT_VideoInfo	 stVideoFmtInfo ;
#endif

	uint8_t j=0;
	uint8_t NumBcmSupported3DFormats ;
	bool OverUnderSupport  ;

	NumBcmSupported3DFormats =
		sizeof(BHDM_EDID_3DFormats) / sizeof(BHDM_EDID_3DFormats[0]) ;

	for (eVideoFmt = 0 ; eVideoFmt < BFMT_VideoFmt_eMaxCount ; eVideoFmt++)
	{
		/* check HDMI formats only */
		if (!BFMT_SUPPORT_HDMI(eVideoFmt))
			continue ;

		/* if format is not supported, continue  */
		if (!hHDMI->AttachedEDID.BcmSupportedVideoFormats[eVideoFmt])
			continue;

		/* if frame packing not supported, continue */
		if (!(hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING))
			continue ;

		OverUnderSupport = false ;

		switch (eVideoFmt)
		{
		case BFMT_VideoFmt_e1080p_24Hz:
			for (j=0 ; j < NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e1080p_24Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e1080p_24Hz_3DOU_AS] = true;
					break;
				}
			break;

		case BFMT_VideoFmt_e1080p_30Hz:
			for (j=0 ; j < NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e1080p_30Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e1080p_30Hz_3DOU_AS] = true;
					break;
				}
			break;

		case BFMT_VideoFmt_e720p:
			for (j=0 ; j < NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e720p_60Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e720p_60Hz_3DOU_AS] = true;
					break;
				}
			break;

		case BFMT_VideoFmt_e720p_50Hz:
			for (j=0 ; j < NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e720p_50Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e720p_50Hz_3DOU_AS] = true;
					break;
				}
			break;

		case BFMT_VideoFmt_e720p_24Hz:
			for (j=0 ; j < NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e720p_24Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e720p_24Hz_3DOU_AS] = true;
					break;
				}
			break;

		case BFMT_VideoFmt_e720p_30Hz:
			for (j=0 ; NumBcmSupported3DFormats ; j++)
				if (BHDM_EDID_3DFormats[j].eVideoFmt == BFMT_VideoFmt_e720p_30Hz_3DOU_AS)
				{
					OverUnderSupport = true ;
					hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e720p_30Hz_3DOU_AS] = true;
					break;
				}
			break;

		default:
			break;
		}/* end switch */

#if BHDM_CONFIG_DEBUG_EDID_3DSTRUCT
		if (OverUnderSupport)
		{
			/* there is a separate FMT enumeration for OU formats; just print corresponding 2D format */
			BFMT_GetVideoFormatInfo(eVideoFmt, &stVideoFmtInfo) ;
			BDBG_LOG(("3D Top/Bottom support found for %s", stVideoFmtInfo.pchFormatStr)) ;
		}
#else
		/*
			suppress compilation warnings when
			BHDM_CONFIG_DEBUG_EDID_3DSTRUCT is enabled
		*/
		BSTD_UNUSED(OverUnderSupport) ;
#endif
	}

	return;
}


/******************************************************************************
Summary:
Return all 3D video formats supported by the attached monitor.
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupported3DFormats(
	const BHDM_Handle hHDMI,					 /* [in] HDMI handle  */
	BHDM_EDID_3D_Structure_ALL *_3DFormats					 /* [out] supported true/false */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetSupported3DFormats) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(_3DFormats, 0, sizeof(BHDM_EDID_3D_Structure_ALL)) ;

	/* Need HDMI for 3D Support */
	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
		goto done ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	if (hHDMI->AttachedEDID.Bcm3DFormatsChecked == 0)
	{
		rc = BERR_TRACE(BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE) ;
		goto done ;
	}

	BKNI_Memcpy(_3DFormats, &hHDMI->AttachedEDID.BcmSupported3DFormats,
		sizeof(hHDMI->AttachedEDID.BcmSupported3DFormats)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetSupported3DFormats) ;
	return rc ;

} /* BHDM_EDID_GetSupported3DFormats */


/******************************************************************************
Summary:
Parse the VSDB 3D Structure Bytes
*******************************************************************************/
static void BHDM_EDID_P_ParseVSDB_3DStruct(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t *index)
{
	hHDMI->AttachedEDID.BcmSupported3DStructureAll =
		  (hHDMI->AttachedEDID.Block[DataBlockIndex + *index] << 8)
		| hHDMI->AttachedEDID.Block[DataBlockIndex + *index + 1] ;
#if BHDM_CONFIG_DEBUG_EDID_3DSTRUCT
	BDBG_LOG(("3D_Structure_ALL: %#04x located at offset %#02x (%d)",
		hHDMI->AttachedEDID.BcmSupported3DStructureAll, DataBlockIndex + *index, DataBlockIndex + *index)) ;
#endif
	*index = *index + 2 ;

#if BHDM_CONFIG_DEBUG_EDID_3DSTRUCT
	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING) {
		BDBG_LOG(("Rx Supports 3D FramePacking")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FIELD_ALTERNATIVE) {
		BDBG_LOG(("Rx Supports 3D Field Alternative")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LINE_ALTERNATIVE) {
		BDBG_LOG(("Rx Supports 3D Line Alternative")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_FULL) {
		BDBG_LOG(("Rx Supports 3D Side by Side Full")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH) {
		BDBG_LOG(("Rx Supports 3D Line Depth")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH_GFX) {
		BDBG_LOG(("Rx Supports 3D LDepth + Graphics")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM) {
		BDBG_LOG(("Rx Supports 3D Top and Bottom")) ;
	}

	if (hHDMI->AttachedEDID.BcmSupported3DStructureAll & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ) {
		BDBG_LOG(("Rx Supports 3D Side by Side Half")) ;
	}
#endif

	return;
}


/******************************************************************************
Summary:
Parse the VSDB 3D Mask bytes
*******************************************************************************/
static void BHDM_EDID_P_ParseVSDB_3DMask(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t *index)
{
	uint16_t Edid3DMask ;
	uint16_t i;
	BHDM_EDID_P_VideoDescriptor  *pVideoDescriptor ;

	Edid3DMask =
		  (hHDMI->AttachedEDID.Block[DataBlockIndex + *index] << 8)
		| hHDMI->AttachedEDID.Block[DataBlockIndex + *index + 1] ;
#if BHDM_CONFIG_DEBUG_EDID_3DMASK
	BDBG_LOG(("3D_Structure_MASK: %#04x located at offset %#02x (%d)",
		Edid3DMask, DataBlockIndex + *index, DataBlockIndex + *index)) ;

	if (Edid3DMask)
		BDBG_LOG(("Rx Supports 3D_Structure_ALL for the following formats:"));
#endif

	*index = *index + 2 ;

	pVideoDescriptor = BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList);
	for (i=0; i<16 && pVideoDescriptor; i++)
	{
		if (hHDMI->AttachedEDID.First16VideoDescriptorsMask & (1 << i))
		{
			if (Edid3DMask & (1 << i))
			{
				hHDMI->AttachedEDID.BcmSupported3DFormats[(BFMT_VideoFmt) pVideoDescriptor->eVideoFmt] |=
					hHDMI->AttachedEDID.BcmSupported3DStructureAll;

#if BHDM_CONFIG_DEBUG_EDID_3DMASK
				BDBG_LOG((" 	 %s",
					(char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr((uint8_t) pVideoDescriptor->VideoIdCode)));
#endif
			}

			pVideoDescriptor = BLST_Q_NEXT(pVideoDescriptor, link);
		}
	}

	return;
}


/******************************************************************************
Summary:
Parse the HDMI Extended Resolution VICs
*******************************************************************************/
void BHDM_EDID_P_ParseVSDB_HdmiVICs(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t *offset, uint8_t DataBlockLength)
{
	uint8_t i ;
	uint8_t hdmiVic ;
	BFMT_VideoInfo *pVideoFormatInfo ;

	BSTD_UNUSED(DataBlockLength) ;

	BDBG_MSG(("HDMI Extended Resolution Formats:")) ;
	for (i = 0 ; i < hHDMI->AttachedEDID.RxVSDB.HDMI_VIC_Len; i++)
	{
		hdmiVic = hHDMI->AttachedEDID.Block[DataBlockIndex + *offset] ;
		pVideoFormatInfo = (BFMT_VideoInfo *) NULL ;
		switch (hdmiVic)
		{
#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
		case BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2997_30Hz :
			hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e3840x2160p_30Hz] = true ;
			pVideoFormatInfo =
				(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_e3840x2160p_30Hz) ;
			break ;

		case BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_25Hz :
			hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e3840x2160p_25Hz] = true ;
			pVideoFormatInfo =
				(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_e3840x2160p_25Hz) ;
			break ;

		case BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2398_24Hz :
			hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e3840x2160p_24Hz] = true ;
			pVideoFormatInfo =
				(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_e3840x2160p_24Hz) ;
			break ;

		case BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_SMPTE_24Hz :
			hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_e4096x2160p_24Hz] = true ;
			pVideoFormatInfo =
				(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_e4096x2160p_24Hz) ;
			break ;
#endif
		default :
			BDBG_WRN(("No Support for HDMI VIC %d", hdmiVic)) ;
			break ;
		}

#if BDBG_DEBUG_BUILD
		if (pVideoFormatInfo)
		{
			BDBG_MSG(("Found BCM supported HDMI Extended Format <%s>",
				pVideoFormatInfo->pchFormatStr)) ;
		}
#endif
		(*offset)++ ;
	}
	BDBG_MSG((" ")) ;
}



/******************************************************************************
Summary:
Parse the 2D VIC Order _ 3D Structure Bytes
*******************************************************************************/
void BHDM_EDID_P_ParseVSDB_2DVIC3DStructureBytes(const BHDM_Handle hHDMI,
	uint8_t DataBlockIndex, uint8_t *offset, uint8_t DataBlockLength, uint8_t *remaining3dFmtBytes)
{
	uint8_t i ;
	uint8_t offsetByte;
	uint8_t _2dVicOrderX, _3dStructureX, _3dDetailX = 0x0F ;
	BHDM_EDID_P_VideoDescriptor  *pVideoDescriptor ;
	char *pchFormatStr ;
	BFMT_VideoFmt eVideoFmt ;

	BSTD_UNUSED(DataBlockLength) ;

	do
	{
		offsetByte = hHDMI->AttachedEDID.Block[DataBlockIndex + *offset] ;
		(*offset)++ ;

		_2dVicOrderX = (offsetByte & 0xF0) >> 4 ;
		_3dStructureX = (offsetByte & 0x0F);
		if (_3dStructureX == BHDM_EDID_VSDB_3D_STRUCTURE_X_SBS_HALF)
		{
			offsetByte = hHDMI->AttachedEDID.Block[DataBlockIndex + *offset] ;
			(*offset)++;
			_3dDetailX = (offsetByte & 0xF0) >> 4 ;
		}

		/* Set the supported 3D Structure for the specific 2D video format */
		pVideoDescriptor = BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList);
		for (i=0; i<16 && pVideoDescriptor; i++)
		{
			if (!(hHDMI->AttachedEDID.First16VideoDescriptorsMask & (1 << i)))
				continue;

			if (i != _2dVicOrderX)
			{
				goto next_format;
			}

			/* get eVidoeFmt and Format String for the video format in the list */
			pchFormatStr =
				(char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr((uint8_t) pVideoDescriptor->VideoIdCode)  ;
			eVideoFmt = pVideoDescriptor->eVideoFmt ;

			switch (_3dStructureX)
			{
			case BHDM_EDID_VSDB_3D_STRUCTURE_X_FRAME_PACKING:
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING;
				BDBG_MSG(("Rx supports 3D Frame Packing for %s format", pchFormatStr)) ;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_FIELD_ALTERNATIVE:
				BDBG_MSG(("Rx supports 3D Field Alternative for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FIELD_ALTERNATIVE;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_LINE_ALTERNATIVE:
				BDBG_MSG(("Rx supports 3D Line Alternative for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LINE_ALTERNATIVE;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_SBS_FULL:
				BDBG_MSG(("Rx supports 3D SidebySide Full for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_FULL;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_LDEPTH:
				BDBG_MSG(("Rx supports 3D LDepth for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_LDEPTH_GFX:
				BDBG_MSG(("Rx supports 3D LDepth GFX for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH_GFX;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_TOP_BOTTOM:
				BDBG_MSG(("Rx supports 3D TopAndBottom for %s format", pchFormatStr)) ;
				hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
					BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM;
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_SBS_HALF:
				if (_3dDetailX <= BHDM_EDID_VSDB_3D_EXT_DATA_HORIZ_SUB_SAMPLING) {
					BDBG_MSG(("Rx supports 3D SidexSide Half with Horizontal sub-sampling for %s format", pchFormatStr)) ;
					hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
						BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ;
				}
				else if (_3dDetailX <= BHDM_EDID_VSDB_3D_EXT_DATA_QUINCUNX_MATRIX) {
					BDBG_MSG(("Rx supports 3D SidexSide Half Quincunx matrix for %s format",  pchFormatStr)) ;
					hHDMI->AttachedEDID.BcmSupported3DFormats[eVideoFmt] |=
						BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_QUINC;
				}
				break;

			case BHDM_EDID_VSDB_3D_STRUCTURE_X_RESERVED:
			default:
				BDBG_MSG(("RESERVED 3D_Structure_X %x", _3dStructureX)) ;
				break;
			}

			/* Already found the corresponding 2D_VIC and set the supported 3D format.
			Stop searching and move on to the next 2D_VIC, break out of the for loop. */
			break;

next_format:
			/* Move to next supported format in the list */
			pVideoDescriptor = BLST_Q_NEXT(pVideoDescriptor, link);

		}
	}while ((*remaining3dFmtBytes)-- > 0) ;
}


/******************************************************************************
Summary:
Parse the Vendor Specific Data Block (VSDB) in the V3 Timing Extension
*******************************************************************************/
void BHDM_EDID_P_ParseVSDB3D(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t *offset, uint8_t DataBlockLength)
{
	uint8_t i;
	uint8_t offsetByte;
	uint8_t remaining3dFmtBytes;
	BHDM_EDID_P_VideoDescriptor  *pVideoDescriptor ;


	/* check for HDMI VICs and 3D support info */
	offsetByte = hHDMI->AttachedEDID.Block[DataBlockIndex + *offset] ;
	(*offset)++;


	/* retrieve 3D_present, 3D_Multi_present fields */
	hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Present = (offsetByte & 0x80) >> 7 ;
	hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Multi_Present = (offsetByte & 0x60) >> 5 ;
	hHDMI->AttachedEDID.RxVSDB.HDMI_Image_Size = (offsetByte & 0x18) >> 3 ;

	BDBG_MSG(("HDMI_3D_Present=%d, HDMI_3D_Multi_Present=%d, HDMI_Image_Size=%d",
		hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Present, hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Multi_Present,
		hHDMI->AttachedEDID.RxVSDB.HDMI_Image_Size));

	/************************
	 * Rx has 3D support.
	 * It must support all the mandatory formats below
	 *************************/
	if (hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Present)
	{
		hHDMI->AttachedEDID.BcmSupported3DFormats[BFMT_VideoFmt_e1080p_24Hz] =
			  BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING
			| BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM ;


		if ((50 >= hHDMI->AttachedEDID.MonitorRange.MinVertical) &&
			(50 <= hHDMI->AttachedEDID.MonitorRange.MaxVertical))
		{
			hHDMI->AttachedEDID.BcmSupported3DFormats[BFMT_VideoFmt_e720p_50Hz] =
			  BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING
			| BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM ;

			hHDMI->AttachedEDID.BcmSupported3DFormats[BFMT_VideoFmt_e1080i_50Hz] =
				BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ ;
		}

		if ((59 >= hHDMI->AttachedEDID.MonitorRange.MinVertical) &&
			(59 <= hHDMI->AttachedEDID.MonitorRange.MaxVertical))
		{
			hHDMI->AttachedEDID.BcmSupported3DFormats[BFMT_VideoFmt_e720p] =
				  BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING
				| BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM ;
			hHDMI->AttachedEDID.BcmSupported3DFormats[BFMT_VideoFmt_e1080i] =
				BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ ;
		}
	}


	offsetByte = hHDMI->AttachedEDID.Block[DataBlockIndex + *offset] ;
	(*offset)++;

	hHDMI->AttachedEDID.RxVSDB.HDMI_VIC_Len = (offsetByte & 0xE0) >> 5 ;
	hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Len = (offsetByte & 0x1F) ;
	remaining3dFmtBytes = hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Len;

	/* finished parsing 3D_present, 3D_Multi_present, HDMI_VIC_LEN, and HDMI_3D_LEN */
	if (*offset > DataBlockLength)
		goto done ;  /* no more optional data */

	/* Parse HDMI Formats */
	if (hHDMI->AttachedEDID.RxVSDB.HDMI_VIC_Len)
		BHDM_EDID_P_ParseVSDB_HdmiVICs(hHDMI, DataBlockIndex, offset, DataBlockLength) ;


	/********
	Parse 3D Timing/Structure

	if 3D_Multi_Present = 00
		3D_Structure_ALL_15...0 and
		3D_MASK_15..0 are not present

	if 3D_Multi_Present = 01
		3D_Structure_ALL_15...0 is present and assigns 3D formats to all the VICs listed in the first 16 entires in the EDID
		3D_MASK_15..0 is not present

	if 3D_Multi_Present = 10
		3D_Structure_ALL_15...0 and
		3D_MASK_15..0 are present and assign 3D formats to some of the VICs listed in the first 16 entires in the EDID

	if 3D_Multi_Present = 11
		Reserved for future use
		3D_Structure_ALL_15...0 and
		3D_MASK_15..0 are not present
	********/
#define BHDM_EDID_P_VSDB_3D_MULTI_NONE 0x0
#define BHDM_EDID_P_VSDB_3D_MULTI_STRUCT 0x1
#define BHDM_EDID_P_VSDB_3D_MULTI_STRUCT_AND_MASK 0x2
#define BHDM_EDID_P_VSDB_3D_MULTI_RESERVED 0x3

	switch(hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Multi_Present)
	{
	case BHDM_EDID_P_VSDB_3D_MULTI_STRUCT :
		BHDM_EDID_P_ParseVSDB_3DStruct(hHDMI, DataBlockIndex, offset) ;
		remaining3dFmtBytes -= 2;

#if BHDM_CONFIG_DEBUG_EDID_3DSTRUCT
		BDBG_LOG(("All 3D Structures are supported on the first 16th Short Video Descriptors"));
#endif
		/* Set 3D Structure support for ALL of the first 16 short video descriptors */
		pVideoDescriptor = BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList);
		for (i=0; i<16 && pVideoDescriptor; i++)
		{
			if (hHDMI->AttachedEDID.First16VideoDescriptorsMask & (1 << i))
			{
				hHDMI->AttachedEDID.BcmSupported3DFormats[(BFMT_VideoFmt) pVideoDescriptor->eVideoFmt] |=
					hHDMI->AttachedEDID.BcmSupported3DStructureAll;
				pVideoDescriptor = BLST_Q_NEXT(pVideoDescriptor, link);
			}
		}
		break ;

	case BHDM_EDID_P_VSDB_3D_MULTI_STRUCT_AND_MASK :
		BHDM_EDID_P_ParseVSDB_3DStruct(hHDMI, DataBlockIndex, offset) ;
		BHDM_EDID_P_ParseVSDB_3DMask(hHDMI, DataBlockIndex, offset) ;
		remaining3dFmtBytes -= 4;
		break;

	case BHDM_EDID_P_VSDB_3D_MULTI_NONE :
		/* do nothing */
		break ;

	default :
	case BHDM_EDID_P_VSDB_3D_MULTI_RESERVED :
		BDBG_WRN(("Use of 3D_Multi_Present Reserved Value %#x; No 3D formats detected",
			hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Multi_Present)) ;
		break ;
	}


	/* Parse any remaining 3D Timing/Structure Support (bytes) and indicate support if applicable */
	if (remaining3dFmtBytes)
		BHDM_EDID_P_ParseVSDB_2DVIC3DStructureBytes(hHDMI,
			DataBlockIndex, offset, DataBlockLength, &remaining3dFmtBytes) ;

	/* Check HDMI 3D formats for supported */
	BHDM_EDID_P_SetSupportedHdmi3DFormats(hHDMI);

done:

	hHDMI->AttachedEDID.Bcm3DFormatsChecked = true;
	return;
}


