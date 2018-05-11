/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"
#include "bchp.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayvip_priv.h"

#if (BVDC_P_SUPPORT_VIP)

#define MAX_STATS_TO_PROCESS 3

#define SIGMA_RS         5
#define CORR_THRESH_BITS 10         /* Number of bits in correlation thresholds (25 + 1 - sigma_rs + corr_thresh_bits) <= max bits in ALU (31 or 32, depending on sign) */

#define CAD_LOCK_ENTER 6
#define CAD_LOCK_EXIT 5
#define CAD_LOCK_SAT 32

#define CAD_3_2_LENGTH 5
#define CAD_3_2_REP_PAT 0xb
#define CAD_3_2_R2_PAT 0x1
#define CAD_3_2_PAIR_PAT 0x14

#define CAD_2_2_LENGTH 4
#define CAD_2_2_REP_PAT 0x5
#define CAD_2_2_R2_PAT 0x0
#define CAD_2_2_PAIR_PAT 0xa

#define PCAD_3_2_LENGTH 5
#define PCAD_3_2_REP_PAT 0x16
#define PCAD_3_2_R2_PAT 0x0b
#define PCAD_3_2_PAIR_PAT 0

#define PCAD_2_2_LENGTH 4
#define PCAD_2_2_REP_PAT 0xa
#define PCAD_2_2_R2_PAT 0x05
#define PCAD_2_2_PAIR_PAT 0

#define CAD_FLD_CNT_MAX 20  /* 5 x 4 */

#define PCAD_3_2_CNTR_SAT 60
#define PCAD_3_2_CNTR_THR 40

#define GET_CAD_PHASE_COUNTER(c , n)      ((c)->phase_counter[n] & 0x7ff)
#define INC_CAD_PHASE_COUNTER(c , n)      if(((c)->phase_counter[n] & 0x7ff) != 0x7ff) (c)->phase_counter[n]++;
#define DEC_CAD_PHASE_COUNTER(c , n)      if(((c)->phase_counter[n] & 0x7ff) != 0) (c)->phase_counter[n]--;
#define ZERO_CAD_PHASE_COUNTER(c , n)     (c)->phase_counter[n] &= 0xf800;
#define GET_CAD_LOCKED(c , n)             ((c)->phase_counter[n] & 0x800)
#define SET_CAD_LOCKED(c , n)             (c)->phase_counter[n] |= (1 << 11);
#define CLEAR_CAD_LOCKED(c , n)           (c)->phase_counter[n] &= 0xf7ff;
#define GET_CAD_VETO(c , n)               ((c)->phase_counter[n] & 0x1000)
#define SET_CAD_VETO(c , n)               (c)->phase_counter[n] |= (1 << 12);
#define CLEAR_CAD_VETO(c , n)             (c)->phase_counter[n] &= 0xefff;
#define GET_CAD_VOF(c , n)                ((c)->phase_counter[n] & 0x2000)
#define SET_CAD_VOF(c , n)                (c)->phase_counter[n] |= (1 << 13);
#define CLEAR_CAD_VOF(c , n)              (c)->phase_counter[n] &= 0xdfff;
#define ABS(x)                            ((x) > 0? (x) : -(x))
#define MAX(x, y)                         ((x) > (y)? (x) : (y))
#define MIN(x, y)                         ((x) < (y)? (x) : (y))
#define DIV16_ROUNDUP(x)                  ( ((x) + 15) >> 4 )

/*Use defines instead of structure to save memroy */
#define     VICE_CD_FW_CORR_INC_THRESH          750
#define     VICE_CD_FW_CORR_DEC_THRESH          600
#define     VICE_CD_FW_CORR_ZERO_THRESH         400
#define     VICE_CD_FW_CORR_INC_THRESH_T1       600
#define     VICE_CD_FW_CORR_DEC_THRESH_T1_PROG  500
#define     VICE_CD_FW_CORR_ZERO_THRESH_T1_PROG 500
#define     VICE_CD_FW_CORR_DEC_THRESH_T1_INT   570
#define     VICE_CD_FW_CORR_ZERO_THRESH_T1_INT  400
#define     VICE_CD_FW_CORR_INC_SEL             1
#define     VICE_CD_FW_CORR_DEC_SEL             0
#define     VICE_CD_FW_CORR_ZERO_SEL            0
#define     VICE_CD_FW_MAX_SIGMA_NOINC          6
#define     VICE_CD_FW_PAIR_THRESH              0x2000 /*THIS THRESHOLD SHOULD BE SCALED ACCORDING TO RESOLUTION */
#define     VICE_CD_FW_PAIR_RATIO               16*2
#define     VICE_CD_FW_PAIR_NOISE_THRESH        1200
#define     VICE_CD_FW_REPF_VETO_LEVEL          14000 /*THIS THRESHOLD SHOULD BE SCALED ACCORDING TO RESOLUTION */
#define     VICE_CD_FW_REPF2_VETO_LEVEL         0x4000 /*THIS THRESHOLD SHOULD BE SCALED ACCORDING TO RESOLUTION */
#define     VICE_CD_FW_EXTRA_REPEAT_RATIO       8
#define     VICE_CD_FW_MISSING_REPEAT_RATIO     7
#define     VICE_CD_FW_REPF_EXTRA_REPEAT_THRESH 1750
#define     VICE_CD_FW_SWITCH_POL_STILL_THRESH  0x2000

BDBG_MODULE(BVDC_VIP_ITFP);
BDBG_FILE_MODULE(BVDC_ITFP_Q);
BDBG_FILE_MODULE(BVDC_ITFP_CAD);

static int32_t BVDC_P_EPM_mod_isrsafe(int32_t x, int32_t y)
{
    int32_t ret = x % y;
    if (ret < 0)
        ret = ret + y;
    return ret;
}

/*!
 ************************************************************************
 * \brief
 *    BVDC_P_cadence_detection_isr() - Cadence detection for a single cadence
 *
 ************************************************************************
 */
