#define STRICT
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <tchar.h>
#include "arg.inl"

#ifdef UNICODE
#define tWinMain wWinMain
#else
#define tWinMain WinMain
#endif

#define IDD_INPUTBOX 1001
#define IDC_PROMPT 101
#define IDC_EDIT1 102

BOOL SetClientWindowRect(_In_ HWND hWnd, _In_ const RECT* prc)
{
    return SetWindowPos(hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER);
}

BOOL GetClientWindowRect(_In_ HWND hWnd, _In_ RECT* prc)
{
    HWND hParent = GetParent(hWnd);
    BOOL b = GetWindowRect(hWnd, prc);
    ::ScreenToClient(hParent, (POINT*) &prc->left);
    ::ScreenToClient(hParent, (POINT*) &prc->right);
    return b;
}

struct InputBoxParam
{
    LPCTSTR title;
    LPCTSTR prompt;
    LPCTSTR default;
    TCHAR result[2048];
};

struct MoveChildData
{
    int dy;
};

BOOL CALLBACK MoveChild(
  _In_ HWND   hWnd,
  _In_ LPARAM lParam
)
{
    const MoveChildData* mcd = (MoveChildData*) lParam;
    const int id = GetWindowLong(hWnd, GWL_ID);
    switch (id)
    {
    case IDC_EDIT1:
    case IDOK:
    case IDCANCEL:
        {
            RECT rc;
            GetClientWindowRect(hWnd, &rc);
            rc.top += mcd->dy;
            rc.bottom += mcd->dy;
            SetClientWindowRect(hWnd, &rc);
        }
        break;
    }
    return TRUE;
}

void CalcRect(HWND hWnd, RECT* prc)
{
    TCHAR text[1024];
    HDC hDC = GetWindowDC(hWnd);
    HFONT hFont = GetWindowFont(hWnd);
    HFONT hOldFont = SelectFont(hDC, hFont);
    GetWindowText(hWnd, text, ARRAYSIZE(text));
    DrawText(hDC, text, -1, prc, DT_CALCRECT | DT_WORDBREAK);
    SelectFont(hDC, hOldFont);
    ReleaseDC(hWnd, hDC);
}

void FixSize(HWND hDlg)
{
    HWND hPrompt = GetDlgItem(hDlg, IDC_PROMPT);
    RECT rc;
    GetClientWindowRect(hPrompt, &rc);
    RECT nrc = rc;
    CalcRect(hPrompt, &nrc);
    SetClientWindowRect(hPrompt, &nrc);

    MoveChildData mcd = {};
    mcd.dy = (nrc.bottom - nrc.top) - (rc.bottom - rc.top);
    EnumChildWindows(hDlg, MoveChild, LPARAM(&mcd));

    GetClientWindowRect(hDlg, &rc);
    rc.bottom += mcd.dy;
    SetClientWindowRect(hDlg, &rc);
}

INT_PTR InputBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    InputBoxParam* p = reinterpret_cast<InputBoxParam*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        p = (InputBoxParam*) lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, LONG_PTR(p));
        if (p->title) SetWindowText(hDlg, p->title);
        if (p->prompt) SetDlgItemText(hDlg, IDC_PROMPT, p->prompt);
        if (p->default) SetDlgItemText(hDlg, IDC_EDIT1, p->default);
        FixSize(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT1, p->result, ARRAYSIZE(p->result));
            EndDialog(hDlg, EXIT_SUCCESS);
            break;

        case IDCANCEL:
            EndDialog(hDlg, EXIT_FAILURE);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        EndDialog(hDlg, EXIT_FAILURE + 1);
        return TRUE;

    default:
        return FALSE;
    }
}

int WINAPI tWinMain(/*[in]*/ HINSTANCE hInstance, /*[in, optional]*/ HINSTANCE hPrevInstance, /*[in]*/ LPTSTR lpCmdLine, /*[in]*/ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    InputBoxParam p = {};
    arginit(__argc, __wargv);
    p.title = argvaluedesc(_T("/Title"), nullptr, _T("title"), _T("Dialog title"));
    p.prompt = argnumdesc(1, nullptr, _T("prompt"), _T("Dialog prompt"));
    p.default = argvaluedesc(_T("/Default"), nullptr, _T("initial_value"), _T("Input initial value"));
    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
        return EXIT_SUCCESS;

    INT_PTR ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_INPUTBOX), NULL, InputBoxDlgProc, LPARAM(&p));
    if (ret == 0)
        _tprintf(_T("%s\n"), p.result);

    return int(ret);
}
