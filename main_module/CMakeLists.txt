cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(/opt/pico/TM1637-pico/PicoTM1637.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(master
	master.c
)

pico_enable_stdio_usb(master 1)
pico_enable_stdio_uart(master 0)
pico_add_extra_outputs(master)
target_link_libraries(
	master
	pico_stdlib
	pico_multicore
	pico_rand
	hardware_gpio
	hardware_i2c
	PicoTM1637
)
