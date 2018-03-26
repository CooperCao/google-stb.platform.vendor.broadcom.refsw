/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef BHDR_PRIV_H__
#define BHDR_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "bchp_sun_top_ctrl.h"
#include "bchp_common.h"


#include "bchp_hdmi_rx_0.h"
#include "bchp_hdmi_rx_intr2_0.h"
#include "bchp_int_id_hdmi_rx_intr2_0.h"

#if BHDR_HAS_MULTIPLE_PORTS
#include "bchp_hdmi_rx_1.h"
#include "bchp_hdmi_rx_intr2_1.h"
#include "bchp_int_id_hdmi_rx_intr2_1.h"
#endif

#include "bchp_dvp_hr.h"

#include "bavc_hdmi.h"

#include "bhdr_fe.h"
#include "bhdr_packet_priv.h"
#include "bhdr_hdcp.h"

#if BHDR_CONFIG_HDCP2X_SUPPORT
#include "bchp_hdcp2_rx_0.h"
#endif


#define BHDR_P_AUDIO_ERROR_FREE_FRAMES 20

/* number of errors in a field to consider the stream unstable */
#define BHDR_P_PACKET_ERROR_THRESHOLD 5

/* number of frames to be free of errors to consider stream OK */
#define BHDR_P_MAX_ERROR_FREE_PACKET_FRAMES 5

#define BHDR_P_MICROSECONDS_PER_SECOND 1000000

/* time units for BTMR which returns microseconds */
#define BHDR_P_MILLISECOND 1000
#define BHDR_P_SECOND 1000000


#define BHDR_P_MAX_SYMBOL_LOSS_COUNT      (3)

#define	MAKE_INTR_ENUM(IntName)	BHDR_INTR_e##IntName
#define	MAKE_INTR_NAME(IntName)	"BHDR_" #IntName

#define BHDR_PACKET_HEADER_LEN 3

#define BHDR_P_NUM_PACKETS 28
#define BHDR_P_PACKET_BYTES  28
#define BHDR_P_PACKET_WORDS 9

typedef enum
{
	BHDR_P_TIMER_eOtpCrcCalculation,
	BHDR_P_TIMER_eHdmiFeReset,
	BHDR_P_TIMER_eDviHdmiModeChange
} BHDR_P_TIMER ;


#define BCHP_INT_ID_HDMI_RX_INTR2_0_SET_AV_MUTE                       BCHP_INT_ID_SET_AV_MUTE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_AV_MUTE_UPDATE                    BCHP_INT_ID_AV_MUTE_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_DVI_TO_HDMI                       BCHP_INT_ID_DVI_TO_HDMI
#define BCHP_INT_ID_HDMI_RX_INTR2_0_HDMI_TO_DVI                       BCHP_INT_ID_HDMI_TO_DVI
#define BCHP_INT_ID_HDMI_RX_INTR2_0_AKSV_UPDATE                       BCHP_INT_ID_AKSV_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_REQUEST_KEYS                      BCHP_INT_ID_REQUEST_KEYS
#define BCHP_INT_ID_HDMI_RX_INTR2_0_REQUEST_KSVS                      BCHP_INT_ID_REQUEST_KSVS
#define BCHP_INT_ID_HDMI_RX_INTR2_0_I2C_TRANSACTION_COMPLETE          BCHP_INT_ID_I2C_TRANSACTION_COMPLETE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_PJ_UPDATE                         BCHP_INT_ID_PJ_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_SYMBOL_LOSS                       BCHP_INT_ID_SYMBOL_LOSS
#define BCHP_INT_ID_HDMI_RX_INTR2_0_INVALID_DATA_ISLAND_LENGTH        BCHP_INT_ID_INVALID_DATA_ISLAND_LENGTH
#define BCHP_INT_ID_HDMI_RX_INTR2_0_CHANNEL_STATUS_UPDATE             BCHP_INT_ID_CHANNEL_STATUS_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_VALIDITY_BIT_UPDATE         BCHP_INT_ID_AUDIO_VALIDITY_BIT_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_RAM_PACKET_UPDATE                 BCHP_INT_ID_RAM_PACKET_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_RAM_PACKET_STOP                   BCHP_INT_ID_RAM_PACKET_STOP
#define BCHP_INT_ID_HDMI_RX_INTR2_0_PACKET_SYNC_ERROR                 BCHP_INT_ID_PACKET_SYNC_ERROR
#define BCHP_INT_ID_HDMI_RX_INTR2_0_LAYOUT_UPDATE                     BCHP_INT_ID_LAYOUT_UPDATE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_TYPE_CHANGE                 BCHP_INT_ID_AUDIO_TYPE_CHANGE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_VSYNC_LEAD_EDGE                   BCHP_INT_ID_VSYNC_LEAD_EDGE
#define BCHP_INT_ID_HDMI_RX_INTR2_0_VERTICAL_BLANK_END                BCHP_INT_ID_VERTICAL_BLANK_END
#if !defined(BCHP_INT_ID_HDMI_RX_INTR2_0_EXCESSIVE_PACKET_ERRORS)
#define BCHP_INT_ID_HDMI_RX_INTR2_0_EXCESSIVE_PACKET_ERRORS           BCHP_INT_ID_EXCESSIVE_PACKET_ERRORS
#endif
#define BCHP_INT_ID_HDMI_RX_INTR2_0_FORMAT_DETECT_COUNT_SATURATED     BCHP_INT_ID_FORMAT_DETECT_COUNT_SATURATED
#define BCHP_INT_ID_HDMI_RX_INTR2_0_ERROR_INTERRUPT                   BCHP_INT_ID_ERROR_INTERRUPT