static uint8_t BVDC_P_cadence_detection_isr(BVDC_P_ITFP_vice_cd_fw_regs_t *fw,
                       int32_t *prev_sigma_arg,
                       int32_t *prev_sigma2_arg,
                       BVDC_P_ITFP_cad_info_t *cad_info,
                       BVDC_P_ITFP_cad_chan_state_t *cad_chan,
                       int32_t fcnt,
                       int8_t *lock,
                       int8_t *repeat,
                       int8_t *repeat2,
                       int32_t t2_flag) /*NS */
{
    uint8_t i;
    uint8_t phase;
    uint8_t j;
    int32_t t;
    int32_t cur_sigma;
    int32_t cur_sigma2;
    int32_t numerator;
    int32_t denominator;
    int32_t numerator_alt;
    int32_t denominator_alt;
    uint8_t cad_index;
    uint8_t num_hi;
    uint8_t num_lo;
    int32_t max_sigma;
    uint8_t shift_phase;
    int32_t hi[16];
    int32_t max_lo;
    int8_t sigma_too_small;
    uint8_t locked_phase = 0;

    max_sigma = 0;
    *lock = 0;
    *repeat = 0;
    *repeat2 = 0;
    /* Find max sigma value */
    for (i = 0; i < cad_info->control.length; i += 1)
    {
        cur_sigma = prev_sigma_arg[i];
        if (cur_sigma > max_sigma) max_sigma = cur_sigma;
    }

    sigma_too_small = (max_sigma < fw->min_sigma_thresh);
    /*sigma_too_small = 0; */

    /* Loop over each phase */
    for (phase=0; phase<cad_info->control.length; phase+=1)
    {
        shift_phase = (fcnt - phase + 1 + cad_info->control.length) % cad_info->control.length;
        num_hi = num_lo = 0;
        max_lo = -1;
        /* Pattern match */
        for (i = 0; i < cad_info->control.length; i += 1)
        {
            cur_sigma = prev_sigma_arg[i];
            cur_sigma2 = prev_sigma2_arg[i];
            /*if (cur_sigma > (1<<22)) */
            /*{ */
            /*    fprintf(stderr, "sigma too large %d", cur_sigma); */
            /*    exit(1); */
            /*} */

            /*if(cad_info->pattern[(i+phase)%cad_info->control.length].hl==1) */
            if ((cad_info->control.metric_sel == 0) && (cad_info->pattern[(i+shift_phase)%cad_info->control.length].hl==1))
            {
                hi[num_hi] = cur_sigma;
                num_hi+=1;
            }
            else if (cad_info->control.metric_sel == 0)
            {
                if (cur_sigma > max_lo)
                    max_lo = cur_sigma;
                num_lo+=1;
            }
            else if ((cad_info->control.metric_sel == 1) && (cad_info->pattern[(i+shift_phase+cad_info->control.length-1)%cad_info->control.length].t2==1))
            {
                hi[num_hi] = cur_sigma2;
                num_hi+=1;
            }
            else if (cad_info->control.metric_sel == 1)
            {
                /*       if((i==cad_info->control.length-1) && (veto)) */
                /*    do_veto = 1; */
                if (cur_sigma2 > max_lo)
                    max_lo = cur_sigma2;
                num_lo+=1;
            }
        }

        /* Bubble sort hi[] (could be more efficient time-wise) */
        for (i = 0; i < num_hi-1; ++i)
        {
            for (j = i + 1; j < num_hi; ++j)
            {
                if (hi[i] > hi[j])
                {
                    t = hi[i]; hi[i] = hi[j]; hi[j] = t;
                }
            }
        }

        /* Compute numerator and denominator for both metrics */
        if (num_hi%2)
            numerator = hi[num_hi/2];
        else
            numerator = (hi[num_hi/2-1] + hi[num_hi/2] + 1) >> 1;
        numerator_alt = hi[0];

        denominator = max_lo + numerator;
        denominator_alt = max_lo + numerator_alt;

        /*yehuda comment the next line since it is not used */
        /*corr = (int)(1024 * numerator / denominator); */

        /* Phase counter update */
        cad_index = (fcnt - shift_phase + 1 + cad_info->control.length) % cad_info->control.length;

        if ((!sigma_too_small) && t2_flag)
        {
            if (((!VICE_CD_FW_CORR_INC_SEL) && ((numerator << CORR_THRESH_BITS)  > denominator * VICE_CD_FW_CORR_INC_THRESH)) ||
                (VICE_CD_FW_CORR_INC_SEL && ((numerator_alt << CORR_THRESH_BITS) > denominator_alt * VICE_CD_FW_CORR_INC_THRESH)))
            {
                INC_CAD_PHASE_COUNTER(cad_chan, cad_index);
            }
            if (((!VICE_CD_FW_CORR_DEC_SEL) && ((numerator << CORR_THRESH_BITS) < denominator * VICE_CD_FW_CORR_DEC_THRESH)) ||
                (VICE_CD_FW_CORR_DEC_SEL && ((numerator_alt << CORR_THRESH_BITS) > denominator_alt * VICE_CD_FW_CORR_DEC_THRESH)))
            {
                DEC_CAD_PHASE_COUNTER(cad_chan, cad_index);
            }
            if (((!VICE_CD_FW_CORR_ZERO_SEL) && ((numerator << CORR_THRESH_BITS) < denominator * VICE_CD_FW_CORR_ZERO_THRESH)) ||
                (VICE_CD_FW_CORR_ZERO_SEL && ((numerator_alt << CORR_THRESH_BITS) > denominator_alt * VICE_CD_FW_CORR_ZERO_THRESH)))
            {
                ZERO_CAD_PHASE_COUNTER(cad_chan, cad_index);
/*              if ( */
/*                  (phase == 2) */
/*                  ) */
/*              { */
/*                  printf("Phase2 Zeroed: numerator=%d numerator_alt=%d denominator=%d denominator_alt=%d\n", numerator, numerator_alt, denominator, denominator_alt); */
/*              } */
            }
        }
        else if ((!sigma_too_small) && !t2_flag)
        {
            if (((!VICE_CD_FW_CORR_INC_SEL) && ((numerator << CORR_THRESH_BITS) > denominator * VICE_CD_FW_CORR_INC_THRESH_T1)) ||
                (VICE_CD_FW_CORR_INC_SEL && ((numerator_alt << CORR_THRESH_BITS) > denominator_alt * VICE_CD_FW_CORR_INC_THRESH_T1)))
            {
                INC_CAD_PHASE_COUNTER(cad_chan, cad_index);
            }
            if (((!VICE_CD_FW_CORR_DEC_SEL) && ((numerator << CORR_THRESH_BITS) < denominator * fw->corr_dec_thresh_t1)) ||
                (VICE_CD_FW_CORR_DEC_SEL && ((numerator_alt << CORR_THRESH_BITS) > denominator_alt * fw->corr_dec_thresh_t1)))
            {
                DEC_CAD_PHASE_COUNTER(cad_chan, cad_index);
            }
            if (((!VICE_CD_FW_CORR_ZERO_SEL) && ((numerator << CORR_THRESH_BITS) < denominator * fw->corr_zero_thresh_t1)) ||
                (VICE_CD_FW_CORR_ZERO_SEL && ((numerator_alt << CORR_THRESH_BITS) < denominator_alt * fw->corr_zero_thresh_t1)))
            {
                ZERO_CAD_PHASE_COUNTER(cad_chan, cad_index);
            }
        }
        else
        {
            /*DEC_CAD_PHASE_COUNTER(cad_chan, cad_index);   // Decrement phase counters if sigmas are too small */
        /* TBD: add variance threshold for RF - or check matching pairs */
        }

        if (GET_CAD_VETO(cad_chan, cad_index))
        {
            ZERO_CAD_PHASE_COUNTER(cad_chan, cad_index);
        }
        while (GET_CAD_PHASE_COUNTER(cad_chan, cad_index) > cad_chan->lock_sat_level)
        {
            DEC_CAD_PHASE_COUNTER(cad_chan, cad_index);
        }
        /*printf("Phase: %d, count: %d, corr=%d, do_veto=%d, sigma_too_small=%d\n", phase, GET_CAD_PHASE_COUNTER(cad_chan, phase), corr, */
        /*        GET_CAD_VETO(cad_chan, cad_index), sigma_too_small); */
    }

    /* Update lock status and find lock */
    for (phase = 0; phase < cad_info->control.length; phase+=1)
    {
        cad_index = phase;
        if (GET_CAD_PHASE_COUNTER(cad_chan, cad_index) <= cad_chan->exit_ll)
        {
            CLEAR_CAD_LOCKED(cad_chan, cad_index);
        }
        if (GET_CAD_LOCKED(cad_chan, cad_index) || (GET_CAD_PHASE_COUNTER(cad_chan, cad_index) > cad_chan->enter_ll))
        {
            SET_CAD_LOCKED(cad_chan, cad_index);
            if (cad_info->control.enable && (*lock == 0))
            {
                locked_phase = (fcnt - phase + cad_info->control.length)%cad_info->control.length;
                *lock = 1;
                *repeat = (!cad_info->pattern[(fcnt - phase + cad_info->control.length)%cad_info->control.length].hl);
                /* :(!cad_info->pattern[(fcnt - phase + cad_info->control.length + 1)%cad_info->control.length].hl); */
                *repeat2 = (!cad_info->pattern[(fcnt - phase - 1 + cad_info->control.length)%cad_info->control.length].t2);
            }
        }
    }
    return(locked_phase);
}

/*!
 ************************************************************************
 * brief
 *    BVDC_P_compute_cadence_isr() - Cadence algorithm entry point
 *
 ************************************************************************
 */

