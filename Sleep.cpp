#include <cstdio>
#include <Windows.h>
#include <tchar.h>
#include "arg.inl"

int _tmain(int argc, const TCHAR* argv[])
{
    arginit(argc, argv);
    const DWORD sleep_ms = _ttoi(argnum(1, _T("0"), _T("time"), _T("Time in milliseconds"))) * 1000;
	if (!argcleanup())
        return EXIT_FAILURE;
	if (argusage(sleep_ms == 0))
        return EXIT_SUCCESS;

    //_tprintf(_T("Calling the sleep function for %d ms\n"), sleep_ms);

    Sleep(sleep_ms);
    return EXIT_SUCCESS;
}
