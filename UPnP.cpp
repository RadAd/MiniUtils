#include <SDKDDKVer.h>
#include <Objbase.h>
#include <Upnp.h>
#include <Natupnp.h>
#include <atlbase.h>
#include <atlconv.h>
#include <stdio.h>
#include <tchar.h>
#include <exception>

void OutputDetails(IUPnPDevice* pDevice)
{
    CComBSTR bStr;
    VARIANT_BOOL bValue = VARIANT_FALSE;
    if (SUCCEEDED(pDevice->get_Description(&bStr)))
        _tprintf(_T("Description: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_FriendlyName(&bStr)))
        _tprintf(_T("Friendly Name: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_HasChildren(&bValue)))
        _tprintf(_T("Has Children: %s\n"), (bValue == VARIANT_FALSE) ? _T("No") : _T("Yes"));
    CComBSTR bStrMime = OLESTR("image/gif");
    if (SUCCEEDED(pDevice->IconURL(bStrMime, 32, 32, 8, &bStr)))
        _tprintf(_T("Icon URL: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_IsRootDevice(&bValue)))
        _tprintf(_T("Is Root Device: %s\n"), (bValue == VARIANT_FALSE) ? _T("No") : _T("Yes"));
    if (SUCCEEDED(pDevice->get_FriendlyName(&bStr)))
        _tprintf(_T("Friendly Name: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_ManufacturerName(&bStr)))
        _tprintf(_T("Manufacturer Name: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_ManufacturerURL(&bStr)))
        _tprintf(_T("Manufacturer URL: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_ModelName(&bStr)))
        _tprintf(_T("Model Name: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_ModelNumber(&bStr)))
        _tprintf(_T("Model Number: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_ModelURL(&bStr)))
        _tprintf(_T("Model URL: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_PresentationURL(&bStr)))
        _tprintf(_T("Presentation URL: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_SerialNumber(&bStr)))
        _tprintf(_T("Serial Number: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_Type(&bStr)))
        _tprintf(_T("Type: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_UniqueDeviceName(&bStr)))
        _tprintf(_T("Unique Device Name: %s\n"), OLE2T(bStr));
    if (SUCCEEDED(pDevice->get_UPC(&bStr)))
        _tprintf(_T("UPC: %s\n"), OLE2T(bStr));
    // get_Children
    // get_ParentDevice
    // get_RootDevice
    // get_Services
    _tprintf(_T("\n"));
}

void OutputHeadingIStaticPortMapping()
{
    _tprintf(_T("%15s, "), _T("External Address"));
    _tprintf(_T("%5s, "), _T("External Port"));
    _tprintf(_T("%5s, "), _T("Internal Port"));
    _tprintf(_T("%3s, "), _T("Protocol"));
    _tprintf(_T("%15s, "), _T("Internal Client"));
    _tprintf(_T("%-3s, "), _T("Enabled"));
    _tprintf(_T("%s"), _T("Description"));
    _tprintf(_T("\n"));
}

void OutputDetails(IStaticPortMapping* piDispMap)
{
    CComBSTR bStr;
    long lValue = 0;
    VARIANT_BOOL bValue = VARIANT_FALSE;
    if (SUCCEEDED(piDispMap->get_ExternalIPAddress(&bStr)))
        _tprintf(_T("%15s, "), OLE2T(bStr));
    if (SUCCEEDED(piDispMap->get_ExternalPort(&lValue)))
        _tprintf(_T("%5d, "), lValue);
    if (SUCCEEDED(piDispMap->get_InternalPort(&lValue)))
        _tprintf(_T("%5d, "), lValue);
    if (SUCCEEDED(piDispMap->get_Protocol(&bStr)))
        _tprintf(_T("%3s, "), OLE2T(bStr));
    if (SUCCEEDED(piDispMap->get_InternalClient(&bStr)))
        _tprintf(_T("%15s, "), OLE2T(bStr));
    if (SUCCEEDED(piDispMap->get_Enabled(&bValue)))
        _tprintf(_T("%-3s, "), (bValue == VARIANT_FALSE) ? _T("No") : _T("Yes"));
    if (SUCCEEDED(piDispMap->get_Description(&bStr)))
        _tprintf(_T("%s"), OLE2T(bStr));
    _tprintf(_T("\n"));
}

template<class T>
void OutputDetails(IDispatch* pIDispatch)
{
    CComPtr<T> pT;
    if (SUCCEEDED(pIDispatch->QueryInterface(&pT)))
    {
        OutputDetails(pT);
    }
}

template<class T>
void Enum(IUnknown* pIUnknown)
{
    CComPtr<IEnumVARIANT> pIEnumVARIANT;
    if (!SUCCEEDED(pIUnknown->QueryInterface(&pIEnumVARIANT)))
        throw std::exception("QueryInterface IEnumVARIANT");

    pIEnumVARIANT->Reset();
    CComVariant varCurrent;
    while (S_OK == pIEnumVARIANT->Next(1, &varCurrent, NULL))
    {
        IDispatch* pIDispatch = V_DISPATCH(&varCurrent);
        OutputDetails<T>(pIDispatch);
    }
}

void ListGatewayDevices()
{
    CComPtr<IUPnPDeviceFinder> piDeviceFinder;
    if (!SUCCEEDED(piDeviceFinder.CoCreateInstance(CLSID_UPnPDeviceFinder)))
        throw std::exception("CoCreateInstance CLSID_UPnPDeviceFinder");

    CComPtr<IUPnPDevices> piFoundDevices;
    {
        CComBSTR bStrDev = OLESTR("urn:schemas-upnp-org:device:InternetGatewayDevice:1");
        if (!SUCCEEDED(piDeviceFinder->FindByType(bStrDev, 0, &piFoundDevices)))
            throw std::exception("IUPnPDevices FindByType");
    }

    CComPtr<IUnknown> pUnk;
    if (!SUCCEEDED(piFoundDevices->get__NewEnum(&pUnk)))
        throw std::exception("IUPnPDevices NewEnum");

    Enum<IUPnPDevice>(pUnk);
}

void ListPortMapping()
{
    CComPtr<IUPnPNAT> piNAT;
    if (!SUCCEEDED(piNAT.CoCreateInstance(__uuidof(UPnPNAT))))
        throw std::exception("CoCreateInstance UPnPNAT");

    CComPtr<IStaticPortMappingCollection> piPortMappingCollection;
    if (!SUCCEEDED(piNAT->get_StaticPortMappingCollection(&piPortMappingCollection)))
        throw std::exception("get_StaticPortMappingCollection");

    if (!piPortMappingCollection)
    {
        _tprintf(_T("No Port Mapping Collection\n"));
        return;
    }

    CComPtr<IUnknown> pUnk;
    if (!SUCCEEDED(piPortMappingCollection->get__NewEnum(&pUnk)))
        throw std::exception("IUPnPDevices NewEnum");

    OutputHeadingIStaticPortMapping();
    Enum<IStaticPortMapping>(pUnk);
}

int main()
{
    int ret = 0;
    CoInitialize(NULL);

    try
    {
        ListGatewayDevices();
        ListPortMapping();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Error: %s\n", e.what());
        ret = 1;
    }

    CoUninitialize();

    return ret;
}

