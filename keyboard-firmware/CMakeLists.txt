cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(keeb_firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(keeb_firmware
        keeb_firmware.c
        )

# pull in common dependencies
target_link_libraries(keeb_firmware pico_stdlib)

#enable uart; disable usb (printf)
#pico_enable_stdio_usb(keeb_firmware 1)
#pico_enable_stdio_uart(keeb_firmware 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(keeb_firmware)

# add url via pico_set_program_url
# example_auto_set_url(blink)
