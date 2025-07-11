#include <windows.h>
#include <stdio.h>
#include <Winternl.h>
#include <tchar.h>
#include <memory>
#include <vector>
#include "arg.inl"

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

#define CHECK(x, m) if (!(x)) { _fputts((m), stderr); return GetLastError(); }

int _tmain(int argc, TCHAR *argv[])
{
    arginit(argc, argv, _T("Show command line for a process"));
    const int pid = _tstoi(argnum(1, _T("0"), _T("pid"), _T("Process id")));
	if (!argcleanup())
        return EXIT_FAILURE;
	if (argusage(pid == 0))
        return EXIT_SUCCESS;

    auto hProcess = CreateUnique(OpenProcess(
        PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
        PROCESS_VM_READ, /* required for ReadProcessMemory */
        FALSE, pid));
    CHECK(hProcess, _T("Could not open process!\n"));

    PROCESS_BASIC_INFORMATION pbi;
    CHECK(NT_SUCCESS(NtQueryInformationProcess(hProcess.get(),
        ProcessBasicInformation,
        &pbi, sizeof(pbi), NULL)),
        _T("Could not read process basic information!\n"));

    PRTL_USER_PROCESS_PARAMETERS rtlUserProcParamsAddress;
    CHECK(ReadProcessMemory(hProcess.get(),
        &(pbi.PebBaseAddress->ProcessParameters),
        &rtlUserProcParamsAddress, sizeof(rtlUserProcParamsAddress), NULL),
        _T("Could not read the address of ProcessParameters!\n"));

    UNICODE_STRING commandLine;
    CHECK(ReadProcessMemory(hProcess.get(),
        &(rtlUserProcParamsAddress->CommandLine),
        &commandLine, sizeof(commandLine), NULL),
        _T("Could not read CommandLine!\n"));

    std::vector<WCHAR> commandLineContents(StrLen(&commandLine));
    CHECK(ReadProcessMemory(hProcess.get(), commandLine.Buffer,
        commandLineContents.data(), commandLine.Length, NULL),
        _T("Could not read the command line string!\n"));

    fwprintf(stdout, L"%.*s\n", static_cast<int>(commandLineContents.size()), commandLineContents.data());

    return ERROR_SUCCESS;
}
