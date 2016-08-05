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
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1_priv_rfagc);


static const uint32_t rfagcLut[BSRF_RFAGC_LUT_COUNT] =
{
   2304, 2305, 2306, 2307, 2308, 2309, 2310, 2311,
   2312, 2313, 2314, 2315, 2316, 2317, 2318, 2319,
   2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327,
   2328, 2329, 2330, 2331, 2332, 2333, 2334, 2335,
   2336, 2337, 2338, 2339, 2340, 2341, 2342, 2343,
   2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351,
   2352, 2353, 2354, 2355, 2356, 2357, 2421, 2485,
   2549, 2613, 2677, 2741, 2805, 2869, 2933, 2997,
   3061, 3125, 3189, 3253, 3317, 3381, 3445, 3509,
   3573, 3637, 3701, 3765, 3829, 3893, 3957, 4021,
   4021, 4021, 4021, 4021, 4021, 4021, 4021, 4021
};


/******************************************************************************
 BSRF_g1_Rfagc_P_TwosCompToOffBin
******************************************************************************/
uint32_t BSRF_g1_Rfagc_P_TcToOb(int32_t x, uint8_t bits)
{
   if (x < 0)
      return x + (1 << bits);
   else
      return x;
}


/******************************************************************************
 BSRF_g1_Rfagc_P_Init
******************************************************************************/
BERR_Code BSRF_g1_Rfagc_P_Init(BSRF_ChannelHandle h)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint8_t i, j = 0;
   int8_t maxLimit = 15;

#ifdef BSRF_SXM_OVERRIDE
   BKNI_Printf("BSRF_g1_Rfagc_P_Init\n");
#endif

   /* load rfagc lut */
   for (i = 0; i < BSRF_RFAGC_LUT_COUNT; i++)
   {
      /* load lut value if not omitted */
      if (hChn->bOmitRfagcLut[i] == false)
         BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LUT_DATAi_ARRAY_BASE+4*j++, rfagcLut[i]);
   #ifdef BSRF_SXM_OVERRIDE
      else
         BKNI_Printf(" omit idx %d!\n", i);
   #endif
   }

   /* enable rfagc */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_CTRL, 0x00580000);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_CTRL, 0x00580000);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_CTRL, 0x00580000);

   /* configure rfagc settings */
   BSRF_g1_Rfagc_P_SetSettings(h, hChn->rfagcSettings);

   /* set limits */
   maxLimit -= hChn->numRfagcLutOmitted;
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_LF_MAX_LIMIT, maxLimit << 24);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_LF_MIN_LIMIT, 0xC0000000);

   /* setup classic agc */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FS_CNT, BSRF_AGC_SAMPLE_FREQ_KHZ / 1000);

   /* initialize rfagc integrators */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, 0x0A000000);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LA_INT_WDATA, 0);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_CTRL_LF_INT_WDATA, 0);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_CTRL_LA_INT_WDATA, 0);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_INT_WDATA, 0);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_INT_WDATA, 0);

   /* set rfagc threshold control */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_THRA1, 0x03C0FD00);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_THRA2, 0x64002400);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Rfagc_P_SetSettings
******************************************************************************/
#define BSRF_PREC_SHIFT 8
#define BSRF_PREC_SCALE 256
#define BSRF_LOG2_SHIFT 16
#define BSRF_LOG2_SCALE 65536
BERR_Code BSRF_g1_Rfagc_P_CalcSettings(BSRF_ChannelHandle h, uint32_t time, int32_t thr, int32_t deadzone, uint32_t *cnt, int32_t *prec, int32_t *thr2, int32_t *ref)
{
   bool bNegative = false;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   int32_t tmp, precision, residue, threshold;
   uint32_t val;

   /* calculate count */
   BMTH_HILO_32TO64_Mul(time, BSRF_AGC_SAMPLE_FREQ_KHZ * 1000, &P_hi, &P_lo); /* integration time x Fs_agc */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1000000000, &Q_hi, &Q_lo);  /* div by 10^9ns/sec */
   *cnt = Q_lo;

   /* calculate precision and residue */
   tmp = BMTH_log2fix(Q_lo << BSRF_PREC_SHIFT, BSRF_PREC_SHIFT);  /* (log2(cnt)) scaled by 2^8 */
   precision = (tmp + BSRF_PREC_SCALE/2) >> BSRF_PREC_SHIFT;      /* round and scale down 2^8 */
   residue = tmp - precision * BSRF_PREC_SCALE;                  /* log2(cnt/2^(prec)), scaled by 2^8 */
   residue <<= (BSRF_LOG2_SHIFT - BSRF_PREC_SHIFT);               /* scale up residue to 2^16 */
   if (precision > 24)
      precision = 24;   /* cap precision at 24 */
   *prec = precision;

