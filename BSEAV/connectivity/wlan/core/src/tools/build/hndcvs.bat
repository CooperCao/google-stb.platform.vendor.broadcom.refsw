@echo off

REM
REM On Windows platforms locate hndcvs and run it. On windows, files without
REM extensions are not called
REM
REM $Id: hndcvs.bat,v 12.1 2009-05-12 22:43:35 prakashd Exp $
REM

setlocal

set TOOLSDIR_C=C:\tools\build
set TOOLSDIR_D=D:\tools\build
set TOOLSDIR_H=Z:\home\hwnbuild\src\tools\build
set TOOLSDIR_G=Z:\projects\hnd_software\gallery\src\tools\build

if exist %TOOLSDIR_C%\hndcvs set HNDCVS=%TOOLSDIR_C%\hndcvs && goto runit
if exist %TOOLSDIR_D%\hndcvs set HNDCVS=%TOOLSDIR_D%\hndcvs && goto runit
if exist %TOOLSDIR_H%\hndcvs set HNDCVS=%TOOLSDIR_H%\hndcvs && goto runit
if exist %TOOLSDIR_Z%\hndcvs set HNDCVS=%TOOLSDIR_Z%\hndcvs && goto runit

:runit
echo bash %HNDCVS% %*
bash %HNDCVS% %*

endlocal
