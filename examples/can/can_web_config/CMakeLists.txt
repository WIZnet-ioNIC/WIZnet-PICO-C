set(TARGET_NAME w5x00_can_web_config)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
)

target_sources(${TARGET_NAME} PRIVATE
    ./../can_parser.c
    ./src/can_to_eth.c
    ./src/canHandler.c
    ./src/httpParser.c
    ./src/httpServer.c
    ./src/httpUtil.c
    ./src/httpHandler.c
)

target_include_directories(${TARGET_NAME} PRIVATE
        ./../
        ./inc/
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
        BOARD_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})