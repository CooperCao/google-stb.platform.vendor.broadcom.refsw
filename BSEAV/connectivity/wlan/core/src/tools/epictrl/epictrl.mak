# Microsoft Developer Studio Generated NMAKE File, Based on epictrl.dsp
!IF "$(CFG)" == ""
CFG=epictrl - Win32 Debug
!MESSAGE No configuration specified. Defaulting to epictrl - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "epictrl - Win32 Release" && "$(CFG)" != "epictrl - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "epictrl.mak" CFG="epictrl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "epictrl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "epictrl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "epictrl - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\epictrl.dll"


CLEAN :
	-@erase "$(INTDIR)\epictrl.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\epictrl.dll"
	-@erase "$(OUTDIR)\epictrl.exp"
	-@erase "$(OUTDIR)\epictrl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\include" /I "..\..\common\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EPICTRL_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /Fp"$(INTDIR)\epictrl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\epictrl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\epictrl.pdb" /machine:I386 /def:"epictrl.def" /out:"$(OUTDIR)\epictrl.dll" /implib:"$(OUTDIR)\epictrl.lib" 
LINK32_OBJS= \
	"$(INTDIR)\epictrl.obj"

"$(OUTDIR)\epictrl.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "epictrl - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\epictrl.dll"


CLEAN :
	-@erase "$(INTDIR)\epictrl.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\epictrl.dll"
	-@erase "$(OUTDIR)\epictrl.exp"
	-@erase "$(OUTDIR)\epictrl.ilk"
	-@erase "$(OUTDIR)\epictrl.lib"
	-@erase "$(OUTDIR)\epictrl.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\common\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EPICTRL_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /Fp"$(INTDIR)\epictrl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\epictrl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\epictrl.pdb" /debug /machine:I386 /def:"epictrl.def" /out:"$(OUTDIR)\epictrl.dll" /implib:"$(OUTDIR)\epictrl.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\epictrl.obj"

"$(OUTDIR)\epictrl.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("epictrl.dep")
!INCLUDE "epictrl.dep"
!ELSE 
!MESSAGE Warning: cannot find "epictrl.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "epictrl - Win32 Release" || "$(CFG)" == "epictrl - Win32 Debug"
SOURCE=.\epictrl.c

"$(INTDIR)\epictrl.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 
