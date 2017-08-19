#include "stdafx.h"

class CMyApp : public CWinApp
{
public:
    virtual BOOL InitInstance ();
    virtual int ExitInstance();
	 void RegisterCustomWindowClasses();
};

class CMyListCtrl : public CListCtrl
{
public:
   enum {
      kFileName,
      kLangId,
      kDialogId,
      kControlId,
	  kCharset,
      kX,
      kY,
      kText,
      kEnglish
   } columnIndices;

   void CMyListCtrl::ReportTruncation(DWORD dlgId, DWORD controlId, DWORD charset, LPCTSTR controlText, SIZE &sizeNeeded, LPCTSTR refString);
   void CMyListCtrl::ReportWrongFont(DWORD dlgId, DWORD charset,CString typeface);
   CString m_dllName;

protected:
   //{{AFX_MSG(CMyListCtrl)
   afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};


class CMainWindow : public CFrameWnd
{
public:
    CMainWindow ();
    CMyListCtrl m_listCtrl;

protected:
   void OnResizeWindow();
   //{{AFX_MSG(CMainWindow)
    void OnSizing(UINT fwSide, LPRECT pRect) ;
    void OnSize(UINT nType, int cx, int cy);
   //}}AFX_MSG
    DECLARE_MESSAGE_MAP ()
};

class CMyDialog : public CDialog
 {
public:
   BOOL CMyDialog::IsReady();

protected:
	virtual BOOL OnInitDialog();
   DECLARE_MESSAGE_MAP()
 };
