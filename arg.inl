#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

BOOL* g_argb = NULL;
int g_argc = 0;
const TCHAR* const* g_argv = NULL;
const TCHAR* g_argdescription = NULL;
BOOL g_argshowUsage = FALSE;
typedef struct TagArgArgDescription
{
    const TCHAR*  arg;
    const TCHAR*  value;
    const TCHAR*  desc;
} ArgArgDescription;
ArgArgDescription g_argargdescription[10];
int g_argargdescriptioncount = 0;
typedef struct TagArgArgNumDescription
{
    const TCHAR*  arg;
    const TCHAR*  desc;
} ArgArgNumDescription;
ArgArgNumDescription g_argargnumdescription[10];
int g_argargnumdescriptioncount = 0;
int g_argargnumoptional = 65536;

#ifdef __cplusplus
#define ARG_OPTIONAL(x) = x
#else
#define ARG_OPTIONAL(x)
#endif
#define ARG_COLOR(n, s) "\x1B[" #n "m" s "\x1B[0m"
#define ARG_COMMAND(s) ARG_COLOR(37, s)
#define ARG_OPTION(s) ARG_COLOR(33, s)
#define ARG_VALUE(s) ARG_COLOR(36, s)

BOOL argswitchdesc(const TCHAR* argf, const TCHAR* desc);

void arginit(int argc, const TCHAR* const argv[], const TCHAR* argdescription ARG_OPTIONAL(NULL))
{
    g_argc = argc;
    g_argv = argv;
    g_argb = (BOOL*) malloc(argc * sizeof(BOOL));
    for (int argi = 1; argi < g_argc; ++argi)
    {
        g_argb[argi] = FALSE;
    }

    g_argdescription = argdescription;
}

BOOL argcleanup()
{
    g_argshowUsage = argswitchdesc(_T("/?"), _T("Show usage"));
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

BOOL argusage(BOOL bforce ARG_OPTIONAL(FALSE))
{
    if (!bforce && !g_argshowUsage)
        return FALSE;

    if (g_argdescription != NULL)
        _ftprintf(stdout, _T(ARG_COMMAND("%s") " - %s\n\n"), argapp(), g_argdescription);
    else if (g_argargdescriptioncount == 0)
        _ftprintf(stdout, _T(ARG_COMMAND("%s") "\n\n"), argapp());
    if (g_argargdescriptioncount > 0)
    {
        _ftprintf(stdout, _T(ARG_COMMAND("%s")), argapp());
        int i;
        for (i = 0; i < g_argargdescriptioncount; ++i)
        {
            const ArgArgDescription* argargdescription = &g_argargdescription[i];
            if (argargdescription->value != NULL)
                _ftprintf(stdout, _T(" [" ARG_OPTION("%s") "=" ARG_VALUE("%s") "]"), argargdescription->arg, argargdescription->value);
            else
                _ftprintf(stdout, _T(" [" ARG_OPTION("%s") "]"), argargdescription->arg);
        }
        for (i = 0; i < g_argargnumdescriptioncount; ++i)
        {
            const ArgArgNumDescription* argargnumdescription = &g_argargnumdescription[i];
            if (i >= g_argargnumoptional)
                _ftprintf(stdout, _T(" [" ARG_OPTION("%s") "]"), argargnumdescription->arg);
            else
                _ftprintf(stdout, _T(" <" ARG_OPTION("%s")) ">", argargnumdescription->arg);
        }
        _ftprintf(stdout, _T("\n"));
        for (i = 0; i < g_argargdescriptioncount; ++i)
        {
            const ArgArgDescription* argargdescription = &g_argargdescription[i];
            if (argargdescription->desc != NULL)
                _ftprintf(stdout, _T("   " ARG_OPTION("%s") "\t%s\n"), argargdescription->arg, argargdescription->desc);
        }
        for (i = 0; i < g_argargnumdescriptioncount; ++i)
        {
            const ArgArgNumDescription* argargnumdescription = &g_argargnumdescription[i];
            if (argargnumdescription->desc != NULL)
                _ftprintf(stdout, _T("   " ARG_OPTION("%s") "\t%s\n"), argargnumdescription->arg, argargnumdescription->desc);
        }
        _ftprintf(stdout, _T("\n"));
    }
    return TRUE;
}

BOOL argswitch(const TCHAR* argf)
{
    ArgArgDescription* argargdescription = &g_argargdescription[g_argargdescriptioncount++];
    argargdescription->arg = argf;
    argargdescription->value = NULL;
    argargdescription->desc = NULL;
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

const TCHAR* argvalue(const TCHAR* argf, const TCHAR* def ARG_OPTIONAL(NULL))
{
    ArgArgDescription* argargdescription = &g_argargdescription[g_argargdescriptioncount++];
    argargdescription->arg = argf;
    argargdescription->value = _T("<unknown>");
    argargdescription->desc = NULL;
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

const TCHAR* argnum(int i, const TCHAR* def ARG_OPTIONAL(NULL))
{
    ArgArgNumDescription* argargnumdescription = &g_argargnumdescription[g_argargnumdescriptioncount++];
    argargnumdescription->arg = _T("unknown");
    argargnumdescription->desc = NULL;
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

BOOL argswitchdesc(const TCHAR* argf, const TCHAR* desc)
{
    ArgArgDescription* argargdescription = &g_argargdescription[g_argargdescriptioncount];
    BOOL ret = argswitch(argf);
    argargdescription->desc = desc;
    return ret;
}

const TCHAR* argvaluedesc(const TCHAR* argf, const TCHAR* def, const TCHAR* descvalue, const TCHAR* desc)
{
    ArgArgDescription* argargdescription = &g_argargdescription[g_argargdescriptioncount];
    const TCHAR* ret = argvalue(argf, def);
    if (descvalue != NULL) argargdescription->value = descvalue;
    argargdescription->desc = desc;
    return ret;
}

const TCHAR* argnumdesc(int i, const TCHAR* def, const TCHAR* descvalue, const TCHAR* desc)
{
    ArgArgNumDescription* argargnumdescription = &g_argargnumdescription[g_argargnumdescriptioncount];
    const TCHAR* ret = argnum(i, def);
    if (descvalue != NULL) argargnumdescription->arg = descvalue;
    argargnumdescription->desc = desc;
    return ret;
}

void argoptional()
{
    g_argargnumoptional = g_argargnumdescriptioncount;
}
