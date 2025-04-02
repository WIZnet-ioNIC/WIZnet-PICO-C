# How to Test CAN TO ETHERNET Example

Reference CAN driver code is based on [**CAN2040**][link-can_driver].

Please install the tools required for testing through the following link [**CAN EXAMPLE README**][link-can_example_readme].


## Step 1: Prepare software

The following serial terminal programs are required for CAN TO ETHERNET example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]
- [**Klipper**][link-klipper]


## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.

4. Connect as shown in the diagram below, to test the CAN example.

![][link-hardware_wiring]


## Step 3: Setup CAN TO ETHERNET Example

To test the CAN TO ETHERNET example, minor settings shall be done in code.

1. Setup SPI port and pin in 'w5x00_spi.h' in 'WIZnet-PICO-C/port/ioLibrary_Driver/' directory.

Setup the SPI interface you use.
- If you use the W5100S-EVB-Pico, W5500-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2,

```cpp
/* SPI */
#define SPI_PORT spi0

#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_RST 20
```

If you want to test with the CAN TO ETHERNET example using SPI DMA, uncomment USE_SPI_DMA.

```cpp
/* Use SPI DMA */
//#define USE_SPI_DMA // if you want to use SPI DMA, uncomment.
```
- If you use the W55RP20-EVB-Pico,
```cpp
/* SPI */
#define USE_SPI_PIO

#define PIN_SCK 21
#define PIN_MOSI 23
#define PIN_MISO 22
#define PIN_CS 20
#define PIN_RST 25
```

2. Setup network configuration such as IP in 'w5x00_can_to_eth_tcpc.c' which is the Loopback example in 'WIZnet-PICO-C/examples/can/can_to_ethernet/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
#if _WIZCHIP_ > W5500
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
#else
        .dhcp = NETINFO_STATIC        
#endif
};
```

3. Setup ethernet configuration in 'w5x00_can_to_eth_tcpc.c' in 'WIZnet-PICO-C/examples/can/can_to_ethernet/' directory.

```cpp
/* Socket */
#define SOCKET_TCP 0

/* Port */
#define PORT_TCP 5000
```

4. Setup CAN Configuration and pin in 'w5x00_can_to_eth_tcpc.c' in 'WIZnet-PICO-C/examples/can/can_to_ethernet/' directory.

```cpp
/* Can */
#define CAN_PIO_INDEX       0           // !! do not use PIO1 (using in w5x00) !!
#define CAN_BITRATE         100000      // 100kbps
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5
```



## Step 4: Build

1. After completing the CAN TO ETHERNET example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'w5x00_can_to_eth_tcpc.uf2' is generated in 'WIZnet-PICO-C/build/examples/can/can_to_ethernet/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'w5x00_can_to_eth_tcpc.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the CAN TO ETHERNET example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can see the network information of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 and the TCP server is open.

![][link-see_network_information_of_raspberry_pi_pico_and_open_tcp_server]

6. Connect to the open TCP server using Hercules TCP client. When connecting to the TCP server, you need to enter is the IP that was configured in Step 3, the port is 5000 by default.

![][link-connect_to_tcp_server_using_hercules_tcp_client]

7. When the TCP client sends a message, you can verify that the board receives it and transmits the message via CAN.

![][link-see_send_message_by_hercules_tcp_client]
![][link-see_recv_message_by_can_utils]

8. When the CAN Utils sends a message, you can verify that the board receives it and transmits the message via ETH.

![][link-see_send_message_by_can_utils]
![][link-see_recv_message_by_hercules_tcp_client]


<!--
Link
-->

[link-can_driver]: https://github.com/KevinOConnor/can2040
[link-can_example_readme]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/examples/can/README.md
[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-klipper]: https://www.klipper3d.org/
[link-hardware_wiring]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/hardware_wiring.png
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_and_open_tcp_server]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/see_network_information_of_raspberry_pi_pico_and_open_tcp_server.png
[link-connect_to_tcp_server_using_hercules_tcp_client]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/connect_to_tcp_server_using_hercules_tcp_client.png
[link-see_send_message_by_hercules_tcp_client]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/see_send_message_by_hercules_tcp_client.png
[link-see_recv_message_by_can_utils]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/see_recv_message_by_can_utils.png
[link-see_send_message_by_can_utils]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/see_send_message_by_can_utils.png
[link-see_recv_message_by_hercules_tcp_client]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_to_ethernet/see_recv_message_by_hercules_tcp_client.png
