# CMake minimum version
cmake_minimum_required (VERSION 3.1)

# Project Infomation write your project name to here
project( PIRM )
enable_language(ASM)

# Use the FreeRTOS default is OFF, and you can set ON
option(FreeRTOS "Use the FreeRTOS ?" ON)
option(Modbus "Use the Modbus ?" ON)

# Setting the DEFINE to environment variables, other device can set here
set (ENV{DEFINE_DEVICE} STM32F40_41xxx)
set (ENV{DEFINE_DRIVER} USE_STDPERIPH_DRIVER)
set (ENV{MCPU} cortex-m4)
set (ENV{MFPU} fpv4-sp-d16)
# Set the startup and flash path to the variables
set (startup ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/Source/startup/startup_stm32f40_41xxx.s)
set (ldscript ${CMAKE_SOURCE_DIR}/ldscripts/STM32F40_41xxx/STM32F40_41xxx_FLASH.ld)

# Reset output path
set (CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# The need build source path and build all files
aux_source_directory (User/Source/ DIR_USER_SRCS)
aux_source_directory (Drivers/CMSIS/Device/Source DIR_CMSIS_SRCS)

# Set include path
include_directories (Drivers/CMSIS/Device/Include )
include_directories (Drivers/CMSIS/Include )
include_directories (User/Include)
include_directories (Drivers/STM32F4xx_StdPeriph_Driver/inc)

# Add the STM32F4xx_StdPeriph_Driver sub directory
add_subdirectory(Drivers)
IF(FreeRTOS)
	# Add the FreeRTOS sub directory
	add_subdirectory(ThirdParty/FreeRTOS)
	include_directories (ThirdParty/FreeRTOS/Include)
ENDIF()
IF(Modbus)
	# Add the FreeRTOS sub directory
	add_subdirectory(ThirdParty/Modbus)
	include_directories (ThirdParty/Modbus/include)
ENDIF()

# CC AR LD AS
set (CMAKE_C_COMPILER "arm-none-eabi-gcc")
set (CMAKE_C_AR "arm-none-eabi-ar")
set (CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
# STDLIB
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

# CFLAGS
set (CMAKE_C_FLAGS "-g -mthumb -fno-builtin -mcpu=$ENV{MCPU} -mfpu=$ENV{MFPU} -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D $ENV{DEFINE_DEVICE} -D $ENV{DEFINE_DRIVER}")
# CXXFLAGS
set (CMAKE_CXX_FLAGS "-g -mthumb -fno-builtin -mcpu=$ENV{MCPU} -mfpu=$ENV{MFPU} -mfloat-abi=softfp -Wall -std=c++11 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "cxx compiler flags")
# ASMFLAGS
set (CMAKE_ASM_FLAGS "-g -mthumb -mcpu=$ENV{MCPU} -mfpu=$ENV{MFPU} -mfloat-abi=softfp -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")
# LDFLAGS
set (CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -mthumb -mcpu=$ENV{MCPU} -mabi=aapcs -specs=nosys.specs" CACHE INTERNAL "executable linker flags")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${ldscript}")

# Generate the target
add_executable (${CMAKE_PROJECT_NAME}.elf ${DIR_USER_SRCS} ${DIR_CMSIS_SRCS} ${startup})
# Link the standard library to the target
target_link_libraries (${CMAKE_PROJECT_NAME}.elf STM32F4xx_StdPeriph_Driver)
IF(FreeRTOS)
	# Link the FreeRTOS library to the target
	target_link_libraries (${CMAKE_PROJECT_NAME}.elf STM32F4xx_FreeRTOS)
ENDIF()
IF(Modbus)
	# Link the FreeRTOS library to the target
	target_link_libraries (${CMAKE_PROJECT_NAME}.elf STM32F4xx_Modbus)
ENDIF()

# Load the the extern library
# add_library (STM32F4xx_StdPeriph_Driver STATIC IMPORTED)
# set_property (TARGET STM32F4xx_StdPeriph_Driver PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/Drivers/Build/lib/libSTM32F4xx_StdPeriph_Driver.a)

# Generate the binary file
add_custom_target(${CMAKE_PROJECT_NAME}.bin ALL arm-none-eabi-objcopy -Obinary "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)
# Generate the hex file
add_custom_target(${CMAKE_PROJECT_NAME}.hex ALL arm-none-eabi-objcopy -Oihex "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)
# Echo the size Infomation
add_custom_target (size ALL arm-none-eabi-size "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)
# Make flash to the board by st-link
add_custom_target (flash COMMAND st-flash write ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin 0x8000000)
# Make clean-all
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*)
