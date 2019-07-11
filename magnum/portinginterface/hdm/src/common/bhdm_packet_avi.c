/******************************************************************************
 *  Copyright (C) 2019 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_PACKET_AVI) ;

static void BHDM_P_CEA861Code2PixelRepeat(uint8_t VideoID, uint8_t *PixelRepeat)
{
    switch (VideoID)
    {
    case  6 :   case  7 :
    case 14 :   case 15 :
    case 21 :   case 22 :
    case 29 :   case 30 :
        *PixelRepeat = BAVC_HDMI_PixelRepetition_e1x;
        break;

    case 10 :   case 11 :
    case 25 :   case 26 :
    case 35 :   case 36 :
    case 37 :   case 38 :
        *PixelRepeat = BAVC_HDMI_PixelRepetition_e4x;
        break;

    default:
        *PixelRepeat = BAVC_HDMI_PixelRepetition_eNone;
        break;
    }
}


void BHDM_VideoFmt2CEA861Code(
    BFMT_VideoFmt eVideoFmt,
    BFMT_AspectRatio eAspectRatio,
    BAVC_HDMI_PixelRepetition ePixelRepetition,
    uint8_t *pVideoID,
    uint8_t *pPixelRepeat
)
{
    switch (eVideoFmt)
    {
    case BFMT_VideoFmt_e1080i  :           /* HD 1080i */
        *pVideoID = 5 ;
        break ;

    case BFMT_VideoFmt_e720p   :           /* HD 720p */
    case BFMT_VideoFmt_e720p_60Hz_3DOU_AS:  /* 720p 60Hz 3D frame packing */
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
    case BFMT_VideoFmt_e720p_3D :          /* HD 720p 3D */
#endif
        *pVideoID = 4 ;
        break ;

    case BFMT_VideoFmt_e480p   :           /* HD 480p */
        if (eAspectRatio == BFMT_AspectRatio_e16_9)
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 3 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 15 ;
                break;
            case BAVC_HDMI_PixelRepetition_e4x:
                *pVideoID = 36 ;
                break;
            default:
                break;
            }
        }
        else   /* default 4:3 */
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 2 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 14 ;
                break;
            case BAVC_HDMI_PixelRepetition_e4x:
                *pVideoID = 35 ;
                break;
            default:
                break;
            }
        }
        break ;

    case BFMT_VideoFmt_eNTSC   :           /* 480i, NSTC-M for North America */
    case BFMT_VideoFmt_eNTSC_J :           /* 480i (Japan) */
    case BFMT_VideoFmt_ePAL_M  :           /* 525-lines (Brazil) */
        if (eAspectRatio == BFMT_AspectRatio_e16_9)
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 7 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 11 ;
                break;
            default:
                break;
            }
        }
        else   /* default 4:3 */
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 6 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 10 ;
                break;
            default:
                break;
            }
        }
        break ;

    case BFMT_VideoFmt_ePAL_B  :           /* Australia */
    case BFMT_VideoFmt_ePAL_B1 :           /* Hungary */
    case BFMT_VideoFmt_ePAL_D  :           /* China */
    case BFMT_VideoFmt_ePAL_D1 :           /* Poland */
    case BFMT_VideoFmt_ePAL_G  :           /* Europe */
    case BFMT_VideoFmt_ePAL_H  :           /* Europe */
    case BFMT_VideoFmt_ePAL_K  :           /* Europe */
    case BFMT_VideoFmt_ePAL_I  :           /* U.K. */
    case BFMT_VideoFmt_ePAL_N  :           /* Jamaica, Uruguay */
    case BFMT_VideoFmt_ePAL_NC :           /* N combination (Argentina) */
    case BFMT_VideoFmt_eSECAM  :           /* LDK/SECAM (France,Russia) */
        if (eAspectRatio == BFMT_AspectRatio_e16_9)
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 22 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 26 ;
                break;
            default:
                break;
            }
        }
        else   /* default 4:3 */
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 21 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 25 ;
                break;
            default:
                break;
            }
        }
        break ;

    case BFMT_VideoFmt_e1250i_50Hz :       /* HD 1250i 50Hz, another 1080i_50hz standard SMPTE 295M */
        BDBG_WRN(("Verify AVI Frame Video Code for 1250i Format")) ;
        /* fall through to use 1080i 50Hz Video Code */
    case BFMT_VideoFmt_e1080i_50Hz :       /* HD 1080i 50Hz, 1125 line, SMPTE 274M */
        *pVideoID = 20 ;
        break ;

    case BFMT_VideoFmt_e720p_50Hz  :       /* HD 720p 50Hz (Australia) */
    case BFMT_VideoFmt_e720p_50Hz_3DOU_AS: /* 720p 50Hz 3D Frame Packing */
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
    case BFMT_VideoFmt_e720p_50Hz_3D :     /* HD 720p 50Hz 3D */
