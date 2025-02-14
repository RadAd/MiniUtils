#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>

#define BOOL int
#define TRUE 1
#define FALSE 0

#include "arg.inl"

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#define FILE_ERR ((FILE*) -1)

FILE* argfile(int i, FILE* def, const TCHAR* mode, const TCHAR* descvalue, const TCHAR* desc)
{
    const TCHAR* name = argnumdesc(i, NULL, descvalue, desc);
    if (name == NULL)
        return NULL;
    else if (strcmp(name, "-") == 0)
        return def;
    else
    {
        FILE* f = NULL;
        errno_t e = _tfopen_s(&f, name, mode);
        if (e != 0)
        {
            TCHAR err[1024];
            strerror_s(err, ARRAYSIZE(err), e);
            _ftprintf(stderr, _T("Error: %d (%s)\n"), e, err);
            return FILE_ERR;
        }
        else
            return f;
    }
}

typedef struct
{
    FILE** files;
    int capacity;
    int size;
} FileList;

void appendFileList(FileList* fl, FILE* nf)
{
    if (fl->size == fl->capacity)
    {
        fl->capacity *= 2;
        fl->files = realloc(fl->files, fl->capacity * sizeof(FILE*));
    }
    fl->files[fl->size++] = nf;
}

FileList argfilelist(int i, FILE* def, const TCHAR* mode, const TCHAR* descvalue, const TCHAR* desc)
{
    FileList fl = {
        malloc(1 * sizeof(FILE*)),
        1,
        0
    };
    
    FILE* nf = NULL;
    while ((nf = argfile(i++, def, mode, descvalue, desc)) != NULL)
    {
        appendFileList(&fl, nf);
    }
    
    return fl;
}

BOOL fileListErr(const FileList* fl)
{
    for (int i = 0; i < fl->size; ++i)
    {
        if (fl->files[i] == FILE_ERR)
            return TRUE;
    }
    return FALSE;
}

int _tmain(int argc, const TCHAR* const argv[])
{
    arginit(argc, argv, _T("Read and output a file to stdout"));
    BOOL unicode = argswitchdesc(_T("/U"), _T("Use unicode mode"));
    FILE* i = argfile(1, stdin, _T("rb"), _T("input"), _T("The file to read ('-' for stdin, 'CONIN$' for console)"));   // TODO Should be a mandatory option
    argoptional();
    FileList o = argfilelist(2, stdout, _T("wb"), _T("output..."), _T("The file(s) to write ('-' for stdout, 'CONOUT$' for console)")); // TODO Should be an optional option (with many)
	if (!argcleanup())
        return EXIT_FAILURE;
	if (argusage(i == NULL))
        return EXIT_SUCCESS;
    
    if (i == FILE_ERR || fileListErr(&o))
        return EXIT_FAILURE;

    if (o.size == 0)
        appendFileList(&o, stdout);
    
    if (unicode)
    {
        wint_t c;
        while ((c = fgetwc(i)) != EOF)
        {
            for (int j = 0; j < o.size; ++j)
                fputwc(c, o.files[j]);
        }
    }
    else
    {
        int c;
        while ((c = fgetc(i)) != EOF)
        {
            for (int j = 0; j < o.size; ++j)
                fputc(c, o.files[j]);
        }
    }
    
    if (i != stdin)
        fclose(i);
    
    for (int j = 0; j < o.size; ++j)
    {
        if (o.files[j] != stdout)
            fclose(o.files[j]);
    }
    
    free(o.files);
    
    return EXIT_SUCCESS;
}
