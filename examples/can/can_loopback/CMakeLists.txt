set(TARGET_NAME w5x00_can_loopback)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        )

target_compile_options(${TARGET_NAME} PRIVATE -O2)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        cmsis_core
        hardware_spi
        hardware_dma
        hardware_irq
        ETHERNET_FILES
        IOLIBRARY_FILES
        LOOPBACK_FILES 
        BOARD_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})