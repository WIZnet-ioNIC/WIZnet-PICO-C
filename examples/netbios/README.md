# How to Test Netbios Example



## Step 1: Prepare software

The following serial terminal programs are required for Netbios example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]



## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.



## Step 3: Setup Netbios Example

To test the Netbios example, minor settings shall be done in code.

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

If you want to test with the Netbios example using SPI DMA, uncomment USE_SPI_DMA.

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

2. Setup network configuration such as IP in 'w5x00_netbios.c' which is the Loopback example in 'WIZnet-PICO-C/examples/netbios/' directory.

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

3. Setup netbios configuration in 'netbios.c' in 'WIZnet-PICO-C/examples/netbios/' directory.

```cpp
#define NETBIOS_BOARD_NAME     "W55RP20"             /*Define the NetBIOS name*/
#define NETBIOS_PORT           137                  /*The default port for the NetBIOS name service*/
```



## Step 4: Build

1. After completing the Loopback example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'w5x00_netbios.uf2' is generated in 'WIZnet-PICO-C/build/examples/netbios/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'w5x00_netbios.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the Netbios example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can see the network information of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 and the udp is open.

![][link-see_network_information_of_raspberry_pi_pico_and_open_udp]

6. Send search netbios command in PC.

```cpp
# Linux
nmblookup -U 192.168.11.2 W55RP20
```

7. If search command's name and name on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 is correct, you can see the receive name on terminal.

![][link-send_netbios_command]

![][link-receive_back_netbios_com_port]

8. You can see NBNS packet in wireshark log.

![][link-wireshark_log]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/loopback/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/loopback/connect_to_serial_com_port.png

[link-see_network_information_of_raspberry_pi_pico_and_open_udp]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/netbios/connect_to_serial_com_port.png

[link-send_netbios_command]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/netbios/send_netbios_command.png
[link-receive_back_netbios_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/netbios/receive_back_netbios_com_port.png
[link-wireshark_log]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/netbios/wireshark_log.png