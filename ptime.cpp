#include <windows.h>
#include <cstdio>
#include <tchar.h>
#include <memory>

#define _MILLISECOND ((__int64) 10000)
#define _SECOND ((__int64) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR)

inline ULONGLONG Expand(FILETIME ft)
{
    return (((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
}

inline FILETIME Contract(ULONGLONG qwResult)
{
    FILETIME ft;
    ft.dwLowDateTime  = (DWORD) (qwResult & 0xFFFFFFFF );
    ft.dwHighDateTime = (DWORD) (qwResult >> 32 );
    return ft;
}

inline FILETIME operator+(FILETIME ft1, FILETIME ft2)
{
    return Contract(Expand(ft1) + Expand(ft2));
}

inline FILETIME operator-(FILETIME ft1, FILETIME ft2)
{
    return Contract(Expand(ft1) - Expand(ft2));
}

void DisplayTime(const TCHAR* name, const SYSTEMTIME* st)
{
    TCHAR szLocalDate[255], szLocalTime[255];

    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, st, NULL, szLocalDate, 255 );
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, st, NULL, szLocalTime, 255 );
    _tprintf(TEXT("%s %s %s\n"), name, szLocalDate, szLocalTime );
}

void DisplayTime(const TCHAR* name, FILETIME ft)
{
    SYSTEMTIME st;
    FileTimeToLocalFileTime( &ft, &ft );
    FileTimeToSystemTime( &ft, &st );
    DisplayTime(name, &st);
}

void DisplayElapsedTime(const TCHAR* name, FILETIME ft)
{
    ULONGLONG qwResult = Expand(ft);

    int hours = (int) (qwResult / _HOUR);
    qwResult -= hours * _HOUR;
    int minutes = (int) (qwResult / _MINUTE);
    qwResult -= minutes * _MINUTE;
    int seconds = (int) (qwResult / _SECOND);
    qwResult -= seconds * _SECOND;
    int milliseconds = (int) (qwResult / _MILLISECOND);
    qwResult -= milliseconds * _MILLISECOND;
    _tprintf(TEXT("%s %d:%02d:%02d.%03d sec\n"), name, hours, minutes, seconds, milliseconds);
}

BOOL DisplayProcessTimes(HANDLE hProcess, BOOL exited)
{
    FILETIME ftCreationTime;
    FILETIME ftExitTime;
    FILETIME ftKernelTime;
    FILETIME ftUserTime;
    if (GetProcessTimes(hProcess, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime) == 0)
    {
        _tprintf(TEXT("Error GetProcessTimes: %d\n"), GetLastError());
        return FALSE;
    }

    DisplayTime(TEXT("Started: "), ftCreationTime);
    if (exited)
    {
        DisplayTime(TEXT("Exited: "), ftExitTime);
        DisplayElapsedTime(TEXT("Elapsed:"), ftExitTime - ftCreationTime);
    }
    else
    {
        SYSTEMTIME stLocalTime;
        GetLocalTime(&stLocalTime);
        DisplayTime(TEXT("Current: "), &stLocalTime);

        FILETIME ftLocalTime;
        SystemTimeToFileTime( &stLocalTime, &ftLocalTime );
        LocalFileTimeToFileTime( &ftLocalTime, &ftLocalTime );
        DisplayElapsedTime(TEXT("Elapsed:"), ftLocalTime - ftCreationTime);
    }
    DisplayElapsedTime(TEXT("Kernel: "), ftKernelTime);
    DisplayElapsedTime(TEXT("User:   "), ftUserTime);
    DisplayElapsedTime(TEXT("Total:  "), ftKernelTime + ftUserTime);

    return TRUE;
}

template <class T>
auto CreateUnique(HANDLE h, T* f)
{
    return std::unique_ptr<std::remove_pointer<HANDLE>::type, T*>(h, f);
}

auto CreateUnique(HANDLE h)
{
    return CreateUnique(h, CloseHandle);
}

#define CHECK(p, m) \
    if ((p)) \
    { \
        _ftprintf(stderr, TEXT(m), GetLastError()); \
        return EXIT_FAILURE; \
    }

int _tmain(int argc, const TCHAR* argv[])
{
    bool wait = false;
    DWORD dwProcessId = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (_tcscmp(argv[i], TEXT("/w")) == 0 || _tcscmp(argv[i], TEXT("/wait")) == 0)
            wait = true;
        else
            dwProcessId = _tstoi(argv[i]);
    }

    if (dwProcessId == 0)
    {
        _tprintf(TEXT("Displays process times\n"));
        _tprintf(TEXT("\n"));
        _tprintf(TEXT("ptime [/wait] pid\n"));
        _tprintf(TEXT("\n"));
        _tprintf(TEXT("  pid   - process id\n"));
        _tprintf(TEXT("  /wait - wait for process to end\n"));
        return EXIT_FAILURE;
    }

    auto hProcess = CreateUnique(OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwProcessId));
    CHECK(!hProcess, "Error OpenProcess: %d\n");

    DWORD dwExitCode = 0;
    CHECK(GetExitCodeProcess(hProcess.get(), &dwExitCode) == 0, "Error GetExitCodeProcess: %d\n");

    CHECK(DisplayProcessTimes(hProcess.get(), dwExitCode != STILL_ACTIVE) == 0, "Error DisplayProcessTimes: %d\n");

    if (wait)
    {
        _tprintf(TEXT("\nWaiting for process to exit...\n\n"));
        CHECK(WaitForSingleObject(hProcess.get(), INFINITE) == WAIT_FAILED, "Error WaitForSingleObject: %d\n");

        CHECK(DisplayProcessTimes(hProcess.get(), TRUE) == 0, "Error DisplayProcessTimes: %d\n");
    }

    return EXIT_SUCCESS;
}
