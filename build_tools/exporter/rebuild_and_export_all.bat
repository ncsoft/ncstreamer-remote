@echo off
@echo Rebuild-And-Export-All begins.

call rebuild_all.bat
if %ERRORLEVEL% NEQ 0 (
  goto quit
)

call export_all.bat
if %ERRORLEVEL% NEQ 0 (
  goto quit
)

:quit
@echo Rebuild-And-Export-All ends: %ERRORLEVEL%
exit /b %ERRORLEVEL%
