// ArmAUnbinDlg.h : header file
//

#pragma once


// CArmAUnbinDlg dialog
class CArmAUnbinDlg : public CDialog
{
// Construction
public:
	CArmAUnbinDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ARMAUNBIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSave();
	CString configEditValue;
private:
	int ReadCompressedInt(CFile &f);
	void ReadAsciiz(char *str, int n, CFile &f);
	bool ReadArray(CFile &f, CString &cpp, bool recursive, int recursionLevel);
	long ReadClassBody(CFile &f, CString &cpp, int recursionLevel);
	void ReadConfigBin(CString filename, CString &cpp);
	void ReadEnumList(CFile &f, CString &cpp);

};