static void BVDC_P_compute_cadence_isr(BVDC_P_Vip_Handle hVip,
                     int32_t sigma2,
                     int32_t repf_motion2,
                     int32_t pcc,
                     BVDC_P_ITFP_vice_cd_fw_regs_t *fw,
                     int32_t *lock,
                     int32_t *repeat,
                     int32_t *phase,
                     bool InterlaceItfp) /*NS */
{
    int32_t sigma;
    int32_t repf_motion;
    int8_t cad_lock;
    int8_t cad_repeat;
    int8_t veto;
    int8_t cad_repeat2;
    int16_t i;
    int16_t j;
    int16_t k;
    uint8_t sigma_idx;
    uint8_t prev_sigma_idx;
    int32_t cad_fcnt;
    int32_t sigma_array[BVDC_P_ITFP_MAX_SIGMA];
    int32_t sigma2_array[BVDC_P_ITFP_MAX_SIGMA];
    int32_t cad_index;
    uint8_t  locked_phase = 0;
    BVDC_P_ITFP_cad_info_t *cad = hVip->vice_cd_cad;
    int32_t fcnt = hVip->vice_cd_fcnt;

    cad_fcnt = fcnt - 1;
    sigma_idx = (fcnt % BVDC_P_ITFP_MAX_SIGMA);
    prev_sigma_idx = ((cad_fcnt + BVDC_P_ITFP_MAX_SIGMA) % BVDC_P_ITFP_MAX_SIGMA);
    repf_motion = pcc;
    sigma = pcc;

    /* Is there a 3:2 lock? */
    for (j = 0; j < cad[0].control.length; ++j)
        if (GET_CAD_LOCKED(&(cad[0].state), j))
        {
            break;
        }


    cad_fcnt = fcnt;

    hVip->prev_sigma[sigma_idx] = sigma;
    hVip->prev_sigma2[sigma_idx] = sigma2 >> SIGMA_RS;

    *lock = 0;

    i = 0;

    /* Correlate sigmas & adjust phase counters - veto for repf_motion */
    for (i=BVDC_P_ITFP_NUM_CADENCE-1; i>=0; --i)   /* Traverse cadences in reverse order (lock priority is reversed) */
    {
        /* Update phase veto */
        for (j = 0; j < cad[i].control.length; j+=1)
        {
            cad_index = (cad_fcnt-j+cad[i].control.length)%cad[i].control.length;
            /*veto = -1; */
            veto = cad[i].state.InitialVeto;

            /*          if(cad[i].same_pol_expected[(fcnt+j)%cad[i].total_n]) */
            /* veto = (repf_motion > 0x4000); */
            /*if (!cad[i].pattern[j].hl && (cad[i].control.metric_sel == 1)) */
            if(!cad[i].pattern[j].hl)
            {
                veto = (repf_motion > fw->repf_veto_level);
            }
            if (!cad[i].pattern[(j+cad[i].control.length-1)%cad[i].control.length].t2 && (cad[i].control.metric_sel == 0))
            {
            if(veto==-1) veto = 0;
                veto |= (repf_motion2 > fw->repf2_veto_level);
            }
            /* if (( cad[i].pattern[(j+cad[i].control.length-1)%cad[i].control.length].pair ) && (i!=0)) */
            if (( cad[i].pattern[(j+cad[i].control.length-1)%cad[i].control.length].pair ))
            {
                if(veto==-1) veto = 0;
                if (hVip->prev_sigma[sigma_idx] > fw->pair_noise_thresh)
                {
                    veto |= (ABS(hVip->prev_sigma2[sigma_idx] - hVip->prev_sigma2[prev_sigma_idx]) > fw->pair_thresh) ||
                        (hVip->prev_sigma2[sigma_idx] * 16 > VICE_CD_FW_PAIR_RATIO * hVip->prev_sigma2[prev_sigma_idx]) ||
                        (hVip->prev_sigma2[prev_sigma_idx] * 16 > VICE_CD_FW_PAIR_RATIO * hVip->prev_sigma2[sigma_idx]);
                }
            }

            if(veto==1)
            {
                SET_CAD_VETO(&(cad[i].state), cad_index);
            }
            if(veto==0)
            {
                CLEAR_CAD_VETO(&(cad[i].state), cad_index);
            }  /* veto==-1 means do nothing */

        }

        /* Do correlation for each phase */
        /*printf("Cadence %d:\n", i); */
        if (i <= 1)
        {
            k = sigma_idx;
            for(j=cad[i].control.length-1; j>=0; --j)
            {
                sigma_array[j] = hVip->prev_sigma[k];
                sigma2_array[j] = hVip->prev_sigma2[k];
                k--;
                if (k<0) k=BVDC_P_ITFP_MAX_SIGMA-1;
            }
            locked_phase = BVDC_P_cadence_detection_isr(fw, sigma_array, sigma2_array, &(cad[i]), &(cad[i].state), cad_fcnt, &cad_lock, &cad_repeat, &cad_repeat2, ((!(i == 1)) && InterlaceItfp) );
        }
        /*      BVDC_P_cadence_detection_isr(prev_sigma, prev_sigma2, &(cad[i]), cad_fcnt, repf_motion, repf_motion2, &cad_lock, &cad_repeat, &cad_repeat2); */

        /* Select first lock we find */
        if (cad_lock && (*lock==0))
        {
            *lock = i + 1;
            *repeat = cad_repeat;
            *phase = locked_phase;
        }
    }

    if (*lock == 0)
    {
        *repeat = 0;
    }

    /* Still mode removed (not required) */

    /* Final decision - print result */
    BDBG_MSG(("Frame %d: lock=%d, repeat=%d, repeat2=%d", fcnt, *lock, *repeat, cad_repeat2));

}

