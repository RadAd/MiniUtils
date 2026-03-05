#include <windows.h>
#include <tchar.h>
#include <dwmapi.h>

#include <cstdlib>
#include <cstdio>

#include <string>
#include <vector>
#include <inttypes.h>

struct PrintWindowOptions
{
    std::wstring columns;
    TCHAR sep;
};

void PrintWindowHeadings(const PrintWindowOptions& print)
{
    bool first = true;
    for (auto c : print.columns)
    {
        if (!first)
            _tprintf(_T("%c"), print.sep);
        switch (c)
        {
        case _T('h'):
            _tprintf(_T("%-10s"), _T("HANDLE"));
            break;

        case _T('P'):
            _tprintf(_T("%-10s"), _T("PARENT"));
            break;

        case _T('R'):
            _tprintf(_T("%-10s"), _T("ROOT"));
            break;

        case _T('t'):
            _tprintf(_T("%s"), _T("TITLE"));
            break;

        case _T('c'):
            _tprintf(_T("%-30s"), _T("CLASS"));
            break;

        case _T('s'):
            _tprintf(_T("%-10s"), _T("STYLE"));
            break;

        case _T('x'):
            _tprintf(_T("%-10s"), _T("EX. STYLE"));
            break;

        case _T('p'):
            _tprintf(_T("%-5s"), _T("PID"));
            break;

        case _T('C'):
            _tprintf(_T("%-5s"), _T("CLOAK"));
            break;

        case _T('m'):
            _tprintf(_T("%-10s"), _T("MONITOR"));
            break;
        }
        first = false;
    }
    _tprintf(_T("\n"));
}

