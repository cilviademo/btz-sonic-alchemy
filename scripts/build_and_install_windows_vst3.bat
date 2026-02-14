@echo off
REM Build and install BTZ VST3 on Windows in one step.
setlocal

call "%~dp0build_windows_with_vs_env.bat"
if %errorlevel% neq 0 exit /b %errorlevel%

call "%~dp0install_vst3_windows.bat"
if %errorlevel% neq 0 exit /b %errorlevel%

endlocal