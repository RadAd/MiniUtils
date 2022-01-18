// https://docs.microsoft.com/en-us/previous-versions/windows/apps/hh465448(v=win.10)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <wrl/implements.h>
#include <inspectable.h>
#include <windows.ui.notifications.h>
#include <windows.data.xml.dom.h>
#include <string>
#include <vector>

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;

inline void Check(HRESULT hr)
{
    if (!SUCCEEDED(hr))
    {
        throw std::exception();
    }
}

class HStringRef
{
public:
    HStringRef(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) noexcept
    {
        Check(WindowsCreateStringReference(stringRef, length, &_header, &_hstring));
    }

    HStringRef(_In_ const std::wstring& stringRef) noexcept
    {
        Check(WindowsCreateStringReference(stringRef.c_str(), static_cast<UINT32>(stringRef.length()), &_header, &_hstring));
    }

    ~HStringRef()
    {
        WindowsDeleteString(_hstring);
    }

    inline operator HSTRING() const noexcept
    {
        return _hstring;
    }

private:
    HSTRING _hstring;
    HSTRING_HEADER _header;
};

inline void setNodeStringValue(const std::wstring& string, IXmlNode* node, IXmlDocument* xml)
{
    ComPtr<IXmlText> textNode;
    Check(xml->CreateTextNode(HStringRef(string), &textNode));

    ComPtr<IXmlNode> stringNode;
    Check(textNode.As(&stringNode));

    ComPtr<IXmlNode> appendedChild;
    Check(node->AppendChild(stringNode.Get(), &appendedChild));
}


void setTextFieldHelper(_In_ IXmlDocument* xml, _In_ const std::wstring& text, _In_ UINT32 pos)
{
    ComPtr<IXmlNodeList> nodeList;
    Check(xml->GetElementsByTagName(HStringRef(L"text"), &nodeList));

    ComPtr<IXmlNode> node;
    Check(nodeList->Item(pos, &node));

    setNodeStringValue(text, node.Get(), xml);
}

void setImageFieldHelper(_In_ IXmlDocument* xml, _In_ const std::wstring& path)
{
    //assert(path.size() < MAX_PATH);

    ComPtr<IXmlNodeList> nodeList;
    Check(xml->GetElementsByTagName(HStringRef(L"image"), &nodeList));

    ComPtr<IXmlNode> node;
    Check(nodeList->Item(0, &node));

    ComPtr<IXmlNamedNodeMap> attributes;
    Check(node->get_Attributes(&attributes));

    ComPtr<IXmlNode> editedNode;
    Check(attributes->GetNamedItem(HStringRef(L"src"), &editedNode));

    setNodeStringValue(path, editedNode.Get(), xml);
}

template<class T>
HRESULT GetActivationFactory(const std::wstring& text, ComPtr<T>& p)
{
    return RoGetActivationFactory(HStringRef(text), IID_INS_ARGS(p.ReleaseAndGetAddressOf()));
}

class CoInit
{
public:
    CoInit(_In_ DWORD dwCoInit)
    {
        Check(CoInitializeEx(nullptr, dwCoInit));
    }
    ~CoInit()
    {
        CoUninitialize();
    }
};

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
    try
    {
        CoInit ci(COINIT::COINIT_MULTITHREADED);

        std::wstring applicationid = L"Rad Toast";
        std::vector<std::wstring> text;
        std::wstring imagepath;

        for (int argi = 1; argi < __argc; ++argi)
        {
            LPCTSTR arg = __wargv[argi];
            text.push_back(arg);
        }

        if (text.empty())
        {
            MessageBoxW(NULL, L"No text lines.", L"Rad Toast", MB_OK | MB_ICONERROR);
            return 2;
        }

        const ToastTemplateType type = static_cast<ToastTemplateType>((imagepath.empty() ? ToastTemplateType::ToastTemplateType_ToastText01 : ToastTemplateType::ToastTemplateType_ToastImageAndText01) + text.size() - 1);

        ComPtr<IToastNotificationManagerStatics> notificationManager;
        Check(GetActivationFactory(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager, notificationManager));

        ComPtr<IToastNotifier> notifier;
        Check(notificationManager->CreateToastNotifierWithId(HStringRef(applicationid), &notifier));

        ComPtr<IToastNotificationFactory> notificationFactory;
        Check(GetActivationFactory(RuntimeClass_Windows_UI_Notifications_ToastNotification, notificationFactory));

        ComPtr<IXmlDocument> xmlDocument;
        Check(notificationManager->GetTemplateContent(type, &xmlDocument));

        for (size_t i = 0; i < text.size(); ++i)
            setTextFieldHelper(xmlDocument.Get(), text[i], static_cast<UINT32>(i));
        if (!imagepath.empty())
            setImageFieldHelper(xmlDocument.Get(), L"file:///" + imagepath);

        ComPtr<IToastNotification> notification;
        Check(notificationFactory->CreateToastNotification(xmlDocument.Get(), &notification));

        Check(notifier->Show(notification.Get()));

        Sleep(1);

        return 0;
    }
    catch (const std::exception& e)
    {
        MessageBoxW(NULL, L"Exception", L"Rad Toast", MB_OK | MB_ICONERROR);
        return 1;
    }
}
