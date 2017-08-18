// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

// NOTE - this is value is not strongly correlated to the Windows CE OS version being targeted
#define WINVER _WIN32_WCE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif



#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT



#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif

#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 

#if (_WIN32_WCE < 0x500) && ( defined(WIN32_PLATFORM_PSPC) || \
	defined(WIN32_PLATFORM_WFSP) )
	#pragma comment(lib, "ccrtrtti.lib")
	#ifdef _X86_	
		#if defined(_DEBUG)
			#pragma comment(lib, "libcmtx86d.lib")
		#else
			#pragma comment(lib, "libcmtx86.lib")
		#endif
	#endif
#endif

#include <altcecrt.h>

#include "../../include/wps_sdk.h"

/* WPS SDK functions */
 extern wps_open_fptr wps_open;
 extern wps_close_fptr wps_close;
 extern wps_configure_wzcsvc_fptr wps_configure_wzcsvc;
 extern wps_findAP_fptr wps_findAP;
 extern wps_getAP_fptr	wps_getAP;
 extern wps_join_fptr	wps_join;
 extern wps_get_AP_info_fptr wps_get_AP_info;
 extern wps_get_AP_infoEx_fptr wps_get_AP_infoEx;
 extern wps_create_profile_fptr wps_create_profile;
 extern wps_configureAP_fptr wps_configureAP;
 extern wps_generate_pin_fptr wps_generate_pin;
 extern wps_generate_cred_fptr wps_generate_cred;
 extern wps_is_reg_activated_fptr wps_is_reg_activated;
 extern wps_validate_checksum_fptr wps_validate_checksum;
 extern wps_get_AP_scstate_fptr wps_get_AP_scstate;

 #define atow(strA,strW,lenW) \
MultiByteToWideChar(CP_ACP,0,strA,-1,strW,lenW)

#define wtoa(strW,strA,lenA) \
WideCharToMultiByte(CP_ACP,0,strW,-1,strA,lenA,NULL,NULL)
