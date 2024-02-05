# MiniUtils
![Console](https://img.shields.io/badge/platform-Console-blue.svg)
[![Releases](https://img.shields.io/github/release/RadAd/MiniUtils.svg)](https://github.com/RadAd/MiniUtils/releases/latest)
[![commits-since](https://img.shields.io/github/commits-since/RadAd/MiniUtils/latest.svg)](commits/master)
[![Build](https://img.shields.io/appveyor/ci/RadAd/MiniUtils.svg)](https://ci.appveyor.com/project/RadAd/MiniUtils)

One file utilties

## [bgstart](bgstart.c) - Background start for windows
This starts a process in the background with no
window. Its useful for console programs that you
don't wish to see. One difference with this version
is that it passes on the standard input and output
handles and waits for the process to end so that
it can return the exit code. 

## [cattee](cattee.c)
Combination of cat and tee. Read from a file or stdin and output to a file or stdout.

## [catwin](catwin.c)
Read and output a file to stdout using windows file functions. Works with named pipes also.

## [ClipWatch](ClipWatch.cpp)
Monitors the clipboard and outputs the text it contains

## [ConFlash](ConFlash.cpp)
Flashes the console window. Useful for when you have a
long process that you want to be alerted when it finishes

## [ConIcon](ConIcon.cpp)
Set the icon of the console window

## [CreateGuid](CreateGuid.cpp)
Create a guid.

## [DragDrop](DragDrop.cpp)
Simulates drag and dropping a file onto a window

## [Drives](Drives.cpp)
List local drives

## [FindWindow](FindWindow.cpp)
Search for a window by title or class.

## [FormatMessage](FormatMessage.cpp)
Lookup error message.

## [GetBinaryType](GetBinaryType.cpp)
Show the binary type as retrieved from [GetBinaryType](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getbinarytypea)

## [GetCh](GetCh.cpp)
Get the next keyboard character

## [GetCmdLine](GetCmdLine.cpp)
Get the command line of a process.

## [GetExeType](GetExeType.cpp)
Show the exe type as retrieved from [SHGetFileInfo](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shgetfileinfoa) using the SHGFI_EXETYPE flag.

## [HexDump](HexDump.cpp)
Dump a hex format of a binary file

## [IsElevated](IsElevated.cpp)
Determine is the process is elevated or not. It inherits this status from its parent.

## [IsInteractive](IsInteractive.cpp)
Determine is the process is in an interactive mode. ie Are the stdin and stdout of type char.

## [KeyHook](KeyHook.cpp)
Output log of keys pressed using the [low-level keyboard hook](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644985(v=vs.85)).

## [KeyScanLog](KeyScanLog.cpp)
Output log of keys pressed using [ReadConsoleInput](https://learn.microsoft.com/en-us/windows/console/readconsoleinput).

## [Piper](Piper.cpp)
Create a named pipe and wait for a connection.

## [ptime](ptime.cpp)
Display process times as retrieved from [GetProcessTimes](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes)

## [PsList](PsList.cpp)
List processes using [EnumProcesses](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses)

## [RadClip](RadClip.cpp)
Output the clipboard contents.

## [RadRunAs](RadRunAs.cpp)
Like RunAs except it takes the password from the commandline. Also supports elevation. And Errors are reported in a GUI MessageBox.

## [RadSpeak](RadSpeak.cpp)
Uses [ISpVoice](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee125164(v=vs.85)) to
convert text to speech.

## [RadToast](RadToast.cpp)
Create a windows notification.

## [RealPath](RealPath.cpp)
Output the real path of a file. Useful to resolve
hard and soft links.

## [RegExport](RegExport.cpp)
Output a tree view of the registry.

## [SendMessage](SendMessage.cpp)
Use the [SendMessage](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage) function.

## [ShellExecute](ShellExecute.cpp)
Launch using the [ShellExecuteEx](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecuteexa) function.

## [Shortcut](Shortcut.cpp)
Edit shortcut properties from the command line.

## [Sleep](Sleep.cpp)
Sleep in seconds.

## [TimeStamp](TimeStamp.cpp)
Copies stdin to stdout prepending each line with a timestamp.

## [UPnP](UPnP.cpp)
Output UPnP devices and static port mapping.

## [whoami](whoami.c)
Outputs the name of the current user

## [WndList](WndList.c)
Outputs a list of windows


# Build Instructions

## Build All
`msbuild /nologo Compile.proj /t:BuildAll`

## Build Single
`msbuild /nologo Compile.proj /t:Build /p:TargetName=[Project]`

where `[Project]` is one from the list given by:
`msbuild /nologo Compile.proj /t:ListAll`
