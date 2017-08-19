/* 
DlgTest.cpp

Description:
   -Localization tool to test for text truncation in dialogs
   -tool enumerates all languages and all dialogs, translates each dialog and then checks each dialog element for truncations
   -this was originallly hacked together very quickly from gprs version of same tool so it is probably still a little rough around the edges
   -by default dlgtest check the list of apps/dlls listed in defaultTestApps variable below
   -command line usage: dlgtest <file1> <file2> ...

   Colin Fraser 3/1/05

*/


#include <afxwin.h>
#include "DlgTest.h"

//////////////////////////////////////////////////
// Constants
#define countof(x) (sizeof(x)/sizeof(*x))

#define IDC_LISTBOX 1800

#define kManyDots "...................................................................................................."

const DWORD kEndOfList = 30000;

#define RUNNING_WIN_VER_UNKNOWN	0
#define RUNNING_WIN_VER_NT4		1
#define RUNNING_WIN_VER_2K		2
#define RUNNING_WIN_VER_95		3
#define RUNNING_WIN_VER_98		4
#define RUNNING_WIN_VER_98SE	5
#define RUNNING_WIN_VER_ME		6
#define RUNNING_WIN_VER_XP		7

LPTSTR defaultTestApps[] =
{
	_T("bcmwlcpl.cpl"),
	_T("wltray.exe"),
	_T("bcmlogon.dll"),
	_T("bcmcfg.cpl"),
};

struct structLangData
{
   WORD langId;
   WCHAR * name;
   LGRPID langGroupId;
};

// maintain some info about lang ids
// list of lanaguages to be tested is determined by enumerating languages in resources
const structLangData langCodes[] =
{
0x0401, L"arabic",					LGRPID_ARABIC,
0x0402, L"bulgarian",				LGRPID_CENTRAL_EUROPE,
0x0404, L"chinese_traditional",	LGRPID_TRADITIONAL_CHINESE,
0x0405, L"czech", 					LGRPID_CENTRAL_EUROPE,
0x0406, L"danish",					LGRPID_WESTERN_EUROPE,
0x0407, L"german",					LGRPID_WESTERN_EUROPE,
0x0408, L"greek", 					LGRPID_GREEK,
0x0409, L"english",					LGRPID_WESTERN_EUROPE,
0x040b, L"finnish",					LGRPID_WESTERN_EUROPE,
0x040c, L"french",					LGRPID_WESTERN_EUROPE,
0x040e, L"hungarian",				LGRPID_CENTRAL_EUROPE,
0x040d, L"hebrew",					LGRPID_HEBREW,
0x0410, L"italian",					LGRPID_WESTERN_EUROPE,
0x0411, L"japanese",					LGRPID_JAPANESE,
0x0412, L"korean",					LGRPID_KOREAN,
0x0413, L"dutch", 					LGRPID_WESTERN_EUROPE,
0x0414, L"norwegian",				LGRPID_WESTERN_EUROPE,
0x0415, L"polish",					LGRPID_CENTRAL_EUROPE,
0x0416, L"brazilian",				LGRPID_WESTERN_EUROPE,
0x0418, L"romanian",					LGRPID_CENTRAL_EUROPE,
0x0419, L"russian",					LGRPID_CYRILLIC,
0x041f, L"turkish",					LGRPID_TURKISH,
0x041a, L"croatian",					LGRPID_CENTRAL_EUROPE,
0x041b, L"slovak",					LGRPID_CENTRAL_EUROPE,
0x041d, L"swedish",					LGRPID_WESTERN_EUROPE,
0x041e, L"thai",						LGRPID_THAI,
0x0424, L"slovenian",				LGRPID_CENTRAL_EUROPE,
0x0425, L"estonian",					LGRPID_BALTIC,
0x0426, L"latvian",					LGRPID_BALTIC,
0x0427, L"lithuanian",				LGRPID_BALTIC,
0x0804, L"chinese_simplified",	LGRPID_SIMPLIFIED_CHINESE,
0x0816, L"portugese",				LGRPID_WESTERN_EUROPE,
0x040a, L"spanish",					LGRPID_WESTERN_EUROPE
};

// for debug use only: to save time, limit the number of languages to test (instead of enumerating and testing all of them)
// search for "langIds" to enable
const WORD langIds[] =
{
   0x0416,            // brazilian     
   0x0804,             // chinese_simplified 
   0x0404,             // chinese_traditional 
   0x0012,             // korean 
   0x0409,             // english 
   0x000c,             // french 
   0x0007,             // german 
   0x0010,             // italian 
   0x0011,             // japanese 
   0x000a              //  spanish
};

//////////////////////////////////////////////////
// Variables

