#include <windows.h>
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

struct InputBoxParam
{
    LPCTSTR title;
    LPCTSTR prompt;
    LPCTSTR default;
    TCHAR result[2048];
};

INT_PTR InputBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    InputBoxParam* p = reinterpret_cast<InputBoxParam*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));;
    UNREFERENCED_PARAMETER(lParam);
    //_tprintf(stderr, TEXT("InputBoxDlgProc uMsg %8X \twParam %p \tlParam %p\n"), uMsg, LPCVOID(INT_PTR(wParam)), LPCVOID(INT_PTR(lParam)));
    switch (uMsg)
    {
    case WM_INITDIALOG:
        p = (InputBoxParam*) lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, LONG_PTR(p));
        if (p->title) SetWindowText(hDlg, p->title);
        if (p->prompt) SetDlgItemText(hDlg, IDC_PROMPT, p->prompt);
        if (p->default) SetDlgItemText(hDlg, IDC_EDIT1, p->default);
        return TRUE;

    case WM_COMMAND:
        //_tprintf(stderr, TEXT("InputBoxDlgProc uMsg %8X \twParam %p \tlParam %p\n"), uMsg, LPCVOID(INT_PTR(wParam)), LPCVOID(INT_PTR(lParam)));
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
