#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#define and &&
#define or ||

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

#ifdef UNICODE
#define tstring wstring
#define tifstream wifstream
#define tistringstream wistringstream
#else
#define tstring string
#define tifstream ifstream
#define tistringstream istringstream
#endif

#define STRINGIZE2(s) TEXT(#s)
#define STRINGIZE(s) STRINGIZE2(s)

// https://www.temblast.com/dbplot/color5.htm

const TCHAR* ColorName(int i)
{
    switch (i)
    {
#define x(v) case v: return TEXT(#v)
        x(COLOR_SCROLLBAR);
        x(COLOR_BACKGROUND);
        x(COLOR_ACTIVECAPTION);
        x(COLOR_INACTIVECAPTION);
        x(COLOR_MENU);
        x(COLOR_WINDOW);
        x(COLOR_WINDOWFRAME);
        x(COLOR_MENUTEXT);
        x(COLOR_WINDOWTEXT);
        x(COLOR_CAPTIONTEXT);
        x(COLOR_ACTIVEBORDER);
        x(COLOR_INACTIVEBORDER);
        x(COLOR_APPWORKSPACE);
        x(COLOR_HIGHLIGHT);
        x(COLOR_HIGHLIGHTTEXT);
        x(COLOR_BTNFACE);
        x(COLOR_BTNSHADOW);
        x(COLOR_GRAYTEXT);
        x(COLOR_BTNTEXT);
        x(COLOR_INACTIVECAPTIONTEXT);
        x(COLOR_BTNHIGHLIGHT);

#if(WINVER >= 0x0400)
        x(COLOR_3DDKSHADOW);
        x(COLOR_3DLIGHT);
        x(COLOR_INFOTEXT);
        x(COLOR_INFOBK);
#endif /* WINVER >= 0x0400 */

#if(WINVER >= 0x0500)
        x(COLOR_HOTLIGHT);
        x(COLOR_GRADIENTACTIVECAPTION);
        x(COLOR_GRADIENTINACTIVECAPTION);
#if(WINVER >= 0x0501)
        x(COLOR_MENUHILIGHT);
        x(COLOR_MENUBAR);
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */
#undef x
    case 25: return TEXT("COLOR_UNUSED");
    default: return TEXT("Unknown");
    }
}

const TCHAR* RegColorName(int i)
{
    switch (i)
    {
#define x(v, s) case v: return TEXT(s)
        x(COLOR_SCROLLBAR, "Scrollbar");
        x(COLOR_BACKGROUND , "Background");
        x(COLOR_ACTIVECAPTION, "ActiveTitle");
        x(COLOR_INACTIVECAPTION , "InactiveTitle");
        x(COLOR_MENU, "Menu");
        x(COLOR_WINDOW, "Window");
        x(COLOR_WINDOWFRAME, "WindowFrame");
        x(COLOR_MENUTEXT, "MenuText");
        x(COLOR_WINDOWTEXT, "WindowText");
        x(COLOR_CAPTIONTEXT, "TitleText");
        x(COLOR_ACTIVEBORDER, "ActiveBorder");
        x(COLOR_INACTIVEBORDER, "InactiveBorder");
        x(COLOR_APPWORKSPACE, "AppWorkspace");
        x(COLOR_HIGHLIGHT, "Hilight");
        x(COLOR_HIGHLIGHTTEXT, "HilightText");
        x(COLOR_BTNFACE, "ButtonFace");
        x(COLOR_BTNSHADOW, "ButtonShadow");
        x(COLOR_GRAYTEXT, "GrayText");
        x(COLOR_BTNTEXT, "ButtonText");
        x(COLOR_INACTIVECAPTIONTEXT, "InactiveTitleText");
        x(COLOR_BTNHIGHLIGHT, "ButtonHilight");

#if(WINVER >= 0x0400)
        x(COLOR_3DDKSHADOW, "ButtonDkShadow");
        x(COLOR_3DLIGHT, "ButtonLight");
        x(COLOR_INFOTEXT, "InfoText");
        x(COLOR_INFOBK, "InfoWindow");
#endif /* WINVER >= 0x0400 */
        x(25, "ButtonAlternateFace");

#if(WINVER >= 0x0500)
        x(COLOR_HOTLIGHT, "HotTrackingColor");
        x(COLOR_GRADIENTACTIVECAPTION, "GradientActiveTitle");
        x(COLOR_GRADIENTINACTIVECAPTION, "GradientInactiveTitle");
#if(WINVER >= 0x0501)
        x(COLOR_MENUHILIGHT, "MenuHilight");
        x(COLOR_MENUBAR, "MenuBar");
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */
#undef x
    default: return TEXT("Unknown");
    }
}

void InitSysColours(_In_ int cElements,
    _Out_writes_(cElements) INT* lpaElements)
{
    for (int i = 0; i < cElements; ++i)
        lpaElements[i] = i;
}

void LoadSysColors(
    _In_ int cElements,
    _In_reads_(cElements) CONST INT* lpaElements,
    _Out_writes_(cElements) COLORREF* lpaRgbValues)
{
    for (int i = 0; i < cElements; ++i)
        lpaRgbValues[i] = GetSysColor(lpaElements[i]);
}

