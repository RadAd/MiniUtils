#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <string>

int _tmain(int argc, const TCHAR* const argv[])
{
    DWORD dwError = std::stoul(argv[1], nullptr, 0);
    const TCHAR* szModule = nullptr;
    
    const HMODULE hLibrary = szModule != nullptr ? GetModuleHandle(szModule) : NULL;
    LPTSTR pMessage = NULL;
    
    //_ftprintf(stdout, _T("dwError: %d 0x%08x\n"), dwError, dwError);
    //_ftprintf(stdout, _T("hLibrary: 0x%p\n"), hLibrary);
    
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
        _ftprintf(stderr, _T("Format message failed with 0x%08x\n"), GetLastError());
        return EXIT_FAILURE;
    }
    else
    {
        _ftprintf(stdout, _T("Error: 0x%08x %s\n"), dwError, pMessage);
        LocalFree(pMessage);
        return EXIT_SUCCESS;
    }
}
