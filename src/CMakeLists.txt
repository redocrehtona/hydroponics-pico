cmake_minimum_required(VERSION 3.12)

# Add source files to executable
add_executable(hydroponics-pico
	main.cpp)

# Add include files from the 'include' directory
target_include_directories(hydroponics-pico PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../lib)

# Link used libraries from pico-sdk and FreeRTOS-Kernel
target_link_libraries(hydroponics-pico pico_ssd1306 pico_stdlib hardware_i2c hardware_adc FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)

# Set pico i/o states
pico_enable_stdio_usb(hydroponics-pico 1)
pico_enable_stdio_uart(hydroponics-pico 0)

# Create UF2 and other files
pico_add_extra_outputs(hydroponics-pico)

