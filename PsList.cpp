#define PSAPI_VERSION 2

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <winternl.h>
#include <shlwapi.h>
#include "arg.inl"

// TODO Get other inforamtion
// process times
// mem usage
// cpu usage - needs to done over time
// command line
// main window handle and title

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

DWORD GetProcessName(HANDLE hProcess, LPTSTR lpProcessName, DWORD nSize)
{
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        return GetModuleBaseName(hProcess, hMod, lpProcessName, nSize);
    else
        return 0;
}

DWORD GetParentProcessId(HANDLE hProcess)
{
    PROCESS_BASIC_INFORMATION pbi = {};
    if (NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr) == 0)
        return *reinterpret_cast<DWORD*>(&pbi.Reserved3); /*InheritedFromUniqueProcessId*/
    else
        return 0;
}

#include <comdef.h>
#define MAX_NAME 256
BOOL GetLogonFromToken(HANDLE hToken, LPTSTR lpUser, DWORD nSizeUser, LPTSTR lpDomain, DWORD nSizeDomain)
{
    BOOL bSuccess = FALSE;
    
    lpUser[0] = _T('\0');
    lpDomain[0] = _T('\0');

    PTOKEN_USER ptu = NULL;
    DWORD dwLength = 0;
    if (!GetTokenInformation(
            hToken,         // handle to the access token
            TokenUser,    // get information about the token's groups
            (LPVOID) ptu,   // pointer to PTOKEN_USER buffer
            0,              // size of buffer
            &dwLength       // receives required buffer size
        ))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto Cleanup;

        ptu = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);

        if (ptu == NULL)
            goto Cleanup;
    }

    if (!GetTokenInformation(
            hToken,         // handle to the access token
            TokenUser,    // get information about the token's groups
            (LPVOID) ptu,   // pointer to PTOKEN_USER buffer
            dwLength,       // size of buffer
            &dwLength       // receives required buffer size
        ))
    {
        goto Cleanup;
    }

    SID_NAME_USE SidType;
    if (!LookupAccountSid(NULL , ptu->User.Sid, lpUser, &nSizeUser, lpDomain, &nSizeDomain, &SidType))
    {
        DWORD dwResult = GetLastError();
        if (dwResult == ERROR_NONE_MAPPED)
            _tcscpy_s(lpUser, nSizeUser, "NONE_MAPPED");
    }
    else
    {
        bSuccess = TRUE;
    }

Cleanup:
    if (ptu != NULL)
        HeapFree(GetProcessHeap(), 0, (LPVOID) ptu);
    return bSuccess;
}

void PrintProcess(DWORD processID, HANDLE hProcess, LPCTSTR lpFilter)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    TCHAR strUser[MAX_NAME] = TEXT("<unknown>");
    TCHAR strDomain[MAX_NAME] = TEXT("<unknown>");
    BOOL bWow64 = FALSE;
    DWORD processParentID = 0;

    if (NULL != hProcess)
    {
        processParentID = GetParentProcessId(hProcess);
        IsWow64Process(hProcess, &bWow64);
        
        //if (GetProcessName(hProcess, szProcessName, ARRAYSIZE(szProcessName)) == 0)
        //if (GetModuleBaseName(hProcess, NULL, szProcessName, ARRAYSIZE(szProcessName)) == 0)
        {
            //GetProcessImageFileName(hProcess, szProcessName, ARRAYSIZE(szProcessName));
            DWORD nSize = ARRAYSIZE(szProcessName);
            QueryFullProcessImageName(hProcess, 0, szProcessName, &nSize);
        }
        
        HANDLE hToken = NULL;
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
        {
            /*BOOL bres =*/ GetLogonFromToken(hToken, strUser, ARRAYSIZE(strUser), strDomain, ARRAYSIZE(strDomain));
            CloseHandle(hToken);
        }
    }

    if (lpFilter == nullptr || PathMatchSpec(szProcessName, lpFilter))
    {
        _tprintf(TEXT("%5u "), processID);
        _tprintf(TEXT("%5u "), processParentID);
        _tprintf(TEXT("%2u "), bWow64 ? 32u : 64u);
        _tprintf(TEXT("%-15s "), strUser);
        _tprintf(TEXT("%-15s "), strDomain);
        _tprintf(TEXT("%s "), szProcessName);
        _tprintf(TEXT("\n"));
    }
}

struct ProcessInfo
{
    DWORD dwProcessId;
    DWORD dwParentProcessId;
    HANDLE hProcess;
};

