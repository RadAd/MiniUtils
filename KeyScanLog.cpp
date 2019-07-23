#include <Windows.h>
#include <cstdio>

#define caseStringify(x) case x: return #x

const char* GetKeyName(WORD vk) {
    switch (vk) {
        caseStringify('0');
        caseStringify('1');
        caseStringify('2');
        caseStringify('3');
        caseStringify('4');
        caseStringify('5');
        caseStringify('6');
        caseStringify('7');
        caseStringify('8');
        caseStringify('9');
        // 0x40 : unassigned
        caseStringify('A');
        caseStringify('B');
        caseStringify('C');
        caseStringify('D');
        caseStringify('E');
        caseStringify('F');
        caseStringify('G');
        caseStringify('H');
        caseStringify('I');
        caseStringify('J');
        caseStringify('K');
        caseStringify('L');
        caseStringify('M');
        caseStringify('N');
        caseStringify('O');
        caseStringify('P');
        caseStringify('Q');
        caseStringify('R');
        caseStringify('S');
        caseStringify('T');
        caseStringify('U');
        caseStringify('V');
        caseStringify('W');
        caseStringify('X');
        caseStringify('Y');
        caseStringify('Z');
        caseStringify(VK_LBUTTON);
        caseStringify(VK_RBUTTON);
        caseStringify(VK_CANCEL);
        caseStringify(VK_MBUTTON);  // NOT contiguous with L & RBUTTON
        caseStringify(VK_XBUTTON1); // NOT contiguous with L & RBUTTON
        caseStringify(VK_XBUTTON2); // NOT contiguous with L & RBUTTON
        caseStringify(VK_BACK);
        caseStringify(VK_TAB);
        caseStringify(VK_CLEAR);
        caseStringify(VK_RETURN);
        caseStringify(VK_SHIFT);
        caseStringify(VK_CONTROL);
        caseStringify(VK_MENU);
        caseStringify(VK_PAUSE);
        caseStringify(VK_CAPITAL);
        caseStringify(VK_KANA);
        caseStringify(VK_JUNJA);
        caseStringify(VK_FINAL);
        caseStringify(VK_KANJI);
        caseStringify(VK_ESCAPE);
        caseStringify(VK_CONVERT);
        caseStringify(VK_NONCONVERT);
        caseStringify(VK_ACCEPT);
        caseStringify(VK_MODECHANGE);
        caseStringify(VK_SPACE);
        caseStringify(VK_PRIOR);
        caseStringify(VK_NEXT);
        caseStringify(VK_END);
        caseStringify(VK_HOME);
        caseStringify(VK_LEFT);
        caseStringify(VK_UP);
        caseStringify(VK_RIGHT);
        caseStringify(VK_DOWN);
        caseStringify(VK_SELECT);
        caseStringify(VK_PRINT);
        caseStringify(VK_EXECUTE);
        caseStringify(VK_SNAPSHOT);
        caseStringify(VK_INSERT);
        caseStringify(VK_DELETE);
        caseStringify(VK_HELP);
        caseStringify(VK_LWIN);
        caseStringify(VK_RWIN);
        caseStringify(VK_APPS);
        caseStringify(VK_SLEEP);
        caseStringify(VK_NUMPAD0);
        caseStringify(VK_NUMPAD1);
        caseStringify(VK_NUMPAD2);
        caseStringify(VK_NUMPAD3);
        caseStringify(VK_NUMPAD4);
        caseStringify(VK_NUMPAD5);
        caseStringify(VK_NUMPAD6);
        caseStringify(VK_NUMPAD7);
        caseStringify(VK_NUMPAD8);
        caseStringify(VK_NUMPAD9);
        caseStringify(VK_MULTIPLY);
        caseStringify(VK_ADD);
        caseStringify(VK_SEPARATOR);
        caseStringify(VK_SUBTRACT);
        caseStringify(VK_DECIMAL);
        caseStringify(VK_DIVIDE);
        caseStringify(VK_F1);
        caseStringify(VK_F2);
        caseStringify(VK_F3);
        caseStringify(VK_F4);
        caseStringify(VK_F5);
        caseStringify(VK_F6);
        caseStringify(VK_F7);
        caseStringify(VK_F8);
        caseStringify(VK_F9);
        caseStringify(VK_F10);
        caseStringify(VK_F11);
        caseStringify(VK_F12);
        caseStringify(VK_F13);
        caseStringify(VK_F14);
        caseStringify(VK_F15);
        caseStringify(VK_F16);
        caseStringify(VK_F17);
        caseStringify(VK_F18);
        caseStringify(VK_F19);
        caseStringify(VK_F20);
        caseStringify(VK_F21);
        caseStringify(VK_F22);
        caseStringify(VK_F23);
        caseStringify(VK_F24);
        caseStringify(VK_NUMLOCK);
        caseStringify(VK_SCROLL);
        caseStringify(VK_OEM_NEC_EQUAL);  // '=' key on numpad
        caseStringify(VK_OEM_FJ_MASSHOU); // 'Unregister word' key
        caseStringify(VK_OEM_FJ_TOUROKU); // 'Register word' key
        caseStringify(VK_OEM_FJ_LOYA);    // 'Left OYAYUBI' key
        caseStringify(VK_OEM_FJ_ROYA);    // 'Right OYAYUBI' key
        caseStringify(VK_LSHIFT);
        caseStringify(VK_RSHIFT);
        caseStringify(VK_LCONTROL);
        caseStringify(VK_RCONTROL);
        caseStringify(VK_LMENU);
        caseStringify(VK_RMENU);
        caseStringify(VK_BROWSER_BACK);
        caseStringify(VK_BROWSER_FORWARD);
        caseStringify(VK_BROWSER_REFRESH);
        caseStringify(VK_BROWSER_STOP);
        caseStringify(VK_BROWSER_SEARCH);
        caseStringify(VK_BROWSER_FAVORITES);
        caseStringify(VK_BROWSER_HOME);
        caseStringify(VK_VOLUME_MUTE);
        caseStringify(VK_VOLUME_DOWN);
        caseStringify(VK_VOLUME_UP);
        caseStringify(VK_MEDIA_NEXT_TRACK);
        caseStringify(VK_MEDIA_PREV_TRACK);
        caseStringify(VK_MEDIA_STOP);
        caseStringify(VK_MEDIA_PLAY_PAUSE);
        caseStringify(VK_LAUNCH_MAIL);
        caseStringify(VK_LAUNCH_MEDIA_SELECT);
        caseStringify(VK_LAUNCH_APP1);
        caseStringify(VK_LAUNCH_APP2);
        caseStringify(VK_OEM_1);      // ';:' for US
        caseStringify(VK_OEM_PLUS);   // '+' any country
        caseStringify(VK_OEM_COMMA);  // ',' any country
        caseStringify(VK_OEM_MINUS);  // '-' any country
        caseStringify(VK_OEM_PERIOD); // '.' any country
        caseStringify(VK_OEM_2);  // '/?' for US
        caseStringify(VK_OEM_3);  // '`~' for US
        caseStringify(VK_OEM_4);  //  '[{' for US
        caseStringify(VK_OEM_5);  //  '\|' for US
        caseStringify(VK_OEM_6);  //  ']}' for US
        caseStringify(VK_OEM_7);  //  ''"' for US
        caseStringify(VK_OEM_8);
        caseStringify(VK_OEM_AX);   //  'AX' key on Japanese AX kbd
        caseStringify(VK_OEM_102);  //  "<>" or "\|" on RT 102-key kbd.
        caseStringify(VK_ICO_HELP); //  Help key on ICO
        caseStringify(VK_ICO_00);   //  00 key on ICO
        caseStringify(VK_PROCESSKEY);
        caseStringify(VK_ICO_CLEAR);
        caseStringify(VK_PACKET);
        default: return "";
    }
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
                printf("%#.2x ", ke.wVirtualKeyCode);
                printf("%3d ", ke.wRepeatCount);
                printf("%-10s ", GetKeyName(ke.wVirtualKeyCode));
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
