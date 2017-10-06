#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <tchar.h>

#include "oidencap.h"  // for OIDENCAP_COOKIE
#include "epivers.h"
#include "epictrl.h" //#include "bcmpdlib.h"
#include "irelay.h"
#include "vendor.h"
#include "assert.h"
#include "epictrl_private.h"
#include "wlioctl.h"


WINERR ir_init(HANDLE *irh)
{
    WINERR status = NO_ERROR;
    HANDLE handle = INVALID_HANDLE_VALUE;
    TCHAR devname[200];
	TCHAR devfilename[200];
	TCHAR devbase[256];

    assert(irh);
    if (!irh)
	return ERROR_INVALID_HANDLE;

    GetOSVer();

#if _UNICODE
	mbstowcs(devname, RELAY_NT_FILE, ARRAYSIZE(devname));
	mbstowcs(devfilename, RELAY_NT_NAME, ARRAYSIZE(devfilename));
	mbstowcs(devbase, RELAY_DEV_BASE, ARRAYSIZE(devbase));
#else
	_sntprintf(devname, ARRAYSIZE(devname), RELAY_NT_FILE);
	_sntprintf(devfilename, ARRAYSIZE(devfilename), RELAY_NT_NAME);
	_sntprintf(devbase, ARRAYSIZE(devbase), RELAY_DEV_BASE);
#endif

	handle = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
			    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	/* If we failed to open the hardware access device, try to load the driver and give it another
	   shot. */
	if (handle == INVALID_HANDLE_VALUE) {
		if ( findloaddriver(devfilename, devbase) ) {
			handle = CreateFile(devname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	    }
	}

	/* We *still* couldn't open the device...give it up. */
	if ( handle == INVALID_HANDLE_VALUE )
	{
	    status = GetLastError();
	}

    if ( status == NO_ERROR )	{
		*irh = handle;
//		last_handle= handle;
	}
	else if(handle != INVALID_HANDLE_VALUE)
	{
//		*irh = last_handle;
//		status = NO_ERROR;
	}

    return status;
}



WINERR ir_adapter_list(
    HANDLE		handle,
    ADAPTER		devlist[],
    PDWORD		maxdevs
)
{
    WINERR status;

    switch (g_osver) {
    case VER_VISTA:
	status = GetAdapterListFromRegistry(handle, devlist, maxdevs);
	break;

    case VER_WINXP:
	status = GetAdapterListFromIPHlpApi(handle, devlist, maxdevs);
	break;
    }
    return status;
}


WINERR ir_exit(HANDLE irh)
{
    WINERR status = NO_ERROR;

    assert(irh);
    assert(irh != INVALID_HANDLE_VALUE);
    if (!irh || irh == INVALID_HANDLE_VALUE)
	return ERROR_INVALID_HANDLE;

    if (irh !=  INVALID_HANDLE_VALUE && !CloseHandle(irh)) {
	status = GetLastError();
    }

    return status;
}


/* send an ioctl down to the irelay driver and wait for it to complete.
   If you want the wait to timeout, supply a timeout value for
   'millis' in milliseconds. Set millis to INFINITE if you do not want
   a timeout at all. */
WINERR ir_ioctl(HANDLE m_dh, DWORD ioctl,
		PCHAR buf, PDWORD plen, DWORD millis)
{
    DWORD  status;
    int ok;
    OVERLAPPED  ovlp = { 0, 0, 0, 0, 0 };
    HANDLE      hEvent = 0;
    DWORD		wait;

    hEvent = CreateEvent( 0, TRUE, 0, NULL);
    if ( hEvent == NULL ){
        return GetLastError();
    }
    ovlp.hEvent = hEvent;

    status = NO_ERROR;
    ok = DeviceIoControl( m_dh, ioctl, buf, *plen,
			  buf, *plen, plen, &ovlp );
    if (!ok) {
	status = GetLastError( );
	if ( status == ERROR_IO_PENDING ) {
	    wait = WaitForSingleObject(hEvent, millis);
	    switch (wait) {
	    case WAIT_OBJECT_0:
		if (!GetOverlappedResult( m_dh, &ovlp, plen, TRUE )) {
		    status = GetLastError();
		} else {
		    if (ovlp.Internal != 0) {
			status = ovlp.Internal;
		    } else {
			status = ERROR_SUCCESS;
		    }
		}
		break;
	    case WAIT_FAILED:
		status = GetLastError( );
		break;
	    case WAIT_TIMEOUT:
		*plen = 0;
		status = ERROR_TIMEOUT;
		break;
	    default:
		_ftprintf(stderr, _T("Received unexpected status from WaitForSingleObject = 0x%x"), wait);
		status = ERROR_INVALID_FUNCTION;
	    }
	}
    }

    CloseHandle(hEvent);
    return status;
}

char *ValidBindIfName(const char *ifname)
{
	static char ValidACMName[256];
	int i,j;

#ifdef _UNICODE
		wcscpy(ValidACMName,ifname);
#else
		strcpy(ValidACMName,ifname);
#endif

	if(ValidACMName[0]=='{')
	{
#ifdef _UNICODE
		j=wcslen(ValidACMName);
#else
		j=strlen(ValidACMName);
#endif

		for(i=0;i<j-1;i++)
		{
			ValidACMName[i]=ValidACMName[i+1];
		}
		ValidACMName[j-2]='\0';
	}

	return ValidACMName;
}


//#define DEVICE_PREFIX   _T("\\\\.\\")
#define DEVICE_PREFIX   L"\\DEVICE\\"
#define MAX_NDIS_DEVICE_NAME_LEN        256

WINERR ir_bind(HANDLE m_dh, LPCTSTR DeviceName)
{
    DWORD status;
    BindRequest br;
    DWORD brlen;
    WCHAR   wNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN];
    WCHAR   wFullNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN*2];
	int     i;

    assert(m_dh);
    assert(m_dh != INVALID_HANDLE_VALUE);
    if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
	return ERROR_INVALID_HANDLE;


