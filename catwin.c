#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#include "arg.inl"

#define FILE_ERR ((HANDLE) -1)

void Process(HANDLE hInput, HANDLE hOutput)
{
    BYTE buffer[1024];

    while (TRUE)
    {
        DWORD dwBytesRead = 0;
        BOOL fSuccess = ReadFile(hInput, buffer, (DWORD) ARRAYSIZE(buffer) * sizeof(BYTE), &dwBytesRead, NULL);

        if (!fSuccess || dwBytesRead == 0)
            break;

        DWORD dwBytesWritten = 0;
        fSuccess = WriteFile(hOutput, buffer, dwBytesRead, &dwBytesWritten, NULL);
        if (!fSuccess)
            break;
    }
}

HANDLE argfile(int i, const TCHAR* descvalue, const TCHAR* desc)
{
    const TCHAR* name = argnum(i, NULL, descvalue, desc);
    if (name == NULL)
        return NULL;
    else if (_tcscmp(name, _T("-")) == 0)
        return GetStdHandle(STD_INPUT_HANDLE);
    else
    {
        HANDLE hPipe = NULL;

        while (1)
        {
            hPipe = CreateFile(
                name,            // pipe name 
                GENERIC_READ,
                0,              // no sharing 
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe 
                0,              // default attributes 
                NULL);          // no template file 

            // Break if the pipe handle is valid. 

            if (hPipe != INVALID_HANDLE_VALUE)
                break;

            // Exit if an error other than ERROR_PIPE_BUSY occurs. 

            if (GetLastError() != ERROR_PIPE_BUSY)
            {
                _ftprintf(stderr, TEXT("Error(%d): opening file\n"), GetLastError());
                hPipe = FILE_ERR;
                break;
            }

            // All pipe instances are busy, so wait for 20 seconds. 

            if (!WaitNamedPipe(name, 20000))
            {
                _ftprintf(stderr, TEXT("Error waiting for pipe: 20 second wait timed out."));
                hPipe = FILE_ERR;
                break;
            }
        }

        return hPipe;
    }
}

int _tmain(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv, _T("Read and output a file to stdout using windows file functions"));
    HANDLE i = argfile(1, _T("file"), _T("The file to read ('-' for stdin)"));
    HANDLE o = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!argcleanup())
        return EXIT_FAILURE;
	if (argusage(i == NULL))
        return EXIT_SUCCESS;

    if (i == FILE_ERR)
        return EXIT_FAILURE;

    int ret = EXIT_SUCCESS;
    Process(i, o);

    if (GetLastError() == ERROR_BROKEN_PIPE)
    {
        _fputts(TEXT("Client disconnected.\n"), stderr);
    }
    else
    {
        _ftprintf(stderr, TEXT("Process failed %d\n"), GetLastError());
        ret = EXIT_FAILURE;
    }

    if (i != GetStdHandle(STD_INPUT_HANDLE))
        CloseHandle(i);

    return EXIT_SUCCESS;
}
