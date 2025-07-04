// wdf_gui2Dlg.cpp : ʵļ�
//

#include "stdafx.h"
#include "wdf_gui2.h"
#include "wdf_gui2Dlg.h"
#include "afxdialogex.h"
#include <afxdlgs.h>
#include "wdf_unpacker.h"
#include <fstream>
#include <locale>
#include <set>
#include <vector>
#include <algorithm>
#include <ctime>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 声明string_id
extern DWORD string_id(const char* str);

// Ӧó򡰹ڡ˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cwdf_gui2Dlg �Ի���




Cwdf_gui2Dlg::Cwdf_gui2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cwdf_gui2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cwdf_gui2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_WDF, m_editWdf);
	DDX_Control(pDX, IDC_EDIT_LST, m_editLst);
	DDX_Control(pDX, IDC_EDIT_LOG_WDF, m_editLogWdf);
	DDX_Control(pDX, IDC_EDIT_LOG_LST, m_editLogLst);
	DDX_Control(pDX, IDC_EDIT_LOG_B, m_editLogB);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
}

BEGIN_MESSAGE_MAP(Cwdf_gui2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_LOG, &Cwdf_gui2Dlg::OnTcnSelchangeTabLog)
	ON_BN_CLICKED(IDC_BTN_BROWSE_WDF, &Cwdf_gui2Dlg::OnBnClickedBrowseWdf)
	ON_BN_CLICKED(IDC_BTN_BROWSE_LST, &Cwdf_gui2Dlg::OnBnClickedBrowseLst)
	ON_BN_CLICKED(IDC_BTN_UNPACK, &Cwdf_gui2Dlg::OnBnClickedUnpack)
	ON_BN_CLICKED(IDC_BTN_EXT1, &Cwdf_gui2Dlg::OnBnClickedDictCollect)
	ON_BN_CLICKED(IDC_BTN_EXT2, &Cwdf_gui2Dlg::OnBnClickedDictParse)
END_MESSAGE_MAP()


// Cwdf_gui2Dlg Ϣ

BOOL Cwdf_gui2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ...˵ӵϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void Cwdf_gui2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void Cwdf_gui2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR Cwdf_gui2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cwdf_gui2Dlg::OnTcnSelchangeTabLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	*pResult = 0;
}

void Cwdf_gui2Dlg::OnBnClickedBrowseWdf()
{
	CFileDialog dlg(TRUE, _T("wdf"), NULL, OFN_FILEMUSTEXIST, _T("WDF file (*.wdf)|*.wdf|All files (*.*)|*.*||"));
	if (dlg.DoModal() == IDOK)
	{
		m_editWdf.SetWindowText(dlg.GetPathName());
	}
}

void Cwdf_gui2Dlg::OnBnClickedBrowseLst()
{
	CFileDialog dlg(TRUE, _T("lst"), NULL, OFN_FILEMUSTEXIST, _T("LST file (*.lst)|*.lst|All files (*.*)|*.*||"));
	if (dlg.DoModal() == IDOK)
	{
		m_editLst.SetWindowText(dlg.GetPathName());
	}
}

