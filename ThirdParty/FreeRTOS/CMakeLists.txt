# CMake minimum version
cmake_minimum_required (VERSION 3.1)

# The need build source path and build all files
aux_source_directory (Source DIR_STM32F4xx_FreeRTOS_SRCS)

# Set include path
include_directories(Include)

# CC AR
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_C_AR "arm-none-eabi-ar")

# CFLAGS
set (CMAKE_C_FLAGS "-g -mthumb -fno-builtin -mcpu=$ENV{MCPU} -mfpu=$ENV{MFPU} -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections")
set (${CMAKE_C_FLAGS} "-fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")

# Generate the target to the static library
add_library (STM32F4xx_FreeRTOS ${DIR_STM32F4xx_FreeRTOS_SRCS})
