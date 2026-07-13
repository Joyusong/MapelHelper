
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
	, m_editAccept(0)
	, m_editRepeatDelay(0)
	, m_editRepeatRate(0),
	m_nBackupEditAccept(0),
	m_nBackupEditRepeatDelay(0),
	m_nBackupEditRepeatRate(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

inline void CMapelHelperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_editAccept);
	DDX_Text(pDX, IDC_EDIT2, m_editRepeatDelay);
	DDX_Text(pDX, IDC_EDIT3, m_editRepeatRate);
	DDX_Control(pDX, IDC_COMBO1, m_comboPreset);
}

BEGIN_MESSAGE_MAP(CMapelHelperDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMapelHelperDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CMapelHelperDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMapelHelperDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT3, &CMapelHelperDlg::OnEnChangeEdit3)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CMapelHelperDlg::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_EDIT2, &CMapelHelperDlg::OnEnChangeEdit2)
END_MESSAGE_MAP()


// CMapelHelperDlg 메시지 처리기

BOOL CMapelHelperDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 1. 현재 실행 파일의 경로를 가져옵니다.
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 2. 파일 이름을 제거하여 디렉토리 경로만 남깁니다.
	PathRemoveFileSpec(szPath);
	CString strFilePath;
	strFilePath.Format(_T("%s\\setting.csv"), szPath);

	// 3. 세팅 파일 존재여부와 값 읽기
	CStdioFile file;
	if (LoadPresetCSV(strFilePath))
	{
		if (!m_presets.empty())
		{
			// 첫 번째 프리셋 사용
			m_editAccept = m_presets[0].accept;
			m_editRepeatDelay = m_presets[0].repeatDelay;
			m_editRepeatRate = m_presets[0].repeatRate;
		}
	}
	else // 값 없을 시 (최초 실행 시 PC 순정 값 백업)
	{
		HKEY hKey;
		LPCTSTR regPath = _T("Control Panel\\Accessibility\\Keyboard Response");

		if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			TCHAR szAccept[64] = { 0 };
			TCHAR szDelay[64] = { 0 };
			TCHAR szRate[64] = { 0 };
			DWORD dwSize = sizeof(szAccept);

			// ① DelayBeforeAcceptance (Accept Delay) 읽기
			if (RegQueryValueEx(hKey, _T("DelayBeforeAcceptance"), NULL, NULL, (BYTE*)szAccept, &dwSize) == ERROR_SUCCESS)
				m_editAccept = _ttoi(szAccept);
			else
				m_editAccept = 0;

			// ② AutoRepeatDelay (Repeat Delay) 읽기
			dwSize = sizeof(szDelay);
			if (RegQueryValueEx(hKey, _T("AutoRepeatDelay"), NULL, NULL, (BYTE*)szDelay, &dwSize) == ERROR_SUCCESS)
				m_editRepeatDelay = _ttoi(szDelay);
			else
				m_editRepeatDelay = 0;

			// ③ AutoRepeatRate (Repeat Rate) 읽기
			dwSize = sizeof(szRate);
			if (RegQueryValueEx(hKey, _T("AutoRepeatRate"), NULL, NULL, (BYTE*)szRate, &dwSize) == ERROR_SUCCESS)
				m_editRepeatRate = _ttoi(szRate);
			else
				m_editRepeatRate = 0;

			RegCloseKey(hKey);
		}

		// 💡 [버그 예방 고정장치] 필터키가 꺼져있어 레지스트리 값이 0으로 스캔되었다면 윈도우 표준 기본값 세팅
		if (m_editAccept == 0) 	m_editAccept = 1000;
		if (m_editRepeatDelay == 0) m_editRepeatDelay = 500;
		if (m_editRepeatRate == 0)  m_editRepeatRate = 0;

		// 4. 최초로 보정 완료된 순정 값을 세팅 파일(setting.txt)로 복사 및 저장
		if (file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			wchar_t bom = 0xFEFF;
			file.Write(&bom, sizeof(bom));

			// 쉼표(,) 대신 탭(\t)을 사용합니다.
			file.WriteString(_T("Name\tAcceptDelay\tRepeatDelay\tRepeatRate\r\n"));

			CString str;
			str.Format(
				_T("기본값\t%d\t%d\t%d\r\n"), // 여기도 \t로 변경
				m_editAccept,
				m_editRepeatDelay,
				m_editRepeatRate);

			file.WriteString(str);
			file.Flush();
			file.Close();
			LoadPresetCSV(strFilePath); // 새로 생성된 파일을 다시 로드하여 콤보박스에 반영
		}
		
		AfxMessageBox(_T("현재 PC 설정을 기반으로 setting.csv를 생성했습니다."));
	}

	// 5. 백업 변수에 최종 안전 수치 기록
	m_nBackupEditAccept = m_editAccept;
	m_nBackupEditRepeatDelay = m_editRepeatDelay;
	m_nBackupEditRepeatRate = m_editRepeatRate;

	// UI 에디트 텍스트 상자에 데이터 대입
	UpdateData(FALSE);


	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

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
	if (!UpdateData(TRUE))
	{
		return; // 입력값 예외처리
	}

	// 1. 먼저 레지스트리에 세부 문자열 값을 확실하게 박아 넣습니다.
	HKEY hKey;
	LPCTSTR regPath = _T("Control Panel\\Accessibility\\Keyboard Response");

	if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
	{
		CString strAccept, strDelay, strRate;
		strAccept.Format(_T("%d"), m_editAccept);
		strDelay.Format(_T("%d"), m_editRepeatDelay);
		strRate.Format(_T("%d"), m_editRepeatRate);
		CString strFlags = _T("27"); // 온(On) 플래그

		RegSetValueEx(hKey, _T("DelayBeforeAcceptance"), 0, REG_SZ, (BYTE*)(LPCTSTR)strAccept, (strAccept.GetLength() + 1) * sizeof(TCHAR));
		RegSetValueEx(hKey, _T("AutoRepeatDelay"), 0, REG_SZ, (BYTE*)(LPCTSTR)strDelay, (strDelay.GetLength() + 1) * sizeof(TCHAR));
		RegSetValueEx(hKey, _T("AutoRepeatRate"), 0, REG_SZ, (BYTE*)(LPCTSTR)strRate, (strRate.GetLength() + 1) * sizeof(TCHAR));
		RegSetValueEx(hKey, _T("Flags"), 0, REG_SZ, (BYTE*)(LPCTSTR)strFlags, (strFlags.GetLength() + 1) * sizeof(TCHAR));

		RegCloseKey(hKey);
	}

	// 2. 🔥 [핵심] 윈도우 커널에 FILTERKEYS 구조체를 직접 넘겨서 즉시 드라이버를 리로드시킵니다.
	FILTERKEYS fk;
	fk.cbSize = sizeof(FILTERKEYS);
	// 필터키 On + 핫키 활성화 플래그 설정
	fk.dwFlags = FKF_FILTERKEYSON | FKF_AVAILABLE | FKF_HOTKEYACTIVE;
	fk.iDelayMSec = m_editAccept;
	fk.iRepeatMSec = m_editRepeatDelay;
	fk.iBounceMSec = m_editRepeatRate;
	fk.iWaitMSec = 0;

	// 구조체를 실어서 전달해야 시스템 속도가 즉시 변합니다.
	if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
	{
		AfxMessageBox(_T("키보드 반응 속도 오버클럭이 실시간으로 즉시 적용되었습니다!"));
	}
	else
	{
		DWORD dwError = GetLastError();
		CString strErr;
		strErr.Format(_T("시스템 드라이버 반영 실패 (에러 코드: %d)"), dwError);
		AfxMessageBox(strErr);
	}
}


