
// MapelHelperDlg.h: 헤더 파일
//

#pragma once


// CMapelHelperDlg 대화 상자
class CMapelHelperDlg : public CDialogEx
{
// 생성입니다.
public:
	CMapelHelperDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAPELHELPER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton2();
	int m_editAccept;
	int m_editRepeatDelay;
	int m_editRepeatRate;
	afx_msg void OnEnChangeEdit3();
private:
	int m_nBackupEditAccept;
	int m_nBackupEditRepeatDelay;
	int m_nBackupEditRepeatRate;
};
