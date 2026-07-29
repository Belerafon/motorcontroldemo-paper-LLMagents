@echo off
setlocal EnableExtensions

rem Delete only files and directories ignored by this repository.
rem Tracked files and ordinary untracked source files are preserved.

for %%I in ("%~dp0.") do set "SCRIPT_ROOT=%%~fI"
pushd "%SCRIPT_ROOT%" >nul || (
    echo ERROR: Cannot enter the script directory.
    exit /b 1
)

for /f "delims=" %%I in ('git rev-parse --show-toplevel 2^>nul') do set "REPO_ROOT=%%~fI"
if not defined REPO_ROOT (
    echo ERROR: This script must be run from a Git repository.
    popd
    exit /b 1
)

if /I not "%SCRIPT_ROOT%"=="%REPO_ROOT%" (
    echo ERROR: This script must remain in the repository root.
    echo Script root: %SCRIPT_ROOT%
    echo Git root:    %REPO_ROOT%
    popd
    exit /b 1
)

if /I "%~1"=="--help" goto :usage
if /I "%~1"=="-h" goto :usage
if /I "%~1"=="/?" goto :usage

if /I "%~1"=="--dry-run" (
    if not "%~2"=="" goto :usage_error
    echo DRY RUN: the following ignored files and directories would be deleted:
    echo.
    git clean -ndX -- .
    set "RESULT=%ERRORLEVEL%"
    goto :finish
)

if not "%~1"=="" goto :usage_error

echo Removing ignored generated files from:
echo   %REPO_ROOT%
echo.
echo This includes local artifacts, Debug build output, Python/tool caches,
echo logs, temporary files, backups, and other paths covered by .gitignore.
echo Tracked files and non-ignored untracked files are preserved.
echo.

git clean -fdX -- .
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (
    echo.
    echo Cleanup completed successfully.
) else (
    echo.
    echo ERROR: Cleanup failed with exit code %RESULT%.
)
goto :finish

:usage_error
echo ERROR: Unknown arguments.
echo.

:usage
echo Usage:
echo   %~nx0             Delete ignored generated files.
echo   %~nx0 --dry-run   Show what would be deleted.
echo   %~nx0 --help      Show this help.
set "RESULT=2"

:finish
popd
exit /b %RESULT%
