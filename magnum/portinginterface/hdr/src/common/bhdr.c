/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhdr_config.h"
#include "bhdr_debug.h"

#include "bchp_dvp_hr.h"
#include "bchp_dvp_hr_key_ram.h"
#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bhdr.h"
#include "bhdr_priv.h"
#include "bhdr_hdcp_priv.h"
#include "bhdr_packet_priv.h"

#include "bhdr_phy_priv.h"


#include "bfmt.h"
#include "bkni.h"

#include "bacm_spdif.h"


BDBG_MODULE(BHDR) ;
BDBG_OBJECT_ID(BHDR_P_Handle);

#define	BHDR_CHECK_RC( rc, func )	          \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)



/******************************************************************************
Summary:
Array of BFMT_VideoFmts; indexes match the corresponding CEA-861 Video ID Codes
*******************************************************************************/
static const uint8_t VIC_VideoFmtTable[BAVC_HDMI_MAX_VIDEO_ID_CODES] =
{
	BFMT_VideoFmt_eMaxCount,      /* 00 - not used / unsupported */
	BFMT_VideoFmt_eDVI_640x480p,  /* 01 - Default VIC Code  */
	BFMT_VideoFmt_e480p,          /* 02 HD 480p 4:3 */
	BFMT_VideoFmt_e480p,          /* 03 HD 480p 16:9 */
	BFMT_VideoFmt_e720p,          /* 04 HD 720p */
	BFMT_VideoFmt_e1080i,         /* 05 HD 1080i 4:3 */

	BFMT_VideoFmt_eNTSC,          /* 06 480i, NSTC-M for North America */
	BFMT_VideoFmt_eNTSC,          /* 07 480i, NSTC-M for North America */

	BFMT_VideoFmt_eMaxCount,      /* 08 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 09 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 10 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 11 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 12 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 13 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 14 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 15 - not used / unsupported */
	BFMT_VideoFmt_e1080p,      /* 16 - 1080p  */

	BFMT_VideoFmt_e576p_50Hz,     /* 17 HD 576p 50Hz  4:3 */
	BFMT_VideoFmt_e576p_50Hz,     /* 18 HD 576p 50Hz 16:9 */

	BFMT_VideoFmt_e720p_50Hz,     /* 19 HD 720p 50Hz  */
	BFMT_VideoFmt_e1080i_50Hz,    /* 20 HD 1080i 50Hz */

	BFMT_VideoFmt_ePAL_B,         /* 21 any PAL format */
	BFMT_VideoFmt_ePAL_B,         /* 22 any PAL format */

	BFMT_VideoFmt_e576p_50Hz,     /* 23 HD 576p 50Hz  4:3 */
	BFMT_VideoFmt_e576p_50Hz,     /* 24 HD 576p 50Hz 16:9 */

	BFMT_VideoFmt_ePAL_B,         /* 25 any PAL format 4:3 */
	BFMT_VideoFmt_ePAL_B,         /* 26 any PAL format 16:9 */

	BFMT_VideoFmt_eMaxCount,      /* 27 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 28 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 29 - not used / unsupported */
	BFMT_VideoFmt_eMaxCount,      /* 30 - not used / unsupported */
	BFMT_VideoFmt_e1080p_50Hz,      /* 31 - 1080p 50 Hz */
	BFMT_VideoFmt_e1080p_24Hz,      /* 32 - 1080p 24 Hz  */
	BFMT_VideoFmt_e1080p_25Hz,      /* 33 - 1080p 25 Hz  */
	BFMT_VideoFmt_e1080p_30Hz,      /* 34 - 1080p 30 Hz  */
} ;


/* create lookup table for Channel Status, MAI Format values */
typedef struct CSBitsMaiFormatLookup
{
	uint32_t AudioSampleRateHz ;
	uint32_t MinFreq ;
	uint32_t MaxFreq ;
	uint8_t ChannelStatusBits ;
	uint8_t MaiSampleRate ;
} CSBitsMaiFormatLookup ;


#define BHDR_P_CSBits_48kHz 0x02
#define BHDR_P_CSBits_32kHz 0x03
#define BHDR_P_CSBits_44_1kHz 0x00
#define BHDR_P_CSBits_88_2kHz 0x08
#define BHDR_P_CSBits_96kHz 0x0A
#define BHDR_P_CSBits_176_4kHz 0x0C
#define BHDR_P_CSBits_192kHz 0x0E
#define BHDR_P_CSBits_768kHz 0x09

#define BHDR_P_MaiSampleRate_NotIndicated 0x00


#define BHDR_P_MaiSampleRate_48kHz     0x09
#define BHDR_P_MaiSampleRate_32kHz     0x07
#define BHDR_P_MaiSampleRate_44_1kHz 0x08
#define BHDR_P_MaiSampleRate_88_2kHz    0xB
#define BHDR_P_MaiSampleRate_96kHz     0xC
#define BHDR_P_MaiSampleRate_176_4kHz  0xE
#define BHDR_P_MaiSampleRate_192kHz      0xF

static const CSBitsMaiFormatLookup FsLookupTable[] =
{
	{ 48000, 47000,   49000,   BHDR_P_CSBits_48kHz,    BHDR_P_MaiSampleRate_48kHz},
	{ 32000, 31000,   33000,   BHDR_P_CSBits_32kHz,    BHDR_P_MaiSampleRate_32kHz},
	{ 44100, 43000,   45100,   BHDR_P_CSBits_44_1kHz,    BHDR_P_MaiSampleRate_44_1kHz},
	{ 96000, 95000,   97000,   BHDR_P_CSBits_96kHz,    BHDR_P_MaiSampleRate_96kHz},
	{ 88200, 87200,   89200,   BHDR_P_CSBits_88_2kHz,    BHDR_P_MaiSampleRate_88_2kHz},
	{176400, 175400,  177400,   BHDR_P_CSBits_176_4kHz,    BHDR_P_MaiSampleRate_176_4kHz},
	{192000, 191000,  193000,   BHDR_P_CSBits_192kHz,    BHDR_P_MaiSampleRate_192kHz},
	{768000, 797000,  769000,   BHDR_P_CSBits_768kHz,    BHDR_P_MaiSampleRate_192kHz},      /* HBR is sent as 4x 192kHz on MAI */
	{0, 0, 0,  0x00, 0x00}
} ;


#define BHDR_P_MaiAudioFormat_IDLE 0x00
#define BHDR_P_MaiAudioFormat_PCM_2CH 0xFE
#define BHDR_P_MaiAudioFormat_PCM_3CH 0x03
#define BHDR_P_MaiAudioFormat_PCM_6CH 0x86
#define BHDR_P_MaiAudioFormat_PCM_8CH 0x88

#define BHDR_P_MaiAudioFormat_COMPRESSED 0xFF
#define BHDR_P_MaiAudioFormat_HBR 0xC8



/*Audio sample width look up table*/

#define BHDR_AUDIO_SAMPLE_LENGTH_NUMBER 6

static const int audioSampleLengthTable[][BHDR_AUDIO_SAMPLE_LENGTH_NUMBER] =
{
	/*20 bits*/
	{16, 18, 0, 19, 20, 17} ,

	/*24 bits*/
	{20, 22, 0, 23, 24, 21}
} ;


#define BHDR_P_PACKET_WORDS 9

static void BHDR_P_ProcessModeChange_isr(BHDR_Handle hHDR) ;
static void BHDR_P_ProcessVerticalBlankEnd_isr(BHDR_Handle hHDR) ;

static BERR_Code BHDR_P_WriteRxI2cRegisterSpace_isr(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length) ;

static BERR_Code BHDR_P_GetChannelStatusBits_isr(
	BHDR_Handle hHDR, BACM_SPDIF_ChannelStatus *stChannelStatus) ;

static void BHDR_P_GetVideoFormat_isr(BHDR_Handle hHDR) ;
#if BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY
static void BHDR_P_EnableAudio_isr(BHDR_Handle hHDR, bool enable) ;
#endif

static void BHDR_P_ClearHdmiMode_isr(BHDR_Handle hHDR) ;
static void BHDR_P_ClearScdcStatus_isr(BHDR_Handle hHDR) ;


static BERR_Code BHDR_P_CreateTimer(
	BHDR_Handle hHDR, BTMR_TimerHandle * timerHandle, uint8_t timerId) ;

static BERR_Code BHDR_P_DestroyTimer(
	BHDR_Handle hHDR, BTMR_TimerHandle timerHandle, uint8_t timerId) ;

static void BHDR_P_TimerExpiration_isr (BHDR_Handle hHDR, int parm2) ;


/******************************************************************************
Summary:
Interrupt Callback Table to describe interrupt Names, isrs, and Masks
*******************************************************************************/
typedef struct BHDR_P_InterruptCbTable
{
	BINT_Id       	  IntrId;
	int               iParam2;
	bool              enable ; /* debug purposes */
} BHDR_P_InterruptCbTable ;

static const BHDR_P_InterruptCbTable BHDR_Interrupts0[MAKE_INTR_ENUM(LAST)] =
{
	/* 00 */   { BCHP_INT_ID_HDMI_RX_INTR2_0_SET_AV_MUTE, BHDR_INTR_eSET_AV_MUTE, true },


	/* 01 */   { BCHP_INT_ID_HDMI_RX_INTR2_0_AV_MUTE_UPDATE , BHDR_INTR_eAV_MUTE_UPDATE, true },


	/* 02 */   { BCHP_INT_ID_HDMI_RX_INTR2_0_DVI_TO_HDMI, BHDR_INTR_eDVI_TO_HDMI, true },

	/* 03 */   { BCHP_INT_ID_HDMI_RX_INTR2_0_HDMI_TO_DVI, BHDR_INTR_eHDMI_TO_DVI, true },

	/* 04 */  { BCHP_INT_ID_HDMI_RX_INTR2_0_AKSV_UPDATE, BHDR_INTR_eAKSV_UPDATE, true },

	/* 05 */   { BCHP_INT_ID_HDMI_RX_INTR2_0_REQUEST_KEYS, BHDR_INTR_eREQUEST_KEYS, true },

	/* 06 */  { BCHP_INT_ID_HDMI_RX_INTR2_0_REQUEST_KSVS, BHDR_INTR_eREQUEST_KSVS, BHDR_CONFIG_HDCP_REPEATER == 1},


	/* 07 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_I2C_TRANSACTION_COMPLETE,
		 BHDR_INTR_eI2C_TRANSACTION_COMPLETE, BHDR_CONFIG_DEBUG_I2C==1 },


	/* 08 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_PJ_UPDATE,
		 BHDR_INTR_ePJ_UPDATE, false },


	/* 09 */  { BCHP_INT_ID_HDMI_RX_INTR2_0_SYMBOL_LOSS, BHDR_INTR_eSYMBOL_LOSS, true },

	/* 10 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_INVALID_DATA_ISLAND_LENGTH,
		 BHDR_INTR_eINVALID_DATA_ISLAND_LENGTH, true },

	/* 11 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_CHANNEL_STATUS_UPDATE,
		 BHDR_INTR_eCHANNEL_STATUS_UPDATE, true },

	/* 12 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_VALIDITY_BIT_UPDATE,
		 BHDR_INTR_eAUDIO_VALIDITY_BIT_UPDATE, true },

	/* 13 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_RAM_PACKET_UPDATE,   BHDR_INTR_eRAM_PACKET_UPDATE, true },


	/* 14 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_RAM_PACKET_STOP,   BHDR_INTR_eRAM_PACKET_STOP, true },

	/* 15 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_PACKET_SYNC_ERROR,   BHDR_INTR_ePACKET_SYNC_ERROR, true },

	/* 16 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_LAYOUT_UPDATE,   BHDR_INTR_eLAYOUT_UPDATE, false },

	/* 17 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_TYPE_CHANGE,
		BHDR_INTR_eAUDIO_TYPE_CHANGE, false },

#if ((BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)) && (BCHP_VER >= BCHP_VER_B0)
	/* 18 */ {BCHP_INT_ID_HDMI_RX_INTR2_0_RGB_UNDER_RANGE, BHDR_INTR_eRGB_UNDER_RANGE, false},

	/* 20 */ {BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_FIFO_OVER_FLOW, BHDR_INTR_eAUDIO_FIFO_OVER_FLOW, false},

	/* 21 */ {BCHP_INT_ID_HDMI_RX_INTR2_0_AUDIO_FIFO_UNDER_FLOW, BHDR_INTR_eAUDIO_FIFO_UNDER_FLOW, false},
#endif

	/* 27 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_VSYNC_LEAD_EDGE,  	BHDR_INTR_eVSYNC_LEAD_EDGE, false },

	/* 28 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_VERTICAL_BLANK_END,  	BHDR_INTR_eVERTICAL_BLANK_END, true },


	/* 29 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_EXCESSIVE_PACKET_ERRORS, BHDR_INTR_eEXCESSIVE_PACKET_ERRORS, true },

	/* 30 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_FORMAT_DETECT_COUNT_SATURATED, BHDR_INTR_eFORMAT_DETECT_COUNT_SATURATED, true },


	/* 31 */ { BCHP_INT_ID_HDMI_RX_INTR2_0_ERROR_INTERRUPT, BHDR_INTR_eERROR_INTERRUPT, false }
} ;


