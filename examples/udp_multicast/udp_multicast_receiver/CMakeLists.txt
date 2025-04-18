set(TARGET_NAME wizchip_udp_multicast_receiver)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
)

# pull in common dependencies and additional spi hardware support
target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        ETHERNET_FILES
        IOLIBRARY_FILES
        LOOPBACK_FILES
        MULTI_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${TARGET_NAME})
