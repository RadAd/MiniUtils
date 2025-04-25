#include <cstdio>
#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <memory>

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

template <>
class std::default_delete<HKEY>
{
public:
    typedef HKEY pointer;
    void operator()(HKEY k) const
    {
        RegCloseKey(k);
    }
};

bool Check(LSTATUS retCode, LPCTSTR msg)
{
    if (retCode != ERROR_SUCCESS)
    {
        if (msg)
            WinError({DWORD(retCode)}).print(stderr, msg);
        return false;
    }
        return true;
}

#define CHECK(x) Check(x, TEXT(#x))
#define CHECK_RET(x, r) if (!Check(x, TEXT(#x))) return r
#define CHECK_MSG_RET(x, m, r) if (!Check(x, m)) return r
#define CHECK_CONT(x) if (!Check(x, TEXT(#x))) continue

inline LSTATUS RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, std::unique_ptr<HKEY>* phResultKey)
{
    HKEY hResultKey = NULL;
    LSTATUS retCode = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, &hResultKey);
    if (retCode == ERROR_SUCCESS)
        phResultKey->reset(hResultKey);
    return retCode;
}

inline LSTATUS RegCreateKey(HKEY hKey, LPCTSTR lpSubKey, std::unique_ptr<HKEY>* phResultKey)
{
    HKEY hResultKey = NULL;
    LSTATUS retCode = RegCreateKey(hKey, lpSubKey, &hResultKey);
    if (retCode == ERROR_SUCCESS)
        phResultKey->reset(hResultKey);
    return retCode;
}

bool DisplayValue(LPCTSTR verb, HKEY hCommandKey, LPCTSTR type)
{
    TCHAR data[1024] = TEXT("");
    LONG size = ARRAYSIZE(data) * sizeof(TCHAR);
    CHECK(RegQueryValue(hCommandKey, nullptr, data, &size));

    if (!Empty(data))
    {
        if (Empty(verb))
            _ftprintf(stdout, TEXT("%s=%s\n"), type, data);
        else
            _ftprintf(stdout, TEXT("%s:%s=%s\n"), type, verb, data);
    }

    return !Empty(data);
}

bool ShowFtype(HKEY hBaseKey, LPCTSTR type, LPCTSTR verb)
{
    std::unique_ptr<HKEY> hTypeKey;
    CHECK_MSG_RET(RegOpenKeyEx(hBaseKey, type, 0, KEY_READ, &hTypeKey), nullptr, false);

    std::unique_ptr<HKEY> hVerbKey;
    TCHAR subkey[100];
    _stprintf_s(subkey, TEXT("shell\\%s"), verb);
    CHECK_MSG_RET(RegOpenKeyEx(hTypeKey.get(), subkey, 0, KEY_READ, &hVerbKey), nullptr, false);

    std::unique_ptr<HKEY> hCommandKey;
    CHECK_MSG_RET(RegOpenKeyEx(hVerbKey.get(), TEXT("command"), 0, KEY_READ, &hCommandKey), nullptr, false);

    const bool valid = DisplayValue(verb, hCommandKey.get(), type);

    return valid;
}

void SetFtype(HKEY hBaseKey, LPCTSTR type, LPCTSTR verb, LPCTSTR value, LPCTSTR name)
{
    std::unique_ptr<HKEY> hTypeKey;
    CHECK_RET(RegCreateKey(hBaseKey, type, &hTypeKey), ;);

    std::unique_ptr<HKEY> hVerbKey;
    TCHAR subkey[100];
    _stprintf_s(subkey, TEXT("shell\\%s"), verb);
    CHECK_RET(RegCreateKey(hTypeKey.get(), subkey, &hVerbKey), ;);

    std::unique_ptr<HKEY> hCommandKey;
    CHECK_RET(RegCreateKey(hVerbKey.get(), TEXT("command"), &hCommandKey), ;);

    if (value)
    {
        CHECK(RegSetValue(hCommandKey.get(), nullptr, REG_SZ, value, (lstrlen(value) + 1) * sizeof(TCHAR)));
    }
    else
    {
        CHECK(RegDeleteValue(hCommandKey.get(), nullptr));
    }

    if (name)
    {
        CHECK(RegSetValue(hVerbKey.get(), nullptr, REG_SZ, name, (lstrlen(name) + 1) * sizeof(TCHAR)));
    }

    DisplayValue(verb, hCommandKey.get(), type);
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
    CHECK_RET(RegQueryInfoKey(
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
        &ftLastWriteTime),       // last write time
        ;);

    std::vector<TCHAR> achKey(cbMaxSubKey + 1);

    for (DWORD i = 0; i < cSubKeys; ++i)
    {
        DWORD cbName = static_cast<DWORD>(achKey.size());
        CHECK_CONT(RegEnumKeyEx(hKey, i,
            achKey.data(),
            &cbName,
            NULL,
            NULL,
            NULL,
            &ftLastWriteTime));

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
        _ftprintf(stdout, TEXT("FTYPE [/U] [/V <verb>[:<name>]] [fileType[=[openCommandString]]]\n"));
        _ftprintf(stdout, TEXT("\n"));
        _ftprintf(stdout, TEXT("  /U        Set for current user only\n"));
        _ftprintf(stdout, TEXT("  /V        Verb to use (default: open)\n"));
        _ftprintf(stdout, TEXT("  fileType  Specifies the file type to examine or change\n"));
        _ftprintf(stdout, TEXT("  openCommandString Specifies the open command to use when launching files\n"));
        _ftprintf(stdout, TEXT("                    of this type.\n"));

        return EXIT_SUCCESS;
    }

    std::unique_ptr<HKEY> hKey;
    CHECK_RET(RegOpenKeyEx(hBaseKey, strSubKey, 0, KEY_READ, &hKey), EXIT_FAILURE);

    if (!type.empty())
    {
        if (setValue)
            SetFtype(hKey.get(), type.c_str(), verb.c_str(), value.empty() ? nullptr : value.c_str(), name.empty() ? nullptr : name.c_str());
        else
            if (!ShowFtype(hKey.get(), type.c_str(), verb.c_str()))
                _ftprintf(stderr, TEXT("File type '%s' not found or no %s command associated with it.\n"), type.c_str(), verb.c_str());
    }
    else
        EnumFtype(hKey.get(), verb.c_str());

    return EXIT_SUCCESS;
}
