#!/bin/bash

# Exports the same variables as set_buildenv.bat by running it and
# converting the settings it generates into shell exports.

declare cygdir=C:/${CYGWIN_DIRECTORY:-tools/win32}
declare cygbin=$cygdir/bin
eval $($cygbin/env -i -- C:\\Windows\\system32\\cmd /c "C:\\tools\\build\\set_buildenv.bat $cygdir & set" |\
    $cygbin/egrep -iv '^[0-9]|^BASHOPTS=|^COMSPEC=|PATH=|^PATHEXT=|^PROMPT=|^SHELLOPTS=|^SYSTEMROOT=' |\
    $cygbin/sed "s%\\\\%\\\\%g" |\
    $cygbin/sed "s%\([^=]*\)=\(.*\)%export \1='\2'%")

# The BASHOPTS and SHELLOPTS variables cannot be set directly in bash.
set -o igncr
shopt -s nocaseglob nocasematch
