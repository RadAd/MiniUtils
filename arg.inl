#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

BOOL* g_argb = NULL;
int g_argc = 0;
const TCHAR* const* g_argv = NULL;

void arginit(int argc, const TCHAR* const argv[])
{
    g_argc = argc;
    g_argv = argv;
    g_argb = (BOOL*) malloc(argc * sizeof(BOOL));
    for (int argi = 1; argi < g_argc; ++argi)
    {
        g_argb[argi] = FALSE;
    }
}

BOOL argcleanup()
{
    BOOL ret = TRUE;

    for (int argi = 1; argi < g_argc; ++argi)
    {
        if (!g_argb[argi])
        {
            const TCHAR* arg = g_argv[argi];
            if (arg[0] == '/')
            {
                _ftprintf(stderr, _T("Unknown option: \"%s\".\n"), arg);
                ret = FALSE;
            }
            else
            {
                _ftprintf(stderr, _T("Unknown argument: \"%s\".\n"), arg);
                ret = FALSE;
            }
        }
    }

    free(g_argb);
    g_argb = NULL;

    return ret;
}

const TCHAR* argapp()
{
    const TCHAR* app = _tcsrchr(g_argv[0], _T('\\'));
    return app == NULL ? g_argv[0] : app + 1;
}

BOOL argswitch(const TCHAR* argf)
{
    for (int argi = 1; argi < g_argc; ++argi)
    {
        const TCHAR* arg = g_argv[argi];
        if (_tcsicmp(arg, argf) == 0)
        {
            g_argb[argi] = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef __cplusplus
const TCHAR* argvalue(const TCHAR* argf, const TCHAR* def = NULL)
{
#else
const TCHAR* argvalue(const TCHAR* argf)
{
    const TCHAR* def = NULL;
#endif
    const size_t len = _tcslen(argf);
    for (int argi = 1; argi < g_argc; ++argi)
    {
        const TCHAR* arg = g_argv[argi];
        if (_tcsnicmp(arg, argf, len) == 0 && arg[len] == _T('='))
        {
            g_argb[argi] = TRUE;
            return arg + len + 1;
        }
    }
    return def;
}

#ifdef __cplusplus
const TCHAR* argnum(int i, const TCHAR* def = NULL)
{
#else
const TCHAR* argnum(int i)
{
    const TCHAR* def = NULL;
#endif
    for (int argi = 1; argi < g_argc && i >= 0; ++argi)
    {
        const TCHAR* arg = g_argv[argi];
        if (arg[0] != '/')
        {
            --i;
            if (i == 0)
            {
                g_argb[argi] = TRUE;
                return arg;
            }
        }
    }
    if (i < 0)
        _ftprintf(stderr, _T("Error argument number\n"));
    return def;
}
