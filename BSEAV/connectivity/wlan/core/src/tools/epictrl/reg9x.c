#include <windows.h>
#include <basetyps.h>
#include <cfgmgr32.h>
#include <stdio.h>
#include <string.h>
#include <basetsd.h>
#include <cfgmgr32.h>

#include "epictrl.h"

#define W9XNET "System\\CurrentControlSet\\Services\\Class\\Net"
#define W9XPCIValueToLook "Driver"
#define W9XPCIMatch "Net"
#define BUFFER_SIZE 256

#define W9XPCI "Enum\\PCI"
#define W9XPCMCIA "Enum\\PCMCIA"

typedef DWORD WINERR;

static BOOL Is9xAdapterEnabled(LPTSTR pAdapterName);
static BOOL Get9xAdapterInfo(LPTSTR pAdapterName, LPTSTR pAdapterType, LPTSTR pDevInst);
static BOOL IsCfgAdapterEnabled(LPTSTR pVal);
static WINERR RegEnumKeys(HKEY hKey, int index, HKEY *pKey);

/*
  Check for enabled network adapter
*/
VOID Process9xAdapter(
	IN OUT PADAPTER pdev
)
{
	strcpy(pdev->adaptername, pdev->regkey);

	// Determine if this adapter is enabled
	pdev->valid = Is9xAdapterEnabled(pdev->adaptername);
}

static BOOL Is9xAdapterEnabled(LPTSTR pAdapterName)
{
	BOOL bEnabled = TRUE;
	char cDevInst[2*BUFFER_SIZE];
	cDevInst[0] = '\0';

	if ((Get9xAdapterInfo(pAdapterName, W9XPCI, cDevInst)) || \
		(Get9xAdapterInfo(pAdapterName, W9XPCMCIA, cDevInst)))
	{
		if (cDevInst[0] != '\0')
			bEnabled = IsCfgAdapterEnabled(cDevInst);
	}
	return bEnabled;
}

static BOOL Get9xAdapterInfo(LPTSTR pAdapterName, LPTSTR pAdapterType, LPTSTR pDevInst)
{
	// To keep it safe and simple, not reusing many variables
	WINERR		Status1, Status2, Status3, Status4, Status5;
	FILETIME	ftLastWriteTime;
	HKEY		hKey1, hKey2, hKey3;
	ULONG		ResultLength1, ResultLength2;
	TCHAR		cPCIKeys[BUFFER_SIZE], cDevKey[BUFFER_SIZE], cDevSubKey[BUFFER_SIZE];
	TCHAR		cValue[BUFFER_SIZE], cValueToLook[2*BUFFER_SIZE], cMatch[BUFFER_SIZE];
	int			index1 = 0, index2 = 0;
	DWORD		sz = sizeof(cValue);
	BOOL bFound = FALSE;
	
	// This is what we are looking for
	sprintf(cMatch, "%s\\%s", W9XPCIMatch, pAdapterName);

	// Lot of grovelling here to determine if the adapter is enabled.
	// Outer do while loop goes through all the keys under PCI/PCMCIA type.
	// The inner loop goes through each key within that to determine
	// if we found the adapter. The match is done by looking at the
	// Driver key and comparing it with the adapter name passed as a parameter.
	Status1 = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pAdapterType, 0, KEY_READ, &hKey1 );
	if ( Status1 == ERROR_SUCCESS )
	{
		do
		{
			index2 = 0;
			ResultLength1 = sizeof(cPCIKeys);
			Status2 = RegEnumKeyEx( hKey1, index1++, cPCIKeys, &ResultLength1,
								   NULL, NULL, NULL, &ftLastWriteTime);
			if ( Status2 == ERROR_SUCCESS )
			{
				// Loop through the keys within
				sprintf(cDevKey, "%s\\%s", pAdapterType, cPCIKeys);
				Status2 = RegOpenKeyEx( HKEY_LOCAL_MACHINE, cDevKey, 0, KEY_READ, &hKey2 );
				if ( Status2 == ERROR_SUCCESS )
				{
					// Now loop through each of these. Within each look for the one we
					// want to deal with.
					do
					{
						ResultLength2 = sizeof(cDevSubKey);
						Status3 = RegEnumKeyEx( hKey2, index2++, cDevSubKey, &ResultLength2,
											   NULL, NULL, NULL, &ftLastWriteTime);
						if ( Status3 == ERROR_SUCCESS )
						{
							sprintf(cValueToLook, "%s\\%s", cDevKey, cDevSubKey);
							Status4 = RegOpenKeyEx( HKEY_LOCAL_MACHINE, cValueToLook, 0, KEY_READ, &hKey3 );
							if (Status4 == ERROR_SUCCESS)
							{
								Status5 = RegQueryValueEx( hKey3, W9XPCIValueToLook, 0, NULL, 
															cValue, &sz);
								RegCloseKey ( hKey3);
								if (Status5 == ERROR_SUCCESS)
								{
									// Now we are getting somewhere. Compare this value to
									// the one we are interested in.
									if (strcmpi(cValue, cMatch) == 0)
									{
										// Yes, yes, yes, we got it!
										bFound = TRUE;

										// Use the part of the key used by the CfgMgr32 API to 
										// determine if the adapter is enabled. We use + 5 since
										// we want to start from the "PC..." part.
										lstrcpy(pDevInst, cValueToLook + 5);
										break;
									}
								}
							}
						}

					} while (!bFound && (Status3 != ERROR_NO_MORE_ITEMS));

					RegCloseKey ( hKey2);
				}
			}
		} while ( !bFound && (Status2 != ERROR_NO_MORE_ITEMS));

		RegCloseKey( hKey1 );
	}
	
	return bFound;
}

static BOOL IsCfgAdapterEnabled(LPTSTR pVal)
{
	BOOL bEnabled = TRUE;
    DEVINST devInst;
    CONFIGRET status;
	ULONG uStatus, uProblem;
    
	// Use the CfgMgr32 API to determine if the adapter is enabled.
    status = CM_Locate_DevNode(&devInst, pVal, CM_LOCATE_DEVNODE_NORMAL);
	if (status == CR_SUCCESS)
	{
		status = CM_Get_DevNode_Status(&uStatus, &uProblem, devInst, 0);
		if (status == CR_SUCCESS)
			bEnabled = !(uProblem & CM_PROB_DISABLED);
	}

	return bEnabled;
}


/* Helper routine for enumerating registry keys.
   Not used right now, but keep it around in case it can simplify the deeply nested code above. */
static WINERR RegEnumKeys(HKEY hKey, int index, HKEY *pKey)
{
    WINERR	Status;
    FILETIME	ftLastWriteTime;
    ULONG	ResultLength;
    TCHAR	Keys[BUFFER_SIZE];

    ResultLength = sizeof(Keys);
    Status = RegEnumKeyEx( hKey, index, Keys, &ResultLength,
			   NULL, NULL, NULL, &ftLastWriteTime);
    if ( Status == ERROR_SUCCESS ) {
	Status = RegOpenKeyEx( hKey, Keys, 0, KEY_READ, pKey );
    }

    return Status;
}
