@echo off
echo Building HKCW Engine2 Example...
cd example
flutter build windows --debug
if %errorlevel% == 0 (
    echo Build successful! Starting application...
    start build\windows\x64\runner\Debug\hkcw_engine2_example.exe
) else (
    echo Build failed!
    pause
)