void CMapelHelperDlg::OnBnClickedButton2()
{
	// 1. UI 데이터를 초기 순정 값으로 원상복구
	m_editAccept = m_nBackupEditAccept;
	m_editRepeatDelay = m_nBackupEditRepeatDelay;
	m_editRepeatRate = m_nBackupEditRepeatRate;
	UpdateData(FALSE);

	// 2. 레지스트리 플래그를 오프 상태("126")로 되돌림
	HKEY hKey;
	LPCTSTR regPath = _T("Control Panel\\Accessibility\\Keyboard Response");

	if (RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
	{
		CString strFlags = _T("126");
		RegSetValueEx(hKey, _T("Flags"), 0, REG_SZ, (BYTE*)(LPCTSTR)strFlags, (strFlags.GetLength() + 1) * sizeof(TCHAR));
		RegCloseKey(hKey);
	}

	// 3. 🔥 [핵심] 윈도우 커널에 필터키 기능을 완전히 끄라(dwFlags = 0)고 명시적으로 구조체를 전달합니다.
	FILTERKEYS fk;
	fk.cbSize = sizeof(FILTERKEYS);
	fk.dwFlags = 0; // 마스터 오프(Off)
	fk.iDelayMSec = 0;
	fk.iRepeatMSec = 0;
	fk.iBounceMSec = 0;
	fk.iWaitMSec = 0;

	if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
	{
		AfxMessageBox(_T("필터키 기능이 정상 종료되었으며, 순정 기본 키보드 상태로 복구되었습니다."));
	}
	else
	{
		AfxMessageBox(_T("시스템 원상복구 명령 전달 실패."));
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

void CMapelHelperDlg::OnCbnSelchangeCombo1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = m_comboPreset.GetCurSel();
	if (nSel == CB_ERR || nSel >= (int)m_presets.size()) {
		return;
	}
	FilterPreset selectedData = m_presets[nSel];
	m_editAccept = selectedData.accept;
	m_editRepeatDelay = selectedData.repeatDelay;
	m_editRepeatRate = selectedData.repeatRate;
	UpdateData(FALSE);

}
bool CMapelHelperDlg::LoadPresetCSV(const CString& path)
{
	CStdioFile file;

	if (!file.Open(path, CFile::modeRead | CFile::typeBinary))
		return false;

	CString line;
	m_comboPreset.ResetContent();
	// 첫 줄(Header) 건너뛰기
	file.ReadString(line);

	m_presets.clear();

	while (file.ReadString(line))
	{
		FilterPreset preset;

		int cur = 0;
		CString token;

		token = line.Tokenize(_T("\t"), cur);
		preset.name = token;

		token = line.Tokenize(_T("\t"), cur);
		preset.accept = _ttoi(token);
		 
		token = line.Tokenize(_T("\t"), cur);
		preset.repeatDelay = _ttoi(token);

		token = line.Tokenize(_T("\t"), cur);
		preset.repeatRate = _ttoi(token);

		m_presets.push_back(preset);
		m_comboPreset.AddString(preset.name);
	}
	file.Close();
	if (!m_presets.empty()) {
		m_comboPreset.SetCurSel(0);
		OnCbnSelchangeCombo1();
	}

	return true;
}
void CMapelHelperDlg::OnEnChangeEdit2()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// ENM_CHANGE가 있으면 마스크에 ORed를 플래그합니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CMapelHelperDlg::OnEnChangeEdit1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}