@echo off
REM Copy BTZ.vst3 to Common VST3 folder. Run as Administrator to install system-wide.
setlocal

set "REPO_ROOT=%~dp0.."
set "VST3_DEST=C:\Program Files\Common Files\VST3"
set "VST3_SRC="

REM Nested path: ...\btz-sonic-alchemy-main\BTZ\build\...
if exist "%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\BTZ.vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3" set "VST3_SRC=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\VST3\Release\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\VST3\Release\BTZ.vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\VST3\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\btz-sonic-alchemy-main\BTZ\build\VST3\BTZ.vst3"

REM Flat path: ...\BTZ\build\...
if not defined VST3_SRC if exist "%REPO_ROOT%\BTZ\build\BTZ_artefacts\Release\VST3\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\BTZ\build\BTZ_artefacts\Release\VST3\BTZ.vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3" set "VST3_SRC=%REPO_ROOT%\BTZ\build\BTZ_artefacts\Release\VST3\Box Tone Zone (BTZ).vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\BTZ\build\VST3\Release\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\BTZ\build\VST3\Release\BTZ.vst3"
if not defined VST3_SRC if exist "%REPO_ROOT%\BTZ\build\VST3\BTZ.vst3" set "VST3_SRC=%REPO_ROOT%\BTZ\build\VST3\BTZ.vst3"

if not defined VST3_SRC (
    echo Error: BTZ.vst3 not found under BTZ\build. Build first.
    exit /b 1
)

echo Source:      %VST3_SRC%
echo Destination: %VST3_DEST%\Box Tone Zone (BTZ).vst3

mkdir "%VST3_DEST%" 2>nul
if not exist "%VST3_DEST%" (
    echo Cannot create destination. Run as administrator.
    exit /b 1
)

REM VST3 on Windows is usually a directory bundle; copy it recursively.
if not exist "%VST3_SRC%\." (
    echo Error: Source path is not a directory bundle: %VST3_SRC%
    exit /b 1
)

robocopy "%VST3_SRC%" "%VST3_DEST%\Box Tone Zone (BTZ).vst3" /E /IS /IT
if errorlevel 8 (
    echo Copy failed. Robocopy error %errorlevel%. Run as administrator.
    exit /b 1
)

echo Installed Box Tone Zone (BTZ).vst3. Rescan in FL Studio: Options -^> Manage Plugins -^> Find installed plugins
endlocal