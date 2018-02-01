/***************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Module Description:
*
***************************************************************************/

#ifndef BCEC_H__
#define BCEC_H__

#include "bchp.h"		/* Chip Info */
#include "breg_mem.h"	/* Chip register access. */
#include "bkni.h"		/* Kernel Interface */
#include "bint.h"		/* Interrupt */
#include "btmr.h"

#include "bavc_hdmi.h"
#include "bchp_hdmi.h"
#include "bcec_config.h"


#ifdef __cplusplus
extern "C" {
#endif

/************************* Module Overview ********************************
  The CEC (Consumer Electronic Control) functions provide support for sending
  and receving messages used for the control of HDMI devices connected
  together.

  These functions do not interpret CEC messages, but buffers them for access
  by application software.	Support is also provided to send messages to
  other HDMI connected devices.
***************************************************************************/


/******************************************************************************
Summary:
CEC Context Handle
*******************************************************************************/
typedef struct BCEC_P_Handle *BCEC_Handle ;


/******************************************************************************
Summary:
CEC Function Return Values
*******************************************************************************/
#define BCEC_BASE_ERRORS 0

#define BCEC_NOT_IMPLEMENTED			BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 0)
#define BCEC_NO_PHYS_ADDR				BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 1)
#define BCEC_ASSIGNED_FREEUSE_ADDR		BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 2)
#define BCEC_ASSIGNED_UNREGISTERED_ADDR BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 3)
#define BCEC_NO_LOGICAL_ADDR			BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 4)
#define BCEC_EXCEED_MAX_DEVICE_LEVELS	BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 5)
#define BCEC_TRANSMIT_ERROR 			BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 6)
#define BCEC_RECEIVE_ERROR				BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 7)
#define BCEC_NOT_ENOUGH_MEM 			BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 8)
#define BCEC_EXCEED_MSG_LENGTH			BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 9)
#define BCEC_TRANSMIT_BUSY				BERR_MAKE_CODE(BERR_CEC_ID, BCEC_BASE_ERRORS+ 10)

/* CEC Broadcast address */
#define BCEC_BROADCAST_ADDR  0xF


/**************************************
  * CEC Messages & corresponding Opcodes   *
 **************************************/
#define BCEC_OpCode_FeatureAbort	 0x00
#define BCEC_OpCode_ImageViewOn    0x04
#define BCEC_OpCode_TunerStepIncrement	  0x05
#define BCEC_OpCode_TunerStepDecrement	  0x06
#define BCEC_OpCode_TunerDeviceStatus	 0x07
#define BCEC_OpCode_DiveTunerDeviceStatus	 0x08
#define BCEC_OpCode_RecordOn	0x09
#define BCEC_OpCode_RecordStatus	0x0A
#define BCEC_OpCode_RecordOff	 0x0B
#define BCEC_OpCode_TextViewOn	  0x0D
#define BCEC_OpCode_RecordTVScreen	  0x0F
#define BCEC_OpCode_GiveDeckStatus	  0x1A
#define BCEC_OpCode_DeckStatus	 0x1B
#define BCEC_OpCode_SetMenuLanguage    0x32
#define BCEC_OpCode_ClearAnalogueTimer	  0x33
#define BCEC_OpCode_SetAnalogueTimer   0x34
#define BCEC_OpCode_TimerStatus    0x35
#define BCEC_OpCode_Standby    0x36
#define BCEC_OpCode_Play	0x41
#define BCEC_OpCode_DeckControl    0x42
#define BCEC_OpCode_TimerClearedStatus	  0x43
#define BCEC_OpCode_UserControlPressed	  0x44
#define BCEC_OpCode_UserControlReleased    0x45
#define BCEC_OpCode_GiveOSDName    0x46
#define BCEC_OpCode_SetOSDName	  0x47
#define BCEC_OpCode_SetOSDString	0x64
#define BCEC_OpCode_SetTimerProgramTitle	0x67
#define BCEC_OpCode_SystemAudioModeRequest	  0x70
#define BCEC_OpCode_GiveAudioStatus    0x71
#define BCEC_OpCode_SetSystemAudioMode	  0x72
#define BCEC_OpCode_ReportAudioStatus	 0x7A
#define BCEC_OpCode_GiveSystemAudioModeStatus	 0x7D
#define BCEC_OpCode_SystemAudioModeStatus	 0x7E
#define BCEC_OpCode_RoutingChange	 0x80
#define BCEC_OpCode_RoutingInformation	  0x81
#define BCEC_OpCode_ActiveSource	0x82
#define BCEC_OpCode_GivePhysicalAddress    0x83
#define BCEC_OpCode_ReportPhysicalAddress	 0x84
#define BCEC_OpCode_RequestActiveSource   0x85
#define BCEC_OpCode_SetStreamPath	 0x86
#define BCEC_OpCode_DeviceVendorID	  0x87
#define BCEC_OpCode_VendorCommand	 0x89
#define BCEC_OpCode_VendorRemoteButtonDown	  0x8A
#define BCEC_OpCode_VendorRemoteButtonUp	0x8B
#define BCEC_OpCode_GiveDeviceVendorID	  0x8C
#define BCEC_OpCode_MenuRequest    0x8D
#define BCEC_OpCode_MenuStatus	  0x8E
#define BCEC_OpCode_GiveDevicePowerStatus	 0x8F
#define BCEC_OpCode_ReportPowerStatus	 0x90
#define BCEC_OpCode_GetMenuLanguage    0x91
#define BCEC_OpCode_SelectAnalogueService	 0x92
#define BCEC_OpCode_SelectDigitalService	0x93
#define BCEC_OpCode_SetDigitalTimer    0x97
#define BCEC_OpCode_ClearDigitalTimer	 0x99
#define BCEC_OpCode_SetAudioRate   0x9A
#define BCEC_OpCode_InActiveSource	 0x9D
#define BCEC_OpCode_CECVersion	  0x9E
#define BCEC_OpCode_GetCECVersion	 0x9F
#define BCEC_OpCode_VendorCommandWithID    0xA0
#define BCEC_OpCode_ClearExternalTimer	  0xA1
#define BCEC_OpCode_SetExternalTimer	0xA2
#define BCEC_OpCode_Abort	 0xFF
#define BCEC_VERSION	0x05


typedef enum
{
	BCEC_EventCec_eTransmitted,
	BCEC_EventCec_eReceived
} BCEC_EventType ;


typedef enum BCEC_DeviceType
{
	BCEC_DeviceType_eTv = 0,
	BCEC_DeviceType_eRecordingDevice,
	BCEC_DeviceType_eReserved,
	BCEC_DeviceType_eTuner,
	BCEC_DeviceType_ePlaybackDevice,
	BCEC_DeviceType_eAudioSystem,
	BCEC_DeviceType_ePureCecSwitch,
	BCEC_DeviceType_eVideoProcessor,
	BCEC_DeviceType_eMax
} BCEC_DeviceType;


typedef struct BCEC_DeviceFeatures
{
	bool tvSupportsRecordTvScreen;
	bool tvSupportsSetOsdString;
	bool supportsDeckControl;
	bool sourceSupportsSetAudioRate;
	bool sinkSupportsArcTx;
	bool sourceSupportsArcRx;
} BCEC_DeviceFeatures;


typedef enum BCEC_Hdmi
{
	BCEC_Hdmi_eTx0 = 0,
	BCEC_Hdmi_eRx0,
	BCEC_Hdmi_eMax
} BCEC_Hdmi;


typedef struct BCEC_Dependencies
{
	BCHP_Handle hChip;
	BREG_Handle hRegister;
	BINT_Handle hInterrupt;
	BTMR_Handle hTmr ;
	BCEC_Hdmi eCecHdmiPath;
} BCEC_Dependencies;


typedef struct BCEC_Settings
{
	bool enable;
	uint8_t CecLogicalAddr;
	uint8_t CecPhysicalAddr[2];
	BCEC_DeviceType eDeviceType;
	bool enableAutoOn;
	BCEC_Hdmi eCecHdmiPath;
	unsigned cecVersion;
	BCEC_DeviceFeatures stDeviceFeatures;

} BCEC_Settings;


typedef struct BCEC_MessageStatus
{
	BAVC_HDMI_CEC_IntMessageType eMessageType; /* Interrupt Message Type (Rx or Tx) */
	uint8_t uiStatus;
	uint8_t uiMessageLength;
	uint8_t uiEOM;
	bool bWokeUp;
} BCEC_MessageStatus;


/******************************************************************************
 * Public API
 ******************************************************************************/


/******************************************************************************
BERR_Code BCEC_Open
Summary: Open/Initialize the CEC device
*******************************************************************************/
BERR_Code BCEC_Open(
   BCEC_Handle *phCEC,	  /* [out] CEC handle */
   const BCEC_Dependencies *pstDependencies
);


/******************************************************************************
void BCEC_Close
Summary: Close the CEC handle
*******************************************************************************/
void BCEC_Close(
   BCEC_Handle hCEC  /* [in] CEC handle */
);


/***************************************************************************
void BCEC_GetEventHandle
Summary: Get the event handle for checking CEC events.
****************************************************************************/
void BCEC_GetEventHandle(
   BCEC_Handle hCEC,			/* [in] HDMI handle */
   BCEC_EventType eEventType,
   BKNI_EventHandle *pBCECEvent /* [out] event handle */
);


/******************************************************************************
Summary:
Get default settings

Output
	pstDefaultCecSetting - default settings

Returns:
	BERR_SUCCESS

See Also:
	BCEC_SetSettings
******************************************************************************/
void BCEC_GetDefaultSettings(
	BCEC_Settings *pstDefaultCecSettings
) ;


/******************************************************************************
Summary:
Get the current CEC settings

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	pstCecSettings - returned CEC settings

See Also:
	BCEC_SetSettings
*******************************************************************************/
void BCEC_GetSettings(
	BCEC_Handle hCEC,		  /* [in] CEC handle */
	BCEC_Settings *pstCecSettings
)  ;


/******************************************************************************
Summary:
	Set CEC settings for the current device

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.
	pstCecSettings - CEC settings to be applied

Returns:
	BERR_SUCCESS - Successfully set CEC settings

See Also:
	o BCEC_GetSettings
*******************************************************************************/
BERR_Code BCEC_SetSettings(
	BCEC_Handle hCEC,		   /* [in] CEC handle */
	const BCEC_Settings *pstCecSettings
) ;


/******************************************************************************
Summary:
	Get the transmitted message status from the last CEC_SENT interrupt

Description:
	Read the Interrupt and Status bits to determine the result of transmitted CEC messages

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	pstCecMessageStatus - transmitted Cec message status
*******************************************************************************/
void BCEC_GetTransmitMessageStatus(
	BCEC_Handle hCEC,	   /* [in] CEC handle */
	BCEC_MessageStatus *pstCecMessageStatus /* [out] */
);


/******************************************************************************
Summary:
	Get the received message status from the last CEC_RECEIVED Interrupt

Description:
	Read the Interrupt and Status bits to determine the result of received CEC messages

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	pstCecMessageStatus - received Cec message status
*******************************************************************************/
void BCEC_GetReceivedMessageStatus(
	BCEC_Handle hCEC,	   /* [in] CEC handle */
	BCEC_MessageStatus *pstCecMessageStatus /* [out] */
);


/******************************************************************************
Summary:
	Transmit CEC Polling message

Description:
	Transmit formatted CEC Polling message to determine the device logical address.

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.
	LogicalAddr - The logical address to poll for

Output:
	<none>

Returns:
	BERR_SUCCESS - Successfully set up the message for transmission
	BCEC_TRANSMIT_BUSY - Busy transmitting another CEC message

See Also:
	o BCEC_XmitMsg
	o BCEC_RecvMsg
*******************************************************************************/
BERR_Code BCEC_PingLogicalAddr(
	BCEC_Handle hCEC,	  /* [in] CEC handle */
	uint8_t uiLogicalAddr /* [in] device logical address */
);


/******************************************************************************
Summary:
	Transmit a CEC message

Description:
	Transmit the prevously formatted CEC message.  The message has the
	destinaton embedded in the header block of the message.

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.
	pMessageData - message to be transmitted

Output:
	<none>

Returns:
	BERR_SUCCESS - Successfully set up the message for transmission
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BERR_CEC_TRANSMIT_ERROR - Error Transmittiing CEC Message


See Also:
	o BCEC_RecvMsg

*******************************************************************************/
BERR_Code BCEC_XmitMessage(
	BCEC_Handle hCEC,	  /* [in] CEC handle */
	const BAVC_HDMI_CEC_MessageData *pMessageData 	/* [in] ptr to storage for received CEC msg */
);


/******************************************************************************
Summary:
	Enable the CEC Core to receive a CEC message

Description:
	This function will enable the Core to receive messsages on the CEC bus.
	BCEC_EnableReceive should be called after each message is received and
	acknowledge so the next CEC message intended for the core can be received

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	None

See Also:
	o BCEC_RecvMsg

*******************************************************************************/
void BCEC_EnableReceive(
	BCEC_Handle hCEC	 /* [in] CEC handle */
) ;


/******************************************************************************
Summary:
	Get a received CEC message

Description:
	Copy the contents of the received CEC message to a data structure for use.

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	pRecvMessageData - pointer to memory location to hold received message data

Returns:
	BERR_SUCCESS - Successfully retrieved the stored CEC message.

See Also:
	o BCEC_XmitMsg

*******************************************************************************/
void BCEC_GetReceivedMessage(
	BCEC_Handle hCEC,			/* [in] CEC handle */
	BAVC_HDMI_CEC_MessageData *pRecvMessageData 	/* [out] ptr to storage for received CEC msg */
) ;


/******************************************************************************
Summary:
	Report the Physical Address and Device Type as a broadcast message.

Description:
	CEC Devices shall report the association between its logical and physical
	addresses by broadcasting the <Report Physical Address> message

Input:
	hCEC - CEC control handle that was previously opened by BCEC_Open.

Output:
	<none>

Returns:
	BERR_SUCCESS - Successfully broadcast the <Report Physical Address> message
	BERR_INVALID_PARAMETER - Invalid function parameter.
	BCEC_RECEIVE_ERROR - Error broadcasting <Report Physical Address>
	message


See Also:

*******************************************************************************/
BERR_Code BCEC_ReportPhysicalAddress(
	BCEC_Handle hCEC	 /* [in] CEC handle */
) ;


/***************************************************************************
Summary:
	CEC standby settings

****************************************************************************/
typedef struct BCEC_StandbySettings
{
	unsigned unused;
} BCEC_StandbySettings;


/******************************************************************************
Summary: Enter standby mode
*******************************************************************************/
BERR_Code BCEC_Standby(
	BCEC_Handle hCEC, /* [in] CEC Handle */
	const BCEC_StandbySettings *pSettings
	);


/******************************************************************************
Summary: Resume standby mode
*******************************************************************************/
BERR_Code BCEC_Resume(
	BCEC_Handle hCEC /* [in] CEC Handle */
	);


#ifdef __cplusplus
}
#endif

#endif /* ifndef BCEC_H__ */
/* End of File */