#if BHDR_HAS_MULTIPLE_PORTS
static const BHDR_P_InterruptCbTable BHDR_Interrupts1[MAKE_INTR_ENUM(LAST)] =
{
	/* 00 */   { BCHP_INT_ID_HDMI_RX_INTR2_1_SET_AV_MUTE, BHDR_INTR_eSET_AV_MUTE, true },


	/* 01 */   { BCHP_INT_ID_HDMI_RX_INTR2_1_AV_MUTE_UPDATE , BHDR_INTR_eAV_MUTE_UPDATE, true },


	/* 02 */   { BCHP_INT_ID_HDMI_RX_INTR2_1_DVI_TO_HDMI, BHDR_INTR_eDVI_TO_HDMI, true },

	/* 03 */   { BCHP_INT_ID_HDMI_RX_INTR2_1_HDMI_TO_DVI, BHDR_INTR_eHDMI_TO_DVI, true },

	/* 04 */  { BCHP_INT_ID_HDMI_RX_INTR2_1_AKSV_UPDATE, BHDR_INTR_eAKSV_UPDATE, true },

	/* 05 */   { BCHP_INT_ID_HDMI_RX_INTR2_1_REQUEST_KEYS, BHDR_INTR_eREQUEST_KEYS, true },

	/* 06 */  { BCHP_INT_ID_HDMI_RX_INTR2_1_REQUEST_KSVS, BHDR_INTR_eREQUEST_KSVS, BHDR_CONFIG_HDCP_REPEATER == 1},


	/* 07 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_I2C_TRANSACTION_COMPLETE,
		 BHDR_INTR_eI2C_TRANSACTION_COMPLETE, (bool) BHDR_CONFIG_DEBUG_I2C },


	/* 08 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_PJ_UPDATE,
		 BHDR_INTR_ePJ_UPDATE, false },


	/* 09 */  { BCHP_INT_ID_HDMI_RX_INTR2_1_SYMBOL_LOSS, BHDR_INTR_eSYMBOL_LOSS, true },

	/* 10 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_INVALID_DATA_ISLAND_LENGTH,
		 BHDR_INTR_eINVALID_DATA_ISLAND_LENGTH, true },

	/* 11 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_CHANNEL_STATUS_UPDATE,
		 BHDR_INTR_eCHANNEL_STATUS_UPDATE, true},

	/* 12 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_AUDIO_VALIDITY_BIT_UPDATE,
		 BHDR_INTR_eAUDIO_VALIDITY_BIT_UPDATE, true },

	/* 13 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_RAM_PACKET_UPDATE,   BHDR_INTR_eRAM_PACKET_UPDATE, true },


	/* 14 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_RAM_PACKET_STOP,   BHDR_INTR_eRAM_PACKET_STOP, true },

	/* 15 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_PACKET_SYNC_ERROR,   BHDR_INTR_ePACKET_SYNC_ERROR, true },

	/* 16 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_LAYOUT_UPDATE,   BHDR_INTR_eLAYOUT_UPDATE, false },

	/* 17 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_AUDIO_TYPE_CHANGE,
		BHDR_INTR_eAUDIO_TYPE_CHANGE, false },

#if ((BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)) && (BCHP_VER >= BCHP_VER_B0)
	/* 18 */ {BCHP_INT_ID_HDMI_RX_INTR2_1_RGB_UNDER_RANGE, BHDR_INTR_eRGB_UNDER_RANGE, false},

	/* 20 */ {BCHP_INT_ID_HDMI_RX_INTR2_1_AUDIO_FIFO_OVER_FLOW, BHDR_INTR_eAUDIO_FIFO_OVER_FLOW, false},

	/* 21 */ {BCHP_INT_ID_HDMI_RX_INTR2_1_AUDIO_FIFO_UNDER_FLOW, BHDR_INTR_eAUDIO_FIFO_UNDER_FLOW, false},
#endif

	/* 27 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_VSYNC_LEAD_EDGE,  	BHDR_INTR_eVSYNC_LEAD_EDGE, false },

	/* 28 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_VERTICAL_BLANK_END,  	BHDR_INTR_eVERTICAL_BLANK_END, true },


	/* 29 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_EXCESSIVE_PACKET_ERRORS, BHDR_INTR_eEXCESSIVE_PACKET_ERRORS, true },

	/* 30 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_FORMAT_DETECT_COUNT_SATURATED, BHDR_INTR_eFORMAT_DETECT_COUNT_SATURATED, true },


	/* 31 */ { BCHP_INT_ID_HDMI_RX_INTR2_1_ERROR_INTERRUPT, BHDR_INTR_eERROR_INTERRUPT, false }
} ;
#endif


/*******************************************************************************
BERR_Code BHDR_GetDefaultSettings
Summary: Get the default settings for the HDMI device.
*******************************************************************************/
BERR_Code BHDR_GetDefaultSettings(
   BHDR_Settings *pDefault  /* [in] pointer to memory to hold default settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDR_GetDefaultSettings) ;

	BDBG_ASSERT(pDefault) ;

	BKNI_Memset(pDefault, 0, sizeof(BHDR_Settings)) ;

	pDefault->eCoreId = 0 ;
	pDefault->bParseAVI = true ;  /* HDR PI should process AVI Packets; disable if app is to do processing */
	pDefault->hTmr = (BTMR_Handle) NULL ;

	/* bDisableI2cPadSclPullup = disable I2C Pad SCL Pull Up */
	pDefault->bDisableI2cPadSclPullup = true ;

	/* bDisableI2cPadSdaPullup = disable I2C Pad SDA Pull Up */
	pDefault->bDisableI2cPadSdaPullup =	true ;

	pDefault->hHDRMaster = NULL ;  /* Master HDR handle for 3D, if exists... */

#if BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT
	pDefault->bHdmiHardwarePassthrough = false ;
#endif

	BDBG_LEAVE(BHDR_GetDefaultSettings) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDR_Open
Summary: Open/Initialize the HDMI device
*******************************************************************************/
BERR_Code BHDR_Open(
   BHDR_Handle *phHDR,       /* [out] HDMI handle */
   BCHP_Handle hChip,         /* [in] Chip handle */
   BREG_Handle hRegister,     /* [in] Register Interface to HDMI Tx Core */
   BINT_Handle hInterrupt,    /* [in] Interrupt handle */
   const BHDR_Settings  *pSettings /* [in] default HDMI settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BHDR_Handle hHDR = NULL ;
	BHDR_P_HdrCoreId eCoreId ;

	uint32_t ulOffset, Register ;
	uint8_t i ;

	const BHDR_P_InterruptCbTable *pInterrupts ;
	BTMR_Settings timerSettings  ;
	BHDR_DEBUG_P_EventCounter stEventCounter ;

	BDBG_ENTER(BHDR_Open) ;

	/* verify parameters */
	BDBG_ASSERT(hChip) ;
	BDBG_ASSERT(hRegister) ;
	BDBG_ASSERT(hInterrupt) ;

	/* create the HDMI Handle */
	hHDR = (BHDR_Handle) BKNI_Malloc(sizeof(BHDR_P_Handle)) ;
	if (!hHDR)
	{
		BDBG_ERR(("Unable to allocate memory for HDMI Handle")) ;
		rc = BERR_OUT_OF_SYSTEM_MEMORY ;
		goto done ;
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset(hHDR, 0, sizeof(BHDR_P_Handle)) ;
	BDBG_OBJECT_SET(hHDR, BHDR_P_Handle) ;

	/* assign the handles passed in as parameters */
	hHDR->hChip      = hChip ;
	hHDR->hRegister  = hRegister ;
	hHDR->hInterrupt = hInterrupt ;

	if ((!pSettings) || (!pSettings->hTmr))
	{
		BDBG_ERR(("BTMR Device must be available")) ;
		rc = BERR_NOT_INITIALIZED ;
		goto done ;
	}

 	hHDR->hTimerDevice = pSettings->hTmr ;

	hHDR->DeviceSettings = *pSettings ;

	eCoreId = pSettings->eCoreId ;
	BDBG_ASSERT(eCoreId < BHDR_P_eHdrCoreIdMax) ;
	hHDR->eCoreId = eCoreId ;


	hHDR->AvMute = 0x0 ;

	hHDR->bPacketErrors = false ;
	hHDR->ErrorFreePacketFrames = BHDR_P_MAX_ERROR_FREE_PACKET_FRAMES ;

	hHDR->AudioClockRegenerationPacket.N = 1 ;
	hHDR->AudioClockRegenerationPacket.CTS = 1 ;

	hHDR->uiColorDepthFrameCounter = 0 ;
	hHDR->bDeepColorMode = false ;

	hHDR->bPreviousStatus = true ;

	BTMR_GetDefaultTimerSettings(&timerSettings) ;
    	timerSettings.type =  BTMR_Type_eSharedFreeRun ;
	rc = BTMR_CreateTimer(hHDR->hTimerDevice, &hHDR->timerGeneric,  &timerSettings) ;
	if(rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
		goto done ;
	}

	BHDR_P_CreateTimer(hHDR,
		&hHDR->timerDviHdmiModeChange, BHDR_P_TIMER_eDviHdmiModeChange) ;

#if BHDR_CONFIG_DEBUG_TIMER_S
	/* initialize debug timestamp if used */
	BTMR_ReadTimer(hHDR->timerGeneric, &hHDR->DebugTimeStamp) ;
#endif

	/* settings for recalculation of SR up to BHDR_CONFIG_CONSECUTIVE_SR_CALCS times */
	hHDR->uiConsecutiveSampleRateCalculations = 0 ;
	hHDR->AudioSampleRateHz = 0 ;



 	/* Register offset from HDR */
	hHDR->ulOffset = ulOffset = 0
#if BHDR_HAS_MULTIPLE_PORTS
	+ BHDR_P_GET_REG_OFFSET(hHDR->eHdrCoreId,
		BCHP_HDMI_RX_0_REG_START, BCHP_HDMI_RX_1_REG_START)
#endif
	;

#if BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY
	/* Initialize HDMI Rx Audio to OFF; */
	/* Avoids noise,pops, etc until valid SR is received */
	BKNI_EnterCriticalSection() ;
		BHDR_P_EnableAudio_isr(hHDR, false) ;
	BKNI_LeaveCriticalSection() ;
#endif

	/* settings for recalculation of SR up to BHDR_CONFIG_CONSECUTIVE_SR_CALCS times */
	hHDR->uiConsecutiveSampleRateCalculations = 0 ;
	hHDR->uiPreviousMaiSampleRate = 0 ;
	hHDR->uiPreviousMaiAudioFormat = BHDR_P_MaiAudioFormat_IDLE ;
	hHDR->uiPreviousMaiSampleWidth = 16 ;


	hHDR->VSyncCounter = 0 ;

#if BHDR_CONFIG_DEBUG_DISPLAY_HDMI_RX_CONFIG
	/* display version information */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_CORE_REV + ulOffset);
	BDBG_LOG(("*****************************************")) ;
	BDBG_LOG(("HDMI Receiver Core %d", eCoreId)) ;
	BDBG_LOG(("   Register Offset %#x; Version 0x%08X", ulOffset, Register)) ;
	BDBG_LOG(("Compiled: %s %s",  __DATE__, __TIME__)) ;
	BDBG_LOG(("*****************************************")) ;
#endif

	/* Create Events for use with Interrupts */
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(&(hHDR->BHDR_Event_HdmiToDvi))) ;
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(&(hHDR->BHDR_Event_DviToHdmi))) ;
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(&(hHDR->BHDR_Event_AudioChannelStatusUpdate))) ;
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(&(hHDR->BHDR_Event_VBlankEnd))) ;

	if (hHDR->eCoreId == BHDR_P_eHdrCoreId0)
	{
		pInterrupts = BHDR_Interrupts0 ;
		hHDR->HdcpKeyLoadOffset = 0x0 ;
	}
#if BHDR_HAS_MULTIPLE_PORTS
	else if (hHDR->eCoreId == BHDR_P_eHdrCoreId1)
	{
		pInterrupts = BHDR_Interrupts1 ;
		hHDR->HdcpKeyLoadOffset = 0x28 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDR->eCoreId)) ;
		BDBG_ASSERT(false) ;
		return BERR_INVALID_PARAMETER ;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset) ;
		Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, KEY_LOAD_BASE_ADDRESS) ;
		/* initialize INIT_HDCP_ON_AUTH_REQ to zero */
		Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, INIT_HDCP_ON_AUTH_REQ) ;

		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_CONFIG, KEY_LOAD_BASE_ADDRESS,
			hHDR->HdcpKeyLoadOffset) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset,  Register) ;

#if BHDR_CONFIG_HDCP_REPEATER
	BHDR_HDCP_P_EnableRepeater(hHDR) ;
#endif

#if BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT
	BHDR_P_SetHdmiPassthroughMode(hHDR, pSettings->bHdmiHardwarePassthrough) ;
