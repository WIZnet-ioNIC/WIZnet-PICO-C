#include <stdio.h>
#include <string.h>

#include "port_common.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "socket.h"

#include "can.h"

#include "w5x00_can_config.h"

#include "can_to_eth.h"
#include "canHandler.h"

// #define _CAN_TO_ETH_DEBUG_

static uint16_t server_port = 4001;

static uint8_t destip[4] = {192, 168, 11, 3};
static uint16_t destport = 4001;

extern uint8_t g_tx_buf[DATA_BUF_MAX_SIZE];
extern uint8_t g_rx_buf[DATA_BUF_MAX_SIZE];

extern volatile CAN_RX_RBUF can_rx_rbuf;

static int32_t process_tcp_client(uint8_t sn, uint8_t* destip, uint16_t destport) {
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    uint32_t tx_buf_idx = 0;
    struct can2040_msg recv_msg;
    static uint16_t any_port = 	50000;

    switch (getSn_SR(sn)) {
    case SOCK_ESTABLISHED :
        if (getSn_IR(sn) & Sn_IR_CON) {	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
#ifdef _CAN_TO_ETH_DEBUG_
            printf("%d:Connected to - %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
            setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
        }

        if ((size = getSn_RX_RSR(sn)) > 0) { // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
            if (size > DATA_BUF_MAX_SIZE) {
                size = DATA_BUF_MAX_SIZE;    // DATA_BUF_SIZE means user defined buffer size (array)
            }
            ret = recv(sn, g_rx_buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)

            if (ret <= 0) {
                return ret;    // If the received data length <= 0, receive failed and process end
            }

            ret = send_can_msg(g_rx_buf);

            if (!ret) {
                return ret;
            }

            memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
            if (ret == CAN_ERR_FORMAT) {
                sprintf(g_tx_buf, "ERR) CAN FORMAT is wrong...\r\n");
            } else if (ret == CAN_ERR_ID_OVERFLOW) {
                sprintf(g_tx_buf, "ERR) CAN ID must be lower than 0x800...\r\n");
            }

            ret = send(sn, g_tx_buf, strlen(g_tx_buf));
            if (ret < 0) {
                close(sn);
                return ret;
            }
        }

        do {

            ret = pop_rbuf(&recv_msg);

            if (ret < 0) {
                break;
            }

            memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
            tx_buf_idx = sprintf(&g_tx_buf[0], "%0X,", recv_msg.id);
            tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " [%u]", recv_msg.dlc);
            for (uint32_t i = 0; i < recv_msg.dlc; i++) {
                tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " %0X", recv_msg.data[i]);
            }

            g_tx_buf[tx_buf_idx] = '\n';

            ret = send(sn, g_tx_buf, strlen(g_tx_buf));
            if (ret < 0) {
                close(sn);
                return ret;
            }
        } while (1);

        break;

    case SOCK_CLOSE_WAIT :
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:CloseWait\r\n", sn);
#endif
        if ((ret = disconnect(sn)) != SOCK_OK) {
            return ret;
        }
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:Socket Closed\r\n", sn);
#endif
        break;

    case SOCK_INIT :
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:Try to connect to the %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
        if ((ret = connect(sn, destip, destport)) != SOCK_OK) {
            return ret;    //	Try to TCP connect to the TCP server (destination)
        }
        can_rx_rbuf.push_idx = 0;
        can_rx_rbuf.pop_idx = 0;
        memset(can_rx_rbuf.msg_buf, 0x0, DATA_BUF_MAX_SIZE * sizeof(struct can2040_msg));
        can_rx_rbuf.rx_flag = true;
        break;

    case SOCK_CLOSED:
        close(sn);
        if ((ret = socket(sn, Sn_MR_TCP, any_port++, 0x00)) != sn) {
            if (any_port == 0xffff) {
                any_port = 50000;
            }
            return ret; // TCP socket open with 'any_port' port number
        }
#ifdef _CAN_TO_ETH_DEBUG_
        //printf("%d:TCP client loopback start\r\n",sn);
        //printf("%d:Socket opened\r\n",sn);
#endif
        break;
    default:
        break;
    }
    return 1;
}

