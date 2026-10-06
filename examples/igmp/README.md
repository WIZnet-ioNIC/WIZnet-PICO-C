# How to Test IGMP Example

## Overview

This example opens a multicast UDP socket on the W5500 core and joins the IGMP group `239.1.1.1`.

When the multicast socket is opened, the W5500 handles IGMP by itself. If an IGMP Membership Query arrives, the W5500 automatically sends an IGMP Membership Report for the joined group. The application does not need to parse IGMP Query packets or build IGMP Report packets.

> **Note**
>
> This example has been tested only on the **W55RP20-EVB-PICO** board.
>
> The source code allows W5500 core chips only (`_WIZCHIP_ == W5500`), so it does not build for W5100S, W6100 or W6300 boards.

## Step 1: Prepare software

The following programs are required for the IGMP example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Wireshark**][link-wireshark]
- [**Python 3**][link-python]
- [**Npcap**][link-npcap] (installed together with Wireshark on Windows)
- [**Scapy**][link-scapy]

Install Scapy with the following command.

```powershell
pip install scapy
```

The test was performed with Scapy 2.8.0.

## Step 2: Prepare hardware

1. Connect ethernet cable to the W55RP20-EVB-PICO ethernet port.

2. Connect the other end of the ethernet cable to the PC, directly or through a switch, so that the PC and the board are on the same network.

3. Connect the W55RP20-EVB-PICO to the PC using USB cable.

The test environment is as follows.

```text
PC                                    W55RP20-EVB-PICO
IP: 192.168.11.100                    IP: 192.168.11.12
Python + Scapy        Ethernet        Subnet: 255.255.255.0
Wireshark       <---------------->    Gateway: 192.168.11.1
                                      Multicast Group: 239.1.1.1
                                      UDP Port: 5000
```

| Item | Value |
| --- | --- |
| PC IP | `192.168.11.100` |
| W55RP20 IP | `192.168.11.12` |
| Subnet Mask | `255.255.255.0` |
| Gateway | `192.168.11.1` |
| Multicast Group | `239.1.1.1` |
| Multicast MAC | `01:00:5E:01:01:01` |
| Group Port / Local Port | `5000` / `5000` |
| IGMP Version | IGMPv2 |

## Step 3: Setup IGMP Example

To test the IGMP example, minor settings shall be done in code.

1. Setup SPI port and pin in 'wizchip_spi.h' in 'WIZnet-PICO-C/port/ioLibrary_Driver/' directory.

For the **W55RP20-EVB-PICO**, enable `USE_PIO` and configure as follows:

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

2. Setup network configuration such as IP in 'wizchip_igmp.c' which is the IGMP example in 'WIZnet-PICO-C/examples/igmp/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
/* Network */
static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 11, 12},                    // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC
};
```

3. Setup the multicast group and port in 'wizchip_igmp.c'.

```cpp
/* Port */
#define PORT_IGMP_LOCAL 5000
#define PORT_IGMP_GROUP 5000

/* Multicast group (avoid 224.0.0.x, link-local groups are not reported by IGMP) */
static uint8_t g_igmp_group_ip[4] = {239, 1, 1, 1};
```

Do not use the `224.0.0.x` range for the group. It is reserved for link-local control, and hosts do not send IGMP reports for it.

The multicast MAC address is calculated from the group IP automatically (`01:00:5E` + lower 23 bits of the group IP). For `239.1.1.1` it is `01:00:5E:01:01:01`.

4. Select the IGMP version (optional).

IGMPv2 is used by default. To use IGMPv1, uncomment `USE_IGMP_V1`. This sets the `MC` bit (bit 5) of `Sn_MR`.

```cpp
// #define USE_IGMP_V1
```

> On the W5500, `MC = 0` selects IGMPv2 and `MC = 1` selects IGMPv1. ioLibrary names this bit `SF_IGMP_VER2`, but setting it actually selects IGMPv1.

## Step 4: Build

1. After completing the IGMP example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'wizchip_igmp.uf2' is generated in 'WIZnet-PICO-C/build/examples/igmp/' directory.

## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the W55RP20-EVB-PICO power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'wizchip_igmp.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of the board with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the IGMP example works normally, you can see the network information and the multicast socket information.

![][link-see_network_information_of_raspberry_pi_pico_and_open_igmp_socket]

## Step 6: Send IGMP Query from PC

1. Start Wireshark capture on the ethernet interface connected to the board and set the display filter to `igmp`.

2. Run PowerShell as administrator and start Scapy.

```powershell
scapy
```

![][link-scapy_start]

> The `Can't import PyX` message is related to PDF/PostScript output of Scapy and does not affect this test.

