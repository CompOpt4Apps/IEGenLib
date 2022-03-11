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

# Utility rule file for mtest.

# Include the progress variables for this target.
include CMakeFiles/mtest.dir/progress.make

CMakeFiles/mtest: iegen_lib_memtest


iegen_lib_memtest: bin/iegenlib_t
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/tmp/tmp.8uD4RXirDP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Run iegenlib tests through valgrind for memory error detection"
	IEGEN_HOME=. valgrind --leak-check=full src/iegenlib_t

mtest: CMakeFiles/mtest
mtest: iegen_lib_memtest
mtest: CMakeFiles/mtest.dir/build.make

.PHONY : mtest

# Rule to build all files generated by this target.
CMakeFiles/mtest.dir/build: mtest

.PHONY : CMakeFiles/mtest.dir/build

CMakeFiles/mtest.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mtest.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mtest.dir/clean

CMakeFiles/mtest.dir/depend:
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.8uD4RXirDP /tmp/tmp.8uD4RXirDP /tmp/tmp.8uD4RXirDP/cmake-build-debug /tmp/tmp.8uD4RXirDP/cmake-build-debug /tmp/tmp.8uD4RXirDP/cmake-build-debug/CMakeFiles/mtest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mtest.dir/depend

