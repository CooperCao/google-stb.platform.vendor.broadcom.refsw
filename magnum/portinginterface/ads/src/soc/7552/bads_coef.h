/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef BADS_COEF_H__
#define BADS_COEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_QAM_MODES 7
#define NUM_BAUD_RATES 33

extern const uint32_t BaudRates_TBL_u32[NUM_BAUD_RATES];
extern const uint32_t PhaseLoopAcqCoeffs_TBL_u32[NUM_BAUD_RATES];
extern const uint32_t PhaseLoopTrkCoeffs_TBL_u32[NUM_BAUD_RATES];
extern const uint32_t PhaseLoopTrkBurstModeCoeffs_TBL_u32[NUM_BAUD_RATES];
extern const uint16_t TimingLoopTrk1Coeffs_TBL_u16[NUM_BAUD_RATES];
extern const uint16_t TimingLoopTrk2Coeffs_TBL_u16[NUM_BAUD_RATES];
extern const uint16_t FrequencyLoopCoeffs_TBL_u16[NUM_BAUD_RATES];
extern const uint16_t FrequencyLoopBurstModeCoeffs_TBL_u16[NUM_BAUD_RATES];

typedef struct PhaseLoopSweep_FFT_s
{
uint16_t PosSweepRate;
uint32_t PosSweepStart;
uint16_t SweepTime;
}PhaseLoopSweep_FFT_t;
extern  const PhaseLoopSweep_FFT_t PhaseLoopSweep_FFT_TBL_struct[NUM_BAUD_RATES];

extern const uint32_t SNRLTHRESH_TBL_u32[NUM_QAM_MODES];
extern const uint32_t SNRHTHRESH_TBL_u32[NUM_QAM_MODES];

typedef struct FEC_s
{
uint32_t DS_FECU;
uint32_t DS_FECM;
uint32_t DS_FECL;
uint32_t DS_FECOUT_NCON;
uint32_t DS_FECOUT_NCODL;
}FEC_t;
extern const FEC_t AnnexA_FEC_TBL_struct[NUM_QAM_MODES];
extern const FEC_t AnnexB_FEC_TBL_struct[NUM_QAM_MODES];

extern const uint32_t DS_EQ_CTL_TBL_u32[NUM_QAM_MODES];
extern const uint32_t DS_EQ_CPL_TBL_u32[NUM_QAM_MODES];



#ifdef __cplusplus
}
#endif

#endif


