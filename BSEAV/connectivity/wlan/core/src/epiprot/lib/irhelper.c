#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <assert.h>
#include <TCHAR.h>

/* these two are needed for the IPHlpApi calls used on win2k.
   They may not be available under NT4. */
#include <iphlpapi.h>
#include <iprtrmib.h>

#include "epivers.h"
#include "etioctl.h"
#include "wlioctl.h"
#include "epictrl.h" //#include "bcmpdlib.h"
#include "vendor.h"
#include "irelay.h"
#include "epictrl_private.h"

#include "ntddndis.h"

#define OID_802_3_PERMANENT_ADDRESS         	0x01010101
#define OID_GEN_VENDOR_DESCRIPTION          	0x0001010D

#define GPRS_OLD_STATIC_SPEED						533
#define GPRS_NEW_STATIC_SPEED						536

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
    return ir_queryinformation(m_dh, OID_802_3_PERMANENT_ADDRESS,
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

/* Certain virtual adapters like cisco etc. from 4.01 version onwards
   show up as a valid adapter. We need to remove those from the list.
   Until a better method can be found to do so, for now strip it out
   using the description */
BOOL IsValidIPHlpAdapter(LPCTSTR pDesc)
{
	TCHAR cBuf[512];
	LPTSTR p;
	lstrcpy(cBuf, pDesc);
	_tcsupr(cBuf);
	p = _tcsstr(cBuf, _T("VPN"));
	if (p)
		return FALSE;
	else
		return TRUE;
}

WINERR SearchForAdapters(
	IN ULONG index,
	IN PADAPTER pdev
)
{
    WINERR status = ERROR_BADKEY;

    switch (g_osver) {
    case VER_WINXP:
    case VER_VISTA:
	status = SearchForNT5Adapters( index, pdev );
	break;
    }

    return status;
}

DWORD get_adapter_type(HANDLE lirh, PADAPTER pdev)
{
    DWORD len, status = ERROR_SUCCESS, instance, speed = 0;

	len = sizeof(instance);
	status = ir_queryinformation(lirh, OID_GEN_PHYSICAL_MEDIUM, (PUCHAR) &instance, &len);
	if (status != ERROR_SUCCESS)
	{
		len = sizeof(instance);
		status = ir_queryinformation(lirh, OID_GEN_MEDIA_SUPPORTED, (PUCHAR) &instance, &len);
	}

	if (status == ERROR_SUCCESS)
	{
		switch (instance)
		{
			case NdisPhysicalMediumWirelessLan:
			case NdisPhysicalMediumNative802_11:
				pdev->type = IR_WIRELESS;
				break;

			case NdisMedium802_3:
				pdev->type = IR_802_3;
				break;

			case NdisPhysicalMediumWirelessWan:
				pdev->type = IR_WWAN;
				break;

			default:
				// Set all unknown types to GPRS so we ignore them
				pdev->type = IR_WWAN;
				break;
		}
	}

	// For now, the way to differentiate a GPRS adapter from an
	// ethernet one is using the link speed
	if (pdev->type == IR_802_3)
	{
		status = ir_queryinformation(lirh, OID_GEN_LINK_SPEED, (PUCHAR)&speed, &len);
		if (status == ERROR_SUCCESS)
		{
			if ((speed == GPRS_OLD_STATIC_SPEED) || (speed == GPRS_NEW_STATIC_SPEED))
				pdev->type = IR_WWAN;
		}
	}

	return status;
}

WINERR GetAdapterListFromIPHlpApi(
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
		pdev->type = IR_UNKNOWN;
		pdev->instance = pInfo->Index;
		//memcpy(pdev->macaddr, pInfo->Address,sizeof(pdev->macaddr) );
		//pdev->dhcpenabled = pInfo->DhcpEnabled;

#if _UNICODE
		mbstowcs(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername));
		mbstowcs(pdev->description, pInfo->Description, sizeof(pdev->description));
		//mbstowcs(pdev->dhcpserver, pInfo->DhcpServer.IpAddress.String, sizeof(pdev->dhcpserver));
		//mbstowcs(pdev->ipaddr, pInfo->IpAddressList.IpAddress.String, sizeof(pdev->ipaddr));
		//mbstowcs(pdev->ipgateway, pInfo->GatewayList.IpAddress.String, sizeof(pdev->ipgateway));
#else
		memcpy(pdev->adaptername, pInfo->AdapterName, sizeof(pdev->adaptername) );
		strncpy(pdev->description, pInfo->Description, sizeof(pdev->description));
		//strncpy(pdev->dhcpserver, pInfo->DhcpServer.IpAddress.String, sizeof(pdev->dhcpserver));
		//strncpy(pdev->ipaddr, pInfo->IpAddressList.IpAddress.String, sizeof(pdev->ipaddr));
		//strncpy(pdev->ipgateway, pInfo->GatewayList.IpAddress.String, sizeof(pdev->ipgateway));
#endif

		if (ir_bind(lirh, pdev->adaptername) == ERROR_SUCCESS) {
			if (IsValidIPHlpAdapter(pdev->description)) {
				get_adapter_type(lirh, pdev);
				ndevs++;
			}
			ir_unbind(lirh);
		}
	    }
	    free(pAdapterInfo);
	}
	ir_exit(lirh);
    }

    *maxdevs = ndevs;

    return status;

}

