/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BHDR_CONFIG_H__
#define BHDR_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "bchp.h"
#include "bchp_common.h"
#include "bchp_hdmi_rx_0.h"

#include "bchp_dvp_hr.h"
#include "bchp_dvp_hr_intr2.h"

/*
The Audio Clock Recovery Packet (ACRP) can be used to calculate the audio
sample rate using N and CTS.  While this method can be used to determine
the rate, the preferred method is to use the Channel Status bits extracted
directly from the stream by the hardware.

Setting the define
	BHDR_CONFIG_USE_ACRP_EVERY_N_VSYNCS

will use the calculation. A nominal setting of 3 can be used.  The nominal setting
can also be used to debug the contents of the ACR Packet.   The N, CTS values
will print at each sample rate change.

If the value is set to zero (default), the preferred method of extracting channel
status bits directly from the stream will be used.
*/
#if ((BCHP_CHIP == 3548) || (BCHP_CHIP == 3556))
#define BHDR_CONFIG_USE_ACRP_EVERY_N_VSYNCS 3
#endif
#define BHDR_CONFIG_CONSECUTIVE_SR_CALCS 3


/*
Enable BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S option
if a source device does not properly send Clear_AVMUTE after sending Set_AVMUTE

If set > 0, AvMUTE will automatically clear after N seconds
If set = 0. the Auto Clear AvMute feature will be disabled
NOTE:  Auto Clear AvMute may be a requirement for Simplay Certification
*/
#define BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S 0


/*
Enable BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY option
to have the HDMI Core immediately disable sending audio packets to the audio core.

the alternative is to have the app handle audio and video muting simultaneously
*/
#define BHDR_CONFIG_AVMUTE_AUDIO_IMMEDIATELY 1

/*
Enable BHDR_CONFIG_DEBUG_DISABLE_AVMUTE_CB option
to disable AVMUTE Callback... can be used to check output display
of the screen when AvMute is not processed
*/
#define BHDR_CONFIG_DEBUG_DISABLE_AVMUTE_CB 0


/* HOT PLUG DETECT SIGNAL */

/*
** Set  BHDR_CONFIG_HPD_DISCONNECTED  to 1
** ONLY if board does not route the HPD signal from Pin 19 of the HDMI Connector directly
** to the HPD In of HDMI Rx
**
** Broadcom SV boards usually have this signal connected (default 0)
** Broadcom CARD boards may not have the signal connected.  If not connected set to 1
*/
#define BHDR_CONFIG_HPD_DISCONNECTED 0



/* BHDR_CONFIG_DELAY_MODE_CHANGE_MS
    Delay processing of switch between HDMI and DVI mode
    multiple mode interrupts may occur during transitions;
    delay re-configuration until mode is stable for the specified time
    set to 0 to handle MODE switch immediately
*/
#define BHDR_CONFIG_DELAY_MODE_CHANGE_MS 100
#define BHDR_CONFIG_DEBUG_MODE_CHANGE 0



/* Enable DEBUG_TIMER_S  to fire BHDR_P_DebugMonitorHdmiRx_isr
every N seconds.  function can be modified to read registers etc.
DEBUG ONLY!!
*/
#define BHDR_CONFIG_DEBUG_TIMER_S 0

/* available debug options; enables BDBG_MSGs etc.  */
#define BHDR_CONFIG_DEBUG_I2C 0



/* Enable BHDR_CONFIG_GAMUT_PACKET_SUPPORT option
to support processing of GAMUT Metadata packets

Gamut Packet Processing is not recommended
*/
#define BHDR_CONFIG_GAMUT_PACKET_SUPPORT 0
#define BHDR_CONFIG_DEBUG_PACKET_GAMUT 0


/*
Enable BHDR_CONFIG_ISRC_PACKET_SUPPORT option
to support processing of ISRC packets.

NOTE:  No commercial devices have been found that support ISRC packets
therefore this option has not been adequately tested
*/
#define BHDR_CONFIG_ISRC_PACKET_SUPPORT 0


/* HDMI Rx Debug Options  */
#define BHDR_CONFIG_DEBUG_CHANNEL_STATUS 0

#define BHDR_CONFIG_DEBUG_PACKET_AVI     0
#define BHDR_CONFIG_DEBUG_PACKET_AUDIO   0
#define BHDR_CONFIG_DEBUG_PACKET_SPD     0
#define BHDR_CONFIG_DEBUG_PACKET_VSI     0
#define BHDR_CONFIG_DEBUG_PACKET_ACR     0
#define BHDR_CONFIG_DEBUG_PACKET_DRM     0


/* HDMI General Control Packet Debug Options  */
#define BHDR_CONFIG_DEBUG_GCP_DC 0
#define BHDR_CONFIG_DEBUG_GCP_AV_MUTE 0