#if 0
   BKNI_Printf("cnt=%d\n", Q_lo);
   BKNI_Printf("tmp=%d\n", tmp);
   BKNI_Printf("precision=%d\n", precision);
   BKNI_Printf("residue=%d\n", residue);
   BKNI_Printf("residue >> 16=%d\n", residue >> BSRF_LOG2_SHIFT);
#endif

   /* rescale threshold dBFS + 13.473413698347843 */
   threshold = (thr + 882994 + residue) << 11; /* (thr + res)*2^11, scaled by 2^16 */
   threshold = (threshold + BSRF_LOG2_SCALE/2) >> BSRF_LOG2_SHIFT;   /* round  and unscale */
   threshold = BSRF_g1_Rfagc_P_TcToOb(threshold, 16);                /* thr2 = tc2ob((thr + res)*2^11+0.5), 16) */
   *thr2 = threshold;

   if (deadzone < 0)
   {
      bNegative = true;
      deadzone = -deadzone;
   }

   /* calculate ref = tc2ob(Cint(deadzone/10* logb(10,2) *2^11 + 0.5),17) */
   BMTH_HILO_32TO64_Mul(deadzone << 11, BMTH_log2fix(10 << BSRF_LOG2_SHIFT, BSRF_LOG2_SHIFT), &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 10 * BSRF_LOG2_SCALE, &Q_hi, &Q_lo);    /* undo double 2^16 scale */
   val = (Q_lo + BSRF_LOG2_SCALE/2) >> BSRF_LOG2_SHIFT;      /* round and scale down 2^16 */
   *ref = BSRF_g1_Rfagc_P_TcToOb(val, 17);

   /* recover sign */
   if (bNegative)
      *ref = -val;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Rfagc_P_SetSettings
******************************************************************************/
BERR_Code BSRF_g1_Rfagc_P_SetSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   int32_t precision, threshold, ref;
   uint32_t count;

   /* program attack settings */
   BSRF_g1_Rfagc_P_CalcSettings(h, settings.attackSettings.timeNs, settings.attackSettings.threshold, 58982,
      &count, &precision, &threshold, &ref);

   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_CNT, count);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_CTRL, ~0x00F8FFFF, ((precision & 0x1F) << 19) | (threshold & 0xFFFF));
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_REF, ref & 0x1FFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_INC, settings.attackSettings.step & 0x1FFFF);

   /* program decay settings */
   BSRF_g1_Rfagc_P_CalcSettings(h, settings.decaySettings.timeNs, settings.decaySettings.threshold, -58982,
      &count, &precision, &threshold, &ref);

   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_CNT, count);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_CTRL, ~0x00F8FFFF, ((precision & 0x1F) << 19) | (threshold & 0xFFFF));
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_REF, ref & 0x1FFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_INC, settings.decaySettings.step & 0x1FFFF);

   /* program fast decay settings */
   BSRF_g1_Rfagc_P_CalcSettings(h, settings.fastDecaySettings.timeNs, settings.fastDecaySettings.threshold, 58982,
      &count, &precision, &threshold, &ref);

   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_CNT, count);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_CTRL, ~0x00F8FFFF, ((precision & 0x1F) << 19) | (threshold & 0xFFFF));
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_REF, hChn->fastDecayGainThr & 0x7F);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_INC, settings.fastDecaySettings.step & 0x1FFFF);

   /* program clip window settings */
   BSRF_g1_Rfagc_P_CalcSettings(h, settings.clipWindowSettings.timeNs, settings.clipWindowSettings.threshold, 58982,
      &count, &precision, &threshold, &ref);

   /* calculate clip window percentage */
   if (settings.clipWindowSettings.threshold > 100)
      settings.clipWindowSettings.threshold = 100;
   threshold = (settings.clipWindowSettings.threshold * count + 50) / 100;

   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_M, count);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_N, threshold);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_FRZ_THR, 0x0000000A); /* window freeze threshold */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_CTRL, 0x7000007F);    /* fixed clip threshold */
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_CTRL, ~0x00FFFF80, (settings.clipWindowSettings.step & 0x1FFFF) << 7);

   /* set rfagc control */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, 0xC05CD6CA);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL2, 0x31000732);

   if (hChn->bEnableFastDecay)
      BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL2, 0x80000000);

   /* set agc la beta */
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, ~0x07000000, (settings.powerMeasureBw & 0x7) << 24);

   /* program slew rate */
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER07_RAMP, ~0x000000FF, settings.agcSlewRate & 0xFF);

   return retCode;
}
