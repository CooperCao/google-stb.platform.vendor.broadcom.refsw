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
 * Module Description:  CD Hardware Abstraction layer for 35330 with PMON
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/

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

#include "btfe_scd_35233_priv.h"
#include "btfe_scd_reg35230_hi_priv.h"

#include "btfe_scd_common_func_priv.h"

#define SCD_LEGACY_QAM

extern void BTFE_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo);
extern void BTFE_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo);
extern SCD_RESULT BTFE_P_ChipSetReg16(SCD_RESULT ret_val, SCD_HANDLE handle, uint32_t offset, uint16_t value);

/****************************************************************************/
/*  BTFE_P_SetVideoCarrierSweepFrequencies
 *
 *  Programs the Video Carrier PLL (pilot PLL) registers which determine the
 *   sweep range of the PLL when the TFE is congigured for demodulation of
 *   analog modulation signals.  The sweep is used for Automatic Fine Tuning (AFT)
 *   The sweep range is hard coded here to span 2.2 MHz.  The nominal start
 *   and stop frequencies are hard coded here, but can be shifted by the input
 *   'sweep_frequency_offset'.  The sweep could be made more general but the
 *   analog demodulation performance for AFT has been characterized for
 *   these parameters
 *
 *   Inputs:
 *      sweep_frequency_offset (Hz) - sweep start and stop frequencies
 *       are offset by this amount.
 *
 *   Outputs:
 *      FE_PILOT_FSWEEP_FSTART register is programmed
 *      FE_PILOT_FSWEEP_FSTOP register is programmed
 *
 *
 *    NCO 3 - Video Carrier Recovery PLL
 *           NCO equations  dPhi = F * 2^24 / Fclk
 *                          F    = dPhi * Fclk / 2^N
 *                          
 *                          dPhi:  step size of NCO phase accumulator
 *                          F:     sinusoid freuency (Hz)
 *                          Fclk:  NCO HW clock frequency (sampling rate)
 *                          N:     register width of NCO phase accumulator
 *
 *                          dPhi_norm:  normalized step size of NCO phase accumulator
 *                           dPhi_norm = dPhi / 2^N = F / Fclk   (units: cycles/sample)
 *
 *   This function uses the normalized dPhi calculation, since the bit width
 *    of the start and stop registers is not the same as the NCO's phase accumulator
 */

void BTFE_P_SetVideoCarrierSweepFrequencies(SCD_HANDLE chip_handle, int32_t sweep_frequency_offset)
{
    uint32_t abs_nco_frequency_Hz, P_hi, P_lo, Q_hi, Q_lo;
    int32_t  nco_frequency_Hz;
    int16_t nco_dphi_norm_fixed_point; /* Fixed Point Format: S+[-4:-18] e.g. -0.103436 = 0x9615 */

    SCD_RESULT ret_val = SCD_RESULT__OK;

    /* limit the offset so the sweep range is not exceeded */
    /*      NCO Fmin = -0.125*Fsm =  -1.690625 MHz
                NCO3_FSTART_HZ_NOMINAL   -1.4 MHz
                                          0.290625    */
    if (sweep_frequency_offset > NCO3_SWEEP_OFFSET_MAX)
        sweep_frequency_offset = NCO3_SWEEP_OFFSET_MAX;        
      
    /*      NCO Fmax =  0.125*Fsm =   1.690625 MHz
                NCO3_FSTART_HZ_NOMINAL    0.8 MHz
                                          0.890625    */
    if (sweep_frequency_offset < NCO3_SWEEP_OFFSET_MIN)
        sweep_frequency_offset = NCO3_SWEEP_OFFSET_MIN;        

    nco_frequency_Hz = NCO3_FSTART_HZ_NOMINAL - sweep_frequency_offset;
    if (nco_frequency_Hz < 0)
        abs_nco_frequency_Hz = (uint32_t)-nco_frequency_Hz;
    else
        abs_nco_frequency_Hz = (uint32_t)nco_frequency_Hz;
    BTFE_MultU32U32(abs_nco_frequency_Hz, TWO_TO_POWER_18, &P_hi, &P_lo);
    BTFE_DivU64U32(P_hi, P_lo, NCO3_FCLK, &Q_hi, &Q_lo);
    if (nco_frequency_Hz < 0)
        nco_dphi_norm_fixed_point = (int16_t)-Q_lo;
    else
        nco_dphi_norm_fixed_point = (int16_t)Q_lo;
    ret_val = BTFE_P_ChipSetReg16(ret_val, chip_handle, ixFE_PILOT_FSWEEP_FSTART_MSB, nco_dphi_norm_fixed_point);

   /* Stop frequency */
    nco_frequency_Hz = NCO3_FSTOP_HZ_NOMINAL - sweep_frequency_offset;
    if (nco_frequency_Hz < 0)
        abs_nco_frequency_Hz = (uint32_t)-nco_frequency_Hz;
    else
        abs_nco_frequency_Hz = (uint32_t)nco_frequency_Hz;
    BTFE_MultU32U32(abs_nco_frequency_Hz, TWO_TO_POWER_18, &P_hi, &P_lo);
    BTFE_DivU64U32(P_hi, P_lo, NCO3_FCLK, &Q_hi, &Q_lo);
    if (nco_frequency_Hz < 0)
        nco_dphi_norm_fixed_point = (int16_t)-Q_lo;
    else
        nco_dphi_norm_fixed_point = (int16_t)Q_lo;
    ret_val = BTFE_P_ChipSetReg16(ret_val, chip_handle, ixFE_PILOT_FSWEEP_FSTOP_MSB, nco_dphi_norm_fixed_point);

    return;
}

