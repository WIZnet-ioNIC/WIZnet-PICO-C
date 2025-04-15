#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "w5x00_can_config.h"
#include "httpHandler.h"

void make_json_devinfo(uint8_t * buf, uint16_t * len)
{
	CanConfig *p_can_config = get_CanConfig_pointer();

	*len = sprintf((char *)buf, "DevinfoCallback({\"opmode\":\"%d\","\
                                               "\"baud\":\"%d\","\
                                               "});",
		
		p_can_config->eth_mode,
		p_can_config->baudrate
    );
}

uint8_t set_devinfo(uint8_t * uri)
{
	uint8_t buf[8];
	uint8_t ret = 0;
	uint8_t * param;
	uint8_t str_size;
	CanConfig *can_config = get_CanConfig_pointer();
	uint8_t uart_sel = 0;
  
	if((param = get_http_param_value((char *)uri, "opmode", (char*)buf)))
	{
		uint8_t eth_mode = ATOI(param, 10);

		if(eth_mode < ETH_MODE_MAX)
			can_config->eth_mode = eth_mode;

		ret = 1;
	}

	if((param = get_http_param_value((char *)uri, "baud", (char*)buf)))
	{
		uint8_t baudrate_idx = ATOI(param, 10);
		if(baudrate_idx >= BDRATE_MAX) 
			baudrate_idx = BDRATE_125;

		can_config->baudrate = baudrate_idx;
		ret = 1;
	}

	return ret;
}
