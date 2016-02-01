/***************************************************************************
 *     (c)2004-2010 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * 
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
 ************************************************************/

/****************************************************************************
 *
 *  UAGC_Generic.c - UAGC Generic Initializations for PYRO / ARGO
 *
 *   Theres are 2 sets of tables in this file.  Both set have register
 *    settings which don't depend on the analog board components of
 *    the RF front end.
 *
 *   1. Generic_X244_xxxxUagcData -  UAGC registers settings which
 *    should be reset when the demodulator modulation type has changed.
 *
 *   2. Generic_X244_xxxxUagcDataRest -  UAGC registers settings which
 *    should be reset each time a demodulator is reset, i.e. the demodulator
 *    modulation type has not changed from the last time the demodulator was 
 *    started and stopped.  This is being done to speed up the AFT
 *    for the IF demodulator, since it involves several restarts of the
 *    demodulator.
 *
 * $Id: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 * $Change: 2672 $
 * $LastChangedRevision: $
 *****************************************************************************/

#include "btfe_scd_priv.h"

#include "bchp_dfe_agcdec_8051.h"
#include "bchp_dfe_bertdec_8051.h"
#include "bchp_dfe_eqdec_8051.h"
#include "bchp_dfe_fecdec_8051.h"
#include "bchp_dfe_fedec_8051.h"
#include "bchp_dfe_mfdec_8051.h"
#include "bchp_dfe_miscdec_8051.h"
#include "bchp_dfe_ntscdec_8051.h"
#include "bchp_dfe_ofsdec_8051.h"
#include "bchp_dfe_ucdec_8051.h"

#include "btfe_scd_reg35230_hi_priv.h"

#include "btfe_scd_35233_priv.h"
#include "btfe_scd_uagc_generic_priv.h"

/****************************************************************************/
/* UAGC register initializations common for all modulation types */

static uint32_t UagcConfig_General[] =
{
		/* Vmin(t) =  0.00  Vdd = 3.3 */
		/* Fixed Point Format:  U[-1:-8] 0.996094 = 0xff */

		ixUAGC_VRF_MAX, 0xff,

		/* Vmax(t) =  0.00  Vdd = 3.3 */
		/* Fixed Point Format:  U[-1:-8] 0.996094 = 0xff */

		ixUAGC_VRF_MIN, 0xff,
	
		/* small offset to avoid stron spurs in delta-sigma output */

		ixUAGC_V_OFFSET, 0x03,


        /* init fixed voltage to mid range. */
		ixUAGC_VIF_FIX, 0x80,

        /* -----------------------------------------------------------------*/
        /* Initialize A/D min/max regs */
        /* -----------------------------------------------------------------*/
		ixUAGC_ADC_DATA_MAX, 0x00,
		ixUAGC_ADC_DATA_MIN, 0x00,

};

static uint32_t UagcConfig_General_Length = sizeof (UagcConfig_General);

/****************************************************************************/
/* UAGC register initializations common for all analog modulation types */

static uint32_t UagcConfig_Analog[] =
{
        /* -----------------------------------------------------------------*/
		/* NTSC tuner frequency response and DAC gain */
		/* Video DAC out (CVBS) gain */
		/* Fixed amplifier after BE-AGC */
		/* Fixed Point Format:   [0:-7] 1.414 = 0xb4 */
        /* -----------------------------------------------------------------*/
		ixHI_ANALOG_FREQ_RESPONSE,	0x00,
		ixHI_ANALOG_DAC_GAIN,		0x00,
		ixHI_IFD_FILTER_SELECT,     0x00,   /* 0=Sharp  1=Low ringing */

        /* -----------------------------------------------------------------
			Luma LPF coefficients     
			  from NTSC_front_AGC_design.m

				% April, 2009 mcg:  Audio affects UAGC sync tip SNR estimate.  
				%  Increase rejection.
				Fsm = 13.525e6, Ntaps = 31
				b_luma=fir2(Ntaps-1,[0 100e3 1.5e6 Fsm]/(Fsm), [1 1 0 0]);

				Gw = 1.3; % By inspection, gain needed so that power of A/D
	                      %  samples equals baseband luma power (thru downconverter)
				          %  during sync tip interval
				b_luma = Gw * b_luma / sum(b_luma);


				b_luma_LSB = -11;
				b_luma=2^b_luma_LSB*round(b_luma*2^-b_luma_LSB);

        --------------------------------------------------------------------*/
        ixUAGC_LUMA_DATA_1, 0x00,  /* c0 =  0.000488 */
        ixUAGC_LUMA_DATA_0, 0x01,
        ixUAGC_LUMA_ADDRESS, 0x10,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c1 =  0.000977 */
        ixUAGC_LUMA_DATA_0, 0x02,
        ixUAGC_LUMA_ADDRESS, 0x11,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c2 =  0.001953 */
        ixUAGC_LUMA_DATA_0, 0x04,
        ixUAGC_LUMA_ADDRESS, 0x12,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c3 =  0.003418 */
        ixUAGC_LUMA_DATA_0, 0x07,
        ixUAGC_LUMA_ADDRESS, 0x13,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c4 =  0.006836 */
        ixUAGC_LUMA_DATA_0, 0x0E,
        ixUAGC_LUMA_ADDRESS, 0x14,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c5 =  0.011230 */
        ixUAGC_LUMA_DATA_0, 0x17,
        ixUAGC_LUMA_ADDRESS, 0x15,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c6 =  0.018555 */
        ixUAGC_LUMA_DATA_0, 0x26,
        ixUAGC_LUMA_ADDRESS, 0x16,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c7 =  0.027344 */
        ixUAGC_LUMA_DATA_0, 0x38,
        ixUAGC_LUMA_ADDRESS, 0x17,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c8 =  0.038574 */
        ixUAGC_LUMA_DATA_0, 0x4F,
        ixUAGC_LUMA_ADDRESS, 0x18,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c9 =  0.051270 */
        ixUAGC_LUMA_DATA_0, 0x69,
        ixUAGC_LUMA_ADDRESS, 0x19,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c10 =  0.064453 */
        ixUAGC_LUMA_DATA_0, 0x84,
        ixUAGC_LUMA_ADDRESS, 0x1A,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c11 =  0.077637 */
        ixUAGC_LUMA_DATA_0, 0x9F,
        ixUAGC_LUMA_ADDRESS, 0x1B,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c12 =  0.089844 */
        ixUAGC_LUMA_DATA_0, 0xB8,
        ixUAGC_LUMA_ADDRESS, 0x1C,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c13 =  0.099121 */
        ixUAGC_LUMA_DATA_0, 0xCB,
        ixUAGC_LUMA_ADDRESS, 0x1D,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c14 =  0.104980 */
        ixUAGC_LUMA_DATA_0, 0xD7,
        ixUAGC_LUMA_ADDRESS, 0x1E,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c15 =  0.107422  program reg with c15/2 */
        ixUAGC_LUMA_DATA_0, 0x6E,
        ixUAGC_LUMA_ADDRESS, 0x1F,
};

static uint32_t UagcConfig_Analog_Length = sizeof (UagcConfig_Analog);

/****************************************************************************/
/* UAGC register initializations depending on analog modulation polarity */

