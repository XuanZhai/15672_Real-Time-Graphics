﻿# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2023.3.3\bin\cmake\win\x64\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2023.3.3\bin\cmake\win\x64\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles\XuanJamesZhai_A1.dir\depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles\XuanJamesZhai_A1.dir\compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles\XuanJamesZhai_A1.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\XuanJamesZhai_A1.dir\flags.make

CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj: CMakeFiles\XuanJamesZhai_A1.dir\flags.make
CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj: C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\main.cpp
CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj: CMakeFiles\XuanJamesZhai_A1.dir\compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/XuanJamesZhai_A1.dir/main.cpp.obj"
	$(CMAKE_COMMAND) -E cmake_cl_compile_depends --dep-file=CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj.d --working-dir=C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug --filter-prefix="Note: including file: " -- C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /showIncludes /FoCMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj /FdCMakeFiles\XuanJamesZhai_A1.dir\ /FS -c C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\main.cpp
<<

CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/XuanJamesZhai_A1.dir/main.cpp.i"
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe > CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\main.cpp
<<

CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/XuanJamesZhai_A1.dir/main.cpp.s"
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\XuanJamesZhai_A1.dir\main.cpp.s /c C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\main.cpp
<<

CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj: CMakeFiles\XuanJamesZhai_A1.dir\flags.make
CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj: C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\XZJParser.cpp
CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj: CMakeFiles\XuanJamesZhai_A1.dir\compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/XuanJamesZhai_A1.dir/XZJParser.cpp.obj"
	$(CMAKE_COMMAND) -E cmake_cl_compile_depends --dep-file=CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj.d --working-dir=C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug --filter-prefix="Note: including file: " -- C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /showIncludes /FoCMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj /FdCMakeFiles\XuanJamesZhai_A1.dir\ /FS -c C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\XZJParser.cpp
<<

CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/XuanJamesZhai_A1.dir/XZJParser.cpp.i"
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe > CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\XZJParser.cpp
<<

CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/XuanJamesZhai_A1.dir/XZJParser.cpp.s"
	C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.s /c C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\XZJParser.cpp
<<

# Object files for target XuanJamesZhai_A1
XuanJamesZhai_A1_OBJECTS = \
"CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj" \
"CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj"

# External object files for target XuanJamesZhai_A1
XuanJamesZhai_A1_EXTERNAL_OBJECTS =

XuanJamesZhai_A1.exe: CMakeFiles\XuanJamesZhai_A1.dir\main.cpp.obj
XuanJamesZhai_A1.exe: CMakeFiles\XuanJamesZhai_A1.dir\XZJParser.cpp.obj
XuanJamesZhai_A1.exe: CMakeFiles\XuanJamesZhai_A1.dir\build.make
XuanJamesZhai_A1.exe: CMakeFiles\XuanJamesZhai_A1.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable XuanJamesZhai_A1.exe"
	"C:\Program Files\JetBrains\CLion 2023.3.3\bin\cmake\win\x64\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\XuanJamesZhai_A1.dir --rc=C:\PROGRA~2\WI3CF2~1\10\bin\100190~1.0\x64\rc.exe --mt=C:\PROGRA~2\WI3CF2~1\10\bin\100190~1.0\x64\mt.exe --manifests -- C:\PROGRA~2\MICROS~2\2019\COMMUN~1\VC\Tools\MSVC\1429~1.301\bin\Hostx64\x64\link.exe /nologo @CMakeFiles\XuanJamesZhai_A1.dir\objects1.rsp @<<
 /out:XuanJamesZhai_A1.exe /implib:XuanJamesZhai_A1.lib /pdb:C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug\XuanJamesZhai_A1.pdb /version:0.0 /machine:x64 /debug /INCREMENTAL /subsystem:console  kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\XuanJamesZhai_A1.dir\build: XuanJamesZhai_A1.exe
.PHONY : CMakeFiles\XuanJamesZhai_A1.dir\build

CMakeFiles\XuanJamesZhai_A1.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\XuanJamesZhai_A1.dir\cmake_clean.cmake
.PHONY : CMakeFiles\XuanJamesZhai_A1.dir\clean

CMakeFiles\XuanJamesZhai_A1.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1 C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1 C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug C:\Code\15672_Real-Time-Graphics\XuanJamesZhai_A1\cmake-build-debug\CMakeFiles\XuanJamesZhai_A1.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles\XuanJamesZhai_A1.dir\depend