void PrintWindow(HWND hWnd, const PrintWindowOptions& print)
{
    bool first = true;
    for (auto c : print.columns)
    {
        if (!first)
            _tprintf(_T("%c"), print.sep);
        switch (c)
        {
        case _T('h'):
            _tprintf(_T("0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(hWnd));
            break;

        case _T('P'):
            _tprintf(_T("0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(GetParent(hWnd)));
            break;

        case _T('R'):
            _tprintf(_T("0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(GetAncestor(hWnd, GA_ROOTOWNER)));
            break;

        case _T('t'):
            {
                TCHAR Title[MAX_PATH] = _T("");
                GetWindowText(hWnd, Title, ARRAYSIZE(Title));
                _tprintf(_T("%s"), Title);
            }
            break;

        case _T('c'):
            {
                TCHAR Class[MAX_PATH] = _T("");
                GetClassName(hWnd, Class, ARRAYSIZE(Class));
                _tprintf(_T("%-30s"), Class);
            }
            break;

        case _T('s'):
            {
                LONG Style = GetWindowLong(hWnd, GWL_STYLE);
                _tprintf(_T("0x%08X"), Style);
            }
            break;

        case _T('x'):
            {
                LONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
                _tprintf(_T("0x%08X"), ExStyle);
            }
            break;

        case _T('p'):
            {
                DWORD dwProcessId = 0;
                GetWindowThreadProcessId(hWnd, &dwProcessId);
                _tprintf(_T("%5d"), dwProcessId);
            }
            break;

        case _T('C'):
            {
                DWORD dwCloak = 0;
                DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &dwCloak, sizeof(dwCloak));
                _tprintf(_T("%5d"), dwCloak);
            }
            break;

        case _T('m'):
            _tprintf(_T("0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL)));
            break;
        }
        first = false;
    }
    _tprintf(_T("\n"));
}

void PrintWindowDetails(HWND hWnd, const PrintWindowOptions& print)
{
    for (auto c : print.columns)
    {
        switch (c)
        {
        case _T('h'):
            _tprintf(_T("HWND: 0x%08") _T(PRIXPTR) _T("\n"), reinterpret_cast<uintptr_t>(hWnd));
            break;

        case _T('P'):
            _tprintf(_T("Parent: 0x%08") _T(PRIXPTR) _T("\n"), reinterpret_cast<uintptr_t>(GetParent(hWnd)));
            break;

        case _T('R'):
            _tprintf(_T("Root: 0x%08") _T(PRIXPTR) _T("\n"), reinterpret_cast<uintptr_t>(GetAncestor(hWnd, GA_ROOTOWNER)));
            break;

        case _T('t'):
            {
                TCHAR Title[MAX_PATH] = _T("");
                GetWindowText(hWnd, Title, ARRAYSIZE(Title));
                _tprintf(_T("Title: \"%s\"\n"), Title);
            }
            break;

        case _T('c'):
            {
                TCHAR Class[MAX_PATH] = _T("");
                GetClassName(hWnd, Class, ARRAYSIZE(Class));
                _tprintf(_T("Class: \"%s\"\n"), Class);
            }
            break;

        case _T('s'):
            {
                LONG Style = GetWindowLong(hWnd, GWL_STYLE);
                _tprintf(_T("Style: 0x%08X\n"), Style);
            }
            break;

        case _T('x'):
            {
                LONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
                _tprintf(_T("ExStyle: 0x%08X\n"), ExStyle);
            }
            break;

        case _T('p'):
            {
                DWORD dwProcessId = 0;
                GetWindowThreadProcessId(hWnd, &dwProcessId);
                _tprintf(_T("PID: %d\n"), dwProcessId);
            }
            break;

        case _T('r'):
            {
                RECT rc = {};
                GetWindowRect(hWnd, &rc);
                _tprintf(_T("Rect: { %d, %d, %d, %d } (%d x %d)\n"), rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case _T('C'):
            {
                DWORD dwCloak = 0;
                DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &dwCloak, sizeof(dwCloak));
                _tprintf(_T("Cloak: %ds\n"), dwCloak);
            }
            break;

        case _T('m'):
            _tprintf(_T("Monitor: 0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL)));
            break;
        }
    }
}

BOOL CALLBACK GetWindowsEnumWindowsProc(
  _In_ HWND   hWnd,
  _In_ LPARAM lParam
)
{
    std::vector<HWND>* pVec = reinterpret_cast<std::vector<HWND>*>(lParam);
    pVec->push_back(hWnd);
    return TRUE;
}

void Filter(std::vector<HWND>& windows, HWND hParentWnd)
{
    windows.erase(std::remove_if(windows.begin(), windows.end(), [hParentWnd](HWND hWnd) {
        const LONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        return !(IsWindowVisible(hWnd) && GetParent(hWnd) == hParentWnd && (ExStyle & WS_EX_TOOLWINDOW) == 0);
    }), windows.end());
}

void PrintWindows(const std::vector<HWND>& windows, const BOOL bRecurse, const std::wstring& prefix, const PrintWindowOptions& print)
{
    for (HWND hWnd : windows)
    {
        if (bRecurse)
        {
            _tprintf(prefix.c_str());
            _tprintf(hWnd == windows.back() ? _T("\xC0\xC4\xC4") : _T("\xC3\xC4\xC4"));
        }
        PrintWindow(hWnd, print);

        if (bRecurse)
        {
            std::vector<HWND> childwindows;
            EnumChildWindows(hWnd, GetWindowsEnumWindowsProc, (LPARAM) &childwindows);
            Filter(childwindows, hWnd);
            PrintWindows(childwindows, bRecurse, prefix + (hWnd == windows.back() ? _T("  "): _T("\xB3 ")), print);
        }
    }
}

void ListWindows(const PrintWindowOptions& print, const HWND hParentWnd, const BOOL bRecurse)
{
    _tprintf(_T("Columns %s\n"), print.columns.c_str());
    PrintWindowHeadings(print);

    std::vector<HWND> windows;
    if (hParentWnd == NULL)
        EnumWindows(GetWindowsEnumWindowsProc, (LPARAM) &windows);
    else
        EnumChildWindows(hParentWnd, GetWindowsEnumWindowsProc, (LPARAM) &windows);
    Filter(windows, hParentWnd);
    PrintWindows(windows, bRecurse, _T(""), print);
}

HWND GetWindow(const TCHAR* s)
{
    if (s == nullptr)
        return NULL;
    else if (_tcsicmp(s, _T("{cursor}")) == 0)
    {
        POINT pt;
        GetCursorPos(&pt);
        return WindowFromPoint(pt);
    }
    else if (_tcsicmp(s, _T("{cursor.parent}")) == 0)
    {
        POINT pt;
        GetCursorPos(&pt);
        return GetParent(WindowFromPoint(pt));
    }
    else if (_tcsicmp(s, _T("{cursor.toplevel}")) == 0)
    {
        POINT pt;
        GetCursorPos(&pt);
        return GetAncestor(WindowFromPoint(pt), GA_ROOTOWNER);
    }
    else if (_tcsicmp(s, _T("{console}")) == 0)
        return GetConsoleWindow();
    else if (_tcsicmp(s, _T("{foreground}")) == 0)
        return GetForegroundWindow();
    else
        return static_cast<HWND>(UlongToHandle(_tcstoul(s, nullptr, 0)));
}

const TCHAR* getarg(int argc, const TCHAR* const argv[], const TCHAR* flag, const TCHAR* def = nullptr)
{
    for (int argnum = 1; argnum < argc; ++argnum)
    {
        const TCHAR* const arg = argv[argnum];
        if (_tcsicmp(arg, flag) == 0)
            return argv[argnum + 1];
    }
    return def;
}

int _tmain(int argc, const TCHAR* const argv[])
{
    PrintWindowOptions print = {};
    print.columns = _T("hpsxct");
    //print.columns = _T("hpCt");
    //print.columns = _T("hmCt");
    print.sep = _T(' ');
    //print.sep = _T(',');
    int argnum = 1;
    const TCHAR* cmd = argc >= argnum ? argv[argnum++] : nullptr;
    const TCHAR* wnd = argc >= argnum ? argv[argnum++] : nullptr;
    const TCHAR* columns = getarg(argc, argv, _T("/Columns"));
    if (columns)
        print.columns = columns;

    if (cmd == nullptr)
    {
        _tprintf(_T("WndList <command> [options] [/Columns columns]\n"));

        _tprintf(_T("\nwhere command can be:\n"));
        _tprintf(_T("list   [parent]      - list all child windows\n"));
        _tprintf(_T("tree   [parent]      - show a tree of all child windows\n"));
        _tprintf(_T("print  [window]      - show a window\n"));
        _tprintf(_T("parent [window]      - walk parent list\n"));

        _tprintf(_T("\nwhere window/parent can be:\n"));
        _tprintf(_T("\t{cursor}     - window under cursor\n"));
        _tprintf(_T("\t{cursor.parent}   - \n"));
        _tprintf(_T("\t{cursor.toplevel} - \n"));
        _tprintf(_T("\t{console}    - console window\n"));
        _tprintf(_T("\t{foreground} - foreground window\n"));

        _tprintf(_T("\nwhere columns can be:\n"));
        _tprintf(_T("\th     - window handle\n"));
        _tprintf(_T("\tP     - parent handle\n"));
        _tprintf(_T("\tR     - root handle\n"));
        _tprintf(_T("\tt     - window title\n"));
        _tprintf(_T("\tc     - window class\n"));
        _tprintf(_T("\ts     - window style\n"));
        _tprintf(_T("\tx     - window ex. style\n"));
        _tprintf(_T("\tp     - process id\n"));
        _tprintf(_T("\tC     - window cloaked\n"));
        _tprintf(_T("\tm     - monitor handle\n"));
    }
    else if (_tcsicmp(cmd, _T("list")) == 0)
        ListWindows(print, GetWindow(wnd), FALSE);
    else if (_tcsicmp(cmd, _T("tree")) == 0)
        ListWindows(print, GetWindow(wnd), TRUE);
    else if (_tcsicmp(cmd, _T("print")) == 0)
    {
        if (!columns)
            print.columns = _T("hpPRctsxrm");
        HWND hWnd = GetWindow(wnd);
        //PrintWindowHeadings(print);
        //PrintWindow(hWnd, print);
        PrintWindowDetails(hWnd, print);
    }
    else if (_tcsicmp(cmd, _T("parent")) == 0)
    {
        HWND hWnd = GetWindow(wnd);
        PrintWindowHeadings(print);
        while ((hWnd = GetParent(hWnd)) != NULL)
            PrintWindow(hWnd, print);
    }
    // "show" ShowWindow();
    // "send" SendMessage();
    else
        _tprintf(_T("Unknown command \"%s\"\n"), cmd);

	return EXIT_SUCCESS;
}
