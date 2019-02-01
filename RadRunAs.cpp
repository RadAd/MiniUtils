#include <Windows.h>
#include <Shlwapi.h>
#include <WinCred.H>

// To set credential:
// cmdkey /generic:RadRunAs:<Cred> /user:<User> /pass

// Note: Executable needs to be placed in a location accessible by the RunAs user

#define PROGRAM L"RadRunAs"

const LPCWSTR PARAM_ELEVATED = L"/elevated";
const LPCWSTR PARAM_USER = L"/user:";
const LPCWSTR PARAM_PASSWORD = L"/password:";
const LPCWSTR PARAM_CRED = L"/cred:";

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

bool IsValidArg(LPCWSTR lpArg, _In_ LPCWSTR lpParam)
{
    int nLen = lstrlen(lpParam);
    if (nLen >= 1 && lpParam[nLen - 1] == L':')
    {
        return StrCmpNI(lpArg, lpParam, nLen) == 0;
    }
    else
    {
        return StrCmpI(lpArg, lpParam) == 0;
    }
}

LPCWSTR FindArg(const LPCWSTR* szArgs, int nArgs, _In_ LPCWSTR lpParam)
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

LPWSTR CopyString(const LPCWSTR s, const size_t l)
{
    LPWSTR r = new WCHAR[l + 1];
    //wcscpy_s(r, l, s);
    StrCpyN(r, s, (int) l + 1);
    r[l] = L'\0';
    return r;
}

LPWSTR CopyString(const LPCWSTR s)
{
    return CopyString(s, wcslen(s));
}

struct Credentials
{
    const LPCWSTR fulluser;
    const LPCWSTR password;
};

const Credentials GetCredentials(const LPCWSTR* szArgs, int nArgs)
{
    const LPCWSTR cred = FindArg(szArgs, nArgs, PARAM_CRED);

    if (cred != nullptr)
    {
        PCREDENTIAL pCred = nullptr;
        if (CredRead(cred, CRED_TYPE_GENERIC, 0, &pCred))
        {
            // Note: Both of these CopyString leak memory, but due to short life of process, shouldn't be a problem
            Credentials credentials{
                CopyString(pCred->UserName),
                CopyString((LPTSTR)pCred->CredentialBlob, pCred->CredentialBlobSize / sizeof(TCHAR))
            };
            CredFree(pCred);
            return credentials;
        }
        else
            return Credentials{
                nullptr,
                nullptr
            };
    }
    else
    {
        return Credentials{
            FindArg(szArgs, nArgs, PARAM_USER),
            FindArg(szArgs, nArgs, PARAM_PASSWORD)
        };
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

    for (int i = 0; i < nArgs; ++i)
    {
        LPCWSTR lpArg = szArgs[i];
        if (lpArg[0] == L'/'
            && !(IsValidArg(lpArg, PARAM_ELEVATED) || IsValidArg(lpArg, PARAM_USER) || IsValidArg(lpArg, PARAM_PASSWORD) || IsValidArg(lpArg, PARAM_CRED)))
        {
            ErrorMessage(L"Invalid Argument: %s", lpArg);
            return 1;
        }
    }

    const Credentials cred = GetCredentials(szArgs, nArgs);
    bool elevated = FindArg(szArgs, nArgs, PARAM_ELEVATED) != nullptr;
    const int command = FindCommandLine(szArgs, nArgs);

    if (cred.fulluser != nullptr && command >= 0)
    {
        const LPWSTR origCommandLine = StrStr(lpCmdLine, szArgs[command]);
        WCHAR title[MAX_PATH] = L"";
        StrCpy(title, origCommandLine);
        StrCat(title, L" (");
        StrCat(title, cred.fulluser);
        StrCat(title, L")");

        WCHAR username[1024] = L"";
        WCHAR domain[1024] = L"";
        CredUIParseUserName(cred.fulluser, username, 1024, domain, 1024);

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
                cred.password,
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
        else
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
                cred.password,
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
    else if (elevated && command >= 0)
    {
        const LPCWSTR application = szArgs[command];
        const LPCWSTR parameters = (command + 1) < nArgs ? StrStr(lpCmdLine, szArgs[command + 1]) : nullptr;
        if ((INT_PTR) ShellExecute(NULL, L"runas", application, parameters, nullptr, SW_SHOWNORMAL) < 32)
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
            PROGRAM L" /user:<UserName> [/password:<Password>] [/elevated] <program> [parameters]\n"
            L"\n"
            PROGRAM L" /cred:RadRunAs:<Credential> [/elevated] <program> [parameters]\n"
            L"\n"
            L"    Created using: cmdkey /generic:RadRunAs:<Cred> /user:<User> /pass\n"
            L"\n"
            PROGRAM L" /elevated <program> [parameters]";
        MessageBox(NULL, buf, PROGRAM, MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    LocalFree(szArgs);
    return 0;
}
