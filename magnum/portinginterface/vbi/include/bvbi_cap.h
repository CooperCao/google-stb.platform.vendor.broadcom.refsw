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

#ifndef BVBI_CAP_H__
#define BVBI_CAP_H__

/*
 * Explanation of VEC counts and capabilities:
 * BVBI_NUM_VEC:          Number of full VBI encoders. Does not include
 *                        ITU-R 656 passthrough output.
 * BVBI_NUM_PTVEC:        Number of passthrough or ancillary VBI encoders.
 * BVBI_NUM_AMOLE:        Number of AMOL encoder cores, not including
 *                        ITU-R 656.
 * BVBI_NUM_AMOLE_656:    Number of AMOL encoder cores that are specifically
 *                        ITU-R 656.
 * BVBI_NUM_CCE:          Number of closed caption encoder cores, not
 *                        including ITU-R 656.
 * BVBI_NUM_CCE_656:      Number of closed caption encoder cores that are
 *                        specifically for ITU-R 656 output.
 * BVBI_NUM_CGMSAE:       Number of CGMS encoder cores, not including ITU-R
 *                        656.
 * BVBI_NUM_CGMSAE_656:   Number of CGMS encoder cores that are specifically
 *                        for ITU-R 656 output.
 * BVBI_NUM_GSE:          Number of Gemstar encoder cores, not including
 *                        ITU-R 656.
 * BVBI_NUM_GSE_656:      Number of Gemstar encoder cores that are
 *                        specifically for ITU-R 656 output.
 * BVBI_NUM_SCTEE:        Number of SCTE encoder cores. Assume first core
 *                        that has one is on the primary VEC path.
 * BVBI_NUM_SCTEE_656:    Number of 656 SCTE encoder cores. At present time,
 *                        always zero.
 * BVBI_NUM_TTE:          Number of teletext encoder cores. Assume first core
 *                        that has one is on the primary VEC path. Does not
 *                        include 656 (bypass) cores.
 * BVBI_NUM_TTE_656:      Number of 656 (bypass) teletext encoder cores.
 *                        Assume first core that has one is on the primary
 *                        VEC path.
 * BVBI_NUM_WSE:          Number of WSE cores, not including ITU-R 656.
 * BVBI_NUM_WSE_656:      Number of WSE cores that are specifically for
 *                        ITU-R 656 output.
 * BVBI_NUM_IN656:        Number of ITU-R 656 inputs (IN656 cores)
 * BVBI_NUM_ANCI656_656:  Number of ANCI656_656 or ANCI656_Ancil VEC cores.
 */

#include "bvbi_cap_priv.h"

#endif /* BVBI_CAP_H__ */