CMyApp theApp;
FILE *gLogFile;
HINSTANCE hInstResource = (HINSTANCE) -1;
HINSTANCE hInstRefResource = (HINSTANCE) -1;
CMapStringToPtr stringResMap;          // map foreign strings to resource id's
BOOL gShowEnglishStrings = FALSE;
HANDLE ghLogFile;
int gNumTruncations = 0;
int gNumUntranslatedStrings = 0;
int gNumWrongFont = 0;
BOOL gbError = FALSE;
BOOL gbPassFail;

//////////////////////////////////////////////////
// Prototypes

BOOL AFXAPI AfxGetPropSheetFont(CString& strFace, WORD& wSize, BOOL bWizard);

int TestDialogsWlan(LPCTSTR resourceFile, DWORD stringBlock);
static BOOL CALLBACK EnumResNameProc( HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LPARAM lParam);
BOOL IsDlgTextClipped(CWnd *pWnd, SIZE *pSizeNeeded = 0, SIZE *pSizeAvailable=0);
void CheckDialog(CWnd *pWnd, DWORD dlgId);
void ReportTruncation(DWORD dlgId, DWORD controlId, DWORD charset, LPCTSTR controlText, SIZE &sizeNeeded, SIZE &sizeAvailable, LPCTSTR refString = NULL);
void ReportWrongFont(DWORD dlgId, DWORD charset, CString typeface);
void BuildStringMap(void);
BOOL LoadResources(LPCTSTR resFile);
CString GetRcString(int id);
CString GetEngString(int id);
BOOL CALLBACK EnumResLangProc(HINSTANCE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LPARAM lParam);
void LocalizeDialog(CWnd *pWnd);
CString TranslateString(CString inStr);
static bool HasDots(CString &str);
WCHAR* LocalizedString( HINSTANCE hInstance, UINT id, WORD language );
void FreeString( void* string );
HANDLE UcOpenFile(LPCTSTR fileName);
void UcWriteFile(HANDLE hFile, LPCTSTR buf);
typedef void (CALLBACK *StringEnumFunc)(WCHAR *string, DWORD id);
int ErrorMsg(LPCTSTR format, ...);
int ProgressMsg(LPCTSTR format, ...);
CString LookupLangName(WORD langId);
static DWORD GetRunningWinVer( void );

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP (CMainWindow, CFrameWnd)
    ON_WM_PAINT ()
    ON_WM_SIZING ()
    ON_WM_SIZE ()
END_MESSAGE_MAP ()

BEGIN_MESSAGE_MAP (CMyListCtrl, CListCtrl)
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
END_MESSAGE_MAP ()

BEGIN_MESSAGE_MAP (CMyDialog, CDialog)
//   ON_WM_SETFONT()
END_MESSAGE_MAP ()

/////////////////////////////////////////////////
// CMyApp

BOOL CMyApp::InitInstance ()
{
   CString dllName;
   int numArgs;
   LPWSTR *args;
   LPWSTR *pArgs;
   CString strArg1;
   LPTSTR *testApps;
   int numApps;

	// Parse command line
   args = CommandLineToArgvW(GetCommandLine(), &numArgs);
   pArgs = args;

   if (numArgs >= 2)
      strArg1 = pArgs[1];

   if ((strArg1 == _T("//?")) || (strArg1 == _T("-?")) || (strArg1 == _T("--help")) || (strArg1 == _T("-help")))
   {
      AfxMessageBox(_T("Usage:\n\ndlgtest [-passfail] <file1> <file2> ..."), 0, 0);
      return 0;
   }
   else if (strArg1 == _T("-passfail"))
   {
      gbPassFail = TRUE;
      pArgs++;
      numArgs--;
   }

   if (numArgs > 1)
   {
      testApps = &pArgs[1];
      numApps = numArgs - 1;
   }
   else
   {
      testApps = defaultTestApps;
      numApps = countof(defaultTestApps);
   }



	AfxEnableControlContainer();
	InitCommonControls();
	AfxInitRichEdit();

	RegisterCustomWindowClasses();


   m_pMainWnd = new CMainWindow;

   if (!gbPassFail)
   {
      m_pMainWnd->ShowWindow (m_nCmdShow);
      m_pMainWnd->UpdateWindow ();
   }

   // print trace mesage to identify MFC prop sheet and wizard font on current system
   {
      CString strFace;
      WORD wSize;

      AfxGetPropSheetFont(strFace, wSize, FALSE);
      TRACE(L"prop sheet\nfont: %s \nsize: %d\n", strFace, wSize);
  
      AfxGetPropSheetFont(strFace, wSize, TRUE);
      TRACE(L"wizard\nfont: %s\nsize: %d\n", strFace, wSize);
   }   

   ghLogFile = UcOpenFile(L"log.csv");
   UcWriteFile(ghLogFile, L"file name, langId, dlgId, controlId, controlText, required X, required Y, avail.X, avail.Y, pass/fail\n");

   for (int j=0; j < numApps; j++)
	{
		dllName = testApps[j];

		((CMainWindow*)m_pMainWnd)->m_listCtrl.m_dllName = dllName;
		((CMainWindow*)m_pMainWnd)->m_listCtrl.InsertItem(kEndOfList, dllName);

		TestDialogsWlan(dllName, 251);			// hard code to use string id 4000 to enum languages. (wlan apps should use autores so 4000 should be valid)
	}

   if (gbPassFail)
      m_pMainWnd->DestroyWindow();
   else
   {
      CString title;
		if (gNumTruncations && gNumWrongFont)
	      title.Format(_T("%s - %d truncations & %d wrong fonts detected, double-click on individual entries for details"), AfxGetAppName(), gNumTruncations,gNumWrongFont);
		else if (gNumTruncations)
	      title.Format(_T("%s - %d truncations detected, double-click on individual entries for details"), AfxGetAppName(), gNumTruncations);
		else if (gNumWrongFont)
	      title.Format(_T("%s - %d wrong font(s) detected"), AfxGetAppName(), gNumWrongFont);
		else 
	      title.Format(_T("%s - no truncations or wrong fonts detected"), AfxGetAppName());

      m_pMainWnd->SetWindowText(title);
   }

   CloseHandle(ghLogFile);

   return TRUE;
}

