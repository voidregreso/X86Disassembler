

#pragma once


class CX86DisasmEngineDlg : public CDialogEx
{
public:
	CX86DisasmEngineDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_X86DISASMENGINE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV Support

protected:
	HICON m_hIcon;

	// Funciones generadas de asignación de mensajes
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChfile();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	void ZGDisassemble();
};
