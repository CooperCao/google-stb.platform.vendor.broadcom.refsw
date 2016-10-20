/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef _BSRF_LONESTAR_PRIV_H__
#define _BSRF_LONESTAR_PRIV_H__

#include "bchp_tm.h"
#include "bchp_top_ctrl.h"
#include "bchp_srfe_table_aci.h"
#include "bchp_srfe_table_iqeq_i.h"
#include "bchp_srfe_table_iqeq_q.h"
#include "bchp_srfe_table_therm_i.h"
#include "bchp_srfe_table_therm_q.h"
#include "bchp_srfe_fe.h"
#include "bchp_srfe_rfagc_lut.h"
#include "bchp_srfe_rfagc_loop.h"
#include "bchp_srfe_intr2.h"
#include "bchp_srfe_ana.h"
#include "bchp_diag_capt_new.h"
#include "bchp_leap_l1.h"


#define BSRF_G1_BUILD_VERSION    0x01

#define BSRF_NUM_CHANNELS        1
#define BSRF_NUM_XSINX_COEFF     4
#define BSRF_NUM_ACI_COEFF_TAPS  78
#define BSRF_NUM_IQEQ_COEFF_TAPS 12
#define BSRF_RFAGC_LUT_COUNT     88

#define BSRF_XTAL_FREQ_KHZ       48000
#define BSRF_ADC_SAMPLE_FREQ_KHZ 1728000  /* Fs = Fxtal x 36 */
#define BSRF_FCW_SAMPLE_FREQ_KHZ 144000   /* Fs / 12 */
#define BSRF_AGC_SAMPLE_FREQ_KHZ 432000   /* Fs / 4 */
#define BSRF_DAC_SAMPLE_FREQ_KHZ 288000   /* Fs / 6 */

#define BSRF_DEFAULT_FC_HZ 2332500000
#define BSRF_MPLL_FREQ_HZ  2310000000
#define BSRF_NOTCH_FREQ_HZ -17999966


/* wfe channel 0 interrupts */
#define BCHP_INT_ID_SRF_ATTACK_COUNT_OVF  BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_0_INTR_SHIFT)
#define BCHP_INT_ID_SRF_DECAY_COUNT_OVF   BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_1_INTR_SHIFT)
#define BCHP_INT_ID_SRF_FS_COUNT_OVF      BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_2_INTR_SHIFT)
#define BCHP_INT_ID_SRF_WIN_DETECT        BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_3_INTR_SHIFT)
#define BCHP_INT_ID_SRF_RAMP_ACTIVE       BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_4_INTR_SHIFT)
#define BCHP_INT_ID_SRF_RAMP_INACTIVE     BINT_LEAP_INT_ID_CREATE(BCHP_LEAP_L1_INTR_W0_STATUS_SRFE_TOP_INTR_SHIFT, BCHP_SRFE_INTR2_CPU_STATUS_RFAGC_5_INTR_SHIFT)


#endif /* BSRF_LONESTAR_PRIV_H__*/