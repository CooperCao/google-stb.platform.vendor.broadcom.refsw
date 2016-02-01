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

#define NCO3_FSTART_HZ_NOMINAL -1400000 
#define NCO3_FSTOP_HZ_NOMINAL  800000
#define NCO3_FCLK              13525000 
#define TWO_TO_POWER_18        262144 

/* see comments below for MIN and MAX offset calculations */ 
/* These will change if NCO3_FSTART_HZ_NOMINAL or NCO3_FSTOP_HZ_NOMINAL change */
#define NCO3_SWEEP_OFFSET_MAX   (290000)
#define NCO3_SWEEP_OFFSET_MIN   (-890000)

void BTFE_P_SetVideoCarrierSweepFrequencies(SCD_HANDLE chip_handle, int32_t sweep_frequency_offset);

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

/* The following are thresholds for demodulator lock U[30:-1]  TBR */
#define SNR_MIN_FRACTIONAL_BITS (1)
#define SNR_MIN_VSB     (15<<SNR_MIN_FRACTIONAL_BITS)
#define SNR_MIN_QAM64   (22<<SNR_MIN_FRACTIONAL_BITS)
#define SNR_MIN_QAM256  (28<<SNR_MIN_FRACTIONAL_BITS)
#define SNR_MIN_COFDM   (17<<SNR_MIN_FRACTIONAL_BITS)

/* The threshold for RF input power  S+[29:-1]  */
#define PRF_MIN_DBM (-96<<SNR_MIN_FRACTIONAL_BITS)

int32_t BTFE_P_SignalQualityIndex(CHIP *tchip, SCD_MOD_FORMAT modulation_type, int32_t snr, int32_t Prf_dBm, bool lock_status);

