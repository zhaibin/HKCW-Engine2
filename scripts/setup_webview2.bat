@echo off
echo ================================
echo  WebView2 SDK Setup
echo ================================
echo.

cd /d "%~dp0..\windows"
if not exist nuget.exe (
    echo Downloading NuGet...
    powershell -Command "Invoke-WebRequest -Uri 'https://dist.nuget.org/win-x86-commandline/latest/nuget.exe' -OutFile 'nuget.exe'"
)
echo Installing WebView2 package...
nuget.exe install Microsoft.Web.WebView2 -Version 1.0.2592.51 -OutputDirectory packages
echo WebView2 SDK installed successfully!
cd ..
pause