static uint32_t UagcConfig_Analog_NegativeModulation[] =
{
        /* --------------------------------------------------------------------------
		*   Default delay before starting demodulator.  The reason for the delay
		*    is unclear.  It is necessary to wait for the tuner PLLs to settle,
		*    and for the RF/IF AGC to settle.
		*/
        ixHI_AFE_DELAY_MSEC, 20,

        /* -----------------------------------------------------------------*/
        /* Setpoints                                                        */
        /* -----------------------------------------------------------------*/
		ixUAGC_POWER_SETPOINT_1, 0x00,
		ixUAGC_POWER_SETPOINT_0, 0x00,

        ixUAGC_CLAMP_SETPOINT_1, 0x00,
        ixUAGC_CLAMP_SETPOINT_0, 0x00,

        /* -----------------------------------------------------------------*/
        /* Control Bits */
        /* -----------------------------------------------------------------*/
		/* ixUAGC_CONTROL_1, 0x03, */
		ixUAGC_CONTROL_1 ,0x23,      /* Freeze until ready to go */
		ixUAGC_CONTROL_2, 0x00,
		ixUAGC_RESET_CONTROL, 0x00,

        /* -----------------------------------------------------------------*/
        /* Level Detector Filter BWs                                        */
        /* -----------------------------------------------------------------*/
		ixUAGC_LPF_COEF_CHAN, 0x00,
		ixUAGC_LPF_COEF_AD, 0x44,             /* 530 KHz 3 dB BW */

        /* -----------------------------------------------------------------*/
        /* RF VGA control - based only on 'C' simulation   */
        /* -----------------------------------------------------------------*/
		ixUAGC_VRF_TOP_1, 0x43,
		ixUAGC_VRF_TOP_0, 0x6E,

        /* -----------------------------------------------------------------*/
        /* Lock Detector Controls  */
        /* -----------------------------------------------------------------*/
		ixUAGC_LOCK_DET_COEF, 0xA8,
		ixUAGC_U1_INIT, 0x08,
		ixUAGC_U1_ACQ, 0x0C,
		ixUAGC_U1_DROP, 0x18,
		ixUAGC_U2_INIT, 0x00,
		ixUAGC_U2_ACQ, 0x03,
		ixUAGC_U2_DROP, 0x06,
		ixUAGC_URF_ACQ, 0x08,
		ixUAGC_URF_DROP, 0x10,
		ixUAGC_U2_SATURATION, 0x06,
		ixUAGC_EK_SATURATION_1, 0x00,
		ixUAGC_EK_SATURATION_0, 0x0F,

        /* -----------------------------------------------------------------*/
        /* SNR Estimate filters */
        /* -----------------------------------------------------------------*/
		ixUAGC_ERROR_LPF_COEF, 0x04,
		ixUAGC_SNR_LPF_COEF, 0xD9,

        /* -----------------------------------------------------------------*/
        /* Genlock Bandwidth/Signal Level/ Control Initializations          */
		/*  Negative Modulated Systems                                      */
        /* -----------------------------------------------------------------*/
        /* UAGC_BLACK_CONTROL    [R/W]    8 bits    Access: 8    primaryRegisterAperture:0xf1a3  
        o_irf_posModulation     0 0x0 1: positive modulation (SECAM) 0: negative modulation (NTSC/PAL)
        o_irf_LoadClampEnable   1 0x1 1: load clamp level from VBI black level until lock
        o_irf_ReLoadClampEnable 2 0x0 1: load clamp level from every VBI black level after lock
        o_irf_enable_phih_init  3 0x0 1: enable field to field phase error tracking
        o_irf_clear_phih_init   4 0x1 1: hold phih_init(v) at 0.0
        o_irf_use_clamp_fix     5 0x0 1: c(i) = UAGC_CLAMP_FIX when USE_CLAMP_FIX bit is set.
        o_irf_freeze_clamp      6 0x0 1: c(i) = c(i-1). */

        ixUAGC_BLACK_CONTROL, 0x0A,   /* 0x0b = 000_1010 Negative Modulation */

		/* Vertical Sync Filtering BW control */
		ixUAGC_VSPI_COEF, 0x0A,
		ixUAGC_VSYNC_COEF, 0x09,

		/* Trick mode detector threshold */
		ixUAGC_S_TRICK_THRESH, 0x05,


		/* Offset from horizontal (line) PLL lock point to phase position
		    for sampling the back porch signal level.
		    phiH(m)  phase of line  units are bin_rads [-0.5, 0.5).
		    phiH(0)  center of sync tip
		    phiH_BP  phase of back porch  units are bin_rads  S+[-2:-8]
 		    irf_BACK_PORCH_PHASE = 0 sec * fH (lines/sec) * (1 bin_rad/line)
			                     = 0 * 15625
								 = 0h  Set to 01 because of bug in HW comparitor
		 */
		ixUAGC_BACK_PORCH_PHASE, 0x01,  

		ixUAGC_CLAMP_GAIN, 0x45,
		ixUAGC_CLAMP_FIX_1, 0x20,
		ixUAGC_CLAMP_FIX_0, 0x66,

		ixUAGC_K1_FAST, 0x30,
		ixUAGC_K1_SLOW, 0x31,
		ixUAGC_K2_FAST, 0x13,
		ixUAGC_K2_SLOW, 0x15,

        ixUAGC_PHIH_INIT_GAIN, 0x02,      /* Integrator Gain Gi */
        ixUAGC_PHASE_HIT_THRESHOLD, 0x14, /* phaseHitThreshold_sec*fH, where phaseHitThreshold_sec = 5e-6, fH=15734  */
        ixUAGC_H_LOCK_LPF_COEF, 0x84,

		ixUAGC_UI_THRESH, 0xFF,

		
		ixUAGC_UH_THRESH_ACQ, 0x20,       /* dec2hex(floor(2^8 * 0.125),2)  = 20h U[-1:-8] */
		ixUAGC_UH_THRESH_DROP, 0x10,      /* dec2hex(floor(2^8 * 0.0625),2) = 10h U[-1:-8] */
		ixUAGC_UH_THRESH_HIGH_SNR, 0x21,  /* dec2hex(floor(2^8 * 0.130),2)  = 21h U[-1:-8] */
		ixUAGC_GAIN_LINE_LOCK, 0x04,      /* GnH = 2^4 = 16 */
		ixUAGC_THRESHOLD_LINE_LOCK, 0x05, /* dec2hex(floor(2^9 * 0.01),2)   = 05h U[-2:-9] */

        /* -----------------------------------------------------------------*/
        /* UAGC Test Mux */
        /* -----------------------------------------------------------------*/
		ixUAGC_TMUX_CHAN1_SELECT, 0x02,
		ixUAGC_TMUX_CHAN2_SELECT, 0x03,
		ixUAGC_TMUX_BIT_SELECT1, 0x00,
		ixUAGC_TMUX_BIT_SELECT2, 0x00,
		ixUAGC_TMUX_TRIG_SELECT, 0x02,

        /*----------- IF VGA Setpoint Backoff from Min Gain -------------
        02-Feb-2010
        Vif setpoint backoff:           0.0500 V
        Vif setpoint backoff:           0.0151 V/Vdd
        Vdd:                             3.300 V
        */
        ixHI_UAGC_VIF_SETPOINT_OFFSET4_1, 0x00, /*  0.015  U[2:-12] */
        ixHI_UAGC_VIF_SETPOINT_OFFSET4_0, 0x3e, 

		/* BEAGC Gain Setpoint is  -8.1 dB =  -2.7 * Kdet, Kdet = 0.332 , (Sets CVBS output level) */
		/* Fixed Point Format: S+[4:-10] -2.69076 = 0xf53d */
		/* SET_2_REG(UC_GP_57, 0xf53d); */
		ixHI_BEAGC_GAIN_NOMINAL_1,  0xf5,
		ixHI_BEAGC_GAIN_NOMINAL_0,  0x3d,


}; /* end UagcConfig_Analog_NegativeModulation */


