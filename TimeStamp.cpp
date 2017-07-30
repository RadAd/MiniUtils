#include "stdafx.h"

int main()
{
    FILE* input = stdin;
    FILE* output = stdout;

    TCHAR buffer[1024];
    while (_fgetts(buffer, ARRAYSIZE(buffer), input) != nullptr)
    {
        TCHAR time[16];
        GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, nullptr, nullptr, time, ARRAYSIZE(time));
        _ftprintf(output, _T("%8s "), time);
        _fputts(buffer, output);
    }

    return 0;
}
