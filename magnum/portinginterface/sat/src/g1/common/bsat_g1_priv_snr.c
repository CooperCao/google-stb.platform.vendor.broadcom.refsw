/******************************************************************************
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
*
*****************************************************************************/
#include "bstd.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"
#include "bmth.h"

BDBG_MODULE(bsat_g1_priv_snr);

static BERR_Code BSAT_g1_P_GetSnrSnore_isr(BSAT_ChannelHandle h, uint32_t *pSnr);
static BERR_Code BSAT_g1_P_GetSnrSnre_isr(BSAT_ChannelHandle h, uint32_t *pSnr);


#define BSAT_DEBUG_SNR(x) /* x */


/******************************************************************************
 BSAT_g1_P_GetSnr() - calculates snr in 1/256 dB
******************************************************************************/
BERR_Code BSAT_g1_P_GetSnr(BSAT_ChannelHandle h, uint32_t *pSnr)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_GetSnr_isr(h, pSnr);
   BKNI_LeaveCriticalSection();
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetSnr() - calculates snr in 1/256 dB
******************************************************************************/
BERR_Code BSAT_g1_P_GetSnr_isr(BSAT_ChannelHandle h, uint32_t *pSnr)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   if (hChn->miscSettings.snrEstMethod == 1)
      goto use_snre;
   else if (hChn->miscSettings.snrEstMethod == 2)
      goto use_snore;

   if ((BSAT_MODE_IS_DVBS2X(hChn->actualMode)) ||
       (BSAT_MODE_IS_DVBS2_EXTENDED(hChn->actualMode)) ||
       (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) ||
       (BSAT_MODE_IS_TURBO(hChn->actualMode)))
   {
      use_snore:
      retCode = BSAT_g1_P_GetSnrSnore_isr(h, pSnr);
   }
   else
   {
      use_snre:
      retCode = BSAT_g1_P_GetSnrSnre_isr(h, pSnr);
   }
   return retCode;
}


/* #ifndef BSAT_USE_SNRE */
#define BSAT_SNR_POWER_MIN   34    /* (1.061161 * 2^5) */
#define BSAT_SNR_POWER_MAX   63848 /* (1995.262315 * 2^5) */
#define BSAT_SNR_POWER_SCALE 67304 /* (2153728.216581 / 2^5) */
#define BSAT_SNR_MAX_SNORE   8448  /* (33 * 256) */
#define BSAT_SNR_RESOLUTION  66    /* (0.257813 * 256) */

