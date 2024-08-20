cmake_minimum_required(VERSION 3.27)

set(PICO_BOARD defcon32_badge)

# initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(DEFCON32)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
# Add the source files to your project
add_executable(${PROJECT_NAME} src/main.c src/pio/pioWS2812.c src/timebase.c src/display.c src/ui.c src/chars.c src/fonts.c src/utf.c src/printf.c src/input.c src/usb_descriptors.c src/usb.c)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Link the Pico SDK libraries to your project
target_link_libraries(${PROJECT_NAME} pico_stdlib pico_multicore hardware_spi hardware_pwm hardware_pio hardware_dma pico_unique_id tinyusb_device tinyusb_board)

# Set the C standard to C11
target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_CXX_STANDARD=11)

# Enable extra outputs for debugging
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})