static uint32_t UagcConfig_Analog_NegativeModulation_Length = sizeof (UagcConfig_Analog_NegativeModulation);


static uint32_t UagcConfig_Analog_PositiveModulation[] =
{
        /* --------------------------------------------------------------------------
		*   Default delay before starting demodulator.  The reason for the delay
		*    is unclear.  It is necessary to wait for the tuner PLLs to settle,
		*    and for the RF/IF AGC to settle.
		*/
        ixHI_AFE_DELAY_MSEC, 200,

		/* -----------------------------------------------------------------*/
        /* Setpoints                                                        */
        /* -----------------------------------------------------------------*/
		ixUAGC_POWER_SETPOINT_1, 0x00,
		ixUAGC_POWER_SETPOINT_0, 0x00,

		/* The back porch level is 10.4 dB below Peak White  */
		/*        20*log10(43IRE/143IRE) = -10.4 */

		/* UAGC IF AGC Keyed AGC Setpoint - 20*log10(143IRE/43IRE) */
		/* UAGC Keyed AGC Setpoint:  -10.4 dBFS = -3.45 dBbFS */
		/* Fixed Point Format: S+[4:-6] -3.45481 = 0xf23 */

		ixUAGC_CLAMP_SETPOINT_1, 0x0f,
		ixUAGC_CLAMP_SETPOINT_0, 0x23,

		/* Tuner driver code generated by ifdemod_set_cvbs_level.pl on Fri Sep 24 17:48:40 2010 */
		/*                                                                                      */
		/* BEAGC Takeover point is  -6.0 dBFS = -1.993 dBbFS * dBb2dB, dBb2dB = 3.010 , (Sets CVBS output level) */
		/* Fixed Point Format: S+[4:-10] -1.99316 = 0xf808 */

		ixHI_BEAGC_GAIN_NOMINAL_1, 0xf8,
		ixHI_BEAGC_GAIN_NOMINAL_0, 0x08,

        /* -----------------------------------------------------------------*/
        /* Control Bits */
        /* -----------------------------------------------------------------*/
		/* ixUAGC_CONTROL_1 ,0x03,*/
		ixUAGC_CONTROL_1 ,0x23,      /* Freeze until ready to go */
        ixUAGC_CONTROL_2, 0x30,
		ixUAGC_RESET_CONTROL, 0x00,

        /* -----------------------------------------------------------------*/
        /* Level Detector Filter BWs                                        */
        /* -----------------------------------------------------------------*/
        ixUAGC_LPF_COEF_CHAN, 0x44,
        ixUAGC_LPF_COEF_AD, 0xAA,        /* Ah -> FdB = 8.4 KHz  This is the slowest */  

        /* -----------------------------------------------------------------*/
        /* RF VGA control - based only on 'C' simulation   */
        /* -----------------------------------------------------------------*/
        ixUAGC_VRF_TOP_1, 0x43,
        ixUAGC_VRF_TOP_0, 0x6E,

        /* -----------------------------------------------------------------*/
        /* Lock Detector Controls  */
        /* -----------------------------------------------------------------*/
		ixUAGC_LOCK_DET_COEF, 0xA8,
		ixUAGC_U1_INIT, 0x08,
		ixUAGC_U1_ACQ, 0x0C,
		ixUAGC_U1_DROP, 0x18,
		ixUAGC_U2_INIT, 0x00,
		ixUAGC_U2_ACQ, 0x03,
		ixUAGC_U2_DROP, 0x06,
		ixUAGC_URF_ACQ, 0x08,
		ixUAGC_URF_DROP, 0x10,
		ixUAGC_U2_SATURATION, 0x06,
		ixUAGC_EK_SATURATION_1, 0x00,
		ixUAGC_EK_SATURATION_0, 0x0F,

        /* -----------------------------------------------------------------*/
        /* SNR Estimate filters */
        /* -----------------------------------------------------------------*/
		ixUAGC_ERROR_LPF_COEF, 0x04,
		ixUAGC_SNR_LPF_COEF, 0xD9,

        /* -----------------------------------------------------------------*/
        /* Genlock Bandwidth/Signal Level/ Control Initializations          */
		/*  Positive Modulated Systems                                      */
        /* -----------------------------------------------------------------*/
        /* UAGC_BLACK_CONTROL    [R/W]    8 bits    Access: 8    primaryRegisterAperture:0xf1a3  
        o_irf_posModulation     0 0x0 1: positive modulation (SECAM) 0: negative modulation (NTSC/PAL)
        o_irf_LoadClampEnable   1 0x1 1: load clamp level from VBI black level until lock
        o_irf_ReLoadClampEnable 2 0x0 1: load clamp level from every VBI black level after lock
        o_irf_enable_phih_init  3 0x0 1: enable field to field phase error tracking
        o_irf_clear_phih_init   4 0x1 1: hold phih_init(v) at 0.0
        o_irf_use_clamp_fix     5 0x0 1: c(i) = UAGC_CLAMP_FIX when USE_CLAMP_FIX bit is set.
        o_irf_freeze_clamp      6 0x0 1: c(i) = c(i-1). */

        ixUAGC_BLACK_CONTROL, 0x0b,   /* 0x0b = 000_1011 Positive modulation */

		/* Vertical Sync Filtering BW control */
		ixUAGC_VSPI_COEF, 0x09,
		ixUAGC_VSYNC_COEF, 0x09,

		/* Trick mode detector threshold */
		ixUAGC_S_TRICK_THRESH, 0x05,

		/* Back Porch Target Level (AGC setpoint) */
        ixUAGC_CLAMP_SETPOINT_1, 0x0e,
        ixUAGC_CLAMP_SETPOINT_0, 0x59,

		/* Offset from horizontal (line) PLL lock point to phase position
		    for sampling the back porch signal level.
		    phiH(m)  phase of line  units are bin_rads [-0.5, 0.5).
		    phiH(m)  = 0  center of sync tip
		    phiH_BP  phase of back porch  units are bin_rads  S+[-2:-8]
 		    irf_BACK_PORCH_PHASE = 4.5e-6 sec * fH (lines/sec) * (1 bin_rad/line)
			                     = 4.5e-6 * 15625
								 = dec2hex(floor(2^8 * 4.5e-6*15625),2) = 12h;
		 */
       ixUAGC_BACK_PORCH_PHASE, 0x12,

        /* mcg Aug 21, 2008  change Gc to 0.5, was 0.25 because Fluke sig gen appears undermodulated  */
        ixUAGC_CLAMP_GAIN, 0x44,		/* 44h->Gc=0.5   45h->Gc=0.25 */
        /* mcg Aug 26, 2008  change Gc to 1.0, was 0.25, Fluke sync tip 250mV, back porch 390 mV  */
        /*ixUAGC_CLAMP_GAIN, 0x48,*/		/* 48h->Gc=1.0   45h->Gc=0.25 */
        ixUAGC_CLAMP_FIX_1, 0x00,		/* 02 */
        ixUAGC_CLAMP_FIX_0, 0x21,		/* a3 */

#if 1
        ixUAGC_K1_FAST, 0x30,		/* 31 */
        ixUAGC_K1_SLOW, 0x31,		/* 32 */
        ixUAGC_K2_FAST, 0x13,		/* 14 */
        ixUAGC_K2_SLOW, 0x15,		/* 16 */
#else
        /* for low level tuner, need additional gain of 2 */
        ixUAGC_K1_FAST, 0x30,   /* Can't increase ?? */
        ixUAGC_K1_SLOW, 0x30,
        ixUAGC_K2_FAST, 0x12,
        ixUAGC_K2_SLOW, 0x14,
#endif

        ixUAGC_PHIH_INIT_GAIN, 0x02,
        ixUAGC_PHASE_HIT_THRESHOLD, 0x14,
        ixUAGC_H_LOCK_LPF_COEF, 0x84,

        ixUAGC_UI_THRESH, 0x29,		/* 14 */

		ixUAGC_UH_THRESH_ACQ, 0x20,       /* dec2hex(floor(2^8 * 0.125),2)  = 20h U[-1:-8] */
		ixUAGC_UH_THRESH_DROP, 0x10,      /* dec2hex(floor(2^8 * 0.0625),2) = 10h U[-1:-8] */
		ixUAGC_UH_THRESH_HIGH_SNR, 0x21,  /* dec2hex(floor(2^8 * 0.130),2)  = 21h U[-1:-8] */
		ixUAGC_GAIN_LINE_LOCK, 0x04,      /* GnH = 2^4 = 16 */
		ixUAGC_THRESHOLD_LINE_LOCK, 0x05, /* dec2hex(floor(2^9 * 0.01),2)   = 05h U[-2:-9] */


        /* -----------------------------------------------------------------*/
        /* UAGC Test Mux */
        /* -----------------------------------------------------------------*/
		ixUAGC_TMUX_CHAN1_SELECT, 0x02,
		ixUAGC_TMUX_CHAN2_SELECT, 0x03,
		ixUAGC_TMUX_BIT_SELECT1, 0x00,
		ixUAGC_TMUX_BIT_SELECT2, 0x00,
		ixUAGC_TMUX_TRIG_SELECT, 0x02,

}; /* end UagcConfig_Analog_PositiveModulation */

