
// DingToolDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "DingTool.h"
#include "DingToolDlg.h"
#include "afxdialogex.h"

#include <Windows.h>
#include <Tlhelp32.h>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDingToolDlg 对话框



CDingToolDlg::CDingToolDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DINGTOOL_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDingToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, EDT_PATH, m_edtPath);
}

BEGIN_MESSAGE_MAP(CDingToolDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(BTN_OPENPATH, &CDingToolDlg::OnBnClickedOpenpath)
    ON_BN_CLICKED(BTN_AUTOSEARCH, &CDingToolDlg::OnBnClickedAutosearch)
    ON_BN_CLICKED(BTN_PATCH, &CDingToolDlg::OnBnClickedPatch)
    ON_BN_CLICKED(BTN_UNPATCH, &CDingToolDlg::OnBnClickedUnpatch)
END_MESSAGE_MAP()


// CDingToolDlg 消息处理程序

BOOL CDingToolDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    AfxMessageBox("本程序有且仅仅支持版本：6.0.26-Release.9039987");
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDingToolDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDingToolDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



void CDingToolDlg::OnBnClickedOpenpath()
{
    CFileDialog dlg(TRUE,
        NULL,
        NULL,
        OFN_OVERWRITEPROMPT,
        _T("钉钉主程序(DingTalk.exe)|DingTalk.exe|"),//指定要打开的文件类型
        NULL);
    if (dlg.DoModal() == IDOK)
    {
        CString strPath;
        strPath = dlg.GetPathName();
        int nIdx = strPath.ReverseFind('\\');
        strPath = strPath.Mid(0, nIdx);
        strPath.Format("%s%s", strPath, "\\plugins\\tblive\\bin\\32bit");
        m_edtPath.SetWindowText(strPath);
    }
}

DWORD GetProcessPath(char* szProcessName, char* szOutputBuff)
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapShot == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox("CreateToolhelp32Snapshot Error");
        return -1;
    }

    BOOL bRet = Process32First(hSnapShot, &pe32);
    while (bRet)
    {
        if (_strnicmp(szProcessName, pe32.szExeFile, strlen(szProcessName)) == 0)
        {
            hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            if (hSnapShot == INVALID_HANDLE_VALUE)
            {
                return GetLastError();
            }
            MODULEENTRY32 me32;
            me32.dwSize = sizeof(MODULEENTRY32);

            if (!Module32First(hSnapShot, &me32))
            {
                return GetLastError();
            }
            strcpy_s(szOutputBuff, MAX_PATH, me32.szExePath);
            return 0;
        }
        bRet = ::Process32Next(hSnapShot, &pe32);
    }
}

void CDingToolDlg::OnBnClickedAutosearch()
{
    char szPath[MAX_PATH]{};
    if (GetProcessPath("DingTalk.exe", szPath))
    {
        AfxMessageBox("获取失败");
    }
    CString strPath = szPath;
    int nIdx = strPath.ReverseFind('\\');
    strPath = strPath.Mid(0, nIdx);
    strPath.Format("%s%s", strPath, "\\plugins\\tblive\\bin\\32bit");
    m_edtPath.SetWindowText(strPath);
}

// 提取资源文件
BOOL ExtraceResourceFile(uint16_t resource_id, const TCHAR* output_file, TCHAR* type)
{
    HANDLE hFile = NULL;
    HANDLE hFilemap = NULL;
    BOOL bRet = FALSE;
    do
    {
        HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
        HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resource_id), type);
        if (hResource == nullptr)
        {
            break;
        }
        HGLOBAL hFileResource = LoadResource(nullptr, hResource);
        if (hFileResource == nullptr)
        {
            break;
        }
        LPVOID lpFile = LockResource(hFileResource);
        if (lpFile == nullptr)
        {
            break;
        }
        uint32_t dwSize = SizeofResource(nullptr, hResource);
        if (dwSize <= 0)
        {
            break;
        }

        hFile = CreateFile((LPCSTR)output_file, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            break;
        }
        hFilemap = CreateFileMapping(hFile, nullptr, PAGE_READWRITE, 0, dwSize, nullptr);
        if (hFilemap == NULL)
        {
            break;
        }
        LPVOID lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
        if (lpBaseAddress == NULL)
        {
            break;
        }
        CopyMemory(lpBaseAddress, lpFile, dwSize);
        UnmapViewOfFile(lpBaseAddress);
        bRet = TRUE;
    } while (0);

    if (hFilemap != NULL)
    {
        CloseHandle(hFilemap);
    }
    if (hFile != INVALID_HANDLE_VALUE && hFile != NULL)
    {
        CloseHandle(hFile);
    }
    return bRet;
}

void CDingToolDlg::OnBnClickedPatch()
{
    CString strAir2DllPath;
    CString strAir2OrgDllPath;
    m_edtPath.GetWindowText(strAir2DllPath);
    strAir2OrgDllPath.Format("%s%s", strAir2DllPath, "\\air2Org.dll");
    strAir2DllPath.Format("%s%s", strAir2DllPath, "\\air2.dll");
    if (_access(strAir2DllPath, 0) == 0 && _access(strAir2OrgDllPath, 0) == 0)
    {
        AfxMessageBox("该程序以打过补丁！");
        return;
    }
    // 修改原文件名称
    if (rename(strAir2DllPath, strAir2OrgDllPath) != 0)
    {
        AfxMessageBox("补丁失败");
        return;
    }

    // 写出补丁文件
    if (!ExtraceResourceFile(IDR_AIR21, strAir2DllPath.GetString(), "AIR2"))
    {
        AfxMessageBox("补丁失败");
        return;
    }
    AfxMessageBox("补丁成功");
}


void CDingToolDlg::OnBnClickedUnpatch()
{
    CString strAir2DllPath;
    CString strAir2OrgDllPath;
    m_edtPath.GetWindowText(strAir2DllPath);
    strAir2OrgDllPath.Format("%s%s", strAir2DllPath, "\\air2Org.dll");
    strAir2DllPath.Format("%s%s", strAir2DllPath, "\\air2.dll");
    if (_access(strAir2OrgDllPath, 0) != 0)
    {
        AfxMessageBox("该程序未打过补丁！");
        return;
    }
    // 删除补丁文件
    if (remove(strAir2DllPath) != 0)
    {
        AfxMessageBox("删除失败，文件可能被占用");
        return;
    }
    // 修改为原文件名称
    if (rename(strAir2OrgDllPath, strAir2DllPath) != 0)
    {
        AfxMessageBox("删除补丁失败");
        return;
    }
    AfxMessageBox("删除补丁成功");
}