/* HDMI Audio Format change from Normal to High bitrate audio */
#define BHDR_CONFIG_DEBUG_AUDIO_PACKET_CHANGE 0

/* HDMI Audio FORMAT from Channel Status Bits */
#define BHDR_CONFIG_DEBUG_AUDIO_FORMAT 0

/* HDCP Debug Options */
#define BHDR_CONFIG_DEBUG_HDCP_VALUES 0
#define BHDR_CONFIG_DEBUG_HDCP_KEY_LOADING 0

/* debug option to track RI updating; dependent upon working  */
#define BHDR_CONFIG_DEBUG_HDCP_RI_UPDATES 0

/* debug option to generate eye diagram; always built with kylin */
#if KYLIN
#define BHDR_CONFIG_DEBUG_EYE_DIAGRAM 1
#endif

#define BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY 0
#define BHDR_CONFIG_DEBUG_DETECTED_FORMAT_DETAIL 0

/* debug optiion to verify EDID ram written correctly */
#define BHDR_CONFIG_DEBUG_EDID_RAM 0

#define BHDR_CONFIG_DEBUG_STREAM_ERRORS 0
#define BHDR_CONFIG_DEBUG_AUDIO_SR 0

/* debug option to display configuration of HP, Equalizer, etc. */
#define BHDR_CONFIG_DEBUG_DISPLAY_HDMI_RX_CONFIG 0
#define BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG 0
#define BHDR_CONFIG_DEBUG_INPUT_SIGNAL 0
#define BHDR_CONFIG_DEBUG_FRONT_END 0

/* debug option to display HDMI Rx power management configuration */
#define BHDR_CONFIG_DEBUG_HDR_PWR 0

#define BHDR_CONFIG_DEBUG_OLD_TIMER_DEPRACATED 0


/******************************************************************/
/* Plugfest Configuration START                                      */
/******************************************************************/
#define BHDR_CONFIG_PLUGFEST_TEST 0

#if BHDR_CONFIG_PLUGFEST_TEST

#undef BHDR_CONFIG_DEBUG_HDCP_RI_UPDATES
#define BHDR_CONFIG_DEBUG_HDCP_RI_UPDATES 1

#undef BHDR_CONFIG_DEBUG_INPUT_SIGNAL
#undef BHDR_CONFIG_DEBUG_FRONT_END
#define BHDR_CONFIG_DEBUG_INPUT_SIGNAL 1
#define BHDR_CONFIG_DEBUG_FRONT_END 1

#undef BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY
#define BHDR_CONFIG_DEBUG_DETECTED_FORMAT_SUMMARY 1


#undef BHDR_CONFIG_DEBUG_AUDIO_SR
#define BHDR_CONFIG_DEBUG_AUDIO_SR 1

#endif




/******************************************************************/
/******************************************************************/
/******************************************************************/
/* chip revision specific configuration - DO NOT MODIFY ANY BELOW */
/******************************************************************/
/******************************************************************/
/******************************************************************/

/***********************************/
/* Dual Hotplug Interrupts Support */
/***********************************/
#ifdef  BCHP_DVP_HR_INTR2_CPU_STATUS_HP_CONNECTED_0_MASK
#define BHDR_CONFIG_DUAL_HPD_SUPPORT 1
#endif

/***********************************/
/* HDMI 2.0 Support */
/***********************************/
#ifdef BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2_I2C_SCDC_ENABLE_MASK
#define BHDR_HAS_HDMI_20_SUPPORT 1

#ifdef BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2_SCDC_OFFSET_INC_MODE_MASK
#define BHDR_CONFIG_MANUAL_SCDC_CLEAR 1
#endif

#endif

/***********************************/
/* HDCP 2.2 Support */
/***********************************/
#ifdef BCHP_HDCP2_RX_HAE_INTR2_0_REG_START
#define BHDR_CONFIG_HDCP2X_SUPPORT 1

#endif

#ifdef BCHP_DVP_HR_FREQ_MEASURE_CONTROL
#define BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT 1
#endif

#define BHDR_FE_MAX_CHANNELS 1
#define BHDR_HAS_MULTIPLE_PORTS 0
#define BHDR_CONFIG_RESET_HDCP_ON_SYMBOL_LOCK	1
#define BHDR_CONFIG_UPDATE_MAI_ON_VSYNC 1


/*********** Legacy HDMI Rx Configuration ***********/
#if ((BCHP_CHIP == 7425) || (BCHP__CHIP == 7422) || (BCHP_CHIP == 7640)) \
&& (BCHP_VER < BCHP_VER_B0)
#define BHDR_FE_HP_LEGACY_SUPPORT 1
#endif

#if BCHP_DVP_HR_PTHRU_CFG
#define BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT 1
#endif


#ifdef __cplusplus
}
#endif



#endif
/* end bhdr_config.h */