/******************************************************************************
Summary:
Enumeration of BHDR_Interrupts
*******************************************************************************/
typedef enum
{
	/* 00 */ MAKE_INTR_ENUM(SET_AV_MUTE),

	/* 01 */ MAKE_INTR_ENUM(AV_MUTE_UPDATE),

	/* 02 */ MAKE_INTR_ENUM(DVI_TO_HDMI),
	/* 03 */ MAKE_INTR_ENUM(HDMI_TO_DVI),

	/* 04 */ MAKE_INTR_ENUM(AKSV_UPDATE),
	/* 05 */ MAKE_INTR_ENUM(REQUEST_KEYS),
	/* 06 */ MAKE_INTR_ENUM(REQUEST_KSVS),

	/* 07 */ MAKE_INTR_ENUM(I2C_TRANSACTION_COMPLETE),

	/* 08 */ MAKE_INTR_ENUM( PJ_UPDATE ),


	/* 09 */ MAKE_INTR_ENUM(SYMBOL_LOSS),
	/* 10 */ MAKE_INTR_ENUM(INVALID_DATA_ISLAND_LENGTH),
	/* 11 */ MAKE_INTR_ENUM(CHANNEL_STATUS_UPDATE),
	/* 12 */ MAKE_INTR_ENUM(AUDIO_VALIDITY_BIT_UPDATE),

	/* 13 */ MAKE_INTR_ENUM(RAM_PACKET_UPDATE),
	/* 14 */ MAKE_INTR_ENUM(RAM_PACKET_STOP),
	/* 15 */ MAKE_INTR_ENUM(PACKET_SYNC_ERROR),
	/* 16 */ MAKE_INTR_ENUM(LAYOUT_UPDATE),
	/* 17 */ MAKE_INTR_ENUM(AUDIO_TYPE_CHANGE),

	/* 27 */ MAKE_INTR_ENUM(VSYNC_LEAD_EDGE),
	/* 28 */ MAKE_INTR_ENUM(VERTICAL_BLANK_END),

	/* 29 */ MAKE_INTR_ENUM(EXCESSIVE_PACKET_ERRORS),
	/* 30 */ MAKE_INTR_ENUM(FORMAT_DETECT_COUNT_SATURATED),

	/* 31 */ MAKE_INTR_ENUM(ERROR_INTERRUPT),


	/* 32 */ MAKE_INTR_ENUM(LAST)

} BHDR_P_InterruptMask ;


typedef struct BHDR_P_CallbackFunc
{
	BLST_S_ENTRY(BHDR_P_CallbackFunc) link ;
	BHDR_CallbackFunc pfCallback_isr; /* function to call when the video format changes */
	void * pvParm1; /* optional argument pointer  */
	int    iParm2 ; /* optional integer argument */
} BHDR_P_CallbackFunc;


/* Get the offset of two groups of register. */
#define BHDR_P_GET_REG_OFFSET(eHdrId, ulPriReg, ulSecReg) \
	((BHDR_P_eHdrCoreId0 == (eCoreId)) ? 0 : ((ulSecReg) - (ulPriReg)))


#if BHDR_CONFIG_SYMBOL_LOSS_SM