static const uint32_t BSAT_SNORE_TABLE[128] =
{
   0x00000000, /* 0. snr_power=1.061161, snr=0.257813 */
   0x00022204, /* 1. snr_power=1.126063, snr=0.515625 */
   0x0004656E, /* 2. snr_power=1.194934, snr=0.773438 */
   0x0006CC47, /* 3. snr_power=1.268017, snr=1.031250 */
   0x000958BC, /* 4. snr_power=1.345570, snr=1.289063 */
   0x000C0D17, /* 5. snr_power=1.427866, snr=1.546875 */
   0x000EEBCC, /* 6. snr_power=1.515196, snr=1.804688 */
   0x0011F76F, /* 7. snr_power=1.607867, snr=2.062500 */
   0x001532C2, /* 8. snr_power=1.706205, snr=2.320313 */
   0x0018A0AE, /* 9. snr_power=1.810558, snr=2.578125 */
   0x001C444C, /* 10. snr_power=1.921294, snr=2.835938 */
   0x002020E5, /* 11. snr_power=2.038802, snr=3.093750 */
   0x002439F4, /* 12. snr_power=2.163497, snr=3.351563 */
   0x0028932C, /* 13. snr_power=2.295818, snr=3.609375 */
   0x002D307B, /* 14. snr_power=2.436233, snr=3.867188 */
   0x00321609, /* 15. snr_power=2.585235, snr=4.125000 */
   0x00374842, /* 16. snr_power=2.743350, snr=4.382813 */
   0x003CCBD8, /* 17. snr_power=2.911136, snr=4.640625 */
   0x0042A5C2, /* 18. snr_power=3.089184, snr=4.898438 */
   0x0048DB4A, /* 19. snr_power=3.278121, snr=5.156250 */
   0x004F7209, /* 20. snr_power=3.478614, snr=5.414063 */
   0x00566FF2, /* 21. snr_power=3.691369, snr=5.671875 */
   0x005DDB54, /* 22. snr_power=3.917137, snr=5.929688 */
   0x0065BAE1, /* 23. snr_power=4.156713, snr=6.187500 */
   0x006E15B4, /* 24. snr_power=4.410941, snr=6.445313 */
   0x0076F357, /* 25. snr_power=4.680718, snr=6.703125 */
   0x00805BCA, /* 26. snr_power=4.966995, snr=6.960938 */
   0x008A578A, /* 27. snr_power=5.270781, snr=7.218750 */
   0x0094EF9B, /* 28. snr_power=5.593147, snr=7.476563 */
   0x00A02D8B, /* 29. snr_power=5.935229, snr=7.734375 */
   0x00AC1B7F, /* 30. snr_power=6.298233, snr=7.992188 */
   0x00B8C43B, /* 31. snr_power=6.683439, snr=8.250000 */
   0x00C6332D, /* 32. snr_power=7.092205, snr=8.507813 */
   0x00D47472, /* 33. snr_power=7.525970, snr=8.765625 */
   0x00E394E9, /* 34. snr_power=7.986266, snr=9.023438 */
   0x00F3A238, /* 35. snr_power=8.474713, snr=9.281250 */
   0x0104AADC, /* 36. snr_power=8.993034, snr=9.539063 */
   0x0116BE32, /* 37. snr_power=9.543057, snr=9.796875 */
   0x0129EC8C, /* 38. snr_power=10.126719, snr=10.054688 */
   0x013E4738, /* 39. snr_power=10.746078, snr=10.312500 */
   0x0153E094, /* 40. snr_power=11.403318, snr=10.570313 */
   0x016ACC1F, /* 41. snr_power=12.100756, snr=10.828125 */
   0x01831E87, /* 42. snr_power=12.840849, snr=11.085938 */
   0x019CEDBF, /* 43. snr_power=13.626208, snr=11.343750 */
   0x01B85112, /* 44. snr_power=14.459599, snr=11.601563 */
   0x01D56137, /* 45. snr_power=15.343961, snr=11.859375 */
   0x01F43867, /* 46. snr_power=16.282412, snr=12.117188 */
   0x0214F277, /* 47. snr_power=17.278260, snr=12.375000 */
   0x0237ACF1, /* 48. snr_power=18.335014, snr=12.632813 */
   0x025C872B, /* 49. snr_power=19.456401, snr=12.890625 */
   0x0283A266, /* 50. snr_power=20.646372, snr=13.148438 */
   0x02AD21ED, /* 51. snr_power=21.909123, snr=13.406250 */
   0x02D92B32, /* 52. snr_power=23.249106, snr=13.664063 */
   0x0307E5F4, /* 53. snr_power=24.671042, snr=13.921875 */
   0x03397C5D, /* 54. snr_power=26.179946, snr=14.179688 */
   0x036E1B2C, /* 55. snr_power=27.781136, snr=14.437500 */
   0x03A5F1DF, /* 56. snr_power=29.480256, snr=14.695313 */
   0x03E132D8, /* 57. snr_power=31.283296, snr=14.953125 */
   0x04201392, /* 58. snr_power=33.196611, snr=15.210938 */
   0x0462CCC8, /* 59. snr_power=35.226947, snr=15.468750 */
   0x04A99AB3, /* 60. snr_power=37.381459, snr=15.726563 */
   0x04F4BD37, /* 61. snr_power=39.667744, snr=15.984375 */
   0x05447821, /* 62. snr_power=42.093860, snr=16.242188 */
   0x05991365, /* 63. snr_power=44.668359, snr=16.500000 */
   0x05F2DB5D, /* 64. snr_power=47.400317, snr=16.757813 */
   0x0652210D, /* 65. snr_power=50.299365, snr=17.015625 */
   0x06B73A6F, /* 66. snr_power=53.375720, snr=17.273438 */
   0x072282BF, /* 67. snr_power=56.640229, snr=17.531250 */
   0x07945ACD, /* 68. snr_power=60.104398, snr=17.789063 */
   0x080D2956, /* 69. snr_power=63.780438, snr=18.046875 */
   0x088D5B5D, /* 70. snr_power=67.681309, snr=18.304688 */
   0x09156492, /* 71. snr_power=71.820761, snr=18.562500 */
   0x09A5BFB9, /* 72. snr_power=76.213385, snr=18.820313 */
   0x0A3EEF16, /* 73. snr_power=80.874666, snr=19.078125 */
   0x0AE17CE5, /* 74. snr_power=85.821035, snr=19.335938 */
   0x0B8DFBD8, /* 75. snr_power=91.069929, snr=19.593750 */
   0x0C450799, /* 76. snr_power=96.639851, snr=19.851563 */
   0x0D074556, /* 77. snr_power=102.550433, snr=20.109375 */
   0x0DD56458, /* 78. snr_power=108.822513, snr=20.367188 */
   0x0EB01EA2, /* 79. snr_power=115.478198, snr=20.625000 */
   0x0F983996, /* 80. snr_power=122.540952, snr=20.882813 */
   0x108E86A8, /* 81. snr_power=130.035670, snr=21.140625 */
   0x1193E41C, /* 82. snr_power=137.988772, snr=21.398438 */
   0x12A93DCE, /* 83. snr_power=146.428293, snr=21.656250 */
   0x13CF8E09, /* 84. snr_power=155.383983, snr=21.914063 */
   0x1507DE62, /* 85. snr_power=164.887411, snr=22.171875 */
   0x165348B2, /* 86. snr_power=174.972078, snr=22.429688 */
   0x17B2F80A, /* 87. snr_power=185.673532, snr=22.687500 */
   0x192829C8, /* 88. snr_power=197.029498, snr=22.945313 */
   0x1AB42EB4, /* 89. snr_power=209.080004, snr=23.203125 */
   0x1C586C2D, /* 90. snr_power=221.867531, snr=23.460938 */
   0x1E165D6E, /* 91. snr_power=235.437154, snr=23.718750 */
   0x1FEF94E5, /* 92. snr_power=249.836709, snr=23.976563 */
   0x21E5BD9A, /* 93. snr_power=265.116953, snr=24.234375 */
   0x23FA9CB7, /* 94. snr_power=281.331751, snr=24.492188 */
   0x2630131A, /* 95. snr_power=298.538262, snr=24.750000 */
   0x28881F0C, /* 96. snr_power=316.797138, snr=25.007813 */
   0x2B04DE0A, /* 97. snr_power=336.172745, snr=25.265625 */
   0x2DA88EB0, /* 98. snr_power=356.733381, snr=25.523438 */
   0x307592C0, /* 99. snr_power=378.551525, snr=25.781250 */
   0x336E7146, /* 100. snr_power=401.704087, snr=26.039063 */
   0x3695D8E0, /* 101. snr_power=426.272681, snr=26.296875 */
   0x39EEA22C, /* 102. snr_power=452.343912, snr=26.554688 */
   0x3D7BD256, /* 103. snr_power=480.009685, snr=26.812500 */
   0x41409DD8, /* 104. snr_power=509.367522, snr=27.070313 */
   0x45406B55, /* 105. snr_power=540.520911, snr=27.328125 */
   0x497ED6B4, /* 106. snr_power=573.579670, snr=27.585938 */
   0x4DFFB45B, /* 107. snr_power=608.660334, snr=27.843750 */
   0x52C714A9, /* 108. snr_power=645.886564, snr=28.101563 */
   0x57D9479A, /* 109. snr_power=685.389584, snr=28.359375 */
   0x5D3AE0B4, /* 110. snr_power=727.308645, snr=28.617188 */
   0x62F0BB20, /* 111. snr_power=771.791516, snr=28.875000 */
   0x68FFFE13, /* 112. snr_power=818.994999, snr=29.132813 */
   0x6F6E216E, /* 113. snr_power=869.085492, snr=29.390625 */
   0x7640F2B2, /* 114. snr_power=922.239565, snr=29.648438 */
   0x7D7E9A39, /* 115. snr_power=978.644591, snr=29.906250 */
   0x852DA0C9, /* 116. snr_power=1038.499400, snr=30.164063 */
   0x8D54F574, /* 117. snr_power=1102.014985, snr=30.421875 */
   0x95FBF3DF, /* 118. snr_power=1169.415242, snr=30.679688 */
   0x9F2A6AE2, /* 119. snr_power=1240.937761, snr=30.937500 */
   0xA8E8A396, /* 120. snr_power=1316.834663, snr=31.195313 */
   0xB33F68CF, /* 121. snr_power=1397.373490, snr=31.453125 */
   0xBE380F09, /* 122. snr_power=1482.838147, snr=31.710938 */
   0xC9DC7CD7, /* 123. snr_power=1573.529901, snr=31.968750 */
   0xD63733CE, /* 124. snr_power=1669.768447, snr=32.226563 */
   0xE3535A05, /* 125. snr_power=1771.893031, snr=32.484375 */
   0xF13CC420, /* 126. snr_power=1880.263649, snr=32.742188 */
   0xFFFFFFFF, /* 127. snr_power=1995.262315, snr=33.000000 */
};


static const uint16_t snr_power_compensation[] = {
   62,  79,  95,  110, 124, 138, 151, 165, 177, 190,
   203, 215, 227, 239, 251, 263, 274, 286, 297, 308,
   319, 330, 341, 352, 363, 374, 384, 395, 406, 416,
   427, 437, 448, 458, 469, 479, 490, 500, 511, 521,
   531, 542, 552, 562, 572, 583, 593, 603, 613, 623,
   633, 643, 653, 663, 673, 683, 692, 702, 712, 722,
   733, 743, 753, 763, 774, 784, 795, 805,
};