static uint32_t UagcConfig_Analog_PositiveModulation_Length = sizeof (UagcConfig_Analog_PositiveModulation);

/****************************************************************************/
/* UAGC register initializations for analog depending on field rate */

static uint32_t UagcConfig_Analog_60Hz[] =
{
        /* -----------------------------------------------------------------*/
        /* Peak Detector Parameters */
        /* -----------------------------------------------------------------*/
        /* wait longer for VCR head switch, making sure HOLDOFF > 1 line
          chanPeakDet.n_LINE				4096
          chanPeakDet.n_HOLDOFF			3610
         >> dec2hex(4096,4) = 0x1000
         >> dec2hex(3610,4) = 0x0E1A */
        ixUAGC_NLINE_CHAN_1, 0x10,
        ixUAGC_NLINE_CHAN_0, 0x00,
        ixUAGC_NHOLDOFF_CHAN_1, 0x0e,
        ixUAGC_NHOLDOFF_CHAN_0, 0x1a,
        ixUAGC_NLINE_BB_1, 0x10,
        ixUAGC_NLINE_BB_0, 0x00,
        ixUAGC_NHOLDOFF_BB_1, 0x0e,
        ixUAGC_NHOLDOFF_BB_0, 0x1a,

        /* -----------------------------------------------------------------*/
        /* Genlock Black Level Peak Detector Parameters */
        /* -----------------------------------------------------------------*/
		ixUAGC_NLINE_PEAK_BLACK_1, 0x43,
		ixUAGC_NLINE_PEAK_BLACK_0, 0xD3,
		ixUAGC_NHOLDOFF_PEAK_BLACK_1, 0x3F,
		ixUAGC_NHOLDOFF_PEAK_BLACK_0, 0xEB,

        /* -----------------------------------------------------------------*/
        /* Genlock Frequency/Phase Initializations                          */
		/*  60 Hz Systems - Vertical                                        */
        /* -----------------------------------------------------------------*/
		/* NCO step size for nominal for (Vertical) Field Rate of 59.94 Hz
		    dPhiv_nom = 1 (bin_rad/field)  * 59.94 (fields/sec) / 13.525e6 (samples/sec)
			          = 4.4362e-6 bin_rad/sample
					  = dec2hex(round(2^32*59.94/13.5e6),4) = 4A7Eh   U[-17:-32]    */
		ixUAGC_DPHIV_1, 0x4A,
		ixUAGC_DPHIV_0, 0x7E,

		/* Lock thresholds for Field Rate PLL.
		    u(v) is the filtered magnitude of the PLL error signal |e(v)|
			       e(v) units are in bin_rads [-0.5, 0.5)
			       u(v) units are in bin_rads [0.0, 0.5]
			
		    U_V_THRESH_ACQ  = 0.001 sec * 60 (fields/sec) * (1 bin_rad/field)
			                = 0.06 bin_rad
			                =  dec2hex(floor(2^16 * 0.06),4) = 0F5Ch   U[-1:-16]
		    U_V_THRESH_DROP = 0.002 sec * 60 (fields/sec) * (1 bin_rad/field)
			                = 0.12 bin_rad
			                =  dec2hex(floor(2^16 * 0.12),4) = 1EB8h   U[-1:-16]
		*/
		ixUAGC_U_V_THRESH_ACQ_1, 0x0F,
		ixUAGC_U_V_THRESH_ACQ_0, 0x5C,
		ixUAGC_U_V_THRESH_DROP_1, 0x1E,
		ixUAGC_U_V_THRESH_DROP_0, 0xB8,

		/* Windows to identify Vertical blanking (VBI) Field Rate of 60 Hz.
		    phiV(m)  phase of field  units are bin_rads [-0.5, 0.5). S+[-2:-16]
		    phiV(m) = 0 at end of VSPI pulse.
		 */

        ixUAGC_PHIV_WIN1_START_1, 0xF9, /* -400 us */
        ixUAGC_PHIV_WIN1_START_0, 0xDB, /* dec2hex(floor(2^16*(1-0.0004*60)),4) = F9DBh */
        ixUAGC_PHIV_WIN1_STOP_1, 0x06,  /*  400 us */
        ixUAGC_PHIV_WIN1_STOP_0, 0x24,  /* dec2hex(floor(2^16* 0.0004 * 60),4) = 0624h */

		ixUAGC_PHIV_WIN2_START_1, 0xF0, /* -1 msec */
		ixUAGC_PHIV_WIN2_START_0, 0xA3, /* dec2hex(floor(2^16*(1-0.001*60)),4)= F0A3h */
		ixUAGC_PHIV_WIN2_STOP_1, 0x0F,  /*  1 msec */
		ixUAGC_PHIV_WIN2_STOP_0, 0x5C,  /* dec2hex(floor(2^16* 0.001 * 60),4) = 0F5Ch */

		ixUAGC_PHIV_WIN3_START_1, 0xD9, /* -2.5 msec */
		ixUAGC_PHIV_WIN3_START_0, 0x99, /* dec2hex(floor(2^16*(1-0.0025*60)),4) = D999h */
		ixUAGC_PHIV_WIN3_STOP_1, 0x13,  /*  1.3 msec */
		ixUAGC_PHIV_WIN3_STOP_0, 0xF7,  /* dec2hex(floor(2^16* 0.0013 * 60),4) = 13F7h */


		/* Phase of phiV(m) [in VBI] at which to sample black level */
		ixUAGC_VBI_BP_PHASE_1, 0x04,	/* 300 usec */
		ixUAGC_VBI_BP_PHASE_0, 0x9B,	/* dec2hex(floor(2^16*0.0003*60),4) = 049Bh */

        /* -----------------------------------------------------------------*/
        /* Genlock Frequency/Phase Initializations                          */
		/*  60 Hz Systems - Horizontal                                      */
        /* -----------------------------------------------------------------*/

		/* NCO step size for nominal for (Horizontal) Line Rate of 15734 Hz
		    dPhiH_nom = 1 (bin_rad/line)  * 15734 (lines/sec) / 13.525e6 (samples/sec)
			          = 0.001163 bin_rad/sample
					  = dec2hex(round(2^24*15734/13.5e6),4) = 4C62h   U[-9:-24]    */
		ixUAGC_DPHIH_1, 0x4C,
		ixUAGC_DPHIH_0, 0x62,

#define HFREQ_TEST
#undef HFREQ_TEST
#ifdef HFREQ_TEST

        /* Line frequency offset test >> dec2hex(round(hex2dec('4c3d')*1.04)) = 4F4A */

		ixUAGC_DPHIH_1, 0x4F,
		ixUAGC_DPHIH_0, 0x4A,

        /* Line frequency offset test >> dec2hex(round(hex2dec('4c3d')*0.96)) = 4930 */

		ixUAGC_DPHIH_1, 0x49,
		ixUAGC_DPHIH_0, 0x30,
#endif

		/* Window in which to measure H-PLL phase error.  This window is used
		    after the H-PLL is locked (to improve performance at very low SNRs.
			When the H-PLL is not locked, the window is in the full interval 
		    from phiH(m) = [-0.5, 0.5)  bin_rads.
		    phiH(m) = 0  center of sync tip
		    phiH_WIN  = 4.0e-6 sec * 15625 
			          = dec2hex(floor(2^8 * 4e-6*15625),2) = 10h    S+[-2:-8]
		 */
		ixUAGC_PHASE_DET_WINDOW, 0x10,

		/* Horizontal Sync PLL Lock detector window.  The sync tip power is 
			measured in this interval and compared to the power outside of 
			the interval to determine if the PLL is locked
			tH = 1/15734;
			T_SYNC_TIP = 4.7e-6;
			T_WIN_SYNC_TIP = T_SYNC_TIP/2.0;
			PHIH_WIN_SYNC_TIP = T_WIN_SYNC_TIP/tH;
			irf_PHIH_LOCK_WINDOW = 1.5*PHIH_WIN_SYNC_TIP;
			dec2hex(floor(2^8 * irf_PHIH_LOCK_WINDOW),2) = 0Eh
		*/

        ixUAGC_PHIH_LOCK_WINDOW, 0x0E, /* U[-1:-8] */

		/* Mnv - Decimation factor from 54.1 Msps rate (n index) to 50/60
		    samples/sec rate (v index, field rate processing).  This value is 
			set to be 5% higher because it is being used for an
			asynchronous peak detector, so slightly more than one frame 
			is used.

			irf_Mnv = (int) Clip(1.05*Fsn/fV,21,14,UNSIGNED);
			dec2hex(floor(2^-14 * 1.05 * 54.1e6/60),2) = 39h
		*/

		ixUAGC_DECIMATION_N2V, 0x39,   /* U[21:14] */


};  /* end UagcConfig_Analog_60Hz */

