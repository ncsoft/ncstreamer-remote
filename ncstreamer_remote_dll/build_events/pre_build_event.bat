set RootDir=%~1

call "%RootDir%/build_tools/cpplint/cpplint_directory.bat" "%GOOGLE_STYLEGUIDE_PATH%/cpplint" "%RootDir%/ncstreamer_remote_dll/include"
call "%RootDir%/build_tools/cpplint/cpplint_directory.bat" "%GOOGLE_STYLEGUIDE_PATH%/cpplint" "%RootDir%/ncstreamer_remote_dll/src"
