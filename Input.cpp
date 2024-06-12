#include <Windows.h>
#include <tchar.h>
#include <memory>
#include "arg.inl"

#define ARRAY_X(a) (a), ARRAYSIZE(a)

template <class T>
auto CreateUnique(HANDLE h, T* f)
{
    return std::unique_ptr<std::remove_pointer<HANDLE>::type, T*>(h, f);
}

auto CreateUnique(HANDLE h)
{
    return CreateUnique(h, CloseHandle);
}

template <size_t size>
void StrCopyIf(TCHAR (&dest)[size], const TCHAR *src)
{
    if (src != nullptr)
        _tcscpy_s(dest, src);
}

BOOL WriteFileANSI(HANDLE hFile, _In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr, int cchWideChar = -1)
{
    CHAR szAnsiCharStr[1024];
    int bytes = WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, cchWideChar, szAnsiCharStr, ARRAYSIZE(szAnsiCharStr), nullptr, nullptr);
    DWORD dwWritten;
    return WriteFile(hFile, szAnsiCharStr, bytes, &dwWritten, NULL);
}

BOOL PrintConsole(const HANDLE hOutput, const WCHAR* const format, ...)
{
    WCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnwprintf_s(ARRAY_X(buffer), format, args);
    va_end(args);
    return WriteConsole(hOutput, buffer, DWORD(wcslen(buffer)), nullptr, nullptr);
}

BOOL PrintConsole(const HANDLE hOutput, const CHAR* const format, ...)
{
    CHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(ARRAY_X(buffer), format, args);
    va_end(args);
    return WriteConsole(hOutput, buffer, DWORD(strlen(buffer)), nullptr, nullptr);
}

BOOL PrintANSI(const HANDLE hOutput, const TCHAR* const format, ...)
{
    TCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsntprintf_s(ARRAY_X(buffer), format, args);
    va_end(args);
    return WriteFileANSI(hOutput, buffer, -1);
}

#define CHECK(t, msg) \
    if (!(t)) \
    { \
        DWORD e = GetLastError(); \
        PrintANSI(GetStdHandle(STD_ERROR_HANDLE), _T("ERROR:\t%s (error %d)\n"), (msg), e); \
        return e; \
    };

#define COLOR(n, s) "\x1B[" #n "m" s "\x1B[0m"
#define COMMAND(s) COLOR(37, s)
#define OPTION(s) COLOR(33, s)
#define VALUE(s) COLOR(36, s)

int _tmain(int argc, const TCHAR* const argv[])
{
    //const HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    TCHAR Buffer[1024] = _T("");

    arginit(argc, argv);
    BOOL help = argswitch(_T("/?"));
    const TCHAR* prompt = argnum(1, _T("? "));
    // TODO Expand prompt with cmd-like prompt extensions
    StrCopyIf(Buffer, argvalue(_T("/Default")));
    // TODO Get default value from hInput
    if (!argcleanup())
        return EXIT_FAILURE;

    if (help)
    {
        if (GetFileType(hOutput) == FILE_TYPE_CHAR)
        {
            CHECK(PrintConsole(hOutput, _T(COMMAND("%s")
                " [/" OPTION("Default") "=" VALUE("initial_value") "]"
                " [" VALUE("prompt") "]"
                "\n"), argapp()), _T("PrintConsole Help\n"));
        }
        else
        {
            CHECK(PrintANSI(hOutput, _T("%s"
                " [/" "Default" "=" "initial_value" "]"
                " [" "prompt" "]"
                "\n"), argapp()), _T("PrintANSI Help\n"));
        }
        return EXIT_SUCCESS;
    }

    auto hConsoleInput = CreateUnique(CreateFile(_T("CONIN$"), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0));
    CHECK(hConsoleInput != NULL, _T("CreateFile CONIN$\n"))
    auto hConsoleOutput = CreateUnique(CreateFile(_T("CONOUT$"), GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0));
    CHECK(hConsoleOutput != NULL, _T("CreateFile CONOUT$\n"))

    DWORD Read = 0;
    DWORD Written = 0;
    CONSOLE_READCONSOLE_CONTROL control = { sizeof(CONSOLE_READCONSOLE_CONTROL) };
    control.nInitialChars = ULONG(_tcslen(Buffer));    // Only works in UNICODE mode

    if (prompt)
        CHECK(WriteConsole(hConsoleOutput.get(), prompt, DWORD(_tcslen(prompt)), &Written, nullptr), _T("WriteConsole prompt"));
    CHECK(WriteConsole(hConsoleOutput.get(), Buffer, control.nInitialChars, &Written, nullptr), _T("WriteConsole Buffer"));
    CHECK(ReadConsole(hConsoleInput.get(), ARRAY_X(Buffer), &Read, &control), _T("ReadConsole Buffer"));

    Buffer[Read] = _T('\0');
    //CHECK(WriteConsole(hOutput, Buffer, Read, &Written, nullptr), _T("WriteConsole Buffer"));
    CHECK(WriteFileANSI(hOutput, Buffer, Read), _T("WriteFileANSI Buffer"));
    return EXIT_SUCCESS;
}
