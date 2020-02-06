/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
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


/*= Module Overview *********************************************************
<verbatim>

Overview
The HDMI (High-Definition Multimedia Interface) API is a library used to
provide HDMI/DVI (Digital Visual Interface) functionality between Broadcom
cores and the connected HDMI/DVI receivers.

The API includes support for associated HDMI functionality such as parsing
of the EDID (Extended Display Identification Data) contained in the HDMI/DVI
monitors as well as support for HDCP (High-bandwidth Data Content Protection)

Additional support for reading CEC (Consumer Electronic Control) will also be
provided.


Sample Code
//
// ********************************************
// Delclare / Initialize Device Handles for...
// ********************************************

BHDM_Handle     MyHDMI
BCHP_Handle     MyChip;
BREG_Handle     MyRegister;
BINT_Handle     MyInterupt;
BREG_I2C_Handle MyI2C;


// ********************************************
// Video Display Handle Must be open prior to
// opening HDMI device
// ********************************************
BVDC_Open(&hDisplay,...)

// ********************************************
// Get/Modify Default Settings for HDMI
// ********************************************
BHDM_GetDefaultSettings(&BHDM_Settings)

// ********************************************
// open HDMI Device Handle
// ********************************************
BHDM_Open(..., BHDM_Settings)


// ********************************************
// Set VDC DAC Configuration
// See VDC Porting Interface
// ********************************************


BVDC_ApplyChanges(...)

// ********************************************
// Enable the VEC output to the HDMI device
// ********************************************

BVDC_Display_SetHdmiConfiguration(
	hDisplay, BVDC_Hdmi_0, BVDC_HdmiOutput_eHDRGB) ;

BVDC_ApplyChanges(...)


// ********************************************
// check the attached receiver for HDMI support.
// HDMI support is determined by the existence of
// the Vendor Specific Data Block (VSDB) in the EDID
// ********************************************

rc = BHDM_EDID_IsRxDeviceHdmi(hHDMI, &MyRxVSDB, &RxHasHdmiSupport) ;
if (rc != BERR_SUCCESS)
	process error - return ;

// ********************************************


// ********************************************
// get/modify current HDMI settings for display
// default display mode is DVI
// use the current video format used by the VDC
// GetHdmiSettings can be used if in a different
// area of the code
// ********************************************

BVDC_Display_GetVideoFormat(hDisplay, &MyVDCFormat) ;
MyHdmiSettings.eInputVideoFmt   = MyVDCFormat ;


// ********************************************
// Set the Pixel Clock Rate based on TBD (headend
// or stream)
// ********************************************
MyHdmiSettings.eTmdsClock   = BHDM_P_TmdsClock_eXXX ;


// ********************************************
// if neccessary, set parameters for HDMI Audio
// ********************************************
if (RxHasHdmiSupport)
{
	MyHdmiSettings.eOutputFormat      = BHDM_OutputFormat_eHDMIMode ;
	MyHdmiSettings.eAudioSamplingRate =	BAVC_AudioSamplingRate_eXXX ;
}


// ********************************************
// Enable/Turn on the HDMI Output
// ********************************************
if ((rc = BHDM_EnableDisplay(hHDMI, &MyHdmiSettings)) == BERR_SUCCESS)
	printf("DisplayOutput Enabled") ;
else
{
	// check Return Code for possible errors
	printf("*** ERROR Enabling DVI/HDMI Output to the Display") ;
}
.
.
.
</verbatim>
****************************************************************************/

#ifndef BHDM_H__
#define BHDM_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "breg_i2c.h"   /* I2C */
#include "btmr.h"   /* Timer Handle  */

#include "bavc.h"       /* A/V constants */
#include "bavc_hdmi.h"  /* HDMI A/V constants */
#include "berr_ids.h"   /* Error codes */
#include "bfmt.h"       /* Video Formats */
#include "bdbg.h"       /* Debug Support */


/******************************************************************************
Summary:
HDMI Context Handle
*******************************************************************************/
typedef struct BHDM_P_Handle *BHDM_Handle ;

#include "bchp_common.h"
#if BCHP_HDMI_TX_AUTO_I2C_REG_START
#define BHDM_HAS_HDMI_20_SUPPORT 1
#endif

#define BHDM_HDMI_2_0_MAX_RATE 594
#define BHDM_HDMI_1_4_MAX_RATE 297


#include "bhdm_1_3_features.h"


#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
Summary:
HDMI Basic Return Codes
*******************************************************************************/
#define BHDM_BASE_ERRS                0

#define BHDM_NOT_IMPLEMENTED			BERR_MAKE_CODE(BERR_HDM_ID, BHDM_BASE_ERRS + 1)
#define BHDM_RX_FEATURE_NOT_SUPPORTED	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_BASE_ERRS + 2)
#define BHDM_NO_RX_DEVICE				BERR_MAKE_CODE(BERR_HDM_ID, BHDM_BASE_ERRS + 3)
#define BHDM_UNSUPPORTED_VIDEO_FORMAT	BERR_MAKE_CODE(BERR_HDM_ID, BHDM_BASE_ERRS + 4)
#define BHDM_HARDWARE_ERROR				BERR_MAKE_CODE(BERR_HDM_ID, BHDM_BASE_ERRS + 5)
#define BHDM_FIFO_UNDERFLOW 			BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS + 6)
#define BHDM_FIFO_OVERFLOW  			BERR_MAKE_CODE(BERR_HDM_ID, BHDM_HDCP_ERRS + 7)



#define BHDM_HDCP_ERRS 100     /* HDCP error codes */
#define BHDM_EDID_ERRS 200 	   /* EDID error codes */
#define BHDM_CEC_ERRS  300     /* CEC error codes */

/******************************************************************************
Summary:
HDMI Packet Size in Bytes
*******************************************************************************/
#define BHDM_NUM_PACKET_BYTES  28

#define BHDM_INFOFRAME_PACKET_TYPE 0x80


/******************************************************************************
Summary:
Enumerated Type of different events available from the HDMI code

Description:
The HDMI core supports for five sets of interrupts (HDMI, HDCP, CEC, Hot Plug,
and RAM).  Applications can request notification of events associated with these
interrupts.  This enumeration can be passed to the BHDM_GetEvent function

See Also:
	o BHDM_GetEvent

*******************************************************************************/
typedef enum
{
	BHDM_EventHDMI,
	BHDM_EventHDCP,
	BHDM_EventHDCPRiValue,
	BHDM_EventHDCPPjValue,
	BHDM_EventHDCPRepeater,
	BHDM_EventHDCPPowerDown,
	BHDM_EventFIFO, /* debugging event */
	BHDM_EventRAM,   /* debugging event */
	BHDM_EventRxSense,
	BHDM_EventScramble,
	BHDM_EventAvRateChange,
	BHDM_EventHDCP22EncryptionEnable,
	BHDM_EventHDCP22ReAuthRequest,
	BHDM_EventMhlStandby,
	BHDM_EventInvalid
} BHDM_EventType ;


