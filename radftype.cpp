#include <cstdio>
#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <string>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

struct WinError
{
    DWORD dwError;
    LPCTSTR szModule;

    void print(FILE *stream, LPCTSTR msg) const
    {
        HMODULE hLibrary = szModule != nullptr ? GetModuleHandle(szModule) : NULL;
        LPTSTR pMessage = nullptr;
        if (!FormatMessage((hLibrary != NULL ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM) |
                           FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           hLibrary,
                           dwError,
                           0,
                           (LPTSTR) &pMessage,
                           0,
                           NULL))
        {
            _ftprintf(stream, _T("Format message failed with 0x%08x\n"), GetLastError());
        }
        else
        {
            //_ftprintf(stream, _T("Error: 0x%08x %s %s"), dwError, msg, pMessage);
            //_ftprintf(stream, _T("%s %s"), msg, pMessage);
            msg; //ignore unused
            _ftprintf(stream, _T("%s"), pMessage);
            LocalFree(pMessage);
        }
    }
};

inline bool Empty(LPCTSTR s)
{
    return s[0] == TEXT('\0');
}

bool DisplayValue(HKEY hVerbKey, HKEY hCommandKey, LPCTSTR type)
{
    TCHAR verbname[1024] = TEXT("");
    LONG  size = ARRAYSIZE(verbname) * sizeof(TCHAR);
    DWORD retCode = RegQueryValue(hVerbKey, nullptr, verbname, &size);
    if (retCode != ERROR_SUCCESS)
        WinError({retCode}).print(stderr, TEXT("RegQueryValue"));

    TCHAR data[1024] = TEXT("");
    size = ARRAYSIZE(data) * sizeof(TCHAR);
    retCode = RegQueryValue(hCommandKey, nullptr, data, &size);
    if (retCode != ERROR_SUCCESS)
        WinError({retCode}).print(stderr, TEXT("RegQueryValue"));

    if (!Empty(data))
    {
        if (Empty(verbname))
            _ftprintf(stdout, TEXT("%s=%s\n"), type, data);
        else
            _ftprintf(stdout, TEXT("%s:%s=%s\n"), type, verbname, data);
    }

    return !Empty(data);
}

bool ShowFtype(HKEY hBaseKey, LPCTSTR type, LPCTSTR verb)
{
    HKEY hTypeKey = NULL;
    DWORD retCode = RegOpenKeyEx(hBaseKey, type, 0, KEY_READ, &hTypeKey);
    if (retCode != ERROR_SUCCESS)
        return false;

    HKEY hVerbKey = NULL;
    TCHAR subkey[100];
    _stprintf_s(subkey, TEXT("shell\\%s"), verb);
    retCode = RegOpenKeyEx(hTypeKey, subkey, 0, KEY_READ, &hVerbKey);
    if (retCode != ERROR_SUCCESS)
    {
        //WinError({retCode}).print(stderr, TEXT("RegOpenKeyEx"));
        RegCloseKey(hTypeKey);
        return false;
    }

    HKEY hCommandKey = NULL;
    retCode = RegOpenKeyEx(hVerbKey, TEXT("command"), 0, KEY_READ, &hCommandKey);
    if (retCode != ERROR_SUCCESS)
    {
        //WinError({retCode}).print(stderr, TEXT("RegOpenKeyEx"));
        RegCloseKey(hTypeKey);
        return false;
    }

    const bool valid = DisplayValue(hVerbKey, hCommandKey, type);

    RegCloseKey(hCommandKey);
    RegCloseKey(hVerbKey);
    RegCloseKey(hTypeKey);

    return valid;
}

void SetFtype(HKEY hBaseKey, LPCTSTR type, LPCTSTR verb, LPCTSTR value, LPCTSTR name)
{
    HKEY hTypeKey = NULL;
    DWORD retCode = RegCreateKey(hBaseKey, type, &hTypeKey);
    if (retCode != ERROR_SUCCESS)
    {
        WinError({retCode}).print(stderr, TEXT("RegCreateKey"));
        return;
    }

    HKEY hVerbKey = NULL;
    TCHAR subkey[100];
    _stprintf_s(subkey, TEXT("shell\\%s"), verb);
    retCode = RegCreateKey(hTypeKey, subkey, &hVerbKey);
    if (retCode != ERROR_SUCCESS)
    {
        WinError({retCode}).print(stderr, TEXT("RegCreateKey"));
        RegCloseKey(hTypeKey);
        return;
    }

    HKEY hCommandKey = NULL;
    retCode = RegCreateKey(hVerbKey, TEXT("command"), &hCommandKey);
    if (retCode != ERROR_SUCCESS)
    {
        WinError({retCode}).print(stderr, TEXT("RegCreateKey"));
        RegCloseKey(hTypeKey);
        return;
    }

    if (value)
    {
        retCode = RegSetValue(hCommandKey, nullptr, REG_SZ, value, (lstrlen(value) + 1) * sizeof(TCHAR));
        if (retCode != ERROR_SUCCESS)
            WinError({retCode}).print(stderr, TEXT("RegSetValue"));
    }
    else
    {
        retCode = RegDeleteValue(hCommandKey, nullptr);
        if (retCode != ERROR_SUCCESS && retCode != ERROR_FILE_NOT_FOUND)
            WinError({retCode}).print(stderr, TEXT("RegDeleteValue"));
    }

    if (name)
    {
        retCode = RegSetValue(hVerbKey, nullptr, REG_SZ, name, (lstrlen(name) + 1) * sizeof(TCHAR));
        if (retCode != ERROR_SUCCESS)
            WinError({retCode}).print(stderr, TEXT("RegSetValue"));
    }

    DisplayValue(hVerbKey, hCommandKey, type);

    RegCloseKey(hCommandKey);
    RegCloseKey(hVerbKey);
    RegCloseKey(hTypeKey);
}

