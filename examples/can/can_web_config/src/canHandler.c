#include <stdio.h>
#include <stdlib.h>
#include "port_common.h"
#include "hardware/irq.h"
#include "hardware/structs/pio.h"

#include "can_parser.h"
#include "canHandler.h"
#include "w5x00_can_config.h"

// #define CAN_DEBUG

static CanConfig can_config;

static struct can2040 cbus;

volatile CAN_RX_RBUF can_rx_rbuf; 
static critical_section_t can_cris_sec;

// private funcs
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
static int is_pio_in_use(uint32_t pio_num);


CanConfig* get_CanConfig_pointer(void)
{
	return &can_config;
}

void get_default_can_config(CanConfig* p_can_config)
{
    memset(p_can_config, 0x0, sizeof(CanConfig));
    p_can_config->eth_mode = ETH_TCP_SERVER;           // TCP Server mode
    p_can_config->baudrate = BDRATE_125;               // 125kbps
}

void push_rbuf(struct can2040_msg *msg)
{
    critical_section_enter_blocking(&can_cris_sec);

    memcpy(&can_rx_rbuf.msg_buf[can_rx_rbuf.push_idx], msg, sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.push_idx < DATA_BUF_MAX_SIZE)
        can_rx_rbuf.push_idx++;
    else    
        can_rx_rbuf.push_idx = 0;

    critical_section_exit(&can_cris_sec);
}

int pop_rbuf(struct can2040_msg *msg)
{
    if(can_rx_rbuf.pop_idx == can_rx_rbuf.push_idx)
        return -1;
    
    critical_section_enter_blocking(&can_cris_sec);
    memcpy(msg, &can_rx_rbuf.msg_buf[can_rx_rbuf.pop_idx], sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.pop_idx < DATA_BUF_MAX_SIZE)
        can_rx_rbuf.pop_idx++;
    else    
        can_rx_rbuf.pop_idx = 0;

    critical_section_exit(&can_cris_sec);
    return 0;
}

int can_initialize(CanConfig *p_can_config)
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

    switch(p_can_config->baudrate)
    {
        case BDRATE_125:
            can2040_start(&cbus, clock_get_hz(clk_sys), 125000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        case BDRATE_250:
            can2040_start(&cbus, clock_get_hz(clk_sys), 250000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        case BDRATE_500:
            can2040_start(&cbus, clock_get_hz(clk_sys), 500000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        default:
            break;
    }

    return 0;
}

int can_restart(CanConfig *p_can_config)
{
    can2040_stop(&cbus);

    switch(p_can_config->baudrate)
    {
        case BDRATE_125:
            can2040_start(&cbus, clock_get_hz(clk_sys), 125000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        case BDRATE_250:
            can2040_start(&cbus, clock_get_hz(clk_sys), 250000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        case BDRATE_500:
            can2040_start(&cbus, clock_get_hz(clk_sys), 500000, CAN_RX_PIN, CAN_TX_PIN);
            break;
        default:
            break;
    }
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

void PIOx_IRQHandler(void)
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

void printCanConfig(CanConfig *p_can_config)
{
    printf("\n------------ CAN config ------------\n");

    switch(p_can_config->eth_mode)
    {
        case ETH_TCP_SERVER:
            printf("\tmode : TCP Server\n");
            break;
        case ETH_TCP_CLIENT:
            printf("\tmode : TCP Client\n");
            break;
        default:
            break;
    }

    switch (p_can_config->baudrate)
    {
        case BDRATE_125:
            printf("\tbaudrate : 125kbps\n");
            break;
        case BDRATE_250:
            printf("\tbaudrate : 250kbps\n");
            break;
        case BDRATE_500:
            printf("\tbaudrate : 500kbps\n");
            break;
        default:
            printf("\tErr) baudrate : %u\n", p_can_config->baudrate);
            break;
    }

    printf("\n-------------------------------------\n");
}