/******************************************************************************
Summary:
Enumerated Type of the output destinations for the HDMI data.
This enumeration is used in the Default Settings used by BHDM_Open

Description:
The HDMI core must know where to place the output DVI/HDMI data.
This table enumerates those types

   BHDM_OutputPort_eDVO
   BHDM_OutputPort_eHDMI


See Also:
	o BHDM_Open

*******************************************************************************/
typedef enum
{
	BHDM_OutputPort_eDVI,
	BHDM_OutputPort_eDVO = BHDM_OutputPort_eDVI,
	BHDM_OutputPort_eHDMI
} BHDM_OutputPort ;



/******************************************************************************
Summary:
Enumerated Type of the format of the data from the HDMI Core
This enumeration is used in the Default Settings used by BHDM_Open

Description:
The HDMI core must know which type (DVI / HDMI) of data to send to the Output
Port

   BHDM_OutputFormat_eDVO
   BHDM_OutputFormat_eHDMI
   BHDM_OutputFormat_e12BitDVOMode
   BHDM_OutputFormat_e24BitDVOMode
   BHDM_OutputFormat_e768pPanel
   BHDM_OutputFormat_e1080pPanel
   BHDM_OutputFormat_eSingleChannel1_DVOMode,
   BHDM_OutputFormat_eSingleChannel2_DVOMOde,
   BHDM_OutputFormat_eDualChannel1_DVOMode,
   BHDM_OutputFormat_eDualChannel2_DVOMode

See Also:
	o BHDM_Open

*******************************************************************************/
typedef enum
{
	BHDM_OutputFormat_eDVIMode,
	BHDM_OutputFormat_eHDMIMode,
	BHDM_OutputFormat_e12BitDVOMode,
	BHDM_OutputFormat_e24BitDVOMode,
	BHDM_OutputFormat_e768pPanel
} BHDM_OutputFormat ;


/******************************************************************************
Summary:
Enumerated Type of the type of frames to be sent to the HDMI Rx

Description:
The enumeration can be used to configure packets for sending to the HDMI Rx

*******************************************************************************/
typedef enum
{
	BHDM_PACKET_eGCP_ID = 0,
	BHDM_PACKET_eAVI_ID,
	BHDM_PACKET_eAudioFrame_ID,
	BHDM_PACKET_eSPD_ID,
	BHDM_Packet_eVendorSpecific_ID,
	BHDM_PACKET_eGamutMetadata_ID,
	BHDM_PACKET_eDRM_ID,
	BHDM_Packet_eUser1,
	BHDM_Packet_eUser2,
	BHDM_Packet_eUser3,
	BHDM_Packet_eUser4,
	BHDM_Packet_eUser5,
	BHDM_Packet_eUser6,
	BHDM_Packet_eUser7,
	BHDM_Packet_eMax
} BHDM_Packet ;


/* Moved BHDM_AVI_Packet_XXXX enumerations to
   magnum/commonutils/bavc/bavc_hdmi.h */

/******************************************************************************
Summary:
Enumerated Type to describe changes to the Audio or Video Rates input to
the HDMI core

Description:
The enumeration can be used to set up callbacks to the HDMI


See Also:
*******************************************************************************/
typedef enum
{
	BHDM_Callback_Type_eUnknown = 0,
	BHDM_Callback_Type_eAudioChange,
	BHDM_Callback_Type_eVideoChange,
	BHDM_Callback_Type_eManualAudioChange
} BHDM_Callback_Type ;



/******************************************************************************
Summary:
Enumerated Type for core type

Description:
The enumeration can be used to determine what core is used e.g. 65, 40, 28


See Also:
	o BHDM_EnablePreEmphasis
*******************************************************************************/
typedef enum
{
	BHDM_CoreUnknown,
	BHDM_Core65nm,
	BHDM_Core40nm,
	BHDM_Core28nm,
	BHDM_CoreMax

} BHDM_eCore ;


/******************************************************************************
Summary:
Type for TMDS Settings

Description:
The structure can be used to get/set values for control of the TMDS Signal


See Also:
	o BHDM_GetHdmiSettngs
	o BHDM_SetHdmiSettngs
*******************************************************************************/
typedef struct BHDM_TmdsSettings
{
    bool clockEnabled ;
    bool dataEnabled ;
} BHDM_TmdsSettings ;

/******************************************************************************
Summary:
Enumerated Type for Pre-Emphasis Control

Description:
The enumeration can be used to set up values for Pre-Emphasis Control


See Also:
	o BHDM_EnablePreEmphasis
*******************************************************************************/
typedef enum
{
	BHDM_PreEmphasis_eOFF =  0,
	BHDM_PreEmphasis_eDeepColorMode = 4,
	BHDM_PreEmphasis_eLOW =  5,
	BHDM_PreEmphasis_eMED = 10,
	BHDM_PreEmphasis_eMAX = 15
} BHDM_PreEmphasisSetting ;


/******************************************************************************
Summary:
Pre-emphasis configurations settings

Description:
This structure can be used to set up values for Pre-Emphasis Control


See Also:
	o BHDM_GetPreEmphasisConfiguration
	o BHDM_SetPreEmphasisConfiguration
*******************************************************************************/
typedef struct _BHDM_PREEMPHASIS_CONFIGURATION_
{
	BHDM_eCore eCore ;

	uint16_t uiPreEmphasis_CK;
	uint16_t uiPreEmphasis_Ch0;
	uint16_t uiPreEmphasis_Ch1;
	uint16_t uiPreEmphasis_Ch2;

	/* The following fields are applicable to 65nm platforms only. */
	uint8_t uiPreDriverAmp ;
	uint8_t uiDriverAmp;
	uint8_t uiRefCntl;
	uint8_t uiTermR;

	/* The following fields are applicable to 40nm platforms only */
	uint8_t  uiHfEn;
	uint16_t uiCurrentRatioSel;
	uint8_t  uiKP;
	uint8_t  uiKI;
	uint8_t  uiKA;

	/* The following fields are applicable to 28nm platforms only */
	uint8_t  uiTermResSelData2 ;
	uint8_t  uiTermResSelData1 ;
	uint8_t  uiTermResSelData0 ;
	uint8_t  uiTermResSelDataCK ;

	uint8_t  uiResSelData2 ;
	uint8_t  uiResSelData1 ;
	uint8_t  uiResSelData0 ;
	uint8_t  uiResSelDataCK ;

} BHDM_PreEmphasis_Configuration;