#endif


	hHDR->FrameCount = 0 ;
	hHDR->PreviousEncryptedFrame = 0xFF ;

	/* ... configure EVENT COUNTERS as needed */

	BKNI_Memset(&stEventCounter, 0, sizeof(BHDR_DEBUG_P_EventCounter)) ;

	stEventCounter.bBchEvent = false ;
	stEventCounter.uiCounter = 0 ;
	stEventCounter.ulEventBitMask31_00 = 0 ;
	stEventCounter.ulEventBitMask63_32 = (uint32_t ) (1 << (55 - 32)) ;  /* 55: enc_en_p COUNT_0 */
	BHDR_DEBUG_P_ConfigureEventCounter(hHDR, &stEventCounter) ;

	stEventCounter.uiCounter = 1 ;
	stEventCounter.ulEventBitMask31_00 = 0 ;
	stEventCounter.ulEventBitMask63_32 = (uint32_t ) (1 << (56 - 32)) ;  /* 56: enc_dis_p COUNT_1 */
	BHDR_DEBUG_P_ConfigureEventCounter(hHDR, &stEventCounter) ;

	stEventCounter.uiCounter = 2 ;
	stEventCounter.bBchEvent = false ;
	stEventCounter.ulEventBitMask31_00 = 0 ;
	stEventCounter.ulEventBitMask63_32 =
		  (uint32_t ) (1 << (33 - 32))   /* 33: pr_audio_sample_sp0_p */
		| (uint32_t ) (1 << (34 - 32))   /* 34: pr_audio_sample_sp1_p */
		| (uint32_t ) (1 << (35 - 32))   /*  35: pr_audio_sample_sp2_p */
		| (uint32_t ) (1 << (36 - 32))   /*  36: pr_audio_sample_sp3_p */
		| (uint32_t ) (1 << (37 - 32))   /*  37: pr_audio_sample_p */
		| (uint32_t ) (1 << (38 - 32)) ; /*  38: audio_packet_p */         /* Audio Packets COUNT_2 */
	BHDR_DEBUG_P_ConfigureEventCounter(hHDR, &stEventCounter) ;

	BHDR_DEBUG_P_ResetAllEventCounters_isr(hHDR) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, I2C_ENABLE) ;
		Register |=   BCHP_FIELD_DATA(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, I2C_ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset,  Register) ;

	BHDR_InitializePacketRAM(hHDR) ;

	/* Register/enable interrupt callbacks */
	for (i = 0; i < MAKE_INTR_ENUM(LAST) ; i++)
	{
		BHDR_CHECK_RC( rc, BINT_CreateCallback(&(hHDR->hCallback[i]),
			hHDR->hInterrupt, pInterrupts[i].IntrId,
			BHDR_P_HandleInterrupt_isr, (void *) hHDR, i ));

		/* clear interrupt callback */
		BHDR_CHECK_RC(rc, BINT_ClearCallback( hHDR->hCallback[i])) ;

		/* skip interrupt if not enabled in table...  */
		if (!pInterrupts[i].enable)
			continue ;

#if 0
		/* enable interrupts only when input is active */
		BHDR_CHECK_RC( rc, BINT_EnableCallback( hHDR->hCallback[i] ) );
#endif
	}

	/* clear locked packets  */
	for (i = 0 ; i < BHDR_P_NUM_PACKETS; i++)
	{
		BHDR_CHECK_RC(rc, BHDR_P_ClearPacketRAMLock_isr(hHDR, i)) ;
	}

	/* always start configuration in DVI mode */
	BKNI_EnterCriticalSection() ;
		BHDR_P_ClearHdmiMode_isr(hHDR) ;
		BHDR_P_ClearScdcStatus_isr(hHDR) ;
	BKNI_LeaveCriticalSection() ;



	/* set the threshold for packet errors */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_BCH_ECC_CFG, PACKET_ERR_THRESHOLD) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_BCH_ECC_CFG, PACKET_ERR_THRESHOLD,
		BHDR_P_PACKET_ERROR_THRESHOLD) ;
	BREG_Write32(hRegister,  BCHP_HDMI_RX_0_BCH_ECC_CFG + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_CHANNEL_STATUS_CONFIG + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_STATUS_CONFIG, SAVE_CH_STATUS_EVEN_IF_MISMATCH) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_AUDIO_CHANNEL_STATUS_CONFIG + ulOffset, Register) ;


	/* default to h/w handling Deep Color */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset) ;
		/* clear deep color threshold */
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, ENABLE_FORCE_COLOR_DEPTH) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, BLOCK_DEFAULT_MODE) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, AUTO_CLEAR_DEEPCOLOR) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, AUTO_CLEAR_DEEPCOLOR, 1) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, FORCE_DEFAULT_MODE) ;

		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, BLOCK_DEFAULT_MODE, 1) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD, 15) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_2 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_2, NO_GCP_COUNT_THRESHOLD) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDMI_13_FEATURES_CFG_2, NO_GCP_COUNT_THRESHOLD, 15) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_2 + ulOffset, Register) ;


	/* set the correct default for HDMI DETECT MODE */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CFG_1 + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_DIGITAL_FRONT_END_CFG_1, HDMI_DETECT_MODE) ;
	BREG_Write32(hRegister,  BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CFG_1 + ulOffset, Register) ;

	/* default internal HdmiMode to unknown */
	hHDR->bHdmiMode = 0xFF;

	/* enable the AUTO_CLEAR_AVMUTE  */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_MUTE_CFG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_AUDIO_MUTE_CFG, AUTO_CLEAR_AVMUTE) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_AUDIO_MUTE_CFG, ALWAYS_ALLOW_AVMUTE_UPDATE) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_MUTE_CFG, AUTO_CLEAR_AVMUTE, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_MUTE_CFG, ALWAYS_ALLOW_AVMUTE_UPDATE, 1) ;
	BREG_Write32(hRegister,  BCHP_HDMI_RX_0_AUDIO_MUTE_CFG + ulOffset, Register) ;


	/* format detection input selection should be updated to choose unwrap_packed_video block  */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_CONTROL + ulOffset) ;
		Register &= ~BCHP_MASK(HDMI_RX_0_MISC_CONTROL, FORMAT_DETECT_INPUT_SELECT) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_MISC_CONTROL, FORMAT_DETECT_INPUT_SELECT, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_MISC_CONTROL + ulOffset, Register) ;


	/* clear the format detection registers */
	Register = 0xFFFFFFFF;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

	Register = 0;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

#if BHDR_CONFIG_DISABLE_EDID_RAM
	/* Disable EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_EDID_CONTROL + ulOffset, Register) ;
#endif

	/* keep created pointer */
	*phHDR = hHDR ;



done:
	if( (rc != BERR_SUCCESS) && (hHDR))
	{
		BKNI_Free(hHDR) ;
		*phHDR = NULL  ;
	}

	BDBG_LEAVE(BHDR_Open) ;
	return rc ;
} /* end BHDR_Open */


/******************************************************************************
BERR_Code BHDR_Close
Summary: Close the HDMI connection to the HDMI Rx.
*******************************************************************************/
BERR_Code BHDR_Close(
   BHDR_Handle hHDR
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	BDBG_ENTER(BHDR_Close) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	/* Disable and Destroy the HDMI Callbacks */
	for( i = 0; i < MAKE_INTR_ENUM(LAST); i++ )
	{
		/* all interrupts are now created; disable and destroy all on close */

		if (BINT_DisableCallback( hHDR->hCallback[i]) != BERR_SUCCESS)
		{
			BDBG_ERR(("Error Disabling Callback %d", i)) ;
			rc = BERR_UNKNOWN ;
		}

		if  (BINT_DestroyCallback( hHDR->hCallback[i]) != BERR_SUCCESS)
		{
			BDBG_ERR(("Error Destroying Callback %d", i)) ;
			rc = BERR_UNKNOWN ;
		}
	}

	BTMR_DestroyTimer(hHDR->timerGeneric) ;

	BHDR_P_DestroyTimer(hHDR,
		hHDR->timerDviHdmiModeChange, BHDR_P_TIMER_eDviHdmiModeChange) ;

	BKNI_DestroyEvent(hHDR->BHDR_Event_HdmiToDvi) ;
	BKNI_DestroyEvent(hHDR->BHDR_Event_DviToHdmi) ;
	BKNI_DestroyEvent(hHDR->BHDR_Event_AudioChannelStatusUpdate) ;
	BKNI_DestroyEvent(hHDR->BHDR_Event_VBlankEnd) ;

 	/* free memory associated with the HDMI handle */
	BKNI_Memset(hHDR, 0, sizeof(BHDR_P_Handle)) ;
	BDBG_OBJECT_DESTROY(hHDR, BHDR_P_Handle);
	BKNI_Free( (void *) hHDR) ;

	BDBG_LEAVE(BHDR_Close) ;
	return rc ;
}

/*******************************************************************************
BERR_Code BHDR_GetSettings
Summary: Get the current HDMI Rx settings
*******************************************************************************/
BERR_Code BHDR_GetSettings(
	BHDR_Handle hHDR,            /* [in] HDMI Rx Handle */
	BHDR_Settings *pHdrSettings  /* [in] pointer to memory to hold current settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDR_GetSettings) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memcpy(pHdrSettings, &hHDR->DeviceSettings, sizeof(BHDR_Settings)) ;


	BDBG_LEAVE(BHDR_GetSettings) ;
	return rc ;
}



/*******************************************************************************
BERR_Code BHDR_SetSettings
Summary: Get the current HDMI Rx settings
*******************************************************************************/
BERR_Code BHDR_SetSettings(
	BHDR_Handle hHDR,            /* [in] HDMI Rx Handle */
	BHDR_Settings *pHdrSettings  /* [in] pointer to memory to hold current settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDR_SetSettings) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memcpy(&hHDR->DeviceSettings, pHdrSettings, sizeof(BHDR_Settings)) ;

#if BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT
	BHDR_P_SetHdmiPassthroughMode(hHDR, pHdrSettings->bHdmiHardwarePassthrough) ;
#endif

	BDBG_LEAVE(BHDR_SetSettings) ;
	return rc ;
}



/***************************************************************************
BERR_Code BHDR_GetEventHandle
Summary: Get the event handle for checking HDMI events.
****************************************************************************/
BERR_Code BHDR_GetEventHandle(
   BHDR_Handle hHDR,
   BHDR_EventType eEventType,
   BKNI_EventHandle *pBHDREvent	/* [out] event handle */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_GetEventHandle) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);


	switch (eEventType)
	{
	case BHDR_Event_HdmiToDvi :
		*pBHDREvent = hHDR->BHDR_Event_HdmiToDvi ;
		break ;

	case BHDR_Event_DviToHdmi :
		*pBHDREvent = hHDR->BHDR_Event_DviToHdmi ;
		break ;

	case BHDR_Event_AudioChannelStatusUpdate :
		*pBHDREvent = hHDR->BHDR_Event_AudioChannelStatusUpdate ;
		break ;

	case BHDR_Event_VBlankEnd :
		*pBHDREvent = hHDR->BHDR_Event_VBlankEnd ;
		break ;

	default :
		BDBG_ERR(("Invalid Event Type: %d", eEventType)) ;
		rc = BERR_INVALID_PARAMETER ;
		goto done ;
	}


done:
	BDBG_LEAVE(BHDR_GetEventHandle) ;
	return rc ;
}



BERR_Code BHDR_WriteRxI2cRegisterSpace(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length)
{

	BERR_Code rc ;

	BDBG_ENTER(BHDR_WriteRxI2cRegisterSpace) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BKNI_EnterCriticalSection() ;
		rc = BHDR_P_WriteRxI2cRegisterSpace_isr(hHDR, offset, pData, Length) ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_WriteRxI2cRegisterSpace) ;

	return  rc ;
}


/***************************************************************************
Summary:
	Write Data to Rx I2c Space
***************************************************************************/
static  BERR_Code BHDR_P_WriteRxI2cRegisterSpace_isr(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length)
{
	BREG_Handle hRegister ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	uint8_t i, I2cBusy ;
	uint8_t msTimeout ;
	uint8_t addr = offset ;

	BDBG_ENTER(BHDR_P_WriteRxI2cRegisterSpace_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	msTimeout = 100 ;
	do
	{
		Register = BREG_Read32( hHDR->hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset) ;
		I2cBusy = (uint8_t) BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;

		if (!I2cBusy)
			break ;

		BKNI_Delay(100) ;

	} while (I2cBusy && msTimeout--) ;

	if (I2cBusy)
	{
		rc = BERR_TIMEOUT ;
		goto done ;
	}


	/* Cleared for loading ; set the busy bit */
	Register |= BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

	for (i = 0 ; i < Length; i++)
	{
#if BHDR_CONFIG_DEBUG_I2C
		BDBG_LOG(("I2C Offset %#x = 0x%02x", addr + i, *(pData+i))) ;
#endif
		Register &= ~(
			  BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA)
			| BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_OFFSET)) ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA, *(pData+i))
			| BCHP_FIELD_DATA(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_OFFSET, addr + i) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

		/* write the byte */
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA_P, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

		Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA_P) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;
	}

	/* Done loading; clear the busy bit */
	Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

done:
	BDBG_LEAVE(BHDR_P_WriteRxI2cRegisterSpace_isr) ;
	return rc ;
}



/***************************************************************************
Summary:
	Read Data from Rx I2c Space
***************************************************************************/
BERR_Code BHDR_P_ReadRxI2cRegisterSpace_isr(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length)
{

	BREG_Handle hRegister ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	uint8_t i, I2cBusy ;
	uint8_t msTimeout ;
	uint8_t addr = offset ;

	BDBG_ENTER(BHDR_P_ReadRxI2cRegisterSpace_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	msTimeout = 100;
	do
	{
		Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset) ;
		I2cBusy = (uint8_t) BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;

		if (!I2cBusy)
			break ;

		BKNI_Delay(100) ;

	} while (I2cBusy && msTimeout--) ;

	if (I2cBusy)
	{
		rc = BERR_TIMEOUT ;
		goto done ;
	}


	/* Cleared for reading ; set the busy bit */
	Register |= BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;


	for (i = 0 ; i < Length; i++)
	{
		Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_OFFSET) ;
		Register |=  BCHP_FIELD_DATA(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_OFFSET, addr + i) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

		/* read the byte */
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA_P, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

		Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_WRITE_DATA_P) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset) ;
		*(pData+i) = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_READ_DATA) ;

		BDBG_MSG(("read i2c offset %#x = %#x", addr, *(pData+i))) ;
	}

	/* Done loading; clear the busy bit */
	Register &= ~BCHP_MASK(HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_PEEK_POKE_BUSY) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset, Register) ;

done:
	BDBG_LEAVE(BHDR_P_ReadRxI2cRegisterSpace_isr) ;
	return rc ;
}


/***************************************************************************
Summary:
	Read Data from Rx I2c Space
***************************************************************************/
BERR_Code BHDR_ReadRxI2cRegisterSpace(BHDR_Handle hHDR,
	uint8_t offset, uint8_t *pData, uint8_t Length)
{
	BERR_Code rc ;

	BDBG_ENTER(BHDR_ReadRxI2cRegisterSpace) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BKNI_EnterCriticalSection() ;
		rc = BHDR_P_ReadRxI2cRegisterSpace_isr(hHDR, offset, pData, Length) ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_ReadRxI2cRegisterSpace) ;

	return  rc ;
}


void BHDR_P_ClearCoreAuthentication_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	BDBG_ENTER(BHDR_P_ClearCoreAuthentication_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* clear HDCP authenticated status */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset) ;

	Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset, Register) ;

	Register |= BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG  + ulOffset, Register)  ;

	BDBG_LEAVE(BHDR_P_ClearCoreAuthentication_isr) ;

}

/******************************************************************************
void BHDR_P_ProcessModeChange_isr
Summary: Proces changes between HDMI and DVI modes
*******************************************************************************/
void BHDR_P_ProcessModeChange_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;
	uint8_t HdmiMode ;
	uint16_t BStatus ;

	BDBG_ENTER(BHDR_P_ProcessModeChange_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CFG_1 + ulOffset) ;
	HdmiMode = BCHP_GET_FIELD_DATA(
		Register, HDMI_RX_0_DIGITAL_FRONT_END_CFG_1, HDMI_MODE) ;

	if (HdmiMode == hHDR->bHdmiMode)
		return ;

#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
	BDBG_WRN(("CH%d Hdmi Mode Changed from %d to %d",
		hHDR->eCoreId, hHDR->bHdmiMode, HdmiMode)) ;
