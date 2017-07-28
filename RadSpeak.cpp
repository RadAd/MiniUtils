#include <sapi.h>
#include <Windows.H>
#include <tchar.h>

#ifndef UNICODE
#error set COMPILE_CHARSET=Unicode
#endif
#pragma comment(lib, "ole32.lib")

HRESULT SpeakText(ISpVoice* pVoice, const TCHAR* Text, bool xml, bool file)
{
    DWORD Flags = SPF_DEFAULT;
    if (xml)
        Flags |= SPF_IS_XML;
    else
        Flags |= SPF_IS_NOT_XML;
    if (file)
        Flags |= SPF_IS_FILENAME;

    return pVoice->Speak(Text, Flags, 0);
}

int _tmain(int argc, TCHAR* argv[])
{
    CoInitialize(NULL);

    ISpVoice*   pVoice = nullptr;
    HRESULT hr = ::CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, __uuidof(ISpVoice), (void**) &pVoice);
    if (FAILED(hr))
        return hr;

    int arg = 1;
    bool xml = false;
    bool file = false;
    while (arg < argc)
    {
        if (argv[arg][0] != L'/')
            SpeakText(pVoice, argv[arg], xml, file);
        else if (wcscmp(argv[arg], L"/xml") == 0)
            xml = true;
        else if (wcscmp(argv[arg], L"/-xml") == 0)
            xml = false;
        else if (wcscmp(argv[arg], L"/file") == 0)
            file = true;
        else if (wcscmp(argv[arg], L"/-file") == 0)
            file = false;
        else
            _ftprintf(stderr, _T("Unknown option: %s\n"), argv[arg]);
        ++arg;
    }

    pVoice->Release();

    CoUninitialize();
    return 0;
}