/******************************************************************************
 BSAT_g1_P_GetSnrSnore_isr() - calculates snr in 1/256 dB
******************************************************************************/
static BERR_Code BSAT_g1_P_GetSnrSnore_isr(BSAT_ChannelHandle h, uint32_t *pSnr)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t sig_power, sig_power_scaled, tot_power, snr256, snr_power, err_power, M, N;
   uint32_t lo, hi, target, idx, step, snr_lo, snr_hi, P_hi, P_lo, Q_hi, Q_lo;

   /* take snapshot of total power and signal power */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x40);

   /* wait at least 2 bclks to read snore sp and tp */
   if (hChn->acqSettings.symbolRate < 10000000)
      BKNI_Delay((hChn->acqSettings.symbolRate < 2000000) ? 2 : 1);

   sig_power = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_SNR_SNORESP);
   tot_power = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_SNR_SNORETP);
   BSAT_DEBUG_SNR(BDBG_MSG(("BSAT_g1_P_GetSnr(): sig_power=%u, tot_power=%u", sig_power, tot_power)));

   if ((BSAT_MODE_IS_DVBS2(hChn->actualMode)) || (hChn->acqSettings.mode == BSAT_Mode_eDvbs2_ACM) ||
       (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode)) || (BSAT_MODE_IS_DVBS2X(hChn->actualMode)))
   {
      /* LDPC or Turbo 8PSK */
      err_power = tot_power - sig_power;
      if (err_power == 0)
         goto snr0;
      snr_power = ((sig_power - err_power) * 32) / err_power;
   }
   else
   {
      /* Turbo QPSK or Legacy QPSK */
      sig_power_scaled = sig_power << 1;
      err_power = tot_power - sig_power_scaled;
      if (err_power == 0)
         goto snr0;
      snr_power = ((sig_power_scaled - err_power) * 32) / err_power;

      if (snr_power < 135)
         snr_power = 60;
      else if (snr_power < 805)
      {
         N = (snr_power - 135) / 10;
         M = snr_power - 135 - (N * 10);
         BSAT_DEBUG_SNR(BDBG_MSG(("BSAT_g1_P_GetSnr(): adjust: snr_power=%u, M=%d, N=%d", snr_power, M, N)));
         snr_power = snr_power_compensation[N] + ((snr_power_compensation[N+1] - snr_power_compensation[N]) * M / 10);
      }
   }
   snr_power = (snr_power + 1) >> 1; /* scale 2^5 */

   BSAT_DEBUG_SNR(BDBG_MSG(("BSAT_g1_P_GetSnr(): err_power=%u, snr_power=%u", err_power, snr_power)));
   if (snr_power <= BSAT_SNR_POWER_MIN)
   {
      snr0:
      snr256 = 0;
      goto done;
   }
   if (snr_power >= BSAT_SNR_POWER_MAX)
   {
      if (hChn->bLocked == false)
         goto snr0;
      snr256 = BSAT_SNR_MAX_SNORE;
      goto done;
   }

   target = (snr_power - BSAT_SNR_POWER_MIN) * BSAT_SNR_POWER_SCALE;
   idx = 64; /* start at middle of table and do binary search */
   step = 32;

   while (1)
   {
      lo = BSAT_SNORE_TABLE[idx];
      hi = BSAT_SNORE_TABLE[idx+1];

      if (target < lo)
         idx -= step;
      else if (target > hi)
         idx += step;
      else
         break;
      if (step > 1)
         step = step >> 1;
   }

   /* snr_lo = BSAT_SNR_RESOLUTION * (idx); */
   idx++;
   BMTH_HILO_32TO64_Mul(idx * 2 * 256, 257813, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1000000, &Q_hi, &Q_lo);
   snr_lo = (Q_lo + 1) >> 1;

   /* snr_hi = BSAT_SNR_RESOLUTION * (idx+1); */
   BMTH_HILO_32TO64_Mul((idx+1) * 2 * 256, 257813, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1000000, &Q_hi, &Q_lo);
   snr_hi = (Q_lo + 1) >> 1;

   BSAT_DEBUG_SNR(BDBG_MSG(("BSAT_g1_P_GetSnr(): target=0x%08X, idx=%d, snr_lo=%u, snr_hi=%u", target, idx, snr_lo, snr_hi)));

   /* interpolate between (snr_lo, snr_hi) */
   BMTH_HILO_32TO64_Mul(target - lo, (snr_hi - snr_lo) * 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hi - lo, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   snr256 = snr_lo + Q_lo;
   BSAT_DEBUG_SNR(BDBG_MSG(("BSAT_g1_P_GetSnr(): snr256=%u", snr256)));

   done:
   *pSnr = snr256;
   return BERR_SUCCESS;
}

/* #else */

#define BSAT_SNRE_TABLE_N (128)
#define BSAT_SNR_MAX_SNRE (9344)  /* 36.5 (in units of 2^-8 dB) */
#define BSAT_snr_quant (73)  /* BSAT_SNR_MAX_SNRE / BSAT_SNRE_TABLE_N */

#define BSAT_snr_legacy_qpsk_scale (247878528) /* 3782.326172 * 2^16 */
#define BSAT_snr_legacy_qpsk_min   (0x0061C97E)
#define BSAT_snr_legacy_qpsk_max   (0x0F281D00)

#define BSAT_snr_ldpc_8psk_scale (126049536)   /* 1923.363281 * 2^16 */
#define BSAT_snr_ldpc_8psk_min   (0x0001D56E)
#define BSAT_snr_ldpc_8psk_max   (0x07853270)
#define BSAT_snr_ldpc_qpsk_scale (90691248)    /* 1383.838623 * 2^16 */
#define BSAT_snr_ldpc_qpsk_min   (0x0000EBF0)
#define BSAT_snr_ldpc_qpsk_max   (0x0568C2A0)

#define BSAT_snr_turbo_8psk_scale (63024768)   /* 961.6816405 * 2^16 */
#define BSAT_snr_turbo_8psk_min   (0x0000EAB7)
#define BSAT_snr_turbo_8psk_max   (0x03C29938)

