@echo off
cd /d %~dp0
if not defined VCINSTALLDIR (echo Setup & call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %COMPILE_TARGET%)
forfiles /M *.cpp /C "cmd /C compile @file"