static uint32_t UagcConfig_Analog_60Hz_Length = sizeof (UagcConfig_Analog_60Hz);

static uint32_t UagcConfig_Analog_50Hz[] =
{
        /* -----------------------------------------------------------------*/
        /* Peak Detector Parameters */
        /* -----------------------------------------------------------------*/
        ixUAGC_NLINE_CHAN_1, 0x0E,
        ixUAGC_NLINE_CHAN_0, 0x33,
        ixUAGC_NHOLDOFF_CHAN_1, 0x08,
        ixUAGC_NHOLDOFF_CHAN_0, 0x00,
        ixUAGC_NLINE_BB_1, 0x0E,
        ixUAGC_NLINE_BB_0, 0x33,
        ixUAGC_NHOLDOFF_BB_1, 0x08,
        ixUAGC_NHOLDOFF_BB_0, 0x00,


        /* -----------------------------------------------------------------*/
        /* Genlock Black Level Peak Detector Parameters */
        /* -----------------------------------------------------------------*/
        ixUAGC_NLINE_PEAK_BLACK_1, 0x44,
        ixUAGC_NLINE_PEAK_BLACK_0, 0x4D,
        ixUAGC_NHOLDOFF_PEAK_BLACK_1, 0x40,
        ixUAGC_NHOLDOFF_PEAK_BLACK_0, 0x65,

        /* -----------------------------------------------------------------*/
        /* Genlock Frequency/Phase Initializations                          */
		/*  50 Hz Systems - Vertical                                        */
        /* -----------------------------------------------------------------*/
		/* NCO step size for nominal for (Vertical) Field Rate of 50 Hz
		    dPhiv_nom = 1 (bin_rad/field)  * 50 (fields/sec) / 13.5e6 (samples/sec)
			          = 3.6969e-6 bin_rad/sample
					  = dec2hex(round(2^32*50/13.5e6),4) = 3E23h   U[-17:-32]    */
        ixUAGC_DPHIV_1, 0x3E,
        ixUAGC_DPHIV_0, 0x23,

		/* Lock thresholds for Field Rate PLL.
		    u(v) is the filtered magnitude of the PLL error signal |e(v)|
			       e(v) units are in bin_rads [-0.5, 0.5)
			       u(v) units are in bin_rads [0.0, 0.5]
			
		    U_V_THRESH_ACQ  = 0.001 sec * 50 (fields/sec) * (1 bin_rad/field)
			                = 0.05 bin_rad
			                =  dec2hex(floor(2^16 * 0.05),4) = 0CCCh   U[-1:-16]
		    U_V_THRESH_DROP = 0.002 sec * 50 (fields/sec) * (1 bin_rad/field)
			                = 0.10 bin_rad
			                =  dec2hex(floor(2^16 * 0.10),4) = 1999h   U[-1:-16]
		*/
#define USE_FASTER_V_LOCK_THRESHOLDS
#ifdef USE_FASTER_V_LOCK_THRESHOLDS
        ixUAGC_U_V_THRESH_ACQ_1, 0x0C,
        ixUAGC_U_V_THRESH_ACQ_0, 0xCC,
        ixUAGC_U_V_THRESH_DROP_1, 0x19,
        ixUAGC_U_V_THRESH_DROP_0, 0x99,
#else
        /* This was the setting as of Aug 26, 2008. - BackPorch lock was delayed bacause of vLock */
        ixUAGC_U_V_THRESH_ACQ_1, 0x01,
        ixUAGC_U_V_THRESH_ACQ_0, 0x48,
        ixUAGC_U_V_THRESH_DROP_1, 0x02,
        ixUAGC_U_V_THRESH_DROP_0, 0x8F,
#endif

		/* Windows to identify Vertical blanking (VBI) Field Rate of 60 Hz.
		    phiV(m)  phase of field  units are bin_rads [-0.5, 0.5). S+[-2:-16]
		    phiV(m) = 0 at end of VSPI pulse.
		 */

        ixUAGC_PHIV_WIN1_START_1, 0xFA, /* -400 us */
        ixUAGC_PHIV_WIN1_START_0, 0xE1, /* dec2hex(floor(2^16*(1-0.0004*50)),4) = FAE1h */
        ixUAGC_PHIV_WIN1_STOP_1, 0x05,  /*  400 us */
        ixUAGC_PHIV_WIN1_STOP_0, 0x1E,  /* dec2hex(floor(2^16* 0.0004 * 50),4) = 051Eh */

        ixUAGC_PHIV_WIN2_START_1, 0xF3, /* -1 msec */
        ixUAGC_PHIV_WIN2_START_0, 0x33, /* dec2hex(floor(2^16*(1-0.001*50)),4) = F333h */
        ixUAGC_PHIV_WIN2_STOP_1, 0x0C,  /*  1 msec */
        ixUAGC_PHIV_WIN2_STOP_0, 0xCC,  /* dec2hex(floor(2^16* 0.001 * 50),4) =0CCCh */

        ixUAGC_PHIV_WIN3_START_1, 0xE0, /* -2.5 msec */
        ixUAGC_PHIV_WIN3_START_0, 0x00, /* dec2hex(floor(2^16*(1-0.0025*50)),4) = E000h */
        ixUAGC_PHIV_WIN3_STOP_1, 0x10,  /*  1.3 msec */
        ixUAGC_PHIV_WIN3_STOP_0, 0xA3,  /* dec2hex(floor(2^16* 0.0013 * 50),4) = 10A3h */

        ixUAGC_S_TRICK_THRESH, 0x05,

		/* Phase of phiV(m) [in VBI] at which to sample black level */
        ixUAGC_VBI_BP_PHASE_1, 0x03,	/* 300 usec */
        ixUAGC_VBI_BP_PHASE_0, 0xD7,	/* dec2hex(floor(2^16*0.0003*50),4) = 03D7h */

        /* -----------------------------------------------------------------*/
        /* Genlock Frequency/Phase Initializations                          */
		/*  50 Hz Systems - Horizontal                                      */
        /* -----------------------------------------------------------------*/

		/* NCO step size for nominal for (Horizontal) Line Rate of 15625 Hz
		    dPhiH_nom = 1 (bin_rad/line)  * 15625 (lines/sec) / 13.525e6 (samples/sec)
			          = 0.001155 bin_rad/sample
					  = dec2hex(round(2^24*15625/13.5e6),4) = 4BDAh   U[-9:-24]    */
        ixUAGC_DPHIH_1, 0x4B,
        ixUAGC_DPHIH_0, 0xDA,

		/* Window in which to measure H-PLL phase error.  This window is used
		    after the H-PLL is locked (to improve performance at very low SNRs.
			When the H-PLL is not locked, the window is in the full interval 
		    from phiH(m) = [-0.5, 0.5)  bin_rads.
		    phiH(m) = 0  center of sync tip
		    phiH_WIN  = 4.0e-6 sec * 15734 
			          = dec2hex(floor(2^8 * 4e-6*15734),2) = 10h    U[-1:-8]
		 */
        ixUAGC_PHASE_DET_WINDOW, 0x10, /* U[-1:-8] */

		/* Horizontal Sync PLL Lock detector window.  The sync tip power is 
			measured in this interval and compared to the power outside of 
			the interval to determine if the PLL is locked
			tH = 1/15625;
			T_SYNC_TIP = 4.7e-6;
			T_WIN_SYNC_TIP = T_SYNC_TIP/2.0;
			PHIH_WIN_SYNC_TIP = T_WIN_SYNC_TIP/tH;
			irf_PHIH_LOCK_WINDOW = 1.5*PHIH_WIN_SYNC_TIP;
			dec2hex(floor(2^8 * irf_PHIH_LOCK_WINDOW),2) = 0Eh
		*/

        ixUAGC_PHIH_LOCK_WINDOW, 0x0E, /* U[-1:-8] */

		/* Mnv - Decimation factor from 54.1 Msps rate (n index) to 50/60
		    samples/sec rate (v index, field rate processing).  This value is 
			set to be 5% higher because it is being used for an
			asynchronous peak detector, so slightly more than one frame 
			is used.

			irf_Mnv = (int) Clip(1.05*Fsn/fV,21,14,UNSIGNED);
			dec2hex(floor(2^-14 * 1.05 * 54.1e6/50),2) = 45h
		*/

		ixUAGC_DECIMATION_N2V, 0x45,   /* U[21:14] */



};  /* end UagcConfig_Analog_50Hz */

