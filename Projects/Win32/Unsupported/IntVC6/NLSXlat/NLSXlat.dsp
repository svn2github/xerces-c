# Microsoft Developer Studio Project File - Name="NLSXlat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NLSXlat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NLSXlat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NLSXlat.mak" CFG="NLSXlat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NLSXlat - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NLSXlat - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NLSXlat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\Build\Win32\VC6\Release\"
# PROP Intermediate_Dir "..\..\..\..\..\Build\Win32\VC6\Release\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /Za /W3 /GX /O2 /Ob2 /I "..\..\..\..\..\src" /I "\ICU\Include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib IXDom.lib IXUtil.lib IXPar.lib IXSAX.lib icuuc.lib /nologo /version:3.0 /subsystem:console /machine:I386 /libpath:"..\..\..\..\..\Build\Win32\VC6\Release" /libpath:"\ICU\Lib\Release"

!ELSEIF  "$(CFG)" == "NLSXlat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\Build\Win32\VC6\Debug\"
# PROP Intermediate_Dir "..\..\..\..\..\Build\Win32\VC6\Debug\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /Za /W3 /GX /Zi /Od /I "..\..\..\..\..\src" /I "\ICU\Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib IXDom.lib IXUtil.lib IXPar.lib IXSAX.lib icuuc.lib /nologo /version:3.0 /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\Build\Win32\VC6\Debug" /libpath:"\ICU\Lib\Debug"

!ENDIF 

# Begin Target

# Name "NLSXlat - Win32 Release"
# Name "NLSXlat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_CppSrc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_MsgCatalog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_Win32RC.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_CppSrc.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_ErrHandler.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_Formatter.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_MsgCatalog.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_Types.hpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Tools\NLS\Xlat\Xlat_Win32RC.hpp
# End Source File
# End Group
# End Target
# End Project
