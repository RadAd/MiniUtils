#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

int _tmain()
{
    GUID guid = {};
    wchar_t szGuidW[40] = {};
    CoCreateGuid(&guid);
    StringFromGUID2(guid, szGuidW, 40);
    
#ifdef UNICODE
    TCHAR* szGuid = szGuidW;
#else
    char szGuidA[40] = {};
    WideCharToMultiByte(CP_ACP, 0, szGuidW, -1, szGuidA, 40, NULL, NULL);
    TCHAR* szGuid = szGuidA;
#endif
    _ftprintf(stdout, _T("%s\n"), szGuid);
    
    return EXIT_SUCCESS;
}