/******************************************************************************
Summary:
Enumerated Type for HDMI Audio Channels

Description:
The enumeration can be used to set up the Audio Channels used in the Audio Infoframe Packets
and MAI bus.


See Also:
*******************************************************************************/
typedef enum
{

	/* BHDM_ChannelAllocation_e_Front_Rear_Subwoofer */
	/* See CEA-861-x Audio Info frame */
	 BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx___xx_FR__FL =0,
	 	BHDM_ChannelAllocation_eStereo = 0,
	 	BHDM_ChannelAllocation_e2_0_0 = 0,

	 BHDM_ChannelAllocation_e_xx__xx__xx__xx__xx__LFE_FR__FL = 1,
		BHDM_ChannelAllocation_e2_0_1 = 1,

	 BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC___xx_FR__FL = 2,
		BHDM_ChannelAllocation_e3_0_0 = 2,

	 BHDM_ChannelAllocation_e_xx__xx__xx__xx__FC__LFE_FR__FL = 3,
		BHDM_ChannelAllocation_e3_0_1 = 3,

	 BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx___xx_FR__FL = 4,
		BHDM_ChannelAllocation_e2_1_0 = 4,

	 BHDM_ChannelAllocation_e_xx__xx__xx__RC__xx__LFE_FR__FL = 5,
		BHDM_ChannelAllocation_e2_1_1 = 5,

	 BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC___xx_FR__FL = 6,
		BHDM_ChannelAllocation_e3_1_0 = 6,

	 BHDM_ChannelAllocation_e_xx__xx__xx__RC__FC__LFE_FR__FL = 7,
		BHDM_ChannelAllocation_e3_1_1 = 7,

	 BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx___xx_FR__FL = 8,
		BHDM_ChannelAllocation_e2_2_0 = 8,

	 BHDM_ChannelAllocation_e_xx__xx__RR__RL__xx__LFE_FR__FL = 9,
		BHDM_ChannelAllocation_e2_2_1 = 9,

	 BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC___xx_FR__FL = 10,
		BHDM_ChannelAllocation_e3_2_0 = 10,

	 BHDM_ChannelAllocation_e_xx__xx__RR__RL__FC__LFE_FR__FL = 11,
		BHDM_ChannelAllocation_e5_1 = 11,		  /* 5.1 */
		BHDM_ChannelAllocation_e3_2_1 = 11,

	 BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx___xx_FR__FL = 12,
		BHDM_ChannelAllocation_e2_3_0 = 12,

	 BHDM_ChannelAllocation_e_xx__RC__RR__RL__xx__LFE_FR__FL = 13,
		BHDM_ChannelAllocation_e2_3_1 = 13,

	 BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC___xx_FR__FL = 14,
		BHDM_ChannelAllocation_e3_3_0 = 14,

	 BHDM_ChannelAllocation_e_xx__RC__RR__RL__FC__LFE_FR__FL = 15,
	 	BHDM_ChannelAllocation_e3_3_1 = 15,

	 BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx__xx_FR__FL = 16,
 		BHDM_ChannelAllocation_e2_4_0 = 16,

	 BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__xx_LFE_FR__FL = 17,
 		BHDM_ChannelAllocation_e2_4_1 = 17,

	 BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC__xx_FR__FL = 18,
 		BHDM_ChannelAllocation_e3_4_0 = 18,

	 BHDM_ChannelAllocation_e_RRC_RLC__RR__RL__FC_LFE_FR__FL = 19,
 		BHDM_ChannelAllocation_e3_4_1 = 19,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__xx__xx__xx_FR__FL = 20,
 		BHDM_ChannelAllocation_e4_0_0 = 20,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__xx__xx_LFE_FR__FL = 21,
 		BHDM_ChannelAllocation_e4_0_1 = 21,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__xx__FC__xx_FR__FL = 22,
 		BHDM_ChannelAllocation_e5_0_0 = 22,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__xx__FC_LFE_FR__FL = 23,
 		BHDM_ChannelAllocation_e5_0_1 = 23,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__RC__xx__xx_FR__FL = 24,
 		BHDM_ChannelAllocation_e4_1_0 = 24,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__RC__xx_LFE_FR__FL = 25,
 		BHDM_ChannelAllocation_e4_1_1 = 25,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__RC__FC__xx_FR__FL = 26,
 		BHDM_ChannelAllocation_e5_1_0 = 26,

	 BHDM_ChannelAllocation_e_FRC_FLC__xx__RC__FC_LFE_FR__FL = 27,
 		BHDM_ChannelAllocation_e5_1_1 = 27,

	 BHDM_ChannelAllocation_e_FRC_FLC__RR__RL__xx__xx_FR__FL = 28,
 		BHDM_ChannelAllocation_e4_2_0 = 28,

	 BHDM_ChannelAllocation_e_FRC_FLC__RR__RL__xx_LFE_FR__FL = 29,
 		BHDM_ChannelAllocation_e4_2_1 = 29,

	 BHDM_ChannelAllocation_e_FRC_FLC__RR__RL__FC__xx_FR__FL = 30,
 		BHDM_ChannelAllocation_e5_2_0 = 30,

	 BHDM_ChannelAllocation_e_FRC_FLC__RR__RL__FC_LFE_FR__FL = 31,
 		BHDM_ChannelAllocation_e5_2_1 = 31

} BHDM_AudioChannel  ;


/***************************************************************************
Summary:
	Prototype for external modules to install callback with HDMI to notify of changes

Description:

Input:
	pvParam1 - User defined data structure casted to void.
	iParam2 - Additional user defined value.
	pvData - Data to pass from HDMI to application thru callback.

Output:
	None

See Also:
	None
**************************************************************************/
typedef void (*BHDM_CallbackFunc)
	( void							  *pvParam1,
	  int							   iParam2,
	  void							  *pvData );



#define BHDM_TMDS_RANGES 7


typedef struct
{
	uint32_t MinTmdsRate ;
	uint32_t MaxTmdsRate ;
	uint32_t HDMI_TX_PHY_CTL_0 ;
	uint32_t HDMI_TX_PHY_CTL_1 ;
	uint32_t HDMI_TX_PHY_CTL_2 ;
} BHDM_TmdsPreEmphasisRegisters ;


typedef struct BHDM_PACKET_ACR_CONFIG
 {
	uint32_t NValue ;
	uint32_t HW_NValue ;
	uint32_t CTS_0 ;
	uint32_t CTS_1 ;
	uint32_t CTS_0_REPEAT ;
	uint32_t CTS_1_REPEAT ;
 } BHDM_PACKET_ACR_CONFIG ;
/* } BHDM_P_AUDIO_CLK_VALUES ; */


