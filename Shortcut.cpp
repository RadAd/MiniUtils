#include <windows.h>
#include <comdef.h>
#include <shlobj.h>
#include <atlbase.h>
#include <tchar.h>

// TODO
// Set icon index either as another parameter or using the comma notation

#define CHK_HR(stmt)        do { HRESULT hr=(stmt); if (FAILED(hr)) throw _com_error(hr); } while(0)

class CoInit
{
public:
    CoInit()
    {
        CHK_HR(CoInitialize(NULL));
    }

    ~CoInit()
    {
        CoUninitialize();
    }
};

void DisplayUsage()
{
    _ftprintf(stdout, _T("Shortcut [shortcutfile] [options]\n"));
    _ftprintf(stdout, _T("\n"));
    _ftprintf(stdout, _T("where [options] are:\n"));
    _ftprintf(stdout, _T("\t/t [target] - set target\n"));
    _ftprintf(stdout, _T("\t/a [args]   - set arguments\n"));
    _ftprintf(stdout, _T("\t/-a         - clear arguments\n"));
    _ftprintf(stdout, _T("\t/d [desc]   - set description\n"));
    _ftprintf(stdout, _T("\t/-d         - clear description\n"));
    _ftprintf(stdout, _T("\t/i [icon]   - set icon\n"));
    _ftprintf(stdout, _T("\t/-i         - clear icon\n"));
    _ftprintf(stdout, _T("\t/w [dir]    - set working directory\n"));
    _ftprintf(stdout, _T("\t/-w         - clear working directory\n"));
    _ftprintf(stdout, _T("\t/r          - set runas user\n"));
    _ftprintf(stdout, _T("\t/-r         - clear runas user\n"));
}

int DoShortcut(int argc, const TCHAR *argv[])
{
    int arg = 1;
#if UNICODE
    const TCHAR *file(argv[arg++]);
#else
    const char *filea(argv[arg++]);
    wchar_t file[MAX_PATH];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, filea, -1, file, MAX_PATH);
