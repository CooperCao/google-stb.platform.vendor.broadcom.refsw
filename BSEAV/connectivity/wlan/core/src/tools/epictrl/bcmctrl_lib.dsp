# Microsoft Developer Studio Project File - Name="bcmctrl_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bcmctrl_lib - Win32 Debug MH
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bcmctrl_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bcmctrl_lib.mak" CFG="bcmctrl_lib - Win32 Debug MH"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bcmctrl_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bcmctrl_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bcmctrl_lib - Win32 Debug MH" (based on "Win32 (x86) Static Library")
!MESSAGE "bcmctrl_lib - Win32 Release MH" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bcmctrl_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\BRCM"
# PROP Intermediate_Dir "Release\BRCM"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "../../include" /I "../../common/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\BRCM\bcmctrl.lib"

!ELSEIF  "$(CFG)" == "bcmctrl_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bcmctrl_lib___Win32_Debug"
# PROP BASE Intermediate_Dir "bcmctrl_lib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\BRCM"
# PROP Intermediate_Dir "Debug\BRCM"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../common/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\BRCM\bcmctrl.lib"

!ELSEIF  "$(CFG)" == "bcmctrl_lib - Win32 Debug MH"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bcmctrl_lib___Win32_Debug_MH"
# PROP BASE Intermediate_Dir "bcmctrl_lib___Win32_Debug_MH"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bcmctrl_lib___Win32_Debug_MH"
# PROP Intermediate_Dir "bcmctrl_lib___Win32_Debug_MH"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../common/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../common/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\BRCM\bcmctrl.lib"
# ADD LIB32 /nologo /out:"Debug\BRCM\bcmctrl.lib"

!ELSEIF  "$(CFG)" == "bcmctrl_lib - Win32 Release MH"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bcmctrl_lib___Win32_Release_MH"
# PROP BASE Intermediate_Dir "bcmctrl_lib___Win32_Release_MH"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bcmctrl_lib___Win32_Release_MH"
# PROP Intermediate_Dir "bcmctrl_lib___Win32_Release_MH"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /I "../../include" /I "../../common/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "../../include" /I "../../common/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release\BRCM\bcmctrl.lib"
# ADD LIB32 /nologo /out:"Release\BRCM\bcmctrl.lib"

!ENDIF 

# Begin Target

# Name "bcmctrl_lib - Win32 Release"
# Name "bcmctrl_lib - Win32 Debug"
# Name "bcmctrl_lib - Win32 Debug MH"
# Name "bcmctrl_lib - Win32 Release MH"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\driveloader.c
# End Source File
# Begin Source File

SOURCE=.\epictrl.c
# End Source File
# Begin Source File

SOURCE=.\epictrl.def
# End Source File
# Begin Source File

SOURCE=.\ir.c
# End Source File
# Begin Source File

SOURCE=.\ir_wmi.c
# End Source File
# Begin Source File

SOURCE=.\irhelper.c
# End Source File
# Begin Source File

SOURCE=.\reg9x.c
# End Source File
# Begin Source File

SOURCE=.\regnt4.c
# End Source File
# Begin Source File

SOURCE=.\version.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "../../include" /i "../../common/include"
# End Source File
# Begin Source File

SOURCE=.\win9x2k.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ir_wmi.h
# End Source File
# Begin Source File

SOURCE=.\wmiprocs.h
# End Source File
# End Group
# End Target
# End Project
