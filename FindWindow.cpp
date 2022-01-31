#include <windows.h>
#include <shlwapi.h>
#include <cstdio>
#include <tchar.h>

#include "arg.inl"

struct FindWindowPlusData
{
    LPCTSTR lpClassName;
    LPCTSTR lpWindowName;
    HWND hWnd;
};

BOOL CALLBACK FindWindowPlusEnumWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam)
{
    FindWindowPlusData* data = (FindWindowPlusData*) lParam;
    if (data->lpClassName != nullptr)
    {
        TCHAR strClassName[MAX_PATH];
        if (!GetClassName(hWnd, strClassName, ARRAYSIZE(strClassName)))
            return TRUE;
        
        if (!PathMatchSpec(strClassName, data->lpClassName))
            return TRUE;
            
        //_tprintf(_T("Class: %s\n"), strClassName);
    }
    if (data->lpWindowName != nullptr)
    {
        TCHAR strWindowName[MAX_PATH];
        if (!GetWindowText(hWnd, strWindowName, ARRAYSIZE(strWindowName)))
            return TRUE;
        
        if (!PathMatchSpec(strWindowName, data->lpWindowName))
            return TRUE;
            
        //_tprintf(_T("Title: %s\n"), strWindowName);
    }
    
    data->hWnd = hWnd;
    _tprintf(_T("HWND: 0x%08X\n"), HandleToUlong(hWnd));
    return TRUE;
}

HWND FindWindowPlus(LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
    FindWindowPlusData data = { lpClassName, lpWindowName };
    EnumWindows(FindWindowPlusEnumWindowsProc, (LPARAM) &data);
    return data.hWnd;
}

int _tmain(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv);
    LPCTSTR lpClassName = argvalue(_T("/class"));
    LPCTSTR lpWindowName = argvalue(_T("/title"));
	if (!argcleanup())
        return EXIT_FAILURE;
        
    if (lpClassName == nullptr && lpWindowName == nullptr)
    {
        _ftprintf(stderr, _T("%s [/class=<class>] [/title=<title>]\n"), argapp());
        return EXIT_FAILURE;
    }
    else
    {
        const HWND hWnd = FindWindowPlus(lpClassName, lpWindowName);
        if (hWnd != NULL)
        {
            //_tprintf(_T("HWND: 0x%08X\n"), HandleToUlong(hWnd));
            return EXIT_SUCCESS;
        }
        else if (GetLastError() == 0)
        {
            _tprintf(_T("Error: Not found\n"));
            return EXIT_FAILURE;
        }
        else
        {
            _tprintf(_T("Error: %d\n"), GetLastError());
            return EXIT_FAILURE;
        }
    }
}
