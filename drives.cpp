#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

LPCTSTR GetDriveTypeStr(UINT dt)
{
    switch (dt)
    {
    case DRIVE_UNKNOWN:     return _T("Unknown");
    case DRIVE_NO_ROOT_DIR: return _T("No Root Dir");
    case DRIVE_REMOVABLE:   return _T("Removable");
    case DRIVE_FIXED:       return _T("Fixed");
    case DRIVE_REMOTE:      return _T("Remote");
    case DRIVE_CDROM:       return _T("CdRom");
    case DRIVE_RAMDISK:     return _T("RamDisk");
    default:                return _T("Unknown");
    }
}

static const TCHAR *HumanSize(ULONGLONG bytes, TCHAR* output, size_t sizeOfOutput)
{
	TCHAR *suffix[] = { _T("B "), _T("KB"), _T("MB"), _T("GB"), _T("TB") };

	int i = 0;
	double dblBytes = double(bytes);

	if (bytes > 1024)
	{
		for (i = 0; (bytes / 1024) > 0 && i < ARRAYSIZE(suffix)-1; ++i, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}

	sprintf_s(output, sizeOfOutput, _T("%.02lf %s"), dblBytes, suffix[i]);
	return output;
}

//GetLogicalDriveString
int _tmain(/*int argc, const TCHAR* const argv[]*/)
{
    TCHAR drives[1024];
    GetLogicalDriveStrings(ARRAYSIZE(drives), drives);
    for (const TCHAR* d = drives; *d != _T('\0'); d += _tcslen(d) + 1)
    {
        ULARGE_INTEGER Quota, Total, Free;
        GetDiskFreeSpaceEx(d, &Quota, &Total, &Free);
        double usage = Free.QuadPart * 100.0 / Total.QuadPart;
        
        TCHAR VolumeLabel[100];
        GetVolumeInformation(d, VolumeLabel, ARRAYSIZE(VolumeLabel), nullptr, nullptr, nullptr, nullptr, 0);
        
        TCHAR TotalStr[15];
        TCHAR FreeStr[15];
        _tprintf(_T("%3s\t%10s\t%10s\t%10s\t%10s\t%4.2lf%% Free\n"),
            d,
            VolumeLabel,
            GetDriveTypeStr(GetDriveType(d)),
            HumanSize(Total.QuadPart, TotalStr, ARRAYSIZE(TotalStr)),
            HumanSize(Free.QuadPart, FreeStr, ARRAYSIZE(FreeStr)),
            usage);
    }
    
    return EXIT_SUCCESS;
}
