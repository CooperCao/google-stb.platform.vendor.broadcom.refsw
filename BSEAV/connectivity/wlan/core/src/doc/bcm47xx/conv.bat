@echo off
set InstallPath=C:\estii
if NOT EXIST %InstallPath% goto EST_ERROR
:DIREXISTS
copy %InstallPath%\objcvt.exe  1>nul
echo Deleting existing debug files......

del *.bdx
del *.ab
del *.abg
del *.abx

echo converting to VisionICE symbol files for debug ........
objcvt -stabs_absolute_line_numbers %1

echo conversion completed sucessfully.
echo goodbye!!!
%SystemDrive%
goto TERMINATE

:EST_ERROR
echo The directory %InstallPath% does not exist!!!
echo If you do not have the visionICE software installed
echo please do so now and then run this batch file.  
echo If the software is installed in another location, please
echo modify the line that begins with "set InstallPath=" in
echo the UPDATE.BAT file
goto TERMINATE

:TERMINATE
