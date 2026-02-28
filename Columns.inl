#include <cstdio>
#include <string>
#include <vector>

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

void ColPrintField(const DWORD Val, const DWORD Width) { _tprintf(TEXT("%*u "), Width, Val); }
void ColPrintField(const LONG Val, const DWORD Width)  { _tprintf(TEXT("%*d "), Width, Val); }
void ColPrintField(const VOID* Val, const DWORD Width) { _tprintf(TEXT("0x%*p "), Width - 2, Val); }
void ColPrintField(const HANDLE Val, const DWORD Width){ _tprintf(TEXT("0x%0*X "), Width - 2, (UINT) (INT_PTR) Val); }
void ColPrintField(LPCTSTR Val, const DWORD Width)     { _tprintf(TEXT("%-*s "), Width, Val); }

template <class T>
struct Column
{
    typedef void (*PrintCol)(const T& v, const DWORD Width);
    TCHAR       id;
    LPCTSTR     Name;
    LPCTSTR     Desc;
    DWORD       Width;
    PrintCol    Print;
};

template <class T, size_t N>
const T* GetCol(TCHAR id, const T (&cols)[N])
{
    for (const T& c : cols)
    {
        if (c.id == id)
            return &c;
    }
    return nullptr;
}

template <class T, size_t N>
void ColPrintDescription(const T (&cols)[N])
{
    for (const T& col : cols)
    {
        _tprintf(TEXT("  %c - %s\t%s\n"), col.id, col.Name, col.Desc);
    }
}

template <class T, size_t N>
std::vector<const T*> ColParseFormat(const std::tstring& format, const T (&cols)[N])
{
    std::vector<const T*> printcols;
    for (const TCHAR c : format)
    {
        const T* col = GetCol(c, cols);
        if (col)
            printcols.push_back(col);
    }
    return printcols;
}

template <class T>
void ColPrintHeader(const std::vector<const Column<T>*>& printcols)
{
    for (const Column<T>* col : printcols)
    {
        _tprintf(TEXT("%-*s "), col->Width, col->Name);
    }
    _tprintf(TEXT("\n"));
    for (const Column<T>* col : printcols)
    {
        const LPCTSTR str = TEXT("==================================");
#if 0
        fwrite(str, sizeof(TCHAR), col->Width, tstdout);    // TODO support wstdout
        _tprintf(TEXT(" "));
#else
        _tprintf(TEXT("%*.*s "), col->Width, col->Width, str);
#endif
    }
    _tprintf(TEXT("\n"));
}

template <class T>
void ColPrintRow(const std::vector<const Column<T>*>& printcols, const T& val)
{
    for (const Column<T>* col : printcols)
    {
        col->Print(val, col->Width);
    }
    _tprintf(TEXT("\n"));
}
