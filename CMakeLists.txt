cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
include(pico_sdk_import.cmake)

project(sms C CXX ASM)

pico_sdk_init()
SET(GCC_RELEASENODEBUG_FLAGS "-O2 -DNDEBUG")
SET(CMAKE_ASM_FLAGS_RELEASENODEBUG
        "${GCC_RELEASENODEBUG_FLAGS}"
        CACHE STRING "Flags used by the ASM compiler during REL_O2 builds."
        FORCE)

SET(CMAKE_CXX_FLAGS_RELEASENODEBUG
        "${GCC_RELEASENODEBUG_FLAGS}"
        CACHE STRING "Flags used by the C++ compiler during REL_O2 builds."
        FORCE)
SET(CMAKE_C_FLAGS_RELEASENODEBUG
        "${GCC_RELEASENODEBUG_FLAGS}"
        CACHE STRING "Flags used by the C compiler during REL_O2 builds."
        FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
        ""
        CACHE STRING "Flags used for linking binaries during REL_O2 builds."
        FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG
        ""
        CACHE STRING "Flags used by the shared libraries linker during REL_O2 builds."
        FORCE)
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_RELEASENODEBUG
        CMAKE_ASM_FLAGS_RELEASENODEBUG
        CMAKE_C_FLAGS_RELEASENODEBUG
        CMAKE_EXE_LINKER_FLAGS_RELEASENODEBUG
        CMAKE_SHARED_LINKER_FLAGS_RELEASENODEBUG)

add_subdirectory(TotalSMS)
add_subdirectory(drivers/audio)
add_subdirectory(drivers/ps2kbd)
add_subdirectory(drivers/vga)
add_subdirectory(drivers/nespad)
add_subdirectory(drivers/fatfs)
add_subdirectory(drivers/sdcard)


add_executable(sms
        main.cpp
)

pico_set_program_name(sms "Sega Master System/GameGear emulator by xrip for MURMULATOR devboard")
pico_set_program_version(sms "dev")

if(OVERCLOCKING)
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME  "${PROJECT_NAME}-${OVERCLOCKING}MHz")

    target_compile_definitions(${PROJECT_NAME} PRIVATE OVERCLOCKING=${OVERCLOCKING})

    pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
    target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
    pico_set_boot_stage2(${PROJECT_NAME} slower_boot2)

    message(STATUS "==================================")
    message(STATUS "Overclocking ${OVERCLOCKING}MHz enabled")
    message(STATUS "==================================")
    message(STATUS "")
endif ()


target_include_directories(sms PRIVATE TotalSMS)

pico_enable_stdio_uart(sms 0)
pico_enable_stdio_usb(sms 1)

target_link_libraries(sms PRIVATE
        TotalSMS

        audio
        vga
        sdcard
        fatfs
        nespad
        ps2kbd

        pico_runtime
        pico_stdlib
        pico_multicore
        hardware_dma
        hardware_pio
        hardware_i2c
        hardware_interp
        hardware_timer
        hardware_clocks
        hardware_pwm
        hardware_flash
)

target_compile_definitions(sms PRIVATE
        SPEAKER_ENABLED
        # VGA
        VGA_GPIO_FIRST=6
        VGA_GPIO_OUTNUM=6
        VGA_GPIO_SYNC=12
        VGA_VSYNC=13

        # SDCARD
        SDCARD_PIN_SPI0_CS=5
        SDCARD_PIN_SPI0_SCK=2
        SDCARD_PIN_SPI0_MOSI=3
        SDCARD_PIN_SPI0_MISO=4

        # PS2KBD
        USE_PS2_KBD
        PS2KBD_GPIO_FIRST=0

        # NES Gamepad
        USE_NESPAD
        NES_GPIO_CLK=14
        NES_GPIO_DATA=16
        NES_GPIO_LAT=15

        #AUDIO
        AUDIO_PWM_PIN=26

        # Other
        FF_USE_FIND
)
pico_add_extra_outputs(sms)



