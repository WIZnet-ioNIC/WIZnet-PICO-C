# How to Test UDP multicast receiver Example





## Step 1: Prepare software

The following serial terminal programs are required for UDP multicast receiver example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 



## Step 3: Setup UDP multicast receiver Example

To test the UDP multicast receiver example, minor settings shall be done in code.

1. Setup SPI port and pin in 'wizchip_spi.h' in 'WIZnet-PICO-C/port/ioLibrary_Driver/' directory.

Setup the SPI interface you use.

### For **W55RP20-EVB-PICO**:
If you are using the **W55RP20-EVB-PICO**, enable `USE_PIO` and configure as follows:

```cpp
#if (DEVICE_BOARD_NAME == W55RP20_EVB_PICO)

#define USE_PIO

#define PIN_SCK   21
#define PIN_MOSI  23
#define PIN_MISO  22
#define PIN_CS    20
#define PIN_RST   25
#define PIN_IRQ   24

```

---

### For **W6300-EVB-PICO** or **W6300-EVB-PICO2**:
If you are using the **W6300-EVB-PICO** or **W6300-EVB-PICO2**, use the following pinout and SPI clock divider configuration:

```cpp
#elif (DEVICE_BOARD_NAME == W6300_EVB_PICO || DEVICE_BOARD_NAME == W6300_EVB_PICO2)
#define USE_PIO

#define PIO_IRQ_PIN             15
#define PIO_SPI_SCK_PIN         17
#define PIO_SPI_DATA_IO0_PIN    18
#define PIO_SPI_DATA_IO1_PIN    19
#define PIO_SPI_DATA_IO2_PIN    20
#define PIO_SPI_DATA_IO3_PIN    21
#define PIN_CS                  16
#define PIN_RST                 22


```

---

### For other generic SPI boards
If you are not using any of the above boards, you can fall back to a default SPI configuration:

```cpp
#else

#define SPI_PORT spi0

#define SPI_SCK_PIN  18
#define SPI_MOSI_PIN 19
#define SPI_MISO_PIN 16
#define SPI_CS_PIN   17
#define RST_PIN      20

#endif
```

Make sure you are **not defining `USE_PIO`** in your setup when using DMA:

```cpp
// #define USE_PIO
```


2. Setup network configuration such as IP in 'wizchip_udp_multicast_receiver.c' which is the  receiver example in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.

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

3. Setup multicast configuration in 'wizchip_udp_multicast_receiver.c' in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.

```cpp
static uint8_t multicast_ip[4] = {224, 0, 0, 5};    // multicast ip address
static uint16_t multicast_port = 30000;             // multicast port
```

## Step 4: Build

1. After completing the  receiver example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'wizchip_udp_multicast_receiver.uf2' is generated in 'WIZnet-PICO-C/examples/udp_multicast/udp_multicast_receiver/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'wizchip_udp_multicast_receiver.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. Configure the UDP multicast receiver test using the Hercules program. Open UDP with the previously configured multicast address and port, and send a message.

![][link-test_using_the_Hercules]

6. If the UDP multicast receiver example works correctly on the pico, you will be able to check the network information of the respective board and see the messages sent by the source with membership to the multicast address.

![][link-connect_to_UDP_multicast_receiver]

7. Devices subscribed to the membership of the multicast address can receive the same message simultaneously, as shown.

![][link-multicast_receive_message]

8. This is a screenshot captured using Wireshark, showing a multicast address.

![][link-captured_using_Wireshark]

<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/connect_to_serial_com_port.png
[link-test_using_the_Hercules]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/test_using_the_Hercules.PNG
[link-connect_to_UDP_multicast_receiver]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/connect_to_UDP_multicast_receiver.PNG
[link-multicast_receive_message]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/multicast_receive_message.PNG
[link-captured_using_Wireshark]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/udp_multicast_receiver/captured_using_Wireshark.PNG

