# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build

# Utility rule file for PIRM.bin.

# Include the progress variables for this target.
include CMakeFiles/PIRM.bin.dir/progress.make

CMakeFiles/PIRM.bin: bin/PIRM.elf
	arm-none-eabi-objcopy -Obinary /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build/bin/PIRM.elf /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build/bin/PIRM.bin

PIRM.bin: CMakeFiles/PIRM.bin
PIRM.bin: CMakeFiles/PIRM.bin.dir/build.make

.PHONY : PIRM.bin

# Rule to build all files generated by this target.
CMakeFiles/PIRM.bin.dir/build: PIRM.bin

.PHONY : CMakeFiles/PIRM.bin.dir/build

CMakeFiles/PIRM.bin.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PIRM.bin.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PIRM.bin.dir/clean

CMakeFiles/PIRM.bin.dir/depend:
	cd /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build /home/wangxian/Code/STM32/STM32F4/STM32F405RGT6/PIRM/build/CMakeFiles/PIRM.bin.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PIRM.bin.dir/depend

