# MiniUtils
![Console](https://img.shields.io/badge/platform-Console-blue.svg)
[![Releases](https://img.shields.io/github/release/RadAd/MiniUtils.svg)](https://github.com/RadAd/MiniUtils/releases/latest)
[![Build](https://img.shields.io/appveyor/ci/RadAd/MiniUtils.svg)](https://ci.appveyor.com/project/RadAd/MiniUtils)

One file utilties

## [bgstart](bgstart.c) - Background start for windows
This starts a process in the background with no
window. Its useful for console programs that you
don't wish to see. One difference with this version
is that it passes on the standard input and output
handles and waits for the process to end so that
it can return the exit code. 

## [ClipWatch](ClipWatch.cpp)
Monitors the clipboard and outputs the text it contains

## [ConFlash](ConFlash.cpp)
Flashes the console window. Useful for when you have a
long process that you want to be alerted when it finishes

## [ConIcon](ConIcon.cpp)
Set the icon of the console window

## [HexDump](HexDump.cpp)
Dump a hex format of a binary file

## [RadClip](RadClip.cpp)
Output the clipboard contents

## [RadSpeak](RadSpeak.cpp)
Uses [ISpVoice](https://msdn.microsoft.com/en-us/library/ee125164(v=vs.85).aspx) to
convert text to speech.

## [RealPath](RealPath.cpp)
Output the real path of a file. Useful to resolve
hard and soft links.

## [TimeStamp](TimeStamp.cpp)
Copies stdin to stdout prepending each line with a timestamp.

## [UPnP](UPnP.cpp)
Output UPnP devices and static port mapping.

## [whoami](whoami.c)
Outputs the name of the current user

## [DragDrop](DragDrop.cpp)
Simulates drag and dropping a file onto a window