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
            strcpy_s(lpUser, nSizeUser, "NONE_MAPPED");
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

int main(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv);
    bool showAll = argswitch(_T("/a"));
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
    
    HANDLE hProcesses[1024];
    for (unsigned int i = 0; i < cProcesses; i++)
    {
        hProcesses[i] = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        if (hProcesses[i] == NULL)
            hProcesses[i] = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION  , FALSE, aProcesses[i]);
    }
    for (unsigned int i = 0; i < cProcesses; i++)
        if (hProcesses[i] != NULL)
            PrintProcess(aProcesses[i], hProcesses[i], lpFilter);
        else if (showAll)
            _tprintf(TEXT("%5u \n"), aProcesses[i]);
    for (unsigned int i = 0; i < cProcesses; i++)
        CloseHandle(hProcesses[i]);

    return EXIT_SUCCESS;
}
