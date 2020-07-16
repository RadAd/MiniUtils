#include <Windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "user32.lib")

int wmain(int argc, const wchar_t const* const* argv)
{
    int mode = _O_U8TEXT;
    int format = 0;

    bool error = false;
    bool showUsage = false;

    for (int i = 1; i < argc; ++i)
    {
        const wchar_t const* arg = argv[i];
        if (wcscmp(arg, L"/?") == 0)
            showUsage = true;
        else if (wcscmp(arg, L"/U") == 0)
            mode = _O_U16TEXT;
        else if (wcsncmp(arg, L"/F:", 3) == 0)
            format = _wtoi(arg + 3);
        else
        {
            fwprintf(stderr, L"Unknown argument: %s\n", arg);
            error = true;
        }
    }

    if (showUsage)
    {
        fwprintf(stdout, L"RadClip - output the clipboard contents\n");
        fwprintf(stdout, L"\n");
        fwprintf(stdout, L"RadClip [/U] [/F:n]\n");
        fwprintf(stdout, L"   /U    Output in unicode\n");
        fwprintf(stdout, L"   /F    Clipbaord format\n");
        fwprintf(stdout, L"         where n is 1 for text, 2 for bitmap, 13 for unicode\n");
        fwprintf(stdout, L"\n");
        fwprintf(stdout, L"\tErrorcode 1 if clipboard is empty.\n");
        fwprintf(stdout, L"\tErrorcode 2 if unknown parameter.\n");
        fwprintf(stdout, L"\tErrorcode 3 if unknown format.\n");
        return 0;
    }

    if (error)
        return 2;

    _setmode(_fileno(stdout), mode);

    if (!OpenClipboard(NULL))
    {
        fwprintf(stderr, L"Error OpenClipboard: %d\n", GetLastError());
        return 1;
    }

    if (format == 0)
    {
        UINT f[] = { CF_UNICODETEXT, CF_HDROP, CF_BITMAP };
        format = GetPriorityClipboardFormat(f, ARRAYSIZE(f));
        if (format < 0)
        {
            fwprintf(stderr, L"Error Unknown Format\n");
            CloseClipboard();
            return 3;
        }
    }

    int ret = 0;
    HANDLE hClipboardData = GetClipboardData(format);
    if (hClipboardData != NULL)
    {
        switch (format)
        {
        case CF_UNICODETEXT:
            {
                wchar_t *pchData = (wchar_t*)GlobalLock(hClipboardData);

#if 1
                // Printing "\r\n" causes output of "\r\r\n"
                wchar_t *pchNext;
                while ((pchNext = wcschr(pchData, L'\r')) != nullptr)
                {
                    size_t len = pchNext - pchData;
                    wprintf(L"%.*s", (int) len, pchData);
                    if (pchNext[1] != L'\n')
                        wprintf(L"\r");
                    pchData = pchNext + 1;
                }
                wprintf(pchData);
                size_t len = wcslen(pchData);
                if (len > 0 && pchData[len - 1] != L'\n')
                    wprintf(L"\n");
#else
                wchar_t bom = 0xFFFE;
                fwrite(&bom, sizeof(wchar_t), 1, stdout);
                fwrite(pchData, sizeof(wchar_t), wcslen(pchData), stdout);
#endif

                GlobalUnlock(hClipboardData);
            }
            break;

        case CF_HDROP:
            {
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
            break;

        case CF_BITMAP:
            {
                HBITMAP hBmp = (HBITMAP) hClipboardData;

                BITMAP bm = {};
                GetObject(hBmp, sizeof(BITMAP), &bm);

                fwprintf(stdout, L"Bitmap: %d x %d  %d bpp\n", bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
            }
            break;
        }

    }
    else
    {
        fwprintf(stderr, L"No data\n");
        DWORD Error = GetLastError();
        if (Error != 0)
            fwprintf(stderr, L"Error: %d\n", GetLastError());
        ret = error;
    }

    CloseClipboard();

    return ret;
}
