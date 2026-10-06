# IGMP 예제 테스트 방법

## 개요

이 예제는 W5500 코어에서 멀티캐스트 UDP 소켓을 열고 IGMP 그룹 `239.1.1.1`에 가입합니다.

멀티캐스트 소켓이 열리면 W5500이 IGMP를 자체적으로 처리합니다. IGMP Membership Query가 수신되면 W5500은 가입한 그룹에 대한 IGMP Membership Report를 자동으로 전송합니다. 애플리케이션에서 IGMP Query 패킷을 파싱하거나 IGMP Report 패킷을 생성할 필요가 없습니다.

> **참고**
>
> 이 예제는 **W55RP20-EVB-PICO** 보드에서만 테스트되었습니다.
>
> 소스 코드는 W5500 코어 칩(`_WIZCHIP_ == W5500`)만 허용하므로 W5100S, W6100 또는 W6300 보드에서는 빌드되지 않습니다.

## Step 1: 소프트웨어 준비

IGMP 예제를 테스트하려면 다음 프로그램이 필요합니다. 아래 링크에서 다운로드하여 설치합니다.

- [**Tera Term**][link-tera_term]
- [**Wireshark**][link-wireshark]
- [**Python 3**][link-python]
- [**Npcap**][link-npcap] (Windows에서는 Wireshark 설치 시 함께 설치됨)
- [**Scapy**][link-scapy]

다음 명령으로 Scapy를 설치합니다.

```powershell
pip install scapy
```

테스트는 Scapy 2.8.0 버전으로 수행했습니다.

## Step 2: 하드웨어 준비

1. W55RP20-EVB-PICO의 Ethernet 포트에 Ethernet 케이블을 연결합니다.

2. PC와 보드가 동일한 네트워크에 있도록 Ethernet 케이블의 반대쪽을 PC에 직접 연결하거나 스위치를 통해 연결합니다.

3. USB 케이블을 사용하여 W55RP20-EVB-PICO를 PC에 연결합니다.

테스트 환경은 다음과 같습니다.

```text
PC                                    W55RP20-EVB-PICO
IP: 192.168.11.100                    IP: 192.168.11.12
Python + Scapy        Ethernet        Subnet: 255.255.255.0
Wireshark       <---------------->    Gateway: 192.168.11.1
                                      Multicast Group: 239.1.1.1
                                      UDP Port: 5000
```

| 항목 | 값 |
| --- | --- |
| PC IP | `192.168.11.100` |
| W55RP20 IP | `192.168.11.12` |
| Subnet Mask | `255.255.255.0` |
| Gateway | `192.168.11.1` |
| Multicast Group | `239.1.1.1` |
| Multicast MAC | `01:00:5E:01:01:01` |
| Group Port / Local Port | `5000` / `5000` |
| IGMP Version | IGMPv2 |

## Step 3: IGMP 예제 설정

IGMP 예제를 테스트하려면 코드에서 몇 가지 설정이 필요합니다.

1. `WIZnet-PICO-C/port/ioLibrary_Driver/` 디렉터리의 `wizchip_spi.h`에서 SPI 포트와 핀을 설정합니다.

**W55RP20-EVB-PICO**의 경우 `USE_PIO`를 활성화하고 다음과 같이 설정합니다.

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

2. `WIZnet-PICO-C/examples/igmp/` 디렉터리의 IGMP 예제 파일인 `wizchip_igmp.c`에서 IP 등의 네트워크 설정을 구성합니다.

사용하는 네트워크 환경에 맞게 IP 및 기타 네트워크 설정을 변경합니다.

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

3. `wizchip_igmp.c`에서 멀티캐스트 그룹과 포트를 설정합니다.

```cpp
/* Port */
#define PORT_IGMP_LOCAL 5000
#define PORT_IGMP_GROUP 5000

/* Multicast group (avoid 224.0.0.x, link-local groups are not reported by IGMP) */
static uint8_t g_igmp_group_ip[4] = {239, 1, 1, 1};
```

그룹 주소로 `224.0.0.x` 범위를 사용하지 마십시오. 이 범위는 링크 로컬 제어용으로 예약되어 있으며 호스트는 해당 그룹에 대해 IGMP Report를 전송하지 않습니다.

멀티캐스트 MAC 주소는 그룹 IP에서 자동으로 계산됩니다(`01:00:5E` + 그룹 IP의 하위 23비트). `239.1.1.1`의 경우 `01:00:5E:01:01:01`입니다.

4. IGMP 버전을 선택합니다(선택 사항).

기본적으로 IGMPv2를 사용합니다. IGMPv1을 사용하려면 `USE_IGMP_V1`의 주석을 해제합니다. 이렇게 하면 `Sn_MR`의 `MC` 비트(bit 5)가 설정됩니다.