#endif

	/* process changes only */
	hHDR->bHdmiMode = HdmiMode ;

	if (hHDR->bHdmiMode == 0) /* DVI MODE */
	{
		BDBG_LOG(("CH%d ***DVI MODE***", hHDR->eCoreId)) ;

		/* mask packet stop interrupts... no need to be aware of packet stops  in DVI mode */
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset, 0xFFFFFFF) ;

		/* clear any enabled packet stop detection  */
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset, 0)  ;

		/* clear any HDCP authentication */
		BHDR_P_ClearCoreAuthentication_isr(hHDR) ;

		BHDR_P_ReadRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;
	 		BStatus = BStatus & 0xEFFF ;
#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
		BDBG_WRN(("HDCP BStatus Updated to Indicate DVI Mode:  %X ",  BStatus)) ;
#endif
		BHDR_P_WriteRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;

		/* clear the AvMute Status, packet stop etc.  */
		BHDR_P_ClearHdmiMode_isr(hHDR) ;

 		BKNI_SetEvent(hHDR->BHDR_Event_HdmiToDvi) ;

		BHDR_P_ClearPacketMemory_isr(hHDR) ;

		/* clear all RAM Packets when back to HDMI mode; Packet Update interrupts will fire */
		{
			uint8_t PacketNum ;

			for (PacketNum = 0 ; PacketNum < BHDR_P_NUM_PACKETS; PacketNum++)
			{
				BHDR_P_ClearPacketRAM_isr(hHDR, PacketNum) ;
			}
		}


	}
	else /* HDMI MODE */
	{
		BDBG_LOG(("CH%d ***HDMI MODE***", hHDR->eCoreId)) ;
		BHDR_P_ResetHdcp_isr(hHDR) ;

		/* make sure MAI bus is configured to match current channel status information */
		BHDR_P_ConfigureAudioMaiBus_isr(hHDR) ;

		BKNI_SetEvent(hHDR->BHDR_Event_DviToHdmi) ;

		BHDR_P_ReadRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;
	 		BStatus = BStatus | (uint16_t) BAVC_HDMI_HDCP_RxStatus_eHdmiMode ;
#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
		BDBG_WRN(("HDCP BStatus Updated to Indicate HDMI Mode:  %X ",  BStatus)) ;
#endif
		BHDR_P_WriteRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;
	}
	BDBG_LEAVE(BHDR_P_ProcessModeChange_isr) ;
}



#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
void BHDR_P_ClearAvMuteAfterNSeconds(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;
	uint32_t timestamp ;

	BDBG_ENTER(BHDR_P_ClearAvMuteAfterNSeconds) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	BTMR_ReadTimer_isr(hHDR->timerGeneric, &timestamp) ;
	if ((timestamp - hHDR->AvMuteTimeStamp) > (BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S * BHDR_P_SECOND))
	{
		BDBG_WRN(("CH%d AvMute timer has expired;  force Un-Mute... %d",
			hHDR->eCoreId, hHDR->AvMute)) ;

		Register = BREG_Read32_isr(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset) ;
		Register |= BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, CLEAR_AV_MUTE) ;
		BREG_Write32_isr(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset, Register) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_CONFIG, CLEAR_AV_MUTE) ;
		BREG_Write32_isr(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset, Register) ;

		hHDR->AvMute = false ;

		/* call the callback functions for AvMute (on/off) notification */
		if (hHDR->pfAvMuteNotifyCallback)
		{
			hHDR->pfAvMuteNotifyCallback(
				hHDR->pvAvMuteNotifyParm1, hHDR->iAvMuteNotifyParm2,  &hHDR->AvMute) ;
		}
	}
	BDBG_LEAVE(BHDR_P_ClearAvMuteAfterNSeconds) ;
}
#endif


/******************************************************************************
void BHDR_P_ProcessVerticalBlankEnd_isr
Summary: Proces Vertical Blank End events
*******************************************************************************/
static void BHDR_P_ProcessVerticalBlankEnd_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	uint8_t HdmiMode ;
	bool bRiChange = false ;
	uint16_t ucHdcpRi ;
	uint8_t EncryptedFrame, ClearFrame ;

	BDBG_ENTER(BHDR_P_ProcessVerticalBlankEnd_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* reset all counters at start of vertical blank end */
	BHDR_DEBUG_P_ResetAllEventCounters_isr(hHDR) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CFG_1 + ulOffset) ;
	HdmiMode = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_DIGITAL_FRONT_END_CFG_1, HDMI_MODE) ;

	if (HdmiMode != hHDR->bHdmiMode)
	{
#if BHDR_CONFIG_DELAY_MODE_CHANGE_MS
		/* set all mode changes in the ProcessModeChange function*/
#if BHDR_CONFIG_DEBUG_MODE_CHANGE
		BDBG_WRN(("Start Delay Mode Change Timer for %d ms... %d",
			BHDR_CONFIG_DELAY_MODE_CHANGE_MS, 	__LINE__)) ;
#endif
		BTMR_StartTimer_isr(hHDR->timerDviHdmiModeChange,
			BHDR_P_MILLISECOND * BHDR_CONFIG_DELAY_MODE_CHANGE_MS) ;
#else
		BHDR_P_ProcessModeChange_isr(hHDR) ;
#endif
	}

	hHDR->FrameCount++ ;  /* always increment the frame count */

	/* get status of frame: clear or encrypted */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_EVENT_COUNTER_0_1_COUNT + ulOffset) ;
	EncryptedFrame = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_EVENT_COUNTER_0_1_COUNT, COUNT_0) ;
	ClearFrame     = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_EVENT_COUNTER_0_1_COUNT, COUNT_1) ;

	if (ClearFrame)
	{
		/* reset Encrypted Frame Counter  whenever we have a clear frame */
		hHDR->FrameCountEncrypted = 0 ;
		hHDR->bHdcpRiUpdating = false ; /* HDCP Ri is not updating */
	}
	else if (EncryptedFrame)
	{
		hHDR->FrameCountEncrypted++ ;
		hHDR->bHdcpRiUpdating = true ;	 /* HDCP Ri values are updating */
	}
	else /* Cannot detect enc_en/enc_dis; assume clear */
	{
		hHDR->bHdcpRiUpdating = false ;
	}

	 /*
	 -- HDCP Ri value *may not update* if the transmitter chooses not to send encrypted frames...
	 -- HDCP link can be authenticated, but not send encrypted frames; if  encrypted; Ri will change every 128 Vsyncs;
	 */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_STATUS + ulOffset) ;
	ucHdcpRi = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_STATUS, RX_RI) ;
	if (ucHdcpRi != hHDR->HDCP_RxRi)
	{
		bRiChange = true ;
		hHDR->HDCP_RxRi = ucHdcpRi ;
	}

#if BHDR_CONFIG_DEBUG_HDCP_RI_UPDATES
	if ((hHDR->PreviousEncryptedFrame != EncryptedFrame)
	|| ( bRiChange))
	{
		BDBG_WRN(("CH%d Frame #: %8d  %7s  EncCount: %3d HDCP Ri %02x",
			hHDR->eCoreId, hHDR->FrameCount,
			EncryptedFrame ? "enc_en" : "enc_dis", hHDR->FrameCountEncrypted,
			hHDR->HDCP_RxRi)) ;
		hHDR->PreviousEncryptedFrame	= EncryptedFrame ;
	}
#else
	BSTD_UNUSED(bRiChange) ;
#endif

	if (hHDR->FrameCountEncrypted == 128)
		hHDR->FrameCountEncrypted = 0 ;

	if (hHDR->SymbolLossIntrCount)
	{
		uint8_t bSymbolLock ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_TST_CFG + ulOffset) ;
		bSymbolLock = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_DIGITAL_FRONT_END_TST_CFG, SYMBOL_LOCK) ;
		if (bSymbolLock)
		{
			BHDR_P_ResetHdcp_isr(hHDR);
			hHDR->SymbolLossIntrCount = 0  ;
		}

#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
		BDBG_WRN(("CH%d Symbol Lock Status: %d ; Symbol Loss detect count %d",
			hHDR->eCoreId, bSymbolLock, hHDR->SymbolLossIntrCount)) ;
#endif

	}

	/* count down frames with packet errors - EXCESSIVE_ERROR interrupt resets */
	if (hHDR->ErrorFreePacketFrames)
		hHDR->ErrorFreePacketFrames --;
	else if (hHDR->bPacketErrors)  /* report packet errors stopped once */
	{
		BDBG_WRN(("EXCESSIVE Packet errors have stopped...")) ;
		BDBG_WRN((" ")) ;
		hHDR->bPacketErrors = false ;

#if 0
		BHDR_P_ClearHdmiMode_isr(hHDR) ;
		hHDR->AvMute = false ;
#endif
	}


#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
	if  (hHDR->AvMute)
		BHDR_P_ClearAvMuteAfterNSeconds(hHDR) ;
#endif


	/* check for video format changes */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_STATUS	+ ulOffset) ;

	/* ignore unreliable saturated bits to determine video format change */
	Register &= ~ (
		BCHP_MASK(HDMI_RX_0_FORMAT_DET_UPDATE_STATUS, SATURATED_VBP)
		|BCHP_MASK(HDMI_RX_0_FORMAT_DET_UPDATE_STATUS,SATURATED_HSYNC_DELAY)) ;

	if (Register)  /* format has changed */
	{
#if BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY || BHDR_CONFIG_DEBUG_DETECTED_FORMAT_DETAIL
		BDBG_WRN(("CH%d HDMI Input Format Change Detected : 0x%08X",
			hHDR->eCoreId, Register));
#endif
		BHDR_P_GetVideoFormat_isr(hHDR) ;

		/* Update the pixel clock rate as well */
		BHDR_FE_P_GetPixelClockEstimate_isr(
			hHDR->hFeChannel, &hHDR->hFeChannel->EstimatedPixelClockRate) ;
	}

#if BHDR_CONFIG_UPDATE_MAI_ON_VSYNC
	{
		BACM_SPDIF_ChannelStatus stChannelStatus ;
		uint8_t uiChannelStatusSampleFreq ;

		BHDR_P_GetChannelStatusBits_isr(hHDR, &stChannelStatus) ;

		uiChannelStatusSampleFreq = BCHP_GET_FIELD_DATA(hHDR->uiChannelStatus30,
			HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_3_0,
			CHANNEL_STATUS_27_24_AUDIO_FREQ) ;

		if (uiChannelStatusSampleFreq != hHDR->uiChannelStatusSampleFreq)
		{
#if BHDR_CONFIG_DEBUG_AUDIO_FORMAT
			BDBG_WRN(("Audio Sample Rate Change Detected %x", uiChannelStatusSampleFreq)) ;
#endif
			hHDR->uiChannelStatusSampleFreq = uiChannelStatusSampleFreq ;
			BHDR_P_ConfigureAudioMaiBus_isr(hHDR) ;
		}
	}
#endif

	/* increment Vsync Counter */
	hHDR->VSyncCounter++ ;

	if (!HdmiMode)
	{
		hHDR->bPacketErrors = false ;
		return ;
	}


#if BHDR_CONFIG_DEBUG_TIMER_S
	{
		uint32_t timestamp ;
		BTMR_ReadTimer_isr(hHDR->timerGeneric, &timestamp) ;
		if ((timestamp - hHDR->DebugTimeStamp) > (BHDR_CONFIG_DEBUG_TIMER_S  * BHDR_P_SECOND))
		{
			BHDR_P_DebugMonitorHdmiRx_isr (hHDR) ;
			hHDR->DebugTimeStamp = timestamp ;
		}
	}
#endif
	BDBG_LEAVE(BHDR_P_ProcessVerticalBlankEnd_isr) ;

 }


void BHDR_P_ResetHdcp_isr(BHDR_Handle hHDR)
{
	uint32_t Register ;
	uint32_t ulOffset ;
	BREG_Handle hRegister ;

	BDBG_ENTER(BHDR_P_ResetHdcp_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* HDCP SW_INIT resets Bcaps, DFE SW_INIT resets BStatus */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT + ulOffset) ;

	Register |= BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, HDCP)
			| BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, DFE);

	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT + ulOffset, Register) ;

	Register &= ~ (BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, HDCP)
				| BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, DFE));
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT + ulOffset, Register) ;


	BDBG_LEAVE(BHDR_P_ResetHdcp_isr) ;
}


#if BHDR_HAS_HDMI_20_SUPPORT
static void BHDR_P_ClearScdcStatus_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;


	BDBG_ENTER(BHDR_P_ClearScdcStatus_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* Clear any SCDC Status set by the Source */
#if BHDR_CONFIG_MANUAL_SCDC_CLEAR
	BDBG_MSG(("Manually clear SCDC registers")) ;
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT) ;
		Register &= ~ BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, I2C) ;
		Register |= BCHP_FIELD_DATA(DVP_HR_HDMI_RX_0_SW_INIT, I2C, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT, Register) ;
		Register &= ~ BCHP_MASK(DVP_HR_HDMI_RX_0_SW_INIT, I2C) ;
		Register |= BCHP_FIELD_DATA(DVP_HR_HDMI_RX_0_SW_INIT, I2C, 0) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_0_SW_INIT, Register) ;
#endif

	/* enable SCDC and RDB writes */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2,  ENABLE_SCDC_REGISTER_WRITES) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, I2C_SCDC_ENABLE) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, I2C_SCDC_ENABLE, 1) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2, ENABLE_SCDC_REGISTER_WRITES, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset,  Register) ;

	/* indicate our SCDC Sink version */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_SCDC_CFG_1 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_SCDC_CFG_1, SINK_VERSION) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_SCDC_CFG_1, SINK_VERSION, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_SCDC_CFG_1 + ulOffset, Register) ;

	/* disable SCDC RDB writes */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2,  ENABLE_SCDC_REGISTER_WRITES) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2 + ulOffset,  Register) ;

	BDBG_LEAVE(BHDR_P_ClearScdcStatus_isr) ;
}
#else
static void BHDR_P_ClearScdcStatus_isr(BHDR_Handle hHDR)
{
	BSTD_UNUSED(hHDR) ;
}
#endif



