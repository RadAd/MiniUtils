#include <windows.h>
#include <tchar.h>
#include <dwmapi.h>

#include <cstdlib>
#include <cstdio>

#include <string>
#include <vector>
#include <inttypes.h>

#include "arg.inl"
#include "columns.inl"

template <class T>
struct ColHexFormat
{
    T val;
};

template <class T>
ColHexFormat<T> ColHex(const T& v) { return { v }; }

void ColPrintField(const ColHexFormat<LONG> Val, const DWORD Width){ _tprintf(TEXT("0x%0*X "), Width - 2, Val.val); }

#define PRINT(x) [](const HWND& hWnd, const DWORD Width) { ColPrintField(x, Width); }

DWORD GetWindowProcessId(HWND hWnd)
{
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    return dwProcessId;
}

DWORD GetWindowCloak(HWND hWnd)
{
    DWORD dwCloak = 0;
    DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &dwCloak, sizeof(dwCloak));
    return dwCloak;
}

const Column<HWND> cols[] = {
    { _T('h'), _T("Handle"),    _T("Window Handle"),            10, PRINT(hWnd) },
    { _T('P'), _T("Parent"),    _T("Parent Handle"),            10, PRINT(GetParent(hWnd)) },
    { _T('R'), _T("Root"),      _T("Root Handle"),              10, PRINT(GetAncestor(hWnd, GA_ROOTOWNER)) },
    { _T('t'), _T("Title"),     _T("Window Title"),              5, [](const HWND& hWnd, const DWORD /*Width*/) { TCHAR Text[MAX_PATH] = _T(""); GetWindowText(hWnd, Text, ARRAYSIZE(Text)); _tprintf(TEXT("%s "), Text); } },
    { _T('c'), _T("Class"),     _T("Window Class"),             30, [](const HWND& hWnd, const DWORD Width) { TCHAR Text[MAX_PATH] = _T(""); GetClassName(hWnd, Text, ARRAYSIZE(Text)); ColPrintField(Text, Width); } },
    { _T('s'), _T("Style"),     _T("Window Style"),             10, PRINT(ColHex(GetWindowLong(hWnd, GWL_STYLE))) },
    { _T('x'), _T("Ex. Style"), _T("Window Extended Style"),    10, PRINT(ColHex(GetWindowLong(hWnd, GWL_EXSTYLE))) },
    { _T('P'), _T("PID"),       _T("Process ID"),                5, PRINT(GetWindowProcessId(hWnd)) },
    { _T('C'), _T("Cloak"),     _T("Window Cloaked"),            5, PRINT(GetWindowCloak(hWnd)) },
    { _T('m'), _T("Monitor"),   _T("Monitor Handle"),           10, PRINT(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL)) },
};

struct PrintWindowOptions
{
    std::vector<const Column<HWND>*> printcols;
    TCHAR sep;
};

void PrintWindowDetails(HWND hWnd, const PrintWindowOptions& print)
{
    for (const Column<HWND>* col : print.printcols)
    {
        _tprintf(_T("%-*s: "), 10, col->Name);
        col->Print(hWnd, col->Width);
        _tprintf(_T("\n"));
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
        ColPrintRow(print.printcols, hWnd);

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
    ColPrintHeader(print.printcols);

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

void PrintUsage()
{
    _tprintf(_T("\nwhere command can be:\n"));
    _tprintf(_T("  list   [parent]      - list all child windows\n"));
    _tprintf(_T("  tree   [parent]      - show a tree of all child windows\n"));
    _tprintf(_T("  print  <window>      - show a window\n"));
    _tprintf(_T("  parent <window>      - walk parent list\n"));

    _tprintf(_T("\nwhere window/parent can be:\n"));
    _tprintf(_T("  {cursor}          - window under cursor\n"));
    _tprintf(_T("  {cursor.parent}   - parent of window under cursor\n"));
    _tprintf(_T("  {cursor.toplevel} - toplevel of window under cursor\n"));
    _tprintf(_T("  {console}         - console window\n"));
    _tprintf(_T("  {foreground}      - foreground window\n"));

    _tprintf(_T("\nwhere columns can be:\n"));
    ColPrintDescription(cols);
}

int _tmain(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv);

    const TCHAR* cmd = argnext(nullptr, _T("command"), _T("Command option"));

    if (cmd == nullptr)
    {
        argcleanup();
        _tprintf(_T("\n"));

        if (argusage(true))
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }
    }
    else if (_tcsicmp(cmd, _T("list")) == 0)
    {
        argoptional();
        const TCHAR* wnd = argnext(nullptr, _T("window"), _T("Parent window"));

        PrintWindowOptions print = {};
        print.printcols = ColParseFormat(argvalue(_T("/Columns"), _T("hpsxct"), _T("columns"), _T("Select columns")), cols);

        if (!argcleanup())
            return EXIT_FAILURE;
        if (argusage())
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }

        ListWindows(print, GetWindow(wnd), FALSE);
    }
    else if (_tcsicmp(cmd, _T("tree")) == 0)
    {
        argoptional();
        const TCHAR* wnd = argnext(nullptr, _T("window"), _T("Parent window"));

        PrintWindowOptions print = {};
        print.printcols = ColParseFormat(argvalue(_T("/Columns"), _T("hpsxct"), _T("columns"), _T("Select columns")), cols);

        if (!argcleanup())
            return EXIT_FAILURE;
        if (argusage())
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }

        ListWindows(print, GetWindow(wnd), TRUE);
    }
    else if (_tcsicmp(cmd, _T("print")) == 0)
    {
        const TCHAR* wnd = argnext(nullptr, _T("window"), _T("Window"));

        PrintWindowOptions print = {};
        print.printcols = ColParseFormat(argvalue(_T("/Columns"), _T("hpPRctsxrm"), _T("columns"), _T("Select columns")), cols);

        if (!argcleanup())
            return EXIT_FAILURE;
        if (argusage())
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }

        PrintWindowDetails(GetWindow(wnd), print);
    }
    else if (_tcsicmp(cmd, _T("parent")) == 0)
    {
        const TCHAR* wnd = argnext(nullptr, _T("window"), _T("Window"));

        PrintWindowOptions print = {};
        print.printcols = ColParseFormat(argvalue(_T("/Columns"), _T("hpPRctsxrm"), _T("columns"), _T("Select columns")), cols);

        if (!argcleanup())
            return EXIT_FAILURE;
        if (argusage())
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }

        HWND hWnd = GetWindow(wnd);
        ColPrintHeader(print.printcols);
        while ((hWnd = GetParent(hWnd)) != NULL)
            ColPrintRow(print.printcols, hWnd);
    }
    // "show" ShowWindow();
    // "send" SendMessage();
    else
    {
        _tprintf(_T(ARG_ERROR("Unknown command:") " \"%s\"\n"), cmd);
        argcleanup();
        _tprintf(_T("\n"));

        if (argusage(true))
        {
            PrintUsage();
            return EXIT_SUCCESS;
        }
    }

	return EXIT_SUCCESS;
}
