The file epi_ttcp.c is Epigram's version of ttcp. An executable
(epi_ttcp.exe) compiled with Visual C++6 is included, as well
as the one-line batch file used to compile it (compile.bat).

It is based on the included sources (ttcp.c).

In addition to windows changes it adds:
	- print hash marks to indicate prograss on transmitting side
	- sleep between bursts of buffers
	- set number of buffers/burst
	- send random size buffers
	- added start and end patterns to UDP start and end packets
	- added handling of missed start, end, or data packets for UDP tests