static void BHDR_P_ClearHdmiMode_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;


	BDBG_ENTER(BHDR_P_ClearHdmiMode_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* as long as symbol loss occurs, force core back to DVI mode */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_0_DIGITAL_FRONT_END_CTL, CLEAR_HDMI) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset, Register) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_DIGITAL_FRONT_END_CTL, CLEAR_HDMI) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_DIGITAL_FRONT_END_CTL + ulOffset, Register) ;

	hHDR->bHdmiMode = 0 ;

	/* Clear hdcp register */
	BHDR_P_ResetHdcp_isr(hHDR);

	/* disable AvMute if it is currently enabled */
	if (hHDR->AvMute)
	{
		/* clear the AvMute Status if enabled */
		BDBG_WRN(("Clearing HDMI mode; callback to clear AvMute...")) ;
		hHDR->AvMute = 0 ;

		if (hHDR->pfAvMuteNotifyCallback)
		{
			hHDR->pfAvMuteNotifyCallback(
				hHDR->pvAvMuteNotifyParm1, hHDR->iAvMuteNotifyParm2,  &hHDR->AvMute) ;
		}
	}

	/* clear Deep Color mode flags */
	BDBG_MSG(("CH%d HDMI mode cleared; Color Depth: 24 bpp", hHDR->eCoreId)) ;
	hHDR->bDeepColorMode = false ;
	hHDR->GeneralControlPacket.eColorDepth = BAVC_HDMI_GCP_ColorDepth_e24bpp ;

	/* disable local audio  */
	BHDR_P_EnableAudio_isr(hHDR, false) ;

	/* inform of format change so default to DVI/RGB  mode */
	if (hHDR->pfVideoFormatChangeCallback)
	{
		/* inform of DVI mode to force RGB colorspace */
		hHDR->AviInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
		hHDR->pfVideoFormatChangeCallback(hHDR->pvVideoFormatChangeParm1,
			BAVC_HDMI_PacketType_eAviInfoFrame, &hHDR->AviInfoFrame) ;
	}

	/* if needed, inform of VSI change to 2D (default) mode  */
	if (hHDR->VSInfoFrame.eHdmiVideoFormat != BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone)
	{
		hHDR->VSInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
		hHDR->VSInfoFrame.eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;

		if (hHDR->pfPacketChangeCallback)
		{
			hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
				BAVC_HDMI_PacketType_eVendorSpecificInfoframe, &hHDR->VSInfoFrame) ;
		}
	}
	BDBG_LEAVE(BHDR_P_ClearHdmiMode_isr) ;

}


/******************************************************************************
void BHDR_P_HandleInterrupt_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
void BHDR_P_HandleInterrupt_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
)
{
	BHDR_Handle hHDR ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint16_t BStatus = 0 ;
	BERR_Code rc ;

	BDBG_ENTER(BHDR_P_HandleInterrupt_isr) ;

	hHDR = (BHDR_Handle) pParam1 ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	/* make sure FrontEnd Channel has been attached */
	if (!hHDR->hFeChannel)
		goto done  ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	switch (parm2)
	{
	case MAKE_INTR_ENUM(AKSV_UPDATE) :
		BDBG_WRN(("CH%d HDCP Authentication Request; Tx Aksv Received",
						hHDR->eCoreId)) ;

		if (hHDR->bHdmiMode)
			BStatus = BAVC_HDMI_HDCP_RxStatus_eHdmiMode ;
		else
			BStatus = 0 ;
		BHDR_P_WriteRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;

#if BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING
		/* read back value */
		BHDR_P_ReadRxI2cRegisterSpace_isr(hHDR,  BAVC_HDMI_HDCP_RX_BSTATUS, (uint8_t *) &BStatus, 2) ;
		BDBG_WRN(("BSTATUS After Ksv Update: %x", BStatus)) ;
#endif

		/* Received request to authenticate in HDCP 1.x mode. Need to enable Serial Key RAM
		Refer to JIRA -CRDVP-674 for details on the HW issues that lead to a required of this work-around
		for HDCP 2.x*/
		BHDR_HDCP_P_EnableSerialKeyRam_isr(hHDR, true);


		if (hHDR->pfHdcpStatusChangeCallback)
		{
			hHDR->pfHdcpStatusChangeCallback(hHDR->pvHdcpStatusChangeParm1, 0, NULL) ;
		}

		break ;


	case MAKE_INTR_ENUM(REQUEST_KEYS) :
		{

#if BHDR_CONFIG_DEBUG_HDCP_VALUES
			BHDR_P_DebugHdcpValues_isr(hHDR) ;
#endif

			hHDR->stHdcpStatus.eAuthState = BHDR_HDCP_AuthState_eWaitForKeyloading ;
			if (hHDR->pfHdcpStatusChangeCallback)
			{
				hHDR->pfHdcpStatusChangeCallback(hHDR->pvHdcpStatusChangeParm1, 0, NULL) ;
			}
		}

		break ;

#if BHDR_CONFIG_HDCP_REPEATER
		case MAKE_INTR_ENUM(REQUEST_KSVS):
		{
#if BHDR_HAS_HDMI_20_SUPPORT
			/* for HDCP Repeater Applications */
			Register = BREG_Read32(hRegister, BCHP_HDCP2_RX_0_STATUS_0 + ulOffset);
			/* Fire callback only if in HDCP 1.x */
			if (BCHP_GET_FIELD_DATA(Register, HDCP2_RX_0_STATUS_0, HDCP_VERSION) == 0)
#endif
			{
				BDBG_WRN(("CH%d HDCP Authentication with upstream transmitter SUCCEEDED; Repeater Setting %d",
					hHDR->eCoreId, hHDR->stHdcpSettings.bRepeater)) ;
				if (hHDR->stHdcpSettings.bRepeater)
				{
					hHDR->stHdcpStatus.eAuthState = BHDR_HDCP_AuthState_eWaitForDownstreamKsvs ;
					if (hHDR->pfHdcpStatusChangeCallback)
					{
						hHDR->pfHdcpStatusChangeCallback(hHDR->pvHdcpStatusChangeParm1, 0, NULL) ;
					}
				}
			}
		}
		break ;
#endif

#if BHDR_CONFIG_DEBUG_I2C
	case MAKE_INTR_ENUM(I2C_TRANSACTION_COMPLETE) :
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_I2C_PEEK_POKE + ulOffset) ;
		Register = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_I2C_PEEK_POKE, I2C_OFFSET ) ;
		BDBG_WRN(("CH%d I2C Transaction Complete at Offset: %d",
			hHDR->eCoreId, Register)) ;
		break ;
#endif

	case MAKE_INTR_ENUM(PJ_UPDATE) :
		break ;

	case MAKE_INTR_ENUM(SYMBOL_LOSS) :   /* Int 09 */

		/* mute the audio path immediately */
		BHDR_P_EnableAudio_isr(hHDR, false) ;

		if (hHDR->SymbolLossIntrCount < BHDR_P_MAX_SYMBOL_LOSS_COUNT)
		{
			hHDR->SymbolLossIntrCount++ ;
#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
			BDBG_WRN(("CH%d SYMBOL_LOSS Interrupt #%d",
				hHDR->eCoreId, hHDR->SymbolLossIntrCount )) ;
#endif
		}


		BHDR_P_ClearHdmiMode_isr(hHDR) ;

		break ;

	case MAKE_INTR_ENUM(INVALID_DATA_ISLAND_LENGTH) :   /* Int 10 */
#if BHDR_CONFIG_DEBUG_STREAM_ERRORS
		BDBG_WRN(("CH%d HDMI Invalid Data Island Length!", hHDR->eCoreId )) ;
#endif

		/* fall through */

	case MAKE_INTR_ENUM(AUDIO_VALIDITY_BIT_UPDATE) :     /* Int 12 */
#if BHDR_CONFIG_DEBUG_STREAM_ERRORS
		BDBG_WRN(("CH%d HDMI Audio Validity Bit Update!", hHDR->eCoreId) ;
#endif

	break ;


	case MAKE_INTR_ENUM(CHANNEL_STATUS_UPDATE) :
		BDBG_MSG(("Channel Status Update...")) ;
#if BHDR_CONFIG_UPDATE_MAI_ON_VSYNC == 0
		BHDR_P_GetChannelStatusBits_isr(hHDR, &hHDR->stChannelStatus) ;
#endif
		rc = BHDR_P_ConfigureAudioMaiBus_isr(hHDR) ;
		if (rc) { rc = BERR_TRACE(rc) ; }

 		BKNI_SetEvent(hHDR->BHDR_Event_AudioChannelStatusUpdate) ;

		break ;


	case MAKE_INTR_ENUM(RAM_PACKET_UPDATE) :
		if (hHDR->bPacketErrors)
		{
			BHDR_P_InitializePacketRAM_isr(hHDR) ;
			goto done ;
		}

		BHDR_P_ProcessReceivedPackets_isr(hHDR) ;
		break ;

	case MAKE_INTR_ENUM(RAM_PACKET_STOP) :
		if (hHDR->bPacketErrors)
		{
			BHDR_P_InitializePacketRAM_isr(hHDR) ;
			goto done ;
		}
		BHDR_P_ProcessStoppedPackets_isr(hHDR) ;
		break ;


	case MAKE_INTR_ENUM(PACKET_SYNC_ERROR) :
		BDBG_WRN(("CH%d Packet Error", hHDR->eCoreId)) ;
		break ;

	case MAKE_INTR_ENUM(LAYOUT_UPDATE) :
		break ;

	case MAKE_INTR_ENUM(AUDIO_TYPE_CHANGE) :
		break;

	/* cases to shut off audio */
	case MAKE_INTR_ENUM(SET_AV_MUTE) :
		break ;

	case MAKE_INTR_ENUM(AV_MUTE_UPDATE) :
	{
		uint8_t AvMute, AvMuteMode ;
		uint8_t AvMuteChange ;

		/*
		   AV_MUTE_MODE
		   0- use AvMute from HDCP
		   1- use AvMute from Packet Processor
		*/
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_CONFIG + ulOffset);
		AvMuteMode = (uint8_t) BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_CONFIG, AV_MUTE_MODE) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_STATUS + ulOffset) ;
		AvMute = (uint8_t) BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_0_MISC_STATUS, PKT_PROCESSOR_AV_MUTE) ;

#if BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY
		/* enable/disable audio accordingly */
		BHDR_P_EnableAudio_isr(hHDR, !AvMute) ;
#endif
		BDBG_MSG(("CH%d AvMuteMode: %d AV MUTE Update from %d to %d",
			hHDR->eCoreId, AvMuteMode, hHDR->AvMute, AvMute)) ;
                BSTD_UNUSED(AvMuteMode);

		AvMuteChange =  (hHDR->AvMute != AvMute) ;
		hHDR->AvMute = AvMute ;



		/* call the callback functions for AvMute (on/off) notification */
		if (hHDR->pfAvMuteNotifyCallback)
		{
			hHDR->pfAvMuteNotifyCallback(hHDR->pvAvMuteNotifyParm1, hHDR->iAvMuteNotifyParm2,  &hHDR->AvMute) ;
		}
		else
		{
			BDBG_WRN(("CH%d No AvMute callback installed...",
				hHDR->eCoreId)) ;
		}

#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
		/* if AvMute has been activated start the 5s timer */
		if (AvMuteChange && AvMute)
		{
			BTMR_ReadTimer_isr(hHDR->timerGeneric, &hHDR->AvMuteTimeStamp) ;
			BDBG_MSG(("CH%d START auto Clear_AVMUTE timer!",
				hHDR->eCoreId));
		}
		else /* disable the timer since we are no longer muted */
		{
			BDBG_MSG(("CH%d STOP auto Clear_AVMUTE timer!",
				hHDR->eCoreId));
		}
#else
		BSTD_UNUSED(AvMuteChange) ;
#endif
 		break ;
	}

	case MAKE_INTR_ENUM(VSYNC_LEAD_EDGE):
		BDBG_ERR(("VSYNC LEAD EDGE interrupt should not be used")) ;
		break ;


	case MAKE_INTR_ENUM(VERTICAL_BLANK_END):
		BHDR_P_ProcessVerticalBlankEnd_isr(hHDR) ;

		break ;

	case MAKE_INTR_ENUM(DVI_TO_HDMI) :
	case MAKE_INTR_ENUM(HDMI_TO_DVI) :

#if BHDR_CONFIG_DELAY_MODE_CHANGE_MS

#if BHDR_CONFIG_DEBUG_MODE_CHANGE
		BDBG_WRN(("Start Delay Mode Change Timer for %d ms... %d",
			BHDR_CONFIG_DELAY_MODE_CHANGE_MS, 	__LINE__)) ;
#endif
		BTMR_StartTimer_isr(hHDR->timerDviHdmiModeChange,
			BHDR_P_MILLISECOND * BHDR_CONFIG_DELAY_MODE_CHANGE_MS) ;

#else
		BHDR_P_ProcessModeChange_isr(hHDR) ;
#endif
		break ;


	case MAKE_INTR_ENUM(EXCESSIVE_PACKET_ERRORS) :
		if (!hHDR->bPacketErrors)
		{
			BDBG_WRN(("EXCESSIVE Packet errors detected...")) ;
		}

		hHDR->bPacketErrors = true ;

		if (hHDR->ErrorFreePacketFrames < BHDR_P_MAX_ERROR_FREE_PACKET_FRAMES)
			hHDR->ErrorFreePacketFrames = BHDR_P_MAX_ERROR_FREE_PACKET_FRAMES ;

		break ;

	case MAKE_INTR_ENUM(FORMAT_DETECT_COUNT_SATURATED) :
#if BHDR_CONFIG_DEBUG_DETECTED_FORMAT_DETAIL
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_STATUS + ulOffset) ;
		BDBG_WRN(("CH%d FORMAT DETECTION COUNT SATURATION %X",
			hHDR->eCoreId, Register)) ;
#endif
		break ;

	case MAKE_INTR_ENUM(ERROR_INTERRUPT) :
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_ERROR_INTERRUPT_STATUS + ulOffset) ;
		BDBG_ERR(("CH%d ERROR_INTERRUPT: %d", hHDR->eCoreId, Register)) ;
		break ;


	default	:
		BDBG_ERR(("CH%d Unknown Interrupt ID:%d",  hHDR->eCoreId, parm2)) ;
	} ;

	/* L2 interrupts are reset automatically */
done:
	BDBG_LEAVE(BHDR_P_HandleInterrupt_isr) ;
	return ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_InitializePacketRAM(
   BHDR_Handle hHDR
)
{
	uint32_t rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_InitializePacketRAM) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BKNI_EnterCriticalSection() ;
	BHDR_P_InitializePacketRAM_isr(hHDR) ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InitializePacketRAM) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetGeneralControlPacketData(BHDR_Handle hHDR,
	BAVC_HDMI_GcpData * Gcpdata)
{
	BKNI_Memcpy(Gcpdata, &hHDR->GeneralControlPacket, sizeof(BAVC_HDMI_GcpData)) ;
	return BERR_SUCCESS ;
}


