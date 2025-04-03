# How to Test CAN Web Config Example

Reference CAN driver code is based on [**CAN2040**][link-can_driver].

Please install the tools required for testing through the following link [**CAN EXAMPLE README**][link-can_example_readme].


## Step 1: Prepare software

The following serial terminal programs are required for CAN Web Config example test, download and install from below links.

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


## Step 3: Setup CAN Web Config Example

To test the CAN Web Config example, minor settings shall be done in code.

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

2. Setup network configuration such as IP in 'w5x00_can_web_config.c' which is the example in 'WIZnet-PICO-C/examples/can/can_web_config/' directory.

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
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};
```

## Step 4: Build

1. After completing the CAN TO ETHERNET example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'w5x00_can_web_config.uf2' is generated in 'WIZnet-PICO-C/build/examples/can/can_web_config/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'w5x00_can_web_config.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the CAN Web Config example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can see the network information of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 and CAN config data.

![][link-set_network_information_and_can_config]


6. Connect to the open HTTP server, you can see the output on the web page. When connecting to the HTTP server, you need to enter is the IP that was configured in Step 3.

![][link-connect_to_http_config_server]


7. Select the mode and baudrate to change the CAN settings and press the Set Settings button.

![][link-set_can_config]


8. If the settings have changed normally, you can see CAN config data in teraterm, and if you press the Get Settings button on the web server, it will appear as follows.

![][link-see_can_config]

![][link-get_can_config]



<!--
Link
-->

[link-can_driver]: https://github.com/KevinOConnor/can2040
[link-can_example_readme]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/examples/can/README.md
[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-klipper]: https://www.klipper3d.org/
[link-hardware_wiring]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/hardware_wiring.png
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/connect_to_serial_com_port.png
[link-set_network_information_and_can_config]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/set_network_information_and_can_config.png
[link-connect_to_http_config_server]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/connect_to_http_config_server.png
[link-set_can_config]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/set_can_config.png
[link-see_can_config]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/see_can_config.png
[link-get_can_config]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_web_config/get_can_config.png