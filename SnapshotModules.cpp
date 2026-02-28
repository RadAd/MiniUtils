#define UNICODE
#define _UNICODE
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <memory>

#include "arg.inl"
#include "Columns.inl"

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#define member_size(type, member) (sizeof(((type *)0)->member))

template <class T>
T Convert(const TCHAR* s, const T);

template <>
unsigned long Convert(const TCHAR* s, const unsigned long def)
{
    if (s == nullptr)
        return def;
    else
        return _tcstoul(s, nullptr, 0);
}

void printError(TCHAR const* msg, const DWORD eNum = GetLastError())
{
    TCHAR sysMsg[256];
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, eNum,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         sysMsg, 256, NULL );

    // Trim the end of the line and terminate it with a null
    TCHAR* p = sysMsg;
    while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
    do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );

    _ftprintf(stderr, TEXT("ERROR: %s failed with error %d (%s)\n"), msg, eNum, sysMsg);
}

#define PRINT(x) [](const MODULEENTRY32& me32, const DWORD Width) { ColPrintField(me32.x, Width); }

Column<MODULEENTRY32> cols[] = {
    { _T('a'), _T("Address"), _T("Base Address"),   member_size(MODULEENTRY32, modBaseAddr) * 2 + 2, PRINT(modBaseAddr) },
    { _T('s'), _T("Size"),    _T("Module Size"),     8, PRINT(modBaseSize) },
    { _T('h'), _T("Module"),  _T("Module Handle"),  10, PRINT(hModule) },
    { _T('n'), _T("Name"),    _T("Module Name"),    25, PRINT(szModule) },
    { _T('p'), _T("Path"),    _T("Module Path"),     4, PRINT(szExePath) },
};

HANDLE FixHandle(HANDLE h)
{
    return h == INVALID_HANDLE_VALUE ? NULL : h;
}

struct HandleDeleter
{
    typedef HANDLE pointer;

    void operator()(HANDLE h) const
    {
        CloseHandle(h);
    }
};

int _tmain(const int argc, const LPCTSTR argv[])
{
    arginit(argc, argv);

    const DWORD pid = Convert(argnext(nullptr, _T("pid"), _T("Process id")), ULONG_MAX);
    const std::tstring format = argvalue(_T("/Format"), _T("hn"), _T("columns"), _T("Select columns"));

    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
    {
        _tprintf(TEXT("\nWhere format columns are:\n"));
        ColPrintDescription(cols);
        return EXIT_SUCCESS;
    }

    const std::vector<const Column<MODULEENTRY32>*> printcols = ColParseFormat(format, cols);

    const std::unique_ptr<HANDLE, HandleDeleter> hProcessSnap(FixHandle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid)));
    if (!hProcessSnap)
    {
        printError(TEXT("CreateToolhelp32Snapshot"));
        return EXIT_FAILURE;
    }

    MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

    if (!Module32First(hProcessSnap.get(), &me32))
    {
        printError(TEXT("Module32First"));
        return EXIT_FAILURE;
    }

    ColPrintHeader(printcols);

    do
    {
        ColPrintRow(printcols, me32);
    } while (Module32Next(hProcessSnap.get(), &me32));

    return EXIT_SUCCESS;
}
