/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: StaticBitmap.h,v 1.1 2008/06/12 02:49:03 Exp $
*/
#if !defined(AFX_STATICBITMAP_H__B5A3D294_D207_4B39_90E0_FDC6200DAD6B__INCLUDED_)
#define AFX_STATICBITMAP_H__B5A3D294_D207_4B39_90E0_FDC6200DAD6B__INCLUDED_

#if 0> 1000
#pragma once
#endif 
// StaticBitmap.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticBitmap window

class CStaticBitmap : public CStatic
{

protected:
	UINT    m_resourceID;
	HBITMAP m_hBitmap;
	COLORREF m_transparent_color;// transparent color for the bitmap
	COLORREF m_bkcolor;
	int     m_nXLeading, m_nYLeading;
// Construction
public:
	CStaticBitmap();

// Attributes
public:

// Operations

protected:

	void DrawTransparent(HDC hdc, int x, int y, HBITMAP hBitmap, COLORREF crColour);
	HBITMAP LoadBmp( int resID );

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticBitmap)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticBitmap();

	void SetTranspartColor( COLORREF color ) { m_transparent_color = color; }
	void SetBkColor( COLORREF color ) { m_bkcolor = color; }
	void SetBitmapId( UINT resourceID ) {  m_resourceID = resourceID; }
	void SetXLeading( int x ) { m_nXLeading = x; }
	void SetYLeading( int y ) { m_nYLeading = y; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticBitmap)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

enum eIndicattorState
{
	IDLE_STATE = 0,
	SUCCESS_STATE,
	FAIL_STATE,

	STATE_TOTAL
};

class CStaticBitmap3States : public CStaticBitmap
{
public:
	CStaticBitmap3States();

	afx_msg void OnTimer(UINT nIDEvent);

	void StartBlinking();
	void StopBlinking();

	void SetTranspartColor( COLORREF color ) { m_transparent_color = color; }
	void SetBkColor( COLORREF color ) { m_bkcolor = color; }
	void SetStateBitmapId( UINT IdleId, UINT SuccessId, UINT FailId  );
	void SetXLeading( int x ) { m_nXLeading = x; }
	void SetYLeading( int y ) { m_nYLeading = y; }

	void TriggerState();   // Trigger the blanking state
	void SetTotalBlankingState( int nTotalBlankingState ) { m_nBlankingState = nTotalBlankingState; }
	void SetState( eIndicattorState state, BOOL bUpdateView=TRUE );

private:
	UINT m_IdleStateId;
	UINT m_SuccessStateId;
	UINT m_FailStateId;

	UINT m_nStateId[STATE_TOTAL];

	int  m_nCurrentState;
	int  m_nBlankingState;
	UINT_PTR m_nBlinkingTimer;
};









/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICBITMAP_H__B5A3D294_D207_4B39_90E0_FDC6200DAD6B__INCLUDED_)
