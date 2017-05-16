#!/bin/sh
ProjectName=ncstreamer_remote_reference

exec vs12.0/$ProjectName/build_tools/cpplint/cpplint_directory.bat "$GOOGLE_STYLEGUIDE_PATH/cpplint" $ProjectName/src
