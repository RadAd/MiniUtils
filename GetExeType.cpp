#include <Windows.h>
#include <ShellApi.h>
#include <cstdio>
#include <tchar.h>

int _tmain(const int /*argc*/, const TCHAR* const argv[])
{
    //_ftprintf(stderr, TEXT("File: %s\n"), argv[1]);
    const DWORD Type = (DWORD) SHGetFileInfo(argv[1], 0, nullptr, 0, SHGFI_EXETYPE);
    const DWORD Error = GetLastError();
    if (Error != 0)
        _ftprintf(stderr, TEXT("SHGetFileInfo Error: 0x%08X\n"), Error);

    if (LOWORD(Type) == MAKEWORD('P', 'E') && HIWORD(Type) == 0)
        _ftprintf(stdout, TEXT("Console application\n"));
    else if (LOWORD(Type) == MAKEWORD('P', 'E'))
        _ftprintf(stdout, TEXT("Windows application\n"));
    else if (LOWORD(Type) == MAKEWORD('M', 'Z') && HIWORD(Type) == 0)
        _ftprintf(stdout, TEXT("MS-DOS application\n"));
    else
        _ftprintf(stdout, TEXT("Unknown type\n"));

    return EXIT_SUCCESS;
}
