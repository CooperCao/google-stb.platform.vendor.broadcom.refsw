#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <assert.h>

/* these two are needed for the IPHlpApi calls used on win2k.
   They may not be available under NT4. */
#include <iphlpapi.h>
#include <iprtrmib.h>

#include "epivers.h"
#include "epiioctl.h"
#include "etioctl.h"
#include "wlioctl.h"
#include "epictrl.h"
#include "wmiprocs.h"
#include "vendor.h"
#include "irelay.h"
#include "epictrl_private.h"

#include "ntddndis.h"

extern WINERR
Get9x2kAdapterList(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs,
    BOOL		win9x,
    BOOL		ignore_disabled
);

#define OID_802_3_PERMANENT_ADDRESS         	0x01010101
#define OID_GEN_VENDOR_DESCRIPTION          	0x0001010D

DWORD g_osver;

void GetOSVer()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    if (!GetVersionEx(&osvi)) {
        printf("GetVersionEx() failed with error %u.\n", GetLastError());
    }

	if (osvi.dwMajorVersion == 6) {
		g_osver = VER_VISTA;
	} else {
		g_osver = VER_WINXP;
    }
}

WINERR get_mac_address(HANDLE m_dh, UCHAR macaddr[])
{
    DWORD len;

    len = 6;
    return ir_queryinformation(m_dh, OID_802_3_CURRENT_ADDRESS,
			       (PUCHAR) macaddr, &len);

}

WINERR get_description(HANDLE m_dh, LPTSTR desc, DWORD desclen)
{
    DWORD status, buflen = desclen;
    PCHAR buffer = _alloca(desclen);

    buflen = desclen;
    status = ir_queryinformation(m_dh, OID_GEN_VENDOR_DESCRIPTION,
			       (PUCHAR) buffer, &buflen);
    if (status == NO_ERROR) {
#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) buffer, buflen,
			    desc, desclen);
#else
	memcpy(desc, buffer, desclen);
#endif
    }

    return status;
}

struct {
    DWORD oid;
    IRTYPE type;
} ir_types[] = {
    { OID_EPI_GET_INSTANCE, IR_ILINE },
    { OID_ET_GETINSTANCE, IR_ENET }
};

// Get the adapter type, and a little more for wireless lans
DWORD get_adapter_type(HANDLE lirh, PADAPTER pdev)
{
    DWORD len, status = ERROR_SUCCESS, instance;
    int i;
  	wl_instance_info_t instanceInfo;

    for (i = 0; i < ARRAYSIZE(ir_types); i++)
	{
		len = sizeof(instance);
		status = ir_queryinformation(lirh, ir_types[i].oid, (PUCHAR) &instance, &len);
		if (status == ERROR_SUCCESS)
		{
			/* If we specifically recognize this adapter, set its type accordingly. */
			pdev->type = ir_types[i].type;
			return status;
		}
    }

	/* In order to determine that this is a broadcom wireless adapter
	   do more than just using OID_WL_GETINSTANCE since other broadcom
	   adapters like ethernet seem to implement this OID. Use the WLC_GET_MAGIC
	   OID to ascertain as an additional mechanism. */
	len = sizeof(instance);
	status = ir_queryinformation(lirh, WL_OID_BASE + WLC_GET_MAGIC, (PUCHAR) &instance, &len);
	if (status == ERROR_SUCCESS && instance == WLC_IOCTL_MAGIC)
	{
		memset( &instanceInfo, 0, sizeof(wl_instance_info_t) );
		len = sizeof(wl_instance_info_t);
		status = ir_queryinformation(lirh, OID_WL_GETINSTANCE, (PUCHAR) &instanceInfo, &len);
		if (status == ERROR_SUCCESS)
		{
			pdev->type = IR_WIRELESS;
			strncpy( pdev->brand, instanceInfo.brand, IR_BRAND_MAX );
			pdev->brand[IR_BRAND_MAX-1] = 0;
			return status;
		}
	}

	// If custom queries fail, fall-back to a standard NDIS query
	len = sizeof(instance);
	status = ir_queryinformation(lirh, OID_GEN_PHYSICAL_MEDIUM, (PUCHAR) &instance, &len);
	if (status != ERROR_SUCCESS)
		status = ir_queryinformation(lirh, OID_GEN_MEDIA_SUPPORTED, (PUCHAR) &instance, &len);
	if (status == ERROR_SUCCESS)
	{
		switch (instance)
		{
			// For now, only report BRCM wireless lan NICs (legacy)
			//case NdisPhysicalMediumWirelessLan:
			//	pdev->type = IR_WIRELESS;
			//	return status;

			case NdisMedium802_3:
				if (strcmpi(pdev->wminame, "Microsoft Loopback Adapter") == 0)
					pdev->type = IR_LOOPBACK;
				else
				pdev->type = IR_802_3;
				return status;

			default:
				pdev->type = IR_UNKNOWN;
				return status;
		}
	}

	return status;
}

