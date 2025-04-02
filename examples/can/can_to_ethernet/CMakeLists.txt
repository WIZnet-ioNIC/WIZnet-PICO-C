set(TARGET_TCPS_NAME wizchip_can_to_eth_tcps)
set(TARGET_TCPC_NAME wizchip_can_to_eth_tcpc)

add_executable(${TARGET_TCPS_NAME}
        ${TARGET_TCPS_NAME}.c
)
add_executable(${TARGET_TCPC_NAME}
        ${TARGET_TCPC_NAME}.c
)

target_sources(${TARGET_TCPS_NAME} PRIVATE
    ./../can_parser.c
)
target_sources(${TARGET_TCPC_NAME} PRIVATE
    ./../can_parser.c
)

target_include_directories(${TARGET_TCPS_NAME} PRIVATE
        ./../
        )
target_include_directories(${TARGET_TCPC_NAME} PRIVATE
        ./../
        )

target_compile_options(${TARGET_TCPS_NAME} PRIVATE -O2)
target_compile_options(${TARGET_TCPC_NAME} PRIVATE -O2)

target_link_libraries(${TARGET_TCPS_NAME} PRIVATE
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
target_link_libraries(${TARGET_TCPC_NAME} PRIVATE
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

pico_enable_stdio_usb(${TARGET_TCPS_NAME} 1)
pico_enable_stdio_uart(${TARGET_TCPS_NAME} 0)
pico_enable_stdio_usb(${TARGET_TCPC_NAME} 1)
pico_enable_stdio_uart(${TARGET_TCPC_NAME} 0)

pico_add_extra_outputs(${TARGET_TCPS_NAME})
pico_add_extra_outputs(${TARGET_TCPC_NAME})