/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
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

#endif /* BSRF_LONESTAR_PRIV_H__*/