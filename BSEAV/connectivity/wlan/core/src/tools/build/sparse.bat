@echo off
REM
REM Sparse wrapper for WLAN S/W dev and build environment
REM On Windows platforms locate sparse.bat and run it.
REM - Sparse Source - Z:\projects\hnd_software\gallery-svn\release
REM

setlocal

set TOOLSROOT_C=C:\tools
set TOOLSDIR_C=C:\tools\build
set TOOLSDIR_H=Z:\home\hwnbuild\src\tools\build
set TOOLSDIR_G=Z:\projects\hnd_software\gallery\src\tools\build
set BASH_C=C:\tools\win32\bin
set SPARSE_G=Z:\projects\hnd_software\gallery-svn\release

REM Bash Stuff
if exist %TOOLSDIR_C%\sparse set SPARSE_WRAPPER=%TOOLSDIR_C%\sparse
if exist %TOOLSDIR_H%\sparse set SPARSE_WRAPPER=%TOOLSDIR_H%\sparse
if exist %TOOLSDIR_Z%\sparse set SPARSE_WRAPPER=%TOOLSDIR_Z%\sparse

if exist  %BASH_C%\bash.exe (
       echo Calling Bash Sparse wrapper v1.2
       echo bash %SPARSE_WRAPPER% %*
       bash %SPARSE_WRAPPER% %*
) else if exist %~dp0\sparse.py (
       echo Calling windows local copy of sparse.py
       set PATH=%TOOLSROOT_C%\Python;%TOOLSROOT_C%\Subversion;%~dp0;
       echo PATH is %TOOLSROOT_C%\Python;%TOOLSROOT_C%\Subversion;%~dp0;
       echo Python -E %~dp0\sparse.py %*
       Python.exe -E %~dp0\sparse.py %*
) else if exist %SPARSE_G%\sparse.py (
       echo Calling sparse.py over the network - %SPARSE_G%
       set PATH=%TOOLSROOT_C%\Python;%TOOLSROOT_C%\Subversion;%SPARSE_G%;
       echo PATH is %TOOLSROOT_C%\Python;%TOOLSROOT_C%\Subversion;%SPARSE_G%;
       echo Python -E %SPARSE_G%\sparse.py %*
       Python.exe -E %SPARSE_G%\sparse.py %*
) else (
	REM IF SPARSE DIDN'T execute and exit, then flag as as failure.
	echo FAILURE: Couldn't Execute Cygwin or Windows Sparse
)

endlocal


