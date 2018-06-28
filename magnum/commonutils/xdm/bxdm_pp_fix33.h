/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

/* BXDM_PP_FIX33 is an implementation of fixed point math in the format "Q33.31".
 * The whole component is 33 bits to enable a full -2^32 to 2^32 range of
 * possible values.  This range makes BXDM_PP_FIX33 sufficient for applications
 * such as PTS interpolation where the full 32-bit range is required.
 */

#ifndef BXDM_PP_FIX33_H_
#define BXDM_PP_FIX33_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef int64_t BXDM_PP_Fix33_t;

/* Convert to BXDM_PP_Fix33_t */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_mixedfraction_isrsafe(const uint32_t oiWhole, const uint32_t uiNumerator, const uint32_t uiDenominator);

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_int32_isrsafe(const int32_t iValue);
#endif

BXDM_PP_Fix33_t BXDM_PP_Fix33_from_uint32_isrsafe(const uint32_t uiValue);

/* BXDM_PP_Fix33_t math operations */
BXDM_PP_Fix33_t BXDM_PP_Fix33_add_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_sub_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_mulu_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_divu_isrsafe(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_neg_isrsafe(const BXDM_PP_Fix33_t fixOperand);

/* Convert from BXDM_PP_Fix33_t */
int32_t BXDM_PP_Fix33_to_int32_isrsafe(const BXDM_PP_Fix33_t fixValue);
uint32_t BXDM_PP_Fix33_to_uint32_isrsafe(const BXDM_PP_Fix33_t fixValue);

#ifdef __cplusplus
}
#endif

#endif /* BXDM_PP_FIX33_H_ */
