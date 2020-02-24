#include <stdio.h>
#include <string.h>
#include <windows.h>

// Background start for windows
// This starts a process in the background with no
// window. Its useful for console programs that you
// don't wish to see. One difference with this version
// is that it passes on the standard input and output
// handles and waits for the process to end so that
// it can return the exit code.

#ifdef UNICODE
#define tWinMain wWinMain
#else
#define tWinMain WinMain
#endif

int WINAPI tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    if (lpCmdLine[0] == TEXT('\0'))
    {
        MessageBox(NULL,
            TEXT("ERROR: No arguments found.\n")
            TEXT("This program will launch executables and batch scripts in the background.\n")
            TEXT("\nUsage: bgstart [command] <argument 1> <argument 2> ...\n"),
            TEXT("BGStart"), MB_ICONERROR);
        return 1;
    }

    STARTUPINFO            siStartupInfo;
    PROCESS_INFORMATION    piProcessInfo;

    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));

    siStartupInfo.cb = sizeof(siStartupInfo);
    siStartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    siStartupInfo.wShowWindow = SW_HIDE;
    siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (CreateProcess(NULL, lpCmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartupInfo, &piProcessInfo) == FALSE)
    {
        MessageBox(NULL, TEXT("ERROR: Could not create new process.\n"), TEXT("BGStart"), MB_ICONERROR);
        return 1;
    }

    WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
    DWORD ret = 0;
    GetExitCodeProcess(piProcessInfo.hProcess, &ret);

    CloseHandle(piProcessInfo.hThread);
    CloseHandle(piProcessInfo.hProcess);

    return ret;
}