static uint8_t BVDC_P_ITFP_Detection_Algorithm_isr(
    BVDC_P_Vip_Handle hVip,
    int8_t AllowDanglingFields,
    int8_t AllowBff22PullDown,
    BVDC_P_ItfpInfo_t *ItfpInfo,
    bool *pLocked32,
    bool IsTickerDetected
    ) /*NS */
{

    BVDC_P_ITFP_vice_cd_fw_regs_t vice_cd_fw;   /*!< Registers for firmware */
    int32_t i,j;
    int32_t fields_done = 2;
    int32_t lock, repeat,locked_phase;
    int32_t cur_stat;

    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);

    /*Initialize params */

    for (i = 0; i < BVDC_P_ITFP_NUM_CADENCE; i++)
    {
        for (j = 0; j < BVDC_P_ITFP_MAX_SIGMA; j++)
        {

            hVip->vice_cd_cad[0].pattern[j].hl = !((CAD_3_2_REP_PAT >> j) & 1);
            hVip->vice_cd_cad[0].pattern[j].t2 = !((CAD_3_2_R2_PAT >> j) & 1);
            hVip->vice_cd_cad[0].pattern[j].pair = ((CAD_3_2_PAIR_PAT >> j) & 1);

            hVip->vice_cd_cad[1].pattern[j].hl = !((CAD_2_2_REP_PAT >> j) & 1);
            hVip->vice_cd_cad[1].pattern[j].t2 = !((CAD_2_2_R2_PAT >> j) & 1);
            hVip->vice_cd_cad[1].pattern[j].pair = ((CAD_2_2_PAIR_PAT >> j) & 1);
        }
    }

    hVip->vice_cd_cad[0].control.length  = CAD_3_2_LENGTH;
    hVip->vice_cd_cad[1].control.length  = CAD_2_2_LENGTH;
    hVip->vice_cd_cad[0].control.metric_sel = 1;
    hVip->vice_cd_cad[1].control.metric_sel = 0;

    if ( IsTickerDetected == true )
    {
        hVip->vice_cd_cad[0].state.InitialVeto = 1;
        hVip->vice_cd_cad[1].state.InitialVeto = -1;
    }
    else
    {
        hVip->vice_cd_cad[0].state.InitialVeto = -1;
        hVip->vice_cd_cad[1].state.InitialVeto = -1;
    }


    for (i = 0; i < BVDC_P_ITFP_NUM_CADENCE; i++)
    {
        hVip->vice_cd_cad[i].control.enable  = 1;
    }

    vice_cd_fw.corr_dec_thresh_t1      = VICE_CD_FW_CORR_DEC_THRESH_T1_INT;
    vice_cd_fw.corr_zero_thresh_t1     = VICE_CD_FW_CORR_ZERO_THRESH_T1_INT;

    /*scale thresholds according to resolution */
    vice_cd_fw.pair_noise_thresh    = ((16 * VICE_CD_FW_PAIR_NOISE_THRESH) / 8160);
    vice_cd_fw.pair_thresh          = ((4*VICE_CD_FW_PAIR_THRESH)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.repf_veto_level      = ((4*VICE_CD_FW_REPF_VETO_LEVEL)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.repf2_veto_level     = ((4*VICE_CD_FW_REPF2_VETO_LEVEL)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.switch_pol_still_thresh = ((4*VICE_CD_FW_SWITCH_POL_STILL_THRESH)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.min_sigma_thresh     = ItfpInfo->NumberOfMbsPerFrame >> VICE_CD_FW_MAX_SIGMA_NOINC;

    vice_cd_fw.pair_noise_thresh    = ((vice_cd_fw.pair_noise_thresh*ItfpInfo->NumberOfMbsPerFrame) >> 4);
    vice_cd_fw.pair_thresh          = ((vice_cd_fw.pair_thresh*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.repf_veto_level      = ((vice_cd_fw.repf_veto_level*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.repf2_veto_level     = ((vice_cd_fw.repf2_veto_level*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.switch_pol_still_thresh     = ((vice_cd_fw.switch_pol_still_thresh*ItfpInfo->NumberOfMbsPerFrame)>>2);

    /* TBD: If AllowDanglingFields is false, try to switch field polarity as needed using repeats when possible */

    lock=repeat=0;

    /* Assume video */
    ItfpInfo->Cadence[0] = BVDC_P_ITFP_CADENCE_FIELD;
    ItfpInfo->FieldOrder[0] = BVDC_P_ITFP_FIRST_FIELD;
    ItfpInfo->Cadence[1] = BVDC_P_ITFP_CADENCE_FIELD;
    ItfpInfo->FieldOrder[1] = BVDC_P_ITFP_SECOND_FIELD;
    ItfpInfo->Cadence[2] = BVDC_P_ITFP_CADENCE_UNKNOWN;
    ItfpInfo->FieldOrder[2] = BVDC_P_ITFP_FIRST_FIELD;
    ItfpInfo->Cadence[3] = BVDC_P_ITFP_CADENCE_UNKNOWN;
    ItfpInfo->FieldOrder[3] = BVDC_P_ITFP_FIRST_FIELD;

    for(i = 0; (i < hVip->cad_control.stats_to_load); ++i)
    {
        cur_stat = MAX(0 , i + MAX_STATS_TO_PROCESS - hVip->cad_control.stats_to_load);
        hVip->vice_cd_fcnt++;
        if (hVip->vice_cd_fcnt >= CAD_FLD_CNT_MAX)
        {
            hVip->vice_cd_fcnt = 0;
        }
        BVDC_P_compute_cadence_isr(hVip,
                        ItfpInfo->Sigma[cur_stat],
                        ItfpInfo->Repf_motion[cur_stat],
                        (int)ItfpInfo->Pcc[cur_stat],
                        &vice_cd_fw, &lock, &repeat , &locked_phase , 1 );

        if(hVip->cad_control.found_32)
        {

            *pLocked32 = true;
            ItfpInfo->FieldOrder[cur_stat-2] = BVDC_P_ITFP_FIRST_FIELD;
            ItfpInfo->FieldOrder[cur_stat-1] = BVDC_P_ITFP_SECOND_FIELD;
            if(ItfpInfo->Polarity[cur_stat])  /* BFF */
            {
                ItfpInfo->Cadence[cur_stat-2] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
                ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
            }
            else
            {
                ItfpInfo->Cadence[cur_stat-2] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
                ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
            }
            if((!lock) || (!repeat))
            {
                fields_done = cur_stat;  /* i + 3 - stats_to_load + 1; */
            }
            else
            {
                ItfpInfo->Cadence[cur_stat] = BVDC_P_ITFP_CADENCE_REPEATED_FIELD;
                ItfpInfo->FieldOrder[cur_stat] = BVDC_P_ITFP_THIRD_FIELD;
                fields_done = cur_stat + 1;  /*i + 4 - stats_to_load + 1; */
                hVip->cad_control.using_bff = !hVip->cad_control.using_bff;
            }
            hVip->cad_control.found_32 = 0;
        }
        else if(hVip->cad_control.try_to_switch_pol && (cur_stat-2 >= 0) && (hVip->cad_control.using_bff == ItfpInfo->Polarity[cur_stat]))
        {
            if ((vice_cd_fw.switch_pol_still_thresh >= 0) && (ItfpInfo->Repf_motion[cur_stat] < (uint32_t) vice_cd_fw.switch_pol_still_thresh))
            {
                ItfpInfo->FieldOrder[cur_stat-2] = BVDC_P_ITFP_FIRST_FIELD;
                ItfpInfo->FieldOrder[cur_stat-1] = BVDC_P_ITFP_SECOND_FIELD;
                if(ItfpInfo->Polarity[cur_stat])  /* BFF */
                {
                    ItfpInfo->Cadence[cur_stat-2] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
                    ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
                } else {
                    ItfpInfo->Cadence[cur_stat-2] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
                    ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
                }
                ItfpInfo->Cadence[cur_stat] = BVDC_P_ITFP_CADENCE_REPEATED_FIELD;
                ItfpInfo->FieldOrder[cur_stat] = BVDC_P_ITFP_THIRD_FIELD;
                fields_done = cur_stat+1; /*i + 4 - stats_to_load + 1 */
                hVip->cad_control.using_bff = !hVip->cad_control.using_bff;
                hVip->cad_control.try_to_switch_pol = 0;
            }
        }
        else if(lock)
        {
            if(repeat && (cur_stat > 0))   /* Looking for a repeat along with the ability to annotate the previous field */
            {
                if((lock==2) &&
                   (AllowDanglingFields || (hVip->cad_control.using_bff != ItfpInfo->Polarity[cur_stat])))   /* 22pd */
                {
                    if(ItfpInfo->Polarity[cur_stat])   /* TFF */
                    {
                        ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
                        ItfpInfo->Cadence[cur_stat] = BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR;
                        ItfpInfo->FieldOrder[cur_stat-1] = BVDC_P_ITFP_FIRST_FIELD;
                        ItfpInfo->FieldOrder[cur_stat] = BVDC_P_ITFP_SECOND_FIELD;
                        fields_done = cur_stat + 1;  /*i + 4 - stats_to_load + 1 */
                    }
                    else if(AllowBff22PullDown)     /* BFF */
                    {
                        ItfpInfo->Cadence[cur_stat-1] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
                        ItfpInfo->Cadence[cur_stat] = BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR;
                        ItfpInfo->FieldOrder[cur_stat-1] = BVDC_P_ITFP_FIRST_FIELD;
                        ItfpInfo->FieldOrder[cur_stat] = BVDC_P_ITFP_SECOND_FIELD;
                        fields_done = cur_stat + 1;  /* i + 4 - stats_to_load + 1 */
                    }
                }
                else if((lock==2) && (!AllowDanglingFields))        /* Wrong phase 2:2 */
                {
                    hVip->cad_control.try_to_switch_pol = 1;
                }
                else/* 32pd (or others if enabled */
                {
                    if(repeat && (cur_stat > 0))
                    {
                        if(AllowDanglingFields ||
                           (hVip->cad_control.using_bff != ItfpInfo->Polarity[cur_stat])) /* Dangling field preventer */
                        {
                            hVip->cad_control.found_32 = 1;
                        }
                    }
                }
            }
            else
                hVip->cad_control.found_32 = 0;
        }
        else
        {
            hVip->cad_control.found_32 = 0;
            hVip->cad_control.try_to_switch_pol = 0;
        }

    }

    hVip->cad_control.stats_to_load = fields_done;

    return(fields_done);
}


static void BVDC_P_ITFP_Reset_Detection_isrsafe(
    BVDC_P_Vip_Handle hVip,
    int8_t startPolarity)
{
    int j;

    hVip->vice_cd_fcnt = 0;
    hVip->cad_control.stats_to_load = MAX_STATS_TO_PROCESS;
    hVip->cad_control.found_32 = 0;
    hVip->cad_control.using_bff = startPolarity;
    hVip->cad_control.try_to_switch_pol=0;

    for (j = 0; j < BVDC_P_ITFP_MAX_SIGMA; j++)
    {
        hVip->vice_cd_cad[0].state.phase_counter[j] = 0;
        hVip->vice_cd_cad[1].state.phase_counter[j] = 0;
    }

    hVip->vice_cd_cad[0].state.enter_ll  = CAD_LOCK_ENTER;
    hVip->vice_cd_cad[0].state.exit_ll   = CAD_LOCK_EXIT;
    hVip->vice_cd_cad[0].state.lock_sat_level  = CAD_LOCK_SAT;

    hVip->vice_cd_cad[1].state.enter_ll = CAD_LOCK_ENTER*2;
    hVip->vice_cd_cad[1].state.exit_ll = CAD_LOCK_ENTER * 2 - 1;
    hVip->vice_cd_cad[1].state.lock_sat_level = CAD_LOCK_SAT;

    for (j = 0; j < BVDC_P_ITFP_MAX_SIGMA; ++j)
    {
        hVip->prev_sigma[j] = 0;
        hVip->prev_sigma2[j] = 0;
    }

    hVip->prev1VLumaOffset = 0;
    hVip->prevPrev1VLumaOffset = 0;
}



static void BVDC_P_Progressive_ITFP_Detection_Algorithm_isr(
    BVDC_P_Vip_Handle hVip,
    BVDC_P_ITFP_ProgressiveItfpInfo_t *ItfpInfo,
    bool Phase0Sync,
    bool IsTickerDetected
                              )
{
    int32_t lock, repeat, phase;
    BVDC_P_ITFP_vice_cd_fw_regs_t vice_cd_fw;   /*!< Registers for firmware */

    /*Initialize params */
    int i, j;


    for (i=0; i<BVDC_P_ITFP_NUM_CADENCE; i+=1)
    {
        for (j=0; j<BVDC_P_ITFP_MAX_SIGMA; j+=1)
        {
            hVip->vice_cd_cad[0].pattern[j].hl = !((PCAD_3_2_REP_PAT >> j) & 1);
            hVip->vice_cd_cad[0].pattern[j].t2 = !((PCAD_3_2_R2_PAT >> j) & 1);
            hVip->vice_cd_cad[0].pattern[j].pair = ((PCAD_3_2_PAIR_PAT >> j) & 1);

            hVip->vice_cd_cad[1].pattern[j].hl = !((PCAD_2_2_REP_PAT >> j) & 1);
            hVip->vice_cd_cad[1].pattern[j].t2 = !((PCAD_2_2_R2_PAT >> j) & 1);
            hVip->vice_cd_cad[1].pattern[j].pair = ((PCAD_2_2_PAIR_PAT >> j) & 1);
        }

        hVip->vice_cd_cad[0].control.enable         = 1;
        hVip->vice_cd_cad[0].control.length         = PCAD_3_2_LENGTH;
        hVip->vice_cd_cad[0].control.metric_sel = 1;

        hVip->vice_cd_cad[1].control.enable         = 1;
        hVip->vice_cd_cad[1].control.length         = PCAD_2_2_LENGTH;
        hVip->vice_cd_cad[1].control.metric_sel = 1;


        if ( IsTickerDetected == true )
        {
         hVip->vice_cd_cad[0].state.InitialVeto = 1;
         hVip->vice_cd_cad[1].state.InitialVeto = 1;
        }
        else
        {
         hVip->vice_cd_cad[0].state.InitialVeto = -1;
         hVip->vice_cd_cad[1].state.InitialVeto = -1;
        }
    }



    /*///////////////// */

    /* Tune these for progressive: */
    /*vice_cd_fw.corr_inc_thresh      = 750; */
    /*vice_cd_fw.corr_dec_thresh      = 600; */
    /*vice_cd_fw.corr_zero_thresh     = 400; */
    /*vice_cd_fw.corr_inc_thresh_t1      = 600; */
    vice_cd_fw.corr_dec_thresh_t1      = VICE_CD_FW_CORR_DEC_THRESH_T1_PROG;
    vice_cd_fw.corr_zero_thresh_t1     = VICE_CD_FW_CORR_ZERO_THRESH_T1_PROG;
    /*vice_cd_fw.corr_inc_sel         = 1; */
    /*vice_cd_fw.corr_dec_sel         = 0; */
    /*vice_cd_fw.corr_zero_sel        = 0; */
    /*vice_cd_fw.max_sigma_noinc      = 0; */
    vice_cd_fw.pair_thresh          = 0x2000;
    /*vice_cd_fw.pair_ratio           = 30; */
    vice_cd_fw.repf_veto_level      = 14000;
    vice_cd_fw.repf2_veto_level     = 0x4000;
    /*vice_cd_fw.extra_repeat_ratio   = 8; */
    /*vice_cd_fw.missing_repeat_ratio = 7; */
    /*vice_cd_fw.repf_extra_repeat_thresh = 1750; */
    vice_cd_fw.switch_pol_still_thresh = 0x2000;

    vice_cd_fw.min_sigma_thresh     = 0;

    /*scale thresholds according to resolution */
    vice_cd_fw.pair_noise_thresh    = ((16 * VICE_CD_FW_PAIR_NOISE_THRESH) / 8160);
    vice_cd_fw.pair_thresh          = ((4*VICE_CD_FW_PAIR_THRESH)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.repf_veto_level      = ((4*VICE_CD_FW_REPF_VETO_LEVEL)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.repf2_veto_level     = ((4*VICE_CD_FW_REPF2_VETO_LEVEL)/1350);/*this threshold should be scaled according to resolution */
    vice_cd_fw.switch_pol_still_thresh = ((4*VICE_CD_FW_SWITCH_POL_STILL_THRESH)/1350);/*this threshold should be scaled according to resolution */

    vice_cd_fw.pair_noise_thresh    = ((vice_cd_fw.pair_noise_thresh*ItfpInfo->NumberOfMbsPerFrame) >> 4);
    vice_cd_fw.pair_thresh          = ((vice_cd_fw.pair_thresh*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.repf_veto_level      = ((vice_cd_fw.repf_veto_level*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.repf2_veto_level     = ((vice_cd_fw.repf2_veto_level*ItfpInfo->NumberOfMbsPerFrame)>>2);
    vice_cd_fw.switch_pol_still_thresh     = ((vice_cd_fw.switch_pol_still_thresh*ItfpInfo->NumberOfMbsPerFrame)>>2);

    /*/Added by Yehuda */
        hVip->vice_cd_fcnt++;
        if (hVip->vice_cd_fcnt >= CAD_FLD_CNT_MAX)
        {
            hVip->vice_cd_fcnt = 0;
        }

     if ( hVip->vice_cd_cad[0].state.LockCntr32 < PCAD_3_2_CNTR_THR )
       hVip->vice_cd_cad[0].state.enter_ll       = CAD_LOCK_ENTER;
     else
       hVip->vice_cd_cad[0].state.enter_ll       = CAD_LOCK_ENTER*3;

    BVDC_P_compute_cadence_isr(hVip,
                    ItfpInfo->Sigma,
                    ItfpInfo->Repf_motion,
                    0,
                    &vice_cd_fw, &lock, &repeat, &phase , 0);

    /* Assume video */
    ItfpInfo->ProgressiveCadence = BAVC_CadenceType_eUnlocked;
    ItfpInfo->ProgressivePhase = BAVC_CadencePhase_eMax;
    if(lock)
    {
        if(lock==2)
            phase = phase % 2;
        ItfpInfo->ProgressivePhase = BAVC_CadencePhase_e0 + phase;
        ItfpInfo->ProgressiveCadence = (BAVC_CadenceType)lock;
    }

    if (ItfpInfo->ProgressiveCadence == BAVC_CadenceType_e3_2)
    {
        hVip->vice_cd_cad[0].state.LockCntr32 = 0;
        if (ItfpInfo->ProgressivePhase == BAVC_CadencePhase_e0 )
            hVip->vice_cd_cad[0].state.Locked32Phase0Synced = 1;

        if ( ( hVip->vice_cd_cad[0].state.Locked32Phase0Synced == 0 ) && (Phase0Sync == 1) )
            ItfpInfo->ProgressiveCadence = BAVC_CadenceType_eUnlocked;
    }
    else
    {
        hVip->vice_cd_cad[0].state.LockCntr32++;
        hVip->vice_cd_cad[0].state.LockCntr32 = MIN( hVip->vice_cd_cad[0].state.LockCntr32 , PCAD_3_2_CNTR_SAT );
        hVip->vice_cd_cad[0].state.Locked32Phase0Synced = 0;
    }
}

static void BVDC_P_Progressive_ITFP_Reset_Detection_isrsafe(BVDC_P_Vip_Handle hVip)
{

    BVDC_P_ITFP_Reset_Detection_isrsafe(hVip, 0);

    hVip->vice_cd_cad[0].state.enter_ll       = CAD_LOCK_ENTER;/**3; */
    hVip->vice_cd_cad[0].state.exit_ll        = CAD_LOCK_EXIT;
    hVip->vice_cd_cad[0].state.lock_sat_level = CAD_LOCK_SAT;
    hVip->vice_cd_cad[0].state.LockCntr32 = PCAD_3_2_CNTR_SAT;
    hVip->vice_cd_cad[0].state.Locked32Phase0Synced = 0;


    hVip->vice_cd_cad[1].state.enter_ll       = CAD_LOCK_ENTER*5;
    hVip->vice_cd_cad[1].state.exit_ll        = hVip->vice_cd_cad[1].state.enter_ll - 1;
    hVip->vice_cd_cad[1].state.lock_sat_level = CAD_LOCK_SAT;
    hVip->vice_cd_cad[1].state.LockCntr32 = PCAD_3_2_CNTR_SAT;
    hVip->vice_cd_cad[1].state.Locked32Phase0Synced = 0;

}

void BVDC_P_ITFP_EPM_Preprocessor_ReadVipStats_isr(BVDC_P_Vip_Handle hVip)
{
    uint32_t Histogram, Histogram0, Histogram1, Histogram2, Histogram3, Histogram4, PccSingle;
    uint8_t VipStatsDecimType;
    int8_t WrPtr_PictureQueue, WrPtr;
    uint32_t PrvOpts, PrvPrvOpts;
    BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo;
    unsigned PicWidth = hVip->pCapture->stPicture.ulWidth;
    unsigned Hist4Threshold = ((PicWidth + 3)>>2) << hVip->stEpmInfo.IsProgressive;

    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);
    pEpmPreprocessorInfo = &hVip->stEpmInfo;

    WrPtr_PictureQueue = pEpmPreprocessorInfo->PreProcessorPictureQueue.WrPtr;

    BDBG_MODULE_MSG(BVDC_ITFP_Q,("Wr[%d]Rd[%d] Fullness=%u, IsStatsSet?%d", WrPtr_PictureQueue, pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr,
        pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness,
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].IsVipStatsSet));

    BDBG_ASSERT(pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].IsVipStatsSet!=true);

    /* 1H must have both width and height within the limit; TODO: how about ViCE2? */
    VipStatsDecimType =
        ((hVip->pCapture->stPicture.ulWidth  > BVDC_P_VIP_MAX_H1V_WIDTH) ||
         (hVip->pCapture->stPicture.ulHeight > BVDC_P_VIP_MAX_H1V_HEIGHT)) ?
    #if BVDC_P_VIP_VER == 0
         1 : 1; /* always 2H */
    #elif BVDC_P_VIP_VER == 1
         2 : 1; /* 4H vs 2H */
    #else
         1 : 0; /* 2H vs 1H */
    #endif
    /*read the values of histogram/pcc */
    Histogram0 = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(HIST_STATUS_0)) << VipStatsDecimType;
    Histogram1 = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(HIST_STATUS_1)) << VipStatsDecimType;
    Histogram2 = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(HIST_STATUS_2)) << VipStatsDecimType;
    Histogram3 = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(HIST_STATUS_3)) << VipStatsDecimType;
    Histogram4 = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(HIST_STATUS_4)) << VipStatsDecimType;
    PccSingle  = BREG_Read32_isr(hVip->hDisplay->hVdc->hRegister, hVip->ulRegOffset+BVDC_P_VIP_RDB(SINGLE_PCC_STATUS)) << VipStatsDecimType;

    Histogram = Histogram1 + (Histogram2 << 1) + (Histogram3 << 2) + (Histogram4 << 3);
    BDBG_MSG(("Histogram = %u", Histogram));

    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].PCC = PccSingle;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].Histogram = Histogram;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].Sigma = (Histogram << 1) + Histogram0;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].Polarity = hVip->pCapture->stPicture.ePolarity;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].IsVipStatsSet = true;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].ulOrigPts = hVip->pCapture->stPicture.ulOriginalPTS;

    /* ticker detection */
    if(Histogram4 > Hist4Threshold) {
        pEpmPreprocessorInfo->TickerDetectionCounter++;
    } else {
        pEpmPreprocessorInfo->TickerDetectionCounter = 0;
    }
    pEpmPreprocessorInfo->TickerDetectionCounter = MIN(pEpmPreprocessorInfo->TickerDetectionCounter, BVDC_P_ITFP_MAX_TICKER_DETECTION_CNT);

    WrPtr = BVDC_P_EPM_mod_isrsafe(WrPtr_PictureQueue - 1, pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
    PrvOpts = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr].ulOrigPts;
    if (hVip->pCapture->stPicture.ulOriginalPTS == PrvOpts)
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].OptsDelta1pic= 0;
    else
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].OptsDelta1pic = 1;

    WrPtr = BVDC_P_EPM_mod_isrsafe(WrPtr - 1, pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
    PrvPrvOpts = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr].ulOrigPts;
    if (hVip->pCapture->stPicture.ulOriginalPTS == PrvPrvOpts)
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].OptsDelta2pic = 0;
    else
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].OptsDelta2pic = 1;

    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].OptsDeltaUsed = 0;

    /* propagate pCapture to ITFP pic buffer Q */
    hVip->astItfpPicQ[WrPtr_PictureQueue].pPic     = hVip->pCapture;
    hVip->astItfpPicQ[WrPtr_PictureQueue].pDecim1v = hVip->pCaptureDecim1v;
    hVip->astItfpPicQ[WrPtr_PictureQueue].pDecim2v = hVip->pCaptureDecim2v;
    hVip->astItfpPicQ[WrPtr_PictureQueue].pChroma  = hVip->pCaptureChroma;
    hVip->astItfpPicQ[WrPtr_PictureQueue].pShifted = hVip->pCaptureShifted;
    /* zero out pCapture to defer the delivery until ITFP delay is satisfied */
    hVip->pCapture = NULL;

    /* advance itfp Q write pointer and fullness */
    pEpmPreprocessorInfo->PreProcessorPictureQueue.WrPtr = BVDC_P_EPM_mod_isrsafe((pEpmPreprocessorInfo->PreProcessorPictureQueue.WrPtr + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
    pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness++;
    BDBG_MSG(("===SIGMA=%d PCC=%d, ticker=%u, Histogram4=%u(thresh=%u)", pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[WrPtr_PictureQueue].Sigma,
        PccSingle, pEpmPreprocessorInfo->TickerDetectionCounter, Histogram4, Hist4Threshold));

    return;
}

static bool BVDC_P_ITFP_EPM_Preprocessor_OptsITFP_isr(
    BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo,
    uint8_t *OptsDelta1,
    uint8_t *OptsDelta2)
{
    #define OPTS_LOCK_CNTR_THR 2

    if (*OptsDelta1 == 1)
    {
        if ((pEpmPreprocessorInfo->OptsItfpPhase == 0) || (pEpmPreprocessorInfo->OptsItfpPhase == 3) || (pEpmPreprocessorInfo->OptsItfpPhase == 5))
            pEpmPreprocessorInfo->OptsItfpPhase++;
        else
        {
            pEpmPreprocessorInfo->OptsPattern32Cntr = 0;
                pEpmPreprocessorInfo->OptsItfpPhase = 1;
        }
    }
    else
    {
        if (
            ((pEpmPreprocessorInfo->OptsItfpPhase == 1) || (pEpmPreprocessorInfo->OptsItfpPhase == 2) || (pEpmPreprocessorInfo->OptsItfpPhase == 4)) ||
            ((pEpmPreprocessorInfo->OptsPattern32Cntr>=OPTS_LOCK_CNTR_THR) && ((pEpmPreprocessorInfo->OptsItfpPhase == 3) || (pEpmPreprocessorInfo->OptsItfpPhase == 5)))
            )
            pEpmPreprocessorInfo->OptsItfpPhase++;
        else
        {
            pEpmPreprocessorInfo->OptsPattern32Cntr = 0;
            pEpmPreprocessorInfo->OptsItfpPhase = 0;
        }
    }



    if (pEpmPreprocessorInfo->OptsItfpPhase > 5)
    {
        pEpmPreprocessorInfo->OptsPattern32Cntr++;
        pEpmPreprocessorInfo->OptsPattern32Cntr = MIN(pEpmPreprocessorInfo->OptsPattern32Cntr, 3);
        pEpmPreprocessorInfo->OptsItfpPhase = 1;
    }


    if (pEpmPreprocessorInfo->OptsPattern32Cntr >= OPTS_LOCK_CNTR_THR)
    {
        switch (pEpmPreprocessorInfo->OptsItfpPhase)
        {
        case 1:
            *OptsDelta1 = 1;
            *OptsDelta2 = 1;
            break;
        case 2:
            *OptsDelta1 = 0;
            *OptsDelta2 = 1;
            break;
        case 3:
            *OptsDelta1 = 0;
            *OptsDelta2 = 0;
            break;
        case 4:
            *OptsDelta1 = 1;
            *OptsDelta2 = 1;
            break;
        case 5:
            *OptsDelta1 = 0;
            *OptsDelta2 = 1;
            break;
        }
    }

    return(pEpmPreprocessorInfo->OptsPattern32Cntr >= OPTS_LOCK_CNTR_THR);

}

void BVDC_P_ITFP_EPM_Preprocessor_CadenceDetection_isr(   BVDC_P_Vip_Handle hVip)
{

    int8_t RdPtr_PictureQueue, RdPtr, NextRdPtr;
    BAVC_CadenceType  CadenceType;
    BAVC_CadencePhase  CadencePhase;
    uint16_t PictureWidthInMb, FrameHeightInMb;
    uint32_t NumberOfMbsPerFrame;
    uint8_t ItfpDelay = 0;
    uint8_t EnableItfp;
    uint8_t PictureQueueFullness;
    bool bForceItfpUnlock, bTickerDetected;
    BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo;

    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);
    pEpmPreprocessorInfo = &hVip->stEpmInfo;
    BDBG_ASSERT(pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness);

    RdPtr_PictureQueue = pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr;
    PictureQueueFullness = pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness;
    BDBG_MODULE_MSG(BVDC_ITFP_Q,("Rd[%d][%c]: vFreq=%d, PictureQueueFullness=%u, CadSet?%d", RdPtr_PictureQueue,
        pEpmPreprocessorInfo->IsProgressive? 'p':'i',
        hVip->hDisplay->stCurInfo.ulVertFreq,
        PictureQueueFullness,
        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].IsCadenceSet));

    while (
        (PictureQueueFullness > 0) &&
        (pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].IsCadenceSet == true)
        )
    {
        RdPtr_PictureQueue = BVDC_P_EPM_mod_isrsafe(RdPtr_PictureQueue + 1, pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
        PictureQueueFullness--;
    }

    PictureWidthInMb = hVip->astItfpPicQ[RdPtr_PictureQueue].pPic->stPicture.ulWidthInMbs;
    FrameHeightInMb  = hVip->astItfpPicQ[RdPtr_PictureQueue].pPic->stPicture.ulHeightInMbs;
    NumberOfMbsPerFrame = PictureWidthInMb * FrameHeightInMb;

    bTickerDetected = (BVDC_P_ITFP_MAX_TICKER_DETECTION_CNT == pEpmPreprocessorInfo->TickerDetectionCounter);
    if (pEpmPreprocessorInfo->IsProgressive)
    {

        RdPtr = RdPtr_PictureQueue;
        if (
            (pEpmPreprocessorInfo->IsItfpEnabled == 1) &&
            /*(hVip->EncoderModeOfOperation != LOW_DELAY_MODE) &&*/ /* TODO: add low delay mode support */
            (hVip->hDisplay->stCurInfo.ulVertFreq > 3000) /*>= ENCODING_FRAME_RATE_5000 */
            )
        {
            EnableItfp = 1;
        }
        else
        {
            EnableItfp = 0;
        }

        ItfpDelay = EnableItfp;

        while (PictureQueueFullness > ItfpDelay)
        {
            BVDC_P_ITFP_ProgressiveItfpInfo_t ItfpInfo;

            NextRdPtr = BVDC_P_EPM_mod_isrsafe(RdPtr + ItfpDelay, pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
            ItfpInfo.NumberOfMbsPerFrame = NumberOfMbsPerFrame;
            ItfpInfo.Repf_motion = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[NextRdPtr].Histogram;
            ItfpInfo.Sigma = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[NextRdPtr].Sigma;

            BDBG_MSG(("======================"));
            BDBG_MSG(("Sigma      [-1] = %u", pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Sigma));
            BDBG_MSG(("Repf_motion[-1] = %u", pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Histogram));
            BDBG_MSG(("Sigma       = %u", ItfpInfo.Sigma));
            BDBG_MSG(("Repf_motion = %u", ItfpInfo.Repf_motion));
            BDBG_MSG(("======================"));
            if (EnableItfp)
            {
                BVDC_P_Progressive_ITFP_Detection_Algorithm_isr(hVip, &ItfpInfo, 0, bTickerDetected);
            }
            else
            {
                ItfpInfo.ProgressiveCadence = BAVC_CadenceType_eMax;
                BVDC_P_Progressive_ITFP_Reset_Detection_isrsafe(hVip);
            }

            /* adjust cadence phase for picture(t-1) */
            switch (ItfpInfo.ProgressiveCadence)
            {
            case BAVC_CadenceType_e3_2:
                CadenceType = BAVC_CadenceType_e3_2;
                CadencePhase = BVDC_P_EPM_mod_isrsafe(ItfpInfo.ProgressivePhase - ItfpDelay, BAVC_CadencePhase_eMax);
                break;
            case BAVC_CadenceType_e2_2:
                CadenceType = BAVC_CadenceType_e2_2;
                CadencePhase = BVDC_P_EPM_mod_isrsafe(ItfpInfo.ProgressivePhase - ItfpDelay, BAVC_CadencePhase_e2);
                break;
            default:
                CadenceType = BAVC_CadenceType_eUnlocked;
                CadencePhase = BAVC_CadencePhase_e0;

            }

            hVip->astItfpPicQ[RdPtr].pPic->stPicture.stCadence.type  = CadenceType;
            hVip->astItfpPicQ[RdPtr].pPic->stPicture.stCadence.phase = CadencePhase;
            pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].IsCadenceSet = true;

            RdPtr = BVDC_P_EPM_mod_isrsafe(RdPtr + 1, pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
            PictureQueueFullness--;
        }
    }
    else
    {
        ItfpDelay = 3;
        if (PictureQueueFullness > ItfpDelay)
        {
            uint8_t Idx;
            uint8_t NumberOfFieldsWithDeterminedCadence;
            BVDC_P_ItfpInfo_t ItfpInfo;
            bool Locked32;
            bool OptsLocked32 = false;
            uint8_t OptsDelta1, OptsDelta2;

            /*fill ItfpInfo */
            BDBG_MSG(("======================"));

            /*run Opts ITFP */
            RdPtr = RdPtr_PictureQueue;
            for (Idx = 0; Idx < 4; Idx++)
            {
                if (!pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].OptsDeltaUsed)
                {
                    pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].OptsDeltaUsed = 1;
                    OptsDelta1 = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].OptsDelta1pic;
                    OptsDelta2 = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].OptsDelta2pic;

                    BDBG_MSG(("before: PCC=%d Hist=%d", pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].PCC, pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Sigma));
                    BDBG_MSG(("before: Delta1=%d Delta2=%d", OptsDelta1, OptsDelta2));
                    OptsLocked32 = BVDC_P_ITFP_EPM_Preprocessor_OptsITFP_isr(pEpmPreprocessorInfo, &OptsDelta1, &OptsDelta2);
                    BDBG_MSG(("after: Delta1=%d Delta2=%d OptsLocked32=%d", OptsDelta1, OptsDelta2, OptsLocked32));
                    if (OptsLocked32)
                    {
                        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Sigma = OptsDelta2*100000;
                        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Histogram = OptsDelta2*100000;
                        pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].PCC = OptsDelta1*100000;

                    }
                    BDBG_MSG(("after: PCC=%d Hist=%d", pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].PCC, pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Sigma));
                }
                RdPtr = BVDC_P_EPM_mod_isrsafe((RdPtr + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
            }

            /* ticker detection */
            bForceItfpUnlock = (bTickerDetected && !OptsLocked32);
            if(!OptsLocked32 && pEpmPreprocessorInfo->bPrevOptsLocked32) {
                bForceItfpUnlock = true;
            }
            pEpmPreprocessorInfo->bPrevOptsLocked32 = OptsLocked32;

            RdPtr = RdPtr_PictureQueue;
            for (Idx = 0; Idx < 4; Idx++)
            {
                ItfpInfo.Sigma[Idx] = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Sigma;
                ItfpInfo.Repf_motion[Idx] = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Histogram;
                ItfpInfo.Pcc[Idx] = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].PCC;
                ItfpInfo.Polarity[Idx] = pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].Polarity;

                RdPtr = BVDC_P_EPM_mod_isrsafe((RdPtr + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);

                BDBG_MSG(("Sigma = %d", ItfpInfo.Sigma[Idx]));
                BDBG_MSG(("Repf_motion = %d", ItfpInfo.Repf_motion[Idx]));
                BDBG_MSG(("Pcc = %d", ItfpInfo.Pcc[Idx]));
                BDBG_MSG(("Polarity = %d", ItfpInfo.Polarity[Idx]));
            }

            BDBG_MSG(("======================"));

            ItfpInfo.NumberOfMbsPerFrame = NumberOfMbsPerFrame;

            Locked32 = false;
            NumberOfFieldsWithDeterminedCadence = BVDC_P_ITFP_Detection_Algorithm_isr(hVip, 0, 0, &ItfpInfo, &Locked32, bForceItfpUnlock);

            BDBG_MSG(("NoOfFields=%d Lock = %d, bForceItfpUnlock=%d", NumberOfFieldsWithDeterminedCadence,Locked32, bForceItfpUnlock));
            {
                uint8_t idx;
                for (idx = 0; idx < NumberOfFieldsWithDeterminedCadence; idx++)
                {
                    BDBG_MSG(("FieldOrder=%d  Cadence=%d", ItfpInfo.FieldOrder[idx] , ItfpInfo.Cadence[idx]));
                }
            }

            CadenceType = BAVC_CadenceType_eUnlocked;
            CadencePhase = BAVC_CadencePhase_e0;

            if (NumberOfFieldsWithDeterminedCadence == 3)
            {
                CadenceType = BAVC_CadenceType_e3_2;
                CadencePhase = BAVC_CadencePhase_e0;
            }

            if (
                (NumberOfFieldsWithDeterminedCadence == 2) &&
                (
                (ItfpInfo.Cadence[0] == BVDC_P_ITFP_CADENCE_TFF_FIELD_PAIR) ||
                    (ItfpInfo.Cadence[0] == BVDC_P_ITFP_CADENCE_BFF_FIELD_PAIR)
                    )
                )
            {
                if (Locked32)
                {
                    CadenceType = BAVC_CadenceType_e3_2;
                    CadencePhase = BAVC_CadencePhase_e3;
                }
                else
                {
                    CadenceType = BAVC_CadenceType_e2_2;
                }
            }

            RdPtr = RdPtr_PictureQueue;
            for (Idx = 0; Idx < NumberOfFieldsWithDeterminedCadence; Idx++)
            {
                /* update the cadence info to the just captured picture */
                hVip->astItfpPicQ[RdPtr].pPic->stPicture.stCadence.type  = CadenceType;
                hVip->astItfpPicQ[RdPtr].pPic->stPicture.stCadence.phase = CadencePhase;
                pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr].IsCadenceSet = true;
                RdPtr = BVDC_P_EPM_mod_isrsafe((RdPtr + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
                if (CadenceType != BAVC_CadenceType_eUnlocked) {
                    CadencePhase = BVDC_P_EPM_mod_isrsafe(CadencePhase + 1, (BAVC_CadenceType_e3_2 == CadenceType)? 5 : 2);
                }
            }
        }
    }

    RdPtr_PictureQueue = pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr;
    /* This will delay the VIP picture delivery due to ITFP delay */
    if(pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness > ItfpDelay)
    {
        hVip->pCapture        = hVip->astItfpPicQ[RdPtr_PictureQueue].pPic;
        hVip->pCaptureDecim1v = hVip->astItfpPicQ[RdPtr_PictureQueue].pDecim1v;
        hVip->pCaptureDecim2v = hVip->astItfpPicQ[RdPtr_PictureQueue].pDecim2v;
        hVip->pCaptureChroma  = hVip->astItfpPicQ[RdPtr_PictureQueue].pChroma;
        hVip->pCaptureShifted = hVip->astItfpPicQ[RdPtr_PictureQueue].pShifted;
        BDBG_MSG(("R[%d] cadence: type=%d, phase=%d,! bTickerDetected=%d", RdPtr_PictureQueue, hVip->pCapture->stPicture.stCadence.type,
            hVip->pCapture->stPicture.stCadence.phase, bTickerDetected));
    }

    /* clear the stats Q entry and move read pointer after it's consumed */
    if(pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].IsCadenceSet) {
        BDBG_MODULE_MSG(BVDC_ITFP_Q,("R[%d] clears flags!", RdPtr_PictureQueue));
        BKNI_Memset_isr(&pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue], 0, sizeof(BVDC_P_ITFP_PreProcessorVipBuffers_t));
        BKNI_Memset_isr(&hVip->astItfpPicQ[RdPtr_PictureQueue], 0, sizeof(hVip->astItfpPicQ[RdPtr_PictureQueue]));
        pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr = BVDC_P_EPM_mod_isrsafe((pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
        pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness--;
    }
    return;
}