WINERR
GetAdapterListFromIPHlpApi(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    WINERR  status;
    DWORD   ndevs = 0, index;
    PADAPTER pdev;

    MIB_IFROW IfRow;
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pInfo;
    ULONG needlen, ifindex = 0;
    HANDLE  lirh;  // local handle for duration of this routine.

    status = ir_init(&lirh);
    if (status == NO_ERROR) {
	needlen = 0;
	status = GetAdaptersInfo(NULL, &needlen);
	if (status == ERROR_BUFFER_OVERFLOW) {
	    pAdapterInfo = (PIP_ADAPTER_INFO) malloc(needlen);
	    memset(pAdapterInfo, 0, needlen);
	    status = GetAdaptersInfo(pAdapterInfo, &needlen);

	    index = 0;
	    pInfo = pAdapterInfo;
	    for (pInfo = pAdapterInfo; pInfo; pInfo = pInfo->Next) {
		if (ndevs >= *maxdevs)
		    break;

		IfRow.dwIndex = pInfo->Index;
		status = GetIfEntry(&IfRow);
		if (status != NO_ERROR || IfRow.dwAdminStatus != MIB_IF_ADMIN_STATUS_UP)
		    continue;

		pdev = &devlist[ndevs];
		memset(pdev, 0, sizeof(ADAPTER));
		pdev->instance = pInfo->Index;
		memcpy(pdev->macaddr, pInfo->Address,sizeof(pdev->macaddr) );
		memcpy(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername) );
		strncpy(pdev->description, pInfo->Description, sizeof(pdev->description));
		if (ir_bind1(lirh, pdev->adaptername) == ERROR_SUCCESS) {
		    if (get_adapter_type(lirh, pdev) != ERROR_SUCCESS)
				pdev->type = IR_802_3;	// legacy
		    ir_unbind1(lirh);
		    ndevs++;
		}
	    }
	    free(pAdapterInfo);
	}
	ir_exit(lirh);
    }

    *maxdevs = ndevs;

    return status;

}


WINERR
GetNT4AdapterListFromRegistry(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    WINERR  status;
    DWORD   ndevs = 0, index;
    PADAPTER pdev;
    HANDLE  lirh;  // local handle for duration of this routine.

    status = ir_init(&lirh);
    if (status == NO_ERROR) {
	index = 0;
	pdev = &devlist[ndevs];
	memset(pdev, 0, sizeof(ADAPTER));
	pdev->valid = TRUE;
	while (SearchForNT4Adapters( index, pdev ) == ERROR_SUCCESS) {
	    if (pdev->valid && (ir_bind1(lirh, pdev->adaptername) == ERROR_SUCCESS)) {
			get_mac_address(lirh, pdev->macaddr);
			get_description(lirh, pdev->description,
					sizeof(pdev->description));
			if (get_adapter_type(lirh, pdev) != ERROR_SUCCESS)
				pdev->type = IR_802_3;	// legacy

			ir_unbind1(lirh);
			ndevs++;
		}
	    index++;

	    if (ndevs >= *maxdevs)
		break;

	    pdev = &devlist[ndevs];
	    memset(pdev, 0, sizeof(ADAPTER));
		pdev->valid = TRUE;
	}
	ir_exit(lirh);
    }

    *maxdevs = ndevs;

    return status;
}

WINERR
GetAdapterListFromSetupDi(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    switch (g_osver) {
	case VER_WINXP:
		return Get9x2kAdapterList( ignore, devlist, maxdevs, FALSE, TRUE );
		break;
	default:
		return ERROR_NOT_FOUND;
		break;
    }
}

WINERR
GetAllAdapterListFromSetupDi(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    switch (g_osver) {
	case VER_WINXP:
		return Get9x2kAdapterList( ignore, devlist, maxdevs, FALSE, FALSE);
		break;
	default:
		return ERROR_NOT_FOUND;
		break;
    }
}


WINERR
GetAdapterList(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    switch (g_osver) {
    case VER_VISTA:
		return GetNT4AdapterListFromRegistry( ignore, devlist, maxdevs);
		break;
	case VER_WINXP:
		return Get9x2kAdapterList( ignore, devlist, maxdevs, FALSE, TRUE);
		break;
	default:
		return ERROR_NOT_FOUND;
		break;
    }
}
