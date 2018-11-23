

#ifndef __STM324x7I_EVAL_H
#define __STM324x7I_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32_eval_legacy.h"
#include "portmacro.h"
#include "FreeRTOS.h"    
#include "task.h"
#include "main.h"
#include "my_sys.h"

     
 
#define LED1_ON()           PDout(13)=1
#define LED1_OFF()          PDout(13)=0
#define LED1_TOG()          PDout(13)=~PDout(13)
#define LED2_ON()           PDout(14)=1
#define LED2_OFF()          PDout(14)=0
#define LED2_TOG()          PDout(14)=~PDout(14)
#define LED3_ON()           PCout(6)=1
#define LED3_OFF()          PCout(6)=0
#define LED3_TOG()          PCout(6)=~PCout(6)
#define LED4_ON()           PCout(7)=1
#define LED4_OFF()          PCout(7)=0
#define LED4_TOG()          PCout(7)=~PCout(7)
#define BEEN_ON()           PEout(9)=1
#define BEEN_OFF()          PEout(9)=0
#define RELAY_ON()          PDout(15)=1
#define RELAY_OFF()         PDout(15)=0


#define LED1_PIN                    GPIO_Pin_13                 
#define LED1_PORT                   GPIOD                       
#define LED1_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define LED2_PIN                    GPIO_Pin_14                 
#define LED2_PORT                   GPIOD                       
#define LED2_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define LED3_PIN                    GPIO_Pin_6                 
#define LED3_PORT                   GPIOC                      
#define LED3_GPIO_CLK               RCC_AHB1Periph_GPIOC

#define LED4_PIN                    GPIO_Pin_7                 
#define LED4_PORT                   GPIOC                       
#define LED4_GPIO_CLK               RCC_AHB1Periph_GPIOC

#define RELAY_PIN                    GPIO_Pin_15                 
#define RELAY_PORT                   GPIOD                       
#define RELAY_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define BEEP_PIN                    GPIO_Pin_9                 
#define BEEP_PORT                   GPIOE                       
#define BEEP_GPIO_CLK               RCC_AHB1Periph_GPIOE


#define RX485_PIN                    GPIO_Pin_11                 
#define RX485_PORT                   GPIOC                       
#define RX485_GPIO_CLK               RCC_AHB1Periph_GPIOC


#define TX485_PIN                    GPIO_Pin_8                 
#define TX485_PORT                   GPIOD                       
#define TX485_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define RTS485_PIN                    GPIO_Pin_14                 
#define RTS485_PORT                   GPIOB                       
#define RTS485_GPIO_CLK               RCC_AHB1Periph_GPIOB


#define FLASH_CS_PIN                    GPIO_Pin_0                 
#define FLASH_CS_PORT                   GPIOD                       
#define FLASH_CS_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define FLASH_MISO_PIN                    GPIO_Pin_4                 
#define FLASH_MISO_PORT                   GPIOB                       
#define FLASH_MISO_GPIO_CLK               RCC_AHB1Periph_GPIOB

#define FLASH_MOSI_PIN                    GPIO_Pin_12                 
#define FLASH_MOSI_PORT                   GPIOC                       
#define FLASH_MOSI_GPIO_CLK               RCC_AHB1Periph_GPIOC

#define FLASH_CLK_PIN                    GPIO_Pin_10                 
#define FLASH_CLK_PORT                   GPIOC                       
#define FLASH_CLK_GPIO_CLK               RCC_AHB1Periph_GPIOC


#define KEY1_PIN                    GPIO_Pin_9                 
#define KEY1_PORT                   GPIOD                       
#define KEY1_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define KEY2_PIN                    GPIO_Pin_10                 
#define KEY2_PORT                   GPIOD                       
#define KEY2_GPIO_CLK               RCC_AHB1Periph_GPIOD


#define KEY3_PIN                    GPIO_Pin_11                 
#define KEY3_PORT                   GPIOD                       
#define KEY3_GPIO_CLK               RCC_AHB1Periph_GPIOD

#define KEY4_PIN                    GPIO_Pin_12                 
#define KEY4_PORT                   GPIOD                       
#define KEY4_GPIO_CLK               RCC_AHB1Periph_GPIOD

#include "fifo_bytes.h"

#include "bytes_fifo_malloc.h"




#define MAX_ELE_PACKETS  150
extern STU_QUEUE_MALLOC stu_queue_malloc_tx;
extern STU_QUEUE_MALLOC stu_queue_malloc_rx;

void get_net_mac(uint8_t *macout);
void LED_Init(void);
void delay_ms1(u16 nms);

void Beep(void);

void task_app(void * pvParameters);

#define MAX_UART_LEN  1024
typedef struct
{
uint8_t addr;
uint8_t cmd;
uint8_t data[MAX_UART_LEN];
uint16_t len;

}STU_485_REV;
extern STU_485_REV Stu485rev;

#define CMD_FOOD_IN2000     2000
#define CMD_FOOD_OUT2002    2002
#define CMD_CTL_LED2004     2004
#define CMD_CTL_UI2006      2006
#define CMD_CTL_ZERO2008    2008
#define CMD_CTL_ONLINE2010  2010
#define CMD_RD_SENSORS2012  2012
#define CMD_ST_PERIOD2014	2014
#define CMD_GET_TIME_2016	2016



#define GATE_ACK31     0x31
#define GATE_FOOD_IN32     0x32
#define GATE_FOOD_OUT33    0x33
#define GATE_CTL_LED34     0x34
#define GATE_CTL_LCD35      0x35
#define GATE_CTL_ZERO36    0x36
#define GATE_RD_SENSORS37  0x37
#define GATE_RD_RESULT38  0x38
#define GATE_RD_INFO39  0x39
#define GATE_ST_PERIOD3A  0x3a

#define GET_NET_TIMER_3001  3001


//request information Answer
#define REQUEST_NET_INFOR_9000  9000
#define ANSWER_NET_INFOR_9001  9001

#define REQUEST_DEVICE_INFOR_9002  9002
#define ANSWER_DEVICE_INFOR_9003 9003

#define REQUEST_MQTT_INFOR_9004  9004
#define ANSWER_MQTT_INFOR_9005 9005


#define WAIT_PIG_TIME 7
#define MAX_DEVICE 128

#define LIST_SIZE 255
#define OFFLINE_DEVICE_ID  0xff


#define ASK_FOOD_RESULT_TIMEOUT_SEC 1200

#define ASK_FOOD_RESULT_TIMEOUT_MS (1000*ASK_FOOD_RESULT_TIMEOUT_SEC)


extern volatile uint8_t open_relay;


#define DEVICE_NOT_ONLINE		8
#define DEVICE_NO_FOOD			9
#define DEVICE_MOTOR_BROKEN		100
#define DEVICE_MOTOR_CARDHOLD	101
#define DEVICE_FOOD_NOT_MOVE	102
//#define DEVICE_TIME_OUT 12

bool wait_485_cmd(uint8_t cmd,uint16_t ms,uint8_t id);

void prepare_wait_485_cmd(uint8_t addr);

  
#ifdef __cplusplus
}
#endif

#endif /* __STM324x7I_EVAL_H */


