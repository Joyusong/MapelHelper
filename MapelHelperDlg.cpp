
// MapelHelperDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MapelHelper.h"
#include "MapelHelperDlg.h"
#include "afxdialogex.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMapelHelperDlg 대화 상자



CMapelHelperDlg::CMapelHelperDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAPELHELPER_DIALOG, pParent)
	, m_nDelay(0)
	, m_nRate(0)
	, m_nBounce(0),
	m_nBackupDelay(0),
m_nBackupRate(0),
m_nBackupBounce(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMapelHelperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_EDIT1, m_fDelay);
	DDX_Text(pDX, IDC_EDIT1, m_nDelay);
	DDX_Text(pDX, IDC_EDIT2, m_nRate);
	DDX_Text(pDX, IDC_EDIT3, m_nBounce);
}

BEGIN_MESSAGE_MAP(CMapelHelperDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMapelHelperDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CMapelHelperDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMapelHelperDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT3, &CMapelHelperDlg::OnEnChangeEdit3)
END_MESSAGE_MAP()


// CMapelHelperDlg 메시지 처리기

BOOL CMapelHelperDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 필터키 초기 세팅 확인하기. (txt로 저장)
	//1 . 현재 실행 파일의 경로를 가져옵니다.
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	// 2. 파일 이름을 제거하여 디렉토리 경로만 남깁니다.
	PathRemoveFileSpec(szPath);
	CString strFilePath;
	strFilePath.Format(_T("%s\\setting.txt"), szPath);
	//3. 세팅 파일 존재여부와 값 읽기
	CStdioFile file;
	if (file.Open(strFilePath, CFile::modeRead | CFile::typeText))
	{
		CString strLine;
		if (file.ReadString(strLine)) m_nDelay = _ttoi(strLine);
		if (file.ReadString(strLine)) m_nRate = _ttoi(strLine);
		if (file.ReadString(strLine)) m_nBounce = _ttoi(strLine);
		file.Close();
	}
	else // 값 없을 시 
	{
		FILTERKEYS fk;
		fk.cbSize = sizeof(FILTERKEYS);
		if (SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0))
		{
			// PC에 설정된 실제 값을 멤버 변수에 대입
			m_nDelay = fk.iDelayMSec;
			m_nRate = fk.iRepeatMSec;
			m_nBounce = fk.iBounceMSec;

		}
		else
		{
			// 만약 예외적으로 시스템 값을 읽지 못했을 때 오류 메시지와 프로그램 종료
			AfxMessageBox(_T("레지스트리 읽기 오류"));
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
			
		}
		// 4. 세팅 파일 생성
		if (file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			CString strData;

			strData.Format(_T("%d\n"), m_nDelay);
			file.WriteString(strData);

			strData.Format(_T("%d\n"), m_nRate);
			file.WriteString(strData);

			strData.Format(_T("%d\n"), m_nBounce);
			file.WriteString(strData);

			file.Close();
		}
		AfxMessageBox(_T("초기 세팅 파일을 생성하였습니다. \n *setting.txt파일 변경 금지*"));
		

	}
	m_nBackupDelay = m_nDelay;
	m_nBackupRate = m_nRate;
	m_nBackupBounce = m_nBounce;
	UpdateData(FALSE);


	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMapelHelperDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMapelHelperDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMapelHelperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMapelHelperDlg::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 세팅 버튼 
	if (!UpdateData(TRUE))
	{
		return; // 입력값에 문제가 있으면 중단
	}
	FILTERKEYS fk;
	fk.cbSize = sizeof(FILTERKEYS);
	fk.dwFlags = FKF_FILTERKEYSON | FKF_AVAILABLE | FKF_HOTKEYACTIVE | FKF_CONFIRMHOTKEY;
	fk.iDelayMSec = m_nDelay;
	fk.iRepeatMSec = m_nRate;
	fk.iBounceMSec = m_nBounce;
	fk.iWaitMSec = 0;

	if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
	{
		AfxMessageBox(_T("필터키 설정이 성공적으로 변경 및 적용되었습니다!"));

	}
	else
	{
		DWORD dwError = GetLastError();
		CString strErr;
		//AfxMessageBox(_T("윈도우 시스템에 설정을 반영하는데 실패했습니다. 권한을 확인하세요."));
		strErr.Format(_T("시스템 반영 실패 (에러 코드: %d)\n구조체 값 설정을 확인하세요."), dwError);
		AfxMessageBox(strErr);
	}
}

void CMapelHelperDlg::OnEnChangeEdit1()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// ENM_CHANGE가 있으면 마스크에 ORed를 플래그합니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CMapelHelperDlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//리셋 버튼 구현
	m_nDelay = m_nBackupDelay;
	m_nRate = m_nBackupRate;
	m_nBounce = m_nBackupBounce;

	// 2. 변수 값을 UI(텍스트 박스)에 강제로 갱신하여 보여줍니다.
	UpdateData(FALSE);

	// 3. 사용자에게 안전하게 초기화되었음을 알리는 안내창 
	FILTERKEYS fk;
	fk.cbSize = sizeof(FILTERKEYS);
	fk.dwFlags = 0;
	fk.iDelayMSec = 0;
	fk.iRepeatMSec = 0;
	fk.iBounceMSec = 0;
	fk.iWaitMSec = 0;
	if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
	{
		AfxMessageBox(_T("필터키 기능이 꺼졌으며, 컴퓨터가 일반 키보드 상태로 안전하게 복구되었습니다!"));
	}
	else
	{
		AfxMessageBox(_T("시스템 복구에 실패했습니다. 관리자 권한을 확인하세요."));
	}
}

void CMapelHelperDlg::OnEnChangeEdit3()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// ENM_CHANGE가 있으면 마스크에 ORed를 플래그합니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
