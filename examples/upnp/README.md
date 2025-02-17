# How to Test uPnP Example



## Step 1: Prepare software

The following serial terminal programs are required for uPnP example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]



## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.



## Step 3: Setup uPnP Example

To test the uPnP example, minor settings shall be done in code.

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

If you want to test with the uPnP example using SPI DMA, uncomment USE_SPI_DMA.

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

2. Setup network configuration such as IP in 'w5x00_upnp.c' which is the uPnP example in 'WIZnet-PICO-C/examples/upnp/' directory.

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

3. Setup upnp configuration in 'w5x00_upnp.c' in 'WIZnet-PICO-C/examples/upnp/' directory.

```cpp
/* Socket */
#define SOCKET_UPNP 0

/* Port */
#define PORT_TCP 8000
#define PORT_UDP 5000
```



## Step 4: Build

1. After completing the uPnP example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'w5x00_upnp.uf2' is generated in 'WIZnet-PICO-C/build/examples/upnp/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'w5x00_upnp.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the uPnP example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can see the network information of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 and main menu tree.

![][link-see_network_information_of_raspberry_pi_pico_and_see_main_menutree]

6. Choose Set LED on or Set LED off and you can see user led state on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2.

![][link-set_led_on_off]

For example, you can see user led state on W55RP20-EVB-Pico.

![][link-see_user_led_of_W55RP20_EVB_Pico]

7. Choose set network setting and you can see the network information of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2.

![][link-set_network_setting]

![][link-show_network_setting]

8. Choose set run tcp loopback and connect to the open loopback server using Hercules TCP client. When connecting to the loopback server, you need to enter is the IP that was configured in Step 7, the port is 8000 by default.

![][link-set_run_tcp_loopback]

9. Choose set run udp loopback and connect to the open loopback server using Hercules UDP client. When connecting to the loopback server, you need to enter is the IP that was configured in Step 7, the port is 5000 by default.

![][link-set_run_udp_loopback]

10. Choose set tcp portforwarding add port and you can see the tcp port forwarding settings on router's configuration screen.

![][link-set_tcp_portforwarding_add_port]

![][link-iptime_tcp_porrforwarding_add]

11. Choose set udp portforwarding add port and you can see the udp port forwarding settings on router's configuration screen.

![][link-set_udp_portforwarding_add_port]

![][link-iptime_udp_portforwarding_add]

12. Choose set tcp portforwarding delete port and you can see the tcp port forwarding settings on router's configuration screen.

![][link-set_tcp_portforwarding_delete_port]

![][link-iptime_tcp_portforwarding_delete]

13. Choose set udp portforwarding delete port and you can see the udp port forwarding settings on router's configuration screen.

![][link-set_udp_portforwarding_delete_port]

![][link-iptime_udp_portforwarding_delete]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_and_see_main_menutree]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/see_network_information_of_raspberry_pi_pico_and_see_main_menutree.png
[link-set_led_on_off]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_led_on_off.png
[link-see_user_led_of_W55RP20_EVB_Pico]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/see_user_led_of_W55RP20_EVB_Pico.png
[link-set_network_setting]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_network_setting.png
[link-show_network_setting]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/show_network_setting.png
[link-set_run_tcp_loopback]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_run_tcp_loopback.png
[link-set_run_udp_loopback]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_run_udp_loopback.png
[link-set_tcp_portforwarding_add_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_tcp_portforwarding_add_port.png
[link-iptime_tcp_porrforwarding_add]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/iptime_tcp_porrforwarding_add.png
[link-set_udp_portforwarding_add_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_udp_portforwarding_add_port.png
[link-iptime_udp_portforwarding_add]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/iptime_udp_portforwarding_add.png
[link-set_tcp_portforwarding_delete_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_tcp_portforwarding_delete_port.png
[link-iptime_tcp_portforwarding_delete]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/iptime_tcp_portforwarding_delete.png
[link-set_udp_portforwarding_delete_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/set_udp_portforwarding_delete_port.png
[link-iptime_udp_portforwarding_delete]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/upnp/iptime_udp_portforwarding_delete.png
