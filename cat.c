#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>

#define BOOL int
#define TRUE 1
#define FALSE 0

#include "arg.inl"

#define FILE_ERR ((FILE*) -1)

FILE* argfile(int i)
{
    const TCHAR* name = argnum(i);
    if (name == NULL)
        return NULL;
    else if (strcmp(name, "-") == 0)
        return stdin;
    else
    {
        FILE* f = NULL;
        errno_t e = fopen_s(&f, name, "r");
        if (e != 0)
        {
            TCHAR err[1024];
            strerror_s(err, 1024, e);
            _ftprintf(stderr, _T("Error: %d (%s)\n"), e, err);
            return FILE_ERR;
        }
        else
            return f;
    }
}

int _tmain(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv);
    BOOL unicode = argswitch("/U");
    FILE* i = argfile(0);
    FILE* o = stdout;
	if (!argcleanup())
        return EXIT_FAILURE;
    
    if (i == FILE_ERR)
        return EXIT_FAILURE;
    else if (i == NULL)
    {
        _ftprintf(stderr, _T("cat [/U] file\n"));
        _ftprintf(stderr, _T("\n"));
        _ftprintf(stderr, _T("Read and output a file to stdout.\n"));
        _ftprintf(stderr, _T("\n"));
        _ftprintf(stderr, _T("  file            The file to read ('-' for stdin)\n"));
        _ftprintf(stderr, _T("  /U              Use unicode mode\n"));
        return EXIT_FAILURE;
    }
        
    if (unicode)
    {
        wint_t c;
        while ((c = fgetwc(i)) != EOF)
        {
            fputwc(c, o);
        }
    }
    else
    {
        int c;
        while ((c = fgetc(i)) != EOF)
        {
            fputc(c, o);
        }
    }
    
    if (i != stdin)
        fclose(i);
    
    return EXIT_SUCCESS;
}
