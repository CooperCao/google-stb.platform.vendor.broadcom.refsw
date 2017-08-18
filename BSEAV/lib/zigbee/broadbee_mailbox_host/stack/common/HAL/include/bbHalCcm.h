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

/*******************************************************************************
 *
 * DESCRIPTION:
 *   CCM* routines HAL handler.
 *
*******************************************************************************/

#ifndef _BB_HAL_CCM_H
#define _BB_HAL_CCM_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Sets the CCM encryption key for a subsequent operations.

 \param[in] key - the key to be set.
 \return Nothing.
 ****************************************************************************************/
void HAL_CcmSetKey(const uint8_t *key);

/************************************************************************************//**
 \brief Starts the authentication transformation with given nonce value.

 \param[in] nonce - the given nonce value.
 \param[in] mLength - the given message length value.
 \param[in] m - the M value.
 \param[in] hasA - true if A was set.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmStartAuthTransformation(uint8_t *nonce, uint8_t mLength, uint8_t m, bool hasA);

/************************************************************************************//**
 \brief Make a step of the authentication transformation with a given piece of data.

 \param[in] authData - a pointer to the piece of authentication data.
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmAuthTransformation(uint8_t *authData);

/************************************************************************************//**
 \brief Performs a step of the encryption transformation.

 \param[in] nonce - the given nonce value.
 \param[in] message - a pointer to the piece of data to be encrypted/decrypted.
 \param[in] iteration - current index to form Ai
 \return pointer to the resulting string.
 ****************************************************************************************/
uint8_t* HAL_CcmEncryptionTransformation(uint8_t *nonce, uint8_t *message, uint8_t iteration);

/************************************************************************************//**
 \brief Calculates AES-CMAC value.

 \param[in] key - the security key to be used.
 \param[in] input - the data to be calculated.
 \param[in] length - length of the data to be calculated.
 \param[out] mac - the result output.
 \return Nothing.
 ****************************************************************************************/
void HAL_AES_CMAC(uint8_t *key, uint8_t *input, uint8_t length, uint8_t *mac);

/************************************************************************************//**
 \brief Clears the contents of CcmOut to all zeros.
 ****************************************************************************************/
void HAL_CcmClearOutput(void);

/************************************************************************************//**
 \brief Performs a step of the Hash function calculation.

 \param[in] data - pointer to the given data value.
 \return pointer to the resulting hash.
 ****************************************************************************************/
uint8_t* HAL_CbbHash(uint8_t *data);

/* These tools are needed by SoC. Either put into a separate module or move to SOC. */
void GenerateSubkey(uint8_t *key, uint8_t *K1, uint8_t *K2);
void Padding(uint8_t *lastb, uint8_t *pad, uint8_t length);
void XOR128(uint8_t *a, uint8_t *b, uint8_t *out);
/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Flags for Start Authentication. Reserved || Adata || M || L.
 *        Reserved == 0. One bit.
 *        Adata == 1 if Length(a) != 0 and 0 otherwise. One bit.
 *        M == (M-2)/2 if M > 0 and 0 therwise. 3 bits.
 *        L == L - 1. L == 2 - 1 == 1. 3 bits.
 */
#define SET_ADATA_FLAG(field, value) ((field) | ((value) ? 0x40 : 0))
#define SET_M_FLAG(field, value) ((field) | ((value) == 0 ? 0 : ((((value) - 2) >> 1) << 3)))
#define SET_L_FLAG() (SECURITY_CCM_L - 1)

#endif /* _BB_HAL_CCM_H */

/* eof bbHalCcm.h */