#endif
        *pVideoID  = 19 ;
        break ;

    case BFMT_VideoFmt_e720p_24Hz:          /* 720p 24Hz */
    case BFMT_VideoFmt_e720p_24Hz_3DOU_AS:  /* 720p 24Hz 3D frame packing */
        *pVideoID  = 60 ;
        break ;

    case BFMT_VideoFmt_e720p_25Hz:          /* 720p 25Hz */
        *pVideoID  = 61 ;
        break ;

    case BFMT_VideoFmt_e720p_30Hz:          /* 720p 30Hz */
    case BFMT_VideoFmt_e720p_30Hz_3DOU_AS:  /* 720p 30Hz 3D frame packing */
        *pVideoID  = 62 ;
        break ;

    case BFMT_VideoFmt_e576p_50Hz  :       /* HD 576p 50Hz (Australia) */
        if (eAspectRatio  == BFMT_AspectRatio_e16_9)
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 18 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 30 ;
                break;
            case BAVC_HDMI_PixelRepetition_e4x:
                *pVideoID = 38 ;
                break;
            default:
                break;
            }
        }
        else   /* default 4:3 */
        {
            switch (ePixelRepetition)
            {
            case BAVC_HDMI_PixelRepetition_eNone:
                *pVideoID = 17 ;
                break;
            case BAVC_HDMI_PixelRepetition_e1x:
                *pVideoID = 29 ;
                break;
            case BAVC_HDMI_PixelRepetition_e4x:
                *pVideoID = 37 ;
                break;
            default:
                break;
            }
        }
        break ;

    case BFMT_VideoFmt_eDVI_640x480p :     /* DVI Safe mode for computer monitors */
        *pVideoID = 1 ;
        break ;

    case BFMT_VideoFmt_e1080p   :     /* HD 1080p 60Hz */
    case BFMT_VideoFmt_e1080p_60Hz_3DOU_AS:
    case BFMT_VideoFmt_e1080p_60Hz_3DLR:
        *pVideoID = 16 ;
        break ;

    case BFMT_VideoFmt_e1080p_50Hz  :     /* HD 1080p 50Hz */
        *pVideoID = 31 ;
        break ;

    case BFMT_VideoFmt_e1080p_30Hz  :     /* HD 1080p 30Hz */
    case BFMT_VideoFmt_e1080p_30Hz_3DOU_AS:
        *pVideoID = 34 ;
        break ;

    case BFMT_VideoFmt_e1080p_24Hz  :     /* HD 1080p 24Hz */
    case BFMT_VideoFmt_e1080p_24Hz_3DOU_AS:
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
    case BFMT_VideoFmt_e1080p_24Hz_3D :   /* HD 1080p 24Hz 3D */