WINERR GetAdapterListFromRegistry(
    HANDLE		ignore,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    WINERR  status=-1;
    DWORD   ndevs = 0, index;
    PADAPTER pdev;
    HANDLE  lirh;  // local handle for duration of this routine.

	index = 0;
	pdev = &devlist[ndevs];
	memset(pdev, 0, sizeof(ADAPTER));
	pdev->valid = TRUE;
	while (SearchForAdapters( index, pdev ) == ERROR_SUCCESS)
	{
		if (ndevs >= *maxdevs)
		break;

		if (pdev->valid)
		{
		    status = ir_init(&lirh);
			if (status == NO_ERROR)
			{
				if (ir_bind(lirh, pdev->adaptername) == ERROR_SUCCESS)
				{
					get_mac_address(lirh, pdev->macaddr);
					get_description(lirh, pdev->description,sizeof(pdev->description));
					get_adapter_type(lirh, pdev);

					ir_unbind(lirh);
				}

				ndevs++;
				ir_exit(lirh);
			}
		}

		index++;

		pdev = &devlist[ndevs];
		memset(pdev, 0, sizeof(ADAPTER));
		pdev->valid = TRUE;
	}


    *maxdevs = ndevs;

    return status;
}

WINERR call_irelay(HANDLE m_dh, PRelayHeader prh)
{
    OVERLAPPED  ovlp = { 0, 0, 0, 0, 0 };
    HANDLE      hEvent = 0;
	DWORD		iosize;
	DWORD		status;
    DWORD       count;

    hEvent = CreateEvent( 0, TRUE, 0, NULL);
    if ( hEvent == NULL ){
        return 0;
    }
    ovlp.hEvent = hEvent;
    iosize = sizeof( RelayHeader ) + prh->BufferLength;

	count = 0;
	SetLastError(NO_ERROR);
	if (!DeviceIoControl( m_dh, IOCTL_OID_RELAY,(PVOID)prh, iosize, (PVOID)prh, iosize,&count, &ovlp))
	{
		status = GetLastError( );

		if ( status == ERROR_IO_PENDING )
		{
			if (!GetOverlappedResult( m_dh, &ovlp, &count, TRUE ))
			{
				status = GetLastError( );
			}
			else
			{
				status = ERROR_SUCCESS;
			}
		}
		prh->BufferLength = count - sizeof( RelayHeader );
	}
    CloseHandle(hEvent);

	return status;
}
