

#ifndef _epictrl_private_h_
#define _epictrl_private_h_ 1

#define VER_WINXP 1
#define VER_VISTA 2

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)   (sizeof(a)/sizeof(a[0]))
#endif

extern DWORD get_mac_address(HANDLE m_dh, UCHAR macaddr[]);
extern DWORD get_description(HANDLE m_dh, LPTSTR desc, DWORD desclen);
extern DWORD old_get_adapter_type(HANDLE lirh, PADAPTER pdev);
extern WINERR new_get_adapter_type(PADAPTER pdev);
extern WINERR SearchForAdapters( ULONG index, PADAPTER pdev );

extern BOOL findloaddriver(LPCTSTR DriverFilename, LPCTSTR DriverService);
extern DWORD g_osver;

extern WINERR ir_bind1(HANDLE m_dh, LPCTSTR DeviceName);
extern WINERR ir_unbind1(HANDLE m_dh);
extern WINERR ir_ioctl(HANDLE m_dh, DWORD ioctl,
					   PCHAR buf, PDWORD plen, DWORD millis);
extern WINERR SearchForNT4Adapters( ULONG, PADAPTER );
extern WINERR SearchForNT5Adapters( ULONG, PADAPTER );

extern WINERR GetAdapterList(HANDLE, ADAPTER [], PDWORD);
extern WINERR GetAdapterListFromIPHlpApi(HANDLE, ADAPTER [], PDWORD);
extern WINERR GetAdapterListFromSetupDi(HANDLE, ADAPTER [], PDWORD);
extern WINERR GetAllAdapterListFromSetupDi(HANDLE, ADAPTER [], PDWORD);

extern WINERR EnableAdapter(DWORD, BOOL);

extern void GetOSVer();

extern BOOL g_useWMI;

#endif /* _epictrl_private_h_ */
