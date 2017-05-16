#!/bin/sh
ProjectName=ncstreamer_remote_dll

exec vs14.0/$ProjectName/build_tools/cpplint/cpplint_directory.bat "$GOOGLE_STYLEGUIDE_PATH/cpplint" $ProjectName/src