#endif
    bool changed = false;

    CComPtr<IPersistFile> sppf;
    CHK_HR(sppf.CoCreateInstance(CLSID_ShellLink));
    if (PathFileExistsW(file))
        CHK_HR(sppf->Load(file, STGM_READWRITE));

    CComQIPtr<IShellLink> spsl(sppf);
    if (!spsl)
    {
        _ftprintf(stderr, TEXT("Errror casting to ShellLink\n"));
        return -1;
    }
    CComQIPtr<IShellLinkDataList, &IID_IShellLinkDataList> spdl(sppf);
    if (!spdl)
    {
        _ftprintf(stderr, TEXT("Errror casting to ShellLinkDataList\n"));
        return -1;
    }

    DWORD dwFlags;
    CHK_HR(spdl->GetFlags(&dwFlags));

    {
        TCHAR Data[MAX_PATH];
        int IntData;
        CHK_HR(spsl->GetPath(Data, ARRAYSIZE(Data), NULL, SLGP_RAWPATH));
        _ftprintf(stdout, _T("Target: %s\n"), Data);
        if (dwFlags & SLDF_HAS_ARGS)
        {
            CHK_HR(spsl->GetArguments(Data, ARRAYSIZE(Data)));
            _ftprintf(stdout, _T("Args: %s\n"), Data);
        }
        CHK_HR(spsl->GetDescription(Data, ARRAYSIZE(Data)));
        _ftprintf(stdout, _T("Desc: %s\n"), Data);
        if (dwFlags & SLDF_HAS_WORKINGDIR)
        {
            CHK_HR(spsl->GetWorkingDirectory(Data, ARRAYSIZE(Data)));
            _ftprintf(stdout, _T("Dir: %s\n"), Data);
        }
        if (dwFlags & SLDF_HAS_ICONLOCATION)
        {
            CHK_HR(spsl->GetIconLocation(Data, ARRAYSIZE(Data), &IntData));
            _ftprintf(stdout, _T("Icon: %s,%d\n"), Data, IntData);
        }
        //spsl->GetHotkey
        //spsl->GetShowCommand
    }

    while (arg < argc)
    {
        const TCHAR *command = argv[arg++];

        if (_tcscmp(command, TEXT("/t")) == 0)
        {
            const TCHAR *target = argv[arg++];
            CHK_HR(spsl->SetPath(target));

            TCHAR Data[MAX_PATH];
            CHK_HR(spsl->GetPath(Data, ARRAYSIZE(Data), NULL, SLGP_RAWPATH));
            _ftprintf(stdout, _T("+Target: %s\n"), Data);
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/a")) == 0)
        {
            const TCHAR *args = argv[arg++];
            CHK_HR(spsl->SetArguments(args));

            TCHAR Data[MAX_PATH];
            CHK_HR(spsl->GetArguments(Data, ARRAYSIZE(Data)));
            _ftprintf(stdout, _T("+Args: %s\n"), Data);
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/-a")) == 0)
        {
            CHK_HR(spsl->SetArguments(NULL));
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/w")) == 0)
        {
            const TCHAR *dir = argv[arg++];
            CHK_HR(spsl->SetWorkingDirectory(dir));

            TCHAR Data[MAX_PATH];
            CHK_HR(spsl->GetWorkingDirectory(Data, ARRAYSIZE(Data)));
            _ftprintf(stdout, _T("+Dir: %s\n"), Data);
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/-w")) == 0)
        {
            CHK_HR(spsl->SetWorkingDirectory(NULL));

            changed = true;
        }
        else if (_tcscmp(command, TEXT("/i")) == 0)
        {
            const TCHAR *icon = argv[arg++];
            TCHAR Data[MAX_PATH];
            _tcscpy_s(Data, icon);
            int Index = PathParseIconLocation(Data);
            CHK_HR(spsl->SetIconLocation(Data, Index));

            CHK_HR(spsl->GetIconLocation(Data, ARRAYSIZE(Data), &Index));
            _ftprintf(stdout, _T("+Icon: %s,%d\n"), Data, Index);
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/-i")) == 0)
        {
            CHK_HR(spsl->SetIconLocation(NULL, 0));

            changed = true;
        }
        else if (_tcscmp(command, TEXT("/d")) == 0)
        {
            const TCHAR *desc = argv[arg++];
            CHK_HR(spsl->SetDescription(desc));

            TCHAR Data[MAX_PATH];
            CHK_HR(spsl->GetDescription(Data, ARRAYSIZE(Data)));
            _ftprintf(stdout, _T("+Desc: %s\n"), Data);
            changed = true;
        }
        else if (_tcscmp(command, TEXT("/-d")) == 0)
        {
            CHK_HR(spsl->SetDescription(NULL));

            changed = true;
        }
        else if (_tcscmp(command, TEXT("/r")) == 0)
        {
            dwFlags |= SLDF_RUNAS_USER;
            CHK_HR(spdl->SetFlags(dwFlags));

            changed = true;
        }
        else if (_tcscmp(command, TEXT("/-r")) == 0)
        {
            dwFlags &= ~SLDF_RUNAS_USER;
            CHK_HR(spdl->SetFlags(dwFlags));

            changed = true;
        }
        else
        {
            _ftprintf(stderr, TEXT("Unknown option: %s\n"), command);
        }
    }

    if (changed)
        CHK_HR(sppf->Save(file, TRUE));

    return 0;
}

int _tmain(int argc, const TCHAR *argv[])
{
    CoInit ci;
    try
    {
        if (argc < 2)
            DisplayUsage();
        else
            return DoShortcut(argc, argv);
    }
    catch (_com_error& e)
    {
        fwprintf(stderr, L"%s\n", static_cast<const wchar_t*>(e.Description()));
        return e.Error();
    }
    catch(...)
    {
        _ftprintf(stderr, _T("Unknown Error\n"));
        return -1;
    }
    return 0;
};
