# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = "/Users/bodguy/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/202.6397.106/CLion.app/Contents/bin/cmake/mac/bin/cmake"

# The command to remove a file.
RM = "/Users/bodguy/Library/Application Support/JetBrains/Toolbox/apps/CLion/ch-0/202.6397.106/CLion.app/Contents/bin/cmake/mac/bin/cmake" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/bodguy/Desktop/algorithm/rendering_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/rendering_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rendering_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rendering_test.dir/flags.make

CMakeFiles/rendering_test.dir/main.cpp.o: CMakeFiles/rendering_test.dir/flags.make
CMakeFiles/rendering_test.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rendering_test.dir/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rendering_test.dir/main.cpp.o -c /Users/bodguy/Desktop/algorithm/rendering_test/main.cpp

CMakeFiles/rendering_test.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rendering_test.dir/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/bodguy/Desktop/algorithm/rendering_test/main.cpp > CMakeFiles/rendering_test.dir/main.cpp.i

CMakeFiles/rendering_test.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rendering_test.dir/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/bodguy/Desktop/algorithm/rendering_test/main.cpp -o CMakeFiles/rendering_test.dir/main.cpp.s

# Object files for target rendering_test
rendering_test_OBJECTS = \
"CMakeFiles/rendering_test.dir/main.cpp.o"

# External object files for target rendering_test
rendering_test_EXTERNAL_OBJECTS =

rendering_test: CMakeFiles/rendering_test.dir/main.cpp.o
rendering_test: CMakeFiles/rendering_test.dir/build.make
rendering_test: /usr/local/lib/libGLEW.a
rendering_test: /usr/local/lib/libglfw3.a
rendering_test: CMakeFiles/rendering_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rendering_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rendering_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rendering_test.dir/build: rendering_test

.PHONY : CMakeFiles/rendering_test.dir/build

CMakeFiles/rendering_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rendering_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rendering_test.dir/clean

CMakeFiles/rendering_test.dir/depend:
	cd /Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/bodguy/Desktop/algorithm/rendering_test /Users/bodguy/Desktop/algorithm/rendering_test /Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug /Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug /Users/bodguy/Desktop/algorithm/rendering_test/cmake-build-debug/CMakeFiles/rendering_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rendering_test.dir/depend