static int32_t process_tcp_server(uint8_t sn, uint16_t port) {
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    uint32_t tx_buf_idx = 0;
    struct can2040_msg recv_msg;

#ifdef _CAN_TO_ETH_DEBUG_
    uint8_t destip[4];
    uint16_t destport;
#endif

    switch (getSn_SR(sn)) {
    case SOCK_ESTABLISHED :
        if (getSn_IR(sn) & Sn_IR_CON) {
#ifdef _CAN_TO_ETH_DEBUG_
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);

            printf("%d:Connected - %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
            setSn_IR(sn, Sn_IR_CON);
        }
        if ((size = getSn_RX_RSR(sn)) > 0) { // Don't need to check SOCKERR_BUSY because it doesn't not occur.
            if (size > DATA_BUF_MAX_SIZE) {
                size = DATA_BUF_MAX_SIZE;
            }
            ret = recv(sn, g_rx_buf, size);

            if (ret <= 0) {
                return ret;    // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            }

            ret = send_can_msg(g_rx_buf);

            if (!ret) {
                return ret;
            }

            memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
            if (ret == CAN_ERR_FORMAT) {
                sprintf(g_tx_buf, "ERR) CAN FORMAT is wrong...\r\n");
            } else if (ret == CAN_ERR_ID_OVERFLOW) {
                sprintf(g_tx_buf, "ERR) CAN ID must be lower than 0x800...\r\n");
            }

            ret = send(sn, g_tx_buf, strlen(g_tx_buf));
            if (ret < 0) {
                close(sn);
                return ret;
            }
        }

        do {

            ret = pop_rbuf(&recv_msg);

            if (ret < 0) {
                break;
            }

            memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
            tx_buf_idx = sprintf(&g_tx_buf[0], "%0X,", recv_msg.id);
            tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " [%u]", recv_msg.dlc);
            for (uint32_t i = 0; i < recv_msg.dlc; i++) {
                tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " %0X", recv_msg.data[i]);
            }

            g_tx_buf[tx_buf_idx] = '\n';

            ret = send(sn, g_tx_buf, strlen(g_tx_buf));
            if (ret < 0) {
                close(sn);
                return ret;
            }
        } while (1);

        break;
    case SOCK_CLOSE_WAIT :
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:CloseWait\r\n", sn);
#endif
        if ((ret = disconnect(sn)) != SOCK_OK) {
            return ret;
        }
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:Socket Closed\r\n", sn);
#endif
        break;
    case SOCK_INIT :
#ifdef _CAN_TO_ETH_DEBUG_
        printf("%d:Listen, TCP server loopback, port [%d]\r\n", sn, port);
#endif
        if ((ret = listen(sn)) != SOCK_OK) {
            return ret;
        }
        can_rx_rbuf.push_idx = 0;
        can_rx_rbuf.pop_idx = 0;
        memset(can_rx_rbuf.msg_buf, 0x0, DATA_BUF_MAX_SIZE * sizeof(struct can2040_msg));
        can_rx_rbuf.rx_flag = true;
        break;
    case SOCK_CLOSED:
#ifdef _CAN_TO_ETH_DEBUG_
        //printf("%d:TCP server loopback start\r\n",sn);
#endif
        if ((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn) {
            return ret;
        }
#ifdef _CAN_TO_ETH_DEBUG_
        //printf("%d:Socket opened\r\n",sn);
#endif
        break;
    default:
        break;
    }
    return 1;
}

int32_t can_to_eth(ETH_MODE mode) {
    switch (mode) {
    case ETH_TCP_SERVER:
        process_tcp_server(SOCKET_TCP, server_port);
        break;
    case ETH_TCP_CLIENT:
        process_tcp_client(SOCKET_TCP, destip, destport);
        break;
    default:
        break;
    }
    return 0;
}

int32_t reset_eth() {
    int ret = 0;
    uint8_t sock_status = getSn_SR(SOCKET_TCP);

    if ((sock_status == SOCK_ESTABLISHED) || (sock_status == SOCK_CLOSE_WAIT)) {
        if ((ret = disconnect(SOCKET_TCP)) != SOCK_OK) {
            return ret;
        }
    }

    if ((ret = close(SOCKET_TCP)) != SOCK_OK) {
        return ret;
    }

    return 1;
}