/***************************************************************************
Summary:
HDMI settings used for BHDM_Open().

See Also
	o BHDM_GetDefaultSettings
****************************************************************************/
typedef struct _BHDM_Settings_
{

	/*
	--NOTE: modifications to this BHDM_Settings structure must be
	-- accounted for in the BHDM_P_HdmiSettingsChange function
	*/

	uint8_t eCoreId ;

	BFMT_VideoFmt           eInputVideoFmt ;

	BAVC_Timebase           eTimebase ;

	BHDM_OutputPort         eOutputPort ;
	BHDM_OutputFormat       eOutputFormat ;

	/* overrideDefaultColorimetry - */
	/*    Override the default colorimetry which is based on the OutputFormat */
	/* eColorimetry - */
	/*    Colorimetry used when overrideDefaultColorimetry is set; See HDMI Spec for colorimetries */
	bool overrideDefaultColorimetry ;
	BAVC_MatrixCoefficients	eColorimetry ;

	BFMT_AspectRatio eAspectRatio ;
	BAVC_HDMI_PixelRepetition ePixelRepetition ;

	BHDM_ColorDepth_Settings stColorDepth;
	BHDM_Video_Settings stVideoSettings ;

	/**** AVI Info Frame Structure ****/
	BAVC_HDMI_AviInfoFrame stAviInfoFrame ;
	/**********************************/

	/**** Audio Info Frame Structure ****/
	BAVC_HDMI_AudioInfoFrame stAudioInfoFrame ;
	/**********************************/
	BAVC_AudioSamplingRate  eAudioSamplingRate ;
	BAVC_AudioFormat eAudioFormat ;
	BAVC_AudioBits eAudioBits ;

	BAVC_HDMI_SpdInfoFrame_SourceType eSpdSourceDevice ;
	uint8_t SpdVendorName[BAVC_HDMI_SPD_IF_VENDOR_LEN+1] ;
	uint8_t SpdDescription[BAVC_HDMI_SPD_IF_DESC_LEN+1] ;

	/**** Vendor Specific Info Frame Structure ****/
	BAVC_HDMI_VendorSpecificInfoFrame stVendorSpecificInfoFrame ;
	/**********************************/

	/**** DRM Infoframe Structure ****/
	BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;
	/**********************************/

	/* Currently this parameter is not being used */
	bool CalculateCts  ;

	uint8_t uiDriverAmpDefault;

	bool AltDvoPath ;

	bool BypassEDIDChecking ;  /* debug tool */
	bool bCrcTestMode ; /* auto crc test mode; should never be used in production */
	bool UseDebugEdid ;  /* use statically declared EDID; see bhdm_debug_edid.c */
	uint8_t uiDebugEdid ; /* use EDID n; 0 = do not use any debug EDID*/

	bool bFifoMasterMode;      /* Set true to enable master mode */

	bool bForceEnableDisplay; 	/* Set to true to always enable display even when
								using the same HDMI settings */
	bool bEnableAutoRiPjChecking;	/* Set to true to enable auto Ri, Pj checking handle by HW */
	BTMR_Handle hTMR ;

	uint8_t HotplugDetectThreshold; /* disable Hotplug interrupt if maxHotplugThreshold is reached during 1s */

	bool bEnableScdcMonitoring ;

	bool bResumeFromS3;

	BHDM_TmdsPreEmphasisRegisters TmdsPreEmphasisRegisters[BHDM_TMDS_RANGES] ;
} BHDM_Settings;


typedef enum
{
	BHDM_EDID_STATE_eInvalid,
	BHDM_EDID_STATE_eOK
} BHDM_EDID_STATE;



/***************************************************************************
Summary:
HDMI Status .

See Also
	o None
****************************************************************************/
typedef struct
{
	BAVC_HDMI_Port stPort ;
	BHDM_TmdsSettings tmds ;
	uint32_t pixelClockRate;
	BHDM_PACKET_ACR_CONFIG stAcrPacketConfig ;
	BHDM_EDID_STATE edidState ;
} BHDM_Status ;


typedef struct _BHDM_CrcData_
{
	uint16_t crc;
} BHDM_CrcData;


/******************************************************************************
Summary:
Get the default settings for the HDMI device.

Input:
	<None>

Output:
   pBHDM_Settings  - pointer to memory to hold default settings

Returns:
	BERR_SUCCESS - Successfully opened HDMI connection.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:

*******************************************************************************/
BERR_Code BHDM_GetDefaultSettings(
   BHDM_Settings *pBHDM_DefaultSettings  /* [out] pointer to memory to hold default settings */
) ;



/******************************************************************************
Summary:
Open a HDMI connection to a HDMI Rx.

Input:
	hChip - The chip handle that application created ealier during chip
	initialization sequence.  This handle is use for get chip
	information, chip revision, and miscellaneous chip configurations.

	hRegister - The register handle that application created earlier during
	the chip initialization sequence.  This handle is use to access chip
	registers (HDMI registers).

	hInterrupt - The interrupt handle that the application created earler during the
	initialization sequence.  This handle is used to process interrupts.

	hI2cRegHandle - The I2C handle that the application created earler during the
	initialization sequence.  This handle is used to access the HDMI Rx.

	pBHDM_Settings - The default settings for configuring the HDMI connection. This
	sturcture can be queried prior to BHDM_Open with BHDM_GetDefaultSettings.
	This parameter can be NULL.  In this case the default setting will be used.

Output:
	phHDMI - pointer to previously allocated HDMI structure

Returns:
	BERR_SUCCESS - Successfully opened HDMI connection.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle
	BREG_Handle
	BI2C_ChannelHandle

*******************************************************************************/
BERR_Code BHDM_Open(
   BHDM_Handle *phHDMI,       /* [out] HDMI handle */
   BCHP_Handle hChip,         /* [in] Chip handle */
   BREG_Handle hRegister,     /* [in] Register Interface to HDMI Tx Core */
   BINT_Handle hInterrupt,    /* [in] Interrupt handle */
   BREG_I2C_Handle hI2cRegHandle,      /* [in] I2C Interface to HDMI Rx */
   const BHDM_Settings  *pBHDM_Settings /* [in] default HDMI settings */
) ;



/******************************************************************************
Summary:
Close the HDMI connection to the HDMI Rx.

Description:
This function will close the HDMI connection to the HDMI Rx.  A new HDMI
connection will have to be opened to display to the HDMI Rx again.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS - Successfully closed the HDMI connection.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle
*******************************************************************************/
BERR_Code BHDM_Close(
   BHDM_Handle hHDMI  /* [in] HDMI handle */
) ;



/******************************************************************************
Summary:
Clear the Hot Plug Interrupt for the next event.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS - Successfully reset the HotPlug interrupt.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_ClearHotPlugInterrupt(
   BHDM_Handle hHDMI        /* [in] HDMI handle */
) ;


/******************************************************************************
Summary:
Check the Hot Plug Interrupt to see if there was a change (since last Clear).

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS - Successfully checked the HotPlug interrupt.

See Also:
	BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_CheckHotPlugInterrupt(
	BHDM_Handle hHDMI,         /* [in] HDMI handle */
	uint8_t *bHotPlugInterrupt /* [out] HPD changed or not */
) ;


/******************************************************************************
Summary:
Re-enable the Hotplug Interrupts that have been disabled due to excessive Hot Plug interrupts

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	<None>

See Also:
	BHDM_Handle

*******************************************************************************/
void BHDM_ReenableHotplugInterrupt(BHDM_Handle hHDMI) ;


