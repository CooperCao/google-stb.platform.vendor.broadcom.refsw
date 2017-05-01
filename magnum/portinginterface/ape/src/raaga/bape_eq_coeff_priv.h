/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *   This file contains various equalizer generation algorithms.
 *   The fixed point code is written for 3548 HW where coefs are represented in
 *   2.26 format and the equation implemented is 
 *   y(n) = b0 * x(n) + b1 * x(n-1) + b2 * x(n-2) + a1 * y(n-1) + a2 * y(n-2)
 *
 ***************************************************************************/

#include "bape.h"

#define EQ_FILTER_TYPE_BUTTER   0
#define EQ_FILTER_TYPE_LRILEY   1

/******************************************************************************
 *
 * FUNCTION NAME:  EQ_generate_peq_isrsafe
 *
 * Description:
 *         This function takes the user defined level for parametric equaliser
 *     and generates coefs required for 3548 hardware
 *         This function generated only one biquad. For multiple band this
 *     function shall be called multiple times. For eg. to generate coef. for
 *     7-band equalizer, the function shall be called 7-times. Each function
 *     call shall give one set of parameters.
 *
 * Input:  
 *     
 *     q_int       - 'Q' value of the equalizer
 *                     'Q' = fc/bandwidth. The value q_int = Q * 100. This
 *                     allows 'Q' to be represented in +/- 0.01 resolution.
 *     fc  - center frequency in Hz
 *     
 *     dbval_int   -    Gain of the peaking filter
 *     fs          - Sampling rate at which the equalizer runs
 *                     Tested for 32000, 44100, 48000
 *     gb          - The gain at edge frequencies. It is defined as percentage
 *                     of the gain at peaking frequency. If not exposed to
 *                     user, set this value as 70. (Equal to 70% of peaking
 *                     gain in dB). Allowed range (10, 90)
 *
 * Output:
 *     b_int, a_int
 *         - Biquad filter coefs.
 * Return:
 *     0 - On successful generation of coefs.
 *     Non-zero - In case of input values out of range
 * Note:
 *     This function shall support the dB ranges from -12dB to +12dB.
 *
 ******************************************************************************/
int32_t EQ_generate_peq_isrsafe ( int32_t q_int, int32_t fc,
        int32_t dbval_int, int32_t fs, int32_t gb,
        int32_t b_int[], int32_t a_int[]);
/******************************************************************************
 *
 * FUNCTION NAME:  EQ_generate_geq_isrsafe
 *
 * Description:
 *         This function takes the user defined level for graphic equaliser
 *         and generates coefs required for 3548 hardware
 *
 * Input:  
 *     req_db[]    - Array of values representing the level for 5 different
 *                     center frequency.
 *                     (e.g) req_db[0] = 43;   refers to 4.3dB for 100 Hz
 *                         req_db[3] = -90;    refers to -9dB for 3160 Hz
 *     fs          - Sampling rate at which the equalizer runs
 *                     Tested for 32000, 44100, 48000
 *
 * Output:
 *     b0, b1, b2, a1, a2 - Array of values where the coefs are stored
 * Return:
 *     0 - On successful generation of coefs.
 *     Non-zero - In case of input values out of range
 * Note:
 *     This function shall support the dB ranges from -10dB to +10dB.
 *     The frequencies are fixed at 100 Hz, 316 Hz, 1000 Hz, 3160 Hz, 10000 Hz
 *     The sampling rates are considered to be
 *     2^n * 32000, 2^n * 44100 or 2^n * 48000
 *
 ******************************************************************************/
int32_t EQ_generate_geq_isrsafe ( int32_t fs, int32_t req_db_int[5],
        int32_t b0_int[5], int32_t b1_int[5], int32_t b2_int[5],
        int32_t a1_int[5], int32_t a2_int[5]);
/******************************************************************************
 *
 * FUNCTION NAME:  EQ_subsonic_HPF_isrsafe
 *
 * Description:
 *         This function takes the user defined cut-off level for high pass filter
 *     The function returns the pre-stored coefs in required format. The function supports
 *     2nd/4th/6th order filters. This function can generated HPF of type butterworth and
 *     Linkwitz-Riley filter types.
 *
 * Input:  
 *     fs          - Sample rate.
 *     fc          - Cut-off frequency for HPF
 *     type        - 0 -> Butterworth filter
 *                 - 1 -> Linkwitz-Riley filter
 * Output:
 *     b0[], b1[], b2[], a1[], a2[]
 *         - biquad filter coefs.
 *
 * Return:
 *     0 - On successful generation of coefs.
 *     Non-zero - In case of input values out of range
 *
 ******************************************************************************/
int32_t EQ_subsonic_HPF_isrsafe (int32_t fs, int32_t fc, int32_t type, int32_t order,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[]);
/******************************************************************************
 *
 * FUNCTION NAME:  EQ_subwoofer_LPF_isrsafe
 *
 * Description:
 *         This function takes the user defined cut-off level for low pass filter
 *     The function returns the pre-stored coefs in required format. The function supports
 *     2nd/4th/6th order filters. This function can generated LPF of type butterworth and
 *     Linkwitz-Riley filter types.
 *
 * Input:  
 *     fs          - Sample rate.
 *     fc          - Cut-off frequency for LPF
 *     type        - 0 -> Butterworth filter
 *                 - 1 -> Linkwitz-Riley filter
 * Output:
 *     b0[], b1[], b2[], a1[], a2[]
 *         - biquad filter coefs.
 *
 * Return:
 *     0 - On successful generation of coefs.
 *     Non-zero - In case of input values out of range
 *
 ******************************************************************************/
int32_t EQ_subwoofer_LPF_isrsafe (int32_t fs, int32_t fc, int32_t type, int32_t order,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[]);
/******************************************************************************
 *
 * FUNCTION NAME:  EQ_generate_tone_control_isrsafe
 *
 * Description:
 *         This generates coefs. for high shelving filter and low shelving
 * filter. It takes input value in dB level. Allowed range is +/- 12dB. This
 * function generates coefs. for 2 biquad filter.
 *
 * Input:  
 *     fs          - Sample Rate
 *     gain_bass   - Gain for Bass Boost
 *     gain_treble - Gain for Treble Boost
 *     fc_bass     - Cut-off frequency for Bass. Use default value of 100
 *     fc_treble   - Cut-off frequency for Treble. Use default value of 10000
 *
 * Output:
 *     b0[], b1[], b2[], a1[], a2[]
 *         - biquad filter coefs.
 *
 * Return:
 *     0 - On successful generation of coefs.
 *     Non-zero - In case of input values out of range
 *
 ******************************************************************************/
int32_t EQ_generate_tone_control_isrsafe (int32_t fs, int32_t gain_bass, int32_t gain_treble,
        int32_t fc_bass, int32_t fc_treble, BAPE_ToneControlEqType type_bass,
        BAPE_ToneControlEqType type_treble, int32_t bandwidth_bass,
        int32_t bandwidth_treble,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[]);