void BVDC_P_ITFP_EPM_Preprocessor_init(    BVDC_P_Vip_Handle hVip)
{
    BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo;

    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);
    pEpmPreprocessorInfo = &hVip->stEpmInfo;

    if (hVip->stMemSettings.bInterlaced)
    {
        BVDC_P_ITFP_Reset_Detection_isrsafe(hVip, 0);
    }
    else
    {
        BVDC_P_Progressive_ITFP_Reset_Detection_isrsafe(hVip);
    }

    BKNI_Memset(pEpmPreprocessorInfo, 0, sizeof(BVDC_P_ITFP_EpmPreprocessorInfo_t));

    pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff = BVDC_P_ITFP_PREPROCESSOR_PIC_QUEUE_SIZE;

    pEpmPreprocessorInfo->IsItfpEnabled = true;
    pEpmPreprocessorInfo->EncoderModeOfOperation = 0;

    pEpmPreprocessorInfo->IsProgressive = !hVip->stMemSettings.bInterlaced;
}

void BVDC_P_ITFP_EPM_Preprocessor_flush(BVDC_P_ITFP_EpmPreprocessorInfo_t *pEpmPreprocessorInfo)
{
    uint8_t RdPtr_PictureQueue, PictureQueueFullness;

    if (pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness == 0)
        return;

    RdPtr_PictureQueue = pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr;
    PictureQueueFullness = pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness;

    while (PictureQueueFullness > 0)
    {
        if (pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].IsCadenceSet == false)
        {
            pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].CadenceType = BAVC_CadenceType_eUnlocked;
            pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].CadencePhase = BAVC_CadencePhase_e0;
            pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue].IsCadenceSet = true;
        }

        BKNI_Memset_isr(&pEpmPreprocessorInfo->PreProcessorPictureQueue.PreProcessorVipBuffersArray[RdPtr_PictureQueue], 0, sizeof(BVDC_P_ITFP_PreProcessorVipBuffers_t));
        RdPtr_PictureQueue = BVDC_P_EPM_mod_isrsafe((RdPtr_PictureQueue + 1), pEpmPreprocessorInfo->PreProcessorPictureQueue.NumOfBuff);
        PictureQueueFullness--;
    }
    pEpmPreprocessorInfo->PreProcessorPictureQueue.RdPtr = RdPtr_PictureQueue;
    pEpmPreprocessorInfo->PreProcessorPictureQueue.Fullness = 0;

    return;
}
#endif

/* End of file */