typedef enum
{
/* 0 */	BHDR_P_SYMBOL_LOSS_SM_eWaitAfterHotPlug,
/* 1 */	BHDR_P_SYMBOL_LOSS_SM_eLocked,
/* 2 */	BHDR_P_SYMBOL_LOSS_SM_eClearAuth,
/* 3 */	BHDR_P_SYMBOL_LOSS_SM_eResetPll,
/* 4 */	BHDR_P_SYMBOL_LOSS_SM_eFinal,
/* 5 */	BHDR_P_SYMBOL_LOSS_SM_eClrDCThreshold,
/* 6 */	BHDR_P_SYMBOL_LOSS_SM_eSetDCThreshold,
	BHDR_P_SYMBOL_LOSS_SM_eCount

} BHDR_P_SYMBOL_LOSS_SM ;

#endif


/*******************************************************************************
Private HDMIRx Handle Declaration
*******************************************************************************/
BDBG_OBJECT_ID_DECLARE(BHDR_P_Handle);

typedef struct BHDR_P_Handle
{
	BDBG_OBJECT(BHDR_P_Handle)
	BAVC_HDMI_CoreId eCoreId ;
	uint32_t             ulOffset ;

	BHDR_FE_ChannelHandle hFeChannel ;


	BCHP_Handle   hChip ;
	BREG_Handle   hRegister ;
	BINT_Handle   hInterrupt ;
	BINT_CallbackHandle hCallback[MAKE_INTR_ENUM(LAST)] ;

	BHDR_Mode eUsage ;

	BTMR_Handle hTimerDevice ;
	BTMR_TimerHandle timerGeneric ;

#if BHDR_CONFIG_RESET_FE_ON_SYMBOL_LOSS
	BTMR_TimerHandle timerHdmiFeReset ;
#endif


	BTMR_TimerHandle timerDviHdmiModeChange ;


#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
	uint32_t AvMuteTimeStamp ;
#endif

	BKNI_EventHandle BHDR_Event_DviToHdmi ;
	BKNI_EventHandle BHDR_Event_HdmiToDvi ;
	BKNI_EventHandle BHDR_Event_LoadHdcpKeys ;

	BKNI_EventHandle BHDR_Event_AudioChannelStatusUpdate ;
	BKNI_EventHandle BHDR_Event_VBlankEnd ;

	BAVC_HDMI_Packet RamPacket ;

	BHDR_Settings DeviceSettings ;

	BAVC_HDMI_AviInfoFrame AviInfoFrame ;
	BAVC_HDMI_AudioInfoFrame AudioInfoFrame ;
	BAVC_HDMI_SPDInfoFrame SPDInfoFrame;
	BAVC_HDMI_VendorSpecificInfoFrame VSInfoFrame ;
	BAVC_HDMI_DRMInfoFrame DRMInfoFrame ;
	BAVC_HDMI_ACP AudioContentProtection ;
	BAVC_HDMI_GcpData GeneralControlPacket ;
	BAVC_HDMI_AudioClockRegenerationPacket AudioClockRegenerationPacket ;

#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
	BAVC_HDMI_GamutPacket GamutPacket ;
	BHDR_CallbackFunc pfGamutChangeCallback ;
	void *pvGamutChangeParm1 ;
	int iGamutChangeParm2 ;
#endif

#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
	BAVC_HDMI_ISRC ISRC ;
#endif

	BAVC_Audio_Info AudioData ;

	uint8_t AvMute ;
	uint8_t bHdmiMode ;

	uint32_t AudioSampleRateHz ; /* In Hz */

	BACM_SPDIF_ChannelStatus stChannelStatus ;

	uint32_t SymbolLossIntrCount ;
	uint32_t SymbolLossIntrTimeStamp ;


	/* Handle Audio */
	uint8_t uiPreviousMaiSampleRate ;
	uint8_t uiPreviousMaiAudioFormat ;
	uint8_t uiPreviousMaiSampleWidth ;
	uint8_t uiConsecutiveSampleRateCalculations ;

	/******************/
	/* HDCP variables */
	/******************/

	uint16_t
		HDCP_TxRi,
		HDCP_RxRi ;

	uint8_t
		HDCP_TxPj,
		HDCP_RxPj ;

	uint32_t  FrameCount ;
	uint8_t
		FrameCountEncrypted,
		PreviousEncryptedFrame ;

	bool
		bHdcpRiUpdating  ;

	BHDR_HDCP_Settings stHdcpSettings ;
	BHDR_HDCP_Status stHdcpStatus ;

	BHDR_CallbackFunc pfHdcpStatusChangeCallback ;
	void *pvHdcpStatusChangeParm1  ;
	int iHdcpStatusChangeParm2  ;

#if BHDR_CONFIG_DEBUG_TIMER_S
	uint32_t DebugTimeStamp ;
	uint32_t ulVoCooef_C01_C00 ;
	uint8_t ulAvMute ;
	uint8_t ulUseRgbToRcbCr ;

#endif

	uint8_t HdcpKeyLoadOffset ;

	BHDR_CallbackFunc pfAvMuteNotifyCallback ;
	void *pvAvMuteNotifyParm1 ;
	int iAvMuteNotifyParm2 ;

	BHDR_CallbackFunc pfVideoFormatChangeCallback ;
	void *pvVideoFormatChangeParm1 ;
	int iVideoFormatChangeParm2 ;

	BHDR_CallbackFunc pfAudioFormatChangeCallback ;
	void *pvAudioFormatChangeParm1 ;
	int iAudioFormatChangeParm2 ;

	BHDR_CallbackFunc pfPacketChangeCallback ;
	void *pvPacketChangeParm1 ;
	int iPacketChangeParm2 ;

	BHDR_CallbackFunc pfPacketErrorCallback ;
	void *pvPacketErrorParm1 ;
	int iPacketErrorParm2 ;

	BHDR_CallbackFunc pfHdcpDisconnectNotifyCallback_isr ;
	void *pvHdcpDisconnectNotifyParm1 ;
	int iHdcpDisconnectNotifyParm2 ;


	bool bPacketErrors ;
	uint8_t ErrorFreePacketFrames ;

	bool bProcessPackets ;
	uint32_t VSyncCounter;
	uint8_t PllLock ;
	uint16_t HdmiAudioPackets ;

	uint32_t uiChannelStatus30 ;
	uint32_t uiChannelStatus74 ;

	bool bDeepColorMode ;
	uint32_t uiColorDepthFrameCounter ;
	BAVC_HDMI_VideoFormat stHdmiVideoFormat ;

#if BHDR_CONFIG_SYMBOL_LOSS_SM

	BHDR_P_SYMBOL_LOSS_SM eSymbolLossState ;
	BHDR_P_SYMBOL_LOSS_SM ePreviousSymbolLossState ;

	bool bClockStopped ;

	uint32_t AuthReqCnt ;
	bool bFirstPj ;
	uint8_t R0Equal0Count ;

	uint8_t RequestKeysIntrCount ;
	uint32_t RequestKeysTimeStamp ;

	uint32_t ClockStoppedTimer	;
#endif
	bool bPreviousStatus ;

	BHDR_Handle hHDRSlave ;

#if BHDR_CONFIG_DVP_HR_TMR
	bool bWaitForTimer ;
#endif

#if BHDR_CONFIG_UPDATE_MAI_ON_VSYNC
	uint8_t uiChannelStatusSampleFreq ;
#endif

} BHDR_P_Handle ;






