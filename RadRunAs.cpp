#include <Windows.h>
#include <Shlwapi.h>
#include <WinCred.H>

#define PROGRAM L"RadRunAs"

void ErrorMessage(_In_ LPCWSTR lpFormat, ...)
{
    WCHAR buf[1024];
    va_list vl;
    va_start(vl, lpFormat);
    wvsprintf(buf, lpFormat, vl);
    va_end(vl);
    MessageBox(NULL, buf, PROGRAM, MB_OK | MB_ICONERROR);
}

void ShowError(_In_ LPCWSTR lpWhere)
{
    DWORD errorCode = GetLastError();
    LPWSTR msg = NULL;
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, 0, (LPWSTR) &msg, 0, NULL) == 0)
    {
        ErrorMessage(L"Error: %i FormatMessage.", errorCode);
    }
    else
    {
        ErrorMessage(L"Error: %i %s.\n%s", errorCode, lpWhere, msg);
        LocalFree(msg);
    }
}

LPCWSTR FindArg(LPCWSTR* szArgs, int nArgs, _In_ LPCWSTR lpParam)
{
    int nLen = lstrlen(lpParam);
    if (nLen >= 1 && lpParam[nLen - 1] == L':')
    {
        for (int i = 0; i < nArgs; ++i)
        {
            LPCWSTR lpArg = szArgs[i];
            if (StrCmpNI(lpArg, lpParam, nLen) == 0)
                return lpArg + nLen;
        }
    }
    else
    {
        for (int i = 0; i < nArgs; ++i)
        {
            LPCWSTR lpArg = szArgs[i];
            if (StrCmpI(lpArg, lpParam) == 0)
                return lpArg;
        }
    }
    return nullptr;
}

int FindCommandLine(LPCWSTR* szArgs, int nArgs)
{
    for (int i = 0; i < nArgs; ++i)
    {
        LPCWSTR lpArg = szArgs[i];
        if (lpArg[0] != L'/')
            return i;
    }
    return -1;
}

BOOL MyPathFindOnPath(_Inout_updates_(MAX_PATH) LPWSTR pszPath, _In_opt_ PZPCWSTR ppszOtherDirs)
{
    // TODO Use PATHEXT
    int nLen = lstrlen(pszPath);
    if (nLen <= 4 || StrCmpI(pszPath + nLen - 4, L".exe") != 0)
    {
        StrCat(pszPath, L".exe");
    }

    if (StrCmpI(pszPath, L"regedit.exe") == 0)
    {
        StrCpy(pszPath, L"C:\\Windows\\regedit.exe");
        return TRUE;
    }
    else
    {
        return PathFindOnPath(pszPath, ppszOtherDirs);
    }
}

int CALLBACK wWinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       /*nCmdShow*/
)
{
    int nArgs = 0;
    LPCWSTR* szArgs = (LPCWSTR*) CommandLineToArgvW(lpCmdLine, &nArgs);
    if (szArgs == nullptr)
    {
        ShowError(L"CommandLineToArgvW");
        return 5;
    }

    const LPCWSTR fulluser = FindArg(szArgs, nArgs, L"/user:");
    bool elevated = FindArg(szArgs, nArgs, L"/elevated") != nullptr;
    const int command = FindCommandLine(szArgs, nArgs);

    if (fulluser != nullptr && command >= 0)
    {
        const LPWSTR origCommandLine = StrStr(lpCmdLine, szArgs[command]);
        WCHAR title[MAX_PATH] = L"";
        StrCpy(title, origCommandLine);
        StrCat(title, L" (");
        StrCat(title, fulluser);
        StrCat(title, L")");

        const LPCWSTR password = FindArg(szArgs, nArgs, L"/password:");
        WCHAR username[1024] = L"";
        WCHAR domain[1024] = L"";
        CredUIParseUserName(fulluser, username, 1024, domain, 1024);

        if (!elevated)
        {
            WCHAR application[MAX_PATH] = L"";
            StrCpy(application, szArgs[command]);
            MyPathFindOnPath(application, nullptr);
            const LPWSTR commandLine = origCommandLine;

            STARTUPINFO si = { sizeof(STARTUPINFO) };
            si.lpTitle = title;
            PROCESS_INFORMATION pi = {};

            if (!CreateProcessWithLogonW(
                username,
                domain,
                password,
                LOGON_WITH_PROFILE,
                application,
                commandLine,
                0,
                NULL,
                NULL,
                &si,
                &pi))
            {
                if (!elevated && GetLastError() == ERROR_ELEVATION_REQUIRED)
                {
                    elevated = true;
                }
                else
                {
                    ShowError(L"CreateProcessWithLogonW");
                    LocalFree(szArgs);
                    return 1;
                }
            }

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }

        if (elevated)
        {
            WCHAR application[MAX_PATH] = L"";
            WCHAR commandLine[MAX_PATH] = L"";

            GetModuleFileName(hInstance, application, MAX_PATH);
            StrCpy(commandLine, PathFindFileName(application));
            StrCat(commandLine, L" /elevated ");
            StrCat(commandLine, origCommandLine);

            STARTUPINFO si = { sizeof(STARTUPINFO) };
            si.lpTitle = title;
            PROCESS_INFORMATION pi = {};

            if (!CreateProcessWithLogonW(
                username,
                domain,
                password,
                LOGON_WITH_PROFILE,
                application,
                commandLine,
                0,
                NULL,
                NULL,
                &si,
                &pi))
            {
                ShowError(L"CreateProcessWithLogonW");
                LocalFree(szArgs);
                return 1;
            }

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
    else if (elevated)
    {
        const LPCWSTR application = szArgs[command];
        const LPCWSTR parameters = (command + 1) < nArgs ? StrStr(lpCmdLine, szArgs[command + 1]) : nullptr;
        if ((int) ShellExecute(NULL, L"runas", application, parameters, nullptr, SW_SHOWNORMAL) < 32)
        {
            ShowError(L"ShellExecute");
            LocalFree(szArgs);
            return 1;
        }
    }
    else
    {
        WCHAR buf[] =
            PROGRAM L" Usage:\n"
            L"\n"
            PROGRAM L" /user:<UserName> [/password:<Password>] [/elevated] program parameters\n"
            L"\n"
            PROGRAM L" /elevated program parameters";
        MessageBox(NULL, buf, PROGRAM, MB_OK | MB_ICONINFORMATION);
    }

    LocalFree(szArgs);
    return 0;
}
