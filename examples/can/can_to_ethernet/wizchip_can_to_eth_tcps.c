/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>


#include "port_common.h"

#include "can_parser.h"
#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "wizchip_gpio_irq.h"

#include "socket.h"
#include "can.h" 
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "hardware/structs/pio.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_TCP 0

/* Port */
#define PORT_TCP 5000

/* Can */
#define CAN_PIO_INDEX       0           // do not use PIO1 (using in w5x00)
#define CAN_BITRATE         100000      // 125kbps
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5

#define CAN_ERR_FORMAT          -1
#define CAN_ERR_ID_OVERFLOW     -2

//#define CAN_DEBUG
//#define _CAN_TO_ETH_DEBUG_


/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
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

static uint8_t g_tx_buf[BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_rx_buf[BUF_MAX_SIZE] = {
    0,
};

static uint8_t destip[4] = {192, 168, 11, 3};
static uint16_t destport = 5000;

static struct can2040 cbus;

typedef struct _CAN_RX_RBUF
{
    bool        rx_flag;
    uint32_t    push_idx;
    uint32_t    pop_idx;
    struct can2040_msg msg_buf[BUF_MAX_SIZE];
} CAN_RX_RBUF;

static volatile CAN_RX_RBUF can_rx_rbuf; 
static critical_section_t can_cris_sec;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

static void push_rbuf(struct can2040_msg *msg);
static int pop_rbuf(struct can2040_msg *msg);

static int can_initialize();
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
static void PIOx_IRQHandler(void);
static int is_pio_in_use(uint32_t pio_num) ;

int32_t can_to_eth_process(uint8_t sn, uint8_t* destip, uint16_t destport);


/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int retval = 0;

    set_clock_khz();

    stdio_init_all();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    if(can_initialize() < 0)
    {
        printf("CAN setup failed...\n");
        while(1){}
    }

    can2040_start(&cbus, clock_get_hz(clk_sys), CAN_BITRATE, CAN_RX_PIN, CAN_TX_PIN);

    while(1)
    {
        if ((retval = can_to_eth_process(SOCKET_TCP, destip, destport)) < 0)
        {
            printf(" CAN to Eth loopback error : %d\n", retval);

            while (1)
                ;
        }
    }
}


/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

static void push_rbuf(struct can2040_msg *msg)
{
    critical_section_enter_blocking(&can_cris_sec);

    memcpy(&can_rx_rbuf.msg_buf[can_rx_rbuf.push_idx], msg, sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.push_idx < BUF_MAX_SIZE)
        can_rx_rbuf.push_idx++;
    else    
        can_rx_rbuf.push_idx = 0;

    critical_section_exit(&can_cris_sec);
}

static int pop_rbuf(struct can2040_msg *msg)
{
    if(can_rx_rbuf.pop_idx == can_rx_rbuf.push_idx)
        return -1;

    critical_section_enter_blocking(&can_cris_sec);
    memcpy(msg, &can_rx_rbuf.msg_buf[can_rx_rbuf.pop_idx], sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.pop_idx < BUF_MAX_SIZE)
        can_rx_rbuf.pop_idx++;
    else    
        can_rx_rbuf.pop_idx = 0;

    critical_section_exit(&can_cris_sec);
    return 0;
}

static int can_initialize()
{
    if (is_pio_in_use(CAN_PIO_INDEX)) {
        printf("Error: PIO block %u is already in use\n", CAN_PIO_INDEX);
        return -1;
    }

    can2040_setup(&cbus, CAN_PIO_INDEX);
    can2040_callback_config(&cbus, can2040_cb);

    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    //irq_set_priority(PIO0_IRQ_0, 1);
    irq_set_enabled(PIO0_IRQ_0, true);

    critical_section_init(&can_cris_sec);

    return 0;
}

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    if (notify == CAN2040_NOTIFY_RX) 
    {
        if(can_rx_rbuf.rx_flag == true)
            push_rbuf(msg);

#ifdef CAN_DEBUG        
        printf("Callback RX: ID=0x%08X DLC=%d Data=", msg->id, msg->dlc);
        for (int i = 0; i < msg->dlc; i++) {
            printf("0x%02X ", msg->data[i]);
        }
        printf("\n");
#endif
    }
    else if(notify == CAN2040_NOTIFY_TX) 
    {
#ifdef CAN_DEBUG      
        printf("Callback TX\n");
#endif
    }
    else {
#ifdef CAN_DEBUG  
        printf("Callback error...\n");
#endif
    }
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

static int is_pio_in_use(uint32_t pio_num) 
{
    pio_hw_t *pio_hw;

    if (pio_num == 0) {
        pio_hw = pio0_hw;
    } else if (pio_num == 1) {
        pio_hw = pio1_hw;
#ifdef PICO_RP2350
    } else if (pio_num == 2) {
        pio_hw = pio2_hw;
#endif
    } else {
        printf("Invalid PIO number: %u\n", pio_num);
        return -1;
    }

    for (int sm = 0; sm < 4; sm++) {
        if ((pio_hw->ctrl & (1 << (PIO_CTRL_SM_ENABLE_LSB + sm))) != 0) {
            return 1; 
        }
    }
    return 0;
}

