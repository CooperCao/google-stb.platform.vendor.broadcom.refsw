/********************************************************************************************
*     (c)2004-2010 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 *    DTCP content management function header
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *********************************************************************************************/
/*! \file b_dtcp_exch_key.h
 *  \brief Implement dtcp content management functions.
 */
#ifndef B_DTCP_EXCH_KEY_H
#define B_DTCP_EXCH_KEY_H  

/* For M6, each EMI need a exchange Key (Kx), There 4 types of EMI, e.g.
 * Mode A: copy never
 * Mode B: Copy One Generation.
 * Mode C: No More COpy
 * N/A:    Copy Freely. 
 * For AES, a single exchange Key is used for all EMI value.
 */
#define DTCP_EXCHANGE_KEY_COUNT		4

/*! \skipline foreard declaration
 */
struct B_DTCP_AkeCoreData;

/*! \struct B_DTCP_ExchangeKeyData
 */
typedef struct B_DTCP_ExchKeyData
{
	unsigned char Label;
	unsigned char ExchangeKeys[DTCP_EXCHANGE_KEY_COUNT][DTCP_EXCHANGE_KEY_SIZE];
	unsigned int Valid;
	bool Initialized;
}B_DTCP_ExchKeyData_T;

/*! \brief Inialize exchange key context, called by source device only
 *  \param[in] pExchKeyData input exchange key data.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_InitExchKeyData(BCRYPT_Handle hBcrypt, B_DTCP_ExchKeyData_T * pExchKeyData);

/*! \brief retrieve exchange key from AKE core data, called by source device.
 *  \param[in] CoreData pointer to AKE core data.
 *  \param[in] Type exchange key cipher type. AES or M6
 *  \param[out] key exchange key data.
 *  \param[out] Label current source device's exchange key label.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_GetExchKeyFromCore(struct B_DTCP_AkeCoreData * CoreData, B_ExchangeKeyCipher_T Type, 
		unsigned char key[DTCP_EXCHANGE_KEY_SIZE],
		unsigned char * Label);

/*! \brief retrieve exchange key from AKE session data, called by sink device.
 *  \param[in] hAkeHandle Ake session data pointer.
 *  \param[in] Cipher exhcange key cipher type B_ExchKeyCpher_eM6 or B_ExchKeyCipher_eAes.
 *  \param[out] key exchange key data.
 *  \param[out] Label current source device's exchange key label.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_GetExchKeyFromSession(void * hAkeHandle,
		B_ExchangeKeyCipher_T Cipher, 
		unsigned char key[DTCP_EXCHANGE_KEY_SIZE],
		unsigned char * Label);
/*! \brief check if given session's exchange key is valid. 
 */
bool B_DTCP_IsExchKeyValid(void *hAkeHandle, B_ExchangeKeyCipher_T Cipher);
/*! \brief Source device expire/update current exchange keys.
 *  \param[in] pExchKeys pointer to source device's exchange key data.
 *  \retval BERR_SUCCESS or error code
 */
BERR_Code B_DTCP_UpdateSourceExchKeys(BCRYPT_Handle hBcrypt, B_DTCP_ExchKeyData_T * pExchKeys);

#endif /* B_DTCP_EXCH_KEY_H */
