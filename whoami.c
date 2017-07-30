#include <windows.h>
#include <lmcons.h>
#include <tchar.h>
#include <stdio.h>

int main()
{
    TCHAR name[UNLEN + 1];
    DWORD size = UNLEN;
    GetUserName(name, &size);
    _tprintf(TEXT("%s\n"), name);
    return 0;
}
