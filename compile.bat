@echo off
setlocal
rem https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx

prompt $G

:loop
set CMD=%1
if not defined CMD goto :eof
if /I [%CMD%]==[/Config] (
	set COMPILE_CONFIG=%2
	shift
) else if /I [%CMD%]==[/CharSet] (
	set COMPILE_CHARSET=%2
	shift
) else (
	call :compile %CMD%
)
shift
goto :loop

:compile
setlocal

set COMPILE_CONF=%~n1.conf
rem echo COMPILE_CONF %COMPILE_CONF%
if exist %COMPILE_CONF% for /f "delims== tokens=1,*" %%i in (%COMPILE_CONF%) do @if not defined %%i set %%i=%%j

if not defined COMPILE_TARGET set COMPILE_TARGET=%PROCESSOR_ARCHITECTURE%
rem set COMPILE_TARGET=amd64
rem set COMPILE_TARGET=x86
if not defined COMPILE_CONFIG set COMPILE_CONFIG=Release
rem set COMPILE_CHARSET=Release
rem set COMPILE_CHARSET=Debug
if not defined COMPILE_CHARSET set COMPILE_CHARSET=MBCS
rem set COMPILE_CHARSET=Unicode
rem set COMPILE_CHARSET=MBCS
if not defined COMPILE_SUBSYSTEM set COMPILE_SUBSYSTEM=Console
rem set COMPILE_SUBSYSTEM=Console
rem set COMPILE_SUBSYSTEM=Windows

rem if not defined VCINSTALLDIR (echo Setup & call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" %COMPILE_TARGET%)
if not defined VCINSTALLDIR (echo Setup & call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %COMPILE_TARGET%)

set OPTIONS=/nologo
set LINK_OPTIONS=/OPT:REF

set OPTIONS=%OPTIONS% /W4 /EHa /GS /Zc:forScope /Zc:wchar_t /Zc:inline /Gd

if %COMPILE_CONFIG%==Release (
	set OPTIONS=%OPTIONS% /MD -D NDEBUG /O2 /Ob2 /Oi /GL
)
if %COMPILE_CONFIG%==Debug (
	set OPTIONS=%OPTIONS% /MDd -D _DEBUG /Od /Ob0 /Zi
)

call :upper_case COMPILE_CHARSET COMPILE_CHARSET
set OPTIONS=%OPTIONS% -D _%COMPILE_CHARSET% -D %COMPILE_CHARSET%

call :upper_case COMPILE_SUBSYSTEM COMPILE_SUBSYSTEM
set OPTIONS=%OPTIONS% -D _%COMPILE_SUBSYSTEM%
set LINK_OPTIONS=%LINK_OPTIONS% /SUBSYSTEM:%COMPILE_SUBSYSTEM%

@echo on
cl %OPTIONS% %1 /link %LINK_OPTIONS%
@echo off
del "%~dpn1.obj"
goto :eof

:upper_case input_var output_var
setlocal ENABLEDELAYEDEXPANSION
set _=!%1!
for %%Z in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do set _=!_:%%Z=%%Z!
endlocal & set %2=%_%
goto :eof