#ifdef _UNICODE
		wcstombs( br.name, DeviceName, sizeof(br.name));
#else
		strncpy(br.name, DeviceName, sizeof(br.name));
#endif
		br.name[sizeof(br.name)-1] = '\0';
		brlen = sizeof(br);

		status = ir_ioctl(m_dh, IOCTL_BIND, (PCHAR) &br, &brlen, INFINITE);

    return status;
}



WINERR ir_unbind(HANDLE m_dh)
{
    DWORD len, status=-1;

    assert(m_dh);
    assert(m_dh != INVALID_HANDLE_VALUE);
    if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
	return ERROR_INVALID_HANDLE;

	len = 0;
	status = ir_ioctl(m_dh, IOCTL_UNBIND, NULL, &len, INFINITE);

    return status;
}



WINERR ir_queryinformation(HANDLE m_dh, ULONG oid,
				       PUCHAR inbuf, PDWORD inlen)
{
	DWORD status;
	PIRELAY pr;
	getinformation_t *getinfo, *repinfo;
	ULONG nbytes;
	int offset = 0;

    assert(m_dh);
    assert(m_dh != INVALID_HANDLE_VALUE);
    if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
	return ERROR_INVALID_HANDLE;

		nbytes = GETINFORMATION_SIZE + *inlen;
		pr = (PIRELAY) malloc( sizeof(IRELAY) + nbytes );
		memset(pr, 0, sizeof(IRELAY) + nbytes );
		pr->rh.OID = OID_BCM_GETINFORMATION;

		getinfo = (getinformation_t *)pr->GenOid.Buffer;
		getinfo->cookie = OIDENCAP_COOKIE;
		getinfo->oid = oid;
		getinfo->len = nbytes;
		memcpy(GETINFORMATION_DATA(getinfo), inbuf, *inlen);

	query_again:
		pr->rh.IsQuery = TRUE;
		pr->rh.Status = ERROR_SUCCESS;
		pr->rh.BufferLength = nbytes;

		status = call_irelay( m_dh, (PRelayHeader) pr );

		if (pr->rh.OID == OID_BCM_GETINFORMATION) {
			if (status != ERROR_SUCCESS) {
				/* try using the old method */
				memset(pr, 0, sizeof(IRELAY) + nbytes );
				memcpy( pr->GenOid.Buffer, inbuf, *inlen);
				pr->rh.OID = oid;
				nbytes = *inlen;
				goto query_again;
			}
			else {
				pr->rh.BufferLength -= GETINFORMATION_SIZE;
 				if (pr->rh.BufferLength + offset > *inlen)
					pr->rh.BufferLength = (*inlen - offset);
				repinfo = (getinformation_t *)pr->Buffer;

				memcpy(inbuf + offset, GETINFORMATION_DATA(repinfo), pr->rh.BufferLength);

				if (((pr->rh.BufferLength + offset) < *inlen) && repinfo->cookie != OIDENCAP_COOKIE) {
					offset += pr->rh.BufferLength;
					nbytes = GETINFORMATION_SIZE + *inlen;
					goto query_again;
				}
				*inlen = pr->rh.BufferLength + offset;
			}
		}
		else {
			if ( status == ERROR_SUCCESS ) {
				*inlen = pr->rh.BufferLength;
				memcpy( inbuf, pr->Buffer, *inlen);
			} else if ( status == ERROR_MORE_DATA ) {
				*inlen = pr->rh.BufferLength;
			}
		}

		free(pr);

	return status;
}





WINERR ir_setinformation(HANDLE m_dh, ULONG oid,
				     PUCHAR inbuf, PDWORD inlen)
{
	DWORD status;
	PIRELAY pr;
	ULONG nbytes;

    assert(m_dh);
    assert(m_dh != INVALID_HANDLE_VALUE);
    if (!m_dh || m_dh == INVALID_HANDLE_VALUE)
	return ERROR_INVALID_HANDLE;

		pr = (PIRELAY) malloc( sizeof(IRELAY) + *inlen );
		memset(pr, 0, sizeof(IRELAY) + *inlen );
		pr->rh.OID = oid;
		pr->rh.IsQuery = FALSE;
		pr->rh.Status = ERROR_SUCCESS;

		memcpy( pr->GenOid.Buffer, inbuf, *inlen);
		pr->rh.BufferLength = *inlen;

		status = call_irelay( m_dh, (PRelayHeader) pr );

		free(pr);
	return status;
}
