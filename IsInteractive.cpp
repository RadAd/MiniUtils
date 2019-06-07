#include <Windows.h>

// NOTE Dos devices such as NUL and COM1 are also of type FILE_TYPE_CHAR

int main()
{
    return !(GetFileType(GetStdHandle(STD_INPUT_HANDLE)) == FILE_TYPE_CHAR && GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR);
}
