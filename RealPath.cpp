#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int __cdecl _tmain(int argc, TCHAR *argv[])
{
    if (argc != 2)
    {
        _ftprintf(stderr, _T("ERROR:\tIncorrect number of arguments\n"));
        _ftprintf(stderr, _T("%s <file_name>\n"), argv[0]);
        return 1;
    }

    HANDLE hFile = CreateFile(argv[1],               // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL, // normal file
        NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("ERROR:\tCould not open file (error %d)\n"), GetLastError());
        return EXIT_FAILURE;
    }

    TCHAR Path[MAX_PATH];
    DWORD dwRet = GetFinalPathNameByHandle(hFile, Path, ARRAYSIZE(Path), VOLUME_NAME_DOS);

    if (dwRet == 0)
    {
        CloseHandle(hFile);
        _ftprintf(stderr, _T("ERROR:\tCould not get final path name (error %d)\n"), GetLastError());
        return EXIT_FAILURE;
    }

    if (_tcsnccmp(Path, _T("\\\\?\\"), 4) == 0)
        _tprintf(_T("%s\n"), Path + 4);
    else
        _tprintf(_T("%s\n"), Path);

    CloseHandle(hFile);
    return EXIT_SUCCESS;
}
