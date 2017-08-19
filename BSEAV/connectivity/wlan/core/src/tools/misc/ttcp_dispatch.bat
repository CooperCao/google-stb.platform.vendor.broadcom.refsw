@echo off
rem Batch file to accept batch files from another machine by
rem way of ttcp, then execute them.
rem
rem Copyright 1999, Epigram, Inc.
rem
rem $Id: ttcp_dispatch.bat,v 12.5 1999-04-13 18:29:17 stafford Exp $
rem
set COMMAND_PORT=8410
if not "%1"=="" COMMAND_PORT=%1

:exec_loop

.\epi_ttcp -r -p %COMMAND_PORT% > remote_command.bat
if errorlevel 1 goto ttcp_error
echo.
echo "Executing file:"
type remote_command.bat
echo.
echo "Executing file:" >> \tmp\ttcp_dispatch_log.txt
type remote_command.bat >> \tmp\ttcp_dispatch_log.txt
echo. >> \tmp\ttcp_dispatch_log.txt

call remote_command

goto exec_loop

:ttcp_error

echo There was an error from epi_ttcp.
