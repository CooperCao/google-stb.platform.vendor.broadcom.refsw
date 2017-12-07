/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * [File Description:]
 *
 ***************************************************************************/
#include "bvdc_compositor_priv.h"
#include "bvdc_pep_priv.h"

BDBG_MODULE(BVDC_PEP);

/***************************************************************************
 * {private}
 * This section is for the sharpness algorithms
 */
#define BVDC_P_SHARPNESS_VAL_MIN                 INT16_MIN
#define BVDC_P_SHARPNESS_VAL_MAX                 INT16_MAX
#define BVDC_P_SHARPNESS_CENTER_INDEX            3
#define BVDC_P_SHARPNESS_MAX_INDEX               19
#define BVDC_P_SHAPRNESS_NUM_PEAKING_VALUE_PAIRS 48

/*************************************************************************
 *  {secret}
 *  BVDC_P_Sharpness_Calculate_Gain_Value_isr
 *  Calculate LUMA_Control.gain values from sSharpness value
 *  for TNT_CMP_0_V0_LUMA_CONTROL_GAIN
 **************************************************************************/
void BVDC_P_Sharpness_Calculate_Gain_Value_isr
    ( const int16_t     sSharpness,
      const int16_t     sMinGain,
      const int16_t     sCenterGain,
      const int16_t     sMaxGain,
      uint32_t         *ulSharpnessGain )
{
    const int32_t  sMaxValue    = BVDC_P_SHARPNESS_VAL_MAX;
    const int32_t  sMinValue    = BVDC_P_SHARPNESS_VAL_MIN;
    const int32_t  sCenterValue = 0;

    /* this is a linear mapping from [-32768...0...32767] to [0...sCenterGain...sMaxGain] */
    *ulSharpnessGain = (sSharpness>0)?
        ((sSharpness - sCenterValue) * (sMaxGain - sCenterGain) / (sMaxValue - sCenterValue) + sCenterGain) :
        ((sSharpness - sCenterValue) * (sMinGain - sCenterGain) / (sMinValue - sCenterValue) + sCenterGain);

}

/* End of File */
