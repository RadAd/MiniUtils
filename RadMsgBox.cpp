#include <Windows.h>
#include <strsafe.h>
#include <string>
#include "arg.inl"

#ifdef UNICODE
#define tstring wstring
#define __targv __wargv
#else
#define tstring string
#define __targv __argv
#endif

#define PROGRAM TEXT("RadMsgBox")

void ErrorMessage(_In_ LPCTSTR lpFormat, ...)
{
    TCHAR buf[1024];
    va_list vl;
    va_start(vl, lpFormat);
    StringCchVPrintf(buf, ARRAYSIZE(buf), lpFormat, vl);
    va_end(vl);
    MessageBox(NULL, buf, PROGRAM, MB_OK | MB_ICONERROR);
}

void ShowError(_In_ LPCTSTR lpWhere)
{
    DWORD errorCode = GetLastError();
    LPTSTR msg = NULL;
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, 0, (LPTSTR) &msg, 0, NULL) == 0)
    {
        ErrorMessage(TEXT("Error: %i FormatMessage."), errorCode);
    }
    else
    {
        ErrorMessage(TEXT("Error: %i %s.\n%s"), errorCode, lpWhere, msg);
        LocalFree(msg);
    }
}

int CALLBACK _tWinMain(
    _In_ HINSTANCE /*hInstance*/,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPTSTR    /*lpCmdLine*/,
    _In_ int       /*nCmdShow*/
)
{
    arginit(__argc, __targv, TEXT("Calls MessageBox"));
    std::tstring title = argvalue(_T("/Title"), PROGRAM, _T("title"), _T("Message box title"));
    std::tstring message = argnum(1, TEXT(""), _T("message"), _T("Message box message"));
    UINT icon = 0;
    if (argswitch(TEXT("/IconError"),       TEXT("Error icon")))        icon = MB_ICONERROR;
    if (argswitch(TEXT("/IconInformation"), TEXT("Information icon")))  icon = MB_ICONINFORMATION;
    if (argswitch(TEXT("/IconWarning"),     TEXT("Warning icon")))      icon = MB_ICONWARNING;
    if (argswitch(TEXT("/IconQuestion"),    TEXT("Question icon")))     icon = MB_ICONQUESTION;
    UINT buttons = MB_OK;
    if (argswitch(TEXT("/Ok"),              TEXT("Ok button")))                     buttons = MB_OK;
    if (argswitch(TEXT("/OkCancel"),        TEXT("Ok and Cancel buttons")))         buttons = MB_OKCANCEL;
    if (argswitch(TEXT("/YesNo"),           TEXT("Yes and No buttons")))            buttons = MB_YESNO;
    if (argswitch(TEXT("/YesNoCancel"),     TEXT("Yes, No and Cancel buttons")))    buttons = MB_YESNOCANCEL;
    if (argswitch(TEXT("/RetryCancel"),     TEXT("Retry and Cancel buttons")))      buttons = MB_RETRYCANCEL;
    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
        return EXIT_SUCCESS;

    return MessageBox(NULL, message.c_str(), title.c_str(), buttons | icon);
}