void PrintSysColors(
    _In_ int cElements,
    _In_reads_(cElements) CONST INT* lpaElements,
    _In_reads_(cElements) CONST COLORREF* lpaRgbValues)
{
    for (int i = 0; i < cElements; ++i)
        _tprintf(_T("%s=%u %u %u\n"), ColorName(lpaElements[i]), GetRValue(lpaRgbValues[i]), GetGValue(lpaRgbValues[i]), GetBValue(lpaRgbValues[i]));
}

COLORREF ParseColor(LPCTSTR sColor)
{
    std::tistringstream sstre(sColor);
    int r, g, b;
    sstre >> r;
    sstre >> g;
    sstre >> b;
    if (r >= 0 and r <= 255 and g >= 0 and g <= 255 and b >= 0 and b <= 255)
        return RGB(r, g, b);
    else
    {
        _ftprintf(stderr, _T("Invalid color: %s\n"), sColor);
        return 0;
    }
}

void LoadRegSysColors(
    _In_ int cElements,
    _In_reads_(cElements) CONST INT* lpaElements,
    _Out_writes_(cElements) COLORREF* lpaRgbValues)
{
    _ftprintf(stderr, _T("TODO LoadRegSysColors\n"));
    HKEY hKey = NULL;
    RegOpenKey(HKEY_CURRENT_USER, _T("Control Panel\\Colors"), &hKey);
    if (hKey == NULL)
    {
        _ftprintf(stderr, _T("Error opening key\n"));
        return;
    }

    DWORD dwIndex = 0;
    while (true)
    {
        TCHAR valueName[100];
        DWORD dataType;
        TCHAR data[100];

        DWORD valNameLen = ARRAYSIZE(valueName);
        DWORD dataSize = ARRAYSIZE(data) * sizeof(data[0]);

        if (RegEnumValue(hKey,
            dwIndex,
            valueName,
            &valNameLen,
            NULL,
            &dataType,
            (BYTE*) &data,
            &dataSize) == ERROR_NO_MORE_ITEMS)
            break;

        int i;
        for (i = 0; i < cElements; ++i)
            if (_tcsicmp(valueName, RegColorName(lpaElements[i])) == 0)
            {
                lpaRgbValues[i] = ParseColor(data);
                break;
            }
        if (i >= cElements)
            _ftprintf(stderr, _T("Unknown value: %s\n"), valueName);

        dwIndex++;
    }

    RegCloseKey(hKey);
}

void SaveRegSysColors(
    _In_ int cElements,
    _In_reads_(cElements) CONST INT* lpaElements,
    _In_reads_(cElements) CONST COLORREF* lpaRgbValues)
{
    _ftprintf(stderr, _T("TODO SaveRegSysColors\n"));
}

void LoadFileSysColors(
    _In_ LPCTSTR sFileName,
    _In_ int cElements,
    _In_reads_(cElements) CONST INT* lpaElements,
    _Out_writes_(cElements) COLORREF* lpaRgbValues)
{
    std::tifstream f(sFileName);
    std::tstring line;
    while (std::getline(f, line))
    {
        if (line.empty() or line[0] == _T('#'))
            continue;

        const size_t e = line.find('=');
        if (e == std::string::npos)
        {
            _ftprintf(stderr, _T("Unknown line: %s\n"), line.c_str());
            continue;
        }

        const std::tstring name = line.substr(0, e);
        int i;
        for (i = 0; i < cElements; ++i)
            if (name == ColorName(lpaElements[i]))
            {
                const std::tstring color = line.substr(e + 1);
                lpaRgbValues[i] = ParseColor(color.c_str());
                break;
            }
        if (i >= cElements)
            _ftprintf(stderr, _T("Unknown line: %s\n"), line.c_str());
    }
}

#define COLOR_MAX (COLOR_MENUBAR + 1)

int _tmain(const int argc, const TCHAR* const argv[])
{
    _tprintf(_T("# Sys Colours\n"));
    bool bLoadReg = false;
    bool bSaveReg = false;
    LPCTSTR sFileName = nullptr;

    for (int argi = 1; argi < argc; ++argi)
    {
        LPCTSTR arg = argv[argi];
        if (_tcsicmp(arg, _T("/loadreg")) == 0)
            bLoadReg = true;
        else if (_tcsicmp(arg, _T("/savereg")) == 0)
            bSaveReg = true;
        else if (sFileName == nullptr)
            sFileName = arg;
        else
            _ftprintf(stderr, _T("Unknown argument: %s\n"), arg);
    }

    INT index[COLOR_MAX] = {};
    COLORREF colours[COLOR_MAX] = {};

    InitSysColours(COLOR_MAX, index);

    LoadSysColors(COLOR_MAX, index, colours);

    if (bLoadReg)
        LoadRegSysColors(COLOR_MAX, index, colours);

    if (sFileName)
        LoadFileSysColors(sFileName, COLOR_MAX, index, colours);

    PrintSysColors(COLOR_MAX, index, colours);

    SetSysColors(COLOR_MAX, index, colours);

    if (bSaveReg)
        SaveRegSysColors(COLOR_MAX, index, colours);

    return EXIT_SUCCESS;
}