int CMyApp::ExitInstance()
{
   if (hInstResource != (HMODULE) -1)
   {
      AfxSetResourceHandle(AfxGetInstanceHandle());
      FreeLibrary(hInstResource);
   }

   if (hInstRefResource != (HMODULE) -1)
   {
      FreeLibrary(hInstRefResource);
      hInstRefResource = (HMODULE) -1;
   }

   int iWinAppRet = CWinApp::ExitInstance();

   if (gNumTruncations)
   {
      ProgressMsg(_T("Dlgtest failed, found %d truncations"), gNumTruncations);
      if (gbPassFail)
         ProgressMsg(_T("Rerun dlgtest without 'passfail' option set to see truncations in graphical mode"));
   }

   if (gNumUntranslatedStrings )
      ProgressMsg(_T("Dlgtest failed, found %d untranslated strings"), gNumUntranslatedStrings);

   if (gNumWrongFont)
      ProgressMsg(_T("Dlgtest failed, found %d wrong fonts"), gNumWrongFont);

   if (gNumTruncations || gNumUntranslatedStrings || gNumWrongFont || gbError)
      return 1;
   else
      return iWinAppRet;

}

// Register some custom window classes used in WLAN dialogs
void CMyApp::RegisterCustomWindowClasses()
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	WNDCLASS wndcls;

	if(!(::GetClassInfo(hInstance, _T("X_WND_ANIMATE"), &wndcls)))
	{
		wndcls.style         = CS_SAVEBITS | CS_GLOBALCLASS;
		wndcls.lpfnWndProc   = ::DefWindowProc;
		wndcls.cbClsExtra    = wndcls.cbWndExtra = 0;
		wndcls.hInstance     = hInstance;
		wndcls.hIcon         = NULL;
		wndcls.hCursor       = ::LoadCursor(hInstance, IDC_ARROW );
		wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW); 	
		wndcls.lpszMenuName  = NULL;
		wndcls.lpszClassName = _T("X_WND_ANIMATE");
		
		if (!AfxRegisterClass(&wndcls))
			ProgressMsg(_T("Warning: failed to register X_WND_ANIMATE window class, last err: %d\n"), GetLastError());
	}
}

//////////////////////////////////////////////////
// CMainWindow

CMainWindow::CMainWindow ()
{
   Create (NULL, _T ("Dialog test"));

   CRect rect;
   GetClientRect (&rect);

   m_listCtrl.Create( WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL , rect, this, IDC_LISTBOX );
}

void CMainWindow::OnSizing(UINT fwSide, LPRECT pRect) 
{
   OnResizeWindow();
}

void CMainWindow::OnSize(UINT nType, int cx, int cy)
{
   OnResizeWindow();
}

void CMainWindow::OnResizeWindow(void)
{
   CRect rect;
   GetClientRect (&rect);
   m_listCtrl.MoveWindow(&rect, TRUE);
}

//////////////////////////////////////////////////
// CMyListCtrl

