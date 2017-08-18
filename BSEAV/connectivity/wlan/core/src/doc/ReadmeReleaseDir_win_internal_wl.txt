RELEASE FOLDER CONTENT DETAILS:
-------------------------------
NOTE: If any of the following files do not exist, then they may not have been
successfully built

WinXP/2K directories:
---------------------
WinXP/free: 
	XP Broadcom Internal non-debug (free) package containing Driver-Only
        and InstallShield subfolders
WinXP/checked:
	XP Broadcom Internal debug (checked) package containing Driver-Only
        and InstallShield subfolders

WinXP/2K files:
---------------
WinXP/free/wl.exe: 
	XP Broadcom Internal free/non-debug version of wl.exe

WinXP/checked/wl.exe: 
	XP Broadcom Internal checked/debug version of wl.exe

WinXP Driver Debug Files are not copied over to release-folder.
However they can located at following src paths

src/wl/sys/wdm/buildxp/objfre_wxp_x86/i386/bcmwl5.pdb:
src/wl/sys/wdm/buildxp/objfre_wnet_amd64/amd64/bcmwl564.pdb:
        XP Broadcom Internal 32bit/64bit free driver debug files

src/wl/sys/wdm/buildxp/objchk_wxp_x86/i386/bcmwl5.pdb:
src/wl/sys/wdm/buildxp/objchk_wnet_amd64/amd64/bcmwl564.pdb:
        XP Broadcom Internal 32bit/64bit checked driver debug files

================================================================

Win7/Vista directories:
-----------------------
Win7/free: 
	Win7/Vista Broadcom Internal non-debug (free) package containing 
	Driver-Only and InstallShield subfolders
Win7/checked:
	Win7/Vista Broadcom Internal debug (checked) package containing 
	Driver-Only and InstallShield subfolders

================================================================
All Platform Consolidated directories:
--------------------------------------
If WinALL/<oem> exists, then it contains consolidated package for a
given OEM that supports WinXP/WinVista/Win7 deliverables

WinALL/free:
        WinXP, Win7 and Vista free/non-debug bits

WinALL/checked:
        WinXP, Win7 and Vista checked/debig bits

================================================================
Win7/WinVista files:
--------------------
Win7/free/wl.exe: 
	Win7/Vista free/non-debug version of wl.exe

Win7/checked/wl.exe: 
	Win7/Vista checked/debug version of wl.exe

Win7 Driver Debug Files are not copied over to release-folder.
However they can located at following src paths

src/wl/sys/wdm/buildvista/objfre_wlh_x86/i386/bcmwl6.pdb:
src/wl/sys/wdm/buildvista/objfre_wlh_amd64/amd64/bcmwl664.pdb:
        Win7/Vista Broadcom Internal 32bit/64bit free driver debug files

src/wl/sys/wdm/buildvista/objchk_wlh_x86/i386/bcmwl6.pdb:
src/wl/sys/wdm/buildvista/objchk_wlh_amd64/amd64/bcmwl664.pdb:
        Win7/Vista Broadcom Internal 32bit/64bit checked driver debug files
