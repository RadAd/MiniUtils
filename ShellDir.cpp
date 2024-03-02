#include <tchar.h>

#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <Windows.h>
#include <ShlObj.h>
#include <atlbase.h>
#include <propkey.h>
#include <propvarutil.h>

#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif

void Check(HRESULT hr)
{
    if (FAILED(hr))
        AtlThrow(hr);
}

struct PropVariant : public PROPVARIANT
{
    PropVariant() { PropVariantInit(this); }
    ~PropVariant() { PropVariantClear(this); }

    ULONGLONG ToUInt64() const
    {
        ULONGLONG v;
        Check(PropVariantToUInt64(*this, &v));
        return v;
    };

    FILETIME ToFileTime(_In_ PSTIME_FLAGS pstfOut) const
    {
        FILETIME v;
        Check(PropVariantToFileTime(*this, pstfOut , &v));
        return v;
    };
};

template <class T>
auto AutoTaskMem(T* p = nullptr)
{
    //return std::unique_ptr<T, void (*)(LPVOID)>(p, CoTaskMemFree);
    return std::unique_ptr<T, decltype(&CoTaskMemFree)>(p, CoTaskMemFree);
}

template<class T, class U>
typename T* operator!(const std::unique_ptr<T, U>& up)
{
    return up.get();
}

template <class T, class U>
class unique_ptr_ptr
{
public:
    typedef std::unique_ptr<T, U> unique_ptr;

    unique_ptr_ptr(unique_ptr& up)
        : up(up), p(nullptr)
    {
    }

    ~unique_ptr_ptr()
    {
        up.reset(p);
    }

    operator typename unique_ptr::pointer* ()
    {
        return &p;
    }

private:
    unique_ptr& up;
    typename unique_ptr::pointer p;
};

template<class T, class U>
typename unique_ptr_ptr<T, U> operator&(std::unique_ptr<T, U>& up)
{
    return unique_ptr_ptr<T, U>(up);
}

inline bool IsSet(ULONG v, ULONG t)
{
    return (v & t) == t;
}

struct Options
{
    bool recurse;
    bool showhidden;
};

//typedef std::unique_ptr<ITEMIDLIST, void (*)(LPVOID)> AutoITEMIDLIST;
typedef std::unique_ptr<ITEMIDLIST, decltype(&CoTaskMemFree)> AutoITEMIDLIST;
std::vector<AutoITEMIDLIST> EnumObjects(IShellFolder* psfDir, SHCONTF grfFlags)
{
    std::vector<AutoITEMIDLIST> l;

    auto pEnum = AutoTaskMem<IEnumIDList>();
    Check(psfDir->EnumObjects(nullptr, grfFlags, &pEnum));

    auto pidlItems = AutoTaskMem<ITEMIDLIST>();
    ULONG celtFetched;
    while (SUCCEEDED(pEnum->Next(1, &pidlItems, &celtFetched)) && celtFetched == 1)
    {
        l.emplace_back(std::move(pidlItems));
    }

    return l;
}

ULONG GetAttributesOf(IShellFolder* psfDir, LPCITEMIDLIST i, ULONG uAttr)
{
    Check(psfDir->GetAttributesOf(1, (LPCITEMIDLIST*) &i, &uAttr));
    return uAttr;
}

