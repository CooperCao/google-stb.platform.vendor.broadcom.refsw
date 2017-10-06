cd c:\tools\build
PsExec.exe @dfsutil_config.txt dfsutil /spcflush
PsExec.exe @dfsutil_config.txt dfsutil /pktflush
