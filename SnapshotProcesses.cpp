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

#define PRINT(x) [](const PROCESSENTRY32& pe32, const DWORD Width) { ColPrintField(pe32.x, Width); }

Column<PROCESSENTRY32> cols[] = {
    { _T('i'), _T("PID"),    _T("Process ID"),          6, PRINT(th32ProcessID) },
    { _T('p'), _T("Parent"), _T("Parent Process ID"),   6, PRINT(th32ParentProcessID)  },
    { _T('t'), _T("Thrd"),   _T("Thread count"),        4, PRINT(cntThreads) },
    { _T('r'), _T("Pri"),    _T("Priority"),            4, PRINT(pcPriClassBase) },
    { _T('n'), _T("Name"),   _T("Process Name"),        4, PRINT(szExeFile) },
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

    const std::tstring format = argvalue(_T("/Format"), _T("ipn"), _T("columns"), _T("Select columns"));

    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
    {
        _tprintf(TEXT("\nWhere format columns are:\n"));
        ColPrintDescription(cols);
        return EXIT_SUCCESS;
    }

    const std::vector<const Column<PROCESSENTRY32>*> printcols = ColParseFormat(format, cols);

    const std::unique_ptr<HANDLE, HandleDeleter> hProcessSnap(FixHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)));
    if (!hProcessSnap)
    {
        printError(TEXT("CreateToolhelp32Snapshot"));
        return EXIT_FAILURE;
    }

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

    if (!Process32First(hProcessSnap.get(), &pe32))
    {
        printError(TEXT("Process32First"));
        return EXIT_FAILURE;
    }

    ColPrintHeader(printcols);

    do
    {
        ColPrintRow(printcols, pe32);
    } while (Process32Next(hProcessSnap.get(), &pe32));

    return EXIT_SUCCESS;
}