int RadFormatDateTime(_In_ const FILETIME* pft, _Inout_opt_ DWORD* pdwFlags, _Out_writes_(cchBuf) LPTSTR pszBuf, UINT cchBuf)
{
#if 0
    SHFormatDateTime(&ftDateModified, &dwFlags, strDateModified, ARRAYSIZE(strDateModified));
#else
    // TODO use pdwFlags
    const DWORD dwFlags = pdwFlags ? *pdwFlags : FDTF_DEFAULT;
    SYSTEMTIME st;
    FileTimeToSystemTime(pft, &st);
    DWORD dwDateFlags = 0;
    if (IsSet(dwFlags, FDTF_SHORTDATE)) dwDateFlags = DATE_SHORTDATE;
    if (IsSet(dwFlags, FDTF_LONGDATE)) dwDateFlags = DATE_LONGDATE;
    DWORD dwTimeFlags = 0;
    if (IsSet(dwFlags, FDTF_SHORTTIME)) dwTimeFlags = TIME_NOSECONDS;
    if (IsSet(dwFlags, FDTF_LONGTIME)) dwTimeFlags = 0;
    int count;
    if (IsSet(dwFlags, FDTF_RELATIVE))
    {
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);
        if (stNow.wYear == st.wYear
            && stNow.wMonth == st.wMonth
            && stNow.wDay == st.wDay)
        {
            lstrcpy(pszBuf, TEXT("Today"));
            count = 6;
        }
        else
            count = GetDateFormat(LOCALE_CUSTOM_DEFAULT, dwDateFlags, &st, nullptr, pszBuf, int(cchBuf));
    }
    else
        count = GetDateFormat(LOCALE_CUSTOM_DEFAULT, dwDateFlags, &st, nullptr, pszBuf, int(cchBuf));
    pszBuf[count - 1] = TEXT(' ');
    count += GetTimeFormat(LOCALE_CUSTOM_DEFAULT, dwTimeFlags, &st, nullptr, pszBuf + count, int(cchBuf) - count);
    return count;
#endif
}

void EnumPrint(IShellFolder* psfDir, const Options& options, const std::tstring& pre = std::tstring())
{
    std::vector<AutoITEMIDLIST> l = EnumObjects(psfDir, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS);
    std::sort(l.begin(), l.end(), [&psfDir](const AutoITEMIDLIST& l, const AutoITEMIDLIST& r) {
            const ULONG uAttrL = GetAttributesOf(psfDir, !l, SFGAO_FOLDER);
            const ULONG uAttrR = GetAttributesOf(psfDir, !r, SFGAO_FOLDER);

            if (IsSet(uAttrL, SFGAO_FOLDER) == IsSet(uAttrR, SFGAO_FOLDER))
                return psfDir->CompareIDs(0, l.get(), r.get()) < 0;
            else
                return IsSet(uAttrL, SFGAO_FOLDER);
        });
    for (const AutoITEMIDLIST& pidlItems : l)
    {
        STRRET strDispName;
        Check(psfDir->GetDisplayNameOf(!pidlItems, SHGDN_INFOLDER, &strDispName));
        TCHAR pszDisplayName[MAX_PATH];
        Check(StrRetToBuf(&strDispName, !pidlItems, pszDisplayName, ARRAYSIZE(pszDisplayName)));

        const ULONG uAttr = GetAttributesOf(psfDir, !pidlItems, SFGAO_FOLDER | SFGAO_READONLY | SFGAO_SYSTEM | SFGAO_HIDDEN);
        if (IsSet(uAttr, SFGAO_SYSTEM | SFGAO_HIDDEN))
        {
            // Never show
        }
        else if (options.showhidden || !IsSet(uAttr, SFGAO_HIDDEN))
        {
            _tprintf(_T("%c%c%c%c "),
                IsSet(uAttr, SFGAO_READONLY) ? TEXT('R') : TEXT('-'),
                IsSet(uAttr, SFGAO_SYSTEM) ? TEXT('S') : TEXT('-'),
                IsSet(uAttr, SFGAO_HIDDEN) ? TEXT('H') : TEXT('-'),
                IsSet(uAttr, SFGAO_FOLDER) ? TEXT('D') : TEXT('-'));

            CComPtr<IPropertyStore> ppsItem;
            if (SUCCEEDED(psfDir->BindToObject(!pidlItems, nullptr, IID_PPV_ARGS(&ppsItem))))
            {
                PropVariant pvSize;
                Check(ppsItem->GetValue(PKEY_Size, &pvSize));


                if (pvSize.vt != VT_NULL)
                {
                    //_tprintf(_T("%8llu"), pvSize.ToUInt64());
                    TCHAR strSize[25];
                    StrFormatByteSize64(pvSize.ToUInt64(), strSize, ARRAYSIZE(strSize));
                    _tprintf(_T("%9s"), strSize);
                }
                else
                    _tprintf(_T("%9c"), TEXT(' '));

                PropVariant pvDateModified;
                Check(ppsItem->GetValue(PKEY_DateModified, &pvDateModified));

                const FILETIME ftDateModified = pvDateModified.ToFileTime(PSTF_LOCAL);
                DWORD dwFlags = FDTF_DEFAULT | FDTF_RELATIVE | FDTF_NOAUTOREADINGORDER;
                TCHAR strDateModified[25];
                RadFormatDateTime(&ftDateModified, &dwFlags, strDateModified, ARRAYSIZE(strDateModified));

                _tprintf(_T("%20s "), strDateModified);
                _tprintf(_T("%s"), pre.c_str());

                if (IsSet(uAttr, SFGAO_FOLDER))
                    _tprintf(_T("[%s]"), pszDisplayName);
                else
                    _tprintf(_T("%s"), pszDisplayName);

                _tprintf(_T("\n"));
            }
            else
            {
                if (IsSet(uAttr, SFGAO_FOLDER))
                    _tprintf(_T("%s%8c %s\\\n"), pre.c_str(), TEXT(' '), pszDisplayName);
                else
                    _tprintf(_T("%8c%s %s\n"), TEXT('-'), pre.c_str(), pszDisplayName);
            }

            if (options.recurse && IsSet(uAttr, SFGAO_FOLDER))
            {
                CComPtr<IShellFolder> psfSubDir;
                Check(psfDir->BindToObject(!pidlItems, nullptr, IID_PPV_ARGS(&psfSubDir)));
                EnumPrint(psfSubDir, options, pre + TEXT(' '));
            }
        }
    }
}

