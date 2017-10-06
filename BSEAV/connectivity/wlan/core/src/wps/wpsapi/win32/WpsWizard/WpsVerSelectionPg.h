#pragma once

// CWpsVerSelectionPg dialog

class CWpsVerSelectionPg : public CWizardPropPg
{
//	DECLARE_DYNAMIC(CWpsVerSelectionPg)

public:
	CWpsVerSelectionPg();
	virtual ~CWpsVerSelectionPg();

// Dialog Data
	enum { IDD = IDD_VERSELECTIONPG };

	afx_msg void OnBnClickedWpsV1();
	afx_msg void OnBnClickedWpsV2();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnWizardNext();

	DECLARE_MESSAGE_MAP()
};
