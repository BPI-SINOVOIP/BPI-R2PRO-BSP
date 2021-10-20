# Microsoft Developer Studio Project File - Name="cgicc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cgicc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cgicc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgicc.mak" CFG="cgicc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgicc - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgicc - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cgicc - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CGICC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CGICC_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "cgicc - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CGICC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CGICC_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "cgicc - Win32 Release"
# Name "cgicc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\cgicc\Cgicc.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiInput.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\FormEntry.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\FormFile.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLAttribute.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLAttributeList.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLDoctype.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLElement.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLElementList.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPContentHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPCookie.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPHTMLHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPPlainHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPRedirectHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPResponseHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPStatusHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\cgicc\MStreamable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\cgicc\Cgicc.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiDefs.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiEnvironment.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiInput.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\CgiUtils.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\FormEntry.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\FormFile.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLAtomicElement.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLAttribute.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLAttributeList.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLBooleanElement.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLClasses.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLDoctype.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLElement.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTMLElementList.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPContentHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPCookie.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPHTMLHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPPlainHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPRedirectHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPResponseHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\HTTPStatusHeader.h
# End Source File
# Begin Source File

SOURCE=..\cgicc\MStreamable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