BERR_Code BHDR_GetRawPacketData(BHDR_Handle hHDR,
	BAVC_HDMI_PacketType ePacketType,
	BAVC_HDMI_Packet *pPacketBytes)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_GetRawPacketData) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BKNI_EnterCriticalSection();
	switch (ePacketType)
	{
	case BAVC_HDMI_PacketType_eAviInfoFrame :
		BKNI_Memcpy(pPacketBytes, &hHDR->AviInfoFrame.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eAudioInfoFrame :
		BKNI_Memcpy(pPacketBytes, &hHDR->AudioInfoFrame.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eSpdInfoFrame :
		BKNI_Memcpy(pPacketBytes, &hHDR->SPDInfoFrame.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eVendorSpecificInfoframe:
		BKNI_Memcpy(pPacketBytes, &hHDR->VSInfoFrame.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eAudioContentProtection :
		BKNI_Memcpy(pPacketBytes, &hHDR->AudioContentProtection.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eAudioClockRegeneration :
		BKNI_Memcpy(pPacketBytes, &hHDR->AudioClockRegenerationPacket.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	case BAVC_HDMI_PacketType_eGeneralControl :
		BKNI_Memcpy(pPacketBytes, &hHDR->GeneralControlPacket.stPacket,
			sizeof(BAVC_HDMI_Packet)) ;
		break ;

	default :
		BDBG_ERR(("Raw Packet Copy not supported")) ;
		rc = BERR_NOT_SUPPORTED ;
	}
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDR_GetRawPacketData) ;
	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
static void BHDR_P_GetAudioSampleFreq_isr(BHDR_Handle hHDR,
	uint8_t *AudioSampleFreq)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	BDBG_ENTER(BHDR_P_GetAudioSampleFreq_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_3_0 + ulOffset) ;
	*AudioSampleFreq = BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_3_0, CHANNEL_STATUS_27_24_AUDIO_FREQ) ;

	BDBG_LEAVE(BHDR_P_GetAudioSampleFreq_isr) ;
}


/******************************************************************************
Summary: install Packet Change Callback to notify of packet changes or stopped packets
*******************************************************************************/
BERR_Code BHDR_InstallPacketChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_InstallPacketChangeCallback) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfPacketChangeCallback = pfCallback_isr ;
		hHDR->pvPacketChangeParm1 = pvParm1 ;
		hHDR->iPacketChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InstallPacketChangeCallback);

	return rc ;
}



/******************************************************************************
Summary: Uninstall Packet Change Callback
*******************************************************************************/
BERR_Code BHDR_UnInstallPacketChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_UnInstallPacketChangeCallback) ;

	BKNI_EnterCriticalSection() ;
		hHDR->pfPacketChangeCallback = (BHDR_CallbackFunc) NULL  ;
		hHDR->pvPacketChangeParm1 = NULL ;
		hHDR->iPacketChangeParm2 = 0 ;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDR_UnInstallPacketChangeCallback) ;
	return rc;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_InstallVideoFormatChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for format changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_InstallVideoFormatChangeCallback) ;

	/* Check if this is a valid function */
	if (( pfCallback_isr == NULL ) || (pvParm1 == NULL) )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfVideoFormatChangeCallback = pfCallback_isr ;
		hHDR->pvVideoFormatChangeParm1 = pvParm1 ;
		hHDR->iVideoFormatChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InstallVideoFormatChangeCallback);

	return rc;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_UnInstallVideoFormatChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_UnInstallVideoFormatChangeCallback);

	BKNI_EnterCriticalSection() ;
		hHDR->pfVideoFormatChangeCallback = (BHDR_CallbackFunc) NULL  ;
		hHDR->pvVideoFormatChangeParm1 = NULL ;
		hHDR->iVideoFormatChangeParm2 = 0 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_UnInstallVideoFormatChangeCallback);
	return rc;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_InstallAudioFormatChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for format changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_InstallAudioFormatChangeCallback) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfAudioFormatChangeCallback = pfCallback_isr ;
		hHDR->pvAudioFormatChangeParm1 = pvParm1 ;
		hHDR->iAudioFormatChangeParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InstallAudioFormatChangeCallback) ;

	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_UnInstallAudioFormatChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_UnInstallAudioFormatChangeCallback) ;

	BKNI_EnterCriticalSection() ;
		hHDR->pfAudioFormatChangeCallback = (BHDR_CallbackFunc) NULL  ;
		hHDR->pvAudioFormatChangeParm1 = NULL ;
		hHDR->iAudioFormatChangeParm2 = 0 ;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDR_UnInstallAudioFormatChangeCallback) ;
	return rc;
}




/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_InstallAvMuteNotifyCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for format changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_InstallAvMuteNotifyCallback) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}


	BKNI_EnterCriticalSection() ;
#if BHDR_CONFIG_DEBUG_DISABLE_AVMUTE_CB
		if (pfCallback_isr)
		{
			BDBG_WRN(("CH%d AV MUTE NOTIFY CALLBACK NOT INSTALLED", hHDR->eCoreId)) ;
		}
		else
		{
			BDBG_WRN(("CH%d AV MUTE NOTIFY CALLBACK IS NULL", hHDR->eCoreId )) ;
		}
		hHDR->pfAvMuteNotifyCallback = NULL ;
#else
		hHDR->pfAvMuteNotifyCallback = pfCallback_isr;
#endif
		hHDR->pvAvMuteNotifyParm1 = pvParm1 ;
		hHDR->iAvMuteNotifyParm2 = iParm2;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InstallAvMuteNotifyCallback);

	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_UnInstallAvMuteNotifyCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for Av Mute Notification */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_UnInstallAvMuteNotifyCallback);

	BKNI_EnterCriticalSection() ;
		hHDR->pfAvMuteNotifyCallback = (BHDR_CallbackFunc) NULL ;
		hHDR->pvAvMuteNotifyParm1 = NULL ;
		hHDR->iAvMuteNotifyParm2 = 0 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_UnInstallAvMuteNotifyCallback);
	return rc;
}


/******************************************************************************
Summary: Install Callback used to Notify for Packet Errors
*******************************************************************************/
BERR_Code BHDR_InstallPacketErrorChangeCallback(
	BHDR_Handle hHDR,			/* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr, /* [in] cb for packet error changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code			rc = BERR_SUCCESS;

	BDBG_ENTER(BHDR_InstallPacketErrorChangeCallback) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDR->pfPacketErrorCallback = pfCallback_isr;
		hHDR->pvPacketErrorParm1 = pvParm1 ;
		hHDR->iPacketErrorParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_InstallPacketErrorChangeCallback);

	return rc ;
}


/******************************************************************************
Summary: Uninstall Callback used to Notify for Packet Errors
*******************************************************************************/
BERR_Code BHDR_UnInstallPacketErrorChangeCallback(
	BHDR_Handle hHDR,                       /* [in] HDMI Rx Handle */
	const BHDR_CallbackFunc pfCallback_isr) /* [in] cb for Packet Error change Notification */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDR_UnInstallPacketErrorChangeCallback);
	BKNI_EnterCriticalSection() ;

		hHDR->pfPacketErrorCallback = (BHDR_CallbackFunc) NULL ;
		hHDR->pvPacketErrorParm1 = NULL  ;
		hHDR->iPacketErrorParm2 = 0 ;

	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_UnInstallPacketErrorChangeCallback);

	return rc;
}


static BERR_Code BHDR_P_GetChannelStatusBits_isr(
	BHDR_Handle hHDR, BACM_SPDIF_ChannelStatus *stChannelStatus)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset ;
	uint32_t Register ;
	uint8_t auiChannelStatus[8] ;
	bool change = false ;

	BDBG_ENTER(BHDR_P_GetChannelStatusBits_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);
	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	/* get the audio spdif channel bits */
	Register = BREG_Read32(hRegister,
		BCHP_HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_3_0 + ulOffset) ;
	if (Register != hHDR->uiChannelStatus30)
	{
		change = true ;

		auiChannelStatus[0] = (uint8_t) (Register & 0x000000FF) ;
		auiChannelStatus[1] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		auiChannelStatus[2] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		auiChannelStatus[3] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

		hHDR->uiChannelStatus30 = Register ;
	}

	Register = BREG_Read32(hRegister,
		BCHP_HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_7_4 + ulOffset) ;
	if (Register != hHDR->uiChannelStatus74)
	{
		change = true ;

		auiChannelStatus[4] = (uint8_t) (Register & 0x000000FF) ;
		auiChannelStatus[5] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		auiChannelStatus[6] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		auiChannelStatus[7] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

		hHDR->uiChannelStatus74 = Register ;
	}

	if (change)
	{
		/* initialze the Channel Status Structure; parse the data */
		BACM_SPDIF_InitChannelStatus_isrsafe(stChannelStatus) ;
		BACM_SPDIF_ParseChannelStatus_isrsafe(stChannelStatus, 8, auiChannelStatus) ;

#if BHDR_CONFIG_DEBUG_CHANNEL_STATUS
		BDBG_LOG(("ChannelStatus %08x - %08x",
 			hHDR->uiChannelStatus74, hHDR->uiChannelStatus30)) ;

		BDBG_LOG(("Copyright %d", stChannelStatus->bCopyrighted)) ;
		BDBG_LOG(("eApplication %d", stChannelStatus->eApplication)) ;
		BDBG_LOG(("eChannelNumber %d", stChannelStatus->eChannelNumber)) ;
		BDBG_LOG(("Mode %d", stChannelStatus->eChannelStatusMode)) ;
		BDBG_LOG(("Clock Accuracy %d", stChannelStatus->eClockAccuracy)) ;
		BDBG_LOG(("OSamplingFreq %d", stChannelStatus->eOriginalSamplingFrequency)) ;
		BDBG_LOG(("eSamplingFreq %d", stChannelStatus->eSamplingFrequency)) ;
		BDBG_LOG(("eSourceNumber %d", stChannelStatus->eSourceNumber)) ;
		BDBG_LOG(("eStreamType %d", stChannelStatus->eStreamType)) ;
		BDBG_LOG(("eWordLength %d", stChannelStatus->eWordLength)) ;
		BDBG_LOG(("uAdditionalFormatInfo %d", stChannelStatus->uAdditionalFormatInfo)) ;
		BDBG_LOG(("Category Code %d", stChannelStatus->uiCategoryCode)) ;
#endif
	}
	BDBG_LEAVE(BHDR_P_GetChannelStatusBits_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetPixelClockEstimate(BHDR_Handle hHDR,
	uint32_t *EstimatedPixelClockRate
)
{
	uint32_t rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_GetPixelClockEstimate) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	/* report warning message once */
	if ((!hHDR->hFeChannel->bTxDeviceAttached)
	&&  (hHDR->hFeChannel->bTxDeviceAttached != hHDR->hFeChannel->bPreviousTxDeviceAttached))
	{
		*EstimatedPixelClockRate = 0 ;
		BDBG_WRN(("CH%d Unable to Get Pixel Clock; No HDMI Input Source",
			hHDR->eCoreId)) ;
		hHDR->hFeChannel->bPreviousTxDeviceAttached = false ;
	}
	else
	{
		* EstimatedPixelClockRate = hHDR->hFeChannel->EstimatedPixelClockRate ;
	}

	BDBG_LEAVE(BHDR_GetPixelClockEstimate) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetHdmiRxStatus(
	BHDR_Handle hHDR,           /* [in] HDMI Rx Handle */
	BHDR_Status *pHdmiStatus   /* struct Status Info */
)
{
	BDBG_ENTER(BHDR_GetHdmiRxStatus) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	/* default/ initialize pHdmiStatus */
	BKNI_Memset(pHdmiStatus, 0, sizeof(*pHdmiStatus)) ;
	/* default symbol loss to true  */
	pHdmiStatus->bSymbolLoss = 	true ;

	if (!hHDR->hFeChannel)
	{
		BDBG_WRN(("No Front End attached to HDMI_RX_%d", hHDR->eCoreId)) ;
		goto BHDR_GetHdmiRxStatus_Done ;
	}

	/* report warning message once */
	if ((!hHDR->hFeChannel->bTxDeviceAttached)
	&&  (hHDR->hFeChannel->bTxDeviceAttached != hHDR->hFeChannel->bPreviousTxDeviceAttached))
	{
		BDBG_WRN(("CH%d Unable to Get HDMI Status; NO DEVICE CONNECTED",
			hHDR->eCoreId)) ;

		hHDR->hFeChannel->bPreviousTxDeviceAttached = false ;

		goto BHDR_GetHdmiRxStatus_Done ;
	}

 	pHdmiStatus->PllLocked = hHDR->hFeChannel->bPllLocked ;

	pHdmiStatus->bAvmute = hHDR->AvMute ;

	pHdmiStatus->HdmiMode	      = hHDR->bHdmiMode ;
	pHdmiStatus->DeviceAttached = hHDR->hFeChannel->bTxDeviceAttached ? 1 : 0 ;

	pHdmiStatus->bPacketErrors   = hHDR->bPacketErrors ;

	pHdmiStatus->uiAudioPackets =  hHDR->HdmiAudioPackets ;
	pHdmiStatus->VSyncCounter = hHDR->VSyncCounter ;
	pHdmiStatus->bHdcpRiUpdating = hHDR->bHdcpRiUpdating ;

	pHdmiStatus->bSymbolLoss = 	(hHDR->SymbolLossIntrCount > 0) ;

	/* HDMI_TODO Use Format Detection Registers to indicate Stable Format for now use bPacketErrors */
	pHdmiStatus->bFormatStable = hHDR->bPacketErrors ;

	pHdmiStatus->PixelRepeat = hHDR->AviInfoFrame.PixelRepeat ;

	pHdmiStatus->uiAudioSampleRateHz = hHDR->AudioSampleRateHz ;

	/* Reported Channel Status cannot be used */
	BKNI_Memcpy(&pHdmiStatus->stChannelStatus, &hHDR->stChannelStatus,
		sizeof(BACM_SPDIF_ChannelStatus)) ;
	pHdmiStatus->bValidChannelStatus = true ;

	/* set status to ok if PLL is locked and there is no symbol loss */
	/* otherwise the information is unreliable */
	if ((pHdmiStatus->PllLocked) && (!pHdmiStatus->bSymbolLoss))
	{
		pHdmiStatus->bValidStatus = true  ;
	}
	else if (hHDR->bPreviousStatus)  /* report bad status only once */
	{
#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
		BDBG_WRN(("CH%d PLL Locked: %d; SymbolLossCount: %d",
			hHDR->eCoreId,
			pHdmiStatus ->PllLocked, hHDR->SymbolLossIntrCount)) ;
#endif
	}

	hHDR->bPreviousStatus = pHdmiStatus->bValidStatus ;

BHDR_GetHdmiRxStatus_Done:
	BDBG_LEAVE(BHDR_GetHdmiRxStatus) ;
	return BERR_SUCCESS  ;
}


/******************************************************************************
Summary: Get the status of the HDMI Error Packet Status
return : 1 Error exist on input; 0 No error on input
*******************************************************************************/
void BHDR_GetErrorPacketStatus(BHDR_Handle hHDR, bool *isError)
{
	BDBG_ENTER(BHDR_GetErrorPacketStatus) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	*isError = false;
	if( hHDR->ErrorFreePacketFrames != 0)
	{
		*isError = true;
	}
	BDBG_LEAVE(BHDR_GetErrorPacketStatus) ;
}


static void BHDR_P_GetVideoFormat_isr(BHDR_Handle hHDR)
{

	uint32_t Register ;
   	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	BDBG_ENTER(BHDR_P_GetVideoFormat_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);


	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	Register = 0xFFFFFFFF;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

	Register = 0;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_FORMAT_DET_UPDATE_CLEAR + ulOffset, Register) ;

	/* read format detection registers */

	/*read HDMI_RX_0_FORMAT_DET_1 - VID_FORMAT_1 */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_1  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uHsyncPolarity = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_1, UUT_HPOL) ;
	hHDR->stHdmiVideoFormat.uHsyncPulsePixels = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_1, UUT_HSP) ;
	hHDR->stHdmiVideoFormat.uHsyncBackporchPixels = BCHP_GET_FIELD_DATA (Register, HDMI_RX_0_FORMAT_DET_1, UUT_HBP);

	/*read HDMI_RX_0_FORMAT_DET_2 - VID_FORMAT_2 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_2  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uHorizontalActivePixels = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_2, UUT_HAP) ;
	hHDR->stHdmiVideoFormat.uHorizontalFrontporchPixels = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_2, UUT_HFP) ;

	/*read HDMI_RX_0_FORMAT_DET_3 - VID_FORMAT_3 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_3  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uDelayedFieldVersion = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, UUT_FIELD_LATCHED) ;
	hHDR->stHdmiVideoFormat.uProgressive = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, PROGRESSIVE) ;
	hHDR->stHdmiVideoFormat.uInterlaced = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, INTERLACED) ;
	hHDR->stHdmiVideoFormat.uVsyncPolarity = BCHP_GET_FIELD_DATA (Register, HDMI_RX_0_FORMAT_DET_3, UUT_VPOL);
	hHDR->stHdmiVideoFormat.uField = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, UUT_FIELD) ;
	hHDR->stHdmiVideoFormat.uVerticalDe = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, UUT_VDE) ;
	hHDR->stHdmiVideoFormat.uVerticalFrontporchLinesField1 = BCHP_GET_FIELD_DATA (Register, HDMI_RX_0_FORMAT_DET_3, UUT_VFP0);
	hHDR->stHdmiVideoFormat.uVerticalSyncpulseLinesField1 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_3, UUT_VSP0) ;

	/*read HDMI_RX_0_FORMAT_DET_4 - VID_FORMAT_4 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_4  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uVerticalBackporchLinesField1 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_4, UUT_VBP0) ;
	hHDR->stHdmiVideoFormat.uVerticalSyncPulsePixelsField1 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_4, UUT_VSPO0) ;


	/*read HDMI_RX_0_FORMAT_DET_5 - VID_FORMAT_5 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_5  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uActiveLinesField2 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_5, UUT_VAL1) ;
	hHDR->stHdmiVideoFormat.uVerticalFrontPorchlinesField2 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_5, UUT_VFP1) ;

	/*read HDMI_RX_0_FORMAT_DET_6 - VID_FORMAT_6 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_6  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uVerticalBackporchLinesField2 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_6, UUT_VBP1) ;
	hHDR->stHdmiVideoFormat.uVerticalSyncPulsePixelsField2 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_6, UUT_VSPO1) ;

	/*read HDMI_RX_0_FORMAT_DET_7 - VID_FORMAT_6 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_7  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uVdeCopy = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_7, UUT_VDE_COPY) ;
	hHDR->stHdmiVideoFormat.uCurrentReveivedVideoLine = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_7, UUT_CURRENT_LINE_COUNT) ;

	/*read HDMI_RX_0_FORMAT_DET_8 - VID_FORMAT_6 Register */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_0_FORMAT_DET_8  + ulOffset) ;
	hHDR->stHdmiVideoFormat.uActivelinesField1 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_8, UUT_VAL0) ;
	hHDR->stHdmiVideoFormat.uVerticalSyncPulseLinesField2 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_FORMAT_DET_8, UUT_VSP1) ;

