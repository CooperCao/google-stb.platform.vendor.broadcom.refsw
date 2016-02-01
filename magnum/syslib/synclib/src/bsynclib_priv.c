/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bsynclib.h"
#include "bsynclib_priv.h"

BDBG_MODULE(synclib);

bool BSYNClib_P_Enabled(BSYNClib_Handle hSync)
{
	bool bEnabled = false;
	
	BDBG_ENTER(BSYNClib_P_Enabled);

	BDBG_ASSERT(hSync);

	bEnabled = hSync->sSettings.bEnabled;
	
	BDBG_LEAVE(BSYNClib_P_Enabled);
	return bEnabled;
}

static unsigned int BSYNClib_P_Gcd_isr(unsigned int a, unsigned int b)
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

BERR_Code BSYNClib_ConvertSigned(int iFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, int * piToValue)
{
    int iTemp;
    if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
    {
        iTemp = BSYNClib_P_ConvertSigned(iFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
        iTemp = BSYNClib_P_ConvertSigned(iTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
    }
    else
    {
        iTemp = BSYNClib_P_ConvertSigned(iFromValue, eFromUnits, eToUnits);
    }
    *piToValue = iTemp;
    return BERR_SUCCESS;
}

int BSYNClib_P_ConvertSigned(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	int iResult;
	
	BDBG_ENTER(BSYNClib_P_ConvertSigned);

	BKNI_EnterCriticalSection();
	iResult = BSYNClib_P_ConvertSigned_isr(iValue, eFromUnits, eToUnits);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYNClib_P_ConvertSigned);

	return iResult;
}

BERR_Code BSYNClib_ConvertSigned_isr(int iFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, int * piToValue)
{
    int iTemp;
    if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
    {
        iTemp = BSYNClib_P_ConvertSigned_isr(iFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
        iTemp = BSYNClib_P_ConvertSigned_isr(iTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
    }
    else
    {
        iTemp = BSYNClib_P_ConvertSigned_isr(iFromValue, eFromUnits, eToUnits);
    }
    *piToValue = iTemp;
    return BERR_SUCCESS;
}

int BSYNClib_P_ConvertSigned_isr(int iValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	int iResult;
	unsigned int uiResult;
	unsigned int uiValue;
	bool neg;

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
	
	BDBG_ENTER(BSYNClib_P_ConvertSigned_isr);

	uiResult = BSYNClib_P_Convert_isr(uiValue, eFromUnits, eToUnits);

	BDBG_LEAVE(BSYNClib_P_ConvertSigned_isr);

	iResult = (signed)uiResult;

	if (neg)
	{
		iResult = -iResult;
	}

	return iResult;
}

BERR_Code BSYNClib_Convert(unsigned int uiFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, unsigned int * puiToValue)
{
    unsigned int uiTemp;
    if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
    {
        uiTemp = BSYNClib_P_Convert(uiFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
        uiTemp = BSYNClib_P_Convert(uiTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
    }
    else
    {
        uiTemp = BSYNClib_P_Convert(uiFromValue, eFromUnits, eToUnits);
    }
    *puiToValue = uiTemp;
    return BERR_SUCCESS;
}

unsigned int BSYNClib_P_Convert(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	unsigned int uiResult;
	
	BDBG_ENTER(BSYNClib_P_Convert);

	BKNI_EnterCriticalSection();
	uiResult = BSYNClib_P_Convert_isr(uiValue, eFromUnits, eToUnits);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BSYNClib_P_Convert);

	return uiResult;
}

BERR_Code BSYNClib_Convert_isr(unsigned int uiFromValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits, unsigned int * puiToValue)
{
    unsigned int uiTemp;
    if (eFromUnits != BSYNClib_Units_e27MhzTicks && eToUnits != BSYNClib_Units_e27MhzTicks)
    {
        uiTemp = BSYNClib_P_Convert_isr(uiFromValue, eFromUnits, BSYNClib_Units_e27MhzTicks);
        uiTemp = BSYNClib_P_Convert_isr(uiTemp, BSYNClib_Units_e27MhzTicks, eToUnits);
    }
    else
    {
        uiTemp = BSYNClib_P_Convert_isr(uiFromValue, eFromUnits, eToUnits);
    }
    *puiToValue = uiTemp;
    return BERR_SUCCESS;
}

unsigned int BSYNClib_P_Convert_isr(unsigned int uiValue, BSYNClib_Units eFromUnits, BSYNClib_Units eToUnits)
{
	unsigned int uiResult = 0;
	unsigned int uiMultiplier = 1;
	unsigned int uiDivisor = 1;
	unsigned int uiMDGcd;
	unsigned int uiVDGcd;
	
	BDBG_ENTER(BSYNClib_P_Convert_isr);

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

	uiMDGcd = BSYNClib_P_Gcd_isr(uiMultiplier, uiDivisor);
	if (uiValue > 0)
	{
		uiVDGcd = BSYNClib_P_Gcd_isr(uiValue, uiDivisor);
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

	BDBG_LEAVE(BSYNClib_P_Convert_isr);
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

