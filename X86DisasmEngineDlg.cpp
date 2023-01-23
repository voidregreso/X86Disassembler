#include "stdafx.h"
#include "X86DisasmEngine.h"
#include "X86DisasmEngineDlg.h"
#include "afxdialogex.h"


#include "inteldef.h"
#include "inteldis.h"
//////////////////////////////////////////////////////////////////////////
#define FILEBUGSIZE 1000
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CX86DisasmEngineDlg::CX86DisasmEngineDlg(CWnd *pParent /*=nullptr*/)
    : CDialogEx(IDD_X86DISASMENGINE_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CX86DisasmEngineDlg::DoDataExchange(CDataExchange *pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CX86DisasmEngineDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_CHFILE, &CX86DisasmEngineDlg::OnBnClickedChfile)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CX86DisasmEngineDlg Manejadores de mensajes

int m_nLineCount;

BOOL CX86DisasmEngineDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    
    TEXTMETRIC tm;
    CRect rect;
    GetDlgItem(IDC_EDIT2)->GetClientRect(&rect);
    CDC *pdc = GetDlgItem(IDC_EDIT2)->GetDC();
    ::GetTextMetrics(pdc->m_hDC, &tm);
    GetDlgItem(IDC_EDIT2)->ReleaseDC(pdc);
    m_nLineCount = (int)(rect.bottom / (tm.tmHeight - 1.5));
    return TRUE;
}

void CX86DisasmEngineDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    CDialogEx::OnSysCommand(nID, lParam);
}

void CX86DisasmEngineDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this); 
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

HCURSOR CX86DisasmEngineDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

CString filepath;
int g_nAddrOfEP = 0;

PBYTE GetFileOepBuf(CString pFileName) {
    DWORD dwReadByte = 0;
    PBYTE pFileBuf = NULL;
    BOOL bRet = FALSE;
    STARTUPINFO tagStartupInfo = { 0 };
    PROCESS_INFORMATION tagProcessInfoRmation = { 0 };
    DEBUG_EVENT tagDebugEvent = { 0 };
    pFileBuf = (PBYTE)malloc(sizeof(BYTE) * FILEBUGSIZE);
    tagStartupInfo.cb = sizeof(STARTUPINFO);
    bRet = CreateProcess(pFileName, NULL, NULL, NULL, FALSE,
                         DEBUG_ONLY_THIS_PROCESS, NULL, NULL,
                         &tagStartupInfo, &tagProcessInfoRmation);
    if (bRet == FALSE) {
        goto EXIT_FUN;
    }
    bRet = WaitForDebugEvent(&tagDebugEvent, INFINITE);
    if (bRet == 0) {
        goto EXIT_FUN;
    }
    g_nAddrOfEP = (int)tagDebugEvent.u.CreateProcessInfo.lpStartAddress;
    if (ReadProcessMemory(tagProcessInfoRmation.hProcess,
                          (LPVOID)g_nAddrOfEP,
                          pFileBuf, FILEBUGSIZE, (SIZE_T*)&dwReadByte) && dwReadByte == FILEBUGSIZE) {
        return pFileBuf;
    }
EXIT_FUN:
    if (tagProcessInfoRmation.hThread != NULL) {
        CloseHandle(tagProcessInfoRmation.hThread);
    }
    if (tagProcessInfoRmation.hProcess != NULL) {
        CloseHandle(tagProcessInfoRmation.hProcess);
    }
    return NULL;
}

bool fileExist(CString fp) {
    CFile file;
    CFileException ex;
    if (file.Open(fp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary, &ex) == 0) return false;
    return true;
}

wchar_t *ctow(char *sText) {
    DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, sText, -1, NULL, 0);
    wchar_t *pwText = NULL;
    pwText = new wchar_t[dwNum];
    if (!pwText) {
        delete[]pwText;
        pwText = NULL;
    }
    unsigned nLen = MultiByteToWideChar(CP_ACP, 0, sText, -1, pwText, dwNum + 10);
    if (nLen >= 0) {
        pwText[nLen] = 0;
    }
    return pwText;
}