3. Check the name of the ethernet interface connected to the board.

```python
show_interfaces()
```

![][link-scapy_show_interfaces]

4. Send an IGMPv2 General Membership Query. Change `iface` to the interface name found above.

```python
from scapy.all import *
from scapy.layers.igmp import IGMP_MQ

pkt = (
    Ether(
        dst="01:00:5e:00:00:01"
    )
    /
    IP(
        src="192.168.11.100",
        dst="224.0.0.1",
        ttl=1
    )
    /
    IGMP_MQ(
        gaddr="0.0.0.0"
    )
)

pkt.show()

sendp(pkt, iface="Realtek PCIe GbE Family Controller")
```

`224.0.0.1` is the IPv4 All Hosts multicast address. The query was also tested with `dst="239.1.1.1"` in the IP header.

5. Check in Wireshark that the W5500 sends an IGMPv2 Membership Report for `239.1.1.1` right after the query.

## Test Result

The test was performed on the **W55RP20-EVB-PICO** board. The captured packets are in the 'test' directory.

| File | Query IP destination |
| --- | --- |
| [igmp_general_query_report.pcapng](test/igmp_general_query_report.pcapng) | `224.0.0.1` |
| [igmp_group_specific_query_report.pcapng](test/igmp_group_specific_query_report.pcapng) | `239.1.1.1` |

In both cases, the W5500 answered the General Membership Query with a Membership Report for `239.1.1.1`.

**Query IP destination `224.0.0.1` (igmp_general_query_report.pcapng)**

![][link-packet_capture_with_igmp_general_query]

| No. | Time | Source | Destination | Info |
| --- | --- | --- | --- | --- |
| 308 | 23.681300 | 192.168.11.100 | 224.0.0.1 | Membership Query, general |
| 309 | 23.681593 | 192.168.11.12 | 239.1.1.1 | Membership Report group 239.1.1.1 |

**Query IP destination `239.1.1.1` (igmp_group_specific_query_report.pcapng)**

![][link-packet_capture_with_igmp_group_specific_query]

| No. | Time | Source | Destination | Info |
| --- | --- | --- | --- | --- |
| 416 | 52.881139 | 192.168.11.100 | 239.1.1.1 | Membership Query, general |
| 417 | 52.881474 | 192.168.11.12 | 239.1.1.1 | Membership Report group 239.1.1.1 |

The Membership Report was sent about 293 us and 335 us after the query respectively.

Details of the Membership Report sent by the W5500:

| Item | Value |
| --- | --- |
| Ethernet Source MAC | `00:08:DC:12:34:56` |
| Ethernet Destination MAC | `01:00:5E:01:01:01` |
| Source IP | `192.168.11.12` |
| Destination IP | `239.1.1.1` |
| IP Header Length | `24 bytes` |
| TTL | `1` |
| IP Protocol | `IGMP (2)` |
| IP Option | `Router Alert` |
| IGMP Version | `2` |
| IGMP Type | `Membership Report (0x16)` |
| Max Resp Time | `0.0 sec (0x00)` |
| IGMP Checksum | `0xf9fc [correct]` |
| Multicast Address | `239.1.1.1` |


<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-wireshark]: https://www.wireshark.org/download.html
[link-python]: https://www.python.org/downloads/
[link-npcap]: https://npcap.com/
[link-scapy]: https://scapy.net/
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_and_open_igmp_socket]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/see_network_information_of_raspberry_pi_pico_and_open_igmp_socket.png
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/raspberry_pi_pico_usb_mass_storage.png
[link-scapy_show_interfaces]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/scapy_show_interfaces.png
[link-scapy_start]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/scapy_start.png
[link-packet_capture_with_igmp_general_query]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/packet_capture_with_igmp_general_query.png
[link-packet_capture_with_igmp_group_specific_query]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/igmp/packet_capture_with_igmp_group_specific_query.png