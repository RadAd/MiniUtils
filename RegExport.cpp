#include <cstdio>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <vector>
#include <string>

#define MAX_KEY_LENGTH 255

typedef DWORDLONG QWORD;

LPCTSTR treenode_u16[] = {
    _T("\u2514\u2500"),
    _T("\u251C\u2500"),
    _T("  "),
    _T("\u2502 "),
};
LPCTSTR treenode_u8[] = {
    _T("\\-"),
    _T("+-"),
    _T("  "),
    _T("| "),
};
LPCTSTR* treenode = treenode_u8;

LPTSTR GetTypeName(DWORD dwType)
{
    switch (dwType)
    {
    case REG_NONE:          return _T("None");
    case REG_BINARY:        return _T("Binary");
    case REG_DWORD:         return _T("DWORD");
    case REG_EXPAND_SZ:     return _T("Expand");
    case REG_MULTI_SZ:      return _T("Multi");
    case REG_QWORD:         return _T("QWORD");
    case REG_SZ:            return _T("String");
    default: return _T("Unknown");
    }
}

void ExportKey(HKEY hKey, const std::wstring& prefix)
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
    /*DWORD retCode =*/ RegQueryInfoKey(
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

    if (cSubKeys)
    {
        //printf("\nNumber of subkeys: %d\n", cSubKeys);

        for (DWORD i = 0; i < cSubKeys; ++i)
        {
            DWORD cbName = cbMaxSubKey + 1;
            std::vector<TCHAR> achKey(cbName);

            DWORD retCode = RegEnumKeyEx(hKey, i,
                achKey.data(),
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);
                
            const bool last = cValues == 0 && (i + 1) == cSubKeys;
            _tprintf(TEXT("\x1b[32m%s%s\x1b[34m[%s]\x1b[0m"), prefix.c_str(), last ? treenode[0] : treenode[1], achKey.data());
            if (retCode == ERROR_SUCCESS)
            {
                _tprintf(TEXT("\n"));
                
                HKEY hChildKey = NULL;

                /*DWORD retCode =*/ RegOpenKeyEx(hKey,
                    achKey.data(),
                    0,
                    KEY_READ,
                    &hChildKey);
                
                ExportKey(hChildKey, prefix + (last ? treenode[2] : treenode[3]));
            }
            else
            {
                _tprintf(TEXT(" (Error: %d)\n"), retCode);
            }
        }
    }

    // Enumerate the key values.

    if (cValues)
    {
        //printf("\nNumber of values: %d\n", cValues);

        for (DWORD i = 0; i < cValues; i++)
        {
            DWORD cchValue = cchMaxValue + 1;
            std::vector<TCHAR> achValue(cchValue);
            DWORD dwType = 0;
            DWORD cbData = cbMaxValueData + 1;
            std::vector<BYTE> bData(cbData);

            DWORD retCode = RegEnumValue(hKey, i,
                achValue.data(),
                &cchValue,
                NULL,
                &dwType,
                bData.data(),
                &cbData);

            if (_tcscmp(achValue.data(), _T("")) == 0)
                _tcscpy_s(achValue.data(), cchValue - 1, TEXT("@"));
                
            const bool last = (i + 1) == cValues;
            _tprintf(TEXT("\x1b[32m%s%s\x1b[33m\"%s\"\x1b[36m "), prefix.c_str(), last ? treenode[0] : treenode[1], achValue.data());
            if (retCode == ERROR_SUCCESS)
            {
                _tprintf(TEXT("(%s)\x1b[0m: "), GetTypeName(dwType));
                switch (dwType)
                {
                case REG_BINARY:
                    _tprintf(TEXT("(%d)"), cbData);
                    for (DWORD b = 0; b < cbData; ++b)
                    {
                        _tprintf(TEXT(" %02X"), bData[b]);
                    }
                    break;
                case REG_DWORD:         _tprintf(TEXT("%u (0x%08X)"), *reinterpret_cast<DWORD*>(bData.data()), *reinterpret_cast<DWORD*>(bData.data())); break;
                case REG_EXPAND_SZ:     _tprintf(TEXT("\"%.*s\""), static_cast<int>(cbData / sizeof(TCHAR)), reinterpret_cast<TCHAR*>(bData.data())); break;
                case REG_MULTI_SZ:
                    for (DWORD b = 0; b < (cbData - sizeof(TCHAR)); b += static_cast<DWORD>((_tcslen(reinterpret_cast<TCHAR*>(bData.data() + b)) + 1) * sizeof(TCHAR)))
                    {
                        if (b != 0)
                            _tprintf(TEXT(", "));
                        _tprintf(TEXT("\"%s\""), reinterpret_cast<TCHAR*>(bData.data() + b));
                    }
                    break;
                case REG_QWORD:         _tprintf(TEXT("%I64u (0x%08I64X)"), *reinterpret_cast<QWORD*>(bData.data()), *reinterpret_cast<QWORD*>(bData.data())); break;
                case REG_SZ:            _tprintf(TEXT("\"%.*s\""), static_cast<int>(cbData / sizeof(TCHAR)), reinterpret_cast<TCHAR*>(bData.data())); break;
                }
                _tprintf(TEXT("\n"));
            }
            else
            {
                _tprintf(TEXT("(Error: %d)\n"), retCode);
            }
        }
    }
}

HKEY StringToKey(LPCTSTR strKey)
{
         if (wcscmp(strKey, _T("HKEY_CURRENT_USER")) == 0) return HKEY_CURRENT_USER;
    else if (wcscmp(strKey, _T("HKCU")) == 0) return HKEY_CURRENT_USER;
    else if (wcscmp(strKey, _T("HKEY_LOCAL_MACHINE")) == 0) return HKEY_LOCAL_MACHINE;
    else if (wcscmp(strKey, _T("HKLM")) == 0) return HKEY_LOCAL_MACHINE;
    else if (wcscmp(strKey, _T("HKEY_CLASSES_ROOT")) == 0) return HKEY_CLASSES_ROOT;
    else if (wcscmp(strKey, _T("HKCR")) == 0) return HKEY_CLASSES_ROOT;
    else return NULL;
}

int _tmain(int argc, LPTSTR argv[])
{
    const bool unicode = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR;
    if (unicode)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);
        treenode = treenode_u16;
    }

    if (argc != 2)
    {
        _tprintf(TEXT("%s <key>\n"), argv[0]);
        return EXIT_SUCCESS;
    }
    
    LPTSTR strKey = argv[1];
    const LPTSTR i = _tcschr(strKey, _T('\\'));
    if (i != nullptr)
        *i = _T('\0');
    LPCTSTR strSubKey = i != nullptr ? i + 1 : _T("");
    
    const HKEY hBaseKey = StringToKey(strKey);
    if (hBaseKey == NULL)
    {
        _ftprintf(stderr, TEXT("Unknown base key %s\n"), strKey);
        return EXIT_FAILURE;
    }
    
    HKEY hKey = NULL;
    DWORD retCode = RegOpenKeyEx(hBaseKey,
        strSubKey,
        0,
        KEY_READ,
        &hKey);
    if (retCode != ERROR_SUCCESS)
    {
        _ftprintf(stderr, TEXT("Error opening key 0x%08X\n"), retCode);
        return EXIT_FAILURE;
    }
    
    ExportKey(hKey, _T(""));
    
    return EXIT_SUCCESS;
}
