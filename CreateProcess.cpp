#include <cstdio>
#include <cstdlib>
#include <tchar.h>
#include <windows.h>

int find_command(const int argc, const TCHAR* const argv[])
{
    for (int argi = 1; argi < argc; ++argi)
    {
        const TCHAR* const arg = argv[argi];
        if (arg[0] != _T('/'))
            return argi;
    }
    return argc;
}

int _tmain(const int argc, const TCHAR* const argv[])
{
    bool bHelp = false;
    bool bDebug = false;
    STARTUPINFO            siStartupInfo = { sizeof(siStartupInfo) };
    siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    DWORD dwWait = 0;
    LPCTSTR pidfile = nullptr;

    int arg_command = argc;
    for (int argi = 1; argi < argc; ++argi)
    {
        const TCHAR* const arg = argv[argi];
        if (_tcsicmp(arg, _T("/?")) == 0)
            bHelp = true;
        else if (_tcsicmp(arg, _T("/Debug")) == 0)
            bDebug = true;
        else if (_tcsicmp(arg, _T("/Wait")) == 0)
            dwWait = INFINITE;
        else if (_tcsicmp(arg, _T("/Pid")) == 0)
            pidfile = argv[++argi];
        else if (arg[0] == _T('/'))
            _ftprintf(stderr, _T("ERROR: Unknown option '%s'\n"), arg);
        else
        {
            arg_command = argi;
            break;
        }
    }

    if (bHelp)
    {
        _tprintf(TEXT("%s <options> [command] <arg1> <arg2> <arg...>\n"), argv[0]);
        _tprintf(TEXT("\n"));
        _tprintf(TEXT("options:\n"));
        _tprintf(TEXT("  /Wait       - wait for process toe exit, return error code\n"));
        _tprintf(TEXT("  /Pid [file] - output process id to a file\n"));
        _tprintf(TEXT("  /Debug      - display some debug information\n"));
        return EXIT_SUCCESS;
    }
    else if (arg_command >= argc)
    {
        _ftprintf(stderr, _T("ERROR: No command\n"));
        return EXIT_FAILURE;
    }

    TCHAR cmd[32767] = _T("");
    for (int argi = arg_command; argi < argc; ++argi)
    {
        const TCHAR* const arg = argv[argi];
        if (cmd[0] != _T('\0'))
            _tcscat_s(cmd, _T(" "));
        const bool needquotes = _tcschr(arg, _T(' ')) != nullptr;
        if (needquotes)
            _tcscat_s(cmd, _T("\""));
        _tcscat_s(cmd, arg);
        if (needquotes)
            _tcscat_s(cmd, _T("\""));
    }

    if (bDebug) _tprintf(_T("Command: %s\n"), cmd);

    PROCESS_INFORMATION    piProcessInfo = {};
    if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &siStartupInfo, &piProcessInfo) == FALSE)
    {
        _ftprintf(stderr, TEXT("ERROR: Could not create new process (%d)\n"), GetLastError());
        return EXIT_FAILURE;
    }

    if (bDebug) _tprintf(_T("Process id: %u\n"), piProcessInfo.dwProcessId);
    if (pidfile)
    {
        FILE* f = nullptr;
        const errno_t  e = _tfopen_s(&f, pidfile, _T("w"));
        if (e != 0)
            _ftprintf(stderr, TEXT("ERROR: Could open file '%s' (%d)\n"), pidfile, e);
        else
        {
            _ftprintf(f, _T("%u\n"), piProcessInfo.dwProcessId);
            fclose(f);
        }
    }

    DWORD ret = EXIT_SUCCESS;
    if (dwWait != 0)
    {
        WaitForSingleObject(piProcessInfo.hProcess, dwWait);
        GetExitCodeProcess(piProcessInfo.hProcess, &ret);

        if (bDebug) _tprintf(_T("Exit code: %d\n"), ret);
    }

    CloseHandle(piProcessInfo.hThread);
    CloseHandle(piProcessInfo.hProcess);

    return EXIT_SUCCESS;
}
