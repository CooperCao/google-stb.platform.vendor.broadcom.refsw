/******************************************************************************
 *    (c)2010-2011 Broadcom Corporation
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

#ifndef BAOB_PRIV_H__
#define BAOB_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef LEAP_BASED_CODE
#define BERR_AOB_ID				 5
#endif
#define DEV_MAGIC_ID            ((BERR_AOB_ID<<16) | 0xFACE)
/***************************************************************************
Summary:
    Error Codes specific to BAOB

Description:

See Also:

****************************************************************************/
#define BAOB_ERR_NOTAVAIL_MODULATION_TYPE       BERR_MAKE_CODE(BERR_AOB_ID, 0)
#define BAOB_ERR_NOTAVAIL_CHN_NO                BERR_MAKE_CODE(BERR_AOB_ID, 1)
#define BAOB_ERR_SYMBOLRATE_EXCEEDED            BERR_MAKE_CODE(BERR_AOB_ID, 2)

#define BREG_WriteField(h, Register, Field, Data) \
  BREG_Write32(h, BCHP_##Register, ((BREG_Read32(h, BCHP_##Register) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))


#define BREG_ReadField(h, Register, Field) \
  ((BREG_Read32(h, BCHP_##Register) & BCHP_MASK(Register,Field)) >> \
  BCHP_SHIFT(Register,Field))



/***************************************************************************
Summary:
	The handle for OOB  Downstream module.

Description:

See Also:
	BAOB_Open()

****************************************************************************/
 
#ifndef LEAP_BASED_CODE
typedef struct BAOB_P_Handle
{
	BAOB_Settings settings;
	void *pImpl;						/* Device specific structure */
} BAOB_P_Handle;


typedef struct BAOB_P_3x7x_Handle
{
    uint32_t			magicId;                   /* Used to check if structure is corrupt */
	BCHP_Handle			hChip;
    BREG_Handle			hRegister;
	BINT_Handle			hInterrupt;
	unsigned int		mxChnNo;
	BTMR_TimerHandle	hTimer;
	BAOB_Status_t		*pStatus;
	BAOB_AcqParams_t	*pAcqParam;
	BKNI_EventHandle	hIntEvent;
	BAOB_PowerStatus PowerStatus;
	BAOB_CallbackFunc pCallback[BAOB_Callback_eLast];
	void *pCallbackParam[BAOB_Callback_eLast];
} BAOB_P_3x7x_Handle;
#endif

#ifdef __cplusplus
}
#endif

#endif