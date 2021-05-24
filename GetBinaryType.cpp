#include <Windows.h>
#include <cstdio>

// TODO For dlls:
/*
// map the file to our address space
// first, create a file mapping object
hMap = CreateFileMapping( 
  hFile, 
  NULL,           // security attrs
  PAGE_READONLY,  // protection flags
  0,              // max size - high DWORD
  0,              // max size - low DWORD      
  NULL );         // mapping name - not used

// next, map the file to our address space
void* mapAddr = MapViewOfFileEx( 
  hMap,             // mapping object
  FILE_MAP_READ,  // desired access
  0,              // loc to map - hi DWORD
  0,              // loc to map - lo DWORD
  0,              // #bytes to map - 0=all
  NULL );         // suggested map addr

peHdr = ImageNtHeader( mapAddr );
peHdr->FileHeader.Machine; // See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header
*/

int main(int /*argc*/, const char* argv[])
{
    DWORD Type;
    if (GetBinaryType(argv[1], &Type) == 0)
    {
        fprintf(stderr, "GetBinaryType Error: %d\n", GetLastError());
        return EXIT_FAILURE;
    }
    
    // TODO Also see SHGetFileInfoA for whether it is a windows or console application
    
    switch (Type)
    {
    case SCS_32BIT_BINARY:
        printf("A 32-bit Windows-based application\n");
        break;
    case SCS_64BIT_BINARY:
        printf("A 64-bit Windows-based application\n");
        break;
    case SCS_DOS_BINARY:
        printf("An MS-DOS - based application\n");
        break;
    case SCS_OS216_BINARY:
        printf("A 16-bit OS/2-based application\n");
        break;
    case SCS_PIF_BINARY:
        printf("A PIF file that executes an MS-DOS - based application\n");
        break;
    case SCS_POSIX_BINARY:
        printf("A POSIX - based application\n");
        break;
    case SCS_WOW_BINARY: 
        printf("A 16-bit Windows-based application\n");
        break;
    default:
        printf("Unknown type\n");
        break;
    }
    return EXIT_SUCCESS;
}