/******************************************************************************
Summary:
Check for an attached Rx Device.

Description:
Check if a receiver is attached to the HDMI transmitter


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	DeviceAttached - flag indicating if a receiver device is attached or not

Returns:
	BERR_SUCCESS           - Rx Device is Attached.
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_NO_RX_DEVICE      - No HDMI/DVI Rx Device is Attached.

See Also:
	BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_RxDeviceAttached(
   BHDM_Handle hHDMI,		/* [in] HDMI handle */
   uint8_t *RxDeviceAttached
) ;



/******************************************************************************
Summary:
Reset HDMI/DVI Rx device (for non-compliant receivers)

Description:
Some Silicon Image Receivers do not properly reset.  The HDCP Specification
calls for a reset of the DVI Rx when the HDCP Bksv value is written to the
Rx.  The SI Rxs require the TMDS output be disabled for 100 ms.  This causes
a temporary blanking of the screen; therefore this function should only be
used when initial HDCP Authentication fails.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS - Commands successfully initiated to Reset Rx Device.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_ResetHdmiRx(
   BHDM_Handle hHDMI	    /* [in] HDMI handle */
) ;



/******************************************************************************
Summary:
Enable (Display) TMDS Data (Output) lines

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	bEnableTmdsOutput  - True=Enable TMDS   False=Disable TMDS

Output:
	<None>

Returns:
	BERR_SUCCESS - TMDS Output Enabled.
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	When disabling TMDS data lines (Clock will be left on *if* connected to an Rx)
	Always set the TMDS data lines prior to setting the clock
	Clock is used for Rx Sense detection; Use BHDM_EnableTmdsClock to set clock as desired
	BHDM_EnableTmdsOutput (depracated) is a macro using BHDM_EnbaleTmdsData

See Also:
	BHDM_Handle
	BHDM_EnableTmdsClock

*******************************************************************************/

BERR_Code BHDM_EnableTmdsData(
   BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsOutput   /* [in] boolean to enable/disable */
) ;

/* keep older name for backwards compatibility */
#define BHDM_EnableTmdsOutput BHDM_EnableTmdsData


/******************************************************************************
Summary:
Enable TMDS Clock Only

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	bEnableTmdsClock  - True=Enable TMDS Clock  False=Disable TMDS Ckicj

Note:
	When disabling TMDS data lines (Clock will be left on *if* connected to an Rx)
	Always set the TMDS data lines prior to setting the clock
	Clock is used for Rx Sense detection; Use BHDM_EnableTmdsClock to set clock as desired
	BHDM_EnableTmdsOutput (depracated) is a macro using BHDM_EnbaleTmdsData

Output:
	<None>

Returns:
	BERR_SUCCESS - TMDS Clock Enabled.
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle
	BHDM_EnableTmdsData
*******************************************************************************/
BERR_Code BHDM_EnableTmdsClock(
   BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsClock	/* [in] boolean to enable/disable */
) ;


/******************************************************************************
Summary:
Set the AvMute (True/False) functionality for HDMI.

Description:
	The AvMute (TRUE) signals to the HDMI Receiver that the current audio and
	video data can be considered invalid.  The HDMI Receiver can use this to
	blank the display and mute the audio to minimize problems for the end user.

	AvMute (FALSE) signals the HDMI Receiver that it can begin processing audio
	and video data valid.

	This function is useful to notify the Receiver of changes in the TMDS clock
	frequency (i.e. channel change, format change etc.)


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	bEnableAvMute - Disable/Enable handling audio/video data.

Output:
	<none>

Returns:
	BERR_SUCCESS - AVMute was enabled/disabled
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	Both the audio and video our muted at the receiver.  They are not mutually
	exclusive.

See Also:
	o BHDM_Handle
	o BHDM_Open

*******************************************************************************/
BERR_Code BHDM_SetAvMute(
   BHDM_Handle hHDMI,              /* [in] HDMI handle */
   bool bEnableAvMute              /* [in] boolean to enable/disable */
) ;


/******************************************************************************
Summary:
	Mute/Unmute Audio on the HDMI interface.

Description:
	This function may be used to mute just the audio on HDMI. This is
	independent of the AvMute functionality (done via GCP). It may
	be used in conjunction with BHDM_SetPixelDataOverride to implement
	an A/V mute that does not depend on the receiver's handling of the
	AV_MUTE flag in the GCP.


Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

	bEnableAudioMute - Disable/Enable handling audio data.

Output:
	<none>

Returns:
	BERR_SUCCESS - AVMute was enabled/disabled
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	None

See Also:
	o BHDM_Handle
	o BHDM_Open

*******************************************************************************/
BERR_Code BHDM_SetAudioMute(
	BHDM_Handle hHDMI,			   /* [in] HDMI handle */
	bool bEnableAudioMute		   /* [in] boolean to enable/disable */
) ;



/******************************************************************************
Summary:
Initialize the Drift FIFO

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS -
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_InitializeDriftFIFO(
   BHDM_Handle hHDMI		/* [in] HDMI handle */
) ;



/***************************************************************************
Summary:
	Get the event handle for checking HDMI events.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	EventType - Type of event the requested handle is for (CEC, HDCP, etc.)

Output:
	pBHDMEvent - HDMI Event Handle

Returns:
	BERR_SUCCESS - Sucessfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_EventType

****************************************************************************/
BERR_Code BHDM_GetEventHandle(
   BHDM_Handle hHDMI,           /* [in] HDMI handle */
   BHDM_EventType EventType,    /* [in] HDMI Event Type */
   BKNI_EventHandle *pBHDMEvent	/* [out] event handle */
);



/***************************************************************************
Summary:
	Initialize the HDMI Packet RAM

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	<None>

Returns:
	BERR_SUCCESS - Sucessfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:

****************************************************************************/
BERR_Code BHDM_InitializePacketRAM(
   BHDM_Handle hHDMI          /* [in] HDMI handle */
) ;


/***************************************************************************
Summary:
	Callback used for notification of changes to the Audio or Video Rate.

Input:
	hHDMI  - HDMI control handle that was previously opened by BHDM_Open.
	CallbackType -  see BHDM_Callback_Type ;
	pvAudioOrVideoData - void pointer to structure containing information
	describing Audio or Video rate change

Output:
	<None>

Returns:
	<None>

See Also:
	BHDM_Callback_Type
	BVDC_Display_InstallRateChangeCallback
	BAUD_InstallRateChangeCallback
****************************************************************************/
void BHDM_AudioVideoRateChangeCB_isr(
	BHDM_Handle hHDMI,
	int   CallbackType,
	void *pvAudioOrVideoData) ;


/***************************************************************************
Summary:
	Enable the HDMI/DVI display output


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

    pHdmiSettings - Settings used to enable DVI/HDMI output

Output:
	<None>

Returns:
	BERR_SUCCESS - Enabled the display output
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:

****************************************************************************/
BERR_Code BHDM_EnableDisplay(
   const BHDM_Handle hHDMI,          /* [in] HDMI handle */
   const BHDM_Settings *pHdmiSettings  /* [in] HDMI Settings to enable Display with */
) ;



