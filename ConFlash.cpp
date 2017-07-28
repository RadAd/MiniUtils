#include <Windows.h>

#pragma comment(lib, "User32.lib")

int main()
{
    FLASHWINFO fw = {};
    fw.cbSize = sizeof(fw);
    fw.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    fw.hwnd = GetConsoleWindow();
    FlashWindowEx(&fw);
    return 0;
}
