# How to Test TFTP client Example



## Step 1: Prepare software

The following serial terminal program is required for TFTP client example test, download and install from below link.

- [**Tera Term**][link-tera_term]



## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.



## Step 3: Setup TFTP client Example

To test the TFTP client example, minor settings shall be done in code.

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

If you want to test with the TFTP client example using SPI DMA, uncomment USE_SPI_DMA.

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

2. Setup network configuration such as IP in 'w5x00_tftp_client.c' which is the TFTP client example in 'WIZnet-PICO-C/examples/tftp/' directory.

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

3. Setup TFTP client configuration in 'w5x00_tftp_client.c' in 'WIZnet-PICO-C/examples/tftp/' directory.

Set the IP address of the TFTP server to connect to and the name of the file to be read.

```cpp
#define TFTP_SERVER_IP "192.168.11.2"
#define TFTP_SERVER_FILE_NAME "tftp_test_file.txt"
```



## Step 4: Build

1. After completing the TFTP client example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'w5x00_tftp_client.uf2' is generated in 'WIZnet-PICO-C/build/examples/sntp/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'w5x00_tftp_client.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. The TFTP server used in this example is located [here][link-solarwinds_FTFP_Server].

6. You need to go to the file tab in the top left corner of the server and configure the settings. Set the root directory in the Storage section to define the path where the example client will read the file from.

![][link-configure_TFTP_Server]

7. Then, create a text file with the same name as the TFTP_SERVER_FILE_NAME set in the code in the configured path.

![][link-configure_TFTP_Server]

8. If the TFTP client example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, You can view the network information, send a request to the TFTP server, and successfully read the file.

![][link-tftp_client_read_sucess]

9. If the file reading fails, the following screen will appear.

![][link-tftp_client_read_fail]

10. This is a screenshot capturing the client's IP using Wireshark.

![][link-tftp_client_wireshark_result]











<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/connect_to_serial_com_port.png
[link-solarwinds_FTFP_Server]: https://www.solarwinds.com/free-tools/free-tftp-server
[link-configure_TFTP_Server]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/configure_TFTP_Server.png
[link-create_test_file]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/create_test_file.png
[link-tftp_client_read_sucess]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/tftp_client_read_sucess.png
[link-tftp_client_read_fail]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/tftp_client_read_fail.png
[link-tftp_client_wireshark_result]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftfp/tftp_client_wireshark_result.png

