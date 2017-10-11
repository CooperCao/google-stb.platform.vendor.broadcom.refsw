/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: PbcModePg.cpp,v 1.6 2008/08/20 01:45:24 Exp $
*/

// PbcModePg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "PbcModePg.h"


// CPbcModePg dialog

//IMPLEMENT_DYNAMIC(CPbcModePg, CWizardPropPg)

CPbcModePg::CPbcModePg()
	: CWizardPropPg(CPbcModePg::IDD)
{

}

CPbcModePg::~CPbcModePg()
{
}

void CPbcModePg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_PBC_DESCRIPTION, m_staticPbcDesc);
}


BEGIN_MESSAGE_MAP(CPbcModePg, CWizardPropPg)
END_MESSAGE_MAP()

BOOL CPbcModePg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	SetLabel(IDC_GROUP_PBC_MODE, IDS_PBC);
	SetLabel(IDC_STATIC_PBC_DESCRIPTION, IDS_PBC_DESCRIPTION);

	return TRUE;
}

LRESULT CPbcModePg::OnWizardNext()
{
	m_pNextPage = &(m_pWizardSheet->m_ConfigStatusPg);

	return CWizardPropPg::OnWizardNext(this);
}
