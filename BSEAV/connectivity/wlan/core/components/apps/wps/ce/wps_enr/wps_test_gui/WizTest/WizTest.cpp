// WizTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WizTest.h"
#include "WizTestDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/* WPS SDK functions */
wps_open_fptr wps_open;
wps_close_fptr wps_close;
wps_configure_wzcsvc_fptr wps_configure_wzcsvc;
wps_findAP_fptr wps_findAP;
wps_getAP_fptr	wps_getAP;
wps_join_fptr	wps_join;
wps_get_AP_info_fptr wps_get_AP_info;
wps_get_AP_infoEx_fptr wps_get_AP_infoEx;
wps_create_profile_fptr wps_create_profile;
wps_configureAP_fptr wps_configureAP;
wps_generate_pin_fptr wps_generate_pin;
wps_generate_cred_fptr wps_generate_cred;
wps_is_reg_activated_fptr wps_is_reg_activated;
wps_validate_checksum_fptr wps_validate_checksum;
wps_get_AP_scstate_fptr wps_get_AP_scstate;


HINSTANCE hWpsDll;

 bool loadWpsDll(void)
{
	WCHAR szLog[512];
	hWpsDll = LoadLibraryW(L"wps_api.dll");
	
	if (NULL == hWpsDll) {
		wsprintf(szLog,_T("Cannot find or load the wps_api.dll %ld"), GetLastError());
		AfxMessageBox(szLog, MB_ICONSTOP);
		return FALSE;
	}
	
	wps_open = (wps_open_fptr) GetProcAddress(hWpsDll, _T("wps_open"));
	wps_close = (wps_close_fptr) GetProcAddress(hWpsDll, _T("wps_close"));
	wps_configure_wzcsvc = (wps_configure_wzcsvc_fptr) GetProcAddress(hWpsDll, _T("wps_configure_wzcsvc"));
	wps_findAP = (wps_findAP_fptr) GetProcAddress(hWpsDll, _T("wps_findAP"));
	wps_getAP = (wps_getAP_fptr) GetProcAddress(hWpsDll, _T("wps_getAP"));
	wps_join = (wps_join_fptr) GetProcAddress(hWpsDll, _T("wps_join"));
	wps_get_AP_info = (wps_get_AP_info_fptr) GetProcAddress(hWpsDll, _T("wps_get_AP_info"));
	wps_get_AP_infoEx  = (wps_get_AP_infoEx_fptr) GetProcAddress(hWpsDll, _T("wps_get_AP_infoEx"));
	wps_create_profile  = (wps_create_profile_fptr) GetProcAddress(hWpsDll, _T("wps_create_profile"));
	wps_configureAP  = (wps_configureAP_fptr) GetProcAddress(hWpsDll, _T("wps_configureAP"));
	wps_generate_pin  = (wps_generate_pin_fptr) GetProcAddress(hWpsDll, _T("wps_generate_pin"));
	wps_generate_cred  = (wps_generate_cred_fptr) GetProcAddress(hWpsDll, _T("wps_generate_cred"));
	wps_is_reg_activated  = (wps_is_reg_activated_fptr) GetProcAddress(hWpsDll, _T("wps_is_reg_activated"));
	wps_validate_checksum  = (wps_validate_checksum_fptr) GetProcAddress(hWpsDll, _T("wps_validate_checksum"));
	wps_get_AP_scstate  = (wps_get_AP_scstate_fptr) GetProcAddress(hWpsDll, _T("wps_get_AP_scstate"));

	if (wps_open == NULL || wps_close == NULL || wps_findAP == NULL || wps_configure_wzcsvc == NULL ||
		wps_join == NULL ||  wps_getAP == NULL || wps_get_AP_info == NULL || 
		wps_get_AP_infoEx == NULL || wps_create_profile == NULL  ||
		wps_configureAP == NULL || 	wps_generate_pin == NULL ||
		wps_generate_cred == NULL || wps_is_reg_activated == NULL ||
		wps_validate_checksum == NULL || wps_get_AP_scstate == NULL)
	{
			AfxMessageBox(_T("Failed to find a function."), MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

//-------------------

// CWizTestApp

BEGIN_MESSAGE_MAP(CWizTestApp, CWinApp)
END_MESSAGE_MAP()


// CWizTestApp construction
CWizTestApp::CWizTestApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWizTestApp object
CWizTestApp theApp;

// CWizTestApp initialization

BOOL CWizTestApp::InitInstance()
{
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the Windows Mobile specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	if (loadWpsDll())
	{
		CWizTestDlg dlg;
		if (wps_open(NULL, dlg.OnCallback))
		{
			m_pMainWnd = &dlg;

			dlg.DoModal();
			wps_close();
		}
	}	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