static const uint16_t BSAT_snr_legacy_qpsk_table[128] = {
   0x0000, /* 0: snre=0x0061C97D, snr=36.500000 dB */
   0x0038, /* 1: snre=0x00650CC7, snr=36.214844 dB */
   0x0072, /* 2: snre=0x00686BEC, snr=35.929688 dB */
   0x00AF, /* 3: snre=0x006BE7DA, snr=35.644531 dB */
   0x00ED, /* 4: snre=0x006F8187, snr=35.359375 dB */
   0x012E, /* 5: snre=0x007339EF, snr=35.074219 dB */
   0x0170, /* 6: snre=0x00771218, snr=34.789063 dB */
   0x01B5, /* 7: snre=0x007B0B12, snr=34.503906 dB */
   0x01FC, /* 8: snre=0x007F25F3, snr=34.218750 dB */
   0x0246, /* 9: snre=0x008363DC, snr=33.933594 dB */
   0x0292, /* 10: snre=0x0087C5F8, snr=33.648438 dB */
   0x02E0, /* 11: snre=0x008C4D7B, snr=33.363281 dB */
   0x0331, /* 12: snre=0x0090FBA2, snr=33.078125 dB */
   0x0385, /* 13: snre=0x0095D1B7, snr=32.792969 dB */
   0x03DC, /* 14: snre=0x009AD10D, snr=32.507813 dB */
   0x0435, /* 15: snre=0x009FFB03, snr=32.222656 dB */
   0x0492, /* 16: snre=0x00A55103, snr=31.937500 dB */
   0x04F1, /* 17: snre=0x00AAD483, snr=31.652344 dB */
   0x0554, /* 18: snre=0x00B08706, snr=31.367188 dB */
   0x05BA, /* 19: snre=0x00B66A1B, snr=31.082031 dB */
   0x0623, /* 20: snre=0x00BC7F5D, snr=30.796875 dB */
   0x0690, /* 21: snre=0x00C2C876, snr=30.511719 dB */
   0x0701, /* 22: snre=0x00C9471F, snr=30.226563 dB */
   0x0775, /* 23: snre=0x00CFFD1B, snr=29.941406 dB */
   0x07ED, /* 24: snre=0x00D6EC41, snr=29.656250 dB */
   0x0869, /* 25: snre=0x00DE1674, snr=29.371094 dB */
   0x08EA, /* 26: snre=0x00E57DA6, snr=29.085938 dB */
   0x096E, /* 27: snre=0x00ED23DC, snr=28.800781 dB */
   0x09F7, /* 28: snre=0x00F50B2A, snr=28.515625 dB */
   0x0A84, /* 29: snre=0x00FD35B4, snr=28.230469 dB */
   0x0B17, /* 30: snre=0x0105A5B2, snr=27.945313 dB */
   0x0BAE, /* 31: snre=0x010E5D6D, snr=27.660156 dB */
   0x0C4A, /* 32: snre=0x01175F3F, snr=27.375000 dB */
   0x0CEB, /* 33: snre=0x0120AD99, snr=27.089844 dB */
   0x0D92, /* 34: snre=0x012A4AFC, snr=26.804688 dB */
   0x0E3E, /* 35: snre=0x01343A00, snr=26.519531 dB */
   0x0EF0, /* 36: snre=0x013E7D4F, snr=26.234375 dB */
   0x0FA7, /* 37: snre=0x014917AB, snr=25.949219 dB */
   0x1065, /* 38: snre=0x01540BEC, snr=25.664063 dB */
   0x1129, /* 39: snre=0x015F5CFD, snr=25.378906 dB */
   0x11F4, /* 40: snre=0x016B0DE5, snr=25.093750 dB */
   0x12C5, /* 41: snre=0x017721BF, snr=24.808594 dB */
   0x139D, /* 42: snre=0x01839BC1, snr=24.523438 dB */
   0x147D, /* 43: snre=0x01907F36, snr=24.238281 dB */
   0x1563, /* 44: snre=0x019DCF88, snr=23.953125 dB */
   0x1652, /* 45: snre=0x01AB9035, snr=23.667969 dB */
   0x1748, /* 46: snre=0x01B9C4DB, snr=23.382813 dB */
   0x1846, /* 47: snre=0x01C8712E, snr=23.097656 dB */
   0x194C, /* 48: snre=0x01D79902, snr=22.812500 dB */
   0x1A5C, /* 49: snre=0x01E74045, snr=22.527344 dB */
   0x1B74, /* 50: snre=0x01F76B00, snr=22.242188 dB */
   0x1C95, /* 51: snre=0x02081D5D, snr=21.957031 dB */
   0x1DC0, /* 52: snre=0x02195BA1, snr=21.671875 dB */
   0x1EF4, /* 53: snre=0x022B2A2E, snr=21.386719 dB */
   0x2033, /* 54: snre=0x023D8D86, snr=21.101563 dB */
   0x217C, /* 55: snre=0x02508A48, snr=20.816406 dB */
   0x22D0, /* 56: snre=0x02642534, snr=20.531250 dB */
   0x242E, /* 57: snre=0x02786326, snr=20.246094 dB */
   0x2599, /* 58: snre=0x028D4919, snr=19.960938 dB */
   0x270E, /* 59: snre=0x02A2DC2A, snr=19.675781 dB */
   0x2890, /* 60: snre=0x02B92192, snr=19.390625 dB */
   0x2A1F, /* 61: snre=0x02D01EA8, snr=19.105469 dB */
   0x2BBA, /* 62: snre=0x02E7D8E3, snr=18.820313 dB */
   0x2D62, /* 63: snre=0x030055D6, snr=18.535156 dB */
   0x2F18, /* 64: snre=0x03199B32, snr=18.250000 dB */
   0x30DC, /* 65: snre=0x0333AEC3, snr=17.964844 dB */
   0x32AE, /* 66: snre=0x034E9671, snr=17.679688 dB */
   0x348F, /* 67: snre=0x036A583D, snr=17.394531 dB */
   0x367F, /* 68: snre=0x0386FA41, snr=17.109375 dB */
   0x387F, /* 69: snre=0x03A482AD, snr=16.824219 dB */
   0x3A8E, /* 70: snre=0x03C2F7C5, snr=16.539063 dB */
   0x3CAF, /* 71: snre=0x03E25FDF, snr=16.253906 dB */
   0x3EE0, /* 72: snre=0x0402C163, snr=15.968750 dB */
   0x4122, /* 73: snre=0x042422C1, snr=15.683594 dB */
   0x4376, /* 74: snre=0x04468A76, snr=15.398438 dB */
   0x45DC, /* 75: snre=0x0469FF03, snr=15.113281 dB */
   0x4855, /* 76: snre=0x048E86E7, snr=14.828125 dB */
   0x4AE1, /* 77: snre=0x04B428A0, snr=14.542969 dB */
   0x4D81, /* 78: snre=0x04DAEAA2, snr=14.257813 dB */
   0x5035, /* 79: snre=0x0502D352, snr=13.972656 dB */
   0x52FC, /* 80: snre=0x052BE8FD, snr=13.687500 dB */
   0x55D9, /* 81: snre=0x055631D5, snr=13.402344 dB */
   0x58CB, /* 82: snre=0x0581B3E7, snr=13.117188 dB */
   0x5BD2, /* 83: snre=0x05AE7511, snr=12.832031 dB */
   0x5EF0, /* 84: snre=0x05DC7AFD, snr=12.546875 dB */
   0x6224, /* 85: snre=0x060BCB11, snr=12.261719 dB */
   0x656E, /* 86: snre=0x063C6A6C, snr=11.976563 dB */
   0x68D0, /* 87: snre=0x066E5DD2, snr=11.691406 dB */
   0x6C48, /* 88: snre=0x06A1A9A5, snr=11.406250 dB */
   0x6FD9, /* 89: snre=0x06D651D7, snr=11.121094 dB */
   0x7381, /* 90: snre=0x070C59DB, snr=10.835938 dB */
   0x7741, /* 91: snre=0x0743C497, snr=10.550781 dB */
   0x7B1A, /* 92: snre=0x077C9454, snr=10.265625 dB */
   0x7EA0, /* 93: snre=0x07B0AE18, snr=9.980469 dB */
   0x82D8, /* 94: snre=0x07EEFE62, snr=9.695313 dB */
   0x872F, /* 95: snre=0x082F1C31, snr=9.410156 dB */
   0x8BA3, /* 96: snre=0x0870E749, snr=9.125000 dB */
   0x9031, /* 97: snre=0x08B43347, snr=8.839844 dB */
   0x94D5, /* 98: snre=0x08F8C767, snr=8.554688 dB */
   0x998B, /* 99: snre=0x093E5F27, snr=8.269531 dB */
   0x9E4D, /* 100: snre=0x0984AC02, snr=7.984375 dB */
   0xA316, /* 101: snre=0x09CB581F, snr=7.699219 dB */
   0xA7DF, /* 102: snre=0x0A1209D9, snr=7.414063 dB */
   0xACA2, /* 103: snre=0x0A5867AB, snr=7.128906 dB */
   0xB15A, /* 104: snre=0x0A9E1C10, snr=6.843750 dB */
   0xB601, /* 105: snre=0x0AE2D8EA, snr=6.558594 dB */
   0xBA92, /* 106: snre=0x0B265A0B, snr=6.273438 dB */
   0xBF0B, /* 107: snre=0x0B6866C0, snr=5.988281 dB */
   0xC367, /* 108: snre=0x0BA8D25C, snr=5.703125 dB */
   0xC7A5, /* 109: snre=0x0BE77BEB, snr=5.417969 dB */
   0xCBC2, /* 110: snre=0x0C244D47, snr=5.132813 dB */
   0xCFBF, /* 111: snre=0x0C5F39C5, snr=4.847656 dB */
   0xD39B, /* 112: snre=0x0C983CB7, snr=4.562500 dB */
   0xD756, /* 113: snre=0x0CCF57F2, snr=4.277344 dB */
   0xDAF0, /* 114: snre=0x0D04925D, snr=3.992188 dB */
   0xDE6B, /* 115: snre=0x0D37F6B9, snr=3.707031 dB */
   0xE1C6, /* 116: snre=0x0D69928F, snr=3.421875 dB */
   0xE504, /* 117: snre=0x0D99754D, snr=3.136719 dB */
   0xE825, /* 118: snre=0x0DC7AF95, snr=2.851563 dB */
   0xEB2A, /* 119: snre=0x0DF452AE, snr=2.566406 dB */
   0xEE15, /* 120: snre=0x0E1F701A, snr=2.281250 dB */
   0xF0E7, /* 121: snre=0x0E491943, snr=1.996094 dB */
   0xF3A1, /* 122: snre=0x0E715F40, snr=1.710938 dB */
   0xF644, /* 123: snre=0x0E9852AE, snr=1.425781 dB */
   0xF8D1, /* 124: snre=0x0EBE0390, snr=1.140625 dB */
   0xFB49, /* 125: snre=0x0EE28142, snr=0.855469 dB */
   0xFDAE, /* 126: snre=0x0F05DA6A, snr=0.570313 dB */
   0xFFFF  /* 127: snre=0x0F281CF9, snr=0.285156 dB */
};


