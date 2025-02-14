#include <tchar.h>
#include <Windows.h>

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "arg.inl"

void Process(HANDLE hInput, HANDLE hOutput)
{
	std::vector<BYTE> buffer(1024);

	while (true)
	{
		DWORD dwBytesRead = 0;
		BOOL fSuccess = ReadFile(hInput, buffer.data(), (DWORD) buffer.size() * sizeof(BYTE), &dwBytesRead, nullptr);

		if (!fSuccess || dwBytesRead == 0)
			break;

		DWORD dwBytesWritten = 0;
		fSuccess = WriteFile(hOutput, buffer.data(), dwBytesRead, &dwBytesWritten, nullptr);
		if (!fSuccess)
			break;
	}
}

int _tmain(int argc, const TCHAR* const argv[])
{
	arginit(argc, argv, _T("Create a named pipe and wait for a connection"));
	BOOL bInput = argswitchdesc(TEXT("/I"), _T("Input mode. Copy stdin to client. Default is Output mode. Copy client to stdout."));
	const TCHAR* pipe = argnumdesc(1, nullptr, _T("name"), _T("The name of the pipe"));
	if (!argcleanup())
        return EXIT_FAILURE;
    if (argusage(pipe == nullptr))
        return EXIT_SUCCESS;

	TCHAR name[MAX_PATH] = TEXT("\\\\.\\pipe\\");
    _tcscat_s(name, pipe);

	HANDLE hPipe = CreateNamedPipe(name, (bInput ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND) | FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_TYPE_BYTE | (bInput ? PIPE_READMODE_BYTE : PIPE_READMODE_BYTE) | PIPE_WAIT, 1, 0, 0, NMPWAIT_USE_DEFAULT_WAIT, nullptr);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, TEXT("Piper: CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
		return EXIT_FAILURE;
	}

	_ftprintf(stderr, TEXT("Piper: Listening to %s\n"), name);

	int ret = EXIT_SUCCESS;
	BOOL fConnected = ConnectNamedPipe(hPipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if (fConnected)
	{
		if (bInput)
			Process(GetStdHandle(STD_INPUT_HANDLE), hPipe);
		else
			Process(hPipe, GetStdHandle(STD_OUTPUT_HANDLE));

		if (GetLastError() == ERROR_BROKEN_PIPE)
		{
			_fputts(TEXT("Piper: client disconnected.\n"), stderr);
		}
		else
		{
			_ftprintf(stderr, TEXT("Piper: Process failed, GLE=%d.\n"), GetLastError());
			ret = EXIT_FAILURE;
		}
	}
	else
	{
		_ftprintf(stderr, TEXT("Piper: ConnectNamedPipe failed, GLE=%d.\n"), GetLastError());
		ret = EXIT_FAILURE;
	}

	CloseHandle(hPipe);
	return EXIT_SUCCESS;
}