#if BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY
	BDBG_WRN(("CH%d Detected Format: %d[%s] x %d/%d[%s]",
		hHDR->eCoreId,
		hHDR->stHdmiVideoFormat.uHorizontalActivePixels,
		hHDR->stHdmiVideoFormat.uHsyncPolarity ? "+": "-",
		hHDR->stHdmiVideoFormat.uActivelinesField1, hHDR->stHdmiVideoFormat.uActiveLinesField2,
		hHDR->stHdmiVideoFormat.uVsyncPolarity ? "+" : "-"));
#elif BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY_WITH_VSYNC_RATE
        {
        uint32_t VsyncRateWhole ;
        uint32_t VsyncRateFraction ;

        BHDR_FE_P_VsyncCountToRate(hHDR->hFeChannel, &VsyncRateWhole, &VsyncRateFraction) ;

	BDBG_WRN(("CH%d Detected Format: %d[%s] x %d/%d[%s]: %d.%02d Hz",
		hHDR->eCoreId,
		hHDR->stHdmiVideoFormat.uHorizontalActivePixels,
		hHDR->stHdmiVideoFormat.uHsyncPolarity ? "+": "-",
		hHDR->stHdmiVideoFormat.uActivelinesField1, hHDR->stHdmiVideoFormat.uActiveLinesField2,
                hHDR->stHdmiVideoFormat.uVsyncPolarity ? "+" : "-",
                VsyncRateWhole, VsyncRateFraction));
        }
#endif

#if BHDR_CONFIG_DEBUG_DETECTED_FORMAT_DETAIL
	BDBG_WRN(("\tHSync Pulse (pixels): %d; Back Porch: %d; Front Porch: %d",
		hHDR->stHdmiVideoFormat.uHsyncPulsePixels,
		hHDR->stHdmiVideoFormat.uHsyncBackporchPixels,
		hHDR->stHdmiVideoFormat.uHorizontalFrontporchPixels));

	BDBG_WRN(("\t                  Field1         Field2")) ;
	BDBG_WRN(("\tVert Front Porch:    %3d            %3d",
		hHDR->stHdmiVideoFormat.uVerticalFrontporchLinesField1,
		hHDR->stHdmiVideoFormat.uVerticalFrontPorchlinesField2)); /* The integer number of lines in the vertical front porch of field 1*/

	BDBG_WRN(("\tVert Back Porch:     %3d            %3d",
		hHDR->stHdmiVideoFormat.uVerticalBackporchLinesField1,
		hHDR->stHdmiVideoFormat.uVerticalBackporchLinesField2)) ; /* The number of lines in the vertical back porch of field 1*/
	BDBG_WRN(("\tVert Sync Pulse:     %3d            %3d",
		hHDR->stHdmiVideoFormat.uVerticalSyncpulseLinesField1,
		hHDR->stHdmiVideoFormat.uVerticalSyncPulseLinesField2)) ; /* The number of lines in the vertical sync pulse of field 1*/
	BDBG_WRN(("\tVert Sync Pulse:     %3d            %3d",
		hHDR->stHdmiVideoFormat.uVerticalSyncPulsePixelsField1,
		hHDR->stHdmiVideoFormat.uVerticalSyncPulsePixelsField2)) ; /* The number of pixels the leading edge of the vertical sync pulse is offset from the horizontal sync pulse in field 1*/
	BDBG_WRN(("\tDelayed Field Version: %d",
		hHDR->stHdmiVideoFormat.uDelayedFieldVersion)); /* Delayed version of UUT_FIELD. Updates on the first active line following a vertical blank*/
	BDBG_WRN(("\tActive Field: %d", hHDR->stHdmiVideoFormat.uField)) ; /* This denotes the currently active field, It is updated every leading edge of the vertical sync*/
	BDBG_WRN(("\tData Enable: %d", hHDR->stHdmiVideoFormat.uVerticalDe)) ; /* vertical de*/
	BDBG_WRN(("\tCurrent Received Video Line :%d",
		hHDR->stHdmiVideoFormat.uCurrentReveivedVideoLine)) ; /*The current line of video being recieved, Line 1 is the first line of active video, The count continues into the blanking period*/
#endif

	BDBG_LEAVE(BHDR_P_GetVideoFormat_isr) ;
}


BERR_Code BHDR_GetHdmiRxDetectedVideoFormat(BHDR_Handle hHDR, BAVC_HDMI_VideoFormat *pHdmiFmt)
{
	BDBG_ENTER(BHDR_GetHdmiRxDetectedVideoFormat) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset(pHdmiFmt, 0, sizeof(BAVC_HDMI_VideoFormat)) ;

	/* report warning message once */
	if ((!hHDR->hFeChannel->bTxDeviceAttached)
	&&  (hHDR->hFeChannel->bTxDeviceAttached != hHDR->hFeChannel->bPreviousTxDeviceAttached))
	{
		BDBG_WRN(("CH%d Unable to Get Video Format; No HDMI Input Source",
			hHDR->eCoreId)) ;

		hHDR->hFeChannel->bPreviousTxDeviceAttached = false ;

		goto BHDR_GetHdmiRxDetectedVideoFormat_Done ;
	}

	BKNI_Memcpy(pHdmiFmt, &hHDR->stHdmiVideoFormat, sizeof(BAVC_HDMI_VideoFormat)) ;

BHDR_GetHdmiRxDetectedVideoFormat_Done:
	BDBG_LEAVE(BHDR_GetHdmiRxDetectedVideoFormat) ;
	return BERR_SUCCESS  ;
}

#if BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY
/* Handle Audio - mute the audio here */
static void BHDR_P_EnableAudio_isr(BHDR_Handle hHDR, bool enable)
{

	/* Set  Audio Mute flag for the Audio Core */
	uint32_t Register ;
   	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	BDBG_ENTER(BHDR_P_EnableAudio_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BDBG_MSG(("Turn Audio %s", enable ? "ON" : "OFF")) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_MUTE_CFG + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_AUDIO_MUTE_CFG, ENABLE_AUDIO_MUTE) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_MUTE_CFG, ENABLE_AUDIO_MUTE, !enable) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_AUDIO_MUTE_CFG + ulOffset, Register) ;


	/* enable/disable the MAI Bus */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG, DISABLE_MAI_BUS) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_CFG, DISABLE_MAI_BUS, !enable) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;
	BDBG_LEAVE(BHDR_P_EnableAudio_isr) ;

}
#endif


/******************************************************************************
BHDR_P_HotPlugConnect_isr
Summary: Hot Plug Connection Initialization
*******************************************************************************/
void BHDR_P_HotPlugConnect_isr(BHDR_Handle hHDR)
{

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);

	BHDR_P_InitializePacketRAM_isr(hHDR) ;

	BHDR_DEBUG_P_ResetAllEventCounters_isr(hHDR) ;
}


/******************************************************************************
BHDR_P_HotPlugRemove_isr
Summary: Hot Plug DisConnect  Processing
*******************************************************************************/
void BHDR_P_HotPlugRemove_isr(BHDR_Handle hHDR)
{
	uint32_t Register ;
	uint32_t ulOffset ;
	BREG_Handle hRegister  ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* get Register Handle and offset for Front End */
	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* reset the HDMI MAI format register */
	Register =
		  BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, SAMPLE_WIDTH, 16)
		|BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, AUDIO_FORMAT,
			BHDR_P_MaiAudioFormat_IDLE)
		|BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, SAMPLE_RATE, 0)
		|BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, STREAM_ID, 0)
		|BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, MAI_VERSION, 0) ;
	BREG_Write32_isr(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT + ulOffset, Register) ;

	hHDR->uiConsecutiveSampleRateCalculations = 0 ;

	/* Handle Audio */
	/* settings for recalculation of SR up to BHDR_CONFIG_CONSECUTIVE_SR_CALCS times */
	hHDR->uiConsecutiveSampleRateCalculations = 0 ;

	hHDR->uiPreviousMaiSampleRate = 0 ;
	hHDR->uiPreviousMaiAudioFormat = BHDR_P_MaiAudioFormat_PCM_2CH ;
	hHDR->uiPreviousMaiSampleWidth = 16 ;

	/* clear out all packet RAM */
	BHDR_P_InitializePacketRAM_isr(hHDR) ;

	/* clear all the packet data stored in the BHDR handle; new data will be stored on resumption */
	BHDR_P_ClearPacketMemory_isr(hHDR) ;

	BHDR_P_ClearHdmiMode_isr(hHDR) ;
	BHDR_P_ClearScdcStatus_isr(hHDR) ;

}


BERR_Code BHDR_ConfigureAfterHotPlug(BHDR_Handle hHDR, bool bHotplugStatus)
{
	uint32_t rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);
	if (!hHDR->hFeChannel)
	{
		BDBG_WRN(("No Front End attached to HDMI Rx Core: %x", hHDR->ulOffset )) ;
		return BERR_NOT_INITIALIZED ;
	}

#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
	BDBG_WRN(("CH%d ConfigureAfterHotPlug: %d",
		hHDR->eCoreId, bHotplugStatus)) ;
