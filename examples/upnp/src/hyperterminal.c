
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket.h"
#include "wizchip_conf.h"
#include "hyperterminal.h"
#include "UPnP.h"
#include "MakeXML.h"
#include "pico/time.h"
#include "loopback.h"
#include "pico/stdlib.h"
#include "w5x00_spi.h"
//#include "bsp_spi.h"


typedef enum
{
    RESET = 0,
    SET = !RESET
} FlagStatus,
    ITStatus;

onboard_led_f UserLED_ctl_cb = NULL;

uint8_t gMsgBuf[20];

/**
 * @brief   LED Control init func
 * @param   fun_cb function pointer
 * @return  none
*/
void UserLED_Control_Init(onboard_led_f fun_cb)
{
    if (fun_cb != NULL)
    {
        UserLED_ctl_cb = fun_cb;
    }
    else
    {
        printf("No correlation function is registered!\r\n");
    }
}

// uint16_t UDP_LISTEN_PORT;
// uint16_t num;
// uint16_t num1;
/*******************************************************************************
 * Function Name  : Int2Str
 * Description    : Convert an Integer to a string
 * Input          : - str: The string
 *                  - intnum: The intger to be converted
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Int2Str(char *str, uint32_t intnum)
{
    int i, Div = 1000000000, j = 0, Status = 0;
    for (i = 0; i < 10; i++)
    {
        str[j++] = (intnum / Div) + 48;
        intnum = intnum % Div;
        Div /= 10;
        if ((str[j - 1] == '0') & (Status == 0))
        {
            j = 0;
        }
        else
        {
            Status++;
        }
    }
}

/*******************************************************************************
 * Function Name  : Str2Int
 * Description    : Convert a string to an integer
 * Input 1        : - inputstr: The string to be converted
 *                  - intnum: The intger value
 * Output         : None
 * Return         : 1: Correct
 *                  0: Error
 *******************************************************************************/
uint8_t Str2Int(char *inputstr, uint32_t *intnum)
{
    uint8_t i = 0, res = 0;
    uint32_t val = 0;
    if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
    {
        if (inputstr[2] == '\0')
        {
            return 0;
        }
        for (i = 2; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                res = 1; /* return 1; */
                break;
            }
            if (ISVALIDHEX(inputstr[i]))
            {
                val = (val << 4) + CONVERTHEX(inputstr[i]);
            }
            else
            {
                /* return 0; Invalid input */
                res = 0;
                break;
            }
        }
        if (i >= 11)
            res = 0; /* over 8 digit hex --invalid */
    }
    else /* max 10-digit decimal input */
    {
        for (i = 0; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                /* return 1; */
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
            {
                val = val << 10;
                *intnum = val;
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
            {
                val = val << 20;
                *intnum = val;
                res = 1;
                break;
            }
            else if (ISVALIDDEC(inputstr[i]))
                val = val * 10 + CONVERTDEC(inputstr[i]);
            else
            {
                /* return 0; Invalid input */
                res = 0;
                break;
            }
        }
        if (i >= 11)
            res = 0; /* Over 10 digit decimal --invalid */
    }
    return res;
}

/*******************************************************************************
 * Function Name  : GetIntegerInput
 * Description    : Get an integer from the HyperTerminal
 * Input          : - num: The inetger
 * Output         : None
 * Return         : 1: Correct
 *                  0: Error
 *******************************************************************************/
uint8_t GetIntegerInput(uint32_t *num)
{
    char inputstr[16];
    while (1)
    {
        GetInputString(inputstr);
        if (inputstr[0] == '\0')
            continue;
        if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0')
        {
            printf("User Cancelled \r\n");
            return 0;
        }
        if (Str2Int(inputstr, num) == 0)
        {
            printf("Error, Input again: \r\n");
        }
        else
        {
            return 1;
        }
    }
}

/*******************************************************************************
 * Function Name  : GetInputString
 * Description    : Get Input string from the HyperTerminal
 * Input          : - buffP: The input string
 * Output         : None
 * Return         : None
 *******************************************************************************/