CString doDisassemble(bool UiShow) {
    PBYTE pFileBuf = NULL;
    INSTRUCTION_INFORMATION tagInstInfo = { 0 };
    DIS_CPU tagDispCpu = { 0 };
    char szAsmString[0x80] = { 0 };
    BYTE szOpCode[0x10] = { 0 };
    int nCodeLen = 0;
    CString result = L"";
    
    if (!fileExist(filepath)) AfxMessageBox(L"File does not exist!");
    else {
        pFileBuf = GetFileOepBuf(filepath);
        if (pFileBuf == NULL) return L"";
        while (TRUE) {
            memset(szAsmString, 0, sizeof(szAsmString));
            memcpy(szOpCode, pFileBuf, sizeof(szOpCode));
            tagInstInfo.pAsmString = szAsmString;
            if (OnDisassembly(&tagInstInfo, nCodeLen, szOpCode,
                              g_nAddrOfEP, MODE_32BIT, &tagDispCpu) == TRUE) {
                char f[2048];
                sprintf_s(f, "%08X  %-30s   Length:%d ", g_nAddrOfEP, szAsmString, nCodeLen);
                result += CString(ctow(f));
                if(UiShow) result += L"\r\n";
                else result += L"\n";
            } else break;
            pFileBuf = pFileBuf + nCodeLen;
            g_nAddrOfEP += nCodeLen;
            nCodeLen = 0;
        }
    }
    return result;
}

void CX86DisasmEngineDlg::ZGDisassemble() {
    int nButtonPressed = 0;
    TaskDialog(AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
               L"What's your choice?",
               L"Disassemble to text box or to file?",
               L"Click yes to disassemble to text box, or no to disassemble to file",
               TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON,
               TD_INFORMATION_ICON,
               &nButtonPressed);
    if (IDOK == nButtonPressed) {
        CString ed = doDisassemble(true);
        if (ed == L"") {
            TaskDialog(AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
                       L"Error",
                       L"Cannot disassemble this file due to internal error!",
                       NULL,
                       TDCBF_OK_BUTTON,
                       TD_ERROR_ICON,
                       NULL);
            return;
        }
        SetDlgItemText(IDC_EDIT2, ed);
        int nLine = ((CEdit *)GetDlgItem(IDC_EDIT2))->GetLineCount();
        if (nLine > m_nLineCount) GetDlgItem(IDC_EDIT2)->ShowScrollBar(SB_VERT, TRUE);
        else GetDlgItem(IDC_EDIT2)->ShowScrollBar(SB_VERT, FALSE);
    } else if (IDCANCEL == nButtonPressed) {
        BOOL isOpen = FALSE;
        CString filter = L"Text Files(*.txt)|*.txt|All Files(*.*)|*.*||";
        CFileDialog openFileDlg(isOpen, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
        INT_PTR result = openFileDlg.DoModal();
        if (result == IDOK) {
            CString sfp = openFileDlg.GetPathName();
            switch (openFileDlg.m_ofn.nFilterIndex) {
            case 1:
                sfp += ".txt";
                break;
            default:
                break;
            }
            CFile file;
            file.Open(sfp, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL);
            CString res = doDisassemble(false);
            if (res == L"") {
                TaskDialog(AfxGetMainWnd()->m_hWnd, AfxGetInstanceHandle(),
                           L"Error",
                           L"Cannot disassemble this file due to internal error!",
                           NULL,
                           TDCBF_OK_BUTTON,
                           TD_ERROR_ICON,
                           NULL);
                return;
            }
            file.Write(res, res.GetLength());
            file.Close();
        }
    }
}

void CX86DisasmEngineDlg::OnBnClickedChfile() {
    BOOL isOpen = TRUE;
    CString filter = L"Executables(*.exe)|*.exe|All Files(*.*)|*.*||";
    CFileDialog openFileDlg(isOpen, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
    INT_PTR result = openFileDlg.DoModal();
    if (result == IDOK) {
        filepath = openFileDlg.GetPathName();
        CEdit *pBoxOne = (CEdit *)GetDlgItem(IDC_EDIT1);
        pBoxOne->SetWindowText(filepath);
        ZGDisassemble();
    }
}

void CX86DisasmEngineDlg::OnDropFiles(HDROP hDropInfo) {
    int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);
    if (DropCount > 1) AfxMessageBox(L"You can only place one file in one time!");
    else {
        WCHAR wcStr[MAX_PATH];
        DragQueryFile(hDropInfo, 0, wcStr, MAX_PATH);
        filepath = CString(wcStr);
        CEdit *pBoxOne = (CEdit *)GetDlgItem(IDC_EDIT1);
        pBoxOne->SetWindowText(filepath);
    }
    DragFinish(hDropInfo);
    ZGDisassemble();
    CDialogEx::OnDropFiles(hDropInfo);
}