void EnumFtype(HKEY hKey, LPCTSTR type)
{
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
    DWORD    cchClassName = MAX_PATH;  // size of class string
    DWORD    cSubKeys = 0;               // number of subkeys
    DWORD    cbMaxSubKey;              // longest subkey size
    DWORD    cchMaxClass;              // longest class string
    DWORD    cValues;              // number of values for key
    DWORD    cchMaxValue;          // longest value name
    DWORD    cbMaxValueData;       // longest value data
    DWORD    cbSecurityDescriptor; // size of security descriptor
    FILETIME ftLastWriteTime;      // last write time

    // Get the class name and the value count.
    DWORD retCode = RegQueryInfoKey(
        hKey,                    // key handle
        achClass,                // buffer for class name
        &cchClassName,           // size of class string
        NULL,                    // reserved
        &cSubKeys,               // number of subkeys
        &cbMaxSubKey,            // longest subkey size
        &cchMaxClass,            // longest class string
        &cValues,                // number of values for this key
        &cchMaxValue,            // longest value name
        &cbMaxValueData,         // longest value data
        &cbSecurityDescriptor,   // security descriptor
        &ftLastWriteTime);       // last write time
    if (retCode != ERROR_SUCCESS)
    {
        WinError({retCode}).print(stderr, TEXT("RegQueryInfoKey"));
        return ;
    }

    std::vector<TCHAR> achKey(cbMaxSubKey + 1);

    for (DWORD i = 0; i < cSubKeys; ++i)
    {
        DWORD cbName = static_cast<DWORD>(achKey.size());
        retCode = RegEnumKeyEx(hKey, i,
            achKey.data(),
            &cbName,
            NULL,
            NULL,
            NULL,
            &ftLastWriteTime);

        if (retCode != ERROR_SUCCESS)
        {
            _ftprintf(stderr, TEXT("Error enumerating key %d\n"), i);
            WinError({retCode}).print(stderr, TEXT("RegEnumKeyEx"));
            continue;
        }

        if (!achKey.empty())
            ShowFtype(hKey, achKey.data(), type);
    }
}

int _tmain(const int argc, LPCTSTR argv[])
{
    const LPCTSTR strSubKey = TEXT("Software\\Classes");
    HKEY hBaseKey = HKEY_LOCAL_MACHINE;
    std::tstring type;
    std::tstring name;
    bool setValue = false;
    bool showHelp = false;
    std::tstring value;
    std::tstring verb = TEXT("open");

    for (int argi = 1; argi < argc; ++argi)
    {
        LPCTSTR arg = argv[argi];
        if (arg[0] == TEXT('/'))
        {
            if (lstrcmpi(arg, TEXT("/U")) == 0)
                hBaseKey = HKEY_CURRENT_USER;
            else if (lstrcmpi(arg, TEXT("/V")) == 0)
            {
                verb = argv[++argi];
                size_t f = verb.find(TEXT(':'));
                if (f != std::tstring::npos)
                {
                    name = verb.substr(f + 1);
                    verb = verb.substr(0, f);
                }
            }
            else if (lstrcmpi(arg, TEXT("/?")) == 0)
                showHelp = true;
            else
                _ftprintf(stderr, TEXT("Unknown option %s\n"), arg);
        }
        else if (type.empty())
        {
            type = arg;
            size_t f = type.find(TEXT('='));
            if (f != std::tstring::npos)
            {
                setValue = true;
                value = type.substr(f + 1);
                type = type.substr(0, f);
            }
        }
        else
        {
            value += ' ';
            value += arg;
        }
    }

    if (showHelp)
    {
        _ftprintf(stdout, TEXT("Displays or modifies file types used in file extension associations\n"));
        _ftprintf(stdout, TEXT("\n"));
        _ftprintf(stdout, TEXT("FTYPE [/U] [/V <verb>] [fileType[=[openCommandString]]]\n"));
        _ftprintf(stdout, TEXT("\n"));
        _ftprintf(stdout, TEXT("  /U        Set for current user only\n"));
        _ftprintf(stdout, TEXT("  /V        Verb to use (default: open)\n"));
        _ftprintf(stdout, TEXT("  fileType  Specifies the file type to examine or change\n"));
        _ftprintf(stdout, TEXT("  openCommandString Specifies the open command to use when launching files\n"));
        _ftprintf(stdout, TEXT("                    of this type.\n"));

        return EXIT_SUCCESS;
    }

    HKEY hKey = NULL;
    DWORD retCode = RegOpenKeyEx(hBaseKey, strSubKey, 0, KEY_READ, &hKey);
    if (retCode != ERROR_SUCCESS)
    {
        _ftprintf(stderr, TEXT("Error opening key %s 0x%08X\n"), strSubKey, retCode);
        return EXIT_FAILURE;
    }

    if (!type.empty())
    {
        if (setValue)
            SetFtype(hKey, type.c_str(), verb.c_str(), value.empty() ? nullptr : value.c_str(), name.empty() ? nullptr : name.c_str());
        else
            if (!ShowFtype(hKey, type.c_str(), verb.c_str()))
                _ftprintf(stderr, TEXT("File type '%s' not found or no %s command associated with it.\n"), type.c_str(), verb.c_str());
    }
    else
        EnumFtype(hKey, verb.c_str());

    RegCloseKey(hKey);

    return EXIT_SUCCESS;
}
