@echo off
setlocal EnableDelayedExpansion

echo Rebuild-All begins.

for %%t in (12.0 14.0) do (
  set MsBuild="C:\Program Files (x86)\MSBuild\%%t\Bin\msbuild.exe"
  for %%p in (x86 x64) do (
    for %%c in (MT MTd) do (
      !MsBuild! /m ..\vs%%t\ncstreamer_remote.sln /t:Rebuild /p:Platform=%%p;Configuration=%%c
      if !ERRORLEVEL! NEQ 0 (
	    goto quit
      )
    )
  )
)

:quit
echo Rebuild-All ends with: %ERRORLEVEL%
exit /b %ERRORLEVEL%