static const uint16_t BSAT_snr_ldpc_qpsk_table[128] = {
   0x0000, /* 0: snre=0x0000EBF0, snr=36.500000 dB */
   0x0002, /* 1: snre=0x0000FAAD, snr=36.214844 dB */
   0x0005, /* 2: snre=0x00010BAF, snr=35.929688 dB */
   0x0009, /* 3: snre=0x00011DDA, snr=35.644531 dB */
   0x000C, /* 4: snre=0x00013140, snr=35.359375 dB */
   0x0010, /* 5: snre=0x000145F7, snr=35.074219 dB */
   0x0014, /* 6: snre=0x00015C16, snr=34.789063 dB */
   0x0019, /* 7: snre=0x000173B5, snr=34.503906 dB */
   0x001D, /* 8: snre=0x00018CEE, snr=34.218750 dB */
   0x0022, /* 9: snre=0x0001A7DE, snr=33.933594 dB */
   0x0028, /* 10: snre=0x0001C4A1, snr=33.648438 dB */
   0x002D, /* 11: snre=0x0001E358, snr=33.363281 dB */
   0x0033, /* 12: snre=0x00020425, snr=33.078125 dB */
   0x003A, /* 13: snre=0x0002272B, snr=32.792969 dB */
   0x0041, /* 14: snre=0x00024C92, snr=32.507813 dB */
   0x0048, /* 15: snre=0x00027483, snr=32.222656 dB */
   0x0050, /* 16: snre=0x00029F29, snr=31.937500 dB */
   0x0058, /* 17: snre=0x0002CCB4, snr=31.652344 dB */
   0x0061, /* 18: snre=0x0002FD56, snr=31.367188 dB */
   0x006B, /* 19: snre=0x00033144, snr=31.082031 dB */
   0x0075, /* 20: snre=0x000368B9, snr=30.796875 dB */
   0x0080, /* 21: snre=0x0003A3F1, snr=30.511719 dB */
   0x008C, /* 22: snre=0x0003E32D, snr=30.226563 dB */
   0x0098, /* 23: snre=0x000426B4, snr=29.941406 dB */
   0x00A6, /* 24: snre=0x00046ECF, snr=29.656250 dB */
   0x00B4, /* 25: snre=0x0004BBCE, snr=29.371094 dB */
   0x00C3, /* 26: snre=0x00050E06, snr=29.085938 dB */
   0x00D3, /* 27: snre=0x000565D1, snr=28.800781 dB */
   0x00E5, /* 28: snre=0x0005C391, snr=28.515625 dB */
   0x00F7, /* 29: snre=0x000627AD, snr=28.230469 dB */
   0x010B, /* 30: snre=0x00069293, snr=27.945313 dB */
   0x0120, /* 31: snre=0x000704B8, snr=27.660156 dB */
   0x0137, /* 32: snre=0x00077E9A, snr=27.375000 dB */
   0x014F, /* 33: snre=0x000800C0, snr=27.089844 dB */
   0x0169, /* 34: snre=0x00088BB8, snr=26.804688 dB */
   0x0184, /* 35: snre=0x0009201B, snr=26.519531 dB */
   0x01A1, /* 36: snre=0x0009BE8D, snr=26.234375 dB */
   0x01C1, /* 37: snre=0x000A67BC, snr=25.949219 dB */
   0x01E2, /* 38: snre=0x000B1C61, snr=25.664063 dB */
   0x0206, /* 39: snre=0x000BDD42, snr=25.378906 dB */
   0x022C, /* 40: snre=0x000CAB34, snr=25.093750 dB */
   0x0255, /* 41: snre=0x000D8718, snr=24.808594 dB */
   0x0280, /* 42: snre=0x000E71DF, snr=24.523438 dB */
   0x02AE, /* 43: snre=0x000F6C8A, snr=24.238281 dB */
   0x02E0, /* 44: snre=0x0010782B, snr=23.953125 dB */
   0x0315, /* 45: snre=0x001195E7, snr=23.667969 dB */
   0x034D, /* 46: snre=0x0012C6F4, snr=23.382813 dB */
   0x0389, /* 47: snre=0x00140CA1, snr=23.097656 dB */
   0x03CA, /* 48: snre=0x00156850, snr=22.812500 dB */
   0x040E, /* 49: snre=0x0016DB79, snr=22.527344 dB */
   0x0458, /* 50: snre=0x001867B2, snr=22.242188 dB */
   0x04A6, /* 51: snre=0x001A0EA6, snr=21.957031 dB */
   0x04F9, /* 52: snre=0x001BD21F, snr=21.671875 dB */
   0x0553, /* 53: snre=0x001DB405, snr=21.386719 dB */
   0x05B2, /* 54: snre=0x001FB65E, snr=21.101563 dB */
   0x0617, /* 55: snre=0x0021DB53, snr=20.816406 dB */
   0x0684, /* 56: snre=0x00242530, snr=20.531250 dB */
   0x06F7, /* 57: snre=0x00269666, snr=20.246094 dB */
   0x0773, /* 58: snre=0x0029318E, snr=19.960938 dB */
   0x07F6, /* 59: snre=0x002BF96C, snr=19.675781 dB */
   0x0883, /* 60: snre=0x002EF0EF, snr=19.390625 dB */
   0x0919, /* 61: snre=0x00321B34, snr=19.105469 dB */
   0x09B9, /* 62: snre=0x00357B8A, snr=18.820313 dB */
   0x0A63, /* 63: snre=0x00391575, snr=18.535156 dB */
   0x0B19, /* 64: snre=0x003CECAB, snr=18.250000 dB */
   0x0BDB, /* 65: snre=0x0041051D, snr=17.964844 dB */
   0x0CAA, /* 66: snre=0x004562F5, snr=17.679688 dB */
   0x0D86, /* 67: snre=0x004A0A9D, snr=17.394531 dB */
   0x0E71, /* 68: snre=0x004F00BB, snr=17.109375 dB */
   0x0F6C, /* 69: snre=0x00544A38, snr=16.824219 dB */
   0x1076, /* 70: snre=0x0059EC3E, snr=16.539063 dB */
   0x1193, /* 71: snre=0x005FEC3F, snr=16.253906 dB */
   0x12C1, /* 72: snre=0x00664FF0, snr=15.968750 dB */
   0x1403, /* 73: snre=0x006D1D4D, snr=15.683594 dB */
   0x155A, /* 74: snre=0x00745A99, snr=15.398438 dB */
   0x16C7, /* 75: snre=0x007C0E5D, snr=15.113281 dB */
   0x184B, /* 76: snre=0x00843F66, snr=14.828125 dB */
   0x19E7, /* 77: snre=0x008CF4C3, snr=14.542969 dB */
   0x1B9E, /* 78: snre=0x009635C3, snr=14.257813 dB */
   0x1D6F, /* 79: snre=0x00A009EB, snr=13.972656 dB */
   0x1F5D, /* 80: snre=0x00AA78F6, snr=13.687500 dB */
   0x2169, /* 81: snre=0x00B58AC8, snr=13.402344 dB */
   0x2395, /* 82: snre=0x00C14764, snr=13.117188 dB */
   0x25E2, /* 83: snre=0x00CDB6E0, snr=12.832031 dB */
   0x2852, /* 84: snre=0x00DAE155, snr=12.546875 dB */
   0x2AE5, /* 85: snre=0x00E8CECD, snr=12.261719 dB */
   0x2D9E, /* 86: snre=0x00F7872D, snr=11.976563 dB */
   0x307E, /* 87: snre=0x0107121C, snr=11.691406 dB */
   0x3387, /* 88: snre=0x011776E7, snr=11.406250 dB */
   0x36B9, /* 89: snre=0x0128BC5D, snr=11.121094 dB */
   0x3A15, /* 90: snre=0x013AE8B4, snr=10.835938 dB */
   0x3D9E, /* 91: snre=0x014E0157, snr=10.550781 dB */
   0x4153, /* 92: snre=0x01620AC3, snr=10.265625 dB */
   0x4535, /* 93: snre=0x01770857, snr=9.980469 dB */
   0x4944, /* 94: snre=0x018CFC22, snr=9.695313 dB */
   0x4D82, /* 95: snre=0x01A3E6B8, snr=9.410156 dB */
   0x51EC, /* 96: snre=0x01BBC6FD, snr=9.125000 dB */
   0x5684, /* 97: snre=0x01D499FC, snr=8.839844 dB */
   0x5B48, /* 98: snre=0x01EE5AB9, snr=8.554688 dB */
   0x6036, /* 99: snre=0x02090215, snr=8.269531 dB */
   0x654D, /* 100: snre=0x022486B4, snr=7.984375 dB */
   0x6A8B, /* 101: snre=0x0240DCF1, snr=7.699219 dB */
   0x6FED, /* 102: snre=0x025DF6ED, snr=7.414063 dB */
   0x7571, /* 103: snre=0x027BC4A4, snr=7.128906 dB */
   0x7B12, /* 104: snre=0x029A341E, snr=6.843750 dB */
   0x80CE, /* 105: snre=0x02B931BE, snr=6.558594 dB */
   0x86A0, /* 106: snre=0x02D8A89C, snr=6.273438 dB */
   0x8C84, /* 107: snre=0x02F882F5, snr=5.988281 dB */
   0x9277, /* 108: snre=0x0318AAB1, snr=5.703125 dB */
   0x9874, /* 109: snre=0x033909E5, snr=5.417969 dB */
   0x9E78, /* 110: snre=0x03598B5D, snr=5.132813 dB */
   0xA47E, /* 111: snre=0x037A1B18, snr=4.847656 dB */
   0xAA83, /* 112: snre=0x039AA6B1, snr=4.562500 dB */
   0xB084, /* 113: snre=0x03BB1DBB, snr=4.277344 dB */
   0xB2D8, /* 114: snre=0x03DB71F4, snr=3.992188 dB */ /* low SNR is adjusted at this point and below */
   0xB81C, /* 115: snre=0x03FB9769, snr=3.707031 dB */
   0xBD4C, /* 116: snre=0x041B8478, snr=3.421875 dB */
   0xC267, /* 117: snre=0x043B31BE, snr=3.136719 dB */
   0xC76D, /* 118: snre=0x045A99F8, snr=2.851563 dB */
   0xCC5D, /* 119: snre=0x0479B9CB, snr=2.566406 dB */
   0xD138, /* 120: snre=0x04988F94, snr=2.281250 dB */
   0xD5FB, /* 121: snre=0x04B71B25, snr=1.996094 dB */
   0xDAAA, /* 122: snre=0x04D55D92, snr=1.710938 dB */
   0xDF44, /* 123: snre=0x04F358EF, snr=1.425781 dB */
   0xE3C8, /* 124: snre=0x05111027, snr=1.140625 dB */
   0xE83A, /* 125: snre=0x052E86C4, snr=0.855469 dB */
   0xEC98, /* 126: snre=0x054BC0CC, snr=0.570313 dB */
   0xFFFF  /* 127: snre=0x0568C29E, snr=0.285156 dB */
};


