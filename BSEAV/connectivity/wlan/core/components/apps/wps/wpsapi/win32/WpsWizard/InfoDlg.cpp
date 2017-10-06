/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: InfoDlg.cpp,v 1.2 2008/08/25 21:35:13 Exp $
 */
// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//
// RTF edit control StreamIn's callback function
//    - see CRichEditCtrl::StreamIn
//
static DWORD CALLBACK _LoadRtfCallback2(
		DWORD dwCookie,      // (in) pointer to the string
		LPBYTE pbBuff,       // (in) pointer to the destination buffer
		LONG cb,             // (in) size in bytes of the destination buffer
		LONG FAR *pcb        // (out) number of bytes transfered
		)
{
	
	CString *pstr = (CString *)dwCookie;

	if( pstr->GetLength() < cb )
	{
		*pcb = pstr->GetLength();
		memcpy(pbBuff, (LPCTSTR)*pstr, *pcb );
		pstr->Empty();
	}
	else
	{
		*pcb = cb;
		memcpy(pbBuff, (LPCTSTR)*pstr, *pcb );
		*pstr = pstr->Right( pstr->GetLength() - cb );
	}
	return 0;
}

static BOOL _LoadStringEx2( UINT nResId, LPCTSTR pszRsType, CString& strOut )
{
	LPTSTR pszResId = MAKEINTRESOURCE(nResId);
	HINSTANCE hInst = ::AfxFindResourceHandle( pszResId, pszRsType );

	if( hInst == NULL )
	  return FALSE;

	HRSRC hRsrc = ::FindResource( hInst, pszResId, pszRsType );

	if( hRsrc == NULL )
	  return FALSE;

	HGLOBAL hGlobal = ::LoadResource( hInst, hRsrc );

	if (hGlobal == NULL)
		return FALSE;

	const BYTE* pData = (const BYTE*)::LockResource( hGlobal );
	DWORD dwSize = ::SizeofResource( hInst, hRsrc );

	if( pData == NULL )
	  return FALSE;

	CString str( (LPCTSTR)pData, dwSize );
	strOut = str;

	::UnlockResource( hGlobal );
	::FreeResource( hGlobal );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog


CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDlg)
	DDX_Control(pDX, IDC_RICHEDIT1, m_ctrlRichText);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_ctrlText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers

BOOL CInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString text;


	text.LoadString( m_TitleId );
	SetWindowText( text );

	//text.LoadString( m_BodyTextId );
	//m_ctrlText.SetWindowText( text );


	CString m_Msg;

	//m_BodyText.ShowWindow( SW_HIDE );

	m_ctrlRichText.ShowWindow( SW_SHOW );

	


	m_ctrlRichText.SetBackgroundColor(FALSE, RGB( 255, 255, 255 ));
		m_ctrlRichText.SetOptions(ECOOP_SET,ECO_READONLY );
	
	m_ctrlRichText.SetOptions(ECOOP_SET,ECO_READONLY | ECO_SAVESEL | ECO_AUTOWORDSELECTION);


		if (!_LoadStringEx2(m_BodyTextId, _T("RTF"), m_Msg))
			{
			return FALSE;
			}
	


		// move the RTF to the Rich Edit
		// Note: use of StreamIn is required for WinNT, for Win2K it is allowed to do a SetWindowText.

		EDITSTREAM es;
		es.dwCookie = (DWORD)&m_Msg;
		es.dwError = 0;
		es.pfnCallback = _LoadRtfCallback2;

		m_ctrlRichText.StreamIn( SF_RTF, es );

	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
