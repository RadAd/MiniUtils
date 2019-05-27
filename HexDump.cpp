#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <ctype.h>
#include <stdlib.h>

typedef unsigned char BYTE;

int _tmain(int argc, const TCHAR* argv[])
{
    const TCHAR* filename = nullptr;
    for (int i = 1; i < argc; ++i)
    {
        if (filename == nullptr)
            filename = argv[i];
        else
            _ftprintf(stderr, _T("Too many parameters.\n"));
    }

    if (!_isatty(_fileno(stdin)))
        filename = _T("-");

    FILE* input = nullptr;
    if (filename == nullptr)
    {
        _ftprintf(stderr, _T("HexDump <filename>\n"));
        return EXIT_FAILURE;
    }
    else if (_tcscmp(filename, _T("-")) == 0)
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
    BYTE* data = new BYTE[size];
    size_t offset = 0;
    while (true)
    {
        size_t count = fread_s(data, size * sizeof(BYTE), 1, size, input);
        if (count == 0)
            break;

        _tprintf(_T("%08Xh:"), (unsigned int) offset);
        for (size_t i = 0; i < count; ++i)
        {
            _tprintf(_T(" %02X"), data[i]);
        }
        for (size_t i = count; i < size; ++i)
        {
            _tprintf(_T("   "));
        }
        _tprintf(_T(" "));
        for (size_t i = 0; i < count; ++i)
        {
            _tprintf(_T("%c"), isprint(data[i]) ? data[i] : '.');
        }
        _tprintf(_T("\n"));

        offset += count;
    }

    delete[] data;

    if (input != stdin)
        fclose(input);

    return EXIT_SUCCESS;
}
