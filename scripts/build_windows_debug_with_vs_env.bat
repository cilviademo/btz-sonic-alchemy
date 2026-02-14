@echo off
REM Build BTZ VST3/Standalone Debug with x64 MSVC environment.
setlocal

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Error: VS 2022 vcvars64.bat not found.
    exit /b 1
)

if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo Error: expected x64 toolchain, got "%VSCMD_ARG_TGT_ARCH%".
    exit /b 1
)

set "REPO_ROOT=%~dp0.."
set "BTZ_DIR=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ"
if not exist "%BTZ_DIR%\CMakeLists.txt" set "BTZ_DIR=%REPO_ROOT%\BTZ"
if not exist "%BTZ_DIR%\CMakeLists.txt" (
    echo Error: BTZ folder not found under %REPO_ROOT%
    exit /b 1
)

set "BUILD_DIR=%BTZ_DIR%\build-debug"
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"

cmake -S "%BTZ_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Debug
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build "%BUILD_DIR%" -j 8
if %errorlevel% neq 0 exit /b %errorlevel%

echo Debug build complete.
echo Output under: %BUILD_DIR%\BTZ_artefacts\
endlocal