static uint32_t UagcConfig_Analog_50Hz_Length = sizeof (UagcConfig_Analog_50Hz);


/****************************************************************************/
static uint32_t UagcConfig_Digital_Modulation[] =
{
        /* --------------------------------------------------------------------------
		*   Default delay before starting demodulator.  The reason for the delay
		*    is unclear.  It is necessary to wait for the tuner PLLs to settle,
		*    and for the RF/IF AGC to settle.
		*/
        ixHI_AFE_DELAY_MSEC, 20,
        /* --------------------------------------------------------------------------
		*  UAGC_CONTROL_1   8 bits  DFE_INDIRECT:0xF140  
		*  irf_invertVgaDeltaSigmaIF 0 0x0 Static
		*   Controls the polarity of Delta-Sigma Converter. 
		*     0: for VGAs with negative slope of Gain vs Voltage.
		*     1: for VGAs with positive slope of Gain vs Voltage
		*  irf_invertVgaDeltaSigmaRF 1 0x0 Static
		*   Controls the polarity of Delta-Sigma Converter. 
		*     0: for VGAs with negative slope of Gain vs Voltage.
		*     1: for VGAs with positive slope of Gain vs Voltage
		*  irf_use_v_k_FIX 2 0x0 Static
		*   For uP control of IF/RF VGA gain.
		*     0: Loop filter runs normally.
		*     1: Loop filter output from UAGC_VIF_FIX register
		*  irf_use_vrf_k_FIX 3 0x0 Static
		*   For uP control of RF VGA gain. 
		*     0: RF takeover integrator runs normally.
		*     1: RF takeover integrator output from UAGC_VRF_FIX register
		*  irf_use_r2_k_FIX 4 0x0 Static
		*   For uP control of Setpoint Adjustment. 
		*     0: Inner Loop filter runs normally.
		*     1: Inner Loop filter output from UAGC_R2_FIX register
		*  irf_freeze_vif 5 0x0 Static
		*   Hold AGC Loop Filter output at last value. Note if loop filter coefficients are programmed to compensate for external poles (RCs) the loop filter output value can be noisey. 0: Loop filter runs normally. 1: Loop filter output v(k) frozen.
		*  irf_freeze_vrf 6 0x0 Static
		*   Hold RF takeover integrator output at last value. 0: RF takeover integrator runs normally. 1: RF takeover integrator output vrf(k) frozen
		*  irf_freeze_r2 7 0x0 Static
		*   Hold Inner Loop Filter output at last value. 0: Inner Loop filter runs normally. 1: Inner Loop filter output v(k) frozen 
		*/
        /* -----------------------------------------------------------------*/
		/* ixUAGC_CONTROL_1 ,0x03, */
		ixUAGC_CONTROL_1 ,0x23,      /* Freeze until ready to go */
        /* --------------------------------------------------------------------------
		*  UAGC_CONTROL_2   8 bits  DFE_INDIRECT:0xF141  
		*  irf_loop_filter_integrator_type 0 0x0 Static
		*   Controls the order of AGC loop filter denominator.
		*     0: 2nd order, i.e. 2 integrators.
		*     1: 1st order, i.e. 1 integrator
		*  irf_loop_filter_type 1 0x0 Static
		*   Controls the order of AGC loop filter.
		*     0: 2nd order, i.e. 2 integrators, up to 3 zeros. 
		*     1: 1st order, i.e. 1 integrator, no zeros
		*  irf_loop_filter_bandwidth 2 0x0 Static
		*     0: use b_fast numerator coefficients in 2nd order mode 
		*     1: use b_slow numerator coefficients in 2nd order mode
		*  irf_detTypeChan 4:3 0x0 Static
		*   Controls the type of detector for the inband signal path.
		*     00: Peak Power Detector - with parameters UAGC_NLINE_CHAN and UAGC_NHOLDOFF_CHAN.
		*     01: Average Power Detector
		*     10: Back Porch / Peak White Detection (SECAM-L)
		*  irf_detTypeAD 5 0x0 Static
		*   Controls the type of detector for the A/D signal path.
		*     0: Peak Power Detector - with parameters UAGC_NLINE_AD and UAGC_NHOLDOFF_AD.
		*     1: Average Power Detector. 
		*  irf_detType 6 0x0 Static
		*   Controls the type of AGC power detector TBD. 
		*     0: Channel Power Detector. 
		*     1: Broadband Power Detector.
		*  irf_agc_load
		*   (Access: W) 7 0x0 Self-Resetting
		*   Loads the initial values to the UAGC blocks
		*   Make sure this bit is hit before the GO bit is released
		*/
        ixUAGC_CONTROL_2, 0x2c, /*   2c = 0010 1100 Channel detector (0x68) */
        /* ixUAGC_CONTROL_2, 0x6c, */   /* 6c = 0010 1100 BroadBand detector (0x68) */

		/* ----------------------------------------------------------------------------------------------
		* UAGC_CONTROL_3 8-bits Reset Value = 8'h3c
		* 07 	irf_use_Gt_k_FIX 	
		*			0 - normal hardware operation of Gt(k) for H-Sync PLL gain control
		*			1 - firmware control of Gt(k) using value in `UAGC_HPLL_TIP_GAIN_FIX register
		*			Reset value is 0x0.
		* 06 	irf_detTypeDc 	
		*			0 - Downconverter Wideband power detector uses peak power (sync tips) detector
		*			1 - Downconverter Wideband power detector uses average power detector
		*			Reset value is 0x0.
		* 05 	irf_humType_IF_DCn
		*		 	0 - Downconverter AGC control signal monitored for AM hum level
		*			1 - IF AGC control signal monitored for AM hum level
		*			Reset value is 0x1.
		* 04 	irf_filterType_Outer_ADn 	
		*			0 - ADC loop filter controls AFE AGC
		*			1 - Outer loop filter controls AFE AG
		*			Reset value is 0x1.
		* 03 	irf_detType_AD_DCn 	
		*			0 - wideband power detector at downconverter output used for AGC
		*			1 - wideband power detector at IF A/D output used for AGC
		*			Reset value is 0x1.
		* 02 	irf_use_vdc_k_FIX 	
		*			0 - normal hardware operation of vDC(k) for Downconverter AGC control
		*			1 - firmware control of vDC(k) using value in UAGC_DIGITAL_VGA_FIX_1,0 registers
		*			Reset value is 0x1.
		* 01 	irf_freezeAGC_ad 	
		*			0 - normal operation of vAD(k) for AFE AGC control
		*			1 - freeze vAD(k) at current value
		*			Reset value is 0x0.
		* 00 	irf_use_v_ad_k_FIX 	
		*			0 - normal hardware operation of vAD(k) for AFE AGC control
		*			1 - firmware control of vAD(k) using value in UAGC_V_ADC_FIX_1,0 registers
		*			Reset value is 0x0.
		*/

        ixUAGC_CONTROL_3, 0x2C, /* irf_filterType_Outer_ADn = 0, ADC Power controls IF VGA*/
        
        ixUAGC_DIGITAL_VGA_FIX_1, 0x20, /* 0 dB gain is at vdc(k) = 0.125*/
        ixUAGC_DIGITAL_VGA_FIX_0, 0x00, 

        /* -----------------------------------------------------------------*/
        /* UAGC Control Bits */
        /* -----------------------------------------------------------------*/
		ixUAGC_RESET_CONTROL, 0x00,

        /* -----------------------------------------------------------------*/
        /* Level Detector Filter BWs                                        */
        /* -----------------------------------------------------------------*/
        ixUAGC_LPF_COEF_CHAN, 0x77, /* b=2.0^(-7); f3db_Hz=54.1e6*b/(2*pi)=67268 */
        ixUAGC_LPF_COEF_AD, 0x77,   /* b=2.0^(-7); f3db_Hz=54.1e6*b/(2*pi)=67268 */   

        /* -----------------------------------------------------------------*/
        /* IF VGA voltage                                                   */
        /* -----------------------------------------------------------------*/

        ixUAGC_V_OFFSET, 0x03,
        ixUAGC_VIF_FIX, 0x80,  /* Half range */

        /* -----------------------------------------------------------------*/
        /* Inner Loop (for adjustment to setpoint due to adjacent channel   */
        /* -----------------------------------------------------------------*/
        /*  ixUAGC_INNER_LOOP_COEF, 0x9F,  */ /* FAST 2^-9  SLOW 2^-15 */
        ixUAGC_INNER_LOOP_COEF, 0x99,    /* FAST 2^-9  SLOW 2^-9 */
        ixUAGC_R2_MAX, 0x27,             /* 30 dB */
        ixUAGC_R2_MIN, 0x00,             /*  0 dB */
        ixUAGC_R2_MIN_ADJUST, 0x04,      /* 0.18 dB */
        ixUAGC_R2_FIX_1, 0x08,           /* mid-range */
        ixUAGC_R2_FIX_0, 0x00,
        ixUAGC_SETPOINT_ADJUST_1, 0x00,  /* rADJ(-1) = 0 dB */
        ixUAGC_SETPOINT_ADJUST_0, 0x00,

        /* -----------------------------------------------------------------*/
        /* RF VGA control - based only on 'C' simulation   */
        /* -----------------------------------------------------------------*/
        ixUAGC_B_RF_SLOW, 0x0F,
        ixUAGC_B_RF_FAST, 0x0B,
        ixUAGC_VRF_TOP_1, 0x43,
        ixUAGC_VRF_TOP_0, 0x6E,
        ixUAGC_VRF_MAX, 0xFF,
        ixUAGC_VRF_MIN, 0x00,
        ixUAGC_VRF_FIX_1, 0x7F,
        ixUAGC_VRF_FIX_0, 0x33,

        /* -----------------------------------------------------------------*/
        /* Lock Detector Controls  */
        /* -----------------------------------------------------------------*/
        ixUAGC_LOCK_DET_COEF, 0xD8,
        ixUAGC_LOCK_STATUS, 0x00,
        ixUAGC_U1_K_1, 0x04,
        ixUAGC_U1_K_0, 0x00,
        ixUAGC_U1_INIT, 0x10,
        ixUAGC_U1_ACQ, 0x66,     /*  0.4dBb = 1.2 dB */
        ixUAGC_U1_DROP, 0x80,    /*  0.5dBb = 1.5 dB */
        ixUAGC_U2_K, 0x00,
        ixUAGC_U2_INIT, 0x00,
        ixUAGC_U2_ACQ, 0x03,     /* 0.023  linear A/D range [-1.0, 1.0) */
        ixUAGC_U2_DROP, 0x06,    /* 0.047 */
        ixUAGC_URF_ACQ, 0x08,
        ixUAGC_URF_DROP, 0x10,
        ixUAGC_URF_K, 0x00,
        ixUAGC_U2_SATURATION, 0x12,
        ixUAGC_EK_SATURATION_1, 0x00,
        ixUAGC_EK_SATURATION_0, 0x18,

        /* -----------------------------------------------------------------*/
        /* Initialize A/D min/max regs */
        /* -----------------------------------------------------------------*/
        ixUAGC_ADC_DATA_MAX, 0x00,
        ixUAGC_ADC_DATA_MIN, 0x00,

        /* -----------------------------------------------------------------*/
        /* UAGC SNR filter BW */
        /* -----------------------------------------------------------------*/
        ixUAGC_ERROR_LPF_COEF, 0x04, /* b=2.0^(-4); f3db_Hz=422e3*b/(2*pi)=4198 */
        ixUAGC_SNR_LPF_COEF, 0xD9,   /* b=2.0^(-9); f3db_Hz=422e3*b/(2*pi)=131 */
                                     /* b=2.0^(-13);f3db_Hz=422e3*b/(2*pi)=8 */

        /* -----------------------------------------------------------------*/
        /* Channel LPF coefficients											*/
		/*    3dB BW:   2.5 MHz                                             */
		/*    Attenuation at bandedge (3MHz):  15dB                         */
		/*    Attenuation at adjacent analog audio (3.25MHz):  28dB         */
        /*  For reference, downconverter (brickwall) LFP specs on June, 2009*/
		/*    3dB BW:   2.95 MHz                                            */
		/*    Attenuation at bandedge (3MHz):  6dB                          */
		/*    Attenuation at adjacent analog audio (3.25MHz):  30dB         */
		/*                                                                  */
		/* This filter removes adjacent channel energy from AGC power       */
		/*  detector measurement.                                           */
		/*                                                                  */
        /* -----------------------------------------------------------------*/
        ixUAGC_LUMA_DATA_1, 0x00,  /* c0 =  0.000000 */
        ixUAGC_LUMA_DATA_0, 0x00,
        ixUAGC_LUMA_ADDRESS, 0x10,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c1 = -0.001953 */
        ixUAGC_LUMA_DATA_0, 0xFF,
        ixUAGC_LUMA_ADDRESS, 0x11,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c2 = -0.001953 */
        ixUAGC_LUMA_DATA_0, 0xFF,
        ixUAGC_LUMA_ADDRESS, 0x12,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c3 =  0.003906 */
        ixUAGC_LUMA_DATA_0, 0x02,
        ixUAGC_LUMA_ADDRESS, 0x13,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c4 =  0.007813 */
        ixUAGC_LUMA_DATA_0, 0x04,
        ixUAGC_LUMA_ADDRESS, 0x14,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c5 =  0.000000 */
        ixUAGC_LUMA_DATA_0, 0x00,
        ixUAGC_LUMA_ADDRESS, 0x15,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c6 = -0.017578 */
        ixUAGC_LUMA_DATA_0, 0xF7,
        ixUAGC_LUMA_ADDRESS, 0x16,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c7 = -0.013672 */
        ixUAGC_LUMA_DATA_0, 0xF9,
        ixUAGC_LUMA_ADDRESS, 0x17,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c8 =  0.021484 */
        ixUAGC_LUMA_DATA_0, 0x0B,
        ixUAGC_LUMA_ADDRESS, 0x18,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c9 =  0.044922 */
        ixUAGC_LUMA_DATA_0, 0x17,
        ixUAGC_LUMA_ADDRESS, 0x19,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c10 =  0.000000 */
        ixUAGC_LUMA_DATA_0, 0x00,
        ixUAGC_LUMA_ADDRESS, 0x1A,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c11 = -0.083984 */
        ixUAGC_LUMA_DATA_0, 0xD5,
        ixUAGC_LUMA_ADDRESS, 0x1B,
        ixUAGC_LUMA_DATA_1, 0x0F,  /* c12 = -0.074219 */
        ixUAGC_LUMA_DATA_0, 0xDA,
        ixUAGC_LUMA_ADDRESS, 0x1C,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c13 =  0.117188 */
        ixUAGC_LUMA_DATA_0, 0x3C,
        ixUAGC_LUMA_ADDRESS, 0x1D,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c14 =  0.390625 */
        ixUAGC_LUMA_DATA_0, 0xC8,
        ixUAGC_LUMA_ADDRESS, 0x1E,
        ixUAGC_LUMA_DATA_1, 0x00,  /* c15 =  0.519531  program reg with c15/2 */
        ixUAGC_LUMA_DATA_0, 0x85,
        ixUAGC_LUMA_ADDRESS, 0x1F
};

