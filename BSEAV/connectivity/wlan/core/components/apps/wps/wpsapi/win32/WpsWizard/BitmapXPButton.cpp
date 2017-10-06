/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: BitmapXPButton.cpp,v 1.1 2008/06/12 02:49:03 Exp $
*/
// BitmapXPButton.cpp : implementation file
//

#include "stdafx.h"
//#include "Test.h"
#include "BitmapXPButton.h"

#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapXPButton

CBitmapXPButton::CBitmapXPButton()
{

	m_bOverControl=FALSE;
	m_bTracking = FALSE;
	m_bPressed = FALSE;

}

CBitmapXPButton::~CBitmapXPButton()
{
}


BEGIN_MESSAGE_MAP(CBitmapXPButton, CButton)
	//{{AFX_MSG_MAP(CBitmapXPButton)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitmapXPButton message handlers

void CBitmapXPButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	
}

void CBitmapXPButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
		if (!m_bTracking)
		{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTracking = _TrackMouseEvent(&tme);		
		}	
	CButton::OnMouseMove(nFlags, point);
	
	
}

LRESULT CBitmapXPButton::OnMouseHover(WPARAM wparam, LPARAM lparam)
{
	m_bOverControl=TRUE;
	//Invalidate();
	return 1;
}

LRESULT CBitmapXPButton::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bTracking = FALSE;
	m_bOverControl = FALSE;
	Invalidate(FALSE);
	return 0;
}

void CBitmapXPButton::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT rect;

	::GetClientRect(this->m_hWnd, &rect);

	CBrush br(  m_transparent_color  );
	dc.FillRect( &rect, &br );
	
	// TODO: Add your message handler code here
	if( m_bOverControl )
		{

		if( ! m_bPressed )
		{
		HBITMAP hBitmap = LoadBmp( m_nResourceID );
		DrawTransparent(dc.m_hDC, rect.left+3, rect.top+3, hBitmap, m_transparent_color);
		DeleteObject( hBitmap );
		}
		else
		{

		HBITMAP hBitmap = LoadBmp( /*IDB_BITMAP_PRESSED*/ m_nUnPressedResourceID );
		DrawTransparent(dc.m_hDC, rect.left+3, rect.top+3, hBitmap, m_transparent_color);
		DeleteObject( hBitmap );

		}

		// DrawXP_StyleRawButton( dc.m_hDC, rect.left, rect.top, rect.right, rect.bottom, true, true );
		}
	else
		{

		 HBITMAP hBitmap = LoadBmp( m_nResourceID );
		 DrawTransparent(dc.m_hDC, rect.left+3, rect.top+3, hBitmap, m_transparent_color);
		 DeleteObject( hBitmap );

		// DrawXP_StyleRawButton( dc.m_hDC, rect.left, rect.top, rect.right, rect.bottom, true, false);

		}
	
	// Do not call CButton::OnPaint() for painting messages
}



void CBitmapXPButton::DrawXP_StyleRawButton( HDC hdc, int x1, int y1, int x2, int y2, bool border, bool bBtnPressed )
{
	HPEN penBtnHiLight, pen3DLight, penBtnShadow, pen3DDKShadow;
	HPEN oldpen; // HBRUSH hbClicked;



	penBtnHiLight = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)); // White
	pen3DLight = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));       // Light gray
	penBtnShadow = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));   // Dark gray
	pen3DDKShadow = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW)); // Black

	// DrawColorFrame( hdc, x1+7, y1+6, 20, 20, RGB(0,0,0));
    // DrawColorFrame( hdc, x1+8, y1+7, 18, 18, m_skin.GetCurrSkinColor());

	// ptFont(m_hdc[Mem], NULL, x1+40, (y2-y1)/2 + y1 -8,  m_skin.GetCurrSkinDesc());

	if( !border )
		return;

	if( bBtnPressed )
		{
			// Black line
		    
			oldpen = (HPEN) SelectObject( hdc, pen3DDKShadow);
			MoveToEx( hdc, x1, y2-1, (LPPOINT) NULL);
			LineTo( hdc, x1, y1);
			LineTo( hdc, x2, y1 );

			// Dark gray  line
			SelectObject(hdc, penBtnShadow);
			MoveToEx(hdc, x1+1, y2-1, (LPPOINT) NULL);
			LineTo(hdc, x1+1, y1+1);
			LineTo(hdc,  x2,  y1+1);

			// Draw bottom-right borders
			// White Black line
			
			SelectObject( hdc, penBtnHiLight);
			MoveToEx(hdc,  x1, y2-1, (LPPOINT) NULL);
			LineTo(hdc,  x2-1, y2-1);
			LineTo(hdc,  x2-1, y1-1);
			// Light gray line
			
			SelectObject(hdc, pen3DLight);
			MoveToEx(hdc,  x1+1, y2-2, (LPPOINT) NULL);
			LineTo(hdc,  x2-2, y2-2);
			LineTo(hdc,  x2-2, y1);
			//
			SelectObject( hdc, oldpen);
		
		}
	else
		{
			//
			// Draw top-left borders
			// White line
			oldpen = (HPEN) SelectObject( hdc, penBtnHiLight);
			MoveToEx( hdc, x1, y2-1, (LPPOINT) NULL);
			LineTo( hdc, x1, y1);
			LineTo( hdc, x2, y1 );

			// Light gray line
			SelectObject(hdc, pen3DLight);
			MoveToEx(hdc, x1+1, y2-1, (LPPOINT) NULL);
			LineTo(hdc, x1+1, y1+1);
			LineTo(hdc,  x2,  y1+1);

			// Draw bottom-right borders
			// Black line
			SelectObject( hdc, pen3DDKShadow);
			MoveToEx(hdc,  x1, y2-1, (LPPOINT) NULL);
			LineTo(hdc,  x2-1, y2-1);
			LineTo(hdc,  x2-1, y1-1);
			// Dark gray line
			SelectObject(hdc, penBtnShadow);
			MoveToEx(hdc,  x1+1, y2-2, (LPPOINT) NULL);
			LineTo(hdc,  x2-2, y2-2);
			LineTo(hdc,  x2-2, y1);
			//
			SelectObject( hdc, oldpen);
		}

	DeleteObject( penBtnShadow );
	DeleteObject( pen3DDKShadow );
    DeleteObject( pen3DLight );
	DeleteObject( penBtnHiLight );
			

}




void CBitmapXPButton::DrawTransparent(HDC hdc, int x, int y, HBITMAP hBitmap, COLORREF crColour)
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



HBITMAP CBitmapXPButton::LoadBmp( int resID )
{
	return (HBITMAP)LoadImage( AfxGetApp()->m_hInstance,  MAKEINTRESOURCE(resID), IMAGE_BITMAP, 0, 0,0); 
}





void CBitmapXPButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	m_bPressed = TRUE;
	Invalidate(TRUE);

	
	
	CButton::OnLButtonDown(nFlags, point);
}

void CBitmapXPButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	m_bPressed = FALSE;

	Invalidate(TRUE);

	OnPaint();

	
	Sleep(1000);
	
	CButton::OnLButtonUp(nFlags, point);
}