/****************************************************************************/
/*  BTFE_P_SignalQualityIndex
 *
 *  Compute SQI in the range from 0 to 100.
 *   Used to assist in antenna pointing (and maybe channel scan)
 *
 *   Inputs:
 *      modulation_type:  VSB, 64 QAM etc
 *      snr:  for digital modulation types, this is SNR at slicer
 *            for analog modulation types, this is SNR at sync tip or back porch
 *      Prf_dBm: estimate of RF power at tuner input  S+[29:-1]
 *      lock_status:  1 = demodulator is locked  0 = unlocked
 *
 *   Outputs:
 *      sqi:  signal quality in Range {0 to 100}  The formulas have been adjust
 *             based on the assumption that a signal quality bar display
 *             might be used which only has a granularity of 6 levels
 *             (0  1-20, 21-40, 41-60, 61-80, 81-100)
 */

int32_t BTFE_P_SignalQualityIndex(CHIP *tchip, SCD_MOD_FORMAT modulation_type, int32_t snr, int32_t Prf_dBm, bool lock_status)
{

int32_t  snr_min = 0;          /* minimum SNR for demod lock. S+[29:-1] */
int32_t  snr_min_plus10dB; /* minimum SNR + 10 dB   S+[29:-1] */
int32_t  snr_fixed_pt;     /* snr with adjusted fixed binary point format */
int32_t  Prf_dBm_fixed_pt; /* Prf with adjusted fixed binary point format */
int32_t  sqi;              /* Signal Quality Index [0,1,...100] */

	/* ---------------------------------------------------------------------------------------------
	 *  Input parameters and the SQI formula differ based on the modulation type
     *
	 --------------------------------------------------------------------------------------------- */
	if (IS_ANALOG_FAT_MOD_FORMAT (modulation_type))
	{
		/* Analog SQI - following comments assume SQI is used to point an antenna
		 *   when trying to find an analog channel of known modulation type.
		 *
		 *   If there is a large unknown signal present (i.e. IFD can't lock), SQI will be zero.
		 *     This is because the Analog Demod can lock at very low levels.  Therefore
		 *     the SQI meter will vary from 0 to 100 starting with extremely snowy video
		 *     and ending at 100 with video with high SNR.
		 *
		 * 
		 */
		if (lock_status)
		{
			snr_fixed_pt = snr >> 6;          /* S+[22:-8] -> S+[28:-2]  */
			Prf_dBm_fixed_pt = Prf_dBm << 1;  /* S+[29:-1] -> S+[28:-2]  */

			/* Prf_dBm + 88 dBm  + 1.25 * snr  */
			sqi = Prf_dBm_fixed_pt + (88 << 2) + snr_fixed_pt + (snr_fixed_pt>>2);  /* S+[28:-2] */

			/* Convert to integer format */
			sqi = sqi >> 2;     /* S+[28:-2] -> S+[30:0]  */
		}
		else
		{
			sqi = 0;
		}
	}
	else
	{
		/* Digital SQI - following comments assume SQI is used to point an antenna
		 *   when trying to find a digital channel of known modulation type.
		 *
		 *   Since the demods will not lock until there is a significant signal at
		 *     the RF input, the SQI meter begins to move as soon as there is any
		 *     energy above the noise floor.  Once the demod is able to lock
		 *     the SNR meter will rise quickly depending on SNR and will continue
		 *     to rise slowly depending on RF input level.
		 *
		 *   When the demod is not locked the SNR is set to the minimum so as
		 *     not to affect the SQI
		 *
		 *   With a 6 level display the goal (TBR) is
		 *      0      - just the noise floor
		 *      1 - 20 - some power but not enough to lock
		 *     21 - 40 - intermittent lock, macro blocking
		 *     41 - 60 - locked OK but not much margin, maybe some FEC correction
		 *     61 - 80 - locked with some margin < 10 dB
		 *     81 -100 - locked with significant margin > 10 dB
		 */
		switch(modulation_type)
		{
		case SCD_MOD_FORMAT__FAT_COFDM_5:
		case SCD_MOD_FORMAT__FAT_COFDM_6:
		case SCD_MOD_FORMAT__FAT_COFDM_7:
		case SCD_MOD_FORMAT__FAT_COFDM_8:
		case SCD_MOD_FORMAT__FAT_UNIFIED_COFDM:
			snr_min = SNR_MIN_COFDM;
			snr_fixed_pt = snr >> 2;     /* S+[27:-3] -> S+[29:-1]  */
			break;
		case SCD_MOD_FORMAT__FAT_VSB:
			snr_min = SNR_MIN_VSB;
			snr_fixed_pt = snr >> 8;     /* S+[21:-9] -> S+[29:-1]  */
			break;
#ifdef SCD_LEGACY_QAM
		case SCD_MOD_FORMAT__FAT_QAM64:
			snr_min = SNR_MIN_QAM64;
			snr_fixed_pt = snr >> 8;     /* S+[21:-9] -> S+[29:-1]  */
			break;
		case SCD_MOD_FORMAT__FAT_QAM256:
			snr_min = SNR_MIN_QAM256;
			snr_fixed_pt = snr >> 8;     /* S+[21:-9] -> S+[29:-1]  */
			break;
#endif
		case SCD_MOD_FORMAT__FAT_J83ABC:
			snr_fixed_pt = snr >> 8; 
			if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_64QAM)
				snr_min = SNR_MIN_QAM64;
			if (tchip->j83abc.qamMode == QAM_MODE_DIGITAL_256QAM)
				snr_min = SNR_MIN_QAM256;
			break;
		default:
			snr_min = 0;
			snr_fixed_pt = 0; 
			break;

		}

		/* Limit max SNR to 10 dB above minimum requirement. */

		snr_min_plus10dB = snr_min  + (10 << SNR_MIN_FRACTIONAL_BITS);
		snr_fixed_pt = (snr_fixed_pt >= snr_min_plus10dB) ?
			                 snr_min_plus10dB : snr_fixed_pt;

		/* Limit min SNR. */

		snr_fixed_pt = ( (snr_fixed_pt < snr_min) || (lock_status==0) ) ?
			                 snr_min : snr_fixed_pt;

		/* SQI = 2 * (Prf_dBm + 96 dBm) + 4 * (snr - snr_min) */
		sqi = 2 * (Prf_dBm - PRF_MIN_DBM) + 4 * (snr_fixed_pt - snr_min);  /* S+[29:-1] */

		/* Convert to integer format */
		sqi = sqi >> SNR_MIN_FRACTIONAL_BITS;

	}

	/* Limit SQI to interval [0,1,...100]  */

	sqi = (sqi <    0 ) ?   0 : sqi;
	sqi = (sqi >= 100 ) ? 100 : sqi;

    return sqi;
}

