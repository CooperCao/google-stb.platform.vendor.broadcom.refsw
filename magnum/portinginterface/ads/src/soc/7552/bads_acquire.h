/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
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

#ifndef _BADS_ACQUIRE_H__
#define _BADS_ACQUIRE__H__           

#ifdef __cplusplus
extern "C" {
#endif


/*Combined Timing Loop Coefficients*/
#define BCHP_DS_TLC_COMBO_COEFFS_SHIFT BCHP_DS_TLC_TLICOEFF_SHIFT
#define BCHP_DS_TLC_COMBO_COEFFS_MASK (BCHP_DS_TLC_reserved1_MASK | BCHP_DS_TLC_TLLCOEFF_SIGN_MASK | BCHP_DS_TLC_TLLCOEFF_MASK | BCHP_DS_TLC_reserved2_MASK | BCHP_DS_TLC_TLICOEFF_SIGN_MASK | BCHP_DS_TLC_TLICOEFF_MASK)

/*Combined Frequency Loop Coefficients*/
#define BCHP_DS_CFLC_COMBO_COEFFS_SHIFT BCHP_DS_CFLC_CFLICOEFF_SHIFT
#define BCHP_DS_CFLC_COMBO_COEFFS_MASK (BCHP_DS_CFLC_reserved1_MASK | BCHP_DS_CFLC_CFLLCOEFF_SIGN_MASK | BCHP_DS_CFLC_CFLLCOEFF_MASK | BCHP_DS_CFLC_reserved2_MASK | BCHP_DS_CFLC_CFLICOEFF_SIGN_MASK | BCHP_DS_CFLC_CFLICOEFF_MASK)

/*Combined Phase Loop Coefficients*/
#define BCHP_DS_EQ_CPLC_COMBO_COEFFS_SHIFT BCHP_DS_EQ_CPLC_CPLICOEFF_SHIFT
#define BCHP_DS_EQ_CPLC_COMBO_COEFFS_MASK (BCHP_DS_EQ_CPLC_CPLLCOEFF_MASK | BCHP_DS_EQ_CPLC_CPLICOEFF_MASK)

/*Combined DFE Taps*/
#define BCHP_DS_FRZ_COMBO_DFEFRZ_SHIFT BCHP_DS_FRZ_DFEFRZ1_6_SHIFT
#define BCHP_DS_FRZ_COMBO_DFEFRZ_MASK (BCHP_DS_FRZ_DFEFRZ31_36_MASK | BCHP_DS_FRZ_DFEFRZ25_30_MASK | BCHP_DS_FRZ_DFEFRZ19_24_MASK | BCHP_DS_FRZ_DFEFRZ13_18_MASK | BCHP_DS_FRZ_DFEFRZ7_12_MASK | BCHP_DS_FRZ_DFEFRZ1_6_MASK)

/*combined FEC counter clears*/
#define BCHP_DS_TPFEC_CLEARCNT2_SHIFT  BCHP_DS_TPFEC_CLR_BMPG2_SHIFT
#define BCHP_DS_TPFEC_CLEARCNT2_MASK   (BCHP_DS_TPFEC_CLR_UERC2_MASK | BCHP_DS_TPFEC_CLR_NBERC2_MASK | BCHP_DS_TPFEC_CLR_CBERC2_MASK | BCHP_DS_TPFEC_CLR_BMPG2_MASK)

/*****************************************************************************
 * ADS Function Prototypes Used by PI or Local 
 *****************************************************************************/
BERR_Code BADS_P_Initialize(BADS_3x7x_ChannelHandle hChn);
BERR_Code BADS_P_ChnLockStatus(BADS_3x7x_ChannelHandle hChn);
BERR_Code BADS_P_Acquire(BADS_3x7x_ChannelHandle hChn);
BERR_Code BADS_P_AbortAcquire(BADS_3x7x_ChannelHandle hChn); /* Acquire Early Exit */
BERR_Code BADS_P_HAB_Read_FFEDFE(BADS_3x7x_ChannelHandle hChn, uint8_t *HAB_Buffer, uint8_t Size_HAB, uint8_t FFE);
BERR_Code BADS_P_Read_Constellation(
    BADS_3x7x_ChannelHandle hChn,       /* [in] Device channel handle */
    int16_t nbrToGet_i16,                /* [in] Number values to get */
    int16_t *iVal_pi16,                  /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal_pi16,                  /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten_pi16              /* [out] Number of values gotten/read */
    );

#ifdef __cplusplus
}
#endif

#endif /* _BADS_ACQUIRE_PRIV_H__ */