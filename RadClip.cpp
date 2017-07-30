#include <Windows.h>
#include <stdio.h>
#include <shlobj.h>

#pragma comment(lib, "user32.lib")

int wmain(int argc, wchar_t** argv)
{
    bool error = false;
    bool showUsage = false;

    for (int i = 1; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"/?") == 0)
            showUsage = true;
        else
        {
            fwprintf(stderr, L"Unknown argument: %s\n", argv[i]);
            error = true;
        }
    }
    
    if (showUsage)
    {
        fwprintf(stdout, L"RadClip - output the clipboard contents\n");
        fwprintf(stdout, L"\n");
        fwprintf(stdout, L"\tErrorcode 1 if clipboard is empty.\n");
        return 0;
    }
    
    if (error)
        return -2;

    if (!OpenClipboard(NULL))
    {
        fwprintf(stderr, L"Error OpenClipboard: %d\n", GetLastError());
        return -1;
    }

    HANDLE hClipboardData = NULL;
    int ret = 1;
    
    // Output text if it exists
    hClipboardData = GetClipboardData(CF_UNICODETEXT);
    if (hClipboardData != NULL)
    {
        ret = 0;
        wchar_t *pchData = (wchar_t*)GlobalLock(hClipboardData);

        wprintf(pchData);
        wprintf(L"\n");

        GlobalUnlock(hClipboardData);
    }
    else
    {
        DWORD Error = GetLastError();
        if (Error != 0)
            fwprintf(stderr, L"Error: %d\n", GetLastError());
    }

    // Output filenames if they exist
    hClipboardData = GetClipboardData(CF_HDROP);
    if (hClipboardData != NULL)
    {
        ret = 0;
        DROPFILES* pData = (DROPFILES*)GlobalLock(hClipboardData);

        if (pData->fWide)
        {
            wchar_t *pchData = (wchar_t*)((char*)pData + pData->pFiles);
            while (*pchData != L'\0')
            {
                wprintf(pchData);
                wprintf(L"\n");
                pchData += wcslen(pchData) + 1;
            }
        }
        else
        {
            char *pchData = (char*)((char*)pData + pData->pFiles);
            while (*pchData != L'\0')
            {
                printf(pchData);
                printf("\n");
                pchData += strlen(pchData) + 1;
            }
        }

        GlobalUnlock(hClipboardData);
    }
    else
    {
        DWORD Error = GetLastError();
        if (Error != 0)
            fwprintf(stderr, L"Error: %d\n", GetLastError());
    }

    CloseClipboard();

    return ret;
}