#endif
        *pVideoID = 32 ;
        break ;

    case BFMT_VideoFmt_e1080p_25Hz  :     /* HD 1080p 25Hz */
        *pVideoID = 33 ;
        break ;

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
    case BFMT_VideoFmt_e3840x2160p_30Hz :
    case BFMT_VideoFmt_e3840x2160p_25Hz :
    case BFMT_VideoFmt_e3840x2160p_24Hz :
    case BFMT_VideoFmt_e4096x2160p_24Hz :
        BDBG_MSG(("Video ID Code of 0 used for for HDMI Extended Resolution formats"));
        *pVideoID = 0 ;
        break ;

    case BFMT_VideoFmt_e1080p_100Hz :
        BDBG_MSG(("Video ID Code of 64 used for for 1080p100"));
        *pVideoID = 64 ;
        break ;
    case BFMT_VideoFmt_e1080p_120Hz :
        BDBG_MSG(("Video ID Code of 63 used for for 1080p120"));
        *pVideoID = 63 ;
        break ;
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
    case BFMT_VideoFmt_e3840x2160p_50Hz :
        *pVideoID = 96 ;
        break ;
    case BFMT_VideoFmt_e3840x2160p_60Hz :
        *pVideoID = 97 ;
        break ;
#endif

    case BFMT_VideoFmt_eDVI_800x600p:
    case BFMT_VideoFmt_eDVI_1024x768p:
    case BFMT_VideoFmt_eDVI_1280x768p:
    case BFMT_VideoFmt_eDVI_1280x720p_50Hz:
    case BFMT_VideoFmt_eDVI_1280x720p:
    case BFMT_VideoFmt_eDVI_1280x1024p_60Hz :
        *pVideoID = 0;
        BDBG_MSG(("Video ID Code of 0 used for DVI formats"));
        break;

    default :
        *pVideoID = 0 ;
        BDBG_ERR(("BFMT_VideoFmt %d NOT IMPLEMENTED",  eVideoFmt)) ;
        break ;
    }

    BDBG_MSG(("BFMT_eVideoFmt %d ==> CEA 861 Video ID Code: %d",
        eVideoFmt, *pVideoID)) ;

    BHDM_P_CEA861Code2PixelRepeat(*pVideoID, pPixelRepeat);
}