/***************************************************************************
Summary:
	Disable HDMI/DVI display output


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.


Output:
	<None>

Returns:
	BERR_SUCCESS - Display output disabled
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_EnableDisplay
	BHDM_EnableTmdsData

****************************************************************************/
BERR_Code BHDM_DisableDisplay(
   BHDM_Handle hHDMI          /* [in] HDMI handle */
) ;



/***************************************************************************
Summary:
	Get the current settings used for confuring the HDMI display and audio


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
    pHdmiSettings - Settings used to enable DVI/HDMI output
****************************************************************************/
void BHDM_GetHdmiSettings(
   BHDM_Handle hHDMI,          /* [in] HDMI handle */
   BHDM_Settings *pHdmiSettings  /* [in] HDMI Settings to enable Display with */
) ;


/***************************************************************************
Summary:
	Get the current status of the HDMI Core


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
    pHdmiStatus - Status of the HDMI core
****************************************************************************/
void BHDM_GetHdmiStatus(
   BHDM_Handle hHDMI,          /* [in] HDMI handle */
   BHDM_Status *pHdmiStatus  /* [out] HDMI Status  */
) ;


/***************************************************************************
Summary:
	Set the current settings used to configure the HDMI display and audio


Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pHdmiSettings - Settings used to enable DVI/HDMI output


Returns:
	BERR_SUCCESS - Settings returned in the pHdmiSettings structure

See Also:
	BHDM_EnableDisplay
****************************************************************************/
BERR_Code BHDM_SetHdmiSettings(
	BHDM_Handle hHDMI, /* [in] handle to HDMI device */
	BHDM_Settings *pHdmiSettings  /* [in] pointer to memory to hold the current  HDMI settings */
);


/***************************************************************************
Summary:
	Set the Timebase for the HDMI Rate Manager to use.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eTimebase - the selected Timebase to use

Returns:
	BERR_SUCCESS - Timebase Set for use
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	The selected Timebase should be the same Timebase used for the VEC
	Rate Manager configured for HD

See Also:
	BHDM_GetTimebase

****************************************************************************/
BERR_Code BHDM_SetTimebase(
   BHDM_Handle hHDMI,       /* [in] HDMI handle */
   BAVC_Timebase eTimebase  /* [in] HDMI Rate Manager Timebase to be used */
) ;



/***************************************************************************
Summary:
	Get the current Timebase in use by the HDMI Rate Manager

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	eTimebase - the current Timebase

Returns:
	BERR_SUCCESS - Timebase returned
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	The returned Timebase should be the same Timebase used for the VEC
	Rate Manager configured for HD

See Also:
	BHDM_SetTimebase
	BHDM_GetHDMISettings

****************************************************************************/
BERR_Code BHDM_GetTimebase(
   BHDM_Handle hHDMI,       /* [in] HDMI handle */
   BAVC_Timebase *eTimebase  /* [out] current HDMI Rate Manager Timebase */
) ;



/***************************************************************************
Summary:
	Enable the transmission of the specified packet id

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	ePacketId - Packet ID to enable


Returns:
	BERR_SUCCESS - Timebase returned
	BERR_INVALID_PARAMETER - Invalid function parameter.


See Also:
	BHDM_DisablePacketTransmission
	BHDM_SetUserDefinedPacket

****************************************************************************/
BERR_Code BHDM_EnablePacketTransmission(
   BHDM_Handle hHDMI,
   BHDM_Packet ePacketId) ;



/***************************************************************************
Summary:
	Disable the transmission of the specified packet id

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	ePacketId - Packet ID to disable


Returns:
	BERR_SUCCESS - Timebase returned
	BERR_INVALID_PARAMETER - Invalid function parameter.


See Also:
	BHDM_EnablePacketTransmission
	BHDM_SetUserDefinedPacket

****************************************************************************/
BERR_Code BHDM_DisablePacketTransmission(
	BHDM_Handle hHDMI,
	BHDM_Packet ePacketId) ;



/***************************************************************************
Summary:
	Create/Set a user defined packet

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	ePacketId - Packet ID to store the user defined packet
	PacketType  - Type ID of the Packet to transmit
	PacketVersion - Version of the Packet to transmit
	PacketLength - length of Packet Data excluding checksum (auto calculated)
	PacketData - pointer to the user packet data


Returns:
	BERR_SUCCESS - Packet Created and transmitted
	BERR_INVALID_PARAMETER - Invalid function parameter.

Note:
	The Packet Data checksum value is computed automatically and is not part
	of the Packet Data passed to BHDM_SetUserDefinedPacket.

	Caller must manage available User Defined packets; User Packets can be
	overwritten.


See Also:
	BHDM_EnablePacketTransmission
	BHDM_DisablePacketTransmission
****************************************************************************/
BERR_Code BHDM_SetUserDefinedPacket(BHDM_Handle hHDMI,
	BHDM_Packet ePacketId,

	uint8_t PacketType,
	uint8_t PacketVersion,
	uint8_t PacketLength,

	uint8_t *PacketData
) ;



/***************************************************************************
Summary:
	Configure Pre-Emphasis Control

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eValue - enumeration of the value to use on PreEmphasis Control


Returns:
	BERR_SUCCESS - Pre-Emphasis Control enabled with the selected value
	BERR_INVALID_PARAMETER - Invalid function parameter.


Note:
	Pre-Emphasis *may* be used for extended DVI/HDMI cable lengths.


See Also:
	BHDM_PreEmphasis
****************************************************************************/
BERR_Code BHDM_ConfigurePreEmphasis(
	BHDM_Handle hHDMI,
	BHDM_PreEmphasisSetting eValue
) ;


/***************************************************************************
Summary:
	Get current Pre-Emphasis Control settings

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	stPreEmphasisConfig - structure containing all the pre-emphasis settings


Returns:
	BERR_SUCCESS - Successfully retrieve current Pre-Emphasis settings

Note:
	Pre-Emphasis *may* be used for extended DVI/HDMI cable lengths.

See Also:
	BHDM_SetPreEmphasisConfiguration
****************************************************************************/
BERR_Code BHDM_GetPreEmphasisConfiguration(
	BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
);


/***************************************************************************
Summary:
	Set Pre-Emphasis Control with provided settings

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	stPreEmphasisConfig - structure containing all the pre-emphasis settings to be configured


Returns:
	BERR_SUCCESS - Pre-Emphasis Control enabled with the selected value
	BERR_INVALID_PARAMETER - Provided settings are out of range

Note:
	Pre-Emphasis *may* be used for extended DVI/HDMI cable lengths.

See Also:
	BHDM_GetPreEmphasisConfiguration
****************************************************************************/
BERR_Code BHDM_SetPreEmphasisConfiguration(
	BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
);


