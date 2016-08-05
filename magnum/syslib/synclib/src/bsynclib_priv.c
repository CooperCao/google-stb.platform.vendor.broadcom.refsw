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
#include "bsynclib.h"
#include "bsynclib_priv.h"

BDBG_MODULE(synclib);

bool BSYNClib_P_Enabled_isrsafe(BSYNClib_Handle hSync)
{
	BDBG_ENTER(BSYNClib_P_Enabled_isrsafe);
	BDBG_ASSERT(hSync);
	BDBG_LEAVE(BSYNClib_P_Enabled_isrsafe);
	return hSync->sSettings.bEnabled;
}

static unsigned int BSYNClib_P_Gcd_isrsafe(unsigned int a, unsigned int b)
{
	unsigned int t;

	while (b != 0)
	{
		t = b;
		b = a % b;
		a = t;
	}

	return a;
}

BERR_Code BSYNClib_ConvertSigned_isrsafe(int iFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, int * piToValue)
{
	int iTemp;

	BDBG_ENTER(BSYNClib_ConvertSigned_isrsafe);

	if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
	{
		iTemp = BSYNClib_P_ConvertSigned_isrsafe(iFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
		iTemp = BSYNClib_P_ConvertSigned_isrsafe(iTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
	}
	else
	{
		iTemp = BSYNClib_P_ConvertSigned_isrsafe(iFromValue, eFromUnits, eToUnits);
	}
	*piToValue = iTemp;

	BDBG_LEAVE(BSYNClib_ConvertSigned_isrsafe);
	return BERR_SUCCESS;
}

int BSYNClib_P_ConvertSigned_isrsafe(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	int iResult;
	unsigned int uiResult;
	unsigned int uiValue;
	bool neg;

	BDBG_ENTER(BSYNClib_P_ConvertSigned_isrsafe);

	if (iValue < 0)
	{
		neg = true;
		uiValue = (unsigned)-iValue;
	}
	else
	{
		neg = false;
		uiValue = (unsigned)iValue;
	}

	uiResult = BSYNClib_P_Convert_isrsafe(uiValue, eFromUnits, eToUnits);


	iResult = (signed)uiResult;

	if (neg)
	{
		iResult = -iResult;
	}

	BDBG_LEAVE(BSYNClib_P_ConvertSigned_isrsafe);
	return iResult;
}

BERR_Code BSYNClib_Convert_isrsafe(unsigned int uiFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, unsigned int * puiToValue)
{
	unsigned int uiTemp;

	BDBG_ENTER(BSYNClib_Convert_isrsafe);

	if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
	{
		uiTemp = BSYNClib_P_Convert_isrsafe(uiFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
		uiTemp = BSYNClib_P_Convert_isrsafe(uiTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
	}
	else
	{
		uiTemp = BSYNClib_P_Convert_isrsafe(uiFromValue, eFromUnits, eToUnits);
	}
	*puiToValue = uiTemp;

	BDBG_LEAVE(BSYNClib_Convert_isrsafe);
	return BERR_SUCCESS;
}

unsigned int BSYNClib_P_Convert_isrsafe(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	unsigned int uiResult = 0;
	unsigned int uiMultiplier = 1;
	unsigned int uiDivisor = 1;
	unsigned int uiMDGcd;
	unsigned int uiVDGcd;

	BDBG_ENTER(BSYNClib_P_Convert_isrsafe);

	uiResult = uiValue;

	if (eFromUnits == eToUnits)
	{
		goto end;
	}

	if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
	{
		BDBG_WRN(("Unsupported conversion (%s to %s).  Conversion only supported from 27 MHz ticks to x or from x to 27 MHz ticks", BSYNClib_P_UnitsStrings[eFromUnits], BSYNClib_P_UnitsStrings[eToUnits]));
		goto end;
	}

	if (eFromUnits == BSYNClib_Units_eMilliseconds)
	{
		uiDivisor = 1000;
	}
	else if (eFromUnits == BSYNClib_Units_e24HzVsyncs)
	{
		uiDivisor = 24;
	}
	else if (eFromUnits == BSYNClib_Units_e50HzVsyncs)
	{
		uiDivisor = 50;
	}
	else if (eFromUnits == BSYNClib_Units_e60HzVsyncs)
	{
		uiDivisor = 60;
	}
	else if (eFromUnits == BSYNClib_Units_e45KhzTicks)
	{
		uiDivisor = 45000;
	}
	else if (eFromUnits == BSYNClib_Units_e90KhzTicks)
	{
		uiDivisor = 90000;
	}
	else if (eFromUnits == BSYNClib_Units_e27MhzTicks)
	{
		uiDivisor = 27000000;
	}

	if (eToUnits == BSYNClib_Units_eMilliseconds)
	{
		uiMultiplier = 1000;
	}
	else if (eToUnits == BSYNClib_Units_e24HzVsyncs)
	{
		uiMultiplier = 24;
	}
	else if (eToUnits == BSYNClib_Units_e50HzVsyncs)
	{
		uiMultiplier = 50;
	}
	else if (eToUnits == BSYNClib_Units_e60HzVsyncs)
	{
		uiMultiplier = 60;
	}
	else if (eToUnits == BSYNClib_Units_e45KhzTicks)
	{
		uiMultiplier = 45000;
	}
	else if (eToUnits == BSYNClib_Units_e90KhzTicks)
	{
		uiMultiplier = 90000;
	}
	else if (eToUnits == BSYNClib_Units_e27MhzTicks)
	{
		uiMultiplier = 27000000;
	}

	uiMDGcd = BSYNClib_P_Gcd_isrsafe(uiMultiplier, uiDivisor);
	if (uiValue > 0)
	{
		uiVDGcd = BSYNClib_P_Gcd_isrsafe(uiValue, uiDivisor);
	}
	else
	{
		uiVDGcd = 1;
	}

	if (uiMDGcd > 1 && uiVDGcd > 1)
	{
		if (uiMDGcd > uiVDGcd)
		{
			uiMultiplier /= uiMDGcd;
			uiDivisor /= uiMDGcd;
		}
		else
		{
			uiValue /= uiVDGcd;
			uiDivisor /= uiVDGcd;
		}

	}
	else if (uiMDGcd > 1)
	{
		uiMultiplier /= uiMDGcd;
		uiDivisor /= uiMDGcd;
	}
	else if (uiVDGcd > 1)
	{
		uiValue /= uiVDGcd;
		uiDivisor /= uiVDGcd;
	}

	uiResult = uiValue * uiMultiplier / uiDivisor;

	if (uiValue * uiMultiplier % uiDivisor > uiDivisor / 2)
	{
		uiResult++;
	}

	end:

	BDBG_LEAVE(BSYNClib_P_Convert_isrsafe);
	return uiResult;
}

#if BDBG_DEBUG_BUILD
const char * const BSYNClib_P_UnitsStrings[] =
	{
		"ms",
		"24 Hz vsyncs",
		"50 Hz vsyncs",
		"60 Hz vsyncs",
		"45 KHz ticks",
		"90 KHz ticks",
		"27 MHz ticks",
		"<enum terminator>",
		NULL
	};
#endif

