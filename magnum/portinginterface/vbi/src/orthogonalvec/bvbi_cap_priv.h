/***************************************************************************
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
 *
 * Module Description:
 *   See Module Overview below
 *
 ***************************************************************************/

#ifndef BVBI_CAP_PRIV_H__
#define BVBI_CAP_PRIV_H__

#include "bchp_common.h"

#if defined(BCHP_IT_2_REG_START)
#define BVBI_NUM_VEC 3
#elif defined(BCHP_IT_1_REG_START)
#define BVBI_NUM_VEC 2
#elif defined(BCHP_IT_0_REG_START)
#define BVBI_NUM_VEC 1
#else
#define BVBI_NUM_VEC 0
#endif

#if defined(BCHP_ITU656_2_REG_START)
#define BVBI_NUM_PTVEC 3
#elif defined(BCHP_ITU656_1_REG_START)
#define BVBI_NUM_PTVEC 2
#elif defined(BCHP_ITU656_0_REG_START)
#define BVBI_NUM_PTVEC 1
#else
#define BVBI_NUM_PTVEC 0
#endif

#if defined(BCHP_AMOLE_2_REG_START)
#define BVBI_NUM_AMOLE 3
#elif defined(BCHP_AMOLE_1_REG_START)
#define BVBI_NUM_AMOLE 2
#elif defined(BCHP_AMOLE_0_REG_START)
#define BVBI_NUM_AMOLE 1
#else
#define BVBI_NUM_AMOLE 0
#endif

#if defined(BCHP_AMOLE_ANCIL_2_REG_START)
#define BVBI_NUM_AMOLE_656 3
#elif defined(BCHP_AMOLE_ANCIL_1_REG_START)
#define BVBI_NUM_AMOLE_656 2
#elif defined(BCHP_AMOLE_ANCIL_0_REG_START)
#define BVBI_NUM_AMOLE_656 1
#else
#define BVBI_NUM_AMOLE_656 0
#endif

#if defined(BCHP_CCE_2_REG_START)
#define BVBI_NUM_CCE 3
#elif defined(BCHP_CCE_1_REG_START)
#define BVBI_NUM_CCE 2
#elif defined(BCHP_CCE_0_REG_START)
#define BVBI_NUM_CCE 1
#else
#define BVBI_NUM_CCE 0
#endif

#if defined(BCHP_CCE_ANCIL_2_REG_START)
#define BVBI_NUM_CCE_656 3
#elif defined(BCHP_CCE_ANCIL_1_REG_START)
#define BVBI_NUM_CCE_656 2
#elif defined(BCHP_CCE_ANCIL_0_REG_START)
#define BVBI_NUM_CCE_656 1
#else
#define BVBI_NUM_CCE_656 0
#endif

#if defined(BCHP_CGMSAE_2_REG_START)
#define BVBI_NUM_CGMSAE 3
#elif defined(BCHP_CGMSAE_1_REG_START)
#define BVBI_NUM_CGMSAE 2
#elif defined(BCHP_CGMSAE_0_REG_START)
#define BVBI_NUM_CGMSAE 1
#else
#define BVBI_NUM_CGMSAE 0
#endif

#if defined(BCHP_CGMSAE_ANCIL_2_REG_START)
#define BVBI_NUM_CGMSAE_656 3
#elif defined(BCHP_CGMSAE_ANCIL_1_REG_START)
#define BVBI_NUM_CGMSAE_656 2
#elif defined(BCHP_CGMSAE_ANCIL_0_REG_START)
#define BVBI_NUM_CGMSAE_656 1
#else
#define BVBI_NUM_CGMSAE_656 0
#endif

#if defined(BCHP_GSE_2_REG_START)
#define BVBI_NUM_GSE 3
#elif defined(BCHP_GSE_1_REG_START)
#define BVBI_NUM_GSE 2
#elif defined(BCHP_GSE_0_REG_START)
#define BVBI_NUM_GSE 1
#else
#define BVBI_NUM_GSE 0
#endif

#if defined(BCHP_GSE_ANCIL_2_REG_START)
#define BVBI_NUM_GSE_656 3
#elif defined(BCHP_GSE_ANCIL_1_REG_START)
#define BVBI_NUM_GSE_656 2
#elif defined(BCHP_GSE_ANCIL_0_REG_START)
#define BVBI_NUM_GSE_656 1
#else
#define BVBI_NUM_GSE_656 0
#endif

#if defined(BCHP_SCTE_2_REG_START)
#define BVBI_NUM_SCTEE 3
#elif defined(BCHP_SCTE_1_REG_START)
#define BVBI_NUM_SCTEE 2
#elif defined(BCHP_SCTE_0_REG_START)
#define BVBI_NUM_SCTEE 1
#else
#define BVBI_NUM_SCTEE 0
#endif

#if defined(BCHP_SCTE_ANCIL_2_REG_START)
#define BVBI_NUM_SCTEE_656 3
#elif defined(BCHP_SCTEE_ANCIL_1_REG_START)
#define BVBI_NUM_SCTEE_656 2
#elif defined(BCHP_SCTEE_ANCIL_0_REG_START)
#define BVBI_NUM_SCTEE_656 1
#else
#define BVBI_NUM_SCTEE_656 0
#endif

#if defined(BCHP_TTE_2_REG_START)
#define BVBI_NUM_TTE 3
#elif defined(BCHP_TTE_1_REG_START)
#define BVBI_NUM_TTE 2
#elif defined(BCHP_TTE_0_REG_START)
#define BVBI_NUM_TTE 1
#else
#define BVBI_NUM_TTE 0
#endif

#if defined(BCHP_TTE_ANCIL_2_REG_START)
#define BVBI_NUM_TTE_656 3
#elif defined(BCHP_TTE_ANCIL_1_REG_START)
#define BVBI_NUM_TTE_656 2
#elif defined(BCHP_TTE_ANCIL_0_REG_START)
#define BVBI_NUM_TTE_656 1
#else
#define BVBI_NUM_TTE_656 0
#endif

#if defined(BCHP_WSE_2_REG_START)
#define BVBI_NUM_WSE 3
#elif defined(BCHP_WSE_1_REG_START)
#define BVBI_NUM_WSE 2
#elif defined(BCHP_WSE_0_REG_START)
#define BVBI_NUM_WSE 1
#else
#define BVBI_NUM_WSE 0
#endif

#if defined(BCHP_WSE_ANCIL_2_REG_START)
#define BVBI_NUM_WSE_656 3
#elif defined(BCHP_WSE_ANCIL_1_REG_START)
#define BVBI_NUM_WSE_656 2
#elif defined(BCHP_WSE_ANCIL_0_REG_START)
#define BVBI_NUM_WSE_656 1
#else
#define BVBI_NUM_WSE_656 0
#endif

#if defined(BCHP_IN656_2_REG_START)
#define BVBI_NUM_IN656 3
#elif defined(BCHP_IN656_1_REG_START)
#define BVBI_NUM_IN656 2
#elif defined(BCHP_IN656_0_REG_START)
#define BVBI_NUM_IN656 1
#else
#define BVBI_NUM_IN656 0
#endif

#if defined(BCHP_ANCI656_ANCIL_2_REG_START)
#define BVBI_NUM_ANCI656_656 3
#elif defined(BCHP_ANCI656_ANCIL_1_REG_START)
#define BVBI_NUM_ANCI656_656 2
#elif defined(BCHP_ANCI656_ANCIL_0_REG_START)
#define BVBI_NUM_ANCI656_656 1
#else
#define BVBI_NUM_ANCI656_656 0
#endif

#endif /* BVBI_CAP_PRIV_H__ */