int send_can_msg(char *msg_buf)
{
    struct can2040_msg tx_msg;
    char *p_id_token = NULL, *p_data_token = NULL;

    memset(&tx_msg, 0x0, sizeof(struct can2040_msg));
    p_id_token = NULL;
    p_data_token = NULL;

    p_id_token = custom_strtok(msg_buf, ",");

    if(p_id_token == NULL)
    {
        printf("string format error...\n\n");
        return CAN_ERR_FORMAT;
    }

    tx_msg.id = string_to_hex32(p_id_token);

    do {
        p_data_token = custom_strtok(NULL, " ");
        if(p_data_token == NULL) 
            break;

        if(tx_msg.dlc >= 8)
            break;

        tx_msg.data[tx_msg.dlc] = string_to_hex32(p_data_token);
        tx_msg.dlc++;
    } while(1);

    if( (p_data_token==NULL) && (tx_msg.dlc == 0) )
    {
        printf("string format error...\n\n");
        return CAN_ERR_FORMAT;
    } 

    if(tx_msg.id > 0x7FF)
    {
        printf("ID must be < 0x800\n");
        return CAN_ERR_ID_OVERFLOW;
    }

#ifdef CAN_DEBUG        
    printf("TX: ID=0x%X DLC=%d Data=", tx_msg.id, tx_msg.dlc);
    for (int i = 0; i < tx_msg.dlc; i++) {
        printf("0x%02X ", tx_msg.data[i]);
    }
    printf("\n");
#endif

    if (can2040_transmit(&cbus, &tx_msg) < 0) {
#ifdef CAN_DEBUG
        printf("Transmit queue full. Failed to send message.\n\n");
#endif
    } 

    return 0;
}

int32_t can_to_eth_process(uint8_t sn, uint8_t* destip, uint16_t destport)
{
   int32_t ret;
   uint16_t size = 0, sentsize=0;
   uint32_t tx_buf_idx = 0;
   struct can2040_msg recv_msg;

   static uint16_t any_port = 	50000;

   switch(getSn_SR(sn))
   {
        case SOCK_ESTABLISHED :
            if(getSn_IR(sn) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
            {
#ifdef _CAN_TO_ETH_DEBUG_
                printf("%d:Connected to - %d.%d.%d.%d : %d\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
                setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
            }

            if((size = getSn_RX_RSR(sn)) > 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
            {
                if(size > BUF_MAX_SIZE) size = BUF_MAX_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
                ret = recv(sn, g_rx_buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)

                if(ret <= 0) return ret; // If the received data length <= 0, receive failed and process end
                
                ret = send_can_msg(g_rx_buf);

                if(!ret)
                    return ret;

                memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
                if(ret == CAN_ERR_FORMAT)
                    sprintf(g_tx_buf, "ERR) CAN FORMAT is wrong...\r\n");
                else if(ret == CAN_ERR_ID_OVERFLOW)
                    sprintf(g_tx_buf, "ERR) CAN ID must be lower than 0x800...\r\n");

                ret = send(sn, g_tx_buf, strlen(g_tx_buf));
                if(ret < 0)
                {
                    close(sn);
                    return ret;
                }	
            }

            do {
                
                ret = pop_rbuf(&recv_msg);

                if(ret < 0)
                    break;

#if 1
                printf("\t0X%x\t%lu\t", recv_msg.id, recv_msg.dlc);

                for(uint32_t idx = 0; idx < recv_msg.dlc; idx++)
                {
                    printf("0x%x ", recv_msg.data[idx]);
                }
                printf("\n");
#endif
                memset(g_tx_buf, 0x0, sizeof(g_tx_buf));
                tx_buf_idx = sprintf(&g_tx_buf[0], "%0X,", recv_msg.id);
                tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " [%u]", recv_msg.dlc);
                for(uint32_t i = 0; i < recv_msg.dlc; i++)
                    tx_buf_idx += sprintf(&g_tx_buf[tx_buf_idx], " %0X", recv_msg.data[i]);

                g_tx_buf[tx_buf_idx] = '\n';

                //printf("TX buf(%u) : %s\n", strlen(g_tx_buf), g_tx_buf);
                ret = send(sn, g_tx_buf, strlen(g_tx_buf));
                if(ret < 0)
                {
                    close(sn);
                    return ret;
                }
            } while(1);
            
            break;

        case SOCK_CLOSE_WAIT :
#ifdef _CAN_TO_ETH_DEBUG_
            //printf("%d:CloseWait\r\n",sn);
#endif
            if((ret=disconnect(sn)) != SOCK_OK) return ret;
#ifdef _CAN_TO_ETH_DEBUG_
            printf("%d:Socket Closed\r\n", sn);
#endif
            break;

        case SOCK_INIT :
#ifdef _CAN_TO_ETH_DEBUG_
            printf("%d:Try to connect to the %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
            if( (ret = connect(sn, destip, destport)) != SOCK_OK) return ret;	//	Try to TCP connect to the TCP server (destination)
            can_rx_rbuf.push_idx = 0;
            can_rx_rbuf.pop_idx = 0;
            memset(can_rx_rbuf.msg_buf, 0x0, BUF_MAX_SIZE * sizeof(struct can2040_msg));
            can_rx_rbuf.rx_flag = true;
            break;

        case SOCK_CLOSED:
            close(sn);
            if((ret=socket(sn, Sn_MR_TCP, any_port++, 0x00)) != sn){
            if(any_port == 0xffff) any_port = 50000;
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
