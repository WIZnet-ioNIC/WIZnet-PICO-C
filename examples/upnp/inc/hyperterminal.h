#ifndef _HYPERTERMINAL_H
#define _HYPERTERMINAL_H
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"

/* Exported types ------------------------------------------------------------*/
// ypedef  void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE 128
/* Exported macro ------------------------------------------------------------*/
#define IS_AF(c) ((c >= 'A') && (c <= 'F'))
#define IS_af(c) ((c >= 'a') && (c <= 'f'))
#define IS_09(c) ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c) IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c) IS_09(c)
#define CONVERTDEC(c) (c - '0')
#define CONVERTHEX_alpha(c) (IS_AF(c) ? (c - 'A' + 10) : (c - 'a' + 10))
#define CONVERTHEX(c) (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))


#define LED_ON  0
#define LED_OFF 1

typedef void(*onboard_led_f)(uint8_t status);
void UserLED_Control_Init(onboard_led_f fun_cb);

/* Exported functions ------------------------------------------------------- */
void Int2Str(char *str, uint32_t intnum);
uint8_t Str2Int(char *inputstr, uint32_t *intnum);
uint8_t GetIntegerInput(uint32_t *num);
uint8_t SerialKeyPressed(char *key);
char GetKey(void);
void SerialPutChar(char c);
void SerialPutString(char *s);
void GetInputString(char *buffP);
void Main_Menu(uint8_t sn, uint8_t sn2, uint8_t sn3, uint8_t* buf, uint16_t tcps_port, uint16_t udps_port);



#endif /* _HYPERTERMINAL_H */

/*******************(C)COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
