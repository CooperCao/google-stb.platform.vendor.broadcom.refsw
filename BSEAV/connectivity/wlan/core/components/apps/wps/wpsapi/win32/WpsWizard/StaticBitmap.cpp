/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: StaticBitmap.cpp,v 1.1 2008/06/12 02:49:03 Exp $
*/
// StaticBitmap.cpp : implementation file
//

#include "stdafx.h"
#include "StaticBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMER_BLINKING_EVENT	2
#define TIMER_PERIOD			1000

static CStaticBitmap3States* g_pThis;

CStaticBitmap::CStaticBitmap()
{
	m_nXLeading = 5;
	m_nYLeading = 5;
}

CStaticBitmap::~CStaticBitmap()
{
}


BEGIN_MESSAGE_MAP(CStaticBitmap, CStatic)
	//{{AFX_MSG_MAP(CStaticBitmap)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticBitmap message handlers

void CStaticBitmap::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	

	HBITMAP hBitmap = LoadBmp( m_resourceID );
	DrawTransparent( dc.m_hDC, m_nXLeading, m_nYLeading, hBitmap, m_transparent_color);
	DeleteObject( hBitmap );

	
	// Do not call CStatic::OnPaint() for painting messages
}

BOOL CStaticBitmap::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default

	CRect cr;
	GetClientRect(cr); 


	
	CBrush bk( m_bkcolor  );
	
	pDC->FillRect(&cr, &bk);

	
	return TRUE;
}


// Draw Transparent Bitmap
// hdc: device context to be frawn
// x, y: coordination
// hBitmap: handle of bitmap
// crColour: transparent color
void CStaticBitmap::DrawTransparent(HDC hdc, int x, int y, HBITMAP hBitmap, COLORREF crColour)
{
	
   
	COLORREF crOldBack = ::SetBkColor(hdc, RGB(255, 255, 255));
	COLORREF crOldText = SetTextColor(hdc, RGB(0, 0, 0));
	HDC dcImage, dcTrans;
	
	
	// Create two memory dcs for the image and the mask
	dcImage=CreateCompatibleDC(hdc);
	dcTrans=CreateCompatibleDC(hdc);

	// Select the image into the appropriate dc
	HBITMAP pOldBitmapImage = (HBITMAP)SelectObject(dcImage, hBitmap);

	// Create the mask bitmap
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	HBITMAP bitmapTrans=CreateBitmap(bitmap.bmWidth, bitmap.bmHeight, 1, 1, NULL);

	// Select the mask bitmap into the appropriate dc
	HBITMAP pOldBitmapTrans = (HBITMAP)SelectObject(dcTrans, bitmapTrans);

	// Build mask based on transparent colour
	::SetBkColor(dcImage, crColour);
	BitBlt(dcTrans, 0, 0, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCCOPY);

	// Do the work - True Mask method - cool if not actual display
	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCINVERT);
	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcTrans, 0, 0, SRCAND);
	BitBlt(hdc, x, y, bitmap.bmWidth, bitmap.bmHeight, dcImage, 0, 0, SRCINVERT);

	// Restore settings
	SelectObject(dcImage, pOldBitmapImage);
	SelectObject(dcTrans, pOldBitmapTrans);
	::SetBkColor(hdc, crOldBack);
	SetTextColor(hdc, crOldText);
}



HBITMAP CStaticBitmap::LoadBmp( int resID )
{
	return (HBITMAP)LoadImage( AfxGetApp()->m_hInstance,  MAKEINTRESOURCE(resID), IMAGE_BITMAP, 0, 0,0); 
}



CStaticBitmap3States::CStaticBitmap3States()
{
	m_nCurrentState = IDLE_STATE;
	m_nBlankingState = 2;

}

void CStaticBitmap3States::SetStateBitmapId( UINT idleID, UINT SuccessID, UINT FailID )
{
	m_nStateId[0] = idleID;
	m_nStateId[1] = SuccessID;
	m_nStateId[2] = FailID;
}

void CStaticBitmap3States::TriggerState()
{
	m_nCurrentState = (m_nCurrentState+1) % m_nBlankingState;
	SetBitmapId( m_nStateId[ m_nCurrentState ] );
	Invalidate(TRUE);	
}

void CStaticBitmap3States::SetState(eIndicattorState state, BOOL bUpdateView )
{
	if( state < STATE_TOTAL )
	{
		m_nCurrentState = state;
		SetBitmapId( m_nStateId[ m_nCurrentState ] );
		if( bUpdateView )
			Invalidate(TRUE);
	}
}

VOID CALLBACK BlinkingTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	g_pThis->OnTimer((UINT)idEvent);
}

void CStaticBitmap3States::StartBlinking()
{
	g_pThis = this;
	m_nBlinkingTimer = SetTimer(TIMER_BLINKING_EVENT, TIMER_PERIOD, BlinkingTimerProc);;
}

void CStaticBitmap3States::StopBlinking()
{
	KillTimer(m_nBlinkingTimer);
}

void CStaticBitmap3States::OnTimer(UINT nIDEvent) 
{
	TriggerState();
}