void CMyListCtrl::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
   int item = GetSelectionMark();
   CString str, strTruncated;
   CString strEnglish;
   DWORD dialogId, controlId;
   WORD langId;
	TCHAR *endPtr;
	CString fileName;

   str = GetItemText(item, kDialogId);
   dialogId = _ttoi(str);
   str = GetItemText(item, kControlId);
   controlId = _ttoi(str);
   str = GetItemText(item, kLangId);
   langId = _tcstol(str, &endPtr, 16);
   strTruncated = GetItemText(item, kText);

   TRACE(_T("item: %d, dialog: %d\n"), item, dialogId);
   
	fileName = GetItemText(item, kFileName);
	if (fileName.Left(1) == "-" || !LoadResources(fileName))
      goto exit;

   BuildStringMap();

   if (dialogId && !strTruncated.IsEmpty())
   {
      CMyDialog dlg;

      if (dlg.Create(dialogId, theApp.m_pMainWnd))
      {
         dlg.ShowWindow(SW_SHOW);

         SetThreadLocale(langId);
         dlg.GetDlgItemText(controlId, strEnglish);

         LocalizeDialog(&dlg);

         AfxMessageBox((CString) _T("truncated text:\nforeign str: ") + strTruncated + _T("\nenglish str: ") + strEnglish, 0, 0);

         dlg.DestroyWindow();
      }
    }

exit:
    *pResult = 0;

}

int CMyListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CListCtrl::OnCreate(lpCreateStruct) == -1)
      return -1;

   SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   InsertColumn(kFileName,  _T("file name"),     LVCFMT_LEFT, 100, 0);
   InsertColumn(kLangId,    _T("lang id"),       LVCFMT_LEFT, 50, 0);
   InsertColumn(kDialogId,  _T("dlg id"),        LVCFMT_LEFT, 50, 0);
   InsertColumn(kControlId, _T("ctrl id"),       LVCFMT_LEFT, 50, 0);
   InsertColumn(kCharset,   _T("charset"),       LVCFMT_LEFT, 50, 0);
   InsertColumn(kX,         _T("x"),             LVCFMT_LEFT, 50, 0);
   InsertColumn(kY,         _T("y"),             LVCFMT_LEFT, 50, 0);
   InsertColumn(kText,      _T("text"),          LVCFMT_LEFT, 200, 0);
   InsertColumn(kEnglish,   _T("english text"),  LVCFMT_LEFT, 900, 0);

   return 0;
}

void CMyListCtrl::ReportTruncation(DWORD dlgId, DWORD controlId, DWORD charset,LPCTSTR controlText, SIZE &sizeNeeded, LPCTSTR refString)
{
   int index;
   CString strDlgId, strControlId, strLangId, strCharset,buf;
   CString engText;

   gNumTruncations++;
   strDlgId.Format(_T("%d"), dlgId);
   strControlId.Format(_T("%d"), controlId);
   strLangId.Format(_T("0x%x"), GetThreadLocale());
   strCharset.Format(_T("0x%x"), charset);

   index = InsertItem(kEndOfList, m_dllName);

   SetItemText(index, kDialogId, strDlgId);
   SetItemText(index, kControlId, strControlId);
   SetItemText(index, kLangId, strLangId);
   SetItemText(index, kCharset, strCharset);

   buf.Format(_T("%d"), sizeNeeded.cx );        //just divide by 2 to convert pixels to dialog units (this will not work for all fonts)
   SetItemText(index, kX, buf);
   buf.Format(_T("%d"), sizeNeeded.cy );        //just divide by 2 to convert pixels to dialog units (this will not work for all fonts)
   SetItemText(index, kY, buf);

   SetItemText(index, kText, controlText);

   if (gShowEnglishStrings && refString)
   {
      engText = refString;
      SetItemText(index, kEnglish, engText);
   }

   ProgressMsg(L"Truncation found: lang: %s, dlgId: %d, ctrlId: %d\n", LookupLangName(GetThreadLocale()), dlgId, controlId);

   CString text;
}

void CMyListCtrl::ReportWrongFont(DWORD dlgId, DWORD charset, CString typeface)
{
   int index;
   CString strDlgId, strCharset;

   gNumWrongFont++;
   strDlgId.Format(_T("%d"), dlgId);
   strCharset.Format(_T("0x%x"), charset);

   index = InsertItem(kEndOfList, m_dllName);

   SetItemText(index, kDialogId, strDlgId);
   SetItemText(index, kCharset, strCharset);
   SetItemText(index, kText, typeface);

   ProgressMsg(L"Wrong font found: dlgId: %d, charset: %d\n", dlgId, charset);

   CString text;
}



//////////////////////////////////////////////////
// Functions

