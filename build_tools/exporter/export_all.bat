@echo off
setlocal EnableDelayedExpansion

echo Export-All begins.

set TargetRoot=ncstreamer_remote

if exist "%TargetRoot%" (
  rmdir /s /q "%TargetRoot%"
)
mkdir "%TargetRoot%"

robocopy ^
 ..\..\ncstreamer_remote_dll\include ^
 "%TargetRoot%" ^
 /E
echo ROBOCOPY RESULT: %ERRORLEVEL%
if %ERRORLEVEL% GEQ 8 (
goto quit
)
set ERRORLEVEL=0
 
mkdir "%TargetRoot%\lib"
for %%v in (12 14) do (
  set FromDir=..\..\vs%%v.0\ncstreamer_remote_dll\build\vc_v%%v0
  set ToDir=!TargetRoot!\lib\vc_v%%v0
  
  robocopy /mir "!FromDir!" "!ToDir!" *.lib *.dll *.pdb
  echo ROBOCOPY RESULT: !ERRORLEVEL!
  if !ERRORLEVEL! GEQ 8 (
    goto quit
  )
  set ERRORLEVEL=0
)

:quit
echo Export-All ends: %ERRORLEVEL%
exit /b %ERRORLEVEL%
