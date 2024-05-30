//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#include <cstdlib>
#include <cstdio>
#include <memory>

template <typename T, typename D>
auto unique_resource(T* t, D&& d)
{
    return std::unique_ptr<T, D>(t, std::forward<D>(d));
}

inline const FILE_NOTIFY_INFORMATION* GetNext(const FILE_NOTIFY_INFORMATION* event)
{
    return event->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)((BYTE*)event + event->NextEntryOffset) : nullptr;
}

void OutputDirectoryChanges(HANDLE hDir, const BOOL bSubdir, const DWORD dwNotifyFilter)
{
    BYTE change_buf[1024];
    DWORD bytes = 0;
    if (!ReadDirectoryChangesW(hDir, change_buf, ARRAYSIZE(change_buf) * sizeof(change_buf[0]), bSubdir, dwNotifyFilter, &bytes, nullptr, nullptr))
    {
        _ftprintf(stderr, _T("Error ReadDirectoryChanges: 0x%08X\n"), GetLastError());
    }
    else
    {
        SYSTEMTIME time;
        GetLocalTime(&time);
        _tprintf(_T("%02u/%02u/%04u %02u:%02u:%02u.%03u"),
            time.wDay, time.wMonth, time.wYear,
            time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

        for (const FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*) change_buf; event != nullptr; event = GetNext(event))
        {
            const DWORD name_len = event->FileNameLength / sizeof(wchar_t);
            switch (event->Action)
            {
            case FILE_ACTION_ADDED:             wprintf(L"        Added: %.*s", name_len, event->FileName); break;
            case FILE_ACTION_REMOVED:           wprintf(L"      Removed: %.*s", name_len, event->FileName); break;
            case FILE_ACTION_MODIFIED:          wprintf(L"     Modified: %.*s", name_len, event->FileName); break;
            case FILE_ACTION_RENAMED_OLD_NAME:  wprintf(L" Renamed from: %.*s", name_len, event->FileName); break;
            case FILE_ACTION_RENAMED_NEW_NAME:  wprintf(L" to: %.*s", name_len, event->FileName); break;
            default:                            _tprintf(_T("Unknown action!")); break;
            }
        }
        _tprintf(_T("\n"));
    }
}

int _tmain(const int argc, const LPCTSTR argv[])
{
    LPCTSTR sDir = nullptr;
    BOOL bSubdir = FALSE;
    const DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE;
    for (int argi = 1; argi < argc; ++argi)
    {
        LPCTSTR arg = argv[argi];
        if (_tcsicmp(arg, _T("/s")) == 0)
            bSubdir = TRUE;
        else if (sDir == nullptr)
            sDir = arg;
        else
            _ftprintf(stderr, _T("Unknown argument: %s\n"), arg);
    }

    if (sDir == nullptr)
    {
        _tprintf(_T("Usage: DirWatch [/s] <directory>\n"));
        return EXIT_SUCCESS;
    }

    auto hDir = unique_resource(CreateFile(sDir,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL),
        [](HANDLE h) { CloseHandle(h); });
    if (!hDir)
    {
        _ftprintf(stderr, _T("Error CreateFile: 0x%08X\n"), GetLastError());
        return EXIT_FAILURE;
    }

#if 0
    auto hWatch = unique_resource(FindFirstChangeNotification(sDir, bSubdir, dwNotifyFilter), [](HANDLE h) { FindCloseChangeNotification(h); });
    if (!hWatch)
    {
        _ftprintf(stderr, _T("Error FindFirstChangeNotification: 0x%08X\n"), GetLastError());
        return EXIT_FAILURE;
    }

    OutputDirectoryChanges(hDir.get(), bSubdir, dwNotifyFilter);

    DWORD dwWait;
    while ((dwWait = WaitForSingleObject(hWatch.get(), INFINITE)) == WAIT_OBJECT_0)
    {
        if (FindNextChangeNotification(hWatch.get()) == FALSE)
        {
            _ftprintf(stderr, _T("Error FindNextChangeNotification: 0x%08X\n"), GetLastError());
            break;
        }

        OutputDirectoryChanges(hDir.get(), bSubdir, dwNotifyFilter);
    }

    if (dwWait != WAIT_OBJECT_0)
    {
        _ftprintf(stderr, _T("Error WaitForSingleObject: 0x%08X: 0x%08X\n"), dwWait, GetLastError());
        return EXIT_FAILURE;
    }
#else
    while (true)
        OutputDirectoryChanges(hDir.get(), bSubdir, dwNotifyFilter);
#endif

    return EXIT_SUCCESS;
}
