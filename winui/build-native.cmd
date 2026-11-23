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

if "%~1"=="" exit /b 1
set "nativeOut=%~f1"
set "boostRoot=%LOCALAPPDATA%\Fibonacci\boost_1_92_0"
if not exist "%boostRoot%\boost\multiprecision\cpp_int.hpp" (
    echo Verified Boost 1.92.0 headers are required. See the root README.md. 1>&2
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

set "NoDefaultCurrentDirectoryInExePath="
call "%vsInstall%\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.28000.0 -vcvars_ver=14.52
if errorlevel 1 goto :failed
set "NoDefaultCurrentDirectoryInExePath=1"
if not defined VCToolsInstallDir goto :failed
set "msvcBin=%VCToolsInstallDir%bin\Hostx64\x64"

if not exist "%nativeOut%\" mkdir "%nativeOut%"
if not exist "%nativeOut%\" goto :failed

"%msvcBin%\cl.exe" /nologo /c /std:c++23 /utf-8 /EHsc /O2 /MT /W4 /WX /guard:cf /I"%boostRoot%" /Fo"%nativeOut%\fibonacci_native.obj" "%~dp0fibonacci_native.cpp"
if errorlevel 1 goto :failed
"%msvcBin%\link.exe" /NOLOGO /DLL /GUARD:CF /MANIFEST:EMBED /OUT:"%nativeOut%\FibonacciNative.dll" "%nativeOut%\fibonacci_native.obj" ole32.lib
if errorlevel 1 goto :failed
popd
exit /b 0

:failed
popd
exit /b 1
