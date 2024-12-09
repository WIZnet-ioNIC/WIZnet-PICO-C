# How to Test UDP multicast Example





## Step 1: Prepare software

The following serial terminal programs are required for Loopback example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]



## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.



## Step 3: Setup UDP multicast Example

To test the UDP multicast example, minor settings shall be done in code.

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

If you want to test with the UDP multicast example using SPI DMA, uncomment USE_SPI_DMA.

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


2. Setup network configuration such as IP in 'udp_multicast_receiver.c' which is the UDP multicast example in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
/* Network */
static wiz_NetInfo net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};
```

3. Setup multicast configuration in 'udp_multicast_receiver.c' in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.

```cpp
static uint8_t multicast_ip[4] = {224, 0, 0, 5};    // multicast ip address
static uint16_t multicast_port = 30000;             // multicast port
```

## Step 4: Build

1. After completing the UDP multicast example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'udp_multicast_receiver.uf2' is generated in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

2. Drag and drop 'udp_multicast_receiver.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. Configure the UDP multicast test using the Hercules program. Open UDP with the previously configured multicast address and port, and send a message.

![][link-test_using_the_Hercules]

6. If the UDP multicast example works correctly on the Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2, or W5500-EVB-Pico2, you will be able to check the network information of the respective board and see the messages sent by the source with membership to the multicast address.

![][link-connect_to_UDP_multicast_receiver]

7. Devices subscribed to the membership of the multicast address can receive the same message simultaneously, as shown.

![][link-multicast_receive_message]

8. This is a screenshot captured using Wireshark, showing a multicast address.

![][link-captured_using_Wireshark]

<!--
Link
-->

[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast/udp_multicast_receiver/connect_to_serial_com_port.png
[link-test_using_the_Hercules]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast/udp_multicast_receiver/test_using_the_Hercules.png
[link-connect_to_UDP_multicast_receiver]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast/udp_multicast_receiver/connect_to_UDP_multicast_receiver.png
[link-multicast_receive_message]: https://github.com/WIZnet-PICO-C/static/images/udp_multicast/udp_multicast_receiver/multicast_receive_message.PNG
[link-captured_using_Wireshark]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static//images/udp_multicast/udp_multicast_receiver/captured_using_Wireshark.png

