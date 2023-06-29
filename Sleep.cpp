#include <cstdio>
#include <Windows.h>
#include <tchar.h>
#include "arg.inl"

int _tmain(int argc, const TCHAR* argv[])
{
    arginit(argc, argv);
    const DWORD sleep_ms = _ttoi(argnum(1, _T("0"))) * 1000;
	if (!argcleanup() || sleep_ms == 0)
	{
        _ftprintf(stderr, _T("Usage: %s <time_in_sec> \n"), argapp());
        return EXIT_FAILURE;
    }

    //_tprintf(_T("Calling the sleep function for %d ms\n"), sleep_ms);

    Sleep(sleep_ms);
    return EXIT_SUCCESS;
}
