# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/berk/CLionProjects/MPI/Mark1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Mark1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Mark1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Mark1.dir/flags.make

CMakeFiles/Mark1.dir/main.cpp.o: CMakeFiles/Mark1.dir/flags.make
CMakeFiles/Mark1.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Mark1.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Mark1.dir/main.cpp.o -c /Users/berk/CLionProjects/MPI/Mark1/main.cpp

CMakeFiles/Mark1.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Mark1.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/berk/CLionProjects/MPI/Mark1/main.cpp > CMakeFiles/Mark1.dir/main.cpp.i

CMakeFiles/Mark1.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Mark1.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/berk/CLionProjects/MPI/Mark1/main.cpp -o CMakeFiles/Mark1.dir/main.cpp.s

# Object files for target Mark1
Mark1_OBJECTS = \
"CMakeFiles/Mark1.dir/main.cpp.o"

# External object files for target Mark1
Mark1_EXTERNAL_OBJECTS =

Mark1: CMakeFiles/Mark1.dir/main.cpp.o
Mark1: CMakeFiles/Mark1.dir/build.make
Mark1: CMakeFiles/Mark1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Mark1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Mark1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Mark1.dir/build: Mark1

.PHONY : CMakeFiles/Mark1.dir/build

CMakeFiles/Mark1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Mark1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Mark1.dir/clean

CMakeFiles/Mark1.dir/depend:
	cd /Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/berk/CLionProjects/MPI/Mark1 /Users/berk/CLionProjects/MPI/Mark1 /Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug /Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug /Users/berk/CLionProjects/MPI/Mark1/cmake-build-debug/CMakeFiles/Mark1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Mark1.dir/depend

