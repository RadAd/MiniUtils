// See http://blogs.msdn.com/b/oldnewthing/archive/tags/what+a+drag/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <Shlobj.h>
#include <Shlwapi.h>

// From http://blogs.msdn.com/b/oldnewthing/archive/2004/09/20/231739.aspx

HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr;
    LPITEMIDLIST pidl;
    SFGAOF sfgao;
    if (SUCCEEDED(hr = SHParseDisplayName(pszPath, NULL, &pidl, 0, &sfgao))) {
        IShellFolder *psf;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(hr = SHBindToParent(pidl, IID_IShellFolder,
            (void**)&psf, &pidlChild))) {
                hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
                psf->Release();
        }
        CoTaskMemFree(pidl);
    }
    return hr;
}

// ==== 

struct sFindWindowWildcard
{
    const _TCHAR* lpClassName;
    const _TCHAR* lpWindowName;
    HWND hwnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    if (hwnd != GetConsoleWindow())
    {
        sFindWindowWildcard* fww = (sFindWindowWildcard*) lParam;

        TCHAR name[1024];
        TCHAR wndclass[1024];
        GetWindowText(hwnd, name, 1024);
        GetClassName(hwnd, wndclass, 1024);

        if ((fww->lpClassName == NULL || PathMatchSpec(wndclass, fww->lpClassName))
            && (fww->lpWindowName == NULL || PathMatchSpec(name, fww->lpWindowName)))
        {
            fww->hwnd = hwnd;
            return FALSE;
        }
        else
            return TRUE;
    }
    else
        return TRUE;
}

HWND FindWindowWildcard(const _TCHAR* lpClassName, const _TCHAR* lpWindowName)
{
    //return FindWindow(lpClassName, lpWindowName);

    sFindWindowWildcard fww;
    fww.lpClassName = lpClassName;
    fww.lpWindowName = lpWindowName;
    fww.hwnd = NULL;

    if (lpClassName != NULL || lpWindowName != NULL)
        EnumWindows(EnumWindowsProc, (LPARAM) &fww);

    return fww.hwnd;
}

void DoDragDrop(IDropTarget *pdt, const std::vector<std::wstring>& files)
{
    IDataObject *pdtobj = NULL;
    for (std::vector<std::wstring>::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        // TODO Dropping one at a time. Should lookup how drop all as one.
        if (SUCCEEDED(GetUIObjectOfFile(NULL, it->c_str(), IID_IDataObject, (void**)&pdtobj))) {
            POINTL pt = { 0, 0 };
            DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;

            if (SUCCEEDED(pdt->DragEnter(pdtobj, MK_LBUTTON, pt, &dwEffect))) {
                    dwEffect &= DROPEFFECT_COPY | DROPEFFECT_LINK;
                    if (dwEffect) {
                        pdt->Drop(pdtobj, MK_LBUTTON, pt, &dwEffect);
                    } else {
                        pdt->DragLeave();
                    }
            }
            pdtobj->Release();
        }
    }
}

void DoDragDrop(HWND hWndDest, const std::vector<std::wstring>& files)
{
    size_t size = sizeof(DROPFILES) + sizeof(_TCHAR);
    for (std::vector<std::wstring>::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        size += (it->length() + 1) * sizeof(_TCHAR);
    }

    HGLOBAL hGlobal = GlobalAlloc(GHND, size);
    DROPFILES * df = static_cast<DROPFILES *>(GlobalLock(hGlobal));
    df->pFiles = sizeof(DROPFILES);
    _TCHAR* f = reinterpret_cast<_TCHAR *>(df + 1);
    size -= sizeof(DROPFILES) + sizeof(_TCHAR);
    for (std::vector<std::wstring>::const_iterator it = files.begin(); it != files.end(); ++it)
    {
        _tcscpy_s(f, it->length() + 1, it->c_str());
        size -= (it->length() + 1) * sizeof(_TCHAR);
        f += it->length() + 1;
    }
    df->fNC = TRUE;
    df->fWide = TRUE;
    GlobalUnlock(hGlobal);

    PostMessage(hWndDest, WM_DROPFILES, (WPARAM) hGlobal, 0);
    GlobalFree(hGlobal);
}

int _tmain(int argc, _TCHAR* argv[])
{
    /*HRESULT hr =*/ CoInitialize(0);

    const _TCHAR* lpClassName = NULL;
    const _TCHAR* lpWindowName = NULL;
    std::vector<std::wstring> files;

    //lpClassName = _T("ATL:006AD5B8");
    //files.push_back(_T("C:\\Windows\\ODBC.INI"));
    for (int i = 1; i < argc; ++i)
    {
        const TCHAR* arg = argv[i];

        if (_tcsncmp(arg, _T("/class:"), 7) == 0)
        {
            lpClassName = arg + 7;
        }
        else if (_tcsncmp(arg, _T("/window:"), 8) == 0)
        {
            lpWindowName = arg + 8;
        }
        else
        {
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind = FindFirstFile(arg, &FindFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    TCHAR buffer[4096];
                    if (GetFullPathName(FindFileData.cFileName, 4096, buffer, NULL) != 0)
                    {
                        files.push_back(buffer);
                    }
                } while (FindNextFile(hFind, &FindFileData));
                FindClose(hFind);
            }
        }
    }

    if (files.empty())
    {
        _tprintf(_T("Usage:\n"));
        _tprintf(_T("\tDragDrop [/class:<class name>] [/window:<window name>] <files>\n"));
    }
    else
    {
        HWND hWndDest = FindWindowWildcard(lpClassName, lpWindowName);

        if (hWndDest != NULL)
        {
            LONG ExStyle = GetWindowLong(hWndDest, GWL_EXSTYLE);

            IDropTarget *pdt = (IDropTarget *) GetProp(hWndDest, _T("OleDropTargetInterface"));
            //if (SUCCEEDED(GetUIObjectOfFile(hwnd, _T("C:\\Windows\\notepad.exe"), IID_IDropTarget, (void**)&pdt))) {

            if (pdt != NULL)
            {
                DoDragDrop(pdt, files);
            }
            else if ((ExStyle & WS_EX_ACCEPTFILES) == WS_EX_ACCEPTFILES)
            {
                DoDragDrop(hWndDest, files);
            }
            else
            {
                _tprintf(_T("Window 0x%x doesnt support drag and drop.\n"), HandleToUlong(hWndDest));
            }
        }
        else
        {
            _tprintf(_T("Could not find window with class %s and name %s.\n"), lpClassName, lpWindowName);
        }
    }

    CoUninitialize();

    return 0;
}
