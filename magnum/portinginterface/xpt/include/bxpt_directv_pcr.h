/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/


#ifndef BXPT_DIRECTV_PCR_H__
#define BXPT_DIRECTV_PCR_H__

#include "bxpt_pcr.h"

#ifdef __cplusplus
extern "C"{
#endif

/*=************************ Module Overview ********************************
<verbatim>
The API module provides the direcTV only APIs for the PCR module. The APIs
in bxpt_pcr.h are required to complete PCR modules configuration in directv mode. 

The DirecTV PCR API operates on a PCR channel handle. 
</verbatim>
***************************************************************************/

/***************************************************************************
Summary:
Enumeration for the different types of channels supported by the record 
module.
****************************************************************************/
typedef enum BXPT_PcrMode 
{
	BXPT_PcrMode_eDirecTv,
	BXPT_PcrMode_eMpeg 
}
BXPT_PcrMode;

/***************************************************************************
Summary:
Set the stream type used by a PCR module.
	 
Description: 
This function sets mode for a given PCR module to DirecTV, or back to the
default MPEG.

Returns:
    BERR_SUCCESS                - Directv mode set 
    BERR_INVALID_PARAMETER      - Bad input parameter  
***************************************************************************/
BERR_Code BXPT_PCR_DirecTv_SetPcrMode( 
	BXPT_PCR_Handle hPcr, 			   /* [In]The Pcr handle */
	BXPT_PcrMode Mode
	);

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
	Gets the last PCR captured in last PCR Hi/Lo registers for DirecTv mode
Description:
	This function reads the values from PCR_LAST_PCR_HI/LO registers.
Returns:
    BERR_SUCCESS                - Retrieved last PCR
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetLastPcr( 
	BXPT_PCR_Handle hPcr, 			   /* [in]The Pcr handle */
	uint32_t *pPcr      			  /*[out] 32 bits of RTS*/
	);
#endif

/***************************************************************************
Summary:
	ISR version of BXPT_PCR_DirecTv_GetLastPcr
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetLastPcr_isr(
	BXPT_PCR_Handle hPcr, 			   /* [in]The Pcr handle */
	uint32_t *pPcr      			  /*[out] 32 bits of RTS*/
	);

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
	Gets the STC counter values for DirecTv mode
Description: 
	This function read the PCR STC counters from STC_HI/LO registers.
Returns:
    BERR_SUCCESS                - Retrieved STC counter values
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code	BXPT_PCR_DirecTv_GetStc( 
	BXPT_PCR_Handle hPcr,          /* [in]The Pcr handle */
	uint32_t *pStcHi               /*[out] 32 bits of RTS*/
	);
#endif

#ifdef __cplusplus
}
#endif

#endif
