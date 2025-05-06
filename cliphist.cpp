// https://devblogs.microsoft.com/oldnewthing/20230302-00/?p=107889
// eg cliphist.exe | fzf -d "\x1F" --with-nth=2 --bind "enter:become(echo {1})" --preview "cliphist view {1}" | lineargs cliphist select {}

#include <stdio.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <cstring>
#include <string>
#include <tchar.h>
#include "arg.inl"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
}

void replace_all(std::wstring& str, std::wstring_view find, std::wstring_view replace)
{
    std::wstring::size_type pos = 0;
    while ((pos = str.find(find, pos)) != std::wstring::npos)
    {
        str.replace(pos, find.length(), replace);
    }
}

winrt::ClipboardHistoryItem FindClipboardHistoryItem(winrt::ClipboardHistoryItemsResult result, std::wstring_view id)
{
    for (winrt::ClipboardHistoryItem item : result.Items())
    {
        if (item.Id() == id)
            return item;
    }
    return nullptr;
}

winrt::IAsyncAction DumpClipboardHistoryAsync()
{
    winrt::ClipboardHistoryItemsResult result = co_await winrt::Clipboard::GetHistoryItemsAsync();
    for (winrt::ClipboardHistoryItem item : result.Items())
    {
        winrt::DataPackageView content = item.Content();
        if (content.Contains(winrt::StandardDataFormats::Text()))
        {
            winrt::hstring text = co_await content.GetTextAsync();
            std::wstring text2(text);
            replace_all(text2, L"\r\n", L"|");
            wprintf(L"%s\x1F %.80s\n", item.Id().c_str(), text2.c_str());
        }
        else
        {
            wprintf(L"%s\x1F (no text content)\n", item.Id().c_str());
        }
    }
}

winrt::IAsyncAction DumpClipboardHistoryItemAsync(std::wstring_view id)
{
    winrt::ClipboardHistoryItemsResult result = co_await winrt::Clipboard::GetHistoryItemsAsync();
    winrt::ClipboardHistoryItem item = FindClipboardHistoryItem(result, id);
    if (!item)
    {
        fwprintf(stderr, L"Clipboard history item not found.\n");
        co_return;
    }

    winrt::DataPackageView content = item.Content();
    if (content.Contains(winrt::StandardDataFormats::Text()))
    {
        winrt::hstring text = co_await content.GetTextAsync();
        wprintf(L"%s", text.c_str());
    }
    else
    {
        wprintf(L"(no text content)\n");
    }
}

winrt::IAsyncAction SelectClipboardHistoryItemAsync(std::wstring_view id)
{
    winrt::ClipboardHistoryItemsResult result = co_await winrt::Clipboard::GetHistoryItemsAsync();
    winrt::ClipboardHistoryItem item = FindClipboardHistoryItem(result, id);
    if (!item)
    {
        fwprintf(stderr, L"Clipboard history item not found.\n");
        co_return;
    }

    winrt::Clipboard::SetHistoryItemAsContent(item);
}

int wmain(const int argc, const wchar_t* argv[])
{
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    if (!winrt::Clipboard::IsHistoryEnabled())
    {
        fwprintf(stderr, L"Clipboard history not enabled.\n");
        return 1;
    }

    arginit(argc, argv, L"Show and select clipboard history");

    argoptional();
    const TCHAR* cmd = argnumdesc(1, nullptr, L"command", L"Command option: list, view or select");
    const TCHAR* item = argnumdesc(2, nullptr, L"clip_id", L"Clip item target");

    if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage())
        return EXIT_SUCCESS;

    if (cmd == nullptr || _wcsicmp(cmd, L"list") == 0)
        DumpClipboardHistoryAsync().get();
    else if (item == nullptr)
    {
        fwprintf(stderr, L"Error: Clip id missing\n");
        return EXIT_FAILURE;
    }
    else if (_wcsicmp(cmd, L"view") == 0)
        DumpClipboardHistoryItemAsync(item).get();
    else if (_wcsicmp(cmd, L"select") == 0)
        SelectClipboardHistoryItemAsync(item).get();
    else
    {
        fwprintf(stderr, L"Error: Unknown command: %s\n", cmd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