void PrintTreeItem(const ProcessInfo& pi, const TCHAR* pre, bool last)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    GetProcessName(pi.hProcess, szProcessName, ARRAYSIZE(szProcessName));

    _tprintf(TEXT("%s"), pre);
    _tprintf(last ? TEXT("└─ ") : TEXT("├─ "));
    _tprintf(TEXT("%-5u "), pi.dwProcessId);
    _tprintf(TEXT("%s "), szProcessName);
    _tprintf(TEXT("\n"));
}

void PrintTree(DWORD processID, const TCHAR* pre, const ProcessInfo* processinfo, const DWORD cProcesses)
{
    TCHAR lpre[100];
    _tcscpy_s(lpre, ARRAYSIZE(lpre), pre);
    _tcscat_s(lpre, ARRAYSIZE(lpre), TEXT("│  "));
    TCHAR lpre2[100];
    _tcscpy_s(lpre2, ARRAYSIZE(lpre2), pre);
    _tcscat_s(lpre2, ARRAYSIZE(lpre2), TEXT("   "));

    unsigned int p[1024];
    unsigned int count = 0;
    for (unsigned int i = 0; i < cProcesses; i++)
    {
        const ProcessInfo& pi = processinfo[i];
        if (pi.dwParentProcessId == processID)
            p[count++] = i;
    }

    for (unsigned int i = 0; i < count; i++)
    {
        const bool last = i == count - 1;
        const ProcessInfo& pi = processinfo[p[i]];
        PrintTreeItem(pi, pre, last);
        PrintTree(pi.dwProcessId, last ? lpre2 : lpre, processinfo, cProcesses);
    }
}

int main(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv);
    const bool showAll = argswitch(_T("/a"));
    const bool showTree = argswitch(_T("/t"));
    LPCTSTR lpFilter = argnum(1);
    if (!argcleanup())
        return EXIT_FAILURE;

    DWORD aProcesses[1024], cbNeeded;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        _tprintf(TEXT("Error EnumProcesses\n"));
        return 1;
    }
    const DWORD cProcesses = cbNeeded / sizeof(DWORD);

    if (showTree)
    {
        ProcessInfo processinfo[1024];
        for (unsigned int i = 0; i < cProcesses; i++)
        {
            ProcessInfo& pi = processinfo[i];
            pi.dwProcessId = aProcesses[i];

            pi.hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pi.dwProcessId);
            if (pi.hProcess == NULL)
                pi.hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pi.dwProcessId);

            pi.dwParentProcessId = pi.hProcess ? GetParentProcessId(pi.hProcess) : -1;
        }

        unsigned int p[1024];
        unsigned int count = 0;
        if (lpFilter != nullptr)
        {
            const DWORD dwProcessId = _ttoi(lpFilter);
            for (unsigned int i = 0; i < cProcesses; i++)
            {
                const ProcessInfo& pi = processinfo[i];
                if (pi.dwProcessId == dwProcessId)
                    p[count++] = i;
            }
        }
        else
        {
            for (unsigned int i = 0; i < cProcesses; i++)
            {
                const ProcessInfo& pi = processinfo[i];
                if (showAll || pi.hProcess != NULL)
                {
                    bool fFound = false;
                    for (unsigned int j = 0; j < cProcesses; j++)
                    {
                        if (processinfo[j].dwProcessId == pi.dwParentProcessId)
                        {
                            fFound = true;
                            break;
                        }
                    }
                    if (!fFound)
                        p[count++] = i;
                }
            }
        }

        for (unsigned int i = 0; i < count; i++)
        {
            const bool last = i == count - 1;
            const ProcessInfo& pi = processinfo[p[i]];
            PrintTreeItem(pi, TEXT(""), last);
            PrintTree(pi.dwProcessId, last ? TEXT("   ") : TEXT("│  "), processinfo, cProcesses);
        }

        for (unsigned int i = 0; i < cProcesses; i++)
            CloseHandle(processinfo[p[i]].hProcess);
    }
    else
    {
        for (unsigned int i = 0; i < cProcesses; i++)
        {
            const DWORD dwProcessId = aProcesses[i];
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
            if (hProcess == NULL)
                hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);

            if (hProcess != NULL)
            {
                PrintProcess(dwProcessId, hProcess, lpFilter);
                CloseHandle(hProcess);
            }
            else if (showAll)
                _tprintf(TEXT("%5u \n"), dwProcessId);
        }
    }

    return EXIT_SUCCESS;
}