static uint32_t UagcConfig_Digital_Modulation_Length = sizeof (UagcConfig_Digital_Modulation);


void BTFE_P_InitUagcData(CHIP *pChip)
{
	pChip->UagcConfig_General = UagcConfig_General;
	pChip->UagcConfig_General_Length = UagcConfig_General_Length;
	pChip->UagcConfig_Analog = UagcConfig_Analog;
	pChip->UagcConfig_Analog_Length = UagcConfig_Analog_Length;
	pChip->UagcConfig_Analog_NegativeModulation = UagcConfig_Analog_NegativeModulation;
	pChip->UagcConfig_Analog_NegativeModulation_Length = UagcConfig_Analog_NegativeModulation_Length;
	pChip->UagcConfig_Analog_PositiveModulation = UagcConfig_Analog_PositiveModulation;
	pChip->UagcConfig_Analog_PositiveModulation_Length = UagcConfig_Analog_PositiveModulation_Length;
	pChip->UagcConfig_Analog_60Hz = UagcConfig_Analog_60Hz;
	pChip->UagcConfig_Analog_60Hz_Length = UagcConfig_Analog_60Hz_Length;
	pChip->UagcConfig_Analog_50Hz = UagcConfig_Analog_50Hz;
	pChip->UagcConfig_Analog_50Hz_Length = UagcConfig_Analog_50Hz_Length;
	pChip->UagcConfig_Digital_Modulation = UagcConfig_Digital_Modulation;
	pChip->UagcConfig_Digital_Modulation_Length = UagcConfig_Digital_Modulation_Length;
}

