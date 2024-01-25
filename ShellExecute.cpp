#include <cstdlib>
#include <cstdio>
#include <tchar.h>
#include <windows.h>
#include <string>

#pragma comment(lib, "shell32")
#pragma comment(lib, "ole32")


struct WinError
{
    DWORD dwError;
    LPCTSTR szModule;

    void print(FILE *stream, LPCTSTR szLocation) const
    {
        if (dwError == ERROR_SUCCESS)
            return;

        HMODULE hLibrary = szModule != nullptr ? GetModuleHandle(szModule) : NULL;
        LPTSTR pMessage = nullptr;
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
            _ftprintf(stream, _T("Format message failed with 0x%08x\n"), GetLastError());
        }
        else
        {
            _ftprintf(stream, _T("Error: 0x%08x %s "), dwError, pMessage);
            LocalFree(pMessage);
        }
        _ftprintf(stream, _T("   At: %s\n"), szLocation);
    }
};

#define CHECK_LE(x, r) if (!(x)) { WinError({ GetLastError() }).print(stderr, #x); r; }

int _tmain(const int argc, const TCHAR* const argv[])
{
    DWORD dwExitCode = EXIT_SUCCESS;

    HRESULT hr;
	if (FAILED(hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
        _ftprintf(stderr, _T("Error %d\n"), hr);
	}

	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
	sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_DOENVSUBST | SEE_MASK_NO_CONSOLE | SEE_MASK_NOCLOSEPROCESS;
	sei.nShow = SW_NORMAL;  // TODO
	// sei.lpClass
	// sei.hkeyClass
	// sei.dwHotKey
	// sei.hIcon
	// sei.hMonitor

	// TODO
	// SEE_MASK_ICON
	// SEE_MASK_HOTKEY
	// SEE_MASK_UNICODE
	// SEE_MASK_NO_CONSOLE
	// SEE_MASK_HMONITOR
	// SEE_MASK_FLAG_LOG_USAGE

	bool debug = false;
	bool wait = true;
	std::string Parameters;

	for (int iarg = 1; iarg < argc; ++iarg)
	{
        const TCHAR* const arg = argv[iarg];
        if (sei.lpFile == nullptr && arg[0] == '/')
        {
            if (_tcsicmp(arg + 1, TEXT("Verb")) == 0)    // TODO change this to /Verb:open
                sei.lpVerb = argv[++iarg];
            else if (_tcsicmp(arg + 1, TEXT("Directory")) == 0)    // TODO change this to /Directory=C:\test
                sei.lpDirectory = argv[++iarg];
            else if (_tcsicmp(arg + 1, TEXT("debug")) == 0)
                debug = true;
            else if (_tcsicmp(arg + 1, TEXT("nowait")) == 0)
                wait = false;
            else if (_tcsicmp(arg + 1, TEXT("newconsole")) == 0)
                sei.fMask &= ~SEE_MASK_NO_CONSOLE;
            else if (_tcsicmp(arg + 1, TEXT("unicode")) == 0)
                sei.fMask |= SEE_MASK_UNICODE;
            else
                _ftprintf(stderr, _T("Unknown switch %s\n"), arg);
        }
        else if (sei.lpFile == nullptr)
            sei.lpFile = arg;
        else
        {
            Parameters += TEXT(' ');
            const bool hasspace = _tcschr(arg, TEXT(' ')) != nullptr;
            if (hasspace)
                Parameters += TEXT('"');
            // TODO If hasspace may need to escape some characters
            Parameters += arg;
            if (hasspace)
                Parameters += TEXT('"');
            sei.lpParameters = Parameters.c_str() + 1; // Skip over initial ' '
        }
	}

    if (debug)
    {
        _ftprintf(stderr, _T("File [%s]\n"), sei.lpFile);
        _ftprintf(stderr, _T("Parameters [%s]\n"), sei.lpParameters);
        _ftprintf(stderr, _T("Verb [%s]\n"), sei.lpVerb);
	}

	CHECK_LE(ShellExecuteEx(&sei), 0);

	if (sei.hProcess != NULL)
	{
        if (wait)
        {
            CHECK_LE(WaitForSingleObject(sei.hProcess, INFINITE), 0);
            CHECK_LE(GetExitCodeProcess(sei.hProcess, &dwExitCode), 0);
        }
        CloseHandle(sei.hProcess);
	}

	CoUninitialize();

	return dwExitCode;
}
