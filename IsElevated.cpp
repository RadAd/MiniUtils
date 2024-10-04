#include <Windows.h>
#include <cstdio>

BOOL IsElevated()
{
#if 0
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
            fRet = Elevation.TokenIsElevated;
    }
    if (hToken)
        CloseHandle(hToken);
    return fRet;
#else
    BOOL fRet = FALSE;
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof(TOKEN_ELEVATION);
    if (GetTokenInformation(GetCurrentProcessToken(), TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
        fRet = Elevation.TokenIsElevated;
    else
        fprintf(stderr, "ERROR:\tGetTokenInformation (error %d)\n", GetLastError());
    return fRet;
#endif
}

int main()
{
    return IsElevated() ? EXIT_SUCCESS : EXIT_FAILURE;
}
