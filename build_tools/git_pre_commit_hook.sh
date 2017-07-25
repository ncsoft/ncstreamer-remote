#!/bin/sh
ProjectName=ncstreamer_remote_dll

exec build_tools/cpplint/cpplint_directory.bat "$GOOGLE_STYLEGUIDE_PATH/cpplint" $ProjectName/src