#endif


	BKNI_EnterCriticalSection() ;

		BHDR_ConfigureAfterHotPlug_isr(hHDR, bHotplugStatus) ;

	BKNI_LeaveCriticalSection() ;

	return rc ;
}


BERR_Code BHDR_ConfigureAfterHotPlug_isr(BHDR_Handle hHDR, bool bHotplugStatus)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
	BDBG_WRN(("ConfigureAfterHotPlug Interrupt Status: HDMI_RX_%d %s",
		hHDR->eCoreId, bHotplugStatus ? "ENABLED" : "DISABLED")) ;
#endif


	/* reset Frame Counter */
	hHDR->FrameCount = 0 ;

	/* reset Symbol Loss Count */
	hHDR->SymbolLossIntrCount = 0 ;

	if (!hHDR->hFeChannel)
	{
		BDBG_WRN(("No Front End attached to HDMI Rx Core: %x", hHDR->ulOffset )) ;
		return BERR_NOT_INITIALIZED ;
	}

	/* remember the previous state */
	hHDR->hFeChannel->bPreviousTxDeviceAttached =
		hHDR->hFeChannel->bTxDeviceAttached ;

	if (hHDR->hFeChannel->settings.bHpdDisconnected)
		hHDR->hFeChannel->bTxDeviceAttached = bHotplugStatus ;

	if (bHotplugStatus)
	{
		BHDR_P_HotPlugConnect_isr(hHDR);
		BHDR_FE_P_EnableInterrupts_isr(hHDR->hFeChannel, true) ;
		BHDR_P_EnableInterrupts_isr(hHDR, true) ;
	}
	else
	{
		BHDR_P_EnableInterrupts_isr(hHDR, false) ;
		BHDR_FE_P_EnableInterrupts_isr(hHDR->hFeChannel, false) ;
		BHDR_P_HotPlugRemove_isr(hHDR);
	}

	return rc ;
}


BERR_Code BHDR_P_ConfigureAudioMaiBus_isr(BHDR_Handle hHDR)
{
	uint8_t uiChannelStatusBits ;
	uint8_t uiSpeakerAllocation ;
	uint16_t uiMaiSampleRate ;
	uint16_t uiMaiAudioFormat ;
	uint16_t uiMaiSampleWidth = 0 ;

	uint8_t uiMaxWordLength  ;
	uint8_t uiWordLength  ;

	uint8_t i ;

	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle);
	ulOffset = hHDR->ulOffset ;
	hRegister = hHDR->hRegister ;

	uiChannelStatusBits = BHDR_P_CSBits_48kHz ; /* Default value */

	BHDR_P_GetAudioSampleFreq_isr(hHDR,  &uiChannelStatusBits) ;

	/*
	Use the calculated Sample Rate (see ProcessVerticalBlankEnd)
	to determine the corresponding channel status bit values
	*/
	i = 0 ;
	while (FsLookupTable[i].MinFreq)
	{
		if (uiChannelStatusBits == FsLookupTable[i].ChannelStatusBits)
		{
			hHDR->AudioSampleRateHz  = FsLookupTable[i].AudioSampleRateHz ;
			break ;
		}
		i++ ;
	}

	/* WARN if frequency not found in any range */
	if (!FsLookupTable[i].MinFreq)
	{
		BDBG_WRN(("CH%d Error getting Channel Status bits for %dHz (no Update)",
			hHDR->eCoreId, hHDR->AudioSampleRateHz)) ;

		/* disable audio since we can't determine a valid SR */
		BHDR_P_EnableAudio_isr(hHDR, false) ;

		return BERR_TIMEOUT;
	}

#if BHDR_CONFIG_DEBUG_AUDIO_FORMAT
	BDBG_LOG(("CH%d Configure MAI bus for Audio Sample Rate %dHz",
		hHDR->eCoreId, hHDR->AudioSampleRateHz)) ;
#endif

	/*
	Use the Channel Status Bits to look up the Sample Rate
	to configure the MAI Audio Bus; default to Not Indicated
	*/
	uiMaiSampleRate = BHDR_P_MaiSampleRate_NotIndicated ;
	i = 0 ;
	while (FsLookupTable[i].MinFreq)
	{
		if (uiChannelStatusBits == FsLookupTable[i].ChannelStatusBits)
		{
			uiMaiSampleRate = FsLookupTable[i].MaiSampleRate ;
			break ;
		}
		i++ ;
	}

	/* If we are receiving HBR packets, Indicate HBR */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_PACKET_TYPE + ulOffset) ;
	if ( BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_PACKET_TYPE, CURRENT_AUDIO_TYPE)
	  == BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_PACKET_TYPE, HBR_AUDIO_PACKET_TYPE) )
	{
		uiMaiAudioFormat = BHDR_P_MaiAudioFormat_HBR ;
	}
	else
	{
		/* get audio format from channel status */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_3_0 + ulOffset) ;
		uiMaiAudioFormat = (Register & 0x2)
			? BHDR_P_MaiAudioFormat_COMPRESSED
			: BHDR_P_MaiAudioFormat_PCM_2CH ;
	}

#if BHDR_CONFIG_DEBUG_AUDIO_FORMAT
	BDBG_LOG(("Configure MAI Bus for Audio Packet Type (%d): %s",
		hHDR->AudioPacketType,
		BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->AudioPacketType))) ;

	BDBG_LOG(("Audio Channel Status Bits [31..0] : %#x ; MAI Format: %#x  ",
		Register, uiMaiAudioFormat)) ;

	BDBG_LOG(("CH%d Configure MAI bus for Audio Format: %s",
		hHDR->eCoreId, uiMaiAudioFormat == BHDR_P_MaiAudioFormat_PCM_2CH
			? "PCM" : "Compressed")) ;
#endif

	/* default speaker allocation is 0x03 */
	uiSpeakerAllocation = 0x03 ;

	if (uiMaiAudioFormat == BHDR_P_MaiAudioFormat_PCM_2CH)
	{
		switch(hHDR->AudioInfoFrame.ChannelCount)
		{
		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e2Channels :
			uiMaiAudioFormat = BHDR_P_MaiAudioFormat_PCM_2CH ;
			uiSpeakerAllocation = 0x03 ;
			break ;

		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e3Channels :
			uiMaiAudioFormat = BHDR_P_MaiAudioFormat_PCM_3CH ;
			uiSpeakerAllocation = 0x07 ;
			break ;

		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e6Channels :
			uiMaiAudioFormat = BHDR_P_MaiAudioFormat_PCM_6CH ;
			uiSpeakerAllocation = 0x3F ;
			break ;

		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e8Channels :
			uiMaiAudioFormat = BHDR_P_MaiAudioFormat_PCM_8CH ;
			uiSpeakerAllocation = 0xFF ;
			break ;

		case BAVC_HDMI_AudioInfoFrame_ChannelCount_eReferToStreamHeader :
			BDBG_MSG(("Audio Infoframe Channel Count set to Refer To Stream Header")) ;
			break ;

		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e4Channels :
		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e5Channels :
		case BAVC_HDMI_AudioInfoFrame_ChannelCount_e7Channels :
		default :
			uiSpeakerAllocation = 0xFF ;
			/* do nothing */
			break ;
		}


		/* If audio is PCM, the sample width will be valid, according to IEC60958-3 Table 2 */
		Register = BREG_Read32_isr(hRegister,
			BCHP_HDMI_RX_0_PACKET_PROCESSOR_CHN_STAT_7_4 + ulOffset) ;

		uiMaxWordLength = Register & 0x1 ;
		uiWordLength = (Register & 0xe) >> 1 ;

		if (uiWordLength > BHDR_AUDIO_SAMPLE_LENGTH_NUMBER)
		{
			BDBG_WRN(("CH%d Unknown/Unsupported audio sample width: %#x",
				hHDR->eCoreId, uiWordLength)) ;
		}
		else
		{
			uiMaiSampleWidth = audioSampleLengthTable[uiMaxWordLength][uiWordLength - 1] ;
		}

		/* display format debug info */
#if BHDR_CONFIG_DEBUG_AUDIO_FORMAT
		BDBG_LOG((" ")) ;
		BDBG_LOG(("PCM Channel Count: %s",
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(
				hHDR->AudioInfoFrame.ChannelCount))) ;

		BDBG_LOG(("PCM Sample Width: %d", uiMaiSampleWidth)) ;
		BDBG_LOG(("Speaker Allocation: %x", uiSpeakerAllocation)) ;
		BDBG_LOG(("AudioInfoFrame Speaker Allocation: %x", uiSpeakerAllocation,
			hHDR->AudioInfoFrame.SpeakerAllocation)) ;
#endif
	}

 	/* update channel mapping */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_CHANNEL_MAP + ulOffset) ;
		Register &= ~(
			  BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_7)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_6)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_5)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_4)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_3)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_2)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_1)
			| BCHP_MASK(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_0)) ;

		if ( uiMaiAudioFormat == BHDR_P_MaiAudioFormat_HBR )
		{
			/* Pass HBR straight through */
			Register |=
			  BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_7, 7)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_6, 6)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_5, 5)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_4, 4)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_3, 3)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_2, 2)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_1, 1)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_0, 0) ;
		}
		else
		{
			/* Reorder PCM to match the FMM channel layout */
			Register |=
			  BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_7, 7)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_6, 6)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_5, 2)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_4, 3)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_3, 5)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_2, 4)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_1, 1)
			| BCHP_FIELD_DATA(HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_0, 0) ;
		}
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_AUDIO_CHANNEL_MAP + ulOffset, Register) ;

#if BHDR_CONFIG_DEBUG_AUDIO_FORMAT
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_AUDIO_CHANNEL_MAP + ulOffset) ;
	BDBG_WRN(("Audio Channel Map: %x %x %x %x %x %x %x %x ",
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_7),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_6),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_5),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_4),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_3),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_2),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_1),
		BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_AUDIO_CHANNEL_MAP, MAP_CHANNEL_0))) ;
#endif

	/* configure the MAI bus */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, SAMPLE_RATE) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT,
			SAMPLE_RATE, (uint32_t) uiMaiSampleRate) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, AUDIO_FORMAT) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT,
			AUDIO_FORMAT, (uint32_t) uiMaiAudioFormat ) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT, SAMPLE_WIDTH) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT,
			SAMPLE_WIDTH, (uint32_t) uiMaiSampleWidth ) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_MAI_FORMAT + ulOffset, Register) ;

	/* set the active speaker/channels	*/
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG, SPEAKER_ACTIVE) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_0_PACKET_PROCESSOR_CFG,
			SPEAKER_ACTIVE, (uint32_t) uiSpeakerAllocation) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;

	/* every thing is valid enable audio */
	BHDR_P_EnableAudio_isr(hHDR, !hHDR->AvMute) ;

	return BERR_SUCCESS ;
}


void BHDR_P_EnableInterrupts_isr(BHDR_Handle hHDR, bool enable)
{
	BERR_Code rc ;
	const BHDR_P_InterruptCbTable *pInterrupts ;
	uint8_t i ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	if (hHDR->eCoreId == BHDR_P_eHdrCoreId0)
		pInterrupts = BHDR_Interrupts0 ;
#if BHDR_HAS_MULTIPLE_PORTS
	else if (hHDR->eCoreId == BHDR_P_eHdrCoreId1)
		pInterrupts = BHDR_Interrupts1 ;
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDR->eCoreId)) ;
		pInterrupts = BHDR_Interrupts0 ;
	}

	/* Register/enable interrupt callbacks */
	for (i = 0; i < MAKE_INTR_ENUM(LAST) ; i++)
	{
		/* clear interrupt callback */
		rc = BINT_ClearCallback_isr( hHDR->hCallback[i]) ;
		if (rc) {BDBG_ASSERT(false) ;}


		/* skip interrupt if not enabled in table or slaved core */
		if ((!pInterrupts[i].enable) /*|| (hHDR->eUsage == BHDR_Usage_eSlave)*/)
		{
			continue ;
		}

		if (enable)
			rc = BINT_EnableCallback_isr( hHDR->hCallback[i] ) ;
		else
			rc = BINT_DisableCallback_isr( hHDR->hCallback[i] ) ;
		if (rc) {BDBG_ASSERT(false) ;}
	}
}


static BERR_Code BHDR_P_CreateTimer(BHDR_Handle hHDR, BTMR_TimerHandle * timerHandle, uint8_t timerId)
{
	BERR_Code rc ;
	BTMR_Settings timerSettings  ;

	BDBG_ENTER(BHDR_P_CreateTimer) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BDBG_MSG(("Create BHDR_P_TIMER_eNNN ID %d", timerId)) ;

	/* create OTP Calculation Check expiration timer */
	BTMR_GetDefaultTimerSettings(&timerSettings) ;
		timerSettings.type =  BTMR_Type_eCountDown ;
		timerSettings.cb_isr = (BTMR_CallbackFunc) BHDR_P_TimerExpiration_isr ;
		timerSettings.pParm1 = hHDR ;
		timerSettings.parm2 = timerId ;
		timerSettings.exclusive = false ;
	rc = BTMR_CreateTimer(hHDR->hTimerDevice, timerHandle, &timerSettings) ;

	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
	}

	BDBG_LEAVE(BHDR_P_CreateTimer) ;
	return rc ;
}


static BERR_Code BHDR_P_DestroyTimer(BHDR_Handle hHDR, BTMR_TimerHandle timerHandle, uint8_t timerId)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_P_DestroyTimer) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BDBG_MSG(("Destroy BHDR_P_TIMER_eNNN ID %d", timerId)) ;
	rc = BTMR_DestroyTimer(timerHandle) ;

        BSTD_UNUSED(timerId);
	BDBG_LEAVE(BHDR_P_DestroyTimer) ;
	return rc ;
}


static void BHDR_P_TimerExpiration_isr (BHDR_Handle hHDR, int parm2)
{
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;
	switch (parm2)
	{
	case BHDR_P_TIMER_eDviHdmiModeChange :
#if BHDR_CONFIG_DEBUG_MODE_CHANGE
		BDBG_WRN(("DVI/HDMI Mode Change timer expired....")) ;
#endif
		BHDR_P_ProcessModeChange_isr(hHDR) ;

		break ;

	default :
		BDBG_ERR(("hHDR %p Timer %d not handled", (void *)hHDR, parm2)) ;
	}
}
