cmake_minimum_required(VERSION 3.10)
project(uGB C ASM)

# Set the build type
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker_rp2350_defcon.lkr")

# Define default options
option(BUILD_AVR "Build for AVR microcontroller" OFF)
option(USE_SDL "Build with SDL2 support" OFF)

# Set C and ASM compilers
if(BUILD_AVR)
    set(CMAKE_C_COMPILER avr-gcc)
    set(CMAKE_ASM_COMPILER avr-gcc)
    set(CMAKE_EXE_LINKER_FLAGS "-O3 -mmcu=atmega128 -Wl,--gc-sections")
    set(CMAKE_C_FLAGS "-O3 -mmcu=atmega128 -I/usr/lib/avr/include -DEMBEDDED -D_SIM -ffunction-sections -DAVR_ASM")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
else()
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-T${LINKER_SCRIPT} -lm -mthumb -fsingle-precision-constant -ffast-math -mcpu=cortex-m33 -mfloat-abi=soft")
    set(CMAKE_C_FLAGS "-Ofast -Wall -Wextra -Werror -mthumb -fsingle-precision-constant -ffast-math -mcpu=cortex-m33 -mfloat-abi=soft -Wno-nonnull -ffixed-r11 -fstack-reuse=all -Wno-array-bounds -Wall -Wno-clobbered -ffunction-sections -fdata-sections -fomit-frame-pointer -Wno-unused-parameter -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -DZWT_ADDR=0x20081ff8 -DTICKS_PER_SECOND=125000000U -DNO_SUPPORT_DEBUG_PRINTF -DEMBEDDED -DQUIET_MAPPERS -DFATFS_USE_LFN_SUPPORT=1 -DUSE_WIN_1252_CODE_PAGE -DQSPI_RAM_SIZE_MAX=65536 -DDISP_LCD_DEFCON -DUNSCALED_IMG_ROTATED -DUPSCALER_ROTATES -DUI_ROTATED -DGB_HAVE_EXTRA_IO_REGS -Wall -Wextra -Wno-unused-label -Wno-missing-field-initializers")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
endif()

# Set OBJCOPY
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

# Set output files
set(ELF_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")
set(BIN_FILE "${CMAKE_BINARY_DIR}/FIRMWARE.BIN")

# Include TinyUSB as a subdirectory
#add_subdirectory(tinyusb)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/inc_RP2350)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add source files
set(SOURCES
    src/main_rp2350_defcon.c
    src/crt_rp2350.S
    src/timebase.c
    src/printf.c
    src/dispDefcon.c
    src/utf.c
    src/qspi2350.c
    src/ui.c
    src/settings.c
    src/fonts.c
    src/sleepDefcon.c
    src/sd.c
    src/fatfs.c
    src/sdHwRP2350.c
    src/pioI2C.c
    src/pioIrdaSIR.c
    src/pioWS2812.c
    src/gbC.c
    src/videoC.c
    src/mappersC.c
    src/gb.c
    src/mbc.c
)

if(HQX)
    list(APPEND SOURCES hq3x.c)
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link TinyUSB library (if it provides an `add_library` target)
# target_link_libraries(${PROJECT_NAME} tinyusb)

# Set additional linker flags
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})


# Add a custom command to create the .bin file
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${BIN_FILE} -j.text -j.rodata -j.data -j.vectors
    COMMENT "Generating binary file ${BIN_FILE}"
)

# Add a custom target to depend on the binary file creation
add_custom_target(
    ${PROJECT_NAME}_bin ALL
    DEPENDS ${BIN_FILE}
)