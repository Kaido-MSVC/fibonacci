@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "ComSpec=%SystemRoot%\System32\cmd.exe"
set "PATH=%SystemRoot%\System32;%SystemRoot%"
set "CL="
set "_CL_="
set "LINK="
set "_LINK_="
set "INCLUDE="
set "LIB="
set "LIBPATH="
set "boostRoot=%LOCALAPPDATA%\Fibonacci\boost_1_92_0"
if not exist "%boostRoot%\boost\multiprecision\cpp_int.hpp" (
    echo Verified Boost 1.92.0 headers are required. See README.md. 1>&2
    exit /b 1
)
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" (
    echo vswhere.exe was not found. 1>&2
    exit /b 1
)
set "vsInstall="
pushd "%SystemRoot%\System32" || exit /b 1
for /f "usebackq delims=" %%I in (`"%vswhere%" -latest -version 18.0 -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "vsInstall=%%I"
if not defined vsInstall (
    echo Visual Studio 2026 with MSVC x64 build tools was not found. 1>&2
    goto :failed
)

rem Visual Studio's setup resolves some helpers from its own trusted directories.
set "NoDefaultCurrentDirectoryInExePath="
call "%vsInstall%\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.28000.0 -vcvars_ver=14.52
if errorlevel 1 goto :failed
set "NoDefaultCurrentDirectoryInExePath=1"
if not defined VCToolsInstallDir goto :failed
set "msvcBin=%VCToolsInstallDir%bin\Hostx64\x64"
if not exist "%msvcBin%\cl.exe" goto :failed
if not exist "%msvcBin%\link.exe" goto :failed
if not exist "%msvcBin%\dumpbin.exe" goto :failed

if not exist "%~dp0build\" mkdir "%~dp0build"
if not exist "%~dp0build\" goto :failed

set "CXXFLAGS=/nologo /c /std:c++23 /utf-8 /EHsc /O2 /W4 /WX /guard:cf"

rem Console version
"%msvcBin%\cl.exe" %CXXFLAGS% /Bv /I"%boostRoot%" /Fo"%~dp0build\fibonacci.obj" "%~dp0fibonacci.cpp"
if errorlevel 1 goto :failed
"%msvcBin%\link.exe" /NOLOGO /GUARD:CF /MANIFEST:EMBED /SUBSYSTEM:CONSOLE /OUT:"%~dp0build\fibonacci.exe" "%~dp0build\fibonacci.obj"
if errorlevel 1 goto :failed
call :verify_cfg "%~dp0build\fibonacci.exe"
if errorlevel 1 goto :failed

rem Win32 GUI version
"%msvcBin%\cl.exe" %CXXFLAGS% /I"%boostRoot%" /Fo"%~dp0build\fibonacci_gui.obj" "%~dp0fibonacci_gui.cpp"
if errorlevel 1 goto :failed
"%msvcBin%\link.exe" /NOLOGO /GUARD:CF /MANIFEST:EMBED /SUBSYSTEM:WINDOWS /OUT:"%~dp0build\fibonacci_gui.exe" "%~dp0build\fibonacci_gui.obj" user32.lib gdi32.lib windowsapp.lib
if errorlevel 1 goto :failed
call :verify_cfg "%~dp0build\fibonacci_gui.exe"
if errorlevel 1 goto :failed
popd
exit /b 0

:verify_cfg
setlocal
pushd "%msvcBin%" || exit /b 1
set "cfgHeader="
set "cfgInstrumented="
set "cfgTable="
for /f "usebackq tokens=*" %%L in (`.\dumpbin.exe /NOLOGO /HEADERS /LOADCONFIG "%~1"`) do (
    if /i "%%L"=="Control Flow Guard" set "cfgHeader=1"
    if /i "%%L"=="CF instrumented" set "cfgInstrumented=1"
    if /i "%%L"=="FID table present" set "cfgTable=1"
)
popd
if defined cfgHeader if defined cfgInstrumented if defined cfgTable exit /b 0
echo Control Flow Guard verification failed for "%~1". 1>&2
exit /b 1

:failed
popd
exit /b 1