void GetInputString(char *buffP)
{
    int bytes_read = 0;
    char c = 0;
    do
    {
        c = getchar_timeout_us(0xFFFFFFFF);
        if (c == '\r')
            break;
        if (c == '\b') /* Backspace */
        {
            if (bytes_read > 0)
            {
                printf("\b \b");
                bytes_read--;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE)
        {
            printf("Command string size overflow\r\n");
            bytes_read = 0;
            continue;
        }
        if (c >= 0x20 && c <= 0x7E)
        {
            buffP[bytes_read++] = c;
            printf("%c", c);
        }
    } while (1);
    printf("\n\r");
    buffP[bytes_read] = '\0';
}

uint8_t SerialKeyPressed(char *key)
{
    FlagStatus flag;
    uint8_t i = 0, c;

    c = getchar_timeout_us(0);
    if (c > 0)
    {
        // putchar_raw(c);
        *key = c;
        return SET;
    }
    else
    {
        return RESET;
    }
}

char *STRTOK(char *strToken, const char *strDelimit)
{
    static char *pCurrent;
    char *pDelimit;

    if (strToken != NULL)
        pCurrent = strToken;
    else
        strToken = pCurrent;

    //       if ( *pCurrent == NULL ) return NULL;

    while (*pCurrent)
    {
        pDelimit = (char *)strDelimit;
        while (*pDelimit)
        {
            if (*pCurrent == *pDelimit)
            {
                //*pCurrent = NULL;
                *pCurrent = 0;
                ++pCurrent;
                return strToken;
            }
            ++pDelimit;
        }
        ++pCurrent;
    }

    return strToken;
}

/*******************************************************************************
 * Function Name  : Main_Menu
 * Description    : Display/Manage a Menu on HyperTerminal Window
 * Input          : sn: use for SSDP; sn2: use for run tcp/udp loopback; sn3: use for listenes IGD event message 
 *                  buf: use for tcp/udp loopback rx/tx buff; tcps_port: use for tcp loopback listen; udps_port: use for udp loopback receive
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Main_Menu(uint8_t sn, uint8_t sn2, uint8_t sn3, uint8_t *buf, uint16_t tcps_port, uint16_t udps_port)
{
    static char choice[3];
    static char msg[256], ipaddr[12], protocol[4];
    static unsigned short ret, external_port, internal_port;
    static bool bTreat;
    static uint8_t Sip[4];
    static char key = 0;
    static wiz_NetInfo net_info;

    while (1)
    {
        /* Display Menu on HyperTerminal Window */
        bTreat = (bool)RESET;
        printf("\r\n====================== WIZnet Chip Control Point ===================\r\n");
        printf("This Application is basic example of UART interface with\r\n");
        printf("Windows Hyper Terminal. \r\n");
        printf("\r\n==========================================================\r\n");
        printf("                          APPLICATION MENU :\r\n");
        printf("\r\n==========================================================\r\n\n");
        printf(" 1 - Set LED on \r\n");
        printf(" 2 - Set LED off \r\n");
        printf(" 3 - Show network setting\r\n");
        printf(" 4 - Set  network setting\r\n");
        printf(" 5 - Run TCP Loopback\r\n");
        printf(" 6 - Run UDP Loopback\r\n");
        printf(" 7 - UPnP PortForwarding: AddPort\r\n");
        printf(" 8 - UPnP PortForwarding: DeletePort\r\n");

        printf("Enter your choice : ");
        GetInputString(choice);
        /* Set LD1 on */
        if (strcmp(choice, "1") == 0)
        {
            bTreat = (bool)SET;
            UserLED_ctl_cb(LED_OFF);
        }
        /* Set LD1 off */
        if ((strcmp(choice, "2") == 0))
        {
            bTreat = (bool)SET;
            UserLED_ctl_cb(LED_ON);
        }
        if (strcmp(choice, "3") == 0)
        {
            bTreat = (bool)SET;
            print_network_information(net_info);
        }

        if (strcmp(choice, "4") == 0)
        {
            bTreat = (bool)SET;
            wiz_NetInfo get_info;
            wizchip_getnetinfo(&get_info);
            // IP address
            printf("\r\nIP address : ");
            GetInputString(msg);
            if (!VerifyIPAddress(msg, get_info.ip))
            {
                printf("\aInvalid.");
            }

            // Subnet mask
            printf("\r\nSubnet mask : ");
            GetInputString(msg);
            if (!VerifyIPAddress(msg, get_info.sn))
            {
                printf("\aInvalid.");
            }

            // gateway address
            printf("\r\nGateway address : ");
            GetInputString(msg);
            if (!VerifyIPAddress(msg, get_info.gw))
            {
                printf("\aInvalid.");
            }

            // DNS address
            printf("\r\nDNS address : ");
            GetInputString(msg);
            if (!VerifyIPAddress(msg, get_info.dns))
            {
                printf("\aInvalid.");
            }

            get_info.dhcp = NETINFO_STATIC;

            wizchip_setnetinfo(&get_info);
        }

        if (strcmp(choice, "5") == 0)
        {
            bTreat = (bool)SET;

            printf("\r\nRun TCP loopback");
            printf("\r\nRun TCP loopback, port number [%d] is listened", tcps_port);
            printf("\r\nTo Exit, press [Q]\r\n");
            close(sn2);
            while (1)
            {

                if ((SerialKeyPressed((char *)&key) == 1) && (key == 'Q'))
                {
                    printf("\r\n Stop ");
                    break;
                }
                loopback_tcps(sn2, buf, tcps_port);
            }
        }

        if (strcmp(choice, "6") == 0)
        {
            bTreat = (bool)SET;

            printf("\r\nRun UDP loopback");
            printf("\r\nRun UDP loopback, port number [%d] is listened", udps_port);
            printf("\r\nTo Exit, press [Q]\r\n");
            close(sn2);
            while (1)
            {

                if ((SerialKeyPressed((char *)&key) == 1) && (key == 'Q'))
                {
                    printf("\r\n Stop ");
                    break;
                }
                loopback_udps(sn2, buf, udps_port);
            }
        }
        if (strcmp(choice, "7") == 0)
        {
            bTreat = (bool)SET;

            printf("\r\nType a Protocol(TCP/UDP) : ");
            GetInputString(msg);
            strncpy(protocol, msg, 3);
            protocol[3] = '\0';

            printf("\r\nType a External Port Number : ");
            GetInputString(msg);
            external_port = ATOI(msg, 10);

            printf("\r\nType a Internal Port Number : ");
            GetInputString(msg);
            internal_port = ATOI(msg, 10);
            tcps_port = internal_port;
            close(sn2);
            // Try to Add Port Action
            getSIPR(Sip);
            sprintf(ipaddr, "%d.%d.%d.%d", Sip[0], Sip[1], Sip[2], Sip[3]);
            if ((ret = AddPortProcess(sn, protocol, external_port, ipaddr, internal_port, "W5500_uPnPGetway")) == 0)
                printf("AddPort Success!!\r\n");
            else
                printf("AddPort Error Code is %d\r\n", ret);
        }

        if (strcmp(choice, "8") == 0)
        {
            bTreat = (bool)SET;

            printf("\r\nType a Protocol(TCP/UDP) : ");
            GetInputString(msg);
            strncpy(protocol, msg, 3);
            protocol[3] = '\0';

            printf("\r\nType a External Port Number : ");

            //			TCP_LISTEN_PORT=num;
            //			UDP_LISTEN_PORT=num;
            //			printf("%d\r\n",TCP_LISTEN_PORT);
            GetInputString(msg);
            external_port = ATOI(msg, 10);

            // Try to Delete Port Action
            if ((ret = DeletePortProcess(sn, protocol, external_port)) == 0)
                printf("DeletePort Success!!\r\n");
            else
                printf("DeletePort Error Code is %d\r\n", ret);
        }

        /* OTHERS CHOICE*/
        if (bTreat == (bool)RESET)
        {
            printf(" wrong choice  \r\n");
        }

        eventing_listener(sn3);

    } /* While(1)*/
} /* Main_Menu */

/*******************(C)COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