```cpp
// #define USE_IGMP_V1
```

> W5500에서는 `MC = 0`이면 IGMPv2, `MC = 1`이면 IGMPv1이 선택됩니다. ioLibrary에서는 이 비트의 이름을 `SF_IGMP_VER2`로 정의하고 있지만, 실제로 이 비트를 설정하면 IGMPv1이 선택됩니다.

## Step 4: 빌드

1. IGMP 예제 설정을 완료한 후 Visual Studio Code 하단 상태 표시줄에서 `build`를 클릭하거나 키보드의 `F7` 키를 눌러 빌드합니다.

2. 빌드가 완료되면 `WIZnet-PICO-C/build/examples/igmp/` 디렉터리에 `wizchip_igmp.uf2` 파일이 생성됩니다.

## Step 5: 업로드 및 실행

1. W55RP20-EVB-PICO의 BOOTSEL 버튼을 누른 상태에서 보드의 전원을 켜면 USB 대용량 저장장치 `RPI-RP2`가 자동으로 마운트됩니다.

![][link-raspberry_pi_pico_usb_mass_storage]

2. `wizchip_igmp.uf2` 파일을 USB 대용량 저장장치 `RPI-RP2`로 드래그 앤 드롭합니다.

3. Tera Term을 사용하여 보드의 Serial COM 포트에 연결합니다.

![][link-connect_to_serial_com_port]

4. 보드를 Reset합니다.

5. IGMP 예제가 정상적으로 동작하면 네트워크 정보와 멀티캐스트 소켓 정보를 확인할 수 있습니다.

![][link-see_network_information_of_raspberry_pi_pico_and_open_igmp_socket]

## Step 6: PC에서 IGMP Query 전송

1. 보드가 연결된 Ethernet 인터페이스에서 Wireshark 캡처를 시작하고 Display Filter를 `igmp`로 설정합니다.

2. PowerShell을 관리자 권한으로 실행하고 Scapy를 시작합니다.

```powershell
scapy
```

![][link-scapy_start]

> `Can't import PyX` 메시지는 Scapy의 PDF/PostScript 출력 기능과 관련된 메시지이며 이 테스트에는 영향을 주지 않습니다.

3. 보드가 연결된 Ethernet 인터페이스 이름을 확인합니다.

```python
show_interfaces()
```

![][link-scapy_show_interfaces]

4. IGMPv2 General Membership Query를 전송합니다. `iface`를 위에서 확인한 인터페이스 이름으로 변경합니다.

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

`224.0.0.1`은 IPv4 All Hosts 멀티캐스트 주소입니다. IP 헤더의 `dst="239.1.1.1"`로 설정한 경우에도 Query 전송 테스트를 수행했습니다.

5. Query가 전송된 직후 W5500이 `239.1.1.1`에 대한 IGMPv2 Membership Report를 전송하는지 Wireshark에서 확인합니다.

## 테스트 결과

테스트는 **W55RP20-EVB-PICO** 보드에서 수행했습니다. 캡처한 패킷은 `test` 디렉터리에 있습니다.

| 파일 | Query IP Destination |
| --- | --- |
| [igmp_general_query_report.pcapng](test/igmp_general_query_report.pcapng) | `224.0.0.1` |
| [igmp_group_specific_query_report.pcapng](test/igmp_group_specific_query_report.pcapng) | `239.1.1.1` |

두 경우 모두 W5500은 General Membership Query에 대해 `239.1.1.1` 그룹의 Membership Report로 응답했습니다.

**Query IP Destination `224.0.0.1` (igmp_general_query_report.pcapng)**

![][link-packet_capture_with_igmp_general_query]

| No. | Time | Source | Destination | Info |
| --- | --- | --- | --- | --- |
| 308 | 23.681300 | 192.168.11.100 | 224.0.0.1 | Membership Query, general |
| 309 | 23.681593 | 192.168.11.12 | 239.1.1.1 | Membership Report group 239.1.1.1 |

**Query IP Destination `239.1.1.1` (igmp_group_specific_query_report.pcapng)**

![][link-packet_capture_with_igmp_group_specific_query]

| No. | Time | Source | Destination | Info |
| --- | --- | --- | --- | --- |
| 416 | 52.881139 | 192.168.11.100 | 239.1.1.1 | Membership Query, general |
| 417 | 52.881474 | 192.168.11.12 | 239.1.1.1 | Membership Report group 239.1.1.1 |

Membership Report는 각각 Query 수신 후 약 293 us와 335 us 뒤에 전송되었습니다.

W5500이 전송한 Membership Report의 상세 정보는 다음과 같습니다.

| 항목 | 값 |
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