int TestDialogsWlan(LPCTSTR resFile, DWORD stringBlock)
{
   TRACE(_T("TestDialogsWlan: file: %s\n"), resFile);
   SetThreadLocale(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
   if (!LoadResources(resFile))
   {
      ErrorMsg(_T("Failed to load resources from file: %s\n"), resFile);
      gbError = TRUE;
   }
   else
   {
      BuildStringMap();

      gShowEnglishStrings = TRUE;

      EnumResourceLanguages(hInstResource, RT_STRING, (LPCTSTR)stringBlock, EnumResLangProc, 0);

      /*
         for (int j=0; j < sizeof(langIds)/sizeof(*langIds); j++)
            EnumResLangProc(hInstResource, RT_STRING, (LPCTSTR)stringBlock, langIds[j], 0);
      */
      }
   return 0;
}

CString LookupLangName(WORD langId)
{
   CString strLangId;
   CString name;

   for (int j=0; j<countof(langCodes); j++)
   {
      if ((langId == langCodes[j].langId) ||
			 (( langId | 0x400) == langCodes[j].langId))
      {
         name = langCodes[j].name;
         break;
      }
   }

   if (name.IsEmpty())
      name.Format(L"%04x", langId);

   return name;
}

BOOL CALLBACK EnumResLangProc(HINSTANCE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LPARAM lParam)
{
	BOOL bLangSupported = TRUE;
	structLangData langData;

   TRACE(_T("language: %x\n"), wIDLanguage);

	// Check if language is supported on current system
	for (int j=0; j < countof(langCodes); j++)
	{
		if ((langCodes[j].langId == wIDLanguage ) ||
			(langCodes[j].langId == (wIDLanguage | 0x400)))
		{
			langData = langCodes[j];
		   if (!IsValidLanguageGroup(langCodes[j].langGroupId, LGRPID_INSTALLED))
				bLangSupported = FALSE;

			break;
		}
	}

	ASSERT(langData.langId);

	CString strLangId;
	strLangId.Format(L"%x", wIDLanguage);

	((CMainWindow*)theApp.m_pMainWnd)->m_listCtrl.InsertItem(kEndOfList, "-->" + LookupLangName(wIDLanguage) + L"(" + strLangId + L")");

	if (!bLangSupported)
	{
		ProgressMsg(_T("Warning: %s language not supported on current system, not testing\n"), langData.name);
		((CMainWindow*)theApp.m_pMainWnd)->m_listCtrl.InsertItem(kEndOfList, _T("Language unsupported on system, not tested"));
	}
	else if ((GetRunningWinVer() == RUNNING_WIN_VER_2K) && 
				(( wIDLanguage == MAKELANGID(LANG_JAPANESE, SUBLANG_NEUTRAL) ||
				 ( wIDLanguage == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)))))
	{
		// Special case for Japanese on Windows 2000:
			// don't test to avoid false failures
			// on Windows 2000 the font linking logic for MS Sans Serif falls back to a korean font preferentially over the correct Japanese font
			// the Korean font is much larger than the Japanese font and will produce some false failures
		ProgressMsg(_T("Warning: Testing Japanese on Windows 2000 systems not supported due to font linking problems, not testing\n"), langData.name);
		((CMainWindow*)theApp.m_pMainWnd)->m_listCtrl.InsertItem(kEndOfList, _T("Language unsupported by dlgtest on Windows 2000, not testing"));
   }
	else
	{
		SetThreadLocale(wIDLanguage);
		EnumResourceNames(hModule, RT_DIALOG, EnumResNameProc, 0);
	}

   return TRUE;
}


BOOL LoadResources(LPCTSTR resFile)
{
   if (hInstResource != (HANDLE) -1)
      FreeLibrary(hInstResource);

   if (!(hInstResource = LoadLibraryEx(resFile, NULL, DONT_RESOLVE_DLL_REFERENCES)))
   {
      TRACE(_T("failed to load resources from: %s; last err: %d\n"), resFile, GetLastError());
      return FALSE;
   }

   AfxSetResourceHandle(hInstResource);

   return TRUE;
}

BOOL CALLBACK EnumResNameProc( HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LPARAM lParam)
{
   CMyDialog dlg;
   
   if (dlg.Create(lpszName, theApp.m_pMainWnd))
   {
      dlg.ShowWindow(SW_HIDE);

      TRACE(L"dialog id: %d\n", (DWORD) lpszName);

      if ((DWORD) lpszName != 30721)        // this is an mfc dialog skip it
      {
         LocalizeDialog(&dlg);

			CheckDialog(&dlg, (DWORD)lpszName);
      }

      dlg.DestroyWindow();
   }
   else
      ProgressMsg(_T("Warning: Failed to create dialog, id: %d\n"), (DWORD) lpszName);

   return TRUE;
}


