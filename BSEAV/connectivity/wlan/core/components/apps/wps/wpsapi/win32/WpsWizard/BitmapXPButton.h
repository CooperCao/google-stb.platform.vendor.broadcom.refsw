/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: BitmapXPButton.h,v 1.1 2008/06/12 02:49:03 Exp $
*/
#if !defined(AFX_BITMAPXPBUTTON_H__AF51C176_44A8_4B60_B322_527807F26F7B__INCLUDED_)
#define AFX_BITMAPXPBUTTON_H__AF51C176_44A8_4B60_B322_527807F26F7B__INCLUDED_

#if 0> 1000
#pragma once
#endif 
// BitmapXPButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBitmapXPButton window

class CBitmapXPButton : public CButton
{
// Construction
public:
	CBitmapXPButton();

// Attributes
public:

protected:
	BOOL m_bOverControl;
	BOOL m_bTracking;
	BOOL m_bSelected;
	BOOL m_bPressed;
	int  m_nResourceID;  // resource bitmap id - Pressed
	int  m_nUnPressedResourceID;  // resource bitmap id - Unpressed
	COLORREF m_transparent_color;

	void DrawXP_StyleRawButton( HDC hdc, int x1, int y1, int x2, int y2, bool border, bool bBtnPressed );

	LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	void DrawTransparent(HDC hdc, int x, int y, HBITMAP hBitmap, COLORREF crColour);
	HBITMAP LoadBmp( int resID );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapXPButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBitmapXPButton();
	void     SetBitmapResourceID( int id, COLORREF transparent_color ) { m_nResourceID = id; m_transparent_color = transparent_color; }
    void     SetBitmapResourceID( int id, int unpressid, COLORREF transparent_color ) { m_nResourceID = id; m_nUnPressedResourceID=unpressid; m_transparent_color = transparent_color; }
	// Generated message map functions
protected:
	//{{AFX_MSG(CBitmapXPButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPXPBUTTON_H__AF51C176_44A8_4B60_B322_527807F26F7B__INCLUDED_)