void Cwdf_gui2Dlg::OnBnClickedUnpack()
{
	CString wdfPath, lstPath;
	m_editWdf.GetWindowText(wdfPath);
	m_editLst.GetWindowText(lstPath);
	m_editLogWdf.SetWindowText(_T(""));
	m_editLogLst.SetWindowText(_T(""));
	m_editLogB.SetWindowText(_T(""));
	m_progress.SetRange(0, 100);
	m_progress.SetPos(0);

	// Open WDF
	if (!m_unpacker.OpenWdf(wdfPath.GetString())) {
		m_editLogB.SetWindowText(_T("[Error] Failed to open WDF file!\r\n"));
		return;
	}
	CString wdfInfo;
	wdfInfo.Format(_T("[WDF] Index loaded, file count: %u"), (unsigned)m_unpacker.GetFileCount());
	m_editLogB.SetWindowText(wdfInfo + _T("\r\n"));

	// Show WDF index in m_editLogWdf
	CString wdfIndexLog;
	const std::vector<WDataFileIndex>& index = m_unpacker.GetIndex();
	for (size_t i = 0; i < index.size(); ++i) {
		CString line;
		line.Format(_T("hash: 0x%08X size: %u\r\n"), index[i].uid, index[i].size);
		wdfIndexLog += line;
	}
	m_editLogWdf.SetWindowText(wdfIndexLog);

	// Show LST file content in m_editLogLst (用C方式读取)
	FILE* f = _wfopen(lstPath.GetString(), L"rb");
	CString lstLog;
	if (f) {
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);
		std::vector<char> buffer(size + 1);
		fread(buffer.data(), 1, size, f);
		fclose(f);
		buffer[size] = 0;
		int i = 0;
		char* p = buffer.data();
		char* line = nullptr;
		bool in_line = false;
		for (i = 0; i < size; i++) {
			if (in_line) {
				if (p[i] <= 32 && p[i] >= 0) {
					p[i] = 0;
					lstLog += CString(p[i] ? line : line);
					lstLog += _T("\r\n");
					in_line = false;
				}
			} else {
				if (p[i] > 32 || p[i] < 0) {
					line = &p[i];
					in_line = true;
				}
			}
		}
	} else {
		lstLog = _T("[Error] Failed to open LST file!\r\n");
	}
	m_editLogLst.SetWindowText(lstLog);

	// Auto output dir
	CString wdfDir = wdfPath.Left(wdfPath.ReverseFind(_T('\\')));
	CString wdfName = wdfPath.Mid(wdfPath.ReverseFind(_T('\\')) + 1);
	CString wdfBase = wdfName.Left(wdfName.ReverseFind(_T('.')));
	CString outDir = wdfDir + _T("\\") + wdfBase;
	SHCreateDirectoryEx(NULL, outDir, NULL);

	std::vector<std::wstring> logs;
	int success = 0, fail = 0;
	if (!m_unpacker.ExtractByLst(lstPath.GetString(), outDir.GetString(), logs, success, fail)) {
		m_editLogB.SetWindowText(_T("[Error] Failed to read LST file!\r\n"));
		return;
	}
	// Log output and progress bar
	int total = (int)logs.size();
	for (int i = 0; i < total; ++i) {
		CString oldLog;
		m_editLogB.GetWindowText(oldLog);
		oldLog += CString(logs[i].c_str());
		oldLog += _T("\r\n");
		m_editLogB.SetWindowText(oldLog);
		m_progress.SetPos((i + 1) * 100 / total);
		m_editLogB.LineScroll(m_editLogB.GetLineCount());
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CString summary;
	summary.Format(_T("[Done] Success: %d, Failed: %d\r\n"), success, fail);
	CString oldLog;
	m_editLogB.GetWindowText(oldLog);
	oldLog += summary;
	m_editLogB.SetWindowText(oldLog);
	m_progress.SetPos(100);
}

void Cwdf_gui2Dlg::OnBnClickedDictCollect()
{
	CString lstPath;
	m_editLst.GetWindowText(lstPath);
	if (lstPath.IsEmpty()) {
		m_editLogB.SetWindowText(_T("请先选择LST文件！"));
		return;
	}
	std::wifstream fin(lstPath);
	if (!fin) {
		m_editLogB.SetWindowText(_T("LST文件无法打开！"));
		return;
	}
	std::set<std::wstring> newFrags;
	std::wstring line;
	int count = 0;
	while (std::getline(fin, line)) {
		if (!line.empty()) {
			m_dictMgr.AddPathFragments(line);
			++count;
		}
	}
	fin.close();
	m_dictMgr.Save(L"dictionary.txt");
	CString msg;
	msg.Format(_T("字典收录完成，收录路径数：%d，片段总数：%d"), count, (int)m_dictMgr.GetFragments().size());
	m_editLogB.SetWindowText(msg);
}