void CheckDialog(CWnd *pWnd, DWORD dlgId)
 {
   CWnd *pChild;
   CString oldText;
   SIZE sizeNeeded;
   SIZE sizeAvailable;
   BOOL bClipped;
   BOOL bWrongFont=FALSE;

   pChild = pWnd->GetWindow(GW_CHILD);

  // Check Font character set
  CFont *font = pWnd->GetFont();
  LOGFONT lf;
  font->GetLogFont(&lf);
  if(lf.lfCharSet != ANSI_CHARSET)
  {
		// Wrong character set
	  ReportWrongFont(dlgId, lf.lfCharSet,lf.lfFaceName);
		bWrongFont=TRUE;
  }

   while (pChild)
   {
      if (IsDlgTextClipped(pChild, &sizeNeeded, &sizeAvailable))
      {
         CString clippedText;

         pChild->GetWindowText(clippedText);
         ReportTruncation(dlgId, pChild->GetDlgCtrlID(), lf.lfCharSet,clippedText, sizeNeeded, sizeAvailable);
         bClipped = TRUE;
      }
      else
         bClipped = FALSE;

      {
         CString text;
         CString controlText;

		 text.Format(L"%s, %d, %d, %s\n", (((CMainWindow*)(theApp.m_pMainWnd))->m_listCtrl).m_dllName, GetThreadLocale(), dlgId, bWrongFont ? L"Fail" : L"Pass" );
	     UcWriteFile(ghLogFile, text);

		 pChild->GetWindowText(controlText);
         text.Format(L"%s, %d, %d, %d, %s, %d, %d, %d, %d, %s\n", (((CMainWindow*)(theApp.m_pMainWnd))->m_listCtrl).m_dllName, GetThreadLocale(), dlgId, pChild->GetDlgCtrlID(), controlText, sizeNeeded.cx, sizeNeeded.cy, sizeAvailable.cx, sizeAvailable.cy, bClipped ? L"Fail" : L"Pass" );
         UcWriteFile(ghLogFile, text);
      }

      pChild = pChild->GetNextWindow();
   }
 }

void ReportTruncation(DWORD dlgId, DWORD controlId, DWORD charset,LPCTSTR controlText, SIZE &sizeNeeded, SIZE &sizeAvailable, LPCTSTR refString)
{
   CString strControl(controlText);
   if (
      (strControl == L"Spin1") ||
//      (strControl == L"Hide Password") ||
//      (strControl == L"Hide Network key") ||
      (strControl == L"Static"))
   {
     return;
   }

   CMyListCtrl *pListCtrl = &((CMainWindow*)(theApp.m_pMainWnd))->m_listCtrl;

   pListCtrl->ReportTruncation(dlgId, controlId, charset,controlText, sizeNeeded, refString);
}

void ReportWrongFont(DWORD dlgId, DWORD charset,CString typeface)
{
   CMyListCtrl *pListCtrl = &((CMainWindow*)(theApp.m_pMainWnd))->m_listCtrl;

   pListCtrl->ReportWrongFont(dlgId, charset,typeface);
}

