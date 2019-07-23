#include <windows.h>
#include <stdio.h>
#include <Winternl.h>
#include <tchar.h>
#include <memory>
#include <vector>

template <class T>
auto CreateUnique(HANDLE h, T* f)
{
    return std::unique_ptr<std::remove_pointer<HANDLE>::type, T*>(h, f);
}

auto CreateUnique(HANDLE h)
{
    return CreateUnique(h, CloseHandle);
}

inline int StrLen(const PUNICODE_STRING s)
{
    return (int) (s->Length / sizeof(WCHAR));
}

int _tmain(int argc, TCHAR *argv[])
{
    if (argc < 2)
    {
        _fputts(_T("Usage: GetCmdLine [pid]\n"), stderr);
        return ERROR_INVALID_FUNCTION;
    }

    int pid = _tstoi(argv[1]);

    auto hProcess = CreateUnique(OpenProcess(
        PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
        PROCESS_VM_READ, /* required for ReadProcessMemory */
        FALSE, pid));
    if (!hProcess)
    {
        _fputts(_T("Could not open process!\n"), stderr);
        return GetLastError();
    }

    PROCESS_BASIC_INFORMATION pbi;
    if (!NT_SUCCESS(NtQueryInformationProcess(hProcess.get(), ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
    {
        _fputts(_T("Could not read process basic information!\n"), stderr);
        return GetLastError();
    }

    PRTL_USER_PROCESS_PARAMETERS rtlUserProcParamsAddress;
    if (!ReadProcessMemory(hProcess.get(),
        &(pbi.PebBaseAddress->ProcessParameters),
        &rtlUserProcParamsAddress, sizeof(rtlUserProcParamsAddress), NULL))
    {
        _fputts(_T("Could not read the address of ProcessParameters!\n"), stderr);
        return GetLastError();
    }

    UNICODE_STRING commandLine;
    if (!ReadProcessMemory(hProcess.get(),
        &(rtlUserProcParamsAddress->CommandLine),
        &commandLine, sizeof(commandLine), NULL))
    {
        _fputts(_T("Could not read CommandLine!\n"), stderr);
        return GetLastError();
    }

    std::vector<WCHAR> commandLineContents(StrLen(&commandLine));

    if (!ReadProcessMemory(hProcess.get(), commandLine.Buffer,
        commandLineContents.data(), commandLine.Length, NULL))
    {
        _fputts(_T("Could not read the command line string!\n"), stderr);
        return GetLastError();
    }

    fwprintf(stdout, L"%.*s\n", StrLen(&commandLine), commandLineContents.data());

    return ERROR_SUCCESS;
}
