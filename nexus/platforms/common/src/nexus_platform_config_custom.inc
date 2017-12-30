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
 *****************************************************************************/

#if NEXUS_NUM_COMPOSITE_OUTPUTS
static const NEXUS_VideoDac g_NEXUS_compositeDacs[NEXUS_NUM_COMPOSITE_OUTPUTS] = {
#if ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) ||          \
     (BCHP_CHIP == 7435) || (BCHP_CHIP == 7445) ||          \
     (BCHP_CHIP == 7250 && defined(NEXUS_USE_7250_SV)) ||   \
     (BCHP_CHIP == 7439) ||                                 \
     ((BCHP_CHIP == 7429 || BCHP_CHIP==74295) && (NEXUS_PLATFORM != 97241 && NEXUS_PLATFORM != 972415))) /* i.e. 7429[5] family execpt 7241[5] bond outs */
NEXUS_VideoDac_e3
#else
NEXUS_VideoDac_e0
#endif
};
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
static const struct {
    NEXUS_VideoDac dacY, dacPr, dacPb;
} g_NEXUS_componentDacs[NEXUS_NUM_COMPONENT_OUTPUTS] = {
#if BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7435 || BCHP_CHIP == 7445 || \
    ((BCHP_CHIP == 7439) && (BCHP_VER >= BCHP_VER_B0))
{NEXUS_VideoDac_e0, NEXUS_VideoDac_e2, NEXUS_VideoDac_e1}
#elif BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP == 7584 || BCHP_CHIP == 75845 || BCHP_CHIP == 73465
{NEXUS_VideoDac_e2, NEXUS_VideoDac_e3, NEXUS_VideoDac_e1}
#elif  BCHP_CHIP==7344
{NEXUS_VideoDac_e1, NEXUS_VideoDac_e3, NEXUS_VideoDac_e2}
#elif (BCHP_CHIP == 7358) || (BCHP_CHIP == 7552) || (BCHP_CHIP == 7360) || \
      (BCHP_CHIP == 7362) || (BCHP_CHIP == 7228) || (BCHP_CHIP == 73625)
{NEXUS_VideoDac_e2, NEXUS_VideoDac_e3, NEXUS_VideoDac_e1}
#elif  (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295)
    #if (NEXUS_PLATFORM == 97241) || (NEXUS_PLATFORM == 972415)
    {NEXUS_VideoDac_e2, NEXUS_VideoDac_e3, NEXUS_VideoDac_e1}
    #else
    {NEXUS_VideoDac_e0, NEXUS_VideoDac_e2, NEXUS_VideoDac_e1}
    #endif
#elif (BCHP_CHIP==7364) || (BCHP_CHIP==7366) || (BCHP_CHIP==7250)
    {NEXUS_VideoDac_e0, NEXUS_VideoDac_e2, NEXUS_VideoDac_e1}
#else
    {NEXUS_VideoDac_e4, NEXUS_VideoDac_e5, NEXUS_VideoDac_e3}
#endif
};
#endif