BOOL IsDlgTextClipped(CWnd *pWnd, SIZE *pSizeNeeded, SIZE *pSizeAvailable)
{
   CDC *pDC;
   RECT winRect, textRect;
   CString str;
   CWnd *pWndParent;
   CString className;
   BOOL wrapText = FALSE;
   DWORD dwStyle;
   UINT format;
 
   pDC = pWnd->GetDC();
 
   GetClassName(pWnd->m_hWnd, className.GetBuffer(256), 256);
   className.ReleaseBuffer();
 
   pWndParent = pWnd->GetParent();
   if (pWndParent)
      pDC->SelectObject (pWndParent->GetFont());
 
   pWnd->GetWindowRect(&winRect);

   dwStyle = pWnd->GetStyle();

   if (className == _T("Button"))
   {
      if (dwStyle & BS_MULTILINE)
         wrapText = TRUE;

      switch (dwStyle & 0xff)
      {
         case BS_CHECKBOX:       
         case BS_AUTOCHECKBOX: 
         case BS_RADIOBUTTON:    
         case BS_3STATE:         
         case BS_AUTO3STATE:
            winRect.right -= 19;          // 19 = approximate width of checkbox plus associated space in pixels
            break;
      }
   }
   else if (className == _T("Static"))
   {
      if ((dwStyle & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP)
         wrapText = FALSE;
      else
         wrapText = TRUE;
   }

#ifdef PARANOIA
   // this will generate some false negatives, but may catch some problems due to word-wrapping etc.
   if (className == _T("Button"))
   {
      winRect.right -= 15;
   }
#endif

   textRect = winRect;
 
   pWnd->GetWindowText(str);

   HasDots(str);

   format = DT_CALCRECT;
   if (wrapText)
      format |= DT_WORDBREAK;

   pDC->DrawText(str, &textRect, format);
 
   if (pSizeNeeded)
   {
      pSizeNeeded->cx = textRect.right - textRect.left;
      pSizeNeeded->cy = textRect.bottom - textRect.top;
   }

   if (pSizeAvailable)
   {
      pSizeAvailable->cx = winRect.right - winRect.left;
      pSizeAvailable->cy = winRect.bottom - winRect.top;
   }

   return ((textRect.right > winRect.right) || 
      (textRect.bottom > winRect.bottom) ||
      (textRect.top < winRect.top) ||
      (textRect.left < winRect.left));
 }

BOOL CMyDialog::OnInitDialog()
{
//   ShowWindow(SW_SHOW);

   return TRUE;
}

static BOOL EnumStrings(HMODULE hInstance, DWORD resourceBlock, StringEnumFunc EnumProc)
	{
	unsigned int i, currentStringSize;
	WCHAR* currentString;
	WCHAR* stringTable;
	HRSRC hResource = NULL;
	BOOL result = FALSE;
	WCHAR *wStr;
//	WORD language = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	WORD language = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

	hResource = FindResourceEx( hInstance, RT_STRING,
								MAKEINTRESOURCE(resourceBlock),
								language );
	if( NULL == hResource )
		goto quit;

	stringTable = (WCHAR*)LoadResource( hInstance, hResource );	  
	if( NULL == stringTable )
		goto quit;

	currentString = stringTable;

	for( i=0; i<16; i++ )
		{
		if( *currentString )
			{
			DWORD id;
			
			currentStringSize = *currentString;
			currentString++;
			wStr = (WCHAR*)malloc( sizeof(WCHAR) * (currentStringSize+1) );
			wcsncpy(wStr, currentString, currentStringSize);		
			wStr[ currentStringSize ] = L'\0';
			id = (resourceBlock - 1)*16 + i;
			//if (id <= LAST_TRANSLATABLE_STR_ID)
   		EnumProc(wStr, id);
			free(wStr);
			currentString += currentStringSize;
			}
		else
			currentString++;
		}

  quit:
	  return result;
   } 

static void CALLBACK EnumStringProc(WCHAR *string, DWORD id)
	{
	CString text(string);

	stringResMap[text] = (void*)id;
	}

static BOOL CALLBACK EnumStringSectionsProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LPARAM lParam)
	{
	if (IS_INTRESOURCE(lpszName))
   {
		EnumStrings(hModule, (DWORD) lpszName, EnumStringProc);
   }

	return TRUE;
	}

void BuildStringMap(void)
	{
   stringResMap.RemoveAll();
	EnumResourceNames(hInstResource, RT_STRING, EnumStringSectionsProc, 0);
	}

CString GetRcString(int id)
{
   CString str;
   str.LoadString(id);
   return str;
}

// Load string from english resources
CString GetEngString(int id)
{
   CString str;

   LoadString(hInstRefResource, id, str.GetBuffer(1024), 1024);
   str.ReleaseBuffer();

   return str;
}

void LocalizeDialog(CWnd *pWnd)
	{
	CWnd *pChild;
	CString oldText;

	pChild = pWnd->GetWindow(GW_CHILD);

	while (pChild)
		{
		pChild->GetWindowText(oldText);
		pChild->SetWindowText(TranslateString(oldText));
		pChild = pChild->GetNextWindow();
		}
	}

/*If string has more than four trailing periods remove them and return true */
static bool HasDots(CString &str)
	{
	bool retval = false;

	if (str.Right(5) == ".....")
		{
		retval = true;
		while (str.Right(1) == ".")
			str.Delete(str.GetLength() - 1);
		}
	return retval;
	}

CString TranslateString(CString inStr)
	{
	DWORD id;
	CString outStr;
	void *p;
	bool trailingDots;

	trailingDots = HasDots(inStr);

	outStr = "";

	if (!inStr.GetLength())
		return CString("");

	if (stringResMap.Lookup(inStr, p))
		{
		id = (DWORD) p;
      WCHAR *pWstr;

      outStr = (pWstr = LocalizedString(hInstResource, id, GetThreadLocale()));
      FreeString(pWstr);

		if (!outStr.GetLength())
			{
			outStr = inStr;
			TRACE("Warning: zero length translation for string: " + inStr + "\n");		
			}
		}
	
	if (!outStr.GetLength() && 
       (inStr != _T("Static")) &&
       (inStr != _T("...")) )
		{
		ProgressMsg("Warning: translation not found for string: " + inStr + "\n");
		outStr = inStr;
      gNumUntranslatedStrings++;
		}

	if (trailingDots)
		outStr = outStr + kManyDots;

	return outStr;
	}


