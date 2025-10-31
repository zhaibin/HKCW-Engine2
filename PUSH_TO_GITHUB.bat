@echo off
echo ================================
echo  Push to GitHub
echo ================================
echo.

set /p GITHUB_URL="Enter GitHub repository URL: "

if "%GITHUB_URL%"=="" (
    echo Error: No URL provided
    pause
    exit /b 1
)

echo.
echo Adding remote repository...
git remote add origin %GITHUB_URL% 2>nul
if errorlevel 1 (
    echo Remote already exists, updating URL...
    git remote set-url origin %GITHUB_URL%
)

echo.
echo Current commits:
git log --oneline -5

echo.
echo Pushing to GitHub...
git push -u origin master

if errorlevel 1 (
    echo.
    echo Push failed. If remote has different history, use:
    echo   git push -u origin master --force
    pause
) else (
    echo.
    echo ================================
    echo  Successfully pushed to GitHub!
    echo ================================
    pause
)