static const uint16_t BSAT_snr_ldpc_8psk_table[128] = {
   0x0000, /* 0: snre=0x0001D56E, snr=36.500000 dB */
   0x0004, /* 1: snre=0x0001F548, snr=36.214844 dB */
   0x0008, /* 2: snre=0x0002174A, snr=35.929688 dB */
   0x000D, /* 3: snre=0x00023B9C, snr=35.644531 dB */
   0x0012, /* 4: snre=0x00026264, snr=35.359375 dB */
   0x0018, /* 5: snre=0x00028BCE, snr=35.074219 dB */
   0x001E, /* 6: snre=0x0002B806, snr=34.789063 dB */
   0x0024, /* 7: snre=0x0002E73F, snr=34.503906 dB */
   0x002B, /* 8: snre=0x000319AB, snr=34.218750 dB */
   0x0032, /* 9: snre=0x00034F82, snr=33.933594 dB */
   0x0039, /* 10: snre=0x00038900, snr=33.648438 dB */
   0x0042, /* 11: snre=0x0003C664, snr=33.363281 dB */
   0x004A, /* 12: snre=0x000407F2, snr=33.078125 dB */
   0x0054, /* 13: snre=0x00044DF1, snr=32.792969 dB */
   0x005E, /* 14: snre=0x000498AF, snr=32.507813 dB */
   0x0068, /* 15: snre=0x0004E87D, snr=32.222656 dB */
   0x0074, /* 16: snre=0x00053DB4, snr=31.937500 dB */
   0x0080, /* 17: snre=0x000598B2, snr=31.652344 dB */
   0x008D, /* 18: snre=0x0005F9D9, snr=31.367188 dB */
   0x009A, /* 19: snre=0x00066196, snr=31.082031 dB */
   0x00A9, /* 20: snre=0x0006D05A, snr=30.796875 dB */
   0x00B9, /* 21: snre=0x0007469D, snr=30.511719 dB */
   0x00CA, /* 22: snre=0x0007C4E3, snr=30.226563 dB */
   0x00DC, /* 23: snre=0x00084BB5, snr=29.941406 dB */
   0x00EF, /* 24: snre=0x0008DBA8, snr=29.656250 dB */
   0x0103, /* 25: snre=0x00097558, snr=29.371094 dB */
   0x0119, /* 26: snre=0x000A196E, snr=29.085938 dB */
   0x0130, /* 27: snre=0x000AC89D, snr=28.800781 dB */
   0x0149, /* 28: snre=0x000B83A5, snr=28.515625 dB */
   0x0164, /* 29: snre=0x000C4B51, snr=28.230469 dB */
   0x0180, /* 30: snre=0x000D207B, snr=27.945313 dB */
   0x019F, /* 31: snre=0x000E040C, snr=27.660156 dB */
   0x01BF, /* 32: snre=0x000EF6FA, snr=27.375000 dB */
   0x01E1, /* 33: snre=0x000FFA4E, snr=27.089844 dB */
   0x0206, /* 34: snre=0x00110F21, snr=26.804688 dB */
   0x022E, /* 35: snre=0x0012369E, snr=26.519531 dB */
   0x0258, /* 36: snre=0x00137205, snr=26.234375 dB */
   0x0284, /* 37: snre=0x0014C2AB, snr=25.949219 dB */
   0x02B4, /* 38: snre=0x001629F9, snr=25.664063 dB */
   0x02E7, /* 39: snre=0x0017A973, snr=25.378906 dB */
   0x031E, /* 40: snre=0x001942B3, snr=25.093750 dB */
   0x0358, /* 41: snre=0x001AF76F, snr=24.808594 dB */
   0x0396, /* 42: snre=0x001CC979, snr=24.523438 dB */
   0x03D8, /* 43: snre=0x001EBAC1, snr=24.238281 dB */
   0x041F, /* 44: snre=0x0020CD55, snr=23.953125 dB */
   0x046A, /* 45: snre=0x00230367, snr=23.667969 dB */
   0x04BA, /* 46: snre=0x00255F4A, snr=23.382813 dB */
   0x0510, /* 47: snre=0x0027E378, snr=23.097656 dB */
   0x056C, /* 48: snre=0x002A9290, snr=22.812500 dB */
   0x05CD, /* 49: snre=0x002D6F5C, snr=22.527344 dB */
   0x0635, /* 50: snre=0x00307CD2, snr=22.242188 dB */
   0x06A4, /* 51: snre=0x0033BE13, snr=21.957031 dB */
   0x071A, /* 52: snre=0x00373673, snr=21.671875 dB */
   0x0798, /* 53: snre=0x003AE976, snr=21.386719 dB */
   0x081F, /* 54: snre=0x003EDAD5, snr=21.101563 dB */
   0x08AE, /* 55: snre=0x00430E80, snr=20.816406 dB */
   0x0946, /* 56: snre=0x004788A0, snr=20.531250 dB */
   0x09E9, /* 57: snre=0x004C4D98, snr=20.246094 dB */
   0x0A96, /* 58: snre=0x00516209, snr=19.960938 dB */
   0x0B4E, /* 59: snre=0x0056CAD2, snr=19.675781 dB */
   0x0C13, /* 60: snre=0x005C8D14, snr=19.390625 dB */
   0x0CE3, /* 61: snre=0x0062AE30, snr=19.105469 dB */
   0x0DC2, /* 62: snre=0x006933CB, snr=18.820313 dB */
   0x0EAE, /* 63: snre=0x007023CF, snr=18.535156 dB */
   0x0FA9, /* 64: snre=0x0077846B, snr=18.250000 dB */
   0x10B5, /* 65: snre=0x007F5C0F, snr=17.964844 dB */
   0x11D1, /* 66: snre=0x0087B170, snr=17.679688 dB */
   0x12FE, /* 67: snre=0x00908B87, snr=17.394531 dB */
   0x143E, /* 68: snre=0x0099F188, snr=17.109375 dB */
   0x1592, /* 69: snre=0x00A3EAE7, snr=16.824219 dB */
   0x16FB, /* 70: snre=0x00AE7F4D, snr=16.539063 dB */
   0x1879, /* 71: snre=0x00B9B695, snr=16.253906 dB */
   0x1A0E, /* 72: snre=0x00C598C4, snr=15.968750 dB */
   0x1BBB, /* 73: snre=0x00D22E02, snr=15.683594 dB */
   0x1D80, /* 74: snre=0x00DF7E8C, snr=15.398438 dB */
   0x1F60, /* 75: snre=0x00ED92AA, snr=15.113281 dB */
   0x215B, /* 76: snre=0x00FC72A0, snr=14.828125 dB */
   0x2372, /* 77: snre=0x010C269B, snr=14.542969 dB */
   0x25A6, /* 78: snre=0x011CB6A2, snr=14.257813 dB */
   0x27F9, /* 79: snre=0x012E2A7C, snr=13.972656 dB */
   0x2A6B, /* 80: snre=0x0140899B, snr=13.687500 dB */
   0x2CFD, /* 81: snre=0x0153DB01, snr=13.402344 dB */
   0x2FB0, /* 82: snre=0x01682520, snr=13.117188 dB */
   0x3286, /* 83: snre=0x017D6DBF, snr=12.832031 dB */
   0x357D, /* 84: snre=0x0193B9D9, snr=12.546875 dB */
   0x3898, /* 85: snre=0x01AB0D76, snr=12.261719 dB */
   0x3BD7, /* 86: snre=0x01C36B8E, snr=11.976563 dB */
   0x3F39, /* 87: snre=0x01DCD5E0, snr=11.691406 dB */
   0x42BE, /* 88: snre=0x01F74CD4, snr=11.406250 dB */
   0x4668, /* 89: snre=0x0212CF59, snr=11.121094 dB */
   0x4A34, /* 90: snre=0x022F5AC5, snr=10.835938 dB */
   0x4E24, /* 91: snre=0x024CEAC3, snr=10.550781 dB */
   0x5235, /* 92: snre=0x026B793B, snr=10.265625 dB */
   0x5667, /* 93: snre=0x028AFE4C, snr=9.980469 dB */
   0x5AB8, /* 94: snre=0x02AB704C, snr=9.695313 dB */
   0x5F28, /* 95: snre=0x02CCC3D1, snr=9.410156 dB */
   0x63B4, /* 96: snre=0x02EEEBCE, snr=9.125000 dB */
   0x685A, /* 97: snre=0x0311D9B3, snr=8.839844 dB */
   0x6D18, /* 98: snre=0x03357DA1, snr=8.554688 dB */
   0x71ED, /* 99: snre=0x0359C6A7, snr=8.269531 dB */
   0x76D5, /* 100: snre=0x037EA30D, snr=7.984375 dB */
   0x7BCE, /* 101: snre=0x03A400A1, snr=7.699219 dB */
   0x80D6, /* 102: snre=0x03C9CD0A, snr=7.414063 dB */
   0x85EA, /* 103: snre=0x03EFF622, snr=7.128906 dB */
   0x8B08, /* 104: snre=0x04166A42, snr=6.843750 dB */
   0x902E, /* 105: snre=0x043D1892, snr=6.558594 dB */
   0x955A, /* 106: snre=0x0463F147, snr=6.273438 dB */
   0x9A89, /* 107: snre=0x048AE5D9, snr=5.988281 dB */
   0x9FBB, /* 108: snre=0x04B1E929, snr=5.703125 dB */
   0xA4EC, /* 109: snre=0x04D8EFA0, snr=5.417969 dB */
   0xAA1D, /* 110: snre=0x04FFEF34, snr=5.132813 dB */
   0xAF4C, /* 111: snre=0x0526DF6E, snr=4.847656 dB */
   0xB478, /* 112: snre=0x054DB95C, snr=4.562500 dB */
   0xB9A0, /* 113: snre=0x05747780, snr=4.277344 dB */
   0xBEC4, /* 114: snre=0x059B15B9, snr=3.992188 dB */
   0xC3E3, /* 115: snre=0x05C19120, snr=3.707031 dB */
   0xC8FD, /* 116: snre=0x05E7E7F0, snr=3.421875 dB */
   0xCE13, /* 117: snre=0x060E195F, snr=3.136719 dB */
   0xD323, /* 118: snre=0x06342583, snr=2.851563 dB */
   0xD82F, /* 119: snre=0x065A0D2F, snr=2.566406 dB */
   0xDD35, /* 120: snre=0x067FD1D8, snr=2.281250 dB */
   0xE238, /* 121: snre=0x06A5757B, snr=1.996094 dB */
   0xE736, /* 122: snre=0x06CAFA85, snr=1.710938 dB */
   0xEC31, /* 123: snre=0x06F063BD, snr=1.425781 dB */
   0xF129, /* 124: snre=0x0715B430, snr=1.140625 dB */
   0xF61D, /* 125: snre=0x073AEF25, snr=0.855469 dB */
   0xFB0F, /* 126: snre=0x0760180A, snr=0.570313 dB */
   0xFFFF, /* 127: snre=0x0785326D, snr=0.285156 dB */
};


