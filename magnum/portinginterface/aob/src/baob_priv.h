/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#ifndef BAOB_PRIV_H__
#define BAOB_PRIV_H__

#include "bchp.h"
#include "bhab.h"
#include "baob.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Defines raw HAB test mesg hdr (struct) */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }

#define BAOB_CORE_TYPE		0x2
#define BAOB_CORE_ID		0x0
#define CORE_TYPE_GLOBAL	0x0

typedef enum BAOB_OpCodesDS{
        BAOB_eAcquire = 0x10,
        BAOB_eAcquireParamsWrite = 0x11,
        BAOB_eAcquireParamsRead = 0x91,
        BAOB_eAnnexASymbolRateWrite = 0x12,
        BAOB_eAnnexASymbolRateRead = 0x92,
        BAOB_eScanParamsWrite = 0x13,
        BAOB_eScanParamsRead = 0x93,
        BAOB_eAcqWordsWrite = 0x14,
        BAOB_eAcqWordsRead = 0x94,
        BAOB_eResetStatus = 0x15,
        BAOB_eRequestAsyncStatus = 0x16,
        BAOB_eGetStatus = 0x96,
        BAOB_eGetConstellation = 0xA3,
        BAOB_eGetVersion = 0xB9,
        BAOB_eGetVersionInfo = 0xBA,
        BAOB_ePowerCtrlOn = 0x19,
        BAOB_ePowerCtrlOff = 0x18,
        BAOB_ePowerCtrlRead = 0x19
}BAOB_OpCodesDS;

/***************************************************************************
Summary:
	The handle for Out-of-Band Downstream module.

Description:

See Also:
	BAOB_Open()

****************************************************************************/

typedef struct BAOB_P_Handle
{
    uint32_t magicId;					/* Used to check if structure is corrupt */
    BCHP_Handle hChip;
    BREG_Handle hRegister;
    BINT_Handle hInterrupt;
    BHAB_DevId devId;
    BHAB_Handle hHab;
    BAOB_CallbackFunc pCallback[BAOB_Callback_eLast];
    void *pCallbackParam[BAOB_Callback_eLast];
    bool enableFEC;						/* enable OOB FEC*/
    BKNI_MutexHandle mutex;				/* mutex to protect lock status*/
    bool isLock;						/* current lock status */
    unsigned long ifFreq;				/* IF frequency used. */
    bool bPowerdown;
    BAOB_NyquistFilter nyquist;         /* specifies Nyquist filter rolloff */
    BAOB_OutputMode outputMode;
} BAOB_P_Handle;


#ifdef __cplusplus
}
#endif

#endif