// (from win32common)
WCHAR* LocalizedString( HINSTANCE hInstance, UINT id, WORD language )
{
	  unsigned int i, currentStringSize;
	  WCHAR* currentString;
	  WCHAR* stringTable;

      unsigned int resourceBlock = id / 16 + 1;
      unsigned int stringIndex  = id % 16;
      HRSRC hResource = NULL;
	  WCHAR* result = NULL;

      hResource = FindResourceEx( hInstance, RT_STRING,
                                  MAKEINTRESOURCE(resourceBlock),
								  language );
      if( NULL == hResource )
      {
         TRACE(_T("FindResourceEx failed with code: %d\n"), GetLastError());
		  goto quit;
      }

	  stringTable = (WCHAR*)LoadResource( hInstance, hResource );	  
	  if( NULL == stringTable )
		  goto quit;
	  
	  currentString = stringTable;
	  for( i=0; i<16; i++ )
		  {
		  if( *currentString )
			  {
			  currentStringSize = *currentString;
			  currentString++;
			  if( i == stringIndex )
				  {
				  result = (WCHAR*)malloc( sizeof(WCHAR) * (currentStringSize+1) );
				  /* result = (WCHAR*)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY,
					 sizeof(WCHAR) * (currentStringSize+1) ); */
				  wcsncpy( result, currentString, currentStringSize );
				  result[ currentStringSize ] = L'\0';
				  goto quit;
				  }
			  currentString += currentStringSize;
			  }
		  else
			  currentString++;
		  }

   CloseHandle(hResource);
  quit:
	  return result;
   } 

void FreeString( void* string )
{
if(string)
	free( string );
}

// Open a unicode text file
// call CloseHandle to close handle
HANDLE UcOpenFile(LPCTSTR fileName)
{
   DWORD nBytes;
   unsigned char header[2] = {0xFF, 0xFE};

   HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
      WriteFile(hFile, header, 2, &nBytes, 0);
   }
   else
      ProgressMsg(_T("Unable to open file: %s"), fileName);
   return hFile;
}

// Write to unicode text file
void UcWriteFile(HANDLE hFile, LPCTSTR text)
{
   DWORD nBytes;

   if (hFile == INVALID_HANDLE_VALUE)
      return;

   WriteFile(hFile, (LPCVOID)text, _tcslen(text) * sizeof(TCHAR), &nBytes, 0);
}

int ErrorMsg(LPCTSTR format, ...)
{
   va_list vl;
   CString str;

   va_start(vl,format);
   str.FormatV(format, vl);

   TRACE(str);
   _tprintf(str + "\n");
   if (!gbPassFail)
      AfxMessageBox(str, 0, 0);

   return str.GetLength();
}

int ProgressMsg(LPCTSTR format, ...)
{
   va_list vl;
   CString str;

   va_start(vl,format);
   str.FormatV(format, vl);

   TRACE(str);
   _tprintf(str + "\n");

   return str.GetLength();
}

DWORD
GetRunningWinVer( void )
	{
	OSVERSIONINFOA verInfo;

	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if( !GetVersionExA( &verInfo ) )
		return RUNNING_WIN_VER_UNKNOWN;

	if( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
		if( verInfo.dwMajorVersion >= 5 )
			{
			if (verInfo.dwMinorVersion == 0)
				return RUNNING_WIN_VER_2K;
			else
				return RUNNING_WIN_VER_XP;
			}
		else if( verInfo.dwMajorVersion == 4 )
			return RUNNING_WIN_VER_NT4;
		}
	else if( verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		{
		if( verInfo.dwMajorVersion > 4 )
			return RUNNING_WIN_VER_ME;
		else if( verInfo.dwMajorVersion == 4 )
			{
			if( verInfo.dwMinorVersion >= 90 )
				return RUNNING_WIN_VER_ME;		
			else if( verInfo.dwMinorVersion >= 10 )
				return RUNNING_WIN_VER_98SE;
			else if( verInfo.dwMinorVersion > 0 )
				return RUNNING_WIN_VER_98;
			else if( verInfo.dwMinorVersion == 0 )
				return RUNNING_WIN_VER_95;
			}
		}
	return RUNNING_WIN_VER_UNKNOWN;
	}