/******************************************************************************
Summary:
Set/Enable the Source Product Description Info Frame packet to be sent to the HDMI Rx

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.


Returns:
	BERR_SUCCESS - Packet Created and transmitted
	BERR_INVALID_PARAMETER - Invalid function parameter.


See Also:
*******************************************************************************/
BERR_Code BHDM_SetSPDInfoFramePacket(
   BHDM_Handle hHDMI          /* [in] HDMI handle */
) ;



/******************************************************************************
Summary:
Get Receiver Sense Value

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	ReceiverSense - 1 Rx Powered On; 0 Rx Powered Off


Returns:
	BERR_SUCCESS - Receiver Sense returned
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
*******************************************************************************/
void BHDM_GetReceiverSense(
	const BHDM_Handle hHDMI,
	bool *DeviceAttached,
	bool *ReceiverSense);

/******************************************************************************
Summary:
Display content of Vendor Specific InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstVendorSpecificInfoFrame - pointer to Vendor Specific InfoFrame

Returns:
	None

See Also:
	BHDM_SetVendorSpecificInfoFrame
	BHDM_GetVendorSpecificInfoFrame
*******************************************************************************/
#define BHDM_DisplayVendorSpecificInfoFrame(hHDMI,pstPacket) \
	BAVC_HDMI_DisplayVendorSpecificInfoFrame(NULL, pstPacket)


/******************************************************************************
Summary:
Set Vendor Specific InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	stVendorSpecificInfoFrame - structure containing Vendor Specific Info

Returns:
	BERR_SUCCESS - Vendor Specific  Info Frame data set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_GetVendorSpecificInfoFrame
	BHDM_DisplayVendorSpecificInfoFrame
*******************************************************************************/
BERR_Code BHDM_SetVendorSpecificInfoFrame(
	BHDM_Handle hHDMI,          /* [in] HDMI handle */
	const BAVC_HDMI_VendorSpecificInfoFrame *stVendorSpecificInfoFrame
) ;


/******************************************************************************
Summary:
Get Vendor Specific  InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pVendorSpecificInfoFrame - pointer to structure to hold the Vendor Specific Infoframe values


Returns:
	None
See Also:
	BHDM_SetVendorSpecificInfoFramePacket
	BHDM_DisplayVendorSpecificInfoFrame
*******************************************************************************/
void  BHDM_GetVendorSpecificInfoFrame(
	BHDM_Handle hHDMI,          /* [in] HDMI handle */
	BAVC_HDMI_VendorSpecificInfoFrame *pVendorSpecficInfoFrame
) ;


/******************************************************************************
Summary:
Display AVI InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAviInfoFrame - pointer to Audio Info Frame structure


Returns:
	None

See Also:
	BHDM_GetAviInfoFramePacket
	BHDM_SetAviInfoFramePacket
*******************************************************************************/
#define BHDM_DisplayAVIInfoFrame(hHDMI,pstPacket) \
	BAVC_HDMI_DisplayAVIInfoFramePacket(NULL, pstPacket)

/******************************************************************************
Summary:
Convert the input video format, aspect ration, and pixel repetition triple
into a CEA861 VIC for setting the VideoID in the AVIIF.

Input:
    eVideoFmt - The video format input to the HDMI core from BVN
    eAspectRatio - The aspect ratio of the video coming from BVN
    ePixelRepetition - The pixel repetition setting set by the user
Output:
    pVideoID - pointer to a byte holding the VIC
    pPixelRepeat - pointer to a byte holding the validated pixel repeat setting

Returns:
    None

See Also:
    BHDM_GetAviInfoFramePacket
    BHDM_SetAviInfoFramePacket
*******************************************************************************/
void BHDM_VideoFmt2CEA861Code(
    BFMT_VideoFmt eVideoFmt,
    BFMT_AspectRatio eAspectRatio,
    BAVC_HDMI_PixelRepetition ePixelRepetition,
    uint8_t *pVideoID,
    uint8_t *pPixelRepeat
) ;

/******************************************************************************
Summary:
Set AVI InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAviInfoFrame - values to set info frame to


Returns:
	BERR_SUCCESS - Avi Info Frame Packet information set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_GetAviInfoFramePacket
	BHDM_DisplayAviInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_SetAVIInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_AviInfoFrame *pstAviInfoFrame
) ;



/******************************************************************************
Summary:
Get current AVI InfoFrame Settings

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAviInfoFrame - structure to copy AVI Info Frame values to


Returns:
	BERR_SUCCESS - Avi Info Frame Packet information returned
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_SetAviInfoFramePacket
	BHDM_DisplayAviInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_GetAVIInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_AviInfoFrame *pstAviInfoFrame
) ;



/******************************************************************************
Summary:
Display Audio InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAudioInfoFrame - pointer to Audio Info Frame structure


Returns:
	None

See Also:
	BHDM_GetAudioInfoFramePacket
	BHDM_SetAudioInfoFramePacket
*******************************************************************************/
#define BHDM_DisplayAudioInfoFramePacket(hHDMI,pstPacket) \
	BAVC_HDMI_DisplayAudioInfoFramePacket(NULL, pstPacket)

/******************************************************************************
Summary:
Set Audio InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAudioInfoFrame - pointer to structure to hold the Audio InfoFrame values


Returns:
	BERR_SUCCESS - Avi Info Frame Packet information set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_GetAudioInfoFramePacket
	BHDM_DisplayAudioInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_SetAudioInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame
) ;



/******************************************************************************
Summary:
Get current Audio InfoFrame Settings

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAudioInfoFrame - pointer to structure to copy Audio Info Frame values to


Returns:
	BERR_SUCCESS - Audio Info Frame Packet information returned
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_SetAudioInfoFramePacket
	BHDM_DisplayAudioInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_GetAudioInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame
) ;


/******************************************************************************
Summary:
Display DRM InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstDrmInfoFrame - pointer to DRM Info Frame structure


Returns:
	None

See Also:
	BHDM_GetDrmInfoFramePacket
	BHDM_SetDrmInfoFramePacket
*******************************************************************************/
#define BHDM_DisplayDRMInfoFramePacket(hHDMI,pstPacket) \
	BAVC_HDMI_DisplayDRMInfoFramePacket(NULL, pstPacket)


/******************************************************************************
Summary:
Set DRM InfoFrame

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstDrmInfoFrame - values to set info frame to


Returns:
	BERR_SUCCESS - Drm Info Frame Packet information set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_GetDrmInfoFramePacket
	BHDM_DisplayDrmInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_SetDRMInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_DRMInfoFrame *pstDrmInfoFrame
) ;



/******************************************************************************
Summary:
Get current DRM InfoFrame Settings

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstDrmInfoFrame - structure to copy DRM Info Frame values to


Returns:
	BERR_SUCCESS - Drm Info Frame Packet information returned
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BHDM_HARDWARE_ERROR    - Hardware Error with Device and/or Cable


See Also:
	BHDM_SetDrmInfoFramePacket
	BHDM_DisplayDrmInfoFramePacket
*******************************************************************************/
BERR_Code BHDM_GetDRMInfoFramePacket(
	BHDM_Handle hHDMI,
	BAVC_HDMI_DRMInfoFrame *pstDrmInfoFrame
) ;