void Cwdf_gui2Dlg::OnBnClickedDictParse()
{
	if (!m_dictMgr.Load(L"dictionary.txt")) {
		m_editLogB.SetWindowText(_T("dictionary.txt 加载失败！"));
		return;
	}
	int depth = m_dictMgr.GetDepth();
	int tries = m_dictMgr.GetTries();
	const std::vector<std::wstring>& exts = m_dictMgr.GetExts();
	const std::set<std::wstring>& frags = m_dictMgr.GetFragments();
	const std::vector<std::wstring>& formats = m_dictMgr.GetSampleFormats();
	if (frags.empty()) {
		m_editLogB.SetWindowText(_T("字典片段为空，请先收录！"));
		return;
	}
	// 读取LST，获取已知哈希
	CString lstPath;
	m_editLst.GetWindowText(lstPath);
	std::set<unsigned int> knownHashes;
	std::wifstream fin(lstPath);
	std::wstring line;
	while (std::getline(fin, line)) {
		if (!line.empty()) {
			unsigned int h = string_id(CT2A(line.c_str()));
			knownHashes.insert(h);
		}
	}
	fin.close();
	// 获取WDF所有哈希
	CString wdfPath;
	m_editWdf.GetWindowText(wdfPath);
	std::wstring wdfPathW((LPCTSTR)wdfPath);
	if (!m_unpacker.OpenWdf(wdfPathW)) {
		m_editLogB.SetWindowText(_T("WDF文件无法打开！"));
		return;
	}
	const std::vector<WDataFileIndex>& index = m_unpacker.GetIndex();
	std::vector<unsigned int> unknownHashes;
	for (size_t i=0; i<index.size(); ++i) {
		if (knownHashes.find(index[i].uid) == knownHashes.end())
			unknownHashes.push_back(index[i].uid);
	}
	std::vector<std::wstring> fragVec(frags.begin(), frags.end());
	std::vector<CString> logLines;
	srand((unsigned)time(NULL));
	int totalTries = 0, totalSuccess = 0, totalFail = 0;
	for (size_t i=0; i<unknownHashes.size(); ++i) {
		unsigned int hash = unknownHashes[i];
		bool hit = false;
		int thisTries = 0;
		// 支持示例格式爆破
		for (int t=0; t<tries; ++t) {
			std::wstring path;
			if (!formats.empty()) {
				// 随机选一个示例格式
				const std::wstring& fmt = formats[rand()%formats.size()];
				size_t pos = 0, fragIdx = 0;
				while (pos < fmt.size()) {
					size_t next = fmt.find(L"xxx", pos);
					if (next == std::wstring::npos) {
						path += fmt.substr(pos);
						break;
					}
					path += fmt.substr(pos, next-pos);
					// 判断是否是最后一个片段（即后缀）
					if (next+3 == fmt.size() && fmt.size()>=4 && fmt.substr(fmt.size()-4)==L".xxx") {
						// 随机后缀
						path += exts[rand()%exts.size()];
					} else {
						path += fragVec[rand()%fragVec.size()];
					}
					pos = next + 3;
					fragIdx++;
				}
			} else {
				// 兼容无示例格式，随机拼接
				for (int d=0; d<depth; ++d) {
					if (d>0) path += L"/";
					path += fragVec[rand()%fragVec.size()];
				}
				path += L".";
				path += exts[rand()%exts.size()];
			}
			DWORD hval = string_id(CT2A(path.c_str()));
			CString tryMsg;
			tryMsg.Format(_T("[Try] %ls => 0x%08X"), path.c_str(), hval);
			if (hval == hash) {
				std::wstring outPathW = path;
				std::wstring log;
				m_unpacker.ExtractFile(std::string(CT2A(path.c_str())), outPathW, log);
				m_dictMgr.AddPathFragments(path);
				tryMsg += _T(" [Match]");
				logLines.push_back(tryMsg);
				CString matchMsg;
				matchMsg.Format(_T("[Match] %ls"), path.c_str());
				logLines.push_back(matchMsg);
				hit = true;
				totalSuccess++;
				thisTries = t+1;
				break;
			} else {
				tryMsg += _T(" [Miss]");
				logLines.push_back(tryMsg);
			}
			thisTries = t+1;
		}
		totalTries += thisTries;
		if (!hit) {
			CString missMsg;
			missMsg.Format(_T("[Miss ] 0x%08X"), hash);
			logLines.push_back(missMsg);
			totalFail++;
		}
	}
	m_dictMgr.Save(L"dictionary.txt");
	CString allLog;
	for (size_t i=0; i<logLines.size(); ++i) {
		allLog += logLines[i];
		allLog += _T("\r\n");
	}
	CString statMsg;
	statMsg.Format(_T("总尝试次数：%d\r\n匹配成功：%d\r\n匹配失败：%d\r\n"), totalTries, totalSuccess, totalFail);
	allLog += statMsg;
	m_editLogB.SetWindowText(allLog);
}
