add_library(i2ckbd INTERFACE)

target_sources(i2ckbd INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/util_i2c_kbd.c
)

target_link_libraries(i2ckbd INTERFACE hardware_pio hardware_dma)

target_include_directories(i2ckbd INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