/******************************************************************************
 BSAT_g1_P_GetSnrSnre_isr() - returns SNR in units of 1/256 dB
******************************************************************************/
static BERR_Code BSAT_g1_P_GetSnrSnre_isr(BSAT_ChannelHandle h, uint32_t *pSnr)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t snre, val1, val2, snre_min, snre_max, snre_scale, snre_idx, x, P_hi, P_lo, Q_hi, Q_lo;
   const uint16_t *snre_table;
   uint16_t snre_adj;

   snre = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_SNR_SNRE);

   if ((hChn->actualMode == BSAT_Mode_eBlindScan) || (hChn->actualMode == BSAT_Mode_eUnknown))
   {
      *pSnr = 0;
      goto done;
   }
   else if (BSAT_MODE_IS_LEGACY_QPSK(hChn->actualMode))
   {
      /* Legacy QPSK */
      snre_table = BSAT_snr_legacy_qpsk_table;
      snre_min = BSAT_snr_legacy_qpsk_min;
      snre_max = BSAT_snr_legacy_qpsk_max;
      snre_scale = BSAT_snr_legacy_qpsk_scale;
   }
   else if (BSAT_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      /* Turbo 8PSK */
      snre_table = BSAT_snr_ldpc_8psk_table;
      snre_min = BSAT_snr_turbo_8psk_min;
      snre_max = BSAT_snr_turbo_8psk_max;
      snre_scale = BSAT_snr_turbo_8psk_scale;
   }
   else if (BSAT_MODE_IS_DVBS2_8PSK(hChn->actualMode))
   {
      /* LDPC 8PSK */
      snre_table = BSAT_snr_ldpc_8psk_table;
      snre_min = BSAT_snr_ldpc_8psk_min;
      snre_max = BSAT_snr_ldpc_8psk_max;
      snre_scale = BSAT_snr_ldpc_8psk_scale;
   }
   else
   {
      /* LDPC/Turbo QPSK */
      snre_table = BSAT_snr_ldpc_qpsk_table;
      snre_min = BSAT_snr_ldpc_qpsk_min;
      snre_max = BSAT_snr_ldpc_qpsk_max;
      snre_scale = BSAT_snr_ldpc_qpsk_scale;
   }

   BSAT_DEBUG_SNR(BDBG_MSG(("snre=0x%08X, snre_min=0x%X, snre_max=0x%08X, snre_scale=%u", snre, snre_min, snre_max, snre_scale)));

   if ((snre >= snre_min) && (snre <= snre_max))
   {
      BMTH_HILO_32TO64_Mul(snre - snre_min, 65536, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, snre_scale, &Q_hi, &Q_lo);
      if (Q_lo > 0x0000FFFF)
         snre_adj = 0xFFFF;
      else
         snre_adj = (uint16_t)Q_lo;

      /* find snre_idx such that (snre_adj >= snre_table[snre_idx]) && (snre_adj < snre_table[snre_idx+1]) */
      /* use binary search */
      snre_idx = BSAT_SNRE_TABLE_N >> 1;
      x = BSAT_SNRE_TABLE_N >> 2;
      while (!((snre_adj >= snre_table[snre_idx]) && (snre_adj <= snre_table[snre_idx+1])))
      {
         if (snre_adj < snre_table[snre_idx])
            snre_idx -= x;
         else
            snre_idx += x;
         if (x > 1)
            x = x >> 1;
      }

      /* do linear interpolation */
      val1 = BSAT_snr_quant * (BSAT_SNRE_TABLE_N - snre_idx);     /* in 2^-8 dB SNR */
      val2 = BSAT_snr_quant * (BSAT_SNRE_TABLE_N - snre_idx - 1); /* in 2^-8 dB SNR */
      *pSnr = val1 - (((((snre_adj - snre_table[snre_idx]) * (val1 - val2) * 2) / (snre_table[snre_idx+1] - snre_table[snre_idx])) + 1) >> 1);

      if (hChn->actualMode == BSAT_Mode_eDvbs2_8psk_3_4)
         *pSnr -= 128; /* 0.5*256, adjusted to match with 4506 at 12dB SNR range */
   }
   else if (snre > snre_max)
      *pSnr = 0;
   else /* (snre < snre_min) */
      *pSnr = BSAT_SNR_MAX_SNRE;

   done:
   return BERR_SUCCESS;
}

/* #endif */
