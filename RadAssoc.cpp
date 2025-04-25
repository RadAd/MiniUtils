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

bool DisplayValue(HKEY hKey, LPCTSTR ext)
{
    TCHAR data[1024] = TEXT("");
    LONG  size = ARRAYSIZE(data) * sizeof(TCHAR);
    CHECK(RegQueryValue(hKey, nullptr, data, &size));

    if (!Empty(data))
        _ftprintf(stdout, TEXT("%s=%s\n"), ext, data);

    return !Empty(data);
}

bool ShowAssoc(HKEY hBaseKey, LPCTSTR ext)
{
    std::unique_ptr<HKEY> hKey;
    CHECK_MSG_RET(RegOpenKeyEx(hBaseKey, ext, 0, KEY_READ, &hKey), nullptr, false);

    const bool valid = DisplayValue(hKey.get(), ext);

    return valid;
}

void SetAssoc(HKEY hBaseKey, LPCTSTR ext, LPCTSTR value)
{
    std::unique_ptr<HKEY> hKey;
    CHECK_RET(RegCreateKey(hBaseKey, ext, &hKey), ;);

    if (value)
    {
        CHECK(RegSetValue(hKey.get(), nullptr, REG_SZ, value, (lstrlen(value) + 1) * sizeof(TCHAR)));
    }
    else
    {
        // TODO If hKey is empty then delete it instead
        CHECK(RegDeleteValue(hKey.get(), nullptr));
    }

    DisplayValue(hKey.get(), ext);
}

void EnumAssoc(HKEY hKey)
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
        &ftLastWriteTime),        // last write time
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

        if (!achKey.empty() && achKey[0] == TEXT('.'))
            ShowAssoc(hKey, achKey.data());
    }
}

int _tmain(const int argc, LPCTSTR argv[])
{
    const LPCTSTR strSubKey = TEXT("Software\\Classes");
    HKEY hBaseKey = HKEY_LOCAL_MACHINE;
    std::tstring ext;
    bool setValue = false;
    bool showHelp = false;
    std::tstring value;

    for (int argi = 1; argi < argc; ++argi)
    {
        LPCTSTR arg = argv[argi];
        if (arg[0] == TEXT('/'))
        {
            if (lstrcmpi(arg, TEXT("/U")) == 0)
                hBaseKey = HKEY_CURRENT_USER;
            else if (lstrcmpi(arg, TEXT("/?")) == 0)
                showHelp = true;
            else
                _ftprintf(stderr, TEXT("Unknown option %s\n"), arg);
        }
        else if (ext.empty())
        {
            ext = arg;
            size_t f = ext.find(TEXT('='));
            if (f != std::tstring::npos)
            {
                setValue = true;
                value = ext.substr(f + 1);
                ext = ext.substr(0, f);
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
        _ftprintf(stdout, TEXT("Displays or modifies file extension associations\n"));
        _ftprintf(stdout, TEXT("\n"));
        _ftprintf(stdout, TEXT("ASSOC [/U] [.ext[=[fileType]]]\n"));
        _ftprintf(stdout, TEXT("\n"));
        _ftprintf(stdout, TEXT("  /U        Set for current user only\n"));
        _ftprintf(stdout, TEXT("  .ext      Specifies the file extension to associate the file type with\n"));
        _ftprintf(stdout, TEXT("  fileType  Specifies the file type to associate with the file extension\n"));

        return EXIT_SUCCESS;
    }

    std::unique_ptr<HKEY> hKey;
    CHECK_RET(RegOpenKeyEx(hBaseKey, strSubKey, 0, KEY_READ, &hKey), EXIT_FAILURE);

    if (!ext.empty())
    {
        if (setValue)
            SetAssoc(hKey.get(), ext.c_str(), value.empty() ? nullptr : value.c_str());
        else
            if (!ShowAssoc(hKey.get(), ext.c_str()))
                _ftprintf(stderr, TEXT("File association not found for extension %s\n"), ext.c_str());
    }
    else
        EnumAssoc(hKey.get());

    return EXIT_SUCCESS;
}
