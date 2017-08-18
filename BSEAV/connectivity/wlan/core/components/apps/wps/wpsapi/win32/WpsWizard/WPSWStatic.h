/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WPSWStatic.h,v 1.4 2008/08/25 21:35:13 Exp $
 */
#pragma once
#include "XColorStatic.h"

class CWPSWStatic :	public CXColorStatic
{
public:
	CWPSWStatic(void);
	~CWPSWStatic(void);

	void SetWPSWWindowText(LPCTSTR lpcszText);
	void SetWPSWWindowText(UINT uStringId);

// Overrides
protected:
	//{{AFX_VIRTUAL(CXColorStatic)
    virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};
