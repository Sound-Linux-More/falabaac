# Microsoft Developer Studio Project File - Name="libfalabaac" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libfalabaac - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libfalabaac.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libfalabaac.mak" CFG="libfalabaac - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libfalabaac - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libfalabaac - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libfalabaac - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy       ..\fa_aacapi.h       ..\..\include      	copy       ..\fa_inttypes.h       ..\..\include      	copy       .\Release\libfalabaac.lib       .\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "libfalabaac - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy       ..\fa_aacapi.h       ..\..\include      	copy       ..\fa_inttypes.h       ..\..\include      	copy       .\Debug\libfalabaac.lib        .\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libfalabaac - Win32 Release"
# Name "libfalabaac - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\fa_aacblockswitch.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacchn.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacenc.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacfilterbank.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacms.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacpsy.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacquant.c
# End Source File
# Begin Source File

SOURCE=..\fa_aacstream.c
# End Source File
# Begin Source File

SOURCE=..\fa_bitbuffer.c
# End Source File
# Begin Source File

SOURCE=..\fa_bitstream.c
# End Source File
# Begin Source File

SOURCE=..\fa_corr.c
# End Source File
# Begin Source File

SOURCE=..\fa_fastmath.c
# End Source File
# Begin Source File

SOURCE=..\fa_fft.c
# End Source File
# Begin Source File

SOURCE=..\fa_fir.c
# End Source File
# Begin Source File

SOURCE=..\fa_huffman.c
# End Source File
# Begin Source File

SOURCE=..\fa_huffmantab.c
# End Source File
# Begin Source File

SOURCE=..\fa_iqtab.c
# End Source File
# Begin Source File

SOURCE=..\fa_levinson.c
# End Source File
# Begin Source File

SOURCE=..\fa_lpc.c
# End Source File
# Begin Source File

SOURCE=..\fa_mdct.c
# End Source File
# Begin Source File

SOURCE=..\fa_mdctquant.c
# End Source File
# Begin Source File

SOURCE=..\fa_psychomodel1.c
# End Source File
# Begin Source File

SOURCE=..\fa_psychomodel2.c
# End Source File
# Begin Source File

SOURCE=..\fa_psytab.c
# End Source File
# Begin Source File

SOURCE=..\fa_quantpdf.c
# End Source File
# Begin Source File

SOURCE=..\fa_swbtab.c
# End Source File
# Begin Source File

SOURCE=..\fa_tns.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\fa_aacapi.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacblockswitch.h
# End Source File
# Begin Source File

SOURCE=..\fa_aaccfg.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacchn.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacenc.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacfilterbank.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacms.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacpsy.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacquant.h
# End Source File
# Begin Source File

SOURCE=..\fa_aacstream.h
# End Source File
# Begin Source File

SOURCE=..\fa_bitbuffer.h
# End Source File
# Begin Source File

SOURCE=..\fa_bitstream.h
# End Source File
# Begin Source File

SOURCE=..\fa_corr.h
# End Source File
# Begin Source File

SOURCE=..\fa_fastmath.h
# End Source File
# Begin Source File

SOURCE=..\fa_fft.h
# End Source File
# Begin Source File

SOURCE=..\fa_fir.h
# End Source File
# Begin Source File

SOURCE=..\fa_huffman.h
# End Source File
# Begin Source File

SOURCE=..\fa_huffmantab.h
# End Source File
# Begin Source File

SOURCE=..\fa_inttypes.h
# End Source File
# Begin Source File

SOURCE=..\fa_iqtab.h
# End Source File
# Begin Source File

SOURCE=..\fa_levinson.h
# End Source File
# Begin Source File

SOURCE=..\fa_lpc.h
# End Source File
# Begin Source File

SOURCE=..\fa_mdct.h
# End Source File
# Begin Source File

SOURCE=..\fa_mdctquant.h
# End Source File
# Begin Source File

SOURCE=..\fa_psychomodel1.h
# End Source File
# Begin Source File

SOURCE=..\fa_psychomodel2.h
# End Source File
# Begin Source File

SOURCE=..\fa_psytab.h
# End Source File
# Begin Source File

SOURCE=..\fa_quantpdf.h
# End Source File
# Begin Source File

SOURCE=..\fa_swbtab.h
# End Source File
# Begin Source File

SOURCE=..\fa_tns.h
# End Source File
# End Group
# End Target
# End Project
