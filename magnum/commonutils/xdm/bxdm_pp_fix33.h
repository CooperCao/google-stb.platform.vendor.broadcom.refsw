/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_mixedfraction(const uint32_t oiWhole, const uint32_t uiNumerator, const uint32_t uiDenominator);

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BXDM_PP_Fix33_t BXDM_PP_Fix33_from_int32(const int32_t iValue);
#endif

BXDM_PP_Fix33_t BXDM_PP_Fix33_from_uint32(const uint32_t uiValue);

/* BXDM_PP_Fix33_t math operations */
BXDM_PP_Fix33_t BXDM_PP_Fix33_add(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_sub(const BXDM_PP_Fix33_t fixOperand1, const BXDM_PP_Fix33_t fixOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_mulu(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_divu(const BXDM_PP_Fix33_t fixOperand1, const uint32_t uiOperand2);
BXDM_PP_Fix33_t BXDM_PP_Fix33_neg(const BXDM_PP_Fix33_t fixOperand);

/* Convert from BXDM_PP_Fix33_t */
int32_t BXDM_PP_Fix33_to_int32(const BXDM_PP_Fix33_t fixValue);
uint32_t BXDM_PP_Fix33_to_uint32(const BXDM_PP_Fix33_t fixValue);

#ifdef __cplusplus
}
#endif

#endif /* BXDM_PP_FIX33_H_ */
