/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WPSWStatic.cpp,v 1.4 2008/08/25 21:35:13 Exp $
 */
#include "StdAfx.h"
#include "FontSize.h"

CWPSWStatic::CWPSWStatic(void)
{
	// Default background color
	m_rgbBackground = RGB(255,255,255); 
}

CWPSWStatic::~CWPSWStatic(void)
{
}

BEGIN_MESSAGE_MAP(CWPSWStatic, CXColorStatic)
	//{{AFX_MSG_MAP(CWPSWStatic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWPSWStatic::PreSubclassWindow() 
{
	// get current font
	CFont* pFont = GetFont();
	if (!pFont)
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
		if (hFont)
			pFont = CFont::FromHandle(hFont);
	}
	ASSERT(pFont);
	ASSERT(pFont->GetSafeHandle());

	// create the font for this control
	LOGFONT lf;
	pFont->GetLogFont(&lf);

	// Default font size and typeface 
	lf.lfHeight = GetFontHeight(10);
	_tcscpy(lf.lfFaceName, _T("Times Roman"));

	m_font.CreateFontIndirect(&lf);	

	m_pBrush = new CBrush(m_rgbBackground);
}

void CWPSWStatic::SetWPSWWindowText(LPCTSTR lpcszText)
{
	SetWindowText(lpcszText);
	Invalidate();
}

void CWPSWStatic::SetWPSWWindowText(UINT uStringId)
{
	CString strText;

	strText.LoadString(uStringId);
	SetWindowText(strText);
	Invalidate();
}
