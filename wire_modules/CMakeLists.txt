cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(wire
	wires.c
)

pico_enable_stdio_usb(wire 1)
pico_enable_stdio_uart(wire 0)
pico_add_extra_outputs(wire)
target_link_libraries(
	wire 
	pico_stdlib 
	pico_multicore 
	pico_i2c_slave
	hardware_adc 
	hardware_i2c
)
