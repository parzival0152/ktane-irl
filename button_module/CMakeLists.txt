cmake_minimum_required(VERSION 3.13)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(button
	button.c
	display.c
)

pico_enable_stdio_usb(button 1)
pico_enable_stdio_uart(button 0)
pico_add_extra_outputs(button)
target_link_libraries(
	button 
	pico_stdlib 
	pico_multicore
    pico_i2c_slave
	pico_rand
	hardware_i2c
)
