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
CMAKE_BINARY_DIR = /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote

# Utility rule file for docs.

# Include the progress variables for this target.
include doc/CMakeFiles/docs.dir/progress.make

doc/CMakeFiles/docs:
	cd /tmp/tmp.8uD4RXirDP && doxygen doc/doxyconfig

docs: doc/CMakeFiles/docs
docs: doc/CMakeFiles/docs.dir/build.make

.PHONY : docs

# Rule to build all files generated by this target.
doc/CMakeFiles/docs.dir/build: docs

.PHONY : doc/CMakeFiles/docs.dir/build

doc/CMakeFiles/docs.dir/clean:
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/doc && $(CMAKE_COMMAND) -P CMakeFiles/docs.dir/cmake_clean.cmake
.PHONY : doc/CMakeFiles/docs.dir/clean

doc/CMakeFiles/docs.dir/depend:
	cd /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.8uD4RXirDP /tmp/tmp.8uD4RXirDP/doc /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/doc /tmp/tmp.8uD4RXirDP/cmake-build-debug-remote/doc/CMakeFiles/docs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : doc/CMakeFiles/docs.dir/depend

