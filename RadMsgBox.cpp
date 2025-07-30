#include <Windows.h>
#include <string>
#include "arg.inl"

#define PROGRAM L"RadMsgBox"

void ErrorMessage(_In_ LPCWSTR lpFormat, ...)
{
    WCHAR buf[1024];
    va_list vl;
    va_start(vl, lpFormat);
    wvsprintf(buf, lpFormat, vl);
    va_end(vl);
    MessageBox(NULL, buf, PROGRAM, MB_OK | MB_ICONERROR);
}

void ShowError(_In_ LPCWSTR lpWhere)
{
    DWORD errorCode = GetLastError();
    LPWSTR msg = NULL;
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, 0, (LPWSTR) &msg, 0, NULL) == 0)
    {
        ErrorMessage(L"Error: %i FormatMessage.", errorCode);
    }
    else
    {
        ErrorMessage(L"Error: %i %s.\n%s", errorCode, lpWhere, msg);
        LocalFree(msg);
    }
}

int CALLBACK wWinMain(
    _In_ HINSTANCE /*hInstance*/,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPWSTR    /*lpCmdLine*/,
    _In_ int       /*nCmdShow*/
)
{
    arginit(__argc, __wargv, TEXT("Calls MessageBox"));
    std::wstring title = argvalue(_T("/Title"), PROGRAM, _T("title"), _T("Message box title"));
    std::wstring message = argnum(1, TEXT(""), _T("message"), _T("Message box message"));
    UINT icon = 0;
    if (argswitch(L"/IconError",        L"Error icon"))         icon = MB_ICONERROR;
    if (argswitch(L"/IconInformation",  L"Information icon"))   icon = MB_ICONINFORMATION;
    if (argswitch(L"/IconWarning",      L"Warning icon"))       icon = MB_ICONWARNING;
    if (argswitch(L"/IconQuestion",     L"Question icon"))      icon = MB_ICONQUESTION;
    UINT buttons = MB_OK;
    if (argswitch(L"/Ok",               L"Ok button"))                  buttons = MB_OK;
    if (argswitch(L"/OkCancel",         L"Ok and Cancel buttons"))      buttons = MB_OKCANCEL;
    if (argswitch(L"/YesNo",            L"Yes and No buttons"))         buttons = MB_YESNO;
    if (argswitch(L"/YesNoCancel",      L"Yes, No and Cancel buttons")) buttons = MB_YESNOCANCEL;
    if (argswitch(L"/RetryCancel",      L"Retry and Cancel buttons"))   buttons = MB_RETRYCANCEL;
    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
        return EXIT_SUCCESS;

    return MessageBox(NULL, message.c_str(), title.c_str(), buttons | icon);
}
