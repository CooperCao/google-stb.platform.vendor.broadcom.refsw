# Microsoft Developer Studio Project File - Name="diags" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=diags - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "diags.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "diags.mak" CFG="diags - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "diags - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "diags - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "diags"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "diags - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Ox /Ot /Oa /Ow /Og /Oi /Op /I "..\..\..\..\.." /I "..\..\..\..\..\magnum\commonutils\fec\include" /I "..\..\..\..\..\magnum\basemodules\std" /I "..\..\..\..\..\magnum\basemodules\std\types\generic" /I "..\..\..\..\..\magnum\basemodules\std\config" /I "..\..\..\..\..\magnum\basemodules\err" /I "..\..\..\..\..\magnum\basemodules\dbg" /I "..\..\..\..\..\magnum\basemodules\kni\generic" /I "..\..\..\..\..\magnum\commonutils\lst" /I "..\..\..\..\..\magnum\basemodules\chp\include\common" /I "..\..\..\..\..\magnum\basemodules\chp\include\4538\rdb\a0" /I "..\..\..\..\..\magnum\basemodules\reg" /I "..\..\..\..\..\magnum\basemodules\int" /I "..\..\..\..\..\magnum\portinginterface\ast\include\common" /I "..\..\..\..\..\magnum\portinginterface\ast\include\4538" /I "..\..\..\..\..\magnum\portinginterface\ast\src\common" /I "..\..\..\..\..\magnum\portinginterface\ast\src\4538" /I "..\..\..\..\..\rockford\applications\diags\satfe\bcm94538" /I "..\..\..\..\..\rockford\applications\diags\satfe" /I "..\..\..\..\..\rockford\applications\diags\satfe\4538" /I "..\..\..\..\..\magnum\basemodules\hab\include\common" /I "..\..\..\..\..\magnum\basemodules\hab\include\4538" /I "..\..\..\..\..\magnum\basemodules\hab\src\leapsatfe\4538" /I "..\..\..\..\..\magnum\basemodules\hab\src\common" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BDBG_DEBUG_BUILD" /D "_DEBUG" /D BCHP_CHIP=4538 /D BCHP_VER=A0 /D BSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG /D "PC_HOST" /D "__STDC_CONSTANT_MACROS" /D "BAST_DEBUG" /D "BAST_DONT_USE_INTERRUPT" /D a0=0 /D a1=1 /D b0=65536 /D BAST_4538_VER=a0 /D "BHAB_DONT_USE_INTERRUPT" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrtd.lib /nologo /subsystem:console /machine:I386 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=installing diags...
PostBuild_Cmds=copy Release\diags.exe C:\navarro\bcm4538\enavarro_4538_ap_sview\AP\bcm4538\a0\build
# End Special Build Tool

!ELSEIF  "$(CFG)" == "diags - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GX /Ox /Ot /Oa /Ow /Og /Oi /Op /I "..\..\..\..\..\magnum\basemodules\std" /I "..\..\..\..\..\magnum\basemodules\std\types\generic" /I "..\..\..\..\..\magnum\basemodules\std\config" /I "..\..\..\..\..\magnum\basemodules\err" /I "..\..\..\..\..\magnum\basemodules\dbg" /I "..\..\..\..\..\magnum\basemodules\kni\win32" /I "..\..\..\..\..\magnum\commonutils\lst" /I "..\..\..\..\..\magnum\basemodules\chp\include\common" /I "..\..\..\..\..\magnum\basemodules\chp\include\4538\rdb\a0" /I "..\..\..\..\..\magnum\basemodules\reg" /I "..\..\..\..\..\magnum\basemodules\int" /I "..\..\..\..\..\magnum\portinginterface\ast" /I "..\..\..\..\..\magnum\portinginterface\ast\4538" /I "..\..\..\..\..\magnum\portinginterface\ast\4538\microcode" /I "..\..\..\..\..\rockford\applications\diags\satfe\bcm94538" /I "..\..\..\..\..\rockford\applications\diags\satfe" /I "..\..\..\..\..\rockford\applications\diags\satfe\4538" /I "..\..\..\..\..\magnum\basemodules\hab" /I "..\..\..\..\..\magnum\basemodules\hab\4538" /D "_DEBUG" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BDBG_DEBUG_BUILD" /D BCHP_CHIP=4538 /D BCHP_VER=A0 /D BSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG /D "PC_HOST" /D "__STDC_CONSTANT_MACROS" /D "BAST_DEBUG" /D "BAST_DONT_USE_INTERRUPT" /D a0=0 /D a1=1 /D b0=65536 /D BAST_4517_VER=a0 /D "BHAB_DONT_USE_INTERRUPT" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrtd.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ENDIF

# Begin Target

# Name "diags - Win32 Release"
# Name "diags - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\src\common\bast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\src\4538\bast_4538.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\src\4538\bast_4538_priv.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\chp\src\common\bchp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_fifo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\win32\bdbg_os_priv.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_output.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\common\bhab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\leapsatfe\4538\bhab_4538.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\leapsatfe\4538\bhab_4538_fw_a0.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\leapsatfe\4538\bhab_4538_priv.c
# End Source File
# Begin Source File

SOURCE=.\bi2c.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\kni\win32\bkni_win32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\reg\breg_i2c.c
# End Source File
# Begin Source File

SOURCE=.\diags.cpp
# End Source File
# Begin Source File

SOURCE=.\diags.rc
# End Source File
# Begin Source File

SOURCE=..\satfe.c
# End Source File
# Begin Source File

SOURCE=..\4538\satfe_4538.c
# End Source File
# Begin Source File

SOURCE=..\satfe_data.c
# End Source File
# Begin Source File

SOURCE=.\satfe_platform.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\include\common\bast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\include\4538\bast_4538.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\src\4538\bast_4538_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\portinginterface\ast\src\common\bast_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\chp\include\common\bchp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\chp\src\common\bchp_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_app.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_fifo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_os_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\dbg\bdbg_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\err\berr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\err\berr_ids.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\include\common\bhab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\include\4538\bhab_4538.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\include\4538\bhab_4538_fw.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\leapsatfe\4538\bhab_4538_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\hab\src\common\bhab_priv.h
# End Source File
# Begin Source File

SOURCE=.\bi2c.h
# End Source File
# Begin Source File

SOURCE=.\bi2c_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\int\bint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\kni\generic\bkni.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\kni\generic\bkni_multi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\reg\breg_i2c.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\reg\breg_i2c_priv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\std\bstd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\std\config\bstd_cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\magnum\basemodules\std\types\generic\bstd_defs.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\satfe.h
# End Source File
# Begin Source File

SOURCE=..\4538\satfe_4538.h
# End Source File
# Begin Source File

SOURCE=.\satfe_platform.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
