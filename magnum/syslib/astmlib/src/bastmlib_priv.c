/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bsyslib.h"
#include "bastmlib.h"
#include "bastmlib_priv.h"

BDBG_MODULE(astmlib);

BERR_Code BASTMlib_P_StartTimer_isr(
	BASTMlib_Handle hAstm,
	BSYSlib_Timer_Handle hTimer, 
	unsigned long ulTimeout,
	BSYSlib_Timer_ExpiryHandler pfTimerExpired,
	void * pvParm1,
	int iParm2
)
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_Timer_Start_isr pfStart_isr;
	BSYSlib_Timer_Cancel_isr pfCancel_isr;
	
	BDBG_ENTER(BASTMlib_P_StartTimer_isr);
	
	if (hAstm->sSettings.cbTimer.pfStart_isr)
	{
		BSYSlib_Timer_Settings sTimer;

		sTimer.ulTimeout = ulTimeout;
		sTimer.pvParm1 = pvParm1;
		sTimer.iParm2 = iParm2;
		sTimer.pfTimerExpired = pfTimerExpired;

		pfStart_isr = hAstm->sSettings.cbTimer.pfStart_isr;
		pfCancel_isr = hAstm->sSettings.cbTimer.pfCancel_isr;
		pvParm1 = hAstm->sSettings.cbTimer.pvParm1;
		iParm2 = hAstm->sSettings.cbTimer.iParm2;

		/* clean up old timer if any */
		if (pfCancel_isr)
		{
			pfCancel_isr(pvParm1, iParm2, hTimer);
		}

		/* (re)schedule timer */
		rc = pfStart_isr(pvParm1, iParm2, hTimer, &sTimer);
		if (rc) goto error;
	}

	goto end;

error:

end:

	BDBG_LEAVE(BASTMlib_P_StartTimer_isr);
	return rc;
}

void BASTMlib_P_GetDefaultConfig(
	BASTMlib_Config * psConfig
)
{
	BDBG_ENTER(BASTMlib_P_GetDefaultConfig);

	BDBG_ASSERT(psConfig);

	psConfig->bEnabled = true;
	psConfig->eStcRate = BASTMlib_ClockRate_e45Khz;

	BASTMlib_P_Presentation_GetDefaultConfig(psConfig);
	BASTMlib_P_ClockCoupling_GetDefaultConfig(psConfig);

	BDBG_LEAVE(BASTMlib_P_GetDefaultConfig);
}
