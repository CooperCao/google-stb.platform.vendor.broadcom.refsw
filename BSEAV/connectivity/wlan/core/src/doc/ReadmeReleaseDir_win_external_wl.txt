RELEASE FOLDER CONTENT DETAILS:
-------------------------------
NOTE1: If any of the following files do not exist, then they may not have
       been built.
NOTE2: Installshield package for different OEMs is packaged in self-extractable
       setup.exe. If you want to just extract the setup.exe contents into a 
       private <path> without actually installing, you can create <path> 
       directory and run "setup.exe -extract_all:<path>" 
       (Example: setup.exe -extract_all:c:\temp\bcm)

WinXP/2K directories:
---------------------
WinXP/Apple: 
	XP Installer for Apple pc-oem brand
WinXP/Bcm: 
	XP Installer for Bcm generic brand
WinXP/Dell:
	XP Installer for Dell pc-oem brand
WinXP/HP:
	XP Installer for Hp pc-oem brand
WinXP/DebugSymbols:
	XP Application (tray) symbol files different pc-oem branded apps
	Driver symbol files are located under Internal folder
WinXP/Internal:
	XP Tools and utils for internal usage. 
Win7/SES:
	Common win xp/2k 32bit and 64bit SES installer

WinXP/2K files:
---------------
WinXP/Internal/wl.exe: 
	XP checked/debug version without BCMINTERNAL flag
WinXP/Internal/bcmwl5.sys,bcmwl5.pdb,bcmwl564.sys,bcmwl564.pdb:
	XP 32bit and 64bit checked driver and debug files without BCMINTERNAL flag
WinXP/Internal/free_bcmwl5.pdb,free_bcmwl564.pdb
	XP 32bit and 64bit free driver and debug files without BCMINTERNAL flag

================================================================

Win7 and WinVISTA directories:
------------------------------
Win7/Bcm:
	Win7 and Vista Installer for Bcm pc-oem brand
Win7/Dell:
	Win7 and Vista Installer for Dell pc-oem brand
Win7/HP:
	Win7 and Vista Installer for HP Driveronly pc-oem brand
Win7/DebugSymbols:
	Win7 and Vista Application (tray) symbol files different 
	pc-oem branded apps.
	Driver symbol files are located under Internal folder
Win7/Internal:
	Win7 and Vista Tools and utils for internal usage. 
Win7/SES:
	Common win xp/2k 32bit and 64bit SES installer

================================================================

All Platform Consolidated directories:
--------------------------------------
If WinALL/<oem> exists, then it contains consolidated package for a
given OEM that supports WinXP/WinVista/Win7 deliverables

WinALL/Bcm:
	WinXP, Win7 and Vista Installer for Bcm pc-oem brand

WinALL/HP:
	WinXP, Win7 and Vista Installer for HP pc-oem brand

================================================================

Win7 and WinVISTA files:
------------------------
Win7/Internal/wl.exe: 
	Win7/Vista checked/debug version without BCMINTERNAL flag
Win7/Internal/checked_bcmwl6.sys,checked_bcmwl6.pdb,checked_bcmwl664.sys,checked_bcmwl664.pdb:
	Win7/Vista 32bit and 64bit checked driver and debug files without BCMINTERNAL flag
Win7/Internal/free_bcmwl6.sys,free_bcmwl6.pdb,free_bcmwl664.sys,free_bcmwl664.pdb:
	Win7/Vista 32bit and 64bit free driver and debug files without BCMINTERNAL flag