typedef enum {
    BHDM_HdrRamMode_eDisable = 0,
    BHDM_HdrRamMode_eBitInjection1,
    BHDM_HdrRamMode_eBitInjection2,
    BHDM_HdrRamMode_eMetadataPacket
} BHDM_HdrRamMode;

/******************************************************************************
Summary:
	Set HDR RAM mode

Input:
	BHDM_HdrRamMode

Returns:
	BERR_SUCCESS - HDR RAM mode successfully set
	BERR_NOT_SUPPORTED - HDR RAM mode not supported
*******************************************************************************/
BERR_Code BHDM_SetHdrRamMode(
    BHDM_Handle hHDMI,          /* [in] HDMI handle */
    BHDM_HdrRamMode mode
);

/******************************************************************************
Summary:
	Set pixel repetition

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	ePixelRepetition - pixel repetition configuration to set


Returns:
	BERR_SUCCESS - color depth mode successfully set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BERR_NOT_SUPPORTED - requested color depth is not supported

*******************************************************************************/
BERR_Code BHDM_SetPixelRepetition(
	BHDM_Handle hHDMI,		   /* [in] HDMI handle */
	BAVC_HDMI_PixelRepetition ePixelRepetition
) ;


/******************************************************************************
Summary:
Display ACR Packet Configuration

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pstAcrPacketConfig - pointer to ACR Configuration


Returns:
	None

See Also:
	None
*******************************************************************************/
void BHDM_PACKET_ACR_DisplayConfiguration(
		const BHDM_Handle hHDMI,			/* [in] HDMI handle */
		const BHDM_PACKET_ACR_CONFIG *pstAcrPacketConfig) ;


/******************************************************************************
Summary:
	Get pixel repetition

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	*ePixelRepetition - the current pixel repetition configuration


Returns:
	BERR_SUCCESS - color depth mode successfully set
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BERR_NOT_SUPPORTED - requested color depth is not supported

*******************************************************************************/
BERR_Code BHDM_GetPixelRepetition(
   BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_PixelRepetition *ePixelRepetition
);


/******************************************************************************
Summary:
	Set the HDMI data transferring mode (Master or Slave)

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	masterMode - a boolean indicating master (true) or slave (false) mode


Returns:
	BERR_SUCCESS - data transfer mode successfully set

*******************************************************************************/
BERR_Code BHDM_SetHdmiDataTransferMode(
	BHDM_Handle hHDMI,
	bool masterMode
);


/******************************************************************************
Summary:
	Get the current HDMI data transferring mode (Master or Slave)

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	masterMode - a boolean indicating master (true) or slave (false) mode


Returns:
	BERR_SUCCESS - successfully retrieve data transfer mode

*******************************************************************************/
BERR_Code BHDM_GetHdmiDataTransferMode(
	BHDM_Handle hHDMI,
	bool *masterMode
);


/******************************************************************************
Summary:
	Check for valid/stable video output before starting HDCP authentication

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

*******************************************************************************/
BERR_Code BHDM_CheckForValidVideo(
	BHDM_Handle hHDMI) ;


/******************************************************************************
Summary:
	install Hot Plug Change Callback to notify of HP dtect changes

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pfCallback_isr - callback for hotplug change

*******************************************************************************/
BERR_Code BHDM_InstallHotplugChangeCallback(
	BHDM_Handle hHDMI,			/* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr, /* [in] cb for informing hotplug change */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2) ;	/* [in] the second argument(int) passed to the callback function */


/******************************************************************************
Summary:
	Uninstall HotPlug Change Callback

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	pfCallback_isr - callback for hotplug change

*******************************************************************************/
BERR_Code BHDM_UnInstallHotplugChangeCallback(
	BHDM_Handle hHDMI,						 /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr) ; /* [in] cb for hotplug change */


/***************************************************************************
Summary:
	HDMI standby settings

****************************************************************************/
typedef struct BHDM_StandbySettings
{
	bool bEnableWakeup; /* If true, then allows wakeup from standby using HDM.
	                       If false, the device is powered down during standby */
} BHDM_StandbySettings;

/******************************************************************************
Summary:
	Get default HDMI standby settings

*******************************************************************************/
void BHDM_GetDefaultStandbySettings(
	BHDM_StandbySettings *pSettings
	);


/******************************************************************************
Summary:
	Enter standby mode

*******************************************************************************/
BERR_Code BHDM_Standby(
	BHDM_Handle hHDMI, /* [in] HDMI Handle */
	const BHDM_StandbySettings *pSettings /* optional */
	);

/******************************************************************************
Summary:
	Resume from standby mode

*******************************************************************************/
BERR_Code BHDM_Resume(
	BHDM_Handle hHDMI /* [in] HDMI Handle */
	);


BERR_Code BHDM_GetCrcValue_isr(
	BHDM_Handle hHDMI,	/* [in] HDMI Handle */
	BHDM_CrcData *stCrcData
);

void BHDM_SCDC_DisableScrambleTx(BHDM_Handle hHDMI) ;
BERR_Code BHDM_SCDC_ConfigureScrambling(const BHDM_Handle hHDMI) ;

typedef struct BHDM_ScrambleConfig {
	uint8_t rxTmdsConfig ; /* TMDS Config written the Rx see HDMI 2.0 10.4.1.4 */
	uint8_t rxStatusFlags_Scramble ; /* Scrambler Status read from Rx see HDMI 2.0 10.4.1.5 */
	uint8_t clockWordSelect ;  /* Tx Config setting for HDMI Tx Phy */
	uint8_t txScrambleEnable  ; /* Tx Config setting for HDMI Tx Phy */
} BHDM_ScrambleConfig ;



void BHDM_SCDC_GetScrambleConfiguration(
	BHDM_Handle hHDMI,
	BHDM_ScrambleConfig * pstSettings) ;

/******************************************************************************
Summary:
	Support to get/set PreEmphasis based on TMDS Rate

	Modifications to PreEmphasis settings are expected at initialization time only
	Users must know/understand the modifications made with these APIs.
	Modification to PreEmphasis settings can adversely affect HDMI Compliance and interoperability
*******************************************************************************/
void BHDM_TMDS_GetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters) ;

void BHDM_TMDS_SetPreEmphasisRegisters(
	BHDM_Handle hHDMI,
	const BHDM_TmdsPreEmphasisRegisters *TmdsPreEmphasisRegisters) ;


/******************************************************************************
Summary:
	Print a debug report on the parsed information from the EDID
*******************************************************************************/
void BHDM_EDID_DEBUG_PrintData(BHDM_Handle hHDMI) ;


#ifdef __cplusplus
}
#endif

#endif /* BHDM_H__ */
