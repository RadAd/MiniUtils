#include <windows.h>
#include <tchar.h>

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

        case _T('a'):
            _tprintf(_T("%-10s"), _T("PARENT"));
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
            
        case _T('a'):
            _tprintf(_T("0x%08") _T(PRIXPTR), reinterpret_cast<uintptr_t>(GetParent(hWnd)));
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
        }
        first = false;
    }
    _tprintf(_T("\n"));
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
    else if (_tcsicmp(s, _T("{console}")) == 0)
        return GetConsoleWindow();
    else if (_tcsicmp(s, _T("{foreground}")) == 0)
        return GetForegroundWindow();
    else
        return static_cast<HWND>(UlongToHandle(_tcstoul(s, nullptr, 0)));
}

int _tmain(int argc, const TCHAR* const argv[])
{
    PrintWindowOptions print = {};
    print.columns = _T("hpsxct");
    print.sep = _T(' ');
    //print.sep = _T(',');
    const TCHAR* cmd = argc >= 1 ? argv[1] : nullptr;
    const TCHAR* wnd = argc >= 2 ? argv[2] : nullptr;

    if (cmd == nullptr)
    {
        _tprintf(_T("WndList <command> [options]\n"));

        _tprintf(_T("\nwhere command can be:\n"));
        _tprintf(_T("list  [parent]      - list all child windows\n"));
        _tprintf(_T("tree  [parent]      - show a tree of all child windows\n"));
        _tprintf(_T("print [window]      - show a window\n"));

        _tprintf(_T("\nwhere window/parent can be:\n"));
        _tprintf(_T("cursor     - window under cursor\n"));
        _tprintf(_T("console    - console window\n"));
        _tprintf(_T("foreground - foreground window\n"));
    }
    else if (_tcsicmp(cmd, _T("list")) == 0)
        ListWindows(print, GetWindow(wnd), FALSE);
    else if (_tcsicmp(cmd, _T("tree")) == 0)
        ListWindows(print, GetWindow(wnd), TRUE);
    else if (_tcsicmp(cmd, _T("print")) == 0)
    {
        HWND hWnd = GetWindow(wnd);
        PrintWindowHeadings(print);
        PrintWindow(hWnd, print);
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
#if 0
    else if (_tcsicmp(cmd, _T("cursor")) == 0)
    {
        POINT pt;
        GetCursorPos(&pt);
        HWND hWnd = WindowFromPoint(pt);
        PrintWindowHeadings(print);
        PrintWindow(hWnd, print);
    }
    else if (_tcscmp(cmd, _T("console")) == 0)
    {
        HWND hWnd = GetConsoleWindow();
        PrintWindowHeadings(print);
        PrintWindow(hWnd, print);
    }
    else if (_tcscmp(cmd, _T("foreground")) == 0)
    {
        HWND hWnd = GetForegroundWindow();
        PrintWindowHeadings(print);
        PrintWindow(hWnd, print);
    }
#endif
    else
        _tprintf(_T("Unknown command \"%s\"\n"), cmd);
    
	return EXIT_SUCCESS;
}
