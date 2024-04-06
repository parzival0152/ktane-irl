cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(memory
	memory.c
)

pico_enable_stdio_usb(memory 1)
pico_enable_stdio_uart(memory 0)
pico_add_extra_outputs(memory)
target_link_libraries(
	memory 
	pico_stdlib 
	pico_multicore
    pico_i2c_slave
	pico_rand
	hardware_i2c
)
