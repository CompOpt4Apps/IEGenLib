# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /usr/local/apps/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/apps/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.8uD4RXirDP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.8uD4RXirDP/cmake-build-debug

# Include any dependencies generated for this target.
include tutorial/CMakeFiles/manual_graph_testing.dir/depend.make

# Include the progress variables for this target.
include tutorial/CMakeFiles/manual_graph_testing.dir/progress.make

# Include the compile flags for this target's objects.
include tutorial/CMakeFiles/manual_graph_testing.dir/flags.make

tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o: tutorial/CMakeFiles/manual_graph_testing.dir/flags.make
tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o: ../tutorial/manual_graph_testing.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.8uD4RXirDP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o"
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o -c /tmp/tmp.8uD4RXirDP/tutorial/manual_graph_testing.cc

tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.i"
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.8uD4RXirDP/tutorial/manual_graph_testing.cc > CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.i

tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.s"
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.8uD4RXirDP/tutorial/manual_graph_testing.cc -o CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.s

# Object files for target manual_graph_testing
manual_graph_testing_OBJECTS = \
"CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o"

# External object files for target manual_graph_testing
manual_graph_testing_EXTERNAL_OBJECTS =

bin/tutorial/manual_graph_testing: tutorial/CMakeFiles/manual_graph_testing.dir/manual_graph_testing.cc.o
bin/tutorial/manual_graph_testing: tutorial/CMakeFiles/manual_graph_testing.dir/build.make
bin/tutorial/manual_graph_testing: src/libiegenlib.a
bin/tutorial/manual_graph_testing: tutorial/CMakeFiles/manual_graph_testing.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.8uD4RXirDP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/tutorial/manual_graph_testing"
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/manual_graph_testing.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tutorial/CMakeFiles/manual_graph_testing.dir/build: bin/tutorial/manual_graph_testing

.PHONY : tutorial/CMakeFiles/manual_graph_testing.dir/build

tutorial/CMakeFiles/manual_graph_testing.dir/clean:
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial && $(CMAKE_COMMAND) -P CMakeFiles/manual_graph_testing.dir/cmake_clean.cmake
.PHONY : tutorial/CMakeFiles/manual_graph_testing.dir/clean

tutorial/CMakeFiles/manual_graph_testing.dir/depend:
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.8uD4RXirDP /tmp/tmp.8uD4RXirDP/tutorial /tmp/tmp.8uD4RXirDP/cmake-build-debug /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial /tmp/tmp.8uD4RXirDP/cmake-build-debug/tutorial/CMakeFiles/manual_graph_testing.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tutorial/CMakeFiles/manual_graph_testing.dir/depend

