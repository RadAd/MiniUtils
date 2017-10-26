#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "user32.lib")

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
        if (OpenClipboard(hWnd))
        {
            HGLOBAL hClip = GetClipboardData(CF_UNICODETEXT);
            if (hClip != NULL)
            {
                PCWSTR const buffer = (PCWSTR) GlobalLock(hClip);
                wprintf(buffer);
                size_t len = wcslen(buffer);
                if (len > 0 && buffer[len - 1] != L'\n')
                    wprintf(L"\n");
                GlobalUnlock(hClip);
            }
            CloseClipboard();
        }
        return 0;

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
            // handle the error and possibly exit
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
