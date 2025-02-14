#include <Windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>

#include "arg.inl"

// TODO: Save to file
// Save HBITMAP to file

int _tmain(int argc, TCHAR const* const* const argv)
{
    arginit(argc, argv, _T("Output clipboard contents to stdout"));

    const int mode = argswitchdesc(_T("/U"), _T("Output in unicode")) ? _O_U16TEXT : _O_U8TEXT;
    int format = _ttoi(argvaluedesc(_T("/F"), _T("0"), _T("<format>"), _T("Clipboard format")));

    if (!argcleanup())
        return 2;
    if (argusage())
    {
        _ftprintf(stdout, _T("        where " ARG_VALUE("<format>") " is " ARG_VALUE("1") " for text, " ARG_VALUE("2") " for bitmap, " ARG_VALUE("13") " for unicode\n"));
        _ftprintf(stdout, _T("\n"));
        _ftprintf(stdout, _T("\tErrorcode " ARG_VALUE("1") " if clipboard is empty.\n"));
        _ftprintf(stdout, _T("\tErrorcode " ARG_VALUE("2") " if unknown parameter.\n"));
        _ftprintf(stdout, _T("\tErrorcode " ARG_VALUE("3") " if unknown format.\n"));
        return 0;
    }

    _setmode(_fileno(stdout), mode);

    if (!OpenClipboard(NULL))
    {
        _ftprintf(stderr, _T("Error OpenClipboard: %d\n"), GetLastError());
        return 1;
    }

    if (format == 0)
    {
        UINT f[] = { CF_UNICODETEXT, CF_HDROP, CF_BITMAP };
        format = GetPriorityClipboardFormat(f, ARRAYSIZE(f));
        if (format < 0)
        {
            _ftprintf(stderr, _T("Error Unknown Format\n"));
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

                _ftprintf(stdout, _T("Bitmap: %d x %d  %d bpp\n"), bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
            }
            break;
        }

    }
    else
    {
        _ftprintf(stderr, _T("No data\n"));
        const DWORD Error = GetLastError();
        if (Error != 0)
            _ftprintf(stderr, _T("Error: %d\n"), GetLastError());
        ret = 3;
    }

    CloseClipboard();

    return ret;
}
