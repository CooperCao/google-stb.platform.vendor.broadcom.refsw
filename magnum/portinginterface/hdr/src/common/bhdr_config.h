/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef BHDR_CONFIG_H__
#define BHDR_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "bchp.h"
#include "bchp_common.h"
#include "bchp_hdmi_rx_0.h"

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



/* Enable BHDR_CONFIG_ISRC_PACKET_SUPPORT option
to support processing of GAMUT Metadata packets

Gamut Packet Processing is not recommended
*/
#define BHDR_CONFIG_GAMUT_PACKET_SUPPORT 0
#define BHDR_CONFIG_DEBUG_GAMUT_PACKET 0


/*
Enable BHDR_CONFIG_ISRC_PACKET_SUPPORT option
to support processing of ISRC packets.

NOTE:  No commercial devices have been found that support ISRC packets
therefore this option has not been adequately tested
*/
#define BHDR_CONFIG_ISRC_PACKET_SUPPORT 0


/* HDMI Packet Debug Options  */
#define BHDR_CONFIG_DEBUG_INFO_PACKET_AVI       0
#define BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO   0
#define BHDR_CONFIG_DEBUG_INFO_PACKET_SPD       0
#define BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC     0
#define BHDR_CONFIG_DEBUG_ACR_PACKET 0
#define BHDR_CONFIG_DEBUG_CHANNEL_STATUS 0

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

#undef BHDR_CONFIG_DEBUG_INFO_PACKET_AVI
#undef BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO
#undef BHDR_CONFIG_DEBUG_INFO_PACKET_SPD
#undef BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC
#define BHDR_CONFIG_DEBUG_INFO_PACKET_AVI        1
#define BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO   1
#define BHDR_CONFIG_DEBUG_INFO_PACKET_SPD       1
#define BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC     1

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

#if ((BCHP_CHIP == 3548) || (BCHP_CHIP == 3556))
#define HDMI_RX_GEN 3548

#elif (BCHP_CHIP == 7422)  || (BCHP_CHIP == 7425)  \
   || (BCHP_CHIP == 7640)  || (BCHP_CHIP == 7429)  \
   || (BCHP_CHIP == 7435)  || (BCHP_CHIP == 74295)
#define HDMI_RX_GEN 7422

#elif (BCHP_CHIP == 7445) || (BCHP_CHIP == 7145) \
	|| (BCHP_CHIP == 7439) || (BCHP_CHIP == 74371)|| (BCHP_CHIP == 7251)
#define HDMI_RX_GEN 7445

#elif BCHP_HDCP2_RX_0_REG_START

#define HDMI_RX_GEN 7445

#else
#error UNKNOWN_CHIP
#endif

#ifdef BCHP_HDMI_RX_0_HDCP_RX_I2C_MISC_CFG_2_I2C_SCDC_ENABLE_MASK
#define BHDR_HAS_HDMI_20_SUPPORT 1
#endif

/*********** Basic HDMI Rx Configuration ***********/
#if HDMI_RX_GEN == 7422

#define BHDR_FE_MAX_CHANNELS 1
#define BHDR_HAS_MULTIPLE_PORTS 0
#define BHDR_CONFIG_RESET_HDCP_ON_SYMBOL_LOCK	1
#define BHDR_CONFIG_UPDATE_MAI_ON_VSYNC 1

#define BHDR_CONFIG_REPEATER 1

#if ((BCHP_CHIP == 7425) || (BCHP__CHIP == 7422) || (BCHP_CHIP == 7640)) \
&& (BCHP_VER < BCHP_VER_B0)

#define BHDR_FE_HP_LEGACY_SUPPORT 1

#endif


#if ((BCHP_CHIP == 7425)  && (BCHP_VER >= BCHP_VER_B0)) \
  || (BCHP_CHIP == 7429)  || (BCHP_CHIP == 7435) \
  || (BCHP_CHIP == 74295)
#define BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT 1
#define BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT 1
#endif


#if (BCHP_CHIP == 7435) || (BCHP_CHIP == 74295) \
|| ((BCHP_CHIP == 7429) && (BCHP_VER >= BCHP_VER_B0))
#define BHDR_CONFIG_DUAL_HPD_SUPPORT 1
#endif


#define BHDR_CONFIG_HBR_SUPPORT 1

#elif HDMI_RX_GEN == 7445

#define BHDR_FE_MAX_CHANNELS 1
#define BHDR_HAS_MULTIPLE_PORTS 0
#define BHDR_CONFIG_RESET_HDCP_ON_SYMBOL_LOCK	1
#define BHDR_CONFIG_UPDATE_MAI_ON_VSYNC 1

#define BHDR_CONFIG_REPEATER 1

#define BHDR_CONFIG_DUAL_HPD_SUPPORT 1

#define BHDR_CONFIG_FE_MULTI_CLOCK_SUPPORT 1

#if ((BCHP_CHIP == 7445) && (BCHP_VER < BCHP_VER_C0))
#define BHDR_CONFIG_RDB_MAPPING_WORKAROUND 1
#endif


#ifdef BCHP_HDCP2_RX_HAE_INTR2_0_REG_START
#define BHDR_CONFIG_HDCP2X_SUPPORT 1

#if (BCHP_CHIP == 7445) \
|| ((BCHP_CHIP == 7250)  && (BCHP_VER <= BCHP_VER_B0)) \
|| ((BCHP_CHIP == 7364)  && (BCHP_VER <= BCHP_VER_A0))
#define BHDR_CONFIG_DISABLE_KEY_RAM_SERIAL 1
#endif

#endif

#else
#error Unknown/Unsupported chip
#endif


#ifdef __cplusplus
}
#endif



#endif
/* end bhdr_config.h */

