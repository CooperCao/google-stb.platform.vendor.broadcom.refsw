/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: ConfigStatusPg.h,v 1.10 2009/02/17 18:22:20 Exp $
*/
#pragma once
#include "WizardPropPg.h"
#include "wps_sdk.h"
#include "afxwin.h"
#include "StaticBitmap.h"
#include "XColorStatic.h"

struct ST_WPSSTATEMSG 
{
	eBRCM_WPSSTATE eState;
	UINT szMsgID;
};

class CConfigStatusPg : public CWizardPropPg
{
public:
	CConfigStatusPg();
	virtual ~CConfigStatusPg();

// Dialog Data
	enum { IDD = IDD_CONFIGSTATUSPG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();

public:
	LRESULT OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCancelWps();

	DECLARE_MESSAGE_MAP()

	CListBox m_StatusLog;

	CStaticBitmap3States m_BmpIndInitializing;
	CStaticBitmap3States m_BmpIndSearchingAP;
	CStaticBitmap3States m_BmpIndJoiningAP;
	CStaticBitmap3States m_BmpIndNegoRetr;
	CStaticBitmap3States m_BmpIndCreatingProfile;
	CStaticBitmap3States m_BmpIndRestoringEnv;

	CWPSWStatic m_staticConfStatus;
	CWPSWStatic m_staticResult;

	CWPSWStatic m_LabelInitWps;
	CWPSWStatic m_LabelSearchAP;
	CWPSWStatic m_LabelJoinAP;
	CWPSWStatic m_LabelNegoRetr;
	CWPSWStatic m_LabelCreateProfile;
	CWPSWStatic m_LabelRestoreEnv;

	CWPSWStatic m_LabelInitWpsRel;
	CWPSWStatic m_LabelSearchAPRel;
	CWPSWStatic m_LabelJoinAPRel;
	CWPSWStatic m_LabelNegoRetrRel;
	CWPSWStatic m_LabelCreateProfileRel;
	CWPSWStatic m_LabelRestoreEnvRel;
	CWPSWStatic m_staticPin;
	CWPSWStatic m_labelCurPin;

private:
	int StartWPS();
	void StopWps();
	void UpdateWpsStatus();
	void AddStatusLog(const CString& strLog);
	void AddStatusLog(UINT uStrID);
	void InitBmpIndicators();
	void UpdateBmpIndicators(eBRCM_WPSEVENT eWpsEvent, int nError);
	void UpdateTextIndicators(eBRCM_WPSEVENT eWpsEvent, int nError);
	eBRCM_WPSSTATE GetNextState(eBRCM_WPSSTATE eCurrent, eBRCM_WPSEVENT eEvent);
	CString GetWpsMsgTypeString(int nMsgType);

// Data
	CString m_strSelectedAP;
	CButton m_WpsControl;
	unsigned int m_uiStatus;
	wps_credentials m_credentials;
	CString m_strWpsPin;
	eBRCM_WPSSTATE m_eCurWpsState;
	int m_nLogIndex;
};
