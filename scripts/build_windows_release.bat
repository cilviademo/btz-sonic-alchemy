@echo off
REM Build BTZ VST3 (Release) from repo root. Uses Ninja if available.
setlocal
set "REPO_ROOT=%~dp0.."
set "BTZ=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ"
if not exist "%BTZ%\CMakeLists.txt" set "BTZ=%REPO_ROOT%\BTZ"
set "BUILD_DIR=%BTZ%\build"

if not exist "%BTZ%\CMakeLists.txt" (
    echo Error: BTZ folder not found at %BTZ%
    exit /b 1
)

echo Building BTZ plugin in %BUILD_DIR%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

where ninja >nul 2>nul
if %errorlevel% equ 0 (
    cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Release
)
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build . --config Release -j 8
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo Build done. VST3: %BUILD_DIR%\VST3\BTZ.vst3
endlocal
