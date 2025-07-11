#include <Windows.h>
#include <cstdio>

LPCTSTR VirtualKeyCodeToString(const WORD virtualKey, bool extended)
{
    switch (virtualKey)
    {
    case VK_SELECT: return TEXT("Select");
    case VK_PRINT: return TEXT("Print");
    case VK_EXECUTE: return TEXT("Execute");
    case VK_SNAPSHOT: return TEXT("Print Screen");
    case VK_HELP: return TEXT("Help");
    case VK_SLEEP: return TEXT("Sleep");
    case VK_BROWSER_BACK: return TEXT("Browser Back");
    case VK_BROWSER_FORWARD: return TEXT("Browser Forward");
    case VK_BROWSER_REFRESH: return TEXT("Browser Refresh");
    case VK_BROWSER_STOP: return TEXT("Browser Stop");
    case VK_BROWSER_SEARCH: return TEXT("Browser Search");
    case VK_BROWSER_FAVORITES: return TEXT("Browser Favorites");
    case VK_BROWSER_HOME: return TEXT("Browser Home");
    case VK_VOLUME_MUTE: return TEXT("Mute");
    case VK_VOLUME_DOWN: return TEXT("Volume Down");
    case VK_VOLUME_UP: return TEXT("Volume Up");
    case VK_MEDIA_NEXT_TRACK: return TEXT("Next Track");
    case VK_MEDIA_PREV_TRACK: return TEXT("Previous Track");
    case VK_MEDIA_STOP: return TEXT("Stop");
    case VK_MEDIA_PLAY_PAUSE: return TEXT("Play/Pause");
    case VK_LAUNCH_MAIL: return TEXT("Mail");
    case VK_LAUNCH_MEDIA_SELECT: return TEXT("Select Media");
    case VK_LAUNCH_APP1: return TEXT("App 1");
    case VK_LAUNCH_APP2: return TEXT("App 2");
    case VK_PLAY: return TEXT("Play");
    case VK_ZOOM: return TEXT("Zoom");
    case VK_OEM_CLEAR: return TEXT("Clear");
    default:
    {
        const UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | (extended ? KF_EXTENDED : 0);
        static TCHAR szName[128];
        GetKeyNameText(scanCode << 16, szName, ARRAYSIZE(szName));
        return szName;
    }
    };
}

int main()
{
    HANDLE hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

    while (true)
    {
        INPUT_RECORD ir;
        DWORD read;
        ReadConsoleInput(hConsoleIn, &ir, 1, &read);
        if (read > 0)
        {
            if (ir.EventType == KEY_EVENT)
            {
                const KEY_EVENT_RECORD& ke = ir.Event.KeyEvent;
                putchar(ke.bKeyDown ? '+' : '-'); putchar(' ');
                putchar(ke.dwControlKeyState & ENHANCED_KEY ? 'E' : ' '); putchar(' ');
                printf("%#.2x ", ke.wVirtualKeyCode);
                printf("%#.2x ", ke.wVirtualScanCode);
                printf("%3d ", ke.wRepeatCount);
                printf("%-10s ", VirtualKeyCodeToString(ke.wVirtualKeyCode, ke.dwControlKeyState & ENHANCED_KEY));
                putchar(isprint(ke.uChar.AsciiChar) ? ke.uChar.AsciiChar : ' '); putchar(' ');
                printf("%-5s ", ke.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) ? "Ctrl" : "");
                printf("%-5s ", ke.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) ? "Alt" : "");
                printf("%-5s ", ke.dwControlKeyState & (SHIFT_PRESSED) ? "Shift" : "");
                putchar('\n');
            }
        }
    }

    return 0;
}
