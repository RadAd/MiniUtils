#include <stdio.h>
#include <windows.h>
#include <Shlobj.h>

bool DisplayClipboard(const UINT format)
{
    bool found = false;
    HGLOBAL hClip = GetClipboardData(format);
    if (hClip != NULL)
    {
        found = true;
        switch (format)
        {
        default:
            found = false;
            break;

        case CF_OEMTEXT:
        case CF_TEXT:
            {
                PCSTR const buffer = (PCSTR) GlobalLock(hClip);
                puts(buffer);
                GlobalUnlock(hClip);

            }
            break;

        case CF_UNICODETEXT:
            {
                PCWSTR const buffer = (PCWSTR) GlobalLock(hClip);
                _putws(buffer);
                GlobalUnlock(hClip);
            }
            break;

        case CF_BITMAP:
            {
                HBITMAP hBitmap = (HBITMAP) hClip;
                BITMAP bmp;
                GetObject(hBitmap, sizeof(bmp), &bmp);
                wprintf(L"Bitmap: %dx%d bpp:%d\n", bmp.bmWidth, bmp.bmHeight, bmp.bmBitsPixel);
                GlobalUnlock(hClip);
            }
            break;

        case CF_DIB:
            {
                BITMAPINFO* pBitmapInfo = (BITMAPINFO*) GlobalLock(hClip);
                wprintf(L"DIB: %dx%d bitcount:%d\n", pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight, pBitmapInfo->bmiHeader.biBitCount);
                GlobalUnlock(hClip);
            }
            break;

        case CF_HDROP:
            {
                DROPFILES* pData = (DROPFILES*) GlobalLock(hClip);

                if (pData->fWide)
                {
                    wchar_t *pchData = (wchar_t*) ((char*) pData + pData->pFiles);
                    while (*pchData != L'\0')
                    {
                        wprintf(pchData);
                        wprintf(L"\n");
                        pchData += wcslen(pchData) + 1;
                    }
                }
                else
                {
                    char *pchData = (char*) ((char*) pData + pData->pFiles);
                    while (*pchData != L'\0')
                    {
                        printf(pchData);
                        printf("\n");
                        pchData += strlen(pchData) + 1;
                    }
                }

                GlobalUnlock(hClip);
            }
            break;
        }
    }
    return found;
}

BOOL OpenClipboardRetry(HWND hWndNewOwner)
{
    BOOL r;
    while (!(r = OpenClipboard(hWndNewOwner)))
    {
        if (GetLastError() != ERROR_ACCESS_DENIED)
            break;
        Sleep(100);
    }
    return r;
}

LRESULT CALLBACK ClipWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        AddClipboardFormatListener(hWnd);
        return TRUE;

    case WM_DESTROY:
        RemoveClipboardFormatListener(hWnd);
        return TRUE;

    case WM_CLIPBOARDUPDATE:
        if (OpenClipboardRetry(hWnd))
        {
            bool found = false;
            int index = 0;
            UINT format;
            while (!found && (format = EnumClipboardFormats(index)) > 0)
            {
                //wprintf(L"Format: %#.4x\n", format);
                found = DisplayClipboard(format);
                ++index;
            }
            if (!found)
            {
                UINT f[] = { CF_UNICODETEXT, CF_BITMAP, CF_HDROP };
                format = GetPriorityClipboardFormat(f, ARRAYSIZE(f));
                if (format > 0)
                {
                    //wprintf(L"Priority Format: %#.4x\n", format);
                    DisplayClipboard(format);
                }
            }
            _putws(L"---");
            CloseClipboard();
        }
        else
            printf("Error OpenClipboard: %d\n", GetLastError());
        return !0;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    };
}

ATOM Register(HINSTANCE hInst)
{
    WNDCLASSEX wx = {};
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = ClipWndProc;
    wx.hInstance = hInst;
    wx.lpszClassName = TEXT("RADCLIPWATCH");
    return RegisterClassEx(&wx);
}

void main()
{
    HINSTANCE hInst = GetModuleHandle(NULL);
    ATOM a = Register(hInst);
    /*HWND hWnd =*/ CreateWindow((LPCTSTR) a, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, nullptr);

    BOOL bRet;
    MSG msg;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            fwprintf(stderr, L"Error: GetMessage %d\n", GetLastError());
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
