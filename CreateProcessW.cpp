#define NOMINMAX
//include <string.h>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <string_view>
#include <cctype>
#include <cwctype>

#ifdef UNICODE
#define tstring wstring
#define tstring_view wstring_view
#else
#define tstring string
#define tstring_view string_view
#endif

#define PROG_NAME TEXT("CreateProcess")

std::tstring Format(_In_z_ _Printf_format_string_ TCHAR const* const format, ...)
{
    std::tstring buffer;
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);
    int const _Result1 = _vsctprintf_l(format, NULL, args1);
    va_end(args1);
    buffer.resize(_Result1);
#ifdef DEBUG
    int const _Result2 =
#endif
     _vstprintf_s_l(buffer.data(), buffer.length() + 1, format, NULL, args2);
    assert(-1 != _Result2);
    va_end(args2);
    return buffer;
}

template<int N>
void Message(_In_ UINT uType, _In_z_ const TCHAR(&msg)[N])
{
    MessageBox(NULL, msg, PROG_NAME, uType);
}

void Message(_In_ UINT uType, std::tstring const& msg)
{
    MessageBox(NULL, msg.c_str(), PROG_NAME, uType);
}

inline bool EqualNoCase(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](auto ca, auto cb){ return std::tolower(ca) == std::tolower(cb); });
}

inline bool EqualNoCase(std::wstring_view a, std::wstring_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](auto ca, auto cb) { return std::towlower(ca) == std::towlower(cb); });
}

inline std::string_view substr(std::string_view sv, std::size_t start, std::size_t end)
{
    assert(end >= start);
    return end != std::tstring_view::npos ? sv.substr(start, end - start) : sv.substr(start);
}

inline std::wstring_view substr(std::wstring_view sv, std::size_t start, std::size_t end)
{
    assert(end >= start);
    return end != std::tstring_view::npos ? sv.substr(start, end - start) : sv.substr(start);
}

int WINAPI _tWinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPTSTR lpCmdLine, _In_ int /*nCmdShow*/)
{
    std::tstring_view cmdline(lpCmdLine);

    bool bWait = false;

    std::tstring_view ws = TEXT(" \t");
    std::size_t start = cmdline.find_first_not_of(ws);
    while (start != std::tstring_view::npos && cmdline[start] == TEXT('/'))
    {
        const std::size_t end = cmdline.find_first_of(ws, start);
        const std::tstring_view option = substr(cmdline, start, end);
        if (EqualNoCase(option, TEXT("/Wait")))
            bWait = true;
        else
            Message(MB_ICONERROR, Format(TEXT("ERROR: Unknown option \"%.*s\"."), static_cast<int>(option.length()), option.data()));
        start = cmdline.find_first_not_of(ws, end);
    }

    if (start == std::tstring_view::npos)
    {
        Message(MB_ICONERROR,
            TEXT("ERROR: No command found.\n\n")
            TEXT("Usage: " PROG_NAME " <options> [command] <argument 1> <argument 2> ...\n\n")
            TEXT("Options:\n")
            TEXT("\t/Wait - wait for process to end.\n"));
        return EXIT_FAILURE;
    }
    else
    {
        STARTUPINFO            siStartupInfo = { sizeof(siStartupInfo) };
        siStartupInfo.dwFlags = STARTF_USESTDHANDLES;
        siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        siStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        siStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        PROCESS_INFORMATION    piProcessInfo = {};
        if (CreateProcess(NULL, lpCmdLine + start, NULL, NULL, TRUE, 0, NULL, NULL, &siStartupInfo, &piProcessInfo) == FALSE)
        {
            Message(MB_ICONERROR, Format(TEXT("ERROR: Could not create new process (%d)."), GetLastError()));
            return EXIT_FAILURE;
        }

        DWORD ret = EXIT_SUCCESS;
        if (bWait)
        {
            WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
            GetExitCodeProcess(piProcessInfo.hProcess, &ret);
        }

        CloseHandle(piProcessInfo.hThread);
        CloseHandle(piProcessInfo.hProcess);

        return ret;
    }
}
