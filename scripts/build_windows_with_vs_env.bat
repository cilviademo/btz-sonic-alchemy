@echo off
REM Build BTZ VST3 with a known-good x64 MSVC environment.
setlocal

REM 1) Load VS 2022 x64 toolchain (avoids VS 2026 x86/x64 library mismatch)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Error: VS 2022 vcvars64.bat not found. Install VS 2022/Build Tools with Desktop C++ workload.
    exit /b 1
)

if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo Error: expected x64 toolchain, got "%VSCMD_ARG_TGT_ARCH%".
    exit /b 1
)

REM 2) Find BTZ folder (supports nested repo layout)
set "REPO_ROOT=%~dp0.."
set "BTZ_DIR=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ"
if not exist "%BTZ_DIR%\CMakeLists.txt" set "BTZ_DIR=%REPO_ROOT%\BTZ"
if not exist "%BTZ_DIR%\CMakeLists.txt" (
    echo Error: BTZ folder not found under %REPO_ROOT%
    exit /b 1
)

echo Using x64 MSVC %VCToolsVersion%
echo BTZ directory: %BTZ_DIR%

REM 3) Clean build dir to avoid stale x86/x64 CMake cache/toolchain mix
if exist "%BTZ_DIR%\build" rd /s /q "%BTZ_DIR%\build"

REM 4) Configure and build
cmake -S "%BTZ_DIR%" -B "%BTZ_DIR%\build" -G Ninja -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build "%BTZ_DIR%\build" -j 8
if %errorlevel% neq 0 exit /b %errorlevel%

echo/
echo Build complete.
echo Look for Box Tone Zone (BTZ).vst3 under:
echo   %BTZ_DIR%\build\BTZ_artefacts\Release\VST3\
echo   %BTZ_DIR%\build\VST3\
endlocal