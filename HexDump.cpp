#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <ctype.h>
#include <stdlib.h>
#include "arg.inl"

typedef unsigned char BYTE;

void setcolor(int* curcolor, int c)
{
    if (*curcolor != c)
    {
        _tprintf(_T("\x1b[%dm"), c);
        *curcolor = c;
    }
}

int _tmain(int argc, const TCHAR* argv[])
{
    arginit(argc, argv);
    bool color = _isatty(_fileno(stdout));
    if (argswitch(_T("/color"), _T("Use color")))
        color = true;
    if (argswitch(_T("/nocolor"), _T("Do not use color")))
        color = false;
    bool squeeze = !argswitch(_T("/nosqueeze"), _T("Do not skip over like lines"));
    const bool istty = _isatty(_fileno(stdin));
    const TCHAR* filename = argnext(!istty ? _T("-") : nullptr, _T("filename"), _T("input file (use '-' for stdin)"));
	if (!argcleanup())
        return EXIT_FAILURE;

    if (argusage(filename == nullptr))
        return EXIT_SUCCESS;

    FILE* input = nullptr;
    if (_tcscmp(filename, _T("-")) == 0)
    {
        input = stdin;
        int result = _setmode(_fileno(stdin), _O_BINARY);
        if (result == -1)
        {
            _ftprintf(stderr, _T("Error setting input mode: %d\n"), errno);
            return EXIT_FAILURE;
        }
    }
    else
    {
        errno_t e = _tfopen_s(&input, filename, _T("rb"));
        if (e != 0)
        {
            _ftprintf(stderr, _T("Error opening file: %d\n"), e);
            return EXIT_FAILURE;
        }
    }

    const size_t size = 16;
    int curcolor = 0;
    BYTE* data = new BYTE[size];
    BYTE* prev = new BYTE[size];
    size_t prevcount = 0;
    bool same = false;
    size_t offset = 0;
    while (true)
    {
        const size_t count = fread_s(data, size * sizeof(BYTE), 1, size, input);
        if (count == 0)
            break;

        if (squeeze && count == prevcount && memcmp(prev, data, count * sizeof(BYTE)) == 0)
        {
            if (!same)
                _tprintf(_T(" ***\n"));
            same = true;
        }
        else
        {
            same = false;

            if (color) setcolor(&curcolor, 33);
            _tprintf(_T("%08Xh:"), (unsigned int) offset);
            for (size_t i = 0; i < count; ++i)
            {
                bool isp = isprint(data[i]) != 0;
                if (color) setcolor(&curcolor, isp ? 34 : 0);
                _tprintf(_T(" %02X"), data[i]);
            }
            for (size_t i = count; i < size; ++i)
            {
                _tprintf(_T("   "));
            }
            _tprintf(_T(" "));
            for (size_t i = 0; i < count; ++i)
            {
                bool isp = isprint(data[i]) != 0;
                if (color) setcolor(&curcolor, isp ? 34 : 0);
                _tprintf(_T("%c"), isp ? data[i] : '.');
            }
            _tprintf(_T("\n"));

            memcpy(prev, data, count * sizeof(BYTE));
            prevcount = count;
        }
        offset += count;
    }
    if (color) setcolor(&curcolor, 0);

    delete[] data;
    delete[] prev;

    if (input != stdin)
        fclose(input);

    return EXIT_SUCCESS;
}
