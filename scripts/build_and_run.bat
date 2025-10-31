@echo off
echo ================================
echo  HKCW Engine2 - Build and Run
echo ================================
echo.

cd /d "%~dp0..\example"

echo Building Debug version...
flutter build windows --debug

if %errorlevel% == 0 (
    echo.
    echo Build successful! Starting application...
    start build\windows\x64\runner\Debug\hkcw_engine2_example.exe
    echo.
    echo Application started!
) else (
    echo.
    echo Build failed! Check the error messages above.
    pause
)

