/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/


#ifndef BVBI_CHIP_PRIV_H__
#define BVBI_CHIP_PRIV_H__

#include "bchp_common.h"

/*
 * Explanation of VEC/VDEC counts and capabilities:
 * BVBI_P_HAS_SCTEE_CO      SCTE encoder has component only registers.
 * BVBI_P_HAS_XSER_TT:      TTX core has serial output capability.
 * BVBI_P_TTXADR_WAROUND:   TTX core has the PR22720 bug in accessing DRAM. A
 *                          software fix is provided.
 * BVBI_P_CGMSAE_VER2:      CGMSAE core is version first appearing in 3548-A0.
 *                          Capable of CGMS-B output.
 * BVBI_P_CGMSAE_VER3:      CGMSAE core is version first appearing in 3548-B0.
 *                          Capable of CEA-805-D style output.
 * BVBI_P_CGMSAE_VER4:      CGMSAE core is version first appearing in 7420-A0.
 *                          The BIT_ORDER bitfields were removed.
 * BVBI_P_CGMSAE_VER5:      CGMSAE core is version first appearing in 7420-B0.
 *                          The BIT_ORDER bitfields were restored.
 * BVBI_P_WSE_VER2:         WSE core is version first appearing in 3548-A0.
 *                          ITU-R 656 output is handled in a different way.
 * BVBI_P_WSE_VER3:         WSE core is version first appearing in 7601-A0.
 *                          Capable of IEC-62375 output on 576P video.
 * BVBI_P_WSE_VER4:         WSE core is version first appearing in 3548-B2.
 *                          Has a bug fix related to WSS and VPS output.
 * BVBI_P_WSE_VER5:         WSE core is version first appearing in 7420-B0.
 *                          Register file is identical to VER3 cores.
 * BVBI_P_GSE_VER2:         GSE core is version first appearing in 7408-A0.
 *                          Capable of TVGX2 output.
 * BVBI_P_CCE_VER2:         CCE core version first appearing in 7422-A0. Has
 *                          capability to support subset of SCTE-20 and
 *                          SCTE-21 specs.
 * BVBI_P_ENC_NUM_CROSSBAR_REG
 *                          Number of analog paths through the VBI_ENC
 *                          crossbar that are available for use by BVBI
 *                          software. Two are reserved for use by BVDC.
 * BVBI_P_ENC_NUM_CROSSBAR_REG_656
 *                          Number of ITU-R 656 paths through the VBI_ENC
 *                          crossbar that are available for use by BVBI
 *                          software. Two are reserved for use by BVDC.
 */

#if (BCHP_CHIP==7422) ||(BCHP_CHIP==7425) || (BCHP_CHIP==7435)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7145)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7439)
    #if (BCHP_VER == BCHP_VER_A0)
        #define BVBI_P_HAS_EXT_656 1
        #define BVBI_P_HAS_XSER_TT 1
        #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
        #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
        #define BVBI_P_CGMSAE_VER5 1
        #define BVBI_P_WSE_VER5 1
        #define BVBI_P_GSE_VER2 1
        #define BVBI_P_CCE_VER2 1
    #else
        #define BVBI_P_HAS_EXT_656 1
        #define BVBI_P_HAS_XSER_TT 1
        #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
        #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
        #define BVBI_P_CGMSAE_VER5 1
        #define BVBI_P_WSE_VER5 1
        #define BVBI_P_GSE_VER2 1
        #define BVBI_P_CCE_VER2 1
    #endif

#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268) || (BCHP_CHIP==7260)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7278)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP== 74371) || (BCHP_CHIP==7366) || (BCHP_CHIP==7364) || \
      (BCHP_CHIP == 7586) ||(BCHP_CHIP==7250)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7543)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 0
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 0
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7344)  || (BCHP_CHIP==7346)  || (BCHP_CHIP==7231)  || \
      (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || (BCHP_CHIP==75845) || \
      (BCHP_CHIP==74295) || (BCHP_CHIP==73465)
    #define BVBI_P_HAS_EXT_656 1
    #define BVBI_P_HAS_XSER_TT 1
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 5
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || \
      (BCHP_CHIP==7360) || (BCHP_CHIP == 7362) || (BCHP_CHIP == 7228) || \
      (BCHP_CHIP==73625)
    #define BVBI_P_HAS_EXT_656 0
    #define BVBI_P_HAS_XSER_TT 0
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 0
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#elif ((BCHP_CHIP==7563) || (BCHP_CHIP==75635) || (BCHP_CHIP==75525))
    #define BVBI_P_HAS_EXT_656 0
    #define BVBI_P_HAS_XSER_TT 0
    #define BVBI_P_ENC_NUM_CROSSBAR_REG 6
    #define BVBI_P_ENC_NUM_CROSSBAR_REG_656 0
    #define BVBI_P_CGMSAE_VER5 1
    #define BVBI_P_WSE_VER5 1
    #define BVBI_P_GSE_VER2 1
    #define BVBI_P_CCE_VER2 1

#else
    #error Unknown video chip name
#endif

/* I should have started these series in a different way */
#if !defined(BVBI_P_CGMSAE_VER2) && !defined(BVBI_P_CGMSAE_VER3) && \
    !defined(BVBI_P_CGMSAE_VER4) && !defined(BVBI_P_CGMSAE_VER5)
        #define BVBI_P_CGMSAE_VER1 1
#endif
#if !defined(BVBI_P_WSE_VER2) && !defined(BVBI_P_WSE_VER3) && \
    !defined(BVBI_P_WSE_VER4) && !defined(BVBI_P_WSE_VER5)
    #define BVBI_P_WSE_VER1 1
#endif
#if !defined(BVBI_P_GSE_VER2)
    #define BVBI_P_GSE_VER1 1
#endif
#if !defined(BVBI_P_CCE_VER2)
    #define BVBI_P_CCE_VER1 1
#endif

#endif /* BVBI_CHIP_PRIV_H__ */