/******************************************************************************
Summary:
Handle interrupts from the HDMIRx core.

Description:
Interrupts received from the HDMIRx core must be handled.  The following
is a list of possible interrupts.


Input:
	pParameter - pointer to interrupt specific information BHDR_Open.

Output:
	<None>

Returns:
	<None>

See Also:

*******************************************************************************/
void BHDR_P_HandleInterrupt_isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
) ;


void BHDR_P_HotPlugConnect_isr(BHDR_Handle hHDR) ;
void BHDR_P_HotPlugRemove_isr(BHDR_Handle hHDR) ;

BERR_Code BHDR_P_ConfigureAudioMaiBus_isr(BHDR_Handle hHDR);

#if BHDR_CONFIG_HDCP_REPEATER
BERR_Code  BHDR_HDCP_P_EnableRepeater(BHDR_Handle hHDR)  ;

BERR_Code BHDR_ReadRxI2cRegisterSpace(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length) ;

#endif

void BHDR_P_EnableInterrupts_isr(BHDR_Handle hHDR, bool enable) ;

void BHDR_P_ResetHdcp_isr(BHDR_Handle hHDR) ;
void BHDR_DEBUG_P_ResetAllEventCounters_isr(BHDR_Handle hHDR) ;

#ifdef __cplusplus
}
#endif

#endif
/* end bhdr_priv.h */