/******************************************************************************
Summary:
Set/Enable the Auxillary Video Information Info frame to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetAVIInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AviInfoFrame *stAviInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;
	BAVC_HDMI_AviInfoFrame newAviInfoFrame ;
	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;
	uint8_t M1M0 ;
	uint8_t VideoID = 1 ;
	uint8_t PixelRepeat ;
	uint8_t C1C0 ;
	uint8_t Y1Y0 ;
	uint8_t EC2EC1EC0 = 0 ;
	bool yCbCrColorspace ;
	BAVC_ColorRange eColorRange ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make a local copy of AVI  */
	BKNI_Memcpy(&newAviInfoFrame, stAviInfoFrame,
		sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eAVI_ID  ;

	PacketType	  = BAVC_HDMI_PacketType_eAviInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_AviInfoFrameVersion ;
	PacketLength  = 13 ;

	/* Override AVI infoFrame info. Use settings from the AVI InfoFrame passed in */
	if (stAviInfoFrame->bOverrideDefaults)
	{
		/* Set RGB or YCrCb Colorspace */
		Y1Y0 = stAviInfoFrame->ePixelEncoding;

		/* Set Colorimetry */
		C1C0 = stAviInfoFrame->eColorimetry;

		if (stAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
			EC2EC1EC0 = stAviInfoFrame->eExtendedColorimetry ;

		newAviInfoFrame.eRGBQuantizationRange = stAviInfoFrame->eRGBQuantizationRange ;
		newAviInfoFrame.eYccQuantizationRange = stAviInfoFrame->eYccQuantizationRange ;

		/* Picture Aspect Ratio*/
		M1M0 = stAviInfoFrame->ePictureAspectRatio;

		/* Video ID Code & Pixel Repetition */
		VideoID = stAviInfoFrame->VideoIdCode;
		PixelRepeat = stAviInfoFrame->PixelRepeat;
	}  /* end overrideDefaults */
	else
	{
		/* Set RGB or YCrCb Colorspace */
		switch (hHDMI->DeviceSettings.stVideoSettings.eColorSpace)
		{
		case BAVC_Colorspace_eRGB :
			Y1Y0 =	BAVC_HDMI_AviInfoFrame_Colorspace_eRGB ;
			break ;

		default :
			/* YCbCr output  */

#if BHDM_HAS_HDMI_20_SUPPORT
			/* all YCbCr encodings are supported in HDMI 2.0 enabled chips */
			Y1Y0 = hHDMI->DeviceSettings.stVideoSettings.eColorSpace ;
 #else
			Y1Y0 =  BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444 ;
 #endif
			break ;
		}

		yCbCrColorspace =
			hHDMI->DeviceSettings.stVideoSettings.eColorSpace != BAVC_Colorspace_eRGB ;

		/* set RGB and YCC quantization (colorRange) to defaults;
		    then update based on the in-use colorspace
		*/
		newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault ;
		newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited ;

		if (yCbCrColorspace)
		{
			eColorRange = hHDMI->DeviceSettings.stVideoSettings.eColorRange ;
			switch (eColorRange)
			{
			default :
				BDBG_WRN(("Unknown Ycc Quantization Range%d... using Limited Range", eColorRange)) ;

				/**************************/
				/*       FALL THRHOUGH    */
				/* Use Default / Limited */
				/**************************/
			case BAVC_ColorRange_eLimited :
				newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited ;
				break ;

			case BAVC_ColorRange_eFull :
				if (!hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleYCC)
				{
					BDBG_MSG(("Attached Rx <%s> does not support selectable Ycc Quantization",
						hHDMI->AttachedEDID.MonitorName)) ;
				}
				newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eFull;
				break ;
			}
		}
		else
		{
			if (BFMT_IS_VESA_MODE(hHDMI->DeviceSettings.eInputVideoFmt))
				eColorRange = BAVC_ColorRange_eFull ;
			else
				eColorRange = BAVC_ColorRange_eLimited ;

			hHDMI->DeviceSettings.stVideoSettings.eColorRange = eColorRange ;

			switch (eColorRange)
			{
			default :
				BDBG_WRN(("Unknown RGB Quantization Range %d...using Auto Range", eColorRange)) ;

				/**************************/
				/*       FALL THRHOUGH    */
				/* Use Default */
				/**************************/

			case BAVC_ColorRange_eAuto :
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault ;
				break ;

			case BAVC_ColorRange_eLimited :
				if (!hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleRGB)
				{
					BDBG_MSG(("Attached Rx <%s> does not support selectable RGB Quantization",
						hHDMI->AttachedEDID.MonitorName)) ;
				}
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eLimitedRange ;
				break ;

			case BAVC_ColorRange_eFull :
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eFullRange ;
				break ;
			}
		}


		/* Set Colorimetry */
		EC2EC1EC0 = 0 ;  /* default extended colorimetry to 0 */
		switch (hHDMI->DeviceSettings.eColorimetry)
		{
		default :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eItu709 ;
			break ;

		case BAVC_MatrixCoefficients_eSmpte_170M :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_601 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC709 ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_2020_CL :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YcCbcCrc ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YCbCr ;
			break ;
		}


		/* convert specified Aspect Ratio to use for AVI Infoframe values */
		switch (hHDMI->DeviceSettings.eAspectRatio)
		{
		default :
		case BFMT_AspectRatio_eUnknown	 : /* Unknown/Reserved */
			BDBG_WRN(("Tx%d: Unknown AspectRatio <%d> passed in AVI Frame",
				hHDMI->eCoreId, (uint8_t) hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_eSquarePxl : /* square pixel */
		case BFMT_AspectRatio_e221_1	 :	   /* 2.21:1 */
			BDBG_WRN(("Tx%d: Specified BAVC Aspect Ratio %d, not compatible with HDMI" ,
				hHDMI->eCoreId, (uint8_t) hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_e4_3		 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3 ;
			break ;

		case BFMT_AspectRatio_e16_9 	 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9 ;
			break ;
		}


		/* As per HDMI1.4, if HD and Aspect Ratio is 4:3, then indicate No-Data. */
		switch (hHDMI->DeviceSettings.eInputVideoFmt)
		{
		case BFMT_VideoFmt_e720p	  :
		case BFMT_VideoFmt_e720p_50Hz :
		case BFMT_VideoFmt_e720p_24Hz :
		case BFMT_VideoFmt_e720p_25Hz :
		case BFMT_VideoFmt_e720p_30Hz :
		case BFMT_VideoFmt_e1080i	  :
		case BFMT_VideoFmt_e1080i_50Hz:
		case BFMT_VideoFmt_e1080p	  :
		case BFMT_VideoFmt_e1080p_24Hz:
		case BFMT_VideoFmt_e1080p_25Hz:
		case BFMT_VideoFmt_e1080p_30Hz:
		case BFMT_VideoFmt_e1080p_50Hz:
			if (hHDMI->DeviceSettings.eAspectRatio == BFMT_AspectRatio_e4_3)
					M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
		break ;
			default :
		break ;
		}


		/* Video Id Code & pixel repetitions */
		BHDM_VideoFmt2CEA861Code(hHDMI->DeviceSettings.eInputVideoFmt,
			hHDMI->DeviceSettings.eAspectRatio, hHDMI->DeviceSettings.ePixelRepetition,
			&VideoID, &PixelRepeat) ;
	}

	/* Update derived or overridden AVI fields */

	/* Pixel Encoding  */
	newAviInfoFrame.ePixelEncoding = Y1Y0 ;

	/* Update AVI Colorimetry */
	newAviInfoFrame.eColorimetry = C1C0;
	newAviInfoFrame.eExtendedColorimetry = EC2EC1EC0;

	/* Picture Aspect Ratio */
	newAviInfoFrame.ePictureAspectRatio = M1M0 ;


	/* Video ID and Pixel Repeat */
	newAviInfoFrame.VideoIdCode = VideoID;
	newAviInfoFrame.PixelRepeat = PixelRepeat;

	/* always set the bOverrideDefaults to false */
	/* so subsequent calls to Get/Set AVI do not force override of settings */
	/* all overrides must be explicitly forced by the caller */
	newAviInfoFrame.bOverrideDefaults = false ;
/* BANDREWS - need overrides to be remembered, because user is not the only caller */
	newAviInfoFrame.bOverrideDefaults = stAviInfoFrame->bOverrideDefaults ;

	/* Construct AVI InfoFrame pay loads */
	hHDMI->PacketBytes[1] =
		  newAviInfoFrame.ePixelEncoding << 5 /* Y1Y0 Colorspace */
		| newAviInfoFrame.eActiveInfo << 4   /*    A0 Active Information Valid */
		| newAviInfoFrame.eBarInfo << 2      /* B1B0 Bar Info Valid */
		| newAviInfoFrame.eScanInfo ;	      /* S1S0 Scan Information */

	hHDMI->PacketBytes[2] =
		  newAviInfoFrame.eColorimetry << 6   /* C1C0 Colorimetry */
		| newAviInfoFrame.ePictureAspectRatio << 4     /* M1M0 */
		| newAviInfoFrame.eActiveFormatAspectRatio ;  /* R3..R0 */

	hHDMI->PacketBytes[3] = newAviInfoFrame.eScaling ;
	if (newAviInfoFrame.eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
		hHDMI->PacketBytes[3] |=
			newAviInfoFrame.eExtendedColorimetry << 4;  /* EC2EC1EC0 */
	}

	hHDMI->PacketBytes[3] |=
		  newAviInfoFrame.eITContent << 7  /* ITC */
		| newAviInfoFrame.eRGBQuantizationRange << 2;  /* Q1Q0 */


	hHDMI->PacketBytes[4] = VideoID ;
	hHDMI->PacketBytes[5] = PixelRepeat ;
	hHDMI->PacketBytes[5] |=
		  newAviInfoFrame.eContentType << 4  /* CN1CN0 */
		| newAviInfoFrame.eYccQuantizationRange << 6; /* YQ1YQ0 */


	hHDMI->PacketBytes[6]  = (uint8_t) (newAviInfoFrame.TopBarEndLineNumber & 0x00FF) ;
	hHDMI->PacketBytes[7]  = (uint8_t) (newAviInfoFrame.TopBarEndLineNumber >> 8) ;

	hHDMI->PacketBytes[8]  = (uint8_t) (newAviInfoFrame.BottomBarStartLineNumber & 0x00FF) ;
	hHDMI->PacketBytes[9]  = (uint8_t) (newAviInfoFrame.BottomBarStartLineNumber >> 8) ;

	hHDMI->PacketBytes[10] = (uint8_t) (newAviInfoFrame.LeftBarEndPixelNumber & 0x00FF) ;
	hHDMI->PacketBytes[11] = (uint8_t) (newAviInfoFrame.LeftBarEndPixelNumber >> 8) ;

	hHDMI->PacketBytes[12] = (uint8_t) (newAviInfoFrame.RightBarEndPixelNumber & 0x00FF) ;
	hHDMI->PacketBytes[13] = (uint8_t) (newAviInfoFrame.RightBarEndPixelNumber >> 8) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

	/* update current device settings with new AviInfoFrame settings */
	BKNI_Memcpy(&(hHDMI->DeviceSettings.stAviInfoFrame), &newAviInfoFrame,
		sizeof(BAVC_HDMI_AviInfoFrame)) ;

#if BDBG_DEBUG_BUILD
	{
		BDBG_Level eLevel;

		BDBG_GetModuleLevel("BHDM_PACKET_AVI", &eLevel) ;
		if (eLevel == BDBG_eMsg)
		{
			BAVC_HDMI_DisplayAVIInfoFramePacket(
				&hHDMI->DeviceStatus.stPort, &hHDMI->DeviceSettings.stAviInfoFrame) ;
		}
	}
#endif

	return rc ;
}



/******************************************************************************
Summary:
Get the Auxillary Video Information Info frame sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_GetAVIInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AviInfoFrame *stAviInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;
#if BHDM_CONFIG_DEBUG_AVI_INFOFRAME
	uint8_t
		Y1Y0, C1C0, M1M0,
		EC2EC1EC0=2,
		VideoID,
		PixelRepeat;
#endif

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(stAviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame)) ;
	BKNI_Memcpy(stAviInfoFrame, &(hHDMI->DeviceSettings.stAviInfoFrame),
		sizeof(BAVC_HDMI_AviInfoFrame)) ;


/* translate to AVI Data for debug/display purposes */
#if BHDM_CONFIG_DEBUG_AVI_INFOFRAME

	if (stAviInfoFrame->bOverrideDefaults)
	{
		/* RGB or YCrCb Colorspace */
		Y1Y0 = stAviInfoFrame->ePixelEncoding;

		/* Colorimetry */
		C1C0 = stAviInfoFrame->eColorimetry;

		if (stAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended) {
			EC2EC1EC0 = stAviInfoFrame->eExtendedColorimetry;
		}

		/* Picture Aspect Ratio*/
		M1M0 = stAviInfoFrame->ePictureAspectRatio;

		/* Video ID Code & Pixel Repetition */
		VideoID = stAviInfoFrame->VideoIdCode;
		PixelRepeat = stAviInfoFrame->PixelRepeat;
	}
	else
	{
		Y1Y0 = hHDMI->DeviceSettings.stAviInfoFrame.ePixelEncoding ;

		/* Set Colorimetry */
		switch (hHDMI->DeviceSettings.eColorimetry)
		{
		default :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eItu709 ;
			break ;

		case BAVC_MatrixCoefficients_eSmpte_170M :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_601 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC709;
		}


		/* convert specified Aspect Ratio to use for AVI Infoframe values */
		switch (hHDMI->DeviceSettings.eAspectRatio)
		{
		default :
		case BFMT_AspectRatio_eUnknown	 : /* Unknown/Reserved */
			BDBG_WRN(("Tx%d: Unknown AspectRatio passed in AVI Frame", hHDMI->eCoreId)) ;
			/* fall through */

		case BFMT_AspectRatio_eSquarePxl : /* square pixel */
		case BFMT_AspectRatio_e221_1	 :	   /* 2.21:1 */
			BDBG_WRN(("Tx%d: Specified BAVC Aspect Ratio %d, not compatible with HDMI",
				hHDMI->eCoreId,	hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_e4_3		 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3 ;
			break ;

		case BFMT_AspectRatio_e16_9 	 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9 ;
			break ;
		}

		BHDM_P_VideoFmt2CEA861Code(hHDMI->DeviceSettings.eInputVideoFmt,
			hHDMI->DeviceSettings.eAspectRatio, hHDMI->DeviceSettings.ePixelRepetition,
			&VideoID) ;

		PixelRepeat = (uint8_t) hHDMI->DeviceSettings.ePixelRepetition;
	}

	BDBG_MSG(("*** Current AVI Info Frame Settings ***")) ;
	BDBG_MSG(("Tx%d: ColorSpace (%d): %s", hHDMI->eCoreId, Y1Y0,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(Y1Y0))) ;
	BDBG_MSG(("Tx%d: Active Info (%d): %s ",  hHDMI->eCoreId,
		stAviInfoFrame->eActiveInfo,
		BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(stAviInfoFrame->eActiveInfo))) ;
	BDBG_MSG(("Tx%d: Bar Info (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eBarInfo,
		BAVC_HDMI_AviInfoFrame_BarInfoToStr(stAviInfoFrame->eBarInfo))) ;
	BDBG_MSG(("Tx%d: Scan Info (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eScanInfo,
		BAVC_HDMI_AviInfoFrame_ScanInfoToStr(stAviInfoFrame->eScanInfo))) ;
	BDBG_MSG(("Tx%d: Colorimetry (%d): %s", hHDMI->eCoreId,
		C1C0, BAVC_HDMI_AviInfoFrame_ColorimetryToStr(C1C0))) ;
	BDBG_MSG(("Tx%d: Extended Colorimetry (%d): %s", hHDMI->eCoreId, EC2EC1EC0,
		BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(EC2EC1EC0))) ;
	BDBG_MSG(("Tx%d: Picture AR (%d): %s", hHDMI->eCoreId, M1M0,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(M1M0))) ;

	if ((stAviInfoFrame->eActiveFormatAspectRatio >= 8)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 11))
		BDBG_MSG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 8))) ;
	else if  ((stAviInfoFrame->eActiveFormatAspectRatio > 12)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 15))
		BDBG_MSG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_MSG(("Tx%d: Active Format AR (%d): Other", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio)) ;

	BDBG_MSG(("Tx%d: Picture Scaling (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eScaling,
		BAVC_HDMI_AviInfoFrame_ScalingToStr(stAviInfoFrame->eScaling))) ;

	BDBG_MSG(("Tx%d: Video ID Code = %d", hHDMI->eCoreId, VideoID)) ;
	BDBG_MSG(("Tx%d: Pixel Repeat: %d", hHDMI->eCoreId, PixelRepeat)) ;

	BDBG_MSG(("Tx%d: IT Content (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(stAviInfoFrame->eITContent)));
	BDBG_MSG(("Tx%d: IT Content Type (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(stAviInfoFrame->eContentType)));
	BDBG_MSG(("Tx%d: RGB Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(stAviInfoFrame->eRGBQuantizationRange)));
	BDBG_MSG(("Tx%d: YCC Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(stAviInfoFrame->eYccQuantizationRange)));

	BDBG_MSG(("Tx%d: Top Bar End Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->TopBarEndLineNumber)) ;
	BDBG_MSG(("Tx%d: Bottom Bar Stop Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->BottomBarStartLineNumber)) ;

	BDBG_MSG(("Tx%d: Left Bar End Pixel Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->LeftBarEndPixelNumber )) ;
	BDBG_MSG(("Tx%d: Right Bar End Pixel Number: %d\n", hHDMI->eCoreId,
		stAviInfoFrame->RightBarEndPixelNumber )) ;

#endif

	return rc ;
}
