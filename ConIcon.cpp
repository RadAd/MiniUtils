#include <Windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

int main(int argc, TCHAR* argv[])
{
    TCHAR sIcon[MAX_PATH] = _T("");
    
    for (int arg = 1; arg < argc; ++arg)
    {
        if (sIcon[0] == _T('\0'))
            _tcscpy_s(sIcon, argv[arg]);
        else
            _ftprintf(stderr, _T("Unknown argument: %s\n"), argv[arg]);
    }
    
    if (sIcon[0] != _T('\0'))
    {
        WORD iIndex = (WORD) PathParseIconLocation(sIcon);

        HINSTANCE hInstance = GetModuleHandle(NULL);
#if 1
        HICON hIcon = ExtractAssociatedIcon(hInstance, sIcon, &iIndex);
#else
        HICON hIcon = NULL;
        PTSTR sExt = PathFindExtension(sIcon);
        if (_tcsicmp(sExt, _T(".ico")) == 0)
            hIcon = (HICON) LoadImage(NULL, sIcon, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        else if (_tcsicmp(sExt, _T(".exe")) == 0 || _tcsicmp(sExt, _T(".dll")) == 0)
            hIcon = ExtractIcon(hInstance, sIcon, iIndex);
        else
        {
            _ftprintf(stderr, _T("Unknown extension: %s\n"), sExt);
            return 1;
        }
#endif
        
        if (hIcon == NULL)
        {
            _ftprintf(stderr, _T("Unable to load icon.\n"));
            return 2;
        }
        
        HWND hWnd = GetConsoleWindow();
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
        //DestroyIcon(hIcon);
    }
    else
    {
        // TODO Usage
    }
    
    return 0;
}
