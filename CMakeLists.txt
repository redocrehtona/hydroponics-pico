cmake_minimum_required(VERSION 3.12)

# Set the project name: 
set(NAME desk-pico)

# Include the Raspberry Pi Pico SDK makefile
include(lib/pico-sdk/pico_sdk_init.cmake)

# Create project and set C versions 
project($NAME C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise the Pi Pico SDK
pico_sdk_init()

# Include the directory containing the port-specific config file for FreeRTOS
include_directories(port/FreeRTOS-Kernel)

include_directories(lib/PCF8575)

# Include the FreeRTOS makefile
include(lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Add the main source directory
add_subdirectory(src)