int _tmain(const int argc, const LPCWSTR argv[])
{
    //_tprintf(_T("Hello World\n"));
    Options options = {};
    LPCWSTR dir = nullptr;

    for (int argi = 1; argi < argc; ++argi)
    {
        LPCWSTR arg = argv[argi];
        if (lstrcmp(arg, TEXT("/S")) == 0)
            options.recurse = true;
        else if (lstrcmp(arg, TEXT("/H")) == 0)
            options.showhidden = true;
        else
            dir = arg;
    }

    TCHAR cwd[MAX_PATH];
    GetCurrentDirectory(ARRAYSIZE(cwd), cwd);

    TCHAR combine[MAX_PATH];
    if (dir != nullptr && lstrcmp(dir, TEXT("{desktop}")) == 0)
        combine[0] = TEXT('\0');
    else
        PathCombine(combine, cwd, dir);

    Check(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

    try
    {
        CComPtr<IShellFolder> psfDeskTop;
        Check(SHGetDesktopFolder(&psfDeskTop));

        if (combine[0] == TEXT('\0'))
        {
            EnumPrint(psfDeskTop, options); // TODO Crashing???
        }
        else
        {
            auto pidlDir = AutoTaskMem<ITEMIDLIST>();
            Check(psfDeskTop->ParseDisplayName(nullptr, nullptr, combine, nullptr, &pidlDir, nullptr));

            CComPtr<IShellFolder> psfDir;
            Check(psfDeskTop->BindToObject(!pidlDir, nullptr, IID_PPV_ARGS(&psfDir)));

            EnumPrint(psfDir, options);
        }
    }
    catch (const CAtlException& ex)
    {
        TCHAR   wszMsgBuff[512];
        DWORD   dwChars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            ex.m_hr,
            0,
            wszMsgBuff,
            ARRAYSIZE(wszMsgBuff),
            NULL);
        while (wszMsgBuff[dwChars - 1] == TEXT('\n') || wszMsgBuff[dwChars - 1] == TEXT('\r'))
            --dwChars;
        wszMsgBuff[dwChars] = TEXT('\0');

        _ftprintf(stderr, _T("Error: 0x%08x %s\n"), ex.m_hr, wszMsgBuff);
    }

    CoUninitialize();

    return EXIT_SUCCESS;
}
