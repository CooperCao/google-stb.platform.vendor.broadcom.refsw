@echo off

@REM This is a utility script that is placed in the task scheduler
@REM on the build machine. First use loop /f to see if build queue
@REM folder has contents or if it empty.

for /F %%f in ('dir /b "z:\home\hwnbuild\polling\*.bat"') do (
     goto RUNBUILD
)

@REM if upper loop didn't run then we have an empty queue
goto :EOF


:RUNBUILD

for /F %%f in ('dir /b "z:\home\hwnbuild\polling\*.bat"') do (
     copy z:\home\hwnbuild\polling\%%f  c:\temp\
     del /q z:\home\hwnbuild\polling\%%f
     call c:\temp\%%f
     del /q c:\temp\%%f
)


